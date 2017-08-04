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

#include "common/system.h"

#include "supernova/supernova.h"
#include "supernova/state.h"

namespace Supernova {

void StartingItems::onEntrance() {
	for (int i = 0; i < 3; ++i)
		_gm->_inventory.add(*getObject(i));

	setRoomSeen(true);
	_gm->changeRoom(CABIN_R3);
}

bool ShipCorridor::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_PRESS) && (obj1._id == BUTTON)) {
		if (_objectState[6].hasProperty(OPENED)) {
			_vm->playSound(kAudioDoorSound);
			_objectState[6].disableProperty(OPENED);
			_gm->drawImage(8);
			setSectionVisible(9, false);
			_gm->wait2(2);
			_gm->drawImage(7);
			setSectionVisible(8, false);
			_gm->wait2(2);
			_gm->drawImage(_gm->invertSection(7));
		} else {
			_vm->playSound(kAudioDoorSound);
			_objectState[6].setProperty(OPENED);
			_gm->drawImage(7);
			_gm->wait2(2);
			_gm->drawImage(8);
			setSectionVisible(7, false);
			_gm->wait2(2);
			_gm->drawImage(9);
			setSectionVisible(8, false);
		}
		return true;
	}
	return false;
}

bool ShipHall::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_OPEN) && (obj1._id == KITCHEN_HATCH)) {
		_vm->renderMessage("In der Küche warst du schon|oft genug, im Moment hast|du keinen Appetit.");
	} else if ((verb == ACTION_USE) && Object::combine(obj1,obj2,KEYCARD2,SLEEP_SLOT)) {
		if (_objectState[2].hasProperty(OPENED)) {
			_objectState[2].disableProperty(OPENED);
			_gm->drawImage(3);
			setSectionVisible(4, false);
			_gm->wait2(2);
			_gm->drawImage(2);
			setSectionVisible(3, false);
			_gm->wait2(2);
			_gm->drawImage(_gm->invertSection(2));
		} else {
			_objectState[2].setProperty(OPENED);
			_gm->drawImage(2);
			_gm->wait2(2);
			_gm->drawImage(3);
			setSectionVisible(2, false);
			_gm->wait2(2);
			_gm->drawImage(4);
			setSectionVisible(3, false);
			_gm->great(1);
		}
	} else {
		return false;
	}

	return true;
}

