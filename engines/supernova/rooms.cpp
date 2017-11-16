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
#include "graphics/palette.h"
#include "graphics/cursorman.h"

#include "supernova/supernova.h"
#include "supernova/state.h"

namespace Supernova {

bool Room::serialize(Common::WriteStream *out) {
	if (out->err())
		return false;

	out->writeSint32LE(_id);
	for (int i = 0; i < kMaxSection; ++i)
		out->writeByte(_shown[i]);
	for (int i = 0; i < kMaxDialog ; ++i)
		out->writeByte(_sentenceRemoved[i]);

	int numObjects = 0;
	while ((_objectState[numObjects]._id != INVALIDOBJECT) && (numObjects < kMaxObject))
		++numObjects;
	out->writeSint32LE(numObjects);

	for (int i = 0; i < numObjects; ++i) {
		out->writeSint32LE(_objectState[i]._name);
		out->writeSint32LE(_objectState[i]._description);
		out->writeByte(_objectState[i]._roomId);
		out->writeSint32LE(_objectState[i]._id);
		out->writeSint32LE(_objectState[i]._type);
		out->writeByte(_objectState[i]._click);
		out->writeByte(_objectState[i]._click2);
		out->writeByte(_objectState[i]._section);
		out->writeSint32LE(_objectState[i]._exitRoom);
		out->writeByte(_objectState[i]._direction);
	}

	out->writeByte(_seen);

	return !out->err();
}

bool Room::deserialize(Common::ReadStream *in, int version) {
	if (in->err())
		return false;

	in->readSint32LE();

	for (int i = 0; i < kMaxSection; ++i)
		_shown[i] = in->readByte();

	// Prior to version 3, _sentenceRemoved was part of _shown (the last two values)
	// But on the other hand dialog was not implemented anyway, so we don't even try to
	// recover it.
	for (int i = 0; i < kMaxDialog ; ++i)
		_sentenceRemoved[i] = version < 3 ? 0 : in->readByte();

	int numObjects = in->readSint32LE();
	for (int i = 0; i < numObjects; ++i) {
		_objectState[i]._name = static_cast<StringID>(in->readSint32LE());
		_objectState[i]._description = static_cast<StringID>(in->readSint32LE());
		_objectState[i]._roomId = in->readByte();
		_objectState[i]._id = static_cast<ObjectID>(in->readSint32LE());
		_objectState[i]._type = static_cast<ObjectType>(in->readSint32LE());
		_objectState[i]._click = in->readByte();
		_objectState[i]._click2 = in->readByte();
		_objectState[i]._section = in->readByte();
		_objectState[i]._exitRoom = static_cast<RoomID>(in->readSint32LE());
		_objectState[i]._direction = in->readByte();
	}

	_seen = in->readByte();

	return !in->err();
}

Intro::Intro(SupernovaEngine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = -1;
	_id = INTRO;
	_shown[0] = kShownFalse;

	_objectState[0] =
	Object(_id, kStringKeycard, kStringKeycardDescription, KEYCARD,
		   TAKE | CARRIED | COMBINABLE, 255, 255, 0, NULLROOM, 0);
	_objectState[1] =
	Object(_id, kStringKnife, kStringKnifeDescription, KNIFE,
		   TAKE | CARRIED | COMBINABLE, 255, 255, 0, NULLROOM, 0);
	_objectState[2] =
	Object(_id, kStringWatch, kStringDefaultDescription, WATCH,
		   TAKE | COMBINABLE | CARRIED, 255, 255, 8, NULLROOM, 0);
	_objectState[3] =
	Object(_id, kStringDiscman, kStringDiscmanDescription, DISCMAN,
		   TAKE | COMBINABLE, 255, 255, 0, NULLROOM, 0);

	_shouldExit = false;

	introText =
		_vm->getGameString(kStringIntro1) +
		_vm->getGameString(kStringIntro2) +
		_vm->getGameString(kStringIntro3) +
		_vm->getGameString(kStringIntro4) +
		_vm->getGameString(kStringIntro5) +
		_vm->getGameString(kStringIntro6) +
		_vm->getGameString(kStringIntro7) +
		_vm->getGameString(kStringIntro8) +
		_vm->getGameString(kStringIntro9) +
		_vm->getGameString(kStringIntro10) +
		_vm->getGameString(kStringIntro11) +
		_vm->getGameString(kStringIntro12) +
		_vm->getGameString(kStringIntro13);
}

void Intro::onEntrance() {
	_gm->_guiEnabled = false;
	_vm->_allowSaveGame = false;
	_vm->_allowLoadGame = false;
	titleScreen();
	cutscene();
	leaveCutscene();
}

void Intro::titleScreen() {
	// Newspaper
	CursorMan.showMouse(false);
	_vm->_brightness = 0;
	_vm->paletteBrightness();
	_vm->setCurrentImage(1);
	_vm->renderImage(0);
	_vm->paletteFadeIn();
	_gm->getInput();
	_vm->paletteFadeOut();

	// Title Screen
	_vm->setCurrentImage(31);
	_vm->renderImage(0);
	_vm->paletteFadeIn();
	_gm->wait2(1);
	_vm->playSound(kAudioVoiceSupernova);
	while (_vm->_mixer->isSoundHandleActive(_vm->_soundHandle))
		_gm->wait2(1);
	titleFadeIn();
	_vm->renderText(kStringTitleVersion, 295, 190, kColorWhite44);
	const Common::String& title1 = _vm->getGameString(kStringTitle1);
	const Common::String& title2 = _vm->getGameString(kStringTitle2);
	const Common::String& title3 = _vm->getGameString(kStringTitle3);
	_vm->renderText(title1, 78 - _vm->textWidth(title1) / 2, 120, kColorLightBlue);
	_vm->renderText(title2, 78 - _vm->textWidth(title2) / 2, 132, kColorWhite99);
	_vm->renderText(title3, 78 - _vm->textWidth(title3) / 2, 142, kColorWhite99);
	_gm->wait2(1);
	CursorMan.showMouse(true);
	_vm->playSoundMod(kMusicIntro);
	_gm->getInput();
	// TODO: render animated text
	_vm->playSound(kAudioVoiceYeah);
	while (_vm->_mixer->isSoundHandleActive(_vm->_soundHandle));
	_vm->paletteFadeOut();
}

void Intro::titleFadeIn() {
	byte titlePaletteColor[] = {0xfe, 0xeb};
	byte titleNewColor[2][3] = {{255, 255, 255}, {199, 21, 21}};
	byte newColors[2][3];

	for (int brightness = 1; brightness <= 40; ++brightness) {
		for (int colorIndex = 0; colorIndex < 2; ++colorIndex) {
			for (int i = 0; i < 3; ++i) {
				newColors[colorIndex][i] = (titleNewColor[colorIndex][i] * brightness) / 40;
			}
		}

		_vm->_system->getPaletteManager()->setPalette(newColors[0], titlePaletteColor[0], 1);
		_vm->_system->getPaletteManager()->setPalette(newColors[1], titlePaletteColor[1], 1);
		_vm->_system->updateScreen();
		_vm->_system->delayMillis(_vm->_delay);
	}
}

bool Intro::animate(int section1, int section2, int duration) {
	Common::KeyCode key = Common::KEYCODE_INVALID;
	while (duration) {
		_vm->renderImage(section1);
		if (_gm->waitOnInput(2, key))
			return key != Common::KEYCODE_ESCAPE;
		_vm->renderImage(section2);
		if (_gm->waitOnInput(2, key))
			return key != Common::KEYCODE_ESCAPE;
		--duration;
	}
	return true;
}

bool Intro::animate(int section1, int section2, int duration,
                    MessagePosition position, StringID textId) {
	Common::KeyCode key = Common::KEYCODE_INVALID;
	const Common::String& text = _vm->getGameString(textId);
	_vm->renderMessage(text, position);
	int delay = (MIN(text.size(), (uint)512) + 20) * (10 - duration) * _vm->_textSpeed / 400;
	while (delay) {
		if (section1)
			_vm->renderImage(section1);
		if (_gm->waitOnInput(2, key)) {
			_vm->removeMessage();
			return key != Common::KEYCODE_ESCAPE;
		}
		if (section2)
			_vm->renderImage(section2);
		if (_gm->waitOnInput(2, key)) {
			_vm->removeMessage();
			return key != Common::KEYCODE_ESCAPE;
		}
		--delay;
	}
	_vm->removeMessage();
	return true;
}

bool Intro::animate(int section1, int section2, int section3, int section4,
                    int duration, MessagePosition position, StringID textId) {
	Common::KeyCode key = Common::KEYCODE_INVALID;
	const Common::String& text = _vm->getGameString(textId);
	_vm->renderMessage(text, position);
	if (duration == 0)
		duration = (MIN(text.size(), (uint)512) + 20) * _vm->_textSpeed / 40;

	while(duration) {
		_vm->renderImage(section1);
		_vm->renderImage(section3);
		if (_gm->waitOnInput(2, key)) {
			_vm->removeMessage();
			return key != Common::KEYCODE_ESCAPE;
		}
		_vm->renderImage(section2);
		_vm->renderImage(section4);
		if (_gm->waitOnInput(2, key)) {
			_vm->removeMessage();
			return key != Common::KEYCODE_ESCAPE;
		}
		duration--;
	}
	_vm->removeMessage();
	return true;
}

void Intro::cutscene() {
#define exitOnEscape(X) do { \
	Common::KeyCode key = Common::KEYCODE_INVALID; \
	if (_gm->waitOnInput(X, key) && key == Common::KEYCODE_ESCAPE) \
		return; \
} while (0);

	_vm->_system->fillScreen(kColorBlack);
	_vm->setCurrentImage(31);
	_vm->_menuBrightness = 255;
	_vm->paletteBrightness();
	if (!animate(0, 0, 0, kMessageNormal, kStringIntroCutscene1))
		return;
	_vm->_menuBrightness = 0;
	_vm->paletteBrightness();
	exitOnEscape(1);

	_vm->setCurrentImage(9);
	_vm->renderImage(0);
	_vm->renderImage(1);
	_vm->renderImage(9);
	_vm->paletteFadeIn();
	if (!animate(11, 10, 6, kMessageRight, kStringIntroCutscene2))
		return;
	_vm->renderImage(3);
	exitOnEscape(4);
	_vm->renderImage(4);
	if (!animate(11, 10, 3)) {// test duration
		_vm->removeMessage();
		return;
	}
	_vm->removeMessage();
	if (!animate(5, 4, 0, kMessageLeft, kStringIntroCutscene3))
		return;
	_vm->renderImage(3);
	exitOnEscape(3);
	_vm->renderImage(2);
	exitOnEscape(3);
	_vm->renderImage(7);
	exitOnEscape(6);
	_vm->renderImage(6);
	exitOnEscape(6);
	if (!animate(0, 0, 0, kMessageLeft, kStringIntroCutscene4))
		return;
	_vm->renderMessage(kStringIntroCutscene5, kMessageLeft);
	exitOnEscape(28);
	_vm->removeMessage();
	_vm->renderMessage(kStringIntroCutscene6, kMessageLeft);
	exitOnEscape(28);
	_vm->removeMessage();

	StringID textCounting[4] =
	{kStringIntroCutscene7, kStringIntroCutscene8, kStringIntroCutscene9, kStringIntroCutscene10};
	_vm->setCurrentImage(31);
	_vm->renderImage(0);
	_vm->paletteBrightness();
	for (int i = 0; i < 4; ++i){
		_vm->renderMessage(textCounting[i], kMessageLeft);
		for (int j = 0; j < 28; ++j) {
			_vm->renderImage((j % 3) + 1);
			Common::KeyCode key = Common::KEYCODE_INVALID;
			if (_gm->waitOnInput(1, key)) {
				if (key == Common::KEYCODE_ESCAPE)
					return;
				break;
			}
		}
		_vm->removeMessage();
	}
	_vm->renderMessage(kStringIntroCutscene11, kMessageLeft);
	_vm->renderImage(6);
	exitOnEscape(3);
	_vm->renderImage(3);
	exitOnEscape(3);
	_vm->renderImage(4);
	exitOnEscape(3);
	_vm->renderImage(5);
	exitOnEscape(3);
	_vm->renderImage(_gm->invertSection(5));
	exitOnEscape(18);
	_vm->removeMessage();

	_vm->setCurrentImage(9);
	_vm->renderImage(0);
	_vm->renderImage(1);
	_vm->renderImage(9);
	_vm->paletteBrightness();
	_vm->renderBox(0, 138, 320, 62, kColorBlack);
	_vm->paletteBrightness();
	if (!animate(11, 10, 0, kMessageRight, kStringIntroCutscene12))
		return;
	_vm->renderImage(3);
	exitOnEscape(3);
	_vm->renderImage(4);
	if (!animate(5, 4, 0, kMessageLeft, kStringIntroCutscene13))
		return;
	if (!animate(0, 0, 0, kMessageCenter, kStringIntroCutscene14))
		return;
	_vm->renderImage(12);
	exitOnEscape(2);
	_vm->renderImage(13);
	exitOnEscape(2);
	_vm->renderImage(14);
	if (!animate(19, 20, 0, kMessageRight, kStringIntroCutscene15))
		return;
	if (!animate(0, 0, 0, kMessageCenter, kStringIntroCutscene16))
		return;
	exitOnEscape(20);
	if (!animate(0, 0, 0, kMessageCenter, kStringIntroCutscene17))
		return;
	if (!animate(19, 20, 0, kMessageRight, kStringIntroCutscene18))
		return;
	if (!animate(0, 0, 0, kMessageCenter, kStringIntroCutscene19))
		return;
	_vm->renderImage(16);
	exitOnEscape(3);
	_vm->renderImage(17);
	if (!animate(19, 20, 18, 17, 0, kMessageRight, kStringIntroCutscene20))
		return;
	if (!animate(19, 20, 18, 17, 0, kMessageRight, kStringIntroCutscene21))
		return;
	if (!animate(5, 4, 0, kMessageLeft, kStringIntroCutscene3))
		return;
	_vm->renderImage(3);
	exitOnEscape(3);
	_vm->renderImage(2);
	exitOnEscape(3);
	_vm->renderImage(8);
	exitOnEscape(6);
	_vm->renderImage(6);
	_vm->playSound(kAudioSiren);

	exitOnEscape(6);
	_vm->renderImage(3);
	exitOnEscape(3);
	_vm->renderImage(4);
	_vm->renderImage(16);
	exitOnEscape(3);
	_vm->renderImage(15);
	if (!animate(19, 20, 0, kMessageRight, kStringIntroCutscene22))
		return;
	if (!animate(19, 20, 0, kMessageRight, kStringIntroCutscene23))
		return;
	exitOnEscape(10);
	_vm->renderImage(13);
	exitOnEscape(2);
	_vm->renderImage(12);
	exitOnEscape(2);
	_vm->renderImage(9);
	if (!animate(11, 10, 0, kMessageRight, kStringIntroCutscene24))
		return;
	if (!animate(5, 4, 0, kMessageLeft, kStringIntroCutscene3))
		return;
	_vm->paletteFadeOut();

