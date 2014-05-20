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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#ifndef AVALANCHE_GRAPHICS_H
#define AVALANCHE_GRAPHICS_H

#include "avalanche/enums.h"

#include "common/file.h"
#include "common/rect.h"
#include "graphics/surface.h"

namespace Avalanche {
class AvalancheEngine;
class AnimationType;
struct SpriteType;
struct ChunkBlock;

typedef byte FontType[256][16];
typedef byte ManiType[2049];
typedef byte SilType[51][11]; // 35, 4

struct MouseHotspotType {
	int16 _horizontal, _vertical;
};

class GraphicManager {
public:
	static const MouseHotspotType kMouseHotSpots[9];
	Color _talkBackgroundColor, _talkFontColor;

	GraphicManager(AvalancheEngine *vm);
	~GraphicManager();
	void init();
	void loadDigits();
	void loadMouse(byte which);

	void drawRectangle(Common::Rect rect, Color color);
	void drawFilledRectangle(Common::Rect rect, Color color);
	void blackOutScreen();
	void drawDot(int x, int y, Color color);
	void drawLine(int x1, int y1, int x2, int y2, int penX, int penY, Color color);
	Common::Point drawScreenArc(int16 x, int16 y, int16 stAngle, int16 endAngle, uint16 radius, Color color);
	void drawPieSlice(int16 x, int16 y, int16 stAngle, int16 endAngle, uint16 radius, Color color);
	void drawTriangle(Common::Point *p, Color color);
	void drawNormalText(const Common::String text, FontType font, byte fontHeight, int16 x, int16 y, Color color);
	void drawScrollText(const Common::String text, FontType font, byte fontHeight, int16 x, int16 y, Color color);
	void drawDigit(int index, int x, int y);
	void drawDirection(int index, int x, int y);
	void drawScrollShadow(int16 x1, int16 y1, int16 x2, int16 y2);
	void drawShadowBox(int16 x1, int16 y1, int16 x2, int16 y2, Common::String text);
	void drawScroll(int mx, int lx, int my, int ly);
	void drawMenuBar(Color color);
	void drawSpeedBar(int speed);
	void drawBackgroundSprite(int16 x, int16 y, SpriteType &sprite);
	void drawMenuBlock(int x1, int y1, int x2, int y2, Color color);
	void drawMenuItem(int x1, int y1, int x2, int y2);
	void wipeChar(int x, int y, Color color);
	void drawChar(byte ander, int x, int y, Color color);
	void drawDebugLines();

	// For the mini-game "Nim".
	void nimLoad();
	void nimDrawStone(int x, int y);
	void nimDrawInitials();
	void nimDrawLogo();
	void nimFree();

	// Used in wobble()
	void shiftScreen();

	// Used in winning()
	void drawWinningPic();

	// Ghostroom's functions:
	void ghostDrawMonster(byte ***picture, uint16 destX, int16 destY, MonsterType type);
	Graphics::Surface ghostLoadPicture(Common::File &file, Common::Point &coord);
	void ghostDrawPicture(const Graphics::Surface &picture, uint16 destX, uint16 destY);
	void ghostDrawBackgroundItems(Common::File &file);

	// Help's function:
	void helpDrawButton(int y, byte which);
	void helpDrawHighlight(byte which, Color color);
	void helpDrawBigText(const Common::String text, int16 x, int16 y, Color color);

	// Shoot em' up's functions:
	void seuDrawTitle();
	void seuLoad();
	void seuFree();
	void seuDrawPicture(int x, int y, byte which);
	void seuDrawCameo(int destX, int destY, byte w1, byte w2);
	uint16 seuGetPicWidth(int which);
	uint16 seuGetPicHeight(int which);

	// Main Menu's functions:
	// The main menu uses a different screen height (350) from the game itself (200 * 2)
	// so it needs it's own graphic functions on that matter.
	void menuRefreshScreen();
	void menuInitialize();
	void menuFree();
	void menuRestoreScreen();
	void menuLoadPictures();
	void menuDrawBigText(FontType font, uint16 x, uint16 y, Common::String text, Color color);
	void menuDrawIndicator(int x);

