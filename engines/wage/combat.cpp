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
		appendText(chr->_initialComment.c_str());

	if (chr->_armor[Chr::HEAD_ARMOR] != NULL) {
		snprintf(buf, 512, "%s%s is wearing %s%s.", chr->getDefiniteArticle(true), chr->_name.c_str(),
					getIndefiniteArticle(chr->_armor[Chr::HEAD_ARMOR]->_name), chr->_armor[Chr::HEAD_ARMOR]->_name.c_str());
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
	if (npc->_context._frozen) {
		npc->_context._freezeTimer--;
		if (npc->_context._freezeTimer <= 0) {
			npc->_context._frozen = false;
			npc->_context._freezeTimer = 0;
		} else {
			return;
		}
	}

	RandomHat hat(_rnd);

	bool winning = (npc->_context._statVariables[PHYS_HIT_CUR] > player->_context._statVariables[PHYS_HIT_CUR]);
	int validMoves = getValidMoveDirections(npc);
	ObjArray *weapons = npc->getWeapons(false);
	ObjArray *magics = npc->getMagicalObjects();
	// if all values are zero we add +1 so that an action has some chance of occuring even if
	// all stats have value zero, preventing the AI from getting stuck doing nothing
	if (winning) {
		if (!_world->_weaponMenuDisabled) {
			if (!weapons->empty())
				hat.addTokens(kTokWeapons, npc->_winningWeapons + 1);
			if (!magics->empty())
				hat.addTokens(kTokMagic, npc->_winningMagic);
		}
		if (validMoves != 0)
			hat.addTokens(kTokRun, npc->_winningRun + 1);
		if (!npc->_inventory.empty())
			hat.addTokens(kTokOffer, npc->_winningOffer + 1);
	} else {
		if (!_world->_weaponMenuDisabled) {
			if (!weapons->empty())
				hat.addTokens(kTokWeapons, npc->_losingWeapons + 1);
			if (!magics->empty())
				hat.addTokens(kTokMagic, npc->_losingMagic);
		}
		if (validMoves != 0)
			hat.addTokens(kTokRun, npc->_losingRun + 1);
		if (!npc->_inventory.empty())
			hat.addTokens(kTokOffer, npc->_losingOffer + 1);
	}

	// if an enemy has no weapons or magics then it should have a chance to stay idle
	// if we do not add this then an unarmed enemy will always keep running from room to room
	if (weapons->empty() && magics->empty())
		hat.addTokens(kTokNone, 1);

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
		performAttack(npc, player, weapons->operator[](_rnd->getRandomNumber(weapons->size() - 1)));
		break;
	case kTokMagic:
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
			for (ObjList::const_iterator it = objs->begin(); it != objs->end(); ++it, ++cnt) {
				if (cnt == token) {
					performTake(npc, *it);
					break;
				}
			}
		}
	}

	delete weapons;
	delete magics;
}

static const char *const targets[] = { "head", "chest", "side" };

