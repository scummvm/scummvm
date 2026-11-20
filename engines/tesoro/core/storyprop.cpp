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

#include "tesoro/core/storyprop.h"
#include "tesoro/world.h"

#include "common/textconsole.h"

namespace Tesoro {

enum Place { kInterface = -1, kOffice = 0, kPort = 1, kTavern = 2, kCabin = 4, kKitchen = 6, kBeach = 7, kVillage = 9, kSwamp = 10, kVolcano = 12 };

// The state bytes each subtype keeps (kinds 12, 17 and 20).
static const byte kState12[12] = { 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0 };
static const byte kState17[17] = { 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0 };
static const byte kState20[31] = { 1, 0, 0, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1 };

void StoryProp::init(int kind, int subtype, const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &host) {
	_propKind = kind;
	_subtype = subtype;
	uint16 stateSize = 0;
	if (kind == 12 && subtype >= 0 && subtype < 12)
		stateSize = kState12[subtype];
	else if (kind == 17 && subtype >= 0 && subtype < 17)
		stateSize = kState17[subtype];
	else if (kind == 20 && subtype >= 0 && subtype < 31)
		stateSize = kState20[subtype];
	InteractiveObject::init(record, roomSortOffset, levelPlayers, now, host, stateSize);
	if (kind == 17 && subtype == 13)
		_flags |= kVisible;
}

byte StoryProp::stateOf(int level, const char *name, uint index) {
	Object *object = _host->findObject(level, name);
	if (!object || index >= object->stateBlob().size()) {
		warning("Tesoro: no state byte %u in %s", index, name);
		return 0;
	}
	return object->stateBlob()[index];
}

void StoryProp::setItemActive(int level, const char *name, bool active) {
	if (Object *object = _host->findObject(level, name))
		object->setActive(active);
	else
		warning("Tesoro: no object %s", name);
}

void StoryProp::pickUp(const char *item) {
	setItemActive(kInterface, item, true);
	setActive(false);
}

int StoryProp::runWith(const char *block, const char *label, byte *variables) {
	return _host->startScript(*this, block, label, variables);
}

int StoryProp::run(const char *block, const char *label, bool withState) {
	return runWith(block, label, withState ? state() : nullptr);
}

int StoryProp::verb(int verb) {
	switch (_propKind) {
	case 12:
		return verb12(verb);
	case 17:
		return verb17(verb);
	case 20:
		return verb20(verb);
	default:
		return InteractiveObject::verb(verb);
	}
}

int StoryProp::useItem(int item) {
	switch (_propKind) {
	case 12:
		return use12(item);
	case 17:
		return use17(item);
	case 20:
		return use20(item);
	default:
		return InteractiveObject::useItem(item);
	}
}

int StoryProp::verb12(int verb) {
	byte *s = state();
	switch (_subtype) {
	case 0:
		if (verb == 3 || verb == 6) {
			s[0] = _host->isActive(kOffice, "JARRON");
			run("FTABURETE", verb == 3 ? "coger" : "mover", true);
			return 1;
		}
		break;
	case 1:
		if (verb == 1) {
			run("FPEZESP", "mirar", false);
			return 1;
		}
		break;
	case 2:
		if (verb == 1) {
			Object *captain = _host->findObject(kOffice, "CAPITAN");
			runWith("FCARTA", "mirar", captain && !captain->stateBlob().empty() ? captain->stateBlob().data() : nullptr);
			return 1;
		}
		break;
	case 3:
		if (verb == 4) {
			Object *captain = _host->findObject(kOffice, "CAPITAN");
			if (captain && captain->animation().bankIndex() == 2)
				run("FCAJON", "abrir", true);
			else
				run("FNOCOGER", "coger", false);
			return 1;
		}
		break;
	case 4:
		if (verb == 1) {
			run("FPLACA", "mirar", true);
			return 1;
		}
		break;
	case 5:
		if (verb == 1) {
			run("FBOLI", "mirar", false);
			return 1;
		}
		if (verb == 3) {
			Object *captain = _host->findObject(kOffice, "CAPITAN");
			runWith("FBOLI", "coger", captain && !captain->stateBlob().empty() ? captain->stateBlob().data() : nullptr);
			if (_host->pointerHasItem())
				_host->clearPointerItem();
			return 1;
		}
		if (verb == 4) {
			run("FBOLI", "abrir", false);
			return 1;
		}
		break;
	case 6:
	case 10:
		if (verb == 3) {
			Object *captain = _host->findObject(kOffice, "CAPITAN");
			if (!captain || captain->animation().bankIndex() != 2) {
				run("FNOCOGER", "coger", false);
				if (_subtype == 10)
					setActive(false);
				return 1;
			}
			pickUp(_subtype == 6 ? "IJARRON" : "IPLUMA");
			return 1;
		}
		break;
	case 7:
		if (verb == 1) {
			run("FJOE", "mirar", false);
			return 1;
		}
		if (verb == 2) {
			run("FJOE", "hablar", false);
			return 1;
		}
		break;
	case 8:
		if (verb == 1) {
			run("FTONTO", "mirar", true);
			return 1;
		}
		if (verb == 2) {
			run("FTONTO", "hablar", true);
			return 1;
		}
		break;
	case 9:
		if (verb == 3) {
			pickUp("IJARRA");
			return 1;
		}
		break;
	case 11:
		if (verb == 0) {
			Object *captain = _host->findObject(kOffice, "CAPITAN");
			run("FPUERTARARA", captain && captain->animation().bankIndex() == 2 ? "ir1" : "ir2", false);
			return 1;
		}
		break;
	default:
		break;
	}
	return InteractiveObject::verb(verb);
}

int StoryProp::use12(int item) {
	if (_subtype == 8) {
		if (item == 4)
			run("FIJARRALLENA", "tonto", true);
		else if (item == 5)
			run("FIDOBLON", "tonto", true);
		return 1;
	}
	return InteractiveObject::useItem(item);
}

int StoryProp::verb17(int verb) {
	byte *s = state();
	byte flag = 0;
	switch (_subtype) {
	case 0:
		if (verb == 1 || verb == 3) {
			Object *table = _host->findObject(kCabin, "MESA");
			flag = table && !table->stateBlob().empty() && (int8)table->stateBlob()[0] > 0;
			runWith("FLIBRO", verb == 1 ? "mirar" : "coger", &flag);
			return 1;
		}
		break;
	case 1:
		if (verb == 1) {
			run("FBANDERA", "mirar", false);
			return 1;
		}
		break;
	case 2:
		if (verb == 1) {
			s[0] = (byte)_host->randomNumber(0, 4);
			run("FLIBROS", "mirar", true);
			return 1;
		}
		break;
	case 3:
		if (verb == 3) {
			s[0] = _host->jimStep() == 8;
			run("FALFOMBRA", "coger", true);
			return 1;
		}
		break;
	case 4:
		if (verb == 1) {
			if (_host->isActive(kInterface, "IPAPELES") && (int8)s[0] < 1)
				s[0] = 1;
			run("FMESA", "mirar", true);
			return 1;
		}
		break;
	case 5:
		if (verb == 1) {
			run("FBAULOPEN", "mirar", true);
			return 1;
		}
		break;
	case 6:
		if (verb == 0) {
			flag = _host->isActive(kInterface, "IMAPA");
			runWith("FVENTOPEN", "mirar", &flag);
			return 1;
		}
		break;
	case 7:
		if (verb == 3) {
			s[0] = _host->isActive(kInterface, "ICIRUELA") || _host->isActive(kInterface, "ICIRUELABRT");
			run("FCIRUELAS", "coger", true);
			return 1;
		}
		break;
	case 8:
		if (verb == 1) {
			s[0] = _host->jimStep() == 4;
			run("FCOFRE", "mirar", true);
			return 1;
		}
		if (verb == 3) {
			s[0] = _host->jimStep() == 4;
			if (_host->jimStep() == 4)
				_host->setJimStep(5);
			run("FCOFRE", "coger", true);
			return 1;
		}
		break;
	case 9:
		if (verb == 3) {
			s[0] = _host->isActive(kInterface, "ICUERDANUDO");
			if (s[0] == 0)
				setItemActive(19, "CUERDA", false);
			run("FCUERDA", "coger", true);
			return 1;
		}
		break;
	case 10:
		if (verb == 3) {
			s[0] = _host->jimLevel() != _host->levelAt(20);
			run("FPATATAS", "coger", true);
			return 1;
		}
		break;
	case 11:
		if (verb == 2) {
			run("FPUCHERO", "hablar", false);
			return 1;
		}
		break;
	case 12:
		if (verb == 3) {
			run("FCUBO", "coger", false);
			return 1;
		}
		break;
	case 13:
		if (verb == 3) {
			s[0] = _host->jimStep() == 6;
			run("FPAPELES", "coger", true);
			return 1;
		}
		break;
	case 14:
		if (verb == 3) {
			setItemActive(kInterface, "IPOLVORA", true);
			return 1;
		}
		break;
	case 15:
		if (verb == 3) {
			setItemActive(kInterface, "IREVITALIZA", true);
			return 1;
		}
		break;
	case 16:
		if (verb == 4) {
			if (_host->jimStep() == 2) {
				_host->setJimStep(3);
				run("FBAUL", "main", false);
				return 1;
			}
			run("FBAUL", "main2", false);
			return 0;
		}
		break;
	default:
		break;
	}
	return InteractiveObject::verb(verb);
}

int StoryProp::use17(int item) {
	if (_subtype == 11) {
		if (item != 20)
			return InteractiveObject::useItem(item);
		run("FIPESCAO", "pucherito", false);
		return 1;
	}
	if (_subtype == 16) {
		if (item == 21) {
			run("FILLAVE", "cerradura", false);
			return 3;
		}
		return InteractiveObject::useItem(item);
	}
	return InteractiveObject::useItem(item);
}

int StoryProp::verb20(int verb) {
	byte *s = state();
	byte flag = 0;
	switch (_subtype) {
	case 0:
		if (verb == 2) {
			run("FCOCOTERO", "hablar", true);
			return 1;
		}
		break;
	case 3:
		if (verb == 0) {
			s[0] = _host->isActive(kInterface, "ICOCOABIERT");
			run("FCHOZA", "ir", true);
			return 1;
		}
		if (verb == 1) {
			run("FCHOZA", "mirar", true);
			return 1;
		}
		break;
	case 4:
		if (verb == 1) {
			Object *orzoguei = _host->findObject(kVillage, "ORZOGUEI");
			runWith("FTABLERO", "mirar", orzoguei && !orzoguei->stateBlob().empty() ? orzoguei->stateBlob().data() : nullptr);
			return 1;
		}
		break;
	case 5:
		if (verb == 0) {
			flag = _host->isActive(kVolcano, "JUSTINO");
			runWith("FENTVOLCAN", "ir", &flag);
			return 1;
		}
		break;
	case 6:
		if (verb == 3) {
			pickUp("IPOMEZ");
			return 1;
		}
		break;
	case 9:
		if (verb == 3) {
			s[0] = 1;
			run("FFERNAN", "coger", false);
			return 1;
		}
		if (verb == 2) {
			run("FFERNAN", "hablar", false);
			return 1;
		}
		break;
	case 10:
		if (verb == 3) {
			setItemActive(kInterface, "IBOLSO", true);
			setItemActive(13, "BOLSO", false);
			return 1;
		}
		break;
	case 12:
		if (verb == 1) {
			run("FCAMPMAPA", "mirar", false);
			return 1;
		}
		if (verb == 3) {
			Object *hut = _host->findObject(kVillage, "CHOZA");
			runWith("FCAMPMAPA", "coger", hut && !hut->stateBlob().empty() ? hut->stateBlob().data() + 1 : nullptr);
			return 1;
		}
		break;
	case 13:
		if (verb == 1) {
			run("FPIRATAS", "mirar", false);
			return 1;
		}
		break;
	case 15:
		if (verb == 0)
			return 1;
		break;
	case 18:
		if (verb == 3) {
			run("FESPADA", "coger", false);
			return 1;
		}
		break;
	case 20:
		if (verb == 0) {
			s[0] = _host->isActive(kBeach, "CANOA");
			run("FBARCO", "ir", true);
			return 1;
		}
		break;
	case 22:
		if (verb == 3) {
			s[0] = _host->isActive(kInterface, "IMECHERO");
			run("FSOUVENIR", "coger", true);
			return 1;
		}
		break;
	case 23:
		if (verb == 3) {
			setItemActive(kInterface, "ISOGA", true);
			setActive(false);
			return 1;
		}
		break;
	case 24:
		if (verb == 1) {
			run("FCAPIMUERTO", "mirar", true);
			return 1;
		}
		break;
	case 25:
		if (verb == 1) {
			Object *suicida = _host->findObject(18, "SUICIDA");
			runWith("FNOTARIP", "mirar", suicida && !suicida->stateBlob().empty() ? suicida->stateBlob().data() : nullptr);
			return 1;
		}
		break;
	case 26:
		if (verb == 1) {
			run("FSUICIDA", "mirar", true);
			return 1;
		}
		break;
	case 27:
		if (verb == 3) {
			setItemActive(kBeach, "ANIMCOCO", false);
			setItemActive(kInterface, "ICOCO", true);
			return 1;
		}
		break;
	case 29:
		if (verb == 1) {
			run("FCERRADURA", "mirar", false);
			return 1;
		}
		break;
	case 30:
		if (verb == 0) {
			s[0] = 1;
			if (Object *exit = _host->findObject(kBeach, "SALIDA"))
				exit->verb(0);
			return 1;
		}
		break;
	default:
		break;
	}
	return InteractiveObject::verb(verb);
}

int StoryProp::use20(int item) {
	byte flag = 0;
	switch (_subtype) {
	case 0:
		if (_level != _host->levelAt(kBeach) || item != 27)
			break;
		flag = _host->isActive(kBeach, "RESINA");
		runWith("FIPIEDRAA", "cocotero", &flag);
		return 1;
	case 1:
		if (item != 35)
			break;
		setItemActive(kInterface, "IRAMA", false);
		setItemActive(kInterface, "IRAMARESINA", true);
		return 1;
	case 2:
		if (item == 23) {
			run("FICOCO", "rocas", false);
			return 1;
		}
		if (item == 26) {
			run("FICANTO", "rocas", false);
			return 1;
		}
		if (item == 27) {
			const bool rama = _host->isActive(kInterface, "IRAMA");
			const bool resina = _host->isActive(kInterface, "IRAMARESINA");
			const bool tapon = _host->isActive(kInterface, "ICUBOTAPON");
			const bool mar = _host->isActive(kInterface, "ICUBOMAR");
			const bool pant = _host->isActive(kInterface, "ICUBOPANT");
			flag = pant || rama || resina || tapon || mar;
			runWith("FIPIEDRAA", "rocas", &flag);
			return 1;
		}
		break;
	case 7:
		if (item != 27)
			break;
		setItemActive(kInterface, "IRAMA", true);
		setItemActive(kSwamp, "RAMA", false);
		return 1;
	case 8:
		if (item == 32) {
			setItemActive(kInterface, "ICUBOTAPON", false);
			setItemActive(kInterface, "ICUBOPANT", true);
			return InteractiveObject::useItem(32);
		}
		if (item != 37)
			break;
		setItemActive(kInterface, "ICUBOMAR", false);
		setItemActive(kInterface, "ICUBOPANT", true);
		return InteractiveObject::useItem(37);
	case 11:
		if (item != 30)
			break;
		_host->jimShow(true);
		_level = _host->levelAt(11);
		if (Object *floor = _host->findObject(11, "SUELO"))
			_host->putJimInLevel(_level, floor->x(), floor->y());
		_host->jimFace(4);
		_host->clearPointerItem();
		return 3;
	case 14:
		if (item != 38)
			break;
		if (stateOf(kVillage, "ORZOGUEI", 1) == 0)
			return InteractiveObject::useItem(38);
		if (Object *orzoguei = _host->findObject(kVillage, "ORZOGUEI"))
			orzoguei->stateBlob()[0xd] = 1;
		return run("FIPOMEZ", "geiser", false);
	case 16:
		if (item == 32) {
			setItemActive(kInterface, "ICUBOTAPON", false);
			setItemActive(kInterface, "ICUBOMAR", true);
			return InteractiveObject::useItem(32);
		}
		if (item != 33)
			break;
		setItemActive(kInterface, "ICUBOPANT", false);
		setItemActive(kInterface, "ICUBOMAR", true);
		return InteractiveObject::useItem(33);
	case 19:
		if (item != 40)
			break;
		return run("FCORTAR", "main", false);
	case 21:
		if (item != 43)
			break;
		run("FIXCUERDA", "diente", false);
		return 1;
	case 28:
		if (item != 40) {
			run("FCUERDALUCHA", (item & 1) ? "default1" : "default2", false);
			return 1;
		}
		return run("FCUERDALUCHA", "espada", false);
	default:
		break;
	}
	return InteractiveObject::useItem(item);
}

} // End of namespace Tesoro
