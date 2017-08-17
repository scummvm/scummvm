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
		if (!_gm->_guiEnabled || isSectionVisible(15))
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
	_gm->great(8);
}

bool ArsanoRocks::interact(Action verb, Object &obj1, Object &obj2) {
	if (((verb == ACTION_PULL) || (verb == ACTION_PRESS)) &&
	    (obj1._id == STONE) && !isSectionVisible(3)) {
		_gm->drawImage(1);
		_gm->wait2(2);
		_gm->drawImage(2);
		_gm->wait2(2);
		_gm->drawImage(3);
		_vm->playSound(kAudioRocks);
		obj1._click = 3;
		getObject(3)->_click = 4;
		getObject(3)->setProperty(EXIT);
		return true;
	}
	return false;
}

void ArsanoMeetup::onEntrance() {
	if (isSectionVisible(7)) {
		_gm->wait2(3);
		_gm->drawImage(6);
		setSectionVisible(7, false);
		_gm->wait2(3);
		_gm->drawImage(_gm->invertSection(6));
	}
	if (!(_gm->_state.greatFlag & 0x8000)) {
		_vm->playSound(kAudioGreat);
		_gm->_state.greatFlag |= 0x8000;
	}
}

void ArsanoMeetup::animation() {
	_gm->drawImage(_gm->invertSection(1) + _beacon);
	_beacon = (_beacon + 1) % 5;
	_gm->drawImage(_beacon + 1);
	_gm->drawImage(_beacon + 8);
	if (isSectionVisible(_sign + 13))
		_gm->drawImage(_gm->invertSection(13) + _sign);
	else
		_gm->drawImage(13 + _sign);

	_sign = (_sign + 1) % 14;
	_gm->setAnimationTimer(3);
}

bool ArsanoMeetup::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_WALK) &&
	    ((obj1._id == SPACESHIPS) ||
	     ((obj1._id == SPACESHIP) && !obj1.hasProperty(OPENED)))) {
		_vm->renderMessage("Die Raumschifft sind alle verschlossen.");
	} else if ((verb == ACTION_WALK) && (obj1._id == SPACESHIP)) {
		_gm->changeRoom(GLIDER);
		_gm->_newRoom = true;
	} else if ((verb == ACTION_WALK) && (obj1._id == STAR)) {
		_vm->renderMessage("Unsinn!");
	} else if ((verb == ACTION_LOOK) && (obj1._id == STAR)) {
		_vm->renderImage(26, 0);
		_vm->paletteBrightness();
		_gm->animationOff();
		_gm->getInput();
		_gm->animationOn();
		g_system->fillScreen(kColorBlack);
		// TODO: those two function calls needed?
		_vm->renderRoom(*this);
		_vm->paletteBrightness();
	} else if ((verb == ACTION_WALK) && (obj1._id == DOOR)) {
		_gm->drawImage(6);
		_gm->wait2(3);
		_gm->drawImage(7);
		setSectionVisible(6, false);
		_gm->wait2(3);

		return false;
	} else if ((verb == ACTION_LOOK) && (obj1._id == MEETUP_SIGN) && _gm->_state.language) {
		if (_gm->_state.language == 2)
			_vm->renderMessage("Komisch! Auf einmal kannst du|das Schild lesen! Darauf steht:|\"Treffpunkt Galactica\".");

		obj1._description = "Darauf steht:|\"Treffpunkt Galactica\".";
		if (_gm->_state.language == 1)
			return false;

		_gm->_state.language = 1;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, KEYCARD_R, SPACESHIP)) {
		getObject(5)->setProperty(OPENED);
		_gm->changeRoom(GLIDER);
		_gm->_newRoom = true;
	} else {
		return false;
	}

	return true;
}

void ArsanoEntrance::animation() {
	if (!_vm->_messageDisplayed && isSectionVisible(kMaxSection - 5)) {
		_vm->playSound(kAudioDoorSound);
		_gm->drawImage(8);
		setSectionVisible(9, false);
		_gm->wait2(2);
		_gm->drawImage(7);
		setSectionVisible(8, false);
		_gm->wait2(2);
		_gm->drawImage(6);
		setSectionVisible(7, false);
		_gm->wait2(2);
		_gm->drawImage(5);
		setSectionVisible(6, false);
		_gm->wait2(2);
		_gm->drawImage(_gm->invertSection(5));
		getObject(11)->_click = 255;
		setSectionVisible(kMaxSection - 5, false);
	}
	if (isSectionVisible(2)) {
		_gm->drawImage(_gm->invertSection(2));
	} else {
		if (_eyewitness) {
			--_eyewitness;
		} else {
			_eyewitness = 20;
			_gm->drawImage(2);
		}
	}

	_gm->setAnimationTimer(4);
}

