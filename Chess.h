#ifndef CHESS_H
#define CHESS_H
#include "Minimax.h"
/*~~~~~~~~~~~~~~~~~~~~~~~~~~ GAME SETTINGS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void loadDefaultSettings(game *g);
int gameSettings(game *g);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~ GAME STATE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int makeUserTurn(char *command, char *commandIterator, game *g);
int stringToCastlingMove(game *g, move *m, char *str);
int stringToMove(game *g, move *m, char *str);
int play(game *g);
int isAITurn(game *g);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~ SAVING & LOADING ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int loadChessGame(game *g, const char *address);
int saveChessGame(game *g, const char *address);
int chessConsoleMain(game *g);
#endif 