	void clearAlso();
	void clearTextBar();
	void setAlsoLine(int x1, int y1, int x2, int y2, Color color);
	byte getAlsoColor(int x1, int y1, int x2, int y2);
	byte getScreenColor(Common::Point pos);

	// Further information about this: http://www.shikadi.net/moddingwiki/Raw_EGA_data
	Graphics::Surface loadPictureRaw(Common::File &file, uint16 width, uint16 height);

	void drawSprite(AnimationType *sprite, byte picnum, int16 x, int16 y);
	void drawThinkPic(Common::String filename, int id);
	void drawToolbar();
	void drawCursor(byte pos);
	void drawReadyLight(Color color);
	void drawSoundLight(bool state);
	void drawErrorLight(bool state);
	void drawSign(Common::String name, int16 xl, int16 yl, int16 y);
	void drawIcon(int16 x, int16 y, byte which);
	void drawScreenLine(int16 x, int16 y, int16 x2, int16 y2, Color color);
	void prepareBubble(int xc, int xw, int my, Common::Point points[3]);
	void refreshScreen();
	void loadBackground(Common::File &file);
	void refreshBackground();
	void setBackgroundColor(Color newColor);
	void setDialogColor(Color bg, Color text);

	void zoomOut(int16 x, int16 y);
	void showScroll();
	void getNaturalPicture(SpriteType &sprite);

	void saveScreen();
	void restoreScreen();
	void removeBackup();

private:
	static const int16 kMouseSize = 134;
	static const uint16 kBackgroundWidth = kScreenWidth;
	static const byte kEgaPaletteIndex[16];
	static const byte kBackgroundHeight = 8 * 12080 / kScreenWidth; // With 640 width it's 151.
	// The 8 = number of bits in a byte, and 12080 comes from Lucerna::load().
	static const uint16 kMenuScreenHeight = 350;

	Graphics::Surface _background;
	Graphics::Surface _backup;
	Graphics::Surface _digits[10]; // digitsize and rwlitesize are defined in loadDigits() !!!
	Graphics::Surface _directions[9]; // Maybe it will be needed to move them to the class itself instead.
	Graphics::Surface _magics; // Lucerna::draw_also_lines() draws the "magical" lines here. Further information: https://github.com/urukgit/avalot/wiki/Also
	Graphics::Surface _screen; // Only used in refreshScreen() to make it more optimized. (No recreation of it at every call of the function.)
	Graphics::Surface _scrolls;
	Graphics::Surface _surface;
	Graphics::Surface _menu;

	// For the mini-game "Nim".
	Graphics::Surface _nimStone;
	Graphics::Surface _nimInitials[3];
	Graphics::Surface _nimLogo;

	// For the mini-game "Shoot em' up".
	Graphics::Surface _seuPictures[99];

	byte _egaPalette[64][3];

	AvalancheEngine *_vm;

	void skipDifference(int size, const Graphics::Surface &picture, Common::File &file);

	// Further information about these two: http://www.shikadi.net/moddingwiki/Raw_EGA_data
	Graphics::Surface loadPictureGraphic(Common::File &file); // Reads Graphic-planar EGA data.
	Graphics::Surface loadPictureSign(Common::File &file, uint16 width, uint16 height); // Reads a tricky type of picture used for the "game over"/"about" scrolls and in the mini-game Nim.

	void drawText(Graphics::Surface &surface, const Common::String text, FontType font, byte fontHeight, int16 x, int16 y, Color color);
	void drawBigText(Graphics::Surface &surface, const Common::String text, FontType font, byte fontHeight, int16 x, int16 y, Color color);
	void drawPicture(Graphics::Surface &target, const Graphics::Surface picture, uint16 destX, uint16 destY);

	// Taken from Free Pascal's Procedure InternalEllipseDefault. Used to replace Pascal's procedure arc.
	// Returns the end point of the arc. (Needed in Clock.)
	Common::Point drawArc(Graphics::Surface &surface, int16 x, int16 y, int16 stAngle, int16 endAngle, uint16 radius, Color color);
};

} // End of namespace Avalanche

#endif // AVALANCHE_GRAPHICS_H
