#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define CH_R 0b0001U
#define CH_G 0b0010U
#define CH_B 0b0100U

typedef struct CFA_s {
	Uint8 w, h;
	Uint8 ***pattern;
} CFA;

void getCFAChannel(SDL_Surface *surOriginal, SDL_Surface *surMosaic, CFA *filter, unsigned int channels)
{
	Uint32 *targetPixel, *sourcePixel;
	Uint8 targetPixelR, targetPixelG, targetPixelB, originalPixelR, originalPixelG, originalPixelB;
	SDL_FillRect( surMosaic, NULL, SDL_MapRGB(surMosaic->format, 0, 0, 0));

	// int wByFilter = ceilf(surOriginal->w / filter->w);
	// int hByFilter = ceilf(surOriginal->h / filter->h); // will need to be careful not to go outside the image

	for (int j = 0; j < surOriginal->h; j++)
	{
		for (int i = 0; i < surOriginal->w; i++)
		{
			sourcePixel = (Uint32*)((Uint8*)(surOriginal->pixels) + surOriginal->pitch * j + surOriginal->format->BytesPerPixel * i);
			targetPixel = (Uint32*)((Uint8*)(surMosaic->pixels) + surMosaic->pitch * j + surMosaic->format->BytesPerPixel * i);
			targetPixelR = 0;
			targetPixelG = 0;
			targetPixelB = 0;

			SDL_GetRGB(*sourcePixel, surOriginal->format, &originalPixelR, &originalPixelG, &originalPixelB);
			if (channels & CH_R)
			{
				targetPixelR = originalPixelR * filter->pattern[0][j % filter->h][i % filter->w];
			}
			if (channels & CH_G)
			{
				targetPixelG = originalPixelG * filter->pattern[1][j % filter->h][i % filter->w];
			}
			if (channels & CH_B)
			{
				targetPixelB = originalPixelB * filter->pattern[2][j % filter->h][i % filter->w];
			}
			*targetPixel = SDL_MapRGB(surMosaic->format, targetPixelR, targetPixelG, targetPixelB);
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

	if (argc != 5)
	{
		printf("Error getting program arguments! argc: %d\n");
		for (int i = 1; i < argc; i++)
		{
			printf("%s\n", argv[i]);
		}
		return 1;
	}

	
	

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