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

#ifndef SLUDGE_BACKDROP_H
#define SLUDGE_BACKDROP_H

#include "sludge/graphics.h"
#include "sludge/variable.h"

namespace Sludge {

enum {
	LIGHTMAPMODE_NONE = -1,
	LIGHTMAPMODE_HOTSPOT,
	LIGHTMAPMODE_PIXEL,
	LIGHTMAPMODE_NUM
};

extern unsigned int winWidth, winHeight, sceneWidth, sceneHeight;
extern int lightMapMode;

struct parallaxLayer {
	Graphics::Surface surface;
	int width, height, speedX, speedY;
	bool wrapS, wrapT;
	unsigned short fileNum, fractionX, fractionY;
	int cameraX, cameraY;
	parallaxLayer *next;
	parallaxLayer *prev;
};

void killAllBackDrop();
bool resizeBackdrop(int x, int y);
bool killResizeBackdrop(int x, int y);
void killBackDrop();
void loadBackDrop(int fileNum, int x, int y);
void mixBackDrop(int fileNum, int x, int y);
void drawBackDrop();
void blankScreen(int x1, int y1, int x2, int y2);
void darkScreen();
void saveHSI(Common::WriteStream *stream);
#if 0
void saveCoreHSI(Common::WriteStream *stream, GLuint texture, int w, int h);
#endif
bool loadHSI(Common::SeekableReadStream *stream, int, int, bool);
bool mixHSI(Common::SeekableReadStream *stream, int x = 0, int y = 0);
void drawHorizontalLine(unsigned int, unsigned int, unsigned int);
void drawVerticalLine(unsigned int, unsigned int, unsigned int);
void hardScroll(int distance);
bool getRGBIntoStack(unsigned int x, unsigned int y, stackHandler *sH);

// Also the light map stuff

void killLightMap();
bool loadLightMap(int v);

extern texture lightMap;

// And background parallax scrolling

void killParallax();
bool loadParallax(unsigned short v, unsigned short fracX, unsigned short fracY);
void saveParallaxRecursive(parallaxLayer *me, Common::WriteStream *fp);
void reloadParallaxTextures();

void nosnapshot();
bool snapshot();
void saveSnapshot(Common::WriteStream *stream);
bool restoreSnapshot(Common::SeekableReadStream *stream);

} // End of namespace Sludge

#endif