bool ArsanoEntrance::interact(Action verb, Object &obj1, Object &obj2) {
	// TODO: Refactor row/dialog data structure for dialog()
	byte zeilen1[5] = {1, 1, 1, 1, 1};
	byte zeilen2[5] = {1, 1, 1, 1, 1};
	byte zeilen3[2] = {1, 1};

	char e;

	if ((verb == ACTION_TALK) && (obj1._id == PORTER)) {
		if (_gm->_rooms[AIRLOCK]->getObject(4)->hasProperty(WORN)) {
			_vm->renderMessage("Durch deinen Helm kannst|du nicht sprechen.");
		} else {
			if (_gm->_state.language) {
				do {
					if (_gm->_state.shoes == 1) {
						_dialog2[2] = "Wo soll ich die Schuhe ablegen?";
						_gm->addSentence(2, 2);
					} else if (_gm->_state.shoes > 1) {
						_gm->removeSentence(2, 2);
					}
					switch (e = _gm->dialog(5, nullptr, nullptr, 2)) { // row2, dialog2
					case 0:
						_gm->reply("Was, das wissen Sie nicht?", 1, _gm->invertSection(1));
						_gm->reply("Sie befinden sich im Restaurant|\"Treffpunkt Galactica\".", 1, _gm->invertSection(1));
						_gm->reply("Wir sind bei den interessantesten|Ereignissen in der Galaxis|immer zur Stelle.", 1, _gm->invertSection(1));
						_gm->removeSentence(1, 1);
						break;
					case 1:
						_gm->reply("Wenn Sie meinen.", 1, _gm->invertSection(1));
						_gm->addSentence(1, 2);
						break;
					case 2:
						if (_gm->_state.shoes == 1) {
							_gm->reply("In der Toilette gibt es|Schlieáfcher fr Schuhe.", 1, _gm->invertSection(1));
							_gm->_state.shoes = 2;
						} else {
							_gm->reply("Wenn Sie das Lokal betreten|wollen, mssen Sie erst|ihre Schuhe ausziehen.", 1, _gm->invertSection(1));
							_gm->_state.shoes = 1;
						}
						break;
					case 3:
						_gm->reply("Wollen Sie, daá ich Sie rauáschmeiáe?", 1, _gm->invertSection(1));
					}
				} while (e != 4);
			} else {
				if (_gm->dialog(5, nullptr, nullptr, 0) != 4) // row2, dialog2
					_gm->reply("Hhius otgfh Dgfdrkjlh Fokj gf.", 1, _gm->invertSection(1));
			}
		}
	} else if ((verb == ACTION_WALK) && (obj1._id == STAIRCASE) && (_gm->_state.shoes != 3)) {
		_gm->drawImage(3);
		_gm->wait2(2);
		_gm->drawImage(4);
		setSectionVisible(3, false);
		if (_gm->_rooms[AIRLOCK]->getObject(4)->hasProperty(WORN))
			_gm->reply("|", 1, _gm->invertSection(1));
		else if (_gm->_state.language)
			_gm->reply("Halt!", 1, _gm->invertSection(1));
		else
			_gm->reply("Uhwdejkt!", 1, _gm->invertSection(1));
		_gm->drawImage(3);
		setSectionVisible(4, false);
		_gm->wait2(2);
		_gm->drawImage(_gm->invertSection(3));
		if (!_gm->_rooms[AIRLOCK]->getObject(4)->hasProperty(WORN)) {
			if (_gm->_state.language) {
				if (_gm->_state.shoes)
					_gm->reply("Sie mssen erst ihre Schuhe ausziehen, Sie Trottel!", 1, _gm->invertSection(1));
				else
					_gm->reply("Was fllt ihnen ein!|Sie knnen doch ein Lokal|nicht mit Schuhen betreten!", 1, _gm->invertSection(1));
				e = 0;
				while ((e < 3) && (_shown[kMaxSection - 1] != 15)) {
					switch (e = _gm->dialog(5, nullptr, nullptr, 1)) { // row1, dialog1
					case 0:
						_gm->reply("Fragen Sie nicht so doof!", 1, 1 + 128);
						break;
					case 1:
						_gm->reply("Was, das wissen Sie nicht?", 1, 1 + 128);
						_gm->reply("Sie befinden sich im Restaurant|\"Treffpunkt Galactica\".", 1, 1 + 128);
						_gm->reply("Wir sind bei den interessantesten|Ereignissen in der Galaxis|immer zur Stelle.", 1, 1 + 128);
						_gm->removeSentence(0, 2);
						break;
					case 2:
						_gm->reply("In der Toilette gibt es|Schlieáfcher fr Schuhe.", 1, 1 + 128);
						_gm->_state.shoes = 2;
						break;
					case 3:
						_gm->drawImage(3);
						_gm->wait2(2);
						_gm->drawImage(4);
						setSectionVisible(3, false);
						_gm->reply("Das wrde ich an ihrer|Stelle nicht versuchen!", 1, 1 + 128);
						_gm->drawImage(3);
						setSectionVisible(4, false);
						_gm->wait2(2);
						_gm->drawImage(_gm->invertSection(3));
						break;
					}
					_gm->removeSentence(0, 1);
				}
			} else {
				_gm->dialog(2, nullptr, nullptr, 0); // row3, dialog3
				_gm->reply("Hhius otgfh Dgfdrkjlh Fokj gf.", 1, 1 + 128);
			}
		}
	} else if ((verb == ACTION_PRESS) && (obj1._id == BATHROOM_BUTTON)) {
		_vm->playSound(kAudioDoorSound);
		_gm->drawImage(5);
		_gm->wait2(2);
		_gm->drawImage(6);
		setSectionVisible(5, false);
		_gm->wait2(2);
		_gm->drawImage(7);
		setSectionVisible(6, false);
		_gm->wait2(2);
		_gm->drawImage(8);
		setSectionVisible(7, false);
		_gm->wait2(2);
		_gm->drawImage(9);
		setSectionVisible(8, false);
		getObject(11)->_click = 9;
	} else if ((verb == ACTION_WALK) && (obj1._id == ARSANO_BATHROOM)) {
		if (_gm->_state.coins) {
			if (_gm->_state.shoes == 2) {
				_vm->renderMessage("Du ziehst deine Schuhe|aus und legst sie in|eins der Schlieáfcher.");
				_gm->_state.shoes = 3;
				_gm->removeSentence(2, 2);
				_gm->removeSentence(3, 2);
			} else if (_gm->_state.shoes == 3) {
				_vm->renderMessage("Du ziehst deine Schuhe wieder an.");
				_gm->_state.shoes = 2;
			} else
				_vm->renderMessage("Du durchsuchst die Klos nach|anderen brauchbaren Sachen,|findest aber nichts.");
		} else {
			if (_gm->_rooms[AIRLOCK]->getObject(5)->hasProperty(WORN))
				_vm->renderMessage("Bevor du aufs Klo gehst,|solltest du besser deinen|Raumanzug ausziehen.");
			else {
				_vm->renderMessage("Du gehst seit sieben Jahren das|erste Mal wieder aufs Klo!");
				_gm->mouseWait(_gm->_timer1);
				_vm->removeMessage();
				_vm->renderMessage("In einem der Schlieáfcher,|die sich auch im Raum befinden,|findest du einige Mnzen.");
				_gm->takeObject(*getObject(16));
				_gm->_state.coins = 5;
			}
		}
		_shown[kMaxSection - 5] = true;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, COINS, CAR_SLOT)) {
		if ((_gm->_state.coins < 5) && (getObject(7 - _gm->_state.coins)->_click == 7))
			_vm->renderMessage("Mach doch zuerst das Fach leer!");
		else {
			_gm->drawImage(15 - _gm->_state.coins);
			getObject(8 - _gm->_state.coins)->_click = 7;
			--_gm->_state.coins;
			if (_gm->_state.coins == 1) {
				getObject(16)->_name = "M\201nze";
			}
			if (_gm->_state.coins == 0) {
				_gm->_inventory.remove(*getObject(16));
				_gm->_state.coins = 255;
			}
		}
	} else if ((verb == ACTION_LOOK) && (obj1._id == KITCHEN_SIGN) && _gm->_state.language) {
		if (_gm->_state.language == 2)
			_vm->renderMessage("Komisch! Auf einmal kannst du|das Schild lesen! Darauf steht:|\"Zutritt nur fr Personal\".");
		obj1._description = "Darauf steht:|\"Zutritt nur fr Personal\".";
		if (_gm->_state.language == 1)
			return false;
		_gm->_state.language = 1;
	} else if ((verb == ACTION_LOOK) && (obj1._id == BATHROOM_SIGN) && _gm->_state.language) {
		if (_gm->_state.language == 2)
			_vm->renderMessage("Komisch! Auf einmal kannst|du das Schild lesen!|Darauf steht:\"Toilette\".");
		obj1._description = "Darauf steht:|\"Toilette\".";
		if (_gm->_state.language == 1)
			return false;
		_gm->_state.language = 1;
	} else if ((verb == ACTION_WALK) && (obj1._id == MEETUP_EXIT)) {
		if (!((_gm->_rooms[AIRLOCK]->getObject(4)->hasProperty(WORN)) &&
		      (_gm->_rooms[AIRLOCK]->getObject(5)->hasProperty(WORN)) &&
		      (_gm->_rooms[AIRLOCK]->getObject(6)->hasProperty(WORN)))) {
			_vm->renderMessage("Du ziehst den Raumanzug wieder an.");
			getObject(4)->setProperty(WORN);
			getObject(5)->setProperty(WORN);
			getObject(6)->setProperty(WORN);
			_gm->mouseWait(_gm->_timer1);
			_vm->removeMessage();
		}
		return false;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, KNIFE, PORTER))
		_vm->renderMessage("Nicht so gewaltttig!");
	else
		return false;
	return true;
}

void ArsanoRemaining::animation() {
	switch (_i) {
	case  0:
		_gm->drawImage(1);
		_gm->drawImage(_gm->invertSection(4));
		break;
	case  1:
		_gm->drawImage(_gm->invertSection(1));
		_gm->drawImage(4);
		break;
	case  2:
		_gm->drawImage(2);
		_gm->drawImage(_gm->invertSection(4));
		break;

	case  3:
		_gm->drawImage(7); // Dragon
		_gm->drawImage(_gm->invertSection(2));
		_gm->drawImage(4);
		break;
	case  4:
		_gm->drawImage(8);
		setSectionVisible(7, false);
		_gm->drawImage(2);
		_gm->drawImage(_gm->invertSection(4));
		break;
	case  5:
		_gm->drawImage(_gm->invertSection(8));
		_gm->drawImage(_gm->invertSection(2));
		break;
	case  6:
		_gm->drawImage(3);
		_gm->drawImage(2);
		break;
	case  7:
		_gm->drawImage(_gm->invertSection(3));
		_gm->drawImage(_gm->invertSection(2));
		break;
	case  8:
		_gm->drawImage(3);
		break;
	case  9:
		_gm->drawImage(14); // Card Player 1
		_gm->drawImage(4);
		_gm->drawImage(_gm->invertSection(3));
		break;
	case 10:
		_gm->drawImage(15);
		_gm->drawImage(14);
		_gm->drawImage(_gm->invertSection(4));
		_gm->drawImage(3);
		break;
	case 11:
		_gm->drawImage(16);
		setSectionVisible(15, false);
		_gm->drawImage(4);
		_gm->drawImage(_gm->invertSection(3));
		break;
	case 12:
		_gm->drawImage(17);
		setSectionVisible(16, false);
		_gm->drawImage(_gm->invertSection(4));
		_gm->drawImage(3);
		break;
	case 13:
		_gm->drawImage(_gm->invertSection(17));
		_gm->drawImage(4);
		_gm->drawImage(_gm->invertSection(3));
		break;
	case 14:
		_gm->drawImage(_gm->invertSection(4));
		break;
	case 15:
		_gm->drawImage(6);
		break;
	case 16:
		_gm->drawImage(18); // Card Player 2
		_gm->drawImage(5);
		break;
	case 17:
		_gm->drawImage(19);
		setSectionVisible(18, false);
		_gm->drawImage(_gm->invertSection(5));
		break;
	case 18:
		_gm->drawImage(20);
		setSectionVisible(19, false);
		_gm->drawImage(5);
		break;
	case 19:
		_gm->drawImage(21);
		setSectionVisible(20, false);
		_gm->drawImage(_gm->invertSection(5));
		break;
	case 20:
		_gm->drawImage(_gm->invertSection(21));
		_gm->drawImage(5);
		break;
	case 21:
		_gm->drawImage(_gm->invertSection(5));
		break;
	case 22:
		_gm->drawImage(5);
		break;
	case 23:
		_gm->drawImage(10);
		_chewing = false;
		_gm->drawImage(_gm->invertSection(5));
		break;
	case 24:
		_gm->drawImage(11);
		setSectionVisible(10, false);
		break;
	case 25:
		_gm->drawImage(12);
		setSectionVisible(11, false);
		break;
	case 26:
		_gm->drawImage(13);
		setSectionVisible(12, false);
		break;
	case 27:
		_gm->drawImage(12);
		setSectionVisible(13, false);
		break;
	case 28:
		_gm->drawImage(11);
		setSectionVisible(12, false);
		break;
	case 29:
		_gm->drawImage(10);
		setSectionVisible(11, false);
		break;
	case 30:
		_gm->drawImage(_gm->invertSection(10));
		_chewing = true;
		break;
	case 31:
		_gm->drawImage(22); // Card Player 3
		break;
	case 32:
		_gm->drawImage(_gm->invertSection(22));
		break;
	case 33:
		_gm->drawImage(_gm->invertSection(6));
		break;
	case 34:
		_gm->drawImage(4);
	}

	_i = (_i + 1) % 35;
	if (_chewing) {
		if (isSectionVisible(9))
			_gm->drawImage(_gm->invertSection(9));
		else
			_gm->drawImage(9);
	}
	_gm->setAnimationTimer(3);
}

