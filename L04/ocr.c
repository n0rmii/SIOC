// Text recognition with cross-correlation or convolution
// Successor of litery.c, this time without any windows
//
// Usage: ./ocr <image> <korelacja|splot> <lambda> <alphabet image> <output.txt> [threshold] [reference.txt]
//   image:          picture of the text (e.g. terminal screenshot, monospace font)
//   korelacja|splot cross-correlation or convolution (k / s for short)
//   lambda:         mean photon count for bright pixels (shot noise), 0 = no noise
//   alphabet image: one line with ALPHABET_STRING written in the SAME font,
//                   exactly (number of characters) * (cell width) pixels wide, one cell high
//   output.txt:     recognized text (UTF-8)
//   threshold:      minimal normalized score of a letter, default 0.6
//   reference.txt:  correct text, if given the number of wrong characters is printed
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define DEFAULT_THRESHOLD 0.6
#define MAX_GLYPHS 256
#define MAX_TEXT 65536

#define MODE_CORRELATION 0
#define MODE_CONVOLUTION 1

// Characters in the alphabet image, in this exact order (no space, spaces are found from the gaps)
const char *ALPHABET_STRING =
	"!\"#$%&'()*+,-./0123456789:;<=>?@"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
	"abcdefghijklmnopqrstuvwxyz{|}~"
	"ąćęłńóśźżĄĆĘŁŃÓŚŹŻ„”—…";

typedef struct Template_s {
	char glyph[5]; // one UTF-8 character
	int w, h;
	double *pixels; // zero-mean already
	double energy;  // sum of squares of the zero-mean pixels
} Template;

typedef struct Detection_s {
	int x, y; // top-left corner of the letter's cell
	int glyph;
	double score;
} Detection;

double *getDArrPointer(double *array, unsigned int width, unsigned int x, unsigned int y)
{
	return array + (width * y) + (x);
} // Still funky

int utf8Length(unsigned char c)
{
	if ((c & 0x80) == 0x00) return 1;
	if ((c & 0xE0) == 0xC0) return 2;
	if ((c & 0xF0) == 0xE0) return 3;
	return 4;
}

int poissonRandom(double lambda)
{
	if (lambda <= 0.0) return 0;
	if (lambda > 500.0)
	{
		// Knuth's method would underflow here, so Box-Muller and pray to the central limit theorem
		double u1 = (rand() + 1.0) / (RAND_MAX + 2.0);
		double u2 = (rand() + 1.0) / (RAND_MAX + 2.0);
		double x = round(lambda + sqrt(lambda) * sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2));
		return (x < 0) ? (0) : ((int)x);
	}
	// Knuth: multiply uniform numbers until the product drops below e^-lambda
	double limit = exp(-lambda);
	double product = 1.0;
	int k = -1;
	do
	{
		k++;
		product *= (rand() + 1.0) / (RAND_MAX + 2.0);
	} while (product > limit);
	return k;
}

// Loads any image SDL_image understands into a grayscale array with values in [0, 1]
double *loadGrayImage(char *filename, int *w, int *h)
{
	SDL_Surface *surLoaded = NULL;
	SDL_Surface *surConverted = NULL;
	double *array = NULL;
	Uint8 r, g, b;

	surLoaded = IMG_Load(filename);
	if (!surLoaded)
	{
		printf("Error loading image %s: %s\n", filename, SDL_GetError());
		return NULL;
	}
	surConverted = SDL_ConvertSurfaceFormat(surLoaded, SDL_PIXELFORMAT_ARGB8888, 0); // so that every pixel is a nice Uint32
	SDL_FreeSurface(surLoaded);
	if (!surConverted)
	{
		printf("Error converting image %s: %s\n", filename, SDL_GetError());
		return NULL;
	}

	*w = surConverted->w;
	*h = surConverted->h;
	array = malloc(*w * *h * sizeof(double));
	for (int j = 0; j < *h; j++)
	{
		for (int i = 0; i < *w; i++)
		{
			SDL_GetRGB(*(Uint32*)((Uint8*)(surConverted->pixels) + surConverted->pitch * j + surConverted->format->BytesPerPixel * i), surConverted->format, &r, &g, &b);
			*getDArrPointer(array, *w, i, j) = (r + g + b) / 3; // (0.299 * r + 0.587 * g + 0.114 * b) / 255.0;
		}
	}
	SDL_FreeSurface(surConverted);
	return array;
}

// X -> poisson(X * lambda) / lambda, just like in the footnote
void addShotNoise(double *image, int w, int h, double lambda)
{
	if (lambda <= 0.0) return;
	for (int i = 0; i < w * h; i++)
	{
		image[i] = poissonRandom(image[i] * lambda) / lambda;
	}
}

