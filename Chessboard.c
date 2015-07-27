#include "Chessboard.h"

const int allOffset[8][2]={{1,1},{1,0},{1,-1},{0,1},{-1,1},{0,-1},{-1,-1},{-1,0}};
const int knightOffset[8][2]={{2,1},{2,-1},{1,2},{-1,2},{-2,-1},{-2,1},{1,-2},{-1,-2}};
const char allsets[]={EMPTY, WHITE_P, BLACK_P, WHITE_K, BLACK_K, WHITE_B, BLACK_B, WHITE_R, BLACK_R, WHITE_KN, BLACK_KN, WHITE_Q, BLACK_Q};
const int score_per_set[]={0,1,-1,1000,-1000,3,-3,5,-5,3,-3,9,-9,};
const int max_moves_per_set[]={0,6,6,8,8,13,13,14,14,8,8,27,27};	/* pawn is 6 because of promotions */
int standart_fail=0;

/*get a move and returns true if it is castling move (not check if it's black or white castling move though)*/
int isCastlingMove(move *m) {
	if((m->col!=m->newCol)||(m->newRow!=m->row)) return false;
	if((m->row!=0)&&(m->row!=7)) return false;
	if((m->col!=0)&&(m->col!=7)) return false;
	return true;
}

/*gets a square where a pawn is or could move to, and returns the scoring of the two pieces it secure (left and right) */
int pawnSecuringPieceScore(game *g, int row, int col){
	int currPlayer=g->nextTurnPlayerColor , score=0 , nextRow=(currPlayer==white)?(row+1):(row-1), nextColLeft=col-1, nextColRight=col+1;
	if( (in_range(nextRow,nextColLeft)) && (occupier_color(nextRow,nextColLeft,g->boardPointer) == currPlayer) )
	{ /* if it's left move forward is on square and containing piece from the same player*/
		if((*g->boardPointer)[nextRow][nextColLeft]!=getKingChar(currPlayer))
		{	/* if it's not a king in this sqaure - can't secure kings*/
			if( (in_range(row,nextColLeft-1)) && ((*g->boardPointer)[row][nextColLeft-1]!=getPawnChar(currPlayer)) )
			{	/*not already secured by pawn from the other side*/
				score+=getPoints((*g->boardPointer)[nextRow][nextColLeft],allsets,score_per_set,NUM_OF_SETS);
			}
		}
	}
	/*same for the right move*/
	if( (in_range(nextRow,nextColRight)) && (occupier_color(nextRow,nextColRight,g->boardPointer) == currPlayer) )
		if((*g->boardPointer)[nextRow][nextColRight]!=getKingChar(currPlayer))
			if( (in_range(row,nextColRight+1)) && ((*g->boardPointer)[row][nextColRight+1]!=getPawnChar(currPlayer)) )
				score+=getPoints((*g->boardPointer)[nextRow][nextColRight],allsets,score_per_set,NUM_OF_SETS);
	return score;
}

/*gets a non-eating non-castling non-promotion move and give it pawn-securing score */
int getPawnSecuringMoveScore(game *g, move *m){
	int currentSecuringScore, nextSecuringScore;
	assert(!isCastlingMove(m));
	assert(occupier_color(m->newRow,m->newCol,g->boardPointer)==EOF);
	if((*g->boardPointer)[m->row][m->col]!=getPawnChar(g->nextTurnPlayerColor))
		return 0;
	assert(m->newCol==m->col);
	assert(!isPromoted(m->newRow,g->nextTurnPlayerColor));
	currentSecuringScore=pawnSecuringPieceScore(g,m->row,m->col);
	nextSecuringScore=pawnSecuringPieceScore(g,m->newRow,m->newCol);
	if(g->nextTurnPlayerColor==black){
		currentSecuringScore*=(-1);
		nextSecuringScore*=(-1);
	}
	return (nextSecuringScore-currentSecuringScore);
}

/* gets a move and check if in the move we eat one of the rooks in the initial position, if so updates g's castling accordingly*/
void updateCastlingForEating(game *g, move *m){
	int start=(g->nextTurnPlayerColor==white)?0:2;
	char oppRookChar=getRookChar(opponentColor(g->nextTurnPlayerColor));
	char oppKingChar=getKingChar(opponentColor(g->nextTurnPlayerColor));
	char eatingChar=(*(g->boardPointer))[m->newRow][m->newCol];
	if(eatingChar==oppRookChar){
		if(m->newCol==0) g->castling[start]=false;
		if(m->newCol==7) g->castling[start+1]=false;
	}
	else{
		if(eatingChar==oppKingChar){
			g->castling[start]=false;
			g->castling[start+1]=false;
		}
	}
}

