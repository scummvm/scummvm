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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "wage/wage.h"
#include "wage/entities.h"
#include "wage/randomhat.h"
#include "wage/world.h"

namespace Wage {

Obj *WageEngine::getOffer() {
	if (_offer != NULL) {
		Chr *owner = _offer->_currentOwner;
		if (owner == NULL || owner->_playerCharacter || owner->_currentScene != _world->_player->_currentScene) {
			_offer = NULL;
		}
	}
	return _offer;
}

Chr *WageEngine::getMonster() {
	if (_monster != NULL && _monster->_currentScene != _world->_player->_currentScene) {
		_monster = NULL;
	}
	return _monster;
}

void WageEngine::encounter(Chr *player, Chr *chr) {
	char buf[512];

	snprintf(buf, 512, "You encounter %s%s.", chr->_nameProperNoun ? "" : getIndefiniteArticle(chr->_name),
				chr->_name.c_str());
	appendText(buf);

	if (!chr->_initialComment.empty())
		appendText(chr->_initialComment);

	if (chr->_armor[Chr::HEAD_ARMOR] != NULL) {
		snprintf(buf, 512, "%s%s is wearing %s.", chr->getDefiniteArticle(true), chr->_name.c_str(),
					getIndefiniteArticle(chr->_armor[Chr::HEAD_ARMOR]->_name));
		appendText(buf);
	}
	if (chr->_armor[Chr::BODY_ARMOR] != NULL) {
		snprintf(buf, 512, "%s is protected by %s%s.", getGenderSpecificPronoun(chr->_gender, true),
					prependGenderSpecificPronoun(chr->_gender), chr->_armor[Chr::BODY_ARMOR]->_name.c_str());
		appendText(buf);
	}
	if (chr->_armor[Chr::SHIELD_ARMOR] != NULL) {
		Obj *obj = chr->_armor[Chr::SHIELD_ARMOR];

		snprintf(buf, 512, "%s carries %s%s.", getGenderSpecificPronoun(chr->_gender, true),
				obj->_namePlural ? "" : getIndefiniteArticle(obj->_name), obj->_name.c_str());
		appendText(buf);
	}
}

void WageEngine::performCombatAction(Chr *npc, Chr *player) {
	if (npc->_context._frozen)
		return;

	RandomHat hat(_rnd);

	bool winning = (npc->_context._statVariables[PHYS_HIT_CUR] > player->_context._statVariables[PHYS_HIT_CUR]);
	int validMoves = getValidMoveDirections(npc);
	ObjArray *weapons = npc->getWeapons(false);
	ObjArray *magics = npc->getMagicalObjects();
	// TODO: Figure out under what circumstances we need to add +1
	// for the chance (e.g. only when all values were set to 0?).
	if (winning) {
		if (!_world->_weaponMenuDisabled) {
			if (weapons->size() > 0)
				hat.addTokens(kTokWeapons, npc->_winningWeapons + 1);
			if (magics->size() > 0)
				hat.addTokens(kTokMagic, npc->_winningMagic);
		}
		if (validMoves != 0)
			hat.addTokens(kTokRun, npc->_winningRun + 1);
		if (npc->_inventory.size())
			hat.addTokens(kTokOffer, npc->_winningOffer + 1);
	} else {
		if (!_world->_weaponMenuDisabled) {
			if (weapons->size() > 0)
				hat.addTokens(kTokWeapons, npc->_losingWeapons + 1);
			if (magics->size() > 0)
				hat.addTokens(kTokMagic, npc->_losingMagic);
		}
		if (validMoves != 0)
			hat.addTokens(kTokRun, npc->_losingRun + 1);
		if (npc->_inventory.size())
			hat.addTokens(kTokOffer, npc->_losingOffer + 1);
	}

	ObjList *objs = &npc->_currentScene->_objs;
	if (npc->_inventory.size() < npc->_maximumCarriedObjects) {
		int cnt = 0;
		for (ObjList::const_iterator it = objs->begin(); it != objs->end(); ++it, ++cnt) {
			if ((*it)->_type != Obj::IMMOBILE_OBJECT) {
				// TODO: I'm not sure what the chance should be here.
				hat.addTokens(cnt, 123);
			}
		}
	}

	int token = hat.drawToken();
	switch (token) {
		case kTokWeapons:
			// TODO: I think the monster should choose the "best" weapon.
			performAttack(npc, player, weapons->operator[](_rnd->getRandomNumber(weapons->size() - 1)));
			break;
		case kTokMagic:
			// TODO: I think the monster should choose the "best" magic.
			performMagic(npc, player, magics->operator[](_rnd->getRandomNumber(magics->size() - 1)));
			break;
		case kTokRun:
			performMove(npc, validMoves);
			break;
		case kTokOffer:
			performOffer(npc, player);
			break;
		case kTokNone:
			break;
		default:
			{
				int cnt = 0;
				for (ObjList::const_iterator it = objs->begin(); it != objs->end(); ++it, ++cnt)
					if (cnt == token)
						performTake(npc, *it);
				break;
			}
	}

	delete weapons;
	delete magics;
}

void WageEngine::performAttack(Chr *attacker, Chr *victim, Obj *weapon) {
	warning("STUB: performAttack()");
}

void WageEngine::performMagic(Chr *attacker, Chr *victim, Obj *magicalObject) {
	warning("STUB: performMagic()");
}

void WageEngine::performMove(Chr *chr, int validMoves) {
	warning("STUB: performMove()");
}

void WageEngine::performOffer(Chr *attacker, Chr *victim) {
	/* TODO: choose in a smarter way? */
	Obj *obj = attacker->_inventory[0];
	char buf[512];

	snprintf(buf, 512, "%s%s offers %s%s.", attacker->getDefiniteArticle(true), attacker->_name.c_str(),
			obj->_namePlural ? "some " : getIndefiniteArticle(obj->_name), obj->_name.c_str());

	appendText(buf);

	_offer = obj;
}

void WageEngine::performTake(Chr *npc, Obj *obj) {
	char buf[512];

	snprintf(buf, 512, "%s%s picks up the %s%s.", npc->getDefiniteArticle(true), npc->_name.c_str(),
			getIndefiniteArticle(obj->_name), obj->_name.c_str());

	appendText(buf);

	_world->move(obj, npc);
}

int WageEngine::getValidMoveDirections(Chr *npc) {
	warning("STUB: getValidMoveDirections()");

	return 0;
}

void WageEngine::regen() {
	Chr *player = _world->_player;
	int curHp = player->_context._statVariables[PHYS_HIT_CUR];
	int maxHp = player->_context._statVariables[PHYS_HIT_BAS];
	int delta = maxHp - curHp;

	if (delta > 0) {
		int bonus = (int)(delta / (8 + _rnd->getRandomNumber(2)));
		player->_context._statVariables[PHYS_HIT_CUR] += bonus;
	}
}

} // End of namespace Wage
