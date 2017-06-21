#include "common/system.h"

#include "supernova/supernova.h"

namespace Supernova {

bool ShipCorridor::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_PRESS) && (obj1._id == BUTTON)) {
		if (_objectState[6].hasProperty(OPENED)) {
			_vm->playSound(54, 24020);
			_objectState[6].disableProperty(OPENED);
			_vm->renderImage(_gm->_currentRoom->getFileNumber(), 8);
			_shown[9] = false;
			_gm->wait2(2);
			_vm->renderImage(_gm->_currentRoom->getFileNumber(), 7);
			_shown[8] = false;
			_gm->wait2(2);
			_vm->renderImage(_gm->_currentRoom->getFileNumber(), _gm->invertSection(7));
		} else {
			_vm->playSound(54, 24020);
			_objectState[6].setProperty(OPENED);
			_vm->renderImage(_gm->_currentRoom->getFileNumber(), 7);
			_gm->wait2(2);
			_vm->renderImage(_gm->_currentRoom->getFileNumber(), 8);
			_shown[7] = false;
			_gm->wait2(2);
			_vm->renderImage(_gm->_currentRoom->getFileNumber(), 9);
			_shown[8] = false;
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
			_vm->renderImage(_gm->_currentRoom->getFileNumber(), 3);
			_shown[4] = false;
			_gm->wait2(2);
			_vm->renderImage(_gm->_currentRoom->getFileNumber(), 2);
			_shown[3] = false;
			_gm->wait2(2);
			_vm->renderImage(_gm->_currentRoom->getFileNumber(), _gm->invertSection(2));
		} else {
			_objectState[2].setProperty(OPENED);
			_vm->renderImage(_gm->_currentRoom->getFileNumber(), 2);
			_gm->wait2(2);
			_vm->renderImage(_gm->_currentRoom->getFileNumber(), 3);
			_shown[2] = false;
			_gm->wait2(2);
			_vm->renderImage(_gm->_currentRoom->getFileNumber(), 4);
			_shown[3] = false;
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
		_shown[kMaxSection - 1] = true;
		_shown[4] = false;
		_vm->renderBox(0,0,320,200,kColorDarkBlue);
		if (_gm->_state.time == 0) {
			// Destination reached
			_vm->renderText("Flugziel erreicht", 60, 95, kColorWhite99);
			_gm->mouseInput2();
		} else if (_gm->_state.powerOff) {
			// Energy depleted
			_vm->renderText("Energie erschöpft",60,95,kColorWhite99);
			// Artificial coma interrupted
			_vm->renderText("Tiefschlafprozess abgebrochen",60,115,kColorWhite99);
			_gm->mouseInput2();
		} else if (_shown[5]) {
			// Sleep duration in days
			_vm->renderText("Schlafdauer in Tagen:",30,85,kColorWhite99);
			_vm->renderText(Common::String::format("%d",_gm->_state.timeSleep).c_str(),
			                150,85,kColorWhite99);
			_vm->renderText("Bitte legen Sie sich in die angezeigte Schlafkammer.",
			                30,105,kColorWhite99);
			_gm->mouseInput2();
		} else {
			_vm->renderText("Bitte Passwort eingeben:",100,85,kColorWhite99);
			input[0] = 0;
			do {
				_gm->edit(input,100,105,30);
			} while ((_gm->_key != Common::ASCII_RETURN) && (_gm->_key != Common::ASCII_ESCAPE));
			if (_gm->_key == Common::ASCII_ESCAPE) {
				goto escape;
			}
			for (int i = 0; i < 30; ++i) {
				if ((input[i] >= 'a') && (input[i] <= 'z')) {
					input[i] -= 'a' - 'A';
				}
			}
			if (strcmp(input,codeword_DE) != 0) {
				_vm->renderText("Falsches Passwort",100,125,kColorLightRed);
				_gm->wait2(18);
				goto escape;
			}
			_gm->great(6);
			_vm->renderBox(0,0,320,200,kColorDarkBlue);
			_vm->renderText("Schlafdauer in Tagen:",30,85,kColorWhite99);
			do {
				_vm->renderBox(150,85,150,8,kColorDarkBlue);
				input[0] = 0;
				do {
					_gm->edit(input,150,85,10);
				} while ((_gm->_key != Common::ASCII_RETURN) && (_gm->_key != Common::ASCII_ESCAPE));
				if (_gm->_key == Common::ASCII_ESCAPE) {
					goto escape;
				}
				l = atol(input);
				for (size_t i = 0; i < strlen(input); i++) {
					if ((input[i] < '0') || (input[i] > '9')) {
						l = 0;
					}
				}
			} while (l == 0);
			_gm->_state.timeSleep = l;
			_vm->renderText("Bitte legen Sie sich in die angezeigte Schlafkammer.",30,105,kColorWhite99);
			_gm->wait2(18);
			_shown[5] = true;
		}
        escape:
		_vm->renderRoom(*this);
		_gm->showMenu();
		_gm->drawMapExits();
		_gm->palette();
		_shown[kMaxSection - 1] = false;
	} else if (((verb == ACTION_WALK) || (verb == ACTION_USE)) &&
	           ((obj1._id == CABINS) || (obj1._id == CABIN))) {
		r = &_gm->_rooms[AIRLOCK];
		if (!(obj1._id == CABIN) || !(_shown[5])) {
			_vm->renderMessage("Es würde wenig bringen,|sich in eine Schlafkammer zu legen,|die nicht eingeschaltet ist.");
		} else if (r->getObject(5)->hasProperty(WORN)) {
			_vm->renderMessage("Dazu mußt du erst den Raumanzug ausziehen.");
		} else {
			_vm->paletteFadeOut();
			_vm->renderImage(_gm->_currentRoom->getFileNumber(), _gm->invertSection(5));
			_vm->renderImage(_gm->_currentRoom->getFileNumber(), _gm->invertSection(4));
			r = &_gm->_rooms[GENERATOR];
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
					r = &_gm->_rooms[LANDINGMODULE]; // Monitors off
					r->setSectionVisible(2, false);
					r->setSectionVisible(7, false);
					r->setSectionVisible(8, false);
					r->setSectionVisible(9, false);
					r->setSectionVisible(10, false);
				}
			}
			if (_gm->_state.timeSleep == _gm->_state.time) {
				_vm->renderImage(_gm->_currentRoom->getFileNumber(), 3);  // landed
				r = &_gm->_rooms[COCKPIT];
				r->setSectionVisible(23, true);
				r = &_gm->_rooms[CABIN_R2];
				r->setSectionVisible(5, false);
				r->setSectionVisible(6, true);
				r->getObject(2)->_click = 10;
				r = &_gm->_rooms[HOLD];
				r->setSectionVisible(0, false);
				r->setSectionVisible(1, true);
				r->getObject(1)->_click = 255;
				r->getObject(3)->_click = 255;
				r = &_gm->_rooms[GENERATOR];
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
				r = &_gm->_rooms[GENERATOR];
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
				_vm->playSound(45);
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

	if (this->isSectionVisible(kMaxSection - 1)) {
		if (color) {
			color = kColorBlack;
			_gm->_timer2 = 5;
		} else {
			color = kColorLightYellow;
			_gm->_timer2 = 10;
		}
		_vm->renderText("Achtung: Triebwerke funktionsunfähig",50,145,color);
	} else {
		if (this->isSectionVisible(21)) {
			_vm->renderImage(this->getFileNumber(), _gm->invertSection(21));
			_gm->_timer2 = 5;
		} else {
			_vm->renderImage(this->getFileNumber(), 21);
			_gm->_timer2 = 10;
		}
	}
	if (_gm->_state.powerOff) {
		if (this->isSectionVisible(kMaxSection - 1)) {
			_vm->renderText("Energievorrat erschöpft",97,165,color);
			_vm->renderText("Notstromversorgung aktiv",97,175,color);
		} else {
			if (this->isSectionVisible(21)) {
				_vm->renderImage(this->getFileNumber(), 22);
			} else {
				_vm->renderImage(this->getFileNumber(), _gm->invertSection(22));
			}
		}
	}
}
void ShipSleepCabin::onEntrance() {
	if (_gm->_state.dream && (_gm->_rooms[CAVE].getObject(1)->_exitRoom == MEETUP3)) {
		_vm->renderMessage("Du wachst mit brummendem Schädel auf|und merkst, daß du nur geträumt hast.");
		_gm->mouseWait(_gm->_timer1);
		_vm->removeMessage();
		_vm->renderMessage("Beim Aufprall des Raumschiffs|mußt du mit dem Kopf aufgeschlagen|und bewußtlos geworden sein.");
		_gm->mouseWait(_gm->_timer1);
		_vm->removeMessage();
		_vm->renderMessage("Was steht dir jetzt wohl wirklich bevor?");
		_gm->_state.dream = false;
	}
}

bool ShipCockpit::interact(Action verb, Object &obj1, Object &obj2) {
	char c[2] = {0};

	if ((verb == ACTION_LOOK) && (obj1._id == MONITOR)) {
		this->setSectionVisible(kMaxSection - 1, true);
		_vm->renderBox(0, 0, 320, 200, kColorBlack);
		_vm->renderText("Geschwindigkeit: ", 50, 50, kColorLightYellow);
		if (_gm->_state.time)
			_vm->renderText("8000 hpm");
		else
			_vm->renderText("0 hpm");
		_vm->renderText("Ziel: Arsano 3", 50, 70, kColorLightYellow);
		_vm->renderText("Entfernung: ", 50, 90, kColorLightYellow);
		_vm->renderText(Common::String::format("%d", _gm->_state.time / 400).c_str());
		_vm->renderText(",");
		c[0] = (_gm->_state.time / 40) % 10 + '0';
		_vm->renderText(c);
		c[0] = (_gm->_state.time / 4) % 10 + '0';
		_vm->renderText(c);
		_vm->renderText(" Lichtjahre");
		_vm->renderText("Dauer der Reise bei momentaner Geschwindigkeit:", 50, 110, 14);
		_vm->renderText(Common::String::format("%d", _gm->_state.time).c_str(), 50, 120, kColorLightYellow);
		_vm->renderText(" Tage");
		_gm->mouseInput2();
		this->setSectionVisible(kMaxSection - 1, false);
		_vm->renderRoom(*this);
		_gm->showMenu();
		_gm->drawMapExits();
		_vm->paletteBrightness();
	} else if ((verb == ACTION_USE) && (obj1._id == INSTRUMENTS))
		_vm->renderMessage("Vergiá nicht, du bist nur der|Schiffskoch und hast keine Ahnung,|wie man ein Raumschiff fliegt.");
	else
		return false;

	return true;
}
void ShipCockpit::animation() {
	static char color;

	if (this->isSectionVisible(kMaxSection - 1)) {
		if (color) {
			color = 0;
			_gm->_timer2 = 5;
		} else {
			color = 14;
			_gm->_timer2 = 10;
		}
		_vm->renderText("Achtung: Triebwerke funktionsunfhig", 50, 145, color);
	} else {
		if (this->isSectionVisible(21)) {
			_vm->renderImage(this->getFileNumber(), _gm->invertSection(21));
			_gm->_timer2 = 5;
		} else {
			_vm->renderImage(this->getFileNumber(), 21);
			_gm->_timer2 = 10;
		}
	}
	if (_gm->_state.powerOff) {
		if (this->isSectionVisible(kMaxSection - 1)) {
			_vm->renderText("Energievorrat erschpft", 97, 165, color);
			_vm->renderText("Notstromversorgung aktiv", 97, 175, color);
		} else {
			if (this->isSectionVisible(21))
				_vm->renderImage(this->getFileNumber(), 22);
			else
				_vm->renderImage(this->getFileNumber(), _gm->invertSection(22));
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
		_gm->openLocker(this, this->getObject(4), this->getObject(0), 17);
		if (this->getObject(5)->_click == 255)
			_vm->renderImage(this->getFileNumber(), 20); // Remove Pistol
		_gm->great(2);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_KL2, KEYCARD2)) {
		_gm->openLocker(this, this->getObject(6), this->getObject(1), 18);
		_gm->great(2);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_KL3, KEYCARD2)) {
		_gm->openLocker(this, this->getObject(8), this->getObject(2), 19);
		if (this->getObject(9)->_click == 255)
			_vm->renderImage(this->getFileNumber(), 21);  // Remove cable spool
		_gm->great(2);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_KL4, KEYCARD2)) {
		_gm->openLocker(this, this->getObject(10), this->getObject(3), 22);
		if (this->getObject(11)->_click == 255)
			_vm->renderImage(this->getFileNumber(), 23); // Remove book
		_gm->great(2);
	}

	else if ((verb == ACTION_CLOSE) && (obj1._id == SHELF_L1)) {
		_gm->closeLocker(this, this->getObject(4), this->getObject(0), 17);
		this->setSectionVisible(20, false);
	} else if ((verb == ACTION_CLOSE) && (obj1._id == SHELF_L2))
		_gm->closeLocker(this, this->getObject(6), this->getObject(1), 18);
	else if ((verb == ACTION_CLOSE) && (obj1._id == SHELF_L3)) {
		_gm->closeLocker(this, this->getObject(8), this->getObject(2), 19);
		this->setSectionVisible(21, false);
	} else if ((verb == ACTION_CLOSE) && (obj1._id == SHELF_L4)) {
		_gm->closeLocker(this, this->getObject(10), this->getObject(3), 22);
		this->setSectionVisible(23, false);
	}

	else if ((verb == ACTION_TAKE) && (obj1._id == SPOOL) &&
	         !(obj1.hasProperty(CARRIED))) {
		this->getObject(8)->_click = 42;  // empty shelf
		return false;
	} else if ((verb == ACTION_TAKE) && (obj1._id == BOOK2) &&
	           !(obj1.hasProperty(CARRIED))) {
		this->getObject(10)->_click = 47;  // empty shelf
		return false;
	} else
		return false;

	return true;
}

