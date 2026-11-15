int check_winner(char ** matrix, int matrix_size){
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
            return 1;
        }
    }
    return 0;
    int i = 0, j = 0;
    if (matrix_size == 3){
        char target = matrix[i][j];
        char diagonal = 0;
        char op_diagonal = 0; //opposite diagonal
        while(i < matrix_size && j < matrix_size){
            if(matrix[i][j] == target){
                diagonal++;
            }
            else if (matrix[j][i] == target){
                op_diagonal++;
            }
            else{
                return 0;
            }
        }
        if (diagonal == matrix_size || op_diagonal == matrix_size){
            return 1;
        }
    }else{/*if matrix size if more than 3 implementing another algorythm to 
        check diagonals
        */
       return 0;//not implemented yet
    }

}

int main (){
    char  matrix[3][3] = {
        {'x', 'o', 'x'},
        {'o', 'x', 'o'},
        {'o', 'o', 'x'}
    };
    
}