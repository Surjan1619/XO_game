#include "map_handler.h"
#include <stdlib.h>
#include <stdio.h>
#include "../AppModels.h"
#include <limits.h>
int user_free_step_handler(Map * map, int step){//check is the layer free while user is making a step
    User * steper = {};
    User * another_user = {};
    if (map -> turn == 'X'){
        steper = map -> X;
        another_user = map -> O;
    }else{
        steper = map -> O;
        another_user = map -> X;
    }
    if (steper -> step_set & step){
        return 0;
    }
    if (another_user -> step_set & step){
        return 0;
    }
    //if all test are passes merging user sended set with whole set
    map -> map_set | step;
    return 1;//returning 1 if everything is ok
}

char check_winner_avilability(char ** matrix, int matrix_size){
    for (int i = 0 ; i < matrix_size; i++){
        char target = matrix[i][0];
        char row = 0;
        char col = 0;
        for (int j = 0; j< matrix_size; j ++){//interpriting j as a element of the line
            if (matrix[i][j] == target){ //and checking horizonal lines
                row++;
            }
            else if (matrix[j][i] == target){
                col++;
            }
            else{
                break;
            }
        }
        if (row == matrix_size || col == matrix_size){
            return target;
        }
    }
    int i = 0, j = 0;
    if (matrix_size == 3){
        char target = matrix[i][j];
        char diagonal = 0;
        char op_diagonal = 0; //opposite diagonal
        while(i < matrix_size && j < matrix_size){
            if(matrix[i][i] == target){
                diagonal++;
            }else if(matrix[i][matrix_size - i]){
                op_diagonal++;
            }
        }
        if (diagonal == matrix_size || op_diagonal == matrix_size){
            return target;
        }
    }else{/*if matrix size if more than 3 implementing another algorythm to 
        check diagonals
        */
       return 0;//not implemented yet
    }
    return 0;//returning 0 if not finded matches

}



/*
checking users map set and 
server map set to avoi desyncroniation
if users set doesnt equal whole mpa  that 
means that maps are desyncronized
*/
void check_map_and_users_set_sincron(Map * map){
    int map_set = map ->map_set;
    int usr1_set = map ->X -> step_set;
    int usr2_set = map ->O -> step_set;
    if (map_set != usr1_set | usr2_set){
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