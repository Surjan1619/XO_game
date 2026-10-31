
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
    printf("create_user(), user.fd-%d \n", user.fd);
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
    users[0].sign = 'x';
    users[1].sign = 'o';
    char buff[BUFFSIZE] = {0};
    struct pollfd fds[2] = {0};
    fds[0].fd = users[0].fd;
    fds[1].fd = users[1].fd;
    fds[0].events = POLLIN;
    fds[1].events = POLLIN;
    printf("got game loop\n");
    char turn = 0;
    while(1){
        int poll_res = poll(fds, 2, -1);
        if (poll_res < 0){ //if there is error while polling just exitin from the process
            perror("error while polling\n");
            exit(EXIT_FAILURE);
        }else{//if finded signal from poll handling it
            for (int i = 0; i < 2; ++i){
                if (fds[i].revents & POLLIN){
                    int sender_index = find_usr_by_fd(users, 2, fds[i].fd);
                    int recv_res = recv(users[sender_index].fd, buff, BUFFSIZE, 0);
                    message_rc_handler(recv_res, RECV, users[sender_index].fd);
                    buff[recv_res] = '\0';
                    char msg[BUFFSIZE * 2] = {0};
                    snprintf(msg, BUFFSIZE, "user %s-%s", users[sender_index].username, buff);
                    broadcast_message(buff, users, 2);
                    printf("%s\n", msg);
                }
            }



        }
        sleep(1);
        
    }
    return 1;
}


void prepare_game(User * users){
    char buff[BUFFSIZE];
    strcpy(buff, "finded second player\n");
    printf("oi oi oi\n");
    broadcast_message(buff, users, 2);
    printf("oi oi oi\n");
    game_loop(users);

}

