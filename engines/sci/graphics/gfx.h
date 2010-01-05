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

#ifndef SCI_GRAPHICS_GFX_H
#define SCI_GRAPHICS_GFX_H

#include "sci/graphics/gui.h"

#include "common/hashmap.h"

namespace Sci {

#define SCI_TEXT_ALIGNMENT_RIGHT -1
#define SCI_TEXT_ALIGNMENT_CENTER 1
#define SCI_TEXT_ALIGNMENT_LEFT	0

#define MAX_CACHED_VIEWS 50

class Screen;
class SciPalette;
class Font;
class SciGuiPicture;
class View;

typedef Common::HashMap<int, View *> ViewCache;

class Gfx {
public:
	Gfx(ResourceManager *resMan, SegManager *segMan, Kernel *kernel, Screen *screen, SciPalette *palette);
	~Gfx();

	void init(Text *text);

	byte *GetSegment(byte seg);
	void ResetScreen();

	Port *SetPort(Port *port);
	Port *GetPort();
	void SetOrigin(int16 left, int16 top);
	void MoveTo(int16 left, int16 top);
	void Move(int16 left, int16 top);
	void OpenPort(Port *port);
	void PenColor(int16 color);
	void BackColor(int16 color);
	void PenMode(int16 mode);
	void TextGreyedOutput(bool state);
	int16 GetPointSize();

	void ClearScreen(byte color = 255);
	void InvertRect(const Common::Rect &rect);
	void EraseRect(const Common::Rect &rect);
	void PaintRect(const Common::Rect &rect);
	void FillRect(const Common::Rect &rect, int16 drawFlags, byte clrPen, byte clrBack = 0, byte bControl = 0);
	void FrameRect(const Common::Rect &rect);
	void OffsetRect(Common::Rect &r);
	void OffsetLine(Common::Point &start, Common::Point &end);

	void BitsShow(const Common::Rect &r);
	MemoryHandle BitsSave(const Common::Rect &rect, byte screenFlags);
	void BitsGetRect(MemoryHandle memoryHandle, Common::Rect *destRect);
	void BitsRestore(MemoryHandle memoryHandle);
	void BitsFree(MemoryHandle memoryHandle);

	void drawPicture(GuiResourceId pictureId, int16 animationNr, bool mirroredFlag, bool addToFlag, GuiResourceId paletteId);
	void drawCel(GuiResourceId viewId, LoopNo loopNo, CelNo celNo, uint16 leftPos, uint16 topPos, byte priority, uint16 paletteNo, int16 origHeight = -1, uint16 scaleX = 128, uint16 scaleY = 128);
	void drawCel(GuiResourceId viewId, LoopNo loopNo, CelNo celNo, Common::Rect celRect, byte priority, uint16 paletteNo, int16 origHeight = -1, uint16 scaleX = 128, uint16 scaleY = 128);
	void drawCel(View *view, LoopNo loopNo, CelNo celNo, Common::Rect celRect, byte priority, uint16 paletteNo, int16 origHeight = -1, uint16 scaleX = 128, uint16 scaleY = 128);

	uint16 onControl(uint16 screenMask, Common::Rect rect);

	void PriorityBandsInit(int16 bandCount, int16 top, int16 bottom);
	void PriorityBandsInit(byte *data);
	byte CoordinateToPriority(int16 y);
	int16 PriorityToCoordinate(byte priority);

	bool CanBeHereCheckRectList(reg_t checkObject, Common::Rect checkRect, List *list);

	void SetNowSeen(reg_t objectReference);

	Port *_menuPort;
	Common::Rect _menuBarRect;
	Port *_curPort;

	View *getView(GuiResourceId viewNum);

private:
	void purgeCache();

	ResourceManager *_resMan;
	SegManager *_segMan;
	Kernel *_kernel;
	Screen *_screen;
	SciPalette *_palette;
	Text *_text;

	Common::Rect _bounds;
	Port *_mainPort;

	// Priority Bands related variables
	int16 _priorityTop, _priorityBottom, _priorityBandCount;
	byte _priorityBands[200];

	ViewCache _cachedViews;
};

} // End of namespace Sci

#endif
