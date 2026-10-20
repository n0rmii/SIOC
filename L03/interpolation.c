#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define WIN_WIDTH 640
#define WIN_HEIGHT 480
#define SCREEN_FPS 60
#define SCREEN_MARGIN 0 // Hopefully won't be needed
#define KERNEL_RADIUS 3

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

int scaleImage(SDL_Surface* surOriginal, SDL_Surface* surTransformed, double scale, double (*kernel)(double))
{
	SDL_Surface* surIntermediate = NULL;
	SDL_Window* winIntermediate = NULL;
	int originalW = surOriginal->w;
	int originalH = surOriginal->h;
	int transformedW = round(originalW * scale)-1;
	int transformedH = round(originalH * scale)-1;
	
	// Creating a surface without a window is a pain so we're making a temporary one
	winIntermediate = SDL_CreateWindow( "Intermediate", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, transformedW + 2*SCREEN_MARGIN, originalH + 2*SCREEN_MARGIN, SDL_WINDOW_SHOWN );
	if (!winIntermediate) {
		printf("Error creating window: %s\n", SDL_GetError());
		return 1;
	}
	surIntermediate = SDL_GetWindowSurface(winIntermediate);
	if (!surIntermediate) {
		printf("Error getting surface: %s\n", SDL_GetError());
		return 1;
	}
	SDL_FillRect( surIntermediate, NULL, SDL_MapRGB(surIntermediate->format, 0, 0, 0 ));
	SDL_FillRect( surTransformed, NULL, SDL_MapRGB(surTransformed->format, 0, 0, 0 ));

	Uint32* targetPixel;
	Uint8 targetPixelR, targetPixelG, targetPixelB, originalPixelR, originalPixelG, originalPixelB;
	int clampPixelR, clampPixelG, clampPixelB;

	//stretching horizontally
	for (int j = 0; j < originalH; j++)
	{
		for (int i = 0; i < transformedW; i++)
		{
			targetPixel = (Uint32*)((Uint8*)(surIntermediate->pixels) + surIntermediate->pitch * j + surIntermediate->format->BytesPerPixel * i);
			targetPixelR = 0;
			targetPixelG = 0;
			targetPixelB = 0;
			clampPixelR = 0;
			clampPixelG = 0;
			clampPixelB = 0;
			// Going through the pixels closest to target to go from O(n^3) (atrocious) to O(n^2) (bad)
			for (int k = -KERNEL_RADIUS; k <= KERNEL_RADIUS; k++)
			{
				//technically i should precalucalte this otside the 'i' for-loop and store it in another array but eehhhhhh...
				if (((int)round((double)(i)/scale) + k) >= 0 && ((int)round((double)(i)/scale) + k) < originalW)
				{
					SDL_GetRGB(*(Uint32*)((Uint8*)(surOriginal->pixels) + surOriginal->pitch * j + surOriginal->format->BytesPerPixel * ((int)round((double)(i)/scale) + k)), surOriginal->format, &originalPixelR, &originalPixelG, &originalPixelB);
					clampPixelR += originalPixelR * (*kernel)(((int)round((double)(i)/scale) + k) - (double)i/scale); // I'm smart enough to know the error is here...
					clampPixelG += originalPixelG * (*kernel)(((int)round((double)(i)/scale) + k) - (double)i/scale); // ...but not smart enough to know how to fix it
					clampPixelB += originalPixelB * (*kernel)(((int)round((double)(i)/scale) + k) - (double)i/scale);
					
					printf("something: %lf, %d, %d\n", round((double)(i)/scale), ((int)round((double)(i)/scale) + k), clampPixelR);
				}
				// SDL_Delay(30);
			}
			targetPixelR = (Uint8)((clampPixelR < 0) ? (0) : ((clampPixelR > 255) ? (255) : (clampPixelR)));
			targetPixelG = (Uint8)((clampPixelG < 0) ? (0) : ((clampPixelG > 255) ? (255) : (clampPixelG)));
			targetPixelB = (Uint8)((clampPixelB < 0) ? (0) : ((clampPixelB > 255) ? (255) : (clampPixelB)));
			*targetPixel = SDL_MapRGB(surIntermediate->format, targetPixelR, targetPixelG, targetPixelB);
		}
		if (j%50 == 0)
		{
			SDL_UpdateWindowSurface(winIntermediate);
			// SDL_Delay(10000);
		}
	}

	//stretching vertically
	//i'm doing it separately because i'm not sure it would work if i did horizontal and vertical simultaneously
	for (int i = 0; i < transformedW; i++)
	{
		for (int j = 0; j < transformedH; j++)
		{
			targetPixel = (Uint32*)((Uint8*)(surTransformed->pixels) + surTransformed->pitch * j + surTransformed->format->BytesPerPixel * i);
			targetPixelR = 0;
			targetPixelG = 0;
			targetPixelB = 0;
			clampPixelR = 0;
			clampPixelG = 0;
			clampPixelB = 0;
			for (int k = KERNEL_RADIUS; k <= KERNEL_RADIUS; k++)
			{
				if (((int)round((double)(j)/scale) + k) >= 0 && ((int)round((double)(j)/scale) + k) < originalH)
				{
					SDL_GetRGB(*(Uint32*)((Uint8*)(surIntermediate->pixels) + surIntermediate->pitch * ((int)round((double)(j)/scale) + k) + surIntermediate->format->BytesPerPixel * i), surIntermediate->format, &originalPixelR, &originalPixelG, &originalPixelB);
					clampPixelR += originalPixelR * (*kernel)(((int)round((double)(j)/scale) + k) - (double)j/scale);
					clampPixelG += originalPixelG * (*kernel)(((int)round((double)(j)/scale) + k) - (double)j/scale);
					clampPixelB += originalPixelB * (*kernel)(((int)round((double)(j)/scale) + k) - (double)j/scale);
				}
			}
			targetPixelR = (Uint8)((clampPixelR < 0) ? (0) : ((clampPixelR > 255) ? (255) : (clampPixelR)));
			targetPixelG = (Uint8)((clampPixelG < 0) ? (0) : ((clampPixelG > 255) ? (255) : (clampPixelG)));
			targetPixelB = (Uint8)((clampPixelB < 0) ? (0) : ((clampPixelB > 255) ? (255) : (clampPixelB)));
			*targetPixel = SDL_MapRGB(surIntermediate->format, targetPixelR, targetPixelG, targetPixelB);
		}
		if (i%50 == 0)
		{
			SDL_UpdateWindowSurface(winIntermediate);
			// SDL_Delay(330);
		}
	}

	SDL_UpdateWindowSurface(winIntermediate);
	SDL_DestroyWindow(winIntermediate);
	return 0;
}

