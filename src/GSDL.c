/**********************************************************************
Matz small graphics library (GSDL) using SDL


Description:

This library was created to be able to draw the pixels generated from
my ray tracing program.

Parts are ripped off from: 
   http://www.cpp-home.com/tutorials/154_1.htm

I created the functions <init_screen>, <drawpixel>,...

...................................

History:
21i12~10_21**

[First version], I installed SDL and created functions so I more easily
can put pixels on the screen without bothering with the structs and
stuff, I put <screen> as a global variable for this purpose.

21i12~10_2224

I now prevent "segmentation fault" for those pixels which are out of
bounds using static integer pixel_out_of_bounds_error and the
screen->w, screen->h. Very good source: 
   http://www.libsdl.org/docs/html/sdlsetvideomode.html


**********************************************************************
21i12~10, Matz JB

*/

#include "SDL.h"
#include <stdio.h>
#include <unistd.h>
#include "GSDL.h"


SDL_Surface *screen;//we need this variable everywhere


//abbreviated version of <putpixel>
void drawpixel(int rectw, int x, int y, int r, int g, int b)
{
  if(screen==NULL)
    {
      printf("ERROR: you must initialize screen with init_screen before drawing on it\n");
      exit(-1);    
    }

  static int pixel_out_of_bounds_error=0;
  
  //  screen->w;
  
  if(0>x || x>screen->w || 0>y || y>screen->h)
    {
      if(pixel_out_of_bounds_error==0)
	printf("ERROR: pixel out of bounds\n");   
      
      pixel_out_of_bounds_error++; 
      
      return; 
    }
  
  if(pixel_out_of_bounds_error==2)//not printed?
    printf("...omitting subsequent errors\n");
  
  
  Uint32 color = SDL_MapRGB(screen->format, r, g, b);
  /* 
  // Lock the screen for direct access to the pixels
  if ( SDL_MUSTLOCK(screen) )
    {
      if ( SDL_LockSurface(screen) < 0 )
	{
	  fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
	  exit(-3);//return -3;
	}
    }
  */
  if(rectw==1)
    put_pixel(screen, x, y, color);
  else
    {
      //      void FillRect(int x, int y, int w, int h, int color) 
      
      SDL_Rect rect = {x, y, rectw, rectw};
      SDL_FillRect(screen, &rect, color);
    }
  /*
  // Unlock Surface if necessary
  if ( SDL_MUSTLOCK(screen) )
    SDL_UnlockSurface(screen);
  */
  // Update just the part of the display that we've changed
  SDL_UpdateRect(screen, x, y, rectw, rectw);
}


void put_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
 int bpp = surface->format->BytesPerPixel;
 // Here p is the address to the pixel we want to set
 Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
 
 switch(bpp)
   {
   case 1:
     *p = pixel;
     break;
   case 2:
     *(Uint16 *)p = pixel;
     break;
   case 3:
     if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
       {
	 p[0] = (pixel >> 16) & 0xff;
	 p[1] = (pixel >> 8) & 0xff;
	 p[2] = pixel & 0xff;
       } else {
       p[0] = pixel & 0xff;
       p[1] = (pixel >> 8) & 0xff;
       p[2] = (pixel >> 16) & 0xff;
     }
     break;
   case 4:
     *(Uint32 *)p = pixel;
     break;
   }
}

//handles all the details about creating a screen to draw on:
void init_screen(char *title, int width, int height)
{
    
  // Initialize defaults
  if((SDL_Init(SDL_INIT_VIDEO)==-1))
    {
      printf("Could not initialize SDL: %s.\n", SDL_GetError());
      exit(-1);//return -1;
    }
  
  screen = SDL_SetVideoMode(width, height, 24, SDL_SWSURFACE);
  
  SDL_WM_SetCaption(title, 0);


  if ( screen == NULL )
    {
      fprintf(stderr, "SDL:Couldn't set %d %d video mode: %s\n", width, height,SDL_GetError());
      exit(-2);//return -2;
    }
 
}

//this function keeps the window open and polls for events
void eventloop()
{
  int quit = 0;
  SDL_Event event;
  
  //loop to keep the window open   
  while( !quit )
    {
      sleep(1);//suspend execution, we don't want to hog the CPU with polling
      // Poll for events
      while( SDL_PollEvent( &event ) )
	{
	  switch( event.type )
	    {
	    case SDL_KEYUP:
	      if(event.key.keysym.sym == SDLK_ESCAPE)
		quit = 1;
	      break;
	    case SDL_QUIT:
	      quit = 1;
	      break;
	    default:
	      break;
	    }//Switch
	}//While
      
    }//While
  
  SDL_Quit();
}
