#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#include "chess.h"
#include "config.h"

#define NOFIELDS 64
#define NOPIECES 16

static chess_piece_t *
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

static void
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
                board->field[1][i] = stub_init_piece(white_player,pawn_piece,1,
i,schaak_stukken);
                board->field[6][i] = stub_init_piece(black_player,pawn_piece,6,
i,schaak_stukken);
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

static char
get_column(piece_col_t column)
{
	switch (column)
	{
		case col_a: return 'a';
		case col_b: return 'b';
		case col_c: return 'c';
		case col_d: return 'd';
		case col_e: return 'e';
		case col_f: return 'f';
		case col_g: return 'g';
		case col_h: return 'h';
		default:    return '?';
	}
}

static piece_col_t
get_piece_column(char column)
{
	switch (column)
	{
		case 'a': return col_a;
		case 'b': return col_b;
		case 'c': return col_c;
		case 'd': return col_d;
		case 'e': return col_e;
		case 'f': return col_f;
		case 'g': return col_g;
		case 'h': return col_h;
		default:    return 9;
	}
}

static char *get_kind_name (piece_kind_t kind)
{
	switch (kind)
	{
		case pawn_piece:
			return "pawn";
		case rook_piece:
			return "rook";
		case knight_piece:
			return "knight";
		case bishop_piece:
			return "bishop";
		case queen_piece:
			return "queen";
		case king_piece:
			return "king";
		default:    return "?";
	}
}

#define OK 0
static piece_kind_t get_piece_kind (const char *kind)
{
	if (strcmp (kind, "rook") == OK)
		return rook_piece;
	if (strcmp (kind, "knight") == OK)
		return knight_piece;
	if (strcmp (kind, "bishop") == OK)
		return bishop_piece;
	if (strcmp (kind, "queen") == OK)
		return queen_piece;
	if (strcmp (kind, "king") == OK)
		return king_piece;

	/* default */
	return pawn_piece;
}

static piece_side_t get_piece_side (const char *side)
{
   if (strcmp (side, "black") == OK)
      return black_player;

   /* default */
   return white_player;
}
void chess_board_info (board)
chess_board_t *board;
{
#if 0
        int i;
	chess_piece_t *piece;
    printf("       loc height diam state side\n"); 

	piece = (chess_piece_t *)s;
    for (i = 0; i < NOFIELDS; i++)
    {
        printf("%6s: ",get_kind_name(piece->kind));
        printf("%c%d ",get_column(piece->column),piece->row);
        printf("%5.2f %5.2f ",piece->height, piece->diameter);
        printf("%5s ",piece->dead_or_alive ? "alive":"dead");
        printf("%5s ",piece->side ? "white":"black");
        printf("\n");
	piece++;

    }
#else
    if (board)
        printf("n1 %f\tx %f\ttheta %f\tdelta_y %f, sur_y %f\n",
	   board->n1,board->x,board->theta,board->delta_y,board->sur_y);
    else
        printf("chess_board_info: empty board\n");
#endif
}

