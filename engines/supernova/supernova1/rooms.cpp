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

#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/cursorman.h"

#include "supernova/screen.h"
#include "supernova/supernova.h"
#include "supernova/supernova1/state.h"
#include "supernova/supernova1/stringid.h"

namespace Supernova {

Room1::Room1() {
	_dialogsX[0] = kStringDialogX1;
	_dialogsX[1] = kStringDialogX2;
	_dialogsX[2] = kStringDialogX3;
}

Intro::Intro(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = -1;
	_id = INTRO1;
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
	_objectState[4] =
	Object(_id, kStringInventoryMoney, kStringDefaultDescription, MONEY,
		   TAKE | COMBINABLE, 255, 255, 0);

	_shouldExit = false;

	_introText =
		_vm->getGameString(kStringIntro1) + '\1' +
		_vm->getGameString(kStringIntro2) + '\1' +
		_vm->getGameString(kStringIntro3) + '\1' +
		_vm->getGameString(kStringIntro4) + '\1' +
		_vm->getGameString(kStringIntro5) + '\1' +
		_vm->getGameString(kStringIntro6) + '\1' +
		_vm->getGameString(kStringIntro7) + '\1' +
		_vm->getGameString(kStringIntro8) + '\1' +
		_vm->getGameString(kStringIntro9) + '\1' +
		_vm->getGameString(kStringIntro10) + '\1' +
		_vm->getGameString(kStringIntro11) + '\1' +
		_vm->getGameString(kStringIntro12) + '\1' +
		_vm->getGameString(kStringIntro13) + '\1';
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
	_vm->_screen->setViewportBrightness(0);
	_vm->_screen->setGuiBrightness(0);
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
	_gm->wait(1);
	_vm->playSound(kAudioVoiceSupernova);
	while (_vm->_sound->isPlaying())
		_gm->wait(1);
	titleFadeIn();
	_vm->renderText(kStringTitleVersion, 295, 190, kColorWhite44);
	const Common::String& title1 = _vm->getGameString(kStringTitle1);
	const Common::String& title2 = _vm->getGameString(kStringTitle2);
	const Common::String& title3 = _vm->getGameString(kStringTitle3);
	_vm->_screen->renderText(title1, 78 - Screen::textWidth(title1) / 2, 120, kColorLightBlue);
	_vm->_screen->renderText(title2, 78 - Screen::textWidth(title2) / 2, 132, kColorWhite99);
	_vm->_screen->renderText(title3, 78 - Screen::textWidth(title3) / 2, 142, kColorWhite99);
	_gm->wait(1);
	CursorMan.showMouse(true);
	_vm->playSound(kMusicIntro);

	Marquee marquee(_vm->_screen, Marquee::kMarqueeIntro, _introText.c_str());
	while (!_vm->shouldQuit()) {
		_gm->updateEvents();
		marquee.renderCharacter();
		if (_gm->_mouseClicked || _gm->_keyPressed)
			break;
		g_system->updateScreen();
		g_system->delayMillis(_vm->_delay);
	}
	_vm->playSound(kAudioVoiceYeah);
	while (_vm->_sound->isPlaying() && !_vm->shouldQuit())
		_gm->wait(1);
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
					MessagePosition position, int textId) {
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
					int duration, MessagePosition position, int textId) {
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
	if ((_gm->waitOnInput(X, key) && key == Common::KEYCODE_ESCAPE) || _vm->shouldQuit()) { \
		CursorMan.showMouse(true); \
		return; \
	} \
} while (0);

	_vm->_system->fillScreen(kColorBlack);
	_vm->setCurrentImage(31);
	_vm->_screen->setGuiBrightness(255);
	_vm->paletteBrightness();
	if (!animate(0, 0, 0, kMessageNormal, kStringIntroCutscene1))
		return;
	_vm->_screen->setGuiBrightness(0);
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

	int textCounting[4] =
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

	while (_vm->_sound->isPlaying())
		exitOnEscape(1);

	_vm->_system->fillScreen(kColorBlack);
	_vm->_screen->setGuiBrightness(255);
	_vm->paletteBrightness();
	if (!animate(0, 0, 0, kMessageNormal, kStringIntroCutscene25))
		return;
	_vm->_screen->setGuiBrightness(5);
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

	CursorMan.showMouse(false);
	_vm->_screen->setViewportBrightness(0);
	_vm->paletteBrightness();
	exitOnEscape(10);
	_vm->playSound(kAudioSnoring);
	while (_vm->_sound->isPlaying())
		_gm->wait(1);
	exitOnEscape(10);
	_vm->playSound(kAudioSnoring);
	while (_vm->_sound->isPlaying())
		_gm->wait(1);
	exitOnEscape(10);
	_vm->playSound(kAudioSnoring);
	while (_vm->_sound->isPlaying())
		_gm->wait(1);
	exitOnEscape(30);
	CursorMan.showMouse(true);

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
	_vm->_screen->setViewportBrightness(255);
	_vm->removeMessage();
	_gm->changeRoom(CABIN_R3);
	_gm->_guiEnabled = true;
	_vm->_allowSaveGame = true;
	_vm->_allowLoadGame = true;
}

ShipCorridor::ShipCorridor(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 17;
	_id = CORRIDOR_ROOM;
	_shown[0] = kShownTrue;
	_shown[4] = kShownTrue;

	_objectState[0] = Object(_id, kStringHatch, kStringDefaultDescription, HATCH1, OPENABLE | EXIT, 0, 6, 1, CABIN_L1, 15);
	_objectState[1] = Object(_id, kStringHatch, kStringDefaultDescription, NULLOBJECT, OPENABLE | EXIT, 1, 7, 2, CABIN_L2, 10);
	_objectState[2] = Object(_id, kStringHatch, kStringDefaultDescription, NULLOBJECT, OPENABLE | EXIT, 2, 8, 3, CABIN_L3, 5);
	_objectState[3] = Object(_id, kStringHatch, kStringDefaultDescription, NULLOBJECT, OPENABLE | EXIT, 5, 11, 6, CABIN_R1, 19);
	_objectState[4] = Object(_id, kStringHatch, kStringDefaultDescription, NULLOBJECT, OPENABLE | EXIT, 4, 10, 5, CABIN_R2, 14);
	_objectState[5] = Object(_id, kStringHatch, kStringDefaultDescription, NULLOBJECT, OPENABLE | EXIT | OPENED, 9, 3, 4, CABIN_R3, 9);
	_objectState[6] = Object(_id, kStringHatch, kStringDefaultDescription, NULLOBJECT, OPENABLE | CLOSED | EXIT, 12, 12, 0, AIRLOCK, 2);
	_objectState[7] = Object(_id, kStringButton, kStringHatchButtonDescription, BUTTON, PRESS, 13, 13, 0, NULLROOM, 0);
	_objectState[8] = Object(_id, kStringLadder, kStringDefaultDescription, NULLOBJECT, NULLTYPE, 14, 14, 0, NULLROOM, 0);
	_objectState[9] = Object(_id, kStringExit, kStringDefaultDescription, NULLOBJECT, EXIT, 15, 15, 0, HALL, 22);
}

bool ShipCorridor::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_PRESS) && (obj1._id == BUTTON)) {
		if (_objectState[6].hasProperty(OPENED)) {
			_vm->playSound(kAudioSlideDoor);
			_objectState[6].disableProperty(OPENED);
			_vm->renderImage(8);
			setSectionVisible(9, false);
			_gm->wait(2);
			_vm->renderImage(7);
			setSectionVisible(8, false);
			_gm->wait(2);
			_vm->renderImage(_gm->invertSection(7));
		} else {
			_vm->playSound(kAudioSlideDoor);
			_objectState[6].setProperty(OPENED);
			_vm->renderImage(7);
			_gm->wait(2);
			_vm->renderImage(8);
			setSectionVisible(7, false);
			_gm->wait(2);
			_vm->renderImage(9);
			setSectionVisible(8, false);
		}
		return true;
	}
	return false;
}

ShipHall::ShipHall(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 15;
	_id = HALL;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringHatch, kStringCockpitHatchDescription, NULLOBJECT, OPENABLE | EXIT, 4, 5, 1, COCKPIT, 10);
	_objectState[1] = Object(_id, kStringHatch, kStringKitchenHatchDescription, KITCHEN_HATCH, OPENABLE | EXIT, 0, 0, 0, NULLROOM, 1);
	_objectState[2] = Object(_id, kStringHatch, kStringStasisHatchDescription, NULLOBJECT, OPENABLE | CLOSED | EXIT, 1, 1, 2, SLEEP, 8);
	_objectState[3] = Object(_id, kStringSlot, kStringSlotDescription, SLEEP_SLOT, COMBINABLE, 2, 2, 0, NULLROOM, 0);
	_objectState[4] = Object(_id, kStringLadder, kStringDefaultDescription, NULLOBJECT, NULLTYPE, 3, SLEEP, 0, NULLROOM, 0);
	_objectState[5] = Object(_id, kStringCorridor, kStringDefaultDescription, NULLOBJECT, EXIT, 6, 6, 0, CORRIDOR_ROOM, 19);
}

bool ShipHall::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_OPEN) && (obj1._id == KITCHEN_HATCH)) {
		_vm->renderMessage(kStringShipHall1);
	} else if ((verb == ACTION_USE) && Object::combine(obj1,obj2,KEYCARD2,SLEEP_SLOT)) {
		if (_objectState[2].hasProperty(OPENED)) {
			_objectState[2].disableProperty(OPENED);
			_vm->renderImage(3);
			setSectionVisible(4, false);
			_gm->wait(2);
			_vm->renderImage(2);
			setSectionVisible(3, false);
			_gm->wait(2);
			_vm->renderImage(_gm->invertSection(2));
		} else {
			_objectState[2].setProperty(OPENED);
			_vm->renderImage(2);
			_gm->wait(2);
			_vm->renderImage(3);
			setSectionVisible(2, false);
			_gm->wait(2);
			_vm->renderImage(4);
			setSectionVisible(3, false);
			_gm->great(1);
		}
	} else {
		return false;
	}

	return true;
}

ShipSleepCabin::ShipSleepCabin(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 33;
	_id = SLEEP;
	_shown[0] = kShownTrue;
	_color = kColorBlack;

	_objectState[0] = Object(_id, kStringHatch, kStringStasisHatchDescription2, CABINS, NULLTYPE, 0, 0, 0, NULLROOM, 0);
	_objectState[1] = Object(_id, kStringHatch, kStringStasisHatchDescription2, CABIN, NULLTYPE, 1, 1, 0, NULLROOM, 0);
	_objectState[2] = Object(_id, kStringComputer, kStringDefaultDescription, COMPUTER, NULLTYPE, 2, 2, 0, NULLROOM, 0);
	_objectState[3] = Object(_id, kStringExit, kStringDefaultDescription, NULLOBJECT, EXIT, 255, 255, 0, HALL, 22);
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
							_gm->wait(18);
							setSectionVisible(5, true);
						}
					} while (daysSleep == 0);
				} else {
					_vm->renderText(kStringShipSleepCabin9, 100, 125, kColorLightRed);
					_gm->wait(18);
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
			_gm->_time = ticksToMsec(786520); // 12pm
			_gm->_state._alarmOn = (_gm->_state._timeAlarm > _gm->_time);
			if (*energyDaysLeft == 0) {
				_gm->turnOff();
				room = _gm->_rooms[GENERATOR];
				room->setSectionVisible(4, room->isSectionVisible(2));
			}
			if (_gm->_state._arrivalDaysLeft == 0) {
				_gm->saveTime();
				if (!_vm->saveGame(kSleepAutosaveSlot, "Sleep autosave"))
					_vm->errorTempSave(true);
				_gm->_state._dream = true;
				_gm->loadTime();
			}
			_gm->wait(18);
			_vm->paletteFadeIn();
			if (_gm->_state._arrivalDaysLeft == 0) {
				_vm->playSound(kAudioCrash);
				_gm->screenShake();
				_gm->wait(18);
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
		_gm->wait(_gm->_messageDuration, true, true);
		_vm->removeMessage();
		_vm->renderMessage(kStringShipSleepCabin15);
		_gm->wait(_gm->_messageDuration, true, true);
		_vm->removeMessage();
		_vm->renderMessage(kStringShipSleepCabin16);
		_gm->_state._dream = false;
	}
}

ShipCockpit::ShipCockpit(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 9;
	_id = COCKPIT;
	_shown[0] = kShownTrue;
	_color = kColorBlack;

	_objectState[0] = Object(_id, kStringInstruments, kStringInstrumentsDescription1, INSTRUMENTS, NULLTYPE, 2, 2, 0, NULLROOM, 0);
	_objectState[1] = Object(_id, kStringMonitor, kStringDefaultDescription, MONITOR, NULLTYPE, 0, 0, 0, NULLROOM, 0);
	_objectState[2] = Object(_id, kStringMonitor, kStringMonitorDescription, NULLOBJECT, TAKE, 1, 0, 0, NULLROOM, 0);
	_objectState[3] = Object(_id, kStringExit, kStringDefaultDescription, NULLOBJECT, EXIT, 255, 255, 0, HALL, 22);
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
	} else if (isSectionVisible(21)) {
		_vm->renderImage(_gm->invertSection(21));
		_gm->setAnimationTimer(5);
	} else {
		_vm->renderImage(21);
		_gm->setAnimationTimer(10);
	}

	if (_gm->_state._powerOff) {
		if (!_gm->_guiEnabled) {
			_vm->renderText(kStringShipCockpit11, 97, 165, _color);
			_vm->renderText(kStringShipCockpit12, 97, 175, _color);
		} else if (isSectionVisible(21))
			_vm->renderImage(22);
		else
			_vm->renderImage(_gm->invertSection(22));
	}
}

void ShipCockpit::onEntrance() {
	if (!hasSeen())
		_vm->renderMessage(kStringShipCockpit13);
	setRoomSeen(true);
}

ShipCabinL1::ShipCabinL1(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 21;
	_id = CABIN_L1;
	_shown[0] = kShownTrue;
	_shown[1] = kShownTrue;
	_shown[2] = kShownTrue;
	_shown[3] = kShownTrue;
	_shown[4] = kShownTrue;
	_shown[5] = kShownTrue;

	_objectState[0] = Object(_id, kStringImage, kStringGenericDescription1, NULLOBJECT, UNNECESSARY, 5, 5, 0, NULLROOM, 0);
	_objectState[1] = Object(_id, kStringImage, kStringGenericDescription2, NULLOBJECT, UNNECESSARY, 6, 6, 0, NULLROOM, 0);
	_objectState[2] = Object(_id, kStringImage, kStringGenericDescription3, NULLOBJECT, UNNECESSARY, 7, 7, 0, NULLROOM, 0);
	_objectState[3] = Object(_id, kStringMagnete, kStringMagneteDescription, NULLOBJECT, UNNECESSARY, 8, 8, 0, NULLROOM, 0);
	_objectState[4] = Object(_id, kStringImage, kStringGenericDescription4, NULLOBJECT, UNNECESSARY, 9, 9, 0);
	_objectState[5] = Object(_id, kStringPen, kStringPenDescription, PEN, TAKE | COMBINABLE, 10, 10, 5 | kSectionInvert);
	_objectState[6] = Object(_id, kStringHatch, kStringDefaultDescription, NULLOBJECT, OPENABLE | OPENED | EXIT, 3, 3, 24 | kSectionInvert, CORRIDOR_ROOM, 9);
	_objectState[7] = Object(_id, kStringSlot, kStringSlotDescription, NULLOBJECT, COMBINABLE, 0, 0, 0);
	_objectState[8] = Object(_id, kStringShelf, kStringDefaultDescription, NULLOBJECT, OPENABLE | CLOSED, 1, 1, 0);
	_objectState[9] = Object(_id, kStringCompartment, kStringDefaultDescription, NULLOBJECT, OPENABLE | CLOSED, 2, 2, 0);
	_objectState[10] = Object(_id, kStringSocket, kStringDefaultDescription, SOCKET, COMBINABLE, 4, 4, 0);
	_objectState[11] = Object(_id, kStringToilet, kStringDefaultDescription, BATHROOM_DOOR, EXIT, 255, 255, 0, BATHROOM, 22);
}

