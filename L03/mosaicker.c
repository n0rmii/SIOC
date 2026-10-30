#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef enum Channel_e {
	R,
	G,
	B
} Channel;

typedef struct CFA_s {
	Uint8 w, h;
	Uint8 *pattern;
} CFA;

void getCFAChannel(SDL_Surface *surOriginal, SDL_Surface *surMosaic, CFA *filter, Channel channel)
{
	SDL_FillRect( surMosaic, NULL, SDL_MapRGB(surMosaic->format, 0, 0, 0));

	// int wByFilter = ceilf(surOriginal->w / filter->w);
	// int hByFilter = ceilf(surOriginal->h / filter->h); // will need to be careful not to go outside the image

	for (int j = 0; j < surOriginal->h; j++)
	{
		for (int i = 0; i < surOriginal->w; i++)
		{
			
		}
	}
	
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
	CFABayer.pattern = CFABayerPattern;

	CFAXTrans.w = 6;
	CFAXTrans.h = 6;
	CFAXTrans.pattern = CFAXTransPattern;

	SDL_Surface *surImage = NULL;
	SDL_Surface *surOriginal = NULL;
	SDL_Window *winOriginal = NULL;
	SDL_Surface *surMosaic = NULL;
	SDL_Window *winMosaic = NULL;

	if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
		printf("Error initializing SDL: %s\n", SDL_GetError());
		return 1;
	}

	surImage = IMG_Load(argv[2]);
	if (!surImage)
	{
		printf("Error loading image: %s\n", SDL_GetError());
		return 1;
	}

	int imgW = surImage->w;
	int imgH = surImage->h;

	winOriginal = SDL_CreateWindow( "Original", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, imgW, imgH, SDL_WINDOW_SHOWN );
	winMosaic = SDL_CreateWindow( "Mosaic", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, imgW, imgH, SDL_WINDOW_SHOWN );

	if (!winOriginal || !winMosaic) {
		printf("Error creating window: %s\n", SDL_GetError());
		return 1;
	}

	surOriginal = SDL_GetWindowSurface(winOriginal);
	surMosaic = SDL_GetWindowSurface(winMosaic);

	if (!surOriginal || !surMosaic) {
		printf("Error getting surface: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Rect newImgPos;
	newImgPos.x = 0;
	newImgPos.y = 0;
	SDL_BlitSurface(surImage, NULL, surOriginal, &newImgPos);
	SDL_FreeSurface(surImage);



	SDL_DestroyWindow(winMosaic);
	SDL_DestroyWindow(winOriginal);

	SDL_Quit();

	return 0;
}