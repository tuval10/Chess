#ifndef MINIMAX_H
#define MINIMAX_H
#include "Chessboard.h"

/*~~~~~~~~~~ HELPERS FOR CALCULATING THE MINMAX TREE ~~~~~~~~~~*/

int AlphaBeta(game *g, move *moveFromPreviousBoard, int alpha, int beta);
void createSonGame(game *parent, game *son, move *moveFromParent);

/*~~~~~~~~~~ HELPERS FOR CALCULATING BEST DEPTH ~~~~~~~~~~*/

int getOpenessScore(game *g);
int calculateBestHeight(game *g);
int calculateBestHeight2(game *g);
/*~~~~~~~~~~ THE FUNCTIONS FOR GETTING THE MOVE ~~~~~~~~~~*/

move get_AI_Move(game *g);

typedef struct moveListNode{
	move nodeMove;
	double score;
	struct moveListNode *next, *prev;
} moveListNode;

typedef struct{moveListNode *firstNode;} moveDeque;


#endif