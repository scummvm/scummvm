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

#include "supernova2/screen.h"
#include "supernova2/supernova2.h"
#include "supernova2/state.h"
#include "supernova2/rooms.h"

namespace Supernova2 {

Room::Room() {
	_seen = false;
	_fileNumber = 0;
	_id = NULLROOM;
	_vm = nullptr;
	_gm = nullptr;

	for (int i = 0; i < kMaxSection; ++i)
		_shown[i] = kShownFalse;
	for (int i = 0; i < kMaxDialog; ++i)
		_sentenceRemoved[i] = 0;
}

Room::~Room() {
}

bool Room::serialize(Common::WriteStream *out) {
	if (out->err())
		return false;

	out->writeSint32LE(_id);
	for (int i = 0; i < kMaxSection; ++i)
		out->writeByte(_shown[i]);
	for (int i = 0; i < kMaxDialog ; ++i)
		out->writeByte(_sentenceRemoved[i]);

	int numObjects = 0;
	while ((numObjects < kMaxObject) && (_objectState[numObjects]._id != INVALIDOBJECT))
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
		_objectState[i]._name = static_cast<StringId>(in->readSint32LE());
		_objectState[i]._description = static_cast<StringId>(in->readSint32LE());
		_objectState[i]._roomId = in->readByte();
		_objectState[i]._id = static_cast<ObjectId>(in->readSint32LE());
		_objectState[i]._type = static_cast<ObjectType>(in->readSint32LE());
		_objectState[i]._click = in->readByte();
		_objectState[i]._click2 = in->readByte();
		_objectState[i]._section = in->readByte();
		_objectState[i]._exitRoom = static_cast<RoomId>(in->readSint32LE());
		_objectState[i]._direction = in->readByte();
	}

	_seen = in->readByte();

	return !in->err();
}

bool Room::hasSeen() {
	return _seen;
}
void Room::setRoomSeen(bool seen) {
	_seen = seen;
}

int Room::getFileNumber() const {
	return _fileNumber;
}
RoomId Room::getId() const {
	return _id;
}

void Room::setSectionVisible(uint section, bool visible) {
	if (section < kMaxSection)
		_shown[section] = visible ? kShownTrue : kShownFalse;
	else
		_shown[section - 128] = visible ? kShownFalse : kShownTrue;
}

bool Room::isSectionVisible(uint index) const {
	return _shown[index] == kShownTrue;
}

void Room::removeSentenceByMask(int mask, int number) {
	if (number > 0) {
		_sentenceRemoved[number - 1] |= mask;
	}
}

void Room::removeSentence(int sentence, int number) {
	if (number > 0)
		_sentenceRemoved[number - 1] |= (1 << sentence);
}

void Room::addSentence(int sentence, int number) {
	if (number > 0)
		_sentenceRemoved[number - 1] &= ~(1 << sentence);
}

void Room::addAllSentences(int number) {
	if (number > 0)
		_sentenceRemoved[number - 1] = 0;
}

bool Room::sentenceRemoved(int sentence, int number) {
	if (number <= 0)
		return false;
	return (_sentenceRemoved[number - 1] & (1 << sentence));
}

bool Room::allSentencesRemoved(int maxSentence, int number) {
	if (number <= 0)
		return false;
	for (int i = 0, flag = 1 ; i < maxSentence ; ++i, flag <<= 1)
		if (!(_sentenceRemoved[number - 1] & flag))
			return false;
	return true;
}

Object *Room::getObject(uint index) {
	return &_objectState[index];
}

void Room::animation() {
}

void Room::onEntrance() {
}

bool Room::interact(Action verb, Object &obj1, Object &obj2) {
	return false;
}

Intro::Intro(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;
	_restTime = 0;

	_fileNumber = -1;
	_id = INTRO;

	_objectState[0] = Object(_id, kStringKnife, kStringKnifeDescription, KNIFE, TAKE | CARRIED | COMBINABLE, 255, 255, 0);
	_objectState[1] = Object(_id, kStringMoney, kStringDefaultDescription, MONEY, TAKE | CARRIED | COMBINABLE, 255, 255, 0);
	_objectState[2] = Object(_id, kStringDiscman, kStringDiscmanDescription, DISCMAN, TAKE | CARRIED | COMBINABLE, 255, 255, 0);
	_objectState[3] = Object(_id, kStringSuctionCup, kStringSuctionCupDescription, SUCTION_CUP, TAKE | COMBINABLE, 255, 255, 0);
	_objectState[4] = Object(_id, kStringDefaultDescription, kStringDefaultDescription, NULLOBJECT, NULLTYPE, 0, 0, 0);
	_objectState[5] = Object(_id, kStringSpecialCard, kStringSpecialCardDescription, SP_KEYCARD, TAKE | COMBINABLE, 255, 255, 0);
	_objectState[6] = Object(_id, kStringAlarmCracker, kStringAlarmCrackerDescription, ALARM_CRACKER, TAKE | COMBINABLE, 255, 255, 0);
	_objectState[7] = Object(_id, kStringDinosaurHead, kStringDefaultDescription, NULLOBJECT, TAKE, 255, 255, 0);
	_objectState[8] = Object(_id, kStringKeycard, kStringDefaultDescription, MUSCARD, TAKE, 255, 255, 0);

	_introText = 
		_vm->getGameString(kStringIntro1) + '\0' + 
		_vm->getGameString(kStringIntro2) + '\0' + 
		_vm->getGameString(kStringIntro3) + '\0' + 
		_vm->getGameString(kStringIntro4) + '\0' + 
		_vm->getGameString(kStringIntro5) + '\0' + 
		"^Matthias Neef#" + '\0' +
		"^Sascha Otterbach#" + '\0' +
		"^Thomas Mazzoni#" + '\0' +
		"^Matthias Klein#" + '\0' +
		"^Gerrit Rothmaier#" + '\0' +
		"^Thomas Hassler#" + '\0' +
		"^Rene Kach#" + '\0' +
		'\233' + '\0';
}

