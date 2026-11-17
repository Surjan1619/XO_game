#ifndef EVENT_LOOPS_H
#define EVENT_LOOPS_H

#include "../AppModels.h"


User create_user(int fd);
void broadcast_message(char * msg, User * user_arr, int arr_size);
int game_loop(User * users);
void prepare_game(User * users);



#endif