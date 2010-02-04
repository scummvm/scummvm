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

#ifndef SCI_GRAPHICS_GUI_H
#define SCI_GRAPHICS_GUI_H

#include "sci/graphics/helpers.h"

namespace Sci {

// Control types and flags
enum {
	SCI_CONTROLS_TYPE_BUTTON		= 1,
	SCI_CONTROLS_TYPE_TEXT			= 2,
	SCI_CONTROLS_TYPE_TEXTEDIT		= 3,
	SCI_CONTROLS_TYPE_ICON			= 4,
	SCI_CONTROLS_TYPE_LIST			= 6,
	SCI_CONTROLS_TYPE_LIST_ALIAS	= 7,
	SCI_CONTROLS_TYPE_DUMMY			= 10
};

class GfxScreen;
class GfxPalette;
class Cursor;
class GfxCache;
class GfxCompare;
class GfxPorts;
class GfxPaint16;
class GfxAnimate;
class GfxControls;
class GfxMenu;
class GfxText16;
class GfxTransitions;

class SciGui {
public:
	SciGui(EngineState *s, GfxScreen *screen, GfxPalette *palette, GfxCache *cache, Cursor *cursor, GfxPorts *ports, AudioPlayer *audio);
	virtual ~SciGui();

	virtual void init(bool usesOldGfxFunctions);

	virtual void wait(int16 ticks);
	virtual void globalToLocal(int16 *x, int16 *y);
	virtual void localToGlobal(int16 *x, int16 *y);
	virtual int16 coordinateToPriority(int16 y);
	virtual int16 priorityToCoordinate(int16 priority);

	virtual void display(const char *text, int argc, reg_t *argv);

	virtual void textSize(const char *text, int16 font, int16 maxWidth, int16 *textWidth, int16 *textHeight);
	virtual void textFonts(int argc, reg_t *argv);
	virtual void textColors(int argc, reg_t *argv);

	virtual void drawStatus(const char *text, int16 colorPen, int16 colorBack);
	virtual void drawMenuBar(bool clear);

	virtual void graphRedrawBox(Common::Rect rect);
	virtual void graphAdjustPriority(int top, int bottom);

	virtual void shakeScreen(uint16 shakeCount, uint16 directions);

	virtual uint16 onControl(byte screenMask, Common::Rect rect);
	virtual void setNowSeen(reg_t objectReference);
	virtual bool canBeHere(reg_t curObject, reg_t listReference);
	virtual bool isItSkip(GuiResourceId viewId, int16 loopNo, int16 celNo, Common::Point position);
	virtual void baseSetter(reg_t object);

	void hideCursor();
	void showCursor();
	bool isCursorVisible();
	void setCursorShape(GuiResourceId cursorId);
	void setCursorView(GuiResourceId viewNum, int loopNum, int cellNum, Common::Point *hotspot);
	virtual void setCursorPos(Common::Point pos);
	Common::Point getCursorPos();
	virtual void moveCursor(Common::Point pos);
	void setCursorZone(Common::Rect zone);

	virtual reg_t portraitLoad(Common::String resourceName);
	virtual void portraitShow(Common::String resourceName, Common::Point position, uint16 resourceNum, uint16 noun, uint16 verb, uint16 cond, uint16 seq);
	virtual void portraitUnload(uint16 portraitId);

	virtual bool debugUndither(bool flag);
	virtual bool debugShowMap(int mapNo);
	virtual bool debugEGAdrawingVisualize(bool state);

	// FIXME: Don't store EngineState
	virtual void resetEngineState(EngineState *s);

protected:
	Cursor *_cursor;
	EngineState *_s;
	GfxScreen *_screen;
	GfxPalette *_palette;
	GfxCache *_cache;
	GfxCompare *_compare;
	GfxPorts *_ports;
	GfxPaint16 *_paint16;

private:
	virtual void initPriorityBands();

	AudioPlayer *_audio;
	GfxAnimate *_animate;
	GfxControls *_controls;
	GfxMenu *_menu;
	GfxText16 *_text16;
	GfxTransitions *_transitions;

	bool _usesOldGfxFunctions;
};

} // End of namespace Sci

#endif