// Cuts the alphabet image into cells, one template per character
int makeTemplates(double *alphabet, int alphabetW, int alphabetH, Template *templates)
{
	int count = 0;
	int cellW, len;
	double mean;
	const char *c = ALPHABET_STRING;

	while (*c && count < MAX_GLYPHS)
	{
		len = utf8Length((unsigned char)*c);
		memcpy(templates[count].glyph, c, len);
		templates[count].glyph[len] = 0;
		c += len;
		count++;
	}

	if (alphabetW % count != 0)
	{
		printf("Alphabet image is %d px wide, that's not a multiple of %d characters\n", alphabetW, count);
		return 0;
	}
	cellW = alphabetW / count;

	for (int t = 0; t < count; t++)
	{
		templates[t].w = cellW;
		templates[t].h = alphabetH;
		templates[t].pixels = malloc(cellW * alphabetH * sizeof(double));
		mean = 0.0;
		for (int j = 0; j < alphabetH; j++)
		{
			for (int i = 0; i < cellW; i++)
			{
				*getDArrPointer(templates[t].pixels, cellW, i, j) = *getDArrPointer(alphabet, alphabetW, t * cellW + i, j);
				mean += *getDArrPointer(templates[t].pixels, cellW, i, j);
			}
		}
		mean /= cellW * alphabetH;
		templates[t].energy = 0.0;
		for (int i = 0; i < cellW * alphabetH; i++)
		{
			templates[t].pixels[i] -= mean;
			templates[t].energy += templates[t].pixels[i] * templates[t].pixels[i];
		}
	}
	return count;
}

// Integral image, (w+1) x (h+1), so that any window sum costs 4 lookups
void integralImage(double *image, int w, int h, double *integral, int squared)
{
	double f;
	for (int i = 0; i <= w; i++) integral[i] = 0.0;
	for (int j = 1; j <= h; j++)
	{
		integral[j * (w + 1)] = 0.0;
		for (int i = 1; i <= w; i++)
		{
			f = *getDArrPointer(image, w, i - 1, j - 1);
			if (squared) f *= f;
			integral[j * (w + 1) + i] = f + integral[(j - 1) * (w + 1) + i] + integral[j * (w + 1) + i - 1] - integral[(j - 1) * (w + 1) + i - 1];
		}
	}
}

double windowSum(double *integral, int w, int x, int y, int winW, int winH)
{
	return integral[(y + winH) * (w + 1) + x + winW] - integral[y * (w + 1) + x + winW] - integral[(y + winH) * (w + 1) + x] + integral[y * (w + 1) + x];
}

// The whole point of this exercise.
//
// Cross-correlation:  result(x, y) = sum_k sum_l t(k, l) * f(x + k, y + l)
// Convolution:        result(x, y) = sum_k sum_l t(k, l) * f(x - k, y - l)
//
// The only difference is the sign, but it means that convolution:
//   1. compares the image with the template rotated by 180 degrees,
//   2. looks at the cell to the top-left of (x, y), so its peaks land at the bottom-right corner of a letter.
// Both scores are normalized (Pearson correlation of the template and the image window), so 1 = perfect match.
// For every position only the best letter is remembered.
void matchAllTemplates(double *image, int w, int h, Template *templates, int templateCount, int mode, double *bestScore, int *bestGlyph)
{
	int tW = templates[0].w;
	int tH = templates[0].h;
	int N = tW * tH;
	int winX, winY;
	double *integral = malloc((w + 1) * (h + 1) * sizeof(double));
	double *integralSq = malloc((w + 1) * (h + 1) * sizeof(double));
	double *windowDev = malloc(w * h * sizeof(double)); // sqrt of the window's variance * N
	double s, sq, variance, sum, score;
	double *t;

	integralImage(image, w, h, integral, 0);
	integralImage(image, w, h, integralSq, 1);

	for (int j = 0; j < h; j++)
	{
		for (int i = 0; i < w; i++)
		{
			*getDArrPointer(bestScore, w, i, j) = -2.0;
			bestGlyph[j * w + i] = -1;
			*getDArrPointer(windowDev, w, i, j) = 0.0;

			winX = (mode == MODE_CORRELATION) ? (i) : (i - tW + 1);
			winY = (mode == MODE_CORRELATION) ? (j) : (j - tH + 1);
			if (winX < 0 || winY < 0 || winX + tW > w || winY + tH > h) continue;

			s = windowSum(integral, w, winX, winY, tW, tH);
			sq = windowSum(integralSq, w, winX, winY, tW, tH);
			variance = sq - s * s / N;
			*getDArrPointer(windowDev, w, i, j) = (variance > 1e-12) ? (sqrt(variance)) : (0.0);
		}
	}

	for (int g = 0; g < templateCount; g++)
	{
		if (templates[g].energy < 1e-12) continue; // empty cell, nothing to look for
		t = templates[g].pixels;
		printf("\rMatching templates: %d/%d", g + 1, templateCount);
		fflush(stdout);

		for (int j = 0; j < h; j++)
		{
			for (int i = 0; i < w; i++)
			{
				if (*getDArrPointer(windowDev, w, i, j) == 0.0) continue; // flat window or template doesn't fit

				sum = 0.0;
				if (mode == MODE_CORRELATION)
				{
					for (int l = 0; l < tH; l++)
					{
						for (int k = 0; k < tW; k++)
						{
							sum += t[l * tW + k] * *getDArrPointer(image, w, i + k, j + l);
						}
					}
				}
				else
				{
					for (int l = 0; l < tH; l++)
					{
						for (int k = 0; k < tW; k++)
						{
							sum += t[l * tW + k] * *getDArrPointer(image, w, i - k, j - l); // here it is, the whole difference
						}
					}
				}
				// sum(t' * f) == sum(t' * (f - mean f)) because sum(t') == 0
				score = sum / (sqrt(templates[g].energy) * *getDArrPointer(windowDev, w, i, j));

				if (score > *getDArrPointer(bestScore, w, i, j))
				{
					*getDArrPointer(bestScore, w, i, j) = score;
					bestGlyph[j * w + i] = g;
				}
			}
		}
	}
	printf("\n");

	free(integral);
	free(integralSq);
	free(windowDev);
}

