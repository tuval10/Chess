#include "Minimax.h"
int numOfCallsToAlphaBeta=0;

/* 
*	updates son to be the parent after applying moveFromParent and switching users 
*	if fails return false
*	PRECONDITION: both board are initiallized
*	throws calloc errors
*/
void createSonGame(game *parent, game *son, move *moveFromParent){
	copyChessGame(parent, son);
	applyMove(son,moveFromParent);
	if(standart_fail) return;
	son->depth--;
}

moveDeque initDeque(){
	moveDeque deque;
	deque.firstNode=NULL;
	return deque;
}

int isDequeEmpty(moveDeque *deque){
	return (deque->firstNode==NULL);
}

void pushLast(moveDeque *deque, moveListNode *newNode){
	moveListNode *lastNode;
	if(isDequeEmpty(deque)){
		newNode->next=newNode;
		newNode->prev=newNode;
		deque->firstNode=newNode;
	}
	else{
		lastNode=deque->firstNode->prev;
		newNode->prev=lastNode;
		newNode->next=deque->firstNode;
		lastNode->next=newNode;
		deque->firstNode->prev=newNode;
	}
}

/*
*	create a new node from m & score and append it to deque 
*	throws malloc errors
*/
void pushLast2(moveDeque *deque, move *m, double score){
	moveListNode *newNode;
	newNode=(moveListNode *)malloc(sizeof(moveListNode));
	if(newNode==NULL){
		malloc_error();
		standart_fail=true;
		return;
	}
	copyMove(m,&(newNode->nodeMove));
	newNode->score=score;
	pushLast(deque,newNode);

}

/*
*	cut the moveListNode toBeCutted from the deque 
*	doesn't free toBeCutted
*/
void deleteNode(moveDeque *deque, moveListNode * toBeCutted){
	moveListNode *nextNode=toBeCutted->next, *prevNode=toBeCutted->prev;
	if(isMovesEqual(&(nextNode->nodeMove),&(toBeCutted->nodeMove))){ /* 1-move node queue */
		deque->firstNode=NULL;
	}
	else{
		if(deque->firstNode==toBeCutted)
			deque->firstNode=nextNode;
		nextNode->prev = prevNode;
		prevNode->next = nextNode;
	}
}

moveListNode *popFirst(moveDeque *deque){
	moveListNode *returnNode;
	if(isDequeEmpty(deque)) return NULL;
	returnNode = deque->firstNode;
	deleteNode(deque,returnNode);
	return returnNode;
}

/* frees all the nodes in the deque*/
void deleteDeque(moveDeque *deque){
	moveListNode *iterator=deque->firstNode;
	while(!isDequeEmpty(deque)){
		deleteNode(deque,iterator);
		free(iterator);
		iterator=deque->firstNode;
	}
}

/* cut the node with the highest score from the board*/
moveListNode *popMax(moveDeque *deque){
	moveListNode *tempMax, *iterator; 
	if(isDequeEmpty(deque))
		return NULL;
	tempMax=deque->firstNode;
	iterator=deque->firstNode->next;
	while(!isMovesEqual(&(deque->firstNode->nodeMove),&(iterator->nodeMove)))
	{	/*finding the node with maximum score*/
		if(iterator->score > tempMax->score)
			tempMax=iterator;
		iterator=iterator->next;
	}
	deleteNode(deque,tempMax);
	return tempMax;
}

void bubbleSortMoves(moveDeque *deque){
	moveDeque sortedDeque=initDeque();
	moveListNode *tempMax;
	while(!isDequeEmpty(deque)){
		tempMax=popMax(deque);
		pushLast(&sortedDeque,tempMax);
	}
	deque->firstNode=sortedDeque.firstNode;
}

