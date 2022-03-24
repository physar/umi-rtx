/******************************************************************************/
/***                                                                        ***/
/*** Main program for to play chess                                         ***/
/*** Software originally written by                                         ***/
/***                                                                        ***/
/***    Arnoud Visser, a.visser@uva.nl                                      ***/
/***                                                                        ***/
/***    Reused by Arnoud after 10-20 years                                  ***/
/***    Latest modification: February 2022                                  ***/
/***                                                                        ***/
/******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "gnuchess.h"
#include "chess.h"
#include "path.h"
#include "pathplan.h"
#include "inverse.h"
#include "umi.h"
#include "chesscmds.h"


#ifdef __STDC__
extern  int     chess_init();
extern  char*   chess_command(char * move);
extern  int     chess_verify(char * move);
extern  void    chess_quit();
#else
extern  int     chess_init();
extern  char*   chess_command();
extern  int     chess_verify();
extern  void    chess_quit();
#endif

int
get_command(command, mode)
char *command;
mode_t mode;
{
  int validMove = 1;
  static int i = 1;
  static int color = WHITE;
  static int rocade = 0;
  static int white_rocade_possable = 1;
  static int black_rocade_possable = 1;
  static char next_command[6];
         char user_command[6];
  extern int debug;
         int no_scanned;

  printf("inside get_command(%s)\n", command);

  debug = 1;
  if(rocade) {
    strcpy(command,next_command);
    rocade = 0;
    if(color == WHITE) {
      if(debug)
        printf("%d. %4s\n",i, command);
      color = BLACK;
    } else {
      if(debug)
        printf("%d. ... %4s\n",i++,command);
      color = WHITE;
    }
    printf("exit 1\n");
    return(validMove);
  }


  if((color == WHITE && mode == AUTOMATIC) ||
     (color == BLACK && (mode == USER || mode == FORCE)    )  )
    chess_command("white");
  else
    chess_command("black");

  printf("before mode select\n");
  if(mode == FORCE) {
    chess_command("force");
    char* get_commandResult = chess_command(command);
    if(!get_commandResult)
      validMove = 0;
    chess_command("force");  
  } else if(mode == AUTOMATIC) {
    printf("before chess_command GO\n");
    strcpy(command, chess_command("go"));
    printf("after chess_command GO\n");
    if(command[0] == 'Q'){
       printf("Ready!\n");
       return(!validMove);
    }
  } else {
    printf("before chess_command USER\n");
    do {
      printf("give move:\t(or Quit)\n");
      no_scanned = scanf("%s",user_command);

      user_command[4] = '\n';
      user_command[5] = '\0';

    } while( no_scanned && user_command[0] != 'q' && user_command[0] != 'Q' &&
                         !chess_verify(user_command)); 
    if(user_command[0] == 'q' || user_command[0] == 'Q')
    {
      /* go AUTOMATIC */
      if(color == WHITE)
        chess_command("white");
      else
        chess_command("black");

      strcpy(command,chess_command("go"));
    } else {
      chess_command(user_command);
      chess_command("undo");
      strcpy(command,user_command);
    }
  }
  if(validMove)
  {
    if(color == WHITE) {
      if(debug)
        printf("%d. %4s\n",i, command);
      color = BLACK;
    } else {
      if(debug)
      {
        printf("%d. %4s\n",i, command);
        printf("%d. ... %4s\n",i++,command);
      }
      color = WHITE;
    }
  }

  if(white_rocade_possable & !strcmp(command,"e1g1")) {
    rocade = 1;
    strcpy(next_command,"h1f1");
  }
  if(white_rocade_possable & !strcmp(command,"e1c1")) {
    rocade = 1;
    strcpy(next_command,"a1d1");
  }
  if(white_rocade_possable & !strncmp(command,"e1",2)) {
    white_rocade_possable = 0;
  }

  if(black_rocade_possable & !strcmp(command,"e8g8")) {
    rocade = 1;
    strcpy(next_command,"h8f8");
  }
  if(black_rocade_possable & !strcmp(command,"e8c8")) {
    rocade = 1;
    strcpy(next_command,"a8d8");
  }
  if(black_rocade_possable & !strncmp(command,"e8",2)) {
    black_rocade_possable = 0;
  }

  if(rocade) {
    if(color == WHITE) {
      i--;
      color = BLACK;
    } else
      color = WHITE;
  }
  debug = 0;
  return(validMove);
}

#ifdef MAIN1

#define COUPLED 0