ShipCabinL2::ShipCabinL2(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 21;
	_id = CABIN_L2;
	_shown[0] = kShownTrue;
	_shown[16] = kShownTrue;

	_objectState[0] = Object(_id, kStringSlot,kStringSlotDescription,SLOT_KL1,COMBINABLE,31,31,0);
	_objectState[1] = Object(_id, kStringSlot,kStringSlotDescription,SLOT_KL2,COMBINABLE,32,32,0);
	_objectState[2] = Object(_id, kStringSlot,kStringSlotDescription,SLOT_KL3,COMBINABLE,33,33,0);
	_objectState[3] = Object(_id, kStringSlot,kStringSlotDescription,SLOT_KL4,COMBINABLE,45,45,0);
	_objectState[4] = Object(_id, kStringShelf,kStringDefaultDescription,SHELF_L1,OPENABLE | CLOSED,25,26,17);
	_objectState[5] = Object(_id, kStringPistol,kStringPistolDescription,PISTOL,TAKE,39,39,20);
	_objectState[6] = Object(_id, kStringCompartment,kStringDefaultDescription,SHELF_L2,OPENABLE | CLOSED,27,28,18);
	_objectState[7] = Object(_id, kStringBooks,kStringBooksDescription,NULLOBJECT,UNNECESSARY,40,40,0);
	_objectState[8] = Object(_id, kStringCompartment,kStringDefaultDescription,SHELF_L3,OPENABLE | CLOSED,29,30,19);
	_objectState[9] = Object(_id, kStringSpool,kStringSpoolDescription, SPOOL,TAKE | COMBINABLE,41,41,21);
	_objectState[10] = Object(_id, kStringCompartment,kStringDefaultDescription,SHELF_L4,OPENABLE | CLOSED,43,44,22);
	_objectState[11] = Object(_id, kStringBook,kStringDefaultDescription,BOOK2,TAKE,46,46,23);
	_objectState[12] = Object(_id, kStringUnderwear,kStringUnderwearDescription,NULLOBJECT,UNNECESSARY,34,34,0);
	_objectState[13] = Object(_id, kStringUnderwear,kStringUnderwearDescription,NULLOBJECT,UNNECESSARY,35,35,0);
	_objectState[14] = Object(_id, kStringClothes,kStringDefaultDescription,NULLOBJECT,UNNECESSARY,36,36,0);
	_objectState[15] = Object(_id, kStringJunk,kStringJunkDescription,NULLOBJECT,UNNECESSARY,37,37,0);
	_objectState[16] = Object(_id, kStringJunk,kStringJunkDescription,NULLOBJECT,UNNECESSARY,38,38,0);
	_objectState[17] = Object(_id, kStringMagnete,kStringMagneteDescription,NULLOBJECT,UNNECESSARY,23,23,0);
	_objectState[18] = Object(_id, kStringToilet,kStringDefaultDescription,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22);
	_objectState[19] = Object(_id, kStringHatch,kStringDefaultDescription,NULLOBJECT,OPENABLE | OPENED | EXIT,3,3,24 | kSectionInvert,CORRIDOR_ROOM,9);
	_objectState[20] = Object(_id, kStringSlot,kStringSlotDescription,NULLOBJECT,COMBINABLE,0,0,0);
	_objectState[21] = Object(_id, kStringShelf,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,1,1,0);
	_objectState[22] = Object(_id, kStringCompartment,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,2,2,0);
	_objectState[23] = Object(_id, kStringSocket,kStringDefaultDescription,SOCKET,COMBINABLE,4,4,0);
	_objectState[24] = Object(_id, kStringFolders,kStringFoldersDescription,NULLOBJECT,UNNECESSARY,49,49,0);
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
	} else if ((verb == ACTION_CLOSE) && (obj1._id == SHELF_L1)) {
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
	} else if ((verb == ACTION_TAKE) && (obj1._id == SPOOL) && !obj1.hasProperty(CARRIED)) {
		getObject(8)->_click = 42;  // empty shelf
		return false;
	} else if ((verb == ACTION_TAKE) && (obj1._id == BOOK2) && !obj1.hasProperty(CARRIED)) {
		getObject(10)->_click = 47;  // empty shelf
		return false;
	} else
		return false;

	return true;
}

ShipCabinL3::ShipCabinL3(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 21;
	_id = CABIN_L3;
	_shown[0] = kShownTrue;
	_shown[6] = kShownTrue;
	_shown[7] = kShownTrue;
	_shown[8] = kShownTrue;
	_shown[9] = kShownTrue;
	_shown[12] = kShownTrue;

	_objectState[0] = Object(_id, kStringPoster,kStringPosterDescription1,NULLOBJECT,UNNECESSARY,11,11,0);
	_objectState[1] = Object(_id, kStringPoster,kStringPosterDescription2,NULLOBJECT,UNNECESSARY,12,12,0);
	_objectState[2] = Object(_id, kStringSpeaker,kStringDefaultDescription,NULLOBJECT,NULLTYPE,13,13,0);
	_objectState[3] = Object(_id, kStringMagnete,kStringMagneteDescription,NULLOBJECT,UNNECESSARY,14,14,0);
	_objectState[4] = Object(_id, kStringRecord,kStringRecordDescription,RECORD,TAKE | COMBINABLE,15,15,8 | kSectionInvert);
	_objectState[5] = Object(_id, kStringRecordStand,kStringRecordStandDescription,NULLOBJECT,UNNECESSARY,16,16,0);
	_objectState[6] = Object(_id, kStringButton,kStringDefaultDescription,TURNTABLE_BUTTON,PRESS,22,22,0);
	_objectState[7] = Object(_id, kStringTurntable,kStringTurntableDescription,TURNTABLE,UNNECESSARY | COMBINABLE,17,17,0);
	_objectState[8] = Object(_id, kStringWire,kStringDefaultDescription,WIRE,COMBINABLE,18,18,0);
	_objectState[9] = Object(_id, kStringWire,kStringDefaultDescription,WIRE2,COMBINABLE,19,19,0);
	_objectState[10] = Object(_id, kStringPlug,kStringDefaultDescription,PLUG,COMBINABLE,20,20,0);
	_objectState[11] = Object(_id, kStringHatch,kStringDefaultDescription,NULLOBJECT,OPENABLE | OPENED | EXIT,3,3,24 | kSectionInvert,CORRIDOR_ROOM,9);
	_objectState[12] = Object(_id, kStringSlot,kStringSlotDescription,NULLOBJECT,COMBINABLE,0,0,0);
	_objectState[13] = Object(_id, kStringShelf,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,1,1,0);
	_objectState[14] = Object(_id, kStringCompartment,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,2,2,0);
	_objectState[15] = Object(_id, kStringSocket,kStringDefaultDescription,SOCKET,COMBINABLE,4,4,0);
	_objectState[16] = Object(_id, kStringToilet,kStringDefaultDescription,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22);
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
				while (_vm->_sound->isPlaying()) {
					if (isSectionVisible(13)) {
						_vm->renderImage(14);
						setSectionVisible(13, false);
					} else {
						_vm->renderImage(13);
						setSectionVisible(14, false);
					}
					_gm->wait(3);
				}
			}

			_vm->renderImage(15);
			setSectionVisible(14, false);
			setSectionVisible(13, false);
			_vm->renderMessage(kStringShipCabinL3_3);
		}
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
		_vm->renderMessage(kStringShipCabinL3_4); // cutting near plug
	else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, KNIFE, WIRE)) {
		r = _gm->_rooms[AIRLOCK];
		if (!isSectionVisible(10) && !r->getObject(5)->hasProperty(WORN)) {
			_vm->renderImage(25);
			_gm->shock();
		}
		_vm->renderImage(11);
		_vm->renderImage(26);
		setSectionVisible(12, false);
	} else if ((verb == ACTION_TAKE) && ((obj1._id == WIRE) || (obj1._id == WIRE2) || (obj1._id == PLUG))) {
		if (isSectionVisible(10) && isSectionVisible(11)) {
			_vm->renderImage(_gm->invertSection(10));
			_vm->renderImage(_gm->invertSection(11));
			getObject(8)->_name = kStringWireAndPlug;
			_gm->takeObject(*getObject(8));
			getObject(9)->_click = 255;
			getObject(10)->_click = 255;
		} else
			_vm->renderMessage(kStringShipCabinL3_5);
	} else
		return false;

	return true;
}

ShipCabinR1::ShipCabinR1(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 22;
	_id = CABIN_R1;
	_shown[0] = kShownTrue;
	_shown[1] = kShownTrue;
	_shown[2] = kShownTrue;

	_objectState[0] = Object(_id, kStringImage,kStringImageDescription1,NULLOBJECT,UNNECESSARY,5,5,0);
	_objectState[1] = Object(_id, kStringDrawingInstruments,kStringDrawingInstrumentsDescription,NULLOBJECT,UNNECESSARY,6,6,0);
	_objectState[2] = Object(_id, kStringMagnete,kStringMagneteDescription,NULLOBJECT,UNNECESSARY,7,7,0);
	_objectState[3] = Object(_id, kStringHatch,kStringDefaultDescription,NULLOBJECT,OPENABLE | OPENED | EXIT,3,3,15 | kSectionInvert,CORRIDOR_ROOM,5);
	_objectState[4] = Object(_id, kStringSlot,kStringSlotDescription,NULLOBJECT,COMBINABLE,0,0,0);
	_objectState[5] = Object(_id, kStringShelf,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,1,1,0);
	_objectState[6] = Object(_id, kStringCompartment,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,2,2,0);
	_objectState[7] = Object(_id, kStringSocket,kStringDefaultDescription,SOCKET,COMBINABLE,4,4,0);
	_objectState[8] = Object(_id, kStringToilet,kStringDefaultDescription,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22);
}

ShipCabinR2::ShipCabinR2(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 22;
	_id = CABIN_R2;
	_shown[0] = kShownTrue;
	_shown[3] = kShownTrue;
	_shown[4] = kShownTrue;
	_shown[5] = kShownTrue;

	_objectState[0] = Object(_id, kStringChessGame,kStringChessGameDescription1,NULLOBJECT,UNNECESSARY,11,11,0);
	_objectState[1] = Object(_id, kStringTennisRacket,kStringTennisRacketDescription,NULLOBJECT,UNNECESSARY,8,8,0);
	_objectState[2] = Object(_id, kStringTennisBall,kStringGenericDescription2,NULLOBJECT,UNNECESSARY,9,9,0);
	_objectState[3] = Object(_id, kStringHatch,kStringDefaultDescription,NULLOBJECT,OPENABLE | OPENED | EXIT,3,3,15 | kSectionInvert,CORRIDOR_ROOM,5);
	_objectState[4] = Object(_id, kStringSlot,kStringSlotDescription,NULLOBJECT,COMBINABLE,0,0,0);
	_objectState[5] = Object(_id, kStringShelf,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,1,1,0);
	_objectState[6] = Object(_id, kStringCompartment,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,2,2,0);
	_objectState[7] = Object(_id, kStringSocket,kStringDefaultDescription,SOCKET,COMBINABLE,4,4,0);
	_objectState[8] = Object(_id, kStringToilet,kStringDefaultDescription,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22);
}

ShipCabinR3::ShipCabinR3(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 22;
	_id = CABIN_R3;
	_shown[0] = kShownTrue;
	_shown[7] = kShownTrue;
	_shown[8] = kShownTrue;
	_shown[15] = kShownTrue;

	_objectState[0] = Object(_id, kStringChessGame,kStringChessGameDescription2,CHESS,TAKE | COMBINABLE,12,12,7 | kSectionInvert);
	_objectState[1] = Object(_id, kStringBed,kStringBedDescription,NULLOBJECT,NULLTYPE,13,13,0);
	_objectState[2] = Object(_id, kStringSlot,kStringSlotDescription,SLOT_K1,COMBINABLE,27,27,0);
	_objectState[3] = Object(_id, kStringSlot,kStringSlotDescription,SLOT_K2,COMBINABLE,28,28,0);
	_objectState[4] = Object(_id, kStringSlot,kStringSlotDescription,SLOT_K3,COMBINABLE,29,29,0);
	_objectState[5] = Object(_id, kStringSlot,kStringSlotDescription,SLOT_K4,COMBINABLE,30,30,0);
	_objectState[6] = Object(_id, kStringCompartment,kStringCompartmentDescription,SHELF1,OPENABLE | CLOSED,14,18,9);
	_objectState[7] = Object(_id, kStringAlbums,kStringAlbumsDescription,NULLOBJECT,UNNECESSARY,14,14,0);
	_objectState[8] = Object(_id, kStringCompartment,kStringCompartmentDescription,SHELF2,OPENABLE | CLOSED,15,19,10);
	_objectState[9] = Object(_id, kStringRope,kStringRopeDescription,ROPE,TAKE | COMBINABLE,15,15,12);
	_objectState[10] = Object(_id, kStringShelf,kStringShelfDescription,SHELF3,OPENABLE | CLOSED,16,17,11);
	_objectState[11] = Object(_id, kStringJunk,kStringJunkDescription,NULLOBJECT,UNNECESSARY,20,20,0);
	_objectState[12] = Object(_id, kStringClothes,kStringClothesDescription,NULLOBJECT,UNNECESSARY,21,21,0);
	_objectState[13] = Object(_id, kStringUnderwear,kStringDefaultDescription,NULLOBJECT,UNNECESSARY,22,22,0);
	_objectState[14] = Object(_id, kStringSocks,kStringDefaultDescription,NULLOBJECT,UNNECESSARY,23,23,0);
	_objectState[15] = Object(_id, kStringCompartment,kStringCompartmentDescription,SHELF4,OPENABLE | CLOSED,24,25,13);
	_objectState[16] = Object(_id, kStringBook,kStringBookHitchhiker,BOOK,TAKE,26,26,14);
	_objectState[17] = Object(_id, kStringDiscman,kStringDiscmanDescription,DISCMAN,TAKE | COMBINABLE,33,33,16);
	_objectState[18] = Object(_id, kStringHatch,kStringDefaultDescription,NULLOBJECT,OPENABLE | EXIT,3,3,15 | kSectionInvert,CORRIDOR_ROOM,5);
	_objectState[19] = Object(_id, kStringSlot,kStringSlotDescription,NULLOBJECT,COMBINABLE,0,0,0);
	_objectState[20] = Object(_id, kStringShelf,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,1,1,0);
	_objectState[21] = Object(_id, kStringCompartment,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,2,2,0);
	_objectState[22] = Object(_id, kStringSocket,kStringDefaultDescription,SOCKET,COMBINABLE,4,4,0);
	_objectState[23] = Object(_id, kStringToilet,kStringDefaultDescription,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22);
}

