#ifndef MAP_HANDLER_H
#define MAP_HANDLER_H
#include "../AppModels.h"

int user_free_step_handler(Map * map, int step);
char check_winner_avilability(char ** matrix, int matrix_size);
void check_map_and_users_set_sincron(Map * map);
int check_map_step_avilability(short map_set);

#endif