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
#include <unistd.h>
#include <arpa/inet.h>
//loop to het all info from user 
//and gcreating struct for him

User create_user(int fd){
    User user = {0};
    user.fd = fd;
    char buff[BUFFSIZE] = {0};
    strcpy(buff, "please enter your name");
    int send_res = send_msg(fd, buff);
    message_rc_handler(send_res, SEND, fd);
    memset(buff, 0, BUFFSIZE);
    //reciving the username
    int recv_res = recv_msg(fd, buff);
    message_rc_handler(recv_res, RECV, fd);
    int username_size = 29;//saveing the username
    strncpy(user.username, buff, username_size);
    
    while(1){
        memset(buff, 0, BUFFSIZE);
        strcpy(buff, "enter the map size");
        
        send_res = send_msg(fd, buff);
        message_rc_handler(send_res, SEND, fd);

        recv_res = recv_msg(fd, buff);
        message_rc_handler(recv_res, RECV, fd);
        printf("%s\n", buff);
        if (!strncmp(buff, "3", 1)){
            user.map_size = 3;
            strcpy(buff, "waiting for a new clients to create game room\n . . . ");
            send_res = send_msg(fd, buff);
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
        int send_res = send_msg(user_arr[i].fd, msg);
        message_rc_handler(send_res, SEND, user_arr[i].fd);
    }
}
//chech if user sent number and convert it into int
int str_convert_numset(char * msg, int msglen){
    printf("str_conv....() : %s\n", msg);
    char * end;
    int msg_set = strtol(msg, &end, 10);
    if (msg == end){//if  pointers are equal that means player sended not number value
        return -1;
    }
    return msg_set;
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
    Map map = create_map(users, 2, map_size);
    char ** matrix = init_matrix(map_size);
    //send every user it's sign X or O and then broadcast message START to make 
    //them turn into "game started mode" 
    
    //sending signs
    int sended = send_msg(users[0].fd, "UX");
    message_rc_handler(sended, SEND, users[0].fd);
    sended = send_msg(users[1].fd, "UO");
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
                if (fds[i].revents & POLLIN){//find out who sended message
                    int player_idx = find_usr_by_fd(users, 2, fds[i].fd);
                    User * player = &users[player_idx];//player means the user who send a signal
                    User * opponent = &users[1 - player_idx];
                    //check is message sent a player who not supposted to play not
                    //send him approack message
                    if (map.turn != player -> sign){
                        char * msg = "not your turn";
                        int send_res = send_msg(player ->fd, msg);
                        message_rc_handler(send_res, SEND, player -> fd);
                        //player turn is not now can send message to his opponent
                        recv_msg(fds[i].fd, buff);
                        continue;
                    }
                    int read_res = recv_msg(fds[i].fd, buff);
                    message_rc_handler(read_res, RECV, fds[i].fd);
                    int step_set = str_convert_numset(buff, strlen(buff));
                    char numstr[16];
                    strncpy(numstr, buff, 16);
                    printf("sended number - %d\n", step_set);
                    if (step_set < 0){
                        int send_res = send_msg(player -> fd, "not valid format");
                        message_rc_handler(send_res, SEND, player -> fd);
                        continue;
                    }

                    else{//if everything is OK with text of the sended message, start to handle step
                        printf("_______________\n");
                        char * check_msg = user_make_step(&map, matrix, step_set, player ->sign);
                        char * winner = "WINNER\n";
                        char * draw = "DRAW\n";
                        char * waring = "WARING ";
                        if (check_msg){//check if there is a value in 'check_msg' that mean we find something
                            printf("%s\n", check_msg);
                            if (!strncmp(check_msg, winner, strlen(winner))){//if finded winer 
                                snprintf(buff, BUFFSIZE, "WINNER %c", player ->sign);
                                broadcast_message(buff, users, 2);    
                                printf("exiting . . .\n");
                                exit(0);
                            }
                            else if (!strncmp(check_msg, draw, strlen(draw))){//if players drawed 
                                broadcast_message(draw, users, 2);
                                printf("exiting . . .\n");
                                exit(0);
                            }
                            else if (!strncmp(check_msg, waring, strlen(waring))){//if player played in not valid position
                                send_msg(player ->fd, check_msg);
                                free(check_msg);
                                continue;
                            }
                        }
                        //if we passes all test and make server render the step
                        //we are sending the OK message to the sender(player) and 
                        //sending the player's step to his opponent 
                        send_msg(player ->fd, "OK\n");
                        send_msg(opponent ->fd, numstr);
                        free(check_msg);
                        map.turn = opponent ->sign;
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
    char buff[BUFFSIZE] = {};
    char * msg = "finded second player";
    strncpy(buff, msg, strlen(msg));
    broadcast_message(buff, users, 2);
    game_loop(users);

}

