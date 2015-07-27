INCLUDES = -I/usr/local/lib/SDL_image-1.2.12/lib/include/SDL/include
LFLAGS = -L/usr/local/lib/SDL_image-1.2.12/lib

all: chessprog

clean:
	-rm Chessboard.o Minimax.o Chess.o GUI-components.o gui.o chessprog
	
chessprog: Chessboard.o Minimax.o Chess.o GUI-components.o gui.o 
	gcc $(INCLUDES)  -o $@  Chessboard.o Minimax.o Chess.o GUI-components.o gui.o -lm -ansi -pedantic-errors -g `sdl-config --libs` -lSDL_image $(LFLAGS)
	
Chessboard.o: Chessboard.c Chessboard.h 
	gcc -ansi -pedantic-errors -c -Wall -g -lm Chessboard.c
	
Minimax.o: Minimax.c Minimax.h Chessboard.c Chessboard.h
	gcc -ansi -pedantic-errors -c -Wall -g -lm Minimax.c
	
Chess.o: Chess.c Chess.h Minimax.c Minimax.h Chessboard.c Chessboard.h
	gcc -ansi -pedantic-errors -c -Wall -g -lm Chess.c
	
GUI-components.o: GUI-components.c GUI-components.h Chess.c Chess.h Minimax.c Minimax.h Chessboard.c Chessboard.h
	gcc -ansi -pedantic-errors $(INCLUDES) -c -Wall -g -lm  GUI-components.c `sdl-config --cflags`
	
gui.o: gui.c gui.h GUI-components.c GUI-components.h Chess.c Chess.h Minimax.c Minimax.h Chessboard.c Chessboard.h
	gcc -ansi -pedantic-errors $(INCLUDES) -c -Wall -g -lm gui.c `sdl-config --cflags`