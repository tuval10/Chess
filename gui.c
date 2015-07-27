#include "gui.h"

/* a pointer to all details of the game*/
Game* mainGame;
Slot *file_slots;

/*********************************************************************************************************/
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
	){
		(*allocateGameFunc)(isGUI);
		if(standart_fail) return;
		mainGame->allocateGameFunc=allocateGameFunc;
		mainGame->initializeGameFunc=initializeGameFunc;
		mainGame->deleteGameFunc=deleteGameFunc;
		mainGame->handleGameStartFunc=handleGameStartFunc;
		mainGame->saveGameFunc=saveGameFunc;
		mainGame->loadGameFunc=loadGameFunc;
		mainGame->ConsoleMain=ConsoleMain;
}

void loadChess(int isGUI){
	loadGameFunct(isGUI,allocateChess,initializeChess,deleteChessGame,handleChessGameStart,saveChessGame,loadChessGame,copyChessGame,chessConsoleMain);
}


/* 
*	allocating the resources for the game+initiallizing it
*	throws calloc errors
*/
void allocateChess(int isGUI)
{
	mainGame = (Game*)malloc(sizeof(Game));
	if(mainGame==NULL){ calloc_error(); standart_fail=true; return;}
	/* setting startingGameSetting to save the starting state */
	mainGame->gameInfo=(game *)malloc(sizeof(game));
	if(mainGame->gameInfo==NULL){
		malloc_error(); standart_fail=1;
		free(mainGame); 
		return;
	}
	mainGame->gameInfo->boardPointer = (chessboard *)calloc((BOARD_SIZE * BOARD_SIZE), sizeof(char));
	if(mainGame->gameInfo->boardPointer==NULL){ 
		calloc_error(); standart_fail=1; 
		free(mainGame->gameInfo); free(mainGame); 
		return;
	}
	if(isGUI){
		mainGame->startingGameSettings=(game *)malloc(sizeof(game));
		if(mainGame->startingGameSettings==NULL){ 
			malloc_error(); standart_fail=1; 
			free(mainGame->gameInfo->boardPointer); free(mainGame->gameInfo); free(mainGame); 
			return;
		}
	
		mainGame->startingGameSettings->boardPointer=(chessboard *)calloc((BOARD_SIZE * BOARD_SIZE), sizeof(char));
		if(mainGame->startingGameSettings->boardPointer==NULL){
			malloc_error(); standart_fail=1; 
			free(mainGame->startingGameSettings); free(mainGame->gameInfo->boardPointer); free(mainGame->gameInfo); free(mainGame); 
			return;
		}
		initializeSlots();
		if(standart_fail){ deleteChessGame(isGUI); return;}
	}
	initializeChess(isGUI);
	if(standart_fail){ 
		deleteChessGame(isGUI); 
		if(isGUI){deleteSlots(SLOT_NUM);}
		return;
	}
}

/*	
*	initiallize mainGame + slots for the game
*	PRECONDITION: the game was allocated
*/
void initializeChess(int isGUI){
	loadDefaultSettings(mainGame->gameInfo);
	if(isGUI){
		loadDefaultSettings(mainGame->startingGameSettings);
		mainGame->clickedPieceCol=0;
		mainGame->pieceWasClicked = 0; /* no piece clicked */
		mainGame->isCheckMate = 0; /* game in initial stage */
		mainGame->isLoadingAIMove = mainGame->clickedPieceCol = mainGame->clickedPieceRow=mainGame->promoteTo = 0;
	}
}

/* deletes the mainGame extern*/
void deleteChessGame(int isGUI){
	if(isGUI){
		free(mainGame->startingGameSettings->boardPointer); 
		free(mainGame->startingGameSettings); 
	}
	free(mainGame->gameInfo->boardPointer); 
	free(mainGame->gameInfo); 
	free(mainGame); 
	return;
}

/* 
*	gets a move after clicking the board in 2 locations, and check if we clicked ROOK than KING when they are in their initial spots 
*	if so updates m to be the correct castle movement.
*/
void graphicMoveToCastle(move *m){
	boardp ourBoard=mainGame->gameInfo->boardPointer;
	int currentPlayer=mainGame->gameInfo->nextTurnPlayerColor, firstRow=(currentPlayer==white)?0:7;
	int BothSquaresInFirstCol=((firstRow==m->newRow)&&(firstRow==m->row)), kingInitialCol=4;
	if(((*ourBoard)[m->row][m->col]!=getRookChar(currentPlayer))) return; /* first click not rook*/
	if(((*ourBoard)[m->newRow][m->newCol]!=getKingChar(currentPlayer))) return; /* second click not king*/
	if(!BothSquaresInFirstCol) return; /* at least one click is not in the correct row*/
	if(m->newCol!=kingInitialCol) return;  /* the king is not in the initial Col*/
	if((m->col!=7)&&(m->col!=0)) return; /*first click - not in coreners*/
	m->newCol=m->col;
	m->newRow=m->row;
}

