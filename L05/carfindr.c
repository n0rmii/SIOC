#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define PNG_HEADER_LEN 8

void PPA_dominantChannel()
{

}

int main(int argc, char *argv[])
{
	if (argc == 2 && strcmp("--help", argv[1]) == 0)
	{
		printf("Usage: %s <input image> <preprocessing algorithm> [lambda]\nLambda: controls Poisson noise. Default = 0, no noise\n", argv[0]);
		return 0;
	}
	if (argc < 3 || argc > 4)
	{
		printf("Incorrect number of arguments: %d\n", argc);
		printf("Usage: %s <input image> <preprocessing algorithm> [lambda]\nLambda: controls Poisson noise. Default = 0, no noise\n", argv[0]);
		return 1;
	}
	
	if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
		printf("Error initializing SDL: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Surface *surImage = IMG_Load(argv[1]);

	if (!surImage)
	{
		printf("Error loading image '%s': %s\n", argv[1], SDL_GetError());
		return 1;
	}
	

	return 0;
}