void WageEngine::performAttack(Chr *attacker, Chr *victim, Obj *weapon) {
	if (_world->_weaponMenuDisabled)
		return;

	int targetIndex = -1;
	char buf[256];

	if (weapon->_type != Obj::MAGICAL_OBJECT) {
		if (attacker->_playerCharacter) {
			targetIndex = _aim;
		} else {
			targetIndex = _rnd->getRandomNumber(ARRAYSIZE(targets) - 1);
			_opponentAim = targetIndex + 1;
		}

		if (!attacker->_playerCharacter) {
			snprintf(buf, 256, "%s%s %ss %s%s at %s%s's %s.",
				attacker->getDefiniteArticle(true), attacker->_name.c_str(),
				weapon->_operativeVerb.c_str(),
				prependGenderSpecificPronoun(attacker->_gender), weapon->_name.c_str(),
				victim->getDefiniteArticle(true), victim->_name.c_str(),
				targets[targetIndex]);
			appendText(buf);
		}
	} else if (!attacker->_playerCharacter) {
		snprintf(buf, 256, "%s%s %ss %s%s at %s%s.",
			attacker->getDefiniteArticle(true), attacker->_name.c_str(),
			weapon->_operativeVerb.c_str(),
			prependGenderSpecificPronoun(attacker->_gender), weapon->_name.c_str(),
			victim->getDefiniteArticle(true), victim->_name.c_str());
		appendText(buf);
	}

	debugC(1, kDebugSound, "** Weapon sound: %s", weapon->_sound.c_str());

	playSound(weapon->_sound);

	bool usesDecremented = false;
	int chance = _rnd->getRandomNumber(255);
	// average of character skill + weapon accuracy
	int hitAccuracy = (attacker->_physicalAccuracy + weapon->_accuracy) / 2;
	if (chance < hitAccuracy) {
		usesDecremented = attackHit(attacker, victim, weapon, targetIndex);
	} else if (weapon->_type != Obj::MAGICAL_OBJECT) {
		appendText("A miss!");
	} else if (attacker->_playerCharacter) {
		appendText("The spell has no effect.");
	}

	if (!usesDecremented) {
		decrementUses(weapon);
	}
}

void WageEngine::decrementUses(Obj *obj) {
	int numberOfUses = obj->_numberOfUses;
	if (numberOfUses != -1) {
		numberOfUses--;
		if (numberOfUses > 0) {
			obj->_numberOfUses = numberOfUses;
		} else {
			if (!obj->_failureMessage.empty()) {
				appendText(obj->_failureMessage.c_str());
			}
			if (obj->_returnToRandomScene) {
				_world->move(obj, _world->getRandomScene());
			} else {
				_world->move(obj, _world->_storageScene);
			}
			obj->resetState(obj->_currentOwner, obj->_currentScene);
		}
	}
}