bool ShipCabinR3::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_K1, KEYCARD))
		_gm->openLocker(this, getObject(6), getObject(2), 9);
	else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, SLOT_K2, KEYCARD)) {
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
	} else if ((verb == ACTION_CLOSE) && (obj1._id == SHELF1))
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
	} else if ((verb == ACTION_TAKE) && (obj1._id == DISCMAN) && !_gm->_rooms[0]->getObject(3)->hasProperty(CARRIED)) {
		getObject(10)->_click = 34; // Locker empty
		obj1._click = 255;
		_gm->takeObject(*_gm->_rooms[0]->getObject(3));
		_vm->renderImage(16);
	} else if ((verb == ACTION_TAKE) && (obj1._id == ROPE) && obj1.hasProperty(CARRIED)) {
		getObject(8)->_click = 31; // Shelf empty
		return false;
	} else if ((verb == ACTION_TAKE) && (obj1._id == BOOK) && !obj1.hasProperty(CARRIED)) {
		getObject(15)->_click = 32; // Shelf empty
		return false;
	} else
		return false;

	return true;
}

void ShipCabinR3::onEntrance() {
	for (int i = 0; i < 3; ++i)
		_gm->_inventory.add(*_gm->_rooms[INTRO1]->getObject(i));

	setRoomSeen(true);
}

ShipCabinBathroom::ShipCabinBathroom(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 23;
	_id = BATHROOM;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringBathroom,kStringBathroomDescription,TOILET,NULLTYPE,0,0,0);
	_objectState[1] = Object(_id, kStringShower,kStringDefaultDescription,SHOWER,NULLTYPE,1,1,0);
	_objectState[2] = Object(_id, kStringExit,kStringDefaultDescription,BATHROOM_EXIT,EXIT,255,255,0,CABIN_R3,2);
}

ShipAirlock::ShipAirlock(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 34;
	_id = AIRLOCK;
	_shown[0] = kShownTrue;
	_shown[6] = kShownTrue;

	_objectState[0] = Object(_id, kStringHatch,kStringHatchDescription1,NULLOBJECT,EXIT | OPENABLE | OPENED | CLOSED,0,0,0,CORRIDOR_ROOM,10);
	_objectState[1] = Object(_id, kStringHatch,kStringHatchDescription2,NULLOBJECT,EXIT | OPENABLE | CLOSED,1,1,0,HOLD,14);
	_objectState[2] = Object(_id, kStringButton,kStringDefaultDescription,BUTTON1,PRESS,2,2,0);
	_objectState[3] = Object(_id, kStringButton,kStringDefaultDescription,BUTTON2,PRESS,3,3,0);
	_objectState[4] = Object(_id, kStringHelmet,kStringHelmetDescription,HELMET,TAKE,4,4,7);
	_objectState[5] = Object(_id, kStringSuit,kStringSuitDescription,SUIT,TAKE,5,5,8);
	_objectState[6] = Object(_id, kStringLifeSupport,kStringLifeSupportDescription,LIFESUPPORT,TAKE,6,6,9);
}

bool ShipAirlock::interact(Action verb, Object &obj1, Object &obj2) {
	Room *r;

	if ((verb == ACTION_PRESS) && (obj1._id == BUTTON1)) {
		if (!getObject(1)->hasProperty(OPENED)) {
			_vm->renderImage(10);
			_vm->playSound(kAudioSlideDoor);
			if (getObject(0)->hasProperty(OPENED)) {
				getObject(0)->disableProperty(OPENED);
				_vm->renderImage(1);
				_gm->wait(2);
				_vm->renderImage(2);
				setSectionVisible(1, false);
				_gm->wait(2);
				_vm->renderImage(3);
				setSectionVisible(2, false);
			} else {
				getObject(0)->setProperty(OPENED);
				_vm->renderImage(2);
				setSectionVisible(3, false);
				_gm->wait(2);
				_vm->renderImage(1);
				setSectionVisible(2, false);
				_gm->wait(2);
				_vm->renderImage(_gm->invertSection(1));
			}
			_vm->renderImage(_gm->invertSection(10));
		}
	} else if ((verb == ACTION_PRESS) && (obj1._id == BUTTON2)) {
		if (!getObject(0)->hasProperty(OPENED)) {
			_vm->renderImage(11);
			if (getObject(1)->hasProperty(OPENED)) {
				_vm->playSound(kAudioSlideDoor);
				getObject(1)->disableProperty(OPENED);
				_vm->renderImage(4);
				_gm->wait(2);
				_vm->renderImage(5);
				setSectionVisible(4, false);
				_gm->wait(2);
				_vm->renderImage(6);
				setSectionVisible(5, false);
				_vm->renderImage(16);
				setSectionVisible(17, false);
				_gm->wait(3);
				_vm->renderImage(15);
				setSectionVisible(16, false);
				_gm->wait(3);
				_vm->renderImage(14);
				setSectionVisible(15, false);
				_gm->wait(3);
				_vm->renderImage(13);
				setSectionVisible(14, false);
				_gm->wait(3);
				_vm->renderImage(12);
				setSectionVisible(13, false);
				_gm->wait(3);
				_vm->renderImage(_gm->invertSection(12));
			} else {
				getObject(1)->setProperty(OPENED);
				_vm->renderImage(12);
				_gm->wait(3);
				_vm->renderImage(13);
				setSectionVisible(12, false);
				_gm->wait(3);
				_vm->renderImage(14);
				setSectionVisible(13, false);
				_gm->wait(3);
				_vm->renderImage(15);
				setSectionVisible(14, false);
				_gm->wait(3);
				_vm->renderImage(16);
				setSectionVisible(15, false);
				_gm->wait(3);
				_vm->renderImage(17);
				setSectionVisible(16, false);
				_vm->playSound(kAudioSlideDoor);
				_vm->renderImage(5);
				setSectionVisible(6, false);
				_gm->wait(2);
				_vm->renderImage(4);
				setSectionVisible(5, false);
				_gm->wait(2);
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
	} else if ((verb == ACTION_LOOK) && (obj1._id == MANOMETER))
		_vm->renderMessage(getObject(1)->hasProperty(OPENED) ? kStringShipAirlock2 : kStringShipAirlock3);
	else
		return false;

	return true;
}

void ShipAirlock::onEntrance() {
	if (!isSectionVisible(kMaxSection - 1)) {
		_vm->renderMessage(kStringShipAirlock4);
		setSectionVisible(kMaxSection -1, kShownTrue);
	}
	if (_vm->_improved && isSectionVisible(kMaxSection - 2)) {
		if (_objectState[0].hasProperty(OPENED)) {
			_objectState[4].setProperty(WORN);
			_objectState[5].setProperty(WORN);
			_objectState[6].setProperty(WORN);
			interact(ACTION_PRESS, _objectState[2], _gm->_nullObject);
			interact(ACTION_PRESS, _objectState[3], _gm->_nullObject);
		} else {
			_objectState[4].disableProperty(WORN);
			_objectState[5].disableProperty(WORN);
			_objectState[6].disableProperty(WORN);
			interact(ACTION_PRESS, _objectState[3], _gm->_nullObject);
			interact(ACTION_PRESS, _objectState[2], _gm->_nullObject);
		}
	}

}

ShipHold::ShipHold(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 24;
	_id = HOLD;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kNoString,kStringDefaultDescription,HOLD_WIRE,COMBINABLE,255,255,0);
	_objectState[1] = Object(_id, kStringScrap,kStringScrapDescription1,SCRAP_LK,NULLTYPE,4,4,0);
	_objectState[2] = Object(_id, kStringTerminalStrip,kStringDefaultDescription,TERMINALSTRIP,COMBINABLE,255,255,0);
	_objectState[3] = Object(_id, kStringScrap,kStringScrapDescription2,NULLOBJECT,NULLTYPE,5,5,0);
	_objectState[4] = Object(_id, kStringReactor,kStringReactorDescription,NULLOBJECT,NULLTYPE,6,6,0);
	_objectState[5] = Object(_id, kStringNozzle,kStringDefaultDescription,NULLOBJECT,NULLTYPE,7,7,0);
	_objectState[6] = Object(_id, kStringPumpkin,kStringPumpkinDescription,NULLOBJECT,NULLTYPE,8,8,0);
	_objectState[7] = Object(_id, kStringHatch,kStringDefaultDescription,LANDINGMOD_OUTERHATCH,EXIT | OPENABLE,1,2,2,LANDINGMODULE,6);
	_objectState[8] = Object(_id, kStringLandingModule,kStringLandingModuleDescription,NULLOBJECT,NULLTYPE,0,0,0);
	_objectState[9] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,255,255,0,AIRLOCK,22);
	_objectState[10] = Object(_id, kStringHatch,kStringHatchDescription3,OUTERHATCH_TOP,EXIT | OPENABLE | OPENED,3,3,0,GENERATOR,8);
	_objectState[11] = Object(_id, kStringGenerator,kStringGeneratorDescription,GENERATOR_TOP,EXIT,12,12,0,GENERATOR,8);
}

bool ShipHold::interact(Action verb, Object &obj1, Object &obj2) {
	Room *room;

	if ((verb == ACTION_LOOK) && (obj1._id == SCRAP_LK) && (obj1._description != kStringScrapDescription3)) {
		_vm->renderMessage(obj1._description);
		obj1._description = kStringScrapDescription3;
		_gm->takeObject(*getObject(2));
	} else if (((verb == ACTION_OPEN) || (verb == ACTION_CLOSE)) && (obj1._id == OUTERHATCH_TOP))
		_vm->renderMessage(kStringShipHold1);
	else if ((verb == ACTION_CLOSE) && (obj1._id == LANDINGMOD_HATCH) && (isSectionVisible(4) || isSectionVisible(6)))
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
		} else
			_vm->renderMessage(kStringCable4);
	} else
		return false;

	return true;
}

void ShipHold::onEntrance() {
	if (!hasSeen())
		_vm->renderMessage(kStringShipHold2);
	_gm->_rooms[AIRLOCK]->setSectionVisible(kMaxSection - 2, kShownTrue);
	setRoomSeen(true);
	_gm->_rooms[COCKPIT]->setRoomSeen(true);
}

ShipLandingModule::ShipLandingModule(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 25;
	_id = LANDINGMODULE;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringSocket,kStringDefaultDescription,LANDINGMOD_SOCKET,COMBINABLE,1,1,0);
	_objectState[1] = Object(_id, kStringButton,kSafetyButtonDescription,LANDINGMOD_BUTTON,PRESS | COMBINABLE,2,2,0);
	_objectState[2] = Object(_id, kStringMonitor,kStringDefaultDescription,LANDINGMOD_MONITOR,NULLTYPE,3,3,0);
	_objectState[3] = Object(_id, kStringKeyboard,kStringDefaultDescription,KEYBOARD,NULLTYPE,4,4,0);
	_objectState[4] = Object(_id, kNoString,kStringDefaultDescription,LANDINGMOD_WIRE,COMBINABLE,255,255,0);
	_objectState[5] = Object(_id, kStringHatch,kStringDefaultDescription,LANDINGMOD_HATCH,EXIT | OPENABLE | OPENED | COMBINABLE, 0,0,1 | kSectionInvert,HOLD,10);
}

bool ShipLandingModule::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_PRESS) && (obj1._id == LANDINGMOD_BUTTON))
		_vm->renderMessage(obj1._description);
	else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, PEN, LANDINGMOD_BUTTON)) {
		if (_gm->_state._landingModuleEnergyDaysLeft) {
			Room *r = _gm->_rooms[GENERATOR];
			if (isSectionVisible(7)) {
				_vm->renderImage(_gm->invertSection(9));
				_vm->renderImage(_gm->invertSection(2));
				_vm->renderImage(_gm->invertSection(8));
				_vm->renderImage(_gm->invertSection(7));
				_vm->renderImage(_gm->invertSection(10));
				if (r->isSectionVisible(9))
					_gm->_state._powerOff = true;
				_gm->roomBrightness();
			} else {
				_vm->renderImage(7);
				if (r->isSectionVisible(9))
					_gm->_state._powerOff = false;
				_gm->roomBrightness();
				r = _gm->_rooms[SLEEP];
				r->setSectionVisible(1, false);
				r->setSectionVisible(2, false);
				_gm->wait(2);
				_vm->renderImage(2);
				_gm->wait(3);
				_vm->renderImage(8);
				_gm->wait(2);
				_vm->renderImage(9);
				_gm->wait(1);
				_vm->renderImage(10);
			}
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, KNIFE, LANDINGMOD_BUTTON))
		_vm->renderMessage(kStringShipHold3);
	else if ((verb == ACTION_LOOK) && (obj1._id == LANDINGMOD_MONITOR) && isSectionVisible(7))
		_vm->renderMessage(kStringShipHold4);
	else if ((verb == ACTION_USE) && (obj1._id == KEYBOARD))
		_vm->renderMessage(kStringShipHold5);
	else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, WIRE, LANDINGMOD_SOCKET)) {
		Room *r = _gm->_rooms[CABIN_L3];
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
		Room *r = _gm->_rooms[HOLD];
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
			Room *r = _gm->_rooms[HOLD];
			if (isSectionVisible(5)) {
				r->setSectionVisible(5, true);
				r->getObject(0)->_click = 10;
			} else
				r->getObject(0)->_click = 9;

			r->setSectionVisible(4, true);
			r->getObject(0)->_name = getObject(4)->_name;
			_vm->renderImage(_gm->invertSection(5));
			_vm->renderImage(_gm->invertSection(4));
			setSectionVisible(11, false);
			_vm->renderImage(6);
			getObject(4)->_click = 7;
		} else
			_vm->renderMessage(kStringShipHold6);
	} else if ((verb == ACTION_CLOSE) && (obj1._id == LANDINGMOD_HATCH) && isSectionVisible(6))
		_vm->renderMessage(kStringCable1);
	else if (((verb == ACTION_TAKE) || (verb == ACTION_PULL)) && (obj1._id == LANDINGMOD_WIRE))
		_vm->renderMessage(kStringCable2);
	else
		return false;

	return true;
}

ShipGenerator::ShipGenerator(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 18;
	_id = GENERATOR;
	_shown[0] = kShownTrue;
	_shown[5] = kShownTrue;

	_objectState[0] = Object(_id, kStringGeneratorWire,kStringDefaultDescription,GENERATOR_WIRE,COMBINABLE,255,255,0);
	_objectState[1] = Object(_id, kStringEmptySpool,kStringDefaultDescription,NULLOBJECT,UNNECESSARY,255,255,0);
	_objectState[2] = Object(_id, kStringKeycard2,kStringKeycard2Description,KEYCARD2,COMBINABLE | TAKE,12,12,5 | kSectionInvert);
	_objectState[3] = Object(_id, kStringRope,kStringDefaultDescription,GENERATOR_ROPE,COMBINABLE,255,255,0);
	_objectState[4] = Object(_id, kStringHatch,kStringHatchDescription3,OUTERHATCH,EXIT | OPENABLE,1,2,1,OUTSIDE,22);
	_objectState[5] = Object(_id, kStringHatch,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,3,3,0);
	_objectState[6] = Object(_id, kStringSlot,kStringSlotDescription,NULLOBJECT,COMBINABLE,4,4,0);
	_objectState[7] = Object(_id, kStringTrap,kStringDefaultDescription,TRAP,OPENABLE,5,6,2);
	_objectState[8] = Object(_id, kStringWire,kStringDefaultDescription,NULLOBJECT,NULLTYPE,7,7,0);
	_objectState[9] = Object(_id, kStringVoltmeter,kStringDefaultDescription,VOLTMETER,NULLTYPE,9,9,0,NULLROOM,0);
	_objectState[10] = Object(_id, kStringClip,kStringDefaultDescription,CLIP,COMBINABLE,8,8,0);
	_objectState[11] = Object(_id, kStringWire,kStringWireDescription,SHORT_WIRE,COMBINABLE,10,10,0);
	_objectState[12] = Object(_id, kStringLadder,kStringDefaultDescription,LADDER,EXIT,0,0,0,HOLD,1);
}