	while (_vm->_mixer->isSoundHandleActive(_vm->_soundHandle))
		exitOnEscape(1);

	_vm->_system->fillScreen(kColorBlack);
	_vm->_menuBrightness = 255;
	_vm->paletteBrightness();
	if (!animate(0, 0, 0, kMessageNormal, kStringIntroCutscene25))
		return;
	_vm->_menuBrightness = 5;
	_vm->paletteBrightness();

	_vm->setCurrentImage(31);
	_vm->renderImage(0);
	_vm->paletteFadeIn();
	if (!animate(0, 0, 0, kMessageNormal, kStringIntroCutscene26))
		return;
	if (!animate(0, 0, 0, kMessageNormal, kStringIntroCutscene27))
		return;
	if (!animate(0, 0, 0, kMessageNormal, kStringIntroCutscene28))
		return;
	if (!animate(0, 0, 0, kMessageNormal, kStringIntroCutscene29))
		return;
	exitOnEscape(54);
	if (!animate(0, 0, 0, kMessageNormal, kStringIntroCutscene30))
		return;
	if (!animate(0, 0, 0, kMessageNormal, kStringIntroCutscene31))
		return;
	if (!animate(0, 0, 0, kMessageNormal, kStringIntroCutscene32))
		return;

	_vm->_brightness = 0;
	_vm->paletteBrightness();
	exitOnEscape(10);
	_vm->playSound(kAudioUndef6);
	while (_vm->_mixer->isSoundHandleActive(_vm->_soundHandle))
		;
	exitOnEscape(10);
	_vm->playSound(kAudioUndef6);
	while (_vm->_mixer->isSoundHandleActive(_vm->_soundHandle))
		;
	exitOnEscape(10);
	_vm->playSound(kAudioUndef6);
	while (_vm->_mixer->isSoundHandleActive(_vm->_soundHandle))
		;
	exitOnEscape(30);
	_vm->setCurrentImage(22);
	if (!animate(0, 0, 0, kMessageNormal, kStringIntroCutscene33))
		return;
	exitOnEscape(18);
	if (!animate(0, 0, 0, kMessageNormal, kStringIntroCutscene34))
		return;
	if (!animate(0, 0, 0, kMessageNormal, kStringIntroCutscene35))
		return;
	if (!animate(0, 0, 0, kMessageNormal, kStringIntroCutscene36))
		return;
	if (!animate(0, 0, 0, kMessageNormal, kStringIntroCutscene37))
		return;
	exitOnEscape(18);
	if (!animate(0, 0, 0, kMessageNormal, kStringIntroCutscene38))
		return;
	if (!animate(0, 0, 0, kMessageNormal, kStringIntroCutscene39))
		return;
	exitOnEscape(18);
	if (!animate(0, 0, 0, kMessageNormal, kStringIntroCutscene40))
		return;
	if (!animate(0, 0, 0, kMessageNormal, kStringIntroCutscene41))
		return;
	exitOnEscape(36);
	animate(0, 0, 0, kMessageNormal, kStringIntroCutscene42);
	_vm->removeMessage();

#undef exitOnEscape
}

void Intro::leaveCutscene() {
	_vm->removeMessage();
	_gm->changeRoom(CABIN_R3);
	_gm->_guiEnabled = true;
	_vm->_allowSaveGame = true;
	_vm->_allowLoadGame = true;
}

bool ShipCorridor::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_PRESS) && (obj1._id == BUTTON)) {
		if (_objectState[6].hasProperty(OPENED)) {
			_vm->playSound(kAudioDoorSound);
			_objectState[6].disableProperty(OPENED);
			_vm->renderImage(8);
			setSectionVisible(9, false);
			_gm->wait2(2);
			_vm->renderImage(7);
			setSectionVisible(8, false);
			_gm->wait2(2);
			_vm->renderImage(_gm->invertSection(7));
		} else {
			_vm->playSound(kAudioDoorSound);
			_objectState[6].setProperty(OPENED);
			_vm->renderImage(7);
			_gm->wait2(2);
			_vm->renderImage(8);
			setSectionVisible(7, false);
			_gm->wait2(2);
			_vm->renderImage(9);
			setSectionVisible(8, false);
		}
		return true;
	}
	return false;
}

bool ShipHall::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_OPEN) && (obj1._id == KITCHEN_HATCH)) {
		_vm->renderMessage(kStringShipHall1);
	} else if ((verb == ACTION_USE) && Object::combine(obj1,obj2,KEYCARD2,SLEEP_SLOT)) {
		if (_objectState[2].hasProperty(OPENED)) {
			_objectState[2].disableProperty(OPENED);
			_vm->renderImage(3);
			setSectionVisible(4, false);
			_gm->wait2(2);
			_vm->renderImage(2);
			setSectionVisible(3, false);
			_gm->wait2(2);
			_vm->renderImage(_gm->invertSection(2));
		} else {
			_objectState[2].setProperty(OPENED);
			_vm->renderImage(2);
			_gm->wait2(2);
			_vm->renderImage(3);
			setSectionVisible(2, false);
			_gm->wait2(2);
			_vm->renderImage(4);
			setSectionVisible(3, false);
			_gm->great(1);
		}
	} else {
		return false;
	}

	return true;
}

bool ShipSleepCabin::interact(Action verb, Object &obj1, Object &obj2) {
	Room *room;
	Common::String input;

	if (((verb == ACTION_LOOK) || (verb == ACTION_USE)) && (obj1._id == COMPUTER)) {
		_gm->_guiEnabled = false;
		setSectionVisible(4, false);
		g_system->fillScreen(kColorDarkBlue);
		if (_gm->_state._arrivalDaysLeft == 0) {
			// Destination reached
			_vm->renderText(kStringShipSleepCabin1, 60, 95, kColorWhite99);
			_gm->getInput();
		} else if (_gm->_state._powerOff) {
			// Energy depleted
			_vm->renderText(kStringShipSleepCabin2, 60, 95, kColorWhite99);
			// Artificial coma interrupted
			_vm->renderText(kStringShipSleepCabin3, 60, 115, kColorWhite99);
			_gm->getInput();
		} else if (isSectionVisible(5)) {
			// Sleep duration in days
			_vm->renderText(kStringShipSleepCabin4, 30, 85, kColorWhite99);
			_vm->renderText(Common::String::format("%d",_gm->_state._timeSleep).c_str(),
			                150, 85, kColorWhite99);
			_vm->renderText(kStringShipSleepCabin5, 30, 105, kColorWhite99);
			_gm->getInput();
		} else {
			_vm->renderText(kStringShipSleepCabin6, 100, 85, kColorWhite99);
			_gm->edit(input, 100, 105, 30);

			input.toUppercase();
			if (_gm->_key.keycode != Common::KEYCODE_ESCAPE) {
				if (input == _vm->getGameString(kStringComputerPassword)) {
					_gm->great(6);
					g_system->fillScreen(kColorDarkBlue);
					_vm->renderText(kStringShipSleepCabin7, 30, 85, kColorWhite99);
					uint daysSleep = 0;
					do {
						input.clear();
						_vm->renderBox(150, 85, 150, 8, kColorDarkBlue);
						_gm->edit(input, 150, 85, 10);

						if (_gm->_key.keycode == Common::KEYCODE_ESCAPE) {
							break;
						} else {
							daysSleep = input.asUint64();
							for (uint i = 0; i < input.size(); i++) {
								if (!Common::isDigit(input[i])) {
									daysSleep = 0;
									break;
								}
							}
						}
						if (daysSleep != 0) {
							_gm->_state._timeSleep = daysSleep;
							_vm->renderText(kStringShipSleepCabin8, 30, 105, kColorWhite99);
							_gm->wait2(18);
							setSectionVisible(5, true);
						}
					} while (daysSleep == 0);
				} else {
					_vm->renderText(kStringShipSleepCabin9, 100, 125, kColorLightRed);
					_gm->wait2(18);
				}
			}
		}

		_gm->_guiEnabled = true;
		input.clear();
	} else if (((verb == ACTION_WALK) || (verb == ACTION_USE)) &&
	           ((obj1._id == CABINS) || (obj1._id == CABIN))) {
		room = _gm->_rooms[AIRLOCK];
		if (!(obj1._id == CABIN) || !isSectionVisible(5)) {
			_vm->renderMessage(kStringShipSleepCabin10);
		} else if (room->getObject(5)->hasProperty(WORN)) {
			_vm->renderMessage(kStringShipSleepCabin11);
		} else {
			_vm->paletteFadeOut();
			_vm->renderImage(_gm->invertSection(5));
			_vm->renderImage(_gm->invertSection(4));
			room = _gm->_rooms[GENERATOR];
			int32 *energyDaysLeft;
			if (room->isSectionVisible(9)) {
				energyDaysLeft = &_gm->_state._landingModuleEnergyDaysLeft;
			} else {
				energyDaysLeft = &_gm->_state._shipEnergyDaysLeft;
			}
			if (_gm->_state._timeSleep > _gm->_state._arrivalDaysLeft) {
				_gm->_state._timeSleep = _gm->_state._arrivalDaysLeft;
			}
			if (_gm->_state._timeSleep >= *energyDaysLeft) {
				_gm->_state._timeSleep = *energyDaysLeft;
				if (room->isSectionVisible(9)) {
					room = _gm->_rooms[LANDINGMODULE]; // Monitors off
					room->setSectionVisible(2, false);
					room->setSectionVisible(7, false);
					room->setSectionVisible(8, false);
					room->setSectionVisible(9, false);
					room->setSectionVisible(10, false);
				}
			}
			if (_gm->_state._timeSleep == _gm->_state._arrivalDaysLeft) {
				_vm->renderImage(3);
				room = _gm->_rooms[COCKPIT];
				room->setSectionVisible(23, true);
				room = _gm->_rooms[CABIN_R2];
				room->setSectionVisible(5, false);
				room->setSectionVisible(6, true);
				room->getObject(2)->_click = 10;
				room = _gm->_rooms[HOLD];
				room->setSectionVisible(0, false);
				room->setSectionVisible(1, true);
				room->getObject(1)->_click = 255;
				room->getObject(3)->_click = 255;
				room = _gm->_rooms[GENERATOR];
				room->setSectionVisible(6, false);
				room->setSectionVisible(7, true);
				room->getObject(1)->_click = 14;
				if (room->isSectionVisible(1)) {
					room->setSectionVisible(10, true);
				}
				if (room->isSectionVisible(12)) {
					room->setSectionVisible(12, false);
					room->setSectionVisible(11, true);
				}
			}
			_gm->_state._arrivalDaysLeft -= _gm->_state._timeSleep;
			*energyDaysLeft -= _gm->_state._timeSleep;
			_gm->_state._time = ticksToMsec(786520); // 12pm
			_gm->_state._alarmOn = (_gm->_state._timeAlarm > _gm->_state._time);
			if (*energyDaysLeft == 0) {
				_gm->turnOff();
				room = _gm->_rooms[GENERATOR];
				room->setSectionVisible(4, room->isSectionVisible(2));
			}
			if (_gm->_state._arrivalDaysLeft == 0) {
				_gm->saveTime();
				if (!_gm->saveGame(-2))
					_gm->errorTemp();
				_gm->_state._dream = true;
				_gm->loadTime();
			}
			_gm->wait2(18);
			_vm->paletteFadeIn();
			if (_gm->_state._arrivalDaysLeft == 0) {
				_vm->playSound(kAudioCrash);
				_gm->screenShake();
				_gm->wait2(18);
				_vm->renderMessage(kStringShipSleepCabin12);
			}
		}
	} else {
		return false;
	}

	return true;
}

void ShipSleepCabin::animation() {
	if (_gm->_state._powerOff && _gm->_state._arrivalDaysLeft) {
		if (_gm->_guiEnabled) {
			if (isSectionVisible(1)) {
				_vm->renderImage(2);
				setSectionVisible(1, false);
			} else {
				_vm->renderImage(1);
				setSectionVisible(2, false);
			}
		} else {
			if (_color == kColorLightRed) {
				_color = kColorDarkBlue;
			} else {
				_color = kColorLightRed;
			}

			_vm->renderText(kStringShipSleepCabin13, 60, 75, _color);
		}
	} else if (isSectionVisible(5) && _gm->_guiEnabled) {
		if (isSectionVisible(4))
			_vm->renderImage(_gm->invertSection(4));
		else
			_vm->renderImage(4);
	}

	_gm->setAnimationTimer(6);
}
void ShipSleepCabin::onEntrance() {
	if (_gm->_state._dream && (_gm->_rooms[CAVE]->getObject(1)->_exitRoom == MEETUP3)) {
		_vm->renderMessage(kStringShipSleepCabin14);
		_gm->waitOnInput(_gm->_timer1);
		_vm->removeMessage();
		_vm->renderMessage(kStringShipSleepCabin15);
		_gm->waitOnInput(_gm->_timer1);
		_vm->removeMessage();
		_vm->renderMessage(kStringShipSleepCabin16);
		_gm->_state._dream = false;
	}
	setRoomSeen(true);
}

bool ShipCockpit::interact(Action verb, Object &obj1, Object &obj2) {
	// TODO: distance and remaining time not accurate

	if ((verb == ACTION_LOOK) && (obj1._id == MONITOR)) {
		char c[2] = {0, 0};
		_gm->_guiEnabled = false;
		_vm->renderBox(0, 0, 320, 200, kColorBlack);
		_vm->renderText(kStringShipCockpit1, 50, 50, kColorLightYellow);
		if (_gm->_state._arrivalDaysLeft)
			_vm->renderText(kStringShipCockpit2);
		else
			_vm->renderText(kStringShipCockpit3);
		_vm->renderText(kStringShipCockpit4, 50, 70, kColorLightYellow);
		_vm->renderText(kStringShipCockpit5, 50, 90, kColorLightYellow);
		_vm->renderText(Common::String::format("%d", _gm->_state._arrivalDaysLeft / 400).c_str());
		_vm->renderText(",");
		c[0] = (_gm->_state._arrivalDaysLeft / 40) % 10 + '0';
		_vm->renderText(c);
		c[0] = (_gm->_state._arrivalDaysLeft / 4) % 10 + '0';
		_vm->renderText(c);
		_vm->renderText(kStringShipCockpit6);
		_vm->renderText(kStringShipCockpit7, 50, 110, kColorLightYellow);
		_vm->renderText(Common::String::format("%d", _gm->_state._arrivalDaysLeft).c_str(),
		                50, 120, kColorLightYellow);
		_vm->renderText(kStringShipCockpit8);

		_gm->getInput();
		_gm->_guiEnabled = true;
	} else if ((verb == ACTION_USE) && (obj1._id == INSTRUMENTS))
		_vm->renderMessage(kStringShipCockpit9);
	else
		return false;

	return true;
}
void ShipCockpit::animation() {
	if (!_gm->_guiEnabled) {
		if (_color) {
			_color = kColorBlack;
			_gm->setAnimationTimer(5);
		} else {
			_color = kColorLightYellow;
			_gm->setAnimationTimer(10);
		}
		_vm->renderText(kStringShipCockpit10, 50, 145, _color);
	} else {
		if (isSectionVisible(21)) {
			_vm->renderImage(_gm->invertSection(21));
			_gm->setAnimationTimer(5);
		} else {
			_vm->renderImage(21);
			_gm->setAnimationTimer(10);
		}
	}
	if (_gm->_state._powerOff) {
		if (!_gm->_guiEnabled) {
			_vm->renderText(kStringShipCockpit11, 97, 165, _color);
			_vm->renderText(kStringShipCockpit12, 97, 175, _color);
		} else {
			if (isSectionVisible(21))
				_vm->renderImage(22);
			else
				_vm->renderImage(_gm->invertSection(22));
		}
	}
}

