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
#include "mm/mm1/views/bash.h"
#include "mm/mm1/views/title.h"
#include "mm/mm1/views_enh/character_info.h"
#include "mm/mm1/views_enh/character_inventory.h"
#include "mm/mm1/views_enh/character_select.h"
#include "mm/mm1/views_enh/characters.h"
#include "mm/mm1/views_enh/color_questions.h"
#include "mm/mm1/views_enh/combat.h"
#include "mm/mm1/views_enh/confirm.h"
#include "mm/mm1/views_enh/create_characters.h"
#include "mm/mm1/views_enh/dead.h"
#include "mm/mm1/views_enh/encounter.h"
#include "mm/mm1/views_enh/exchange.h"
#include "mm/mm1/views_enh/game.h"
#include "mm/mm1/views_enh/game_messages.h"
#include "mm/mm1/views_enh/main_menu.h"
#include "mm/mm1/views_enh/map_popup.h"
#include "mm/mm1/views_enh/protect.h"
#include "mm/mm1/views_enh/quick_ref.h"
#include "mm/mm1/views_enh/rest.h"
#include "mm/mm1/views_enh/search.h"
#include "mm/mm1/views_enh/title.h"
#include "mm/mm1/views_enh/trade.h"
#include "mm/mm1/views_enh/trap.h"
#include "mm/mm1/views_enh/unlock.h"
#include "mm/mm1/views_enh/wheel_spin.h"
#include "mm/mm1/views_enh/which_character.h"
#include "mm/mm1/views_enh/which_item.h"
#include "mm/mm1/views_enh/who_will_try.h"
#include "mm/mm1/views_enh/won_game.h"
#include "mm/mm1/views_enh/interactions/access_code.h"
#include "mm/mm1/views_enh/interactions/arenko.h"
#include "mm/mm1/views_enh/interactions/arrested.h"
#include "mm/mm1/views_enh/interactions/chess.h"
#include "mm/mm1/views_enh/interactions/giant.h"
#include "mm/mm1/views_enh/interactions/gypsy.h"
#include "mm/mm1/views_enh/interactions/ice_princess.h"
#include "mm/mm1/views_enh/interactions/prisoners.h"
#include "mm/mm1/views_enh/interactions/resistances.h"
#include "mm/mm1/views_enh/interactions/statue.h"
#include "mm/mm1/views_enh/interactions/trivia.h"
#include "mm/mm1/views_enh/interactions/volcano_god.h"
#include "mm/mm1/views_enh/locations/blacksmith_items.h"
#include "mm/mm1/views_enh/locations/blacksmith.h"
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
	ViewsEnh::Interactions::AccessCode _accessCode;
	ViewsEnh::Interactions::Arenko _arenko;
	ViewsEnh::Interactions::Arrested _arrested;
	ViewsEnh::Interactions::Chess _chess;
	ViewsEnh::Interactions::Giant _giant;
	ViewsEnh::Interactions::Gypsy _gypsy;
	ViewsEnh::Interactions::IcePrincess _icePrincess;
	ViewsEnh::Interactions::Resistances _resistances;
	ViewsEnh::Interactions::Statue _statue;
	ViewsEnh::Interactions::Trivia _trivia;
	ViewsEnh::Interactions::VolcanoGod _volcanoGod;

	ViewsEnh::Interactions::ChildPrisoner _childPrisoner;
	ViewsEnh::Interactions::CloakedPrisoner _cloakedPrisoner;
	ViewsEnh::Interactions::DemonPrisoner _demonPrisoner;
	ViewsEnh::Interactions::MaidenPrisoner _maidenPrisoner;
	ViewsEnh::Interactions::ManPrisoner _manPrisoner;
	ViewsEnh::Interactions::MutatedPrisoner _mutatedPrisoner;
	ViewsEnh::Interactions::VirginPrisoner _virginPrisoner;

	ViewsEnh::Locations::Blacksmith _blacksmith;
	ViewsEnh::Locations::BlacksmithItems _blacksmithItems;
	ViewsEnh::Locations::Inn _inn;
	ViewsEnh::Locations::Market _market;
	ViewsEnh::Locations::Tavern _tavern;
	ViewsEnh::Locations::Temple _temple;
	ViewsEnh::Locations::Training _training;
	ViewsEnh::Spells::CastSpell _castSpell;
	ViewsEnh::Spells::Spellbook _spellbook;
	ViewsEnh::CharacterInfo _characterInfo;
	ViewsEnh::CharacterInfoCombat _characterInfoCombat;
	ViewsEnh::CharacterInventory _characterInventory;
	ViewsEnh::CharacterSelect _characterSelect;
	ViewsEnh::Characters _characters;
	ViewsEnh::ColorQuestions _colorQuestions;
	ViewsEnh::Combat _combat;
	ViewsEnh::Confirm _confirm;
	ViewsEnh::CreateCharacters _createCharacters;
	ViewsEnh::Dead _dead;
	ViewsEnh::Encounter _encounter;
	ViewsEnh::Exchange _exchange;
	ViewsEnh::Game _game;
	ViewsEnh::GameMessages _gameMessages;
	ViewsEnh::MainMenu _mainMenu;
	ViewsEnh::MapPopup _mapPopup;
	ViewsEnh::Protect _protect;
	ViewsEnh::QuickRef _quickRef;
	ViewsEnh::Rest _rest;
	ViewsEnh::Search _search;
	ViewsEnh::Title _title;
	ViewsEnh::Trade _trade;
	ViewsEnh::Trap _trap;
	ViewsEnh::Unlock _unlock;
	ViewsEnh::WheelSpin _wheelSpin;
	ViewsEnh::WhichCharacter _whichCharacter;
	ViewsEnh::WhichItem _whichItem;
	ViewsEnh::WhoWillTry _whoWillTry;
	ViewsEnh::WonGame _wonGame;
	Views::Bash _bash;
public:
	Dialogs() {}
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