void Intro::onEntrance() {
	_gm->_guiEnabled = false;
	_vm->_allowSaveGame = false;
	_vm->_allowLoadGame = false;

	titleScreen();
	if (!(thoughts1() && tvDialogue() && thoughts2()))
		_gm->_rooms[AIRPORT]->setRoomSeen(true);
	_vm->paletteFadeOut();

	for (int i = 0; i < 3; ++i)
		_gm->_inventory.add(*_gm->_rooms[INTRO]->getObject(i));

	_gm->changeRoom(AIRPORT);
	_gm->_guiEnabled = true;
	_vm->_allowSaveGame = true;
	_vm->_allowLoadGame = true;
}

void Intro::titleScreen() {
	CursorMan.showMouse(false);
	_vm->_system->fillScreen(kColorBlack);
	_vm->_screen->setViewportBrightness(0);
	_vm->_screen->setGuiBrightness(0);
	_vm->paletteBrightness();
	_vm->setCurrentImage(1);
	_vm->renderImage(0);
	_vm->paletteFadeIn();
	_gm->wait(15);
	_vm->renderImage(1);
	_gm->wait(15);
	_vm->renderImage(2);
	const Common::String title1 = "V1.02";
	_vm->_screen->renderText(title1, 295, 190, 3);

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
	_vm->stopSound();
	_vm->paletteFadeOut();
	CursorMan.showMouse(true);
}

bool Intro::tvSay(int mod1, int mod2, int rest, MessagePosition pos, StringId id) {
	Common::KeyCode key = Common::KEYCODE_INVALID;
	const Common::String& text = _vm->getGameString(id);

	_vm->renderMessage(text, pos);
	int animation_count = (text.size() + 20) * (10 - rest) * _vm->_textSpeed / 400;
	_restTime =  (text.size() + 20) * rest * _vm->_textSpeed / 400;

	while (animation_count) {
		if (mod1)
			_vm->renderImage(mod1);

		if (_gm->waitOnInput(2, key)) {
			_vm->removeMessage();
			return key != Common::KEYCODE_ESCAPE && !_vm->shouldQuit();
		}
		if (mod2)
			_vm->renderImage(mod2);

		if (_gm->waitOnInput(2, key)) {
			_vm->removeMessage();
			return key != Common::KEYCODE_ESCAPE && !_vm->shouldQuit();
		}
		animation_count--;
	}
	if (_restTime == 0)
		_vm->removeMessage();

	return true;
}

bool Intro::tvRest(int mod1, int mod2, int rest) {
	Common::KeyCode key = Common::KEYCODE_INVALID;
	while (rest) {
		_vm->renderImage(mod1);
		if (_gm->waitOnInput(2, key)) {
			_vm->removeMessage();
			return key != Common::KEYCODE_ESCAPE && !_vm->shouldQuit();
		}
		_vm->renderImage(mod2);
		if (_gm->waitOnInput(2, key)) {
			_vm->removeMessage();
			return key != Common::KEYCODE_ESCAPE && !_vm->shouldQuit();
		}
		rest--;
	}
	return true;
}

bool Intro::displayThoughtMessage(StringId id) {
	Common::KeyCode key = Common::KEYCODE_INVALID;
	const Common::String& text = _vm->getGameString(id);
	_vm->renderMessage(text, kMessageNormal);
	if (_gm->waitOnInput((text.size() + 20) * _vm->_textSpeed / 10, key)) {
		_vm->removeMessage();
		return key != Common::KEYCODE_ESCAPE && !_vm->shouldQuit();
	}
	_vm->removeMessage();
	return true;
}

bool Intro::thoughts1() {
	if(_vm->shouldQuit())
		return false;

	_vm->setCurrentImage(41);
	_vm->renderImage(0);
	_vm->paletteFadeIn();

	if(!displayThoughtMessage(kStringIntro6))
		return false;

	if(!displayThoughtMessage(kStringIntro7))
		return false;

	if(!displayThoughtMessage(kStringIntro8))
		return false;

	_vm->paletteFadeOut();
	return true;
}