bool ShipGenerator::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_OPEN) && (obj1._id == OUTERHATCH)) {
		if (obj1.hasProperty(OPENED))
			return false;
		_vm->playSound(kAudioSlideDoor);
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
		if (isSectionVisible(11) || isSectionVisible(12))
			_vm->renderMessage(kStringShipHold7);
		else {
			_vm->playSound(kAudioSlideDoor);
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
		_vm->playSound(kAudioSlideDoor);
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
		Room *r = _gm->_rooms[LANDINGMODULE];
		if (_gm->_state._landingModuleEnergyDaysLeft && r->isSectionVisible(7))
			_gm->turnOn();
		else
			_vm->renderImage(4);
		_gm->_rooms[HOLD]->setSectionVisible(7, true);
		_gm->great(3);
	} else if ((verb == ACTION_PULL) && (obj1._id == GENERATOR_WIRE) && (obj1._click == 16)) {
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
		Room *r = _gm->_rooms[CABIN_R3];
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
			} else
				_vm->renderImage(12);

			Room *r = _gm->_rooms[OUTSIDE];
			r->setSectionVisible(1, true);
			r->getObject(1)->_click = 1;
			getObject(3)->_click = 17;
		}
	} else if ((verb == ACTION_TAKE) && (obj1._id == GENERATOR_ROPE))
		_vm->renderMessage(kStringShipHold15);
	else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, OUTERHATCH, GENERATOR_WIRE) && isSectionVisible(1))
		_vm->renderMessage(kStringShipHold16);
	else
		return false;

	return true;
}

ShipOuterSpace::ShipOuterSpace(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 4;
	_id = OUTSIDE;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringHatch,kStringDefaultDescription,NULLOBJECT,EXIT,0,0,0,GENERATOR,3);
	_objectState[1] = Object(_id, kStringRope,kStringDefaultDescription,NULLOBJECT,UNNECESSARY,255,255,0);
}


// Arsano
ArsanoRocks::ArsanoRocks(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 12;
	_id = OUTSIDE;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringRope,kStringDefaultDescription,NULLOBJECT,UNNECESSARY | EXIT,0,0,0,GENERATOR,12);
	_objectState[1] = Object(_id, kStringStone,kStringDefaultDescription,STONE,NULLTYPE,1,1,0);
	_objectState[2] = Object(_id, kStringStone,kStringDefaultDescription,NULLOBJECT,NULLTYPE,2,2,0);
	_objectState[3] = Object(_id, kStringCaveOpening,kStringCaveOpeningDescription,NULLOBJECT,NULLTYPE,255,255,0,CAVE,1);
}

void ArsanoRocks::onEntrance() {
	_gm->great(8);
}

bool ArsanoRocks::interact(Action verb, Object &obj1, Object &obj2) {
	if (((verb == ACTION_PULL) || (verb == ACTION_PRESS)) &&
		(obj1._id == STONE) && !isSectionVisible(3)) {
		_vm->renderImage(1);
		_gm->wait(2);
		_vm->renderImage(2);
		_gm->wait(2);
		_vm->renderImage(3);
		_vm->playSound(kAudioRocks);
		obj1._click = 3;
		getObject(3)->_click = 4;
		getObject(3)->setProperty(EXIT);
		return true;
	}
	return false;
}

ArsanoCave::ArsanoCave(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 12;
	_id = CAVE;

	_objectState[0] = Object(_id, kStringExit,kStringExitDescription,NULLOBJECT,EXIT,255,255,0,ROCKS,22);
	_objectState[1] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,255,255,0,MEETUP,2);
}

ArsanoMeetup::ArsanoMeetup(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 37;
	_id = MEETUP;
	_shown[0] = kShownTrue;
	_sign = 0;
	_beacon = 0;

	_objectState[0] = Object(_id, kStringCave,kStringDefaultDescription,NULLOBJECT,EXIT,255,255,0,CAVE,22);
	_objectState[1] = Object(_id, kStringSign,kStringSignDescription,MEETUP_SIGN,NULLTYPE,0,0,0);
	_objectState[2] = Object(_id, kStringEntrance,kStringDefaultDescription,DOOR,EXIT,1,1,0,ENTRANCE,7);
	_objectState[3] = Object(_id, kStringStar,kStringDefaultDescription,STAR,NULLTYPE,2,2,0);
	_objectState[4] = Object(_id, kStringSpaceshift,kStringDefaultDescription,SPACESHIPS,COMBINABLE,3,3,0);
	_objectState[5] = Object(_id, kStringSpaceshift,kStringDefaultDescription,SPACESHIP,COMBINABLE,4,4,0);
}

void ArsanoMeetup::onEntrance() {
	if (isSectionVisible(7)) {
		_gm->wait(3);
		_vm->renderImage(6);
		setSectionVisible(7, false);
		_gm->wait(3);
		_vm->renderImage(_gm->invertSection(6));
	}
	if (!(_gm->_state._greatFlag & 0x8000)) {
		_vm->playSound(kAudioFoundLocation);
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
	} else if ((verb == ACTION_WALK) && (obj1._id == SPACESHIP))
		_gm->changeRoom(GLIDER);
	else if ((verb == ACTION_WALK) && (obj1._id == STAR))
		_vm->renderMessage(kStringArsanoMeetup2);
	else if ((verb == ACTION_LOOK) && (obj1._id == STAR)) {
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
		_gm->wait(3);
		_vm->renderImage(7);
		setSectionVisible(6, false);
		_gm->wait(3);

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
	} else
		return false;

	return true;
}

ArsanoEntrance::ArsanoEntrance(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 10;
	_id = ENTRANCE;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringPorter,kStringPorterDescription,PORTER,TALK,0,0,0);
	_objectState[1] = Object(_id, kStringDoor,kStringDefaultDescription,NULLOBJECT,EXIT | OPENABLE | CLOSED,1,1,0,NULLROOM,5);
	_objectState[2] = Object(_id, kStringSign,kStringSignDescription,KITCHEN_SIGN,NULLTYPE,2,2,0);
	_objectState[3] = Object(_id, kStringChewingGum,kStringDefaultDescription,SCHNUCK,TAKE,255,255,10+kSectionInvert);
	_objectState[4] = Object(_id, kStringGummyBears,kStringDefaultDescription,SCHNUCK,TAKE,255,255,11+kSectionInvert);
	_objectState[5] = Object(_id, kStringChocolateBall,kStringDefaultDescription,SCHNUCK,TAKE,255,255,12+kSectionInvert);
	_objectState[6] = Object(_id, kStringEgg,kStringDefaultDescription,EGG,TAKE,255,255,13+kSectionInvert);
	_objectState[7] = Object(_id, kStringLiquorice,kStringDefaultDescription,SCHNUCK,TAKE,255,255,14+kSectionInvert);
	_objectState[8] = Object(_id, kStringPill,kStringPillDescription,PILL,TAKE,255,255,0);
	_objectState[9] = Object(_id, kStringSlot,kStringDefaultDescription,CAR_SLOT,COMBINABLE,6,6,0);
	_objectState[10] = Object(_id, kStringVendingMachine,kStringVendingMachineDescription,NULLOBJECT,NULLTYPE,5,5,0);
	_objectState[11] = Object(_id, kStringToilet,kStringToiletDescription,ARSANO_BATHROOM,NULLTYPE,255,255,0);
	_objectState[12] = Object(_id, kStringButton,kStringDefaultDescription,BATHROOM_BUTTON,PRESS,3,3,0);
	_objectState[13] = Object(_id, kStringSign,kStringSignDescription,BATHROOM_SIGN,NULLTYPE,4,4,0);
	_objectState[14] = Object(_id, kStringStaircase,kStringDefaultDescription,STAIRCASE,EXIT,8,8,0,REST,3);
	_objectState[15] = Object(_id, kStringExit,kStringDefaultDescription,MEETUP_EXIT,EXIT,255,255,0,MEETUP,22);
	_objectState[16] = Object(_id, kStringCoins,kStringCoinsDescription,COINS,TAKE|COMBINABLE,255,255,0);
	_objectState[17] = Object(_id, kStringTabletPackage,kStringTabletPackageDescription,PILL_HULL,TAKE,255,255,0);

	_dialog1[0] = kStringArsanoDialog7;
	_dialog1[1] = kStringArsanoDialog1;
	_dialog1[2] = kStringArsanoDialog8;
	_dialog1[3] = kStringArsanoDialog9;
	_dialog1[4] = kStringDialogSeparator;

	_dialog2[0] = kStringArsanoDialog1;
	_dialog2[1] = kStringArsanoDialog2;
	_dialog2[2] = kStringArsanoDialog3;
	_dialog2[3] = kStringArsanoDialog4;
	_dialog2[4] = kStringDialogSeparator;

	_dialog3[0] = kStringArsanoDialog5;
	_dialog3[1] = kStringArsanoDialog6;

	_eyewitness = 5;
}

void ArsanoEntrance::animation() {
	if (!_vm->_screen->isMessageShown() && isSectionVisible(kMaxSection - 5)) {
		_gm->animationOff(); // to avoid recursive call
		_vm->playSound(kAudioSlideDoor);
		_vm->renderImage(8);
		setSectionVisible(9, false);
		_gm->wait(2);
		_vm->renderImage(7);
		setSectionVisible(8, false);
		_gm->wait(2);
		_vm->renderImage(6);
		setSectionVisible(7, false);
		_gm->wait(2);
		_vm->renderImage(5);
		setSectionVisible(6, false);
		_gm->wait(2);
		_vm->renderImage(_gm->invertSection(5));
		getObject(11)->_click = 255;
		setSectionVisible(kMaxSection - 5, false);
		_gm->animationOn();
	}
	if (isSectionVisible(2))
		_vm->renderImage(_gm->invertSection(2));
	else if (_eyewitness)
		--_eyewitness;
	else {
		_eyewitness = 20;
		_vm->renderImage(2);
	}

	_gm->setAnimationTimer(4);
}

bool ArsanoEntrance::interact(Action verb, Object &obj1, Object &obj2) {
	static byte row1[6] = {1, 1, 1, 1, 1, 0};
	static byte row2[6] = {1, 1, 1, 1, 1, 0};
	static byte row3[6] = {1, 1, 0, 0, 0, 0};

	if ((verb == ACTION_TALK) && (obj1._id == PORTER)) {
		if (_gm->_rooms[AIRLOCK]->getObject(4)->hasProperty(WORN))
			_vm->renderMessage(kStringArsanoEntrance1);
		else {
			if (_gm->_state._language) {
				int e;
				do {
					if (_gm->_state._shoes == 1) {
						_dialog2[2] = kStringArsanoEntrance2;
						addSentence(2, 2);
					} else if (_gm->_state._shoes > 1)
						removeSentence(2, 2);

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
						break;
					default:
						break;
					}
				} while (e != 4);
			} else if (_gm->dialog(5, row2, _dialog2, 0) != 4)
				_gm->reply(kStringArsanoEntrance10, 1, _gm->invertSection(1));
		}
	} else if ((verb == ACTION_WALK) && (obj1._id == STAIRCASE) && (_gm->_state._shoes != 3)) {
		_vm->renderImage(3);
		_gm->wait(2);
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
		_gm->wait(2);
		_vm->renderImage(_gm->invertSection(3));
		if (!_gm->_rooms[AIRLOCK]->getObject(4)->hasProperty(WORN)) {
			if (_gm->_state._language) {
				if (_gm->_state._shoes)
					_gm->reply(kStringArsanoEntrance13, 1, _gm->invertSection(1));
				else
					_gm->reply(kStringArsanoEntrance14, 1, _gm->invertSection(1));
				int e = 0;
				while ((e < 3) && (!allSentencesRemoved(4, 1))) {
					switch (e = _gm->dialog(5, row1, _dialog1, 1)) {
					case 0:
						_gm->reply(kStringArsanoEntrance15, 1, 1 + kSectionInvert);
						break;
					case 1:
						_gm->reply(kStringArsanoEntrance3, 1, 1 + kSectionInvert);
						_gm->reply(kStringArsanoEntrance4, 1, 1 + kSectionInvert);
						_gm->reply(kStringArsanoEntrance5, 1, 1 + kSectionInvert);
						removeSentence(0, 2);
						break;
					case 2:
						_gm->reply(kStringArsanoEntrance7, 1, 1 + kSectionInvert);
						_gm->_state._shoes = 2;
						break;
					case 3:
						_vm->renderImage(3);
						_gm->wait(2);
						_vm->renderImage(4);
						setSectionVisible(3, false);
						_gm->reply(kStringArsanoEntrance16, 1, 1 + kSectionInvert);
						_vm->renderImage(3);
						setSectionVisible(4, false);
						_gm->wait(2);
						_vm->renderImage(_gm->invertSection(3));
						break;
					default:
						break;
					}
					removeSentence(0, 1);
				}
			} else {
				_gm->dialog(2, row3, _dialog3, 0);
				_gm->reply(kStringArsanoEntrance10, 1, 1 + kSectionInvert);
			}
		}
	} else if ((verb == ACTION_PRESS) && (obj1._id == BATHROOM_BUTTON)) {
		_vm->playSound(kAudioSlideDoor);
		_vm->renderImage(5);
		_gm->wait(2);
		_vm->renderImage(6);
		setSectionVisible(5, false);
		_gm->wait(2);
		_vm->renderImage(7);
		setSectionVisible(6, false);
		_gm->wait(2);
		_vm->renderImage(8);
		setSectionVisible(7, false);
		_gm->wait(2);
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
				_gm->wait(_gm->_messageDuration, true, true);
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
			if (_gm->_state._coins == 1)
				getObject(16)->_name = kStringCoin;

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
			_gm->_rooms[AIRLOCK]->getObject(4)->setProperty(WORN);
			_gm->_rooms[AIRLOCK]->getObject(5)->setProperty(WORN);
			_gm->_rooms[AIRLOCK]->getObject(6)->setProperty(WORN);
			_gm->wait(_gm->_messageDuration, true, true);
			_vm->removeMessage();
		}
		return false;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, KNIFE, PORTER))
		_vm->renderMessage(kStringArsanoEntrance27);
	else
		return false;

	return true;
}

ArsanoRemaining::ArsanoRemaining(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 28;
	_id = REST;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringStaircase,kStringDefaultDescription,NULLOBJECT,EXIT,0,0,0,ENTRANCE,17);
	_objectState[1] = Object(_id, kStringChair,kStringDefaultDescription,NULLOBJECT,EXIT,1,1,0,ROGER,2);
	_objectState[2] = Object(_id, kStringShoes,kStringShoesDescription,NULLOBJECT,NULLTYPE,2,2,0);

	_chewing = kShownTrue;
	_i = 0;
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
		break;
	default:
		break;
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

