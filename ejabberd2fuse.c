
#define FUSE_USE_VERSION 26 // latest version (fuse.h)

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <time.h>
#include "ejabberd.h"

static struct options {
  int show_help;
} options;

#define OPTION(t, p) \
  {t, offsetof(struct options, p), 1}

static const struct fuse_opt options_spec[] = {
  OPTION("-h", show_help),
  OPTION("--help", show_help),
  FUSE_OPT_END
};

static struct ejabberd_connection {
  int fd;
  char* status;
  char* connected_users;
  char* mnesia_system_info;
} ejabberd_connection;

struct ejabberd_connection* ec_get_ejabberd_connection() {
  struct fuse_context* fc = fuse_get_context();
  return (struct ejabberd_connection*)fc->private_data;
}

static void ec_free_status(struct ejabberd_connection* ec) {
  if (ec->status) {
    free(ec->status);
    ec->status = 0;
  }
}

static void ec_free_connected_users(struct ejabberd_connection* ec) {
  if (ec->connected_users) {
    free(ec->connected_users);
    ec->connected_users = 0;
  }
}

static void ec_free_mnesia_system_info(struct ejabberd_connection* ec) {
  if (ec->mnesia_system_info) {
    free(ec->mnesia_system_info);
    ec->mnesia_system_info = 0;
  }
}

static void ec_connect() {
  struct ejabberd_connection* ec = ec_get_ejabberd_connection();
  if (ec->fd < 0) {
    ec->fd = connect_to_ejabberd();
  }
  if (ec->fd < 0) {
    ec_free_status(ec);
    ec_free_connected_users(ec);
    ec_free_mnesia_system_info(ec);
    ec->status = allocate_string("failed connecting to ejabberd");
    ec->connected_users = allocate_string("failed connecting to ejabberd");
    ec->mnesia_system_info = allocate_string("failed connecting to ejabberd");
  }
}

static const char* ec_get_ejabberd_status() {
  ec_connect();
  struct ejabberd_connection* ec = ec_get_ejabberd_connection();
  if (ec->fd >= 0) {
    ec_free_status(ec);
    ec->status = get_ejabberd_status(ec->fd);
  }
  return ec->status;
}

static const char* ec_get_connected_users() {
  ec_connect();
  struct ejabberd_connection* ec = ec_get_ejabberd_connection();
  if (ec->fd >= 0) {
    ec_free_connected_users(ec);
    ec->connected_users = get_connected_users(ec->fd);
  }
  return ec->connected_users;
}

static const char* ec_get_mnesia_system_info() {
  ec_connect();
  struct ejabberd_connection* ec = ec_get_ejabberd_connection();
  if (ec->fd >= 0) {
    ec_free_mnesia_system_info(ec);
    ec->mnesia_system_info = get_mnesia_system_info(ec->fd);
  }
  return ec->mnesia_system_info;
}

enum ejabberd_file_type {
  EJABBERD_NONE,
  EJABBERD_DIR,
  EJABBERD_STATUS,
  EJABBERD_CONNECTED_USERS,
  EJABBERD_MNESIA_SYSTEM_INFO
};

static struct ejabberd_entry {
  const char* path;
  enum ejabberd_file_type file_type;
  const char* contents[3];
} ejabberd_entry;

static const struct ejabberd_entry directory[] = {
  {"/",               EJABBERD_DIR,                {"server", "user", 0}},
  {"/server",         EJABBERD_DIR,                {"mnesia", "status", 0}},
  {"/server/status",  EJABBERD_STATUS,             {0}},
  {"/server/mnesia",  EJABBERD_MNESIA_SYSTEM_INFO, {0}},
  {"/user",           EJABBERD_DIR,                {"connected", 0}},
  {"/user/connected", EJABBERD_CONNECTED_USERS,    {0}},
  {0,                 EJABBERD_NONE,               {0}}
};

static const struct ejabberd_entry* find_directory(const char* path) {
  int i;
  for (i = 0; directory[i].path != 0; ++i) {
    if (strcmp(path, directory[i].path) == 0) {
      return &directory[i];
    }
  }
  return 0;
}

static void* ejabberd2fuse_init(struct fuse_conn_info* conn) {
  (void)conn;
  struct ejabberd_connection* data = (struct ejabberd_connection*)malloc(sizeof(struct ejabberd_connection));
  data->fd = -1;
  data->status = 0;
  data->connected_users = 0;
  data->mnesia_system_info = 0;
  return data;
}