void ShipCockpit::onEntrance() {
	if (!hasSeen())
		_vm->renderMessage(kStringShipCockpit13);
	setRoomSeen(true);
}

bool ShipCabinL2::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_KL1, KEYCARD2)) {
		_gm->openLocker(this, getObject(4), getObject(0), 17);
		if (getObject(5)->_click == 255)
			_vm->renderImage(20); // Remove Pistol
		_gm->great(2);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_KL2, KEYCARD2)) {
		_gm->openLocker(this, getObject(6), getObject(1), 18);
		_gm->great(2);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_KL3, KEYCARD2)) {
		_gm->openLocker(this, getObject(8), getObject(2), 19);
		if (getObject(9)->_click == 255)
			_vm->renderImage(21); // Remove cable spool
		_gm->great(2);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_KL4, KEYCARD2)) {
		_gm->openLocker(this, getObject(10), getObject(3), 22);
		if (getObject(11)->_click == 255)
			_vm->renderImage(23); // Remove book
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
			_vm->renderMessage(kStringShipCabinL3_1);
		else {
			if (!getObject(4)->hasProperty(CARRIED))
				_vm->renderImage(_gm->invertSection(8));
			else
				_gm->_inventory.remove(*getObject(4));
			_vm->renderImage(15);
			getObject(4)->_click = 48;
		}
	} else if ((verb == ACTION_PRESS) && (obj1._id == TURNTABLE_BUTTON)) {
		if (!isSectionVisible(15)) {
			_vm->renderMessage(kStringShipCabinL3_2);
		} else if (!isSectionVisible(10) && !isSectionVisible(11) && isSectionVisible(12)) {
			_vm->renderImage(14);
			setSectionVisible(15, false);
			for (int i = 3; i; i--) {
				_vm->playSound(kAudioTurntable);
				while (_vm->_mixer->isSoundHandleActive(_vm->_soundHandle)) {
					if (isSectionVisible(13)) {
						_vm->renderImage(14);
						setSectionVisible(13, false);
					} else {
						_vm->renderImage(13);
						setSectionVisible(14, false);
					}
					_gm->wait2(3);
				}
			}
		}
		_vm->renderImage(15);
		setSectionVisible(14, false);
		setSectionVisible(13, false);
		_vm->renderMessage(kStringShipCabinL3_3);
	} else if ((verb == ACTION_TAKE) && (obj1._id == RECORD) && (obj1._click != 15)) {
		_vm->renderImage(9);
		setSectionVisible(13, false);
		setSectionVisible(14, false);
		setSectionVisible(15, false);
		obj1._section = 0;
		_gm->takeObject(obj1);
	} else if ((verb == ACTION_PULL) && (obj1._id == PLUG)) {
		_vm->renderImage(10);
		setSectionVisible(7, false);
		obj1._click = 21;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, PLUG, SOCKET)) {
		_vm->renderImage(7);
		setSectionVisible(10, false);
		getObject(10)->_click = 20;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, KNIFE, WIRE2))
		_vm->renderMessage(kStringShipCabinL3_4);
	else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, KNIFE, WIRE)) {
		r = _gm->_rooms[AIRLOCK];
		if (!isSectionVisible(10) && !r->getObject(5)->hasProperty(WORN)) {
			_vm->renderImage(25);
			_gm->shock();
		}
		_vm->renderImage(11);
		_vm->renderImage(26);
		setSectionVisible(12, false);
	} else if ((verb == ACTION_TAKE) &&
	           ((obj1._id == WIRE) || (obj1._id == WIRE2) || (obj1._id == PLUG))) {
		if (isSectionVisible(10) && isSectionVisible(11)) {
			_vm->renderImage(_gm->invertSection(10));
			_vm->renderImage(_gm->invertSection(11));
			getObject(8)->_name = kStringWireAndPlug;
			_gm->takeObject(*getObject(8));
			getObject(9)->_click = 255;
			getObject(10)->_click = 255;
		} else {
			_vm->renderMessage(kStringShipCabinL3_5);
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
			_vm->renderImage(12); // Remove rope
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_K3, KEYCARD)) {
		_gm->openLocker(this, getObject(10), getObject(4), 11);
		if (getObject(17)->_click == 255)
			_vm->renderImage(16); // Remove Discman
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_K4, KEYCARD)) {
		_gm->openLocker(this, getObject(15), getObject(5), 13);
		if (getObject(16)->_click == 255)
			_vm->renderImage(14); // Remove Book
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
		_vm->renderImage(16);
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

void ShipCabinR3::onEntrance() {
	for (int i = 0; i < 3; ++i)
		_gm->_inventory.add(*_gm->_rooms[INTRO]->getObject(i));

	_vm->paletteBrightness();
	_vm->paletteFadeIn();
	setRoomSeen(true);
}


bool ShipAirlock::interact(Action verb, Object &obj1, Object &obj2) {
	Room *r;

	if ((verb == ACTION_PRESS) && (obj1._id == BUTTON1)) {
		if (!getObject(1)->hasProperty(OPENED)) {
			_vm->renderImage(10);
			_vm->playSound(kAudioDoorSound);
			if (getObject(0)->hasProperty(OPENED)) {
				getObject(0)->disableProperty(OPENED);
				_vm->renderImage(1);
				_gm->wait2(2);
				_vm->renderImage(2);
				setSectionVisible(1, false);
				_gm->wait2(2);
				_vm->renderImage(3);
				setSectionVisible(2, false);
			} else {
				getObject(0)->setProperty(OPENED);
				_vm->renderImage(2);
				setSectionVisible(3, false);
				_gm->wait2(2);
				_vm->renderImage(1);
				setSectionVisible(2, false);
				_gm->wait2(2);
				_vm->renderImage(_gm->invertSection(1));
			}
			_vm->renderImage(_gm->invertSection(10));
		}
	} else if ((verb == ACTION_PRESS) && (obj1._id == BUTTON2)) {
		if (!getObject(0)->hasProperty(OPENED)) {
			_vm->renderImage(11);
			if (getObject(1)->hasProperty(OPENED)) {
				_vm->playSound(kAudioDoorSound);
				getObject(1)->disableProperty(OPENED);
				_vm->renderImage(4);
				_gm->wait2(2);
				_vm->renderImage(5);
				setSectionVisible(4, false);
				_gm->wait2(2);
				_vm->renderImage(6);
				setSectionVisible(5, false);
				_vm->renderImage(16);
				setSectionVisible(17, false);
				_gm->wait2(3);
				_vm->renderImage(15);
				setSectionVisible(16, false);
				_gm->wait2(3);
				_vm->renderImage(14);
				setSectionVisible(15, false);
				_gm->wait2(3);
				_vm->renderImage(13);
				setSectionVisible(14, false);
				_gm->wait2(3);
				_vm->renderImage(12);
				setSectionVisible(13, false);
				_gm->wait2(3);
				_vm->renderImage(_gm->invertSection(12));
			} else {
				getObject(1)->setProperty(OPENED);
				_vm->renderImage(12);
				_gm->wait2(3);
				_vm->renderImage(13);
				setSectionVisible(12, false);
				_gm->wait2(3);
				_vm->renderImage(14);
				setSectionVisible(13, false);
				_gm->wait2(3);
				_vm->renderImage(15);
				setSectionVisible(14, false);
				_gm->wait2(3);
				_vm->renderImage(16);
				setSectionVisible(15, false);
				_gm->wait2(3);
				_vm->renderImage(17);
				setSectionVisible(16, false);
				_vm->playSound(kAudioDoorSound);
				_vm->renderImage(5);
				setSectionVisible(6, false);
				_gm->wait2(2);
				_vm->renderImage(4);
				setSectionVisible(5, false);
				_gm->wait2(2);
				_vm->renderImage(_gm->invertSection(4));
				r = _gm->_rooms[AIRLOCK];
				if (!r->getObject(4)->hasProperty(WORN) ||
				    !r->getObject(5)->hasProperty(WORN) ||
				    !r->getObject(6)->hasProperty(WORN)) {
					_gm->dead(kStringShipAirlock1);
					return true;
				}
			}
			_vm->renderImage(_gm->invertSection(11));
		}
	} else if ((verb == ACTION_LOOK) && (obj1._id == MANOMETER)) {
		if (getObject(1)->hasProperty(OPENED))
			_vm->renderMessage(kStringShipAirlock2);
		else
			_vm->renderMessage(kStringShipAirlock3);
	} else
		return false;

	return true;
}

void ShipAirlock::onEntrance() {
	if (!hasSeen())
		_vm->renderMessage(kStringShipAirlock4);

	setRoomSeen(true);
}

bool ShipHold::interact(Action verb, Object &obj1, Object &obj2) {
	Room *room;

	if ((verb == ACTION_LOOK) && (obj1._id == SCRAP_LK) && (obj1._description != kStringScrapDescription3)) {
		_vm->renderMessage(obj1._description);
		obj1._description = kStringScrapDescription3;
		_gm->takeObject(*getObject(2));
	} else if (((verb == ACTION_OPEN) || (verb == ACTION_CLOSE)) &&
	           (obj1._id == OUTERHATCH_TOP)) {
		_vm->renderMessage(kStringShipHold1);
	} else if ((verb == ACTION_CLOSE) && (obj1._id == LANDINGMOD_HATCH) &&
	           (isSectionVisible(4) || isSectionVisible(6)))
		_vm->renderMessage(kStringCable1);
	else if (((verb == ACTION_TAKE) && (obj1._id == HOLD_WIRE)) ||
	         ((verb == ACTION_USE) && Object::combine(obj1, obj2, HOLD_WIRE, LANDINGMOD_HATCH)))
		_vm->renderMessage(kStringCable2);
	else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, TERMINALSTRIP, HOLD_WIRE)) {
		getObject(0)->_name = kStringWireAndClip;
		_gm->_inventory.remove(*getObject(2));
		_gm->_state._terminalStripConnected = true;
		_gm->_state._terminalStripWire = true;
		_vm->renderMessage(kStringOk);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, HOLD_WIRE, SPOOL)) {
		if (!_gm->_state._terminalStripConnected)
			_vm->renderMessage(kStringCable3);
		else {
			_vm->renderImage(5);
			getObject(0)->_name = kStringWireAndPlug2;
			getObject(0)->_click = 10;
			room = _gm->_rooms[CABIN_L2];
			_gm->_inventory.remove(*getObject(9));
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, HOLD_WIRE, GENERATOR_TOP)) {
		if (isSectionVisible(5)) {
			room = _gm->_rooms[GENERATOR];
			room->getObject(0)->_click = 15;
			room->getObject(1)->_click = 13;
			room->setSectionVisible(6, true);
			room->setSectionVisible(8, true);
			_vm->renderImage(_gm->invertSection(5));
			_vm->renderImage(6);
			setSectionVisible(4, false);
			getObject(0)->_click = 11;
		} else {
			_vm->renderMessage(kStringCable4);
		}
	} else {
		return false;
	}

	return true;
}

void ShipHold::onEntrance() {
	if (!hasSeen())
		_vm->renderMessage(kStringShipHold2);
	setRoomSeen(true);
	_gm->_rooms[COCKPIT]->setRoomSeen(true);
}