bool ShipSleepCabin::interact(Action verb, Object &obj1, Object &obj2) {
	char input[31];
	static char codeword_DE[] = "ZWEIUNDVIERZIG";
	static char codeword_EN[] = "FORTYTWO";
	int32 l,*energy;
	Room *r;

	if (((verb == ACTION_LOOK) || (verb == ACTION_USE)) && (obj1._id == COMPUTER)) {
		_gm->_guiEnabled = false;
		setSectionVisible(4, false);
		g_system->fillScreen(kColorDarkBlue);
		if (_gm->_state.time == 0) {
			// Destination reached
			_vm->renderText("Flugziel erreicht", 60, 95, kColorWhite99);
			_gm->getInput();
		} else if (_gm->_state.powerOff) {
			// Energy depleted
			_vm->renderText("Energie erschöpft", 60, 95, kColorWhite99);
			// Artificial coma interrupted
			_vm->renderText("Tiefschlafprozess abgebrochen", 60, 115, kColorWhite99);
			_gm->getInput();
		} else if (isSectionVisible(5)) {
			// Sleep duration in days
			_vm->renderText("Schlafdauer in Tagen:", 30, 85, kColorWhite99);
			_vm->renderText(Common::String::format("%d",_gm->_state.timeSleep).c_str(),
			                150, 85, kColorWhite99);
			_vm->renderText("Bitte legen Sie sich in die angezeigte Schlafkammer.",
			                30, 105, kColorWhite99);
			_gm->getInput();
		} else {
			_vm->renderText("Bitte Passwort eingeben:", 100, 85, kColorWhite99);
			input[0] = 0;
			do {
				_gm->edit(input, 100, 105, 30);
			} while ((_gm->_key.keycode != Common::KEYCODE_RETURN) &&
			         (_gm->_key.keycode != Common::KEYCODE_ESCAPE));
			if (_gm->_key.keycode == Common::KEYCODE_ESCAPE) {
				goto escape;
			}
			for (int i = 0; i < 30; ++i) {
				if ((input[i] >= 'a') && (input[i] <= 'z')) {
					input[i] -= 'a' - 'A';
				}
			}
			if (strcmp(input,codeword_DE) != 0) {
				_vm->renderText("Falsches Passwort", 100, 125, kColorLightRed);
				_gm->wait2(18);
				goto escape;
			}
			_gm->great(6);
			_vm->renderBox(0, 0, 320, 200, kColorDarkBlue);
			_vm->renderText("Schlafdauer in Tagen:", 30, 85, kColorWhite99);
			do {
				_vm->renderBox(150, 85, 150, 8, kColorDarkBlue);
				input[0] = 0;
				do {
					_gm->edit(input, 150, 85, 10);
				} while ((_gm->_key.keycode != Common::KEYCODE_RETURN) &&
				         (_gm->_key.keycode != Common::KEYCODE_ESCAPE));
				if (_gm->_key.keycode == Common::KEYCODE_ESCAPE) {
					goto escape;
				}
				l = atol(input);
				for (uint i = 0; i < strlen(input); i++) {
					if ((input[i] < '0') || (input[i] > '9')) {
						l = 0;
					}
				}
			} while (l == 0);
			_gm->_state.timeSleep = l;
			_vm->renderText("Bitte legen Sie sich in die angezeigte Schlafkammer.",30,105,kColorWhite99);
			_gm->wait2(18);
			setSectionVisible(5, true);
		}
        escape:
		_vm->renderRoom(*this);
		_gm->showMenu();
		_gm->drawMapExits();
		_gm->palette();
		_gm->_guiEnabled = true;
	} else if (((verb == ACTION_WALK) || (verb == ACTION_USE)) &&
	           ((obj1._id == CABINS) || (obj1._id == CABIN))) {
		r = _gm->_rooms[AIRLOCK];
		if (!(obj1._id == CABIN) || !isSectionVisible(5)) {
			_vm->renderMessage("Es würde wenig bringen,|sich in eine Schlafkammer zu legen,|die nicht eingeschaltet ist.");
		} else if (r->getObject(5)->hasProperty(WORN)) {
			_vm->renderMessage("Dazu mußt du erst den Raumanzug ausziehen.");
		} else {
			_vm->paletteFadeOut();
			_gm->drawImage(_gm->invertSection(5));
			_gm->drawImage(_gm->invertSection(4));
			r = _gm->_rooms[GENERATOR];
			if (r->isSectionVisible(9)) {
				energy = &_gm->_state.landingModuleEnergy;
			} else {
				energy = &_gm->_state.shipEnergy;
			}
			if (_gm->_state.timeSleep > _gm->_state.time) {
				_gm->_state.timeSleep = _gm->_state.time;
			}
			if (_gm->_state.timeSleep >= *energy) {
				_gm->_state.timeSleep = *energy;
				if (r->isSectionVisible(9)) {
					r = _gm->_rooms[LANDINGMODULE]; // Monitors off
					r->setSectionVisible(2, false);
					r->setSectionVisible(7, false);
					r->setSectionVisible(8, false);
					r->setSectionVisible(9, false);
					r->setSectionVisible(10, false);
				}
			}
			if (_gm->_state.timeSleep == _gm->_state.time) {
				_gm->drawImage(3);
				r = _gm->_rooms[COCKPIT];
				r->setSectionVisible(23, true);
				r = _gm->_rooms[CABIN_R2];
				r->setSectionVisible(5, false);
				r->setSectionVisible(6, true);
				r->getObject(2)->_click = 10;
				r = _gm->_rooms[HOLD];
				r->setSectionVisible(0, false);
				r->setSectionVisible(1, true);
				r->getObject(1)->_click = 255;
				r->getObject(3)->_click = 255;
				r = _gm->_rooms[GENERATOR];
				r->setSectionVisible(6, false);
				r->setSectionVisible(7, true);
				r->getObject(1)->_click = 14;
				if (r->isSectionVisible(1)) {
					r->setSectionVisible(10, true);
				}
				if (r->isSectionVisible(12)) {
					r->setSectionVisible(12, false);
					r->setSectionVisible(11, true);
				}
			}
			_gm->_state.time -= _gm->_state.timeSleep;
			*energy -= _gm->_state.timeSleep;
			_gm->_state.timeStarting = _vm->getDOSTicks() - 786520;  // 12pm
			_gm->_state.timeAlarmSystem = _gm->_state.timeAlarm + _gm->_state.timeStarting;
			_gm->_state.alarmOn = (_gm->_state.timeAlarmSystem > _vm->getDOSTicks());
			if (!*energy) {
				_gm->turnOff();
				r = _gm->_rooms[GENERATOR];
				r->setSectionVisible(4, r->isSectionVisible(2));
			}
			if (_gm->_state.time == 0) {
				_gm->saveTime();
				if (!_gm->saveGame(-2))
					_gm->errorTemp();
				_gm->_state.dream = true;
				_gm->loadTime();
			}
			_gm->wait2(18);
			_vm->paletteFadeIn();
			if (_gm->_state.time == 0) {
				_vm->playSound(kAudioCrash);
				_gm->screenShake();
				_gm->wait2(18);
				_vm->renderMessage("Was war das?");
			}
		}
	} else {
		return false;
	}

	return true;
}

void ShipSleepCabin::animation() {
	static char color;
	if (_gm->_state.powerOff && _gm->_state.time) {
		if (_gm->_guiEnabled) {
			if (isSectionVisible(1)) {
				_gm->drawImage(2);
				setSectionVisible(1, false);
			} else {
				_gm->drawImage(1);
				setSectionVisible(2, false);
			}
		} else {
			if (color == kColorLightRed) {
				color = kColorDarkBlue;
			} else {
				color = kColorLightRed;
			}

			_vm->renderText("Achtung", 60, 75, color);
		}
	} else if (isSectionVisible(5) && _gm->_guiEnabled) {
		if (isSectionVisible(4))
			_gm->drawImage(_gm->invertSection(4));
		else
			_gm->drawImage(4);
	}

	_gm->setAnimationTimer(6);
}
void ShipSleepCabin::onEntrance() {
	if (_gm->_state.dream && (_gm->_rooms[CAVE]->getObject(1)->_exitRoom == MEETUP3)) {
		_vm->renderMessage("Du wachst mit brummendem Schädel auf|und merkst, daß du nur geträumt hast.");
		_gm->mouseWait(_gm->_timer1);
		_vm->removeMessage();
		_vm->renderMessage("Beim Aufprall des Raumschiffs|mußt du mit dem Kopf aufgeschlagen|und bewußtlos geworden sein.");
		_gm->mouseWait(_gm->_timer1);
		_vm->removeMessage();
		_vm->renderMessage("Was steht dir jetzt wohl wirklich bevor?");
		_gm->_state.dream = false;
	}
	setRoomSeen(true);
}

