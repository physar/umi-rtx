#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <string.h>

#include "chess.h"
#include "path.h"
#include "config.h"
#include "matrix.h"
#include "spp.h"


#define GRIPPER_OPEN    30
#define GRIPPER_CLOSED  0.0
#define FRACT		0.75		/* fract of piece diameter to grab */
#define CLOSE_TO_BOARD  20		/* number of mm close to board */
#define ABOVE_BOARD  200		/* number of mm close to board */
#define SAFE_HEIGHT     220.0
#define GARBMARGE	20		/* margin between board and garbage */
#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

/* #define VERBOSE */

int spp_to_garb;			/* moving to garbage ? */
MAT44 spp_trans;			/* transformation matrix */

void stub_init_pp();
void stub_init_board ();
void old_stub_Path_plan ();

static int  chess_verify(move)
chess_move_t move;
{
	if(move == NULL)
		return(0);

	if( /* format "e2e4" */
               (                      move[0] >= 'a' && move[0] <= 'h' &&
                                      move[1] >= '1' && move[1] <= '8' &&
                                      move[2] >= 'a' && move[2] <= 'h' &&
                                      move[3] >= '1' && move[3] <= '8' )
	)
		return(1);
	else
		return(0);
}

void
std_init_path()
{
	stub_init_pp();
}

void
stub_Path_plan( human_move, computer_move, xyz_list )
chess_move_t    human_move;
chess_move_t    computer_move;
xyz_list_t     *xyz_list;
{

	chess_move_tm   hm;
	chess_move_tm   cm;

	if(!chess_verify(human_move)) {
		fprintf(stderr,"Illegal Human Move\n");
		strcpy(human_move, "e2e4");
		xyz_list = (xyz_list_t *)NULL;
	}
	hm.from_col = human_move[0] - 'a' + '0';
	hm.from_row = human_move[1] - '1' + '0';
	hm.to_col   = human_move[2] - 'a' + '0';
	hm.to_row   = human_move[3] - '1' + '0';

	if(!chess_verify(computer_move)) {
		fprintf(stderr,"Illegal Computer Move\n");
		strcpy(computer_move, "e7e5");
		xyz_list = (xyz_list_t *)NULL;
	}
	if(0) printf("hm = %s, cm = %s\n", human_move, computer_move);
	cm.from_col = computer_move[0] - 'a' + '0';
	cm.from_row = computer_move[1] - '1' + '0';
	cm.to_col   = computer_move[2] - 'a' + '0';
	cm.to_row   = computer_move[3] - '1' + '0';

	old_stub_Path_plan( &hm, &cm, xyz_list );

}

void
std_path_plan ( human_move, computer_move, xyz_list )
/* standard nomencultur for my_chess 		arnoud@fwi.uva.nl	*/
chess_move_t    human_move;
chess_move_t    computer_move;
xyz_list_t     *xyz_list;
{
	stub_Path_plan( human_move, computer_move, xyz_list);
}


void
std_board_to_cart ( board, row, col, from )
     chess_board_t *board;
     int row, col;
     VEC4 from;
{
/*
 * standard nomenclature bla bla
 */

	if(!board || board->n1 <0)
		printf("board not found\n");
	spp_creatcol_eat(board);
	spp_board_to_cart( board, row, col, from );
}


int tocol (m)
     chess_move_tm *m;
{
	return (int) m->to_col - '0';
}

int torow (m)
     chess_move_tm *m;
{
	return (int) m->to_row - '0';
}

int fromcol (m)
     chess_move_tm *m;
{
	return (int) m->from_col - '0';
}

int fromrow (m)
     chess_move_tm *m;
{
	return (int) m->from_row - '0';
}