bool WageEngine::attackHit(Chr *attacker, Chr *victim, Obj *weapon, int targetIndex) {
	bool receivedHitTextPrinted = false;
	char buf[512];
	int damage = 0;
	int defense = 0;

	if (weapon->_type == Obj::MAGICAL_OBJECT) {
		// Damage = (Spell Power + Spiritual Strength) - 1
		damage = (weapon->_damage + attacker->_spiritualStength) - 1;

		// if victim has 0 or less Spiritual HP, any spell power kills them instantly
		// we override defense to ensure the damage goes through
		if (victim->_context._statVariables[SPIR_HIT_CUR] <= 0 && damage > 0) {
			damage = 9999; // massive damage to force kill
			defense = 0;
		} else {
			// Defense = Resistance + Spiritual Armor
			defense = victim->_resistanceToMagic;

			// check all equipped items for spiritual armor
			for (int i = 0; i < Chr::NUMBER_OF_ARMOR_TYPES; i++) {
				Obj *arm = victim->_armor[i];
				if (arm != NULL && arm->_type == Obj::SPIRITUAL_ARMOR) {
					defense += arm->_damage;
				}
			}
		}

		appendText(weapon->_useMessage.c_str());
		appendText("The spell is effective!");

	} else {
		// Damage = (Weapon Power + Phys Strength) - 1
		damage = (weapon->_damage + attacker->_physicalStrength) - 1;

		// Defense = Natural Armor + Item Armor
		defense = victim->_naturalArmor;

		if (targetIndex != -1) {
			Obj *armor = victim->_armor[targetIndex];
			if (armor != NULL) {
				defense += armor->_damage;

				snprintf(buf, 512, "%s%s's %s weakens the impact of %s%s's %s.",
						 victim->getDefiniteArticle(true), victim->_name.c_str(),
						 victim->_armor[targetIndex]->_name.c_str(),
						 attacker->getDefiniteArticle(false), attacker->_name.c_str(),
						 weapon->_name.c_str());
				appendText(buf);
				decrementUses(armor);
			} else {
				snprintf(buf, 512, "A hit to the %s!", targets[targetIndex]);
				appendText(buf);
			}
		}

		debugC(1, kDebugSound, "** Attacker hit sound: %s", attacker->_scoresHitSound.c_str());
		playSound(attacker->_scoresHitSound);
		appendText(attacker->_scoresHitComment.c_str());
		debugC(1, kDebugSound, "** Victim receives sound: %s", victim->_receivesHitSound.c_str());
		playSound(victim->_receivesHitSound);
		appendText(victim->_receivesHitComment.c_str());
		receivedHitTextPrinted = true;
	}

	if (damage < 0)
		damage = 0;

	// apply damage
	bool causesPhysicalDamage = true;
	bool causesSpiritualDamage = false;
	bool freezesOpponent = false;
	bool usesDecremented = false;

	if (weapon->_type == Obj::THROW_WEAPON) {
		_world->move(weapon, victim->_currentScene);
	} else if (weapon->_type == Obj::MAGICAL_OBJECT) {
		int type = weapon->_attackType;
		causesPhysicalDamage = (type == Obj::CAUSES_PHYSICAL_DAMAGE || type == Obj::CAUSES_PHYSICAL_AND_SPIRITUAL_DAMAGE);
		causesSpiritualDamage = (type == Obj::CAUSES_SPIRITUAL_DAMAGE || type == Obj::CAUSES_PHYSICAL_AND_SPIRITUAL_DAMAGE);
		freezesOpponent = (type == Obj::FREEZES_OPPONENT);
	}

	// calculate net damage
	int damageTaken = damage - defense;
	if (damageTaken < 0)
		damageTaken = 0;

	// apply to hit Points
	if (causesPhysicalDamage)
		victim->_context._statVariables[PHYS_HIT_CUR] -= damageTaken;
	if (causesSpiritualDamage)
		victim->_context._statVariables[SPIR_HIT_CUR] -= damageTaken;

	decrementUses(weapon);
	usesDecremented = true;

	// death check
	// if either physical or spiritual HP drops < 0, the character dies.
	if (victim->_context._statVariables[PHYS_HIT_CUR] < 0 ||
		victim->_context._statVariables[SPIR_HIT_CUR] < 0) {

		debugC(1, kDebugSound, "** Victim dying sound: %s", victim->_dyingSound.c_str());
		playSound(victim->_dyingSound);
		appendText(victim->_dyingWords.c_str());
		snprintf(buf, 512, "%s%s is dead!", victim->getDefiniteArticle(true), victim->_name.c_str());
		appendText(buf);

		attacker->_context._kills++;
		attacker->_context._experience += victim->_context._statVariables[SPIR_HIT_BAS] + victim->_context._statVariables[PHYS_HIT_BAS];

		// drop items if not player
		if (!victim->_playerCharacter && !victim->_inventory.empty()) {
			Scene *currentScene = victim->_currentScene;
			for (int i = victim->_inventory.size() - 1; i >= 0; i--) {
				if (i < (int)victim->_inventory.size())
					_world->move(victim->_inventory[i], currentScene);
			}
			Common::String *s = getGroundItemsList(currentScene);
			appendText(s->c_str());
			delete s;
		}
		_world->move(victim, _world->_storageScene);
	} else if (attacker->_playerCharacter && !receivedHitTextPrinted && weapon->_type != Obj::MAGICAL_OBJECT) {
		double physicalPercent = (double)victim->_context._statVariables[PHYS_HIT_CUR] /
								 victim->_context._statVariables[PHYS_HIT_BAS];
		snprintf(buf, 512, "%s%s's condition appears to be %s.",
				 victim->getDefiniteArticle(true), victim->_name.c_str(),
				 getPercentMessage(physicalPercent));
		appendText(buf);
	}

	// freeze Logic (No. Of Turns = Power / 50)
	if (freezesOpponent) {
		victim->_context._frozen = true;
		int duration = 0;
		if (weapon->_damage > 0)
			duration = weapon->_damage / 50;

		victim->_context._freezeTimer = duration;
	}

	return usesDecremented;
}

