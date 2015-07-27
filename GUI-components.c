#include "GUI-components.h"

/* creating a new widget, Widget game = new Widget(params...) */
/* we need to pass a source - this will be a picture loaded into the frame */
/* throws : malloc_error() */
Widget* CreateWidget(Widget *parent, char *source, int relativeX, int relativeY, int width, int height, void (*ActionPerformed)(Widget *component,int x, int y))
{
	Widget *widget = BuildWidget(parent, source, relativeX, relativeY, width, height, ActionPerformed);
	if(standart_fail){return NULL;}
	
	widget->root = parent;/* setting the parent widget */

	/* here widget is the root of our graphic tree */
	if(parent == NULL)
	{ return widget;}

	/*pushing the widget to it's fathers list of children.*/
	parent->widgetChildren = InsertWidget(parent, widget);
	return widget;
}

/* throws : malloc_error() */
Widget* BuildWidget(Widget *parent, char *source, int relativeX, int relativeY, int width, int height, void (*ActionPerformed)(Widget *component,int x, int y))
{
	/*pointers*/
	Widget *widget = (Widget*)malloc(sizeof(Widget));
	if(widget == NULL)/*check for malloc fail*/
	{
		standart_fail = true;
		malloc_error();
		return NULL;
	}

	widget->widgetChildren = NULL;/*no children at creation*/
	widget->brotherNode = NULL;

	widget->offset = (SDL_Rect*)malloc(sizeof(SDL_Rect));
	if(widget->offset == NULL)/*check malloc for offset RECT obj*/
	{
		standart_fail = true;
		free(widget);/* free memory of widget*/
		malloc_error();
		return NULL;
	}

	/*image - the SDL_Surface *surface updated by this image*/
	widget->source =  source;

	if(parent != NULL)/*coordinates of rectangle:*/
	{
		/* used for SDL_Rect *offset - setting where the surface should be on the screen*/
		widget->relativeX = relativeX + parent->relativeX;
		widget->relativeY = relativeY + parent->relativeY;
	}
	else/*coordinates if we have a parent*/
	{
		widget->relativeX = relativeX;
		widget->relativeY = relativeY;
	}

	/* if needed this is kept for the main screen */
	widget->height = height;
	widget->width = width;

	applyOffsetSurface( widget->relativeX, widget->relativeY, widget->width, widget->height, widget->offset );
	/* hadeling a click on the widget - function assignment */
	widget ->ActionPerformed = ActionPerformed;
	widget->surface=NULL;

	return widget;
}

/* INSERT WIDGET - inserts the child widget to the father's list of children (father->widgetChildren) */
Widget* InsertWidget(Widget *father, Widget *Child)
{
	Widget *ChildrenOfFather = father->widgetChildren;
    Widget *head = NULL, *lastChild = NULL;
	/* No children widget in the father*/
	if(ChildrenOfFather == NULL)
	{

		ChildrenOfFather = Child;
		ChildrenOfFather->brotherNode = NULL;
	}
	else/*push the Child widget onto the father's widget list*/
	{   
		/* finding the tail of father's children list */
		for(head = ChildrenOfFather; head != NULL; head = head->brotherNode)
		{   
			lastChild = head;
		} 	
		lastChild->brotherNode = Child;
	}
	/*return father's child list after update*/
	return ChildrenOfFather;
}

/* A label is something that cannot be pressed - it is where for design : for example background.
   ActionPreformed - should be NULL Nothing happens if we press
   throws : malloc_error() */
Widget* BuildLabel(int x, int y, int w, int h, char* source, Widget *parent)
{
	return CreateWidget(parent, source, x, y, w, h, NULL); /*return the label*/
}

/* this is the creation of the screen itself */
/* throw SDL_SetVideoModeError(), SDL_FillRectError() */
Widget* BuildWindow(int WIDTH, int HEIGHT)
{
   Widget* screen = CreateWidget(NULL , NULL, 0, 0, WIDTH, HEIGHT, NULL);
   
   if(standart_fail)
   {
	   free(screen);
	   return NULL;
   }

   screen->surface = SDL_SetVideoMode(screen->width,screen->height, 0, SDL_HWSURFACE | SDL_DOUBLEBUF);
   if(screen->surface == NULL)
   {
	   standart_fail = true;
	   SDL_SetVideoModeError();
	   return NULL;
   }

   if(SDL_FillRect(screen->surface, NULL, SDL_MapRGB(screen->surface->format, 255, 255, 255)) == -1)
   {
	   standart_fail = true;
	   SDL_FillRectError();
	   return NULL;
   }

   return screen;
}

/* BUILDS THE BUTTON - a button has a function ActionPreformed that hadles the action taken
 throws : malloc_error()*/
Widget* BuildButton(int x, int y, int w, int h, char* source, Widget *parent, void (*ActionPerformed)(Widget *component,int x, int y))
{
	return CreateWidget(parent, source, x, y, w, h, ActionPerformed);
}