/*
*	handle clicking on the board
*	throws calloc errors, malloc__error(), calloc_error(),SDL_LoadBMPError(), SDL_DisplayFormatError(), 
*   SDL_SetColorKeyError(), SDL_BlitSurfaceError(),SDL_FlipError()
*/
void handleBoardSelection(Widget *widget, int xClicked, int yClicked)
{
	int row = 0, col = 0;
	move moveToBeApplied={0};
	if(mainGame->isCheckMate==2)
		return;
	yClicked-=20;
	xClicked-=20;
	row = BOARD_SIZE - ( yClicked / PIECE_SIZE ) - 1;
	col =  ( xClicked / PIECE_SIZE );
	if(mainGame->pieceWasClicked){
		moveToBeApplied.row = mainGame->clickedPieceRow;
		moveToBeApplied.col = mainGame->clickedPieceCol;
		moveToBeApplied.newRow = row;
		moveToBeApplied.newCol = col;
		moveToBeApplied.promoteTo = mainGame->promoteTo;
		graphicMoveToCastle(&moveToBeApplied);
		if(isMoveLegal(mainGame->gameInfo,&moveToBeApplied)){
			if(standart_fail) return;
			mainGame->isCheckMate=applyMove(mainGame->gameInfo, &moveToBeApplied);
			if(standart_fail) return;
		}
		mainGame->pieceWasClicked = 0; /* need to reset the flag saying if we clicked */
		mainGame->promoteTo=0; /* reseting the option to promote */
		DeleteWidgets(&(screen->widgetChildren));
		
		/*PvsAI and AI Turn*/
		if( (mainGame->isCheckMate!=2) && (isAITurn(mainGame->gameInfo)) )
			GUImakeAITurn();
		else
			showGame();
	}

	else
	{   /*we might be clicking on a piece for movement and its with our color (player color) */
		if( occupier_color(row, col , mainGame->gameInfo->boardPointer) == mainGame->gameInfo->nextTurnPlayerColor )
		{
			mainGame->pieceWasClicked = 1;/* set the flag saying "we clicked!" */
			mainGame->clickedPieceCol = col;/* save coordinates of the click */
			mainGame->clickedPieceRow = row;
			if(isPawnBeforePromotionMarked()){
				if(standart_fail) return;
				mainGame->promoteTo=getQueenChar(mainGame->gameInfo->nextTurnPlayerColor);
			}
			/*color the appropriate square - do a label on the label place a button - we put the piece */
            DeleteWidgets(&(screen->widgetChildren));
	        showGame();
		}
	}
}

char *getPieceFileName(char piece, int markedPiece){
	if(!markedPiece){
		switch(piece){
		   case WHITE_P :	return "pics/whitePawn.bmp";
		   case BLACK_P :	return "pics/blackPawn.bmp";
		   case WHITE_K :	return "pics/whiteKing.bmp";
		   case BLACK_K :	return "pics/blackKing.bmp";
		   case WHITE_B :	return "pics/whiteBishop.bmp";
		   case BLACK_B :	return "pics/blackBishop.bmp";
		   case WHITE_R :	return "pics/whiteRook.bmp";
		   case BLACK_R :	return "pics/blackRook.bmp";
		   case WHITE_KN :	return "pics/whiteKnight.bmp";
		   case BLACK_KN :	return "pics/blackKnight.bmp";
		   case WHITE_Q :	return "pics/whiteQueen.bmp";
		   case BLACK_Q :	return "pics/blackQueen.bmp";
		   default: return "empty";
		}
	}
	else{
		switch(piece){
		   case WHITE_P :  return "pics/yWhitePawn.bmp";
		   case BLACK_P :  return "pics/yBlackPawn.bmp";
		   case WHITE_K :  return "pics/yWhiteKing.bmp";
		   case BLACK_K :  return "pics/yBlackKing.bmp";
		   case WHITE_B :  return "pics/yWhiteBishop.bmp";
		   case BLACK_B :  return "pics/yBlackBishop.bmp";
		   case WHITE_R :  return "pics/yWhiteRook.bmp";
		   case BLACK_R :  return "pics/yBlackRook.bmp";
		   case WHITE_KN : return "pics/yWhiteKnight.bmp";
		   case BLACK_KN : return "pics/yBlackKnight.bmp";
		   case WHITE_Q :  return "pics/yWhiteQueen.bmp";
		   case BLACK_Q :  return "pics/yBlackQueen.bmp";
		   default: return "empty";
		}
	}
}

/*
*	get a square on the board when mainGame->pieceWasClicked is on
*	check if this square suppose to be marked- because it represent legal move
*	PRECONDITION: mainGame->pieceWasClicked
*	throws calloc error
*/
int isMarked(int row, int col){
	move m={0};
	/*check if it needs to be marked because it's possible move*/
	m.row=mainGame->clickedPieceRow;
	m.col=mainGame->clickedPieceCol;
	m.newCol=col;
	m.newRow=row;
	graphicMoveToCastle(&m);
	return isMoveLegal(mainGame->gameInfo,&m);
}