ArsanoRoger::ArsanoRoger(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 29;
	_id = ROGER;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,255,255,0,REST,19);
	_objectState[1] = Object(_id, kStringFrogFace,kStringDefaultDescription,ROGER_W,TALK,0,0,0);
	_objectState[2] = Object(_id, kStringScrible,kStringScribleDescription,NULLOBJECT,NULLTYPE,3,3,0);
	_objectState[3] = Object(_id, kStringWallet,kStringDefaultDescription,WALLET,TAKE,1,1,4);
	_objectState[4] = Object(_id, kStringMenu,kStringMenuDescription,NULLOBJECT,UNNECESSARY,2,2,0);
	_objectState[5] = Object(_id, kStringCup,kStringCupDescription,CUP,UNNECESSARY,4,4,0);
	_objectState[6] = Object(_id, kStringChessGame,kStringDefaultDescription,NULLOBJECT,UNNECESSARY,255,255,0);
	_objectState[7] = Object(_id, kStringBill,kStringBillDescription,NULLOBJECT,TAKE|COMBINABLE,255,255,0);
	_objectState[8] = Object(_id, kStringKeycard3,kStringDefaultDescription,KEYCARD_R,TAKE|COMBINABLE,255,255,0);

	_dialog1[0] = kStringDialogArsanoRoger1;
	_dialog1[1] = kStringDialogArsanoRoger2;
	_dialog1[2] = kStringDialogArsanoRoger3;
	_dialog1[3] = kStringDialogSeparator;

	_eyewitness = 5;
	_hands = 0;
}

void ArsanoRoger::onEntrance() {
	if (!sentenceRemoved(0, 2)) {
		_gm->say(kStringArsanoRoger1);
		_gm->reply(kStringArsanoRoger2, 2, 2 + kSectionInvert);
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
	} else if (_eyewitness) {
		--_eyewitness;
	} else {
		_eyewitness = 20;
		if (isSectionVisible(3))
			_vm->renderImage(10);
		else
			_vm->renderImage(1);
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
		_gm->reply(kStringArsanoRoger3, 2, 2 + kSectionInvert);
	} else if ((verb == ACTION_USE) && (obj1._id == CUP))
		_vm->renderMessage(kStringArsanoRoger4);
	else if ((verb == ACTION_TALK) && (obj1._id == ROGER_W)) {
		if (isSectionVisible(3))
			_vm->renderMessage(kStringArsanoRoger5);
		else {
			switch (_gm->dialog(4, row1, _dialog1, 1)) {
			case 0:
				_gm->reply(kStringArsanoRoger6, 2, 2 + kSectionInvert);
				_gm->reply(kStringArsanoRoger7, 2, 2 + kSectionInvert);
				break;
			case 1:
				_gm->reply(kStringArsanoRoger8, 2, 2 + kSectionInvert);
				_gm->reply(kStringArsanoRoger9, 2, 2 + kSectionInvert);
				_gm->say(kStringArsanoRoger10);
				break;
			case 2:
				_gm->reply(kStringArsanoRoger11, 2, 2 + kSectionInvert);
				_gm->say(kStringArsanoRoger12);
				_gm->reply(kStringArsanoRoger13, 2, 2 + kSectionInvert);
				_gm->say(kStringArsanoRoger14);
				_gm->reply(kStringArsanoRoger15, 2, 2 + kSectionInvert);
				_gm->reply(kStringArsanoRoger16, 2, 2 + kSectionInvert);
				_gm->say(kStringArsanoRoger17);
				_gm->say(kStringArsanoRoger18);
				_gm->reply(kStringArsanoRoger19, 2, 2 + kSectionInvert);
				_gm->say(kStringArsanoRoger20);
				_gm->say(kStringArsanoRoger21);
				_gm->reply(kStringArsanoRoger22, 2, 2 + kSectionInvert);
				_gm->say(kStringArsanoRoger23);
				_gm->reply(kStringArsanoRoger24, 2, 2 + kSectionInvert);
				_gm->reply(kStringArsanoRoger25, 2, 2 + kSectionInvert);
				_gm->say(kStringArsanoRoger26);
				_gm->reply(kStringArsanoRoger27, 2, 2 + kSectionInvert);
				_gm->reply(kStringArsanoRoger28, 2, 2 + kSectionInvert);
				_gm->say(kStringArsanoRoger29);
				_gm->reply(kStringArsanoRoger30, 2, 2 + kSectionInvert);
				_gm->reply(kStringArsanoRoger31, 2, 2 + kSectionInvert);
				_gm->say(kStringArsanoRoger32);
				_gm->reply(kStringArsanoRoger33, 2, 2 + kSectionInvert);
				_gm->say(kStringArsanoRoger34);
				_gm->reply(kStringArsanoRoger35, 2, 2 + kSectionInvert);
				break;
			default:
				break;
			}
		}
	} else if (((verb == ACTION_USE) && Object::combine(obj1, obj2, CHESS, ROGER_W)) ||
			   ((verb == ACTION_GIVE) && (obj1._id == CHESS) && (obj2._id == ROGER_W))) {
		_vm->renderImage(11);
		_gm->great(0);
		_gm->say(kStringArsanoRoger36);
		_gm->reply(kStringArsanoRoger37, 2, 2 + kSectionInvert);
		_gm->say(kStringArsanoRoger38);
		_vm->paletteFadeOut();
		_gm->_inventory.remove(*_gm->_rooms[CABIN_R3]->getObject(0)); // Chess board
		g_system->fillScreen(kColorBlack);
		_vm->_screen->setGuiBrightness(255);
		_vm->paletteBrightness();
		_vm->renderMessage(kStringArsanoRoger39);
		_gm->wait(_gm->_messageDuration, true, true);
		_vm->removeMessage();
		_vm->_screen->setGuiBrightness(0);
		_vm->paletteBrightness();
		_gm->_time += ticksToMsec(125000); // 2 hours
		_gm->_state._alarmOn = (_gm->_state._timeAlarm > _gm->_time);
		_gm->_state._eventTime = _gm->_time + ticksToMsec(4000);
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
		_gm->wait(_gm->_messageDuration, true, true);
		_vm->removeMessage();
	} else
		return false;

	return true;
}

ArsanoGlider::ArsanoGlider(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 19;
	_id = GLIDER;
	_shown[0] = kShownTrue;
	_sinus = 0;

	_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,255,255,0,MEETUP,15);
	_objectState[1] = Object(_id, kStringButton,kStringDefaultDescription,GLIDER_BUTTON1,PRESS,0,0,0);
	_objectState[2] = Object(_id, kStringButton,kStringDefaultDescription,GLIDER_BUTTON2,PRESS,1,1,0);
	_objectState[3] = Object(_id, kStringButton,kStringDefaultDescription,GLIDER_BUTTON3,PRESS,2,2,0);
	_objectState[4] = Object(_id, kStringButton,kStringDefaultDescription,GLIDER_BUTTON4,PRESS,3,3,0);
	_objectState[5] = Object(_id, kStringKeycard,kStringDefaultDescription,GLIDER_KEYCARD,TAKE|COMBINABLE,255,255,0);
	_objectState[6] = Object(_id, kStringSlot,kStringDefaultDescription,GLIDER_SLOT,COMBINABLE,4,4,0);
	_objectState[7] = Object(_id, kStringCompartment,kStringDefaultDescription,NULLOBJECT,OPENABLE,5,6,6);
	_objectState[8] = Object(_id, kStringKeyboard,kStringDefaultDescription,GLIDER_BUTTONS,NULLTYPE,7,7,0);
	_objectState[9] = Object(_id, kStringAnnouncement,kStringAnnouncementDescription,GLIDER_DISPLAY,NULLTYPE,8,8,0);
	_objectState[10] = Object(_id, kStringInstruments,kStringAnnouncementDescription,GLIDER_INSTRUMENTS,NULLTYPE,9,9,0);
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
		_gm->wait(7);
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
				break;
			default:
				break;
			}
		}
		_gm->wait(4);
		_vm->renderImage(_gm->invertSection(i));
	} else if ((verb == ACTION_USE) && (obj1._id == GLIDER_BUTTONS))
		_vm->renderMessage(kStringArsanoGlider1);
	else
		return false;

	return true;
}

ArsanoMeetup2::ArsanoMeetup2(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 38;
	_id = MEETUP2;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringRoger, kStringDefaultDescription, ROGER_W, TALK, 255, 255, 0);
	_objectState[1] = Object(_id, kStringSpaceshift, kStringDefaultDescription, SPACESHIP, COMBINABLE, 255, 255, 0);
	_objectState[2] = Object(_id, kStringCave, kStringDefaultDescription, NULLOBJECT, EXIT, 255, 255, 0, CAVE, 22);

	_dialog1[0] = kStringDialogArsanoMeetup2_1;
	_dialog1[1] = kStringDialogArsanoMeetup2_2;
	_dialog2[0] = kStringDialogArsanoMeetup2_3;
	_dialog2[1] = kStringDialogArsanoMeetup2_4;
	_dialog3[0] = kStringDialogArsanoMeetup2_5;
	_dialog3[1] = kStringDialogArsanoMeetup2_6;
	_dialog3[2] = kStringDialogArsanoMeetup2_7;
	_dialog3[3] = kStringDialogArsanoMeetup2_8;
	_dialog4[0] = kStringDialogArsanoMeetup2_9;
	_dialog4[1] = kStringDialogArsanoMeetup2_10;
	_dialog4[2] = kStringDialogArsanoMeetup2_11;
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
		_gm->changeRoom(INTRO1);
		_vm->setCurrentImage(30);
		_vm->renderImage(0);
		_vm->paletteBrightness();
		bool found;
		if (sentenceRemoved(0, 2) || sentenceRemoved(1, 2)) {
			_gm->reply(kStringArsanoMeetup2_3, 1, 1 + kSectionInvert);
			found = !_gm->dialog(2, row4, _dialog4, 0);
			if (!(found))
				_gm->reply(kStringArsanoMeetup2_4, 1, 1 + kSectionInvert);
		} else {
			_gm->reply(kStringArsanoMeetup2_5, 1, 1 + kSectionInvert);
			_gm->reply(kStringArsanoMeetup2_6, 1, 1 + kSectionInvert);
			found = !_gm->dialog(2, row1, _dialog1, 0);
			removeSentence(0, 2);
		}
		if (found) {
			_gm->_inventory.remove(*_gm->_rooms[ROGER]->getObject(3));
			_gm->_inventory.remove(*_gm->_rooms[ROGER]->getObject(7));
			_gm->_inventory.remove(*_gm->_rooms[ROGER]->getObject(8));
			_gm->reply(kStringArsanoMeetup2_7, 1, 1 + kSectionInvert);
			_gm->reply(kStringArsanoMeetup2_8, 1, 1 + kSectionInvert);
			bool flight = _gm->dialog(2, row2, _dialog2, 0);
			if (flight) {
				_gm->reply(kStringArsanoMeetup2_9, 1, 1 + kSectionInvert);
				_gm->dialog(4, row3, _dialog3, 0);
				_gm->reply(kStringArsanoMeetup2_10, 1, 1 + kSectionInvert);
			} else
				_gm->reply(kStringArsanoMeetup2_11, 1, 1 + kSectionInvert);

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
				_gm->wait(36);
				for (int i = 1; i <= 13; i++) {
					if (i > 1)
						_vm->renderImage(_gm->invertSection(i - 1));
					_vm->renderImage(i);
					_gm->wait(2);
				}
				_vm->renderImage(_gm->invertSection(13));
				_gm->wait(20);
				_vm->setCurrentImage(14);
				_vm->renderImage(0);
				_vm->paletteBrightness();
				_gm->wait(36);
				for (int i = 1; i <= 13; i++) {
					if (i > 1)
						_vm->renderImage(_gm->invertSection(i - 1));
					_vm->renderImage(i);
					_gm->wait(2);
				}
				_vm->renderImage(_gm->invertSection(13));
				_gm->wait(9);
				_vm->playSound(kAudioCrash);
				for (int i = 14; i <= 19; i++) {
					_vm->renderImage(i);
					_gm->wait(3);
				}
				_vm->paletteFadeOut();
				_vm->setCurrentImage(11);
				_vm->renderImage(0);
				_vm->paletteFadeIn();
				_gm->wait(18);
				_vm->renderMessage(kStringArsanoMeetup2_12);
				_gm->great(0);
				_gm->wait(_gm->_messageDuration, true, true);
				_vm->removeMessage();
				_vm->paletteFadeOut();
				g_system->fillScreen(kColorBlack);
				_gm->_state._dream = false;
				if (!_vm->loadGame(kSleepAutosaveSlot))
					_vm->errorTempSave(false);
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
	_gm->wait(12);
	for (int i = 2; i <= 11; ++i) {
		if (i >= 9)
			_vm->renderImage(i - 1 + kSectionInvert);
		else
			setSectionVisible(i - 1, false);
		_vm->renderImage(i);
		_gm->wait(2);
	}
	_vm->renderImage(11 + kSectionInvert);
}