bool ShipCockpit::interact(Action verb, Object &obj1, Object &obj2) {
	// TODO: distance and remaining time not accurate
	char c[2] = {0};

	if ((verb == ACTION_LOOK) && (obj1._id == MONITOR)) {
		_gm->_guiEnabled = false;
		_vm->renderBox(0, 0, 320, 200, kColorBlack);
		_vm->renderText("Geschwindigkeit: ", 50, 50, kColorLightYellow);
		if (_gm->_state.time)
			_vm->renderText("8000 hpm");
		else
			_vm->renderText("0 hpm");
		_vm->renderText("Ziel: Arsano 3", 50, 70, kColorLightYellow);
		_vm->renderText("Entfernung: ", 50, 90, kColorLightYellow);
		_vm->renderText(Common::String::format("%d", _gm->_state.timeStarting / 7200000).c_str());
		_vm->renderText(",");
		c[0] = (_gm->_state.timeStarting / 720000) % 10 + '0';
		_vm->renderText(c);
		c[0] = (_gm->_state.timeStarting / 72000) % 10 + '0';
		_vm->renderText(c);
		_vm->renderText(" Lichtjahre");
		_vm->renderText("Dauer der Reise bei momentaner Geschwindigkeit:", 50, 110, kColorLightYellow);
		_vm->renderText(Common::String::format("%d", _gm->_state.timeStarting / 18000).c_str(), 50, 120, kColorLightYellow);
		_vm->renderText(" Tage");

		_gm->getInput();
		_gm->_guiEnabled = true;
	} else if ((verb == ACTION_USE) && (obj1._id == INSTRUMENTS))
		_vm->renderMessage("Vergiá nicht, du bist nur der|Schiffskoch und hast keine Ahnung,|wie man ein Raumschiff fliegt.");
	else
		return false;

	return true;
}
void ShipCockpit::animation() {
	static byte color;

	if (!_gm->_guiEnabled) {
		if (color) {
			color = kColorBlack;
			_gm->setAnimationTimer(5);
		} else {
			color = kColorLightYellow;
			_gm->setAnimationTimer(10);
		}
		_vm->renderText("Achtung: Triebwerke funktionsunfhig", 50, 145, color);
	} else {
		if (isSectionVisible(21)) {
			_gm->drawImage(_gm->invertSection(21));
			_gm->setAnimationTimer(5);
		} else {
			_gm->drawImage(21);
			_gm->setAnimationTimer(10);
		}
	}
	if (_gm->_state.powerOff) {
		if (!_gm->_guiEnabled) {
			_vm->renderText("Energievorrat erschpft", 97, 165, color);
			_vm->renderText("Notstromversorgung aktiv", 97, 175, color);
		} else {
			if (isSectionVisible(21))
				_gm->drawImage(22);
			else
				_gm->drawImage(_gm->invertSection(22));
		}
	}
}

void ShipCockpit::onEntrance() {
	if (!hasSeen())
		_vm->renderMessage("Was?! Keiner im Cockpit!|Die sind wohl verrckt!");
	setRoomSeen(true);
}

bool ShipCabinL2::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_KL1, KEYCARD2)) {
		_gm->openLocker(this, getObject(4), getObject(0), 17);
		if (getObject(5)->_click == 255)
			_gm->drawImage(20); // Remove Pistol
		_gm->great(2);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_KL2, KEYCARD2)) {
		_gm->openLocker(this, getObject(6), getObject(1), 18);
		_gm->great(2);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_KL3, KEYCARD2)) {
		_gm->openLocker(this, getObject(8), getObject(2), 19);
		if (getObject(9)->_click == 255)
			_gm->drawImage(21); // Remove cable spool
		_gm->great(2);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_KL4, KEYCARD2)) {
		_gm->openLocker(this, getObject(10), getObject(3), 22);
		if (getObject(11)->_click == 255)
			_gm->drawImage(23); // Remove book
		_gm->great(2);
	}

	else if ((verb == ACTION_CLOSE) && (obj1._id == SHELF_L1)) {
		_gm->closeLocker(this, getObject(4), getObject(0), 17);
		setSectionVisible(20, false);
	} else if ((verb == ACTION_CLOSE) && (obj1._id == SHELF_L2))
		_gm->closeLocker(this, getObject(6), getObject(1), 18);
	else if ((verb == ACTION_CLOSE) && (obj1._id == SHELF_L3)) {
		_gm->closeLocker(this, getObject(8), getObject(2), 19);
		setSectionVisible(21, false);
	} else if ((verb == ACTION_CLOSE) && (obj1._id == SHELF_L4)) {
		_gm->closeLocker(this, getObject(10), getObject(3), 22);
		setSectionVisible(23, false);
	}

	else if ((verb == ACTION_TAKE) && (obj1._id == SPOOL) &&
	         !(obj1.hasProperty(CARRIED))) {
		getObject(8)->_click = 42;  // empty shelf
		return false;
	} else if ((verb == ACTION_TAKE) && (obj1._id == BOOK2) &&
	           !(obj1.hasProperty(CARRIED))) {
		getObject(10)->_click = 47;  // empty shelf
		return false;
	} else
		return false;

	return true;
}

