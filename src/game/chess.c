#include <stdio.h>
#include "gnuchess.h"
#include "chess.h"

extern	int	chess_send();
extern	int	chess_get();

int	
GnuChess_Game(hm, cm)
chess_move_t 	*hm, *cm;
{
char	buf[10];
int	i;

	if ((hm->from_col < a_m) || (hm->from_col > h_m) ||
            (hm->to_col < a_m)   || (hm->to_col > h_m)   ||
            (hm->from_row < 0)   || (hm->from_row > 7)   ||
            (hm->to_row < 0)     || (hm->to_row > 7)) {
                return(ILLEGAL);
        }


        buf[0] = hm->from_col + 'a';
        buf[1] = hm->from_row + '1';
        buf[2] = hm->to_col + 'a';
        buf[3] = hm->to_row + '1';
        buf[4] = '\n';
        buf[5] = '\0';


	if ((i = chess_send(buf)) < 0) {
		/* Send failed */
		cm->from_col = 8;
		cm->from_row = 8;
		cm->to_col = 8;
		cm->to_row = 8;
		return(i);
	}


	if ((i = chess_get(buf)) < 0) {
		cm->from_col = 8;
		cm->from_row = 8;
		cm->to_col = 8;
		cm->to_row = 8;
		return(i);
	}

	cm->from_col = buf[0] - 'a';
	cm->from_row = buf[1] - '1';
	cm->to_col = buf[2] - 'a';
	cm->to_row = buf[3] - '1';

	return(i);

}

