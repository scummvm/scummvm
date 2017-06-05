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
#include "platform-dependent.h"

#include <stdint.h>
#include <string.h>

#include "CommonCode/utf8.h"

#include "debug.h"
#include "allfiles.h"
#include "stringy.h"

#include "sprites.h"
#include "colours.h"
#include "fonttext.h"
#include "newfatal.h"
#include "moreio.h"

namespace Sludge {

spriteBank theFont;
int fontHeight = 0, numFontColours, loadedFontNum;
char *fontOrderString = NULL;
short fontSpace = -1;

uint32_t *fontTable = NULL;
unsigned int fontTableSize = 0;

#define fontInTable(x) ((x<fontTableSize) ? fontTable[(uint32_t) x] : 0)

extern float cameraZoom;

bool isInFont(char *theText) {
	if (!fontTableSize)
		return 0;
	if (!theText[0])
		return 0;

	// We don't want to compare strings. Only single characters allowed!
	if (u8_strlen(theText) > 1)
		return false;

	int i = 0;
	uint32_t c = u8_nextchar(theText, &i);

	return u8_strchr(fontOrderString, c, &i);
}

int stringLength(char *theText) {
	return u8_strlen(theText);
}

int stringWidth(char *theText) {
	int a = 0;
	uint32_t c;
	int xOff = 0;

	if (!fontTableSize)
		return 0;

	while (theText[a]) {
		c = u8_nextchar(theText, &a);
		xOff += theFont.sprites[fontInTable(c)].surface.w + fontSpace;
	}

	return xOff;
}

void pasteString(char *theText, int xOff, int y, spritePalette &thePal) {
	sprite *mySprite;
	int a = 0;
	uint32_t c;

	if (!fontTableSize)
		return;

	xOff += (int)((float)(fontSpace >> 1) / cameraZoom);
	while (theText[a]) {
		c = u8_nextchar(theText, &a);
		mySprite = &theFont.sprites[fontInTable(c)];
		fontSprite(xOff, y, *mySprite, thePal);
		xOff += (int)((double)(mySprite->surface.w + fontSpace) / cameraZoom);
	}
}

void pasteStringToBackdrop(char *theText, int xOff, int y, spritePalette &thePal) {
	sprite *mySprite;
	int a = 0;
	uint32_t c;

	if (!fontTableSize)
		return;

	xOff += fontSpace >> 1;
	while (theText[a]) {
		c = u8_nextchar(theText, &a);
		mySprite = &theFont.sprites[fontInTable(c)];
		pasteSpriteToBackDrop(xOff, y, *mySprite, thePal);
		xOff += mySprite->surface.w + fontSpace;
	}
}

void burnStringToBackdrop(char *theText, int xOff, int y, spritePalette &thePal) {
	sprite *mySprite;
	int a = 0;
	uint32_t c;

	if (!fontTableSize)
		return;

	xOff += fontSpace >> 1;
	while (theText[a]) {
		c = u8_nextchar(theText, &a);
		mySprite = &theFont.sprites[fontInTable(c)];
		burnSpriteToBackDrop(xOff, y, *mySprite, thePal);
		xOff += mySprite->surface.w + fontSpace;
	}
}

void fixFont(spritePalette &spal) {
#if 0
	delete [] spal.tex_names;
	delete [] spal.burnTex_names;
	delete [] spal.tex_h;
	delete [] spal.tex_w;

	spal.numTextures = theFont.myPalette.numTextures;

	spal.tex_names = new GLuint [spal.numTextures];
	if (! checkNew(spal.tex_names)) return;
	spal.burnTex_names = new GLuint [spal.numTextures];
	if (! checkNew(spal.burnTex_names)) return;
	spal.tex_w = new int [spal.numTextures];
	if (! checkNew(spal.tex_w)) return;
	spal.tex_h = new int [spal.numTextures];
	if (! checkNew(spal.tex_h)) return;

	for (int i = 0; i < theFont.myPalette.numTextures; i++) {
		spal.tex_names[i] = theFont.myPalette.tex_names[i];
		spal.burnTex_names[i] = theFont.myPalette.burnTex_names[i];
		spal.tex_w[i] = theFont.myPalette.tex_w[i];
		spal.tex_h[i] = theFont.myPalette.tex_h[i];
	}
#endif
}

void setFontColour(spritePalette &sP, byte r, byte g, byte b) {
	sP.originalRed = r;
	sP.originalGreen = g;
	sP.originalBlue = b;
}

bool loadFont(int filenum, const char *charOrder, int h) {
	int a = 0;
	uint32_t c;

	delete[] fontOrderString;
	fontOrderString = copyString(charOrder);

	forgetSpriteBank(theFont);

	loadedFontNum = filenum;

	fontTableSize = 0;
	while (charOrder[a]) {
		c = u8_nextchar(charOrder, &a);
		if (c > fontTableSize)
			fontTableSize = c;
	}
	fontTableSize++;

	delete[] fontTable;
	fontTable = new uint32_t[fontTableSize];
	if (!checkNew(fontTable))
		return false;

	for (a = 0; a < fontTableSize; a++) {
		fontTable[a] = 0;
	}
	a = 0;
	int i = 0;
	while (charOrder[a]) {
		c = u8_nextchar(charOrder, &a);
		fontTable[c] = i;
		i++;
	}

	if (!loadSpriteBank(filenum, theFont, true)) {
		fatal("Can't load font");
		return false;
	}

	numFontColours = theFont.myPalette.total;
	fontHeight = h;
	return true;
}

} // End of namespace Sludge
