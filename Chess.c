#include "Chess.h"

void loadDefaultSettings(game *g){
	initialize_board(g->boardPointer);
	g->gameMode=1;
	g->nextTurnPlayerColor=white;
	g->castling[0]=true;
	g->castling[1]=true;
	g->castling[2]=true;
	g->castling[3]=true;
	g->depth=1;
	g->difficulty_best=false;
	g->userColor=white;
}

/*
*	INTIALIZATION OF GAME SETTINGS BY THE PLAYER
*	returns true if we can go to gameState, false if we need to quit.
*	throws calloc errors
*/
int gameSettings(game *g){
	char *command=(char*)calloc(MAX_COMMAND_LENGTH,sizeof(char)), *commandIterator=NULL;
	if((command==NULL)){calloc_error(); standart_fail=true; return false;}
	loadDefaultSettings(g);
	print_board(g->boardPointer);
	printf("Enter game setting: \n");
	while (fgets (command, sizeof(char)*MAX_COMMAND_LENGTH,stdin) != NULL ){ /* gets a line */
		*(command+strlen(command)-1)='\0';
		commandIterator = strtok(command, " ");
		if( ! strcmp(commandIterator,"print") )		print_board(g->boardPointer); /* printing the board*/			
		else if (! strcmp(command,"game_mode") ){
			commandIterator= strtok(NULL, " ");
			if ((! strcmp(commandIterator,"1"))||(! strcmp(commandIterator,"2"))){
				g->gameMode=atoi(commandIterator);
				printf("Running game in %s mode\n", (g->gameMode==1?"2 players":"player vs. AI"));
			}
			else{
				printf(WROND_GAME_MODE);
			}
		}
		else if (! strcmp(commandIterator,"clear") )		clear_board(g->boardPointer); /*clears board*/
		else if (! strcmp(commandIterator,"difficulty") ){
			if(g->gameMode==1){ printf(ILLEGAL_COMMAND);}
			else{
				commandIterator= strtok(NULL, " ");
				if (! strcmp(commandIterator,"best"))
					g->difficulty_best=true;
				else if (! strcmp(commandIterator,"depth")){
					commandIterator= strtok(NULL, " ");
					if((strlen(commandIterator)==1)&&(*commandIterator>='1')&&(*commandIterator<='4')){
						g->difficulty_best=false;
						g->depth=atoi(commandIterator);
					}
					else{
						printf(WRONG_MINIMAX_DEPTH);
					}
				}
				else{
					printf(WRONG_MINIMAX_DEPTH);
				}
			}
		}
		else if (! strcmp(commandIterator,"user_color")){
			if(g->gameMode==1){ printf(ILLEGAL_COMMAND);}
			else{
				commandIterator= strtok(NULL, " ");
				if (! strcmp(commandIterator,"white"))
					g->userColor=white;
				else if (! strcmp(commandIterator,"black"))
					g->userColor=black;
				else{
					printf(WROND_PLAYER_COLOR);
				}
			}
		}
		else if (! strcmp(commandIterator,"start") )		{ free(command); return true;} /*go back to main*/
		else if (! strcmp(commandIterator,"quit"))			break;
		else if (! strcmp(commandIterator,"load")){
			commandIterator= strtok(NULL, "");	/* with spaces because filenames might have those*/
			if(loadChessGame(g,commandIterator))
				print_board(g->boardPointer);
		}
		else{
			printf(ILLEGAL_COMMAND);
		}
	}
	if(ferror(stdin)){standart_fail=true; fgets_error();}
	free(command); 
	return false;
}

/*
*	gets a game after loading from file without castling info, and updates it castling info
*	if king & rook are in their initial place assume they haven't move yet
*/
void updateMissingCastlingForLoadedGame(game *g){
	g->castling[0]=(((*g->boardPointer)[7][4]==BLACK_K)&&((*g->boardPointer)[7][0]==BLACK_R));
	g->castling[1]=(((*g->boardPointer)[7][4]==BLACK_K)&&((*g->boardPointer)[7][7]==BLACK_R));
	g->castling[2]=(((*g->boardPointer)[0][4]==WHITE_K)&&((*g->boardPointer)[0][0]==WHITE_R));
	g->castling[3]=(((*g->boardPointer)[0][4]==WHITE_K)&&((*g->boardPointer)[0][7]==WHITE_R));
}

