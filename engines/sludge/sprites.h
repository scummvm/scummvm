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
#ifndef SLUDGE_SPRITE_H
#define SLUDGE_SPRITE_H

#if 0
#if !defined(HAVE_GLES2)
#include "GLee.h"
#else
#include <GLES2/gl2.h>
#endif
#endif

struct onScreenPerson;

struct sprite {
	int width, height, xhot, yhot;
	int tex_x;
	int texNum;
	//unsigned char * data;
};

class spritePalette {
public:
	unsigned short int *pal;
	unsigned char *r;
	unsigned char *g;
	unsigned char *b;
#if 0
	GLuint *tex_names;
	GLuint *burnTex_names;
#endif
	int *tex_w, * tex_h;
	int numTextures;
	unsigned char originalRed, originalGreen, originalBlue, total;

	spritePalette(): pal(0), r(0), g(0), b(0)/*, tex_names(0), burnTex_names(0)*/
		, tex_w(0), tex_h(0), numTextures(0)
		, total(0) {}

	~spritePalette() {
		delete [] pal;
		delete [] r;
		delete [] g;
		delete [] b;
//		delete [] tex_names;
//		delete [] burnTex_names;
		delete [] tex_w;
		delete [] tex_h;
	}
};

struct spriteBank {
	int total;
	int type;
	sprite *sprites;
	spritePalette myPalette;
	bool isFont;
};

void forgetSpriteBank(spriteBank &forgetme);
bool loadSpriteBank(char *filename, spriteBank &loadhere);
bool loadSpriteBank(int fileNum, spriteBank &loadhere, bool isFont);

void fontSprite(int x1, int y1, sprite &single, const spritePalette &fontPal);
void flipFontSprite(int x1, int y1, sprite &single, const spritePalette &fontPal);

bool scaleSprite(sprite &single, const spritePalette &fontPal, onScreenPerson *thisPerson, bool mirror);
void pasteSpriteToBackDrop(int x1, int y1, sprite &single, const spritePalette &fontPal);
bool reserveSpritePal(spritePalette &sP, int n);
void fixScaleSprite(int x1, int y1, sprite &single, const spritePalette &fontPal, onScreenPerson *thisPerson, const int camX, const int camY, bool);
void burnSpriteToBackDrop(int x1, int y1, sprite &single, const spritePalette &fontPal);

#endif
