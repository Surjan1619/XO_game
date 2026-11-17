#include "event_loops.h"
#include "../AppModels.h"
#include "../static.h"
#include "../game/map_service.h"
#include "../conn/conn_handler.h"
#include "tools.h"
#include <string.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>

//loop to het all info from user 
//and gcreating struct for him

User create_user(int fd){
    User user = {0};
    user.fd = fd;
    char buff[BUFFSIZE] = {0};
    strcpy(buff, "please enter your name\n");
    int send_res = send(fd, buff, strlen(buff), 0);
    message_rc_handler(send_res, SEND, fd);
    memset(buff, 0, BUFFSIZE);
    //reciving the username 
    int recv_res = recv(fd, buff, BUFFSIZE, 0);
    message_rc_handler(recv_res, RECV, fd);
    int username_size = 29;//saveing the username
    strncpy(user.username, buff, username_size);
    
    while(1){
        memset(buff, 0, BUFFSIZE);
        strcpy(buff, "enter the map size");
        
        send_res = send(fd,buff, strlen(buff), 0);
        message_rc_handler(send_res, SEND, fd);

        recv_res = recv(fd, buff, BUFFSIZE, 0);
        message_rc_handler(recv_res, RECV, fd);
        buff[recv_res] = '\0';
        printf("%s\n", buff);
        if (!strcmp(buff, "3\n")){
            user.map_size = 3;
            strcpy(buff, "got map result\n");
            send_res = send(fd, buff, strlen(buff), 0);
            message_rc_handler(send_res, SEND, fd);
            break;
        }
    }
    printf("create_user(), user.fd-%d user.username-%s\n", user.fd, user.username);
    return user;
}



//sending message to the group of users 
void broadcast_message(char * msg, User * user_arr, int arr_size){
    for(int i = 0; i < arr_size; ++i){
        int send_res = send(user_arr[i].fd, msg, sizeof(msg), 0);
        message_rc_handler(send_res, SEND, user_arr[i].fd);
    }
}


int game_loop(User * users){
    // !  !  !
    //change the logic of the users 
    //users array is in the temp. stack of the main func.
    users[0].sign = 'X';
    users[1].sign = 'O';
    char buff[BUFFSIZE] = {0};
    struct pollfd fds[2] = {0};
    fds[0].fd = users[0].fd;
    fds[1].fd = users[1].fd;
    fds[0].events = POLLIN;
    fds[1].events = POLLIN;

    int map_size;
    if (users[0].map_size == users[1].map_size){
        map_size = users[0].map_size;
    }
    else{
        printf("map sizes dont matching\n");
        exit(-1);
    }
    char turn = 'X';//first start to play X
    Map map = create_map(users, 2, map_size);
    char ** matrix = init_matrix(map_size);
    //send every user it's sign X or O and then broadcast message START to make 
    //them turn into "game started mode" 
    
    //sending signs
    int sended = send(users[0].fd, "UX", 3, 0);
    message_rc_handler(sended, SEND, users[0].fd);
    sended = send(users[1].fd, "UO", 3, 0);
    message_rc_handler(sended, SEND, users[1].fd);

    //broadcasting start signal
    char * start_signal = "START";
    broadcast_message(start_signal, users, 2);
    /*
    here is the main game loop
    where is writed the main game logic
    by using MapService.h functions
    */
    while(1){
        print_matrix(matrix, map_size);
        int poll_res = poll(fds, 2, -1);
        if (poll_res < 0){ //if there is error while polling just exitin from the process
            perror("error while polling\n");
            exit(EXIT_FAILURE);
        }else{//if finded signal from poll handling it
            for (int i = 0; i < 2; ++i){
                if (fds[i].revents & POLLIN){
                        int usr_idx = find_usr_by_fd(users, 2, fds[i].fd);//giving user fd to get its index in the users array
                        //chacking if can user make a step or no
                        //and if he could, handling message
                        if (turn == users[usr_idx].sign){
                            printf("your turn \n");
                            /*
                            player sended position is checking in 2 steps 

                            1)binary- checking user sended set with servers set to 
                            avoid duble step(when trying to play in the already busy position)

                            2)matrix- checking the position in the matrix to avoid duble step
                            */
                            int recv_res = recv(users[usr_idx].fd, buff, BUFFSIZE, 0);
                            message_rc_handler(recv_res, RECV, users[usr_idx].fd);
                            //handling and converting user sended message from string into decimal
                            int user_step = convert_str_to_int(buff);//returns int if converted and validate successfull
                            if (user_step < 1){//if sended unexpected fromat data
                                //handle case when convertins is aborted or sended message is not right fromat
                            }

                            //checking position avilability and changeing the matrix (first check)
                            char * handler = user_make_step(&map, matrix, user_step, users[usr_idx].sign);                            
                            
                            //handler possible messages
                            char * waring_msg = "WARING";
                            char * ok_message = "OK";
                            char * draw_msg = "DRAW";
                            if (handler){
                                        //if returns waring send handle the case and send user info
                                        //message occurs if the position where player wanna to step is already in use 
                                if (!strncmp(handler, waring_msg, strlen(waring_msg))){
                                    strcpy(buff, handler);
                                    int send_res = send(users[usr_idx].fd, buff, strlen(buff), 0);
                                    message_rc_handler(send_res, SEND, users[usr_idx].fd);
                                }
                                //handling if user wanna make allowed step and no finded problems
                                else if (!strncmp(handler, ok_message, strlen(ok_message))){
                                    int send_res = send(users[usr_idx].fd, ok_message, sizeof(ok_message), 0);
                                    message_rc_handler(send_res, SEND, users[usr_idx].fd);
                                    char  msg[3];
                                    snprintf(msg, 3, "%hd", user_step);
                                    //after approving player's step we need to send signal into another user about current users step
                                    //so we need to broadcast message to let everybody know about map changes
                                    broadcast_message(msg, users, 2);//where 2 is size of the users array 
                                    
                                }
                                else if (!strncmp(handler, draw_msg, strlen(draw_msg))){ //handling case when players drew
                                    broadcast_message(draw_msg, users, 2);
                                    break;
                                    
                                }
                            
                            }
                            else if (!handler){//if no finded error  and no winner yet VVV
                                users[usr_idx].step_set |= user_step;//if all tests passes mergeing users sended step
                                                                  //with his step set
                                merge_matrix_and_sets(&map, matrix);
                            }
                            /*
                            handle the step
                            */
                            free(handler);
                        }
                        else{
                            printf("not your turn\n");
                            int recv_res = recv(users[usr_idx].fd, buff, BUFFSIZE, 0);
                            message_rc_handler(recv_res, RECV, users[usr_idx].fd);
                                
                            broadcast_message(buff, users, 2);
                            continue;
                        }
                }
            }
        }
        printf("end of cycle in game_loop()\n");
        
    }
    free(matrix);
    return 1;
}


void prepare_game(User * users){
    char buff[BUFFSIZE];
    char * msg = "finded second player";
    strncpy(buff, msg, strlen(msg));
    broadcast_message(buff, users, 2);
    game_loop(users);

}