bool Intro::thoughts2() {
	if(_vm->shouldQuit())
		return false;

	_vm->setCurrentImage(41);
	_vm->renderImage(0);
	_vm->paletteFadeIn();

	if(!displayThoughtMessage(kStringIntro9))
		return false;

	if(!displayThoughtMessage(kStringIntro10))
		return false;

	if(!displayThoughtMessage(kStringIntro11))
		return false;

	_vm->paletteFadeOut();

	_vm->setCurrentImage(2);
	_vm->renderImage(0);
	_vm->renderImage(1);
	_vm->paletteFadeIn();

	for (int i = 0; i < 35; i++)
	{
		_vm->renderImage((i % 3) + 2);
		_gm->wait(3);
	}
	_vm->paletteFadeOut();

	_vm->setCurrentImage(41);
	_vm->renderImage(0);
	_vm->renderImage(1);
	_vm->paletteFadeIn();

	if(!displayThoughtMessage(kStringIntro12))
		return false;

	if(!displayThoughtMessage(kStringIntro13))
		return false;

	if(!displayThoughtMessage(kStringIntro14))
		return false;

	_vm->paletteFadeOut();
	return true;
}

bool Intro::tvDialogue() {
	if(_vm->shouldQuit())
		return false;

	_vm->setCurrentImage(39);
	_vm->renderImage(0);
	_vm->paletteFadeIn();
	_gm->wait(50);
	_vm->setCurrentImage(40);
	_vm->renderImage(0);
	for (int i = 1; i < 11; i++)
	{
		_gm->wait(3);
		_vm->renderImage(i);
	}
	_vm->playSound(kAudioIntroDing);
	_gm->wait(30);
	_vm->renderImage(11);
	_vm->playSound(kAudioIntroDing);
	_gm->wait(60);

	_vm->_system->fillScreen(kColorBlack);
	_vm->setCurrentImage(42);
	_vm->renderImage(0);

	if(!tvSay(1, 1+128, 0, kMessageLeft, kStringIntroTV1))
		return false;

	_vm->renderImage(4);
	_gm->wait(3);
	_vm->renderImage(6);

	if(!tvSay(8, 6, 7, kMessageLeft, kStringIntroTV2))
		return false;
	debug("%d", _vm->shouldQuit());

	_vm->renderImage(10);

	if(!tvRest(8, 6, _restTime))
		return false;

	_vm->removeMessage();

	if(!tvSay(8, 6, 0, kMessageLeft, kStringIntroTV3))
		return false;

	if(!tvSay(8, 6, 0, kMessageLeft, kStringIntroTV4))
		return false;

	_vm->renderImage(10 + 128);
	_gm->wait(3);
	_vm->renderImage(5);
	_gm->wait(3);
	_vm->renderImage(7);

	if(!tvSay(9, 7, 0, kMessageCenter, kStringIntroTV5))
		return false;

	if(!tvSay(9, 7, 0, kMessageCenter, kStringIntroTV6))
		return false;

	if(!tvSay(9, 7, 0, kMessageCenter, kStringIntroTV7))
		return false;

	if(!tvSay(3, 3 + 128, 0, kMessageRight, kStringIntroTV8))
		return false;

	if(!tvSay(3, 3 + 128, 0, kMessageRight, kStringIntroTV9))
		return false;

	if(!tvSay(9, 7, 0, kMessageCenter, kStringIntroTV10))
		return false;

	if(!tvSay(3, 3 + 128, 0, kMessageRight, kStringIntroTV11))
		return false;

	if(!tvSay(3, 3 + 128, 0, kMessageRight, kStringIntroTV12))
		return false;

	if(!tvSay(9, 7, 8, kMessageCenter, kStringIntroTV13))
		return false;

	_vm->renderImage(4);

	if(!tvRest(9, 7, 1))
		return false;

	_vm->renderImage(4 + 128);

	if(!tvRest(9, 7, 3))
		return false;

	_vm->renderImage(4);

	if(!tvRest(9, 7, 1))
		return false;

	_vm->renderImage(6);

	if(!tvRest(9, 7, _restTime - 5))
		return false;

	_vm->removeMessage();

	if(!tvSay(3, 3 + 128, 0, kMessageRight, kStringIntroTV14))
		return false;

	if(!tvSay(3, 3 + 128, 0, kMessageRight, kStringIntroTV15))
		return false;

	if(!tvSay(9, 7, 0, kMessageCenter, kStringIntroTV16))
		return false;

	return true;
}

Airport::Airport(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 2;
	_id = AIRPORT;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringSpaceship, kStringSpaceshipDescription, NULLOBJECT, NULLTYPE, 0, 0, 0, NULLROOM, 0);
	_objectState[1] = Object(_id, kStringVehicles, kStringVehiclesDescription, NULLOBJECT, EXIT, 1, 1, 0, TAXISTAND, 8);
}

void Airport::onEntrance() {
	if (hasSeen() == false) {
		_vm->renderMessage(kStringAirportEntrance);
	}
	setRoomSeen(true);
}