/*
* paints the ChessBoard and the pieces on it
* DrawBoard - the widget on which we'll be drawing the pieces on our board
* boardPointer - a pointer to the board we want to show graphically
* DrawBoard - the widget on which we'll be drawing the pieces on our board
* boardPointer - a pointer to the board we want to show graphically
*	throws : malloc__error(), calloc_error(),SDL_LoadBMPError(), SDL_DisplayFormatError(), 
*   SDL_SetColorKeyError(), SDL_BlitSurfaceError(),SDL_FlipError()
*/
void BuildChessBoard(Widget* DrawBoardOn, boardp boardPointer)
{
	int cols = 0, rows = 0 , x_axis, y_axis, isMarkedFlag=false;
	char piece;
	Widget* selectedTile = NULL;
	Widget* DrawBoard = BuildButton(15, 15, 564, 564, "pics/chessBoard.bmp", DrawBoardOn, handleBoardSelection);
	if(standart_fail) return;
     
	for(rows = 0; rows < BOARD_SIZE; rows++)
	{
		for(cols = 0; cols < BOARD_SIZE; cols++)
		{
			x_axis =5+  cols*PIECE_SIZE; /*get coordinate x for drawing */
			y_axis =5+ (BOARD_SIZE - 1 - rows)*PIECE_SIZE; /*get coordinate y for drawing */

			piece =(*boardPointer)[rows][cols];
			
			if(mainGame->pieceWasClicked){
				if(((isMarkedFlag)=(isMarked(rows,cols)))){
					if(standart_fail) return;
					selectedTile = BuildLabel( x_axis , y_axis, PIECE_SIZE, PIECE_SIZE, dark_square(rows,cols)?"pics/tile_w.bmp":"pics/tile_b.bmp", DrawBoard);
					if(standart_fail) return;
				}
				if( piece != EMPTY ){   
					  if(isMarkedFlag)
						 BuildButton(0 , 0 , PIECE_SIZE, PIECE_SIZE, getPieceFileName(piece,false), selectedTile, handleBoardSelection);
					  else{
						  if(mainGame->clickedPieceCol==cols&&mainGame->clickedPieceRow==rows)
							 BuildButton( x_axis , y_axis , PIECE_SIZE, PIECE_SIZE, getPieceFileName(piece,true), DrawBoard, handleBoardSelection);
						  else
							 BuildButton( x_axis , y_axis , PIECE_SIZE, PIECE_SIZE, getPieceFileName(piece,false), DrawBoard, handleBoardSelection);
					  }
				}
			}
			
			else 
			
				if( piece != EMPTY ){   
				 BuildButton( x_axis , y_axis , PIECE_SIZE, PIECE_SIZE, getPieceFileName(piece,false), DrawBoard, handleBoardSelection);
			}
			if(standart_fail) return;
		}
	}
}

/* function handles the quiting of the game - when quit button is prssed */
void handleQuit(Widget* widget, int x, int y)
{
	endLoop = 1;
}

/*
*	handle restarting the game
*	throws : malloc__error(), calloc_error(),SDL_LoadBMPError(), SDL_DisplayFormatError(), 
*   SDL_SetColorKeyError(), SDL_BlitSurfaceError(),SDL_FlipError() 
*/
void handleRestart(Widget* widget, int x, int y)
{
	(*mainGame->copyGameFun)(mainGame->startingGameSettings,mainGame->gameInfo);
	(*mainGame->handleGameStartFunc)(widget, x, y);
}

/*
*	shows the main manu of the game in case someone clicked on "menu"
*	throws : malloc__error(), calloc_error(),SDL_LoadBMPError(), SDL_DisplayFormatError(), 
*   SDL_SetColorKeyError(), SDL_BlitSurfaceError(),SDL_FlipError() 
*/
void handleMenu(Widget* widget, int x, int y )
{
    DeleteWidgets(&(screen->widgetChildren));
	(*mainGame->initializeGameFunc)(true);
	showMenu();
}

/*
*	throws : malloc__error(), calloc_error(),SDL_LoadBMPError(), SDL_DisplayFormatError(), 
*   SDL_SetColorKeyError(), SDL_BlitSurfaceError(),SDL_FlipError() 
*/
void updatePromotionToQueen(Widget* widget, int optionNum, int y ){
	int player=mainGame->gameInfo->nextTurnPlayerColor;
	DeleteWidgets(&(screen->widgetChildren));
	mainGame->promoteTo=getQueenChar(player);
	showGame();
}

/*
*	throws : malloc__error(), calloc_error(),SDL_LoadBMPError(), SDL_DisplayFormatError(), 
*   SDL_SetColorKeyError(), SDL_BlitSurfaceError(),SDL_FlipError() 
*/
void updatePromotionToBishop(Widget* widget, int optionNum, int y ){
	int player=mainGame->gameInfo->nextTurnPlayerColor;
	DeleteWidgets(&(screen->widgetChildren));
	mainGame->promoteTo=getBishopChar(player);
	showGame();
}

/*
*	throws : malloc__error(), calloc_error(),SDL_LoadBMPError(), SDL_DisplayFormatError(), 
*   SDL_SetColorKeyError(), SDL_BlitSurfaceError(),SDL_FlipError() 
*/
void updatePromotionToRook(Widget* widget, int optionNum, int y ){
	int player=mainGame->gameInfo->nextTurnPlayerColor;
	DeleteWidgets(&(screen->widgetChildren));
	mainGame->promoteTo=getRookChar(player);
	showGame();
}

/*
*	throws : malloc__error(), calloc_error(),SDL_LoadBMPError(), SDL_DisplayFormatError(), 
*   SDL_SetColorKeyError(), SDL_BlitSurfaceError(),SDL_FlipError() 
*/
void updatePromotionToKnight(Widget* widget, int optionNum, int y )
{
	int player=mainGame->gameInfo->nextTurnPlayerColor;
	DeleteWidgets(&(screen->widgetChildren));
	mainGame->promoteTo=getKnightChar(player);
	showGame();
}

/* 
*	show promotion option on the game menu 
*	throws malloc errors
*/
void showPromotionScreen()
{
	int option, player=mainGame->gameInfo->nextTurnPlayerColor;
	Widget *mainWidget=screen->widgetChildren;
	if(mainGame->promoteTo==getQueenChar(player)) option=0;
	else if(mainGame->promoteTo==getBishopChar(player)) option=1;
	else if(mainGame->promoteTo==getRookChar(player)) option=2;
	else option=3;
	BuildButton(580+0*(PIECE_SIZE-20), 250, PIECE_SIZE, PIECE_SIZE, getPieceFileName(getQueenChar(player),option==0), mainWidget, updatePromotionToQueen);
	if(standart_fail){return;}
	BuildButton(580+1*(PIECE_SIZE-20), 250, PIECE_SIZE, PIECE_SIZE, getPieceFileName(getBishopChar(player),option==1), mainWidget, updatePromotionToBishop);
	if(standart_fail){return;}
	BuildButton(580+2*(PIECE_SIZE-20), 250, PIECE_SIZE, PIECE_SIZE, getPieceFileName(getRookChar(player),option==2), mainWidget, updatePromotionToRook);
	if(standart_fail){return;}
	BuildButton(580+3*(PIECE_SIZE-20), 250, PIECE_SIZE, PIECE_SIZE, getPieceFileName(getKnightChar(player),option==3), mainWidget, updatePromotionToKnight);
    if(standart_fail){return;}
}

