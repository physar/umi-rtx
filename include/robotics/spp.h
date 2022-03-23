/**************************************************************************/
/* spp.h                                                        /\/\      */
/* Version 1.0   --  March 2022                                 \  /      */
/*                                                              /  \      */
/* Author: Arnoud Visser                                     _  \/\/  _   */
/*         University of Amsterdam                          | |      | |  */
/*         Faculty of Science                               | | /\/\ | |  */
/*         Science Park 904, 1098 XH Amsterdam              | | \  / | |  */
/*         THE NETHERLANDS                                  | | /  \ | |  */
/*         a.visser@uva.nl                                  | | \/\/ | |  */
/*                                                          | \______/ |  */
/* This software has been written for the robot course       \________/   */
/* at our department. No representations are made about                   */
/* the suitability of this software for any purpose other       /\/\      */
/* than education.                                              \  /      */
/*                                                              /  \      */
/* Release note 1.0: The stub pathplanning functions            \/\/      */
/**************************************************************************/
#ifndef SPP_H
#define SPP_H

#include "basics.h"
#include "chess.h"
#include "matrix.h"
#include "path.h"


void       spp_path (chess_board_t *board, chess_piece_t *piece, VEC4 from, VEC4 to, xyz_list_t *list);
void       spp_read_board (chess_board_t *board);
void       spp_write_board ();
void       spp_recol_gove (chess_board_t *board, chess_move_tm *move);
void       spp_to_garbage (chess_board_t *board, chess_move_tm *move, xyz_list_t *list);
void       spp_move (chess_board_t *board, chess_move_tm *move, xyz_list_t *list);
xyz_list_t spp_pickup (chess_board_t *board, chess_piece_t *piece, VEC4 from);
xyz_list_t spp_safe_travel (chess_board_t *board, chess_piece_t *piece, VEC4 from, VEC4 to);
xyz_list_t spp_place (chess_board_t *board, chess_piece_t *piece, VEC4 to);
void       spp_high_above (chess_board_t *board, VEC4 vec, VEC4 above);
void       spp_close (chess_board_t *board, VEC4 vec, VEC4 close);
xyz_list_t spp_node (VEC4 vec, int gri, chess_board_t *board);
void       spp_creatcol_eat (chess_board_t *board);
void       spp_board_to_cart (chess_board_t *board, int row, int col, VEC4 from);

/************* from old pp file */

void       spp_draw_piec (char kind, char player);
void       spp_draw_board ();
void       spp_free_board ();
void       spp_print (chess_board_t *board);


#endif /*SPP_H*/
