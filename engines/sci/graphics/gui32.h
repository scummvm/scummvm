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

class Screen;
class SciPalette;
class Cursor;
class Gfx;
class Text;

class SciGui32 {
public:
	SciGui32(EngineState *s, Screen *screen, SciPalette *palette, Cursor *cursor);
	SciGui32();
	~SciGui32();

	void init();

	void textSize(const char *text, int16 font, int16 maxWidth, int16 *textWidth, int16 *textHeight);

	void shakeScreen(uint16 shakeCount, uint16 directions);

	uint16 onControl(byte screenMask, Common::Rect rect);
	void setNowSeen(reg_t objectReference);
	bool canBeHere(reg_t curObject, reg_t listReference);
	bool isItSkip(GuiResourceId viewId, int16 loopNo, int16 celNo, Common::Point position);
	void baseSetter(reg_t object);

	void hideCursor();
	void showCursor();
	bool isCursorVisible();
	void setCursorShape(GuiResourceId cursorId);
	void setCursorView(GuiResourceId viewNum, int loopNum, int cellNum, Common::Point *hotspot);
	void setCursorPos(Common::Point pos);
	Common::Point getCursorPos();
	void moveCursor(Common::Point pos);
	void setCursorZone(Common::Rect zone);

	int16 getCelWidth(GuiResourceId viewId, int16 loopNo, int16 celNo);
	int16 getCelHeight(GuiResourceId viewId, int16 loopNo, int16 celNo);

	int16 getLoopCount(GuiResourceId viewId);
	int16 getCelCount(GuiResourceId viewId, int16 loopNo);

	void syncWithFramebuffer();

	void addScreenItem(reg_t object);
	void deleteScreenItem(reg_t object);
	void addPlane(reg_t object);
	void updatePlane(reg_t object);
	void deletePlane(reg_t object);
	void frameOut();
	void globalToLocal(int16 *x, int16 *y, reg_t planeObj);
	void localToGlobal(int16 *x, int16 *y, reg_t planeObj);

	void drawRobot(GuiResourceId robotId);

	bool debugShowMap(int mapNo);

	// FIXME: Don't store EngineState
	void resetEngineState(EngineState *s);

protected:
	Cursor *_cursor;
	EngineState *_s;
	Screen *_screen;
	SciPalette *_palette;
	Gfx *_gfx;

private:
	Common::Array<reg_t> _screenItems;
	Common::Array<reg_t> _planes;
};

} // End of namespace Sci

#endif