bool ShipLandingModule::interact(Action verb, Object &obj1, Object &obj2) {
	Room *r;
	if ((verb == ACTION_PRESS) && (obj1._id == LANDINGMOD_BUTTON))
		_vm->renderMessage(obj1._description);
	else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, PEN, LANDINGMOD_BUTTON)) {
		if (_gm->_state._landingModuleEnergyDaysLeft) {
			r = _gm->_rooms[GENERATOR];
			if (isSectionVisible(7)) {
				_vm->renderImage(_gm->invertSection(9));
				_vm->renderImage(_gm->invertSection(2));
				_vm->renderImage(_gm->invertSection(8));
				_vm->renderImage(_gm->invertSection(7));
				_vm->renderImage(_gm->invertSection(10));
				if (r->isSectionVisible(9))
					_gm->_state._powerOff = true;
				_gm->roomBrightness();
				_vm->paletteBrightness();
			} else {
				_vm->renderImage(7);
				if (r->isSectionVisible(9))
					_gm->_state._powerOff = false;
				_gm->roomBrightness();
				_vm->paletteBrightness();
				r = _gm->_rooms[SLEEP];
				r->setSectionVisible(1, false);
				r->setSectionVisible(2, false);
				_gm->wait2(2);
				_vm->renderImage(2);
				_gm->wait2(3);
				_vm->renderImage(8);
				_gm->wait2(2);
				_vm->renderImage(9);
				_gm->wait2(1);
				_vm->renderImage(10);
			}
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, KNIFE, LANDINGMOD_BUTTON))
		_vm->renderMessage(kStringShipHold3);
	else if ((verb == ACTION_LOOK) && (obj1._id == LANDINGMOD_MONITOR) &&
	         isSectionVisible(7))
		_vm->renderMessage(kStringShipHold4);
	else if ((verb == ACTION_USE) && (obj1._id == KEYBOARD))
		_vm->renderMessage(kStringShipHold5);
	else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, WIRE, LANDINGMOD_SOCKET)) {
		r = _gm->_rooms[CABIN_L3];
		_gm->_inventory.remove(*r->getObject(8));
		getObject(4)->_name = r->getObject(8)->_name;
		_vm->renderImage(4);
		if (_gm->_state._cableConnected) {
			_vm->renderImage(5);
			getObject(4)->_click = 6;
		} else {
			getObject(4)->_click = 5;
			if (_gm->_state._terminalStripWire)
				_vm->renderImage(11);
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SPOOL, LANDINGMOD_SOCKET))
		_vm->renderMessage(kStringShipHold8);
	else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, LANDINGMOD_WIRE, TERMINALSTRIP)) {
		_vm->renderImage(11);
		getObject(4)->_name = kStringWireAndClip;
		r = _gm->_rooms[HOLD];
		_gm->_inventory.remove(*r->getObject(2));
		_gm->_state._terminalStripConnected = true;
		_gm->_state._terminalStripWire = true;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, LANDINGMOD_WIRE, SPOOL)) {
		if (!_gm->_state._terminalStripConnected)
			_vm->renderMessage(kStringCable3);
		else {
			_vm->renderImage(5);
			getObject(4)->_name = kStringWireAndPlug2;
			getObject(4)->_click = 6;
			_gm->_inventory.remove(*_gm->_rooms[CABIN_L2]->getObject(9));
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, LANDINGMOD_WIRE, LANDINGMOD_HATCH)) {
		if (getObject(5)->hasProperty(OPENED)) {
			r = _gm->_rooms[HOLD];
			if (isSectionVisible(5)) {
				r->setSectionVisible(5, true);
				r->getObject(0)->_click = 10;
			} else {
				r->getObject(0)->_click = 9;
			}
			r->setSectionVisible(4, true);
			r->getObject(0)->_name = getObject(4)->_name;
			_vm->renderImage(_gm->invertSection(5));
			_vm->renderImage(_gm->invertSection(4));
			setSectionVisible(11, false);
			_vm->renderImage(6);
			getObject(4)->_click = 7;
		} else {
			_vm->renderMessage(kStringShipHold6);
		}
	} else if ((verb == ACTION_CLOSE) && (obj1._id == LANDINGMOD_HATCH) &&
	           isSectionVisible(6)) {
		_vm->renderMessage(kStringCable1);
	} else if (((verb == ACTION_TAKE) || (verb == ACTION_PULL)) &&
	         (obj1._id == LANDINGMOD_WIRE)) {
		_vm->renderMessage(kStringCable2);
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
		_vm->renderImage(1);
		if (isSectionVisible(7))
			_vm->renderImage(10);
		if (isSectionVisible(13))
			_vm->renderImage(13);
		_gm->_rooms[HOLD]->setSectionVisible(3, true);
		obj1.setProperty(OPENED);
		obj1._click = 2;
		_vm->playSound(kAudioDoorOpen);
	} else if ((verb == ACTION_CLOSE) && (obj1._id == OUTERHATCH)) {
		if (!obj1.hasProperty(OPENED))
			return false;
		if (isSectionVisible(11) || isSectionVisible(12)) {
			_vm->renderMessage(kStringShipHold7);
		} else {
			_vm->playSound(kAudioDoorSound);
			_vm->renderImage(_gm->invertSection(1));
			setSectionVisible(10, false);
			if (isSectionVisible(13))
				_vm->renderImage(13);
			_gm->_rooms[HOLD]->setSectionVisible(3, false);
			obj1.disableProperty(OPENED);
			obj1._click = 1;
			_vm->playSound(kAudioDoorClose);
		}
	} else if ((verb == ACTION_WALK) && (obj1._id == OUTERHATCH) &&
	           isSectionVisible(7)) {
		if (!obj1.hasProperty(OPENED))
			_vm->renderMessage(kStringShipHold9);
		else if (!isSectionVisible(11))
			_vm->renderMessage(kStringShipHold10);
		else {
			obj1._exitRoom = ROCKS;
			return false;
		}
	} else if ((verb == ACTION_TAKE) && (obj1._id == GENERATOR_WIRE))
		_vm->renderMessage(kStringCable2);
	else if ((verb == ACTION_PULL) && (obj1._id == SHORT_WIRE) &&
	         (obj1._click != 11)) {
		_vm->renderImage(3);
		_vm->renderImage(4);
		obj1._click = 11;
		_gm->turnOff();
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SHORT_WIRE, CLIP) &&
	           (getObject(11)->_click == 11) && !isSectionVisible(9)) {
		_vm->renderImage(2);
		setSectionVisible(3, false);
		setSectionVisible(4, false);
		getObject(11)->_click = 10;
		if (_gm->_state._shipEnergyDaysLeft)
			_gm->turnOn();
		else
			_vm->renderImage(4);
	} else if ((verb == ACTION_OPEN) && (obj1._id == TRAP)) {
		_vm->playSound(kAudioDoorSound);
		_vm->renderImage(2);
		if (getObject(11)->_click == 11)
			_vm->renderImage(3);
		if (_gm->_state._powerOff)
			_vm->renderImage(4);
		obj1.setProperty(OPENED);
		obj1._click = 6;

		obj1._click2 = 5;
		_vm->playSound(kAudioDoorOpen);
	} else if ((verb == ACTION_CLOSE) && (obj1._id == TRAP)) {
		if (isSectionVisible(9))
			_vm->renderMessage(kStringCable1);
		else {
			setSectionVisible(3, false);
			return false;
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, GENERATOR_WIRE, CLIP) &&
	           isSectionVisible(3) && (getObject(0)->_click != 16)) {
		_vm->renderImage(_gm->invertSection(8));
		_vm->renderImage(2);
		setSectionVisible(4, false);
		_vm->renderImage(3);
		_vm->renderImage(9);
		getObject(0)->_click = 16;
		r = _gm->_rooms[LANDINGMODULE];
		if (_gm->_state._landingModuleEnergyDaysLeft && r->isSectionVisible(7))
			_gm->turnOn();
		else
		_vm->renderImage(4);
		_gm->_rooms[HOLD]->setSectionVisible(7, true);
		_gm->great(3);
	} else if ((verb == ACTION_PULL) && (obj1._id == GENERATOR_WIRE) &&
	           (obj1._click == 16)) {
		_vm->renderImage(_gm->invertSection(9));
		_vm->renderImage(2);
		_vm->renderImage(3);
		_vm->renderImage(4);
		_vm->renderImage(8);
		obj1._click = 15;
		_gm->turnOff();
		_gm->_rooms[HOLD]->setSectionVisible(7, false);
	} else if ((verb == ACTION_USE) &&
	           (Object::combine(obj1, obj2, WIRE, CLIP) ||
	            Object::combine(obj1, obj2, SPOOL, CLIP)) &&
	           isSectionVisible(3)) {
		_vm->renderMessage(kStringShipHold11);
	} else if ((verb == ACTION_LOOK) && (obj1._id == VOLTMETER)) {
		if (_gm->_state._powerOff)
			_vm->renderMessage(kStringShipHold12);
		else
			_vm->renderMessage(kStringShipHold13);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, LADDER, ROPE)) {
		_vm->renderImage(13);
		r = _gm->_rooms[CABIN_R3];
		_gm->_inventory.remove(*r->getObject(9));
		getObject(3)->_click = 18;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, OUTERHATCH, GENERATOR_ROPE)) {
		if (!isSectionVisible(1))
			_vm->renderMessage(kStringShipHold14);
		else {
			_vm->renderImage(_gm->invertSection(13));
			_vm->renderImage(1);
			if (isSectionVisible(7)) {
				_vm->renderImage(10);
				_vm->renderImage(11);
			} else {
				_vm->renderImage(12);
			}
			r = _gm->_rooms[OUTSIDE];
			r->setSectionVisible(1, true);
			r->getObject(1)->_click = 1;
			getObject(3)->_click = 17;
		}
	} else if ((verb == ACTION_TAKE) && (obj1._id == GENERATOR_ROPE)) {
		_vm->renderMessage(kStringShipHold15);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, OUTERHATCH, GENERATOR_WIRE) &&
	           isSectionVisible(1)) {
		_vm->renderMessage(kStringShipHold16);
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
		_vm->renderImage(1);
		_gm->wait2(2);
		_vm->renderImage(2);
		_gm->wait2(2);
		_vm->renderImage(3);
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
		_vm->renderImage(6);
		setSectionVisible(7, false);
		_gm->wait2(3);
		_vm->renderImage(_gm->invertSection(6));
	}
	if (!(_gm->_state._greatFlag & 0x8000)) {
		_vm->playSound(kAudioGreat);
		_gm->_state._greatFlag |= 0x8000;
	}
}

void ArsanoMeetup::animation() {
	_vm->renderImage(_gm->invertSection(1) + _beacon);
	_beacon = (_beacon + 1) % 5;
	_vm->renderImage(_beacon + 1);
	_vm->renderImage(_beacon + 8);
	if (isSectionVisible(_sign + 13))
		_vm->renderImage(_gm->invertSection(13) + _sign);
	else
		_vm->renderImage(13 + _sign);

	_sign = (_sign + 1) % 14;
	_gm->setAnimationTimer(3);
}

bool ArsanoMeetup::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_WALK) &&
	    ((obj1._id == SPACESHIPS) ||
	     ((obj1._id == SPACESHIP) && !obj1.hasProperty(OPENED)))) {
		_vm->renderMessage(kStringArsanoMeetup1);
	} else if ((verb == ACTION_WALK) && (obj1._id == SPACESHIP)) {
		_gm->changeRoom(GLIDER);
	} else if ((verb == ACTION_WALK) && (obj1._id == STAR)) {
		_vm->renderMessage(kStringArsanoMeetup2);
	} else if ((verb == ACTION_LOOK) && (obj1._id == STAR)) {
		_vm->setCurrentImage(26);
		_vm->renderImage(0);
		_vm->paletteBrightness();
		_gm->animationOff();
		_gm->getInput();
		_gm->animationOn();
		g_system->fillScreen(kColorBlack);
		_vm->renderRoom(*this);
		_vm->paletteBrightness();
	} else if ((verb == ACTION_WALK) && (obj1._id == DOOR)) {
		_vm->renderImage(6);
		_gm->wait2(3);
		_vm->renderImage(7);
		setSectionVisible(6, false);
		_gm->wait2(3);

		return false;
	} else if ((verb == ACTION_LOOK) && (obj1._id == MEETUP_SIGN) && _gm->_state._language) {
		if (_gm->_state._language == 2)
			_vm->renderMessage(kStringArsanoMeetup3);

		obj1._description = kStringSignDescription2;
		if (_gm->_state._language == 1)
			return false;

		_gm->_state._language = 1;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, KEYCARD_R, SPACESHIP)) {
		getObject(5)->setProperty(OPENED);
		_gm->changeRoom(GLIDER);
	} else {
		return false;
	}

	return true;
}

void ArsanoEntrance::animation() {
	if (!_vm->_messageDisplayed && isSectionVisible(kMaxSection - 5)) {
		_gm->animationOff(); // to avoid recursive call
		_vm->playSound(kAudioDoorSound);
		_vm->renderImage(8);
		setSectionVisible(9, false);
		_gm->wait2(2);
		_vm->renderImage(7);
		setSectionVisible(8, false);
		_gm->wait2(2);
		_vm->renderImage(6);
		setSectionVisible(7, false);
		_gm->wait2(2);
		_vm->renderImage(5);
		setSectionVisible(6, false);
		_gm->wait2(2);
		_vm->renderImage(_gm->invertSection(5));
		getObject(11)->_click = 255;
		setSectionVisible(kMaxSection - 5, false);
		_gm->animationOn();
	}
	if (isSectionVisible(2)) {
		_vm->renderImage(_gm->invertSection(2));
	} else {
		if (_eyewitness) {
			--_eyewitness;
		} else {
			_eyewitness = 20;
			_vm->renderImage(2);
		}
	}

	_gm->setAnimationTimer(4);
}

