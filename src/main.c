#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#define ROWS 6
#define COLS 7

#define true (0==0)
#define false !true

typedef enum {DEF, WIN, TIE} cond_t;
char player_symbol(int);
void draw_board(int *);
void end_game(cond_t, int);
void free_board(int *);
void close_pipe(int *);
void write_move(int *, int);
int * alloc_board();
int get_turn(int *, int);
int push_token(int *, int, int);
int check_tie(int *);
int connect_four(int *, int, int);
int read_move(int *);

int main(){
	/* Game data */
	int current_player = 0;
	int player;
	cond_t game_over;

	/* Pipe data */
	int p1_pipe[2], p2_pipe[2];
	int * enemy_pipe;
	int * my_pipe;
	pid_t pid;

	/* Shared memory data */
	int * board;

	/* Prepare the Board */
	if((board = alloc_board()) == (void *) -1){
		return EXIT_FAILURE;
	}

	/* Prepare player processes*/
	current_player = 1;

	if((pipe(p1_pipe)) == -1 || (pipe(p2_pipe)) == -1){
		return EXIT_FAILURE;
	}

	pid = fork();
	if(pid == (pid_t) 0){
		player = 2;
		my_pipe = p2_pipe;
		enemy_pipe = p1_pipe;
	} else if(pid > (pid_t) 0){
		player = 1;
		my_pipe = p1_pipe;
		enemy_pipe = p2_pipe;
	} else {
		return EXIT_FAILURE;
	}

	/* The game loop */
	while(true){
		int column;
		int row;

		if(current_player == player){
			draw_board(board);
			column = get_turn(board, player);
			write_move(my_pipe, column);
		} else {
			column = read_move(enemy_pipe);
		}

		if(column < 0){
			game_over = DEF;
			break;
		}

		if(current_player == player){
			row = read_move(enemy_pipe);
		} else {
			row = push_token(board, current_player, column);
			write_move(my_pipe, row);
		}

		if(connect_four(board, row, column)){
			game_over = WIN;
			break;
		} else if(check_tie(board)){
			game_over = TIE;
			break;
		}

		current_player = current_player % 2 + 1;
	}

	close_pipe(my_pipe);

	if(player == 2){
		return EXIT_SUCCESS;
	}

	draw_board(board);
	end_game(game_over, current_player);
	free_board(board);
	return EXIT_SUCCESS;
}


/* Memory mapping for the board */
int * alloc_board(){
	int * board;
	key_t key = ftok("connect-four", 4);
	int shmid = shmget(key, sizeof(int)*ROWS*COLS, IPC_CREAT|0666);
	if(shmid >= 0){
		board = (int *) shmat(shmid, 0, 0);
		if(board != (void *) -1){
			int i, j;
			for (i = 0; i < ROWS; i++){
				for(j = 0; j < COLS; j++){
					board[i*COLS + j] = 0;
				}
			}
		}
	} else {
		return (void *) -1;
	}
	return board;
}

void free_board(int * board){
	shmdt(board);
}

/* Graphic interface */
void draw_board(int * board){
	int i, j;
	putc('\n', stdout);
	for(i = 0; i < ROWS; i++){
		for(j = 0; j < COLS; j++){
			putc(player_symbol(board[i*COLS + j]), stdout);
		}
		putc('\n', stdout);
	}
	for(j = 0; j < COLS; j++){
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

void end_game(cond_t condition, int current_player){
	int player = current_player;
	switch(condition){
		case DEF: printf("Player %d gives up\n", player);
				player = player % 2 + 1;
		case WIN: printf("The winner is player %d (%c)", player, player_symbol(player)); break;
		case TIE: printf("Game is tied"); break;
	}
	printf(", thanks for playing!\n");
}


/* User input */
int get_turn(int * board, int player){
	do {
		printf("Player %d's turn, select a column (1-%d): ", player, COLS);
		int ch;
		int input = 0;
		int column;

		while((ch = getchar()) != '\0' && ch != '\n' && input < INT_MAX){
			input += ch;
		}

		column = input - '0' - 1;
		if(column >= 0 && column < COLS){
			if(board[column] == 0){
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
int push_token(int * board, int player, int column){
	int i;
	for(i = ROWS-1; i >= 0; i--){
		int position = board[i*COLS + column];
		if(position == 0){
			board[i*COLS + column] = player;
			return i;
		}
	}
	return -1;
}

/* Winning logic */
int check_tie(int * board){
	int i;
	int tied = true;
	for (i = 0; i < COLS; i++){
		if(board[i] == 0){
			tied = false;
		}
	}
	return tied;
}

int connect_four(int * board, int row, int col){
	int i, dx, count, start_col, start_row;
	int symbol = board[row*COLS + col];

	//Horizontal
	count = 0;
	for(i = 0; i < COLS; i++){
		if(board[row*COLS + i] == symbol){
			count++;
		} else if(count > 0) {
			break;
		}
	}
	if(count == 4) return true;

	//Vertical
	count = 0;
	for(i = 0; i < ROWS; i++){
		if(board[i*COLS + col] == symbol){
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
	dx = 0;
	for(i = start_row; i < ROWS; i++){
		if(start_col+dx >= COLS) break;
		if(board[i*COLS + (start_col+dx)] == symbol){
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
	for(i = start_row; i >= 0; i--){
		if(start_col+dx >= COLS) break;
		if(board[i*COLS + (start_col+dx)] == symbol){
			count++;
		} else if(count > 0){
			break;
		}
		dx++;
	}
	if(count == 4) return true;

	return false;
}

/* Pipes */
int read_move(int * pipe){
	int column;
	close(pipe[1]);
	read(pipe[0], &column, sizeof(int));
	return column;
}

void write_move(int * pipe, int column){
	close(pipe[0]);
	write(pipe[1], &column, sizeof(int));
}

void close_pipe(int * pipe){
	close(pipe[0]);
	close(pipe[1]);
}