bool ShipCabinL3::interact(Action verb, Object &obj1, Object &obj2) {
	Room *r;

	if ((verb == ACTION_USE) && Object::combine(obj1, obj2, RECORD, TURNTABLE)) {
		if (isSectionVisible(kMaxSection - 1) || isSectionVisible(15))
			_vm->renderMessage("Du hast die Platte schon aufgelegt.");
		else {
			if (!getObject(4)->hasProperty(CARRIED))
				_gm->drawImage(_gm->invertSection(8));
			else
				_gm->_inventory.remove(*getObject(4));
			_gm->drawImage(15);
			getObject(4)->_click = 48;
		}
	} else if ((verb == ACTION_PRESS) && (obj1._id == TURNTABLE_BUTTON)) {
		if (!isSectionVisible(15)) {
			_vm->renderMessage("Es ist doch gar keine Platte aufgelegt.");
		} else if (!isSectionVisible(10) &&
		           !isSectionVisible(11) &&
		            isSectionVisible(12)) {
			if (_gm->_soundDevice != 2) {
				_gm->drawImage(14);
				setSectionVisible(15, false);
				for (int i = 3; i; i--) {
					_vm->playSound(kAudioTurntable);
					if (_gm->_soundDevice) {
						do {
							if (isSectionVisible(13)) {
								_gm->drawImage(14);
								setSectionVisible(13, false);
							} else {
								_gm->drawImage(13);
								setSectionVisible(14, false);
							}
							_gm->wait2(3);
						} while (_gm->_status);
					} else {
						_gm->wait2(1);
					}
				}
			} else {
				for (int i = 10; i; i--) {
					_gm->drawImage(14);
					_gm->wait2(3);
					_gm->drawImage(13);
					_gm->wait2(3);
				}
			}
			_gm->drawImage(15);
			setSectionVisible(14, false);
			setSectionVisible(13, false);
			_vm->renderMessage("Die Platte scheint einen Sprung zu haben.");
		}
	} else if ((verb == ACTION_TAKE) && (obj1._id == RECORD) &&
	           (obj1._click != 15)) {
		_gm->drawImage(9);
		setSectionVisible(13, false);
		setSectionVisible(14, false);
		setSectionVisible(15, false);
		obj1._section = 0;
		_gm->takeObject(obj1);
	} else if ((verb == ACTION_PULL) && (obj1._id == PLUG)) {
		_gm->drawImage(10);
		setSectionVisible(7, false);
		obj1._click = 21;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, PLUG, SOCKET)) {
		_gm->drawImage(7);
		setSectionVisible(10, false);
		getObject(10)->_click = 20;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, KNIFE, WIRE2))
		_vm->renderMessage("Schneid doch besser ein|lngeres Stck Kabel ab!");
	else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, KNIFE, WIRE)) {
		r = _gm->_rooms[AIRLOCK];
		if (!isSectionVisible(10) && !r->getObject(5)->hasProperty(WORN)) {
			_gm->drawImage(25);
			_gm->shock();
		}
		_gm->drawImage(11);
		_gm->drawImage(26);
		setSectionVisible(12, false);
	} else if ((verb == ACTION_TAKE) &&
	           ((obj1._id == WIRE) || (obj1._id == WIRE2) || (obj1._id == PLUG))) {
		if (isSectionVisible(10) && isSectionVisible(11)) {
			_gm->drawImage(_gm->invertSection(10));
			_gm->drawImage(_gm->invertSection(11));
			getObject(8)->_name = "Leitung mit Stecker";
			_gm->takeObject(*getObject(8));
			getObject(9)->_click = 255;
			getObject(10)->_click = 255;
		} else {
			_vm->renderMessage("Das ist befestigt.");
		}
	} else {
		return false;
	}

	return true;
}

bool ShipCabinR3::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_K1, KEYCARD)) {
		_gm->openLocker(this, getObject(6), getObject(2), 9);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_K2, KEYCARD)) {
		_gm->openLocker(this, getObject(8), getObject(3), 10);
		if (getObject(9)->_click == 255)
			_gm->drawImage(12); // Remove rope
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_K3, KEYCARD)) {
		_gm->openLocker(this, getObject(10), getObject(4), 11);
		if (getObject(17)->_click == 255)
			_gm->drawImage(16); // Remove Discman
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_K4, KEYCARD)) {
		_gm->openLocker(this, getObject(15), getObject(5), 13);
		if (getObject(16)->_click == 255)
			_gm->drawImage(14); // Remove Book
	}

	else if ((verb == ACTION_CLOSE) && (obj1._id == SHELF1))
		_gm->closeLocker(this, getObject(6), getObject(2), 9);
	else if ((verb == ACTION_CLOSE) && (obj1._id == SHELF2)) {
		setSectionVisible(12, false);
		_gm->closeLocker(this, getObject(8), getObject(3), 10);
	} else if ((verb == ACTION_CLOSE) && (obj1._id == SHELF3)) {
		setSectionVisible(16, false);
		_gm->closeLocker(this, getObject(10), getObject(4), 11);
	} else if ((verb == ACTION_CLOSE) && (obj1._id == SHELF4)) {
		setSectionVisible(14, false);
		setSectionVisible(14, false);
		_gm->closeLocker(this, getObject(15), getObject(5), 13);
	}

	else if ((verb == ACTION_TAKE) && (obj1._id == DISCMAN) &&
	         !_gm->_rooms[0]->getObject(3)->hasProperty(CARRIED)) {
		getObject(10)->_click = 34; // Locker empty
		obj1._click = 255;
		_gm->takeObject(*_gm->_rooms[0]->getObject(3));
		_gm->drawImage(16);
	} else if ((verb == ACTION_TAKE) && (obj1._id == ROPE) &&
	           obj1.hasProperty(CARRIED)) {
		getObject(8)->_click = 31; // Shelf empty
		return false;
	} else if ((verb == ACTION_TAKE) && (obj1._id == BOOK) &&
	           !obj1.hasProperty(CARRIED)) {
		getObject(15)->_click = 32; // Shelf empty
		return false;
	} else
		return false;

	return true;
}


