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

#ifndef SCI_GUI_Gui_H
#define SCI_GUI_Gui_H

#include "sci/gui/gui_helpers.h"

namespace Sci {

class SciGuiScreen;
class SciGuiPalette;
class SciGuiGfx;
class SciGuiresources;
class SciGuiWindowMgr;
class SciGui {
public:
	SciGui(OSystem *system, EngineState *s, SciGuiScreen *screen, SciGuiPalette *palette);
	SciGui();
	virtual ~SciGui();

	// FIXME: Don't store EngineState
	virtual void resetEngineState(EngineState *s) { _s = s; }

	virtual void init(bool usesOldGfxFunctions);

	virtual void wait(int16 ticks);
	virtual void setPort(uint16 portPtr);
	virtual void setPortPic(Common::Rect rect, int16 picTop, int16 picLeft);
	virtual reg_t getPort();
	virtual void globalToLocal(int16 *x, int16 *y);
	virtual void localToGlobal(int16 *x, int16 *y);
	virtual reg_t newWindow(Common::Rect dims, Common::Rect restoreRect, uint16 style, int16 priority, int16 colorPen, int16 colorBack, const char *title);
	virtual void disposeWindow(uint16 windowPtr, int16 arg2);

	virtual void display(const char *text, int argc, reg_t *argv);

	virtual void textSize(const char *text, int16 font, int16 maxWidth, int16 *textWidth, int16 *textHeight);
	virtual void textFonts(int argc, reg_t *argv);
	virtual void textColors(int argc, reg_t *argv);

	virtual void drawStatus(const char *text, int16 colorPen, int16 colorBack);
	virtual void drawPicture(GuiResourceId pictureId, uint16 showStyle, uint16 flags, int16 EGApaletteNo);
	virtual void drawCel(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, uint16 leftPos, uint16 topPos, int16 priority, uint16 paletteNo);
	virtual void drawControlButton(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 style, bool hilite);
	virtual void drawControlText(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 mode, int16 style, bool hilite);
	virtual void drawControlIcon(Common::Rect rect, reg_t obj, GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, int16 style, bool hilite);

	virtual void graphFillBoxForeground(Common::Rect rect);
	virtual void graphFillBoxBackground(Common::Rect rect);
	virtual void graphFillBox(Common::Rect rect, uint16 colorMask, int16 color, int16 priority, int16 control);
	virtual void graphDrawLine(Common::Point startPoint, Common::Point endPoint, int16 color, int16 priority, int16 control);
	virtual reg_t graphSaveBox(Common::Rect rect, uint16 flags);
	virtual void graphRestoreBox(reg_t handle);

	virtual void paletteSet(int resourceNo, int flags);
	virtual int16 paletteFind(int r, int g, int b);
	virtual void paletteSetIntensity(int fromColor, int toColor, int intensity, bool setPalette);
	virtual void paletteAnimate(int fromColor, int toColor, int speed);

	virtual int16 onControl(byte screenMask, Common::Rect rect);
	virtual void animate(reg_t listReference, bool cycle, int argc, reg_t *argv);
	virtual void addToPicList(reg_t listReference, int argc, reg_t *argv);
	virtual void addToPicView(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, int16 leftPos, int16 topPos, int16 priority, int16 control);
	virtual void setNowSeen(reg_t objectReference);

	virtual void moveCursor(int16 x, int16 y, int16 scaleFactor = 1);
	void moveCursor(Common::Point p, int16 scaleFactor = 1) { moveCursor(p.x, p.y, scaleFactor); }

	SciGuiPalette *getPalette() { return _palette; }

private:
	OSystem *_system;
	EngineState *_s;
	SciGuiScreen *_screen;
	SciGuiPalette *_palette;
	SciGuiGfx *_gfx;
	SciGuiresources *_resources;
	SciGuiWindowMgr *_windowMgr;
};

} // End of namespace Sci

#endif
