
#include <ei.h>
#include <erl_interface.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int get_nodename(const char* name, char nodename[], int len) {
  int ok = -1;
  char hostname[64];
  if (gethostname(hostname, sizeof(hostname)) == 0) {
    if (len >= strlen(name) + 1 + strlen(hostname) + 2) {
      sprintf(nodename, "%s@%s", name, hostname);
      ok = 0;
    }
  }
  return ok;
}

int read_cookie(char cookie[], int len) {
  FILE* fp = fopen(".erlang.cookie", "r");
  int ok;
  if (fp != 0) {
    ok = fgets(cookie, len, fp) == 0 ? -1 : 0;
    fclose(fp);
  } else {
    ok = -1;
  }
  return ok;
}

int connect_to_ejabberd() {
  erl_init(NULL, 0);
  char cookie[30];
  if (read_cookie(cookie, sizeof(cookie)) != 0) {
    fprintf(stderr, "could not read cookie");
    return -1;
  }
  if (!erl_connect_init(1, cookie, 0)) {
    erl_err_ret("connect init:");
    return -1;
  } 
  char ejabberd_nodename[100];
  if (get_nodename("ejabberd", ejabberd_nodename, sizeof(ejabberd_nodename)) != 0) {
    fprintf(stderr, "could not get nodename");
    return -1;
  }
  int fd = erl_connect(ejabberd_nodename);
  if (fd < 0) {
    fprintf(stderr, "connect: error\n");
    return -1;
  }
  return fd;
}

ETERM* reverse_list(ETERM* list) {
  ETERM* reversed = erl_mk_empty_list();
  while (!ERL_IS_EMPTY_LIST(list)) {
    ETERM* head = erl_hd(list);
    ETERM* tail = erl_tl(list);
    reversed = erl_cons(head, reversed);
    list = tail;
  }
  return reversed;
}

ETERM* flatten_list(ETERM* list) {
  ETERM* flattened = erl_mk_empty_list();
  while (!ERL_IS_EMPTY_LIST(list)) {
    ETERM* head = erl_hd(list);
    ETERM* tail = erl_tl(list);
    if (ERL_IS_LIST(head)) {
      ETERM* flat_head = flatten_list(head);
      while (!ERL_IS_EMPTY_LIST(flat_head)) {
        ETERM* flat_head_head = erl_hd(flat_head);
        flattened = erl_cons(flat_head_head, flattened);
        flat_head = erl_tl(flat_head);
      }
    } else {
      flattened = erl_cons(head, flattened);
    }
    list = tail;
  }
  return reverse_list(flattened);
}

ETERM* join_lists(ETERM* listoflists, int character) {
  ETERM* flattened = erl_mk_empty_list();
  while (!ERL_IS_EMPTY_LIST(listoflists)) {
    ETERM* head = erl_hd(listoflists);
    ETERM* tail = erl_tl(listoflists);
    if (ERL_IS_LIST(head)) {
      ETERM* flat_head = flatten_list(head);
      while (!ERL_IS_EMPTY_LIST(flat_head)) {
        ETERM* flat_head_head = erl_hd(flat_head);
        flattened = erl_cons(flat_head_head, flattened);
        flat_head = erl_tl(flat_head);
      }
    } else {
      flattened = erl_cons(head, flattened);
    }
    flattened = erl_cons(erl_mk_int(character), flattened);
    listoflists = tail;
  }
  return reverse_list(flattened);
}

char* allocate_string(const char* str) {
  char* copy = (char*)malloc(strlen(str) + 1);
  strcpy(copy, str);
  return copy;
}

char* eterm_to_string(ETERM* eterm) {
  FILE* temp = tmpfile();
  if (ERL_IS_LIST(eterm)) {
    fputs("[\n", temp);
    ETERM* list = eterm;
    while (!ERL_IS_EMPTY_LIST(list)) {
      ETERM* hd = erl_hd(list);
      fprintf(temp, "  ");
      erl_print_term(temp, hd);
      fprintf(temp, "\n");
      list = erl_tl(list);
    }
    fputs("]\n", temp);
  } else {
    erl_print_term(temp, eterm);
  }
  long len = ftell(temp);
  rewind(temp);
  char* str = (char*)malloc(len+1);
  int i = 0;
  int c;
  while ((c = fgetc(temp)) != EOF) {
    str[i++] = (char)c;
  }
  str[i] = '\000';
  fclose(temp);
  return str;
}

char* get_connected_users(int fd) {
  char* connected_users = 0;
  ETERM* result = erl_rpc(fd, "ejabberd_sm", "connected_users", erl_mk_empty_list());
  if (result != 0) {
    if (ERL_IS_LIST(result)) {
      ETERM* joined = join_lists(result, '\n');
      connected_users = erl_iolist_to_string(joined);
      erl_free_term(joined);
    }
    erl_free_term(result);
  }
  if (!connected_users) {
    connected_users = allocate_string("failed getting connected users");
  }
  return connected_users;
}

char* get_ejabberd_status(int fd) {
  char* ejabberd_status = 0;
  ETERM* result = erl_rpc(fd, "ejabberd_admin", "status", erl_mk_empty_list());
  if (result != 0) {
    if (ERL_IS_TUPLE(result) && ERL_TUPLE_SIZE(result) == 2) {
      ETERM* ok = erl_element(1, result);
      ETERM* status = erl_element(2, result);
      if (ERL_IS_ATOM(ok) && ERL_IS_LIST(status)) {
        ETERM* flattened = flatten_list(status);
        ejabberd_status = erl_iolist_to_string(flattened);
        erl_free_term(flattened);
      }
    }
    erl_free_term(result);
  } 
  if (!ejabberd_status) {
    ejabberd_status = allocate_string("failed getting ejabberd status");
  }
  return ejabberd_status;
}

char* get_mnesia_system_info(int fd) {
  char* mnesia_info = 0;
  ETERM* args = erl_cons(erl_mk_atom("all"), erl_mk_empty_list()); 
  ETERM* result = erl_rpc(fd, "mnesia", "system_info", args);
  erl_free_term(args);
  if (result != 0) {
    if (ERL_IS_LIST(result)) {
      mnesia_info = eterm_to_string(result);
      erl_free_term(result);
    }
  }
  if (!mnesia_info) {
   mnesia_info = allocate_string("failed getting mnesia info");
  }
  return mnesia_info;
}

