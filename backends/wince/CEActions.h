/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef CEACTIONS
#define CEACTIONS

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"


#include "base/gameDetector.h"
#include "wince-sdl.h"
#include "Key.h"

enum ActionType {
        ACTION_NONE = 0,
        ACTION_PAUSE,
        ACTION_SAVE,
        ACTION_QUIT,
        ACTION_SKIP,
        ACTION_HIDE,
        ACTION_KEYBOARD,
        ACTION_SOUND,
        ACTION_RIGHTCLICK,
        ACTION_CURSOR,
        ACTION_FREELOOK,

		ACTION_LAST
};

#define ACTIONS_VERSION 1

class OSystem_WINCE3;

class CEActions {
	public:
		static CEActions* Instance();
		static void init(OSystem_WINCE3 *mainSystem, GameDetector &detector);

		// Actions
		bool perform(ActionType action);
		bool isActive(ActionType action);
		bool isEnabled(ActionType action);
		String actionName(ActionType action);
		int size();

		// Mapping
		void beginMapping(bool start);
		bool mappingActive();
		bool performMapped(unsigned int keyCode, bool pushed);
		bool loadMapping();
		bool saveMapping();
		unsigned int getMapping(ActionType action);
		void setMapping(ActionType action, unsigned int keyCode);

		// Utility
		bool needsRightClickMapping();

		~CEActions();
	private:
		CEActions(OSystem_WINCE3 *mainSystem, GameDetector &detector);
		static CEActions* _instance;
		OSystem_WINCE3 *_mainSystem;
		Key _key_action[ACTION_LAST];
		bool _action_active[ACTION_LAST];
		bool _action_enabled[ACTION_LAST];
		unsigned int _action_mapping[ACTION_LAST];
		bool _mapping_active;
		bool _right_click_needed;
	};	

#endif