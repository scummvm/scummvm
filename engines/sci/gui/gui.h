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

#ifndef SCI_GUI_GUI_H
#define SCI_GUI_GUI_H

#include "sci/gui/gui_helpers.h"

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

class SciGuiScreen;
class SciGuiPalette;
class SciGuiCursor;
class SciGuiGfx;
class SciGuiWindowMgr;
class SciGuiAnimate;
class SciGuiControls;
class SciGuiMenu;
class SciGuiText;
class SciGuiTransitions;
class SciGui32; // for debug purposes

class SciGui {
public:
	SciGui(EngineState *s, SciGuiScreen *screen, SciGuiPalette *palette, SciGuiCursor *cursor);
	SciGui();
	virtual ~SciGui();

	virtual void init(bool usesOldGfxFunctions);

	virtual void wait(int16 ticks);
	virtual void setPort(uint16 portPtr);
	virtual void setPortPic(Common::Rect rect, int16 picTop, int16 picLeft, bool initPriorityBandsFlag);
	virtual reg_t getPort();
	virtual void globalToLocal(int16 *x, int16 *y);
	virtual void localToGlobal(int16 *x, int16 *y);
	virtual int16 coordinateToPriority(int16 y);
	virtual int16 priorityToCoordinate(int16 priority);

	virtual reg_t newWindow(Common::Rect dims, Common::Rect restoreRect, uint16 style, int16 priority, int16 colorPen, int16 colorBack, const char *title);
	virtual void disposeWindow(uint16 windowPtr, bool reanimate);

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
	virtual void drawCel(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, uint16 leftPos, uint16 topPos, int16 priority, uint16 paletteNo, int16 origHeight = -1);
	virtual void drawControlButton(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 style, bool hilite);
	virtual void drawControlText(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 alignment, int16 style, bool hilite);
	virtual void drawControlTextEdit(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 mode, int16 style, int16 cursorPos, int16 maxChars, bool hilite);
	virtual void drawControlIcon(Common::Rect rect, reg_t obj, GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, int16 style, bool hilite);
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
	virtual void graphUpdateBox(Common::Rect rect);
	virtual void graphRedrawBox(Common::Rect rect);
	virtual void graphAdjustPriority(int top, int bottom);

	virtual int16 picNotValid(int16 newPicNotValid);

	virtual void paletteSet(GuiResourceId resourceNo, uint16 flags);
	virtual void paletteSetFlag(uint16 fromColor, uint16 toColor, uint16 flag);
	virtual void paletteUnsetFlag(uint16 fromColor, uint16 toColor, uint16 flag);
	virtual int16 paletteFind(uint16 r, uint16 g, uint16 b);
	virtual void paletteSetIntensity(uint16 fromColor, uint16 toColor, uint16 intensity, bool setPalette);
	virtual void paletteAnimate(uint16 fromColor, uint16 toColor, int16 speed);

	virtual void shakeScreen(uint16 shakeCount, uint16 directions);

	virtual uint16 onControl(byte screenMask, Common::Rect rect);
	virtual void animate(reg_t listReference, bool cycle, int argc, reg_t *argv);
	virtual void addToPicList(reg_t listReference, int argc, reg_t *argv);
	virtual void addToPicView(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, int16 leftPos, int16 topPos, int16 priority, int16 control);
	virtual void setNowSeen(reg_t objectReference);
	virtual bool canBeHere(reg_t curObject, reg_t listReference);
	virtual bool isItSkip(GuiResourceId viewId, int16 loopNo, int16 celNo, Common::Point position);
	virtual void baseSetter(reg_t object);

	void hideCursor();
	void showCursor();
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

	virtual reg_t portraitLoad(Common::String resourceName);
	virtual void portraitShow(Common::String resourceName, Common::Point position, uint16 resourceNum, uint16 noun, uint16 verb, uint16 cond, uint16 seq);
	virtual void portraitUnload(uint16 portraitId);

	virtual bool debugUndither(bool flag);
	virtual bool debugShowMap(int mapNo);

	// FIXME: Don't store EngineState
	virtual void resetEngineState(EngineState *s);

protected:
	SciGuiCursor *_cursor;
	EngineState *_s;
	SciGuiScreen *_screen;
	SciGuiPalette *_palette;
	SciGuiGfx *_gfx;

private:
	virtual void initPriorityBands();
	virtual void animateShowPic();
	virtual void addToPicSetPicNotValid();

	SciGuiWindowMgr *_windowMgr;
	SciGuiAnimate *_animate;
	SciGuiControls *_controls;
	SciGuiMenu *_menu;
	SciGuiText *_text;
	SciGuiTransitions *_transitions;
// 	SciGui32 *_gui32; // for debug purposes

	bool _usesOldGfxFunctions;
};

} // End of namespace Sci

#endif