bool ArsanoEntrance::interact(Action verb, Object &obj1, Object &obj2) {
	static byte row1[6] = {1, 1, 1, 1, 1, 0};
	static byte row2[6] = {1, 1, 1, 1, 1, 0};
	static byte row3[6] = {1, 1, 0, 0, 0, 0};

	char e;

	if ((verb == ACTION_TALK) && (obj1._id == PORTER)) {
		if (_gm->_rooms[AIRLOCK]->getObject(4)->hasProperty(WORN)) {
			_vm->renderMessage(kStringArsanoEntrance1);
		} else {
			if (_gm->_state._language) {
				do {
					if (_gm->_state._shoes == 1) {
						_dialog2[2] = kStringArsanoEntrance2;
						addSentence(2, 2);
					} else if (_gm->_state._shoes > 1) {
						removeSentence(2, 2);
					}
					switch (e = _gm->dialog(5, row2, _dialog2, 2)) {
					case 0:
						_gm->reply(kStringArsanoEntrance3, 1, _gm->invertSection(1));
						_gm->reply(kStringArsanoEntrance4, 1, _gm->invertSection(1));
						_gm->reply(kStringArsanoEntrance5, 1, _gm->invertSection(1));
						removeSentence(1, 1);
						break;
					case 1:
						_gm->reply(kStringArsanoEntrance6, 1, _gm->invertSection(1));
						addSentence(1, 2);
						break;
					case 2:
						if (_gm->_state._shoes == 1) {
							_gm->reply(kStringArsanoEntrance7, 1, _gm->invertSection(1));
							_gm->_state._shoes = 2;
						} else {
							_gm->reply(kStringArsanoEntrance8, 1, _gm->invertSection(1));
							_gm->_state._shoes = 1;
						}
						break;
					case 3:
						_gm->reply(kStringArsanoEntrance9, 1, _gm->invertSection(1));
					}
				} while (e != 4);
			} else if (_gm->dialog(5, row2, _dialog2, 0) != 4)
				_gm->reply(kStringArsanoEntrance10, 1, _gm->invertSection(1));
		}
	} else if ((verb == ACTION_WALK) && (obj1._id == STAIRCASE) && (_gm->_state._shoes != 3)) {
		_vm->renderImage(3);
		_gm->wait2(2);
		_vm->renderImage(4);
		setSectionVisible(3, false);
		if (_gm->_rooms[AIRLOCK]->getObject(4)->hasProperty(WORN))
			_gm->reply(kStringDialogSeparator, 1, _gm->invertSection(1));
		else if (_gm->_state._language)
			_gm->reply(kStringArsanoEntrance11, 1, _gm->invertSection(1));
		else
			_gm->reply(kStringArsanoEntrance12, 1, _gm->invertSection(1));
		_vm->renderImage(3);
		setSectionVisible(4, false);
		_gm->wait2(2);
		_vm->renderImage(_gm->invertSection(3));
		if (!_gm->_rooms[AIRLOCK]->getObject(4)->hasProperty(WORN)) {
			if (_gm->_state._language) {
				if (_gm->_state._shoes)
					_gm->reply(kStringArsanoEntrance13, 1, _gm->invertSection(1));
				else
					_gm->reply(kStringArsanoEntrance14, 1, _gm->invertSection(1));
				e = 0;
				while ((e < 3) && (!allSentencesRemoved(4, 1))) {
					switch (e = _gm->dialog(5, row1, _dialog1, 1)) {
					case 0:
						_gm->reply(kStringArsanoEntrance15, 1, 1 + 128);
						break;
					case 1:
						_gm->reply(kStringArsanoEntrance3, 1, 1 + 128);
						_gm->reply(kStringArsanoEntrance4, 1, 1 + 128);
						_gm->reply(kStringArsanoEntrance5, 1, 1 + 128);
						removeSentence(0, 2);
						break;
					case 2:
						_gm->reply(kStringArsanoEntrance7, 1, 1 + 128);
						_gm->_state._shoes = 2;
						break;
					case 3:
						_vm->renderImage(3);
						_gm->wait2(2);
						_vm->renderImage(4);
						setSectionVisible(3, false);
						_gm->reply(kStringArsanoEntrance16, 1, 1 + 128);
						_vm->renderImage(3);
						setSectionVisible(4, false);
						_gm->wait2(2);
						_vm->renderImage(_gm->invertSection(3));
						break;
					}
					removeSentence(0, 1);
				}
			} else {
				_gm->dialog(2, row3, _dialog3, 0);
				_gm->reply(kStringArsanoEntrance10, 1, 1 + 128);
			}
		}
	} else if ((verb == ACTION_PRESS) && (obj1._id == BATHROOM_BUTTON)) {
		_vm->playSound(kAudioDoorSound);
		_vm->renderImage(5);
		_gm->wait2(2);
		_vm->renderImage(6);
		setSectionVisible(5, false);
		_gm->wait2(2);
		_vm->renderImage(7);
		setSectionVisible(6, false);
		_gm->wait2(2);
		_vm->renderImage(8);
		setSectionVisible(7, false);
		_gm->wait2(2);
		_vm->renderImage(9);
		setSectionVisible(8, false);
		getObject(11)->_click = 9;
	} else if ((verb == ACTION_WALK) && (obj1._id == ARSANO_BATHROOM)) {
		if (_gm->_state._coins) {
			if (_gm->_state._shoes == 2) {
				_vm->renderMessage(kStringArsanoEntrance17);
				_gm->_state._shoes = 3;
				removeSentence(2, 2);
				removeSentence(3, 2);
			} else if (_gm->_state._shoes == 3) {
				_vm->renderMessage(kStringArsanoEntrance18);
				_gm->_state._shoes = 2;
			} else
				_vm->renderMessage(kStringArsanoEntrance19);
		} else {
			if (_gm->_rooms[AIRLOCK]->getObject(5)->hasProperty(WORN))
				_vm->renderMessage(kStringArsanoEntrance20);
			else {
				_vm->renderMessage(kStringArsanoEntrance21);
				_gm->waitOnInput(_gm->_timer1);
				_vm->removeMessage();
				_vm->renderMessage(kStringArsanoEntrance22);
				_gm->takeObject(*getObject(16));
				_gm->_state._coins = 5;
			}
		}
		// This shown object is an abuse in the original engine as it's not a real shown variable
		// It's an internal (boolean) status
		_shown[kMaxSection - 5] = kShownTrue;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, COINS, CAR_SLOT)) {
		if ((_gm->_state._coins < 5) && (getObject(7 - _gm->_state._coins)->_click == 7))
			_vm->renderMessage(kStringArsanoEntrance23);
		else {
			_vm->renderImage(15 - _gm->_state._coins);
			getObject(8 - _gm->_state._coins)->_click = 7;
			--_gm->_state._coins;
			if (_gm->_state._coins == 1) {
				getObject(16)->_name = kStringCoin;
			}
			if (_gm->_state._coins == 0) {
				_gm->_inventory.remove(*getObject(16));
				_gm->_state._coins = 255;
			}
		}
	} else if ((verb == ACTION_LOOK) && (obj1._id == KITCHEN_SIGN) && _gm->_state._language) {
		if (_gm->_state._language == 2)
			_vm->renderMessage(kStringArsanoEntrance24);
		obj1._description = kStringDoorDescription5;
		if (_gm->_state._language == 1)
			return false;
		_gm->_state._language = 1;
	} else if ((verb == ACTION_LOOK) && (obj1._id == BATHROOM_SIGN) && _gm->_state._language) {
		if (_gm->_state._language == 2)
			_vm->renderMessage(kStringArsanoEntrance25);
		obj1._description = kStringDoorDescription6;
		if (_gm->_state._language == 1)
			return false;
		_gm->_state._language = 1;
	} else if ((verb == ACTION_WALK) && (obj1._id == MEETUP_EXIT)) {
		if (!((_gm->_rooms[AIRLOCK]->getObject(4)->hasProperty(WORN)) &&
		      (_gm->_rooms[AIRLOCK]->getObject(5)->hasProperty(WORN)) &&
		      (_gm->_rooms[AIRLOCK]->getObject(6)->hasProperty(WORN)))) {
			_vm->renderMessage(kStringArsanoEntrance26);
			getObject(4)->setProperty(WORN);
			getObject(5)->setProperty(WORN);
			getObject(6)->setProperty(WORN);
			_gm->waitOnInput(_gm->_timer1);
			_vm->removeMessage();
		}
		return false;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, KNIFE, PORTER))
		_vm->renderMessage(kStringArsanoEntrance27);
	else
		return false;

	return true;
}

void ArsanoRemaining::animation() {
	switch (_i) {
	case  0:
		_vm->renderImage(1);
		_vm->renderImage(_gm->invertSection(4));
		break;
	case  1:
		_vm->renderImage(_gm->invertSection(1));
		_vm->renderImage(4);
		break;
	case  2:
		_vm->renderImage(2);
		_vm->renderImage(_gm->invertSection(4));
		break;

	case  3:
		_vm->renderImage(7); // Dragon
		_vm->renderImage(_gm->invertSection(2));
		_vm->renderImage(4);
		break;
	case  4:
		_vm->renderImage(8);
		setSectionVisible(7, false);
		_vm->renderImage(2);
		_vm->renderImage(_gm->invertSection(4));
		break;
	case  5:
		_vm->renderImage(_gm->invertSection(8));
		_vm->renderImage(_gm->invertSection(2));
		break;
	case  6:
		_vm->renderImage(3);
		_vm->renderImage(2);
		break;
	case  7:
		_vm->renderImage(_gm->invertSection(3));
		_vm->renderImage(_gm->invertSection(2));
		break;
	case  8:
		_vm->renderImage(3);
		break;
	case  9:
		_vm->renderImage(14); // Card Player 1
		_vm->renderImage(4);
		_vm->renderImage(_gm->invertSection(3));
		break;
	case 10:
		_vm->renderImage(15);
		_vm->renderImage(14);
		_vm->renderImage(_gm->invertSection(4));
		_vm->renderImage(3);
		break;
	case 11:
		_vm->renderImage(16);
		setSectionVisible(15, false);
		_vm->renderImage(4);
		_vm->renderImage(_gm->invertSection(3));
		break;
	case 12:
		_vm->renderImage(17);
		setSectionVisible(16, false);
		_vm->renderImage(_gm->invertSection(4));
		_vm->renderImage(3);
		break;
	case 13:
		_vm->renderImage(_gm->invertSection(17));
		_vm->renderImage(4);
		_vm->renderImage(_gm->invertSection(3));
		break;
	case 14:
		_vm->renderImage(_gm->invertSection(4));
		break;
	case 15:
		_vm->renderImage(6);
		break;
	case 16:
		_vm->renderImage(18); // Card Player 2
		_vm->renderImage(5);
		break;
	case 17:
		_vm->renderImage(19);
		setSectionVisible(18, false);
		_vm->renderImage(_gm->invertSection(5));
		break;
	case 18:
		_vm->renderImage(20);
		setSectionVisible(19, false);
		_vm->renderImage(5);
		break;
	case 19:
		_vm->renderImage(21);
		setSectionVisible(20, false);
		_vm->renderImage(_gm->invertSection(5));
		break;
	case 20:
		_vm->renderImage(_gm->invertSection(21));
		_vm->renderImage(5);
		break;
	case 21:
		_vm->renderImage(_gm->invertSection(5));
		break;
	case 22:
		_vm->renderImage(5);
		break;
	case 23:
		_vm->renderImage(10);
		_chewing = false;
		_vm->renderImage(_gm->invertSection(5));
		break;
	case 24:
		_vm->renderImage(11);
		setSectionVisible(10, false);
		break;
	case 25:
		_vm->renderImage(12);
		setSectionVisible(11, false);
		break;
	case 26:
		_vm->renderImage(13);
		setSectionVisible(12, false);
		break;
	case 27:
		_vm->renderImage(12);
		setSectionVisible(13, false);
		break;
	case 28:
		_vm->renderImage(11);
		setSectionVisible(12, false);
		break;
	case 29:
		_vm->renderImage(10);
		setSectionVisible(11, false);
		break;
	case 30:
		_vm->renderImage(_gm->invertSection(10));
		_chewing = true;
		break;
	case 31:
		_vm->renderImage(22); // Card Player 3
		break;
	case 32:
		_vm->renderImage(_gm->invertSection(22));
		break;
	case 33:
		_vm->renderImage(_gm->invertSection(6));
		break;
	case 34:
		_vm->renderImage(4);
	}

	_i = (_i + 1) % 35;
	if (_chewing) {
		if (isSectionVisible(9))
			_vm->renderImage(_gm->invertSection(9));
		else
			_vm->renderImage(9);
	}
	_gm->setAnimationTimer(3);
}

void ArsanoRoger::onEntrance() {
	if (!sentenceRemoved(0, 2)) {
		_gm->say(kStringArsanoRoger1);
		_gm->reply(kStringArsanoRoger2, 2, 2 + 128);
		removeSentence(0, 2);
	}
}

void ArsanoRoger::animation() {
	if (isSectionVisible(1))
		_vm->renderImage(_gm->invertSection(1));
	else if (isSectionVisible(10)) {
		_vm->renderImage(12);
		setSectionVisible(10, false);
		setSectionVisible(12, false);
	} else {
		if (_eyewitness) {
			--_eyewitness;
		} else {
			_eyewitness = 20;
			if (isSectionVisible(3))
				_vm->renderImage(10);
			else
				_vm->renderImage(1);
		}
	}
	if (isSectionVisible(3)) {
		setSectionVisible(5 + _hands, false);
		_hands = (_hands + 1) % 5;
		_vm->renderImage(5 + _hands);
	}
	_gm->setAnimationTimer(4);
}

bool ArsanoRoger::interact(Action verb, Object &obj1, Object &obj2) {
	static byte row1[6] = {1, 1, 1, 1, 0, 0};

	if ((verb == ACTION_TAKE) && (obj1._id == WALLET)) {
		if (isSectionVisible(3)) {
			_gm->great(0);
			return false;
		}
		_gm->reply(kStringArsanoRoger3, 2, 2 + 128);
	} else if ((verb == ACTION_USE) && (obj1._id == CUP)) {
		_vm->renderMessage(kStringArsanoRoger4);
	} else if ((verb == ACTION_TALK) && (obj1._id == ROGER_W)) {
		if (isSectionVisible(3))
			_vm->renderMessage(kStringArsanoRoger5);
		else {
			switch (_gm->dialog(4, row1, _dialog1, 1)) {
			case 0:
				_gm->reply(kStringArsanoRoger6, 2, 2 + 128);
				_gm->reply(kStringArsanoRoger7, 2, 2 + 128);
				break;
			case 1:
				_gm->reply(kStringArsanoRoger8, 2, 2 + 128);
				_gm->reply(kStringArsanoRoger9, 2, 2 + 128);
				_gm->say(kStringArsanoRoger10);
				break;
			case 2:
				_gm->reply(kStringArsanoRoger11, 2, 2 + 128);
				_gm->say(kStringArsanoRoger12);
				_gm->reply(kStringArsanoRoger13, 2, 2 + 128);
				_gm->say(kStringArsanoRoger14);
				_gm->reply(kStringArsanoRoger15, 2, 2 + 128);
				_gm->reply(kStringArsanoRoger16, 2, 2 + 128);
				_gm->say(kStringArsanoRoger17);
				_gm->say(kStringArsanoRoger18);
				_gm->reply(kStringArsanoRoger19, 2, 2 + 128);
				_gm->say(kStringArsanoRoger20);
				_gm->say(kStringArsanoRoger21);
				_gm->reply(kStringArsanoRoger22, 2, 2 + 128);
				_gm->say(kStringArsanoRoger23);
				_gm->reply(kStringArsanoRoger24, 2, 2 + 128);
				_gm->reply(kStringArsanoRoger25, 2, 2 + 128);
				_gm->say(kStringArsanoRoger26);
				_gm->reply(kStringArsanoRoger27, 2, 2 + 128);
				_gm->reply(kStringArsanoRoger28, 2, 2 + 128);
				_gm->say(kStringArsanoRoger29);
				_gm->reply(kStringArsanoRoger30, 2, 2 + 128);
				_gm->reply(kStringArsanoRoger31, 2, 2 + 128);
				_gm->say(kStringArsanoRoger32);
				_gm->reply(kStringArsanoRoger33, 2, 2 + 128);
				_gm->say(kStringArsanoRoger34);
				_gm->reply(kStringArsanoRoger35, 2, 2 + 128);
			}
		}
	} else if (((verb == ACTION_USE) && Object::combine(obj1, obj2, CHESS, ROGER_W)) ||
	           ((verb == ACTION_GIVE) && (obj1._id == CHESS) && (obj2._id == ROGER_W))) {
		_vm->renderImage(11);
		_gm->great(0);
		_gm->say(kStringArsanoRoger36);
		_gm->reply(kStringArsanoRoger37, 2, 2 + 128);
		_gm->say(kStringArsanoRoger38);
		_vm->paletteFadeOut();
		_gm->_inventory.remove(*_gm->_rooms[CABIN_R3]->getObject(0)); // Chess board
		g_system->fillScreen(kColorBlack);
		_vm->_menuBrightness = 255;
		_vm->paletteBrightness();
		_vm->renderMessage(kStringArsanoRoger39);
		_gm->waitOnInput(_gm->_timer1);
		_vm->removeMessage();
		_vm->_menuBrightness = 0;
		_vm->paletteBrightness();
		_gm->_state._time += ticksToMsec(125000); // 2 hours
		_gm->_state._alarmOn = (_gm->_state._timeAlarm > _gm->_state._time);
		_gm->_state._eventTime = _gm->_state._time + ticksToMsec(4000);
		_gm->_state._eventCallback = kSupernovaFn;
		setSectionVisible(11, false);
		setSectionVisible(1, false);
		_vm->renderRoom(*this);
		_vm->renderImage(3);
		getObject(3)->_click = 5;
		getObject(5)->_click = 6;
		getObject(6)->_click = 7;
		_vm->paletteFadeIn();
		_vm->renderMessage(kStringArsanoRoger40);
		_gm->waitOnInput(_gm->_timer1);
		_vm->removeMessage();
	} else
		return false;

	return true;
}