void ArsanoRoger::onEntrance() {
	if (!_shown[kMaxSection - 2]) {
		_gm->say("Darf ich hier Platz nehmen?");
		_gm->reply("Klar!", 2, 2 + 128);
		_shown[kMaxSection - 2] = true;
	}
}

void ArsanoRoger::animation() {
	if (isSectionVisible(1))
		_gm->drawImage(_gm->invertSection(1));
	else if (isSectionVisible(10)) {
		_gm->drawImage(12);
		setSectionVisible(10, false);
		setSectionVisible(12, false);
	} else {
		if (_eyewitness) {
			--_eyewitness;
		} else {
			_eyewitness = 20;
			if (isSectionVisible(3))
				_gm->drawImage(10);
			else
				_gm->drawImage(1);
		}
	}
	if (isSectionVisible(3)) {
		setSectionVisible(5 + _hands, false);
		_hands = (_hands + 1) % 5;
		_gm->drawImage(5 + _hands);
	}
	_gm->setAnimationTimer(4);
}

bool ArsanoRoger::interact(Action verb, Object &obj1, Object &obj2) {
	byte zeilen1[4] = {1, 1, 1, 1};

	if ((verb == ACTION_TAKE) && (obj1._id == WALLET)) {
		if (isSectionVisible(3)) {
			_gm->great(0);
			return false;
		}
		_gm->reply("Hey, Witzkeks, laá die Brieftasche da liegen!", 2, 2 + 128);
	} else if ((verb == ACTION_USE) && (obj1._id == CUP)) {
		_vm->renderMessage("Das ist nicht deine.");
	} else if ((verb == ACTION_TALK) && (obj1._id == ROGER_W)) {
		if (isSectionVisible(3))
			_vm->renderMessage("Roger ist im Moment nicht ansprechbar.");
		else {
			switch (_gm->dialog(4, nullptr, nullptr, 1)) { // row1, dialog1
			case 0:
				_gm->reply("Bestellen Sie lieber nichts!", 2, 2 + 128);
				_gm->reply("Ich habe vor zwei Stunden mein Essen|bestellt und immer noch nichts bekommen.", 2, 2 + 128);
				break;
			case 1:
				_gm->reply("Noch mindestens zwei Stunden.", 2, 2 + 128);
				_gm->reply("Haben Sie keine Idee, womit wir uns|bis dahin die Zeit vertreiben knnen?", 2, 2 + 128);
				_gm->say("Hmm ... im Moment fllt mir nichts ein, aber vielleicht|hat der Spieler des Adventures ja eine Idee.");
				break;
			case 2:
				_gm->reply("Nein, Sie mssen sich irren.|Ich kenne Sie jedenfalls nicht.", 2, 2 + 128);
				_gm->say("Aber ihre Kleidung habe ich irgendwo schon mal gesehen.");
				_gm->reply("Ja? Komisch.", 2, 2 + 128);
				_gm->say("Jetzt weiá ich's. Sie sind Roger W. !");
				_gm->reply("Pssst, nicht so laut, sonst will|gleich jeder ein Autogramm von mir.", 2, 2 + 128);
				_gm->reply("Ich habe extra eine Maske auf, damit|ich nicht von jedem angelabert werde.", 2, 2 + 128);
				_gm->say("h ... ach so.");
				_gm->say("Wann kommt denn das nchste SQ-Abenteuer raus?");
				_gm->reply("SQ 127 máte in einem Monat erscheinen.", 2, 2 + 128);
				_gm->say("Was, Teil 127 ??");
				_gm->say("Bei uns ist gerade Teil 8 erschienen.");
				_gm->reply("Hmm ... von welchem Planeten sind Sie denn?", 2, 2 + 128);
				_gm->say("Von der Erde.");
				_gm->reply("Erde? Nie gehrt.", 2, 2 + 128);
				_gm->reply("Wahrscheinlich irgendein Kaff, wo Neuerungen|erst hundert Jahre spter hingelangen.", 2, 2 + 128);
				_gm->say("h ... kann sein.");
				_gm->reply("Aber eins mssen Sie mir erklren!", 2, 2 + 128);
				_gm->reply("Wieso sehen Sie mir so verdammt hnlich, wenn|Sie nicht von Xenon stammen, wie ich?", 2, 2 + 128);
				_gm->say("Keine Ahnung. Bis jetzt dachte ich immer, Sie wren ein|von Programmierern auf der Erde erfundenes Computersprite.");
				_gm->reply("Was? Lachhaft!", 2, 2 + 128);
				_gm->reply("Wie erklren Sie sich dann,|daá ich ihnen gegenbersitze?", 2, 2 + 128);
				_gm->say("Ja, das ist in der Tat seltsam.");
				_gm->reply("Halt, jetzt weiá ich es. Sie sind von der Konkurrenz,|von \"Georgefilm Games\" und wollen mich verunsichern.", 2, 2 + 128);
				_gm->say("Nein, ich bin nur ein Ahnungsloser Koch von der Erde.");
				_gm->reply("Na gut, ich glaube Ihnen. Lassen wir jetzt|dieses Thema, langsam wird es mir zu bunt!", 2, 2 + 128);
			}
		}
	} else if (((verb == ACTION_USE) && Object::combine(obj1, obj2, CHESS, ROGER_W)) ||
	           ((verb == ACTION_GIVE) && (obj1._id == CHESS) && (obj2._id == ROGER_W))) {
		_gm->drawImage(11);
		_gm->great(0);
		_gm->say("Eine Partie Schach! Das ist eine gute Idee.");
		_gm->reply("Schach? Was ist das denn?", 2, 2 + 128);
		_gm->say("Schach ist ein interessantes Spiel.|Ich werde es Ihnen erklren.");
		_vm->paletteFadeOut();
		_gm->_inventory.remove(*_gm->_rooms[CABIN_R3]->getObject(0)); // Chess board
		g_system->fillScreen(kColorBlack);
		_vm->_menuBrightness = 255;
		_vm->paletteBrightness();
		_vm->renderMessage("Knapp zwei Stunden spter ...");
		_gm->mouseWait(_gm->_timer1);
		_vm->removeMessage();
		_vm->_menuBrightness = 0;
		_vm->paletteBrightness();
		_gm->_state.timeStarting -= 125000; /* 2 Stunden */
		_gm->_state.timeAlarmSystem -= 125000;
		_gm->_state.eventTime = _vm->getDOSTicks() + 4000;
		// TODO: implement event calling
//		_gm->_state.event = &supernova;
		_gm->_state.alarmOn = (_gm->_state.timeAlarmSystem > _vm->getDOSTicks());
		setSectionVisible(11, false);
		setSectionVisible(1, false);
		_vm->renderRoom(*this);
		_gm->drawImage(3);
		getObject(3)->_click = 5;
		getObject(5)->_click = 6;
		getObject(6)->_click = 7;
		_vm->paletteFadeIn();
		_vm->renderMessage("Roger W. steht kurz vor dem Schachmatt|und grbelt nach einem Ausweg.");
		_gm->mouseWait(_gm->_timer1);
		_vm->removeMessage();
	} else {
		return false;
	}

	return true;
}

void ArsanoGlider::animation() {
	if (isSectionVisible(8)) {
		setSectionVisible(24 + _sinus, false);
		_sinus = (_sinus + 1) % 14;
		_gm->drawImage(24 + _sinus);
	} else if (isSectionVisible(24 + _sinus))
		_gm->drawImage(_gm->invertSection(24 + _sinus));

	_gm->setAnimationTimer(2);
}