/* DRAWING AND LOADING FUNCTIONS:

UPDATNG RECTANGELES HIGHT WIDTH from pixel (X,Y) on the screen.
updates the parameters of the rectangle object so we know how the object looks on screen
we apply the offset later with blit surface function when we draw
*/
void applyOffsetSurface( int x, int y,int width, int height, SDL_Rect* offset )
{
	offset->w = width;
	offset->h = height;
	offset->x = x;
	offset->y = y;
}

/* Loads the image in a resolotion of 32 bits */
/* throws SDL_LoadBMPError(), SDL_DisplayFormatError(), SDL_SetColorKeyError() */
SDL_Surface *load_image( char *filename ) 
{
	/* Temporary storage for the image that's loaded */
	SDL_Surface* loadedImage = NULL;

	/*The optimized image that will be used*/
	SDL_Surface* optimizedImage = NULL;
	Uint32 colorkey;

	/*Load the image*/
	loadedImage = SDL_LoadBMP( filename );

	/* If something went wrong in loading the image */  
	if( loadedImage == NULL )
	{
		standart_fail = true;
		SDL_LoadBMPError();
		printf("\n FILE: %s" ,filename);
		return NULL;
	}

	/*Create an optimized image*/
	optimizedImage = SDL_DisplayFormat( loadedImage );
	/*Free the old image*/
	SDL_FreeSurface( loadedImage );

	/*If the image wasn't optimized*/
	if( optimizedImage == NULL )
	{
		standart_fail = true;
		SDL_DisplayFormatError();
		return NULL;
	}

	/* Map the color key */
	colorkey = SDL_MapRGB( optimizedImage->format, 255, 0, 255 );
	/*Set all pixels of color magnata = RGB(255, 0 ,255) to be transparent
	SDL_SRCCOLORKEY - is for transparency*/
	if(SDL_SetColorKey( optimizedImage, SDL_SRCCOLORKEY, colorkey) == -1)
	{
		standart_fail = true;
		SDL_SetColorKeyError();
		return NULL;
	}

	/*Return the optimized image*/
	return optimizedImage;
}

/* draw the widget on the screenWindow */
/* throws SDL_LoadBMPError(), SDL_DisplayFormatError(), SDL_SetColorKeyError(), SDL_BlitSurface() */
void drawWidget(Widget *widget,Widget *screenWindow)
{
		if(widget->root != NULL)
		{
			widget->surface = load_image(widget->source);
			if(standart_fail)
			{
				return;
			}

			if(SDL_BlitSurface( widget->surface, NULL, screenWindow->surface , widget->offset) != 0)
			{
				standart_fail = true;
				SDL_BlitSurfaceError();
				return;
			}
		} 
}

/* DRAWS ALL WIDGETS GRAPHICALLY ON THE SCREEN */
/* throws : SDL_LoadBMPError(), SDL_DisplayFormatError(), SDL_SetColorKeyError(), SDL_BlitSurface()*/
void DrawAllWidgets(Widget *nodeWindow, Widget *screenWindow)
{
	if(nodeWindow == NULL){return;}

	/*Draw the widget nodeWindow*/
	drawWidget(nodeWindow, screenWindow);
	if(standart_fail){return;}/* on fail of SDL functions*/

	if(nodeWindow->widgetChildren == NULL && nodeWindow->brotherNode == NULL)
	{ 
		Widget* root = nodeWindow->root;	
		if( root != NULL && root->brotherNode != NULL)
		{
			DrawAllWidgets(root->brotherNode, screenWindow); 
			if(standart_fail){return;}/* on fail of SDL functions*/
			return;
		}
		else return;
	}

	/* if this node has no children move to brother */
	if(nodeWindow->widgetChildren == NULL)
	{
		if(nodeWindow->brotherNode != NULL)
		{
			DrawAllWidgets(nodeWindow->brotherNode, screenWindow);
			if(standart_fail){return;}/* on fail of SDL functions*/
		}
	}

	else{   
		/* here we go to the children - widget children aren't null */
		DrawAllWidgets(nodeWindow->widgetChildren, screenWindow);
		if(standart_fail){return;}/* on fail of SDL functions*/
	}

}

/* 
 DRAW THE GENERIC TREE ONTO THE SCREEN
 screenWindow is the root of the generic tree
 throws : SDL_LoadBMPError(), SDL_DisplayFormatError(), SDL_SetColorKeyError(), SDL_BlitSurfaceError(), SDL_FlipError() 
*/
void DrawWidgets(Widget *screenWindow)
{
	DrawAllWidgets(screenWindow, screenWindow);
	if(standart_fail){return;}
	
	if(SDL_Flip(screenWindow->surface) == -1)
	{
		standart_fail = true;
		return;
	}
}