bool ShipAirlock::interact(Action verb, Object &obj1, Object &obj2) {
	Room *r;

	if ((verb == ACTION_PRESS) && (obj1._id == BUTTON1)) {
		if (!getObject(1)->hasProperty(OPENED)) {
			_gm->drawImage(10);
			_vm->playSound(kAudioDoorSound);
			if (getObject(0)->hasProperty(OPENED)) {
				getObject(0)->disableProperty(OPENED);
				_gm->drawImage(1);
				_gm->wait2(2);
				_gm->drawImage(2);
				setSectionVisible(1, false);
				_gm->wait2(2);
				_gm->drawImage(3);
				setSectionVisible(2, false);
			} else {
				getObject(0)->setProperty(OPENED);
				_gm->drawImage(2);
				setSectionVisible(3, false);
				_gm->wait2(2);
				_gm->drawImage(1);
				setSectionVisible(2, false);
				_gm->wait2(2);
				_gm->drawImage(_gm->invertSection(1));
			}
			_gm->drawImage(_gm->invertSection(10));
		}
	} else if ((verb == ACTION_PRESS) && (obj1._id == BUTTON2)) {
		if (!getObject(0)->hasProperty(OPENED)) {
			_gm->drawImage(11);
			if (getObject(1)->hasProperty(OPENED)) {
				_vm->playSound(kAudioDoorSound);
				getObject(1)->disableProperty(OPENED);
				_gm->drawImage(4);
				_gm->wait2(2);
				_gm->drawImage(5);
				setSectionVisible(4, false);
				_gm->wait2(2);
				_gm->drawImage(6);
				setSectionVisible(5, false);
				_gm->drawImage(16);
				setSectionVisible(17, false);
				_gm->wait2(3);
				_gm->drawImage(15);
				setSectionVisible(16, false);
				_gm->wait2(3);
				_gm->drawImage(14);
				setSectionVisible(15, false);
				_gm->wait2(3);
				_gm->drawImage(13);
				setSectionVisible(14, false);
				_gm->wait2(3);
				_gm->drawImage(12);
				setSectionVisible(13, false);
				_gm->wait2(3);
				_gm->drawImage(_gm->invertSection(12));
			} else {
				getObject(1)->setProperty(OPENED);
				_gm->drawImage(12);
				_gm->wait2(3);
				_gm->drawImage(13);
				setSectionVisible(12, false);
				_gm->wait2(3);
				_gm->drawImage(14);
				setSectionVisible(13, false);
				_gm->wait2(3);
				_gm->drawImage(15);
				setSectionVisible(14, false);
				_gm->wait2(3);
				_gm->drawImage(16);
				setSectionVisible(15, false);
				_gm->wait2(3);
				_gm->drawImage(17);
				setSectionVisible(16, false);
				_vm->playSound(kAudioDoorSound);
				_gm->drawImage(5);
				setSectionVisible(6, false);
				_gm->wait2(2);
				_gm->drawImage(4);
				setSectionVisible(5, false);
				_gm->wait2(2);
				_gm->drawImage(_gm->invertSection(4));
				r = _gm->_rooms[AIRLOCK];
				if (!r->getObject(4)->hasProperty(WORN) ||
				    !r->getObject(5)->hasProperty(WORN) ||
				    !r->getObject(6)->hasProperty(WORN)) {
					_gm->death("Zu niedriger Luftdruck soll ungesund sein.");
					return true;
				}
			}
			_gm->drawImage(_gm->invertSection(11));
		}
	} else if ((verb == ACTION_LOOK) && (obj1._id == MANOMETER)) {
		if (getObject(1)->hasProperty(OPENED))
			_vm->renderMessage("Er zeigt Null an.");
		else
			_vm->renderMessage("Er zeigt Normaldruck an.");
	} else
		return false;

	return true;
}

void ShipAirlock::onEntrance() {
	if (!hasSeen())
		_vm->renderMessage("Komisch, es ist nur|noch ein Raumanzug da.");

	setRoomSeen(true);
}