/* gets a move that was made and updates g castling accordingly (the move can be castling itself)*/
void updateCastling(game *g, move *m){
	int start=(g->nextTurnPlayerColor==white)?2:0;
	updateCastlingForEating(g,m);
	if(((g->nextTurnPlayerColor==white)&&(m->row!=0))||((g->nextTurnPlayerColor==black)&&(m->row!=7))) return;
	if((m->col!=0)&&(m->col!=4)&&(m->col!=7)) return;
	if((isCastlingMove(m))||(m->col==4)){	/* the move is king move or castling */
		g->castling[start]=false;
		g->castling[start+1]=false;
	}
	if(m->col==0) g->castling[start]=false;
	if(m->col==7) g->castling[start+1]=false;
}

/* 
*	gets a game and a move and check if it's possible to be made 
*	throws calloc errors
*/
int isMoveLegal(game *g, move *searchFor){	
	int startingRow = g->nextTurnPlayerColor == white ? 0:7;
	move moveIterator={0}, leftCastling={0}, rightCastling={0};
	move *correctCastle = (searchFor->col == 0) ? &leftCastling : &rightCastling;
	moveIterator.col=searchFor->col, moveIterator.row=searchFor->row;
	leftCastling.row = leftCastling.newRow = rightCastling.row = rightCastling.newRow = startingRow;
	leftCastling.col = leftCastling.newCol = 0;
	rightCastling.col = rightCastling.newCol = 7;
	if( (!in_range( searchFor->row , searchFor->col ))||(!in_range( searchFor->newRow , searchFor->newCol))){ return false;}
	else if(occupier_color(searchFor->row,searchFor->col,g->boardPointer)!=g->nextTurnPlayerColor){ return false;}
	else if(getFirstPieceMove(g->boardPointer,&moveIterator,g->nextTurnPlayerColor)){
		do{		/* count all the regulars moves from [row,col] which doesn't include castling*/
			if( !isKingInDangerAfterMove( g->boardPointer, &moveIterator) ){
				if(standart_fail) return 0;
				if(isMoveEqualWithoutPromotion(&moveIterator,searchFor))
					return true;
			}
		} while(getNextPieceMove(g->boardPointer,&moveIterator)); /*generation of movement for the piece at <row, col>*/

		if( (*g->boardPointer)[searchFor->row][searchFor->col] == getRookChar( g->nextTurnPlayerColor ) )
		{	/* [row,col] contains rook- check the appropriate castling */
			if(isCastlingPossible(g,searchFor->col==0))
			{
				if(standart_fail) return 0;
				/* m might be a castling move - check if equal */
				if(isMovesEqual(correctCastle,searchFor))
						return true;
			}
		}
	}

	return false;
}

/* 
*	gets a move pointer where (*boardPointer)[m->row][m->col] contains a queen, a rook, or a bishop, updates m to the first move from it, and returns true
*	if it can't be moved return false;
*	doesn't check if king is threatened after the move
*/
int findQBRFirstMove(boardp boardPointer, move *m){
	char set=(*boardPointer)[m->row][m->col];
	int playerColor=occupier_color(m->row,m->col,boardPointer),  i=(set==getRookChar(playerColor))?1:0, iIncrease=(set==getQueenChar(playerColor))?1:2;
	for(;i<8;i+=iIncrease){
		m->newRow=m->row+allOffset[i][0];
		m->newCol=m->col+allOffset[i][1];
		if(in_range(m->newRow,m->newCol))
			if(occupier_color(m->newRow,m->newCol,boardPointer)!=playerColor)
				return true;
	}
	return false;
}

/* 
*	gets a move pointer where (*boardPointer)[m->row][m->col] contains a queen, a rook, or a bishop, updates m to the next move from it, and returns true
*	if it was the last move return false;
*	doesn't check if king is threatened after the move, and doesn't return castling moves
*/
int findQBRNextMove(boardp boardPointer, move *m){
	char set=(*boardPointer)[m->row][m->col];
	int playerColor=occupier_color(m->row,m->col,boardPointer),  i=(set==getRookChar(playerColor))?1:0, iIncrease=(set==getQueenChar(playerColor))?1:2;
	/* the last move offset ended (because of eating)*/
	int colOffset, rowOffset, lastMoveOffsetEnded=(occupier_color(m->newRow,m->newCol,boardPointer)==opponentColor(playerColor)); 
	if(isCastlingMove(m)) return false; /*castling moves are always the last*/
	get_direction_offset(m,&rowOffset,&colOffset);
	for(;i<8;i+=iIncrease){if((rowOffset==allOffset[i][0])&&(colOffset==allOffset[i][1])) break;};		/* skip the previous offsets */
	if(!lastMoveOffsetEnded){ /* now we check if a move in the same offset as before can be made */
		m->newRow+=allOffset[i][0];
		m->newCol+=allOffset[i][1];
		if((in_range(m->newRow,m->newCol))&&((occupier_color(m->newRow,m->newCol,boardPointer)!=playerColor))) 
			return true;	/*we can keep go in the same direction*/
	}
	for(i+=iIncrease;i<8;i+=iIncrease){			/* current offset direction is stuck - this for loop check the other directions for legal 1-step move*/
		m->newRow=m->row+allOffset[i][0];
		m->newCol=m->col+allOffset[i][1];
		if((in_range(m->newRow,m->newCol))&&(occupier_color(m->newRow,m->newCol,boardPointer)!=playerColor))
			return true;
	}
	return false;
}

