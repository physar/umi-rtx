#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "chess.h"
#include "config.h"

#define NOPIECES 16

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

void chess_set_info (s)
chess_set_t *s;
{
        int i;
	chess_piece_t *piece;
    printf("       loc height diam state side\n"); 

	piece = (chess_piece_t *)s;
    for (i = 0; i < NOPIECES; i++)
    {
        printf("%6s: ",get_kind_name(piece->kind));
        printf("%c%d ",get_column(piece->column),piece->row);
        printf("%5.2f %5.2f ",piece->height, piece->diameter);
        printf("%5s ",piece->dead_or_alive ? "alive":"dead");
        printf("%5s ",piece->side ? "white":"black");
        printf("\n");
	piece++;

    }
}

int
write_xml_chess_set (char *name, chess_set_t *s)
{
   FILE  *fd;
   int i;
   chess_piece_t *piece;

    fd = fopen (name, "w");
    if (fd == NULL)
    {
        perror(name);
        fprintf(stderr,"Sorry, cannot store the lengths of the arm.\n");
        fprintf(stderr,"Can't continue.\n");
        return -1;
    }
    fprintf(fd, "<?xml version='1.0'?>\n");
    fprintf(fd, "<chess_set>\n");
    fprintf(fd, "<number_of_pieces>%d</number_of_pieces>\n", NOPIECES);

	piece = (chess_piece_t *)s;
    for (i = 0; i < NOPIECES; i++)
    {
        fprintf(fd, "  <piece>\n");
	fprintf(fd, "    <kind>'%6s'</kind>\n", get_kind_name(piece->kind));
	fprintf(fd, "    <location>%c%d</location>\n", get_column(piece->column),piece->row);
	fprintf(fd, "    <height>%5.2f</height>\n", piece->height);
	fprintf(fd, "    <diameter>%5.2f</diameter>\n", piece->diameter);
	fprintf(fd, "    <state>%d</state>\n", piece->dead_or_alive);
	fprintf(fd, "    <side>'%s'</side>\n", piece->side ? "white": "black");
        fprintf(fd, "  </piece>\n");
	piece++;
    }
    fprintf(fd, "</chess_set>\n");
    fclose (fd);

    printf("Written information about chess_set into file:\n\t'%s'.\n", name);

    return 0;
}


int
read_xml_chess_set (char *name, chess_set_t *s)
{
   FILE  *fd;
   int i, items, no_pieces;
   float xml_version;
   char column, tag[50];
   chess_piece_t *piece;

    fd = fopen (name, "r");
    if (fd == NULL)
    {
        perror(name);
        fprintf(stderr,"Sorry, cannot read the lengths of the arm.\n");
        fprintf(stderr,"Can't continue.\n");
        return -1;
    }
    items = fscanf(fd, "<?xml version='%f'?>\n",&xml_version);
    items = fscanf(fd, "<%9s>\n",tag);

    if (items != 1 || strncmp (tag, "chess_set",5) != 0)
    {
        perror(name);
        fprintf(stderr,"Sorry, file doesn't contain expected xml-data.\n");
        fprintf(stderr,"Can't continue.\n");
        fclose (fd);
        return -1;
    }

    items = fscanf(fd, "<number_of_pieces>%d</number_of_pieces>\n", &no_pieces);

    if (items != 1 || no_pieces > NOPIECES)
    {
        perror(name);
        fprintf(stderr,"Sorry, file contains more pieces than expected\n");
        fprintf(stderr,"Can't continue.\n");
        fclose (fd);
        return -1;
    }

    piece = (chess_piece_t *)s;

    for (i = 0; i < no_pieces; i++)
    {
        items = fscanf(fd, "  <%s>\n", tag);
	items = fscanf(fd, "    <kind>'%[^']'</kind>\n", tag);
	piece->kind = get_piece_kind( tag );
        items = fscanf(fd, "    <location>%c%d</location>\n",
			&column,(int *)&(piece->row));
	piece->column = get_piece_column (column);
        items = fscanf(fd, "    <height>%lf</height>\n",&(piece->height));
        items = fscanf(fd, "    <diameter>%lf</diameter>\n",&(piece->diameter));
        items = fscanf(fd, "    <state>%d</state>\n",(int *)&(piece->dead_or_alive));
        items = fscanf(fd, "    <side>'%[^']'</side>\n",tag);
	if (strcmp (tag, "white") == OK)
			piece->side = white_player;
	if (strcmp (tag, "black") == OK)
			piece->side = black_player;
        items = fscanf(fd, "  <%s>\n",tag);
	piece++;
    }
    items = fscanf(fd, "  <%s>\n",tag);

    fclose (fd);
    printf("Read information about chess_set from file:\n\t'%s'.\n", name);

    return 0;
}


int
read_binary_chess_set (char *name, chess_set_t *chess_set)
{
    int fd;

    fd = open (name, O_RDWR);
    if (fd < 0)
    {
        perror(name);
        fprintf(stderr,"Sorry, don't know the lengths of the arm.\n");
        fprintf(stderr,"Can't continue.\n");
        return -1;
    }
    if (read (fd, chess_set, sizeof (chess_set_t)) < sizeof (chess_set_t))
    {
        perror ("bad read");
        fprintf(stderr,"Sorry, couldn't read all information for the chess_set.\n");
        fprintf(stderr,"Maybe I could try to deduce some more information..\n");
        fprintf(stderr,"But quitting is easier.\n");
        return -1;
    }
    close (fd);
    printf("Read information about chess_set from file:\n\t'%s'.\n", name);

    return 0;
}

int
write_binary_chess_set (char *name, chess_set_t *chess_set)
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
    if (write (fd, chess_set, sizeof (chess_set_t)) < sizeof (chess_set_t))
    {
        perror ("bad read");
        fprintf(stderr,"Sorry, couldn't write all information for the chess_set.\n");
        fprintf(stderr,"Maybe I could try to deduce some more information..\n");
        fprintf(stderr,"But quitting is easier.\n");
        return -1;
    }
    close (fd);
    printf("Written information about chess_set into file:\n\t'%s'.\n", name);

    return 0;
}

#ifdef MAIN
int
main()
{
    chess_set_t chess_set_solaris;
    chess_set_t chess_set_linux;
    int return_value;

#if 0
    read_binary_chess_set (PIECES, &chess_set_solaris);
    chess_set_info(&chess_set_solaris);
    write_xml_chess_set ("/home/arnoud/src/robotica/data/linux/pieces.rtx", &chess_set_solaris);
#else
    return_value = read_xml_chess_set ("/home/arnoud/src/robotica/data/pieces.rtx", &chess_set_linux);
    if (return_value >= 0)
    write_binary_chess_set ("/home/arnoud/src/robotica/data/linux/pieces.rtx", &chess_set_linux);
#endif
    return return_value;
}
#endif
