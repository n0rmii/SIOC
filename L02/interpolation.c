#include <stdio.h>
#include <math.h>
#include <string.h>
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
	int clampPixelR, clampPixelG, clampPixelB, n;

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
				n = ((int)round((double)(i)/scale) + k);
				//technically i should precalucalte this otside the 'i' for-loop and store it in another array but eehhhhhh...
				if (n >= 0 && n < originalW)
				{
					SDL_GetRGB(*(Uint32*)((Uint8*)(surOriginal->pixels) + surOriginal->pitch * j + surOriginal->format->BytesPerPixel * n), surOriginal->format, &originalPixelR, &originalPixelG, &originalPixelB);
					clampPixelR += originalPixelR * (*kernel)(n - (double)i/scale); // I'm smart enough to know the error is here...
					clampPixelG += originalPixelG * (*kernel)(n - (double)i/scale); // ...but not smart enough to know how to fix it
					clampPixelB += originalPixelB * (*kernel)(n - (double)i/scale);
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
			for (int k = -KERNEL_RADIUS; k <= KERNEL_RADIUS; k++)
			{
				n = ((int)round((double)(j)/scale) + k);
				if (n >= 0 && n < originalH)
				{
					SDL_GetRGB(*(Uint32*)((Uint8*)(surIntermediate->pixels) + surIntermediate->pitch * n + surIntermediate->format->BytesPerPixel * i), surIntermediate->format, &originalPixelR, &originalPixelG, &originalPixelB);
					clampPixelR += originalPixelR * (*kernel)(n - (double)j/scale);
					clampPixelG += originalPixelG * (*kernel)(n - (double)j/scale);
					clampPixelB += originalPixelB * (*kernel)(n - (double)j/scale);
					
					// printf("something: %lf, %d, %d\n", round((double)(j)/scale), n, clampPixelR);
				}
			}
			targetPixelR = (Uint8)((clampPixelR < 0) ? (0) : ((clampPixelR > 255) ? (255) : (clampPixelR)));
			targetPixelG = (Uint8)((clampPixelG < 0) ? (0) : ((clampPixelG > 255) ? (255) : (clampPixelG)));
			targetPixelB = (Uint8)((clampPixelB < 0) ? (0) : ((clampPixelB > 255) ? (255) : (clampPixelB)));
			*targetPixel = SDL_MapRGB(surIntermediate->format, targetPixelR, targetPixelG, targetPixelB);
		}
		if (i%50 == 0)
		{
			// SDL_UpdateWindowSurface(winIntermediate); // I cant update winTransformed guhhhhh
			// SDL_Delay(330);
		}
	}

	SDL_UpdateWindowSurface(winIntermediate);
	SDL_DestroyWindow(winIntermediate);
	return 0;
}