TaxiStand::TaxiStand(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 3;
	_id = TAXISTAND;
	_shown[0] = kShownTrue;
	_shown[1] = kShownTrue;
	_shown[2] = kShownTrue;
	_shown[3] = kShownFalse;
	_shown[4] = kShownFalse;
	_shown[5] = kShownFalse;
	_shown[6] = kShownFalse;
	_shown[7] = kShownTrue;

	_objectState[0] = Object(_id, kStringVehicle, kStringVehicleDescription, TAXI, NULLTYPE, 2, 2, 0, NULLROOM, 11);
	_objectState[1] = Object(_id, kStringVehicle, kStringVehicleDescription, NULLOBJECT, NULLTYPE, 1, 1, 0);
	_objectState[2] = Object(_id, kStringEntrance, kStringEntranceDescription, DOOR, EXIT | OPENABLE | CLOSED, 3, 3, 0, NULLROOM, 1);
	_objectState[3] = Object(_id, kStringWallet, kStringWalletDescription, WALLET, TAKE, 0, 0, 7 + 128);
	_objectState[4] = Object(_id, kStringDevice, kStringDeviceDescription, TRANSMITTER, TAKE | PRESS, 255, 255, 0);
	_objectState[5] = Object(_id, kStringIdCard, kStringIdCardDescription, ID_CARD, TAKE | COMBINABLE, 255, 255, 0);
	_objectState[6] = Object(_id, kStringAirport, kStringDefaultDescription, NULLOBJECT, EXIT, 255, 255, 0, AIRPORT, 22);
}

void TaxiStand::onEntrance() {
	setRoomSeen(true);
}

void TaxiStand::animation() {
	if (isSectionVisible(4)) {
		setSectionVisible(1, kShownFalse);
		setSectionVisible(2, kShownFalse);
		setSectionVisible(3, kShownFalse);
		setSectionVisible(4, kShownFalse);
	}
	else if (isSectionVisible(3))
		setSectionVisible(4, kShownTrue);
	else if (isSectionVisible(2))
		setSectionVisible(3, kShownTrue);
	else if (isSectionVisible(1))
		setSectionVisible(2, kShownTrue);
	else
		setSectionVisible(1, kShownTrue);
	_gm->setAnimationTimer(7);
}

bool TaxiStand::interact(Action verb, Object &obj1, Object &obj2) {
	if (verb == ACTION_WALK && obj1._id == TAXI && isSectionVisible(6))
		_gm->taxi();
	else if ((verb == ACTION_WALK || verb == ACTION_OPEN) && obj1._id == DOOR)
		_vm->renderMessage(obj1._description);
	else 
		return false;
	return true;
}

Street::Street(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 5;
	_id = STREET;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringEntrance, kStringDefaultDescription, NULLOBJECT, EXIT, 0, 0, 0, GAMES, 10);
	_objectState[1] = Object(_id, kStringStaircase, kStringStaircaseDescription, NULLOBJECT, NULLTYPE, 2, 2, 0);
	_objectState[2] = Object(_id, kStringBusinessStreet, kStringBusinessStreetDescription, REAR_STREET, EXIT, 3, 3, 0, KIOSK, 3);
	_objectState[3] = Object(_id, kStringRod, kStringLooksMetal, ROD, COMBINABLE, 7, 7, 22);
	_objectState[4] = Object(_id, kStringRod, kStringLooksMetal, ROD, COMBINABLE, 6, 6, 0);
	_objectState[5] = Object(_id, kStringPost, kStringLooksMetal, NULLOBJECT, NULLTYPE, 4, 4, 0);
	_objectState[6] = Object(_id, kStringRailing, kStringLooksMetal, NULLOBJECT, NULLTYPE, 5, 5, 0);
}

void Street::onEntrance() {
	setRoomSeen(true);
}

void Street::animation() {
	static int ltab[36] = {
		8, 9 + 128, 10, 11 + 128, 6, 12, 13 + 128, 9, 14, 15 + 128, 19,
		16, 17 + 128, 9 + 128, 18, 19 + 128, 6 + 128, 20, 21 + 128,
		8 + 128, 9, 10 + 128, 11, 6, 12 + 128, 13, 14 + 128, 15, 19,
		16 + 128, 17, 18 + 128, 19 + 128, 6 + 128, 20 + 128, 21
	};

	static int i, banks, light;

	if (isSectionVisible(7))
		setSectionVisible(7, kShownFalse);
	else
		setSectionVisible(7, kShownTrue);

	if (++i == 4) {
		i = 0;
		switch (banks) {
		case 0:
			setSectionVisible(1, kShownTrue);
			break;
		case 1:
			setSectionVisible(2, kShownTrue);
			break;
		case 2:
			setSectionVisible(3, kShownTrue);
			break;
		case 3:
			setSectionVisible(4, kShownTrue);
			break;
		case 4:
			setSectionVisible(5, kShownTrue);
			break;
		case 5:
			// fall through
		case 7:
			// fall through
		case 9:
			setSectionVisible(1, kShownFalse);
			setSectionVisible(2, kShownFalse);
			setSectionVisible(3, kShownFalse);
			setSectionVisible(4, kShownFalse);
			setSectionVisible(5, kShownFalse);
			break;
		case 6:
			// fall through
		case 8:
			setSectionVisible(1, kShownTrue);
			setSectionVisible(2, kShownTrue);
			setSectionVisible(3, kShownTrue);
			setSectionVisible(4, kShownTrue);
			setSectionVisible(5, kShownTrue);
			break;
		}
		banks++;
		if (banks == 10) banks = 0;
	}
	setSectionVisible(ltab[light], kShownTrue);
	light++;
	if (light == 36)
		light = 0;
	_gm->setAnimationTimer(2);
}

