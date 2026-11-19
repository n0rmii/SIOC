#ifndef FONTLOADER_H
#define FONTLOADER_H 1

#include <SDL2/SDL.h>

typedef struct Font_s
{
	char *charList;
	unsigned int charListLen;
	unsigned int charSize;
	Uint32 *charMap;
} Font;


int loadFont(Font *font, char *fontDir); // add scaling later
// fontDir (no trailing '/') has to contain: fontmap.png, characterlist.txt, mapdims.txt
// mapdims.txt:
//		character size in pixels (square)
//		bounding cell size in pixels (square)
//		cells per row


#endif /* FONTLOADER_H */