double getMoveScore(game *g, move *m){
	double score=0, ourPieceScore, opponentPieceScore, promotionScore;
	if(occupier_color(m->newRow,m->newCol,g->boardPointer)!=EOF){
		ourPieceScore = (double) getPoints ( (*g->boardPointer)[m->row][m->col] ,allsets , score_per_set , NUM_OF_SETS);
		opponentPieceScore = (double) getPoints ( (*g->boardPointer)[m->newRow][m->newCol] , allsets,score_per_set,NUM_OF_SETS);
		score = opponentPieceScore / (-ourPieceScore);
		assert(score>0);
		if(m->promoteTo!=0){ /*add promotion score*/
			promotionScore = getPoints( m->promoteTo , allsets,score_per_set,NUM_OF_SETS);
			if(promotionScore<0) promotionScore*=(-1);
			score+=promotionScore;
		}
	}
	return score;
}

/*
*	append deque2 to the end of deque1
*	POSTCONDITION: isDequeEmpty(deque2)
*/
void appendDeques(moveDeque *deque1,moveDeque *deque2){
	moveListNode *deque1Last, *deque2Last;
	if(isDequeEmpty(deque2)) return ;
	else if(isDequeEmpty(deque1)){
		deque1->firstNode=deque2->firstNode;
	}
	else{
		deque1Last=deque1->firstNode->prev;
		deque2Last=deque2->firstNode->prev;
		deque1Last->next=deque2->firstNode;
		deque2->firstNode->prev=deque1Last;
		deque2Last->next=deque1->firstNode;
		deque1->firstNode->prev=deque2Last;
	}
	deque2->firstNode=NULL;
}

int isKingEatingMove(game *g, move *m){
	char opponentKingChar=getKingChar(opponentColor(g->nextTurnPlayerColor));
	return ((*g->boardPointer)[m->newRow][m->newCol]==opponentKingChar);
}

int isSorted(moveDeque * deque){
	moveListNode * iterator;
	if(isDequeEmpty(deque)) return true;
	for(
		iterator=deque->firstNode;
		(!isMovesEqual (&(iterator->next->nodeMove) , &(deque->firstNode->nodeMove)));
		iterator = iterator->next )
	{
		if( iterator->score < iterator->next->score)
			return false;
	}
	return true;
}

int isOnlyPieceLeftWithKings(game *g, int row, int col){
	int rowIndex=0, colIndex=0, currPlayer = g->nextTurnPlayerColor;
	char tempChar, kingChar=getKingChar(currPlayer), opponentKingChar=getKingChar(opponentColor(currPlayer));
	if((*g->boardPointer)[row][col]==opponentKingChar)
		return false;
	for(rowIndex=0 ; rowIndex<BOARD_SIZE ; rowIndex++){
		for(colIndex=0 ; colIndex<BOARD_SIZE ; colIndex++){
			if((row!=rowIndex)||(col!=colIndex)){
				if(occupier_color(rowIndex,colIndex,g->boardPointer)!=EOF){
					tempChar=(*g->boardPointer)[rowIndex][colIndex];
					if( (tempChar!=kingChar) && (tempChar!=opponentKingChar) )
						return false;
				}
			}
		}
	}
	return true;
}

/* gets a  move and check if it's a stalemate move*/
int isStalemateMove(game *g, move *m){
	int oppKingRow=0, oppKingCol=0, currPlayer=g->nextTurnPlayerColor;
	if((*g->boardPointer)[m->row][m->col]==getKingChar(currPlayer))
	{ /* king eating other piece*/
		if(isOnlyPieceLeftWithKings(g,m->newRow,m->newRow))
		{	/*only piece left with kings- check that the other king can't eat our king after moving*/
			getKingCoordinates(g->boardPointer,&oppKingRow,&oppKingCol,opponentColor(currPlayer));
			if( ((intAbs(oppKingRow - m->newRow)) > 1) || ((intAbs(oppKingCol - m->newCol)) > 1) )
			{	
				return true;
			}
		}
	}
	return false;
}


