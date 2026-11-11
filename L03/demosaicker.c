// This is a modified version of L02/interpolation.c
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

#define CH_R 0
#define CH_G 1
#define CH_B 2

typedef struct CFA_s {
	Uint8 w, h;
	Uint8 *pattern;
} CFA;

Uint8 CFAMask (CFA *filter, unsigned int x, unsigned int y, unsigned int channel)
{
	return *(filter->pattern + (channel * filter->h * filter->w) + filter->w * (y % filter->h) + x % filter->w);
}

Uint8 clampToByte(double x)
{
	x = round(x);
	return (Uint8)((x < 0) ? (0) : ((x > 255) ? (255) : (x)));
}

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

int interpolateImage(SDL_Surface* surOriginal, SDL_Surface* surTransformed, double scale, double (*kernel)(double), CFA *filter)
{
	SDL_Surface* surIntermediate = NULL;
	SDL_Window* winIntermediate = NULL;
	int originalW = surOriginal->w;
	int originalH = surOriginal->h;
	// int transformedW = round(originalW * scale)-1;
	// int transformedH = round(originalH * scale)-1;
	int transformedW = originalW;
	int transformedH = originalH;
	
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
	double clampPixelR, clampPixelG, clampPixelB, weightR, weightG, weighB;
	int n;

	//interpolating horizontally

	for (int j = 0; j < originalH; j++)
	{
		for (int i = 0; i < originalW; i++)
		{
			targetPixel = (Uint32*)((Uint8*)(surIntermediate->pixels) + surIntermediate->pitch * j + surIntermediate->format->BytesPerPixel * i);
			targetPixelR = 0;
			targetPixelG = 0;
			targetPixelB = 0;
			clampPixelR = 0;
			clampPixelG = 0;
			clampPixelB = 0;
			for (int k = -KERNEL_RADIUS*ceil(scale); k <= KERNEL_RADIUS*ceil(scale); k++)
			{
				if (i+k < 0 || i+k >= originalW) continue;

				
				
			}
		}
	}
	

	// for (int j = 0; j < originalH; j++)
	// {
	// 	for (int i = 0; i < transformedW; i++)
	// 	{
	// 		targetPixel = (Uint32*)((Uint8*)(surIntermediate->pixels) + surIntermediate->pitch * j + surIntermediate->format->BytesPerPixel * i);
	// 		targetPixelR = 0;
	// 		targetPixelG = 0;
	// 		targetPixelB = 0;
	// 		clampPixelR = 0;
	// 		clampPixelG = 0;
	// 		clampPixelB = 0;
	// 		// Going through the pixels closest to target to go from O(n^3) (atrocious) to O(n^2) (bad)
	// 		for (int k = -KERNEL_RADIUS; k <= KERNEL_RADIUS; k++)
	// 		{
	// 			n = ((int)round((double)(i)/scale) + k);
	// 			//technically i should precalucalte this otside the 'i' for-loop and store it in another array but eehhhhhh...
	// 			if (n >= 0 && n < originalW)
	// 			{
	// 				SDL_GetRGB(*(Uint32*)((Uint8*)(surOriginal->pixels) + surOriginal->pitch * j + surOriginal->format->BytesPerPixel * n), surOriginal->format, &originalPixelR, &originalPixelG, &originalPixelB);
	// 				weight = (*kernel)(n - (double)i/scale);
	// 				clampPixelR += (double)originalPixelR * weight; // I'm smart enough to know the error is here...
	// 				clampPixelG += (double)originalPixelG * weight; // ...but not smart enough to know how to fix it
	// 				clampPixelB += (double)originalPixelB * weight;
	// 			}
	// 			// SDL_Delay(30);
	// 		}
	// 		targetPixelR = clampToByte(clampPixelR);
	// 		targetPixelG = clampToByte(clampPixelG);
	// 		targetPixelB = clampToByte(clampPixelB);
	// 		*targetPixel = SDL_MapRGB(surIntermediate->format, targetPixelR, targetPixelG, targetPixelB);
	// 	}
	// 	if (j%50 == 0)
	// 	{
	// 		SDL_UpdateWindowSurface(winIntermediate);
	// 		// SDL_Delay(10000);
	// 	}
	// }

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
					weight = (*kernel)(n - (double)j/scale);
					clampPixelR += (double)originalPixelR * weight;
					clampPixelG += (double)originalPixelG * weight;
					clampPixelB += (double)originalPixelB * weight;
					
					// printf("something: %lf, %d, %d\n", round((double)(j)/scale), n, clampPixelR);
				}
			}
			targetPixelR = clampToByte(clampPixelR);
			targetPixelG = clampToByte(clampPixelG);
			targetPixelB = clampToByte(clampPixelB);
			*targetPixel = SDL_MapRGB(surIntermediate->format, targetPixelR, targetPixelG, targetPixelB);
		}
	}

	SDL_UpdateWindowSurface(winIntermediate);
	SDL_DestroyWindow(winIntermediate);
	return 0;
}

