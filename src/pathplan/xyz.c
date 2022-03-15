/* xyz unit */

#include "basics.h"

extern chess_board_t board;

xyz_list_t make_new_xyz(x, y, z, grip, n1, n2 ,n3, theta)
        double            x, y, z, grip, n1, n2, n3, theta;

{

        xyz_list_t      elt;

        elt = (xyz_list_t ) malloc(sizeof(xyz_list_item_t));

        elt->x          = x;
        elt->y          = y;
        elt->z          = z;
        elt->grip       = grip;
        elt->n1         = n1;
        elt->n2         = n2;
        elt->n3         = n3;
        elt->theta      = theta;
        elt->next_data  = NULL;

        return elt;
}


/* find_last vindt een pointer die naar het laatste element van "list" wijst */

xyz_list_t find_last_xyz(list)
        xyz_list_t      list;

{

        xyz_list_t      p = list;

        if (p != NULL)
                while (p->next_data != NULL)
                        p = p->next_data;

        return p;
}

void change_xy(list, x, y)
        xyz_list_t      list;
        double          x, y;
{
        xyz_list_t      l;
        xyz_list_t	new_elt;

        if (list == NULL)
        {
                printf("Error in change_xy: empty list.\n");
                return;     
        }

        l = find_last_xyz(list);
        new_elt = make_new_xyz( x, y , l->z,
                        l->grip, l->n1, l->n2, l->n3, l->theta);
        l->next_data = new_elt;

/*      printf("  Position to: [ %f %f ]\n", x, y);           voor testen */
}

void change_height(list, height)
        xyz_list_t      list;
        double          height;
{
        xyz_list_t      l;
        xyz_list_t      new_elt;

        if (list == NULL)
        {
                printf("Error in change_height: empty list.\n");
                return;     
        }

        l = find_last_xyz(list);
        new_elt = make_new_xyz(l->x, l->y, height,
                        l->grip, l->n1, l->n2, l->n3, l->theta);
        l->next_data = new_elt;

/*      printf("  Height to: [ %f ]\n", height);              voor testen */
}

void change_normal(list, n1, n2, n3)
        xyz_list_t      list;
        double          n1,n2,n3;
{
        xyz_list_t      l;
        xyz_list_t      new_elt;

        if (list == NULL)
        {
                printf("Error in change_height: empty list.\n");
                return;     
        }

        l = find_last_xyz(list);
        new_elt = make_new_xyz(l->x, l->y, l->z,
                        l->grip, n1, n2, n3, l->theta);
        l->next_data = new_elt;

/*      printf("  Normal to: [ %f %f %f ]\n", n1, n2, n3);     voor testen */
}

int 
change_grip(list, grip_status, piece)
	xyz_list_t list;
	int		grip_status;
	chess_piece_t 	*piece;
	
{
	xyz_list_t	l;
	xyz_list_t	new_elt;
	double 		grip;

	if (list == NULL)
	{
		printf("Error in change_grip: empty list.\n");
		return (FALSE);
	}

	if (grip_status == OPEN)
#ifndef ARNOUD
		grip = 2.35  * piece->diameter; /* om stuk */
#else
		grip = 0.75 * board.delta_x;
#endif
	else
		grip = 0.70 * piece->diameter;  /* stuk vast */

	l = find_last_xyz(list);
	new_elt = make_new_xyz(l->x, l->y, l->z, grip, l->n1, l->n2, l->n3, l->theta);
	l->next_data = new_elt;

/*	printf("  Grip to: [ %f ]\n",grip);                    voor testen */
	return (TRUE);
}


/* find_garbage geeft het nummer vad de eerste vrije plaats in de garbage- */
/* lijst van de betreffende speler                                         */

int find_garbage(board, player)
	chess_board_t	*board;
	int             player;
{

	int	i, free_spot = 16;   /* geef 16 terug als garb. vol is */
	
	for(i = 0; i < 16; i++)
		if (board->garbage[player][i] == NULL)
			{
				free_spot = i;
				i = 16;
			}
	
	if(player == black_player)
		return -1 * free_spot - 1;
	else
		return  1 * free_spot + 1;
}


/* coord_of_garb vindt voor de geslagen witte stukken een stukje naast de  */
/* rand van het bord, uitgaande van het nummer dat ze hebben in            */
/* board->garbage[white_player]                                            */

