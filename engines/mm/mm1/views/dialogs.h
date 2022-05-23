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

#ifndef MM1_VIEWS_DIALOGS_H
#define MM1_VIEWS_DIALOGS_H

#include "mm/mm1/events.h"
#include "mm/mm1/views/title.h"
#include "mm/mm1/views/are_you_ready.h"
#include "mm/mm1/views/main_menu.h"
#include "mm/mm1/views/view_characters.h"

namespace MM {
namespace MM1 {
namespace Views {

struct Dialogs {
private:
	Views::AreYouReady _areYouReady;
	Views::MainMenu _mainMenu;
	Views::Title _title;
	Views::ViewCharacters _characters;
	Views::ViewCharacter _character;
public:
	Dialogs() {}
};

} // namespace Views
} // namespace MM1
} // namespace MM

#endif
