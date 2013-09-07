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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#ifndef AVALANCHE_PARSER_H
#define AVALANCHE_PARSER_H

#include "common/events.h"

namespace Avalanche {
class AvalancheEngine;

class Parser {
public:
	Common::String _inputText; // Original name: current
	Common::String _inputTextBackup;
	byte _inputTextPos; // Original name: curpos
	bool _quote; // 66 or 99 next?
	byte _leftMargin;
	bool _cursorState;



	Parser(AvalancheEngine *vm);

	void init();

	void handleInputText(const Common::Event &event);

	void handleBackspace();

	void handleReturn();

	void handleFunctionKey(const Common::Event &event);

	void plotText();

	void cursorOn();

	void cursorOff();

	void tryDropdown(); // This asks the parsekey proc in Dropdown if it knows it.

	int16 pos(const Common::String &crit, const Common::String &src); // Returns the index of the first appearance of crit in src.

private:
	AvalancheEngine *_vm;

	void drawCursor();

	void wipeText();

};

} // End of namespace Avalanche

#endif // AVALANCHE_PARSER_H