/* 
*	add the check/mate label if there is chess to the game screen
*	throw malloc errors
*/
void showCheckLabel(){
	Widget *mainWidget=screen->widgetChildren;
	if(mainGame->isCheckMate)
	{
		BuildLabel(585, 325, 102, 49, "pics/Check.bmp", mainWidget);
		if(standart_fail){return;}
		
		if(mainGame->isCheckMate==2)
		{
			BuildLabel(685, 325, 102, 49, "pics/Mate.bmp", mainWidget);
		    if(standart_fail){return;}
		}
	}
}

/* 
*	Checks if the piece we clicked on is a pawn which can make a promotion move
*	throws calloc errors
*/
int isPawnBeforePromotionMarked()
{
	int playerColor=mainGame->gameInfo->nextTurnPlayerColor;
	move m={0};
	boardp ourBoard=mainGame->gameInfo->boardPointer;
	if(!(mainGame->pieceWasClicked)) return false;
	if((*ourBoard)[mainGame->clickedPieceRow][mainGame->clickedPieceCol]!=getPawnChar(playerColor))
		return false;
	if((playerColor==white?6:1)!=mainGame->clickedPieceRow) /*nor one row before the end*/
		return false;
	/* check if we can move to the last row*/
	m.row=mainGame->clickedPieceRow;
	m.col=mainGame->clickedPieceCol;
	if(findPawnFirstMove(ourBoard,&m)){
		do{
			if(!isKingInDangerAfterMove(ourBoard,&m))
				return true;
		}
		while((!standart_fail)&&(findPawnNextMove(ourBoard,&m)));
	}
	return false;
}


/* 
*	SHOWS GRAPHICALLY THE GAME WINDOW WITH THE CHESSBOARD AND ALL THE PIECES ON 
*	THE RIGHT SIDE AND ON THE LEFT HAND SIDE WILL SHOW A SET OF 4 BUTTONS: 
*	RESTART GAME BUTTON, SAVE GAME BUTTON, RETURN TO THE MAIN MENU BUTTON AND A QUIT BUTTON
*	throws : malloc__error(), calloc_error(),SDL_LoadBMPError(), SDL_DisplayFormatError(), 
*   SDL_SetColorKeyError(), SDL_BlitSurfaceError(),SDL_FlipError() 
*	PRECONDITION: screen has no childs
*/
void showGame()
{
	Widget *mainWidget = NULL;
	mainWidget = BuildLabel(0, 0, 996, 692, "pics/main.bmp", screen);
	if(standart_fail){return;}

	if(mainGame->isLoadingAIMove)
	{
		BuildLabel(590, 25, 190, 146, "pics/loading.bmp", mainWidget);
		if(standart_fail){return;}
	}
	else{
		BuildButton(620, 25, 127, 49, "pics/RestartGame.bmp", mainWidget, handleRestart);
		if(standart_fail){return;}
		BuildButton(590, 100, 186, 49, "pics/SaveGame.bmp", mainWidget, showSavingMenu);
		if(standart_fail){return;}
		BuildButton(590, 175, 190, 49, "pics/MainMenu.bmp", mainWidget, handleMenu);
		if(standart_fail){return;}
		BuildButton(610, 475, 152, 90, "pics/quit.bmp", mainWidget, handleQuit);
		if(standart_fail){return;}
	}

	if(  isPawnBeforePromotionMarked() && (!standart_fail) )
	{
		
		showPromotionScreen();
		if(standart_fail){return;}
	}
	showCheckLabel();
	if(standart_fail){return;}
	BuildChessBoard(mainWidget, mainGame->gameInfo->boardPointer);
	if(standart_fail){return;}

	DrawWidgets(screen);
}


/* SHOWS THE GAME, IF COMPUTER STARTS FIRST IT WILL MAKE ITS TURN AND WILL BE SHOWN. 
   THIS FUNCTION DEALS WITH AI STARTING FIRST AS WHITE
   throws : malloc__error(), calloc_error(),SDL_LoadBMPError(), SDL_DisplayFormatError(), 
   SDL_SetColorKeyError(), SDL_BlitSurfaceError(),SDL_FlipError() */
void handleChessGameStart(Widget* widget, int x, int y)
{
	copyChessGame(mainGame->gameInfo,mainGame->startingGameSettings); /*save settings */
	mainGame->pieceWasClicked = 0;
	mainGame->promoteTo=0;
	if((!canPlayerMove(mainGame->gameInfo))&&(!standart_fail)){ /*check for check/mate */
		mainGame->isCheckMate=2;
	}
	else if((!standart_fail)&&(isKingThreatened(mainGame->gameInfo->boardPointer,mainGame->gameInfo->nextTurnPlayerColor))&&(!standart_fail)){
		mainGame->isCheckMate=1;
	}
	else
		mainGame->isCheckMate=0; 
	if(standart_fail) return;
	DeleteWidgets(&(screen->widgetChildren));
	if( (mainGame->gameInfo->gameMode==2) && (mainGame->gameInfo->userColor!=mainGame->gameInfo->nextTurnPlayerColor) )
		GUImakeAITurn();
	else
		showGame();
}