/*
*	gets a promotion move and change its promotion character to the next by the order Q,B,R,KN
*	if m->promoteTo is a knight char, change it to 0 and return false;
*/
int updatePromotion(move *m, int playerColor){
	if(m->promoteTo==getKnightChar(playerColor)){ m->promoteTo=0; return false;}
	if(m->promoteTo==getQueenChar(playerColor)) m->promoteTo=getBishopChar(playerColor);
	else if(m->promoteTo==getBishopChar(playerColor)) m->promoteTo=getRookChar(playerColor);
	else if(m->promoteTo==getRookChar(playerColor)) m->promoteTo=getKnightChar(playerColor);
	else m->promoteTo=getQueenChar(playerColor);
	return true;
}

/*
* gets pointers to move and board which have a pawn in the [m->row,m->col] coordinates. updates m to it's first move and returns true, if no move possible return 0;
* if eating moves possible- return eating move. promotion order: Q,B,R,KN- so if promotion is possible returns queen promotion.
*/
int findPawnFirstMove(boardp boardPointer, move *m){
	int playerColor=occupier_color(m->row,m->col,boardPointer), i, cols[3];
	cols[0]=m->col-1, cols[1]=m->col+1, cols[2]=m->col;
	m->newRow=(playerColor==white?m->row+1:m->row-1);
	if((in_range(m->newRow,cols[0]))&&(occupier_color(m->newRow,cols[0],boardPointer)==opponentColor(playerColor))) i=0;
	else if((in_range(m->newRow,cols[1]))&&(occupier_color(m->newRow,cols[1],boardPointer)==opponentColor(playerColor))) i=1;
	else if((in_range(m->newRow,cols[2]))&&(occupier_color(m->newRow,cols[2],boardPointer)==EOF)) i=2;
	else return false;
	m->newCol=cols[i];
	if(isPromoted(m->newRow,playerColor)) m->promoteTo=getQueenChar(playerColor);
	return true;
}

/*
* gets pointers to move by a pawn and board. updates m to this pawn next move and returns true, if no more moves are possible return 0;
* return eating move first promotion order: Q,B,R,KN- so if promotion is possible returns queen promotion.
*/
int findPawnNextMove(boardp boardPointer, move *m){
	int playerColor=occupier_color(m->row,m->col,boardPointer), i, cols[3], canBeMade[3]={0};
	int promotionFlag;
	cols[0]=m->col-1, cols[1]=m->col+1, cols[2]=m->col;
	m->newRow=(playerColor==white?m->row+1:m->row-1);
	promotionFlag=isPromoted(m->newRow,playerColor);
	if((m->newCol==m->col-1)&&(in_range(m->newRow,cols[0]))&&(occupier_color(m->newRow,cols[0],boardPointer)==opponentColor(playerColor))) canBeMade[0]=true;
	if((m->newCol!=m->col)&&(in_range(m->newRow,cols[1]))&&(occupier_color(m->newRow,cols[1],boardPointer)==opponentColor(playerColor))) canBeMade[1]=true;
	if((in_range(m->newRow,cols[2]))&&(occupier_color(m->newRow,cols[2],boardPointer)==EOF)) canBeMade[2]=true;
	for(i=0;i<3;i++){	/* if it's not promotion move change the first "true" to false. */
		if((canBeMade[i])&&(!promotionFlag)){
			canBeMade[i]=false;
			break;
		}
	}
	for(i=0;i<3;i++){
		if(canBeMade[i]){
			m->newCol=cols[i];
			if(!promotionFlag) /* not a promotion move */
				return true;
			else if(updatePromotion(m,playerColor))
				return true;
		}
	}
	return false;
}

/*
*	gets a game and returns a copy of it, if unsuccessfull returns NULL
*	PRECONDITION: both boards are initiallized
*/
void copyChessGame(game *from, game *to){
	int i;
	for(i=0;i<4;i++) to->castling[i]=from->castling[i];
	to->depth=from->depth;
	to->difficulty_best=from->difficulty_best;
	to->gameMode=from->gameMode;
	to->nextTurnPlayerColor=from->nextTurnPlayerColor;
	to->userColor=from->userColor;
	copy_board(from->boardPointer,to->boardPointer);
}