/*
*	Load the game from "address".
*	if file does not exist, returns false and print message
*	otherwise returns true
*/
int loadChessGame(game *g, const char *address){
	FILE *file;
	char line[MAX_FILE_LINE_LENGTH], *lineIterator=NULL;
	int row,col,i, castlingInfoFound=false;
    file = fopen (address,"r");	
	if(file == NULL){	printf(WROND_FILE_NAME);	return false;}
	while(fgets (line, sizeof(char)*MAX_FILE_LINE_LENGTH,file) != NULL ){ /* reads a line from the file and translate it*/
		lineIterator = strtok(line, " \t<>?");
		if(! strcmp( lineIterator, "xml" ) ) continue;
		else if(! strcmp( lineIterator, "type" )){
			lineIterator=strtok(NULL, "\t<>?");
			g->gameMode=atoi(lineIterator);
		}
		else if(! strcmp( lineIterator, "difficulty" )){
			lineIterator=strtok(NULL, "\t<>?");
			if(! strcmp( lineIterator, "Best" ))
				g->difficulty_best=true;
			else{
				g->difficulty_best=false;
				g->depth=atoi(lineIterator);
			}
		}
		else if(! strcmp( lineIterator, "user_color" )){
			lineIterator=strtok(NULL, "\t<>?");
			g->userColor=(!strcmp(lineIterator,"White"))?white:black;
			g->nextTurnPlayerColor=g->userColor;
		}
		else if(! strcmp( lineIterator, "next_turn" )){
			lineIterator=strtok(NULL, "\t<>?");
			g->nextTurnPlayerColor=(!strcmp(lineIterator,"White"))?white:black;
		}
		else if(! strcmp( lineIterator, "board" )){
			for(row=BOARD_SIZE;row>=1;row--){
				if(fgets (line, sizeof(char)*MAX_FILE_LINE_LENGTH,file) != NULL){
					lineIterator=strtok(line, "\t<>?");
					lineIterator=strtok(NULL, "\t<>?");
					for(col=0;col<BOARD_SIZE;col++)
						(*(g->boardPointer))[row-1][col]=*(lineIterator+col);
				}
			}
		} 
		else if(! strcmp( lineIterator, "general" )){
			castlingInfoFound=true;
			for(i=0;i<4;i++){
				if(fgets (line, sizeof(char)*MAX_FILE_LINE_LENGTH,file) != NULL){
						lineIterator=strtok(line, "\t<>?");
						lineIterator=strtok(NULL, "\t<>?");
						g->castling[i]=atoi(lineIterator);
				}
			}
		}
	}
	if(ferror(file)){	printf(PROBLEM_DURING_READING);	fclose(file); return false;}
	fclose(file);
	if(!castlingInfoFound)
		updateMissingCastlingForLoadedGame(g);
	return true;
}

/*
*	gets a string from the format <x,i> to <y,j> [promotion] and turn it into move
*	checks that it's coordinates are correct, it contains player and legal, if not print the correct message
*	throws calloc errors
*/
int stringToMove(game *g, move *m, char *str){
	char *commandIterator=NULL, type;
	m->promoteTo=0;
	commandIterator= strtok(NULL, " ,<>");
	m->col=*commandIterator-'a';
	commandIterator= strtok(NULL, " ,<>");
	m->row=*commandIterator-'1';
	commandIterator= strtok(NULL, " ,<>to");
	m->newCol=*commandIterator-'a';
	commandIterator= strtok(NULL, " ,<>");
	m->newRow=*commandIterator-'1';
	if((!in_range(m->row,m->col))||(!in_range(m->newRow,m->newCol))){
		printf(WRONG_POSITION); 
		return false;
	}
	else if(occupier_color(m->row,m->col,g->boardPointer)!=g->nextTurnPlayerColor){
		printf(NO_PIECE);
		return false;
	}
	else{
		type=(*(g->boardPointer))[m->row][m->col];
		if((type==getPawnChar(g->nextTurnPlayerColor))&&(isPromoted(m->newRow,g->nextTurnPlayerColor))){
			commandIterator= strtok(NULL, " ,<>");
			if(commandIterator==NULL)	/* if not specified- default promotion is queen */
				m->promoteTo=getQueenChar(g->nextTurnPlayerColor);
			else if((m->promoteTo=getCharForPromotion(commandIterator, g->nextTurnPlayerColor))==0){
				printf(ILLEGAL_MOVE);
				return false;
			}
		}
		if(!isLegalMove(g,m)){
			if(!standart_fail) printf(ILLEGAL_MOVE);
			return false;
		}
	}
	return true;
}

