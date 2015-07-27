#ifndef GUI_C
#define GUI_C

#include <stdio.h>
#include <SDL.h>
#include <SDL_video.h>
#include <stdlib.h>
#include "Chess.h"

#define SDL_SetVideoModeError() perror("Error: standard function SDL_SetVideoMode has failed")
#define SDL_FillRectError() perror("Error: standard function SDL_FillRect has failed")
#define SDL_LoadBMPError() perror("Error: standard function SDL_LoadBMP has failed")
#define SDL_DisplayFormatError() perror("Error: standard function SDL_DisplayFormat has failed")
#define SDL_SetColorKeyError() perror("Error: standard function SDL_SetColorKey has failed")
#define SDL_FlipError() perror("Error: standard function SDL_Flip has failed")
#define SDL_BlitSurfaceError() perror("Error: standard function SDL_BlitSurface has failed")
#define SDL_InitError() perror("Error: standard function SDL_Init has failed")

/* a variable to describe exit from the loop in case game ended */
extern int endLoop;

#define Widget struct Widget


/*The event structure that will be used - catching clicks and so on*/
SDL_Event Event;


/* 
*	A widget is a GUI component of any type: widow, frame, button, label and ect...
*	surface - the SDL_surface we're working on.
*	source - this is the image to be applied onto the surface, the image is usually converted to the surface - SDL_surface. 
*	root - the father/parent of "this" widget.
*	widgetChildren - a pointer to the inner widgets that this widget contains, null if where are none.
*	brotherNode - the conection to the next widget after "this widget". 
*/

Widget{
	SDL_Surface* surface;/* current component shown on screen */
	char *source;/*image loaded to surface */
	
	Widget* root; /* the widget on top of which "this" object is built. */
	Widget* brotherNode;/*my brother, the widget after me */
	Widget* widgetChildren;/* children widgets of "this" object */
	
	SDL_Rect *offset; /* we have offset to tell us where to put the surface componnent */
	int width; /*width of rectangle offset */
	int height; /*height of rectangle offset */
	int relativeX; /* X - cord */
	int relativeY;/* Y-cord */

	void 	(*ActionPerformed)(Widget *component,int x, int y); /*if we are clicked - we should be able to react	*/
};

/*
*	creating a new widget, Widget game = new Widget(params...) 
*	we need to pass a source - this will be a picture loaded into the frame 
*/
Widget* CreateWidget(Widget *parent, char *source, int relativeX, int relativeY, int width, int height, void (*ActionPerformed)(Widget *component,int x, int y));

/*ALLOCATE MEMORY FOR THE WIDGET AND FILL IT UP WITH VALUES */
Widget* BuildWidget(Widget *parent, char *source, int relativeX, int relativeY, int width, int height, void (*ActionPerformed)(Widget *component,int x, int y));

/* INSERT WIDGET - inserts the child widget to the father's list of children (father->widgetChildren) */
Widget* InsertWidget(Widget *father, Widget *Child);

/*
*	BUILDS A LABEL, label - is something you can't click, it is a background graphical element.
*	X and Y relative to parent and H-height W-width of label
*	source - image to be loaded onto the screen */
Widget* BuildLabel(int x, int y, int w, int h, char* source, Widget *parent);

/* 
*	BUILDING THE WINDOW ON WHICH EVERY WIDJET OBJECT IS DISPLAYED
* this is baiscally our main window - it is the actual control panel for every other widjet - returns a widjet object (pointer)
*/
Widget* BuildWindow(int WIDTH, int HEIGHT);

/* 
*	BUILD A BUTTON - something you can click, and a function - callback will be executed.
*	x , y -  x and y pixels relative to the parent (parent - the widget we draw on)
*	h - height and w - width of the button we are puttin on the parent widget
*	parent - the widget on top of which we draw this widget (the button)
*	ActionPerformed - function that delas with aq click.
*/
Widget* BuildButton(int x, int y, int w, int h, char* source, Widget *parent, void (*ActionPerformed)(Widget *component,int x, int y));

/* DRAWING AND LOADING FUNCTIONS: */

/*
*	UPDATNG RECTANGELES HIGHT WIDTH from pixel (X,Y) on the screen.
*	updates the parameters of the rectangle object so we know how the object looks on screen
*	we apply the offset later with blit surface function when we draw 
*/
void applyOffsetSurface( int x, int y,int width, int height, SDL_Rect* offset );

/* Loads the image in a resolotion of 32 bits */
SDL_Surface *load_image( char *filename );

/* 
*	DRAW ALL WIDGETS ON THE SCREEN
*	screenWindow - root of the generic tree
*	algorithm traverses all the tree and prints every node of it
*/
void DrawWidgets(Widget *screenWindow);

/* 
*	DELETE ALL THE WIIDGETS FROM THE COMPUTER MEMORY - destroying the tree
*	algorithm takes root of generic tree i.e: nodeWindow and deletes it with DFS alg.
*/
void DeleteWidgets(Widget **nodeWindow);

/* FREE A WIDGET FROM MEMORY */
void freeWidget(Widget *nodeWindow);

/*
*	IF ( X_clicked , Y_clicked ) ARE CLICKED ON A WIDGET RETURN 1, ELSE 0
*	widget - the widget to be checked for a click 
*/
int isWidgetClicked(Widget* widget, int X_clicked, int Y_clicked);

/* IF THIS IS A BUTTON RETURNS 1 OTHERWISE NOT A BUTTON - RETURN 0. */
int isWidgetButton(Widget* widget);

/* FINDS THE FIRST OUTER BUTTON CLICKED, USING BFS */
Widget* firstButtonClicked( Widget* widget , int clickedX, int clickedY);

/* FINDS THE BUTTON CLICKED AND RETURNS THAT BUTTON, IF NO BUTTON WAS CLICKED RETURNS NULL */
Widget* findButtonClicked( Widget* widget , int clickedX, int clickedY);

/* 	FUNCTION THAT HANDLES EVENTS - EVENTS ARE MOUSE CLICKS */
void handleEvents(Widget* root);

#endif