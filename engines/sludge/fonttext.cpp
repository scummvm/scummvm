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

#include "sludge/allfiles.h"
#include "sludge/stringy.h"
#include "sludge/sprites.h"
#include "sludge/fonttext.h"
#include "sludge/newfatal.h"
#include "sludge/moreio.h"
#include "sludge/utf8.h"

namespace Sludge {

spriteBank theFont;
int fontHeight = 0, numFontColours, loadedFontNum;
UTF8Converter fontOrder;
int16 fontSpace = -1;

uint32 *fontTable = NULL;
uint fontTableSize = 0;

#define fontInTable(x) ((x<fontTableSize) ? fontTable[(uint32) x] : 0)

extern float cameraZoom;

bool isInFont(char *theText) {
	if (!fontTableSize)
		return 0;
	if (!theText[0])
		return 0;

	Common::U32String str32 = UTF8Converter::convertUtf8ToUtf32(theText);

	// We don't want to compare strings. Only single characters allowed!
	if (str32.size() > 1)
		return false;

	uint32 c = str32[0];

	// check if font order contains the utf8 char
	return fontOrder.getU32String().contains(c);
}

int stringLength(char *theText) {
	Common::U32String str32 = UTF8Converter::convertUtf8ToUtf32(theText);
	return str32.size();
}

int stringWidth(char *theText) {
	int xOff = 0;

	if (!fontTableSize)
		return 0;

	Common::U32String str32 = UTF8Converter::convertUtf8ToUtf32(theText);

	for (uint i = 0; i < str32.size(); ++i) {
		uint32 c = str32[i];
		xOff += theFont.sprites[fontInTable(c)].surface.w + fontSpace;
	}

	return xOff;
}

void pasteString(char *theText, int xOff, int y, spritePalette &thePal) {
	if (!fontTableSize)
		return;

	xOff += (int)((float)(fontSpace >> 1) / cameraZoom);

	Common::U32String str32 = UTF8Converter::convertUtf8ToUtf32(theText);

	for (uint32 i = 0; i < str32.size(); ++i) {
		uint32 c = str32[i];
		sprite *mySprite = &theFont.sprites[fontInTable(c)];
		fontSprite(xOff, y, *mySprite, thePal);
		xOff += (int)((double)(mySprite->surface.w + fontSpace) / cameraZoom);
	}
}

void pasteStringToBackdrop(char *theText, int xOff, int y, spritePalette &thePal) {
	if (!fontTableSize)
		return;

	Common::U32String str32 = UTF8Converter::convertUtf8ToUtf32(theText);

	xOff += fontSpace >> 1;
	for (uint32 i = 0; i < str32.size(); ++i) {
		uint32 c = str32[i];
		sprite *mySprite = &theFont.sprites[fontInTable(c)];
		pasteSpriteToBackDrop(xOff, y, *mySprite, thePal);
		xOff += mySprite->surface.w + fontSpace;
	}
}

void burnStringToBackdrop(char *theText, int xOff, int y, spritePalette &thePal) {
	if (!fontTableSize)
		return;

	Common::U32String str32 = UTF8Converter::convertUtf8ToUtf32(theText);

	xOff += fontSpace >> 1;
	for (uint i = 0; i < str32.size(); ++i) {
		uint32 c = str32[i];
		sprite *mySprite = &theFont.sprites[fontInTable(c)];
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
	fontOrder.setUTF8String(charOrder);

	forgetSpriteBank(theFont);

	loadedFontNum = filenum;

	// get max value among all utf8 chars
	Common::U32String fontOrderString = fontOrder.getU32String();
	fontTableSize = 0;
	for (uint32 i = 0; i < fontOrderString.size(); ++i) {
		uint32 c = fontOrderString[i];
		if (c > fontTableSize)
			fontTableSize = c;
	}
	fontTableSize++;

	// create an index table from utf8 char to the index
	delete[] fontTable;
	fontTable = new uint32[fontTableSize];
	if (!checkNew(fontTable))
		return false;

	for (uint i = 0; i < fontTableSize; i++) {
		fontTable[i] = 0;
	}

	for (uint i = 0; i < fontOrderString.size(); ++i) {
		uint32 c = fontOrderString[i];
		fontTable[c] = i;
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
