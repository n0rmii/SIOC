#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

int main(int argc, char *argv[])
{
	if (argc == 2 && strcmp(argv[1], "--help") == 0)
	{
		printf("Argument list:\nfontmap dir name | characters per line | pixels per grid cell | pixels per character | (not yet) text to print\nThe fontmap dir (no trailing '/') has to contain a PNG file named fontmap.png with dimensions matching the parameters given and a characterlist.txt file containing the list of the characters in the fontmap, in the same order.\n");
		return 0;
	}
	if (argc != 5) // change to 6 when we start working with text
	{
		printf("Incorrect number of arguments: %d\n", argc);
		return 1;
	}

	char fontmapFilename[100] = "";
	char characterlistFilename[100] = "";
	char *characterlist = NULL;
	int characterlistLen = 0;
	FILE  *characterlistFile = NULL;
	SDL_Surface *surFontmap = NULL;

	snprintf(fontmapFilename, 99, "%s%s", argv[1], "/fontmap.png");
	snprintf(characterlistFilename, 99, "%s%s", argv[1], "/characterlist.txt");
	// printf("'%s'\n'%s'\n", fontmapFilename, characterlistFilename);
	
	characterlistFile = fopen(characterlistFilename, "r");
	if (characterlistFile == NULL)
	{
		printf("No '%s' found.\n", characterlistFilename);
		return 1;
	}

	fseek(characterlistFile, 0L, SEEK_END);
	characterlistLen = ftell(characterlistFile);
	rewind(characterlistFile);
	characterlist = malloc(sizeof(char) * characterlistLen + 1);
	for (int i = 0; i < characterlistLen; i++)
	{
		characterlist[i] = fgetc(characterlistFile);
	}
	
	// printf("Len: %d\n", characterlistLen);
	// fclose(characterlistFile);
	// return 0;
	
	if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
		printf("Error initializing SDL: %s\n", SDL_GetError());
		return 1;
	}

	surFontmap =  IMG_Load(characterlistFilename);
	if (surFontmap == NULL)
	{
		printf("No '%s' found.\n", fontmapFilename);
		SDL_Quit();
		return 1;
	}

	free(characterlist);
	SDL_Quit();
	return 0;
}