int
write_xml_chess_board (char *name, chess_board_t *board)
{
   FILE  *fd;
   int i,j;
   chess_piece_t *piece;

    fd = fopen (name, "w");
    if (fd == NULL)
    {
        perror(name);
        fprintf(stderr,"Sorry, cannot store the details of the board.\n");
        fprintf(stderr,"Can't continue.\n");
        return -1;
    }
    fprintf(fd, "<?xml version='1.0'?>\n");
    fprintf(fd, "<chess_board>\n");

	fprintf(fd, "    <x>%5.2f</x>\n", board->x);
	fprintf(fd, "    <y>%5.2f</y>\n", board->y);
	fprintf(fd, "    <z>%5.2f</z>\n", board->z);
	fprintf(fd, "    <n1>%5.2f</n1>\n", board->n1);
	fprintf(fd, "    <n2>%5.2f</n2>\n", board->n2);
	fprintf(fd, "    <n3>%5.2f</n3>\n", board->n3);
	fprintf(fd, "    <theta>%5.2f</theta>\n", board->theta);

	fprintf(fd, "    <delta_x>%5.2f</delta_x>\n", board->delta_x);
	fprintf(fd, "    <delta_y>%5.2f</delta_y>\n", board->delta_y);

	fprintf(fd, "    <board_thickness>%5.2f</board_thickness>\n", board->board_thickness);

	fprintf(fd, "    <sur_x>%5.2f</sur_x>\n", board->sur_x);
	fprintf(fd, "    <sur_y>%5.2f</sur_y>\n", board->sur_y);
    for (i = 0; i < 8; i++)
    {
      for (j = 0; j < 8; j++)
      {
	piece = (chess_piece_t *)board->field[i][j]; /* first row, than col */
	if (piece && piece->dead_or_alive == alive && 
            piece->column == '0' + j &&
            piece->row == '0' + i)
	{
            fprintf(fd, "  <field>\n");
	    fprintf(fd, "    <location>%c%c</location>\n", get_column(piece->column - '0'),piece->row+1);
	    fprintf(fd, "    <side>'%s'</side>\n", piece->side ? "white": "black");
	    fprintf(fd, "    <kind>'%6s'</kind>\n", get_kind_name(piece->kind));

            fprintf(fd, "  </field>\n");
	}
#if 0
    fprintf(fd, "    <state>%d</state>\n", piece->dead_or_alive);
	fprintf(fd, "    <height>%5.2f</height>\n", piece->height);
	fprintf(fd, "    <diameter>%5.2f</diameter>\n", piece->diameter);
#endif
      }
    }
    for (i = 0; i < 2; i++)
    {
      for (j = 0; j < NOPIECES; j++)
      {
	piece = (chess_piece_t *)board->garbage[i][j];
	if (piece && piece->dead_or_alive == dead &&
            piece->column == i &&
            piece->column == piece->side &&
            piece->row == j)
	{
            fprintf(fd, "  <garbage>\n");
	    fprintf(fd, "    <location>%d%d</location>\n", piece->column,piece->row);
	    fprintf(fd, "    <side>%d</side>\n", piece->side);
	    fprintf(fd, "    <kind>'%6s'</kind>\n", get_kind_name(piece->kind));
            fprintf(fd, "  </garbage>\n");
	}
      }
    }
    fprintf(fd, "</chess_board>\n");
    fclose (fd);

    printf("Written information about chess_board into file:\n\t'%s'.\n", name);

    return 0;
}


int
read_xml_chess_board (char *name, chess_board_t *board)
{
   FILE  *fd;
   int i, items, strange = 0;
   float xml_version;
   char column, row, tag[50];
   chess_piece_t *piece;

    fd = fopen (name, "r");
    if (fd == NULL)
    {
        perror(name);
        fprintf(stderr,"Sorry, cannot read the details of board.\n");
        fprintf(stderr,"Can't continue.\n");
        return -1;
    }
    items = fscanf(fd, "<?xml version='%f'?>\n",&xml_version);
    items = fscanf(fd, "<%11s>\n",tag);

    if (items != 1 || strncmp (tag, "chess_board",5) != 0)
    {
        perror(name);
        fprintf(stderr,"Sorry, file doesn't contain expected xml-data.\n");
        fprintf(stderr,"Can't continue.\n");
        fclose (fd);
        return -1;
    }


    items = fscanf(fd, "    <x>%lf</x>\n",&(board->x));
    items = fscanf(fd, "    <y>%lf</y>\n",&(board->y));
    items = fscanf(fd, "    <z>%lf</z>\n",&(board->z));
    items = fscanf(fd, "    <n1>%lf</n1>\n",&(board->n1));
    items = fscanf(fd, "    <n2>%lf</n2>\n",&(board->n2));
    items = fscanf(fd, "    <n3>%lf</n3>\n",&(board->n3));
    items = fscanf(fd, "    <theta>%lf</theta>\n",&(board->theta));
    items = fscanf(fd, "    <delta_x>%lf</delta_x>\n",&(board->delta_x));
    items = fscanf(fd, "    <delta_y>%lf</delta_y>\n",&(board->delta_y));
    items = fscanf(fd, "    <board_thickness>%lf</board_thickness>\n",&(board->board_thickness));
    items = fscanf(fd, "    <sur_x>%lf</sur_x>\n",&(board->sur_x));
    items = fscanf(fd, "    <sur_y>%lf</sur_y>\n",&(board->sur_y));

    items = fscanf(fd, "  <%[^>]>\n", tag);
    while (items == 1 && strcmp(tag,"field") == OK)
    {
        items = fscanf(fd, "    <location>%c%c</location>\n",
			&column,&row);
        if (board->field[row-'1'][column-'a'] != NULL)
        { 
           strange = 0;
           piece = (chess_piece_t *)board->field[row-'1'][column-'a'];
        } else {
           strange = 1;
           piece = NULL;
        }
        items = fscanf(fd, "    <side>'%[^']'</side>\n",tag);
        if (items && piece && get_piece_side(tag) != piece->side)
           strange = 1;
	items = fscanf(fd, "    <kind>'%[^']'</kind>\n", tag);
        if (items && piece && get_piece_kind(tag) != piece->kind)
           strange = 1;
        items = fscanf(fd, "  </%[^>]>\n",tag);

        if (strange)
        {
           if (piece)
              fprintf(stderr, "Trying to overwrite field '%c%c'\n",column, row);
           else
              fprintf(stderr, "Trying to write uninitialized field '%c%c'\n",column,row);
        }

        items = fscanf(fd, "  <%[^>]>\n", tag); /* new tag */
    };
    while (items == 1 && strcmp(tag,"garbage") == OK)
    {
        items = fscanf(fd, "    <location>%c%c</location>\n",
                        &column,&row);
        items = fscanf(fd, "    <side>'%[^']'</side>\n",tag);
        items = fscanf(fd, "    <kind>'%[^']'</kind>\n", tag);
        items = fscanf(fd, "  </%[^>]>\n",tag);


        items = fscanf(fd, "  <%[^>]>\n", tag); /* new tag */
    };

    fclose (fd);
    printf("Read information about chess_board from file:\n\t'%s'.\n", name);

    return 0;
}


