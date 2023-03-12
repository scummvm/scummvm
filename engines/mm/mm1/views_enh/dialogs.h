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
#include "mm/mm1/views_enh/create_characters.h"
#include "mm/mm1/views/locations/inn.h"
#include "mm/mm1/views/protect.h"
#include "mm/mm1/views/title.h"
#include "mm/mm1/views_enh/character_info.h"
#include "mm/mm1/views_enh/character_select.h"
#include "mm/mm1/views_enh/characters.h"
#include "mm/mm1/views_enh/game.h"
#include "mm/mm1/views_enh/game_messages.h"
#include "mm/mm1/views_enh/main_menu.h"
#include "mm/mm1/views_enh/map_popup.h"
#include "mm/mm1/views_enh/quick_ref.h"
#include "mm/mm1/views_enh/search.h"
#include "mm/mm1/views_enh/title.h"
#include "mm/mm1/views_enh/trap.h"
#include "mm/mm1/views_enh/unlock.h"
#include "mm/mm1/views_enh/who_will_try.h"
#include "mm/mm1/views_enh/interactions/statue.h"
#include "mm/mm1/views_enh/locations/inn.h"
#include "mm/mm1/views_enh/locations/market.h"
#include "mm/mm1/views_enh/locations/tavern.h"
#include "mm/mm1/views_enh/locations/temple.h"
#include "mm/mm1/views_enh/locations/training.h"
#include "mm/mm1/views_enh/spells/cast_spell.h"
#include "mm/mm1/views_enh/spells/spellbook.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

struct Dialogs {
private:
	ViewsEnh::CreateCharacters _createCharacters;
	Views::Protect _protect;
	ViewsEnh::Title _title;
	ViewsEnh::Characters _characters;
	ViewsEnh::Interactions::Statue _statue;
	ViewsEnh::Locations::Inn _inn;
	ViewsEnh::Locations::Market _market;
	ViewsEnh::Search _search;
	ViewsEnh::Trap _trap;
	ViewsEnh::Unlock _unlock;
	ViewsEnh::WhoWillTry _whoWillTry;
	ViewsEnh::Locations::Tavern _tavern;
	ViewsEnh::Locations::Temple _temple;
	ViewsEnh::Locations::Training _training;
	ViewsEnh::Spells::CastSpell _castSpell;
	ViewsEnh::Spells::Spellbook _spellbook;
	ViewsEnh::CharacterInfo _characterInfo;
	ViewsEnh::CharacterSelect _characterSelect;
	ViewsEnh::Game _game;
	ViewsEnh::GameMessages _gameMessages;
	ViewsEnh::MainMenu _mainMenu;
	ViewsEnh::MapPopup _mapPopup;
	ViewsEnh::QuickRef _quickRef;
public:
	Dialogs() {}
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
