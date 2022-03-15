/* make_path unit */

#include "basics.h"
#ifdef ARNOUD
#include "make_path.h"
#endif

sll make_path(board, cm)
	chess_board_t 	*board;
	chess_move_tm 	*cm;
{
	sll new= NULL, open = NULL, closed = NULL, path = NULL;
	ourboardT ourboard;
	data first_elt, pp_elt, new_elt;
#ifdef ARNOUD
#if 0
	data goal_elt;
	int teller = 1000;
#endif
#endif

	/* zet de stukken uit de chess_board_t  structure over */
        /* naar de configuratieruimte.                         */
	fill_private_board(board, ourboard);

#ifdef ARNOUD
#if 0
        goal_elt = make_elt(cm->to_row + 1, cm->to_col + 1);
	new = check(goal_elt, ourboard, cm);
	if(new == NULL)
	{
		printf("arnoud says: no solution possible\n");
		return(NULL);
	}
#endif
#endif
	/* maakt het eerste element van de A* search.          */
	first_elt = make_elt(cm->from_row + 1, cm->from_col + 1);
	first_elt->est_cost = distance(first_elt, cm);

	/* zet het eerste element in de "open"-lijst.          */
	open = add_first(open, first_elt);
	
	/* de A* search.                                       */
	while ((open != NULL) && (open->elt->est_cost != 0))
	{
		/* vindt de nieuwe elementen            */
		new = expand(open->elt, ourboard, cm);
		
		/* verwijdert dubbele elementen         */
		new = remove_old(new, closed);
#ifdef ARNOUD
		new = remove_old(new, open);
#endif

		/* stopt gebruikte elt in closed-lijst  */
		closed = add_first(closed, open->elt);
		
		/* verwijdert 1e elt uit de open-lijst  */
		open = open->next;

		/* voeg nieuwe elt toe in de open-lijst */
		open = add_sorted_list(open, new);

#ifdef ARNOUD
#if 0
		print_list(open);
		if(!(teller--)) {
			printf("arnoud says: time is up!\n");
                return(NULL);
        	}
#endif
#endif

	}

	if (open == NULL)                /* geen pad  gevonden */
	{
		return(NULL);
	}

	pp_elt = open->elt;    /* 1e elt van het pad: het doel */

	while (pp_elt != NULL)      /* zet het pad in een lijst*/
	{
		new_elt = make_elt(pp_elt->pos.row - 1, pp_elt->pos.col - 1);

		path = add_first(path, new_elt);

		pp_elt = pp_elt->parent;
	}

	/* verwijder de lijsten uit het geheugen.              */	
	delete_list(open);
	delete_list(closed);
	delete_list(new);

	return(path);
}


/* expand vindt alle nieuwe mogelijke zetten vanaf een bepaald punt */
/* in de configuratieruimte                                         */

static sll expand(parent, board, cm)
	data		parent;
	ourboardT	board;
	chess_move_tm 	*cm;
{
	sll  new = NULL;
	data elt;

	if (C2I(parent->pos.col) < 9) 
	{ 
		elt = make_elt(parent->pos.row, 
			       parent->pos.col + 1);
		new = check_and_add(elt, parent, board, new, cm);
	}

	if (C2I(parent->pos.col) > 0) 
	{ 
		elt = make_elt(parent->pos.row, 
			       parent->pos.col - 1);
		new = check_and_add(elt, parent, board, new, cm);
	}		
	
	if (C2I(parent->pos.row) < 9) 
	{ 
		elt = make_elt(parent->pos.row + 1, 
			       parent->pos.col);
		new = check_and_add(elt, parent, board, new, cm); 
	}
	
	if (C2I(parent->pos.row) > 0) 
	{ 
		elt = make_elt(parent->pos.row - 1, 
			       parent->pos.col);
		new = check_and_add(elt, parent, board, new, cm);
	}
	
	return(new);
}


/* add_and_check voegt een bepaald punt toe aan een lijst met punten */
/* mits dat vrij is in de configuratieruimte of de 'from'-positie.   */ 

sll add_and_check(elt, parent, board, list, cm)
	data	       elt, parent;
	ourboardT      board;
	sll	       list;
	chess_move_tm   *cm;
{
	if ( (board[C2I(elt->pos.row)][C2I(elt->pos.col)] == 0) ||
             (elt->pos.row == cm->from_row && elt->pos.col == cm->from_col) )
	{	
		elt->parent = parent;
		elt->est_cost = distance(elt, cm);
		elt->cost_so_far = parent->cost_so_far + 1;
		list = add_first(list, elt);
	}
	else
		free(elt);
 		
	return(list);
}
/* check_and_add voegt een bepaald punt toe aan een lijst met punten */
/* mits dat vrij is in de configuratieruimte.                        */ 

sll check_and_add(elt, parent, board, list, cm)
	data	       elt, parent;
	ourboardT      board;
	sll	       list;
	chess_move_tm   *cm;
{
	if (board[C2I(elt->pos.row)][C2I(elt->pos.col)] == 0) 
	{	
		elt->parent = parent;
		elt->est_cost = distance(elt, cm);
		elt->cost_so_far = parent->cost_so_far + 1;
		list = add_first(list, elt);
	}
	else
		free(elt);
 		
	return(list);
}

/* check controleer of een bepaald punt in de configuratieruimte    */
/* bereikbaar is.                                                   */

sll check(parent, board, cm)
        data            parent;
        ourboardT       board;
        chess_move_tm    *cm;
{
        sll  new = NULL;
        data elt;

        if (C2I(parent->pos.col) < 9)
        {
                elt = make_elt(parent->pos.row,
                               parent->pos.col + 1);
                new = add_and_check(elt, parent, board, new, cm);
        }

        if (C2I(parent->pos.col) > 0)
        {
                elt = make_elt(parent->pos.row,
                               parent->pos.col - 1);
                new = add_and_check(elt, parent, board, new, cm);
        }

        if (C2I(parent->pos.row) < 9)
        {
                elt = make_elt(parent->pos.row + 1,
                               parent->pos.col);
                new = add_and_check(elt, parent, board, new, cm);
        }

        if (C2I(parent->pos.row) > 0)
        {
                elt = make_elt(parent->pos.row - 1,
                               parent->pos.col);
                new = add_and_check(elt, parent, board, new, cm);
        }

        return(new);
}

/* distance berekent de geschatte kosten om vanaf een bepaald punt tot */ 
/* het doel te komen.                                                  */

static int distance(elt, move)
	data		elt;
	chess_move_tm 	*move;
{
	int getal;
	
	getal =	abs((int) move->to_row - elt->pos.row + 1) +
	     	abs((int) move->to_col - elt->pos.col + 1);

	return(getal);
}


/* fill_private_board zet de stukken in de chess_board_t  structure om */
/* naar de configuratie-ruimte.                                        */

void fill_private_board(board, ourboard)            
	chess_board_t 	*board;
	ourboardT        ourboard;
{
	int	i = 0 , j = 0;
	
	for (i=0;i < 10; i++)
		for (j=0; j < 10; j++)
		{
			if (i<1 || i>8 || j<1 || j>8)
			{                       /* de rand om het bord */
				ourboard[i][j] = 0;
			}
			else if (board->field[i-1][j-1] == NULL)
			{                       /* het punt is vrij    */
				ourboard[i][j] = 0;
			}
			else                    /* het punt is bezet   */
			{
				ourboard[i][j] = 1;
			}
		}

}
