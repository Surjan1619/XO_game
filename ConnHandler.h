#include "dependencies.h"


    /*
    SERVER CONFIGURATIONS
*/
int create_conn(){
    int sock_accepter = socket(AF_INET, SOCK_STREAM, 0);//creating sonnection accepter table
    if(sock_accepter < 0){ //avoiding from incorrect creation
        perror("error while creating socket file\n");
        exit(EXIT_FAILURE);
    }
    int opt = 1;
    if (setsockopt(sock_accepter, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
    //creating and initilazing the connection structure
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;  //ip v4 connection 
    addr.sin_addr.s_addr = INADDR_ANY;  //accepting connection from everywhere
    addr.sin_port = htons(PORT);   //changing port signature from host endian into network endian 

    int bind_result = bind(sock_accepter, (struct sockaddr *) &addr, sizeof(addr));
    if(bind_result < 0){//bindin socket file with port and let OS know our socket file as a connection reciver
        perror("error while binding socket file\n");
        exit(EXIT_FAILURE);
    }
    int listen_result = listen(sock_accepter, 5);
    if (listen_result < 0){
        perror("error while initilizing port\n");
        exit(EXIT_FAILURE);
    }
    return sock_accepter;
}


void message_rc_handler(int return_code, int action, int session_fd){
    if (action == RECV){
        if (return_code <= 0){
            perror("client closed connection or occured error while reciving\n");
            close(session_fd);
            exit(0);
        }
        else{
            printf("INFO: recived '%i' bytes\n", return_code);
        }
    }
    else if(action == SEND){
        if(return_code <= 0){
            perror("connection closed or occured error while sending\n");
            close(session_fd);
            exit(0);

        }
        else{
            printf("INFO: sended '%i' bytes pid-%d\n", return_code, getpid());
        }
    }
}

int convert_str_to_int(char * str){
    for (int i = 0; i < strlen(str); i++){
        if(!isdigit(str[i])){
            return -1;
        }
    }
    return atoi(str);
}