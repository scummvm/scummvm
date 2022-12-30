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

#ifndef TETRAEDGE_GAME_CELLPHONE_H
#define TETRAEDGE_GAME_CELLPHONE_H

#include "common/array.h"
#include "common/callback.h"
#include "common/str.h"
#include "common/formats/xmlparser.h"

#include "tetraedge/te/te_layout.h"
#include "tetraedge/te/te_text_layout.h"
#include "tetraedge/te/te_lua_gui.h"

namespace Tetraedge {

class Cellphone : public TeLayout {
public:
	Cellphone();
	virtual ~Cellphone() {}

	bool addNumber(const Common::String &num);
	void currentPage(int offset);

	void enter();
	void leave();

	void load();
	void loadFromBackup(const Common::XMLParser::ParserNode *node);

	bool onCallNumberValidated();
	bool onCloseButtonValidated();
	bool onNextNumber();
	bool onPreviousNumber();

	void saveToBackup(Common::XMLParser::ParserNode *xmlnode);
	void setVisible(bool visible);

	TeSignal1Param<Common::String> &onCallNumber() {
		return _onCallNumberSignal;
	}

	void unload();

	Common::Error syncState(Common::Serializer &s);

	TeLuaGUI &gui() { return _gui; }

private:

	int _nextNumber;
	Common::Array<TeTextLayout*> _textLayoutArray;
	Common::Array<Common::String> _addedNumbers;

	TeSignal1Param<Common::String> _onCallNumberSignal;

	TeLuaGUI _gui;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_CELLPHONE_H