// Local maxima of the best score, at most one per cell
int findLetters(double *bestScore, int *bestGlyph, int w, int h, int cellW, int cellH, int mode, double threshold, Detection *detections, int maxDetections)
{
	int count = 0;
	int isPeak, nx, ny;
	// a whole cell minus one pixel: letters in a monospace font are exactly one cell apart,
	// anything closer is a letter matched "half-way" between two real ones
	int radiusX = (cellW - 1 > 0) ? (cellW - 1) : (1);
	int radiusY = (cellH - 1 > 0) ? (cellH - 1) : (1);
	double score, neighbour;

	for (int j = 0; j < h; j++)
	{
		for (int i = 0; i < w; i++)
		{
			score = *getDArrPointer(bestScore, w, i, j);
			if (score < threshold) continue;

			isPeak = 1;
			for (int dy = -radiusY; dy <= radiusY && isPeak; dy++)
			{
				for (int dx = -radiusX; dx <= radiusX && isPeak; dx++)
				{
					nx = i + dx;
					ny = j + dy;
					if ((dx == 0 && dy == 0) || nx < 0 || ny < 0 || nx >= w || ny >= h) continue;
					neighbour = *getDArrPointer(bestScore, w, nx, ny);
					// on a plateau only the first pixel (in scan order) wins
					if (neighbour > score || (neighbour == score && (dy < 0 || (dy == 0 && dx < 0)))) isPeak = 0;
				}
			}
			if (!isPeak) continue;

			if (count >= maxDetections)
			{
				printf("Too many letters, threshold too low? Stopping at %d\n", maxDetections);
				return count;
			}
			// convolution "looks" at the cell to the top-left of the peak, see matchAllTemplates
			detections[count].x = (mode == MODE_CORRELATION) ? (i) : (i - cellW + 1);
			detections[count].y = (mode == MODE_CORRELATION) ? (j) : (j - cellH + 1);
			detections[count].glyph = bestGlyph[j * w + i];
			detections[count].score = score;
			count++;
		}
	}
	return count;
}

int compareByY(const void *a, const void *b)
{
	const Detection *da = a, *db = b;
	return (da->y != db->y) ? (da->y - db->y) : (da->x - db->x);
}

int compareByX(const void *a, const void *b)
{
	const Detection *da = a, *db = b;
	return da->x - db->x;
}

