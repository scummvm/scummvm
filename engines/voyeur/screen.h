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

#ifndef VOYEUR_GRAPHICS_H
#define VOYEUR_GRAPHICS_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "graphics/screen.h"

namespace Voyeur {

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

class VoyeurEngine;
class Screen;
class DisplayResource;
class PictureResource;
class ViewPortResource;
class ViewPortListResource;
class FontResource;
class FontInfoResource;
class CMapResource;

class DrawInfo {
public:
	int _penColor;
	Common::Point _pos;
public:
	DrawInfo(int penColor, const Common::Point &pos);
};

typedef void (Screen::*ScreenMethodPtr)();
typedef void (Screen::*ViewPortSetupPtr)(ViewPortResource *);
typedef void (Screen::*ViewPortAddPtr)(ViewPortResource *, int idx, const Common::Rect &bounds);
typedef void (Screen::*ViewPortRestorePtr)(ViewPortResource *);

class Screen: public Graphics::Screen {
public:
	byte _VGAColors[PALETTE_SIZE];
	PictureResource *_backgroundPage;
	int _SVGAMode;
	ViewPortListResource *_viewPortListPtr;
	ViewPortResource *_vPort;
	bool _saveBack;
	Common::Rect *_clipPtr;
	uint _planeSelect;
	CMapResource *_backColors;
	FontInfoResource *_fontPtr;
	PictureResource *_fontChar;
	DrawInfo *_drawPtr;
	DrawInfo _defaultDrawInfo;
private:
	VoyeurEngine *_vm;

	void restoreBack(Common::Array<Common::Rect> &rectList, int rectListCount,
		PictureResource *srcPic, PictureResource *destPic);
public:
	Screen(VoyeurEngine *vm);
	~Screen() override;

	void sInitGraphics();

	void setupMCGASaveRect(ViewPortResource *viewPort);
	void addRectOptSaveRect(ViewPortResource *viewPort, int idx, const Common::Rect &bounds);
	void restoreMCGASaveRect(ViewPortResource *viewPort);
	void addRectNoSaveBack(ViewPortResource *viewPort, int idx, const Common::Rect &bounds);

	void sDrawPic(DisplayResource *srcDisplay, DisplayResource *destDisplay, const Common::Point &initialOffset);
	void fillPic(DisplayResource *display, byte onOff);
	void sDisplayPic(PictureResource *pic);
	void drawANumber(DisplayResource *display, int num, const Common::Point &pt);
	void flipPage();
	void setPalette(const byte *palette, int start, int count);
	void setPalette128(const byte *palette, int start, int count);
	void resetPalette();
	void setColor(int idx, byte r, byte g, byte b);
	void setOneColor(int idx, byte r, byte g, byte b);
	void setColors(int start, int count, const byte *pal);
	void screenReset();
	void fadeDownICF1(int steps);
	void fadeUpICF1(int steps = 0);
	void fadeDownICF(int steps);
	void drawDot();

	/**
	 * Synchronizes the game data
	 */
	void synchronize(Common::Serializer &s);
};

} // End of namespace Voyeur

#endif /* VOYEUR_GRAPHICS_H */
