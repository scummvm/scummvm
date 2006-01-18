/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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
 * $Header$
 *
 */

#ifndef SYMBIANACTIONS_H
#define SYMBIANACTIONS_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "base/gameDetector.h"
#include "gui/Key.h"
#include "gui/Actions.h"

namespace GUI {

#define ACTION_VERSION 6

enum actionType {
        ACTION_UP = 0,
        ACTION_DOWN,
        ACTION_LEFT,
        ACTION_RIGHT,
        ACTION_LEFTCLICK,
        ACTION_RIGHTCLICK,
        ACTION_SAVE,
        ACTION_SKIP,
        ACTION_ZONE,
		ACTION_FT_CHEAT,
		ACTION_SWAPCHAR,
		ACTION_SKIP_TEXT,
		ACTION_PAUSE,
		ACTION_QUIT,
		ACTION_LAST
};

class SymbianActions : public Actions {
public:
	// Actions
	bool perform(ActionType action, bool pushed = true);
	Common::String actionName(ActionType action);
	int size();
	static void init(GameDetector &detector);
	void initInstanceMain(OSystem *mainSystem);
	void initInstanceGame();

	// Action domain
	Common::String domain();
	int version();

	~SymbianActions();

private:
	SymbianActions(GameDetector &detector);
	bool _right_click_needed;
};

} // namespace GUI

#endif