int main(int argc, char *argv[])
{
	Uint8 CFABayerPattern[3][2][2] = {
		{{1, 0},
		{0, 0}}, // R
		{{0, 1},
		{1, 0}}, // G
		{{0, 0},
		{0, 1}}  // B
	};

					// c  y  x
	Uint8 CFAXTransPattern[3][6][6] = {
		{{0, 0, 1, 0, 1, 0},
		{1, 0, 0, 0, 0, 0},
		{0, 0, 0, 1, 0, 0},
		{0, 1, 0, 0, 0, 1},
		{0, 0, 0, 1, 0, 0},
		{1, 0, 0, 0, 0, 0}}, // R

		{{1, 0, 0, 1, 0, 0},
		{0, 1, 1, 0, 1, 1},
		{0, 1, 1, 0, 1, 1},
		{1, 0, 0, 1, 0, 0},
		{0, 1, 1, 0, 1, 1},
		{0, 1, 1, 0, 1, 1}}, // G

		{{0, 1, 0, 0, 0, 1},
		{0, 0, 0, 1, 0, 0},
		{1, 0, 0, 0, 0, 0},
		{0, 0, 1, 0, 1, 0},
		{1, 0, 0, 0, 0, 0},
		{0, 0, 0, 1, 0, 0}}  // B
	};

	CFA CFABayer, CFAXTrans;
	CFABayer.w = 2;
	CFABayer.h = 2;
	CFABayer.pattern = (Uint8 *)CFABayerPattern;

	CFAXTrans.w = 6;
	CFAXTrans.h = 6;
	CFAXTrans.pattern = (Uint8 *)CFAXTransPattern;

	SDL_Surface* surOriginal = NULL;
	SDL_Window* winOriginal = NULL;
	SDL_Surface* surTransformed = NULL;
	SDL_Window* winTransformed = NULL;

	SDL_Surface* surImage = NULL;

	char originalFilename[70] = {0};
	char transformedFilename[100] = {0};
	double userScale = 1.0;
	char userKernel = 'w';
	CFA *userCFA = NULL;

	if (argc != 6)
	{
		printf("Incorrect number of arguments: %d\n", argc);
		return 1;
	}
	
	userKernel = argv[1][0];
	if (argv[2][0] == 'b' || argv[2][0] == 'B')
	{
		userCFA = &CFABayer;
	}
	else if (argv[2][0] == 'x' || argv[2][0] == 'X')
	{
		userCFA = &CFAXTrans;
	}
	else
	{
		printf("Incorrect CFA code: %c\n", argv[2][0]);
		return 1;
	}
	userScale = atof(argv[3]); // should be 2 for Bayer and ~3 for XTrans
	strncpy(originalFilename, argv[4], 70);
	strncpy(transformedFilename, argv[5], 100);

	if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
		printf("Error initializing SDL: %s\n", SDL_GetError());
		return 1;
	}

	surImage = IMG_Load(originalFilename);
	if (!surImage)
	{
		printf("Error loading image: %s\n", SDL_GetError());
		return 1;
	}

	int imgW = surImage->w;
	int imgH = surImage->h;
	int originalW = imgW;
	int originalH = imgH;
	// int originalW = round(sqrt(imgW*imgW + imgH*imgH));
	// int originalH = originalW;
	// int transformedW = round(originalW * userScale);
	// int transformedH = round(originalH * userScale);
	int transformedW = originalW;
	int transformedH = originalH;

	winOriginal = SDL_CreateWindow( "Original", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, originalW + 2*SCREEN_MARGIN, originalH + 2*SCREEN_MARGIN, SDL_WINDOW_SHOWN );
	winTransformed = SDL_CreateWindow( "Transformed", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, transformedW + 2*SCREEN_MARGIN, transformedH + 2*SCREEN_MARGIN, SDL_WINDOW_SHOWN );

	if (!winOriginal || !winTransformed) {
		printf("Error creating window: %s\n", SDL_GetError());
		return 1;
	}

	// Get the surface from the winOriginal
	surOriginal = SDL_GetWindowSurface(winOriginal);
	surTransformed = SDL_GetWindowSurface(winTransformed);

	if (!surOriginal || !surTransformed) {
		printf("Error getting surface: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Rect newImgPos;
	newImgPos.x = 0;
	newImgPos.y = 0;
	// newImgPos.x = originalW/2 - imgW/2;
	// newImgPos.y = originalH/2 - imgH/2; // width and height should be ignored according to wiki, so im only setting offset
	SDL_BlitSurface(surImage, NULL, surOriginal, &newImgPos);
	SDL_FreeSurface(surImage);
	switch (userKernel)
	{
	case 'w':
	case 'W':

		(void)interpolateImage(surOriginal, surTransformed, userScale, &kernelWindow, userCFA);
		break;
	case 't':
	case 'T':

		(void)interpolateImage(surOriginal, surTransformed, userScale, &kernelTent, userCFA);
		break;
	case 'k':
	case 'K':

		(void)interpolateImage(surOriginal, surTransformed, userScale, &kernelKeys, userCFA);
		break;
	
	default:
		printf("Wrong kernel letter: '%c'\n", userKernel);
		SDL_DestroyWindow(winOriginal);
		SDL_DestroyWindow(winTransformed);
		SDL_Quit();
		return 1;
		break;
	}
	

	SDL_UpdateWindowSurface(winOriginal);
	SDL_UpdateWindowSurface(winTransformed);
	
	IMG_SavePNG(surTransformed, transformedFilename);
	
	SDL_DestroyWindow(winOriginal);
	SDL_DestroyWindow(winTransformed);
	SDL_Quit();

	return 0;
}