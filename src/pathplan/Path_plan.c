/* Path-plan unit */

#include <stdlib.h>

#ifndef ARNOUD
#include "queue.h"
#include "queue.c"

#include "vector.h"
#include "vector.c"

#include "xyz.h"
#include "xyz.c"

#include "io.h"
#include "io.c"

#include "make_path.h"
#include "make_path.c"

#include "move_piece.h"
#include "move_piece.c"

#include "Path_plan.h"
#else
#include "queue.c"

#include "vector.c"

#include "xyz.c"

#include "io.c"

#include "make_path.c"

#include "move_piece.c"

void make_move_list();
void update_board_strike();
void update_board_move();
#endif

static void print_board(chess_board_t  *board);      

void Path_plan(hm, cm, traject)
	chess_move_tm 	*hm, *cm;
	xyz_list_t 	*traject;
{
	chess_board_t 	board;
#ifndef ARNOUD
        xyz_list_item_t *remove_list, *move_list, *traject;
#endif

	puts("Path Planning: Start");

	if(load_board_pp(&board) != EXIT_SUCCESS)
		return;

        print_board(&board);

	puts("Path Planning: Step2");

	make_move_list(&board, cm, hm, traject);

	puts("Path Planning: Step3");

	write_board_pp(&board);

/* 	print_xyz_list(*traject);                       voor het testen */

	puts("Path Planning: End");
}

void make_move_list(board, cm, hm, traject)
	chess_board_t  *board;
	chess_move_tm   *cm, *hm;
	xyz_list_t     *traject;
{
	xyz_list_t     remove_list;
	xyz_list_t     move_list;
	MAT44          transform_matrix;

	puts("make_move_list: Start");

	/* init transformatie matrix */
	b2l_matrix(board, transform_matrix); 

	puts("make_move_list: Step2");

	/* update board structure */	
	update_board_strike(board, hm);
	update_board_move(board, hm);

	puts("make_move_list: Step3");

	/* verwijdert geslagen stuk */
	remove_list = remove_piece(board, cm, transform_matrix);
		
	puts("make_move_list: Step4");

	update_board_strike(board, cm);

	puts("make_move_list: Step5");

	/* verplaatst zwart stuk */
	move_list   = move_piece(board, cm, transform_matrix);

	puts("make_move_list: Step6");

	*traject    = add_xyz(remove_list, move_list);

	puts("make_move_list: Step7");

	update_board_move(board, cm);

	puts("make_move_list: End");


}

/* update_board_strike verplaatst een geslagen stuk (mits aanwezig) van  */
/* het bord naar de garbage- place in de structure chess_board_t .       */

void update_board_strike(board, move)
	chess_board_t  *board;
	chess_move_tm   *move;
{      
        if(board == NULL) {
 	    fprintf(stderr, "Cannot update strike with empty board\n");
        }

        if(move == NULL) {
 	    fprintf(stderr, "Cannot update strike with empty move\n");
        }

        puts("Start of update_board_strike()");
	chess_piece_t 	*to_pos   = board->field[C2I(move->to_row)]
			                        [C2I(move->to_col)];
	int		 nr;    

        printf("Checking if there is a piece on row %d and %d (%d,%d)\n", move->to_row, move->to_col, C2I(move->to_row), C2I(move->to_col));

	if (to_pos != NULL)   /* er moet een stuk verwijderd worden */
	{
                puts("Step2 of update_board_strike()");
		nr = find_garbage(board, to_pos->side); /* vrije positie */
                puts("Step3 of update_board_strike()");
                printf("Board-garbage %s exist, with side %d and pos %d\n", board->garbage ? "does" : "doesn't", to_pos->side, abs(nr)-1); 
		board->garbage[to_pos->side][abs(nr)-1] = to_pos;
                puts("Step4 of update_board_strike()");
		board->field[C2I(move->to_row)][C2I(move->to_col)] = NULL;
	}
        puts("End of update_board_strike()");
}


/* update_board_move verplaatst een stuk op het bord in de structure    */
/* chess_board_t .                                                      */

void update_board_move(board, move) 
	chess_board_t  *board;
	chess_move_tm   *move;
{
	chess_piece_t 	*from_pos = board->field[C2I(move->from_row)]
			                        [C2I(move->from_col)];

	board->field[C2I(move->to_row)][C2I(move->to_col)] = from_pos;
	board->field[C2I(move->from_row)][C2I(move->from_col)] = NULL;

/*	print_board(board)                             voor het testen */
}


/* print het bord en de geslagen stukken, alleen voor het testen       */

static void print_board(board)       
	chess_board_t  *board;
{
	char		r, c;
	chess_piece_t  *piece; 
	int		i;

	/* Schrijf bord op scherm */
	for(r = '7'; r >= '0'; r--)
	{
		printf("  %c  ",r);
		for (c = '0'; c <= '7' ; c++)
		{
			piece = board->field[r-'0'][c-'0'];
			if (piece == NULL)
				printf(" . ");
			else	
				printf (" %d ",piece->kind);
		}
		printf("\n");
	}
	printf("\n     ");
	for (c = '0'; c <= '7' ; c++)
		printf(" %c ",c);
	printf("\n");

	/* De geslagen stukken. ('white_player' is een #define) */
	printf("\nWhite trash:  ");
	for (i = 0; i < 16; i++)
	{
		piece = board->garbage[white_player][i];
		if (piece == NULL)
			printf(" . ");
		else	
			printf (" %d ",piece->kind);
	}
	printf("\nBlack trash:  ");
	for (i = 0; i < 16; i++)
	{
		piece = board->garbage[black_player][i];
		if (piece == NULL)
			printf(" . ");
		else	
			printf (" %d ",piece->kind);
	}
	printf("\n");	
}