bool Street::interact(Action verb, Object &obj1, Object &obj2) {
	if (verb == ACTION_USE && Object::combine(obj1, obj2, KNIFE, ROD)) {
		if (getObject(3)->_type & CARRIED)
			_vm->renderMessage(kStringAlreadyHavePole);
		else {
			_vm->renderMessage(kStringSawPole);
			_gm->takeObject(*getObject(3));
			_vm->playSound(kAudioSuccess);
		}
	}
	else if (verb == ACTION_WALK && obj1._id == REAR_STREET) {
		Common::String text = _vm->getGameString(kStringOnlyShop);
		_vm->renderMessage(text);
		_gm->waitOnInput((text.size() + 20) * _vm->_textSpeed / 10);
		_vm->removeMessage();
		return false;
	}
	else 
		return false;
	return true;
}

Games::Games(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = GAMES;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringPoster, kStringPosterDescription, POSTER, UNNECESSARY, 3, 3, 0);
	_objectState[1] = Object(_id, kStringCabin, kStringCabinFree, NULLOBJECT, EXIT, 1, 1, 0, CABIN, 9);
	_objectState[2] = Object(_id, kStringCabin, kStringCabinOccupied, OCCUPIED_CABIN, NULLTYPE, 0, 0, 0);
	_objectState[3] = Object(_id, kStringFeet, kStringFeetDescription, NULLOBJECT, NULLTYPE, 2, 2, 0);
	_objectState[4] = Object(_id, kStringExit, kStringDefaultDescription, NULLOBJECT, EXIT, 255, 255, 0, STREET, 22);
}

void Games::onEntrance() {
	setRoomSeen(true);
}

bool Games::interact(Action verb, Object &obj1, Object &obj2) {
	if (verb == ACTION_WALK && obj1._id == OCCUPIED_CABIN) {
		_vm->renderMessage(kStringCabinOccupiedSay);
	}
	else if (verb == ACTION_LOOK && obj1._id == POSTER) {
		_gm->_taxi_possibility &= ~4; // add culture palace
		return false;
	}
	else 
		return false;
	return true;
}

Cabin::Cabin(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;
	_paid = false;

	_fileNumber = 7;
	_id = CABIN;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringExit, kStringDefaultDescription, NULLOBJECT, EXIT, 255, 255, 0, GAMES, 22);
	_objectState[1] = Object(_id, kStringHood, kStringHoodDescription, NULLOBJECT, NULLTYPE, 0, 0, 0);
	_objectState[2] = Object(_id, kString400Xa, kStringDefaultDescription, PRIZE, TAKE, 255, 255, 2 + 180);
	_objectState[3] = Object(_id, kString10Xa, kStringDefaultDescription, BACK_MONEY, TAKE, 255, 255, 2 + 128);
	_objectState[4] = Object(_id, kStringSlot, kStringSlotDescription1, SLOT1, COMBINABLE, 2, 2, 0);
	_objectState[5] = Object(_id, kStringSlot, kStringSlotDescription2, NULLOBJECT, COMBINABLE, 3, 3, 0);
	_objectState[6] = Object(_id, kStringChair, kStringChairDescription, CHAIR, NULLTYPE, 4, 4, 0);
	_objectState[7] = Object(_id, kStringScribble, kStringDefaultDescription, SCRIBBLE1, NULLTYPE, 5, 5, 0);
	_objectState[8] = Object(_id, kStringScribble, kStringDefaultDescription, SCRIBBLE2, NULLTYPE, 6, 6, 0);
	_objectState[9] = Object(_id, kStringFace, kStringFaceDescription, NULLOBJECT, NULLTYPE, 7, 7, 0);
	_objectState[10] = Object(_id, kStringSign, kStringDefaultDescription, SIGN, UNNECESSARY, 1, 1, 0);
}

void Cabin::onEntrance() {
	setRoomSeen(true);
}

void Cabin::animation() {
	if (_paid) {
		if (isSectionVisible(1))
			_vm->renderImage(1 + 128);
		else
			_vm->renderImage(1);
	}
	_gm->setAnimationTimer(4);
}

bool Cabin::interact(Action verb, Object &obj1, Object &obj2) {
	if (verb == ACTION_USE && Object::combine(obj1, obj2, MONEY, SLOT1)) {
		if (isSectionVisible(2))
			_vm->renderMessage(kStringTakeMoney);
		else if (_paid)
			_vm->renderMessage(kStringAlreadyPaid);
		else if (_gm->_state._money < 10)
			_vm->renderMessage(kStringNoMoney);
		else {
			_vm->renderMessage(kStringPay10Xa);
			_gm->takeMoney(-10);
			_paid = true;
		}
	}
	else if (verb == ACTION_USE && obj1._id == CHAIR) {
		if (_paid) {
			if (_var2) {
				_vm->paletteFadeOut();
				_vm->setCurrentImage(31);
				_vm->renderImage(0);
				_vm->paletteFadeIn();
				_paid = true;
				_gm->waitOnInput(100000);
				_vm->paletteFadeOut();
				_vm->setCurrentImage(7);
				_vm->renderImage(0);
				setSectionVisible(1, kShownFalse);
				_paid = false;
				_vm->renderRoom(*this);
				_vm->renderImage(2);
				_gm->drawMapExits();
				_gm->drawInventory();
				_gm->drawStatus();
				_gm->drawCommandBox();
				_vm->paletteFadeIn();
				getObject(3)->_click = 8;
			} else {
				_gm->_state._tipsy = false;
				_vm->paletteFadeOut();
				_vm->_system->fillScreen(kColorBlack);
				Common::String text = _vm->getGameString(kStringWillPassOut);
				_vm->renderMessage(text);
				_gm->waitOnInput((text.size() + 20) * _vm->_textSpeed / 10);
				_vm->removeMessage();
				_vm->saveGame(kSleepAutosaveSlot, "autosave");
				_gm->_inventory.clear();
				_gm->changeRoom(PYRAMID);
				_gm->drawStatus();
				_gm->drawInventory();
				_gm->drawMapExits();
				_gm->drawCommandBox();
			}
		} else
			_vm->renderMessage(kStringRest);
	}
	else 
		return false;
	return true;
}