// Letters -> lines -> text. Gaps are turned into spaces and empty lines using the cell size (monospace!)
int assembleText(Detection *detections, int count, Template *templates, int cellW, int cellH, char *text, int maxText)
{
	int length = 0;
	int lineStart = 0, lineEnd;
	int minX, spaces, emptyLines;
	double lineY, previousLineY = 0.0;
	const char *piece;

	text[0] = 0;
	if (count == 0) return 0;

	qsort(detections, count, sizeof(Detection), compareByY);

	minX = detections[0].x;
	for (int i = 1; i < count; i++)
	{
		if (detections[i].x < minX) minX = detections[i].x;
	}

	while (lineStart < count)
	{
		// one line = letters whose y is within half a cell of the first one
		lineEnd = lineStart + 1;
		while (lineEnd < count && detections[lineEnd].y - detections[lineStart].y < cellH / 2) lineEnd++;
		qsort(detections + lineStart, lineEnd - lineStart, sizeof(Detection), compareByX);

		lineY = 0.0;
		for (int i = lineStart; i < lineEnd; i++) lineY += detections[i].y;
		lineY /= (lineEnd - lineStart);

		if (lineStart > 0)
		{
			emptyLines = (int)round((lineY - previousLineY) / cellH) - 1;
			if (emptyLines < 0) emptyLines = 0;
			for (int e = 0; e <= emptyLines; e++) // end of the previous line + the empty ones
			{
				if (length + 1 < maxText) text[length++] = '\n';
			}
		}
		previousLineY = lineY;

		for (int i = lineStart; i < lineEnd; i++)
		{
			if (i == lineStart) spaces = (int)round((double)(detections[i].x - minX) / cellW);
			else spaces = (int)round((double)(detections[i].x - detections[i - 1].x) / cellW) - 1;

			for (int s = 0; s < spaces; s++)
			{
				if (length + 1 < maxText) text[length++] = ' ';
			}
			piece = templates[detections[i].glyph].glyph;
			if (length + (int)strlen(piece) < maxText)
			{
				memcpy(text + length, piece, strlen(piece));
				length += strlen(piece);
			}
		}
		lineStart = lineEnd;
	}
	if (length + 1 < maxText) text[length++] = '\n';
	text[length] = 0;
	return length;
}

// UTF-8 -> code points, without the spaces at the ends of lines and empty lines at the end
int decodeNormalized(const char *text, unsigned int *codes, int maxCodes)
{
	int count = 0;
	int len;
	unsigned char c;
	unsigned int code;

	while (*text && count < maxCodes)
	{
		c = (unsigned char)*text;
		len = utf8Length(c);
		code = (len == 1) ? (c) : ((len == 2) ? (c & 0x1F) : ((len == 3) ? (c & 0x0F) : (c & 0x07)));
		for (int i = 1; i < len && text[i]; i++) code = (code << 6) | ((unsigned char)text[i] & 0x3F);
		text += len;

		if (code == '\r') continue;
		if (code == '\n')
		{
			while (count > 0 && codes[count - 1] == ' ') count--;
		}
		codes[count++] = code;
	}
	while (count > 0 && (codes[count - 1] == ' ' || codes[count - 1] == '\n')) count--;
	return count;
}

// Number of wrong characters = edit distance (substitutions, missing and extra letters)
int levenshtein(unsigned int *a, int lenA, unsigned int *b, int lenB)
{
	int *previous = malloc((lenB + 1) * sizeof(int));
	int *current = malloc((lenB + 1) * sizeof(int));
	int *swap, result, best;

	for (int j = 0; j <= lenB; j++) previous[j] = j;
	for (int i = 1; i <= lenA; i++)
	{
		current[0] = i;
		for (int j = 1; j <= lenB; j++)
		{
			best = previous[j - 1] + (a[i - 1] != b[j - 1]);
			if (previous[j] + 1 < best) best = previous[j] + 1;
			if (current[j - 1] + 1 < best) best = current[j - 1] + 1;
			current[j] = best;
		}
		swap = previous;
		previous = current;
		current = swap;
	}
	result = previous[lenB];
	free(previous);
	free(current);
	return result;
}