int
read_binary_chess_board (char *name, chess_board_t *chess_board)
{
    int fd, white_pieces=0, black_pieces=0;
    chess_piece_t *piece;

    fd = open (name, O_RDWR);
    if (fd < 0)
    {
        perror(name);
        fprintf(stderr,"Sorry, don't know the lengths of the arm.\n");
        fprintf(stderr,"Can't continue.\n");
        return -1;
    }
    if (read (fd, chess_board, sizeof (chess_board_t)) < sizeof (chess_board_t))
    {
        perror ("bad read");
        fprintf(stderr,"Sorry, couldn't read all information for the chess_board.\n");
        fprintf(stderr,"Maybe I could try to deduce some more information..\n");
        fprintf(stderr,"But quitting is easier.\n");
        return -1;
    }
    close (fd);
    printf("Read information about chess_board from file:\n\t'%s'.\n", name);
#if 0
    /* check concistency */
    piece = (chess_piece_t *)board->field;
    for (i = 0; i < NOFIELDS; i++)
    {
	if (piece && piece->dead_or_alive == alive)
	{
		if (piece->side == white)
		   white_pieces++;
		if (piece->side == black)
		   black_pieces++;
	}
	piece++;
    }
    piece = (chess_piece_t *)board->garbage;
    for (i = 0; i < NOPIECES * 2; i++)
    {
	if (piece && piece->dead_or_alive == dead)
	{
		if (piece->side == white)
		   white_pieces++;
		if (piece->side == black)
		   black_pieces++;
	}
	piece++;
    }
    if !(white_pieces == NOPIECES && black_pieces == NOPIECES)
    {
        piece = (chess_piece_t *)board->field;
        for (i = 0; i < NOFIELDS; i++)
        {
		piece->dead_or_alive = dead;
        }
        piece = (chess_piece_t *)board->garbage;
        for (i = 0; i < NOPIECES * 2; i++)
        {
		piece->dead_or_alive = dead;
        }
    }
#endif
    return 0;
}

int
write_binary_chess_board (char *name, chess_board_t *chess_board)
{
   int fd;

    fd = open (name, O_CREAT, O_RDWR);
    if (fd < 0)
    {
        perror(name);
        fprintf(stderr,"Sorry, cannot store the lengths of the arm.\n");
        fprintf(stderr,"Can't continue.\n");
        return -1;
    }
    if (write (fd, chess_board, sizeof (chess_board_t)) < sizeof (chess_board_t))
    {
        perror ("bad read");
        fprintf(stderr,"Sorry, couldn't write all information for the chess_board.\n");
        fprintf(stderr,"Maybe I could try to deduce some more information..\n");
        fprintf(stderr,"But quitting is easier.\n");
        return -1;
    }
    close (fd);
    printf("Written information about chess_board into file:\n\t'%s'.\n", name);

    return 0;
}

#ifdef MAIN
int
main()
{
    chess_board_t chess_board_solaris;
    chess_set_t   chess_set_solaris;
    chess_board_t chess_board_linux;
    int return_value;

#if 0
    read_binary_chess_set (PIECES, &chess_set_solaris);
    read_binary_chess_board (BOARD, &chess_board_solaris);
    stub_init_board ( &chess_board_solaris,  &chess_set_solaris);
    chess_board_info(&chess_board_solaris);
    write_xml_chess_board ("/home/arnoud/src/robotica/data/linux/board.rtx", &chess_board_solaris);
#else
    return_value = read_xml_chess_board ("/home/arnoud/src/robotica/data/board.rtx", &chess_board_linux);
    if (return_value >= 0)
        write_binary_chess_board ("/home/arnoud/src/robotica/data/linux/board.rtx", &chess_board_linux);
#endif
    return return_value;
}
#endif /* MAIN */