bool ShipCabinL3::interact(Action verb, Object &obj1, Object &obj2) {
	Room *r;

	if ((verb == ACTION_USE) && Object::combine(obj1, obj2, RECORD, TURNTABLE)) {
		if (this->isSectionVisible(kMaxSection - 1) || this->isSectionVisible(15))
			_vm->renderMessage("Du hast die Platte schon aufgelegt.");
		else {
			if (!this->getObject(4)->hasProperty(CARRIED))
				_vm->renderImage(this->getFileNumber(), _gm->invertSection(8));
			else
				_gm->_inventory.remove(*this->getObject(4));
			_vm->renderImage(this->getFileNumber(), 15);
			this->getObject(4)->_click = 48;
		}
	} else if ((verb == ACTION_PRESS) && (obj1._id == TURNTABLE_BUTTON)) {
		if (!this->isSectionVisible(15)) {
			_vm->renderMessage("Es ist doch gar keine Platte aufgelegt.");
		} else if (!this->isSectionVisible(10) &&
		           !this->isSectionVisible(11) &&
		            this->isSectionVisible(12)) {
			if (_gm->_soundDevice != 2) {
				_vm->renderImage(this->getFileNumber(), 14);
				this->setSectionVisible(15, false);
				for (int i = 3; i; i--) {
					_vm->playSound(48, 13530);
					if (_gm->_soundDevice) {
						do {
							if (this->isSectionVisible(13)) {
								_vm->renderImage(this->getFileNumber(), 14);
								this->setSectionVisible(13, false);
							} else {
								_vm->renderImage(this->getFileNumber(), 13);
								this->setSectionVisible(14, false);
							}
							_gm->wait2(3);
						} while (_gm->_status);
					} else {
						_gm->wait2(1);
					}
				}
			} else {
				for (int i = 10; i; i--) {
					_vm->renderImage(this->getFileNumber(), 14);
					_gm->wait2(3);
					_vm->renderImage(this->getFileNumber(), 13);
					_gm->wait2(3);
				}
			}
			_vm->renderImage(this->getFileNumber(), 15);
			this->setSectionVisible(14, false);
			this->setSectionVisible(13, false);
			_vm->renderMessage("Die Platte scheint einen Sprung zu haben.");
		}
	} else if ((verb == ACTION_TAKE) && (obj1._id == RECORD) &&
	           (obj1._click != 15)) {
		_vm->renderImage(this->getFileNumber(), 9);
		this->setSectionVisible(13, false);
		this->setSectionVisible(14, false);
		this->setSectionVisible(15, false);
		obj1._section = 0;
		_gm->takeObject(obj1);
	} else if ((verb == ACTION_PULL) && (obj1._id == PLUG)) {
		_vm->renderImage(this->getFileNumber(), 10);
		this->setSectionVisible(7, false);
		obj1._click = 21;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, PLUG, SOCKET)) {
		_vm->renderImage(this->getFileNumber(), 7);
		this->setSectionVisible(10, false);
		this->getObject(10)->_click = 20;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, KNIFE, WIRE2))
		_vm->renderMessage("Schneid doch besser ein|lngeres Stck Kabel ab!");
	else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, KNIFE, WIRE)) {
		r = &_gm->_rooms[AIRLOCK];
		if (!this->isSectionVisible(10) && !r->getObject(5)->hasProperty(WORN)) {
			_vm->renderImage(this->getFileNumber(), 25);
			_gm->shock();
		}
		_vm->renderImage(this->getFileNumber(), 11);
		_vm->renderImage(this->getFileNumber(), 26);
		this->setSectionVisible(12, false);
	} else if ((verb == ACTION_TAKE) &&
	           ((obj1._id == WIRE) || (obj1._id == WIRE2) || (obj1._id == PLUG))) {
		if (this->isSectionVisible(10) && this->isSectionVisible(11)) {
			_vm->renderImage(this->getFileNumber(), _gm->invertSection(10));
			_vm->renderImage(this->getFileNumber(), _gm->invertSection(11));
			this->getObject(8)->_name = "Leitung mit Stecker";
			_gm->takeObject(*this->getObject(8));
			this->getObject(9)->_click = 255;
			this->getObject(10)->_click = 255;
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
		_gm->openLocker(this, this->getObject(6), this->getObject(2), 9);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_K2, KEYCARD)) {
		_gm->openLocker(this, this->getObject(8), this->getObject(3), 10);
		if (this->getObject(9)->_click == 255)
			_vm->renderImage(this->getFileNumber(), 12); // Remove rope
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_K3, KEYCARD)) {
		_gm->openLocker(this, this->getObject(10), this->getObject(4), 11);
		if (this->getObject(17)->_click == 255)
			_vm->renderImage(this->getFileNumber(), 16); // Remove Discman
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_K4, KEYCARD)) {
		_gm->openLocker(this, this->getObject(15), this->getObject(5), 13);
		if (this->getObject(16)->_click == 255)
			_vm->renderImage(this->getFileNumber(), 14); // Remove book
	}

	else if ((verb == ACTION_CLOSE) && (obj1._id == SHELF1))
		_gm->closeLocker(this, this->getObject(6), this->getObject(2), 9);
	else if ((verb == ACTION_CLOSE) && (obj1._id == SHELF2)) {
		this->setSectionVisible(12, false);
		_gm->closeLocker(this, this->getObject(8), this->getObject(3), 10);
	} else if ((verb == ACTION_CLOSE) && (obj1._id == SHELF3)) {
		this->setSectionVisible(16, false);
		_gm->closeLocker(this, this->getObject(10), this->getObject(4), 11);
	} else if ((verb == ACTION_CLOSE) && (obj1._id == SHELF4)) {
		this->setSectionVisible(14, false);
		this->setSectionVisible(14, false);
		_gm->closeLocker(this, this->getObject(15), this->getObject(5), 13);
	}

	else if ((verb == ACTION_TAKE) && (obj1._id == DISCMAN) &&
	         !_gm->_rooms[0].getObject(3)->hasProperty(CARRIED)) {
		this->getObject(10)->_click = 34; // Locker empty
		obj1._click = 255;
		_gm->takeObject(*_gm->_rooms[0].getObject(3));
		_vm->renderImage(this->getFileNumber(), 16);
	} else if ((verb == ACTION_TAKE) && (obj1._id == ROPE) &&
	           obj1.hasProperty(CARRIED)) {
		this->getObject(8)->_click = 31; // Shelf empty
		return false;
	} else if ((verb == ACTION_TAKE) && (obj1._id == BOOK) &&
	           !obj1.hasProperty(CARRIED)) {
		this->getObject(15)->_click = 32; // Shelf empty
		return false;
	} else
		return false;

	return true;
}


