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

// Flags for the signal selector
enum ViewSignals {
	kSignalStopUpdate    = 0x0001,
	kSignalViewUpdated   = 0x0002,
	kSignalNoUpdate      = 0x0004,
	kSignalHidden        = 0x0008,
	kSignalFixedPriority = 0x0010,
	kSignalAlwaysUpdate  = 0x0020,
	kSignalForceUpdate   = 0x0040,
	kSignalRemoveView    = 0x0080,
	kSignalFrozen        = 0x0100,
	kSignalExtraActor	 = 0x0200, // unused by us, defines all actors that may be included into the background if speed to slow
	kSignalHitObstacle	 = 0x0400, // used in the actor movement code by kDoBresen()
	kSignalDoesntTurn	 = 0x0800, // used by _k_dirloop() to determine if an actor can turn or not
	kSignalNoCycler		 = 0x1000, // unused by us
	kSignalIgnoreHorizon = 0x2000, // unused by us, defines actor that can ignore horizon
	kSignalIgnoreActor   = 0x4000,
	kSignalDisposeMe     = 0x8000,

	kSignalStopUpdHack	 = 0x20000000 // View has been stop-updated (again???) - a hack used by the old GUI code only, for dynamic views
};

class SciGuiGfx;
class SciGuiScreen;
class SciGuiPalette;
class SciGuiTransitions;
class SciGuiAnimate {
public:
	SciGuiAnimate(EngineState *state, SciGuiGfx *gfx, SciGuiScreen *screen, SciGuiPalette *palette);
	~SciGuiAnimate();

	// FIXME: Don't store EngineState
	void resetEngineState(EngineState *newState) { _s = newState; }

	void disposeLastCast();
	bool invoke(List *list, int argc, reg_t *argv);
	void makeSortedList(List *list);
	void fill(byte &oldPicNotValid);
	void update();
	void drawCels();
	void updateScreen(byte oldPicNotValid);
	void restoreAndDelete(int argc, reg_t *argv);
	void reAnimate(Common::Rect rect);
	void addToPicDrawCels();
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

	bool _ignoreFastCast;
};

} // End of namespace Sci

#endif
