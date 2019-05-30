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
	_shown[section] = visible ? kShownTrue : kShownFalse;
}

bool Room::isSectionVisible(uint index) const {
	return _shown[index] == kShownTrue;
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
	thoughts1() && tvDialogue() && thoughts2();

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

	Marquee marquee(_vm->_screen, Marquee::kMarqueeIntro, _introText.c_str());
	while (!_vm->shouldQuit()) {
		_gm->updateEvents();
		marquee.renderCharacter();
		if (_gm->_mouseClicked || _gm->_keyPressed)
			break;
		g_system->updateScreen();
		g_system->delayMillis(_vm->_delay);
	}
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
			return key != Common::KEYCODE_ESCAPE;
		}
		if (mod2)
			_vm->renderImage(mod2);

		if (_gm->waitOnInput(2, key)) {
			_vm->removeMessage();
			return key != Common::KEYCODE_ESCAPE;
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
			return key != Common::KEYCODE_ESCAPE;
		}
		_vm->renderImage(mod2);
		if (_gm->waitOnInput(2, key)) {
			_vm->removeMessage();
			return key != Common::KEYCODE_ESCAPE;
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
		return key != Common::KEYCODE_ESCAPE;
	}
	_vm->removeMessage();
	return true;
}

bool Intro::thoughts1() {
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
	_gm->wait(30);
	_vm->renderImage(11);
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

	_vm->paletteFadeOut();
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
	for (int i = 0; i < 3; ++i)
		_gm->_inventory.add(*_gm->_rooms[INTRO]->getObject(i));

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
	_objectState[5] = Object(_id, kStringAirport, kStringDefaultDescription, NULLOBJECT, EXIT, 255, 255, 0, AIRPORT, 22);
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
		;//taxi
	else if ((verb == ACTION_WALK || verb == ACTION_OPEN) && obj1._id == DOOR)
		_vm->renderMessage(obj1._description);
	else 
		return false;
	return true;
}

}
