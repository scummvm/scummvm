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

#include "sci/gui/gui_helpers.h"

namespace Sci {

class SciGUIscreen;
class SciGUIgfx;
class SciGUIresources;
class SciGUIwindowMgr;
class SciGUI {
public:
	SciGUI(OSystem *system, EngineState *s);
	SciGUI();
	virtual ~SciGUI();

	// FIXME: Don't store EngineState
	virtual void resetEngineState(EngineState *s) { _s = s; }

	virtual void init(bool oldGfxFunctions);

	virtual int16 getTimeTicks();
	virtual void wait(int16 ticks);
	virtual void setPort(uint16 portPtr);
	virtual void setPortPic(Common::Rect rect, int16 picTop, int16 picLeft);
	virtual reg_t getPort();
	virtual void globalToLocal(int16 *x, int16 *y);
	virtual void localToGlobal(int16 *x, int16 *y);
	virtual reg_t newWindow(Common::Rect rect1, Common::Rect rect2, uint16 style, int16 priority, int16 colorPen, int16 colorBack, const char *title);
	virtual void disposeWindow(uint16 windowPtr, int16 arg2);

	virtual void display(const char *text, int argc, reg_t *argv);

	virtual void textSize(const char *text, int16 font, int16 maxWidth, int16 *textWidth, int16 *textHeight);
	virtual void textFonts(int argc, reg_t *argv);
	virtual void textColors(int argc, reg_t *argv);

	virtual void drawPicture(sciResourceId pictureId, uint16 showStyle, uint16 flags, int16 EGApaletteNo);
	virtual void drawCell(sciResourceId viewId, uint16 loopNo, uint16 cellNo, uint16 leftPos, uint16 topPos, int16 priority, uint16 paletteNo);
	virtual void drawControlButton(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 style, bool inverse);
	virtual void drawControlText(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 mode, int16 style, bool inverse);

	virtual void graphFillBoxForeground(Common::Rect rect);
	virtual void graphFillBoxBackground(Common::Rect rect);
	virtual void graphFillBox(Common::Rect rect, uint16 colorMask, int16 color, int16 priority, int16 control);
	virtual void graphDrawLine(Common::Rect rect, int16 color, int16 priority, int16 control);
	virtual reg_t graphSaveBox(Common::Rect rect, uint16 flags);
	virtual void graphRestoreBox(reg_t handle);

	virtual void paletteSet(int resourceNo, int flags);
	virtual int16 paletteFind(int r, int g, int b);
	virtual void paletteAnimate(int fromColor, int toColor, int speed);

	virtual void moveCursor(int16 x, int16 y);

private:
	OSystem *_system;
	EngineState *_s;
	SciGUIscreen *_screen;
	SciGUIgfx *_gfx;
	SciGUIresources *_resources;
	SciGUIwindowMgr *_windowMgr;
	bool _usesOldGfxFunctions;
};

} // End of namespace Sci
