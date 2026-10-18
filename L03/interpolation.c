#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define WIN_WIDTH 640
#define WIN_HEIGHT 480
#define SCREEN_FPS 60
#define SCREEN_MARGIN 0 // Hopefully won't be needed

double kernelWindow(double x)
{
	if (x >= 0.5 || x < -0.5)
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
	// x = fabs(x);
	if (x >= 1.0 || x < -1.0)
	{
		return 0.0;
	}
	else
	{
		return 1.0 - fabs(x);
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
	SDL_Surface* surIntermediate = NULL;
	SDL_Window* winIntermediate = NULL;
	SDL_Surface* surTransformed = NULL;
	SDL_Window* winTransformed = NULL;

	SDL_Surface* surImage = NULL;

	SDL_Event event;

	Uint32 msPerFrame = 1000 / SCREEN_FPS;

	int quit = 0;
	const char originalFilename[100] = {0};
	const char transformedFilename[100] = {0};
	double userScale = 1.0;

	printf("Enter the original image's filename: ");
	scanf("%99s", originalFilename);
	printf("Enter the transformed image's filename: ");
	scanf("%99s", transformedFilename);
	printf("Please enter the scaling factor: ");
	scanf("%lf", &userScale);

	if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
		printf("Error initializing SDL: %s\n", SDL_GetError());
		// system("pause");
		// End the program
		return 1;
	}

	surImage = IMG_Load(originalFilename);
	if (!surImage)
	{
		printf("Error loading image: %s\n", SDL_GetError());
		return 1;
	}

	int originalW = surImage->w;
	int originalH = surImage->h;
	int transformedW = round(originalW * userScale);
	int transformedH = round(originalH * userScale);

	winOriginal = SDL_CreateWindow( "Original", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, originalW + 2*SCREEN_MARGIN, originalH + 2*SCREEN_MARGIN, SDL_WINDOW_SHOWN );
	winIntermediate = SDL_CreateWindow( "Intermediate", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, transformedW + 2*SCREEN_MARGIN, originalH + 2*SCREEN_MARGIN, SDL_WINDOW_SHOWN );
	winTransformed = SDL_CreateWindow( "Transformed", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, transformedW + 2*SCREEN_MARGIN, transformedH + 2*SCREEN_MARGIN, SDL_WINDOW_SHOWN );
	// Make sure creating the winOriginal succeeded
	if ( !winOriginal || !winIntermediate || !winTransformed) {
		printf("Error creating window: %s\n", SDL_GetError());
		// system("wait");
		// End the program
		return 1;
	}

	// Get the surface from the winOriginal
	surOriginal = SDL_GetWindowSurface( winOriginal );
	surIntermediate = SDL_GetWindowSurface( winIntermediate );
	surTransformed = SDL_GetWindowSurface( winTransformed );

	// Make sure getting the surface succeeded
	if ( !surOriginal || !surIntermediate || !surTransformed) {
		printf("Error getting surface: %s\n", SDL_GetError());
		// system("wait");
		// End the program
		return 1;
	}


	SDL_Rect newImgPos;
	newImgPos.x = SCREEN_MARGIN;
	newImgPos.y = SCREEN_MARGIN; // width and height should be ignored according to wiki, so im only setting offset
	SDL_BlitSurface(surImage, NULL, surOriginal, &newImgPos);
	SDL_FreeSurface(surImage);

	SDL_FillRect( surIntermediate, NULL, SDL_MapRGB( surIntermediate->format, 0, 0, 0 ) );
	SDL_FillRect( surTransformed, NULL, SDL_MapRGB( surTransformed->format, 0, 0, 0 ) );

	Uint32* targetPixel;
	Uint8 targetPixelR, targetPixelG, targetPixelB, originalPixelR, originalPixelG, originalPixelB;

	//stretching horizontally
	for (int j = 0; j < originalH; j++)
	{
		for (int i = 0; i < transformedW; i++)
		{
			targetPixel = (Uint32*)((Uint8*)(surIntermediate->pixels) + surIntermediate->pitch * j + surIntermediate->format->BytesPerPixel * i);
			// *targetPixel = 0;
			targetPixelR = 0;
			targetPixelG = 0;
			targetPixelB = 0;
			for (int k = 0; k < originalW; k++)
			{
				//technically i should precalucalte this otside the 'i' for-loop and store it in a nother array but eehhhhhh...
				SDL_GetRGB(*(Uint32*)((Uint8*)(surOriginal->pixels) + surOriginal->pitch * j + surOriginal->format->BytesPerPixel * k), surOriginal->format, &originalPixelR, &originalPixelG, &originalPixelB);
				targetPixelR += originalPixelR * kernelKeys((double)k - (double)i/userScale);
				targetPixelG += originalPixelG * kernelKeys((double)k - (double)i/userScale);
				targetPixelB += originalPixelB * kernelKeys((double)k - (double)i/userScale);
			}
			*targetPixel = SDL_MapRGB(surIntermediate->format, targetPixelR, targetPixelG, targetPixelB);
		}
	}

	//stretching vertically
	//i'm doing it separately because i'm not sure it would work if i did horizontal and vertical simultaneously
	for (int i = 0; i < transformedW; i++)
	{
		for (int j = 0; j < transformedH; j++)
		{
			targetPixel = (Uint32*)((Uint8*)(surTransformed->pixels) + surTransformed->pitch * j + surTransformed->format->BytesPerPixel * i);
			// *targetPixel = 0;
			targetPixelR = 0;
			targetPixelG = 0;
			targetPixelB = 0;
			for (int k = 0; k < originalH; k++)
			{
				SDL_GetRGB(*(Uint32*)((Uint8*)(surIntermediate->pixels) + surIntermediate->pitch * k + surIntermediate->format->BytesPerPixel * i), surIntermediate->format, &originalPixelR, &originalPixelG, &originalPixelB);
				targetPixelR += originalPixelR * kernelKeys((double)k - (double)j/userScale);
				targetPixelG += originalPixelG * kernelKeys((double)k - (double)j/userScale);
				targetPixelB += originalPixelB * kernelKeys((double)k - (double)j/userScale);
			}
			*targetPixel = SDL_MapRGB(surTransformed->format, targetPixelR, targetPixelG, targetPixelB);
		}
	}
	

	do // main loop
	{
		SDL_UpdateWindowSurface(winOriginal);
		SDL_UpdateWindowSurface(winIntermediate);
		SDL_UpdateWindowSurface(winTransformed);
		
		SDL_Delay(msPerFrame);
		SDL_PollEvent(&event);
			//Handle events on queue
		while( SDL_PollEvent( &event ) != 0 )
		{
			//User requests quit
			printf("Event: %d\n", event.type);
			if( event.type == SDL_QUIT )
			{
				quit = 1;
			}
			else if( event.type == SDL_WINDOWEVENT )
			{
				// Sprawdzenie, czy zdarzenie dotyczy żądania zamknięcia okna
				if( event.window.event == SDL_WINDOWEVENT_CLOSE )
				{
					// Pobranie wskaźnika na konkretne okno na podstawie jego ID
					SDL_Window* windowToClose = SDL_GetWindowFromID(event.window.windowID);
					if( windowToClose != NULL )
					{
						SDL_DestroyWindow( windowToClose );
					}
				}
			}
		}
	} while (!quit);
	

	// do // end loop to keep displaying the changes
	// {
	// 	SDL_UpdateWindowSurface(winOriginal);
	// 	SDL_UpdateWindowSurface(winTransformed);
	// 	SDL_Delay(msPerFrame);
	// 	while( SDL_PollEvent( &event ) != 0 )
	// 	{
	// 		//User requests quit
	// 		if( event.type == SDL_QUIT )
	// 		{
	// 			quit = 1;
	// 		}
	// 	}
	// } while (!quit);
	
	SDL_DestroyWindow(winOriginal);
	SDL_DestroyWindow(winIntermediate);
	SDL_DestroyWindow(winTransformed);
	SDL_Quit();

	return 0;
}