/* DELETING THE SCREEN FROM ALL THE WIDGETS 
nodeWindow is a pointer to the root of the generic tree we built using the other functions
we use a depth first traversal to delete the entire tree from memory*/
void DeleteWidgets(Widget **nodeWindow)
{
	if(nodeWindow == NULL){return;}/* do nothing*/

	/* go through the brothers when we reach the last one we check its children*/
	if( (*nodeWindow)->brotherNode != NULL)
	{ 
		DeleteWidgets(&(*nodeWindow)->brotherNode);
	}

	/* if this node has no brothers - its the last brother move to its children */
	if((*nodeWindow)->widgetChildren != NULL)
	{
		/* here we go to the children - widget children aren't null*/
		DeleteWidgets(&(*nodeWindow)->widgetChildren);
	}

	/* we reched the bottom rightmost widget in the tree it has to be removed first*/
	if((*nodeWindow)->brotherNode == NULL && (*nodeWindow)->widgetChildren == NULL)
	{
		/* free the surface*/
		if((*nodeWindow)->surface!=NULL)
			SDL_FreeSurface( (*nodeWindow)->surface);

		if( (*nodeWindow)->offset != NULL)
		{/* free the offset rectangle in the database of the widget*/
			free((*nodeWindow)->offset);
		}

		if( (*nodeWindow)->widgetChildren != NULL )
		{/* free the children of the widget */
			free( (*nodeWindow)->widgetChildren);
		}

		if( (*nodeWindow)->brotherNode != NULL)
		{/*free the brother of the widget*/
			free( (*nodeWindow)->brotherNode );
		}


		/* finally remove the nodeWindow from computer memory*/
		free(*nodeWindow);
		*nodeWindow = NULL;
		return;
	}

}


/*check that the X and Y that are clicked by the mouse event are located inside the widget*/
/* if x and y are in widget return 1 otherwise return 0.*/
int isWidgetClicked(Widget* widget, int X_clicked, int Y_clicked)
{
	int x, y, width, height;

	x = widget->relativeX;
	y = widget->relativeY;
	width = widget->width;
	height = widget->height;
	/* we have clicked on a the widget's area*/
	if( (x < X_clicked) && (X_clicked < x + width) && (y < Y_clicked) && (Y_clicked < y + height))
	return 1;
	
	else return 0;
}

/* IF THIS IS A BUTTON RETURNS 1 OTHERWISE NOT A BUTTON - RETURN 0.*/
int isWidgetButton(Widget* widget)
{
	if(widget->ActionPerformed == NULL) return 0;
	
	else 
		return 1;
}

/* FINDS THE FIRST OUTER BUTTON CLICKED, USING BFS*/
Widget* firstButtonClicked( Widget* widget , int clickedX, int clickedY)
{
	Widget *head = NULL;
	
	/* first look through the list of brothers - find which widget is clicked */
	for( head = widget; head != NULL; )
	{
		if(isWidgetClicked(head, clickedX, clickedY))/* some widget clicked*/
		{
			if(isWidgetButton(head))/*if the widget is a buttton we are done and a pointer to the button will be returned*/
			return head;

			else if(head->widgetChildren != NULL) /* look inside children maybe one of its buttons was clicked */
			  return firstButtonClicked(head->widgetChildren ,clickedX, clickedY);
		}
		
		if(head->brotherNode == NULL) break;
		else head = head->brotherNode;
	}
	/* no button pressed inside the list of brother widgets */
	/* assumption is that every inner widget is encapsulated in its parent node */
	return NULL;
}

/* GIVEN A WIDGET IT FINDS IF CLICKED ON A BUTTON AND RETURNS THAT BUTTON
   THE WIDGET WILL USUALLY BE THER SCREEN OR IN OTHER WORDS ROOT OF THE UI TREE */
Widget* findButtonClicked( Widget* widget , int clickedX, int clickedY )
{
	Widget *button = firstButtonClicked(widget , clickedX, clickedY);/*get the first button pressed*/
	Widget *innerButton = button;/* we want to find the actual button that is pressed - inner button*/

	while(button != NULL)
	{
		if(button->widgetChildren != NULL)
		{ /* maybe a more inner button was clicked? find it*/
			innerButton = firstButtonClicked(button->widgetChildren , clickedX, clickedY);
		}
		else
		{ /* if the button has no children it is the most inner button*/
			return button;
		}

		if(innerButton != NULL)
		{/* we found an inner button*/
			button = innerButton;/* keep searching for a more inner button*/
		}
		else
		{ /* no more buttons to search through, found it!*/
			return button;
		}
	}

	return button;
}

/* one loop that handles all the events */ 
/* events = our events are defined to be only mouse clicks */
void handleEvents(Widget* root)
{
	Widget* buttonClicked;
	int clickedX, clickedY;

	while(!endLoop && !standart_fail)
	{	
		while ( !standart_fail && SDL_PollEvent(&Event) != 0 && !endLoop )
		{
			switch (Event.type)
			{
			case (SDL_QUIT):/*pressing the X on the window*/
				endLoop = 1;
				break;/*END OF SDL_QUIT*/				

			case (SDL_MOUSEBUTTONUP):	/* mouse was clicked*/

				clickedX = Event.button.x;
				clickedY = Event.button.y;
				buttonClicked = findButtonClicked( root , clickedX, clickedY);/* find button pressed ( clicked )*/

				if(buttonClicked != NULL)
					buttonClicked->ActionPerformed(buttonClicked , clickedX, clickedY);/* callback of a click*/

				break;/*END OF SDL_MOUSEBUTTONUP*/

			default: break; /* take no action */
			}
		}

	}
}