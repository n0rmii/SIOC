#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

int main(int argc, char *argv[])
{
	if (argc == 2 && strcmp(argv[1], "--help") == 0)
	{
		printf("Argument list:\nfontmap dir name | (not yet) text to print\nThe fontmap dir (no trailing '/') has to contain fontmap.png, mapdims.txt and charlist.txt.\n");
		return 0;
	}
	if (argc != 2) // change to 6 when we start working with text
	{
		printf("Incorrect number of arguments: %d\n", argc);
		return 1;
	}

	
	
	// printf("Len: %d\n", characterlistLen);
	// fclose(characterlistFile);
	// return 0;
	
	if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
		printf("Error initializing SDL: %s\n", SDL_GetError());
		return 1;
	}

	
	SDL_Quit();
	return 0;
}