void copyMove(move *from, move *to){
	to->col=from->col;
	to->newCol=from->newCol;
	to->newRow=from->newRow;
	to->promoteTo=from->promoteTo;
	to->row=from->row;
}

/*
*	gets all the moves that can be done by playerColor on the board recursivly, first call need to be with *m={0,0,0,0,0}
*	doesn't check if king is threatened after move or castling move
*/
int getNextMove(boardp boardPointer, move *m, int playerColor){
	/*not first time and there is a move that can be done by the same piece*/
	if(((m->row!=0)||(m->col!=0)||(m->newRow!=0)||(m->newCol!=0))&&(getNextPieceMove(boardPointer,m)))	return true;
	else{	/* first time using getNext move / last move for the piece*/
		/* if it's the first move and [0,0] contains a set belongs to player, it's ok, otherwise move to the next moveable piece*/
		if(!(((m->row==0)&&(m->col==0)&&(m->newRow==0)&&(m->newCol==0))&&(occupier_color(0,0,boardPointer)==playerColor))) 
			if(!getNextPiece(boardPointer,playerColor,&(m->row),&(m->col)))
				return false; 
		m->newCol=m->newRow=-1;
	}
	while(!getFirstPieceMove(boardPointer,m,playerColor))	/*try to find this piece first move*/
		if(!getNextPiece(boardPointer,playerColor,&(m->row),&(m->col))) /* the piece can't be moved, move to the next one*/
			return false;	/* we reached the end of the board- m  was the last move possible*/
	return true;
}

int isMoveEqualWithoutPromotion(move *m1, move *m2){ 
	return ((m1->row==m2->row)&&(m1->col==m2->col)&&(m1->newRow==m2->newRow)&&(m1->newCol==m2->newCol));
}

/* checks if move m1==move m2*/
int isMovesEqual(move *m1, move *m2){
	return (isMoveEqualWithoutPromotion(m1,m2)&&(m1->promoteTo==m2->promoteTo));
}

/* 
*	returns true iff the move is legal && doesn't endanger the player king (include castling)
*	Preconditions: (*boardPointer)[m->row][m->col]!=EMPTY && in_range(row,col) && in_range(newRow,newCol)
*	throws calloc errors
*/
int isLegalMove(game *g, move *m){
	move moveTemp={0,0,0,0,0};
	if(!getFirstLegalMove(g ,&moveTemp)) return false;
	while((!standart_fail)&&(!isMovesEqual(m,&moveTemp)))
		if(!getNextLegalMove(g,&moveTemp)) return false;	/* we reached the end of our moves and didn't found it */
	return true;
}


/*
*	get a pointer to a move, when (*boardPointer)[m->row][m->col] have a piece belongs to playerColor
*	updates the move to the first move of that piece, if exist and returns true, otherwise returns false;
*	doesn't check if the move endanger the king.
*/
int getFirstPieceMove(boardp boardPointer, move *m, int playerColor){
	char type=(*boardPointer)[m->row][m->col]; int i, kingType;
	if((kingType=(type==getKingChar(playerColor)))||(type==getKnightChar(playerColor))){
		for(i=0;i<8;i++){
			m->newRow=m->row+(kingType?allOffset[i][0]:knightOffset[i][0]);
			m->newCol=m->col+(kingType?allOffset[i][1]:knightOffset[i][1]);
			if((in_range(m->newRow,m->newCol))&&(occupier_color(m->newRow,m->newCol,boardPointer)!= playerColor) )
				return true;
		}
		return false;
	}
	else if(type==getPawnChar(playerColor))	return findPawnFirstMove(boardPointer,m);
	else return findQBRFirstMove(boardPointer,m); /*QBR Move*/
}

/*
*	gets the previous possible move, updates it to the next possible move for this piece and returns true
*	if it was the last move for this piece returns false
*	doesn't check if the move endanger the king, or castling moves.
*/
int getNextPieceMove(boardp boardPointer, move *m){
	char type=(*boardPointer)[m->row][m->col];
	int playerColor=occupier_color(m->row,m->col,boardPointer), kingType, i;
	if((kingType=(type==getKingChar(playerColor)))||(type==getKnightChar(playerColor))){
		/* skip previous offsets */
		for(i=0;((m->newRow!=(m->row+(kingType?allOffset[i][0]:knightOffset[i][0])))||(m->newCol!=(m->col+(kingType?allOffset[i][1]:knightOffset[i][1]))));i++); 
		if(i==7) return false;	/*previous move was the last*/
		else{
			for(i++;i<8;i++){
				m->newRow=m->row+(kingType?allOffset[i][0]:knightOffset[i][0]);
				m->newCol=m->col+(kingType?allOffset[i][1]:knightOffset[i][1]);
				if((in_range(m->newRow,m->newCol))&&(occupier_color(m->newRow,m->newCol,boardPointer)!= playerColor) )
					return true;
			}
		}
		return false;
	}
	else if(type==getPawnChar(playerColor)) return findPawnNextMove(boardPointer,m);
	else return findQBRNextMove(boardPointer,m);	/*QBR Move*/
}