Kiosk::Kiosk(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = KIOSK;
	_shown[0] = kShownTrue;
}

void Kiosk::onEntrance() {
	setRoomSeen(true);
}

void Kiosk::animation() {
}

bool Kiosk::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

CulturePalace::CulturePalace(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = CULTURE_PALACE;
	_shown[0] = kShownTrue;
}

void CulturePalace::onEntrance() {
	setRoomSeen(true);
}

void CulturePalace::animation() {
}

bool CulturePalace::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Cashbox::Cashbox(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = CASHBOX;
	_shown[0] = kShownTrue;
}

void Cashbox::onEntrance() {
	setRoomSeen(true);
}

void Cashbox::animation() {
}

bool Cashbox::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

City1::City1(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = CITY1;
	_shown[0] = kShownTrue;
}

void City1::onEntrance() {
	setRoomSeen(true);
}

void City1::animation() {
}

bool City1::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

City2::City2(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = CITY2;
	_shown[0] = kShownTrue;
}

void City2::onEntrance() {
	setRoomSeen(true);
}

void City2::animation() {
}

bool City2::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Elevator::Elevator(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = ELEVATOR;
	_shown[0] = kShownTrue;
}

void Elevator::onEntrance() {
	setRoomSeen(true);
}

void Elevator::animation() {
}

bool Elevator::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Apartment::Apartment(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = APARTMENT;
	_shown[0] = kShownTrue;
}

void Apartment::onEntrance() {
	setRoomSeen(true);
}

void Apartment::animation() {
}

bool Apartment::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Ship::Ship(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = SHIP;
	_shown[0] = kShownTrue;
}

void Ship::onEntrance() {
	setRoomSeen(true);
}

void Ship::animation() {
}

bool Ship::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Pyramid::Pyramid(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = PYRAMID;
	_shown[0] = kShownTrue;
}

void Pyramid::onEntrance() {
	setRoomSeen(true);
}

void Pyramid::animation() {
}

bool Pyramid::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

PyrEntrance::PyrEntrance(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = PYR_ENTRANCE;
	_shown[0] = kShownTrue;
}

void PyrEntrance::onEntrance() {
	setRoomSeen(true);
}

void PyrEntrance::animation() {
}

bool PyrEntrance::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Upstairs1::Upstairs1(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = UPSTAIRS1;
	_shown[0] = kShownTrue;
}

void Upstairs1::onEntrance() {
	setRoomSeen(true);
}

void Upstairs1::animation() {
}

bool Upstairs1::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Downstairs1::Downstairs1(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = DOWNSTAIRS1;
	_shown[0] = kShownTrue;
}

void Downstairs1::onEntrance() {
	setRoomSeen(true);
}

void Downstairs1::animation() {
}

bool Downstairs1::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

BottomRightDoor::BottomRightDoor(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = BOTTOM_RIGHT_DOOR;
	_shown[0] = kShownTrue;
}

void BottomRightDoor::onEntrance() {
	setRoomSeen(true);
}

void BottomRightDoor::animation() {
}

bool BottomRightDoor::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

BottomLeftDoor::BottomLeftDoor(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = BOTTOM_LEFT_DOOR;
	_shown[0] = kShownTrue;
}

void BottomLeftDoor::onEntrance() {
	setRoomSeen(true);
}

void BottomLeftDoor::animation() {
}

bool BottomLeftDoor::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Upstairs2::Upstairs2(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = UPSTAIRS2;
	_shown[0] = kShownTrue;
}

void Upstairs2::onEntrance() {
	setRoomSeen(true);
}

void Upstairs2::animation() {
}

bool Upstairs2::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Downstairs2::Downstairs2(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = DOWNSTAIRS2;
	_shown[0] = kShownTrue;
}

void Downstairs2::onEntrance() {
	setRoomSeen(true);
}

void Downstairs2::animation() {
}

bool Downstairs2::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

UpperDoor::UpperDoor(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = UPPER_DOOR;
	_shown[0] = kShownTrue;
}

void UpperDoor::onEntrance() {
	setRoomSeen(true);
}

void UpperDoor::animation() {
}

bool UpperDoor::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

PuzzleFront::PuzzleFront(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = PUZZLE_FRONT;
	_shown[0] = kShownTrue;
}

void PuzzleFront::onEntrance() {
	setRoomSeen(true);
}