void WageEngine::performMagic(Chr *attacker, Chr *victim, Obj *magicalObject) {
	switch (magicalObject->_attackType) {
	case Obj::HEALS_PHYSICAL_DAMAGE:
	case Obj::HEALS_SPIRITUAL_DAMAGE:
	case Obj::HEALS_PHYSICAL_AND_SPIRITUAL_DAMAGE:
		performHealingMagic(attacker, magicalObject);
		break;
	default:
		break;
	}

	char buf[256];
	if (!attacker->_playerCharacter) {
		snprintf(buf, 256, "%s%s %ss %s%s at %s%s.",
				 attacker->getDefiniteArticle(true), attacker->_name.c_str(),
				 magicalObject->_operativeVerb.c_str(),
				 getIndefiniteArticle(magicalObject->_name), magicalObject->_name.c_str(),
				 victim->getDefiniteArticle(true), victim->_name.c_str());
		appendText(buf);
	}

	debugC(1, kDebugSound, "** Magic sound: %s", magicalObject->_sound.c_str());
	playSound(magicalObject->_sound);

	int magicAcc = (attacker->_spiritualAccuracy + magicalObject->_accuracy) / 2;
	int chance = _rnd->getRandomNumber(255);

	// pass -1 to indicate a non physical hit
	if (chance < magicAcc) 
		attackHit(attacker, victim, magicalObject, -1);
	else if (attacker->_playerCharacter) 
		appendText("The spell has no effect.");
	else 
		appendText("A miss!");

	decrementUses(magicalObject);
}

void WageEngine::performHealingMagic(Chr *chr, Obj *magicalObject) {
	char buf[512];

	if (!chr->_playerCharacter) {
		snprintf(buf, 512, "%s%s %ss %s%s.",
			chr->getDefiniteArticle(true), chr->_name.c_str(),
			magicalObject->_operativeVerb.c_str(),
			getIndefiniteArticle(magicalObject->_name), magicalObject->_name.c_str());
		appendText(buf);
	}

	uint chance = _rnd->getRandomNumber(255);
	if (chance < magicalObject->_accuracy) {
		int type = magicalObject->_attackType;
		int power = magicalObject->_damage;

		if (type == Obj::HEALS_PHYSICAL_DAMAGE || type == Obj::HEALS_PHYSICAL_AND_SPIRITUAL_DAMAGE) {
			int current = chr->_context._statVariables[PHYS_HIT_CUR];
			int max = chr->_context._statVariables[PHYS_HIT_BAS];

			// do not exceed base HP
			if (current < max) {
				current += power;
				if (current > max)
					current = max;
				chr->_context._statVariables[PHYS_HIT_CUR] = current;
			}
		}

		if (type == Obj::HEALS_SPIRITUAL_DAMAGE || type == Obj::HEALS_PHYSICAL_AND_SPIRITUAL_DAMAGE) {
			int current = chr->_context._statVariables[SPIR_HIT_CUR];
			int max = chr->_context._statVariables[SPIR_HIT_BAS];

			// do not exceed base HP
			if (current < max) {
				current += power;
				if (current > max)
					current = max;
				chr->_context._statVariables[SPIR_HIT_CUR] = current;
			}
		}

		debugC(1, kDebugSound, "** Magical object sound: %s", magicalObject->_sound.c_str());
		playSound(magicalObject->_sound);
		appendText(magicalObject->_useMessage.c_str());

		if (chr->_playerCharacter) {
			double physicalPercent = (double)chr->_context._statVariables[PHYS_HIT_CUR] / chr->_context._statVariables[PHYS_HIT_BAS];
			double spiritualPercent = (double)chr->_context._statVariables[SPIR_HIT_CUR] / chr->_context._statVariables[SPIR_HIT_BAS];
			snprintf(buf, 256, "Your physical condition is %s.", getPercentMessage(physicalPercent));
			appendText(buf);

			snprintf(buf, 256, "Your spiritual condition is %s.", getPercentMessage(spiritualPercent));
			appendText(buf);
		}
	}

	decrementUses(magicalObject);
}

