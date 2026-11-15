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
int decode_opponent_step(int step,int map_size, int *x, int *y){
    *x = map_size - 1;
    *y = map_size - 1;
    for (*x; *x >=0; *x--){
        for(*y; *y >= 0; *y--){
            if (step == 1){
                return 1;
            }
        }
    }
    return 0;
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
            close(session_fd);
            exit(0);

        }
        else{
            //printf("INFO: sended '%i' bytes pid-%d\n", return_code, getpid());
        }
    }
}





int main(){
    int test;
    scanf("%i", &test);
    if (!test){
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
        int recived = recv(sock_sender, msg, BUFFSIZE, 0);
        msg[recived] = '\0';
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
                    int recived = recv(sock_sender, msg, BUFFSIZE, 0);
                    message_rc_handler(recived, RECV, sock_sender);
                    printf("server message-%s\n", msg);
                    if (!strncmp(msg, "START", 5)){
                        game_start_status = true;
                    }
                    if (!strcmp(msg, "UX")){//UX means you are X
                        client_sign = 'X';
                        opponent_sign = 'O';
                    }
                    else if(!strcmp(msg, "UO")){//UO means you are O
                        client_sign = 'O';
                        opponent_sign = 'X';
                    }
                }
                if (fds[1].revents & POLLIN){//if user wants to send a message
                    memset(msg, 0, BUFFSIZE);
                    int readed = read(STDIN_FILENO, msg, BUFFSIZE);
                    if (readed){
                        int send_res = send(sock_sender, msg, strlen(msg), 0);
                        message_rc_handler(send_res, SEND, sock_sender);
                    }
                }
            }
            
            //code part where game already started
            else if (pollres && game_start_status){
                if (fds[0].revents & POLLIN){ //if recived something from server
                    int recived = recv(sock_sender, msg, BUFFSIZE, 0);
                    message_rc_handler(recived, RECV, sock_sender);
                    printf("server message-%s\n", msg);
                    //NOTE : handle case when sened not number containing message
                    if (!strncmp(msg, "WINNER", 6)){
                        //handle winner case 
                    }

                    
                    /*when user send step to the server 
                    server needs to check his step validate and decide
                    approve or not, and when he approves the step, 
                    we need to add it into our map*/
                    else if (!strncmp(msg, "OK", 3)){
                        //if server approves the step we just rendering it in the our map
                        matrix[sended_x][sended_y] = client_sign;
                        printf("INFO:   accepted user sign -'%c' in position'x:%i y:%i'\n", client_sign, sended_x, sended_y);
                        print_matrix(matrix, map_size);
                    }
                    else if (strncmp(msg , "DRAW", 4)){
                        printf("daw, won friendship\n");
                        exit(0);
                    }
                    else {//if no winner signal is received continue the game

                        int server_sended_step;
                        sscanf(msg, "%i", &server_sended_step);
                        int x, y;//definig cordinates to give them value later;
                        
                        //giving x  and y to the function "decode_opponent_step()"to initilize their values
                        //and validating the step to avoid non real values
                        if (decode_opponent_step(server_sended_step, map_size, &x, &y)){
                            
                            
                            // ' '-is for player mode and '#' is for tests 
                            if (matrix[x][y] == ' '  || matrix[x][y] == '#'){//check if there is a free position and then make move
                                matrix[x][y] = opponent_sign;
                            }
                            else{
                                printf("WARING:     this position is already using\n");
                                //handle case when server sends non valid(buisy) cordinate
                            }
                        }
                        else{
                            //handle case when occured error while decoding position binary fromat
                            printf("WARING:  error while decoding position binary fromat\n");
                        }
                    }
                }
                if (fds[1].revents & POLLIN){//if user wants to send a message
                    memset(msg, 0, BUFFSIZE);
                    int readed = read(STDIN_FILENO, msg, BUFFSIZE);//reading users message from stdin
                    if (readed){
                        sscanf(msg, "%i %i", &sended_x, &sended_y);
                        int send_res = send(sock_sender, msg, strlen(msg), 0);
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
            if (x >= map_size || y >= map_size){
                printf("size of map nor required\n no such a cordiante\n");
                exit(1);
            }
            printf("\n");
            int step = init_step(matrix, map_size, x, y);
            printf("step-%b\n", step);
            matrix[x][y] = 'X';
            print_matrix(matrix, map_size);
        }
    }
}