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

#ifndef SCI_GRAPHICS_VIEW_H
#define SCI_GRAPHICS_VIEW_H

namespace Sci {

struct CelInfo {
	int16 width, height;
	int16 displaceX;
	int16 displaceY;
	byte clearKey;
	uint16 offsetEGA;
	uint16 offsetRLE;
	uint16 offsetLiteral;
	byte *rawBitmap;
};

struct LoopInfo {
	bool mirrorFlag;
	uint16 celCount;
	CelInfo *cel;
};

#define SCI_VIEW_EGAMAPPING_SIZE 16
#define SCI_VIEW_EGAMAPPING_COUNT 8

class View {
public:
	View(ResourceManager *resMan, Screen *screen, SciPalette *palette, GuiResourceId resourceId);
	~View();

	GuiResourceId getResourceId();
	int16 getWidth(int16 loopNo, int16 celNo);
	int16 getHeight(int16 loopNo, int16 celNo);
	CelInfo *getCelInfo(int16 loopNo, int16 celNo);
	LoopInfo *getLoopInfo(int16 loopNo);
	void getCelRect(int16 loopNo, int16 celNo, int16 x, int16 y, int16 z, Common::Rect *outRect);
	byte *getBitmap(int16 loopNo, int16 celNo);
	void draw(Common::Rect rect, Common::Rect clipRect, Common::Rect clipRectTranslated, int16 loopNo, int16 celNo, byte priority, uint16 EGAmappingNr, bool upscaledHires, uint16 scaleX = 128, uint16 scaleY = 128);
	uint16 getLoopCount() const { return _loopCount; }
	uint16 getCelCount(int16 loopNo) { return _loop[loopNo].celCount; }
	Palette *getPalette();

private:
	void initData(GuiResourceId resourceId);
	void unpackCel(int16 loopNo, int16 celNo, byte *outPtr, uint16 pixelCount);
	void unditherBitmap(byte *bitmap, int16 width, int16 height, byte clearKey);

	ResourceManager *_resMan;
	Screen *_screen;
	SciPalette *_palette;

	GuiResourceId _resourceId;
	Resource *_resource;
	byte *_resourceData;

	uint16 _loopCount;
	LoopInfo *_loop;
	bool _embeddedPal;
	Palette _viewPalette;

	byte *_EGAmapping;
};

} // End of namespace Sci

#endif