static const int directionsX[] = { 0, 0, 1, -1 };
static const int directionsY[] = { -1, 1, 0, 0 };
static const char *const directionsS[] = { "north", "south", "east", "west" };

void WageEngine::performMove(Chr *chr, int validMoves) {
	// if there are no open exits (validMoves is 0), the enemy is trapped.
	// they cannot run, and therefore cannot escape.
	if (validMoves == 0) {
		return;
	}

	// we only choose from directions that are not blocked.
	int numValidMoves = 0;
	for (int i = 0; i < 4; i++)
		if ((validMoves & (1 << i)) != 0)
			numValidMoves++;

	int dirNum = _rnd->getRandomNumber(numValidMoves - 1);
	int dir = -1;

	for (int i = 0; i < 4; i++) {
		if ((validMoves & (1 << i)) != 0) {
			if (dirNum == 0) {
				dir = i;
				break;
			}
			dirNum--;
		}
	}

	// The enemy always runs to the next room first.
	Scene *currentScene = chr->_currentScene;
	int destX = currentScene->_worldX + directionsX[dir];
	int destY = currentScene->_worldY + directionsY[dir];
	Scene *destScene = _world->getSceneAt(destX, destY);

	if (destScene != NULL) {
		char buf[256];
		// prints: "The Wuggly Ump runs West."
		snprintf(buf, 256, "%s%s runs %s.", chr->getDefiniteArticle(true), chr->_name.c_str(), directionsS[dir]);
		appendText(buf);

		_running = chr;

		// the enemy is in the new room.
		_world->move(chr, destScene);
	}

	// now that they are in the new room, we check if they can "Escape".
	if (chr->_runningSpeed > 0) {
		int playerSpeed = _world->_player->_runningSpeed;
		int enemySpeed = chr->_runningSpeed;
		int totalSpeed = playerSpeed + enemySpeed;

		// Stop Chance = Player Speed / (Player Speed + Enemy Speed)
		int stopChance = 0;
		if (totalSpeed > 0) {
			stopChance = (playerSpeed * 100) / totalSpeed;
		}

		// if rand(0, 99) >= stopChance, player failed to stop them.
		// result: enemy vanishes from the room they just entered (escapes).
		if (_rnd->getRandomNumber(99) >= stopChance) {
			_world->move(chr, _world->_storageScene);
		}
	}
}

