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
#include "graphics/surface.h"

namespace Voyeur {

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define PALETTE_COUNT 256
#define PALETTE_SIZE (256 * 3)

class VoyeurEngine;
class GraphicsManager;
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
	int _flags;
public:
	DrawInfo(int penColor, const Common::Point &pos, int flags);
};

typedef void (GraphicsManager::*GraphicMethodPtr)(); 
typedef void (GraphicsManager::*ViewPortSetupPtr)(ViewPortResource *);
typedef void (GraphicsManager::*ViewPortAddPtr)(ViewPortResource *, int idx, const Common::Rect &bounds);
typedef void (GraphicsManager::*ViewPortRestorePtr)(ViewPortResource *);

class GraphicsManager {
public:
	VoyeurEngine *_vm;
	bool _palFlag;
	byte _VGAColors[PALETTE_SIZE];
	Common::Array<byte *> _colorChain;
	PictureResource *_backgroundPage;
	int _SVGAPage;
	int _SVGAMode;
	int _SVGAReset;
	ViewPortListResource *_viewPortListPtr;
	ViewPortResource **_vPort;
	bool _MCGAMode;
	bool _saveBack;
	Common::Rect *_clipPtr;
	int _screenOffset;
	uint _planeSelect;
	int _sImageShift;
	Graphics::Surface _screenSurface;
	CMapResource *_backColors;
	FontInfoResource *_fontPtr;
	PictureResource *_fontChar;
	DrawInfo *_drawPtr;
	DrawInfo _defaultDrawInfo;
	bool _drawTextPermFlag;
private:
	static void fadeIntFunc();
	static void vDoCycleInt();

	void restoreBack(Common::Array<Common::Rect> &rectList, int rectListCount,
		PictureResource *srcPic, PictureResource *destPic);
public:
	GraphicsManager();
	~GraphicsManager();
	void setVm(VoyeurEngine *vm) { _vm = vm; }
	void sInitGraphics();

	void setupMCGASaveRect(ViewPortResource *viewPort);
	void addRectOptSaveRect(ViewPortResource *viewPort, int idx, const Common::Rect &bounds);	
	void restoreMCGASaveRect(ViewPortResource *viewPort);
	void addRectNoSaveBack(ViewPortResource *viewPort, int idx, const Common::Rect &bounds);

	void sDrawPic(DisplayResource *srcDisplay, DisplayResource *destDisplay, const Common::Point &initialOffset);
	void fillPic(DisplayResource *display, byte onOff = 0);
	void sDisplayPic(PictureResource *pic);
	void drawANumber(DisplayResource *display, int num, const Common::Point &pt);
	void flipPage();
	void clearPalette();
	void setPalette(const byte *palette, int start, int count);
	void setPalette128(const byte *palette, int start, int count);
	void resetPalette();
	void setColor(int idx, byte r, byte g, byte b);
	void setOneColor(int idx, byte r, byte g, byte b);
	void setColors(int start, int count, const byte *pal);	
	void screenReset();
	void fadeDownICF1(int steps);
	void fadeUpICF1(int steps);
	void fadeDownICF(int steps);
	void drawDot();

	/**
	 * Synchronizes the game data
	 */
	void synchronize(Common::Serializer &s);

	// Methods in the original that are stubbed in ScummVM
	void EMSMapPageHandle(int v1, int v2, int v3) {}
};

} // End of namespace Voyeur

#endif /* VOYEUR_GRAPHICS_H */