/* 
*	gets a game and castling and check if it can be made
*	throws calloc errors
*/
int isCastlingPossible(game *g, int isLeftCastling){
	int rookCol=isLeftCastling?0:7, startingRow=g->nextTurnPlayerColor==white?0:7, kingCol=4; /* if not moved- king is in 5th col */
	int kingColOffset=isLeftCastling?-1:1, i=((g->nextTurnPlayerColor==white)?2:0)+(isLeftCastling?0:1);
	move oneStepMove={0}, twoStepMove={0};
	char firstSquareCotent=(*(g->boardPointer))[startingRow][kingCol+kingColOffset];
	char secondSquareCotent=(*(g->boardPointer))[startingRow][kingCol+2*kingColOffset];
	char thirdSquareCotent=isLeftCastling?(*(g->boardPointer))[startingRow][kingCol+2*kingColOffset]:EMPTY; /* apply only to left castling*/
	oneStepMove.row=oneStepMove.newRow=twoStepMove.row=twoStepMove.newRow=startingRow;
	oneStepMove.col=twoStepMove.col=kingCol;
	oneStepMove.newCol=kingCol+kingColOffset,0;
	twoStepMove.newCol=kingCol+2*kingColOffset;
	if(!g->castling[i]) return false; /*can't be made due to previous movement of king/rook */
	else{
		assert((*g->boardPointer)[startingRow][rookCol]==getRookChar(g->nextTurnPlayerColor));
		assert((*g->boardPointer)[startingRow][kingCol]==getKingChar(g->nextTurnPlayerColor));
	}
	if(isKingThreatened(g->boardPointer,g->nextTurnPlayerColor)) return false;	/* king is under check- can't castle */
	if((firstSquareCotent!=EMPTY)||(secondSquareCotent!=EMPTY)||(thirdSquareCotent!=EMPTY)) return false; /* path between king and rook isn't empty*/
	if(isKingInDangerAfterMove(g->boardPointer,&oneStepMove)||(standart_fail)) return false; /*1-step move endanger king*/
	return(!isKingInDangerAfterMove(g->boardPointer,&twoStepMove)); /*2-step move endanger king otherwise- legal*/
}



void print_line(){
	int i;
	printf(" |");
	for (i = 1; i < BOARD_SIZE*4; i++)
		printf("-");
	printf("|\n");
}

/*	prints the board 	*/
void print_board(boardp boardPointer){	
	int i,j;
	printf("\n");
	print_line();
	for (i = BOARD_SIZE-1; i >= 0 ; i--){
		printf("%d", i+1);
		for (j = 0; j < BOARD_SIZE; j++)
			printf("| %c ", (((*boardPointer)[i][j]!=EMPTY)?(*boardPointer)[i][j]:' '));
		printf("|\n");
		print_line();
	}
	printf("  ");
	for (i = 0; i < BOARD_SIZE; i++)
		printf(" %c  ", (char)('a' + i));
	printf("\n");
}

/* gets a char and return the correct scoring of this square*/
int getPoints(char c, const char sets[], const int scores_per_sets[], const int Num_of_diff_sets){
	int i;
	for(i=0;i<Num_of_diff_sets;i++)
		if(c==sets[i])
			 return scores_per_sets[i];
	return 0;
}

/* 
*	gets a board pointer, an array "sets" contains all the letters representing the sets, 
*	an array "scores_per_sets" contains score per set by the same order, and the number of different sets
*	return the result of the scoring function of the board - white player will want it to be high while black would want it low. */
int getBoardScore(boardp boardPointer, const char sets[], const int scores_per_sets[], const int Num_of_diff_sets){
	int row, col, score=0;
	for (row = 0; row < BOARD_SIZE; row++)
		for (col = 0; col < BOARD_SIZE; col++)
			score+=getPoints( (*boardPointer)[row][col] ,sets,scores_per_sets,Num_of_diff_sets);
	return score;
}

/*
*	count how many moves can be done from a specific game state by the current player
*	throws calloc errors
*/
int countAllMoves(game *g){ return printAllMoves(g,false,NULL,NULL);}