ArsanoMeetup3::ArsanoMeetup3(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 39;
	_id = MEETUP3;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringUfo,kStringUfoDescription,UFO,EXIT,0,0,0,NULLROOM,3);
	_objectState[1] = Object(_id, kStringStar,kStringDefaultDescription,STAR,NULLTYPE,1,1,0);
	_objectState[2] = Object(_id, kStringCave,kStringDefaultDescription,NULLOBJECT,EXIT,255,255,0,CAVE,22);

	_dialog2[0] = kStringArsanoDialog1;
	_dialog2[1] = kStringDialogArsanoMeetup3_1;
	_dialog2[2] = kStringDialogArsanoMeetup3_2;
	_dialog2[3] = kStringDialogArsanoMeetup3_3;
	_dialog3[0] = kStringDialogArsanoMeetup3_4;
	_dialog3[1] = kStringDialogArsanoMeetup3_5;
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
	} else if ((verb == ACTION_WALK) && (obj1._id == UFO)) {
		g_system->fillScreen(kColorBlack);
		_vm->setCurrentImage(36);
		_vm->renderImage(0);
		_vm->paletteBrightness();
		_gm->dialog(3, rowsX, _dialogsX, 0);
		_vm->renderImage(1);
		_gm->wait(3);
		_vm->renderImage(2);
		_gm->wait(3);
		_vm->renderImage(3);
		_gm->wait(6);
		_vm->renderImage(4);
		_vm->playSound(kAudioGunShot);

		while (_vm->_sound->isPlaying())
			_gm->wait(1);

		_vm->renderImage(5);
		_gm->wait(3);
		_vm->renderImage(4);
		_vm->playSound(kAudioGunShot);

		while (_vm->_sound->isPlaying())
			_gm->wait(1);

		_vm->renderImage(5);
		_vm->paletteFadeOut();
		_gm->wait(12);
		_vm->setCurrentImage(0);
		_vm->renderImage(0);
		_vm->paletteFadeIn();
		_gm->wait(18);
		_gm->reply(kStringArsanoMeetup3_1, 2, 2 + kSectionInvert);
		_gm->wait(10);
		_gm->reply(kStringArsanoMeetup3_2, 1, 1 + kSectionInvert);

		do {
			int i = _gm->dialog(4, row2, _dialog2, 2);
			switch (i) {
			case 0:
				_gm->reply(kStringArsanoMeetup3_3, 1, 1 + kSectionInvert);
				_gm->reply(kStringArsanoMeetup3_4, 1, 1 + kSectionInvert);
				break;
			case 1:
				_gm->reply(kStringArsanoMeetup3_5, 2, 2 + kSectionInvert);
				_gm->say(kStringArsanoMeetup3_6);
				_gm->reply(kStringArsanoMeetup3_7, 2, 2 + kSectionInvert);
				_gm->reply(kStringArsanoMeetup3_8, 2, 2 + kSectionInvert);
				_gm->reply(kStringArsanoMeetup3_9, 2, 2 + kSectionInvert);
				_gm->reply(kStringArsanoMeetup3_10, 2, 2 + kSectionInvert);
				_gm->reply(kStringArsanoMeetup3_11, 2, 2 + kSectionInvert);
				if (_gm->dialog(2, row3, _dialog3, 0)) {
					_gm->reply(kStringArsanoMeetup3_12, 2, 2 + kSectionInvert);
					_gm->say(kStringArsanoMeetup3_13);
				}
				_gm->reply(kStringArsanoMeetup3_14, 2, 2 + kSectionInvert);
				_gm->reply(kStringArsanoMeetup3_15, 2, 2 + kSectionInvert);
				_gm->reply(kStringArsanoMeetup3_16, 2, 2 + kSectionInvert);
				_gm->reply(kStringArsanoMeetup3_17, 2, 2 + kSectionInvert);
				if (_gm->dialog(2, row3, _dialog3, 0)) {
					_gm->reply(kStringArsanoMeetup3_12, 2, 2 + kSectionInvert);
					_gm->say(kStringArsanoMeetup3_13);
				}
				_gm->reply(kStringArsanoMeetup3_18, 2, 2 + kSectionInvert);
				break;
			case 2:
				_gm->reply(kStringArsanoMeetup3_19, 2, 2 + kSectionInvert);
				_gm->reply(kStringArsanoMeetup3_20, 2, 2 + kSectionInvert);
				break;
			case 3:
				_gm->reply(kStringArsanoMeetup3_21, 1, 1 + kSectionInvert);
				_gm->reply(kStringArsanoMeetup3_22, 1, 1 + kSectionInvert);
				_gm->say(kStringArsanoMeetup3_23);
				_gm->reply(kStringArsanoMeetup3_24, 1, 1 + kSectionInvert);
				_gm->reply(kStringArsanoMeetup3_25, 1, 1 + kSectionInvert);
				break;
			default:
				break;
			}
			removeSentence(2, 2);
		} while (!allSentencesRemoved(4, 2));
		_gm->say(kStringArsanoMeetup3_26);
		_gm->reply(kStringArsanoMeetup3_27, 1, 1 + kSectionInvert);
		_gm->reply(kStringArsanoMeetup3_28, 1, 1 + kSectionInvert);
		_vm->paletteFadeOut();
		// Remove all objects from the inventory except the Knife, Watch and Discman
		bool has_knife = _gm->_rooms[INTRO1]->getObject(1)->hasProperty(CARRIED);
		bool has_watch = _gm->_rooms[INTRO1]->getObject(2)->hasProperty(CARRIED);
		bool has_discman = _gm->_rooms[INTRO1]->getObject(3)->hasProperty(CARRIED);
		_gm->_inventory.clear();
		_gm->_inventoryScroll = 0;
		if (has_knife)
			_gm->_inventory.add(*_gm->_rooms[INTRO1]->getObject(1));
		if (has_watch)
			_gm->_inventory.add(*_gm->_rooms[INTRO1]->getObject(2));
		if (has_discman)
			_gm->_inventory.add(*_gm->_rooms[INTRO1]->getObject(3));
		_gm->changeRoom(CELL);
		_gm->_state._dream = true;
	} else
		return false;

	return true;
}

// Axacuss
AxacussCell::AxacussCell(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 43;
	_id = CELL;
	_shown[0] = kShownTrue;
	_shown[1] = kShownTrue;
	_shown[31] = kShownTrue;

	_objectState[0] = Object(_id, kStringButton,kStringDefaultDescription,CELL_BUTTON,PRESS,1,1,0);
	_objectState[1] = Object(_id, kStringDoor,kStringDefaultDescription,CELL_DOOR,EXIT|OPENABLE|CLOSED,0,0,31+kSectionInvert,CORRIDOR4,1);
	_objectState[2] = Object(_id, kStringTray,kStringTrayDescription,TRAY,UNNECESSARY,255,255,0);
	_objectState[3] = Object(_id, kStringLamp,kStringLampDescription,NULLOBJECT,COMBINABLE,3,3,0);
	_objectState[4] = Object(_id, kStringEyes,kStringEyesDescription,NULLOBJECT,NULLTYPE,4,4,0);
	_objectState[5] = Object(_id, kStringWire,kStringDefaultDescription,CELL_WIRE,COMBINABLE|TAKE,6,6,0);
	_objectState[6] = Object(_id, kStringSocket,kStringSocketDescription,SOCKET,COMBINABLE,5,5,0);
	_objectState[7] = Object(_id, kStringMetalBlock,kStringMetalBlockDescription,MAGNET,TAKE|COMBINABLE,255,255,30);
	_objectState[8] = Object(_id, kStringRobot,kStringRobotDescription,NULLOBJECT,NULLTYPE,255,255,0);
	_objectState[9] = Object(_id, kStringTable,kStringTableDescription,CELL_TABLE,COMBINABLE,2,2,0);
}

void AxacussCell::onEntrance() {
	if (_gm->_state._dream) {
		_vm->renderMessage(kStringAxacussCell_1);
		_gm->_time = ticksToMsec(500000);
		_gm->_state._alarmOn = (_gm->_state._timeAlarm > _gm->_time);
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
		getObject(1)->resetProperty(EXIT | OPENABLE | OPENED | CLOSED);
	} else if ((_gm->_state._timeRobot >= 301) && (_gm->_state._timeRobot <= 320)) {
		_vm->renderImage(_gm->invertSection(329 - _gm->_state._timeRobot));
		_vm->renderImage(328 - _gm->_state._timeRobot);
	} else if (_gm->_state._timeRobot == 321) {
		_vm->renderImage(31);
		setSectionVisible(8, false);
		getObject(0)->_click = 1;
		getObject(1)->resetProperty(EXIT | OPENABLE | CLOSED);
	}

	if (_gm->_state._timeRobot == 599) {
		_vm->renderImage(_gm->invertSection(31));
		_vm->renderImage(8);
		getObject(0)->_click = 255;
		getObject(1)->resetProperty(EXIT | OPENABLE | OPENED | CLOSED);
	} else if ((_gm->_state._timeRobot >= 601) && (_gm->_state._timeRobot <= 620)) {
		_vm->renderImage(_gm->_state._timeRobot - 593 + kSectionInvert);
		_vm->renderImage(_gm->_state._timeRobot - 592);
	} else if (_gm->_state._timeRobot == 621) {
		_vm->renderImage(31);
		setSectionVisible(28, false);
		getObject(0)->_click = 1;
		getObject(1)->resetProperty(EXIT | OPENABLE | CLOSED);
	} else if (_gm->_state._timeRobot == 700)
		_gm->_state._timeRobot = 0;
	else if (_gm->_state._timeRobot == 10002) {
		_vm->renderImage(18 + kSectionInvert);
		_vm->renderImage(29);
		_vm->renderImage(7);
		getObject(2)->_click = 13;
	} else if (_gm->_state._timeRobot == 10003) {
		setSectionVisible(29, false);
		_vm->renderImage(30);
		getObject(8)->_click = 12;
		getObject(7)->_click = 14;
		_vm->playSound(kAudioRobotBreaks);
	} else if (_gm->_state._timeRobot == 10010)
		--_gm->_state._timeRobot;

	if (_gm->_state._timeRobot == 312) {
		_vm->renderImage(7);
		getObject(2)->_click = 13;
	} else if (_gm->_state._timeRobot == 610) {
		setSectionVisible(7, false);
		getObject(2)->_click = 255;
	}

	if ((isSectionVisible(6)) &&
		((_gm->_state._timeRobot == 310) || (_gm->_state._timeRobot == 610))) {
		_vm->playSound(kAudioRobotShock);
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
			   !isSectionVisible(1) && !isSectionVisible(4) && !isSectionVisible(6)) {
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
	} else if ((verb == ACTION_TAKE) && (obj1._id == CELL_WIRE) && !(obj1.hasProperty(CARRIED))) {
		if (isSectionVisible(3)) {
			_vm->renderImage(_gm->invertSection(3));
			_gm->takeObject(obj1);
		} else if (isSectionVisible(5)) {
			_vm->renderImage(_gm->invertSection(5));
			_gm->takeObject(obj1);
		} else
			_vm->renderMessage(kStringAxacussCell_3);
	} else if ((verb == ACTION_WALK) && (obj1._id == CELL_DOOR) && (obj1.hasProperty(OPENED))) {
		if (isSectionVisible(30) || isSectionVisible(29))
			return false;
		_vm->playSound(kAudioGunShot);

		while (_vm->_sound->isPlaying())
			_gm->wait(1);

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

AxacussCorridor1::AxacussCorridor1(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 16;
	_id = CORRIDOR1;
	_shown[0] = kShownTrue;
	_shown[3] = kShownTrue;
	_shown[4] = kShownTrue;
	_shown[5] = kShownTrue;
	_shown[13] = kShownTrue;
	_shown[21] = kShownTrue;
	_shown[23] = kShownTrue;
	_shown[25] = kShownTrue;

	_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,2,2,0,GUARD3,2);
	_objectState[1] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,3,3,0,CORRIDOR2,22);
}

void AxacussCorridor1::onEntrance() {
	_gm->corridorOnEntrance();
}


AxacussCorridor2::AxacussCorridor2(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 16;
	_id = CORRIDOR2;
	_shown[0] = kShownTrue;
	_shown[2] = kShownTrue;
	_shown[3] = kShownTrue;
	_shown[4] = kShownTrue;
	_shown[5] = kShownTrue;
	_shown[17] = kShownTrue;
	_shown[21] = kShownTrue;
	_shown[24] = kShownTrue;

	_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,2,2,0,CORRIDOR1,2);
	_objectState[1] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,3,3,0,CORRIDOR3,22);
	_objectState[2] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,1,1,0,CORRIDOR4,14);
}

void AxacussCorridor2::onEntrance() {
	_gm->corridorOnEntrance();
}

AxacussCorridor3::AxacussCorridor3(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 16;
	_id = CORRIDOR3;
	_shown[0] = kShownTrue;
	_shown[3] = kShownTrue;
	_shown[5] = kShownTrue;
	_shown[19] = kShownTrue;
	_shown[23] = kShownTrue;

	_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,2,2,0,CORRIDOR2,2);
}

void AxacussCorridor3::onEntrance() {
	_gm->corridorOnEntrance();
}

void AxacussCorridor4::onEntrance() {
	_gm->great(4);
	if (_gm->_state._corridorSearch || _gm->_rooms[GUARD]->isSectionVisible(1))
		_gm->busted(0);
}

AxacussCorridor4::AxacussCorridor4(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 16;
	_id = CORRIDOR4;
	_shown[0] = kShownTrue;
	_shown[1] = kShownTrue;
	_shown[2] = kShownTrue;
	_shown[8] = kShownTrue;
	_shown[9] = kShownTrue;
	_shown[11] = kShownTrue;
	_shown[15] = kShownTrue;
	_shown[18] = kShownTrue;
	_shown[20] = kShownTrue;
	_shown[26] = kShownTrue;

	_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,0,0,0,CORRIDOR2,10);
	_objectState[1] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,1,1,0,GUARD,14);
	_objectState[2] = Object(_id, kStringCellDoor,kStringCellDoorDescription,DOOR,EXIT|OPENABLE|OPENED|CLOSED,7,7,0,CELL,16);
	_objectState[3] = Object(_id, kStringLaptop,kStringDefaultDescription,NEWSPAPER,TAKE,6,6,8);
	_objectState[4] = Object(_id, kStringWristwatch,kStringDefaultDescription,WATCH,TAKE|COMBINABLE,255,255,8);
	_objectState[5] = Object(_id, kStringTable,kStringDefaultDescription,TABLE,COMBINABLE,5,5,0);
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
			   ((verb == ACTION_GIVE) && (obj1._id == WATCH) && (obj2._id == TABLE))) {
		if (obj1._id == WATCH)
			_gm->_inventory.remove(obj1);
		else
			_gm->_inventory.remove(obj2);

		_vm->renderImage(29);
		getObject(4)->_click = 8;
	} else if ((verb == ACTION_TAKE) && (obj1._id == WATCH) && !obj1.hasProperty(CARRIED)) {
		setSectionVisible(29, false);
		getObject(4)->_click = 255;
		_gm->takeObject(*_gm->_rooms[INTRO1]->getObject(2));
		if (isSectionVisible(9))
			_vm->renderImage(9);
	} else
		return false;

	return true;
}

AxacussCorridor5::AxacussCorridor5(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 16;
	_id = CORRIDOR5;
	_shown[0] = kShownTrue;
	_shown[3] = kShownTrue;
	_shown[4] = kShownTrue;
	_shown[5] = kShownTrue;
	_shown[12] = kShownTrue;
	_shown[22] = kShownTrue;
	_shown[23] = kShownTrue;
	_shown[24] = kShownTrue;

	_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,DOOR,EXIT,2,2,0,NULLROOM,2);
	_objectState[1] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,3,3,0,CORRIDOR6,22);

	_dialog1[0] = kStringDialogAxacussCorridor5_1;
	_dialog1[1] = kStringDialogAxacussCorridor5_2;
	_dialog2[0] = kStringDialogAxacussCorridor5_3;
	_dialog2[1] = kStringDialogAxacussCorridor5_4;
	_dialog3[0] = kStringDialogAxacussCorridor5_5;
	_dialog3[1] = kStringDialogAxacussCorridor5_6;
	_dialog3[2] = kStringDialogAxacussCorridor5_7;
	_dialog3[3] = kStringDialogAxacussCorridor5_7;

	_rows[0] = 1;
	_rows[1] = 1;
	_rows[2] = 1;
	_rows[3] = 1;
	_rows[4] = 0;
	_rows[5] = 0;
}

void AxacussCorridor5::onEntrance() {
	_gm->corridorOnEntrance();
}

bool AxacussCorridor5::handleMoneyDialog() {
	if (_gm->dialog(2, _rows, _dialog2, 0) == 0) {
		_gm->reply(kStringAxacussCorridor5_5, 1, 1 + kSectionInvert);
		addAllSentences(2);
		if (_gm->_state._money == 0) {
			removeSentence(2, 2);
			removeSentence(3, 2);
		} else {
			Common::String string = _vm->getGameString(kStringDialogAxacussCorridor5_7);
			_vm->setGameString(kStringPlaceholder1, Common::String::format(string.c_str(), _gm->_state._money - 200));
			_vm->setGameString(kStringPlaceholder2, Common::String::format(string.c_str(), _gm->_state._money));
			_dialog3[2] = kStringPlaceholder1;
			_dialog3[3] = kStringPlaceholder2;
		}
		switch (_gm->dialog(4, _rows, _dialog3, 2)) {
		case 1:
			_gm->wait(3);
			_vm->renderImage(1);
			_vm->playSound(kAudioVoiceHalt);
			_vm->renderImage(_gm->invertSection(1));
			_gm->wait(5);
			_vm->renderImage(2);
			_gm->wait(2);
			_gm->shot(3, _gm->invertSection(3));
			break;
		case 2:
			if (_gm->_state._money > 1100) {
				stopInteract(_gm->_state._money - 200);
				return true;
			}
			_gm->reply(kStringAxacussCorridor5_6, 1, 1 + kSectionInvert);
			break;
		case 3:
			if (_gm->_state._money >= 900) {
				stopInteract(_gm->_state._money);
				return true;
			}
			_gm->reply(kStringAxacussCorridor5_6, 1, 1 + kSectionInvert);
			break;
		default:
			break;
		}
	}
	return false;
}

