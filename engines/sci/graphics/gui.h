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

class Screen;
class GfxPalette;
class Cursor;
class GfxCache;
class GfxCompare;
class GfxPorts;
class GfxPaint16;
class WindowMgr;
class SciGuiAnimate;
class Controls;
class Menu;
class Text;
class Transitions;

class SciGui {
public:
	SciGui(EngineState *s, GfxScreen *screen, GfxPalette *palette, Cursor *cursor, GfxPorts *ports, AudioPlayer *audio);
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
	virtual void menuReset();
	virtual void menuAdd(Common::String title, Common::String content, reg_t contentVmPtr);
	virtual void menuSet(uint16 menuId, uint16 itemId, uint16 attributeId, reg_t value);
	virtual reg_t menuGet(uint16 menuId, uint16 itemId, uint16 attributeId);
	virtual reg_t menuSelect(reg_t eventObject);

	virtual void drawPicture(GuiResourceId pictureId, int16 animationNr, bool animationBlackoutFlag, bool mirroredFlag, bool addToFlag, int16 EGApaletteNo);
	virtual void drawCel(GuiResourceId viewId, int16 loopNo, int16 celNo, uint16 leftPos, uint16 topPos, int16 priority, uint16 paletteNo, bool hiresMode = false, reg_t upscaledHiresHandle = NULL_REG);
	virtual void drawControlButton(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 style, bool hilite);
	virtual void drawControlText(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 alignment, int16 style, bool hilite);
	virtual void drawControlTextEdit(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 mode, int16 style, int16 cursorPos, int16 maxChars, bool hilite);
	virtual void drawControlIcon(Common::Rect rect, reg_t obj, GuiResourceId viewId, int16 loopNo, int16 celNo, int16 priority, int16 style, bool hilite);
	virtual void drawControlList(Common::Rect rect, reg_t obj, int16 maxChars, int16 count, const char **entries, GuiResourceId fontId, int16 style, int16 upperPos, int16 cursorPos, bool isAlias, bool hilite);
	virtual void editControl(reg_t controlObject, reg_t eventObject);

	virtual void graphFillBoxForeground(Common::Rect rect);
	virtual void graphFillBoxBackground(Common::Rect rect);
	virtual void graphFillBox(Common::Rect rect, uint16 colorMask, int16 color, int16 priority, int16 control);
	virtual void graphFrameBox(Common::Rect rect, int16 color);
	virtual void graphDrawLine(Common::Point startPoint, Common::Point endPoint, int16 color, int16 priority, int16 control);
	virtual reg_t graphSaveBox(Common::Rect rect, uint16 flags);
	virtual reg_t graphSaveUpscaledHiresBox(Common::Rect rect);
	virtual void graphRestoreBox(reg_t handle);
	virtual void graphUpdateBox(Common::Rect rect, bool hiresMode);
	virtual void graphRedrawBox(Common::Rect rect);
	virtual void graphAdjustPriority(int top, int bottom);

	virtual int16 picNotValid(int16 newPicNotValid);

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

	virtual int16 getCelWidth(GuiResourceId viewId, int16 loopNo, int16 celNo);
	virtual int16 getCelHeight(GuiResourceId viewId, int16 loopNo, int16 celNo);

	virtual int16 getLoopCount(GuiResourceId viewId);
	virtual int16 getCelCount(GuiResourceId viewId, int16 loopNo);

	virtual void syncWithFramebuffer();

	virtual reg_t portraitLoad(Common::String resourceName);
	virtual void portraitShow(Common::String resourceName, Common::Point position, uint16 resourceNum, uint16 noun, uint16 verb, uint16 cond, uint16 seq);
	virtual void portraitUnload(uint16 portraitId);

	void startPalVary(uint16 paletteId, uint16 ticks);
	void togglePalVary(bool pause);
	void stopPalVary();

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
	virtual int getControlPicNotValid();
	static void palVaryCallback(void *refCon);
	void doPalVary();

	AudioPlayer *_audio;
	GfxAnimate *_animate;
	Controls *_controls;
	Menu *_menu;
	Text *_text;
	Transitions *_transitions;
	int16 _palVaryId;
	uint32 _palVaryStart;
	uint32 _palVaryEnd;

	bool _usesOldGfxFunctions;
};

} // End of namespace Sci

#endif