/* iniitialize board	*/
void initialize_board(boardp boardPointer){
	int i,j;
	char row1[BOARD_SIZE]={WHITE_R,WHITE_KN,WHITE_B,WHITE_Q,WHITE_K,WHITE_B,WHITE_KN,WHITE_R};
	char row8[BOARD_SIZE]={BLACK_R,BLACK_KN,BLACK_B,BLACK_Q,BLACK_K,BLACK_B,BLACK_KN,BLACK_R};
	for (i = 0; i < BOARD_SIZE; i++){
		for (j = 0; j < BOARD_SIZE; j++){
			switch(i){
			case 0:
				(*boardPointer)[i][j] = row1[j];
				break;
			case 1:
				(*boardPointer)[i][j] = WHITE_P;
				break;
			case 6:
				(*boardPointer)[i][j] = BLACK_P;
				break;
			case 7:
				(*boardPointer)[i][j] = row8[j];
				break;
			default:
				(*boardPointer)[i][j]=EMPTY;
			}
		}
	}
}

/* clears the board by setting all the board to the empty char */
void clear_board(boardp boardPointer){
	int i,j;
		for (i = 0; i < BOARD_SIZE; i++)
			for (j = 0; j < BOARD_SIZE; j++)
				(*boardPointer)[i][j]=EMPTY;
}

/* 
*	copies board from "from" to "to" 
*	PRECONDITION: to, from must be initillized earlier.
*/
void copy_board(boardp from, boardp to){
	int i,j;
		for (i = 0; i < BOARD_SIZE; i++)
			for (j = 0; j < BOARD_SIZE; j++)
				(*to)[i][j]=(*from)[i][j];	
}

/* This function gets a position <row,col> and tells us if the square is black */
int dark_square(int row, int col){
	return ((row % 2) == (col % 2));
}
 
/* This function gets a position <row,col> and tells us the color of the piece at place, returns EOF if empty*/
int occupier_color(int row, int col, boardp boardPointer){
	char c=(*boardPointer)[row][col];
	if(islower(c))
		return white;
	if(isupper(c))
		return black;
	return EOF; /* if we're in an empty position on the board	*/
}

/*
*	ARE COL AND ROW IN THE RANGE OF OUR BOARD
*	Function checks validity of range
*/
int in_range(int row, int col){
    return ((row>=0) && (row<BOARD_SIZE) && (col>=0) && (col<BOARD_SIZE));
}

/* 
*	gets a move and calculate the direction of it
*	dirX and dirY will contain the offset for the movement
*/
void get_direction_offset(move *m, int *rowOffset, int *colOffset){
	*rowOffset=m->newRow-m->row;
	*colOffset=m->newCol-m->col;
	if(*rowOffset) *rowOffset/=abs(*rowOffset);
	if(*colOffset) *colOffset/=abs(*colOffset);
}

/* 
*	gets a legal move (include legal castling) and apply it on the board
*	Precondition: move must be legal move 
*/
void applyMoveOnBoard(boardp boardPointer, move *m){
	int kingCol=4;
	if(isCastlingMove(m)){
		if(m->col==0){	/* left castling */
			(*boardPointer)[m->row][kingCol-2]=(*boardPointer)[m->row][kingCol];	/* king move 2  */
			(*boardPointer)[m->row][m->col+3]=(*boardPointer)[m->row][m->col];	/* rook move 3 right */
		}
		else{	/* right castling */
			(*boardPointer)[m->row][kingCol+2]=(*boardPointer)[m->row][kingCol];	/* king move 2 right */
			(*boardPointer)[m->row][m->col-2]=(*boardPointer)[m->row][m->col];	/* rook move 2 left */
		}
		(*boardPointer)[m->row][m->col]=EMPTY;
		(*boardPointer)[m->row][kingCol]=EMPTY;
	}
	else{
		if(m->promoteTo) (*boardPointer)[m->newRow][m->newCol]=m->promoteTo;
		else			(*boardPointer)[m->newRow][m->newCol]=(*boardPointer)[m->row][m->col];
		(*boardPointer)[m->row][m->col]=EMPTY;
	}
}

/* 
*	gets a legal move, apply it no board, updates castling accordingly, switch players, and return 2 if mate, 1 if check and 0 otherwise
*	throws calloc errors
*/
int applyMove(game *g, move *m){
	updateCastling(g ,m);
	applyMoveOnBoard(g->boardPointer, m);
	switchColor(g->nextTurnPlayerColor);
	if((!canPlayerMove(g))||(standart_fail)) return 2;
	else if(isKingThreatened(g->boardPointer,g->nextTurnPlayerColor)) return 1;
	return 0;
}

/* check if pawn that reached row should be promoted*/
int isPromoted(int row, int pawnColor){
	return (pawnColor==white)?(row==7):(row==0);
}

/* returns the Coordinates of the king of color kingcolor*/
void getKingCoordinates(boardp boardPointer, int *row, int *col, int kingColor){
	for(*row=0;(*row)<BOARD_SIZE;(*row)++)
		for(*col=0;(*col)<BOARD_SIZE;(*col)++)
			if((*boardPointer)[*row][*col]==getKingChar(kingColor)) return;
}