/* 
*	generates the main menu of the game 
*	throws malloc_errors(), SDL_LoadBMPError(), SDL_DisplayFormatError(), SDL_SetColorKeyError(), SDL_BlitSurfaceError(), SDL_FlipError()
*/
void showMenu()
{
	Widget* mainLabel = NULL;

	mainLabel = BuildLabel(0, 0, 996, 692, "pics/main.bmp", screen);
	if(standart_fail){return;}
	BuildLabel(50, 50, 700, 124, "pics/logo.bmp", mainLabel);
	if(standart_fail){return;}
	BuildButton(250, 250, 312, 90, "pics/newGame.bmp", mainLabel, ShowSettingsWindow);
	if(standart_fail){return;}
	BuildButton(250, 350, 312, 90, "pics/loadGame.bmp", mainLabel, showLoadingMenu);
	if(standart_fail){return;}
	BuildButton(325, 450, 152, 90, "pics/quit.bmp", mainLabel, handleQuit);
	if(standart_fail){return;}
	DrawWidgets(screen);
	if(standart_fail){return;}
}

/* function handles choosing the white player 
   throws : malloc_error(), SDL_LoadBMPError(), SDL_DisplayFormatError(), SDL_SetColorKeyError(), SDL_BlitSurfaceError(), SDL_FlipError() */
void handleWP(Widget* widget, int x, int y){
	if((mainGame->gameInfo->gameMode==2)&&(mainGame->gameInfo->userColor==black)){ /*changed from WAIvsBP to WPvsBP*/
		mainGame->gameInfo->gameMode=1;
		ShowSettingsWindow(widget,x,y);
	}
}

/* function handles choosing the black player
   throws : malloc_error(), SDL_LoadBMPError(), SDL_DisplayFormatError(), SDL_SetColorKeyError(), SDL_BlitSurfaceError(), SDL_FlipError() */
void handleBP(Widget* widget, int x, int y){
	if((mainGame->gameInfo->gameMode==2)&&(mainGame->gameInfo->userColor==white)){ /*changed from WPvsBAI to WPvsBP*/
		mainGame->gameInfo->gameMode=1;
		ShowSettingsWindow(widget,x,y);
	}
}

/* function handles choosing the white AI 
   throws : malloc_error(), SDL_LoadBMPError(), SDL_DisplayFormatError(), SDL_SetColorKeyError(), SDL_BlitSurfaceError(), SDL_FlipError()*/
void handleWAI(Widget* widget, int x, int y){
	if(mainGame->gameInfo->gameMode==1){
		mainGame->gameInfo->gameMode=2;
		mainGame->gameInfo->userColor=black;
		mainGame->gameInfo->depth=1;
		mainGame->gameInfo->difficulty_best=false;
		ShowSettingsWindow(widget,x,y);
	}
}

/* function handles choosing the black AI 
   throws : malloc_error(), SDL_LoadBMPError(), SDL_DisplayFormatError(), SDL_SetColorKeyError(), SDL_BlitSurfaceError(), SDL_FlipError()*/
void handleBAI(Widget* widget, int x, int y){
	if(mainGame->gameInfo->gameMode==1){
		mainGame->gameInfo->gameMode=2;
		mainGame->gameInfo->userColor=white;
		mainGame->gameInfo->depth=1;
		mainGame->gameInfo->difficulty_best=false;
		ShowSettingsWindow(widget,x,y);
	}
}

/* function handles choosing depth 1 
throws : malloc_error(), SDL_LoadBMPError(), SDL_DisplayFormatError(), SDL_SetColorKeyError(), SDL_BlitSurfaceError(), SDL_FlipError()*/
void handle1(Widget* widget, int x, int y){
	mainGame->gameInfo->difficulty_best=false;
	if(mainGame->gameInfo->depth!=1)
		mainGame->gameInfo->depth=1;
	ShowSettingsWindow(widget,x,y);
}

/* function handles choosing depth 2 
throws : malloc_error(), SDL_LoadBMPError(), SDL_DisplayFormatError(), SDL_SetColorKeyError(), SDL_BlitSurfaceError(), SDL_FlipError()*/
void handle2(Widget* widget, int x, int y){
	mainGame->gameInfo->difficulty_best=false;
	if(mainGame->gameInfo->depth!=2)
		mainGame->gameInfo->depth=2;
	ShowSettingsWindow(widget,x,y);
}

/* function handles choosing depth 3 
throws : malloc_error(), SDL_LoadBMPError(), SDL_DisplayFormatError(), SDL_SetColorKeyError(), SDL_BlitSurfaceError(), SDL_FlipError()*/
void handle3(Widget* widget, int x, int y)
{
	mainGame->gameInfo->difficulty_best=false;
	if(mainGame->gameInfo->depth!=3)
		mainGame->gameInfo->depth=3;
	ShowSettingsWindow(widget,x,y);
}

/* function handles choosing depth 4 
throws : malloc_error(), SDL_LoadBMPError(), SDL_DisplayFormatError(), SDL_SetColorKeyError(), SDL_BlitSurfaceError(), SDL_FlipError()*/
void handle4(Widget* widget, int x, int y)
{
	mainGame->gameInfo->difficulty_best=false;
	if(mainGame->gameInfo->depth!=4)
		mainGame->gameInfo->depth=4;
	ShowSettingsWindow(widget,x,y);
}

/* function handles choosing depth 1 
throws : malloc_error(), SDL_LoadBMPError(), SDL_DisplayFormatError(), SDL_SetColorKeyError(), SDL_BlitSurfaceError(), SDL_FlipError()*/
void handleBest(Widget* widget, int x, int y)
{
	if(!(mainGame->gameInfo->difficulty_best))
	{
		mainGame->gameInfo->difficulty_best=true;
		ShowSettingsWindow(widget,x,y);
	}
}

