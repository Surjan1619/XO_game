#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "AppModels.h"
#include "./event/event_loops.h"
#include "./conn/conn_handler.h"
#include "static.h"

/*
int mmain (){
    int sock_accepter = create_conn();
    struct pollfd fds[20] = {0}; 
    int poll_count = 0;
    fds[poll_count].fd = sock_accepter;
    fds[poll_count].events = POLLIN;
    User users[20] = {0};
    int user_count = 0;
    ++poll_count;
    while(1){
        poll(fds, poll_count, -1);
        if (fds[0].revents & POLLIN){
            int conn_fd = accept(sock_accepter, 0, 0);
            User user = create_user(conn_fd);
            users[user_count++] = user;//adding user into the user list
            
        }
        if (user_count == 2){
            printf("INFO:     forking\n");
            int pid = fork();
            if(pid > 0){//parent
                user_count = 0;
                memset(users, 0, 20);
                continue;


            }
            else if (pid == 0){//child
                prepare_game(users);
                exit(0);
            }
            else{//error
                
            }
        }
    }
    
}
*/
int main (){
    int sock_accepter = create_conn();
    struct pollfd fds[20] = {0}; 
    int poll_count = 0;
    fds[poll_count].fd = sock_accepter;
    fds[poll_count].events = POLLIN;
    poll_count++;
    User user_queue[20] = {0};
    int user_count = 0; 
    while(1){
        int pollres = poll(fds, poll_count, -1);
        if (pollres < 0){
            perror("error while polling\n");
            exit(EXIT_FAILURE);
        }
        if (fds[0].revents & POLLIN){
            int conn_fd = accept(sock_accepter, 0, 0);
            User user = create_user(conn_fd);//event where geting from user his information and initilizing structure 
            printf("%s \n", user.username);
            user_queue[user_count++] = user;
            fds[poll_count].fd = user.fd;
            fds[poll_count].events = POLLIN;
            poll_count++;

            if (poll_count >= 3){
                printf("forking\n");
                int pid = fork();
                if(pid > 0){//parent
                    close(user_queue[0].fd);
                    close(user_queue[1].fd);
                    user_count = 0;
                    poll_count = 1;
                    memset(user_queue, 0, 20);
                    continue;
                }
                else if (pid == 0){//child
                    prepare_game(user_queue);   //starting the game
                    exit(0);
                }
                else{//error
                    perror("error while forking\n");
                    exit(EXIT_FAILURE);
                }
            }
        }

        for (int i = 1; i < poll_count; i++){
            if (fds[i].revents & POLLIN){
                char buff[BUFFSIZE];
                if (recv_msg(fds[i].fd, buff) < 0){
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    continue;
                }
                printf("%s\n", buff);
                char * msg = "wating for a new clinets . . . ";
                int send_res = send_msg(fds[i].fd, msg);
                message_rc_handler(send_res, SEND, fds[i].fd);
            }
        }

    }
}
