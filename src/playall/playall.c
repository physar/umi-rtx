
/******************************************************************************/
/***                                                                        ***/
/*** Main program for to read and execute joints.txt list.                  ***/
/*** Software originally written by                     		    ***/
/***                                                                        ***/
/***    Arnoud Visser, a.visser@uva.nl   				    ***/
/***                                                                        ***/
/***    Software thorougly messed up by Matthijs Spaan                      ***/
/***    Reused by Arnoud after 10-20 years                                  ***/
/***    Latest modification: February 2022                                  ***/
/***                                                                        ***/
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "chess.h"
#include "path.h"

void move_list(joint_list_t l); /* move_list.c */

#define NO_WINDOW
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0


static void
read_joint_list (joint_list)
  joint_list_t* joint_list;
{
  FILE* fid;
  joint_list_t j_list;
  float zed, shoulder, elbow, yaw, pitch, roll, grip;
  int notfirst;
  
  fid = fopen("joints.txt", "r");
  if (fid == NULL){
    fprintf(stderr, "Cannot open file joints.txt\n Exiting...");
    exit(EXIT_FAILURE);
  }

  j_list = (joint_list_t) malloc(sizeof(joint_list_item_t));
  (*joint_list) = j_list;
  notfirst = 0;
  while(fscanf(fid,"%f %f %f %f %f %f %f \n", &zed, &shoulder, &elbow,
               &yaw, &pitch, &roll, &grip) != EOF)
  {
    if(notfirst == 1){
      j_list->next_data = (joint_list_t) malloc(sizeof(joint_list_item_t));
      j_list = j_list->next_data;
    }
    j_list->zed = zed;
    j_list->shoulder = shoulder;
    j_list->elbow = elbow;
    j_list->yaw = yaw;
    j_list->pitch = pitch;
    j_list->roll = roll;
    j_list->grip = grip;
    j_list->next_data = NULL;
    notfirst = 1;
  }
  fclose(fid);

}

typedef struct{ int row; int col;} board_index_t;


void 
board_to_indices(move, from, to)
  chess_move_t move;
  board_index_t* from;
  board_index_t *to;
{
  if(move != NULL){
    from->col = move[0] - 'a';
    from->row = move[1] - '1';
    to->col = move[2] - 'a';
    to->row = move[3] - '1';
  }
  else{
    printf("Empty move was passed to board_to_indices");
    printf("The program might crash\n");
  }
}


static void
playall()
{
  /* read the current joint_list, and execute them all */
  joint_list_t joint_list;

  read_joint_list(&joint_list);

  if (joint_list == (joint_list_t) NULL)
  {
    fprintf(stderr,"Inverse Kinematics Could not come up with solution. Exiting\n");
    exit (EXIT_FAILURE);
  }

  move_list(joint_list);
}

int main(int argc, char *argv[])
{
  playall();
  return(EXIT_SUCCESS);
}