/* 
*	add the difficulty to the screen if needed in the settings menu
*	throws : malloc_error() 
*/
void showDifficultyMenu(){
	Widget *settingsMenu = screen->widgetChildren;
	game *g=mainGame->gameInfo;
	BuildLabel(25 , 250 , 329, 82, "pics/difficulty.bmp", settingsMenu);
	if(standart_fail){return;}
	BuildButton(25 , 350 , 71, 71, ((!(g->difficulty_best))&&(g->depth==1))?"pics/y1.bmp":"pics/1.bmp", settingsMenu, handle1);
	if(standart_fail){return;}
	BuildButton(100 , 350 , 71, 71, ((!(g->difficulty_best))&&(g->depth==2))?"pics/y2.bmp":"pics/2.bmp", settingsMenu, handle2);
	if(standart_fail){return;}
	BuildButton(175 , 350 , 71, 71, ((!(g->difficulty_best))&&(g->depth==3))?"pics/y3.bmp":"pics/3.bmp", settingsMenu, handle3);
	if(standart_fail){return;}
	BuildButton(250 , 350 , 71, 71, ((!(g->difficulty_best))&&(g->depth==4))?"pics/y4.bmp":"pics/4.bmp", settingsMenu, handle4);
	if(standart_fail){return;}
	BuildButton(325 , 350 , 171, 71, (g->difficulty_best)?"pics/yBest.bmp":"pics/best.bmp", settingsMenu, handleBest);
}

/*
*	shows the settings menu for choosing game
*	throws : malloc_error(), SDL_LoadBMPError(), SDL_DisplayFormatError(), SDL_SetColorKeyError(), SDL_BlitSurfaceError(), SDL_FlipError()
*/
void ShowSettingsWindow(Widget* widget, int x, int y){
	int blackIsAI=false, whiteIsAI=false;
	Widget *settingsMenu = NULL;
	DeleteWidgets(&screen->widgetChildren);
	settingsMenu = BuildLabel(0, 0, 996, 692, "pics/main.bmp", screen);
	if(standart_fail){return;}
	BuildLabel(25 , 50 , 219, 82, "pics/white.bmp", settingsMenu);
	if(standart_fail){return;}
	BuildLabel(25 , 75+PIECE_SIZE , 219, 70, "pics/black.bmp", settingsMenu);
	if(standart_fail){return;}

	if(mainGame->gameInfo->gameMode==2){
		(mainGame->gameInfo->userColor==white)?(blackIsAI=true):(whiteIsAI=true);
		showDifficultyMenu(); 
		if(standart_fail){return;}
	}
	if(whiteIsAI){
		BuildButton(375 , 50 , PIECE_SIZE, PIECE_SIZE, "pics/whitePlayer.bmp", settingsMenu, handleWP);
		if(standart_fail){return;}
		BuildButton(400+PIECE_SIZE , 50 , PIECE_SIZE, PIECE_SIZE, "pics/yWhiteAI.bmp", settingsMenu, NULL);
		if(standart_fail){return;}
	}
	else{
		BuildButton(375 , 50 , PIECE_SIZE, PIECE_SIZE, "pics/yWhitePlayer.bmp", settingsMenu, NULL);
		if(standart_fail){return;}
		BuildButton(400+PIECE_SIZE , 50 , PIECE_SIZE, PIECE_SIZE, "pics/whiteAI.bmp", settingsMenu, handleWAI);
		if(standart_fail){return;}
	}
	if(blackIsAI){
		BuildButton(375 , 75+PIECE_SIZE , PIECE_SIZE, PIECE_SIZE, "pics/blackPlayer.bmp", settingsMenu, handleBP);
		if(standart_fail){return;}
		BuildButton(400+PIECE_SIZE , 75+PIECE_SIZE , PIECE_SIZE, PIECE_SIZE, "pics/yBlackAI.bmp", settingsMenu, NULL);
		if(standart_fail){return;}
	}
	else{
		BuildButton(375 , 75+PIECE_SIZE , PIECE_SIZE, PIECE_SIZE, "pics/yBlackPlayer.bmp", settingsMenu, NULL);
		if(standart_fail){return;}
		BuildButton(400+PIECE_SIZE , 75+PIECE_SIZE , PIECE_SIZE, PIECE_SIZE, "pics/blackAI.bmp", settingsMenu, handleBAI);
		if(standart_fail){return;}
	}
	BuildButton(375, 450, 123, 65, "pics/ok.bmp", settingsMenu, mainGame->handleGameStartFunc);
	if(standart_fail){return;}
	BuildButton(525, 450, 219, 65, "pics/cancel.bmp", settingsMenu, handleMenu);
	if(standart_fail){return;}
	DrawWidgets(screen);
}

/*
	gets a game where the next player is the AI. show loading Image until the computer makes it's move and then make the move on board
	PRECONDITON: DeleteWidgets(&(screen->widgetChildren)) was made
	throws : malloc__error(), calloc_error(),SDL_LoadBMPError(), SDL_DisplayFormatError(), 
	SDL_SetColorKeyError(), SDL_BlitSurfaceError(),SDL_FlipError() 
*/
void GUImakeAITurn(){
	move m; 
	mainGame->isLoadingAIMove=1;
	showGame();
	if(standart_fail){return;}
	m=get_AI_Move(mainGame->gameInfo);
	if(standart_fail){return;}
	mainGame->isCheckMate=applyMove(mainGame->gameInfo,&m);
	if(standart_fail){return;}
	DeleteWidgets(&screen->widgetChildren);
	mainGame->isLoadingAIMove=0;
	showGame();
}

