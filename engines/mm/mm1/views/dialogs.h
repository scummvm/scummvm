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
#include "mm/mm1/views/are_you_ready.h"
#include "mm/mm1/views/bash.h"
#include "mm/mm1/views/characters.h"
#include "mm/mm1/views/character_view_combat.h"
#include "mm/mm1/views/combat.h"
#include "mm/mm1/views/create_characters.h"
#include "mm/mm1/views/dead.h"
#include "mm/mm1/views/encounter.h"
#include "mm/mm1/views/game.h"
#include "mm/mm1/views/main_menu.h"
#include "mm/mm1/views/order.h"
#include "mm/mm1/views/protect.h"
#include "mm/mm1/views/quick_ref.h"
#include "mm/mm1/views/title.h"
#include "mm/mm1/views/character_info.h"
#include "mm/mm1/views/character_view_combat.h"
#include "mm/mm1/views/characters.h"
#include "mm/mm1/views/rest.h"
#include "mm/mm1/views/search.h"
#include "mm/mm1/views/trap.h"
#include "mm/mm1/views/unlock.h"
#include "mm/mm1/views/wheel_spin.h"
#include "mm/mm1/views/locations/blacksmith.h"
#include "mm/mm1/views/locations/inn.h"
#include "mm/mm1/views/locations/market.h"
#include "mm/mm1/views/locations/statue.h"
#include "mm/mm1/views/locations/tavern.h"
#include "mm/mm1/views/locations/temple.h"
#include "mm/mm1/views/locations/training.h"
#include "mm/mm1/views/maps/access_code.h"
#include "mm/mm1/views/maps/alamar.h"
#include "mm/mm1/views/maps/alien.h"
#include "mm/mm1/views/maps/arrested.h"
#include "mm/mm1/views/maps/chess.h"
#include "mm/mm1/views/maps/dog_statue.h"
#include "mm/mm1/views/maps/ghost.h"
#include "mm/mm1/views/maps/giant.h"
#include "mm/mm1/views/maps/gypsy.h"
#include "mm/mm1/views/maps/hacker.h"
#include "mm/mm1/views/maps/ice_princess.h"
#include "mm/mm1/views/maps/inspectron.h"
#include "mm/mm1/views/maps/keeper.h"
#include "mm/mm1/views/maps/lion.h"
#include "mm/mm1/views/maps/lord_archer.h"
#include "mm/mm1/views/maps/lord_ironfist.h"
#include "mm/mm1/views/maps/orango.h"
#include "mm/mm1/views/maps/prisoners.h"
#include "mm/mm1/views/maps/resistances.h"
#include "mm/mm1/views/maps/ruby.h"
#include "mm/mm1/views/maps/trivia.h"
#include "mm/mm1/views/maps/volcano_god.h"
#include "mm/mm1/views/maps/won_game.h"
#include "mm/mm1/views/spells/cast_spell.h"
#include "mm/mm1/views/spells/detect_magic.h"
#include "mm/mm1/views/spells/duplication.h"
#include "mm/mm1/views/spells/fly.h"
#include "mm/mm1/views/spells/location.h"
#include "mm/mm1/views/spells/recharge_item.h"
#include "mm/mm1/views/spells/teleport.h"

namespace MM {
namespace MM1 {
namespace Views {

struct Dialogs {
private:
	Views::AreYouReady _areYouReady;
	Views::Bash _bash;
	Views::CharacterInfo _characterInfo;
	Views::Characters _characters;
	Views::CharacterViewCombat _characterViewCombat;
	Views::Combat _combat;
	Views::CreateCharacters _createCharacters;
	Views::Dead _dead;
	Views::Encounter _encounter;
	Views::Game _game;
	Views::MainMenu _mainMenu;
	Views::Order _order;
	Views::Protect _protect;
	Views::QuickRef _quickRef;
	Views::Rest _rest;
	Views::Search _search;
	Views::Title _title;
	Views::Trap _trap;
	Views::Unlock _unlock;
	Views::WheelSpin _wheelSpin;

	Views::Locations::Blacksmith _blacksmith;
	Views::Locations::Inn _inn;
	Views::Locations::Market _market;
	Views::Locations::Statue _statue;
	Views::Locations::Tavern _tavern;
	Views::Locations::Temple _temple;
	Views::Locations::Training _training;

	Views::Maps::AccessCode _accessCode;
	Views::Maps::Alamar _alamar;
	Views::Maps::Alien _alien;
	Views::Maps::Arrested _arrested;
	Views::Maps::Chess _chess;
	Views::Maps::ChildPrisoner _childPrisoner;
	Views::Maps::CloakedPrisoner _cloakedPrisoner;
	Views::Maps::DemonPrisoner _demonPrisoner;
	Views::Maps::MaidenPrisoner _maidenPrisoner;
	Views::Maps::ManPrisoner _manPrisoner;
	Views::Maps::MutatedPrisoner _mutatedPrisoner;
	Views::Maps::VirginPrisoner _virginPrisoner;
	Views::Maps::DogStatue _dogStatue;
	Views::Maps::Ghost _ghost;
	Views::Maps::Giant _giant;
	Views::Maps::Gypsy _gypsy;
	Views::Maps::Hacker _hacker;
	Views::Maps::IcePrincess _icePrincess;
	Views::Maps::Inspectron _inspectron;
	Views::Maps::Keeper _keeper;
	Views::Maps::Lion _lion;
	Views::Maps::LordArcher _lordArcher;
	Views::Maps::LordIronfist _lordIronfist;
	Views::Maps::Orango _orango;
	Views::Maps::Resistances _resistances;
	Views::Maps::Ruby _ruby;
	Views::Maps::Trivia _trivia;
	Views::Maps::VolcanoGod _volcanoGod;
	Views::Maps::WonGame _wonGame;

	Views::Spells::CastSpell _castSpell;
	Views::Spells::DetectMagic _detectMagicSpell;
	Views::Spells::Duplication _duplicationSpell;
	Views::Spells::Fly _flySpell;
	Views::Spells::Location _locationSpell;
	Views::Spells::RechargeItem _rechargeItemSpell;
	Views::Spells::Teleport _telportSpell;
public:
	Dialogs() {}
};

} // namespace Views
} // namespace MM1
} // namespace MM

#endif