void AxacussCorridor5::stopInteract(int sum) {
	_gm->reply(kStringAxacussCorridor5_7, 1, 1 + kSectionInvert);
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
			_gm->reply(kStringAxacussCorridor5_1, 1, 1 + kSectionInvert);
			if (handleMoneyDialog())
				return true;
		} else {
			_gm->_guiEnabled = true;
			_gm->reply(kStringAxacussCorridor5_2, 1, 1 + kSectionInvert);
			if (_gm->dialog(2, _rows, _dialog1, 0))
				_gm->reply(kStringAxacussCorridor5_3, 1, 1 + kSectionInvert);
			else {
				_gm->reply(kStringAxacussCorridor5_4, 1, 1 + kSectionInvert);
				if (handleMoneyDialog())
					return true;
			}
		}
		g_system->fillScreen(kColorBlack);
		return true;
	}
	return false;
}

AxacussCorridor6::AxacussCorridor6(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 16;
	_id = CORRIDOR6;
	_shown[0] = kShownTrue;
	_shown[3] = kShownTrue;
	_shown[4] = kShownTrue;
	_shown[5] = kShownTrue;
	_shown[6] = kShownTrue;
	_shown[22] = kShownTrue;
	_shown[24] = kShownTrue;
	_shown[25] = kShownTrue;

	_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,2,2,0,CORRIDOR5,2);
	_objectState[1] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,3,3,0,CORRIDOR7,22);
	_objectState[2] = Object(_id, kStringDoor,kStringDefaultDescription,DOOR,OPENABLE|CLOSED,255,255,0,CORRIDOR8,13);
}

void AxacussCorridor6::onEntrance() {
	_gm->corridorOnEntrance();
}

bool AxacussCorridor6::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) &&
			(obj1.hasProperty(OPENED))) {
		_vm->renderImage(6);
		setSectionVisible(7, false);
		obj1.resetProperty(EXIT | OPENABLE | CLOSED);
		_gm->_rooms[CORRIDOR8]->setSectionVisible(27, false);
		_gm->_rooms[CORRIDOR8]->setSectionVisible(28, true);
		_gm->_rooms[CORRIDOR8]->getObject(0)->disableProperty(OPENED);
		_vm->playSound(kAudioDoorClose);
	} else
		return false;

	return true;
}

AxacussCorridor7::AxacussCorridor7(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 16;
	_id = CORRIDOR7;
	_shown[0] = kShownTrue;
	_shown[3] = kShownTrue;
	_shown[4] = kShownTrue;
	_shown[5] = kShownTrue;
	_shown[10] = kShownTrue;
	_shown[21] = kShownTrue;
	_shown[24] = kShownTrue;
	_shown[25] = kShownTrue;

	_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,2,2,0,CORRIDOR6,2);
	_objectState[1] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,3,3,0,GUARD,22);
}

void AxacussCorridor7::onEntrance() {
	_gm->corridorOnEntrance();
}

AxacussCorridor8::AxacussCorridor8(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 16;
	_id = CORRIDOR8;
	_shown[0] = kShownTrue;
	_shown[1] = kShownTrue;
	_shown[4] = kShownTrue;
	_shown[15] = kShownTrue;
	_shown[20] = kShownTrue;
	_shown[22] = kShownTrue;
	_shown[28] = kShownTrue;

	_objectState[0] = Object(_id, kStringDoor,kStringDefaultDescription,DOOR,EXIT|OPENABLE,0,0,0,CORRIDOR6,10);
	_objectState[1] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,3,3,0,BCORRIDOR,22);
}

void AxacussCorridor8::onEntrance() {
	_gm->corridorOnEntrance();
}

bool AxacussCorridor8::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_OPEN) && (obj1._id == DOOR) && !obj1.hasProperty(OPENED)) {
		_vm->renderImage(27);
		setSectionVisible(28, false);
		obj1.setProperty(OPENED);
		_gm->_rooms[CORRIDOR6]->setSectionVisible(6, false);
		_gm->_rooms[CORRIDOR6]->setSectionVisible(7, true);
		_gm->_rooms[CORRIDOR6]->getObject(2)->resetProperty(EXIT | OPENED | OPENABLE);
		_gm->_rooms[CORRIDOR6]->getObject(2)->_click = 4;
		_vm->playSound(kAudioDoorOpen);
	} else if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) && (obj1._type & OPENED)) {
		_vm->renderImage(28);
		setSectionVisible(27, false);
		obj1.disableProperty(OPENED);
		_gm->_rooms[CORRIDOR6]->setSectionVisible(6, true);
		_gm->_rooms[CORRIDOR6]->setSectionVisible(7, false);
		_gm->_rooms[CORRIDOR6]->getObject(2)->resetProperty(EXIT | CLOSED | OPENABLE);
		_vm->playSound(kAudioDoorClose);
	} else
		return false;

	return true;
}

AxacussCorridor9::AxacussCorridor9(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 16;
	_id = CORRIDOR9;
	_shown[0] = kShownTrue;
	_shown[1] = kShownTrue;
	_shown[3] = kShownTrue;
	_shown[14] = kShownTrue;
	_shown[19] = kShownTrue;
	_shown[23] = kShownTrue;
	_shown[28] = kShownTrue;

	_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,2,2,0,BCORRIDOR,2);
	_objectState[1] = Object(_id, kStringDoor,kStringDefaultDescription,DOOR,EXIT|OPENABLE,0,0,0,GUARD,10);
}

void AxacussCorridor9::onEntrance() {
	_gm->corridorOnEntrance();
}

bool AxacussCorridor9::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) && (obj1.hasProperty(OPENED))) {
		_vm->renderImage(28);
		setSectionVisible(27, false);
		obj1.disableProperty(OPENED);
		_gm->_rooms[GUARD]->setSectionVisible(6, false);
		_gm->_rooms[GUARD]->getObject(2)->disableProperty(OPENED);
		_vm->playSound(kAudioDoorClose);
	} else if ((verb == ACTION_OPEN) && (obj1._id == DOOR) && !obj1.hasProperty(OPENED)) {
		_vm->renderImage(27);
		setSectionVisible(28, false);
		obj1.setProperty(OPENED);
		_gm->_rooms[GUARD]->setSectionVisible(6, true);
		_gm->_rooms[GUARD]->getObject(2)->setProperty(OPENED);
		_vm->playSound(kAudioDoorOpen);
		if (!_gm->_rooms[GUARD]->isSectionVisible(1))
			_gm->busted(0);
	} else
		return false;

	return true;
}

AxacussBcorridor::AxacussBcorridor(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = BCORRIDOR;
	_shown[0] = kShownTrue;
	_shown[3] = kShownTrue;

	_objectState[0] = Object(_id, kStringPillar,kStringDefaultDescription,PILLAR1,NULLTYPE,4,4,0);
	_objectState[1] = Object(_id, kStringPillar,kStringDefaultDescription,PILLAR2,NULLTYPE,5,5,0);
	_objectState[2] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,6,6,0,CORRIDOR8,2);
	_objectState[3] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,7,7,0,CORRIDOR9,22);
	_objectState[4] = Object(_id, kStringDoor,kStringDoorDescription1,DOOR1,EXIT|OPENABLE|CLOSED|OCCUPIED,0,0,1,OFFICE_L1,6);
	_objectState[5] = Object(_id, kStringDoor,kStringDoorDescription2,DOOR2,EXIT|OPENABLE|CLOSED|OCCUPIED,1,1,2,OFFICE_L2,16);
	_objectState[6] = Object(_id, kStringDoor,kStringDoorDescription3,DOOR3,EXIT|OPENABLE|OPENED,2,2,3,OFFICE_R1,8);
	_objectState[7] = Object(_id, kStringDoor,kStringDoorDescription4,DOOR4,EXIT|OPENABLE|CLOSED|OCCUPIED,3,3,4,OFFICE_R2,18);
}

void AxacussBcorridor::onEntrance() {
	if (_gm->_state._corridorSearch)
		_gm->busted(0);
	else if (isSectionVisible(7))
		_gm->busted(-1);
}

bool AxacussBcorridor::interact(Action verb, Object &obj1, Object &obj2) {
	if (obj1.hasProperty(EXIT) ||
		((verb == ACTION_USE) && obj1.hasProperty(COMBINABLE) && obj2.hasProperty(EXIT))) {
		_gm->_state._playerHidden = false;
	}

	if ((verb == ACTION_CLOSE) && (obj1._id >= DOOR1) && (obj1._id <= DOOR4) && obj1.hasProperty(OPENED)) {
		_vm->renderImage(_gm->invertSection(obj1._id - DOOR1 + 1));
		_vm->playSound(kAudioDoorClose);
		obj1.disableProperty(OPENED);
		obj1.setProperty(CLOSED);
		if (obj1.hasProperty(OCCUPIED)) {
			_gm->_state._destination = 255;
			obj1.disableProperty(OCCUPIED);
			obj1.setProperty(CAUGHT);
			if (!_gm->_rooms[OFFICE_L1 + obj1._id - DOOR1]->isSectionVisible(4))
				_gm->search(180);
			else
				_gm->_state._eventTime = kMaxTimerValue;
		}
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
			if (getObject(4)->hasProperty(CAUGHT)) {
				_gm->busted(11);
				return true;
			}
			getObject(4)->resetProperty(EXIT | OPENABLE | OPENED);
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR2) && !getObject(5)->hasProperty(OPENED)) {
		if (getObject(5)->hasProperty(OCCUPIED))
			_vm->renderMessage(kStringDontEnter);
		else {
			_vm->renderImage(2);
			_vm->playSound(kAudioDoorOpen);
			if (getObject(5)->hasProperty(CAUGHT)) {
				_gm->busted(16);
				return true;
			}
			getObject(5)->resetProperty(EXIT | OPENABLE | OPENED);
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR3) && !getObject(6)->hasProperty(OPENED)) {
		if (getObject(6)->hasProperty(OCCUPIED))
			_vm->renderMessage(kStringDontEnter);
		else {
			_vm->renderImage(3);
			_vm->playSound(kAudioDoorOpen);
			if (getObject(6)->hasProperty(CAUGHT)) {
				_gm->busted(15);
				return true;
			}
			getObject(6)->resetProperty(EXIT | OPENABLE | OPENED);
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR4) && !getObject(7)->hasProperty(OPENED)) {
		if (getObject(7)->hasProperty(OCCUPIED)) {
			_vm->renderMessage(kStringDontEnter);
		} else {
			_vm->renderImage(4);
			_vm->playSound(kAudioDoorOpen);
			if (getObject(7)->hasProperty(CAUGHT)) {
				_gm->busted(20);
				return true;
			}
			getObject(7)->resetProperty(EXIT | OPENABLE | OPENED);
		}
	} else if ((verb == ACTION_LOOK) && (obj1._id >= DOOR1) && (obj1._id <= DOOR4)) {
		_gm->_state._nameSeen[obj1._id - DOOR1] = true;
		return false;
	} else if ((verb == ACTION_WALK) && ((obj1._id == PILLAR1) || (obj1._id == PILLAR2))) {
		_vm->renderMessage(kStringAxacussBcorridor_1);
		_gm->_state._playerHidden = true;
	} else
		return false;

	return true;
}

AxacussIntersection::AxacussIntersection(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 40;
	_id = GUARD;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringExit, kStringDefaultDescription, NULLOBJECT, EXIT, 255, 255, 0, CORRIDOR4, 21);
	_objectState[1] = Object(_id, kStringCorridor, kStringDefaultDescription, NULLOBJECT, EXIT, 3, 3, 0, CORRIDOR7, 5);
	_objectState[2] = Object(_id, kStringDoor, kStringDefaultDescription, DOOR, EXIT | OPENABLE, 1, 1, 6, CORRIDOR9, 3);
	_objectState[3] = Object(_id, kStringAxacussan, kStringAxacussanDescription, GUARDIAN, TALK, 0, 0, 0);
	_objectState[4] = Object(_id, kStringImage, kStringImageDescription2, NULLOBJECT, NULLTYPE, 2, 2, 0);
	_objectState[5] = Object(_id, kStringMastercard, kStringMastercardDescription, MASTERKEYCARD, TAKE | COMBINABLE, 255, 255, 1);
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

AxacussExit::AxacussExit(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 42;
	_id = GUARD3;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,255,255,0,CORRIDOR1,22);
	_objectState[1] = Object(_id, kStringDoor,kStringDefaultDescription,NULLOBJECT,EXIT|OPENABLE|CLOSED,0,0,0,NULLROOM,20);
	_objectState[2] = Object(_id, kStringDoor,kStringDefaultDescription,NULLOBJECT,EXIT|OPENABLE|CLOSED,1,1,0,NULLROOM,15);
	_objectState[3] = Object(_id, kStringDoor,kStringDefaultDescription,DOOR,EXIT|OPENABLE,2,2,11,OFFICE_L,0);
	_objectState[4] = Object(_id, kStringLamp2,kStringDefaultDescription,LAMP,COMBINABLE,3,3,0);
	_objectState[5] = Object(_id, kStringAxacussan,kStringDefaultDescription,GUARDIAN,TALK,5,5,0);
	_objectState[6] = Object(_id, kStringImage,kStringGenericDescription5,NULLOBJECT,NULLTYPE,4,4,0);
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
				_vm->playSound(kAudioSmash); // 046/4020
			_gm->wait(1);
			_vm->renderImage(i + kSectionInvert);
		}
		_gm->_state._powerOff = true;
		_objectState[5]._click = 255;

		_gm->search(450);
		_gm->roomBrightness();
	} else if ((verb == ACTION_USE) && (Object::combine(obj1,obj2,MAGNET,GUARDIAN) || Object::combine(obj1,obj2,KNIFE,GUARDIAN)))
		_vm->renderMessage(kStringArsanoEntrance27);
	else
		return false;

	return true;
}

AxacussOffice1::AxacussOffice1(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 7;
	_id = OFFICE_L1;
	_shown[0] = kShownTrue;
	_shown[2] = kShownTrue;
	_shown[7] = kShownTrue;
	_shown[9] = kShownTrue;
	_shown[16] = kShownTrue;

	_objectState[0] = Object(_id, kStringDoor,kStringDefaultDescription,DOOR,EXIT|OPENABLE|OPENED,6,6,9,BCORRIDOR,9);
	_objectState[1] = Object(_id, kStringComputer,kStringDefaultDescription,COMPUTER,COMBINABLE,4,4,0);
	_objectState[2] = Object(_id, kStringMoney,kStringMoneyDescription1,MONEY,TAKE,255,255,0);
	_objectState[3] = Object(_id, kStringLocker,kStringLockerDescription,LOCKER,OPENABLE|CLOSED,5,5,0);
	_objectState[4] = Object(_id, kStringLetter,kStringDefaultDescription,LETTER,UNNECESSARY,3,3,0);
}