int main(int argc, char *argv[])
{
	char imageFilename[256] = {0};
	char alphabetFilename[256] = {0};
	char outputFilename[256] = {0};
	char referenceFilename[256] = {0};
	char *text = NULL;
	char *reference = NULL;
	unsigned int *codesText = NULL, *codesReference = NULL;

	int mode = MODE_CORRELATION;
	double lambda = 0.0;
	double threshold = DEFAULT_THRESHOLD;

	double *image = NULL, *alphabet = NULL, *bestScore = NULL;
	int *bestGlyph = NULL;
	int w, h, alphabetW, alphabetH;
	Template templates[MAX_GLYPHS];
	int templateCount;
	Detection *detections = NULL;
	int detectionCount, maxDetections, textLength;
	FILE *file;

	if (argc < 6 || argc > 8)
	{
		printf("Incorrect number of arguments: %d\n", argc);
		printf("Usage: %s <image> <korelacja|splot> <lambda> <alphabet image> <output.txt> [threshold] [reference.txt]\n", argv[0]);
		return 1;
	}

	strncpy(imageFilename, argv[1], 255);
	if (argv[2][0] == 'k' || argv[2][0] == 'K' || argv[2][0] == 'x' || argv[2][0] == 'X')
	{
		mode = MODE_CORRELATION;
	}
	else if (argv[2][0] == 's' || argv[2][0] == 'S' || argv[2][0] == 'c' || argv[2][0] == 'C')
	{
		mode = MODE_CONVOLUTION;
	}
	else
	{
		printf("Wrong method: '%s' (use korelacja or splot)\n", argv[2]);
		return 1;
	}
	lambda = atof(argv[3]);
	strncpy(alphabetFilename, argv[4], 255);
	strncpy(outputFilename, argv[5], 255);
	if (argc >= 7) threshold = atof(argv[6]);
	if (argc >= 8) strncpy(referenceFilename, argv[7], 255);

	srand(time(NULL));

	if ( SDL_Init( 0 ) < 0 ) { // no video needed, only SDL_image
		printf("Error initializing SDL: %s\n", SDL_GetError());
		return 1;
	}

	image = loadGrayImage(imageFilename, &w, &h);
	alphabet = loadGrayImage(alphabetFilename, &alphabetW, &alphabetH);
	if (!image || !alphabet)
	{
		SDL_Quit();
		return 1;
	}
	addShotNoise(image, w, h, lambda);

	templateCount = makeTemplates(alphabet, alphabetW, alphabetH, templates);
	if (templateCount == 0)
	{
		SDL_Quit();
		return 1;
	}
	if (templates[0].w > w || templates[0].h > h)
	{
		printf("Letter cell (%dx%d) is bigger than the image (%dx%d)\n", templates[0].w, templates[0].h, w, h);
		SDL_Quit();
		return 1;
	}
	printf("Image: %dx%d, %d characters, cell: %dx%d, method: %s, lambda: %g, threshold: %.2f\n",
		w, h, templateCount, templates[0].w, templates[0].h, (mode == MODE_CORRELATION) ? ("cross-correlation") : ("convolution"), lambda, threshold);

	bestScore = malloc(w * h * sizeof(double));
	bestGlyph = malloc(w * h * sizeof(int));
	matchAllTemplates(image, w, h, templates, templateCount, mode, bestScore, bestGlyph);

	maxDetections = (w / templates[0].w + 1) * (h / templates[0].h + 1) * 4;
	detections = malloc(maxDetections * sizeof(Detection));
	detectionCount = findLetters(bestScore, bestGlyph, w, h, templates[0].w, templates[0].h, mode, threshold, detections, maxDetections);

	text = malloc(MAX_TEXT);
	textLength = assembleText(detections, detectionCount, templates, templates[0].w, templates[0].h, text, MAX_TEXT);

	file = fopen(outputFilename, "w");
	if (!file)
	{
		printf("Error opening %s\n", outputFilename);
	}
	else
	{
		fwrite(text, 1, textLength, file);
		fclose(file);
	}
	printf("Found %d letters, text saved to %s:\n\n%s\n", detectionCount, outputFilename, text);

	// Quality measure from the exercise: number of wrongly recognized characters
	if (referenceFilename[0])
	{
		file = fopen(referenceFilename, "rb");
		if (!file)
		{
			printf("Error opening %s\n", referenceFilename);
		}
		else
		{
			reference = calloc(MAX_TEXT, 1);
			fread(reference, 1, MAX_TEXT - 1, file);
			fclose(file);

			codesText = malloc(MAX_TEXT * sizeof(unsigned int));
			codesReference = malloc(MAX_TEXT * sizeof(unsigned int));
			int lenText = decodeNormalized(text, codesText, MAX_TEXT);
			int lenReference = decodeNormalized(reference, codesReference, MAX_TEXT);
			int errors = levenshtein(codesText, lenText, codesReference, lenReference);
			printf("Wrong characters: %d of %d (%.1f%%)\n", errors, lenReference, (lenReference > 0) ? (100.0 * errors / lenReference) : (0.0));

			free(reference);
			free(codesText);
			free(codesReference);
		}
	}

	for (int t = 0; t < templateCount; t++) free(templates[t].pixels);
	free(image);
	free(alphabet);
	free(bestScore);
	free(bestGlyph);
	free(detections);
	free(text);
	SDL_Quit();

	return 0;
}
