#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

int main(int argc, char *argv[])
{
	if (argc == 2 && strcmp(argv[1], "--help") == 0)
	{
		printf("Arg 1: input image filename\nArg 2: output text filename\n");
		return 0;
	}
	if (argc != 3)
	{
		printf("Incorrect number of arguments: %d\n", argc);
		return 1;
	}
	
	if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
		printf("Error initializing SDL: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Surface *surImage = NULL;

	surImage = IMG_Load(argv[1]);
	if (!surImage)
	{
		printf("Error loading image: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	

	
	SDL_Quit();
	return 0;
}