int main()
{
	SDL_Surface* surOriginal = NULL;
	SDL_Window* winOriginal = NULL;
	SDL_Surface* surWindow = NULL;
	SDL_Window* winWindow = NULL;
	SDL_Surface* surTent = NULL;
	SDL_Window* winTent = NULL;
	SDL_Surface* surKeys = NULL;
	SDL_Window* winKeys = NULL;

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
	int transformedW = round(originalW * userScale)-1;
	int transformedH = round(originalH * userScale)-1;
	// int transformedW = originalW * userScale;
	// int transformedH = originalH * userScale;

	winOriginal = SDL_CreateWindow( "Original", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, originalW + 2*SCREEN_MARGIN, originalH + 2*SCREEN_MARGIN, SDL_WINDOW_SHOWN );
	winWindow = SDL_CreateWindow( "Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, transformedW + 2*SCREEN_MARGIN, transformedH + 2*SCREEN_MARGIN, SDL_WINDOW_SHOWN );
	winTent = SDL_CreateWindow( "Tent", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, transformedW + 2*SCREEN_MARGIN, transformedH + 2*SCREEN_MARGIN, SDL_WINDOW_SHOWN );
	winKeys = SDL_CreateWindow( "Keys", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, transformedW + 2*SCREEN_MARGIN, transformedH + 2*SCREEN_MARGIN, SDL_WINDOW_SHOWN );
	// Make sure creating the winOriginal succeeded
	if (!winOriginal || !winWindow || !winTent || !winKeys) {
		printf("Error creating window: %s\n", SDL_GetError());
		return 1;
	}

	// Get the surface from the winOriginal
	surOriginal = SDL_GetWindowSurface(winOriginal);
	surWindow = SDL_GetWindowSurface(winWindow);
	surTent = SDL_GetWindowSurface(winTent);
	surKeys = SDL_GetWindowSurface(winKeys);

	// Make sure getting the surface succeeded
	if (!surOriginal || !surWindow || !surTent || !surKeys) {
		printf("Error getting surface: %s\n", SDL_GetError());
		return 1;
	}


	SDL_Rect newImgPos;
	newImgPos.x = SCREEN_MARGIN;
	newImgPos.y = SCREEN_MARGIN; // width and height should be ignored according to wiki, so im only setting offset
	SDL_BlitSurface(surImage, NULL, surOriginal, &newImgPos);
	SDL_FreeSurface(surImage);

	// if (scaleImage(surOriginal, surWindow, userScale, &kernelWindow))
	// {
	// 	printf("Error in window\n");
	// 	return 1;
	// }
	// if (scaleImage(surOriginal, surTent, userScale, &kernelTent))
	// {
	// 	printf("Error in tent\n");
	// 	return 1;
	// }
	if (scaleImage(surOriginal, surKeys, userScale, &kernelKeys))
	{
		printf("Error in keys\n");
		return 1;
	}
	

	do // main loop
	{
		SDL_UpdateWindowSurface(winOriginal);
		// SDL_UpdateWindowSurface(winIntermediate);
		SDL_UpdateWindowSurface(winWindow);
		SDL_UpdateWindowSurface(winTent);
		SDL_UpdateWindowSurface(winKeys);
		
		SDL_Delay(msPerFrame);
		SDL_PollEvent(&event);
			//Handle events on queue
		while( SDL_PollEvent( &event ) != 0 )
		{
			//User requests quit
			// printf("Event: %d\n", event.type);
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
	SDL_DestroyWindow(winWindow);
	SDL_DestroyWindow(winTent);
	SDL_DestroyWindow(winKeys);
	SDL_Quit();

	return 0;
}