void ArsanoGlider::animation() {
	if (isSectionVisible(8)) {
		setSectionVisible(24 + _sinus, false);
		_sinus = (_sinus + 1) % 14;
		_vm->renderImage(24 + _sinus);
	} else if (isSectionVisible(24 + _sinus))
		_vm->renderImage(_gm->invertSection(24 + _sinus));

	_gm->setAnimationTimer(2);
}

bool ArsanoGlider::interact(Action verb, Object &obj1, Object &obj2) {
	static char l, r;
	if ((verb == ACTION_USE) && Object::combine(obj1, obj2, KEYCARD_R, GLIDER_SLOT)) {
		_vm->renderImage(5);
		_gm->wait2(7);
		_vm->renderImage(8);
		getObject(5)->_click = 10;
		_gm->_inventory.remove(*_gm->_rooms[ROGER]->getObject(8));
	} else if (((verb == ACTION_TAKE) || (verb == ACTION_PULL)) &&
	           (obj1._id == GLIDER_KEYCARD)) {
		_vm->renderImage(_gm->invertSection(5));
		_vm->renderImage(_gm->invertSection(8));
		getObject(5)->_click = 255;
		_gm->takeObject(*_gm->_rooms[ROGER]->getObject(8));
		for (int i = 9; i <= 22; i++)
			_vm->renderImage(_gm->invertSection(i));
		l = r = 0;
	} else if ((verb == ACTION_PRESS) &&
	           (obj1._id >= GLIDER_BUTTON1) && (obj1._id <= GLIDER_BUTTON4)) {
		int i = obj1._id - GLIDER_BUTTON1 + 1;
		_vm->renderImage(i);
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
					_vm->renderImage(l + 8);
				}
				break;
			case 3:
				if (r < 7) {
					r++;
					_vm->renderImage(r + 15);
				}
				break;
			case 2:
				if (l) {
					_vm->renderImage(_gm->invertSection(l + 8));
					l--;
				}
				break;
			case 4:
				if (r) {
					_vm->renderImage(_gm->invertSection(r + 15));
					r--;
				}
			}
		}
		_gm->wait2(4);
		_vm->renderImage(_gm->invertSection(i));
	} else if ((verb == ACTION_USE) && (obj1._id == GLIDER_BUTTONS)) {
		_vm->renderMessage(kStringArsanoGlider1);
	} else
		return false;

	return true;
}

void ArsanoMeetup2::onEntrance() {
	if (sentenceRemoved(0, 1)) {
		if (sentenceRemoved(1, 1))
			_vm->renderMessage(kStringArsanoMeetup2_2); // All spaceships have left the planet, except one ...
		else
			shipStart();
	} else if (sentenceRemoved(1, 1))
		_vm->renderMessage(kStringArsanoMeetup2_1); // All spaceships have left the planet

	addAllSentences(1);
}

bool ArsanoMeetup2::interact(Action verb, Object &obj1, Object &obj2) {
	static byte row1[6] = {1, 1, 0, 0, 0, 0};
	static byte row2[6] = {1, 1, 0, 0, 0, 0};
	static byte row3[6] = {1, 1, 1, 1, 0, 0};
	static byte row4[6] = {2, 1, 0, 0, 0, 0};

	if (((verb == ACTION_WALK) &&
	        ((obj1._id == SPACESHIP) || (obj1._id == ROGER_W))) ||
	        ((verb == ACTION_TALK) && (obj1._id == ROGER_W))) {
		_gm->changeRoom(INTRO);
		_vm->setCurrentImage(30);
		_vm->renderImage(0);
		_vm->paletteBrightness();
		bool found;
		if (sentenceRemoved(0, 2) || sentenceRemoved(1, 2)) {
			_gm->reply(kStringArsanoMeetup2_3, 1, 1 + 128);
			found = !_gm->dialog(2, row4, _dialog4, 0);
			if (!(found))
				_gm->reply(kStringArsanoMeetup2_4, 1, 1 + 128);
		} else {
			_gm->reply(kStringArsanoMeetup2_5, 1, 1 + 128);
			_gm->reply(kStringArsanoMeetup2_6, 1, 1 + 128);
			found = !_gm->dialog(2, row1, _dialog1, 0);
			removeSentence(0, 2);
		}
		if (found) {
			_gm->_inventory.remove(*_gm->_rooms[ROGER]->getObject(3));
			_gm->_inventory.remove(*_gm->_rooms[ROGER]->getObject(7));
			_gm->_inventory.remove(*_gm->_rooms[ROGER]->getObject(8));
			_gm->reply(kStringArsanoMeetup2_7, 1, 1 + 128);
			_gm->reply(kStringArsanoMeetup2_8, 1, 1 + 128);
			bool flight = _gm->dialog(2, row2, _dialog2, 0);
			if (flight) {
				_gm->reply(kStringArsanoMeetup2_9, 1, 1 + 128);
				_gm->dialog(4, row3, _dialog3, 0);
				_gm->reply(kStringArsanoMeetup2_10, 1, 1 + 128);
			} else {
				_gm->reply(kStringArsanoMeetup2_11, 1, 1 + 128);
			}
			_gm->changeRoom(MEETUP2);
			_gm->_rooms[MEETUP2]->setSectionVisible(12, false);
			_gm->_rooms[MEETUP2]->getObject(0)->_click = 255;
			_gm->_rooms[MEETUP2]->getObject(1)->_click = 255;
			_vm->renderRoom(*this);
			_vm->paletteBrightness();
			shipStart();
			if (flight) {
				_vm->setCurrentImage(13);
				_vm->renderImage(0);
				_vm->paletteBrightness();
				_gm->wait2(36);
				for (int i = 1; i <= 13; i++) {
					if (i > 1)
						_vm->renderImage(_gm->invertSection(i - 1));
					_vm->renderImage(i);
					_gm->wait2(2);
				}
				_vm->renderImage(_gm->invertSection(13));
				_gm->wait2(20);
				_vm->setCurrentImage(14);
				_vm->renderImage(0);
				_vm->paletteBrightness();
				_gm->wait2(36);
				for (int i = 1; i <= 13; i++) {
					if (i > 1)
						_vm->renderImage(_gm->invertSection(i - 1));
					_vm->renderImage(i);
					_gm->wait2(2);
				}
				_vm->renderImage(_gm->invertSection(13));
				_gm->wait2(9);
				_vm->playSound(kAudioCrash);
				for (int i = 14; i <= 19; i++) {
					_vm->renderImage(i);
					_gm->wait2(3);
				}
				_vm->paletteFadeOut();
				_vm->setCurrentImage(11);
				_vm->renderImage(0);
				_vm->paletteFadeIn();
				_gm->wait2(18);
				_vm->renderMessage(kStringArsanoMeetup2_12);
				_gm->great(0);
				_gm->waitOnInput(_gm->_timer1);
				_vm->removeMessage();
				_vm->paletteFadeOut();
				g_system->fillScreen(kColorBlack);
				_gm->_state._dream = false;
				// TODO:
				// The original at this points loads a savegame from "MSN.TMP"
				// which is provided with the data file.
//				if (!load_game(-2)) error_temp();
				_gm->loadTime();
				_gm->_rooms[CAVE]->getObject(1)->_exitRoom = MEETUP3;
				_gm->_state._dream = true;
			}
		} else {
			_gm->changeRoom(MEETUP2);
			_vm->renderRoom(*this);
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, KEYCARD_R, SPACESHIP))
		_vm->renderMessage(kStringArsanoMeetup2_13);
	else
		return false;

	return true;
}

void ArsanoMeetup2::shipStart() {
	_gm->wait2(12);
	for (int i = 2; i <= 11; ++i) {
		if (i >= 9)
			_vm->renderImage(i - 1 + 128);
		else
			setSectionVisible(i - 1, false);
		_vm->renderImage(i);
		_gm->wait2(2);
	}
	_vm->renderImage(11 + 128);
}

bool ArsanoMeetup3::interact(Action verb, Object &obj1, Object &obj2) {
	byte row2[6] = {1, 1, 1, 1, 0, 0};
	byte row3[6] = {1, 1, 0, 0, 0, 0};
	byte rowsX[6] = {1, 1, 1, 0, 0, 0};

	if ((verb == ACTION_WALK) && (obj1._id == STAR))
		_vm->renderMessage(kStringArsanoMeetup2);
	else if ((verb == ACTION_LOOK) && (obj1._id == STAR)) {
		_vm->setCurrentImage(26);
		_vm->renderImage(0);
		_vm->paletteBrightness();
		_gm->getInput();
		g_system->fillScreen(kColorBlack);
		_vm->renderRoom(*this);
		// CHECKME: Doesn't look complete - check sb_meetup()
	} else if ((verb == ACTION_WALK) && (obj1._id == UFO)) {
		g_system->fillScreen(kColorBlack);
		_vm->setCurrentImage(36);
		_vm->renderImage(0);
		_vm->paletteBrightness();
		_gm->dialog(3, rowsX, _dialogsX, 0);
		_vm->renderImage(1);
		_gm->wait2(3);
		_vm->renderImage(2);
		_gm->wait2(3);
		_vm->renderImage(3);
		_gm->wait2(6);
		_vm->renderImage(4);
		_vm->playSound(kAudioGunShot);
		// TODO: wait until audio finished playing
		_vm->renderImage(5);
		_gm->wait2(3);
		_vm->renderImage(4);
		_vm->playSound(kAudioGunShot);
		// TODO: wait until audio finished playing
		_vm->renderImage(5);
		_vm->paletteFadeOut();
		_gm->wait2(12);
		_vm->setCurrentImage(0);
		_vm->renderImage(0);
		_vm->paletteFadeIn();
		_gm->wait2(18);
		_gm->reply(kStringArsanoMeetup3_1, 2, 2 + 128);
		_gm->wait2(10);
		_gm->reply(kStringArsanoMeetup3_2, 1, 1 + 128);

		do {
			int i = _gm->dialog(4, row2, _dialog2, 2);
			switch (i) {
			case 0:
				_gm->reply(kStringArsanoMeetup3_3, 1, 1 + 128);
				_gm->reply(kStringArsanoMeetup3_4, 1, 1 + 128);
				break;
			case 1:
				_gm->reply(kStringArsanoMeetup3_5, 2, 2 + 128);
				_gm->say(kStringArsanoMeetup3_6);
				_gm->reply(kStringArsanoMeetup3_7, 2, 2 + 128);
				_gm->reply(kStringArsanoMeetup3_8, 2, 2 + 128);
				_gm->reply(kStringArsanoMeetup3_9, 2, 2 + 128);
				_gm->reply(kStringArsanoMeetup3_10, 2, 2 + 128);
				_gm->reply(kStringArsanoMeetup3_11, 2, 2 + 128);
				if (_gm->dialog(2, row3, _dialog3, 0)) {
					_gm->reply(kStringArsanoMeetup3_12, 2, 2 + 128);
					_gm->say(kStringArsanoMeetup3_13);
				}
				_gm->reply(kStringArsanoMeetup3_14, 2, 2 + 128);
				_gm->reply(kStringArsanoMeetup3_15, 2, 2 + 128);
				_gm->reply(kStringArsanoMeetup3_16, 2, 2 + 128);
				_gm->reply(kStringArsanoMeetup3_17, 2, 2 + 128);
				if (_gm->dialog(2, row3, _dialog3, 0)) {
					_gm->reply(kStringArsanoMeetup3_12, 2, 2 + 128);
					_gm->say(kStringArsanoMeetup3_13);
				}
				_gm->reply(kStringArsanoMeetup3_18, 2, 2 + 128);
				break;
			case 2:
				_gm->reply(kStringArsanoMeetup3_19, 2, 2 + 128);
				_gm->reply(kStringArsanoMeetup3_20, 2, 2 + 128);
				break;
			case 3:
				_gm->reply(kStringArsanoMeetup3_21, 1, 1 + 128);
				_gm->reply(kStringArsanoMeetup3_22, 1, 1 + 128);
				_gm->say(kStringArsanoMeetup3_23);
				_gm->reply(kStringArsanoMeetup3_24, 1, 1 + 128);
				_gm->reply(kStringArsanoMeetup3_25, 1, 1 + 128);
			}
			removeSentence(2, 2);
		} while (!allSentencesRemoved(4, 2));
		_gm->say(kStringArsanoMeetup3_26);
		_gm->reply(kStringArsanoMeetup3_27, 1, 1 + 128);
		_gm->reply(kStringArsanoMeetup3_28, 1, 1 + 128);
		_vm->paletteFadeOut();
		_gm->_inventory.clear();
		_gm->_inventory.add(*_gm->_rooms[INTRO]->getObject(1));  // Knife
		_gm->_inventory.add(*_gm->_rooms[INTRO]->getObject(2));  // Watch
		_gm->_inventory.add(*_gm->_rooms[INTRO]->getObject(3));  // Discman
		_gm->changeRoom(CELL);
		_gm->_state._dream = true;
	} else
		return false;

	return true;
}

void AxacussCell::onEntrance() {
	if (_gm->_state._dream) {
		_vm->renderMessage(kStringAxacussCell_1);
		_gm->_state._time = ticksToMsec(500000);
		_gm->_state._alarmOn = (_gm->_state._timeAlarm > _gm->_state._time);
		_gm->_state._powerOff = false;
		_gm->_state._dream = false;
	}
}

void AxacussCell::animation() {
	++_gm->_state._timeRobot;

	if (_gm->_state._timeRobot == 299) {
		_vm->renderImage(_gm->invertSection(31));
		_vm->renderImage(28);
		getObject(0)->_click = 255;
		getObject(1)->setProperty(EXIT | OPENABLE | OPENED | CLOSED);
	} else if ((_gm->_state._timeRobot >= 301) && (_gm->_state._timeRobot <= 320)) {
		_vm->renderImage(_gm->invertSection(329 - _gm->_state._timeRobot));
		_vm->renderImage(328 - _gm->_state._timeRobot);
	} else if (_gm->_state._timeRobot == 321) {
		_vm->renderImage(31);
		setSectionVisible(8, false);
		getObject(0)->_click = 1;
		getObject(1)->setProperty(EXIT | OPENABLE | CLOSED);
	}

	if (_gm->_state._timeRobot == 599) {
		_vm->renderImage(_gm->invertSection(31));
		_vm->renderImage(8);
		getObject(0)->_click = 255;
		getObject(1)->setProperty(EXIT | OPENABLE | OPENED | CLOSED);
	} else if ((_gm->_state._timeRobot >= 601) && (_gm->_state._timeRobot <= 620)) {
		_vm->renderImage(_gm->_state._timeRobot - 593 + 128);
		_vm->renderImage(_gm->_state._timeRobot - 592);
	} else if (_gm->_state._timeRobot == 621) {
		_vm->renderImage(31);
		setSectionVisible(28, false);
		getObject(0)->_click = 1;
		getObject(1)->setProperty(EXIT | OPENABLE | CLOSED);
	} else if (_gm->_state._timeRobot == 700) {
		_gm->_state._timeRobot = 0;
	} else if (_gm->_state._timeRobot == 10002) {
		_vm->renderImage(18 + 128);
		_vm->renderImage(29);
		_vm->renderImage(7);
		getObject(2)->_click = 13;
	} else if (_gm->_state._timeRobot == 10003) {
		setSectionVisible(29, false);
		_vm->renderImage(30);
		getObject(8)->_click = 12;
		getObject(7)->_click = 14;
		_vm->playSound(kAudioUndef4);
	} else if (_gm->_state._timeRobot == 10010) {
		--_gm->_state._timeRobot;
	}

	if (_gm->_state._timeRobot == 312) {
		_vm->renderImage(7);
		getObject(2)->_click = 13;
	} else if (_gm->_state._timeRobot == 610) {
		setSectionVisible(7, false);
		getObject(2)->_click = 255;
	}

	if ((isSectionVisible(6)) &&
	    ((_gm->_state._timeRobot == 310) || (_gm->_state._timeRobot == 610))) {
		_vm->playSound(kAudioUndef3);
		_gm->_state._timeRobot = 10000;
	}

	_gm->setAnimationTimer(3);
}

