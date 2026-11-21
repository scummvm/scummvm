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

#include "tesoro/kinds/storycharacter.h"
#include "tesoro/world.h"

#include "common/textconsole.h"

namespace Tesoro {

// The state sizes the subtypes keep, and whether they start looping their first animation.
static const byte kStateSize[17] = { 4, 14, 4, 0, 10, 3, 6, 4, 3, 0, 1, 1, 0, 15, 2, 11, 7 };
static const bool kLooping[17] = { false, true, true, false, true, false, true, false, false, false, false, false, false, false, false, false, false };

void StoryCharacter::init(int subtype, const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &host) {
	_subtype = subtype;
	// A subtype the game does not use (12 and beyond 16) is built like subtype 9: no state.
	const uint16 stateSize = subtype >= 0 && subtype <= 16 && subtype != 12 ? kStateSize[subtype] : 0;
	Character::init(record, roomSortOffset, levelPlayers, now, host, stateSize);
	if (subtype >= 0 && subtype <= 16 && kLooping[subtype])
		_anim.selectBank(0, 0, -1, true, false, now);
}

byte StoryCharacter::stateOf(int level, const char *name, uint index) {
	Object *object = _host->findObject(level, name);
	if (!object || index >= object->stateBlob().size()) {
		warning("Tesoro: no state byte %u in %s", index, name);
		return 0;
	}
	return object->stateBlob()[index];
}

int StoryCharacter::run(const char *block, const char *label, bool withState) {
	return _host->startScript(*this, block, label, withState && !_stateBlob.empty() ? _stateBlob.data() : nullptr);
}

int StoryCharacter::verb(int verb) {
	byte *s = _stateBlob.empty() ? nullptr : _stateBlob.data();
	switch (_subtype) {
	case 0:
		if (verb != 2)
			break;
		s[1] = stateOf(kTavern, "NUDERO", 4) != 0;
		s[2] = stateOf(kPort, "RECLUTADOR", 3);
		run("FCAPITAN", "hablar", true);
		return 1;
	case 1:
		if (verb != 2)
			break;
		if ((int8)s[0] < 4 && stateOf(kOffice, "CAPITAN", 0) != 0)
			s[0] = 4;
		if ((int8)s[0] < 6 && _host->isActive(kInterface, "IHALFDOBLON"))
			s[0] = 6;
		run("FRECLUTADOR", "hablar", true);
		return 1;
	case 2:
		if (verb != 2)
			break;
		if (_host->isActive(kInterface, "IDOBLON"))
			s[0] = 2;
		run("FDICKTABER", "hablar", true);
		return 1;
	case 3:
		if (verb != 2)
			break;
		run("FLORO", "hablar", false);
		return 1;
	case 4:
		if (verb != 2)
			break;
		s[1] = _host->isActive(kInterface, "ICUERDA");
		s[2] = _host->isActive(kInterface, "IJARRALLENA");
		if (s[4] == 0)
			s[4] = stateOf(kPort, "RECLUTADOR", 0xb);
		run("FNUDERO", "hablar", true);
		if (stateOf(kTavern, "TONTO", 0) == 0) {
			Object *tonto = _host->findObject(kTavern, "TONTO");
			if (tonto && !tonto->stateBlob().empty())
				tonto->stateBlob()[0] = 1;
		}
		return 1;
	case 5:
		if (verb != 2)
			break;
		run("FPESCADOR", "hablar", true);
		return 1;
	case 6:
		if (verb != 2)
			break;
		if (s[0] == 0 && _host->isActive(kInterface, "IPLATO") && s[3] == 0)
			s[3] = 1;
		run("FLUDOPATA", "hablar", true);
		return 1;
	case 7:
		if (verb != 2)
			break;
		run("FDICKCOCINA", "hablar", true);
		return 1;
	case 8:
		if (verb == 1) {
			run("FLOROCOCINA", "mirar", true);
			return 1;
		}
		if (verb == 2) {
			run("FLOROCOCINA", "hablar", true);
			return 1;
		}
		break;
	case 13:
		if (verb == 1) {
			if (s[0] == 0)
				s[0] = stateOf(kVillage, "CHOZA", 1);
			return run("FORZOGUEI", "mirar", true);
		}
		if (verb == 2) {
			s[3] = _host->isActive(kInterface, "ICUBOBARRO");
			if (s[0] == 0)
				s[0] = stateOf(kVillage, "CHOZA", 1);
			if (s[0xb] && s[0xc] && s[0xd] && s[0xe])
				s[1] = 5;
			if (s[1] == 5 && stateOf(kEarth, "JUSTINO", 2) != 0)
				s[1] = 6;
			return run("FORZOGUEI", "hablar", true);
		}
		break;
	case 14:
		if (verb == 1) {
			run("FOBTUSU", "mirar", true);
			return 1;
		}
		if (verb == 2) {
			s[1] = stateOf(kVillage, "ORZOGUEI", 1) == 5;
			run("FOBTUSU", "hablar", true);
			return 1;
		}
		break;
	case 15:
		if (verb != 2)
			break;
		{
			const bool cubo = _host->isActive(kInterface, "ICUBOBARRO");
			const byte orzoguei = stateOf(kVillage, "ORZOGUEI", 4);
			s[2] = (orzoguei | (cubo ? 1 : 0)) & 1;
		}
		s[3] = stateOf(kVillage, "CHOZA", 1);
		run("FJUSTINO", "hablar", true);
		return 1;
	case 16:
		if (verb != 2)
			break;
		s[0] = stateOf(kVolcano, "JUSTINO", 1);
		if (_host->isActive(kInterface, "IMECHERO") && (int8)s[1] < 2)
			s[1] = 2;
		run("FTIENDA", "hablar", true);
		return 1;
	default:
		break;
	}
	return Character::verb(verb);
}

int StoryCharacter::useItem(int item) {
	byte *s = _stateBlob.empty() ? nullptr : _stateBlob.data();
	switch (_subtype) {
	case 0:
		if (item != 0)
			break;
		s[1] = stateOf(kTavern, "NUDERO", 4) != 0;
		run("FIBOLI", "capitan", true);
		return 3;
	case 1:
		if ((int8)s[0] < 4 && stateOf(kOffice, "CAPITAN", 0) != 0)
			s[0] = 4;
		if ((int8)s[0] < 6 && _host->isActive(kInterface, "IHALFDOBLON"))
			s[0] = 6;
		if (item == 0)
			run("FIBOLI", "reclutador", false);
		else if (item == 7)
			run("FICNUDO", "reclutador", false);
		else if (item == 45)
			run("FIHLFDBLN", "reclutador", true);
		else
			break;
		return 1;
	case 2:
		switch (item) {
		case 0:
			run("FIBOLI", "dick", false);
			break;
		case 3:
			if (_host->isActive(kInterface, "IDOBLON"))
				s[0] = 2;
			run("FIJARRA", "dick", true);
			break;
		case 5:
			run("FIDOBLON", "dick", true);
			break;
		case 45:
			run("FIHLFDBLN", "dick", false);
			break;
		default:
			break;
		}
		return 1;
	case 4:
		switch (item) {
		case 0:
			run("FIBOLI", "nudero", false);
			break;
		case 4:
			run("FIJARRALLENA", "nudero", true);
			break;
		case 5:
			run("FIDOBLON", "nudero", false);
			break;
		case 6:
			run("FICUERDA", "nudero", true);
			break;
		default:
			break;
		}
		return 1;
	case 5:
		if (item == 11 || item == 17) {
			if (item == 11)
				run("FIPIMIENTO", "pescador", false);
			else
				run("FIGUSANO", "pescador", true);
			return 1;
		}
		break;
	case 7:
		if (item == 7)
			return run("FICNUDO", "dick", true);
		if (item == 12) {
			Object *loro = _host->findObject(kKitchen, "LOROPOCHO");
			byte *vars = loro && !loro->stateBlob().empty() ? loro->stateBlob().data() : nullptr;
			return _host->startScript(*this, "FIREVITALIZA", "loro", vars);
		}
		break;
	case 8:
		if (item == 12)
			return run("FIREVITALIZA", "loro", true);
		if (item == 17)
			return run("FIGUSANO", "loro", true);
		break;
	case 13:
		if (item == 41)
			return run("FIMECHERO", "orzoguei", false);
		break;
	case 14:
		if (item == 26) {
			Object *canto = _host->findObject(kInterface, "ICANTORODD");
			byte *vars = canto && !canto->stateBlob().empty() ? canto->stateBlob().data() : nullptr;
			return _host->startScript(*this, "FICANTO", "obtusu", vars);
		}
		if (item == 27)
			return run("FIPIEDRAA", "obtusu", false);
		break;
	default:
		break;
	}
	return Character::useItem(item);
}

void StoryCharacter::prepareToSpeak(uint32 pose) {
	Character::prepareToSpeak(pose);
	if (_subtype == 3) {
		if (Object *dick = _host->findObject(kTavern, "DICK"))
			dick->setVisible(false);
	}
}

void StoryCharacter::speechCleared() {
	if (_subtype == 3) {
		if (Object *dick = _host->findObject(kTavern, "DICK"))
			dick->setVisible(true);
	}
}

void StoryCharacter::draw(const Camera &camera, Display &display) {
	// This one is only drawn while it is not holding a speaking pose it is not using.
	if (_subtype == 3 && animation().isStashed() && !speaker().speaking())
		return;
	Character::draw(camera, display);
}

} // End of namespace Tesoro
