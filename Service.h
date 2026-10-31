
//finding user by fd and returning its index in the array
int find_usr_by_fd(User * userlist, int list_size, int fd){
    for (int i = 0; i < list_size; ++i){
        if (userlist[i].fd == fd){
            return i;
        }
    }
    return -1;
}



