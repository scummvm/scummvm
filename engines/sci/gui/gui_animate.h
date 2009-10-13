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

#ifndef SCI_GUI_ANIMATE_H
#define SCI_GUI_ANIMATE_H

#include "sci/gui/gui_helpers.h"

namespace Sci {

enum {
	SCI_ANIMATE_SIGNAL_STOPUPDATE    = 0x0001,
	SCI_ANIMATE_SIGNAL_VIEWUPDATED   = 0x0002,
	SCI_ANIMATE_SIGNAL_NOUPDATE      = 0x0004,
	SCI_ANIMATE_SIGNAL_HIDDEN        = 0x0008,
	SCI_ANIMATE_SIGNAL_FIXEDPRIORITY = 0x0010,
	SCI_ANIMATE_SIGNAL_ALWAYSUPDATE  = 0x0020,
	SCI_ANIMATE_SIGNAL_FORCEUPDATE   = 0x0040,
	SCI_ANIMATE_SIGNAL_REMOVEVIEW    = 0x0080,
	SCI_ANIMATE_SIGNAL_FROZEN        = 0x0100,
	SCI_ANIMATE_SIGNAL_IGNOREACTOR   = 0x4000,
	SCI_ANIMATE_SIGNAL_DISPOSEME     = 0x8000
};

class SciGuiScreen;
class SciGuiView;
class SciGuiAnimate {
public:
	SciGuiAnimate(EngineState *state, SciGuiGfx *gfx, SciGuiScreen *screen, SciGuiPalette *palette);
	~SciGuiAnimate();

	// FIXME: Don't store EngineState
	void resetEngineState(EngineState *newState) { _s = newState; }

	void disposeLastCast();
	void invoke(List *list, int argc, reg_t *argv);
	void makeSortedList(List *list);
	void fill(byte &oldPicNotValid);
	void update();
	void drawCels();
	void updateScreen(byte oldPicNotValid);
	void restoreAndDelete(int argc, reg_t *argv);
	void reAnimate(Common::Rect rect);
	void addToPicDrawCels(List *list);
	void addToPicDrawView(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, int16 leftPos, int16 topPos, int16 priority, int16 control);

private:
	void init(void);

	EngineState *_s;
	SciGuiGfx *_gfx;
	SciGuiScreen *_screen;
	SciGuiPalette *_palette;

	uint16 _listSize;
	GuiAnimateEntry *_listData;
	GuiAnimateList _list;
	//List *_lastCast;
};

} // End of namespace Sci

#endif