bool ArsanoGlider::interact(Action verb, Object &obj1, Object &obj2) {
	static char l, r;
	if ((verb == ACTION_USE) && Object::combine(obj1, obj2, KEYCARD_R, GLIDER_SLOT)) {
		_gm->drawImage(5);
		_gm->wait2(7);
		_gm->drawImage(8);
		getObject(5)->_click = 10;
		_gm->_inventory.remove(*_gm->_rooms[ROGER]->getObject(8));
	} else if (((verb == ACTION_TAKE) || (verb == ACTION_PULL)) &&
	           (obj1._id == GLIDER_KEYCARD)) {
		_gm->drawImage(_gm->invertSection(5));
		_gm->drawImage(_gm->invertSection(8));
		getObject(5)->_click = 255;
		_gm->takeObject(*_gm->_rooms[ROGER]->getObject(8));
		for (int i = 9; i <= 22; i++)
			_gm->drawImage(_gm->invertSection(i));
		l = r = 0;
	} else if ((verb == ACTION_PRESS) &&
	           (obj1._id >= GLIDER_BUTTON1) && (obj1._id <= GLIDER_BUTTON4)) {
		int i = obj1._id - GLIDER_BUTTON1 + 1;
		_gm->drawImage(i);
		if (isSectionVisible(8)) {
			l = 0;
			r = 0;
			for (int j = 1; j < 8; j++) {
				if (isSectionVisible(j + 8))
					l = j;
				if (isSectionVisible(j + 15))
					r = j;
			}
			switch (i) {
			case 1:
				if (l < 7) {
					l++;
					_gm->drawImage(l + 8);
				}
				break;
			case 3:
				if (r < 7) {
					r++;
					_gm->drawImage(r + 15);
				}
				break;
			case 2:
				if (l) {
					_gm->drawImage(_gm->invertSection(l + 8));
					l--;
				}
				break;
			case 4:
				if (r) {
					_gm->drawImage(_gm->invertSection(r + 15));
					r--;
				}
			}
		}
		_gm->wait2(4);
		_gm->drawImage(_gm->invertSection(i));
	} else if ((verb == ACTION_USE) && (obj1._id == GLIDER_BUTTONS)) {
		_vm->renderMessage("Du tippst auf den Tasten herum,|aber es passiert nichts.");
	} else {
		return false;
	}

	return true;
}

void ArsanoMeetup2::onEntrance() {
	switch (!_gm->_guiEnabled) {
	case 1:
		_gm->shipStart();
		break;
	case 2:
		_vm->renderMessage("Alle Raumschiffe haben|den Planeten verlassen.");
		break;
	case 3:
		_vm->renderMessage("Alle Raumschiffe haben den Planeten|verlassen, bis auf eins ...");
	}
	_gm->_guiEnabled = true;
}

bool ArsanoMeetup2::interact(Action verb, Object &obj1, Object &obj2) {
	byte zeilen1[2] = {1, 1};
	byte zeilen2[2] = {1, 1};
	byte zeilen3[4] = {1, 1, 1, 1};
	byte zeilen4[2] = {2, 1};

	bool found, flight;

	if (((verb == ACTION_WALK) &&
	        ((obj1._id == SPACESHIP) || (obj1._id == ROGER_W))) ||
	        ((verb == ACTION_TALK) && (obj1._id == ROGER_W))) {
		_gm->changeRoom(INTRO);
		_vm->renderImage(30, 0);
		_vm->paletteBrightness();
		if (_gm->_rooms[MEETUP2]->isSectionVisible(kMaxSection - 2)) {
			_gm->reply("Was wollen Sie denn schon wieder?", 1, 1 + 128);
			found = !_gm->dialog(2, nullptr, nullptr, 0); // row4, dialog4
			if (!(found))
				_gm->reply("Nein.", 1, 1 + 128);
		} else {
			_gm->reply("Haben Sie zufllig meine Brieftasche gesehen?|Ich muá Sie irgendwo verloren haben.", 1, 1 + 128);
			_gm->reply("Ohne die Brieftasche kann ich nicht|starten, weil meine Keycard darin ist.", 1, 1 + 128);
			found = !_gm->dialog(2, nullptr, nullptr, 0); // row1, dialog1
			_gm->_rooms[MEETUP2]->setSectionVisible(kMaxSection - 2, true);
		}
		if (found) {
			_gm->_inventory.remove(*_gm->_rooms[ROGER]->getObject(3));
			_gm->_inventory.remove(*_gm->_rooms[ROGER]->getObject(7));
			_gm->_inventory.remove(*_gm->_rooms[ROGER]->getObject(8));
			_gm->reply("Oh! Vielen Dank.", 1, 1 + 128);
			_gm->reply("Wo ist denn Ihr Raumschiff?|Soll ich Sie ein Stck mitnehmen?", 1, 1 + 128);
			flight = _gm->dialog(2, nullptr, nullptr, 0); // row2, dialog2
			if (flight) {
				_gm->reply("Wo wollen Sie denn hin?", 1, 1 + 128);
				_gm->dialog(4, nullptr, nullptr, 0); // row3, dialog3
				_gm->reply("Ok, steigen Sie ein!", 1, 1 + 128);
			} else {
				_gm->reply("Wie Sie wollen.", 1, 1 + 128);
			}
			_gm->changeRoom(MEETUP2);
			_gm->_rooms[MEETUP2]->setSectionVisible(12, false);
			_gm->_rooms[MEETUP2]->getObject(0)->_click = 255;
			_gm->_rooms[MEETUP2]->getObject(1)->_click = 255;
			_vm->renderRoom(*this);
			_vm->paletteBrightness();
			_gm->shipStart();
			if (flight) {
				_vm->renderImage(13, 0);
				_vm->paletteBrightness();
				_gm->wait2(36);
				for (int i = 1; i <= 13; i++) {
					if (i > 1)
						_gm->drawImage(_gm->invertSection(i - 1));
					_gm->drawImage(i);
					_gm->wait2(2);
				}
				_gm->drawImage(_gm->invertSection(13));
				_gm->wait2(20);
				_vm->renderImage(14, 0);
				_vm->paletteBrightness();
				_gm->wait2(36);
				for (int i = 1; i <= 13; i++) {
					if (i > 1)
						_gm->drawImage(_gm->invertSection(i - 1));
					_gm->drawImage(i);
					_gm->wait2(2);
				}
				_gm->drawImage(_gm->invertSection(13));
				_gm->wait2(9);
				_vm->playSound(kAudioCrash);
				for (int i = 14; i <= 19; i++) {
					_gm->drawImage(i);
					_gm->wait2(3);
				}
				_vm->paletteFadeOut();
				_vm->renderImage(11, 0);
				_vm->paletteFadeIn();
				_gm->wait2(18);
				_vm->renderMessage("Huch, du lebst ja noch!");
				_gm->great(0);
				_gm->mouseWait(_gm->_timer1);
				_vm->removeMessage();
				_vm->paletteFadeOut();
				g_system->fillScreen(kColorBlack);
				_gm->_state.dream = false;
				// TODO:
//				if (!load_game(-2)) error_temp();
				_gm->loadTime();
				_gm->_newRoom = true;
				_gm->_rooms[CAVE]->getObject(1)->_exitRoom = MEETUP3;
				_gm->_state.dream = true;
			}
		} else {
			_gm->changeRoom(MEETUP2);
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, KEYCARD_R, SPACESHIP))
		_vm->renderMessage("Das wrde ich jetzt nicht tun, schlieálich|steht Roger W. neben seinem Schiff.");
	else
		return false;

	return true;
}