void
spp_read_board(board)
     chess_board_t *board;
{
	FILE *fp;
        int num_items;
#if 0
	printf("spp_read_board: opening file %s\n",MYBOARD);
#endif
	if ((fp = (FILE *) fopen (MYBOARD,"r")) == (FILE *) NULL)
	{
		fprintf(stderr, "Cannot read current veld file\n");
		return;
	}
	
	num_items = fread(board, 1, sizeof(chess_board_t), fp);
	printf("Read current board file %s with %d items in read_board\n",MYBOARD,num_items);
	
	fclose(fp);
}


void
spp_print(board)
     chess_board_t *board;
{

	printf("%f\t%f\t%f\t\n",board->n1,board->x,board->theta);
}


void
spp_write_board(board)
     chess_board_t *board;
{
        FILE *fp;

        printf("spp_write_board: opening file %s\n",MYBOARD);
        if ((fp = (FILE *) fopen (MYBOARD,"w")) == (FILE *) NULL)
        {
                fprintf(stderr, "Cannot write current veld file\n");
                return;
        }
        
        fwrite(board, 1, sizeof(chess_board_t), fp);
        
        fclose(fp);
}


void
spp_recol_gove(board, move)
     chess_board_t *board;
     chess_move_tm *move;
{

	int sid;
	int i,tc,tr,fc,fr;


	tc  = tocol (move);
	tr = torow (move);
	fc = fromcol (move);
	fr = fromrow(move);

	if (board->field[tr][tc] != (chess_piece_t *) NULL)
	{
   	     /* move piece to garbage */
		sid = board->field[tr][tc]->side;
		for(i=0;board->garbage[sid][i] != NULL;i++)
			;	/* find emtpy garbage */
		board->field[tr][tc]->dead_or_alive = dead;
		board->garbage[sid][i] = board->field[tr][tc];
	}

	board->field[tr][tc] = board->field[fr][fc];
	board->field[fr][fc] = (chess_piece_t *) NULL;
	printf("integers = %d%d%d%d\n",tc,tr,fc,fr);
}


void
spp_to_garbage (board, move, list)
     chess_board_t *board;
     chess_move_tm *move;
     xyz_list_t *list;
{
     /* check if a piece has to be moved to garbage and dos so if necc */

	VEC4 from, to;
	int tr,tc;
	int i,sid;
	double x_off, bordlenx,bordleny,angle;
	chess_piece_t *piece;

	tr = torow(move);
	tc = tocol(move);
	
	if(board->field[tr][tc] == (chess_piece_t *) NULL)
	{
#ifdef VERBOSE
		printf("NO PIECE AT TO POS\n");
#endif
		return;
	}
#ifdef VERBOSE
	printf(" PIECE AT TO POS\n");
#endif

     /* find empty garbage */
	piece = board->field[tr][tc];
	if(piece && piece->side)
	  sid = piece->side;
        else {
#ifdef VERBOSE
                printf("NO PIECE AT TO POS\n");
#endif
                return;
        }

	for(i=0;i<16 && board->garbage[sid][i] != (chess_piece_t *) NULL ;i++)
		;
	if (i==16)
	{
		fprintf(stderr,"no more garbage side %d, exiting \n",sid);
		return;
	}
	     /* twee rijen garbage naast elkaar */
	if(i<8)
		x_off = board->sur_x + GARBMARGE;
	else
		x_off = board->sur_x + board->delta_x + GARBMARGE;
	angle = board->theta / 180 * M_PI;
	if (sid == white_player)
	{
	     /* wit stuk geslagen, zet rechts van bord */
		to[0] = board->x + x_off - sin(angle) * (i * board->delta_y);
		to[1] = board->y + cos(angle) * (i * board->delta_y);
		to[2] = 0;   /* put on floor */
		to[3] = 1;  /* homogene coordinaten */
	}
	else
	{
	     /* zwart, zet links van bord */
	     	     /* first set x,y to A1 */
		bordlenx = 8 * board->delta_x;
		bordleny = 8 * board->delta_y;
		to[0] = board->x - (bordlenx * cos(angle) + bordleny * sin(angle));
		to[0] -=  x_off - sin(angle) * ( i * board->delta_y);
		to[1] = board->y + (bordlenx * sin(angle) + bordleny * cos(angle));
		to[1] -= cos(angle) * (i * board->delta_y);
                to[2] =  0;   /* put on floor */
                to[3] = 1;  /* homogene coordinaten */
	}

    /* what are coordinates piece is moved from */
	spp_board_to_cart(board,torow(move),tocol(move),from);

     /* actually move the piece */
	spp_to_garb = 1;
	piece = board->field[torow(move)][tocol(move)];
	spp_path(board,  piece, from, to, list);

}


