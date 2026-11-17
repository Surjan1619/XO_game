#ifndef MAP_SERVICE_H
#define MAP_SERVICE_H

#include "../AppModels.h"
Map create_map(User * users, int user_count, int map_size);
char ** init_matrix(int map_size);
int print_matrix(char ** map, int map_size);
int merge_matrix_and_sets(Map * map, char ** matrix);
char * user_make_step(Map * map, char ** matrix, int step, char type);


#endif