bool ArsanoMeetup3::interact(Action verb, Object &obj1, Object &obj2) {
	byte zeilen2[4] = {1, 1, 1, 1};
	byte zeilen3[2] = {1, 1};

	if ((verb == ACTION_WALK) && (obj1._id == STAR))
		_vm->renderMessage("Unsinn!");
	else if ((verb == ACTION_LOOK) && (obj1._id == STAR)) {
		_vm->renderImage(26, 0);
		_vm->paletteBrightness();
		_gm->getInput();
		g_system->fillScreen(kColorBlack);
	} else if ((verb == ACTION_WALK) && (obj1._id == UFO)) {
		g_system->fillScreen(kColorBlack);
		_vm->renderImage(36, 0);
		_vm->paletteBrightness();
		_gm->dialog(3, nullptr, nullptr, 0); // rowX, dialogX
		_vm->renderImage(36, 1);
		_gm->wait2(3);
		_vm->renderImage(36, 2);
		_gm->wait2(3);
		_vm->renderImage(36, 3);
		_gm->wait2(6);
		_vm->renderImage(36, 4);
		_vm->playSound(kAudioGunShot);
		// TODO: wait until audio finshed playing
		_vm->renderImage(36, 5);
		_gm->wait2(3);
		_vm->renderImage(36, 4);
		_vm->playSound(kAudioGunShot);
		// TODO: wait until audio finshed playing
		_vm->renderImage(36, 5);
		_vm->paletteFadeOut();
		_gm->wait2(12);
		_vm->renderImage(0, 0);
		_vm->paletteFadeIn();
		_gm->wait2(18);
		_gm->reply("Ich glaube, er wacht auf.", 2, 2 + 128);
		_gm->wait2(10);
		_gm->reply("Ja, sieht so aus.", 1, 1 + 128);

		int i;
		do {
			switch (i = _gm->dialog(4, nullptr, nullptr, 2)) { // row2, dialog2
			case 0:
				_gm->reply("Sie befinden sich im Raumschiff \"Dexxa\".", 1, 1 + 128);
				_gm->reply("Wir kommen vom Planeten Axacuss und|sind aus dem gleichen Grund hier wie Sie,|nmlich zur Erforschung der Supernova.", 1, 1 + 128);
				break;
			case 1:
				_gm->reply("Sie knnen beruhigt sein, wir wollen Ihnen nur helfen.", 2, 2 + 128);
				_gm->say("Und wieso hat der Typ im Raumanzug|eben auf mich geschossen?");
				_gm->reply("Das war eine Schreckreaktion.", 2, 2 + 128);
				_gm->reply("Schlieálich ist es fr uns das erste Mal,|daá wir auf eine fremde Intelligenz treffen.", 2, 2 + 128);
				_gm->reply("Wie wir festgestellt haben, ist|Ihr Raumschiff vllig zerstrt.", 2, 2 + 128);
				_gm->reply("Wahrscheinlich knnen Sie nicht|mehr auf ihren Heimatplaneten zurck.", 2, 2 + 128);
				_gm->reply("Wir bieten Ihnen an, Sie|mit nach Axacuss zu nehmen.", 2, 2 + 128);
				if (_gm->dialog(2, nullptr, nullptr, 0)) { // row3, dialog3
					_gm->reply("Sind Sie sich da wirklich sicher?", 2, 2 + 128);
					_gm->say("Wenn ich es mir genau berlege,|fliege ich doch lieber mit.");
				}
				_gm->reply("Gut, wir nehmen Sie unter der|Bedingung mit, daá wir Sie jetzt|sofort in Tiefschlaf versetzen drfen.", 2, 2 + 128);
				_gm->reply("Diese Art des Reisens ist Ihnen|ja scheinbar nicht unbekannt.", 2, 2 + 128);
				_gm->reply("Sie werden in vier Jahren nach der|Landung der \"Dexxa\" wieder aufgeweckt.", 2, 2 + 128);
				_gm->reply("Sind Sie damit einverstanden?", 2, 2 + 128);
				if (_gm->dialog(2, nullptr, nullptr, 0)) { // row3, dialog3
					_gm->reply("Sind Sie sich da wirklich sicher?", 2, 2 + 128);
					_gm->say("Wenn ich es mir genau berlege,|fliege ich doch lieber mit.");
				}
				_gm->reply("Gut, haben Sie noch irgendwelche Fragen?", 2, 2 + 128);
				break;
			case 2:
				_gm->reply("Keine Panik!", 2, 2 + 128);
				_gm->reply("Wir tun Ihnen nichts.", 2, 2 + 128);
				break;
			case 3:
				_gm->reply("Wir sprechen nicht ihre Sprache,|sondern Sie sprechen unsere.", 1, 1 + 128);
				_gm->reply("Nach einer Gehirnanalyse konnten|wir Ihr Gehirn an unsere Sprache anpassen.", 1, 1 + 128);
				_gm->say("Was? Sie haben in mein Gehirn eingegriffen?");
				_gm->reply("Keine Angst, wir haben sonst nichts verndert.", 1, 1 + 128);
				_gm->reply("Ohne diesen Eingriff wren|Sie verloren gewesen.", 1, 1 + 128);
			}
			_gm->removeSentence(2, 2);
		} while (_shown[kMaxSection - 2] != 15);
		_gm->say("Ich habe keine weiteren Fragen mehr.");
		_gm->reply("Gut, dann versetzen wir Sie jetzt in Tiefschlaf.", 1, 1 + 128);
		_gm->reply("Gute Nacht!", 1, 1 + 128);
		_vm->paletteFadeOut();
		_gm->_inventory.clear();
		_gm->_inventory.add(*_gm->_rooms[INTRO]->getObject(1));  // Knife
		_gm->_inventory.add(*_gm->_rooms[INTRO]->getObject(2));  // Watch
		_gm->_inventory.add(*_gm->_rooms[INTRO]->getObject(3));  // Discman
		_gm->changeRoom(CELL);
		_gm->_newRoom = true;
		_gm->_state.benOverlay = 2;
		_gm->_newOverlay = true;
		_gm->_state.dream = true;
	} else
		return false;

	return true;
}

void AxacussCell::onEntrance() {
	if (_gm->_state.dream) {
		_vm->renderMessage("Du wachst auf und findest dich in|einem geschlossenen Raum wieder.");
		_gm->_state.timeStarting = _gm->_state.time - 500000;
		_gm->_state.timeAlarmSystem = _gm->_state.timeAlarm + _gm->_state.timeStarting;
		_gm->_state.alarmOn = (_gm->_state.timeAlarmSystem > _gm->_state.time);
		_gm->_state.powerOff = false;
		_gm->_state.dream = false;
	}
}

void AxacussCell::animation() {
	++_gm->_state.timeRobot;

	if (_gm->_state.timeRobot == 299) {
		_gm->drawImage(_gm->invertSection(31));
		_gm->drawImage(28);
		getObject(0)->_click = 255;
		getObject(1)->setProperty(EXIT | OPENABLE | OPENED | CLOSED);
	} else if ((_gm->_state.timeRobot >= 301) && (_gm->_state.timeRobot <= 320)) {
		_gm->drawImage(_gm->invertSection(329 - _gm->_state.timeRobot));
		_gm->drawImage(328 - _gm->_state.timeRobot);
	} else if (_gm->_state.timeRobot == 321) {
		_gm->drawImage(31);
		setSectionVisible(8, false);
		getObject(0)->_click = 1;
		getObject(1)->setProperty(EXIT | OPENABLE | CLOSED);
	}

	if (_gm->_state.timeRobot == 599) {
		_gm->drawImage(_gm->invertSection(31));
		_gm->drawImage(8);
		getObject(0)->_click = 255;
		getObject(1)->setProperty(EXIT | OPENABLE | OPENED | CLOSED);
	} else if ((_gm->_state.timeRobot >= 601) && (_gm->_state.timeRobot <= 620)) {
		_gm->drawImage(_gm->_state.timeRobot - 593 + 128);
		_gm->drawImage(_gm->_state.timeRobot - 592);
	} else if (_gm->_state.timeRobot == 621) {
		_gm->drawImage(31);
		setSectionVisible(28, false);
		getObject(0)->_click = 1;
		getObject(1)->setProperty(EXIT | OPENABLE | CLOSED);
	} else if (_gm->_state.timeRobot == 700) {
		_gm->_state.timeRobot = 0;
	} else if (_gm->_state.timeRobot == 10002) {
		_gm->drawImage(18 + 128);
		_gm->drawImage(29);
		_gm->drawImage(7);
		getObject(2)->_click = 13;
	} else if (_gm->_state.timeRobot == 10003) {
		setSectionVisible(29, false);
		_gm->drawImage(30);
		getObject(8)->_click = 12;
		getObject(7)->_click = 14;
		_vm->playSound(kAudioUndef4);
	} else if (_gm->_state.timeRobot == 10010) {
		--_gm->_state.timeRobot;
	}

	if (_gm->_state.timeRobot == 312) {
		_gm->drawImage(7);
		getObject(2)->_click = 13;
	} else if (_gm->_state.timeRobot == 610) {
		setSectionVisible(7, false);
		getObject(2)->_click = 255;
	}

	if ((isSectionVisible(6)) &&
	    ((_gm->_state.timeRobot == 310) || (_gm->_state.timeRobot == 610))) {
		_vm->playSound(kAudioUndef3);
		_gm->_state.timeRobot = 10000;
	}

	_gm->setAnimationTimer(3);
}

