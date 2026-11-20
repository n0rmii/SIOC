#include "fontloader.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

void loadCharFromSur(SDL_Surface *sur, Uint32 *char_p, unsigned int pxPerChar, unsigned int pxPerCell, unsigned int cellsPerRow, unsigned int n)
{
	int xstart = (n%cellsPerRow) * pxPerCell + (pxPerCell - pxPerChar)/2;
	int ystart = (((int)floor(n/cellsPerRow)) * pxPerCell + (pxPerCell - pxPerChar)/2) * (pxPerCell*cellsPerRow); // Gonna need math.h for scaling anyway
	// int ystart = ((i/cellsPerRow) * pxPerCell + (pxPerCell - pxPerChar)/2) * (pxPerCell*cellsPerRow); // I'm gonna trust it to work

	for (unsigned int j = 0; j < pxPerChar; j++)
	{
		for (unsigned int i = 0; i < pxPerChar; i++)
		{
			*(char_p + j*pxPerChar + i) = *((Uint32 *)(sur->pixels) + ystart + j*pxPerCell*cellsPerRow + xstart + i);
		}
	}
	
}

int loadFont(Font *font, char *fontDir)
{
	char fontMapFilename[MFFL] = "";
	char charListFilename[MFFL] = "";
	char mapDimsFilename[MFFL] = "";
	unsigned int charListLen = 0;
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
	for (unsigned int i = 0; i < charListLen; i++)
	{
		font->charList[i] = fgetc(charListFile);
	}
	fclose(charListFile);
	charListFile = NULL;

	fscanf(mapDimsFile, "%u %u %u", &pxPerChar, &pxPerCell, &cellsPerRow);
	fclose(mapDimsFile);
	font->charSize = pxPerChar;

	surfontMap =  IMG_Load(charListFilename);
	if (surfontMap == NULL)
	{
		printf("No '%s' found.\n", fontMapFilename);
		free(font->charList);
		return 1;
	}

	font->charMap = malloc(sizeof(Uint32 *) * MAX_CHAR); // I'm assuming we don't get UTF-8 over 255. If we do, too bad so sad.
	for (unsigned int i = 0; i < MAX_CHAR; i++)
	{
		font->charMap[i] = NULL;
	}

	for (unsigned int i = 0; i < font->charListLen; i++)
	{
		font->charMap[font->charList[i]] = malloc(sizeof(Uint32) * pxPerChar * pxPerChar);
		loadCharFromSur(surfontMap, font->charMap[font->charList[i]], pxPerChar, pxPerCell, cellsPerRow, i);
	}

	return 0;
}

void unloadFont(Font *font)
{
	free(font->charList);
	font->charList = NULL;

	for (int i = 0; i < MAX_CHAR; i++)
	{
		if (font->charMap[i] != NULL)
		{
			free(font->charMap[i]);
			font->charMap[i] = NULL;
		}
	}
	free(font->charMap);
}

Uint32 *getFontChar(Font *font, char c)
{
	// if (c >= MAX_CHAR) // Literally cannot happen even for unsigned char but maybe UTF-8 someday?
	// {
	// 	printf("UTF-8 over 255: %d: '%c'\n", c, c);
	// 	return font->charMap[DEFAULT_CHAR];
	// }
	if (font->charMap[c] == NULL)
	{
		printf("ASCII not available for this font: %d: '%c'\n", c, c);
		return font->charMap[DEFAULT_CHAR];
	}
	return font->charMap[c];
}