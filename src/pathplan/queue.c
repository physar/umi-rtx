/* queue unit */

#include <stdlib.h>
#include "basics.h"
#include "queue.h"

sll add_first(list, new_elt)
	sll list;
	data new_elt;
{
	sll p;

	p = (sll) malloc(sizeof(struct elementT));

	if (p == NULL) {
		puts("add_first: MALLOC error");
		return( p );     
	}

	p->elt  = new_elt;
	p->next = list;
	
	return(p);
}


/* voegt een element gesorteerd op kosten toe aan een lijst */

sll add_sorted(list, new_elt)
	sll list;
	data new_elt;
{
	int	cost;       
	int	cost_new_elt; 

	if (list == NULL)
	{
		return(add_first(list, new_elt));
	}

	cost         = list->elt->est_cost + list->elt->cost_so_far;
	cost_new_elt = new_elt->est_cost + new_elt->cost_so_far;

	if (cost < cost_new_elt)
	{
		if ((list->elt->pos.row == new_elt->pos.row) &&
		    (list->elt->pos.col == new_elt->pos.col))
		{
			return(list);
		} 
		else
		{
			list->next = add_sorted(list->next, new_elt);
			return (list);
		}
	}
	else if (cost == cost_new_elt)
	{
		if ((list->elt->pos.row == new_elt->pos.row) &&
		    (list->elt->pos.col == new_elt->pos.col))
		{
			return(list);
		} 
		else
		{
			return(add_first(list, new_elt));
		}
	}
	else
	{
		return(add_first(list, new_elt));
	}
}


/* voegt de elementen uit lijst1 gesorteerd op kosten toe aan lijst 2 */

sll add_sorted_list(list1, list2)
	sll list1, list2;
{
	
	while (list2 != NULL) 
	{
		list1 = add_sorted(list1, list2->elt);
		list2 = list2->next;
	}

	return(list1);
}


/* verwijdert alle elementen die ook in "closed" voorkomen uit "new" */
  
sll remove_old(new, closed)
	sll new, closed;
{
	sll new_list = NULL;

	while (new != NULL) 
	{
		if (member(new->elt, closed) == 0) 
		{
			new_list = add_first(new_list, new->elt);
			new = new->next;
		}
		else
		{		
			new = remove_first(new);
		}
	}

	return(new_list);
}

sll remove_first(list)
	sll list;
{
	sll p;
	
	if (list == NULL) 
	{
		printf("Error in remove_first! \n");
		return(list);
	}

	p = list;
	list = list->next;
	free(p->elt);
	free(p);
	return(list);
}


/* geeft "1" terug als een element in "list" staat; anders "0" */

int member(elt, list)
	data elt;
	sll list;
{
	int found = 0;
	
	while ((list != NULL) && (found == 0))
	{
		if ((list->elt->pos.row == elt->pos.row)   &&
		    (list->elt->pos.col == elt->pos.col)   &&
#ifndef ARNOUD
		    (list->elt->est_cost == elt->est_cost) &&
		    (list->elt->cost_so_far == elt->cost_so_far))
#else
		    (elt->cost_so_far + elt->est_cost >=
				list->elt->cost_so_far + list->elt->est_cost))
#endif
			found = 1;
		list = list->next;
	}
	return(found);
}

sll delete_list(list)
	sll list;
{
	sll p;

	while (list != NULL)
	{
		p    = list->next;
/* ??? mag niet van SCIL:	free(list->elt); " cannot free non-allocated memory" */
		free(list);
		list = p;
	}

	return(NULL);
}

data make_elt(row, col)
	char row, col;
{
	data elt;

	elt = (data) malloc(sizeof(struct dataT));

	if (elt == NULL) 
	{
		puts("make_elt: malloc-error");
		return(elt);     
	}
	
	elt->pos.row     = row;
	elt->pos.col     = col;
	elt->parent      = NULL;
        elt->est_cost    = 0;
	elt->cost_so_far = 0; 

	return(elt);
}


/* print een sll; alleen om te testen */
	 
void print_list(list)
	sll list;
{
	while (list != NULL)
	{
#ifndef ARNOUD
		printf("\n     row: [ %d ]  ", C2I(list->elt->pos.row));
		printf("colum: [ %d ]  ", C2I(list->elt->pos.col));
		printf("est_cost: [ %d ]  ", list->elt->est_cost);
		printf("cost_so_far: [ %d ]", list->elt->cost_so_far);
#else
		printf(" %c%c(%d,%d)",  C2I(list->elt->pos.col) + '`',
					C2I(list->elt->pos.row) + '0',
					list->elt->cost_so_far,
					list->elt->cost_so_far + list->elt->est_cost);
#endif
	
		list = list->next;
	}

	printf("\n\n");
}

					

