/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 */


#include "engines/grim/emi/emi.h"
#include "engines/grim/emi/lua_v2.h"


namespace Grim {

EMIEngine *g_emi = NULL;

EMIEngine::EMIEngine(OSystem *syst, uint32 gameFlags, GrimGameType gameType, Common::Platform platform, Common::Language language) :
		GrimEngine(syst, gameFlags, gameType, platform, language) {
	g_emi = this;
}


EMIEngine::~EMIEngine() {
	g_emi = NULL;
}

LuaBase *EMIEngine::createLua() {
	return new Lua_V2();
}

void EMIEngine::pushText(Common::List<TextObject *> *objects) {
	_textstack.push_back(objects);
}

Common::List<TextObject *> *EMIEngine::popText() {
	Common::List<TextObject *> *object = _textstack.front();
	_textstack.pop_front();
	return object;
}

} // end of namespace Grim