static void
move_list(l)
joint_list_t l;
{
#if 1 /* obsolete I hope, Matthijs */
  joint_list_t c;
  if (l == NULL)
  {
    printf("Can't execute an empty list.\n");
    return;
  }
  if(0) printf("  zed\tshoul\telbow\t  yaw\tpitch\t roll\t grip\n");

  c = l;
  do
  {
    if(0) printf("%5.1f\t%5.1f\t%5.1f\t%5.1f\t%5.1f\t%5.1f\t%5.1f\n",
    c->zed, c->shoulder, c->elbow, c->yaw, c->pitch, c->roll, c->grip);
#if 1
    if(COUPLED)
      umi_move(c->zed,
          c->shoulder,
          c->elbow,
          c->yaw,
          c->pitch,
          c->roll,
          c->grip);
    else
      umi_move(c->zed,
          c->shoulder,
          c->elbow,
          c->yaw - (c->elbow / 2.0),
          c->pitch,
          c->roll,
          c->grip);
#else
    if(COUPLED)
      move_real(c->zed,
          c->shoulder,
          c->elbow,
          c->yaw,
          c->pitch,
          c->roll,
          c->grip);
    else
      move_real(c->zed,
          c->shoulder,
          c->elbow,
          c->yaw - (c->elbow / 2.0),
          c->pitch,
          c->roll,
          c->grip);
#endif
  } while ( (c = c->next_data) != NULL );
  if(1) printf("Done.\n");
#endif
}

static void
print_xyz_list(l0)
xyz_list_t l0;
{
  xyz_list_t next;

  printf("xyz_list:\n");
  printf("    x\t    y\t    z\t   n1\t   n2\t   n3\ttheta\t grip\n");
  next = l0;
  while(next != NULL) {
    printf("%5.1f\t%5.1f\t%5.1f\t%5.1f\t%5.1f\t%5.1f\t%5.1f\t%5.1f\n",
    next->x,
    next->y,
    next->z,
    next->n1,
    next->n2,
    next->n3,
    next->theta,
    next->grip);
    next = next->next_data;
  }
}

static void 
free_joint_val(l)
joint_list_t l;
{
#if 0
  joint_list_t c;
  if (l == NULL)
  {
    printf("Can't free an empty list.\n");
    return;
  }
  do
  {
    c = l->next_data;
    free((char *)l);
  } while ( (l = c ) != NULL );
#endif
}

int
main()
{
  char command[80] = "black";
  xyz_list_t traject;
  joint_list_t configurations = NULL;
  int validMove;
#ifdef SPEAK
  int fd;

  fd = speak_open(0, 0);
  if (speak_load_samples("/home/stud/robotics/speak/phonemes"))
    exit(1);
#endif

  init_board();
  init_invkin();
#if 1
  if(1) umi_init();
#else
  if(0) init_arm();
#endif

  printf("before get_command(%s)\n", command);

  do {
#if 1
    validMove = get_command(command, AUTOMATIC);
#else
    validMove = get_command(command, USER);
#endif

  printf("after get_command\n");

  if(!validMove) {
    printf("Time to stop\n");
    break;
  }
#ifdef SPEAK
    speak_string(fd,command);
#endif
    pathplan(command, &traject);
    if(0) print_xyz_list(traject);
    inverse_kinematics(traject,&configurations);

    printf("before move_list\n");

    if(1) move_list(configurations);
    free_joint_val(configurations);
    configurations = NULL;

  } while(1);
#ifdef SPEAK 
  speak_close(fd);
#endif

  chess_quit();
}
#endif /* ifdef MAIN1 */

#ifdef MAIN2
#define CSIZE 80
int 
main()
{
  char user_command[CSIZE];

  while(1) 
  {
    printf("user      : ");
    fgets(user_command,CSIZE,stdin);
    chess_command(user_command);
  };
}
#endif /* ifdef MAIN2 */

#ifdef MAIN3
#define CSIZE 80
void
main()
{
  char user_command[CSIZE];

  while(1)
  {
    printf("move      : ");
    fgets(user_command,CSIZE,stdin);
    printf("verified  : %s\n", (chess_verify(user_command)==LEGAL) ? "true": "false");
  };
}
#endif /* ifdef MAIN3 */

#ifdef MAIN4
#define CSIZE 80
main()
{
  char user_command[CSIZE];

  while(1) 
  {
    printf("get user command: ");
    fgets(user_command,CSIZE,stdin);
    get_command(user_command, FORCE);
  };
}
#endif /* ifdef MAIN4 */