/*
*	build the list of moves from the board
*	if a king eating move available, returns it only. otherwise returns all moves sorted by the order:
*	1.eating moves - sorted by priority- lowest score of current player moved, high score piece was eaten, pawn promotion was made
*	2.non-eating Queen-Promotion Moves 
*	3.non-eating knight-Promotion Moves
*	4.castling move
*	5. regular not eating move by the order:
*		5.1	pawn securing other pieces (which are not secured)
*		5.2	regular non eating moves (not pawn, or pawn which isn't securing/desecuring) - randomly sorted 
*		5.3	pawns desecuring (going away from a square where they secured)
*	
*	throws malloc errors
*/
moveDeque getSortedMoves(game *g){
	moveDeque eatingMoves = initDeque();
	moveDeque nonEeatingQueenPromotionMoves = initDeque();
	moveDeque nonEeatingKnightPromotionMoves = initDeque();
	moveDeque castingMoves = initDeque();
	moveDeque pawnSecuring = initDeque();
	moveDeque nonEatingMoves = initDeque();
	moveDeque pawnDesecuring = initDeque();
	moveDeque allMoves = initDeque();
	move m={0,0,0,0,0}, leftCastling={0}, rightCastling={0}, *temp;
	int currPlayer=g->nextTurnPlayerColor, startingRow=(currPlayer==white)?0:7;
	double score;
	while(getNextMove(g->boardPointer,&m,g->nextTurnPlayerColor)){
		if((m.promoteTo==getBishopChar(currPlayer))||(m.promoteTo==getRookChar(currPlayer)))
			continue;
		if(isKingEatingMove(g,&m)){ /*found a king eating move- returns only this move*/
			pushLast2(&allMoves,&m,(g->nextTurnPlayerColor==white)?INT_MAX:INT_MIN);
			if(standart_fail) break;
			deleteDeque(&eatingMoves);
			deleteDeque(&nonEeatingQueenPromotionMoves);
			deleteDeque(&nonEeatingKnightPromotionMoves);
			deleteDeque(&pawnSecuring);
			deleteDeque(&nonEatingMoves);
			deleteDeque(&pawnDesecuring);
			return  allMoves;
		}
		score=getMoveScore(g,&m);
		if(score>0){
			pushLast2(&eatingMoves,&m,score);
			if(standart_fail) break;
		}
		else if(m.promoteTo!=0){
			pushLast2(
				(m.promoteTo==getQueenChar(currPlayer)) ? (&nonEeatingQueenPromotionMoves) : (&nonEeatingKnightPromotionMoves) , &m , 0);
			if(standart_fail) break;
		}
		else
		{	/* regular non eating move- then sort by pawnSecureScore*/
			score=getPawnSecuringMoveScore(g,&m);
			if(score>0)
				pushLast2(&pawnSecuring, &m, score);
			else if(score==0)
				pushLast2(&nonEatingMoves, &m, (rand()%100)); /*give random score in [0,99]*/
			else
				pushLast2(&pawnDesecuring, &m, score);		
			if(standart_fail) break;
		}
	}
	if(standart_fail){
		deleteDeque(&eatingMoves); deleteDeque(&nonEeatingQueenPromotionMoves); deleteDeque(&nonEeatingKnightPromotionMoves); deleteDeque(&pawnSecuring); deleteDeque(&nonEatingMoves); deleteDeque(&pawnDesecuring);
		return  allMoves;
	}
	/* check for castling */
	leftCastling.row=leftCastling.newRow=rightCastling.row=rightCastling.newRow=startingRow;
	leftCastling.col=leftCastling.newCol=0;
	rightCastling.col=rightCastling.newCol=7;
	if(isCastlingPossible(g,true)){
		pushLast2(&castingMoves, &leftCastling, 0);	
		if(standart_fail){
			deleteDeque(&eatingMoves); deleteDeque(&nonEeatingQueenPromotionMoves); deleteDeque(&nonEeatingKnightPromotionMoves); deleteDeque(&pawnSecuring); deleteDeque(&nonEatingMoves); deleteDeque(&pawnDesecuring);
			return  allMoves;
		}
	}
	if(isCastlingPossible(g,false)){
		pushLast2(&castingMoves, &rightCastling, 0);
		if(standart_fail){
			deleteDeque(&eatingMoves); deleteDeque(&nonEeatingQueenPromotionMoves); deleteDeque(&nonEeatingKnightPromotionMoves); deleteDeque(&pawnSecuring); deleteDeque(&nonEatingMoves); deleteDeque(&pawnDesecuring);
			return  allMoves;
		}
	}

	/* sorting all the queues which need sorting*/
	bubbleSortMoves(&eatingMoves);
	if((!isDequeEmpty(&eatingMoves))&&(eatingMoves.firstNode->next==eatingMoves.firstNode))
	{ /* check if there's only 1 move in eating moves*/
		temp=&(eatingMoves.firstNode->nodeMove);
		if(isStalemateMove(g,temp))
		{	/*it's a stalemate- return only it*/
			deleteDeque(&nonEeatingQueenPromotionMoves);
			deleteDeque(&nonEeatingKnightPromotionMoves);
			deleteDeque(&castingMoves);
			deleteDeque(&pawnSecuring);
			deleteDeque(&nonEatingMoves);
			deleteDeque(&pawnDesecuring);					
		}
	}
	bubbleSortMoves(&pawnSecuring);
	bubbleSortMoves(&nonEatingMoves);
	bubbleSortMoves(&pawnDesecuring);

	assert(isSorted(&eatingMoves));
	assert(isSorted(&pawnSecuring));
	assert(isSorted(&nonEatingMoves));
	assert(isSorted(&pawnDesecuring));

	/* creating one list to all */
	appendDeques(&allMoves,&eatingMoves);
	appendDeques(&allMoves,&nonEeatingQueenPromotionMoves);
	appendDeques(&allMoves,&nonEeatingKnightPromotionMoves);
	appendDeques(&allMoves,&castingMoves);
	appendDeques(&allMoves,&pawnSecuring);
	appendDeques(&allMoves,&nonEatingMoves);
	appendDeques(&allMoves,&pawnDesecuring);
	return allMoves;
}