void coord_of_garb(board, nr, x, y)  /* altijd voor een wit stuk */
	chess_board_t *board;
	int	nr;
	double *x, *y;
{
	double alfa, x1, y1;

        if(nr < black_player) { /* Black next to the A column */
                alfa = ( 90 - board->theta) / 180 * M_PI;
                x1 = board->x - 2 * board->board_thickness - board->delta_x -
                                8 * board->delta_x + ((nr+1) / 8) * board->delta_x;
                y1 = board->y + board->board_thickness - 0.5 * board->delta_y +
                                8 * board->delta_y + ((nr+1) % 8) * board->delta_y;
        } else {                /* White next to the H column */
                alfa = ( 90 - board->theta) / 180 * M_PI;
                x1 = board->x + board->delta_x +
                                ((nr-1) / 8) * board->delta_x;
                y1 = board->y + board->board_thickness + 0.5 * board->delta_y +
                                ((nr-1) % 8) * board->delta_y;
        }

	*x = sin(alfa) * x1 - cos(alfa) * y1;  /* om de garbage-place mee te */
	*y = cos(alfa) * x1 + sin(alfa) * y1;  /* laten draaien met het bord */

/*	printf("Coordinates of garbage spot: [ %f , %f ]\n",*x,*y);   test */  
	
}

void print_xyz_list(list)
	xyz_list_t list;
{
	while (list != NULL)
	{
		printf("[ %f ]  [ %f ]  [ %f ] (coordinaten) \n",list->x, list->y, list->z);
		printf("[ %f ]  [ %f ]  [ %f ] (normaal) \n",list->n1, list->n2, list->n3);
		printf(" %f  (grip)\n\n",list->grip);

		list = list->next_data;
	}
}


/* board_list2cart_list zet een lijst met punten in een sll om naar een */
/* een lijst met punten relatief tov het nulpunt van het bord en plakt  */
/* die aan de meegegeven xyz_list.                                      */

void board_list2cart_list(board, board_pos_list, xyz_list)
	chess_board_t	*board;
	sll        	board_pos_list;
	xyz_list_t xyz_list;
{
	double	x,y; 
	
	while(board_pos_list != NULL)
	{
		/* zet configuratie-ruimte coordinaten om in */
		/* coordinaten tov. het bord                 */
		board_coord(board, board_pos_list->elt->pos.row,
				       board_pos_list->elt->pos.col, &x, &y);

		change_xy(xyz_list, x, y);
		board_pos_list = board_pos_list->next;
	}
}


/* board_2_lab zet de xyz coordinaten tov het bord-nulpunt om in coordinaten */
/* ten opzichte van het lab-nulpunt.                                         */

void board_2_lab(board_list, board, trans_matrix)
	xyz_list_t board_list;
	chess_board_t	*board;
	MAT44           trans_matrix;
{
	xyz_list_t p = board_list;
	VEC4       	board_vec, lab_vec;

	while (p != NULL)
	{
		board_vec[0] = p->x;
		board_vec[1] = p->y;
		board_vec[2] = p->z;
		board_vec[3] = 1.0;
		
		transform(board_vec, lab_vec, trans_matrix);

		p->x = lab_vec[0];
		p->y = lab_vec[1];
		p->z = lab_vec[2];
		
		p = p->next_data;	
	}
}


/* zet coordinaten in de configuratieruimte om in coordinaten ten opzichte */
/* van het "nulpunt" van het bord (zie de tekening in de opdracht).        */

void board_coord(board, row, col, x, y)
	chess_board_t	*board;
	char     	row, col;
	double        	*x, *y;
{
#if 0
        (void )chess_board_info(board);
        printf ("y = (%d - %d) + 1.5) * %g - %g = %g\n",
           '7',row,board->delta_y,board->sur_y,*y);
#endif
	*x = -((double) ('7' - col) + 0.5) * board->delta_x - board->sur_x;
	*y =  ((double) ('7' - row) + 1.5) * board->delta_y - board->sur_y;

}


/* add_xyz knoop twee xyz_list_item_t structures aan elkaar */

xyz_list_t add_xyz(list1, list2)
	xyz_list_t list1;
	xyz_list_t list2;
{
	xyz_list_t p = list1;

	if (list1 == NULL)
		return(list2);
	while (p->next_data != NULL)
		p = p->next_data;

	p->next_data = list2;

	return(list1);
}	