void
spp_move (board,move,list)
     chess_board_t *board;
     chess_move_tm *move;
     xyz_list_t *list;
{
     	VEC4 from, to;
	chess_piece_t *piece;

	spp_board_to_cart(board,fromrow(move),fromcol(move),from);
	spp_board_to_cart(board,torow(move),tocol(move),to);

/* actually move the piece */
	piece = board->field[fromrow(move)][fromcol(move)];
	if (piece == (chess_piece_t *) NULL)
	{
		fprintf(stderr,"missing piece\n");
		return;
	}
        spp_path (board, piece, from, to, list);
}



void
spp_path (board, piece, from, to, list)
     chess_board_t *board;
     chess_piece_t *piece;
     VEC4 from, to;
     xyz_list_t *list;
{
#ifdef TYPEDEF
	xyz_list_item_t *first,*L1, *L2, *L3;
#else
	xyz_list_t first,L1,L2,L3;
#endif

     /* add piece height, scale */
	from[0] = from[0] / from[3];
	from[1] = from[1] / from[3];
	from[2] = from[2] / from[3] + (0.5 * piece->height);
	from[3] = 1.0;
	to[0] = to[0] / to[3];
	to[1] = to[1] / to[3];
	to[2] = to[2] / to[3] + (0.5 * piece->height);
	to[3] = 1.0;


      /* a move is divided in three parts */

	L1  = spp_pickup(board, piece, from);

	L2 = spp_safe_travel(board, piece, from, to);   /* move high above board */

	L3 = spp_place(board, piece, to);

     /* check if parts succeed */
	if ( L1 == (xyz_list_t ) NULL || L2 == (xyz_list_t ) NULL || L3 == (xyz_list_t ) NULL)
		*list = (xyz_list_t ) NULL;
	else
	{
     /* connect parts */
		if (*list == (xyz_list_t ) NULL)
		{
			*list = L1;
		}
		else
		{
			for(first = *list;first->next_data!=(xyz_list_t ) NULL;first = first->next_data)
				;
			first->next_data = L1;
		}
		if (!L1)
			printf("no pickup list\n");
		for(;L1->next_data!=(xyz_list_t ) NULL;L1=L1->next_data)
				;
		if (!L1)
			printf("no path list\n");
		L1->next_data = L2;
		if (!L1)
			printf("no place list\n");
		for(;L1->next_data!=(xyz_list_t ) NULL;L1=L1->next_data)
			;
		L1->next_data = L3;
		if (!L1)
			printf("no final list\n");
	}
}


xyz_list_t   
spp_pickup(board,piece,vec)
     chess_board_t *board;
     chess_piece_t *piece;
     VEC4 vec;
{
     /* pickup 'piece' from vec */

	xyz_list_item_t *first, *cur;
	VEC4 close, above;
	int grip;

	if (piece == (chess_piece_t *) NULL)
                grip = GRIPPER_OPEN;
        else
                grip = FRACT * piece->diameter;

	spp_high_above(board,vec,above);   /* move above piece */
	first = spp_node(above,GRIPPER_OPEN,board);
	cur = first;
	spp_close(board,vec,close);   /* move close to piece */
	cur->next_data =  spp_node(close,GRIPPER_OPEN,board);
	cur = cur->next_data;
	cur->next_data = spp_node(vec,GRIPPER_OPEN,board);
	cur = cur->next_data;
	cur->next_data = spp_node(vec,grip,board);
#ifdef LIFT_AFTER_GRIP
	cur = cur->next_data;
	cur->next_data = spp_node(close,grip,board);
#endif
	return first;
}


