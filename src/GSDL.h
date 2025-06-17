/*
  GSDL is a simple SDL wrapper made to be used with my simple ray tracer
*/

#include "SDL.h"

//private:
static void put_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

//public:
void drawpixel(int res, int x, int y, int r, int g, int b);
void init_screen(char *title, int width, int height);

void eventloop();

/*
  This function polls for events and sleeps for a second, this is done
  to not hog the CPU. As a side effect the event poller will be
  slower, it will take a second for it to react, however for my ray
  tracer this is not a major concern since I will only show a ray
  traced image and not any interactive stuff.
*/
