/* init pathplan unit     */
/* Aangeleverd door Joris */
/* dus dan zal ie wl goed zijn */

#ifndef ARNOUD
#include "basics.h"
#include "config.h"
#else
#include <stdio.h>
#include "chess.h"
#include "path.h"
#include "basics.h"
#include "config.h"
#include "xml_parser.h"
#include "spp.h"

void stub_init_board();
#endif


void
init_pathplan()
{

/* 
 * initialsie path planning module. Read initial board data from file BOARD
 * (which you should have defined somewhere), do some initialisation and write
 * it back to MYFILEDS (which you should have defined somewhere). MYBOARD
 * is just a local copy of BOARD. Data from other files is also read
 *
 * PAY ATTENTION to how data structures are read
 *
 */
#ifdef ARNOUD
#if 0
	init_board();
#else
#endif
        chess_board_t board;
        chess_set_t schaak_stukken;
        FILE *fp;
#if 0
     /* first read board from file */
        if ((fp = fopen(BOARD,"r")) == NULL)
        {
                fprintf(stderr,"Cannot open veld file during initialisation\n");
                return;     
        }
        fread(&board, 1, sizeof(chess_board_t), fp);
        fclose (fp);
	printf("Read information about board from file:\n\t'%s'.\n",BOARD);
#else
	if (read_xml_chess_board (BOARD,&board) < 0)
		return;
#endif

#if 0
     /* then get data of chess pieces */
        if ((fp = fopen(PIECES,"r")) == NULL)
        {
                fprintf(stderr,"Cannot open piece file during init\n");
                return;     
        }
        fread(&schaak_stukken, 1, sizeof(chess_set_t), fp);
        fclose(fp);
	printf("Read information about pieces from file:\n\t'%s'.\n",PIECES);
#else
	if (read_xml_chess_set (PIECES,&schaak_stukken) < 0)
		return;
#endif
     /* initialise board */
        stub_init_board (&board,&schaak_stukken);
        spp_draw_board (board);

     /* and write back to local file */
        if ((fp = fopen(MYBOARD,"w")) == NULL)
        {
                fprintf(stderr,"2Cannot open veld file during initialisation\n");
                return;     
        }
        fwrite(&board, 1, sizeof(chess_board_t), fp);
        fclose (fp);
#endif /* ARNOUD */
}

#ifndef ARNOUD
chess_piece_tm *
stub_init_piece (sid, kin, rownr, colnr,schaak_stukken)
     int sid, kin, rownr, colnr;
     chess_set_t *schaak_stukken;
{
/*
 * Create and initialise a single piece
 * Just fill in the data structure with the data from the file
 */
        chess_piece_tm *new_piece;
        chess_piece_dimension_tm *WITH1;

        new_piece = (chess_piece_tm *)malloc(sizeof(chess_piece_tm));
        if (new_piece == (chess_piece_tm *) NULL)
        {
                fprintf(stderr,"Cannot allocate new piece \n");
                return;     
        }
        new_piece->dead_or_alive = alive;
        new_piece->side = sid;
        new_piece->kind = kin;
        WITH1 = &new_piece->dimension;
        switch (kin) {
        case pawn_piece:
                WITH1->height = schaak_stukken->pawn[0].dimension.height;
                WITH1->diameter = schaak_stukken->pawn[0].dimension.diameter;
                break;
        case rook_piece:
                WITH1->height = schaak_stukken->rook[0].dimension.height;
                WITH1->diameter = schaak_stukken->rook[0].dimension.diameter;
                break;
        case knight_piece:
                WITH1->height = schaak_stukken->knight[0].dimension.height;
                WITH1->diameter = schaak_stukken->knight[0].dimension.diameter;
                break;
        case bishop_piece:
                WITH1->height = schaak_stukken->bishop[0].dimension.height;
                WITH1->diameter = schaak_stukken->bishop[0].dimension.diameter;
                break;
        case queen_piece:
                WITH1->height = schaak_stukken->queen.dimension.height;
                WITH1->diameter = schaak_stukken->queen.dimension.diameter;
                break;
        case king_piece:
                WITH1->height = schaak_stukken->king.dimension.height;
                WITH1->diameter = schaak_stukken->king.dimension.diameter;
                break;
        }
     /* bewqare : use chars here instead of integers */
        new_piece->position.row = (char) '0' + rownr;
        new_piece->position.column = (char) '0' + colnr;

        return new_piece;
}

void
stub_init_board (board,schaak_stukken)
     chess_board_t *board;
     chess_set_t *schaak_stukken;
{
/*
 * Initialise the board structure with the initial board position. Here, a
 * conventional starting position is initialised.
 */

        int i,j;

     /* "fill" garbage place */

        for(i=0;i<16;i++)
        {
                board->garbage_place[black_player][i] = (chess_piece_tm *) NULL;
                board->garbage_place[white_player][i] = (chess_piece_tm *) NULL;
        }


     /* put pieces on the board */
             /* pionnen */
        for(i=a_m;i<=h_m;i++)
        {
                board->place[1][i] = stub_init_piece(white_player,pawn_piece,1,i,schaak_stukken);
                board->place[6][i] = stub_init_piece(black_player,pawn_piece,6,i,schaak_stukken);
        }
             /* torens */
        board->place[0][a_m] = stub_init_piece(white_player,rook_piece,0,a_m,schaak_stukken);
        board->place[0][h_m] = stub_init_piece(white_player,rook_piece,0,h_m,schaak_stukken);
        board->place[7][a_m] = stub_init_piece(black_player,rook_piece,7,a_m,schaak_stukken);
        board->place[7][h_m] = stub_init_piece(black_player,rook_piece,7,h_m,schaak_stukken);
             /* paarden */
        board->place[0][b_m] = stub_init_piece(white_player,knight_piece,0,b_m,schaak_stukken);
        board->place[0][g_m] = stub_init_piece(white_player,knight_piece,0,g_m,schaak_stukken);
        board->place[7][b_m] = stub_init_piece(black_player,knight_piece,7,b_m,schaak_stukken);
        board->place[7][g_m] = stub_init_piece(black_player,knight_piece,7,g_m,schaak_stukken);
             /* lopers */
        board->place[0][c_m] = stub_init_piece(white_player,bishop_piece,0,c_m,schaak_stukken);
        board->place[0][f_m] = stub_init_piece(white_player,bishop_piece,0,f_m,schaak_stukken);
        board->place[7][c_m] = stub_init_piece(black_player,bishop_piece,7,c_m,schaak_stukken);
        board->place[7][f_m] = stub_init_piece(black_player,bishop_piece,7,f_m,schaak_stukken);

            /* koningen */
        board->place[0][e_m] = stub_init_piece(white_player,king_piece,0,d_m,schaak_stukken);
        board->place[7][e_m] = stub_init_piece(black_player,king_piece,7,d_m,schaak_stukken);
             /* koninginnen */
        board->place[0][d_m] = stub_init_piece(white_player,queen_piece,0,d_m,schaak_stukken);
        board->place[7][d_m] = stub_init_piece(black_player,queen_piece,7,d_m,schaak_stukken);
             /*empty */
        for(i=2;i<=5;i++)
                for(j=a_m;j<=h_m;j++)
                        board->place[i][j] = (chess_piece_tm *) NULL;
}
#endif /* ARNOUD */