xyz_list_t   
spp_safe_travel(board, piece, from, to)
     chess_board_t *board;
     chess_piece_t *piece;
     VEC4 from, to;
{
    /* move from to above all pieces */

	int grip;
	VEC4 above;
	xyz_list_t cur;

	if (piece == (chess_piece_t *) NULL)
		grip = GRIPPER_OPEN;
	else
		grip = FRACT * piece->diameter;

/*
	of = from[2];
	from[2] = from[3] * SAFE_HEIGHT;
	cur = spp_node(from,grip,board);
	from[2] = of;
	ot = to[2];
	to[2] = to[3] * SAFE_HEIGHT;
	cur->next_data = spp_node(to,grip,board);
	to[2] = ot;
	return cur;
*/
	spp_high_above(board,from,above);
	cur =  spp_node(above,grip, board);
	spp_high_above(board,to,above);
	 cur->next_data = spp_node(above,grip,board);
	return cur;
}


xyz_list_t   
spp_place(board, piece, vec)
     chess_board_t *board;	
     chess_piece_t *piece;
     VEC4 vec;
{
     /* place a piece on board */

	VEC4 norm,close, above;
	xyz_list_t cur;
	int grip;

	if (piece == (chess_piece_t *) NULL)
                grip = GRIPPER_OPEN;
        else
                grip = FRACT * piece->diameter;
	
	/* dirty */
	if (spp_to_garb == 1)
	{
		/* temporarily change normal */
		norm[0] = board->n1;
		norm[1] = board->n2;
		norm[2] = board->n3;
		board->n1 = 0;
		board->n2 = 0;
		board->n3 = 1;
	}
	spp_close(board,vec,close);
	spp_high_above(board,vec,above);
	cur = spp_node(close,grip,board);
	cur->next_data = spp_node(vec,grip,board);
	cur->next_data->next_data = spp_node(vec,GRIPPER_OPEN,board);
	cur->next_data->next_data->next_data = spp_node(above,GRIPPER_OPEN,board);

	if (spp_to_garb == 1)
	{
		/* restore normel */
		board->n1 = norm[0];
		board->n2 = norm[1];
		board->n3 = norm[2];
		spp_to_garb = 0;
	}
	return cur;
}





void
spp_high_above(board, vec, above)
     chess_board_t *board;
     VEC4 vec, above;
{
     /* calculate position high above vec following normal */

	above[0] = vec[0]/vec[3] + (ABOVE_BOARD * board->n1);
        above[1] = vec[1]/vec[3] + (ABOVE_BOARD * board->n2);
        above[2] = vec[2]/vec[3] + (ABOVE_BOARD * board->n3);
        above[3] = 1;
}


void 
spp_close(board, vec, close)
     chess_board_t *board;
     VEC4 vec, close;
{
     /* calculate position close to vec following normal of board */

	close[0] = vec[0]/vec[3] + (CLOSE_TO_BOARD * board->n1);
	close[1] = vec[1]/vec[3] + (CLOSE_TO_BOARD * board->n2);
	close[2] = vec[2]/vec[3] + (CLOSE_TO_BOARD * board->n3);
	close[3] = 1;
}


xyz_list_t   
spp_node(vec,gri,board)
     VEC4 vec;
     int gri;
     chess_board_t *board;
{
 	xyz_list_t node;

	if ((node = (xyz_list_t ) malloc(sizeof(xyz_list_item_t))) == (xyz_list_t ) NULL)
	{
		fprintf(stderr,"Cannot allocate node in path\n");
		return( (xyz_list_t ) NULL);
	}

	node->x = vec[0] / vec[3];
	node->y = vec[1] / vec[3];
	node->z = vec[2] / vec[3];
	node->grip = gri;
	node->n1 = board->n1;
	node->n2 = board->n2;
	node->n3 = board->n3;
	node->theta = board->theta;
	node->next_data = (xyz_list_t ) NULL;
	
	return node;
}


