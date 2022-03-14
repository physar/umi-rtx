/**************************************************************************/
/* chess.h                                                      /\/\      */
/* Version 2.0   --  November 1994                              \  /      */
/*                                                              /  \      */
/* Author: Arnoud Visser, Joris van Dam                      _  \/\/  _   */
/*         University of Amsterdam                          | |      | |  */
/*         Dept. of Computer Systems                        | | /\/\ | |  */
/*         Kruislaan 403, NL 1098 SJ Amsterdam              | | \  / | |  */
/*         THE NETHERLANDS                                  | | /  \ | |  */
/*         arnoud@fwi.uva.nl, dam@fwi.uva.nl                | | \/\/ | |  */
/*                                                          | \______/ |  */
/* This software has been written for the robot course       \________/   */
/* at our department. No representations are made about                   */
/* the suitability of this software for any purpose other       /\/\      */
/* than education.                                              \  /      */
/*                                                              /  \      */
/* Release note 2.0: other names, enum for standard C           \/\/      */
/**************************************************************************/
#ifndef CHESS_H
#define CHESS_H

typedef char *chess_move_t;

#ifdef __STDC__

typedef enum {col_a, col_b, col_c, col_d,
              col_e, col_f, col_g, col_h} piece_col_t;
typedef enum {row_1, row_2, row_3, row_4,
              row_5, row_6, row_7, row_8} piece_row_t;
typedef enum {dead, alive} piece_state_t;
typedef enum {black_player, white_player} piece_side_t;
typedef enum {pawn_piece, rook_piece, knight_piece,
              bishop_piece, queen_piece, king_piece} piece_kind_t;

typedef struct chess_piece_s{
  piece_row_t   row;
  piece_col_t   column;
  double        height, diameter;   /* mm */
  piece_state_t dead_or_alive;
  piece_side_t  side;
  piece_kind_t  kind;
  int dummy;         /* due to allignment problems on SunOs 5.0 stations */
} chess_piece_t;

#else /* __STDC__ */

#define col_a           0
#define col_b           1
#define col_c           2
#define col_d           3
#define col_e           4
#define col_f           5
#define col_g           6
#define col_h           7

#define row_1           0
#define row_2           1
#define row_3           2
#define row_4           3
#define row_5           4
#define row_6           5
#define row_7           6
#define row_8           7

#define dead            0
#define alive           1

#define black_player    0
#define white_player    1

#define pawn_piece      0
#define rook_piece      1
#define knight_piece    2
#define bishop_piece    3
#define queen_piece     4
#define king_piece      5

typedef struct chess_piece_s{
  int row, column;		/* valid are 0 to 7     */
  double height, diameter;	/* mm */
  int dead_or_alive;		/* dead = 0,  alive = 1 */
  int side;			/* black = 0, white = 1 */
  int kind;			/* valid are 0 to 5     */
  int dummy;         /* due to allignment problems on sun sparc stations */
} chess_piece_t;

#endif /* __STDC__ */

typedef struct chess_set_s {
  chess_piece_t pawn[8];
  chess_piece_t rook[2];
  chess_piece_t knight[2];
  chess_piece_t bishop[2];
  chess_piece_t queen, king;
} chess_set_t;

typedef struct chess_board_s {
  double x, y, z, n1, n2, n3, theta;	/* Cartesian position of H8-edge    */
  double delta_x, delta_y;		/* Geometrical description          */
  double board_thickness;		/*    of the board                  */
  double sur_x, sur_y;			/*                                  */
  chess_piece_t *garbage[2][16];	/* Dynamical data, has to be linked */
  chess_piece_t *field[8][8];		/*    to the black&white chess_set  */
} chess_board_t;
#endif /*CHESS_H*/
