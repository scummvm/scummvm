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

#ifndef CEACTIONSSMARTPHONE
#define CEACTIONSSMARTPHONE

//#ifdef WIN32_PLATFORM_WFSP

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"


#include "base/gameDetector.h"
#include "wince-sdl.h"
#include "gui/Key.h"

#include "gui/Actions.h"

#define SMARTPHONE_ACTION_VERSION 4

enum smartphoneActionType {
        SMARTPHONE_ACTION_UP = 0,
        SMARTPHONE_ACTION_DOWN,
        SMARTPHONE_ACTION_LEFT,
        SMARTPHONE_ACTION_RIGHT,
        SMARTPHONE_ACTION_LEFTCLICK,
        SMARTPHONE_ACTION_RIGHTCLICK,
        SMARTPHONE_ACTION_SAVE,
        SMARTPHONE_ACTION_SKIP,
        SMARTPHONE_ACTION_ZONE,
	SMARTPHONE_ACTION_FT_CHEAT,

	SMARTPHONE_ACTION_LAST
};


class CEActionsSmartphone : public GUI::Actions {
	public:
		// Actions
		bool perform(GUI::ActionType action, bool pushed = true);
		String actionName(GUI::ActionType action);
		int size();
		static void init(GameDetector &detector);
		void initInstanceMain(OSystem *mainSystem);
		void initInstanceGame();

		// Action domain
		String domain();
		int version();

		~CEActionsSmartphone();
	private:
		CEActionsSmartphone(GameDetector &detector);
		bool _right_click_needed;
		OSystem_WINCE3 *_CESystem;
	};

#endif

//#endif
