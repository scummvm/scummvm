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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "xeen/spells.h"
#include "xeen/files.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

Spells::Spells(XeenEngine *vm) : _vm(vm) {
	_lastCaster = 0;

	load();
}

void Spells::load() {
	File f1("spells.xen");
	while (f1.pos() < f1.size())
		_spellNames.push_back(f1.readString());
	f1.close();
}

int Spells::calcSpellCost(int spellId, int expenseFactor) const {
	int amount = SPELL_COSTS[spellId];
	return (amount >= 0) ? (amount * 100) << expenseFactor :
		(amount * -500) << expenseFactor;
}

int Spells::calcSpellPoints(int spellId, int expenseFactor) const {
	int amount = SPELL_COSTS[spellId];
	return (amount >= 0) ? amount : amount * -1 * expenseFactor;
}

typedef void(Spells::*SpellMethodPtr)();

void Spells::doSpell(int spellId) {
	static const SpellMethodPtr SPELL_LIST[73] = {
		&Spells::light, &Spells::awaken, &Spells::magicArrow, &Spells::firstAid,
		&Spells::flyingFist, &Spells::energyBlast, &Spells::sleep,
		&Spells::revitalize, &Spells::cureWounds, &Spells::sparks,

		&Spells::shrapMetal, &Spells::insectSpray, &Spells::toxicCloud,
		&Spells::protectionFromElements, &Spells::pain, &Spells::jump,
		&Spells::beastMaster, &Spells::clairvoyance, &Spells::turnUndead,
		&Spells::levitate,

		&Spells::wizardEye, &Spells::bless, &Spells::identifyMonster,
		&Spells::lightningBolt, &Spells::holyBonus, &Spells::powerCure,
		&Spells::naturesCure, &Spells::lloydsBeacon, &Spells::powerShield,
		&Spells::heroism,

		&Spells::hypnotize, &Spells::walkOnWater, &Spells::frostByte,
		&Spells::detectMonster, &Spells::fireball, &Spells::coldRay,
		&Spells::curePoison, &Spells::acidSpray, &Spells::timeDistortion,
		&Spells::dragonSleep,

		&Spells::cureDisease, &Spells::teleport, &Spells::fingerOfDeath,
		&Spells::cureParalysis, &Spells::golemStopper, &Spells::poisonVolley,
		&Spells::deadlySwarm, &Spells::superShelter, &Spells::dayOfProtection,
		&Spells::dayOfSorcery,

		&Spells::createFood, &Spells::fieryFlail, &Spells::rechargeItem,
		&Spells::fantasticFreeze, &Spells::townPortal, &Spells::stoneToFlesh,
		&Spells::raiseDead, &Spells::etherialize, &Spells::dancingSword,
		&Spells::moonRay,

		&Spells::massDistortion, &Spells::prismaticLight, &Spells::enchantItem,
		&Spells::incinerate, &Spells::holyWord, &Spells::resurrection,
		&Spells::elementalStorm, &Spells::megaVolts, &Spells::inferno,
		&Spells::sunRay,

		&Spells::implosion, &Spells::starBurst, &Spells::divineIntervention
	};

	if (_vm->_mode == MODE_COMBAT) {
		if (spellId == 15 || spellId == 20 || spellId == 27 || spellId == 41
				|| spellId == 47 || spellId == 54 || spellId == 57) {
			ErrorDialog::show(_vm, Common::String::format(CANT_CAST_WHILE_ENGAGED,
				_spellNames[spellId].c_str()));
			return;
		}
	}

	(this->*SPELL_LIST[spellId])();
}

void Spells::light() { error("TODO: spell"); }
void Spells::awaken() { error("TODO: spell"); }
void Spells::magicArrow() { error("TODO: spell"); }
void Spells::firstAid() { error("TODO: spell"); }
void Spells::flyingFist() { error("TODO: spell"); }
void Spells::energyBlast() { error("TODO: spell"); }
void Spells::sleep() { error("TODO: spell"); }
void Spells::revitalize() { error("TODO: spell"); }
void Spells::cureWounds() { error("TODO: spell"); }
void Spells::sparks() { error("TODO: spell"); }

