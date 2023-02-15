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

#ifndef MM1_VIEWS_ENH_DIALOGS_H
#define MM1_VIEWS_ENH_DIALOGS_H

#include "mm/mm1/events.h"
#include "mm/mm1/views/create_characters.h"
#include "mm/mm1/views/locations/inn.h"
#include "mm/mm1/views/protect.h"
#include "mm/mm1/views/title.h"
#include "mm/mm1/views_enh/cast_spell.h"
#include "mm/mm1/views_enh/character_info.h"
#include "mm/mm1/views_enh/game.h"
#include "mm/mm1/views_enh/main_menu.h"
#include "mm/mm1/views_enh/map_popup.h"
#include "mm/mm1/views_enh/quick_ref.h"
#include "mm/mm1/views_enh/locations/market.h"
#include "mm/mm1/views_enh/locations/temple.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

struct Dialogs {
private:
	ViewsEnh::CharacterInfo _characterInfo;
	ViewsEnh::Game _game;

	// Unenhanced views
	Views::Characters _characters;
	Views::CreateCharacters _createCharacters;
	Views::Locations::Inn _inn;
	Views::Protect _protect;
	Views::Title _title;
//	Views::ViewCharacter _viewCharacter;
	ViewsEnh::CastSpell _castSpell;
	ViewsEnh::MainMenu _mainMenu;
	ViewsEnh::MapPopup _mapPopup;
	ViewsEnh::QuickRef _quickRef;
	ViewsEnh::Locations::Market _market;
	ViewsEnh::Locations::Temple _temple;
public:
	Dialogs() {}
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