/*	gets a game pointer and check if the king of playerColor is threatened by the opposite player. */
int isKingThreatened(boardp boardPointer, int playerColor){
	move m={0,0,0,0,0};
	int kingCol=0, kingRow=0;
	getKingCoordinates(boardPointer,&kingRow,&kingCol,playerColor);
	while(getNextMove(boardPointer,&m,opponentColor(playerColor)))
		if((m.newRow==kingRow)&&(m.newCol==kingCol))
			return true;
	return false;
}

/*
*   checks if a leggit move is endangering the king and therefore cannot be done. if so returns true
*	Precondtion: the move is leggit in all other ways, accept it might danger the king 
*	can be castling move (if leggit)
*	throws calloc errors
*/
int isKingInDangerAfterMove(boardp boardPointer,move *m){
	boardp changedBoardp = (chessboard *)calloc((BOARD_SIZE * BOARD_SIZE), sizeof(char));
	int result, playerColor=occupier_color(m->row,m->col,boardPointer);
	if(changedBoardp==NULL){calloc_error(); standart_fail=true; return 0;}
	copy_board(boardPointer,changedBoardp);
	applyMoveOnBoard(changedBoardp,m);
	result=isKingThreatened(changedBoardp,playerColor);
	free(changedBoardp); 
	return result;
}


/*	
*	gets a legal move and check if after making the move:
*	the next player would be under check (isKingInDangerAfterMove) return 1
*	the next player would be under mate (isMateMove) return 2
*	otherwise return 0
*	throws calloc errors
*/
int isCheckOrMateMove(game *g, move *m){
	int result=0; game temp;
	temp.boardPointer=(chessboard *)calloc((BOARD_SIZE * BOARD_SIZE), sizeof(char));
	if(temp.boardPointer==NULL){ calloc_error(); standart_fail=true ; return 0;}
	/*copying the game and trying the move */
	copyChessGame(g,&temp);
	result=applyMove(&temp,m);
	free(temp.boardPointer);
	return result;
}

/* 
*	updates m to the first legal move on the board by playerColor that doesn't endanger the king if the player can't move at all, returns false
*	returning castling moves first.
*	throws calloc errors
*/
int getFirstLegalMove(game *g, move *m){
	int startingRow=g->nextTurnPlayerColor==white?0:7, moveFound=false;
	m->row=m->newRow=startingRow;
	m->promoteTo=0;
	if(isCastlingPossible(g,true)){	/*check left castling*/
		if(standart_fail) return false;
		m->col=m->newCol=0;
		return true;
	}
	if(isCastlingPossible(g,false)){	/*check right castling*/
		if(standart_fail) return false;
		m->col=m->newCol=7;
		return true;
	}
	m->row=m->col=m->newRow=m->newCol=0;
	/* skips all the moves that endanger the king*/
	while((moveFound=getNextMove(g->boardPointer,m,g->nextTurnPlayerColor))&&(isKingInDangerAfterMove(g->boardPointer,m))&&(!standart_fail)); 
	return moveFound;
}

/* 
*	gets a pointer to a legal move by playerColor on the board, and update it to the next legal move that doesn't endanger the king. 
*	if it was the last one, returns false
*	returning castling moves first.
*	throws calloc errors
*/
int getNextLegalMove(game *g, move *m){
	int moveFound=false;
	if((isCastlingMove(m))&&(m->col==0)){	/*last move was left castling*/
		if(isCastlingPossible(g,false)){	/*check right castling*/
			if(standart_fail) return false;
			m->col=m->newCol=7;
			return true;
		}
	}
	/* skips all the moves that endanger the king*/
	while((moveFound=getNextMove(g->boardPointer,m,g->nextTurnPlayerColor))&&(isKingInDangerAfterMove(g->boardPointer,m))&&(!standart_fail)); 
	return moveFound;
}

/* 
*	get a game pointer and check if the current player can move at all 
*	throws calloc errors
*/
int canPlayerMove(game *g){
	move m={0,0,0,0,0};
	return getFirstLegalMove(g,&m);
}

/*
*	get a coordinate of a piece belongs to playerColor, - <row,col>.
*	updates row,col to the next piece which belong to playerColor on the board, if exists and return true
*	otherwise (that was the last piece) return false
*/
int getNextPiece(boardp boardPointer, int playerColor, int *row, int *col){
	for((*row)=((*col)==7)?((*row)+1):(*row);(*row)<BOARD_SIZE;(*row)++)
		for((*col)=((*col)>=7)?0:((*col)+1);(*col)<BOARD_SIZE;(*col)++)
			if(occupier_color(*row, *col,boardPointer)==playerColor)
				return true;
	return false;
}

