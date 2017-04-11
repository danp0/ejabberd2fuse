#ifndef EJABBERD_H_
#define EJABBERD_H_

extern int connect_to_ejabberd();
extern int get_nodename(const char* name, char nodename[], int len);
extern int read_cookie(char cookie[], int len);
extern char* allocate_string(const char* str);
extern char* get_connected_users(int fd);
extern char* get_ejabberd_status(int fd);
extern char* get_mnesia_system_info(int fd);

#endif
