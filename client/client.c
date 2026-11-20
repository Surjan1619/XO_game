#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <poll.h>

#define BUFFSIZE 1024
#define PORT 8080
#define SEND 1
#define RECV 0



int print_matrix(char ** map, int map_size){
    for(int i = 0 ; i < map_size;i++){
        for(int j = 0 ; j < map_size; j++){
            printf("%c ", map[i][j]);
        }
        printf("\n");
    }
    printf("\n___________\n");
}

//FOR TEST
int init_step(char ** map,int map_size, int x, int y){//converting cordinate from x y fromat into binary
    int step = 1;
    for(int i = map_size - 1; i >= 0 ; i--){
        for(int j = map_size - 1; j >= 0; j--){
            if (i == x && j == y){
                return step;
            }    
            step <<= 1;
        }
    }
    return -1;
}

//chech if the sended step_set contains only one symbol to avoid unexpected cases
int validate_sended_step(int step){
    int numbers_count = 0;
    while(step){
        if (step & 1){
            ++numbers_count;
        }
        step >>= 1;
    }
    if (numbers_count != 1){
        return -1;//if after validating count of number '1' is not equal 1 returning error signal
    }
    return 1;
}

//decoding binary fromat sended message into x and y cordinates
int binar_into_cordinates(int step,int map_size, int *x, int *y){
    
    if (validate_sended_step(step) < 0){
        printf("ERROR: non valid step_set ::binar->cord\n");
        return -1;
    }
    for (int i = map_size - 1;i >= 0; i--){
        for (int j = map_size - 1; j >= 0; j--){
            if (step == 1){
                //saving the values of the x and y into user understanding fromat
                *x = i;
                *y = j;
                return 1;
            }
            step >>= 1;
        }
    }
    return 0;
}

int cordinate_into_binary(int * step, int map_size, int x, int y){
    *step = 1;
    //decreasing the values of the x and y because they are user understanding format 
    for (int i = map_size - 1; i >= 0 ; i--){
        for (int j = map_size - 1; j >= 0; j--){
            if (x == i && y == j){
                return 1;
            }
            *step <<= 1;
        }
    }
    if (validate_sended_step(*step) < 0){
        printf("ERROR: non valid step_set ::cord->binar\n");
        return -1;
    }
    return 1;
}

char ** init_matrix(int map_size){//initilizing map and filling fields with whitespace to mark
    char ** base = (char**)malloc(map_size * sizeof(char));//as a free
    for (int i = 0; i < map_size; ++i){
        base[i] = (char *)malloc(map_size * sizeof(int));
        memset(base[i], '#', map_size);
        }
    return base;
}



void message_rc_handler(int return_code, int action, int session_fd){
    if (action == RECV){
        if (return_code <= 0){
            perror("client closed connection or occured error while reciving\n");
            close(session_fd);
            exit(0);
        }
        else{
            //printf("INFO: recived '%i' bytes\n", return_code);
        }
    }
    else if(action == SEND){
        if(return_code <= 0){
            perror("connection closed or occured error while sending\n");
            close(session_fd);int convert_str_to_int(char * str);
            exit(0);

        }
        else{
            //printf("INFO: sended '%i' bytes pid-%d\n", return_code, getpid());
        }
    }
}
/*
this 2 function is protocol function
where first byte is header and means how many bytes contain the text*/
int send_msg(int fd, const char *text) {
    int text_len = strlen(text);
    if (text_len > 255) return -1;
    unsigned char len = (unsigned char)text_len;
    if (send(fd, &len, 1, MSG_NOSIGNAL) != 1) return -1;
    if (len && send(fd, text, len, MSG_NOSIGNAL) != len) return -1;
    return 1;
}

int recv_msg(int fd, char * text){
    unsigned char text_len;
    if (recv(fd, &text_len, 1, MSG_WAITALL) != 1) return -1;
    if (text_len && recv(fd, text, text_len, MSG_WAITALL) != text_len) return -1;
    text[text_len] = '\0';
    return 1;
}