/*	
*	implementing alpha beta pruning
*	sending sentinel with *moveFromPreviousBoard={-1,-1,-1,-1,-1} returns the chosen move to take
*	throws calloc errors
*/
int AlphaBeta(game *g, move *moveFromPreviousBoard, int alpha, int beta){
	game newGameState;
	moveListNode *currCheckedMove;
	move chosenMoveFromSons={0};
	int score=0, *alphaOrBeta, playerColor;
	moveDeque deque;
	++numOfCallsToAlphaBeta;
	newGameState.boardPointer=(chessboard *)calloc((BOARD_SIZE * BOARD_SIZE), sizeof(char));
	if(newGameState.boardPointer==NULL){ calloc_error(); standart_fail=true; return false;}
	do{
		/*if root-copy otherwise build the son by applying the move */
		(moveFromPreviousBoard->col==-1)?(copyChessGame(g,&newGameState)):(createSonGame(g, &newGameState, moveFromPreviousBoard));
		if(standart_fail){ free(newGameState.boardPointer); return 0;}
		playerColor=newGameState.nextTurnPlayerColor;
		/*it's a leaf because we reached max num of calls to alphabeta or it's depth is 0 */
		if((newGameState.depth==0)||(numOfCallsToAlphaBeta>=MAX_LEAVES)){ 
			score=getBoardScore(newGameState.boardPointer,allsets,score_per_set,NUM_OF_SETS); 
			break;
		}
		else{
			deque=getSortedMoves(&newGameState);
			if(standart_fail){ deleteDeque(&deque); break;}
			alphaOrBeta=(playerColor==white)?&alpha:&beta;
			if(isDequeEmpty(&deque)){ /* no moves possible */
				score=getBoardScore(newGameState.boardPointer,allsets,score_per_set,NUM_OF_SETS); 
				break;
			}
			currCheckedMove=popFirst(&deque);
			copyMove(&currCheckedMove->nodeMove,&chosenMoveFromSons);
			if(isDequeEmpty(&deque))
			{	/*one move only- check if it's a kingEatingMove or stale mate*/
				if(isKingEatingMove(g,&(currCheckedMove->nodeMove)))
				{
					score=(playerColor==white)?INT_MAX:INT_MIN; 
					free(currCheckedMove);
					break;
				}
				else if(isStalemateMove(g,&(currCheckedMove->nodeMove)))
				{
					score=0;
					free(currCheckedMove);
					break;
				}
			}
			do{
				score=AlphaBeta ( &newGameState , &(currCheckedMove->nodeMove) , alpha , beta);
				if(standart_fail){break;}
				if((playerColor==white)?(*alphaOrBeta<score):(*alphaOrBeta>score)){
					*alphaOrBeta=score;
					copyMove(&(currCheckedMove->nodeMove),&chosenMoveFromSons);
				}
				free(currCheckedMove);
			} while( (numOfCallsToAlphaBeta<MAX_LEAVES) && (alpha<beta) && ((currCheckedMove=popFirst(&deque))!=NULL) );
			deleteDeque(&deque);
			score=*alphaOrBeta;
			break;
		}
	} while (true);
	if((!standart_fail)&&(moveFromPreviousBoard->col==-1)) copyMove(&chosenMoveFromSons,moveFromPreviousBoard);
	free(newGameState.boardPointer);
	return score;
}