void Spells::shrapMetal() { error("TODO: spell"); }
void Spells::insectSpray() { error("TODO: spell"); }
void Spells::toxicCloud() { error("TODO: spell"); }
void Spells::protectionFromElements() { error("TODO: spell"); }
void Spells::pain() { error("TODO: spell"); }
void Spells::jump() { error("TODO: spell"); }			// Not while engaged
void Spells::beastMaster() { error("TODO: spell"); }
void Spells::clairvoyance() { error("TODO: spell"); }
void Spells::turnUndead() { error("TODO: spell"); }
void Spells::levitate() { error("TODO: spell"); }

void Spells::wizardEye() { error("TODO: spell"); }		// Not while engaged
void Spells::bless() { error("TODO: spell"); }
void Spells::identifyMonster() { error("TODO: spell"); }
void Spells::lightningBolt() { error("TODO: spell"); }
void Spells::holyBonus() { error("TODO: spell"); }
void Spells::powerCure() { error("TODO: spell"); }
void Spells::naturesCure() { error("TODO: spell"); }
void Spells::lloydsBeacon() { error("TODO: spell"); }	// Not while engaged
void Spells::powerShield() { error("TODO: spell"); }
void Spells::heroism() { error("TODO: spell"); }

void Spells::hypnotize() { error("TODO: spell"); }
void Spells::walkOnWater() { error("TODO: spell"); }
void Spells::frostByte() { error("TODO: spell"); }
void Spells::detectMonster() { error("TODO: spell"); }
void Spells::fireball() { error("TODO: spell"); }
void Spells::coldRay() { error("TODO: spell"); }
void Spells::curePoison() { error("TODO: spell"); }
void Spells::acidSpray() { error("TODO: spell"); }
void Spells::timeDistortion() { error("TODO: spell"); }
void Spells::dragonSleep() { error("TODO: spell"); }

void Spells::cureDisease() { error("TODO: spell"); }
void Spells::teleport() { error("TODO: spell"); }		// Not while engaged
void Spells:: fingerOfDeath() { error("TODO: spell"); }
void Spells::cureParalysis() { error("TODO: spell"); }
void Spells::golemStopper() { error("TODO: spell"); }
void Spells::poisonVolley() { error("TODO: spell"); }
void Spells::deadlySwarm() { error("TODO: spell"); }
void Spells::superShelter() { error("TODO: spell"); }	// Not while engaged
void Spells::dayOfProtection() { error("TODO: spell"); }
void Spells::dayOfSorcery() { error("TODO: spell"); }

void Spells::createFood() { error("TODO: spell"); }
void Spells::fieryFlail() { error("TODO: spell"); }
void Spells::rechargeItem() { error("TODO: spell"); }
void Spells::fantasticFreeze() { error("TODO: spell"); }
void Spells::townPortal() { error("TODO: spell"); }		// Not while engaged
void Spells::stoneToFlesh() { error("TODO: spell"); }
void Spells::raiseDead() { error("TODO: spell"); }
void Spells::etherialize() { error("TODO: spell"); }		// Not while engaged
void Spells::dancingSword() { error("TODO: spell"); }
void Spells::moonRay() { error("TODO: spell"); }

void Spells::massDistortion() { error("TODO: spell"); }
void Spells::prismaticLight() { error("TODO: spell"); }
void Spells::enchantItem() { error("TODO: spell"); }
void Spells::incinerate() { error("TODO: spell"); }
void Spells::holyWord() { error("TODO: spell"); }
void Spells::resurrection() { error("TODO: spell"); }
void Spells::elementalStorm() { error("TODO: spell"); }
void Spells::megaVolts() { error("TODO: spell"); }
void Spells::inferno() { error("TODO: spell"); }
void Spells::sunRay() { error("TODO: spell"); }

void Spells::implosion() { error("TODO: spell"); }
void Spells::starBurst() { error("TODO: spell"); }
void Spells::divineIntervention() { error("TODO: spell"); }

} // End of namespace Xeen
