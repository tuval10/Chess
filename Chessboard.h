#ifndef CHESSBOARD_H
#define CHESSBOARD_H
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "assert.h"

 /*~~~~~~~~~~~~~~~~~~~~~~~~~~ DEFINITIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#define WHITE_P 'm'
#define BLACK_P 'M'
#define WHITE_K 'k'
#define BLACK_K 'K'
#define WHITE_B 'b'
#define BLACK_B 'B'
#define WHITE_R 'r'
#define BLACK_R 'R'
#define WHITE_KN 'n'
#define BLACK_KN 'N'
#define WHITE_Q 'q'
#define BLACK_Q 'Q'
#define EMPTY '_'
#define true 1
#define false 0
#define white 1
#define black 0
#define BOARD_SIZE 8
#define MAX_LEAVES 1000000
#define MAX_COMMAND_LENGTH 50
#define MAX_FILE_LINE_LENGTH 50
#define NUM_OF_SETS 13 /* include empty square doesn't include king */
#define WRONG_MINIMAX_DEPTH "Wrong value for minimax depth. The value should be between 1 to 4\n"
#define WRONG_POSITION "Invalid position on the board\n"
#define NO_PIECE "The specified position does not contain your piece\n"
#define NO_ROOK "Wrong position for a rook\n"
#define ILLEGAL_COMMAND "Illegal command, please try again\n"
#define ILLEGAL_MOVE "Illegal move\n"
#define ILLEGAL_CASTELING "Illegal castling move\n"
#define WROND_BOARD_INITIALIZATION "Wrong board initialization\n"
#define WROND_GAME_MODE "Wrong game mode\n"
#define WROND_PLAYER_COLOR "Wrong player color\n"
#define WROND_FILE_NAME "Wrong file name\n"
#define PROBLEM_DURING_READING "Problem occurred during reading the file\n"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~ MACROS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#define getPawnChar(player) (((player)==white)?WHITE_P:BLACK_P)
#define getKingChar(player) (((player)==white)?WHITE_K:BLACK_K)
#define getBishopChar(player) (((player)==white)?WHITE_B:BLACK_B)
#define getQueenChar(player) (((player)==white)?WHITE_Q:BLACK_Q)
#define getKnightChar(player) (((player)==white)?WHITE_KN:BLACK_KN)
#define getRookChar(player) (((player)==white)?WHITE_R:BLACK_R)
#define perror_message(func_name) (perror("Error: standard function %s has failed", func_name))
#define calloc_error() perror("Error: standard function calloc has failed")
#define fgets_error() perror("Error: standard function fgets has failed")
#define scanf_error() perror("Error: standard function scanf has failed")
#define malloc_error() perror("Error: standard function malloc has failed")
#define opponentColor(x) ((x)==white?black:white)
#define switchColor(x) ((x)=(opponentColor(x)))
#define playerCapitalString(x) (((x)==white)?"White":"Black")
#define intAbs(x) (((x)<0)?(-(x)):(x))

/*~~~~~~~~~~~~~~~~~~~~~~~~~~ GLOBAL VARIABLES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
extern const char allsets[];
extern const int score_per_set[];
extern const int max_moves_per_set[];
extern int standart_fail;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~ STRUCTS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
typedef char chessboard[BOARD_SIZE][BOARD_SIZE];
typedef chessboard* boardp;
typedef struct game{
	int castling[4]; /*bl, br, wl, wr*/
	boardp boardPointer;
	int gameMode, nextTurnPlayerColor;
	int userColor, depth, difficulty_best; /* for PvsAI only */
} game;

/*
*	a struct to represent a move while:
*	if it's a promotion move promoteTo=(char of promoted set), otherwise 0
*	if it's a castling move then [row,col]=[newRow,newCol]=rook position
*/
typedef struct move{
	int row,col,newRow,newCol;
	char promoteTo;
} move;


/*~~~~~~~~~~~~~~~~~~~~~~~~~~ FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~ SCORING ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int getPoints(char c, const char sets[], const int scores_per_sets[], const int Num_of_diff_sets);
int getPawnSecuringMoveScore(game *g, move *m);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~ BOARD MANIPULATION ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void initialize_board(boardp boardPointer);
void clear_board(boardp boardPointer);
void copy_board(boardp from, boardp to);
void print_board(boardp boardPointer);
int getBoardScore(boardp boardPointer, const char sets[], const int scores_per_sets[], const int Num_of_diff_sets);
int isKingThreatened(boardp boardPointer, int playerColor);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~ QUERIES ABOUT A SINGLE POSITION ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int occupier_color(int row, int col, boardp boardPointer);
int in_range(int row, int col);
int isPromoted(int row, int pawnColor);
int dark_square(int row, int col);
void getKingCoordinates(boardp boardPointer, int *row, int *col, int kingColor);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~ MOVE ITERATORS & HELPERS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
char getCharForPromotion(char *promotionString, int playerColor);
int findPawnFirstMove(boardp boardPointer, move *m);
int findPawnNextMove(boardp boardPointer, move *m);
int findQBRFirstMove(boardp boardPointer, move *m);
int findQBRNextMove(boardp boardPointer, move *m);
void get_direction_offset(move *m, int *rowOffset, int *colOffset);
int getFirstLegalMove(game *g, move *m );
int getFirstPieceMove(boardp boardPointer, move *m, int playerColor);
int getNextLegalMove(game *g, move *m );
int getNextMove(boardp boardPointer, move *m, int playerColor);
int getNextPiece(boardp boardPointer, int playerColor, int *row, int *col);
int getNextPieceMove(boardp boardPointer, move *m);
void updateCastling(game *g, move *m);
int isCastlingPossible(game *g, int isLeftCastling );
/*~~~~~~~~~~~~~~~~~~~~~~~~~~ FUNCTION FOR ONE MOVE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void printMove(move *m);
void applyMoveOnBoard(boardp boardPointer, move *m);
int applyMove(game *g, move *m);
int isCastlingMove(move *m);
int isKingInDangerAfterMove(boardp boardPointer,move *m);
int isLegalMove(game *g, move *m);
int isCheckOrMateMove(game *g, move *m);
int isMoveEqualWithoutPromotion(move *m1, move *m2);
int isMovesEqual(move *m1, move *m2);
void copyMove(move *from, move *to);
int isMoveLegal(game *g, move *searchFor);
/*~~~~~~~~~~~~~~~~~~~~~~~~~~ FUNCTION FOR ALL MOVES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int printMoves(game *g, int row,int col, int printingOptions, int *mateMoveExist, move *mateMove );
int printAllMoves(game *g, int printOrNot, int *mateMoveExist,move *mateMove );
int canPlayerMove(game *g );
int countAllMoves(game *g );
/*~~~~~~~~~~~~~~~~~~~~~~~~~~ GAME HELPERS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void copyChessGame(game *from, game *to);

#endif