bool ShipHold::interact(Action verb, Object &obj1, Object &obj2) {
	static char beschr2[] = "Ein Stck Schrott.";
	Room *r;

	if ((verb == ACTION_LOOK) && (obj1._id == SCRAP_LK) &&
	        (obj1._description != beschr2)) {
		_vm->renderMessage(obj1._description);
		obj1._description = beschr2;
		_gm->takeObject(*getObject(2));
	} else if (((verb == ACTION_OPEN) || (verb == ACTION_CLOSE)) &&
	           (obj1._id == OUTERHATCH_TOP)) {
		_vm->renderMessage("Du muát erst hingehen.");
	} else if ((verb == ACTION_CLOSE) && (obj1._id == LANDINGMOD_HATCH) &&
	           (isSectionVisible(4) || isSectionVisible(6)))
		_vm->renderMessage("Das Kabel ist im Weg.");
	else if (((verb == ACTION_TAKE) && (obj1._id == HOLD_WIRE)) ||
	         ((verb == ACTION_USE) && Object::combine(obj1, obj2, HOLD_WIRE, LANDINGMOD_HATCH)))
		_vm->renderMessage("Das Kabel ist schon ganz|richtig an dieser Stelle.");
	else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, TERMINALSTRIP, HOLD_WIRE)) {
		getObject(0)->_name = "Leitung mit Lsterklemme";
		_gm->_inventory.remove(*getObject(2));
		_gm->_state.terminalStripConnected = true;
		_gm->_state.terminalStripWire = true;
		_vm->renderMessage("Ok.");
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, HOLD_WIRE, SPOOL)) {
		if (!_gm->_state.terminalStripConnected)
			_vm->renderMessage("Womit denn?");
		else {
			_gm->drawImage(5);
			getObject(0)->_name = "langes Kabel mit Stecker";
			getObject(0)->_click = 10;
			r = _gm->_rooms[CABIN_L2];
			_gm->_inventory.remove(*getObject(9));
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, HOLD_WIRE, GENERATOR_TOP)) {
		if (isSectionVisible(5)) {
			r = _gm->_rooms[GENERATOR];
			r->getObject(0)->_click = 15;
			r->getObject(1)->_click = 13;
			r->setSectionVisible(6, false);
			r->setSectionVisible(8, false);
			_gm->drawImage(_gm->invertSection(5));
			_gm->drawImage(6);
			setSectionVisible(4, false);
			getObject(0)->_click = 11;
		} else {
			_vm->renderMessage("Die Leitung ist zu kurz.");
		}
	} else {
		return false;
	}

	return true;
}

void ShipHold::onEntrance() {
	if (!hasSeen())
		_vm->renderMessage("Was ist denn das f\201r ein Chaos?|Und au\341erdem fehlt das Notraumschiff!|Jetzt wird mir einiges klar.|Die anderen sind gefl\201chtet,|und ich habe es verpennt.");
	setRoomSeen(true);
	_gm->_rooms[COCKPIT]->setRoomSeen(true);
}

bool ShipLandingModule::interact(Action verb, Object &obj1, Object &obj2) {
	Room *r;
	if ((verb == ACTION_PRESS) && (obj1._id == LANDINGMOD_BUTTON))
		_vm->renderMessage(obj1._description);
	else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, PEN, LANDINGMOD_BUTTON)) {
		if (_gm->_state.landingModuleEnergy) {
			r = _gm->_rooms[GENERATOR];
			if (isSectionVisible(7)) {
				_gm->drawImage(_gm->invertSection(9));
				_gm->drawImage(_gm->invertSection(2));
				_gm->drawImage(_gm->invertSection(8));
				_gm->drawImage(_gm->invertSection(7));
				_gm->drawImage(_gm->invertSection(10));
				if (r->isSectionVisible(9))
					_gm->_state.powerOff = true;
				_gm->roomBrightness();
				_vm->paletteBrightness();
			} else {
				_gm->drawImage(7);
				if (r->isSectionVisible(9))
					_gm->_state.powerOff = false;
//				load("MSN_DATA.025");
				_gm->roomBrightness();
				_vm->paletteBrightness();
				r = _gm->_rooms[SLEEP];
				r->setSectionVisible(1, false);
				r->setSectionVisible(2, false);
				_gm->wait2(2);
				_gm->drawImage(2);
				_gm->wait2(3);
				_gm->drawImage(8);
				_gm->wait2(2);
				_gm->drawImage(9);
				_gm->wait2(1);
				_gm->drawImage(10);
			}
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, KNIFE, LANDINGMOD_BUTTON))
		_vm->renderMessage("Es ist nicht spitz genug.");
	else if ((verb == ACTION_LOOK) && (obj1._id == LANDINGMOD_MONITOR) &&
	         isSectionVisible(7))
		_vm->renderMessage("Du wirst aus den Anzeigen nicht schlau.");
	else if ((verb == ACTION_USE) && (obj1._id == KEYBOARD))
		_vm->renderMessage("La\341 lieber die Finger davon!");
	else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, WIRE, LANDINGMOD_SOCKET)) {
		r = _gm->_rooms[CABIN_L3];
		_gm->_inventory.remove(*r->getObject(8));
		getObject(4)->_name = r->getObject(8)->_name;
		_gm->drawImage(4);
		if (_gm->_state.cableConnected) {
			_gm->drawImage(5);
			getObject(4)->_click = 6;
		} else {
			getObject(4)->_click = 5;
			if (_gm->_state.terminalStripWire)
				_gm->drawImage(11);
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SPOOL, LANDINGMOD_SOCKET))
		_vm->renderMessage("An dem Kabel ist doch gar kein Stecker.");
	else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, LANDINGMOD_WIRE, TERMINALSTRIP)) {
		_gm->drawImage(11);
		getObject(4)->_name = "Leitung mit L\201sterklemme";
		r = _gm->_rooms[HOLD];
		_gm->_inventory.remove(*r->getObject(2));
		_gm->_state.terminalStripConnected = true;
		_gm->_state.terminalStripWire = true;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, LANDINGMOD_WIRE, SPOOL)) {
		if (!_gm->_state.terminalStripConnected)
			_vm->renderMessage("Womit denn?");
		else {
			_gm->drawImage(5);
			getObject(4)->_name = "langes Kabel mit Stecker";
			getObject(4)->_click = 6;
			r = _gm->_rooms[CABIN_L2];
			_gm->_inventory.remove(*r->getObject(9));
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, LANDINGMOD_WIRE, LANDINGMOD_HATCH)) {
		if (getObject(5)->hasProperty(OPENED)) {
			r = _gm->_rooms[HOLD];
			if (isSectionVisible(5)) {
				_gm->_rooms[HOLD]->setSectionVisible(5, false);
				r->getObject(0)->_click = 10;

			} else
				r->getObject(0)->_click = 9;
			_gm->_rooms[HOLD]->setSectionVisible(4, false);
			r->getObject(0)->_name = getObject(4)->_name;
			_gm->drawImage(_gm->invertSection(4));
			_gm->drawImage(_gm->invertSection(5));
			setSectionVisible(11, false);
			_gm->drawImage(6);
			getObject(4)->_click = 7;
		} else {
			_vm->renderMessage("Du solltest die Luke vielleicht erst \224ffnen.");
		}
	} else if ((verb == ACTION_CLOSE) && (obj1._id == LANDINGMOD_HATCH) &&
	           isSectionVisible(6)) {
		_vm->renderMessage("Das Kabel ist im Weg.");
	} else if (((verb == ACTION_TAKE) || (verb == ACTION_PULL)) &&
	         (obj1._id == LANDINGMOD_WIRE)) {
		_vm->renderMessage("Das Kabel ist schon ganz|richtig an dieser Stelle.");
	} else {
		return false;
	}

	return true;
}

