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

#ifndef SCI_GRAPHICS_GUI32_H
#define SCI_GRAPHICS_GUI32_H

#include "sci/graphics/helpers.h"

namespace Sci {

class GfxCursor;
class GfxScreen;
class GfxPalette;
class GfxCache;
class GfxCompare;
class GfxFrameout;
class GfxPaint32;

class SciGui32 {
public:
	SciGui32(EngineState *s, GfxScreen *screen, GfxPalette *palette, GfxCache *cache, GfxCursor *cursor);
	~SciGui32();

	void init();

	void textSize(const char *text, int16 font, int16 maxWidth, int16 *textWidth, int16 *textHeight);

	void shakeScreen(uint16 shakeCount, uint16 directions);

	uint16 onControl(byte screenMask, Common::Rect rect);
	void setNowSeen(reg_t objectReference);
	bool canBeHere(reg_t curObject, reg_t listReference);
	bool isItSkip(GuiResourceId viewId, int16 loopNo, int16 celNo, Common::Point position);
	void baseSetter(reg_t object);

	void setCursorPos(Common::Point pos);
	void moveCursor(Common::Point pos);
	void setCursorZone(Common::Rect zone);

	void addScreenItem(reg_t object);
	void deleteScreenItem(reg_t object);
	void addPlane(reg_t object);
	void updatePlane(reg_t object);
	void deletePlane(reg_t object);
	int16 getHighPlanePri();
	void frameOut();
	void globalToLocal(int16 *x, int16 *y, reg_t planeObj);
	void localToGlobal(int16 *x, int16 *y, reg_t planeObj);

	void drawRobot(GuiResourceId robotId);

	// FIXME: Don't store EngineState
	void resetEngineState(EngineState *s);

protected:
	GfxCursor *_cursor;
	EngineState *_s;
	GfxScreen *_screen;
	GfxPalette *_palette;
	GfxCache *_cache;
	GfxCompare *_compare;
	GfxFrameout *_frameout;
	GfxPaint32 *_paint32;

private:
};

} // End of namespace Sci

#endif
