#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#define ROWS 6
#define COLS 7

#define true (0==0)
#define false !true

void draw_board(int**);
char player_symbol(int);
int** alloc_board();
void free_board(int**);
int get_turn(int**, int);
int push_token(int**, int, int);
int check_win(int**, int, int);
int connect_four(int**, int, int);
void end_game(int);

int main(){
	int** board;
	int game_over = -1;
	int player = 0;

	board = alloc_board();
	draw_board(board);
	while(game_over == -1){
		player = player % 2 + 1;
		int column = get_turn(board, player);
		if(column < 0){
			game_over = player % 2 + 1;
			break;
		}
		int row = push_token(board, player, column);
		if(check_win(board, row, column)){
			game_over = player;
		}
		draw_board(board);
	}
	end_game(game_over);
	free_board(board);
}


/* Memory mapping for the board */
int** alloc_board(){  
	int** board;  
	board = (int**) malloc(ROWS*sizeof(int*));  
	for (int i = 0; i < ROWS; i++){
		board[i] = (int*) malloc(COLS*sizeof(int));
		for(int j = 0; j < COLS; j++){
			board[i][j] = 0;
		}
	}
	return board;  
} 

void free_board(int** board){
	for(int i = 0; i < ROWS; i++){
		free(board[i]);
	}
	free(board);
}

/* Graphic interface */
void draw_board(int** board){
	putc('\n', stdout);
	for(int i = 0; i < ROWS; i++){
		for(int j = 0; j < COLS; j++){
			putc(player_symbol(board[i][j]), stdout);
		}
		putc('\n', stdout);
	}
	for(int j = 0; j < COLS; j++){
		putc(49+j, stdout);
	}
	printf("\n\n");
}

char player_symbol(int player){
	char symbol;
	switch(player){
		case 1: symbol = 'O'; break;
		case 2: symbol = 'X'; break;
		default: symbol = '-'; break;
	}
	return symbol;
}

void end_game(int condition){
	if(condition == 0){
		printf("Game is tied");
	} else {
		printf("The winner is player %d (%c)", condition, player_symbol(condition));
	}
	printf(", thanks for playing!\n");
}


/* User input */
int get_turn(int** board, int player){
	do {
		printf("Player %d's turn, select a column (1-%d): ", player, COLS);
		int ch;
		int column;
		while((ch = getchar()) != '\0' && ch != '\n'){
			column = ch - '0' - 1;
		}
		if(column >= 0 && column < COLS){
			if(board[0][column] == 0){
				return column;
			} else {
				printf("That column is full. Choose again.\n");
			}
		} else if(column == -1){
			printf("Player %d gives up. ", player);
			return column;
		} else {
			printf("It must be a number between 1 and %d\n", COLS);
		}
	} while(true);
}

/* Player logic */
int push_token(int** board, int player, int column){
	for(int i = ROWS-1; i >= 0; i--){
		int position = board[i][column];
		if(position == 0){
			board[i][column] = player;
			return i;
		}
	}
	return -1;
}

/* Winning logic */
int check_win(int** board, int row, int col){
	if(row == 0){
		int tied = true;
		for (int i = 0; i < COLS; i++){
			if(board[0][i] == 0){
				tied = false;
			}
		}
		if(tied){
			return 0;
		}
	}
	return connect_four(board, row, col);
}

int connect_four(int** board, int row, int col){
	int count, start_col, start_row;
	int symbol = board[row][col];
	//Horizontal
	count = 0;
	for(int i = 0; i < COLS; i++){
		if(board[row][i] == symbol){
			count++;
		} else if(count > 0) {
			break;
		}
	}
	if(count == 4) return true;

	//Vertical
	count = 0;
	for(int i = 0; i < ROWS; i++){
		if(board[i][col] == symbol){
			count++;
		} else if(count > 0) {
			break;
		}
	}
	if(count == 4) return true;

	//Diagonal
	count = 0;
	if(col > row){
		start_row = 0;
		start_col = col-row;
	} else {
		start_col = 0;
		start_row = row-col;
	}
	int dx = 0;
	for(int i = start_row; i < ROWS; i++){
		if(start_col+dx >= COLS) break;
		if(board[i][start_col+dx] == symbol){
			count++;
		} else if(count > 0){
			break;
		}
		dx++;
	}
	if(count == 4) return true;

	//Anti-diagonal
	count = 0;
	if(row + col < ROWS) {
		start_row = row+col;
		start_col = 0;
	} else {
		start_row = ROWS-1;
		start_col = row+col-(start_row);
	}
	dx = 0;
	for(int i = start_row; i >= 0; i--){
		if(start_col+dx >= COLS) break;
		if(board[i][start_col+dx] == symbol){
			count++;
		} else if(count > 0){
			break;
		}
		dx++;
	}
	if(count == 4) return true;

	return false;
}