bool ShipGenerator::interact(Action verb, Object &obj1, Object &obj2) {
	Room *r;

	if ((verb == ACTION_OPEN) && (obj1._id == OUTERHATCH)) {
		if (obj1.hasProperty(OPENED))
			return false;
		_vm->playSound(kAudioDoorSound);
		_gm->drawImage(1);
		if (isSectionVisible(7))
			_gm->drawImage(10);
		if (isSectionVisible(13))
			_gm->drawImage(13);
		_gm->_rooms[HOLD]->setSectionVisible(3, true);
		obj1.setProperty(OPENED);
		obj1._click = 2;
		_vm->playSound(kAudioDoorOpen);
	} else if ((verb == ACTION_CLOSE) && (obj1._id == OUTERHATCH)) {
		if (!obj1.hasProperty(OPENED))
			return false;
		if (isSectionVisible(11) || isSectionVisible(12)) {
			_vm->renderMessage("Das Seil ist im Weg.");
		} else {
			_vm->playSound(kAudioDoorSound);
			_gm->drawImage(_gm->invertSection(1));
			setSectionVisible(10, false);
			if (isSectionVisible(13))
				_gm->drawImage(13);
			_gm->_rooms[HOLD]->setSectionVisible(3, false);
			obj1.disableProperty(OPENED);
			obj1._click = 1;
			_vm->playSound(kAudioDoorClose);
		}
	} else if ((verb == ACTION_WALK) && (obj1._id == OUTERHATCH) &&
	           isSectionVisible(7)) {
		if (!obj1.hasProperty(OPENED))
			_vm->renderMessage("Das ist geschlossen.");
		else if (!isSectionVisible(11))
			_vm->renderMessage("Das geht nicht.|Die Luke ist mindestens|5 Meter ber dem Boden.");
		else {
			obj1._exitRoom = ROCKS;
			return false;
		}
	} else if ((verb == ACTION_TAKE) && (obj1._id == KEYCARD2)) {
		obj1._name = "Keycard des Commanders";
		return false;
	} else if ((verb == ACTION_TAKE) && (obj1._id == GENERATOR_WIRE))
		_vm->renderMessage("Das Kabel ist schon ganz|richtig an dieser Stelle.");
	else if ((verb == ACTION_PULL) && (obj1._id == SHORT_WIRE) &&
	         (obj1._click != 11)) {
		_gm->drawImage(3);
		_gm->drawImage(4);
		obj1._click = 11;
		_gm->turnOff();
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SHORT_WIRE, CLIP) &&
	           (getObject(11)->_click == 11) && !isSectionVisible(9)) {
		_gm->drawImage(2);
		setSectionVisible(3, false);
		setSectionVisible(4, false);
		getObject(11)->_click = 10;
		if (_gm->_state.shipEnergy)
			_gm->turnOn();
		else
			_gm->drawImage(4);
	} else if ((verb == ACTION_OPEN) && (obj1._id == TRAP)) {
		_vm->playSound(kAudioDoorSound);
		_gm->drawImage(2);
		if (getObject(11)->_click == 11)
			_gm->drawImage(3);
		if (_gm->_state.powerOff)
			_gm->drawImage(4);
		obj1.setProperty(OPENED);
		obj1._click = 6;

		obj1._click2 = 5;
		_vm->playSound(kAudioDoorOpen);
	} else if ((verb == ACTION_CLOSE) && (obj1._id == TRAP)) {
		if (isSectionVisible(9))
			_vm->renderMessage("Das Kabel ist im Weg.");
		else {
			setSectionVisible(3, false);
			return false;
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, GENERATOR_WIRE, CLIP) &&
	           isSectionVisible(3) && (getObject(0)->_click != 16)) {
		_gm->drawImage(_gm->invertSection(8));
		_gm->drawImage(2);
		setSectionVisible(4, false);
		_gm->drawImage(3);
		_gm->drawImage(9);
		getObject(0)->_click = 16;
		r = _gm->_rooms[LANDINGMODULE];
		if (_gm->_state.landingModuleEnergy && r->isSectionVisible(7))
			_gm->turnOn();
		else
		_gm->drawImage(4);
		_gm->_rooms[HOLD]->setSectionVisible(7, true);
		_gm->great(3);
	} else if ((verb == ACTION_PULL) && (obj1._id == GENERATOR_WIRE) &&
	           (obj1._click == 16)) {
		_gm->drawImage(_gm->invertSection(9));
		_gm->drawImage(2);
		_gm->drawImage(3);
		_gm->drawImage(4);
		_gm->drawImage(8);
		obj1._click = 15;
		_gm->turnOff();
		_gm->_rooms[HOLD]->setSectionVisible(7, false);
	} else if ((verb == ACTION_USE) &&
	           (Object::combine(obj1, obj2, WIRE, CLIP) ||
	            Object::combine(obj1, obj2, SPOOL, CLIP)) &&
	           isSectionVisible(3)) {
		_vm->renderMessage("Was n\201tzt dir der Anschlu\341|ohne eine Stromquelle?!");
	} else if ((verb == ACTION_LOOK) && (obj1._id == VOLTMETER)) {
		if (_gm->_state.powerOff)
			_vm->renderMessage("Die Spannung ist auf Null abgesunken.");
		else
			_vm->renderMessage("Es zeigt volle Spannung an.");
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, LADDER, ROPE)) {
		_gm->drawImage(13);
		r = _gm->_rooms[CABIN_R3];
		_gm->_inventory.remove(*r->getObject(9));
		getObject(3)->_click = 18;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, OUTERHATCH, GENERATOR_ROPE)) {
		if (!isSectionVisible(1))
			_vm->renderMessage("Du mu\341t die Luke erst \224ffnen.");
		else {
			_gm->drawImage(_gm->invertSection(13));
			_gm->drawImage(1);
			if (isSectionVisible(7)) {
				_gm->drawImage(10);
				_gm->drawImage(11);
			} else {
				_gm->drawImage(12);
			}
			r = _gm->_rooms[OUTSIDE];
			r->setSectionVisible(1, true);
			r->getObject(1)->_click = 1;
			getObject(3)->_click = 17;
		}
	} else if ((verb == ACTION_TAKE) && (obj1._id == GENERATOR_ROPE)) {
		_vm->renderMessage("Das Seil ist hier schon ganz richtig.");
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, OUTERHATCH, GENERATOR_WIRE) &&
	           isSectionVisible(1)) {
		_vm->renderMessage("Das Kabel ist zu kurz.");
	} else {
		return false;
	}

	return true;
}

