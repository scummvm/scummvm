/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef M4_RIDDLE_VARS_H
#define M4_RIDDLE_VARS_H

#include "common/textconsole.h"
#include "m4/riddle/flags.h"
#include "m4/riddle/hotkeys.h"
#include "m4/riddle/walker.h"
#include "m4/riddle/gui/interface.h"
#include "m4/vars.h"

namespace M4 {
namespace Riddle {

#define LETTERBOX_Y 30

enum global_triggers {
	kINVENTORY_CLOSEUP_END = 990,
	kCALLED_EACH_LOOP = 32764,
	kGOTO_MAIN_MENU = 32766
};

class Vars : public M4::Vars {
protected:
	void main_cold_data_init() override;

public:
	Riddle::Flags _flags;
	Riddle::Walker _walker;
	Riddle::Hotkeys _hotkeys;
	Riddle::GUI::Interface _interface;
	bool _menuSystemInitialized = false;
	int _global301 = 0;
	int _flag1 = 0;
	bool _kittyScreaming = false;
	bool _spleenSpraying = false;
	Common::String _string1;
	Common::String _string2;
	Common::String _string3;
	int _ripSketching = 0;

public:
	Vars();
	virtual ~Vars();

	M4::Interface *getInterface() override {
		return &_interface;
	}
	Hotkeys *getHotkeys() override {
		return &_hotkeys;
	}
	M4::Walker *getWalker() override {
		return &_walker;
	}
	void initialize_game() override;

	void global_menu_system_init() override;
};

extern Vars *g_vars;

#undef _G
#undef _GI
#define _G(X) (::M4::Riddle::g_vars->_##X)
#define _GI(X) _G(interface)._##X
#define _GINT() _G(interface)

} // namespace Riddle
} // namespace M4

#endif
