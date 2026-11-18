#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

int main(int argc, char *argv[])
{
	if (argc == 2 && strcmp(argv[1], "--help") == 0)
	{
		printf("Argument list: fontmap dir name | characters per line | pixels per grid cell | pixels per character | (not yet) text to print\nThe fontmap dir has to contain a BMP file named fontmap.bmp with dimensions matching the parameters given and a characterlist.txt file containing the list of the characters in the fontmap, in the same order.\n");
		return 0;
	}
	
	
	return 0;
}