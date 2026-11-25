#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

int main(int argc, char *argv[])
{
	if (argc == 2 && strcmp(argv[1], "--help") == 0)
	{
		// printf("Argument list:\nfontmap dir name | (not yet) text to print\nThe fontmap dir (no trailing '/') has to contain fontmap.png, mapdims.txt and charlist.txt.\n");
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

	
	SDL_Quit();
	return 0;
}