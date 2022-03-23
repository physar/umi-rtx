/* io unit */

#include <stdio.h>

#include "config.h"
#ifndef ARNOUD
#include "basics.h"
#endif

#define OK 0

int load_board_pp(board)
	chess_board_t *board;
{
	FILE 		*chess_stream;
        size_t             num_items;
		
	chess_stream = fopen(MYBOARD,"r");

	if (chess_stream == NULL)
	{
		printf("Error in 'load_board'; couldn't open file.\n");
		return EXIT_FAILURE;     
	}


	num_items = fread(board, sizeof(chess_board_t), 1, chess_stream);
#ifdef ARNOUD
	printf("Succesfull opened file %s with %ld items\n",MYBOARD, num_items);
#endif

	if (fclose(chess_stream) != OK)
	{
		printf("Error in 'load_board'; couldn't close file.\n");
		return EXIT_FAILURE;     
	}
	return EXIT_SUCCESS;     
}

void write_board_pp(board)
	chess_board_t *board;
{
	FILE 		*chess_stream;
		
	chess_stream = fopen(MYBOARD,"w");

	if (chess_stream == NULL)
	{
		printf("Error in 'write_board'; couldn't open file.\n");
		return;     
	}

	fwrite(board, sizeof(chess_board_t), 1, chess_stream);

	if (fclose(chess_stream) != OK)
	{
		printf("Error in 'write_board'; couldn't close file.\n");
		return;     
	}

	fclose(chess_stream);
}

