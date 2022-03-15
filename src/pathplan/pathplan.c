/* Code inspired by a solution from Frank Kresin and Hes Siemelink (1994) */
#include <stdio.h>

#include "path.h"
#include "chess.h"
#include "config.h"
#include "basics.h"
#include "vector.h"

#include "move_piece.h"
#include "xyz.h"
#include "xml_parser.h"

#include "Path_plan.h"

       chess_board_t  board; /* used in xyz.c for the delta.x = open_gripper_width */
static chess_set_t pieces;
extern void stub_init_board();

void init_board()
{
/*
 * reads the geometrical data of board and pieces from files (defined in "config.h")
 * and put the pieces on the initial locations of the chess game.
 *
 */

        printf("init_board\n");
     /* first read the geometrical data of the board from file */
#if 0
        FILE *fp;
        if ((fp = fopen(BOARD,"r")) == NULL)
        {
                fprintf(stderr,"Cannot open veld file during initialisation\n");
                return;     
        }
        fread(&board, 1, sizeof(chess_board_t), fp);
        fclose (fp);
	printf("Read information about board from file:\n\t'%s'.\n",BOARD);
#else
	        read_xml_chess_board (BOARD,&board);
#endif

     /* then get the geometrical data of chess pieces */
#if 0
        if ((fp = fopen(PIECES,"r")) == NULL)
        {
                fprintf(stderr,"Cannot open piece file during init\n");
                return;     
        }
        fread(&pieces, 1, sizeof(chess_set_t), fp);
        fclose(fp);
	printf("Read information about pieces from file:\n\t'%s'.\n",PIECES);
#else
	        read_xml_chess_set (PIECES,&pieces);
#endif
     /* combine the geometrical data of pieces and board 
        	and put the pieces on their standard locations */
        stub_init_board (&board,&pieces);
}

void pathplan(command, traject)
	chess_move_t	command;
        xyz_list_t     *traject;
{
/*
 * generates a path for a chess move, keeping the piece so low above the board
 * that the other pieces has to be avoided. An A*-star algorithm is used to 
 * solve the problem, on a 10-10 grid, what means that the sides of the board are
 * also used as possable solution. If no path can be found, the piece is lifted
 * high above the board.
 *
 */
        chess_move_tm   chess_move;
	xyz_list_t     remove_list;
        xyz_list_t     move_list;
        double         transform_matrix[4][4];

#if 0
        printf("Pathplaning %c%c%c%c\n",
            command[0],command[1],command[2],command[3]);
#endif
	/* transformation of the command e2e4 into the integers 4141 */
	chess_move.from_col = command[0] - 'a' + '0';
	chess_move.from_row = command[1] - '1' + '0'; 
	chess_move.to_col   = command[2] - 'a' + '0'; 
	chess_move.to_row   = command[3] - '1' + '0';

        /* init board->robot transformation matrix */
        b2l_matrix(&board, transform_matrix);
#if 0
        Mprint (transform_matrix);
#endif

        /* generate a path to the garbage for the piece that is at the goal */
        remove_list = remove_piece(&board, &chess_move, transform_matrix);
        update_board_strike(&board, &chess_move);

        /* generate a path to the goal avoiding the other pieces */
        move_list   = move_piece(&board, &chess_move, transform_matrix);
        update_board_move(&board, &chess_move);

        *traject    = add_xyz(remove_list, move_list);
}