/* 
IF FILE EXISTS RETURN 0 OTHERWISE RETURN 1 
0 - slot isn't free it contains a file and 1 - if slot is free
*/
int FileExists(const char *fname)
{
    FILE *file = NULL;
	file = fopen(fname, "r");
    if (file != NULL)
    {
        fclose(file);
        return 0;
    }
    return 1;
}

/*
*	INITIALIZE THE SLOTS IN COMPUTER MEMORY FOR LOAD/SAVE ACTIONS 
*	throws calloc errors
*/
void initializeSlots()
{
	int slotNum, i;
	char fileNumber[3]={0};/*file number in string form - slot number file stored here*/
	/* an array representing the slots*/
	file_slots = (Slot*)calloc(sizeof(Slot),SLOT_NUM);
	if(file_slots==NULL){standart_fail=true; malloc_error(); return;}
	for(i=0 ; i < SLOT_NUM ; i++)
	{
		slotNum=i+1;
		/*saving the number as a string to the buffer fileNumber */
		if((slotNum)/10==1){
			fileNumber[0]='1';
			fileNumber[1]=((slotNum)%10)+'0';
			fileNumber[2]='\0';
		}
		else{
			fileNumber[0]=((slotNum)%10)+'0';
			fileNumber[1]='\0';
		}
		/* file name is : chess1.xml chess(slot number).xml */
		file_slots[i].name = (char*)calloc(sizeof(char),12);
		if(file_slots[i].name==NULL){
			calloc_error(); standart_fail=1;
			deleteSlots(i-1); /* delete all the slots we created so far*/
			return;
		}

		strcpy(file_slots[i].name,"chess");
		strcat(file_slots[i].name, fileNumber);/* file name insertion*/
		strcat(file_slots[i].name,".xml");
        file_slots[i].isFree = FileExists(file_slots[i].name);
	}
}

/*gets a string starts with a number from 1 to 10 and returns this number*/
int atoi10(char *string){
	int x=(int)(*string-'0');
	if(strlen(string)==1) return x; /*1-char string*/
	else if(!isdigit(*(string+1))) return x; /*next char isn't a digit*/
	else return (x*10); /*num is 10*/
}

/* 
*	gets the name of the buttonImageSrc (from the form "pics\[num]_[1/0]" , 
*	updates slotNum to num and returns a string from the form chess[num].xml
*	throws calloc errors
*/
char* getFileName(char *buttonImageSrc, int *slotNum)
{
	char *fileName, *numIndex=buttonImageSrc;
	fileName = (char *) calloc (13, sizeof(char));
	if(fileName==NULL){
		standart_fail=true;
		calloc_error();
		return NULL;
	}
	while(!isdigit(*numIndex)) /*go to the first digit*/
		numIndex++;
	*slotNum=atoi10(numIndex);
	strcat(fileName ,"chess");
	(*slotNum==10)?	/*copies the number of the button */
		strncat(fileName,numIndex,2):
		strncat(fileName,numIndex,1);
	strcat(fileName , ".xml"); /*copies the number of the button */
	return fileName;
}

/*Recives a widget which is a button (namely a slot) and according to that buttton 
  constructs the name of its co-relating file and saves the game's info in a file: chess(file number).xml
  throws : malloc__error(), calloc_error(),SDL_LoadBMPError(), SDL_DisplayFormatError(), 
  SDL_SetColorKeyError(), SDL_BlitSurfaceError(),SDL_FlipError() */
void SaveSlotClicked(Widget *widget, int x, int y)
{
	int slotNumber = 0;
	char *fileName=getFileName(widget->source, &slotNumber);
	if(standart_fail) return;
	(*mainGame->saveGameFunc)(mainGame->gameInfo, fileName);
	free(fileName);
    file_slots[slotNumber-1].isFree = false;
	DeleteWidgets(&screen->widgetChildren);
	showGame();
}

/*Recives a widget which is a button (namely a slot) and according to that buttton 
  load the game properties from file: chess(file number).xml
  throws : malloc__error(), calloc_error(),SDL_LoadBMPError(), SDL_DisplayFormatError(), 
  SDL_SetColorKeyError(), SDL_BlitSurfaceError(),SDL_FlipError() */
void LoadSlotClicked(Widget *widget,int x, int y)
{
	int slotNumber = 0;
	char *fileName=getFileName(widget->source, &slotNumber);
	if(standart_fail) return;
	(*mainGame->loadGameFunc)(mainGame->gameInfo, fileName);
	free(fileName);
	ShowSettingsWindow(widget,x,y);
}