/*
*	gets a game where the computer should play the next turn.
*	updates *moveChosen to the move the computer should do.
*	need to call with *moveChosen={-1,-1,-1,-1,-1} */
move get_AI_Move(game *g){
	move  moveChosen={-1,-1,-1,-1,-1};
	int depthIncrease=0, secondDepth, firstDepth;
	if(g->difficulty_best){
		firstDepth=calculateBestHeight(g);
		secondDepth=calculateBestHeight2(g)-2;
		g->depth=(firstDepth<secondDepth)?firstDepth:secondDepth;
	}
	if(standart_fail) return moveChosen;
	AlphaBeta(g, &moveChosen, INT_MIN, INT_MAX);
	if(standart_fail) return moveChosen;
	if(g->difficulty_best){
		while((!standart_fail)&&(numOfCallsToAlphaBeta<MAX_LEAVES)){
			/* http://en.wikipedia.org/wiki/Shannon_number branching factor of 35 - but with alpha-beta it's square(35)=6
			we use log base 10 just in case */
			depthIncrease=(int)(log10((double)(MAX_LEAVES/numOfCallsToAlphaBeta)));
			if(depthIncrease<2) break;
			numOfCallsToAlphaBeta=0;
			g->depth+=depthIncrease;
			moveChosen.col=moveChosen.newCol=moveChosen.newRow=moveChosen.promoteTo=moveChosen.row=-1;
			AlphaBeta(g, &moveChosen, INT_MIN, INT_MAX);
		}
	}
	if(standart_fail) return moveChosen;
	if(numOfCallsToAlphaBeta==MAX_LEAVES){
		g->depth--;
		moveChosen.col=moveChosen.newCol=moveChosen.newRow=moveChosen.promoteTo=moveChosen.row=-1;
		numOfCallsToAlphaBeta=0;
		AlphaBeta(g, &moveChosen, INT_MIN, INT_MAX);
		if(standart_fail) return moveChosen;
	}
	numOfCallsToAlphaBeta=0;
	return moveChosen;
}

/*
*	get the game and a possible moves from it
*	returns how open is it after applying the move (how many moves are in both sides)
*	and how many the first player can move in the second one
*	throws calloc errors
*/
int getOpenessScore(game *g){
	int movesCounter=0;
	movesCounter+=countAllMoves(g); /*count moves for the current player*/
	if(standart_fail){return false;}
	switchColor(g->nextTurnPlayerColor);
	movesCounter+=countAllMoves(g); /*count moves for the current player+opponent*/
	switchColor(g->nextTurnPlayerColor);
	return movesCounter;
}

