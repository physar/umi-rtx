#include <stdio.h>
#include <stdlib.h>
#include "chess.h"
#include "xyz.h"
#include "pathplan.h"
#include "init_pathplan.c"
#include "Path_plan.c"

#define New( a )          (a *) calloc( (size_t)sizeof( a ), (size_t)1)

#if NEEDED
static void print_xyz_node(xyz)
xyz_list_t xyz;
{
    printf("xyz_node: \n");

    if (xyz == NULL)
        printf("NULL\n");
    else {
        printf("x:     %f\t", xyz->x);
        printf("y:     %f\t", xyz->y);
        printf("z:     %f\n", xyz->z);
        printf("grip:  %f\n", xyz->grip);
        printf("n1:    %f\t", xyz->n1);
        printf("n2:    %f\t", xyz->n2);
        printf("n3:    %f\n", xyz->n3);
        printf("theta: %f\n", xyz->theta);

        printf("next_data: ");

        if (xyz->next_data != NULL)
            printf("not NULL\n");
        else
            printf("NULL\n");
    }
}
#endif /* NEEDED */

#if NEEDED
static void print_xyz_list(xyz)
xyz_list_t xyz;
{
    xyz_list_t tmp;

    printf("xyz_list: \n");

    if (xyz == NULL)
        printf("NULL \n");
    else {
        tmp = xyz;

        while (tmp != NULL) {
            print_xyz_node(tmp);
            tmp = tmp->next_data;
        }
    }

}
#endif /* NEEDED */

int main()
{
    chess_move_tm *hmove, *cmove;
    // struct chess_board_tm *board;
    xyz_list_t xyz_path;

    xyz_path = NULL;

#ifdef NEEDED
    struct chess_piece_tm *default_piece;
    default_piece = New(chess_piece_tm);

    default_piece->dead_or_alive = 1;
    default_piece->side = 1;
    default_piece->kind = 1;
    default_piece->dimension.height = 1.0;
    default_piece->dimension.diameter = 1.0;
    default_piece->position.row = 1;
    default_piece->position.column = 1;
#endif /* NEEDED */

    printf("MAIN begin\n");

    init_pathplan();

    printf("after init_pathplan\n");

    cmove = New(chess_move_tm);
    hmove = New(chess_move_tm);

    printf("From (row column) To (row column): ");
    scanf("%c%c%c%c", &(cmove->from_row), &(cmove->from_col),
                      &(cmove->to_row), &(cmove->to_col));

    hmove->from_row = '4';
    hmove->from_col = '6';
    hmove->to_row = '4';
    hmove->to_col = '7';

    Path_plan(hmove, cmove, &xyz_path);
    printf("after Path_plan\n");

    print_xyz_list(xyz_path);

    printf("MAIN end\n");

    return(EXIT_SUCCESS);

}