bool AxacussCell::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_PRESS) && (obj1._id == CELL_BUTTON))
		_vm->renderMessage(kStringAxacussCell_2);
	else if ((verb == ACTION_PULL) && (obj1._id == CELL_WIRE) &&
	         !isSectionVisible(2) &&
	         !isSectionVisible(3) &&
	         !isSectionVisible(5)) {
		if (isSectionVisible(1)) {
			_vm->renderImage(_gm->invertSection(1));
			_vm->renderImage(2);
			getObject(5)->_click = 7;
		} else if (isSectionVisible(4)) {
			_vm->renderImage(_gm->invertSection(4));
			_vm->renderImage(3);
			getObject(5)->_click = 8;
		} else if (isSectionVisible(6)) {
			_vm->renderImage(_gm->invertSection(6));
			_vm->renderImage(5);
			getObject(5)->_click = 10;
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, CELL_WIRE, SOCKET) &&
	           !isSectionVisible(1) &&
	           !isSectionVisible(4) &&
	           !isSectionVisible(6)) {
		if (isSectionVisible(2)) {
			_vm->renderImage(_gm->invertSection(2));
			_vm->renderImage(1);
			getObject(5)->_click = 6;
		} else if (isSectionVisible(3)) {
			_vm->renderImage(_gm->invertSection(3));
			_vm->renderImage(4);
			getObject(5)->_click = 9;
		} else if (isSectionVisible(5)) {
			_vm->renderImage(_gm->invertSection(5));
			_vm->renderImage(6);
			getObject(5)->_click = 11;
		} else {
			_gm->_inventory.remove(*getObject(5));
			_vm->renderImage(4);
			getObject(5)->_click = 9;
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, CELL_WIRE, KNIFE) &&
	           ((isSectionVisible(1)) || (isSectionVisible(2)))) {
		if (isSectionVisible(1))
			_gm->shock();
		else {
			_vm->renderImage(_gm->invertSection(2));
			_vm->renderImage(3);
			getObject(5)->_click = 8;
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, CELL_WIRE, CELL_TABLE) &&
	           !isSectionVisible(1) &&
	           !isSectionVisible(2) &&
	           !isSectionVisible(5) &&
	           !isSectionVisible(6)) {
		if (isSectionVisible(3)) {
			_vm->renderImage(_gm->invertSection(3));
			_vm->renderImage(5);
			getObject(5)->_click = 10;
		} else if (isSectionVisible(4)) {
			_vm->renderImage(_gm->invertSection(4));
			_vm->renderImage(6);
			_gm->shock();
		} else {
			_gm->_inventory.remove(*getObject(5));
			_vm->renderImage(5);
			getObject(5)->_click = 10;
		}
	} else if ((verb == ACTION_TAKE) && (obj1._id == CELL_WIRE) &&
	           !(obj1.hasProperty(CARRIED))) {
		if (isSectionVisible(3)) {
			_vm->renderImage(_gm->invertSection(3));
			_gm->takeObject(obj1);
		} else if (isSectionVisible(5)) {
			_vm->renderImage(_gm->invertSection(5));
			_gm->takeObject(obj1);
		} else {
			_vm->renderMessage(kStringAxacussCell_3);
		}
	} else if ((verb == ACTION_WALK) && (obj1._id == CELL_DOOR) &&
	           (obj1.hasProperty(OPENED))) {
		if (isSectionVisible(30) || isSectionVisible(29))
			return false;
		_vm->playSound(kAudioGunShot);
		// TODO: wait till sound is played
		_vm->playSound(kAudioGunShot);
		_vm->playSound(kAudioGunShot);
		_gm->dead(kStringAxacussCell_4);
	} else if ((verb == ACTION_USE) && (obj1._id == TRAY))
		_vm->renderMessage(kStringAxacussCell_5);
	else if ((verb == ACTION_TAKE) && (obj1._id == MAGNET)) {
		if (isSectionVisible(6))
			_gm->shock();
		_gm->takeObject(obj1);
		_vm->renderMessage(kStringOk);
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
			_vm->renderImage(29);
	} else if (((verb == ACTION_USE) && Object::combine(obj1, obj2, TABLE, WATCH)) ||
	           ((verb == ACTION_GIVE) && (obj1._id == WATCH) &&
	            (obj2._id == TABLE))) {
		if (obj1._id == WATCH)
			_gm->_inventory.remove(obj1);
		else
			_gm->_inventory.remove(obj2);

		_vm->renderImage(29);
		getObject(4)->_click = 8;
	} else if ((verb == ACTION_TAKE) && (obj1._id == WATCH) &&
	           !obj1.hasProperty(CARRIED)) {
		setSectionVisible(29, false);
		getObject(4)->_click = 255;
		_gm->takeObject(*_gm->_rooms[INTRO]->getObject(2));
		if (isSectionVisible(9))
			_vm->renderImage(9);
	} else
		return false;

	return true;
}

void AxacussCorridor5::onEntrance() {
}

bool AxacussCorridor5::handleMoneyDialog() {
	if (_gm->dialog(2, _rows, _dialog2, 0) == 0) {
		_gm->reply(kStringAxacussCorridor5_5, 1, 1 + 128);
		addAllSentences(2);
		if (_gm->_state._money == 0) {
			removeSentence(2, 2);
			removeSentence(3, 2);
		} else {
			// TODO: Handle string manipulation in dialogs
			// _dialog3[2] and _dialog3[3] are both using kStringDialogAxacussCorridor5_7
			//    ("Wenn Sie mich durchlassen gebe ich Ihnen %d Xa.")
			// One way could be to keep an array of string[6], replace the %d of the previous sentence by a %s,
			// and format the dialog string when the associated parameter string isn't empty. 
			// The following code is broken and only kept in order to remember the values used.
			// _dialog3[2] += Common::String::format(kStringDialogAxacussCorridor5_7, _gm->_state._money - 200);
			// _dialog3[3] += Common::String::format(kStringDialogAxacussCorridor5_7, _gm->_state._money);
		}
		switch (_gm->dialog(4, _rows, _dialog3, 2)) {
		case 1:
			_gm->wait2(3);
			_vm->renderImage(1);
			_vm->playSound(kAudioVoiceHalt);
			_vm->renderImage(_gm->invertSection(1));
			_gm->wait2(5);
			_vm->renderImage(2);
			_gm->wait2(2);
			_gm->shot(3, _gm->invertSection(3));
			break;
		case 3:
			if (_gm->_state._money >= 900) {
				stopInteract(_gm->_state._money);
				return true;
			}
		case 2:
			if (_gm->_state._money > 1100) {
				stopInteract(_gm->_state._money - 200);
				return true;
			}
			_gm->reply(kStringAxacussCorridor5_6, 1, 1 + 128);
		}
	}
	return false;
}

void AxacussCorridor5::stopInteract(int sum) {
	_gm->reply(kStringAxacussCorridor5_7, 1, 1 + 128);
	_gm->great(0);
	_gm->changeRoom(ELEVATOR);
	_gm->takeMoney(-sum);
}

bool AxacussCorridor5::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_WALK) && (obj1._id == DOOR)) {
		g_system->fillScreen(kColorBlack);
		_vm->setCurrentImage(41);
		_vm->renderImage(0);
		_vm->paletteBrightness();
		if (_gm->_guiEnabled) {
			_gm->reply(kStringAxacussCorridor5_1, 1, 1 + 128);
			if (handleMoneyDialog())
				return true;
		} else {
			_gm->_guiEnabled = true;
			_gm->reply(kStringAxacussCorridor5_2, 1, 1 + 128);
			if (_gm->dialog(2, _rows, _dialog1, 0))
				_gm->reply(kStringAxacussCorridor5_3, 1, 1 + 128);
			else {
				_gm->reply(kStringAxacussCorridor5_4, 1, 1 + 128);
				if (handleMoneyDialog())
					return true;
			}
		}
		g_system->fillScreen(kColorBlack);
		return true;
	}
	return false;
}

void AxacussCorridor6::onEntrance() {

}

bool AxacussCorridor6::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) &&
	        (obj1.hasProperty(OPENED))) {
		_vm->renderImage(6);
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
		_vm->renderImage(27);
		setSectionVisible(28, false);
		obj1.setProperty(OPENED);
		_gm->_rooms[CORRIDOR6]->setSectionVisible(6, false);
		_gm->_rooms[CORRIDOR6]->setSectionVisible(7, true);
		_gm->_rooms[CORRIDOR6]->getObject(2)->setProperty(EXIT | OPENED | OPENABLE);
		_gm->_rooms[CORRIDOR6]->getObject(2)->_click = 4;
		_vm->playSound(kAudioDoorOpen);
	} else if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) &&
	           (obj1._type & OPENED)) {
		_vm->renderImage(28);
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
		_vm->renderImage(28);
		setSectionVisible(27, false);
		obj1.disableProperty(OPENED);
		_gm->_rooms[GUARD]->setSectionVisible(6, false);
		_gm->_rooms[GUARD]->getObject(2)->disableProperty(OPENED);
		_vm->playSound(kAudioDoorClose);
	} else if ((verb == ACTION_OPEN) && (obj1._id == DOOR) &&
	           !obj1.hasProperty(OPENED)) {
		_vm->renderImage(27);
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
		_vm->renderImage(_gm->invertSection(obj1._id - DOOR1 + 1));
		_vm->playSound(kAudioDoorClose);
		if (obj1.hasProperty(OCCUPIED)) {
			_gm->_state._destination = 255;
			obj1.setProperty(EXIT | OPENABLE | CLOSED | CAUGHT);
			if (!_gm->_rooms[OFFICE_L1 + obj1._id - DOOR1]->isSectionVisible(4))
				_gm->search(180);
			else
				_gm->_state._eventTime = 0xffffffff;
		} else
			obj1.setProperty(EXIT | OPENABLE | CLOSED);
	} else if (((verb == ACTION_WALK) || ((verb == ACTION_OPEN) && !obj1.hasProperty(OPENED))) &&
	           (obj1._id >= DOOR1) && (obj1._id <= DOOR4) &&
	           obj1.hasProperty(OCCUPIED)) {
		_vm->renderMessage(kStringDontEnter);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR1) &&
	           !getObject(4)->hasProperty(OPENED)) {
		if (getObject(4)->hasProperty(OCCUPIED))
			_vm->renderMessage(kStringDontEnter);
		else {
			_vm->renderImage(1);
			_vm->playSound(kAudioDoorOpen);
			if (getObject(4)->hasProperty(CAUGHT))
				_gm->busted(11);
			getObject(4)->setProperty(EXIT | OPENABLE | OPENED);
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR2) &&
	           !getObject(5)->hasProperty(OPENED)) {
		if (getObject(5)->hasProperty(OCCUPIED)) {
			_vm->renderMessage(kStringDontEnter);
		} else {
			_vm->renderImage(2);
			_vm->playSound(kAudioDoorOpen);
			if (getObject(5)->hasProperty(CAUGHT))
				_gm->busted(16);
			getObject(5)->setProperty(EXIT | OPENABLE | OPENED);
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR3) &&
	           !getObject(6)->hasProperty(OPENED)) {
		if (getObject(6)->hasProperty(OCCUPIED)) {
			_vm->renderMessage(kStringDontEnter);
		} else {
			_vm->renderImage(3);
			_vm->playSound(kAudioDoorOpen);
			if (getObject(6)->hasProperty(CAUGHT))
				_gm->busted(15);
			getObject(6)->setProperty(EXIT | OPENABLE | OPENED);
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR4) &&
	           !getObject(7)->hasProperty(OPENED)) {
		if (getObject(7)->hasProperty(OCCUPIED)) {
			_vm->renderMessage(kStringDontEnter);
		} else {
			_vm->renderImage(4);
			_vm->playSound(kAudioDoorOpen);
			if (getObject(7)->hasProperty(CAUGHT))
				_gm->busted(20);
			getObject(7)->setProperty(EXIT | OPENABLE | OPENED);
		}
	} else if ((verb == ACTION_LOOK) &&
	           (obj1._id >= DOOR1) && (obj1._id <= DOOR4)) {
		_nameSeen |= 1 << (obj1._id - DOOR1);
		return false;
	} else if ((verb == ACTION_WALK) &&
	           ((obj1._id == PILLAR1) || (obj1._id == PILLAR2))) {
		_vm->renderMessage(kStringAxacussBcorridor_1);
		_gm->_guiEnabled = true;
	} else
		return false;

	return true;
}

bool AxacussIntersection::interact(Action verb, Object &obj1, Object &obj2) {
	byte rowsX[6] = {1, 1, 1, 0, 0, 0};

	if (((verb == ACTION_WALK) || (verb == ACTION_OPEN)) && (obj1._id == DOOR) && !isSectionVisible(1))
		_gm->guardShot();
	else if ((verb == ACTION_OPEN) && (obj1._id == DOOR) && !obj1.hasProperty(OPENED)) {
		_gm->_rooms[CORRIDOR9]->setSectionVisible(27, true);
		_gm->_rooms[CORRIDOR9]->setSectionVisible(28, false);
		_gm->_rooms[CORRIDOR9]->getObject(1)->setProperty(OPENED);
		return false;
	} else if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) && obj1.hasProperty(OPENED)) {
		_gm->_rooms[CORRIDOR9]->setSectionVisible(27, false);
		_gm->_rooms[CORRIDOR9]->setSectionVisible(28, true);
		_gm->_rooms[CORRIDOR9]->getObject(1)->disableProperty(OPENED);
		return false;
	} else if ((verb == ACTION_TALK) && (obj1._id == GUARDIAN)) {
		_gm->dialog(3, rowsX, _dialogsX, 0);
		_gm->guardShot();
	} else if ((verb == ACTION_TAKE) && (obj1._id == MASTERKEYCARD)) {
		_gm->great(0);
		setSectionVisible(7, false);
		return false;
	} else if ((verb == ACTION_USE) && (Object::combine(obj1, obj2, MAGNET, GUARDIAN) || Object::combine(obj1, obj2, KNIFE, GUARDIAN)))
		_vm->renderMessage(kStringArsanoEntrance27);
	else
		return false;

	return true;
}