int rotateImage(SDL_Surface* surOriginal, SDL_Surface* surTransformed, double angle, int pivotX, int pivotY, double (*kernel)(double))
{
	SDL_FillRect( surTransformed, NULL, SDL_MapRGB(surTransformed->format, 0, 0, 0 ));
	int originalW = surOriginal->w;
	int originalH = surOriginal->h;
	int iprim, jprim;
	double xprim, yprim;
	Uint32 *targetPixel, *sourcePixel;
	Uint8 *pivotPtr, *pivotPtrTransformed;
	Uint8 targetPixelR, targetPixelG, targetPixelB, originalPixelR, originalPixelG, originalPixelB;
	int clampPixelR, clampPixelG, clampPixelB, n;

	if (pivotX <= -originalW/2 || pivotY <= -originalH/2 || pivotX >= originalW/2 || pivotY >= originalH/2)
	{
		printf("Error rotating image: invalid pivot: width: %d, pivot X: %d, height: %d pivot Y: %d\n", originalW, pivotX, originalH, pivotY);
		return 1;
	}
	
	pivotPtr = (Uint8*)(surOriginal->pixels) + surOriginal->pitch * (originalH/2 + pivotY) + surOriginal->format->BytesPerPixel * (originalW/2 + pivotX);
	pivotPtrTransformed = (Uint8*)(surTransformed->pixels) + surTransformed->pitch * (originalH/2 + pivotY) + surTransformed->format->BytesPerPixel * (originalW/2 + pivotX);

	for (int j = -originalH/2; j < originalH/2; j++)
	{
		for (int i = -originalW/2; i < originalW/2; i++)
		{
			targetPixel = (Uint32*)(pivotPtrTransformed + surTransformed->pitch * j + surTransformed->format->BytesPerPixel * i);

			targetPixelR = 0;
			targetPixelG = 0;
			targetPixelB = 0;
			clampPixelR = 0;
			clampPixelG = 0;
			clampPixelB = 0;

			// x' = x*cos(a) + y*sin(a)   y' = x*-sin(a) + y*cos(a)
			yprim = -i * sin(angle) + j * cos(angle);
			jprim = (int)round(yprim);
			xprim = i * cos(angle) + j * sin(angle);
			iprim = (int)round(xprim);
			for (int m = -KERNEL_RADIUS; m <= KERNEL_RADIUS; m++)
			{
				for (int n = -KERNEL_RADIUS; n <= KERNEL_RADIUS; n++)
				{
					if (iprim+m >= -originalW/2 && iprim+m < originalW/2 && jprim+n >= -originalH/2 && jprim+n < originalH/2)
					{
						sourcePixel = (Uint32*)(pivotPtr + surOriginal->pitch * (jprim+n) + surOriginal->format->BytesPerPixel * (iprim+m));
						SDL_GetRGB(*sourcePixel, surOriginal->format, &originalPixelR, &originalPixelG, &originalPixelB);

						clampPixelR += originalPixelR * (*kernel)(xprim - (iprim+m)) * (*kernel)(yprim - (jprim+n));
						clampPixelG += originalPixelG * (*kernel)(xprim - (iprim+m)) * (*kernel)(yprim - (jprim+n));
						clampPixelB += originalPixelB * (*kernel)(xprim - (iprim+m)) * (*kernel)(yprim - (jprim+n));
					}
					
				}
			}
			targetPixelR = (Uint8)((clampPixelR < 0) ? (0) : ((clampPixelR > 255) ? (255) : (clampPixelR)));
			targetPixelG = (Uint8)((clampPixelG < 0) ? (0) : ((clampPixelG > 255) ? (255) : (clampPixelG)));
			targetPixelB = (Uint8)((clampPixelB < 0) ? (0) : ((clampPixelB > 255) ? (255) : (clampPixelB)));
			*targetPixel = SDL_MapRGB(surTransformed->format, targetPixelR, targetPixelG, targetPixelB);
		}
	}

	return 0;
}