bool AxacussOffice1::interact(Action verb, Object &obj1, Object &obj2) {
	Common::String input;
	if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) &&
			obj1.hasProperty(OPENED)) {
		_vm->renderImage(_gm->invertSection(9));
		obj1.disableProperty(OPENED);
		obj1.setProperty(CLOSED);
		_vm->playSound(kAudioDoorClose);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR) &&
			   !getObject(0)->hasProperty(OPENED)) {
		_vm->renderImage(9);
		getObject(0)->disableProperty(CLOSED);
		getObject(0)->setProperty(OPENED);
		_vm->playSound(kAudioDoorOpen);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, COMPUTER, MAGNET)) {
		_vm->renderImage(4);
		setSectionVisible(16, false);
		_vm->playSound(kAudioSmash);
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
				obj1.resetProperty(OPENABLE | OPENED);
				if (getObject(2)->hasProperty(TAKE)) {
					_vm->renderImage(8);
					getObject(2)->_click = 9;
				}
				_vm->playSound(kAudioDoorOpen);
				_gm->great(7);
			}
		}
	} else if ((verb == ACTION_CLOSE) && (obj1._id == LOCKER) && obj1.hasProperty(OPENED)) {
		_vm->renderImage(7);
		setSectionVisible(6, false);
		obj1.resetProperty(OPENABLE | CLOSED);
		setSectionVisible(8, false);
		getObject(2)->_click = 255;
		_vm->playSound(kAudioDoorClose);
	} else if ((verb == ACTION_TAKE) && (obj1._id == MONEY)) {
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

AxacussOffice2::AxacussOffice2(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 7;
	_id = OFFICE_L2;
	_shown[0] = kShownTrue;
	_shown[1] = kShownTrue;
	_shown[9] = kShownTrue;
	_shown[16] = kShownTrue;

	_objectState[0] = Object(_id, kStringDoor,kStringDefaultDescription,DOOR,EXIT|OPENABLE|OPENED,6,6,9,BCORRIDOR,9);
	_objectState[1] = Object(_id, kStringComputer,kStringDefaultDescription,COMPUTER,COMBINABLE,4,4,0);
	_objectState[2] = Object(_id, kStringCube,kStringGenericDescription6,NULLOBJECT,NULLTYPE,0,0,0);
	_objectState[3] = Object(_id, kStringImage,kStringGenericDescription7,NULLOBJECT,NULLTYPE,1,1,0);
	_objectState[4] = Object(_id, kStringStrangeThing,kStringGenericDescription8,NULLOBJECT,UNNECESSARY,2,2,0);
}

bool AxacussOffice2::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) &&
			obj1.hasProperty(OPENED)) {
		_vm->renderImage(_gm->invertSection(9));
		obj1.disableProperty(OPENED);
		obj1.setProperty(CLOSED);
		_vm->playSound(kAudioDoorClose);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR) && !getObject(0)->hasProperty(OPENED)) {
		_vm->renderImage(9);
		getObject(0)->disableProperty(CLOSED);
		getObject(0)->setProperty(OPENED);
		_vm->playSound(kAudioDoorOpen);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, COMPUTER, MAGNET)) {
		_vm->renderImage(4);
		setSectionVisible(16, false);
		_vm->playSound(kAudioSmash);
	} else if ((verb == ACTION_LOOK) && (obj1._id == COMPUTER)) {
		if (isSectionVisible(4))
			_vm->renderMessage(kStringBroken);
		else
			_gm->telomat(1);
	} else
		return false;

	return true;
}

AxacussOffice3::AxacussOffice3(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 8;
	_id = OFFICE_R1;
	_shown[0] = kShownTrue;
	_shown[1] = kShownTrue;
	_shown[3] = kShownTrue;

	_objectState[0] = Object(_id, kStringDoor,kStringDefaultDescription,DOOR,EXIT|OPENABLE|OPENED,0,0,3,BCORRIDOR,5);
	_objectState[1] = Object(_id, kStringComputer,kStringDefaultDescription,COMPUTER,COMBINABLE,4,4,0);
	_objectState[2] = Object(_id, kStringImage,kStringImageDescription2,NULLOBJECT,UNNECESSARY,1,1,0);
	_objectState[3] = Object(_id, kStringImage,kStringImageDescription2,PAINTING,UNNECESSARY,2,2,0);
	_objectState[4] = Object(_id, kStringPlant,kStringDefaultDescription,NULLOBJECT,UNNECESSARY,3,3,0);
	_objectState[5] = Object(_id, kNoString,kStringDefaultDescription,MONEY,TAKE|COMBINABLE,255,255,0);
}

bool AxacussOffice3::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) &&
			obj1.hasProperty(OPENED)) {
		_vm->renderImage(_gm->invertSection(3));
		obj1.disableProperty(OPENED);
		obj1.setProperty(CLOSED);
		_vm->playSound(kAudioDoorClose);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR) &&
			   !getObject(0)->hasProperty(OPENED)) {
		_vm->renderImage(3);
		getObject(0)->disableProperty(CLOSED);
		getObject(0)->setProperty(OPENED);
		_vm->playSound(kAudioDoorOpen);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, COMPUTER, MAGNET)) {
		_vm->renderImage(4);
		_vm->playSound(kAudioSmash);
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

AxacussOffice4::AxacussOffice4(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 8;
	_id = OFFICE_R2;
	_shown[0] = kShownTrue;
	_shown[2] = kShownTrue;
	_shown[3] = kShownTrue;

	_objectState[0] = Object(_id, kStringDoor,kStringDefaultDescription,DOOR,EXIT|OPENABLE|OPENED,0,0,3,BCORRIDOR,5);
	_objectState[1] = Object(_id, kStringComputer,kStringDefaultDescription,COMPUTER,COMBINABLE,4,4,0);
	_objectState[2] = Object(_id, kStringStatue,kStringStatueDescription,NULLOBJECT,UNNECESSARY,6,6,0);
	_objectState[3] = Object(_id, kStringPlant,kStringPlantDescription,NULLOBJECT,UNNECESSARY,5,5,0);
}

bool AxacussOffice4::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_CLOSE) && (obj1._id == DOOR) && obj1.hasProperty(OPENED)) {
		_vm->renderImage(_gm->invertSection(3));
		obj1.disableProperty(OPENED);
		obj1.setProperty(CLOSED);
		_vm->playSound(kAudioDoorClose);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, MASTERKEYCARD, DOOR) &&
			   !getObject(0)->hasProperty(OPENED)) {
		_vm->renderImage(3);
		getObject(0)->disableProperty(CLOSED);
		getObject(0)->setProperty(OPENED);
		_vm->playSound(kAudioDoorOpen);
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, COMPUTER, MAGNET)) {
		_vm->renderImage(4);
		_vm->playSound(kAudioSmash);
	} else if ((verb == ACTION_LOOK) && (obj1._id == COMPUTER)) {
		if (isSectionVisible(4))
			_vm->renderMessage(kStringBroken);
		else
			_gm->telomat(3);
	} else
		return false;

	return true;
}

AxacussOffice5::AxacussOffice5(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 7;
	_id = OFFICE_L;
	_shown[0] = kShownTrue;
	_shown[3] = kShownTrue;
	_shown[5] = kShownTrue;
	_shown[17] = kShownTrue;

	_objectState[0] = Object(_id, kStringDoor,kStringDefaultDescription,DOOR,EXIT|OPENABLE|OPENED,6,6,17,GUARD3,9);
	_objectState[1] = Object(_id, kStringComputer,kStringComputerDescription,COMPUTER,COMBINABLE,4,4,0);
	_objectState[2] = Object(_id, kStringGraffiti,kStringGraffitiDescription,NULLOBJECT,NULLTYPE,7,7,0);
	_objectState[3] = Object(_id, kStringMoney,kStringMoneyDescription2,MONEY,TAKE,8,8,0);
}

void AxacussOffice5::onEntrance() {
	_gm->great(5);
}

bool AxacussOffice5::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_USE) && Object::combine(obj1, obj2, COMPUTER, MAGNET)) {
		_vm->renderImage(4);
		_vm->playSound(kAudioSmash);
	} else if ((verb == ACTION_TAKE) && (obj1._id == MONEY)) {
		_vm->renderImage(_gm->invertSection(5));
		obj1._click = 255;
		_gm->takeMoney(350);
	} else
		return false;

	return true;
}

AxacussElevator::AxacussElevator(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 3;
	_id = ELEVATOR;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringButton,kStringDefaultDescription,BUTTON1,PRESS,0,0,0);
	_objectState[1] = Object(_id, kStringButton,kStringDefaultDescription,BUTTON2,PRESS,1,1,0);
	_objectState[2] = Object(_id, kStringExit,kStringDefaultDescription,DOOR,EXIT,255,255,0,NULLROOM,22);
	_objectState[3] = Object(_id, kStringJungle,kStringJungleDescription,JUNGLE,NULLTYPE,255,255,0,STATION,2);
}

bool AxacussElevator::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_WALK) && (obj1._id == DOOR)) {
		g_system->fillScreen(kColorBlack);
		_vm->setCurrentImage(41);
		_vm->renderImage(0);
		_vm->paletteBrightness();
		_gm->reply(kStringAxacussElevator_1, 1, 1 + kSectionInvert);
		_gm->say(kStringAxacussElevator_2);
		g_system->fillScreen(kColorBlack);
	} else if ((verb == ACTION_PRESS) && (obj1._id == BUTTON1)) {
		if (!isSectionVisible(3)) {
			_vm->renderImage(1);
			getObject(2)->resetProperty();
			_vm->playSound(kAudioSlideDoor);
			_gm->wait(25);
			for (int i = 3; i <= 7; i++) {
				_gm->wait(2);
				_vm->renderImage(i);
			}
			getObject(3)->resetProperty(EXIT);
			getObject(3)->_click = 2;
			_vm->renderImage(_gm->invertSection(1));
			if (!(_gm->_state._greatFlag & 0x4000)) {
				_vm->playSound(kAudioFoundLocation);
				_gm->_state._greatFlag |= 0x4000;
			}
		}
	} else if ((verb == ACTION_PRESS) && (obj1._id == BUTTON2)) {
		if (isSectionVisible(3)) {
			_vm->renderImage(2);
			getObject(3)->resetProperty();
			getObject(3)->_click = 255;
			_vm->playSound(kAudioSlideDoor);
			for (int i = 7; i >= 3; i--) {
				_gm->wait(2);
				_vm->renderImage(_gm->invertSection(i));
			}
			_gm->wait(25);
			_vm->playSound(kAudioSlideDoor);
			getObject(2)->resetProperty(EXIT);
			_vm->renderImage(_gm->invertSection(2));
		}
	} else if ((verb == ACTION_WALK) && (obj1._id == JUNGLE)) {
		_vm->paletteFadeOut();
		g_system->fillScreen(kColorBlack);
		_vm->_screen->setGuiBrightness(255);
		_vm->paletteBrightness();
		_vm->renderMessage(kStringAxacussElevator_3);
		_gm->wait(_gm->_messageDuration, true, true);
		_vm->removeMessage();
		_vm->_screen->setGuiBrightness(0);
		_vm->paletteBrightness();
		_gm->_time += ticksToMsec(125000); // 2 hours
		_gm->_state._alarmOn = (_gm->_state._timeAlarm > _gm->_time);
		return false;
	} else
		return false;

	return true;
}

AxacussStation::AxacussStation(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 5;
	_id = STATION;
	_shown[0] = kShownTrue;
	_objectState[0] = Object(_id, kStringSign,kStringDefaultDescription,STATION_SIGN,NULLTYPE,0,0,0);
	_objectState[1] = Object(_id, kStringDoor,kStringDefaultDescription,DOOR,EXIT|OPENABLE|CLOSED,1,1,0,NULLROOM,7);
}

bool AxacussStation::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_LOOK) && (obj1._id == STATION_SIGN)) {
		_gm->changeRoom(SIGN_ROOM);
	} else if ((verb == ACTION_WALK) && (obj1._id == DOOR) && obj1.hasProperty(OPENED)) {
		_gm->great(0);
		_gm->_guiEnabled = false;
		_vm->paletteFadeOut();
		_vm->_system->fillScreen(kColorBlack);
		_gm->changeRoom(OUTRO);
	} else
		return false;

	return true;
}

AxacussSign::AxacussSign(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 32;
	_id = SIGN_ROOM;
	_shown[0] = kShownTrue;
	_shown[1] = kShownTrue;

	_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,255,255,0,STATION,22);
	_objectState[1] = Object(_id, kStringSlot,kStringDefaultDescription,STATION_SLOT,COMBINABLE,0,0,0);
}

bool AxacussSign::interact(Action verb, Object &obj1, Object &obj2) {
	if ((verb == ACTION_USE) && Object::combine(obj1, obj2, STATION_SLOT, MONEY) &&
		isSectionVisible(1)) {
		_gm->takeMoney(-180);
		_vm->renderImage(2);
		setSectionVisible(1, false);
		_gm->_state._eventTime = _gm->_time + ticksToMsec(600);
		_gm->_state._eventCallback = kTaxiFn;
		return true;
	}
	return false;
}


Outro::Outro(SupernovaEngine *vm, GameManager1 *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = -1;
	_id = OUTRO;
	_shown[0] = kShownFalse;

	_outroText =
		_vm->getGameString(kStringOutro1) + '\1' +
		_vm->getGameString(kStringOutro2) + '\1' +
		_vm->getGameString(kStringOutro3) + '\1' +
		_vm->getGameString(kStringOutro4) + '\1' +
		_vm->getGameString(kStringOutro5) + '\1' +
		_vm->getGameString(kStringOutro6) + '\1' +
		_vm->getGameString(kStringOutro7) + '\1' +
		_vm->getGameString(kStringOutro8) + '\1' +
		_vm->getGameString(kStringOutro9) + '\1' +
		_vm->getGameString(kStringOutro10) + '\1' +
		_vm->getGameString(kStringOutro11) + '\1' +
		_vm->getGameString(kStringOutro12) + '\1' +
		_vm->getGameString(kStringOutro13) + '\1' +
		_vm->getGameString(kStringOutro14) + '\1';
}

void Outro::onEntrance() {
	_vm->setCurrentImage(35);
	_vm->renderImage(0);
	_vm->renderImage(1);
	_vm->paletteFadeIn();
	_gm->wait(10);
	for (int i = 8; i <= 21; i++) {
		_vm->renderImage(i);
		_gm->wait(2);
		_vm->renderImage(_gm->invertSection(i));
	}
	_gm->wait(18);
	_vm->renderImage(_gm->invertSection(1));
	for (int i = 2; i <= 7; i++) {
		_vm->renderImage(i);
		_gm->wait(3);
		_vm->renderImage(_gm->invertSection(i));
	}

	_vm->playSound(kMusicOutro);
	Marquee marquee(_vm->_screen, Marquee::kMarqueeOutro, _outroText.c_str());
	while (!_vm->shouldQuit()) {
		_gm->updateEvents();
		marquee.renderCharacter();
		if (_gm->_mouseClicked || _gm->_keyPressed)
			break;
		g_system->updateScreen();
		g_system->delayMillis(_vm->_delay);
	}
	_vm->paletteFadeOut();
	_vm->setCurrentImage(55);
	_vm->renderImage(0);
	_vm->paletteFadeIn();
	_gm->getInput();
	_vm->paletteFadeOut();
	_vm->_screen->setViewportBrightness(1);

	Common::Event event;
	event.type = Common::EVENT_RETURN_TO_LAUNCHER;
	_vm->getEventManager()->pushEvent(event);
}

void Outro::animation() {
}

void Outro::animate(int filenumber, int section1, int section2, int duration) {
	_vm->setCurrentImage(filenumber);
	while (duration) {
		_vm->renderImage(section1);
		_gm->wait(2);
		_vm->renderImage(section2);
		_gm->wait(2);
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
		_gm->wait(2);
		if (section2)
			_vm->renderImage(section2);
		_gm->wait(2);
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
		_gm->wait(2);
		_vm->renderImage(section2);
		_vm->renderImage(section4);
		_gm->wait(2);
		duration--;
	}
	_vm->removeMessage();
}

}
