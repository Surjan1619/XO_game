#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "map_service.h"
#include "map_handler.h"
#include "../AppModels.h"
#include "../static.h"
Map create_map(User * users, int user_count, int map_size){
    Map map = {};
    map.X = &users[0];
    map.O = &users[1];
    map.map_size = map_size;
    map.turn = 'X';
    return map;
}


char ** init_matrix(int map_size){//initilizing map and filling fields with whitespace to mark
    char ** base = (char**)malloc(map_size * sizeof(char));//as a free
    for (int i = 0; i < map_size; ++i){
        base[i] = (char *)malloc(map_size * sizeof(int));
        memset(base[i], '#', map_size);//temprary
        }
    return base;
}
//just ordinary matrix  mapping
int print_matrix(char ** map, int map_size){
    for(int i = 0 ; i < map_size;i++){
        for(int j = 0 ; j < map_size; j++){
            printf("%c ", map[i][j]);
        }
        printf("\n");
    }
}

//encoding user steps and passing values into the matrix
int merge_matrix_and_sets(Map * map, char ** matrix){
    //getting X and O step sets
    int x_step_set = map->X ->step_set;
    int o_step_set = map->O -> step_set;
    int map_size = map -> map_size;
    printf("map_size:%i\n", map ->map_size);
 
    /*
    map set contains 0 1 where 1 means position is busy and 0 means free
    and if the map size is 3 
    the last 3 bites mean last line of matrix
    the and from 9-th to 7-th bits mean first line
    and we are finding used positions by shifting
    and if we reach non zero value that means that position 
    is busy. to place correct sign( X or O) in that position
    we need to shit set and move j and i pointers sincronously
    */
    for (int i = map_size - 1; i > 0; i--){
        for (int j = map_size - 1; j > 0; j--){
            printf("x_ss:%i, y_ss:%i \n", x_step_set, o_step_set);
            if (x_step_set & 1){
                matrix[i][j] = 'X';
            }
            else if (o_step_set & 1){
                matrix[i][j] = 'O';
            }
            x_step_set >>= 1;
            o_step_set >>= 1;
        }
    }
    printf("merge_matrix_sets()\n");
    return 1;
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
int  binar_into_cordinates(int step,int map_size, int *x, int *y){
    printf("starting to convert the number %d\n", step);
    
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
                printf("number %i, x-%d, y-%d\n", step, *x, *y);
                return 1;
            }
            step >>= 1;
        }
    }
    return 1;
}
char * user_make_step(Map * map, char ** matrix, int step, char sign){//function return 0 if there is no errors occured
    char * buff = malloc(BUFFSIZE);
    if (!user_free_step_handler(map, step))//handling step to avoid rewriting users steps
    {
        strcpy(buff, "WARING  : unacceptable step\n the position is already in use:");
        return buff;//need to free this returned value
    }
    else{
        if (sign == 'X'){
            map -> X ->step_set |= step;
        }else if (sign = 'O'){
            map -> O ->step_set |=  step;
        }
        check_map_and_users_set_sincron(map);//map sync check
        //afters all tests we updating cheching user 
        //who make step and updating his map set
        if (sign == 'X'){
            map -> X ->step_set |= step;
        }else if (sign = 'O'){
            map -> O ->step_set |=  step;
        }
        map -> map_set |= (map ->X ->step_set | map -> O -> step_set);//mergeing users step set with map's set
        //mergeing map's set with map's matrix
        merge_matrix_and_sets(map, matrix);
        
        //check winner avilability and if winner finded
        //handling winner finded event loop

        //check if map is full 
        //if yes, that mean that players drew so we sending the right message
        
        if (check_map_step_avilability((short)map ->map_set )){//if map is full
            char * msg = "DRAW\n";
            strncpy(buff, msg, strlen(msg));
            return buff;        
        }
        int x,y;
        if (binar_into_cordinates(step, map ->map_size, &x, &y) < 0){
            printf("binar into codinate() unable to convert\n");
            exit(1);
        }
        matrix[x][y] = sign;
        printf("got here\n");
        char  result = check_winner_avilability(matrix, map ->map_size);
  
        if (result == 'X' || result == 'O'){
            strncpy(buff, "WINNER\n", 7);
            return buff;
        }
        //if no finded winner just ignore 

        return 0;
    }
}