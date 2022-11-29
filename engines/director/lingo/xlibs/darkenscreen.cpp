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

/*************************************
 *
 * USED IN:
 * Secrets of the Pyramids (Mac)
 * Mindscape's Wonder Land (Mac)
 * Grackon's Curse (Mac)
 * Return to Jurassic (Mac)
 * Ednovation demos (Mac)
 *
 *************************************/

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/darkenscreen.h"


namespace Director {

const char *DarkenScreen::xlibName = "darkenScreen";
const char *DarkenScreen::fileNames[] = {
	"darkenScreen",
	0
};

static BuiltinProto builtins[] = {
	{ "darkenScreen", DarkenScreen::m_darkenscreen, 0, 0, 300, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

void DarkenScreen::open(int type) {
	g_lingo->initBuiltIns(builtins);
}

void DarkenScreen::close(int type) {
	g_lingo->cleanupBuiltIns(builtins);
}

void DarkenScreen::m_darkenscreen(int nargs) {
    g_lingo->printSTUBWithArglist("DarkenScreen::m_darkenscreen", nargs);
}

} // End of namespace Director