/*
*	shows the menu for loading files
*	throws malloc errors, SDL_LoadBMPError(), SDL_DisplayFormatError(), SDL_SetColorKeyError(), SDL_BlitSurfaceError(), SDL_FlipError()
*/
void showLoadingMenu(Widget *widget, int x, int y)
{
	Widget* loadMenu;
	int slotNum = 0;
	char *image_source;
	char *fullSlotFiles[]={"pics/1_0.bmp","pics/2_0.bmp","pics/3_0.bmp","pics/4_0.bmp","pics/5_0.bmp","pics/6_0.bmp","pics/7_0.bmp","pics/8_0.bmp","pics/9_0.bmp","pics/10_0.bmp"};
	char *emptySlotFiles[]={"pics/1_1.bmp","pics/2_1.bmp","pics/3_1.bmp","pics/4_1.bmp","pics/5_1.bmp","pics/6_1.bmp","pics/7_1.bmp","pics/8_1.bmp","pics/9_1.bmp","pics/10_1.bmp"};
	
	DeleteWidgets(&screen->widgetChildren);
	loadMenu = BuildLabel(0, 0, 996, 692, "pics/main.bmp", screen);
	if(standart_fail) return;
	BuildLabel( 50, 25, 700, 92, "pics/loadDest.bmp", loadMenu);
	if(standart_fail) return;
	for(slotNum = 0; slotNum < SLOT_NUM; slotNum++)
	{
		image_source=(file_slots[slotNum].isFree)?emptySlotFiles[slotNum]:fullSlotFiles[slotNum];
		if( file_slots[slotNum].isFree )
		{   /* if the slot is free means no file was saved to that slot, so we need to put a label */
			if(slotNum < 5){
				BuildLabel( 100+125*slotNum, 175, 108, 48, image_source, loadMenu);
				if(standart_fail) return;
			}
		    else{
				BuildLabel( 100+125*(slotNum-5), 250, 108, 48, image_source, loadMenu);
				if(standart_fail) return;
			}
	    }
		else
		{   
			if(slotNum < 5){
				BuildButton( 100+125*slotNum, 175, 108, 48, image_source, loadMenu, LoadSlotClicked);
				if(standart_fail) return;
			}
		    else{
				BuildButton( 100+125*(slotNum-5), 250,  108, 48, image_source, loadMenu, LoadSlotClicked);
				if(standart_fail) return;
			}
		}
	}
	BuildButton(300, 325, 219, 65, "pics/cancel.bmp", loadMenu, handleMenu);
	if(standart_fail) return;
	DrawWidgets(screen);
}

/*
*	shows the menu for saving files
*	throws malloc errors, SDL_LoadBMPError(), SDL_DisplayFormatError(), SDL_SetColorKeyError(), SDL_BlitSurfaceError(), SDL_FlipError()
*/
void showSavingMenu(Widget *widget, int x, int y)
{
	Widget* saveMenu;
	int slotNum = 0;
	char *image_source;
	char *fullSlotFiles[]={"pics/1_0.bmp","pics/2_0.bmp","pics/3_0.bmp","pics/4_0.bmp","pics/5_0.bmp","pics/6_0.bmp","pics/7_0.bmp","pics/8_0.bmp","pics/9_0.bmp","pics/10_0.bmp"};
	char *emptySlotFiles[]={"pics/1_1.bmp","pics/2_1.bmp","pics/3_1.bmp","pics/4_1.bmp","pics/5_1.bmp","pics/6_1.bmp","pics/7_1.bmp","pics/8_1.bmp","pics/9_1.bmp","pics/10_1.bmp"};
	if(mainGame->isCheckMate==2) /* can't save during mate */
		return;
	DeleteWidgets(&screen->widgetChildren);
	saveMenu = BuildLabel(0, 0, 996, 692, "pics/main.bmp", screen);
	if(standart_fail) return;
	BuildLabel( 50, 25, 700, 113, "pics/saveDest.bmp", saveMenu);
	if(standart_fail) return;
	for(slotNum = 0; slotNum < SLOT_NUM; slotNum++)
	{
		image_source=(file_slots[slotNum].isFree)?emptySlotFiles[slotNum]:fullSlotFiles[slotNum];
		if(slotNum < 5){
			BuildButton( 100+125*slotNum, 175, 108, 48, image_source, saveMenu, SaveSlotClicked);
			if(standart_fail) return;
		}
		else{
			BuildButton( 100+125*(slotNum-5), 250,  108, 48, image_source, saveMenu, SaveSlotClicked);
			if(standart_fail) return;
		}
	}
	BuildButton(300, 325, 219, 65, "pics/cancel.bmp", saveMenu, mainGame->handleGameStartFunc);
	if(standart_fail) return;
	DrawWidgets(screen);
}

/* gets the number of slots successfull loaded and frees them */
void deleteSlots(int indexOfSuccessfullyLoaded)
{
	int i = 0;
	for(i = 0 ; i < indexOfSuccessfullyLoaded; i++)
		free(file_slots[i].name);
	free(file_slots);
}

/*free all the resources and returns 1*/
int quitGame(int isGUI){
	(*mainGame->deleteGameFunc)(isGUI);
	if(isGUI){
		deleteSlots(SLOT_NUM);
		DeleteWidgets(&screen->widgetChildren);
		SDL_Quit();
		free(screen->offset);
		free(screen);
	}
	return 1;
}

/* main function of the game. */
int main( int argc, char* args[] ){
	/*need to free mainGame, file_slots, and screen*/
	int gui=((argc>1)&&(!strcmp(args[1],"gui")))?1:0;
	srand ((unsigned int)time(NULL) );
	standart_fail = 0;
	loadChess(gui); /* generic function to load the game with all the functions specific per chess*/
	if(standart_fail){ return 1;}
	if(gui==0){
		(*mainGame->ConsoleMain)(mainGame->gameInfo);
	}
	else{
		/*Initialize all SDL subsystems - where are 8 subsystems composing the SDL=GUI */
		if( SDL_Init( SDL_INIT_VIDEO ) == -1 ){SDL_InitError(); (*mainGame->deleteGameFunc)(gui); deleteSlots(SLOT_NUM); standart_fail=true; return 1;}
		/*THE GUI TREE ROOT */
		screen = BuildWindow(SCREEN_WIDTH, SCREEN_HEIGHT);
		/* never deleted (as long as we don't quit - quit button) */
		if(standart_fail){(*mainGame->deleteGameFunc)(gui) ; deleteSlots(SLOT_NUM); return 1;}
		showMenu();
		if(standart_fail){return quitGame(gui);}
		handleEvents(screen);
	}
	return quitGame(gui);
}