/*
*	gets a string from the format castle <x,i> and turn it into  castling move
*	checks that it's coordinates are correct, it contains player and legal, if not print the correct message
*	throws calloc errors
*/
int stringToCastlingMove(game *g, move *m, char *str){
	char *commandIterator=NULL;	int startingRow=g->nextTurnPlayerColor==white?0:7;
	m->promoteTo=0;
	commandIterator= strtok(NULL, " ,<>");
	m->col=m->newCol=*commandIterator-'a';
	commandIterator= strtok(NULL, " ,<>");
	m->row=m->newRow=*commandIterator-'1';
	if((!in_range(m->row,m->col))||(!in_range(m->newRow,m->newCol))){
		printf(WRONG_POSITION); 
		return false;
	}
	else if(occupier_color(m->row,m->col,g->boardPointer)!=g->nextTurnPlayerColor){
		printf(NO_PIECE);
		return false;
	}
	else if((*g->boardPointer)[m->row][m->col]!=getRookChar(g->nextTurnPlayerColor)){
		printf(NO_ROOK);
		return false;
	}
	else if((!isCastlingMove(m))||(m->row!=startingRow)||(!isCastlingPossible(g,m->col==0))){
		if(!standart_fail) printf(ILLEGAL_CASTELING);
		return false;
	}
	else return true;
}

/* 
*	gets a string which have command from the user and handle it
*	returns true after we made a move on the board, and switched players
*	returns 0 in case of a regular move
*	returns 1 in case of move that caused check
*	returns 2 in case of move that caused mate
*	returns -1 in case we need to restart and go back to settings
*	returns -2 in case we need to quit
*	POSTCONDITION: command doesn't get freed
*	throws calloc and fgets errors
*/
int makeUserTurn(char *command, char *commandIterator, game *g){
	move m={0}; int row, col;
	/* print message and gets a line */
	while((!standart_fail)&&(printf("%s player - enter your move:\n", playerCapitalString(g->nextTurnPlayerColor)))&&(fgets (command, sizeof(char)*MAX_COMMAND_LENGTH,stdin) != NULL )){ 
		*(command+strlen(command)-1)='\0';
		commandIterator = strtok(command, " ,<>");
		/* move or castling move*/
		if(((!strcmp( commandIterator, "move" ))&&(stringToMove(g, &m, command))&&(!standart_fail))||
			((! strcmp( commandIterator, "castle"))&&(stringToCastlingMove(g, &m, command))&&(!standart_fail))){
				return applyMove(g,&m);
		}
		else if(! strcmp( command, "save")) {
			commandIterator= strtok(NULL, "");
			saveChessGame(g,commandIterator);
		}
		else if(! strcmp( command, "restart")) {
			return -1;
		}
		else if(! strcmp(command, "get_moves")){
			commandIterator= strtok(NULL, " ,<>");
			col=*commandIterator-'a';
			commandIterator= strtok(NULL, " ,<>");
			row=*commandIterator-'1';
			printMoves(g,row,col,2,NULL,NULL);
		}
		else if(! strcmp(command, "quit")) break;
		else{
			printf(ILLEGAL_COMMAND);
		}
	}
	if(ferror(stdin)){fgets_error(); standart_fail=true;}
	return -2;	/* finish reading the file, or need to quit. */
}

