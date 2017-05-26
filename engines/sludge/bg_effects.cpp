/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "allfiles.h"
#include "backdrop.h"
#include "colours.h"
#include "debug.h"
#include "graphics.h"
#include "newfatal.h"
#include "moreio.h"

namespace Sludge {

#if 0
//extern unsigned short int * * backDropImage;
extern GLuint backdropTextureName;
#endif

#if 0
// Raised
static int s_matrixEffectDivide = 2;
static int s_matrixEffectWidth = 3;
static int s_matrixEffectHeight = 3;
static int s_matrixEffectData[9] = {0, 0, 0, 0, -1, 0, 0, 0, 2};
static int s_matrixEffectBase = 0;
#elif 0
// Stay put
static int s_matrixEffectDivide = 1;
static int s_matrixEffectWidth = 3;
static int s_matrixEffectHeight = 3;
static int s_matrixEffectData[9] = {0, 0, 0, 0, 1, 0, 0, 0, 0};
static int s_matrixEffectBase = 0;
#elif 0
// Brighten
static int s_matrixEffectDivide = 9;
static int s_matrixEffectWidth = 1;
static int s_matrixEffectHeight = 1;
static int s_matrixEffectData[9] = {10};
static int s_matrixEffectBase = 15;
#elif 0
// Raised up/left
static int s_matrixEffectDivide = 4;
static int s_matrixEffectWidth = 3;
static int s_matrixEffectHeight = 3;
static int s_matrixEffectData[9] = { -2, -1, 0, -1, 1, 1, 0, 1, 2};
static int s_matrixEffectBase = 16;
#elif 0
// Standard emboss
static int s_matrixEffectDivide = 2;
static int s_matrixEffectWidth = 3;
static int s_matrixEffectHeight = 3;
static int s_matrixEffectData[9] = { -1, 0, 0, 0, 0, 0, 0, 0, 1};
static int s_matrixEffectBase = 128;
#elif 0
// Horizontal blur
static int s_matrixEffectDivide = 11;
static int s_matrixEffectWidth = 5;
static int s_matrixEffectHeight = 1;
static int s_matrixEffectData[9] = {1, 3, 3, 3, 1};
static int s_matrixEffectBase = 0;
#elif 0
// Double vision
static int s_matrixEffectDivide = 6;
static int s_matrixEffectWidth = 13;
static int s_matrixEffectHeight = 2;
static int s_matrixEffectData[26] = {2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3};
static int s_matrixEffectBase = 0;
#elif 0
// Negative
static int s_matrixEffectDivide = 1;
static int s_matrixEffectWidth = 1;
static int s_matrixEffectHeight = 1;
static int s_matrixEffectData[9] = { -1};
static int s_matrixEffectBase = 255;
#elif 0
// Fog
static int s_matrixEffectDivide = 4;
static int s_matrixEffectWidth = 1;
static int s_matrixEffectHeight = 1;
static int s_matrixEffectData[9] = {3};
static int s_matrixEffectBase = 45;
#elif 0
// Blur
static int s_matrixEffectDivide = 14;
static int s_matrixEffectWidth = 3;
static int s_matrixEffectHeight = 3;
static int s_matrixEffectData[9] = {1, 2, 1, 2, 2, 2, 1, 2, 1};
static int s_matrixEffectBase = 0;
#else
static int s_matrixEffectDivide = 0;
static int s_matrixEffectWidth = 0;
static int s_matrixEffectHeight = 0;
static int *s_matrixEffectData = NULL;
static int s_matrixEffectBase = 0;
#endif

#if ALLOW_FILE
void blur_saveSettings(FILE *fp) {
	if (s_matrixEffectData) {
		put4bytes(s_matrixEffectDivide, fp);
		put4bytes(s_matrixEffectWidth, fp);
		put4bytes(s_matrixEffectHeight, fp);
		put4bytes(s_matrixEffectBase, fp);
		fwrite(s_matrixEffectData, sizeof(int), s_matrixEffectWidth * s_matrixEffectHeight, fp);
	} else {
		put4bytes(0, fp);
		put4bytes(0, fp);
		put4bytes(0, fp);
		put4bytes(0, fp);
	}
}
#endif

static int *blur_allocateMemoryForEffect() {
	free(s_matrixEffectData);
	s_matrixEffectData = NULL;

	if (s_matrixEffectWidth && s_matrixEffectHeight) {
		s_matrixEffectData = (int *) malloc(sizeof(int) * s_matrixEffectHeight * s_matrixEffectWidth);
		checkNew(s_matrixEffectData);
	}
	return s_matrixEffectData;
}

#if ALLOW_FILE
void blur_loadSettings(FILE *fp) {
	s_matrixEffectDivide = get4bytes(fp);
	s_matrixEffectWidth = get4bytes(fp);
	s_matrixEffectHeight = get4bytes(fp);
	s_matrixEffectBase = get4bytes(fp);

	if (blur_allocateMemoryForEffect()) {
		size_t bytes_read = fread(s_matrixEffectData, sizeof(int), s_matrixEffectWidth * s_matrixEffectHeight, fp);
		if (bytes_read != sizeof(int) * s_matrixEffectWidth * s_matrixEffectHeight && ferror(fp)) {
			debugOut("Reading error in blur_loadSettings.\n");
		}
	} else {
		fseek(fp, sizeof(int) * s_matrixEffectWidth * s_matrixEffectHeight, SEEK_CUR);
	}
}
#endif

bool blur_createSettings(int numParams, variableStack *&stack) {
	bool createNullThing = true;
	const char *error = NULL;

	if (numParams >= 3) {
		// PARAMETERS: base, divide, stack (, stack (, stack...))

		int height = numParams - 2;
		int width = 0;

		variableStack *justToCheckSizes = stack;
		for (int a = 0; a < height; a ++) {
			if (justToCheckSizes->thisVar.varType != SVT_STACK) {
				error = "Third and subsequent parameters in setBackgroundEffect should be arrays";
				break;
			} else {
				int w = stackSize(justToCheckSizes->thisVar.varData.theStack);
				if (a) {
					if (w != width) {
						error = "Arrays in setBackgroundEffect must be the same size";
						break;
					}
					if (w < width) {
						width = w;
					}
				} else {
					width = w;
				}
			}
		}

		if (width == 0 && ! error) {
			error = "Empty arrays found in setBackgroundEffect parameters";
		}

		if (! error) {
			s_matrixEffectWidth = width;
			s_matrixEffectHeight = height;

			if (blur_allocateMemoryForEffect()) {
				for (int y = height - 1; y >= 0; y --) {
					variableStack *eachNumber = stack->thisVar.varData.theStack->first;
					if (! error) {
						for (int x = 0; x < width; x ++) {
							int arraySlot = x + (y * width);
//							s_matrixEffectData[arraySlot] = (rand() % 4);
							if (!getValueType(s_matrixEffectData[arraySlot], SVT_INT, eachNumber->thisVar)) {
								error = "";
								break;
							}
							eachNumber = eachNumber->next;
						}
						trimStack(stack);
					}
				}
				if (! error && !getValueType(s_matrixEffectDivide, SVT_INT, stack -> thisVar))
					error = "";
				trimStack(stack);
				if (! error && !getValueType(s_matrixEffectBase, SVT_INT, stack -> thisVar))
					error = "";
				trimStack(stack);
				if (! error) {
					if (s_matrixEffectDivide) {
						createNullThing = false;
					} else {
						error = "Second parameter of setBackgroundEffect (the 'divide' value) should not be 0!";
					}
				}
			} else {
				error = "Couldn't allocate memory for effect";
			}
		}
	} else {
		if (numParams) {
			error = "setBackgroundEffect should either have 0 parameters or more than 2";
		}
	}

	if (createNullThing) {
		s_matrixEffectDivide = 0;
		s_matrixEffectWidth = 0;
		s_matrixEffectHeight = 0;
		s_matrixEffectBase = 0;
		delete s_matrixEffectData;
		s_matrixEffectData = NULL;
	}

	if (error && error[0]) {
		fatal(error);
	}

	return ! createNullThing;
}

static inline int clampi(int i, int min, int max) {
	return (i >= max) ? max : ((i <= min) ? min : i);
}

static inline void blur_createSourceLine(unsigned char *createLine, unsigned char *fromLine, int overlapOnLeft, int width) {
	int miniX;
	memcpy(createLine + overlapOnLeft * 4, fromLine, width * 4);

	for (miniX = 0; miniX < overlapOnLeft; miniX ++) {
		createLine[miniX * 4] = fromLine[0];
		createLine[miniX * 4 + 1] = fromLine[1];
		createLine[miniX * 4 + 2] = fromLine[2];
	}

	for (miniX = width + overlapOnLeft; miniX < width + s_matrixEffectWidth - 1; miniX ++) {
		createLine[miniX * 4] = fromLine[width * 4 - 4];
		createLine[miniX * 4 + 1] = fromLine[width * 4 - 3];
		createLine[miniX * 4 + 2] = fromLine[width * 4 - 2];
	}
}

bool blurScreen() {
#if 0
	if (s_matrixEffectWidth && s_matrixEffectHeight && s_matrixEffectDivide && s_matrixEffectData) {
		unsigned char *thisLine;
		int y, x;
		bool ok = true;
		int overlapOnLeft = s_matrixEffectWidth / 2;
		int overlapAbove  = s_matrixEffectHeight / 2;

		unsigned char **sourceLine = new unsigned char *[s_matrixEffectHeight];
		if (! checkNew(sourceLine)) return false;

		int picWidth = sceneWidth;
		int picHeight = sceneHeight;

		if (! NPOT_textures) {
			picWidth = getNextPOT(sceneWidth);
			picHeight = getNextPOT(sceneHeight);
		}

		// Retrieve the texture
		saveTexture(backdropTextureName, backdropTexture);

		for (y = 0; y < s_matrixEffectHeight; y ++) {
			sourceLine[y] = new unsigned char[(s_matrixEffectWidth - 1 + picWidth) * 4];
			ok &= (sourceLine[y] != NULL);
		}

		if (ok) {
			for (y = 0; y < s_matrixEffectHeight; y ++) {
				int miniY = clampi(y - overlapAbove - 1, 0, sceneHeight - 1);

				blur_createSourceLine(sourceLine[y], backdropTexture + miniY * picWidth * 4, overlapOnLeft, picWidth);
			}

			for (y = 0; y < sceneHeight; y ++) {
				thisLine = backdropTexture + y * picWidth * 4;

				//-------------------------
				// Scroll source lines
				//-------------------------
				unsigned char *tempLine = sourceLine[0];
				for (int miniY = 0; miniY < s_matrixEffectHeight - 1; miniY ++) {
					sourceLine[miniY] = sourceLine[miniY + 1];
				}
				sourceLine[s_matrixEffectHeight - 1] = tempLine;
				{
					int h = s_matrixEffectHeight - 1;
					int miniY = clampi(y + (s_matrixEffectHeight - overlapAbove - 1), 0, sceneHeight - 1);

					blur_createSourceLine(sourceLine[h], backdropTexture + miniY * picWidth * 4, overlapOnLeft, picWidth);
				}
				for (x = 0; x < sceneWidth; x ++) {
					int totalRed = 0;
					int totalGreen = 0;
					int totalBlue = 0;
					int *matrixElement = s_matrixEffectData;
					for (int miniY = 0; miniY < s_matrixEffectHeight; ++ miniY) {
						unsigned char *pixel = & sourceLine[miniY][x * 4];
						for (int miniX = 0; miniX < s_matrixEffectWidth; ++ miniX) {

							totalRed    += pixel[0]     * * matrixElement;
							totalGreen  += pixel[1]     * * matrixElement;
							totalBlue   += pixel[2]     * * matrixElement;
							++ matrixElement;
							pixel += 4;
						}
					}
					totalRed = (totalRed + s_matrixEffectDivide / 2) / s_matrixEffectDivide + s_matrixEffectBase;
					totalRed = (totalRed < 0) ? 0 : ((totalRed > 255) ? 255 : totalRed);

					totalGreen = (totalGreen + s_matrixEffectDivide / 2) / s_matrixEffectDivide + s_matrixEffectBase;
					totalGreen = (totalGreen < 0) ? 0 : ((totalGreen > 255) ? 255 : totalGreen);

					totalBlue = (totalBlue + s_matrixEffectDivide / 2) / s_matrixEffectDivide + s_matrixEffectBase;
					totalBlue = (totalBlue < 0) ? 0 : ((totalBlue > 255) ? 255 : totalBlue);

					* thisLine = totalRed;
					++ thisLine;
					* thisLine = totalGreen;
					++ thisLine;
					* thisLine = totalBlue;
					++ thisLine;
//					* thisLine = totalAlpha;
					++ thisLine;
				}
			}
		}

		for (y = 0; y < s_matrixEffectHeight; y ++) {
			delete sourceLine[y];
		}
		delete sourceLine;
		sourceLine = NULL;

		texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, backdropTexture, backdropTextureName);
		return true;
	}
#endif
	return false;
}

} // End of namespace Sludge
