#include "fontloader.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

void loadCharFromSur(SDL_Surface *sur, Uint32 *charr, unsigned int pxPerChar, unsigned int pxPerCell, unsigned int cellsPerRow, unsigned int i)
{
	int xstart = (i%cellsPerRow) * pxPerCell + (pxPerCell - pxPerChar)/2;
	int ystart = (((int)floor(i/cellsPerRow)) * pxPerCell + (pxPerCell - pxPerChar)/2) * (pxPerCell*cellsPerRow);
}

int loadFont(Font *font, char *fontDir)
{
	char fontMapFilename[MFFL] = "";
	char charListFilename[MFFL] = "";
	char mapDimsFilename[MFFL] = "";
	char *charList = NULL;
	int charListLen = 0;
	unsigned int pxPerChar, pxPerCell, cellsPerRow;
	FILE  *charListFile = NULL, *mapDimsFile = NULL;
	SDL_Surface *surfontMap = NULL;

	snprintf(fontMapFilename, MFFL-1, "%s%s", fontDir, "/fontmap.png");
	snprintf(charListFilename, MFFL-1, "%s%s", fontDir, "/charlist.txt");
	snprintf(mapDimsFilename, MFFL-1, "%s%s", fontDir, "/mapdims.txt");
	// printf("'%s'\n'%s'\n", fontMapFilename, charListFilename);
	
	charListFile = fopen(charListFilename, "r");
	if (charListFile == NULL)
	{
		printf("No '%s' found.\n", charListFilename);
		return 1;
	}
	mapDimsFile = fopen(mapDimsFilename, "r");
	if (mapDimsFile == NULL)
	{
		printf("No '%s' found.\n", mapDimsFilename);
		return 1;
	}

	fseek(charListFile, 0L, SEEK_END);
	font->charListLen = ftell(charListFile);
	rewind(charListFile);
	font->charList = malloc(sizeof(char) * charListLen);
	for (int i = 0; i < charListLen; i++)
	{
		font->charList[i] = fgetc(charListFile);
	}
	fclose(charListFile);
	charListFile = NULL;

	fscanf(mapDimsFile, "%u %u %u", &pxPerChar, &pxPerCell, &cellsPerRow);
	fclose(mapDimsFile);

	surfontMap =  IMG_Load(charListFilename);
	if (surfontMap == NULL)
	{
		printf("No '%s' found.\n", fontMapFilename);
		free(font->charList);
		return 1;
	}

	font->charMap = malloc(sizeof(Uint32 *) * 256); // I'm assuming we don't get UTF-8 over 255. If we do, too bad so sad.
	for (int i = 0; i < 256; i++)
	{
		font->charMap[i] = NULL;
	}
	


	for (int i = 0; i < font->charListLen; i++)
	{
		font->charMap[font->charList[i]] = malloc(sizeof(Uint32) * pxPerChar * pxPerChar);
	}
	
}