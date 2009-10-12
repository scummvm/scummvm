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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_GUI_SCREEN_H
#define SCI_GUI_SCREEN_H

#include "sci/sci.h"
#include "sci/gui/gui_helpers.h"

namespace Sci {

#define SCI_SCREEN_MAXHEIGHT 400

#define SCI_SCREEN_MASK_VISUAL   1
#define SCI_SCREEN_MASK_PRIORITY 2
#define SCI_SCREEN_MASK_CONTROL  4
#define SCI_SCREEN_MASK_ALL      SCI_SCREEN_MASK_VISUAL|SCI_SCREEN_MASK_PRIORITY|SCI_SCREEN_MASK_CONTROL

#define SCI_SCREEN_UNDITHERMEMORIAL_SIZE 256

class SciGuiScreen {
public:
	SciGuiScreen(int16 width = 320, int16 height = 200, int16 scaleFactor = 1);
	~SciGuiScreen();

	void copyToScreen();
	void copyRectToScreen(const Common::Rect &rect);

	byte getDrawingMask(byte color, byte prio, byte control);
	void putPixel(int x, int y, byte drawMask, byte color, byte prio, byte control);
	void drawLine(Common::Point startPoint, Common::Point endPoint, byte color, byte prio, byte control);
	void drawLine(int16 left, int16 top, int16 right, int16 bottom, byte color, byte prio, byte control) {
		drawLine(Common::Point(left, top), Common::Point(right, bottom), color, prio, control);
	}
	byte getVisual(int x, int y);
	byte getPriority(int x, int y);
	byte getControl(int x, int y);
	byte isFillMatch(int16 x, int16 y, byte drawMask, byte t_color, byte t_pri, byte t_con);

	int getBitsDataSize(Common::Rect rect, byte mask);
	void saveBits(Common::Rect rect, byte mask, byte *memoryPtr);
	void restoreBits(byte *memoryPtr);

	void setPalette(GuiPalette*pal);

	void setVerticalShakePos(uint16 shakePos);

	void dither(bool addToFlag);
	void unditherSetState(bool flag);
	int16 *unditherGetMemorial();

	void debugShowMap(int mapNo);

	uint16 _width;
	uint16 _height;
	uint _pixels;
	uint16 _displayWidth;
	uint16 _displayHeight;
	uint _displayPixels;

	int _picNotValid; // possible values 0, 1 and 2

private:
	void restoreBitsScreen(Common::Rect rect, byte *&memoryPtr, byte *screen);
	void saveBitsScreen(Common::Rect rect, byte *screen, byte *&memoryPtr);

	uint16 _baseTable[SCI_SCREEN_MAXHEIGHT];
	uint16 _baseDisplayTable[SCI_SCREEN_MAXHEIGHT];

	bool _unditherState;
	int16 _unditherMemorial[SCI_SCREEN_UNDITHERMEMORIAL_SIZE];

public:	// HACK. TODO: make private
	// these screens have the real resolution of the game engine (320x200 for SCI0/SCI1/SCI11 games, 640x480 for SCI2 games)
	//  SCI0 games will be dithered in here at any time
	byte *_visualScreen;
	byte *_priorityScreen;
	byte *_controlScreen;

	// this screen is the one that is actually displayed to the user. It may be 640x480 for japanese SCI1 games
	//  SCI0 games may be undithered in here. Only read from this buffer for Save/ShowBits usage.
	byte *_displayScreen;
private:

	// this is a pointer to the currently active screen (changing it only required for debug purposes)
	byte *_activeScreen;
};

} // End of namespace Sci

#endif
