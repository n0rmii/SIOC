#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>

#define WIN_WIDTH 640
#define WIN_HEIGHT 480
#define SCREEN_FPS 60
#define PROP_RADIUS 200
#define PROP_WIDTH 10
#define PROP_SPEED 20000
#define PROP_ARMCOUNT 2

// const float winCenterX = WIN_WIDTH/2;
// const float winCenterY = WIN_HEIGHT/2;

void drawPropeller(SDL_Surface* surface, double angle, Uint8 armCount)
{
	Uint32* targetPixel;
	const Uint32 mappedPixel = SDL_MapRGB(surface->format, 255, 0, 0);
	Uint8* surfaceCenter = (Uint8 *) surface->pixels + surface->pitch * WIN_HEIGHT/2 + surface->format->BytesPerPixel * WIN_WIDTH/2;
	// Uint8* surfaceCentera = (Uint8 *) (surface->pixels[1][1]);
	// Uint32** surfaceCenter = &(((Uint32**)(surface->pixels))[WIN_HEIGHT/2][WIN_WIDTH/2]);
	// Uint32** nicePixelMap = ((Uint32**)(surface->pixels));
	for (int a = 0; a < armCount; a++)
	{
		for (int i = 0; i < PROP_RADIUS; i++)
		{
			for (int j = -PROP_WIDTH; j < PROP_WIDTH; j++)
			{
				// targetPixel = (Uint32*) (surfaceCenter + (Sint64)(/*surface->pitch*/ WIN_WIDTH * j * sin(angle) + /*surface->format->BytesPerPixel **/ i * cos(angle)) * surface->format->BytesPerPixel);
				// surface->pixels[i] = SDL_MapRGB(surface->format, 255, 0, 0);
				// targetPixel = (Uint32 *) ((Uint8 *) surfaceCenter + j * surface->pitch + i * surface->format->BytesPerPixel);
				// nicePixelMap[(Uint32)(WIN_WIDTH/2 + i * cos(angle) + j * sin(angle))][(Uint32)(WIN_HEIGHT/2 + i * sin(angle) + j * cos(angle))] = mappedPixel;
				targetPixel = (Uint32*)(surfaceCenter + (Sint64)(surface->pitch * (Sint64)(j * cos(angle + a * (2*M_PI)/armCount) - i * sin(angle + a * (2*M_PI)/armCount)) + 4 * (Sint64)(j * sin(angle + a * (2*M_PI)/armCount) + i * cos(angle + a * (2*M_PI)/armCount))));
				*targetPixel = mappedPixel;
			}
		}
	}
}

void scanHorizontal(SDL_Surface* surface, SDL_Surface* capture, Uint32 scanline)
{
	for (int i = 0; i < WIN_WIDTH; i++)
	{
		*(Uint32*)((Uint8*)capture->pixels + capture->pitch * scanline + 4 * i) = *(Uint32*)((Uint8*)surface->pixels + surface->pitch * scanline + 4 * i);
		*(Uint32*)((Uint8*)surface->pixels + capture->pitch * scanline + 4 * i) = SDL_MapRGB(surface->format, 0, 255, 0);
	}
	
}

int main()
{
	SDL_Surface* capSurface = NULL;
	SDL_Window* capWindow = NULL;
	SDL_Surface* winSurface = NULL;
	SDL_Window* window = NULL;

	SDL_Event event;

	Uint32 msPerFrame = 1000 / SCREEN_FPS;

	int quit = 0;
	double userSpeed = 1;
	int userArmCount = 1;

	printf("Speed: ");
	scanf("%lf", &userSpeed);
	printf("Speed: %lf\nArm count: ", userSpeed);
	scanf("%d", &userArmCount);

	if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
		printf("Error initializing SDL: %s\n", SDL_GetError());
		// system("pause");
		// End the program
		return 1;
	}

	window = SDL_CreateWindow( "Propeller", 100, 100, WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_SHOWN );
	capWindow = SDL_CreateWindow( "Capture", 800, 100, WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_SHOWN );
	// Make sure creating the window succeeded
	if ( !window || !capWindow ) {
		printf("Error creating window: %s\n", SDL_GetError());
		// system("wait");
		// End the program
		return 1;
	}

	// Get the surface from the window
	winSurface = SDL_GetWindowSurface( window );
	capSurface = SDL_GetWindowSurface( capWindow );

	// Make sure getting the surface succeeded
	if ( !winSurface || !capSurface ) {
		printf("Error getting surface: %s\n", SDL_GetError());
		// system("wait");
		// End the program
		return 1;
	}

	double angle = 0.0;

	Uint32 scanline = 0;

	SDL_FillRect( capSurface, NULL, SDL_MapRGB( capSurface->format, 0, 0, 0 ) );

	do // Main program loop
	{
		SDL_FillRect( winSurface, NULL, SDL_MapRGB( winSurface->format, 0, 0, 0 ) );
		drawPropeller(winSurface, angle, userArmCount);
		scanHorizontal(winSurface, capSurface, scanline);
		SDL_UpdateWindowSurface( window );
		SDL_UpdateWindowSurface(capWindow);
		scanline++;
		angle += 0.0001 * userSpeed;
		
		
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
		
	} while (/*event.type != SDL_QUIT && scanline < WIN_HEIGHT*/ !quit && scanline < WIN_HEIGHT);
	
	if (SDL_SaveBMP(capSurface, "Capture.bmp") != 0 || SDL_SaveBMP(winSurface, "Propeller.bmp") != 0)
	{
		printf("Error saving bitmap: %s\n", SDL_GetError());
	}
	

	/*while (!quit)
	{
		// SDL_Delay(1000);
		SDL_PollEvent(&event);
		 //Handle events on queue
		//  printf("waiting...\n");
		while( SDL_PollEvent( &event ) != 0 )
		{
			// printf("processing...\n");
			//User requests quit
			if( event.type == SDL_QUIT )
			{
				// printf("setting quit...\n");
				quit = 1;
			}
		}
	}*/
	printf("destroying...\n");

	// Destroy the window. This will also destroy the surface
	SDL_DestroyWindow( window );
	SDL_DestroyWindow(capWindow);

	// Quit SDL
	SDL_Quit();

	return 0;
}