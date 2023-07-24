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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_OPTIONSELECT_H
#define CRAB_OPTIONSELECT_H
#include "crab/common_header.h"

#include "crab/ui/TextData.h"
#include "crab/ui/button.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class OptionSelect {
	struct {
		Common::Array<Common::String> _text;
		TextData _data;

		void draw(const int &index) {
			if (index >= 0 && (unsigned int)index < _text.size())
				_data.draw(_text[index]);
		}
	} option;

	Button _prev, _next;
	bool _usekeyboard;

public:
	int _cur;

	OptionSelect() {
		_cur = 0;
		_usekeyboard = false;
	}
	void load(rapidxml::xml_node<char> *node);

	void draw();

	bool handleEvents(const Common::Event &Event);

	void setUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_OPTIONSELECT_H
