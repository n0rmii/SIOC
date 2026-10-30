#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

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

typedef struct CFA_s {
	Uint8 w, h;
	Uint8 *pattern;
} CFA;

int main(int argc, char *argv[])
{

	CFA CFABayer, CFAXTrans;
	CFABayer.w = 2;
	CFABayer.h = 2;
	CFABayer.pattern = CFABayerPattern;

	CFAXTrans.w = 6;
	CFAXTrans.h = 6;
	CFAXTrans.pattern = CFAXTransPattern;

	return 0;
}