bool AxacussExit::interact(Action verb, Object &obj1, Object &obj2) {
	byte rowsX[6] = {1, 1, 1, 0, 0, 0};

	if (((verb == ACTION_WALK) || (verb == ACTION_OPEN)) && (obj1._id == DOOR) && !_gm->_state._powerOff)
		_gm->guard3Shot();
	else if ((verb == ACTION_TALK) && (obj1._id == GUARDIAN)) {
		_gm->dialog(3, rowsX, _dialogsX,0);
		_gm->guard3Shot();
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, LAMP, MAGNET)) {
		_gm->_inventory.remove(*_gm->_rooms[CELL]->getObject(7));
		for (int i = 4; i <= 11; i++) {
			_vm->renderImage(i);
			if (i == 11)
				_vm->playSound(kAudioUndef2); // 046/4020
			_gm->wait2(1);
			_vm->renderImage(i + 128);
		}
		_gm->_state._powerOff = true;
		_objectState[5]._click = 255;

		_gm->search(450);
		_gm->roomBrightness();
		_vm->paletteBrightness();
	} else if ((verb == ACTION_USE) && (Object::combine(obj1,obj2,MAGNET,GUARDIAN) || Object::combine(obj1,obj2,KNIFE,GUARDIAN)))
		_vm->renderMessage(kStringArsanoEntrance27);
	else
		return false;

	return true;
}

bool AxacussOffice1::interact(Action verb, Object &obj1, Object &obj2) {
	Common::String input;
	if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) &&
	        obj1.hasProperty(OPENED)) {
		_vm->renderImage(_gm->invertSection(9));
		obj1.setProperty(EXIT | OPENABLE | CLOSED);
		_vm->playSound(kAudioDoorClose);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR) &&
	           !getObject(0)->hasProperty(OPENED)) {
		_vm->renderImage(9);
		getObject(0)->setProperty(EXIT | OPENABLE | OPENED);
		_vm->playSound(kAudioDoorOpen);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, COMPUTER, MAGNET)) {
		_vm->renderImage(4);
		setSectionVisible(16, false);
		_vm->playSound(kAudioUndef2);
	} else if ((verb == ACTION_LOOK) && (obj1._id == COMPUTER)) {
		if (isSectionVisible(4))
			_vm->renderMessage(kStringBroken);
		else
			_gm->telomat(0);
	} else if (((verb == ACTION_OPEN) || (verb == ACTION_USE)) &&
	           (obj1._id == LOCKER) &&
	           !obj1.hasProperty(OPENED)) {
		_vm->renderMessage(kStringAxacussOffice1_1);
		_vm->renderBox(160, 70, 70, 10, kColorDarkBlue);
		_gm->edit(input, 161, 71, 10);

		_vm->removeMessage();
		if (_gm->_key.keycode != Common::KEYCODE_ESCAPE) {
			if (!input.equals("89814")) {
				if (input.equals("41898"))
					_vm->renderMessage(kStringAxacussOffice1_2);
				else
					_vm->renderMessage(kStringAxacussOffice1_3);
			} else {
				_vm->renderImage(6);
				setSectionVisible(7, false);
				obj1.setProperty(OPENABLE | OPENED);
				if (getObject(2)->hasProperty(TAKE)) {
					_vm->renderImage(8);
					getObject(2)->_click = 9;
				}
				_vm->playSound(kAudioDoorOpen);
				_gm->great(7);
			}
		}
	} else if ((verb == ACTION_CLOSE) && (obj1._id == LOCKER) &&
	           obj1.hasProperty(OPENED)) {
		_vm->renderImage(7);
		setSectionVisible(6, false);
		obj1.setProperty(OPENABLE | CLOSED);
		setSectionVisible(8, false);
		getObject(2)->_click = 255;
		_vm->playSound(kAudioDoorClose);
	} else if ((verb == ACTION_TAKE) && (obj1._id == TICKETS)) {
		_vm->renderImage(6);
		setSectionVisible(8, false);
		getObject(2)->_click = 255;
		getObject(2)->resetProperty();
		_gm->takeMoney(500);
	} else if ((verb == ACTION_LOOK) && (obj1._id == LETTER)) {
		g_system->fillScreen(kColorBlack);
		_vm->renderText(kStringAxacussOffice1_4, 10, 10, 4);
		_vm->renderText(kStringAxacussOffice1_5, 270, 10, 4);
		_vm->renderText(kStringAxacussOffice1_6, 10, 60, 4);
		_vm->renderText(kStringAxacussOffice1_7, 10, 75, 4);
		_vm->renderText(kStringAxacussOffice1_8, 10, 86, 4);
		_vm->renderText(kStringAxacussOffice1_9, 10, 97, 4);
		_vm->renderText(kStringAxacussOffice1_10, 10, 108, 4);
		_vm->renderText(kStringAxacussOffice1_11, 10, 119, 4);
		_vm->renderText(kStringAxacussOffice1_12, 10, 130, 4);
		_vm->renderText(kStringAxacussOffice1_13, 10, 147, 4);
		_vm->renderText(kStringAxacussOffice1_14, 200, 170, 4);
		_vm->renderText(kStringAxacussOffice1_15, 200, 181, 4);
		_gm->getInput();
		g_system->fillScreen(kColorBlack);
		_vm->renderMessage(kStringAxacussOffice1_16);
	} else
		return false;

	return true;
}

bool AxacussOffice2::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) &&
	        obj1.hasProperty(OPENED)) {
		_vm->renderImage(_gm->invertSection(9));
		obj1.setProperty(EXIT | OPENABLE | CLOSED);
		_vm->playSound(kAudioDoorClose);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR) &&
	           !getObject(0)->hasProperty(OPENED)) {
		_vm->renderImage(9);
		getObject(0)->setProperty(EXIT | OPENABLE | OPENED);
		_vm->playSound(kAudioDoorOpen);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, COMPUTER, MAGNET)) {
		_vm->renderImage(4);
		setSectionVisible(16, false);
		_vm->playSound(kAudioUndef2);
	} else if ((verb == ACTION_LOOK) && (obj1._id == COMPUTER)) {
		if (isSectionVisible(4))
			_vm->renderMessage(kStringBroken);
		else
			_gm->telomat(1);
	} else
		return false;

	return true;
}

bool AxacussOffice3::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) &&
	        obj1.hasProperty(OPENED)) {
		_vm->renderImage(_gm->invertSection(3));
		obj1.setProperty(EXIT | OPENABLE | CLOSED);
		_vm->playSound(kAudioDoorClose);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR) &&
	           !getObject(0)->hasProperty(OPENED)) {
		_vm->renderImage(3);
		getObject(0)->setProperty(EXIT | OPENABLE | OPENED);
		_vm->playSound(kAudioDoorOpen);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, COMPUTER, MAGNET)) {
		_vm->renderImage(4);
		_vm->playSound(kAudioUndef2);
	} else if ((verb == ACTION_LOOK) && (obj1._id == COMPUTER)) {
		if (isSectionVisible(4))
			_vm->renderMessage(kStringBroken);
		else
			_gm->telomat(2);
	} else if ((verb == ACTION_LOOK) && (obj1._id == PAINTING)) {
		_vm->renderMessage(kStringAxacussOffice3_1);
		_gm->takeMoney(300);
		obj1._id = NULLOBJECT;
	} else
		return false;

	return true;
}

bool AxacussOffice4::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) &&
	        obj1.hasProperty(OPENED)) {
		_vm->renderImage(_gm->invertSection(3));
		obj1.setProperty(EXIT | OPENABLE | CLOSED);
		_vm->playSound(kAudioDoorClose);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR) &&
	           !getObject(0)->hasProperty(OPENED)) {
		_vm->renderImage(3);
		getObject(0)->setProperty(EXIT | OPENABLE | OPENED);
		_vm->playSound(kAudioDoorOpen);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, COMPUTER, MAGNET)) {
		_vm->renderImage(4);
		_vm->playSound(kAudioUndef2);
	} else if ((verb == ACTION_LOOK) && (obj1._id == COMPUTER)) {
		if (isSectionVisible(4))
			_vm->renderMessage(kStringBroken);
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
		_vm->renderImage(4);
		_vm->playSound(kAudioUndef2);
	} else if ((verb == ACTION_TAKE) && (obj1._id == TICKETS)) {
		_vm->renderImage(_gm->invertSection(5));
		obj1._click = 255;
		_gm->takeMoney(350);
	} else
		return false;

	return true;
}

bool AxacussElevator::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_WALK) && (obj1._id == DOOR)) {
		g_system->fillScreen(kColorBlack);
		_vm->setCurrentImage(41);
		_vm->renderImage(0);
		_vm->paletteBrightness();
		_gm->reply(kStringAxacussElevator_1, 1, 1 + 128);
		_gm->say(kStringAxacussElevator_2);
		g_system->fillScreen(kColorBlack);
	} else if ((verb == ACTION_PRESS) && (obj1._id == BUTTON1)) {
		if (!isSectionVisible(3)) {
			_vm->renderImage(1);
			getObject(2)->resetProperty();
			_vm->playSound(kAudioDoorSound);
			_gm->wait2(25);
			for (int i = 3; i <= 7; i++) {
				_gm->wait2(2);
				_vm->renderImage(i);
			}
			getObject(3)->resetProperty(EXIT);
			getObject(3)->_click = 2;
			_vm->renderImage(_gm->invertSection(1));
			if (!(_gm->_state._greatFlag & 0x4000)) {
				_vm->playSound(kAudioGreat);
				_gm->_state._greatFlag |= 0x4000;
			}
		}
	} else if ((verb == ACTION_PRESS) && (obj1._id == BUTTON2)) {
		if (isSectionVisible(3)) {
			_vm->renderImage(2);
			getObject(3)->resetProperty();
			getObject(3)->_click = 255;
			_vm->playSound(kAudioDoorSound);
			for (int i = 7; i >= 3; i--) {
				_gm->wait2(2);
				_vm->renderImage(_gm->invertSection(i));
			}
			_gm->wait2(25);
			_vm->playSound(kAudioDoorSound);
			getObject(2)->resetProperty(EXIT);
			_vm->renderImage(_gm->invertSection(2));
		}
	} else if ((verb == ACTION_WALK) && (obj1._id == JUNGLE)) {
		_vm->paletteFadeOut();
		g_system->fillScreen(kColorBlack);
		_vm->_menuBrightness = 255;
		_vm->paletteBrightness();
		_vm->renderMessage(kStringAxacussElevator_3);
		_gm->waitOnInput(_gm->_timer1);
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
	} else if ((verb == ACTION_WALK) && (obj1._id == DOOR) &&
	           obj1.hasProperty(OPENED)) {
		_gm->great(0);
		_vm->paletteFadeOut();
		_vm->setCurrentImage(35);
		_vm->renderImage(0);
		_vm->renderImage(1);
		_vm->paletteFadeIn();
		_gm->wait2(10);
		for (int i = 8; i <= 21; i++) {
			_vm->renderImage(i);
			_gm->wait2(2);
			_vm->renderImage(_gm->invertSection(i));
		}
		_gm->wait2(18);
		_vm->renderImage(_gm->invertSection(1));
		for (int i = 2; i <= 7; i++) {
			_vm->renderImage(_gm->invertSection(i));
			_gm->wait2(3);
			_vm->renderImage(_gm->invertSection(i));
		}
		_gm->outro();
	} else {
		return false;
	}

	return true;
}

bool AxacussSign::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_USE) && Object::combine(obj1, obj2, STATION_SLOT, MONEY) &&
	    isSectionVisible(1)) {
		_gm->takeMoney(-180);
		_vm->renderImage(2);
		setSectionVisible(1, false);
		_gm->_state._eventTime = _gm->_state._time + ticksToMsec(600);
		_gm->_state._eventCallback = kTaxiFn;
		return true;
	}
	return false;
}

Outro::Outro(SupernovaEngine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = -1;
	_id = OUTRO;
	_shown[0] = kShownFalse;

	outroText =
		_vm->getGameString(kStringOutro1) +
		_vm->getGameString(kStringOutro2) +
		_vm->getGameString(kStringOutro3) +
		_vm->getGameString(kStringOutro4) +
		_vm->getGameString(kStringOutro5) +
		_vm->getGameString(kStringOutro6) +
		_vm->getGameString(kStringOutro7) +
		_vm->getGameString(kStringOutro8) +
		_vm->getGameString(kStringOutro9) +
		_vm->getGameString(kStringOutro10) +
		_vm->getGameString(kStringOutro11) +
		_vm->getGameString(kStringOutro12) +
		_vm->getGameString(kStringOutro13) +
		_vm->getGameString(kStringOutro14);
}

void Outro::onEntrance() {

}

void Outro::animation() {

}

void Outro::animate(int filenumber, int section1, int section2, int duration) {
	_vm->setCurrentImage(filenumber);
	while (duration) {
		_vm->renderImage(section1);
		_gm->wait2(2);
		_vm->renderImage(section2);
		_gm->wait2(2);
		--duration;
	}
}

void Outro::animate(int filenumber, int section1, int section2, int duration,
                    MessagePosition position, const char *text) {
	_vm->renderMessage(text, position);
	int delay = (Common::strnlen(text, 512) + 20) * (10 - duration) * _vm->_textSpeed / 400;
	_vm->setCurrentImage(filenumber);
	while (delay) {
		if (section1)
			_vm->renderImage(section1);
		_gm->wait2(2);
		if (section2)
			_vm->renderImage(section2);
		_gm->wait2(2);
		--delay;
	}
	_vm->removeMessage();
}

void Outro::animate(int filenumber, int section1, int section2, int section3, int section4,
                    int duration, MessagePosition position, const char *text) {
	_vm->renderMessage(text, position);
	if (duration == 0)
		duration = (Common::strnlen(text, 512) + 20) * _vm->_textSpeed / 40;

	_vm->setCurrentImage(filenumber);
	while(duration) {
		_vm->renderImage(section1);
		_vm->renderImage(section3);
		_gm->wait2(2);
		_vm->renderImage(section2);
		_vm->renderImage(section4);
		_gm->wait2(2);
		duration--;
	}
	_vm->removeMessage();
}

}
