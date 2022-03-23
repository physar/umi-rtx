#ifndef BASICS_H
#define BASICS_H

/* estimated basics.h by arnoud@fwi.uva.nl */

#include <stdio.h>
#include <math.h>
#include "path.h"
#include "chess.h"
#include "matrix.h"

typedef struct chess_moveT {
  char from_col, to_col;                /* valid are '0' + 0 to '0' + 7 */
  char from_row, to_row;                /* valid are '0' + 0 to '0' + 7 */
} chess_move_tm;

typedef struct positionT {
	int row;
	int col;
}position_t;

typedef struct dataT {
	struct positionT pos;
	struct dataT *parent;
	int est_cost;
	int cost_so_far;
}data_t;
typedef struct dataT *data;
	
typedef struct elementT {
	data elt;
	struct elementT *next;
}element_t;
typedef struct elementT *sll;

typedef int ourboardT[10][10];

#define C2I(x) (x - '0')

/* estimated queue.h */
sll remove_first();

/* estimated xyz.h */
#define OPEN 1
#define CLOSED 0
#define TWEE_VINGERS 15
void board_coord();

/* estimated make_path.h */
sll check();
sll check_and_add();
void fill_private_board();

/* estimated move_piece.h */
#define SAFE_HEIGHT 140
#define KING_HEIGHT 71.0
xyz_list_t pick_up();
chess_piece_t *identify_piece();
void *put_down();
void simple_move();

#endif