static void ejabberd2fuse_destroy(void* data) {
  if (data) {
    struct ejabberd_connection* ec = (struct ejabberd_connection*)data;
    ec_free_status(ec);
    ec_free_connected_users(ec);
    ec_free_mnesia_system_info(ec);
    free(data);
  }
}

static int ejabberd2fuse_getattr(const char* path, struct stat* stbuf) {
  int ok = 0;
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  const struct ejabberd_entry* entry = find_directory(path);
  if (entry) {
    switch (entry->file_type) {
      case EJABBERD_DIR:
        {
          stbuf->st_mode = S_IFDIR | 0755;
          stbuf->st_nlink = 2;
          stbuf->st_mtim = ts;
        }
        break;
      case EJABBERD_STATUS:
        {
          stbuf->st_mode = S_IFREG | 0444;
          stbuf->st_nlink = 1;
          stbuf->st_mtim = ts;
          stbuf->st_size = strlen(ec_get_ejabberd_status());
        }
        break;
      case EJABBERD_CONNECTED_USERS:
        {
          stbuf->st_mode = S_IFREG | 0444;
          stbuf->st_nlink = 1;
          stbuf->st_mtim = ts;
          stbuf->st_size = strlen(ec_get_connected_users());
        }
        break;
      case EJABBERD_MNESIA_SYSTEM_INFO:
        {
          stbuf->st_mode = S_IFREG | 0444;
          stbuf->st_nlink = 1;
          stbuf->st_mtim = ts;
          stbuf->st_size = strlen(ec_get_mnesia_system_info());
        }
        break;
      default:
        ok = -ENOENT;
    }
  } else {
    ok = -ENOENT;
  }
  return ok;
}

static int ejabberd2fuse_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
  (void) offset;
  (void) fi;
  int ok = 0;
  const struct ejabberd_entry* entry = find_directory(path);
  if (entry && entry->file_type == EJABBERD_DIR) {
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    int i;
    for (i = 0; entry->contents[i]; ++i) {
      filler(buf, entry->contents[i], NULL, 0);
    }
  } else {
    ok = -ENOENT;
  }
  return ok;
}

static int ejabberd2fuse_open(const char* path, struct fuse_file_info* fi) {
  int ok = 0;
  const struct ejabberd_entry* entry = find_directory(path);
  if (entry) {
    switch (entry->file_type) {
      case EJABBERD_STATUS:
      case EJABBERD_CONNECTED_USERS:
      case EJABBERD_MNESIA_SYSTEM_INFO:
        {
          if ((fi->flags & O_ACCMODE) != O_RDONLY) {
            ok = -EACCES;
          }
        }
        break;
      default:
        ok = -ENOENT;
    }
  } else {
    ok = -ENOENT;
  }
  return ok;
}

static int ejabberd2fuse_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
  (void)fi;
  const struct ejabberd_entry* entry = find_directory(path);
  if (!entry) {
    return -ENOENT;
  }
  if (entry->file_type != EJABBERD_STATUS && 
      entry->file_type != EJABBERD_CONNECTED_USERS &&
      entry->file_type != EJABBERD_MNESIA_SYSTEM_INFO) {
    return -ENOENT;
  }
  const char* contents = "";
  size_t len;
  if (entry->file_type == EJABBERD_STATUS) {
    contents = ec_get_ejabberd_status();
  } else if (entry->file_type == EJABBERD_CONNECTED_USERS) {
    contents = ec_get_connected_users();
  } else if (entry->file_type == EJABBERD_MNESIA_SYSTEM_INFO) {
    contents = ec_get_mnesia_system_info();
  }
  len = strlen(contents);
  if (offset < len) {
    if (offset + size > len) {
      size = len - offset;
    }
    memcpy(buf, contents + offset, size);
  } else {
    size = 0;
  }
  return size;
}

static struct fuse_operations ejabberd2fuse_oper = {
  .init    = ejabberd2fuse_init,
  .destroy = ejabberd2fuse_destroy,
  .getattr = ejabberd2fuse_getattr,
  .readdir = ejabberd2fuse_readdir,
  .open    = ejabberd2fuse_open,
  .read    = ejabberd2fuse_read
};

static void show_help(const char* progname) {
  printf("usage: %s [options] <mountpoint>\n\n", progname);
}

int ejabberd2fuse_main(int argc, char* argv[]) {
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
  if (fuse_opt_parse(&args, &options, options_spec, NULL) == -1) {
    return 1;
  }
  if (options.show_help) {
    show_help(argv[0]);
    assert(fuse_opt_add_arg(&args, "--help") == 0);
    args.argv[0] = "";
  }
  return fuse_main(args.argc, args.argv, &ejabberd2fuse_oper, NULL);
}