double atan3(y, x)
double y, x;
{
  double res;   /* result */

  if (y == 0)
    res = 0.0;   /* we define: arctan(0 / 0) := 0 */
  else if (x == 0)
    res = M_PI / 2;   /* infinite case */
  else
    res = atan(fabs(y) / fabs(x));

 if (x < 0)
    res = M_PI - res;

  if (y < 0)
    res = -res;

  return res;
}


void
spp_creatcol_eat (board)
     chess_board_t *board;
{
     /* create trans matrix */

	VEC4 normal;
	double alpha, beta, length, theta;
	MAT44 M1, M2, M3, M4;

#ifdef VERBOSE
	printf("create trans matrix\n");
#endif
     /* specify normal in bordfixed coor system */
	length = sqrt(board->n1 * board->n1 + board->n2 * board->n2 +
		board->n3 * board->n3);
#ifdef VERBOSE
	printf("normal is (%f, %f, %f) length %f\n",board->n1, board->n2, board->n3,length);
#endif
	normal[0] = -board->n1 / length;
	normal[1] =  board->n2 / length;
	normal[2] =  board->n3 / length;
	normal[3] = 1;
#ifdef VERBOSE
	printf("normal is (%f, %f, %f)\n",normal[0],normal[1],normal[2]);
#endif

	alpha = -atan3(normal[1], normal[2]);
#ifdef VERBOSE
	printf("calling rotx with %f\n",alpha);
#endif
	Rotx3D(alpha, M1);
#ifdef VERBOSE
	Mprint(M1);
#endif
	
	length = sqrt(normal[1] * normal[1] + normal[2] * normal[2]);
	beta = atan3(normal[0],length);
#ifdef VERBOSE
	printf("calling roty with %f\n",beta);
#endif
	Roty3D(beta,M2);
#ifdef VERBOSE
	Mprint(M2);
#endif

	theta = board->theta / 180 * M_PI;

#ifdef VERBOSE
	printf("calling rotz with %f\n",theta);
#endif
	Rotz3D(theta,M3);
#ifdef VERBOSE
	Mprint(M3);
#endif

	MMprod4 (M2, M3, M4);
#ifdef VERBOSE
	Mprint(M4);
#endif
	MMprod4(M1, M4, spp_trans);
#ifdef VERBOSE
	Mprint(spp_trans);
#endif
	
}


void
spp_board_to_cart(board, row, col, from)
	chess_board_t *board;
	int row, col;
	VEC4 from;
{
     /* transform from board to cartesian coordinates */
     /* PRE : spp_creatcol_eat has been called */

/*
	printf ("callc coords for row %d col %d\n",row,col);
*/
     /* set coordinates in board system */
	from[0] = (7 - col) * board->delta_x + board->sur_x + 0.5 * board->delta_x;
	from[1] = (7 - row) * board->delta_y + board->sur_y + 0.5 * board->delta_y;
	from[2] = board->board_thickness;
	from[3] = 1;
/*
	printf("boord coords before trans %f	%f %f\n",from[0],from[1],from[2]);
*/

     /* transform to board fixed system */
	MVprod4(spp_trans, from, from);
/*
	printf("boord coords %f	%f %f\n",from[0],from[1],from[2]);
*/

     /* to base system */
	from[0] = board->x - (from[0] / from[3]);
	from[1] = board->y + (from[1] / from[3]);
	from[2] = board->z + (from[2] / from[3]);
	from[3] = 1;
/*
	printf("cart coords %f	%f %f\n",from[0],from[1],from[2]);
*/
}
	