bool ShipAirlock::interact(Action verb, Object &obj1, Object &obj2) {
	Room *r;

	if ((verb == ACTION_PRESS) && (obj1._id == BUTTON1)) {
		if (!this->getObject(1)->hasProperty(OPENED)) {
			_vm->renderImage(this->getFileNumber(), 10);
			_vm->playSound(54, 24020);
			if (this->getObject(0)->hasProperty(OPENED)) {
				this->getObject(0)->disableProperty(OPENED);
				_vm->renderImage(this->getFileNumber(), 1);
				_gm->wait2(2);
				_vm->renderImage(this->getFileNumber(), 2);
				this->setSectionVisible(1, false);
				_gm->wait2(2);
				_vm->renderImage(this->getFileNumber(), 3);
				this->setSectionVisible(2, false);
			} else {
				this->getObject(0)->setProperty(OPENED);
				_vm->renderImage(this->getFileNumber(), 2);
				this->setSectionVisible(3, false);
				_gm->wait2(2);
				_vm->renderImage(this->getFileNumber(), 1);
				this->setSectionVisible(2, false);
				_gm->wait2(2);
				_vm->renderImage(this->getFileNumber(), 1 + 128);
			}
			_vm->renderImage(this->getFileNumber(), _gm->invertSection(10));
		}
	} else if ((verb == ACTION_PRESS) && (obj1._id == BUTTON2)) {
		if (!this->getObject(0)->hasProperty(OPENED)) {
			_vm->renderImage(this->getFileNumber(), 11);
			if (this->getObject(1)->hasProperty(OPENED)) {
				_vm->playSound(54, 24020);
				this->getObject(1)->disableProperty(OPENED);
				_vm->renderImage(this->getFileNumber(), 4);
				_gm->wait2(2);
				_vm->renderImage(this->getFileNumber(), 5);
				this->setSectionVisible(4, false);
				_gm->wait2(2);
				_vm->renderImage(this->getFileNumber(), 6);
				this->setSectionVisible(5, false);
				_vm->renderImage(this->getFileNumber(), 16);
				this->setSectionVisible(17, false);
				_gm->wait2(3);
				_vm->renderImage(this->getFileNumber(), 15);
				this->setSectionVisible(16, false);
				_gm->wait2(3);
				_vm->renderImage(this->getFileNumber(), 14);
				this->setSectionVisible(15, false);
				_gm->wait2(3);
				_vm->renderImage(this->getFileNumber(), 13);
				this->setSectionVisible(14, false);
				_gm->wait2(3);
				_vm->renderImage(this->getFileNumber(), 12);
				this->setSectionVisible(13, false);
				_gm->wait2(3);
				_vm->renderImage(this->getFileNumber(), _gm->invertSection(12));
			} else {
				this->getObject(1)->setProperty(OPENED);
				_vm->renderImage(this->getFileNumber(), 12);
				_gm->wait2(3);
				_vm->renderImage(this->getFileNumber(), 13);
				this->setSectionVisible(12, false);
				_gm->wait2(3);
				_vm->renderImage(this->getFileNumber(), 14);
				this->setSectionVisible(13, false);
				_gm->wait2(3);
				_vm->renderImage(this->getFileNumber(), 15);
				this->setSectionVisible(14, false);
				_gm->wait2(3);
				_vm->renderImage(this->getFileNumber(), 16);
				this->setSectionVisible(15, false);
				_gm->wait2(3);
				_vm->renderImage(this->getFileNumber(), 17);
				this->setSectionVisible(16, false);
				_vm->playSound(54, 24020);
				_vm->renderImage(this->getFileNumber(), 5);
				this->setSectionVisible(6, false);
				_gm->wait2(2);
				_vm->renderImage(this->getFileNumber(), 4);
				this->setSectionVisible(5, false);
				_gm->wait2(2);
				_vm->renderImage(this->getFileNumber(), _gm->invertSection(4));
				r = &_gm->_rooms[AIRLOCK];
				if (!r->getObject(4)->hasProperty(WORN) ||
				    !r->getObject(5)->hasProperty(WORN) ||
				    !r->getObject(6)->hasProperty(WORN)) {
//					longjmp(dead, "Zu niedriger Luftdruck soll ungesund sein.");
				}
			}
			_vm->renderImage(this->getFileNumber(), 11 + 128);
		}
	} else if ((verb == ACTION_LOOK) && (obj1._id == MANOMETER)) {
		if (this->getObject(1)->hasProperty(OPENED))
			_vm->renderMessage("Er zeigt Null an.");
		else
			_vm->renderMessage("Er zeigt Normaldruck an.");
	} else
		return false;

	return true;
}