void WageEngine::performOffer(Chr *attacker, Chr *victim) {
	// pick a random object from inventory
	int r = _rnd->getRandomNumber(attacker->_inventory.size() - 1);
	Obj *obj = attacker->_inventory[r];
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
	int directions = 0;
	Scene *currentScene = npc->_currentScene;
	for (int dir = 0; dir < 4; dir++) {
		if (!currentScene->_blocked[dir]) {
			int destX = currentScene->_worldX + directionsX[dir];
			int destY = currentScene->_worldY + directionsY[dir];

			Scene *scene = _world->getSceneAt(destX, destY);

			if (scene != NULL && scene->_chrs.empty()) {
				directions |= (1 << dir);
			}
		}
	}

	return directions;
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

void WageEngine::takeObj(Obj *obj) {
	if (_world->_player->_inventory.size() >= _world->_player->_maximumCarriedObjects) {
		appendText("Your pack is full, you must drop something.");
	} else {
		char buf[256];

		_world->move(obj, _world->_player);
		int type = _world->_player->wearObjIfPossible(obj);
		if (type == Chr::BODY_ARMOR || type == Chr::SHIELD_ARMOR || type == Chr::MAGIC_ARMOR) {
			snprintf(buf, 256, "You are now wearing the %s.", obj->_name.c_str());
			appendText(buf);
		} else {
			snprintf(buf, 256, "You now have the %s.", obj->_name.c_str());
			appendText(buf);
		}
		appendText(obj->_clickMessage.c_str());
	}
}

bool WageEngine::handleMoveCommand(Directions dir, const char *dirName) {
	Scene *playerScene = _world->_player->_currentScene;
	const char *msg = playerScene->_messages[dir].c_str();

	if (!playerScene->_blocked[dir]) {
		int destX = playerScene->_worldX + directionsX[dir];
		int destY = playerScene->_worldY + directionsY[dir];

		Scene *scene = _world->getSceneAt(destX, destY);

		if (scene != NULL) {
			if (strlen(msg) > 0) {
				appendText(msg);
			}
			_soundQueue.clear();
			_world->move(_world->_player, scene);
			return true;
		}
	}
	if (strlen(msg) > 0) {
		appendText(msg);
	} else {
		Common::String txt("You can't go ");
		txt += dirName;
		txt += ".";
		appendText(txt.c_str());
	}

	return true;
}

bool WageEngine::handleLookCommand() {
	appendText(_world->_player->_currentScene->_text.c_str());

	Common::String *items = getGroundItemsList(_world->_player->_currentScene);
	if (items != NULL) {
		appendText(items->c_str());

		delete items;
	}

	return true;
}

Common::String *WageEngine::getGroundItemsList(Scene *scene) {
	ObjArray objs;

	for (ObjList::const_iterator it = scene->_objs.begin(); it != scene->_objs.end(); ++it)
		if ((*it)->_type != Obj::IMMOBILE_OBJECT)
			objs.push_back(*it);

	if (!objs.empty()) {
		Common::String *res = new Common::String("On the ground you see ");
		appendObjNames(*res, objs);
		return res;
	}
	return NULL;
}

void WageEngine::appendObjNames(Common::String &str, const ObjArray &objs) {
	for (uint i = 0; i < objs.size(); i++) {
		Obj *obj = objs[i];

		if (!obj->_namePlural)
			str += getIndefiniteArticle(obj->_name);
		else
			str += "some ";

		str += obj->_name;

		if (i == objs.size() - 1) {
			str += ".";
		} else if (i == objs.size() - 2) {
			if (objs.size() > 2)
				str += ",";
			str += " and ";
		} else {
			str += ", ";
		}
	}
}

bool WageEngine::handleInventoryCommand() {
	Chr *player = _world->_player;
	ObjArray objs;

	for (ObjArray::const_iterator it = player->_inventory.begin(); it != player->_inventory.end(); ++it)
		if (!player->isWearing(*it))
			objs.push_back(*it);

	if (objs.empty()) {
		appendText("Your pack is empty.");
	} else {
		Common::String res("Your pack contains ");
		appendObjNames(res, objs);
		appendText(res.c_str());
	}

	return true;
}

static const char *const armorMessages[] = {
	"Head protection:",
	"Chest protection:",
	"Side protection:",
	"Magical protection:"
};

bool WageEngine::handleStatusCommand() {
	Chr *player = _world->_player;
	char buf[512];

	snprintf(buf, 512, "Character name: %s%s", player->getDefiniteArticle(false), player->_name.c_str());
	appendText(buf);
	snprintf(buf, 512, "Experience: %d", player->_context._experience);
	appendText(buf);

	int wealth = 0;
	for (ObjArray::const_iterator it = player->_inventory.begin(); it != player->_inventory.end(); ++it)
		wealth += (*it)->_value;

	snprintf(buf, 512, "Wealth: %d", wealth);
	appendText(buf);

	for (int i = 0; i < Chr::NUMBER_OF_ARMOR_TYPES; i++) {
		if (player->_armor[i] != NULL) {
			snprintf(buf, 512, "%s %s", armorMessages[i], player->_armor[i]->_name.c_str());
			appendText(buf);
		}
	}

	for (ObjArray::const_iterator it = player->_inventory.begin(); it != player->_inventory.end(); ++it) {
		int uses = (*it)->_numberOfUses;

		if (uses > 0) {
			snprintf(buf, 512, "Your %s has %d uses left.", (*it)->_name.c_str(), uses);
			appendText(buf);
		}
	}

	printPlayerCondition(player);

	_commandWasQuick = true;

	return true;
}

bool WageEngine::handleRestCommand() {
	if (getMonster() != NULL) {
		appendText("This is no time to rest!");
		_commandWasQuick = true;
	} else {
		regen();
		printPlayerCondition(_world->_player);
	}

	return true;
}

bool WageEngine::handleAcceptCommand() {
	Chr *chr = _offer->_currentOwner;

	char buf[512];
	snprintf(buf, 512, "%s%s lays the %s on the ground and departs peacefully.",
		chr->getDefiniteArticle(true), chr->_name.c_str(), _offer->_name.c_str());
	appendText(buf);

	_world->move(_offer, chr->_currentScene);
	_world->move(chr, _world->_storageScene);

	return true;
}

bool WageEngine::handleTakeCommand(const char *target) {
	Common::String t(target);
	bool handled = false;

	for (ObjList::const_iterator it = _world->_player->_currentScene->_objs.begin(); it != _world->_player->_currentScene->_objs.end(); ++it) {
		Common::String n((*it)->_name);
		n.toLowercase();

		if (t.contains(n)) {
			if ((*it)->_type == Obj::IMMOBILE_OBJECT) {
				appendText("You can't move it.");
			} else {
				takeObj(*it);
			}

			handled = true;
			break;
		}
	}

	return handled;
}

bool WageEngine::handleDropCommand(const char *target) {
	Common::String t(target);
	bool handled = false;

	t.toLowercase();

	for (ObjArray::const_iterator it = _world->_player->_inventory.begin(); it != _world->_player->_inventory.end(); ++it) {
		Common::String n((*it)->_name);
		n.toLowercase();

		if (t.contains(n)) {
			char buf[256];

			snprintf(buf, 256, "You no longer have the %s.", (*it)->_name.c_str());
			appendText(buf);
			_world->move(*it, _world->_player->_currentScene);

			handled = true;
			break;
		}
	}

	return handled;
}

bool WageEngine::handleAimCommand(const char *t) {
	bool wasHandled = true;
	Common::String target(t);

	target.toLowercase();

	if (target.contains("head")) {
		_aim = Chr::HEAD;
	} else if (target.contains("chest")) {
		_aim = Chr::CHEST;
	} else if (target.contains("side")) {
		_aim = Chr::SIDE;
	} else {
		wasHandled = false;
		appendText("Please aim for the head, chest, or side.");
	}

	_commandWasQuick = true;

	return wasHandled;
}

bool WageEngine::handleWearCommand(const char *t) {
	Chr *player = _world->_player;
	char buf[512];
	Common::String target(t);
	bool handled = false;

	target.toLowercase();

	for (ObjArray::const_iterator it = _world->_player->_inventory.begin(); it != _world->_player->_inventory.end(); ++it) {
		Common::String n((*it)->_name);

		if (target.contains(n)) {
			if ((*it)->_type == Obj::HELMET) {
				wearObj(*it, Chr::HEAD_ARMOR);
			} else if ((*it)->_type == Obj::CHEST_ARMOR) {
				wearObj(*it, Chr::BODY_ARMOR);
			} else if ((*it)->_type == Obj::SHIELD) {
				wearObj(*it, Chr::SHIELD_ARMOR);
			} else if ((*it)->_type == Obj::SPIRITUAL_ARMOR) {
				wearObj(*it, Chr::MAGIC_ARMOR);
			} else {
				appendText("You cannot wear that object.");
			}

			handled = true;
			break;
		}
	}

	for (ObjList::const_iterator it = player->_currentScene->_objs.begin(); it != player->_currentScene->_objs.end(); ++it) {
		Common::String n((*it)->_name);
		n.toLowercase();
		if (target.contains(n)) {
			snprintf(buf, 512, "First you must get the %s.", (*it)->_name.c_str());
			appendText(buf);

			handled = true;
			break;
		}
	}

	return handled;
}

void WageEngine::wearObj(Obj *o, int pos) {
	Chr *player = _world->_player;
	char buf[512];

	if (player->_armor[pos] == o) {
		snprintf(buf, 512, "You are already wearing the %s.", o->_name.c_str());
		appendText(buf);
	} else {
		if (player->_armor[pos] != NULL) {
			snprintf(buf, 512, "You are no longer wearing the %s.", player->_armor[pos]->_name.c_str());
			appendText(buf);
		}

		player->_armor[pos] = o;
		snprintf(buf, 512, "You are now wearing the %s.", o->_name.c_str());
		appendText(buf);
	}
}


bool WageEngine::handleOfferCommand(const char *target) {
	Chr *player = _world->_player;
	Chr *enemy = getMonster();

	if (enemy != NULL) {
		Common::String t(target);
		t.toLowercase();

		for (ObjArray::const_iterator it = player->_inventory.begin(); it != player->_inventory.end(); ++it) {
			Common::String n((*it)->_name);
			n.toLowercase();

			if (t.contains(n)) {
				if ((*it)->_value < enemy->_rejectsOffers) {
					appendText("Your offer is rejected.");
				} else {
					appendText("Your offer is accepted.");
					appendText(enemy->_acceptsOfferComment.c_str());
					_world->move(*it, enemy);
					_world->move(enemy, _world->_storageScene);
				}

				return true;
			}
		}
	}

	return false;
}

bool WageEngine::tryAttack(const Obj *weapon, const Common::String &input) {
	Common::String w(weapon->_name);
	w.toLowercase();
	Common::String i(input);
	i.toLowercase();
	Common::String v(weapon->_operativeVerb);
	v.toLowercase();

	return i.contains(w) && i.contains(v);
}

bool WageEngine::handleAttack(Obj *weapon) {
	Chr *player = _world->_player;
	Chr *enemy = getMonster();

	if (weapon->_type == Obj::MAGICAL_OBJECT) {
		switch (weapon->_attackType) {
		case Obj::HEALS_PHYSICAL_AND_SPIRITUAL_DAMAGE:
		case Obj::HEALS_PHYSICAL_DAMAGE:
		case Obj::HEALS_SPIRITUAL_DAMAGE:
			performMagic(player, enemy, weapon);
			return true;
		default:
			break;
		}
	}
	if (enemy != NULL)
		performAttack(player, enemy, weapon);
	else if (weapon->_type == Obj::MAGICAL_OBJECT)
		appendText("There is nobody to cast a spell at.");
	else
		appendText("There is no one to fight.");

	return true;
}

const char *WageEngine::getPercentMessage(double percent) {
	if (percent < 0.40) {
		return "very bad";
	} else if (percent < 0.55) {
		return "bad";
	} else if (percent < 0.70) {
		return "average";
	} else if (percent < 0.85) {
		return "good";
	} else if (percent <= 1.00) {
		return "very good";
	} else {
		return "enhanced";
	}
}

void WageEngine::printPlayerCondition(Chr *player) {
	double physicalPercent = (double)player->_context._statVariables[PHYS_HIT_CUR] / player->_context._statVariables[PHYS_HIT_BAS];
	double spiritualPercent = (double)player->_context._statVariables[SPIR_HIT_CUR] / player->_context._statVariables[SPIR_HIT_BAS];
	char buf[256];

	snprintf(buf, 256, "Your physical condition is %s.", getPercentMessage(physicalPercent));
	appendText(buf);

	snprintf(buf, 256, "Your spiritual condition is %s.", getPercentMessage(spiritualPercent));
	appendText(buf);
}

} // End of namespace Wage