int main()
{
	SDL_Surface* surOriginal = NULL;
	SDL_Window* winOriginal = NULL;
	SDL_Surface* surTransformed = NULL;
	SDL_Window* winTransformed = NULL;
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
	const char originalFilename[70] = {0};
	const char transformedFilename[100] = {0};
	const char windowFilename[130] = "C3window.png";
	const char tentFilename[130] = "C3tent.png";
	const char keysFilename[130] = "C3keys.png";
	double userScale = 1.0;
	char userKernel = 'w';
	const char userScaleStr[30] = {0};

	printf("Please enter the kernel letter: ");
	scanf("%c", &userKernel);
	printf("Enter the original image's filename: ");
	scanf("%69s", originalFilename);
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
	/*sprintf(userScaleStr, "%lf", userScale); // This is all very janky string concating so I won't be surprised by segfaults here
	strcat(windowFilename, userScaleStr);
	strcat(windowFilename, "_");
	strcat(windowFilename, originalFilename);

	strcat(tentFilename, userScaleStr);
	strcat(tentFilename, "_");
	strcat(tentFilename, originalFilename);

	strcat(keysFilename, userScaleStr);
	strcat(keysFilename, "_");
	strcat(keysFilename, originalFilename);*/

	int originalW = surImage->w;
	int originalH = surImage->h;
	int transformedW = round(originalW * userScale);
	int transformedH = round(originalH * userScale);
	// int transformedW = originalW;
	// int transformedH = originalH;

	winOriginal = SDL_CreateWindow( "Original", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, originalW + 2*SCREEN_MARGIN, originalH + 2*SCREEN_MARGIN, SDL_WINDOW_SHOWN );
	winTransformed = SDL_CreateWindow( "Transformed", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, transformedW + 2*SCREEN_MARGIN, transformedH + 2*SCREEN_MARGIN, SDL_WINDOW_SHOWN );
	// winWindow = SDL_CreateWindow( "Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, transformedW + 2*SCREEN_MARGIN, transformedH + 2*SCREEN_MARGIN, SDL_WINDOW_SHOWN );
	// winTent = SDL_CreateWindow( "Tent", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, transformedW + 2*SCREEN_MARGIN, transformedH + 2*SCREEN_MARGIN, SDL_WINDOW_SHOWN );
	// winKeys = SDL_CreateWindow( "Keys", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, transformedW + 2*SCREEN_MARGIN, transformedH + 2*SCREEN_MARGIN, SDL_WINDOW_SHOWN );
	// Make sure creating the winOriginal succeeded
	// if (!winOriginal || !winWindow || !winTent || !winKeys) {
	// 	printf("Error creating window: %s\n", SDL_GetError());
	// 	return 1;
	// }
	if (!winOriginal || !winTransformed) {
		printf("Error creating window: %s\n", SDL_GetError());
		return 1;
	}

	// Get the surface from the winOriginal
	surOriginal = SDL_GetWindowSurface(winOriginal);
	surTransformed = SDL_GetWindowSurface(winTransformed);
	surWindow = SDL_GetWindowSurface(winWindow);
	surTent = SDL_GetWindowSurface(winTent);
	surKeys = SDL_GetWindowSurface(winKeys);

	// Make sure getting the surface succeeded
	// if (!surOriginal || !surWindow || !surTent || !surKeys) {
	// 	printf("Error getting surface: %s\n", SDL_GetError());
	// 	return 1;
	// }
	if (!surOriginal || !surTransformed) {
		printf("Error getting surface: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Rect newImgPos;
	newImgPos.x = SCREEN_MARGIN;
	newImgPos.y = SCREEN_MARGIN; // width and height should be ignored according to wiki, so im only setting offset
	SDL_BlitSurface(surImage, NULL, surOriginal, &newImgPos);
	SDL_FreeSurface(surImage);

	// if (rotateImage(surOriginal, surWindow, userScale, 0, 0, &kernelWindow))
	// {
	// 	printf("Error in window\n");
	// 	return 1;
	// }
	// if (rotateImage(surOriginal, surTent, userScale, 0, 0, &kernelTent))
	// {
	// 	printf("Error in tent\n");
	// 	return 1;
	// }
	// if (rotateImage(surOriginal, surKeys, userScale, 0, 0, &kernelKeys))
	// {
	// 	printf("Error in keys\n");
	// 	return 1;
	// }
	switch (userKernel)
	{
	case 'w':
		(void)scaleImage(surOriginal, surTransformed, userScale, &kernelWindow);
		break;
	case 't':
		(void)scaleImage(surOriginal, surTransformed, userScale, &kernelTent);
		break;
	case 'k':
		(void)scaleImage(surOriginal, surTransformed, userScale, &kernelKeys);
		break;
	
	default:
	printf("Goddamn scanf: '%c'\n", userKernel);
	return 1;
		break;
	}
	

	SDL_UpdateWindowSurface(winOriginal);
	SDL_UpdateWindowSurface(winTransformed);
	// SDL_UpdateWindowSurface(winWindow);
	// SDL_UpdateWindowSurface(winTent);
	// SDL_UpdateWindowSurface(winKeys);
	
	IMG_SavePNG(surTransformed, transformedFilename);
	// IMG_SavePNG(surWindow, windowFilename); // PNG supremacy
	// IMG_SavePNG(surTent, tentFilename);
	// IMG_SavePNG(surKeys, keysFilename);

	// SDL_Delay(5000);

	// do // main loop
	// {
	// 	SDL_UpdateWindowSurface(winOriginal);
	// 	// SDL_UpdateWindowSurface(winIntermediate);
	// 	SDL_UpdateWindowSurface(winWindow);
	// 	SDL_UpdateWindowSurface(winTent);
	// 	SDL_UpdateWindowSurface(winKeys);
		
	// 	SDL_Delay(msPerFrame);
	// 	SDL_PollEvent(&event);
	// 		//Handle events on queue
	// 	while( SDL_PollEvent( &event ) != 0 )
	// 	{
	// 		//User requests quit
	// 		// printf("Event: %d\n", event.type);
	// 		if( event.type == SDL_QUIT )
	// 		{
	// 			quit = 1;
	// 		}
	// 		else if( event.type == SDL_WINDOWEVENT )
	// 		{
	// 			// Sprawdzenie, czy zdarzenie dotyczy żądania zamknięcia okna
	// 			if( event.window.event == SDL_WINDOWEVENT_CLOSE )
	// 			{
	// 				// Pobranie wskaźnika na konkretne okno na podstawie jego ID
	// 				SDL_Window* windowToClose = SDL_GetWindowFromID(event.window.windowID);
	// 				if( windowToClose != NULL )
	// 				{
	// 					SDL_DestroyWindow( windowToClose );
	// 				}
	// 			}
	// 		}
	// 	}
	// } while (!quit);
	

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
	SDL_DestroyWindow(winTransformed);
	// SDL_DestroyWindow(winWindow);
	// SDL_DestroyWindow(winTent);
	// SDL_DestroyWindow(winKeys);
	SDL_Quit();

	return 0;
}