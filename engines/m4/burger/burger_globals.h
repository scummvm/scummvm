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

#ifndef M4_BURGER_BURGER_GLOBALS_H
#define M4_BURGER_BURGER_GLOBALS_H

#include "m4/globals.h"
#include "m4/burger/interface.h"
#include "m4/burger/inventory.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {

enum GameMode {
	JUST_OVERVIEW = 0, INTERACTIVE_DEMO = 1, MAGAZINE_DEMO = 2, WHOLE_GAME = 3
};

class BurgerGlobals : public Globals {
private:
	void initMouseSeries(const Common::String &assetName, RGB8 *myPalette);
	void custom_ascii_converter_proc(char *string);
	static void escape_key_pressed(void *, void *);
	static void cb_F2(void *, void *);
	static void cb_F3(void *, void *);

protected:
	void main_cold_data_init() override;
	void global_menu_system_init() override;
	void initialize_game() override;

public:
	//GlobalVars _globalVars;
	GameMode _gameMode = WHOLE_GAME;
	int _wilburTerm = 2;
	Interface _interface;
	Common::String _wilbur = "WILBUR";

public:
	BurgerGlobals();
	virtual ~BurgerGlobals();
};

extern BurgerGlobals *g_globals;

#undef _G
#define _G(X) (::M4::Burger::g_globals->_##X)
#define _GI(X) _G(interface)._##X
#define VAL(X) _G(_globalVars)[X]

} // namespace Burger
} // namespace M4

#endif
