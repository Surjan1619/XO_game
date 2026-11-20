#ifndef CONN_HANDLER_H
#define CONN_HANDLER_H

int create_conn();
void message_rc_handler(int return_code, int action, int session_fd);
int convert_str_to_int(char * str);
int send_msg(int fd, const char *text);
int recv_msg(int fd, char * text);
#endif