void PuzzleFront::animation() {
}

bool PuzzleFront::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

PuzzleBehind::PuzzleBehind(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = PUZZLE_BEHIND;
	_shown[0] = kShownTrue;
}

void PuzzleBehind::onEntrance() {
	setRoomSeen(true);
}

void PuzzleBehind::animation() {
}

bool PuzzleBehind::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Formula1F::Formula1F(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = FORMULA1_F;
	_shown[0] = kShownTrue;
}

void Formula1F::onEntrance() {
	setRoomSeen(true);
}

void Formula1F::animation() {
}

bool Formula1F::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Formula1N::Formula1N(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = FORMULA1_N;
	_shown[0] = kShownTrue;
}

void Formula1N::onEntrance() {
	setRoomSeen(true);
}

void Formula1N::animation() {
}

bool Formula1N::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Formula2F::Formula2F(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = FORMULA2_F;
	_shown[0] = kShownTrue;
}

void Formula2F::onEntrance() {
	setRoomSeen(true);
}

void Formula2F::animation() {
}

bool Formula2F::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Formula2N::Formula2N(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = FORMULA2_N;
	_shown[0] = kShownTrue;
}

void Formula2N::onEntrance() {
	setRoomSeen(true);
}

void Formula2N::animation() {
}

bool Formula2N::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

TomatoF::TomatoF(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = TOMATO_F;
	_shown[0] = kShownTrue;
}

void TomatoF::onEntrance() {
	setRoomSeen(true);
}

void TomatoF::animation() {
}

bool TomatoF::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

TomatoN::TomatoN(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = TOMATO_N;
	_shown[0] = kShownTrue;
}

void TomatoN::onEntrance() {
	setRoomSeen(true);
}

void TomatoN::animation() {
}

bool TomatoN::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

MonsterF::MonsterF(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MONSTER_F;
	_shown[0] = kShownTrue;
}

void MonsterF::onEntrance() {
	setRoomSeen(true);
}

void MonsterF::animation() {
}

bool MonsterF::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Monster1N::Monster1N(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MONSTER1_N;
	_shown[0] = kShownTrue;
}

void Monster1N::onEntrance() {
	setRoomSeen(true);
}

void Monster1N::animation() {
}

bool Monster1N::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Monster2N::Monster2N(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MONSTER2_N;
	_shown[0] = kShownTrue;
}

void Monster2N::onEntrance() {
	setRoomSeen(true);
}

void Monster2N::animation() {
}

bool Monster2N::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Upstairs3::Upstairs3(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = UPSTAIRS3;
	_shown[0] = kShownTrue;
}

void Upstairs3::onEntrance() {
	setRoomSeen(true);
}

void Upstairs3::animation() {
}

bool Upstairs3::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Downstairs3::Downstairs3(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = DOWNSTAIRS3;
	_shown[0] = kShownTrue;
}

void Downstairs3::onEntrance() {
	setRoomSeen(true);
}

void Downstairs3::animation() {
}

bool Downstairs3::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

LGang1::LGang1(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = LGANG1;
	_shown[0] = kShownTrue;
}

void LGang1::onEntrance() {
	setRoomSeen(true);
}

void LGang1::animation() {
}

bool LGang1::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

LGang2::LGang2(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = LGANG2;
	_shown[0] = kShownTrue;
}

void LGang2::onEntrance() {
	setRoomSeen(true);
}

void LGang2::animation() {
}

bool LGang2::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

HoleRoom::HoleRoom(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = HOLE_ROOM;
	_shown[0] = kShownTrue;
}

void HoleRoom::onEntrance() {
	setRoomSeen(true);
}

void HoleRoom::animation() {
}

bool HoleRoom::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

InHole::InHole(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = IN_HOLE;
	_shown[0] = kShownTrue;
}

void InHole::onEntrance() {
	setRoomSeen(true);
}

void InHole::animation() {
}

bool InHole::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Bodentuer::Bodentuer(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = BODENTUER;
	_shown[0] = kShownTrue;
}

void Bodentuer::onEntrance() {
	setRoomSeen(true);
}

void Bodentuer::animation() {
}

bool Bodentuer::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

BodentuerU::BodentuerU(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = BODENTUER_U;
	_shown[0] = kShownTrue;
}

void BodentuerU::onEntrance() {
	setRoomSeen(true);
}

void BodentuerU::animation() {
}

bool BodentuerU::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

BstDoor::BstDoor(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = BST_DOOR;
	_shown[0] = kShownTrue;
}

void BstDoor::onEntrance() {
	setRoomSeen(true);
}

void BstDoor::animation() {
}

bool BstDoor::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Hall::Hall(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = HALL;
	_shown[0] = kShownTrue;
}

void Hall::onEntrance() {
	setRoomSeen(true);
}

void Hall::animation() {
}

bool Hall::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

CoffinRoom::CoffinRoom(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = COFFIN_ROOM;
	_shown[0] = kShownTrue;
}

void CoffinRoom::onEntrance() {
	setRoomSeen(true);
}

void CoffinRoom::animation() {
}

bool CoffinRoom::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mask::Mask(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MASK;
	_shown[0] = kShownTrue;
}

void Mask::onEntrance() {
	setRoomSeen(true);
}

