/* move_piece unit */

#include "basics.h"

xyz_list_t move_piece(board, cm, trans_matrix)
	chess_board_t 	*board;
	chess_move_tm 	*cm;
	MAT44           trans_matrix;
{
	sll        	board_pos_list;
	xyz_list_t 	traject;

	traject = pick_up(board, cm->from_row, cm->from_col);

	if (traject == NULL)   /* Er is staat geen stuk om op te pakken */
		return(NULL);

	board_pos_list = make_path(board, cm);

	if (board_pos_list == NULL)          /* Er is geen pad gevonden */
		simple_move(board, traject, cm);
	else      /* zet de configuratie-notatie om in bord-coordinaten */              	board_list2cart_list(board, board_pos_list, traject);
	
	put_down(board, cm->from_row, cm->from_col, traject);

	/* zet de bord-coordinaten om in lab-coordinaten                */
	board_2_lab(traject, board, trans_matrix);  

	return traject;
}

static int _once = 0;
/* pick_up genereert een pad in bord_coordinaten                        */

xyz_list_t pick_up(board, row, col)
	chess_board_t 	*board;
	char          	row, col;
{
	xyz_list_t 	list;
	chess_piece_t 	*piece = NULL;
	double        	x, y, z, height;

	/* board_coord geeft de coordinaten van een bepaalde positie in */
	/* coordinaten ten opzichte van het bord-nulpunt (vd tekening) */
	board_coord(board, row, col, &x, &y);	 

	z      = SAFE_HEIGHT;                   /* anders mekkert scil */

	piece  = identify_piece(board, row, col);

	if ((piece == NULL) )
	{
                if(!_once) {
			puts(" No Piece in PICK_UP!");
                        _once = 1;
                }
		return(NULL);
	}
	if (board == NULL )
        {
                puts(" No board in PICK_UP!");
                return(NULL);
        }
	
	height = board->board_thickness + piece->height * 0.5;

	/* maakt een eerste punt boven het op te pakken stuk. De rest */
	/* wordt dan relatief tot deze positie berekend.              */
	list   = make_new_xyz(x, y, z, 0.0, board->n1, board->n2, board->n3, 
                                            board->theta);
	
	z = height + TWEE_VINGERS;           /* hoogte boven het bord */
	
	/* pak het stuk op */
	change_grip(list, OPEN, piece);
	change_height(list, z);
	change_height(list, height);
	change_grip  (list, CLOSED, piece);
	change_height(list, z);

	return list;
}


/* put_down genereert een pad in bord_coordinaten                     */

void *put_down(board, row, col, list)
	chess_board_t 	*board;
	char          	row, col;
	xyz_list_t 	list;
{
	chess_piece_t 	*piece;
	double        	z, height;
	
	piece  = identify_piece(board, row, col);
	height = board->board_thickness + piece->height * 0.5;
	z      = height + TWEE_VINGERS;

	/* zet het stuk neer */
	change_height(list, z);
	change_height(list, height);
	change_grip  (list, OPEN, piece);
	z      = SAFE_HEIGHT;               /* anders mekkert scil */     
	change_height(list, z);

	return list;
}


/* remove_piece genereert een pad in lab_coordinaten                 */
xyz_list_t  remove_piece(board, move, trans_matrix)
	chess_board_t 	*board;
	chess_move_tm 	*move;
	MAT44           trans_matrix;
{
	xyz_list_t 	remove_xyz;
	chess_piece_t 	*piece;
	int           	spot;
	double        	garbage_x, garbage_y, z, height;

	piece = identify_piece(board, move->to_row, move->to_col);
	z = SAFE_HEIGHT;
	if (piece == NULL) {
                printf("No piece to remove\n");
		return NULL;           /* geen stuk om te verwijderen */
        }

        printf("There is a piece to remove at %d, %d\n", C2I(move->to_row), C2I(move->to_col));
 	/* pak het stuk op */
	remove_xyz = pick_up(board, move->to_row, move->to_col);
	change_height(remove_xyz, z);

	/* zet het tot nu toe gevonden pad om in lab-coordinaten */
	board_2_lab(remove_xyz, board, trans_matrix);

	/* ga naar de garbage place */
#ifndef ARNOUD
	spot  = find_garbage(board, white_player) - 1;  /*altijd een wit stuk!*/
#else
	spot  = find_garbage(board, piece->side);
#endif
	coord_of_garb(board, spot, &garbage_x, &garbage_y);
	change_xy(remove_xyz, garbage_x, garbage_y);
	change_normal(remove_xyz, 0.0, 0.0, 1.0); 
	
	/* zet het stuk neer op het lab-vlak */
	height = piece->height * 0.5;
	z = height + TWEE_VINGERS;
	change_height(remove_xyz, z);
	change_height(remove_xyz, height);
	change_grip  (remove_xyz, OPEN, piece);
	z = SAFE_HEIGHT;
	change_height(remove_xyz, z);

	return(remove_xyz);
}

chess_piece_t  *identify_piece(board, row, col)
	chess_board_t 	*board;	
	char		row, col;
{
	return board->field[C2I(row)][C2I(col)];
}


/* Als er geen andere oplossing gevonden wordt, dan pakt beweegt simple_move */
/* de hand omhoog tot "SAFE_HEIGHT" en dan rechtreeks naar de doel-positie.  */
/* Simple_move genereert een pad in bord-coordinaten.                        */

void simple_move(board, list, move)	
	chess_board_t 	*board;
	xyz_list_t 	list;
	chess_move_tm 	*move;
{
	double x,y,z;

#if 1           
	z = SAFE_HEIGHT;                       /* anders mekkert scil */
#else
	z = board->board_thickness + piece->dimension.height + KING_HEIGHT + TWEE_VINGERS;
#endif
	change_height(list, z);
	/* zet de configuratie-notatie om in bord-coordinaten */
	board_coord(board, move->to_row, move->to_col, &x, &y);
	change_xy(list, x, y);
	
}
	