void ShipAirlock::onEntrance() {
	if (!this->hasSeen())
		_vm->renderMessage("Komisch, es ist nur|noch ein Raumanzug da.");

	this->setRoomSeen(true);
}

bool ShipHold::interact(Action verb, Object &obj1, Object &obj2) {
	static char beschr2[] = "Ein Stck Schrott.";
	Room *r;

	if ((verb == ACTION_LOOK) && (obj1._id == SCRAP_LK) &&
	        (obj1._description != beschr2)) {
		_vm->renderMessage(obj1._description);
		obj1._description = beschr2;
		_gm->takeObject(*this->getObject(2));
	} else if (((verb == ACTION_OPEN) || (verb == ACTION_CLOSE)) &&
	           (obj1._id == OUTERHATCH_TOP)) {
		_vm->renderMessage("Du muát erst hingehen.");
	} else if ((verb == ACTION_CLOSE) && (obj1._id == LANDINGMOD_HATCH) &&
	           (this->isSectionVisible(4) || this->isSectionVisible(6)))
		_vm->renderMessage("Das Kabel ist im Weg.");
	else if (((verb == ACTION_TAKE) && (obj1._id == HOLD_WIRE)) ||
	         ((verb == ACTION_USE) && Object::combine(obj1, obj2, HOLD_WIRE, LANDINGMOD_HATCH)))
		_vm->renderMessage("Das Kabel ist schon ganz|richtig an dieser Stelle.");
	else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, TERMINALSTRIP, HOLD_WIRE)) {
		this->getObject(0)->_name = "Leitung mit Lsterklemme";
		_gm->_inventory.remove(*this->getObject(2));
		_gm->_state.terminalStripConnected = true;
		_gm->_state.terminalStripWire = true;
		_vm->renderMessage("Ok.");
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, HOLD_WIRE, SPOOL)) {
		if (!_gm->_state.terminalStripConnected)
			_vm->renderMessage("Womit denn?");
		else {
			_vm->renderImage(this->getFileNumber(), 5);
			this->getObject(0)->_name = "langes Kabel mit Stecker";
			this->getObject(0)->_click = 10;
			r = &_gm->_rooms[CABIN_L2];
			_gm->_inventory.remove(*this->getObject(9));
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, HOLD_WIRE, GENERATOR_TOP)) {
		if (this->isSectionVisible(5)) {
			r = &_gm->_rooms[GENERATOR];
			r->getObject(0)->_click = 15;
			r->getObject(1)->_click = 13;
			r->setSectionVisible(6, false);
			r->setSectionVisible(8, false);
			_vm->renderImage(this->getFileNumber(), _gm->invertSection(5));
			_vm->renderImage(this->getFileNumber(), 6);
			this->setSectionVisible(4, false);
			this->getObject(0)->_click = 11;
		} else {
			_vm->renderMessage("Die Leitung ist zu kurz.");
		}
	} else {
		return false;
	}

	return true;
}

