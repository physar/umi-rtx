/* vector unit */

#include "basics.h"


/* transform zet een vector met bord-coordinaten om in een vector */
/* met lab-coordinaten.                                           */

void transform (board_pos, lab_pos, trans_matrix)
	VEC4	board_pos;
	VEC4	lab_pos;
	MAT44   trans_matrix;
{
	MVprod4 (trans_matrix, board_pos, lab_pos);
}


/* Bereken een homogene transformatie-matrix om bord-coordinaten om te      */
/* zetten in lab-coordinaten, gebruik makend van de informatie in board.rtx */

void b2l_matrix(board, tot_mat)
	chess_board_t	*board;
	MAT44	tot_mat;
{
	MAT44	transl_mat, Xrot_mat, Yrot_mat, Zrot_mat, tot1, tot2;
	double	alfa, beta, theta = -(board->theta / 180.0) * M_PI;

	Rotz3D(theta, Zrot_mat);

	beta = atan2(board->n1,sqrt(pow(board->n2,2.0) +  pow(board->n3,2.0)));
				/* pythagoras */
	Roty3D(beta, Yrot_mat);

	alfa = -atan2(board->n2, board->n3);
	Rotx3D(alfa, Xrot_mat);

	Transl3D(board->x, board->y, board->z, transl_mat);

	MMprod4(Yrot_mat, Zrot_mat, tot1);
	MMprod4(Xrot_mat, tot1, tot2);
	MMprod4(transl_mat, tot2, tot_mat);

}