void
old_stub_Path_plan(hm,cm,xyz_list)
     chess_move_tm *hm, *cm;
     xyz_list_t *xyz_list;
{
        chess_board_t board;

	fprintf(stderr,"USING STANDARD PATH PLANNING PROCEDURE\n");

        spp_to_garb = 0;

        spp_read_board(&board);                 /* lees huidig board */

        spp_creatcol_eat(&board);                 /* create trasnf matrix */

        spp_recol_gove(&board,hm);                /* register human move in board file */

     /* create pointer */
	*xyz_list = (xyz_list_t ) malloc(sizeof(xyz_list_item_t));
	if (*xyz_list == (xyz_list_t ) NULL)
	{
		printf("Cannot allocate ptr\n");
		return;
	}
	*xyz_list = (xyz_list_t ) NULL;

	spp_draw_board();

        spp_to_garbage(&board, cm, xyz_list);   /* check if piece to garbage */

        spp_move (&board, cm, xyz_list);        /* move the moved piece */

        spp_recol_gove(&board, cm);                       /* register computer move */

        spp_write_board(&board);                /* save current bord pos */

/*
	for(p=*xyz_list;p!=(xyz_list_t ) NULL;p=p->next_data)
	{
		printf("moving to %f %f %f \n",p->x,p->y,p->z);
		printf("with normal %f %f %f and theta %f\n",p->n1,p->n2,p->n3,p->theta);
	}
*/
}



/************* from old pp file */

void
spp_draw_piece(kind, player)
char kind;
char player;
{  /** draw_piece **/
  switch (kind) {

  /** hoofdletters voor wit en kleine letters voor zwart **/
  case pawn_piece:   /** pion **/
    if (player == white_player)
      putchar('p');
    else
      putchar('P');
    break;

  case rook_piece:   /** toren **/
    if (player == white_player)
      putchar('r');
    else
      putchar('R');
    break;

  case knight_piece:   /** paard **/
    if (player == white_player)
      putchar('n');
    else
      putchar('N');
    break;

  case bishop_piece:   /** loper **/
    if (player == white_player)
      putchar('b');
    else
      putchar('B');
    break;

  case queen_piece:   /** dame **/
    if (player == white_player)
      putchar('q');
    else
      putchar('Q');
    break;

  case king_piece:   /** koning **/
    if (player == white_player)
      putchar('k');
    else
      putchar('K');
    break;
  }
}  /** draw_piece **/


void 
spp_draw_board()
{  /** draw_board **/
  char row;
  char column;
  short i;
  FILE *fp;
  chess_board_t  board;
  int num_items;


        printf("spp_draw_board: opening file %s\n",MYBOARD);
 	if ((fp = (FILE *) fopen (MYBOARD,"r")) == (FILE *) NULL)
        {
                fprintf(stderr, "Cannot read current veld file for deletion\n");
                return;
        }
 	num_items = fread(&board, 1, sizeof(chess_board_t), fp);
	printf("Read current board file %s with %d items in draw_board\n",MYBOARD,num_items);

        fclose(fp);


  for (i = 0; i <= 15; i++) {
    if (board.garbage[0][i] != NULL)
      spp_draw_piece(board.garbage[0][i]->kind, black_player);
  }
  printf("\n");
  for (row = 7; row >= 0; row--) {
    for (column = col_a; column <= col_h; column++) {
      if (board.field[(int )row][(int )(column - col_a)] == NULL)
        putchar(' ');
      else {
        /** wel een stuk op het veld **/
        spp_draw_piece(board.field[(int )row][(int )(column - col_a)]->kind,
                   board.field[(int )row][(int )(column - col_a)]->side);
      }
      /** geen stuk op het veld **/
    }
    putchar('\n');
  }
  for (i = 0; i <= 15; i++) {
    if (board.garbage[(int )(white_player - black_player)]
        [i] != NULL)
      spp_draw_piece(board.garbage[(int )(white_player - black_player)]
                 [i]->kind, white_player);
  }
  putchar('\n');
}  /** draw_board **/


