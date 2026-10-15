#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>

#define WIN_WIDTH 640
#define WIN_HEIGHT 480
#define SCREEN_FPS 60

double kernelWindow(double x)
{
	if (fabs(x) > 0.5)
	{
		return 0.0;
	}
	else
	{
		return 1.0;
	}
}

double kernelTent(double x)
{
	x = fabs(x);
	if (x > 1.0)
	{
		return 0.0;
	}
	else
	{
		return 1.0 - x;
	}
}

double kernelKeys(double x)
{
	x = fabs(x);
	if (x == 0.0)
	{
		return 1;
	}
	else if (x >= 2.0 || x == 1.0)
	{
		return 0;
	}
	else if (x < 2.0 && x > 1.0)
	{
		return -0.5 * x*x*x + 2.5 * x*x - 4 * x + 2;
	}
	else if (x < 1)
	{
		return 1.5 * x*x*x - 2.5 * x*x + 1;
	}
	else
	{
		printf("WTF that's not supposed to happen: %lf\n", x);
		return 0;
	}
}

int main()
{
	SDL_Surface* surOriginal = NULL;
	SDL_Window* winOriginal = NULL;
	SDL_Surface* surTransformed = NULL;
	SDL_Window* winTransformed = NULL;

	SDL_Event event;

	Uint32 msPerFrame = 1000 / SCREEN_FPS;

	int quit = 0;

	if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
		printf("Error initializing SDL: %s\n", SDL_GetError());
		// system("pause");
		// End the program
		return 1;
	}

	winOriginal = SDL_CreateWindow( "Original", 100, 100, WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_SHOWN );
	winTransformed = SDL_CreateWindow( "Transformed", 800, 100, WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_SHOWN );
	// Make sure creating the winOriginal succeeded
	if ( !winOriginal || !winTransformed) {
		printf("Error creating window: %s\n", SDL_GetError());
		// system("wait");
		// End the program
		return 1;
	}

	// Get the surface from the winOriginal
	surOriginal = SDL_GetWindowSurface( winOriginal );
	surTransformed = SDL_GetWindowSurface( winTransformed );

	// Make sure getting the surface succeeded
	if ( !surOriginal || !surTransformed) {
		printf("Error getting surface: %s\n", SDL_GetError());
		// system("wait");
		// End the program
		return 1;
	}

	SDL_FillRect( surOriginal, NULL, SDL_MapRGB( surOriginal->format, 0, 0, 0 ) );
	SDL_UpdateWindowSurface(winOriginal);
	
	SDL_Delay(msPerFrame);
	SDL_PollEvent(&event);
		//Handle events on queue
	while( SDL_PollEvent( &event ) != 0 )
	{
		//User requests quit
		if( event.type == SDL_QUIT )
		{
			quit = 1;
		}
	}

	SDL_DestroyWindow(winOriginal);
	SDL_DestroyWindow(winTransformed);
	SDL_Quit();

	return 0;
}