bool AxacussCell::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_PRESS) && (obj1._id == CELL_BUTTON))
		_vm->renderMessage("Du drckst den Knopf,|aber nichts passiert.");
	else if ((verb == ACTION_PULL) && (obj1._id == CELL_WIRE) &&
	         !isSectionVisible(2) &&
	         !isSectionVisible(3) &&
	         !isSectionVisible(5)) {
		if (isSectionVisible(1)) {
			_gm->drawImage(_gm->invertSection(1));
			_gm->drawImage(2);
			getObject(5)->_click = 7;
		} else if (isSectionVisible(4)) {
			_gm->drawImage(_gm->invertSection(4));
			_gm->drawImage(3);
			getObject(5)->_click = 8;
		} else if (isSectionVisible(6)) {
			_gm->drawImage(_gm->invertSection(6));
			_gm->drawImage(5);
			getObject(5)->_click = 10;
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, CELL_WIRE, SOCKET) &&
	           !isSectionVisible(1) &&
	           !isSectionVisible(4) &&
	           !isSectionVisible(6)) {
		if (isSectionVisible(2)) {
			_gm->drawImage(_gm->invertSection(2));
			_gm->drawImage(1);
			getObject(5)->_click = 6;
		} else if (isSectionVisible(3)) {
			_gm->drawImage(_gm->invertSection(3));
			_gm->drawImage(4);
			getObject(5)->_click = 9;
		} else if (isSectionVisible(5)) {
			_gm->drawImage(_gm->invertSection(5));
			_gm->drawImage(6);
			getObject(5)->_click = 11;
		} else {
			_gm->_inventory.remove(*getObject(5));
			_gm->drawImage(4);
			getObject(5)->_click = 9;
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, CELL_WIRE, KNIFE) &&
	           ((isSectionVisible(1)) || (isSectionVisible(2)))) {
		if (isSectionVisible(1))
			_gm->shock();
		else {
			_gm->drawImage(_gm->invertSection(2));
			_gm->drawImage(3);
			getObject(5)->_click = 8;
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, CELL_WIRE, CELL_TABLE) &&
	           !isSectionVisible(1) &&
	           !isSectionVisible(2) &&
	           !isSectionVisible(5) &&
	           !isSectionVisible(6)) {
		if (isSectionVisible(3)) {
			_gm->drawImage(_gm->invertSection(3));
			_gm->drawImage(5);
			getObject(5)->_click = 10;
		} else if (isSectionVisible(4)) {
			_gm->drawImage(_gm->invertSection(4));
			_gm->drawImage(6);
			_gm->shock();
		} else {
			_gm->_inventory.remove(*getObject(5));
			_gm->drawImage(5);
			getObject(5)->_click = 10;
		}
	} else if ((verb == ACTION_TAKE) && (obj1._id == CELL_WIRE) &&
	           !(obj1.hasProperty(CARRIED))) {
		if (isSectionVisible(3)) {
			_gm->drawImage(_gm->invertSection(3));
			_gm->takeObject(obj1);
		} else if (isSectionVisible(5)) {
			_gm->drawImage(_gm->invertSection(5));
			_gm->takeObject(obj1);
		} else {
			_vm->renderMessage("Das ist befestigt.");
		}
	} else if ((verb == ACTION_WALK) && (obj1._id == CELL_DOOR) &&
	           (obj1.hasProperty(OPENED))) {
		if (isSectionVisible(30) || isSectionVisible(29))
			return false;
		_vm->playSound(kAudioGunShot);
		// TODO: wait till sound is played
		_vm->playSound(kAudioGunShot);
		_vm->playSound(kAudioGunShot);
		_gm->death("Bei deinem Fluchtversuch hat|dich der Roboter erschossen.");
	} else if ((verb == ACTION_USE) && (obj1._id == TRAY))
		_vm->renderMessage("Du iát etwas, aber|es schmeckt scheuálich.");
	else if ((verb == ACTION_TAKE) && (obj1._id == MAGNET)) {
		if (isSectionVisible(6))
			_gm->shock();
		_gm->takeObject(obj1);
		_vm->renderMessage("Ok.");
	} else
		return false;

	return true;
}


void AxacussCorridor1::onEntrance() {

}

void AxacussCorridor2::onEntrance() {

}

void AxacussCorridor3::onEntrance() {

}

void AxacussCorridor4::onEntrance() {
	_gm->great(4);
	_gm->corridorOnEntrance();
	if (_gm->_rooms[GUARD]->isSectionVisible(1))
		_gm->busted(0);
}

void AxacussCorridor4::animation() {

}

bool AxacussCorridor4::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_TAKE) && (obj1._id == NEWSPAPER)) {
		setSectionVisible(9, false);
		_gm->takeObject(obj1);
		if (isSectionVisible(29))
			_gm->drawImage(29);
	} else if (((verb == ACTION_USE) && Object::combine(obj1, obj2, TABLE, WATCH)) ||
	           ((verb == ACTION_GIVE) && (obj1._id == WATCH) &&
	            (obj2._id == TABLE))) {
		if (obj1._id == WATCH)
			_gm->_inventory.remove(obj1);
		else
			_gm->_inventory.remove(obj2);

		_gm->drawImage(29);
		getObject(4)->_click = 8;
	} else if ((verb == ACTION_TAKE) && (obj1._id == WATCH) &&
	           !obj1.hasProperty(CARRIED)) {
		setSectionVisible(29, false);
		getObject(4)->_click = 255;
		_gm->takeObject(*_gm->_rooms[INTRO]->getObject(2));
		if (isSectionVisible(9))
			_gm->drawImage(9);
	} else
		return false;

	return true;
}

void AxacussCorridor5::onEntrance() {

}

bool AxacussCorridor5::interact(Action verb, Object &obj1, Object &obj2) {
	// TODO: needs to be refactored
//	static char
//	rows[] = {1, 1, 1, 1},

	int sum;

	if ((verb == ACTION_WALK) && (obj1._id == DOOR)) {
		g_system->fillScreen(kColorBlack);
		_vm->renderImage(41, 0);
		_vm->paletteBrightness();
		if (_gm->_guiEnabled) {
			_gm->reply("Sie schon wieder?", 1, 1 + 128);
			goto bestechen;
		} else {
			_gm->_guiEnabled = true;
			_gm->reply("Halt! Sie sind doch dieser Hummel.|Bleiben Sie sofort stehen!", 1, 1 + 128);
			if (_gm->dialog(2, nullptr, nullptr, 0)) // rows, dialog1
				_gm->reply("Sehr witzig!", 1, 1 + 128);
			else {
				_gm->reply("Kann auch sein, auf jeden Fall|sind Sie der Nicht-Axacussaner.", 1, 1 + 128);
bestechen:
				if (_gm->dialog(2, nullptr, nullptr, 0) == 0) { // rows, dialog2
					_gm->reply("Nein!", 1, 1 + 128);
					setSectionVisible(kMaxSection - 2, false);
					if (_gm->_state.money == 0) {
						_gm->removeSentence(2, 2);
						_gm->removeSentence(3, 2);
					} else {
						_dialog3[2] += Common::String::format("%d Xa.", _gm->_state.money - 200);
						_dialog3[3] += Common::String::format("%d Xa.", _gm->_state.money);
					}
					switch (_gm->dialog(4, nullptr, nullptr, 2)) { // rows, dialog3
					case 1:
						_gm->wait2(3);
						_gm->drawImage(1);
						_vm->playSound(kAudioVoiceHalt);
						_gm->drawImage(_gm->invertSection(1));
						_gm->wait2(5);
						_gm->drawImage(2);
						_gm->wait2(2);
						_gm->shot(3, _gm->invertSection(3));
						break;
					case 3:
						if (_gm->_state.money >= 900) {
							sum = _gm->_state.money;
							goto genug;
						}
					case 2:
						if (_gm->_state.money > 1100) {
							sum = _gm->_state.money - 200;
							goto genug;
						}
						_gm->reply("Das máte schon ein biáchen mehr sein.", 1, 1 + 128);
					}
				}
			}
		}
		g_system->fillScreen(kColorBlack);
		return true;

genug:
		_gm->reply("Ok, dann machen Sie daá Sie wegkommen!", 1, 1 + 128);
		_gm->great(0);
		_gm->changeRoom(ELEVATOR);
		_gm->_newRoom = true;
		_gm->takeMoney(-sum);
		return true;
	}
	return false;
}

void AxacussCorridor6::onEntrance() {

}

bool AxacussCorridor6::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) &&
	        (obj1.hasProperty(OPENED))) {
		_gm->drawImage(6);
		setSectionVisible(7, false);
		obj1.setProperty(EXIT | OPENABLE | CLOSED);
		_gm->_rooms[CORRIDOR8]->setSectionVisible(27, false);
		_gm->_rooms[CORRIDOR8]->setSectionVisible(28, true);
		_gm->_rooms[CORRIDOR8]->getObject(0)->disableProperty(OPENED);
		_vm->playSound(kAudioDoorClose);
	} else
		return false;

	return true;
}

void AxacussCorridor7::onEntrance() {

}

void AxacussCorridor8::onEntrance() {

}