void
spp_free_board()
{  /** free_board **/
  char row;
  char column;
  short i;
  FILE *fp;
  chess_board_t board;
  int num_items;


  printf("spp_free_board: open file %s\n",MYBOARD);
  if ((fp = (FILE *) fopen (MYBOARD,"r")) == (FILE *) NULL)
  {
     fprintf(stderr, "Cannot read current veld file for deletion\n");
     return;
  }
  num_items = fread(&board, 1, sizeof(chess_board_t), fp);

  printf("Read current board file %s with %d items in free_board\n",MYBOARD,num_items);
  fclose(fp);

  for (row = 0; row <= 7; row++) {
    for (column = col_a; column <= col_h; column++) {
      if (board.field[(int )row][(int )(column - col_a)] != NULL)
        free(board.field[(int )row][(int )(column - col_a)]);
    }
  }

  for (i = 0; i <= 15; i++) {
    if (board.garbage[0][i] != NULL)
      free(board.garbage[0][i]);
    if (board.garbage[(long)white_player - (long)black_player]
        [i] != NULL)
      free(board.garbage[(long)white_player - (long)black_player][i]);
  }
}  /** free_board **/

/******************************************************************************************/
/***											***/
/*** Initialisation of board move. That is, fill board and  garbage place with		***/
/*** initial positions.									***/
/***											***/
/******************************************************************************************/


void
stub_init_pp ()
{

	chess_board_t board;
	chess_set_t schaak_stukken;

	FILE *fp;
#if 0

     /* first read board from file */
        printf("stub_init_pp: opening file %s\n", BOARD);
	if ((fp = fopen(BOARD,"r")) == NULL)
        {
                fprintf(stderr,"Cannot open veld file during initialisation\n");
		return;
	}
	fread(&board, 1, sizeof(chess_board_t), fp);
	fclose (fp);

        printf("stub_init_pp: opening file %s\n", PIECES);
	if ((fp = fopen(PIECES,"r")) == NULL)
        {
                fprintf(stderr,"Cannot open piece file during init\n");
		return;
	}
	fread(&schaak_stukken, 1, sizeof(chess_set_t), fp);
	fclose(fp);
#endif
     /* initialise board */
	stub_init_board (&board,&schaak_stukken);
        spp_draw_board(board);

     /* and write back to file */
	printf("stub_init_pp: writing file %s\n", MYBOARD);

	if ((fp = fopen(MYBOARD,"w")) == NULL)
        {
                fprintf(stderr,"2Cannot open veld file during initialisation\n");
                fp = fopen("/tmp/board.rtx","w");
        }
        fwrite(&board, 1, sizeof(chess_board_t), fp);
        fclose (fp);


}


chess_piece_t *
stub_init_piece (sid, kin, rownr, colnr,schaak_stukken)
     int sid, kin, rownr, colnr;
     chess_set_t *schaak_stukken;
{
	chess_piece_t *new_piece;

	new_piece = (chess_piece_t *)malloc(sizeof(chess_piece_t));
	if (new_piece == (chess_piece_t *) NULL)
	{
		fprintf(stderr,"Cannot allocate new piece \n");
		return ((chess_piece_t *) NULL);
	}
	new_piece->dead_or_alive = alive;
	new_piece->side = sid;
	new_piece->kind = kin;

  	switch (kin) {
  	case pawn_piece:
    		new_piece->height =
                           schaak_stukken->pawn[0].height;
    		new_piece->diameter =
                           schaak_stukken->pawn[0].diameter;
    		break;
  	case rook_piece:
    		new_piece->height =
                           schaak_stukken->rook[0].height;
    		new_piece->diameter =
                           schaak_stukken->rook[0].diameter;
    		break;
  	case knight_piece:
    		new_piece->height =
                           schaak_stukken->knight[0].height;
    		new_piece->diameter =
                           schaak_stukken->knight[0].diameter;
    		break;
  	case bishop_piece:
    		new_piece->height =
                           schaak_stukken->bishop[0].height;
    		new_piece->diameter =
                           schaak_stukken->bishop[0].diameter;
    		break;
  	case queen_piece:
    		new_piece->height =
                           schaak_stukken->queen.height;
    		new_piece->diameter =
                           schaak_stukken->queen.diameter;
    		break;
  	case king_piece:
    		new_piece->height =
                           schaak_stukken->king.height;
    		new_piece->diameter =
                           schaak_stukken->king.diameter;
    		break;
  	}
	new_piece->row = (char) '0' + rownr;
	new_piece->column = (char) '0' + colnr;

	return new_piece;
}