void Mask::animation() {
}

bool Mask::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Museum::Museum(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUSEUM;
	_shown[0] = kShownTrue;
}

void Museum::onEntrance() {
	setRoomSeen(true);
}

void Museum::animation() {
}

bool Museum::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

MusEing::MusEing(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS_EING;
	_shown[0] = kShownTrue;
}

void MusEing::onEntrance() {
	setRoomSeen(true);
}

void MusEing::animation() {
}

bool MusEing::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus1::Mus1(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS1;
	_shown[0] = kShownTrue;
}

void Mus1::onEntrance() {
	setRoomSeen(true);
}

void Mus1::animation() {
}

bool Mus1::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus2::Mus2(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS2;
	_shown[0] = kShownTrue;
}

void Mus2::onEntrance() {
	setRoomSeen(true);
}

void Mus2::animation() {
}

bool Mus2::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus3::Mus3(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS3;
	_shown[0] = kShownTrue;
}

void Mus3::onEntrance() {
	setRoomSeen(true);
}

void Mus3::animation() {
}

bool Mus3::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus4::Mus4(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS4;
	_shown[0] = kShownTrue;
}

void Mus4::onEntrance() {
	setRoomSeen(true);
}

void Mus4::animation() {
}

bool Mus4::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus5::Mus5(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS5;
	_shown[0] = kShownTrue;
}

void Mus5::onEntrance() {
	setRoomSeen(true);
}

void Mus5::animation() {
}

bool Mus5::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus6::Mus6(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS6;
	_shown[0] = kShownTrue;
}

void Mus6::onEntrance() {
	setRoomSeen(true);
}

void Mus6::animation() {
}

bool Mus6::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus7::Mus7(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS7;
	_shown[0] = kShownTrue;
}

void Mus7::onEntrance() {
	setRoomSeen(true);
}

void Mus7::animation() {
}

bool Mus7::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus8::Mus8(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS8;
	_shown[0] = kShownTrue;
}

void Mus8::onEntrance() {
	setRoomSeen(true);
}

void Mus8::animation() {
}

bool Mus8::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus9::Mus9(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS9;
	_shown[0] = kShownTrue;
}

void Mus9::onEntrance() {
	setRoomSeen(true);
}

void Mus9::animation() {
}

bool Mus9::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus10::Mus10(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS10;
	_shown[0] = kShownTrue;
}

void Mus10::onEntrance() {
	setRoomSeen(true);
}

void Mus10::animation() {
}

bool Mus10::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus11::Mus11(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS11;
	_shown[0] = kShownTrue;
}

void Mus11::onEntrance() {
	setRoomSeen(true);
}

void Mus11::animation() {
}

bool Mus11::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

MusRund::MusRund(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS_RUND;
	_shown[0] = kShownTrue;
}

void MusRund::onEntrance() {
	setRoomSeen(true);
}

void MusRund::animation() {
}

bool MusRund::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus12::Mus12(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS12;
	_shown[0] = kShownTrue;
}

void Mus12::onEntrance() {
	setRoomSeen(true);
}

void Mus12::animation() {
}

bool Mus12::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus13::Mus13(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS13;
	_shown[0] = kShownTrue;
}

void Mus13::onEntrance() {
	setRoomSeen(true);
}

void Mus13::animation() {
}

bool Mus13::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus14::Mus14(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS14;
	_shown[0] = kShownTrue;
}

void Mus14::onEntrance() {
	setRoomSeen(true);
}

void Mus14::animation() {
}

bool Mus14::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus15::Mus15(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS15;
	_shown[0] = kShownTrue;
}

void Mus15::onEntrance() {
	setRoomSeen(true);
}

void Mus15::animation() {
}

bool Mus15::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus16::Mus16(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS16;
	_shown[0] = kShownTrue;
}

void Mus16::onEntrance() {
	setRoomSeen(true);
}

void Mus16::animation() {
}

bool Mus16::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus17::Mus17(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS17;
	_shown[0] = kShownTrue;
}

void Mus17::onEntrance() {
	setRoomSeen(true);
}

void Mus17::animation() {
}

bool Mus17::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus18::Mus18(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS18;
	_shown[0] = kShownTrue;
}

void Mus18::onEntrance() {
	setRoomSeen(true);
}

void Mus18::animation() {
}

bool Mus18::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus19::Mus19(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS19;
	_shown[0] = kShownTrue;
}

void Mus19::onEntrance() {
	setRoomSeen(true);
}

void Mus19::animation() {
}

bool Mus19::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus20::Mus20(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS20;
	_shown[0] = kShownTrue;
}

void Mus20::onEntrance() {
	setRoomSeen(true);
}

void Mus20::animation() {
}

bool Mus20::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus21::Mus21(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS21;
	_shown[0] = kShownTrue;
}

void Mus21::onEntrance() {
	setRoomSeen(true);
}

void Mus21::animation() {
}

bool Mus21::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

Mus22::Mus22(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = MUS22;
	_shown[0] = kShownTrue;
}

void Mus22::onEntrance() {
	setRoomSeen(true);
}

void Mus22::animation() {
}

bool Mus22::interact(Action verb, Object &obj1, Object &obj2) {
	return true;
}

}