/* 
*	gets a game where the current player is the AI, choose a move using alpha-beta pruning, print it and apply on board and switch players
*	returns 0 in case of a regular move
*	returns 1 in case of move that caused check
*	returns 2 in case of move that caused mate
*	throws calloc errors
*/
int makeAITurn(game *g){
	move m=get_AI_Move(g);
	if(standart_fail) return false;
	printf("Computer: ");
	printMove(&m);
	return applyMove(g,&m);
}

/*  
*	play mode
*	returns true if we need to go to gameSettings - because of restart
*	returns false if we need to quit- because the user has quitted the game/someone has won/an error has occured.
*	throws AI and fgets errors
*/
int play(game *g){
	int result=1; char *command=(char*)calloc(MAX_COMMAND_LENGTH,sizeof(char)), *commandIterator=NULL;
	if(command==NULL){calloc_error(); standart_fail=true; return false;}
	while(result>=0){
		result=(isAITurn(g))?(makeAITurn(g)):(makeUserTurn(command, commandIterator, g));
		if((standart_fail)||(result<0)) break;
		print_board(g->boardPointer);
		if(result==1)	
			printf("Check!\n");
		else if(result==2){
			printf("Mate! %s player wins the game\n", playerCapitalString(opponentColor(g->nextTurnPlayerColor)));
			free(command); return false; 
		}
	}
	free(command);
	return ((standart_fail)||(result==-2))?false:true; /*if we need to quit- false, if we need to restart true */
}

/*
*	Saves the game to *address and returns true
*	if any error occured (like file cannot be created or overridden) returns false and print message
*/
int saveChessGame(game *g, const char *address){
	const char *castlingLetters[4];
	char row; int col, i;
	FILE *gameFile=fopen(address, "w");
	castlingLetters[0]="BL"; castlingLetters[1]="BR"; castlingLetters[2]="WL"; castlingLetters[3]="WR";
	if(gameFile == NULL){	printf(WROND_FILE_NAME);	return false;	}
    fprintf(gameFile, "%s", "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \n");  
	fprintf(gameFile, "%s", "<game>\n"); 
	fprintf(gameFile, "%s%d%s", "\t<type>",g->gameMode,"</type>\n"); 
	if(g->gameMode == 2){
		fprintf(gameFile, "%s", "\t<difficulty>");
		if(g->difficulty_best)		fprintf(gameFile, "%s", "Best");
		else					fputc((g->depth+'0'), gameFile);
		fprintf(gameFile, "%s", "</difficulty>\n");
		fprintf(gameFile, "%s%s%s", "\t<user_color>",playerCapitalString(g->userColor),"</user_color>\n");
	}
	fprintf(gameFile, "%s%s%s", "\t<next_turn>",playerCapitalString(g->nextTurnPlayerColor),"</next_turn>\n");
	fprintf(gameFile, "%s", "\t<board>\n"); 
	for(row='0'+BOARD_SIZE;row>='1';row--){
		fprintf (gameFile, "\t\t<row_%c>", row);
		for(col=0;col<BOARD_SIZE;col++)	fprintf(gameFile, "%c", (*(g->boardPointer))[row-'1'][col]);
		fprintf (gameFile, "</row_%c>\n", row);
	}
	fprintf (gameFile, "%s", "\t</board>\n"); 
	fprintf (gameFile, "%s", "\t<general>\n");
	for(i=0;i<4;i++)
		fprintf (gameFile, "\t\t<castle_%s>%d</castle_%s>\n", castlingLetters[i], g->castling[i], castlingLetters[i]);
	fprintf (gameFile, "%s", "\t</general>\n"); 
	fprintf (gameFile, "%s", "</game>");
    fclose(gameFile);
	return true;
}

/*gets a game and check if the next player is AI*/
int isAITurn(game *g){
	if(g->gameMode==1) /*PVP*/
		return false;
	if(g->nextTurnPlayerColor==g->userColor) /*user turn*/
		return false;
	return true;
}

int chessConsoleMain(game *g){
	while(!standart_fail){
		if(!gameSettings(g))	break;
		if(standart_fail) break;
		if(!play(g)) break;	
	}
	return 0;
}