void ShipHold::onEntrance() {
	if (!this->hasSeen())
		_vm->renderMessage("Was ist denn das fr ein Chaos?|Und auáerdem fehlt das Notraumschiff!|Jetzt wird mir einiges klar.|Die anderen sind geflchtet,|und ich habe es verpennt.");
	this->setRoomSeen(true);
	_gm->_rooms[COCKPIT].setRoomSeen(true);
}

bool ShipLandingModule::interact(Action verb, Object &obj1, Object &obj2) {
	Room *r;
	if ((verb == ACTION_PRESS) && (obj1._id == LANDINGMOD_BUTTON))
		_vm->renderMessage(obj1._description);
	else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, PEN, LANDINGMOD_BUTTON)) {
		if (_gm->_state.landingModuleEnergy) {
			r = &_gm->_rooms[GENERATOR];
			if (this->isSectionVisible(7)) {
				_vm->renderImage(this->getFileNumber(), _gm->invertSection(9));
				_vm->renderImage(this->getFileNumber(), _gm->invertSection(2));
				_vm->renderImage(this->getFileNumber(), _gm->invertSection(8));
				_vm->renderImage(this->getFileNumber(), _gm->invertSection(7));
				_vm->renderImage(this->getFileNumber(), _gm->invertSection(10));
				if (r->isSectionVisible(9))
					_gm->_state.powerOff = true;
				_gm->roomBrightness();
				_vm->paletteBrightness();
			} else {
				_vm->renderImage(this->getFileNumber(), 7);
				if (r->isSectionVisible(9))
					_gm->_state.powerOff = false;
//				load("MSN_DATA.025");
				_gm->roomBrightness();
				_vm->paletteBrightness();
				r = &_gm->_rooms[SLEEP];
				r->setSectionVisible(1, false);
				r->setSectionVisible(2, false);
				_gm->wait2(2);
				_vm->renderImage(this->getFileNumber(), 2);
				_gm->wait2(3);
				_vm->renderImage(this->getFileNumber(), 8);
				_gm->wait2(2);
				_vm->renderImage(this->getFileNumber(), 9);
				_gm->wait2(1);
				_vm->renderImage(this->getFileNumber(), 10);
			}
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, KNIFE, LANDINGMOD_BUTTON))
		_vm->renderMessage("Es ist nicht spitz genug.");
	else if ((verb == ACTION_LOOK) && (obj1._id == LANDINGMOD_MONITOR) &&
	         this->isSectionVisible(7))
		_vm->renderMessage("Du wirst aus den Anzeigen nicht schlau.");
	else if ((verb == ACTION_USE) && (obj1._id == KEYBOARD))
		_vm->renderMessage("Laá lieber die Finger davon!");
	else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, WIRE, LANDINGMOD_SOCKET)) {
		r = &_gm->_rooms[CABIN_L3];
		_gm->_inventory.remove(*r->getObject(8));
		this->getObject(4)->_name = r->getObject(8)->_name;
		_vm->renderImage(this->getFileNumber(), 4);
		if (_gm->_state.cableConnected) {
			_vm->renderImage(this->getFileNumber(), 5);
			this->getObject(4)->_click = 6;
		} else {
			this->getObject(4)->_click = 5;
			if (_gm->_state.terminalStripWire)
				_vm->renderImage(this->getFileNumber(), 11);
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SPOOL, LANDINGMOD_SOCKET))
		_vm->renderMessage("An dem Kabel ist doch gar kein Stecker.");
	else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, LANDINGMOD_WIRE, TERMINALSTRIP)) {
		_vm->renderImage(this->getFileNumber(), 11);
		this->getObject(4)->_name = "Leitung mit Lsterklemme";
		r = &_gm->_rooms[HOLD];
		_gm->_inventory.remove(*r->getObject(2));
		_gm->_state.terminalStripConnected = true;
		_gm->_state.terminalStripWire = true;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, LANDINGMOD_WIRE, SPOOL)) {
		if (!_gm->_state.terminalStripConnected)
			_vm->renderMessage("Womit denn?");
		else {
			_vm->renderImage(this->getFileNumber(), 5);
			this->getObject(4)->_name = "langes Kabel mit Stecker";
			this->getObject(4)->_click = 6;
			r = &_gm->_rooms[CABIN_L2];
			_gm->_inventory.remove(*r->getObject(9));
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, LANDINGMOD_WIRE, LANDINGMOD_HATCH)) {
		if (this->getObject(5)->hasProperty(OPENED)) {
			r = &_gm->_rooms[HOLD];
			if (this->isSectionVisible(5)) {
				_gm->_rooms[HOLD].setSectionVisible(5, false);
				r->getObject(0)->_click = 10;

			} else
				r->getObject(0)->_click = 9;
			_gm->_rooms[HOLD].setSectionVisible(4, false);
			r->getObject(0)->_name = this->getObject(4)->_name;
			_vm->renderImage(this->getFileNumber(), _gm->invertSection(5));
			_vm->renderImage(this->getFileNumber(), _gm->invertSection(4));
			this->setSectionVisible(11, false);
			_vm->renderImage(this->getFileNumber(), 6);
			this->getObject(4)->_click = 7;
		} else {
			_vm->renderMessage("Du solltest die Luke vielleicht erst ffnen.");
		}
	} else if ((verb == ACTION_CLOSE) && (obj1._id == LANDINGMOD_HATCH) &&
	           this->isSectionVisible(6)) {
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
		_vm->playSound(54, 24020);
		_vm->renderImage(this->getFileNumber(), 1);
		if (this->isSectionVisible(7))
			_vm->renderImage(this->getFileNumber(), 10);
		if (this->isSectionVisible(13))
			_vm->renderImage(this->getFileNumber(), 13);
		_gm->_rooms[HOLD].setSectionVisible(3, true);
		obj1.setProperty(OPENED);
		obj1._click = 2;
		_vm->playSound(54, 30030);
	} else if ((verb == ACTION_CLOSE) && (obj1._id == OUTERHATCH)) {
		if (!obj1.hasProperty(OPENED))
			return false;
		if (this->isSectionVisible(11) || this->isSectionVisible(12)) {
			_vm->renderMessage("Das Seil ist im Weg.");
		} else {
			_vm->playSound(54, 24020);
			_vm->renderImage(this->getFileNumber(), _gm->invertSection(1));
			this->setSectionVisible(10, false);
			if (this->isSectionVisible(13))
				_vm->renderImage(this->getFileNumber(), 13);
			_gm->_rooms[HOLD].setSectionVisible(3, false);
			obj1.disableProperty(OPENED);
			obj1._click = 1;
			_vm->playSound(54, 31040);
		}
	} else if ((verb == ACTION_WALK) && (obj1._id == OUTERHATCH) &&
	           this->isSectionVisible(7)) {
		if (!obj1.hasProperty(OPENED))
			_vm->renderMessage("Das ist geschlossen.");
		else if (!this->isSectionVisible(11))
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
		_vm->renderImage(this->getFileNumber(), 3);
		_vm->renderImage(this->getFileNumber(), 4);
		obj1._click = 11;
		_gm->turnOff();
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SHORT_WIRE, CLIP) &&
	           (this->getObject(11)->_click == 11) && !this->isSectionVisible(9)) {
		_vm->renderImage(this->getFileNumber(), 2);
		this->setSectionVisible(3, false);
		this->setSectionVisible(4, false);
		this->getObject(11)->_click = 10;
		if (_gm->_state.shipEnergy)
			_gm->turnOn();
		else
			_vm->renderImage(this->getFileNumber(), 4);
	} else if ((verb == ACTION_OPEN) && (obj1._id == TRAP)) {
		_vm->playSound(54, 24020);
		_vm->renderImage(this->getFileNumber(), 2);
		if (this->getObject(11)->_click == 11)
			_vm->renderImage(this->getFileNumber(), 3);
		if (_gm->_state.powerOff)
			_vm->renderImage(this->getFileNumber(), 4);
		obj1.setProperty(OPENED);
		obj1._click = 6;

		obj1._click2 = 5;
		_vm->playSound(54, 30030);
	} else if ((verb == ACTION_CLOSE) && (obj1._id == TRAP)) {
		if (this->isSectionVisible(9))
			_vm->renderMessage("Das Kabel ist im Weg.");
		else {
			this->setSectionVisible(3, false);
			return false;
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, GENERATOR_WIRE, CLIP) &&
	           this->isSectionVisible(3) && (this->getObject(0)->_click != 16)) {
		_vm->renderImage(this->getFileNumber(), _gm->invertSection(8));
		_vm->renderImage(this->getFileNumber(), 2);
		this->setSectionVisible(4, false);
		_vm->renderImage(this->getFileNumber(), 3);
		_vm->renderImage(this->getFileNumber(), 9);
		this->getObject(0)->_click = 16;
		r = &_gm->_rooms[LANDINGMODULE];
		if (_gm->_state.landingModuleEnergy && r->isSectionVisible(7))
			_gm->turnOn();
		else
			_vm->renderImage(this->getFileNumber(), 4);
		_gm->_rooms[HOLD].setSectionVisible(7, true);
		_gm->great(3);
	} else if ((verb == ACTION_PULL) && (obj1._id == GENERATOR_WIRE) &&
	           (obj1._click == 16)) {
		_vm->renderImage(this->getFileNumber(), _gm->invertSection(9));
		_vm->renderImage(this->getFileNumber(), 2);
		_vm->renderImage(this->getFileNumber(), 3);
		_vm->renderImage(this->getFileNumber(), 4);
		_vm->renderImage(this->getFileNumber(), 8);
		obj1._click = 15;
		_gm->turnOff();
		_gm->_rooms[HOLD].setSectionVisible(7, false);
	} else if ((verb == ACTION_USE) &&
	           (Object::combine(obj1, obj2, WIRE, CLIP) ||
	            Object::combine(obj1, obj2, SPOOL, CLIP)) &&
	           this->isSectionVisible(3)) {
		_vm->renderMessage("Was ntzt dir der Anschluá|ohne eine Stromquelle?!");
	} else if ((verb == ACTION_LOOK) && (obj1._id == VOLTMETER)) {
		if (_gm->_state.powerOff)
			_vm->renderMessage("Die Spannung ist auf Null abgesunken.");
		else
			_vm->renderMessage("Es zeigt volle Spannung an.");
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, LADDER, ROPE)) {
		_vm->renderImage(this->getFileNumber(), 13);
		r = &_gm->_rooms[CABIN_R3];
		_gm->_inventory.remove(*r->getObject(9));
		this->getObject(3)->_click = 18;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, OUTERHATCH, GENERATOR_ROPE)) {
		if (!this->isSectionVisible(1))
			_vm->renderMessage("Du muát die Luke erst ffnen.");
		else {
			_vm->renderImage(this->getFileNumber(), _gm->invertSection(13));
			_vm->renderImage(this->getFileNumber(), 1);
			if (this->isSectionVisible(7)) {
				_vm->renderImage(this->getFileNumber(), 10);
				_vm->renderImage(this->getFileNumber(), 11);
			} else {
				_vm->renderImage(this->getFileNumber(), 12);
			}
			r = &_gm->_rooms[OUTSIDE];
			r->setSectionVisible(1, true);
			r->getObject(1)->_click = 1;
			this->getObject(3)->_click = 17;
		}
	} else if ((verb == ACTION_TAKE) && (obj1._id == GENERATOR_ROPE)) {
		_vm->renderMessage("Das Seil ist hier schon ganz richtig.");
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, OUTERHATCH, GENERATOR_WIRE) &&
	           this->isSectionVisible(1)) {
		_vm->renderMessage("Das Kabel ist zu kurz.");
	} else {
		return false;
	}

	return true;
}

}