void
stub_init_board (board,schaak_stukken)
     chess_board_t *board;
     chess_set_t *schaak_stukken;
{

	int i,j;
	
     /* "fill" garbage place */

	for(i=0;i<16;i++)
	{
		board->garbage[black_player][i] = (chess_piece_t *) NULL;
		board->garbage[white_player][i] = (chess_piece_t *) NULL;
	}


     /* put pieces on the board */
	     /* pionnen */
	for(i=col_a;i<=col_h;i++)
	{
		board->field[1][i] = stub_init_piece(white_player,pawn_piece,1,i,schaak_stukken);
		board->field[6][i] = stub_init_piece(black_player,pawn_piece,6,i,schaak_stukken);
	}
	     /* torens */
	board->field[0][col_a] = stub_init_piece(white_player,rook_piece,0,col_a,schaak_stukken);
	board->field[0][col_h] = stub_init_piece(white_player,rook_piece,0,col_h,schaak_stukken);
	board->field[7][col_a] = stub_init_piece(black_player,rook_piece,7,col_a,schaak_stukken);
	board->field[7][col_h] = stub_init_piece(black_player,rook_piece,7,col_h,schaak_stukken);
	     /* paarden */
	board->field[0][col_b] = stub_init_piece(white_player,knight_piece,0,col_b,schaak_stukken);
	board->field[0][col_g] = stub_init_piece(white_player,knight_piece,0,col_g,schaak_stukken);
	board->field[7][col_b] = stub_init_piece(black_player,knight_piece,7,col_b,schaak_stukken);
	board->field[7][col_g] = stub_init_piece(black_player,knight_piece,7,col_g,schaak_stukken);
	     /* lopers */
	board->field[0][col_c] = stub_init_piece(white_player,bishop_piece,0,col_c,schaak_stukken);
	board->field[0][col_f] = stub_init_piece(white_player,bishop_piece,0,col_f,schaak_stukken);
	board->field[7][col_c] = stub_init_piece(black_player,bishop_piece,7,col_c,schaak_stukken);
	board->field[7][col_f] = stub_init_piece(black_player,bishop_piece,7,col_f,schaak_stukken);
	     /* koningen */
	board->field[0][col_e] = stub_init_piece(white_player,king_piece,0,col_d,schaak_stukken);
	board->field[7][col_e] = stub_init_piece(black_player,king_piece,7,col_d,schaak_stukken);
	     /* koninginnen */
	board->field[0][col_d] = stub_init_piece(white_player,queen_piece,0,col_d,schaak_stukken);
	board->field[7][col_d] = stub_init_piece(black_player,queen_piece,7,col_d,schaak_stukken);
	     /*empty */
	for(i=2;i<=5;i++)
		for(j=col_a;j<=col_h;j++)
			board->field[i][j] = (chess_piece_t *) NULL;
}



/*
 * backward compatability
 */

void
PathPlanCorrect()
{
	fprintf(stderr,"this path planning function no longer exists. You are working with ancient software\n");
	return;
}