// Arsano
void ArsanoRocks::onEntrance() {

}

bool ArsanoRocks::interact(Action verb, Object &obj1, Object &obj2) {

}

void ArsanoMeetup::onEntrance() {

}

void ArsanoMeetup::animation() {

}

bool ArsanoMeetup::interact(Action verb, Object &obj1, Object &obj2) {

}

void ArsanoEntrance::animation() {

}

bool ArsanoEntrance::interact(Action verb, Object &obj1, Object &obj2) {

}

void ArsanoRemaining::animation() {

}

void ArsanoRoger::onEntrance() {

}

void ArsanoRoger::animation() {

}

bool ArsanoRoger::interact(Action verb, Object &obj1, Object &obj2) {

}

void ArsanoGlider::onEntrance() {

}

void ArsanoGlider::animation() {

}

void ArsanoMeetup2::onEntrance() {

}

bool ArsanoMeetup2::interact(Action verb, Object &obj1, Object &obj2) {

}

bool ArsanoMeetup3::interact(Action verb, Object &obj1, Object &obj2) {

}

void AxacussCell::onEntrance() {

}

void AxacussCell::animation() {

}

bool AxacussCell::interact(Action verb, Object &obj1, Object &obj2) {

}

void AxacussCorridor1::onEntrance() {

}

void AxacussCorridor2::onEntrance() {

}

void AxacussCorridor3::onEntrance() {

}

void AxacussCorridor4::onEntrance() {

}

void AxacussCorridor4::animation() {

}

bool AxacussCorridor4::interact(Action verb, Object &obj1, Object &obj2) {

}

void AxacussCorridor5::onEntrance() {

}

bool AxacussCorridor5::interact(Action verb, Object &obj1, Object &obj2) {

}

void AxacussCorridor6::onEntrance() {

}

bool AxacussCorridor6::interact(Action verb, Object &obj1, Object &obj2) {

}

void AxacussCorridor7::onEntrance() {

}

void AxacussCorridor8::onEntrance() {

}

bool AxacussCorridor8::interact(Action verb, Object &obj1, Object &obj2) {

}

void AxacussCorridor9::onEntrance() {

}

bool AxacussCorridor9::interact(Action verb, Object &obj1, Object &obj2) {

}

void AxacussBcorridor::onEntrance() {

}

bool AxacussBcorridor::interact(Action verb, Object &obj1, Object &obj2) {

}

bool AxacussIntersection::interact(Action verb, Object &obj1, Object &obj2) {

}

bool AxacussExit::interact(Action verb, Object &obj1, Object &obj2) {

}

bool AxacussOffice1::interact(Action verb, Object &obj1, Object &obj2) {

}

bool AxacussOffice2::interact(Action verb, Object &obj1, Object &obj2) {

}

bool AxacussOffice3::interact(Action verb, Object &obj1, Object &obj2) {

}

bool AxacussOffice4::interact(Action verb, Object &obj1, Object &obj2) {

}

void AxacussOffice5::onEntrance() {

}

bool AxacussOffice5::interact(Action verb, Object &obj1, Object &obj2) {

}

bool AxacussElevator::interact(Action verb, Object &obj1, Object &obj2) {

}

bool AxacussStation::interact(Action verb, Object &obj1, Object &obj2) {

}

bool AxacussSign::interact(Action verb, Object &obj1, Object &obj2) {

}

}
