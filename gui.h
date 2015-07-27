#ifndef GUI_H
#define GUI_H

#include "GUI-components.h"
#include <stdio.h>
#include <SDL.h>
#include <SDL_video.h>
#include <time.h>
#include "GUI-components.h"

/*loop handling events it is defined externally in GUI-components header file */
int endLoop = 0;

/*The attributes of the game screen */
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define SCREEN_BPP 32/* bits per pixel, 32- bit color will be used */
#define PIECE_SIZE 69/* the width and hight of the piece's corrseponding picture */
#define SLOT_NUM 5 /* slots for files */

/*The screen */
Widget *screen = NULL;

/******************************************************/
/* Slots For loading Or saving - working with files*/
#define Slot struct Slot

Slot{
	char 	*name;/* name of file to be loaded from slot */
	int 	isFree; /* is slot occupied by a file that has been saved to it */
};

/******************************************************/
#define Game struct Game
Game{
	game *startingGameSettings; /* the game as we start to play it. */
	game *gameInfo;/*this contains all we need to know about the game */
	int pieceWasClicked;/* we want to know if a piece of the users color was clicked */
	int clickedPieceRow, clickedPieceCol;
	int isCheckMate; /*1 if check, 2 if mate, 0 otherwise */
	int isLoadingAIMove;
	char promoteTo; /* store the player decision- to what do we need to promote (if not a promotion move than promoteTo=0) */
	void (*allocateGameFunc)		(int isGUI);
	void (*initializeGameFunc)		(int isGUI);
	void (*deleteGameFunc)			(int isGUI);
	void (*handleGameStartFunc)		(Widget* widget , int x , int y);
	int  (*saveGameFunc)			(game *g , const char *address);
	int  (*loadGameFunc)			(game *g , const char *address);
	void (*copyGameFun)				(game *from, game *to);
	int  (*ConsoleMain)				(game *g);
};

void loadGameFunct(
	int isGUI,
	void (*allocateGameFunc)		(int isGUI),
	void (*initializeGameFunc)		(int isGUI),
	void (*deleteGameFunc)			(int isGUI),
	void (*handleGameStartFunc)		(Widget* widget , int x , int y),
	int  (*saveGameFunc)			(game *g , const char *address),
	int  (*loadGameFunc)			(game *g , const char *address),
	void (*copyGameFunc)			(game *from, game *to),
	int  (*ConsoleMain)				(game *g)
	);

void showMenu();

void showGame( );

void handleChessGameStart(Widget* widget, int x, int y);

void BuildChessBoard(Widget* DrawBoard, boardp boardPointer);

int isPawnBeforePromotionMarked( );

void ShowSettingsWindow(Widget* widget, int x, int y);

void GUImakeAITurn( );

void showLoadingMenu(Widget *widget, int x, int y);

void showSavingMenu(Widget *widget, int x, int y);

void initializeSlots( );

void initializeChess(int isGUI);

void deleteChessGame(int isGUI);
void deleteSlots(int numOfSlotsLoadedSuccessfully);
void allocateChess(int isGUI);
#endif