bool AxacussCorridor8::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_OPEN) && (obj1._id == DOOR) &&
	        !obj1.hasProperty(OPENED)) {
		_gm->drawImage(27);
		setSectionVisible(28, false);
		obj1.setProperty(OPENED);
		_gm->_rooms[CORRIDOR6]->setSectionVisible(6, false);
		_gm->_rooms[CORRIDOR6]->setSectionVisible(7, true);
		_gm->_rooms[CORRIDOR6]->getObject(2)->setProperty(EXIT | OPENED | OPENABLE);
		_gm->_rooms[CORRIDOR6]->getObject(2)->_click = 4;
		_vm->playSound(kAudioDoorOpen);
	} else if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) &&
	           (obj1._type & OPENED)) {
		_gm->drawImage(28);
		setSectionVisible(27, false);
		obj1.disableProperty(OPENED);
		_gm->_rooms[CORRIDOR6]->setSectionVisible(6, true);
		_gm->_rooms[CORRIDOR6]->setSectionVisible(7, false);
		_gm->_rooms[CORRIDOR6]->getObject(2)->setProperty(EXIT | CLOSED | OPENABLE);
		_vm->playSound(kAudioDoorClose);
	} else
		return false;

	return true;
}

void AxacussCorridor9::onEntrance() {

}

bool AxacussCorridor9::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) &&
	        (obj1.hasProperty(OPENED))) {
		_gm->drawImage(28);
		setSectionVisible(27, false);
		obj1.disableProperty(OPENED);
		_gm->_rooms[GUARD]->setSectionVisible(6, false);
		_gm->_rooms[GUARD]->getObject(2)->disableProperty(OPENED);
		_vm->playSound(kAudioDoorClose);
	} else if ((verb == ACTION_OPEN) && (obj1._id == DOOR) &&
	           !obj1.hasProperty(OPENED)) {
		_gm->drawImage(27);
		setSectionVisible(28, false);
		obj1.setProperty(OPENED);
		_gm->_rooms[GUARD]->setSectionVisible(6, true);
		_gm->_rooms[GUARD]->getObject(2)->setProperty(OPENED);
		_vm->playSound(kAudioDoorOpen);
		if (!_gm->_rooms[GUARD]->isSectionVisible(1)) {
			_gm->busted(0);
		}
	} else
		return false;

	return true;
}

void AxacussBcorridor::onEntrance() {
	_gm->corridorOnEntrance();
	if (isSectionVisible(7))
		_gm->busted(-1);
}

bool AxacussBcorridor::interact(Action verb, Object &obj1, Object &obj2) {
	if (obj1.hasProperty(EXIT) ||
	    ((verb == ACTION_USE) && obj1.hasProperty(COMBINABLE) && obj2.hasProperty(EXIT)))
		_gm->_guiEnabled = false;

	if ((verb == ACTION_CLOSE) && (obj1._id >= DOOR1) && (obj1._id <= DOOR4) &&
	    obj1.hasProperty(OPENED)) {
		_gm->drawImage(_gm->invertSection(obj1._id - DOOR1 + 1));
		_vm->playSound(kAudioDoorClose);
		if (obj1.hasProperty(OCCUPIED)) {
			_gm->_state.destination = 255;
			obj1.setProperty(EXIT | OPENABLE | CLOSED | CAUGHT);
			if (!_gm->_rooms[OFFICE_L1 + obj1._id - DOOR1]->isSectionVisible(4))
				_gm->search(180);
			else
				_gm->_state.eventTime = 0xffffffff;
		} else
			obj1.setProperty(EXIT | OPENABLE | CLOSED);
	} else if (((verb == ACTION_WALK) || ((verb == ACTION_OPEN) && !obj1.hasProperty(OPENED))) &&
	           (obj1._id >= DOOR1) && (obj1._id <= DOOR4) &&
	           obj1.hasProperty(OCCUPIED)) {
		_vm->renderMessage(_dontEnter.c_str());
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR1) &&
	           !getObject(4)->hasProperty(OPENED)) {
		if (getObject(4)->hasProperty(OCCUPIED))
			_vm->renderMessage(_dontEnter.c_str());
		else {
			_gm->drawImage(1);
			_vm->playSound(kAudioDoorOpen);
			if (getObject(4)->hasProperty(CAUGHT))
				_gm->busted(11);
			getObject(4)->setProperty(EXIT | OPENABLE | OPENED);
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR2) &&
	           !getObject(5)->hasProperty(OPENED)) {
		if (getObject(5)->hasProperty(OCCUPIED)) {
			_vm->renderMessage(_dontEnter.c_str());
		} else {
			_gm->drawImage(2);
			_vm->playSound(kAudioDoorOpen);
			if (getObject(5)->hasProperty(CAUGHT))
				_gm->busted(16);
			getObject(5)->setProperty(EXIT | OPENABLE | OPENED);
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR3) &&
	           !getObject(6)->hasProperty(OPENED)) {
		if (getObject(6)->hasProperty(OCCUPIED)) {
			_vm->renderMessage(_dontEnter.c_str());
		} else {
			_gm->drawImage(3);
			_vm->playSound(kAudioDoorOpen);
			if (getObject(6)->hasProperty(CAUGHT))
				_gm->busted(15);
			getObject(6)->setProperty(EXIT | OPENABLE | OPENED);
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR4) &&
	           !getObject(7)->hasProperty(OPENED)) {
		if (getObject(7)->hasProperty(OCCUPIED)) {
			_vm->renderMessage(_dontEnter.c_str());
		} else {
			_gm->drawImage(4);
			_vm->playSound(kAudioDoorOpen);
			if (getObject(7)->hasProperty(CAUGHT))
				_gm->busted(20);
			getObject(7)->setProperty(EXIT | OPENABLE | OPENED);
		}
	} else if ((verb == ACTION_LOOK) &&
	           (obj1._id >= DOOR1) && (obj1._id <= DOOR4)) {
		_gm->_state.nameSeen |= 1 << (obj1._id - DOOR1);
		return false;
	} else if ((verb == ACTION_WALK) &&
	           ((obj1._id == PILLAR1) || (obj1._id == PILLAR2))) {
		_vm->renderMessage("Du stellst dich hinter die Sule.");
		_gm->_guiEnabled = true;
	} else
		return false;

	return true;
}

bool AxacussIntersection::interact(Action verb, Object &obj1, Object &obj2) {
	// STUB
	return false;
}

bool AxacussExit::interact(Action verb, Object &obj1, Object &obj2) {
	// STUB
	return false;
}

bool AxacussOffice1::interact(Action verb, Object &obj1, Object &obj2) {
	// TODO: use String for manipulation instead of raw char string?
	char input[11];
	if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) &&
	        obj1.hasProperty(OPENED)) {
		_gm->drawImage(_gm->invertSection(9));
		obj1.setProperty(EXIT | OPENABLE | CLOSED);
		_vm->playSound(kAudioDoorClose);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR) &&
	           !getObject(0)->hasProperty(OPENED)) {
		_gm->drawImage(9);
		getObject(0)->setProperty(EXIT | OPENABLE | OPENED);
		_vm->playSound(kAudioDoorOpen);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, COMPUTER, MAGNET)) {
		_gm->drawImage(4);
		setSectionVisible(16, false);
		_vm->playSound(kAudioUndef2);
	} else if ((verb == ACTION_LOOK) && (obj1._id == COMPUTER)) {
		if (isSectionVisible(4))
			_vm->renderMessage(broken_DE);
		else
			_gm->telomat(0);
	} else if (((verb == ACTION_OPEN) || (verb == ACTION_USE)) &&
	           (obj1._id == LOCKER) &&
	           !obj1.hasProperty(OPENED)) {
		_vm->renderMessage("Welche Zahlenkombination willst|du eingeben?");
		_vm->renderBox(160, 70, 70, 10, kColorDarkBlue);
		input[0] = 0;
		do
			_gm->edit(input, 161, 71, 10);
		while ((_gm->_key.keycode != Common::KEYCODE_ESCAPE) &&
		       (_gm->_key.keycode != Common::KEYCODE_RETURN));
		_vm->removeMessage();
		if (_gm->_key.keycode != Common::KEYCODE_ESCAPE) {
			if (strcmp(input, "89814") != 0) {
				if (strcmp(input, "41898") == 0)
					_vm->renderMessage("Hmm, das haut nicht ganz hin,|aber irgendwie muá die Zahl|mit dem Code zusammenhngen.");
				else
					_vm->renderMessage("Das war die falsche Kombination.");
			} else {
				_gm->drawImage(6);
				setSectionVisible(7, false);
				obj1.setProperty(OPENABLE | OPENED);
				if (getObject(2)->hasProperty(TAKE)) {
					_gm->drawImage(8);
					getObject(2)->_click = 9;
				}
				_vm->playSound(kAudioDoorOpen);
				_gm->great(7);
			}
		}
	} else if ((verb == ACTION_CLOSE) && (obj1._id == LOCKER) &&
	           obj1.hasProperty(OPENED)) {
		_gm->drawImage(7);
		setSectionVisible(6, false);
		obj1.setProperty(OPENABLE | CLOSED);
		setSectionVisible(8, false);
		getObject(2)->_click = 255;
		_vm->playSound(kAudioDoorClose);
	} else if ((verb == ACTION_TAKE) && (obj1._id == TICKETS)) {
		_gm->drawImage(6);
		setSectionVisible(8, false);
		getObject(2)->_click = 255;
		getObject(2)->resetProperty();
		_gm->takeMoney(500);
	} else if ((verb == ACTION_LOOK) && (obj1._id == LETTER)) {
		g_system->fillScreen(kColorBlack);
		_vm->renderText("Streng geheim", 10, 10, 4);
		_vm->renderText("418-98", 270, 10, 4);
		_vm->renderText("Sehr geehrter Dr. Hansi,", 10, 60, 4);
		_vm->renderText("Ich muá Ihren Roboterexperten ein Lob aussprechen. Die", 10, 75, 4);
		_vm->renderText("Imitation von Horst Hummel ist perfekt gelungen, wie ich", 10, 86, 4);
		_vm->renderText("heute bei der bertragung des Interviews feststellen", 10, 97, 4);
		_vm->renderText("konnte. Dem Aufschwung Ihrer Firma durch die Werbe-", 10, 108, 4);
		_vm->renderText("kampagne mit dem falschen Horst Hummel drfte ja jetzt", 10, 119, 4);
		_vm->renderText("nichts mehr im Wege stehen.", 10, 130, 4);
		_vm->renderText("PS: Herzlichen zum Geburtstag!", 10, 147, 4);
		_vm->renderText("Hochachtungsvoll", 200, 170, 4);
		_vm->renderText("Commander Sumoti", 200, 181, 4);
		_gm->getInput();
		g_system->fillScreen(kColorBlack);
		_vm->renderMessage("Nicht zu fassen!");
	} else
		return false;

	return true;
}