int main(){
    //int test;
    //scanf("%i", &test);
    if (1){
        int sock_sender = socket(AF_INET, SOCK_STREAM, 0);//creating sonnection accepter table
        if(sock_sender < 0){ //avoiding from incorrect creation
            perror("error while creating socket file\n");
            exit(EXIT_FAILURE);
        }
        //creating and initilazing the connection structure
        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;  //ip v4 connection 
        addr.sin_port = htons(PORT);   //changing port signature from host endian into network endian 
        if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) {
            perror("Invalid address / Address not supported");
            close(sock_sender);
            exit(EXIT_FAILURE);
        }
        if (connect(sock_sender, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("Connection Failed");
            close(sock_sender);
            exit(EXIT_FAILURE);
        }

        char msg[BUFFSIZE] = {0};
        int recived = recv_msg(sock_sender, msg);
        message_rc_handler(recived, RECV, sock_sender);
        printf("%s\n", msg);
        struct pollfd fds[2] = {0};
        //here 1 is the stdin listener
        //and 0 is the server connection
        fds[0].fd = sock_sender;
        fds[0].events = POLLIN;
        fds[1].fd = STDIN_FILENO;
        fds[1].events = POLLIN;
        int map_size = 3;
        char ** matrix = init_matrix(map_size);
        bool game_start_status = false;
        char client_sign = ' ';
        char opponent_sign = ' ';
        int sended_x;
        int sended_y;
        /*
        in first server asks username and wanted map size
        and if server finds opponent its sending signal 
        UX or UO which menas you - X/O
        then server send message "STARTED"
        and changing status of the game_started_status into true
        then client can send to the server only cordinates x or y
        */
        while(1){
            int pollres = poll(fds, 2, -1);
            /*
            here if server dont started game yet
            client and server just sending each other 
            string messages, and when they start the game 
            they ll be abele to send each other only numbers
            */
            if (pollres && !game_start_status){
                if (fds[0].revents & POLLIN){ //if recived something from server
                    memset(msg, 0, BUFFSIZE);
                    int recived = recv_msg(sock_sender, msg);
                    message_rc_handler(recived, RECV, sock_sender);
                    printf("server message-%s\n", msg);
                    if (!strncmp(msg, "START", 5)){
                        printf("starting the game . . . \n");
                        game_start_status = true;
                    }
                    else if (!strncmp(msg, "UX", 2)){//UX means you are X
                        printf("you are X:\n");
                        client_sign = 'X';
                        opponent_sign = 'O';
                    }
                    else if(!strncmp(msg, "UO", 2)){//UO means you are O
                        printf("you are O:\n");
                        client_sign = 'O';
                        opponent_sign = 'X';
                    }

                }
                if (fds[1].revents & POLLIN){//if user wants to send a message
                    memset(msg, 0, BUFFSIZE);
                    int readed = read(STDIN_FILENO, msg, 255);
                    if (readed){
                        msg[strcspn(msg, "\n")] = '\0';
                        int send_res = send_msg(sock_sender, msg);
                        message_rc_handler(send_res, SEND, sock_sender);
                    }
                }
            }
            
            //code part where game already started
            else if (pollres && game_start_status){
                if (fds[0].revents & POLLIN){ //if recived something from server
                    memset(msg, 0, BUFFSIZE);
                    int recived = recv_msg(sock_sender, msg);
                    message_rc_handler(recived, RECV, sock_sender);
                    //NOTE : handle case when sened not number containing message
                    char * waring = "WARING";
                    if (!strncmp(msg, "WINNER", 6)){
                        //handle winner case
                        char winner;
                        char * garbage;
                        sscanf(msg, "%s %c", garbage, &winner);
                        printf("winner is %c\n", winner);
                        exit(0);   
                    }
                    else if (!strncmp(msg , "DRAW", 4)){
                        printf("draw, won friendship\n");
                        exit(0);
                    }
                    else if (!strncmp(msg, "OK", 2)){
                        printf("server approved step\n");
                        matrix[sended_x][sended_y] = client_sign;
                        print_matrix(matrix, map_size);
                        continue;
                    }
                    else if (!strncmp(msg, waring, strlen(waring))){
                        printf("position is already in using choose another\n");
                        continue;
                    }
                    else {//if  winner signal is not received continue the game
                        char * end;
                        int msg_set = strtol(msg, &end, 10);
                        if(validate_sended_step(msg_set) < 0){
                            continue;
                            //not ok, sended signal contains more than one '1'-s
                        }
                        int recived_x, recived_y;
                        binar_into_cordinates(msg_set, map_size, &recived_x, &recived_y);
                        matrix[recived_x][recived_y] = opponent_sign;
                        print_matrix(matrix, map_size);

                    }
                }
                if (fds[1].revents & POLLIN){//if user wants to send a message
                    memset(msg, 0, BUFFSIZE);
                    int readed = read(STDIN_FILENO, msg, 255);//reading users message from stdin
                    if (readed){
                        sscanf(msg, "%i %i", &sended_x, &sended_y);
                        int step_set = 1;
                        sended_x--;
                        sended_y--;
                        cordinate_into_binary(&step_set, map_size, sended_x, sended_y);
                        snprintf(msg, BUFFSIZE, "%d", step_set);
                        if (step_set > 256){
                            printf("invalid message\n");
                            continue;
                        }
                        int send_res = send_msg(sock_sender, msg);
                        message_rc_handler(send_res, SEND, sock_sender);
                    }
                }
            }

            
        }
    }
    else{
        /*
            TEST PART
        */
        int map_size = 3;
        char ** matrix = init_matrix(3);
        int x, y;
        while(1){
            printf("\n___\nenter numbers: ");
            scanf("%i %i", &x, &y);
            x--;
            y--;
            printf("here x-%d, y-%d", x, y);
            if (x >= map_size || y >= map_size){
                printf("size of map nor required\n no such a cordiante\n");
                exit(1);
            }
            printf("\n");
            int step;
            cordinate_into_binary(&step, map_size, x, y);
            printf("step_binary-%b\n", step);
            matrix[x][y] = 'X';
            //print_matrix(matrix, map_size);
            binar_into_cordinates(step, map_size, &x, &y);
            printf("after bin-cord x-%d, y-%d\n", x, y);

        }
    }
}