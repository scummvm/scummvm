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

#ifndef SCI_GRAPHICS_ANIMATE_H
#define SCI_GRAPHICS_ANIMATE_H

#include "sci/graphics/helpers.h"

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
	kSignalExtraActor	 = 0x0200, // unused by us, defines all actors that may be included into the background if speed is too slow
	kSignalHitObstacle	 = 0x0400, // used in the actor movement code by kDoBresen()
	kSignalDoesntTurn	 = 0x0800, // used by _k_dirloop() to determine if an actor can turn or not
	kSignalNoCycler		 = 0x1000, // unused by us
	kSignalIgnoreHorizon = 0x2000, // unused by us, defines actor that can ignore horizon
	kSignalIgnoreActor   = 0x4000,
	kSignalDisposeMe     = 0x8000
};

enum ViewScaleSignals {
	kScaleSignalDoScaling	= 0x0001, // enables scaling when drawing that cel (involves scaleX and scaleY)
	kScaleSignalUnknown1	= 0x0002, // seems to do something with globalvar 2, sets scaleX/scaleY
	kScaleSignalUnknown2	= 0x0004 // really unknown
};

class Gfx;
class Screen;
class SciPalette;
class Transitions;
class SciGuiAnimate {
public:
	SciGuiAnimate(EngineState *state, Gfx *gfx, Screen *screen, SciPalette *palette);
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
	void addToPicDrawView(GuiResourceId viewId, int16 loopNo, int16 celNo, int16 leftPos, int16 topPos, int16 priority, int16 control);

private:
	void init();

	EngineState *_s;
	Gfx *_gfx;
	Screen *_screen;
	SciPalette *_palette;

	uint16 _listCount;
	AnimateEntry *_listData;
	AnimateList _list;

	uint16 _lastCastCount;
	AnimateEntry *_lastCastData;

	bool _ignoreFastCast;
};

} // End of namespace Sci

#endif