void printMove(move *m){
	if(isCastlingMove(m))
		printf("castle <%c,%d>", ('a'+m->col), m->row+1);
	else{
		printf("<%c,%d> to <%c,%d>", ('a'+m->col), m->row+1,('a'+m->newCol), m->newRow+1);
		switch(m->promoteTo){
		case BLACK_Q: case WHITE_Q:		printf(" queen"); break;
		case BLACK_B: case WHITE_B:		printf(" bishop"); break;
		case BLACK_R: case WHITE_R:		printf(" rook"); break;
		case BLACK_KN: case WHITE_KN:	printf(" knight"); break;
		}	
	}
	printf("\n");
}

/*
*	gets a square [row,col] and check if it's contain a piece belongs to the current player, and if it's on the board
*	if so prints it's possible moves and returns the number of possible moves
*	otherwise (bad coordinates, or no piece on this square) do whatever we asked it to do using the functions arguments and return 0;
*	printingOptions-	0 doesn't print anything just count moves;
*						1 print all the moves, but doesn't print anyhting if the square not on board or empty
*						2 print moves if it's on a legal square with piece belongs to player, otherwise error message
*	if it's been called with mateMove!=null, checks if one of the moves from <row,col> that can lead to mate 
*	and updates mateMoveExist & mateMove accordingly
*	throws calloc errors
*/
int printMoves(game *g, int row,int col, int printingOptions, int *mateMoveExist, move *mateMove){
	int movesCounter=0, startingRow=g->nextTurnPlayerColor==white?0:7;
	move m={0}, leftCastling={0}, rightCastling={0};
	move *correctCastle=(col==0)?&leftCastling:&rightCastling;
	m.row=row; m.col=col; 
	leftCastling.row=leftCastling.newRow=rightCastling.row=rightCastling.newRow=startingRow;
	leftCastling.col=leftCastling.newCol=0;
	rightCastling.col=rightCastling.newCol=7;
	if(!in_range(row,col)){
		if(printingOptions==2) printf(WRONG_POSITION); 
	}
	else if(occupier_color(row,col,g->boardPointer)!=g->nextTurnPlayerColor){
		if(printingOptions==2) printf(NO_PIECE);
	}
	else if(getFirstPieceMove(g->boardPointer,&m,g->nextTurnPlayerColor)){
		do{		/* count all the regulars moves from [row,col] which doesn't include castling*/
			if(!isKingInDangerAfterMove(g->boardPointer,&m)){
				if(standart_fail) return 0;
				if(printingOptions!=0)	printMove(&m);
				movesCounter++;
				if((mateMove!=NULL)&&(!*mateMoveExist)){	/* we are looking for mate moves and didn't found one yet */
					*mateMoveExist=(isCheckOrMateMove(g,&m)==2);
					if(standart_fail) return 0;
					if(*mateMoveExist) copyMove(&m,mateMove);
				}
			}
		} while(getNextPieceMove(g->boardPointer,&m));
		if((*g->boardPointer)[row][col]==getRookChar(g->nextTurnPlayerColor)){	/* [row,col] contains rook- check the appropriate castling*/
			if(isCastlingPossible(g,col==0)){
				if(standart_fail){return 0;}
				if(printingOptions!=0)	printMove(correctCastle);
				movesCounter++;
				if((mateMove!=NULL)&&(!*mateMoveExist)){
					*mateMoveExist=(isCheckOrMateMove(g,correctCastle)==2);
					if(standart_fail) return 0;
					if(*mateMoveExist){
						mateMove->row=m.row; mateMove->col=m.col; 
						mateMove->newRow=m.newRow, mateMove->newCol=m.newCol; 
						mateMove->promoteTo=m.promoteTo;
					}
				}
			}
		}
	}
	return movesCounter;
}

/* 
*	returns how many legal moves can be made by the current player
*	if printOrNot==true also print them
*	if mateMove!=null, checks if one of those moves can lead to mate and updates mateMoveExist & mateMove accordingly
*	throws calloc errors
*/
int printAllMoves(game *g, int printOrNot, int *mateMoveExist,move *mateMove){
	int row,col, movesCounter=0;
	for(row=0;(row<BOARD_SIZE)&&(!standart_fail);row++)
		for(col=0;(col<BOARD_SIZE)&&(!standart_fail);col++)
			movesCounter+=printMoves(g,row,col,printOrNot?1:0,mateMoveExist,mateMove);
	return movesCounter;
}

/* 
*	gets a string representing the promotion and returns the character that represent it
*	if no match was found return 0;
*/
char getCharForPromotion(char *promotionString, int playerColor){
	char typeChar=0;
	if(isupper(*promotionString)) *promotionString=tolower(*promotionString);
	if(! strcmp( promotionString, "queen"))	return getQueenChar(playerColor);
	if(! strcmp( promotionString, "bishop"))	return getBishopChar(playerColor);
	if(! strcmp( promotionString, "rook"))	return getRookChar(playerColor);
	if(! strcmp( promotionString, "knight"))	return getKnightChar(playerColor);
	return typeChar;
}