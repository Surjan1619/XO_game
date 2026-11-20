#include "map_handler.h"
#include <stdlib.h>
#include <stdio.h>
#include "../AppModels.h"
#include <limits.h>
int user_free_step_handler(Map * map, int step){//check is the layer free while user is making a step
    if (map ->map_set & step){
        printf("not valid step,position is already in use\n");
        return 0;
    }
    printf("step & step: %i\n", map -> map_set & step);
    map -> map_set |= step;
    return 1;//returning 1 if everything is ok
}

char check_winner_avilability(char ** matrix, int matrix_size){
    int n = matrix_size;

    for (int i = 0; i < n; i++){
        char target = matrix[i][0];
        if (target != '#'  && target != ' '){
            int j = 1;
            while (j < n && matrix[i][j] == target) j++;
            if (j == n) return target;
        }

        target = matrix[0][i];
        if (target != '#'  && target != ' '){
            int j = 1;
            while (j < n && matrix[j][i] == target) j++;
            if (j == n) return target;
        }
    }

    char target = matrix[0][0];
    if (target != '#'  && target != ' '){
        int i = 1;
        while (i < n && matrix[i][i] == target) i++;
        if (i == n) return target;
    }

    target = matrix[0][n - 1];
    if (target != '#'  && target != ' '){
        int i = 1;
        while (i < n && matrix[i][n - 1 - i] == target) i++;
        if (i == n) return target;
    }

    return 0;
}


/*
checking users map set and 
server map set to avoi desyncroniation
if users set doesnt equal whole mpa  that 
means that maps are desyncronized
*/
void check_map_and_users_set_sincron(Map * map){
    printf("map_set-%i, x_set-%i, o_set-%i\n", map ->map_set, map->X->step_set, map->O->step_set);
    int map_set = map ->map_set;
    int usr1_set = map ->X -> step_set;
    int usr2_set = map ->O -> step_set;
    if (map_set != (usr1_set | usr2_set)){
        perror("syncronization error: map sets doesnt matching\n");
        exit(EXIT_FAILURE);
    }
    printf("INFO : maps are  syncrone\n");
}


int check_map_step_avilability(short map_set){
    short full_map_set = SHRT_MAX;
    printf("check_map_step_avilability()    map_set-%i\n", map_set);
    if (full_map_set == map_set){
        return 1;//1 means that every cell of the map is used and there is no avilable space
    }
    return 0;//zero means that the map is free
}