bool AxacussOffice2::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) &&
	        obj1.hasProperty(OPENED)) {
		_gm->drawImage(_gm->invertSection(9));
		obj1.setProperty(EXIT | OPENABLE | CLOSED);
		_vm->playSound(kAudioDoorClose);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR) &&
	           !getObject(0)->hasProperty(OPENED)) {
		_gm->drawImage(9);
		getObject(0)->setProperty(EXIT | OPENABLE | OPENED);
		_vm->playSound(kAudioDoorOpen);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, COMPUTER, MAGNET)) {
		_gm->drawImage(4);
		setSectionVisible(16, false);
		_vm->playSound(kAudioUndef2);
	} else if ((verb == ACTION_LOOK) && (obj1._id == COMPUTER)) {
		if (isSectionVisible(4))
			_vm->renderMessage(broken_DE);
		else
			_gm->telomat(1);
	} else
		return false;

	return true;
}

bool AxacussOffice3::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) &&
	        obj1.hasProperty(OPENED)) {
		_gm->drawImage(_gm->invertSection(3));
		obj1.setProperty(EXIT | OPENABLE | CLOSED);
		_vm->playSound(kAudioDoorClose);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR) &&
	           !getObject(0)->hasProperty(OPENED)) {
		_gm->drawImage(3);
		getObject(0)->setProperty(EXIT | OPENABLE | OPENED);
		_vm->playSound(kAudioDoorOpen);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, COMPUTER, MAGNET)) {
		_gm->drawImage(4);
		_vm->playSound(kAudioUndef2);
	} else if ((verb == ACTION_LOOK) && (obj1._id == COMPUTER)) {
		if (isSectionVisible(4))
			_vm->renderMessage(broken_DE);
		else
			_gm->telomat(2);
	} else if ((verb == ACTION_LOOK) && (obj1._id == PAINTING)) {
		_vm->renderMessage("Hey, hinter dem Bild ist Geld|versteckt. Ich nehme es mit.");
		_gm->takeMoney(300);
		obj1._id = NULLOBJECT;
	} else
		return false;

	return true;
}

bool AxacussOffice4::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) &&
	        obj1.hasProperty(OPENED)) {
		_gm->drawImage(_gm->invertSection(3));
		obj1.setProperty(EXIT | OPENABLE | CLOSED);
		_vm->playSound(kAudioDoorClose);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR) &&
	           !getObject(0)->hasProperty(OPENED)) {
		_gm->drawImage(3);
		getObject(0)->setProperty(EXIT | OPENABLE | OPENED);
		_vm->playSound(kAudioDoorOpen);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, COMPUTER, MAGNET)) {
		_gm->drawImage(4);
		_vm->playSound(kAudioUndef2);
	} else if ((verb == ACTION_LOOK) && (obj1._id == COMPUTER)) {
		if (isSectionVisible(4))
			_vm->renderMessage(broken_DE);
		else
			_gm->telomat(3);
	} else
		return false;

	return true;
}

void AxacussOffice5::onEntrance() {
	_gm->great(5);
}

bool AxacussOffice5::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_USE) && Object::combine(obj1, obj2, COMPUTER, MAGNET)) {
		_gm->drawImage(4);
		_vm->playSound(kAudioUndef2);
	} else if ((verb == ACTION_TAKE) && (obj1._id == TICKETS)) {
		_gm->drawImage(_gm->invertSection(5));
		obj1._click = 255;
		_gm->takeMoney(350);
	} else
		return false;

	return true;
}

bool AxacussElevator::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_WALK) && (obj1._id == DOOR)) {
		g_system->fillScreen(kColorBlack);
		_vm->renderImage(41, 0);
		_vm->paletteBrightness();
		_gm->reply("Jetzt verschwinden Sie endlich!", 1, 1 + 128);
		_gm->say("Huch, ich habe mich vertan.");
		g_system->fillScreen(kColorBlack);
	} else if ((verb == ACTION_PRESS) && (obj1._id == BUTTON1)) {
		if (!isSectionVisible(3)) {
			_gm->drawImage(1);
			getObject(2)->resetProperty();
			_vm->playSound(kAudioDoorSound);
			_gm->wait2(25);
			for (int i = 3; i <= 7; i++) {
				_gm->wait2(2);
				_gm->drawImage(i);
			}
			getObject(3)->resetProperty(EXIT);
			getObject(3)->_click = 2;
			_gm->drawImage(_gm->invertSection(1));
			if (!(_gm->_state.greatFlag & 0x4000)) {
				_vm->playSound(kAudioGreat);
				_gm->_state.greatFlag |= 0x4000;
			}
		}
	} else if ((verb == ACTION_PRESS) && (obj1._id == BUTTON2)) {
		if (isSectionVisible(3)) {
			_gm->drawImage(2);
			getObject(3)->resetProperty();
			getObject(3)->_click = 255;
			_vm->playSound(kAudioDoorSound);
			for (int i = 7; i >= 3; i--) {
				_gm->wait2(2);
				_gm->drawImage(_gm->invertSection(i));
			}
			_gm->wait2(25);
			_vm->playSound(kAudioDoorSound);
			getObject(2)->resetProperty(EXIT);
			_gm->drawImage(_gm->invertSection(2));
		}
	} else if ((verb == ACTION_WALK) && (obj1._id == JUNGLE)) {
		_vm->paletteFadeOut();
		g_system->fillScreen(kColorBlack);
		_vm->_menuBrightness = 255;
		_vm->paletteBrightness();
		_vm->renderMessage("Nachdem du zwei Stunden im|Dschungel herumgeirrt bist,|findest du ein Gebude.");
		_gm->mouseWait(_gm->_timer1);
		_vm->removeMessage();
		_vm->_menuBrightness = 0;
		_vm->paletteBrightness();
		// TODO: Reset Time
//		*startingtime -= 125000; // 2 hours
//		*alarmsystime -= 125000;
//		*alarm_on = (*alarmsystime > systime());
		return false;
	} else
		return false;

	return true;
}

bool AxacussStation::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_LOOK) && (obj1._id == STATION_SIGN)) {
		_gm->changeRoom(SIGN);
		_gm->_newRoom = true;
	} else if ((verb == ACTION_WALK) && (obj1._id == DOOR) &&
	           obj1.hasProperty(OPENED)) {
		_gm->great(0);
		_vm->paletteFadeOut();
		_vm->renderImage(35, 0);
		_vm->renderImage(35, 1);
		_vm->paletteFadeIn();
		_gm->wait2(10);
		for (int i = 8; i <= 21; i++) {
			_vm->renderImage(35, i);
			_gm->wait2(2);
			_vm->renderImage(35, _gm->invertSection(i));
		}
		_gm->wait2(18);
		_vm->renderImage(35, _gm->invertSection(1));
		for (int i = 2; i <= 7; i++) {
			_vm->renderImage(35, _gm->invertSection(i));
			_gm->wait2(3);
			_vm->renderImage(35, _gm->invertSection(i));
		}
		_gm->outro();
	} else {
		return false;
	}

	return true;
}

bool AxacussSign::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT, MONEY) &&
	    isSectionVisible(1)) {
		_gm->takeMoney(-180);
		_gm->drawImage(2);
		setSectionVisible(1, false);
		_gm->_state.eventTime = _vm->getDOSTicks() + 600;
//		*event = &taxi;
		return true;
	}
	return false;
}

}