/*
*	get the game and a possible moves from it
*	returns how open is it after applying the move (how many moves are in both sides)
*	and how many the first player can move in the second one
*	throws calloc errors
*/
int getOpenessMoveScore(game *g, move *m, int *firstPlayerScore){
	int movesCounter;
	game temp;
	temp.boardPointer=(chessboard *)calloc((BOARD_SIZE * BOARD_SIZE), sizeof(char));
	if(temp.boardPointer==NULL){calloc_error(); standart_fail=true; return false;}
	copyChessGame(g,&temp);
	applyMove(&temp,m);
	if(standart_fail){free(temp.boardPointer); return false;}
	switchColor(temp.nextTurnPlayerColor);
	*firstPlayerScore=countAllMoves(&temp); /*count moves for the current player*/
	if(standart_fail){free(temp.boardPointer); return false;}
	switchColor(temp.nextTurnPlayerColor);
	movesCounter=(*firstPlayerScore)+countAllMoves(&temp); /*count moves for the current player+opponent*/
	free(temp.boardPointer); 
	return movesCounter;
}

/*
*	gets a game where the computer should play in difficulty best,
*	decides which height would be best to reach the highest tree with less than million nodes
*	it does so by estimating the possible moves in each height by greedily choosing the board which makes the board more "open" - with more options to move
*	throws calloc errors
*/
int calculateBestHeight(game *g){
	int openessScore=getOpenessScore(g), height=2;
	if(standart_fail) return 0;
	for(;openessScore<MAX_LEAVES;openessScore*=10)
		height++;
	return height;
}

/*
*	gets a COPY of the game where the computer should play in difficulty best,
*	decides which height would be best to reach the highest tree with less than million nodes
*	it does so by estimating the possible moves in each height by greedily choosing the board which makes the board more "open" - with more options to move
*	WARNING: change g, so send a copy.
*	throws calloc errors
*/
int calculateBestHeightRec(game *g, int leaves){
	move mostOpeningMove={0}, moveIterator;
	int mostOpeningMoveOpenScore, onePlayerMostOpenScore, score, onePlayerScore;
	if(leaves>=MAX_LEAVES)	/* STOPING condition*/
		return 0;
	getFirstLegalMove(g,&moveIterator);
	if(standart_fail){return false;}
	copyMove(&moveIterator,&mostOpeningMove);
	mostOpeningMoveOpenScore=getOpenessMoveScore(g,&moveIterator, &onePlayerMostOpenScore);
	if(standart_fail){return false;}
	while((getNextLegalMove(g,&moveIterator)&&(!standart_fail))){
		score=getOpenessMoveScore(g,&moveIterator,&onePlayerScore);
		if(standart_fail){break;}
		if(mostOpeningMoveOpenScore<score){
			onePlayerMostOpenScore=onePlayerScore;
			mostOpeningMoveOpenScore=score;
			copyMove(&moveIterator,&mostOpeningMove);
		}
	}
	if(standart_fail){return false;}
	applyMoveOnBoard(g->boardPointer,&mostOpeningMove);
	updateCastling(g,&mostOpeningMove);
	switchColor(g->nextTurnPlayerColor);
	return 1+calculateBestHeightRec(g, leaves*((int)sqrt((double)onePlayerMostOpenScore))); 
}

/*
*	gets a game where the computer should play in difficulty best,
*	decides which height would be best to reach the highest tree with less than million nodes
*	it does so by estimating the possible moves in each height by greedily choosing the board which makes the board more "open" - with more options to move
*	throws calloc errors
*/
int calculateBestHeight2(game *g){
	game temp; int height=0;
	temp.boardPointer=(chessboard *)calloc((BOARD_SIZE * BOARD_SIZE), sizeof(char));
	if(temp.boardPointer==NULL){calloc_error(); standart_fail=true; return false;}
	copyChessGame(g,&temp);
	height=calculateBestHeightRec(&temp, 1);
	free(temp.boardPointer);
	return height;
}