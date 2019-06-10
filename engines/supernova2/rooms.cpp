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
#include "supernova2/graphics.h"

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

	if(!_gm->talk(1, 1+128, 0, kMessageLeft, kStringIntroTV1))
		return false;

	_vm->renderImage(4);
	_gm->wait(3);
	_vm->renderImage(6);

	if(!_gm->talk(8, 6, 7, kMessageLeft, kStringIntroTV2))
		return false;
	debug("%d", _vm->shouldQuit());

	_vm->renderImage(10);

	if(!_gm->talkRest(8, 6, _gm->_restTime))
		return false;

	_vm->removeMessage();

	if(!_gm->talk(8, 6, 0, kMessageLeft, kStringIntroTV3))
		return false;

	if(!_gm->talk(8, 6, 0, kMessageLeft, kStringIntroTV4))
		return false;

	_vm->renderImage(10 + 128);
	_gm->wait(3);
	_vm->renderImage(5);
	_gm->wait(3);
	_vm->renderImage(7);

	if(!_gm->talk(9, 7, 0, kMessageCenter, kStringIntroTV5))
		return false;

	if(!_gm->talk(9, 7, 0, kMessageCenter, kStringIntroTV6))
		return false;

	if(!_gm->talk(9, 7, 0, kMessageCenter, kStringIntroTV7))
		return false;

	if(!_gm->talk(3, 3 + 128, 0, kMessageRight, kStringIntroTV8))
		return false;

	if(!_gm->talk(3, 3 + 128, 0, kMessageRight, kStringIntroTV9))
		return false;

	if(!_gm->talk(9, 7, 0, kMessageCenter, kStringIntroTV10))
		return false;

	if(!_gm->talk(3, 3 + 128, 0, kMessageRight, kStringIntroTV11))
		return false;

	if(!_gm->talk(3, 3 + 128, 0, kMessageRight, kStringIntroTV12))
		return false;

	if(!_gm->talk(9, 7, 8, kMessageCenter, kStringIntroTV13))
		return false;

	_vm->renderImage(4);

	if(!_gm->talkRest(9, 7, 1))
		return false;

	_vm->renderImage(4 + 128);

	if(!_gm->talkRest(9, 7, 3))
		return false;

	_vm->renderImage(4);

	if(!_gm->talkRest(9, 7, 1))
		return false;

	_vm->renderImage(6);

	if(!_gm->talkRest(9, 7, _gm->_restTime - 5))
		return false;

	_vm->removeMessage();

	if(!_gm->talk(3, 3 + 128, 0, kMessageRight, kStringIntroTV14))
		return false;

	if(!_gm->talk(3, 3 + 128, 0, kMessageRight, kStringIntroTV15))
		return false;

	if(!_gm->talk(9, 7, 0, kMessageCenter, kStringIntroTV16))
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
	if (_shown[kMaxSection - 1]) {
		if (isSectionVisible(1))
			setSectionVisible(1, kShownFalse);
		else
			setSectionVisible(1, kShownTrue);
	}
	_gm->setAnimationTimer(4);
}

bool Cabin::interact(Action verb, Object &obj1, Object &obj2) {
	if (verb == ACTION_USE && Object::combine(obj1, obj2, MONEY, SLOT1)) {
		if (isSectionVisible(2))
			_vm->renderMessage(kStringTakeMoney);
		else if (_shown[kMaxSection - 1])
			_vm->renderMessage(kStringAlreadyPaid);
		else if (_gm->_state._money < 10)
			_vm->renderMessage(kStringNoMoney);
		else {
			_vm->renderMessage(kStringPay10Xa);
			_gm->takeMoney(-10);
			_shown[kMaxSection - 1] = true;
		}
	} else if (verb == ACTION_USE && obj1._id == CHAIR) {
		if (_shown[kMaxSection - 1]) {
			if (_shown[kMaxSection - 2]) {
				_vm->paletteFadeOut();
				_vm->setCurrentImage(31);
				_vm->renderImage(0);
				_vm->paletteFadeIn();
				_shown[kMaxSection - 1] = true;
				_gm->waitOnInput(100000);
				_vm->paletteFadeOut();
				_vm->setCurrentImage(7);
				_vm->renderImage(0);
				setSectionVisible(1, kShownFalse);
				_shown[kMaxSection - 1] = false;
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
				_vm->paletteFadeIn();
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
	} else if (verb == ACTION_TAKE && obj1._id == PRIZE) {
		_vm->renderImage(2 + 128);
		obj1._click = 255;
		_gm->takeMoney(400);
	} else if (verb == ACTION_TAKE && obj1._id == BACK_MONEY) {
		_vm->renderImage(2 + 128);
		obj1._click = 255;
		_gm->takeMoney(10);
	} else if (verb == ACTION_LOOK && obj1._id == SCRIBBLE1) {
		_vm->renderMessage(kStringCypher);
	} else if (verb == ACTION_LOOK && obj1._id == SCRIBBLE2) {
		_gm->animationOff();
		_vm->setCurrentImage(28);
		_vm->renderImage(0);
		_gm->waitOnInput(100000);
		_vm->setCurrentImage(7);
		_vm->renderRoom(*this);
		_gm->drawGUI();
		_gm->_state._addressKnown = true;
		_gm->animationOn();
	} else if (verb == ACTION_LOOK && obj1._id == SIGN) {
		_gm->animationOff();
		_vm->setCurrentImage(38);
		_vm->renderImage(0);
		_gm->waitOnInput(100000);
		_vm->setCurrentImage(7);
		_vm->renderRoom(*this);
		_gm->drawGUI();
		_gm->animationOn();
	} else 
		return false;
	return true;
}

Kiosk::Kiosk(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 19;
	_id = KIOSK;
	_shown[0] = kShownTrue;
	_shown[1] = kShownFalse;
	_shown[2] = kShownTrue;
	_shown[3] = kShownTrue;
	_shown[4] = kShownTrue;
	_shown[5] = kShownTrue;

	_objectState[0] = Object(_id, kStringExit, kStringDefaultDescription, NULLOBJECT, EXIT, 255, 255, 0, STREET, 22);
	_objectState[1] = Object(_id, kStringBooks, kStringDefaultDescription, BOOKS, UNNECESSARY, 0, 0, 0);
	_objectState[2] = Object(_id, kStringDictionary, kStringDefaultDescription, LEXICON, UNNECESSARY, 1, 1, 0);
	_objectState[3] = Object(_id, kStringPlant, kStringDefaultDescription, PLANT, UNNECESSARY, 2, 2, 0);
	_objectState[4] = Object(_id, kStringMask, kStringDefaultDescription, BMASK, UNNECESSARY, 4, 4, 2 + 128);
	_objectState[5] = Object(_id, kStringSnake, kStringDefaultDescription, SNAKE, UNNECESSARY, 3, 3, 0);
	_objectState[6] = Object(_id, kStringCup, kStringDefaultDescription, CUP, UNNECESSARY, 5, 5, 0);
	_objectState[7] = Object(_id, kStringJoystick, kStringDefaultDescription, JOYSTICK, UNNECESSARY, 6, 6, 0);
	_objectState[8] = Object(_id, kStringToothbrush, kStringToothbrushDescription, TOOTHBRUSH, TAKE, 7, 7, 5 + 128);
	_objectState[9] = Object(_id, kStringMusic, kStringMusicDescription, PLAYER, TAKE | COMBINABLE, 8, 8, 4 + 128);
	_objectState[10] = Object(_id, kStringBottle, kStringBottleDescription, BOTTLE, TAKE, 9, 9, 3 + 128);
	_objectState[11] = Object(_id, kStringBottle, kStringDefaultDescription, BOTTLE1, UNNECESSARY, 10, 10, 0);
	_objectState[12] = Object(_id, kStringBottle, kStringDefaultDescription, BOTTLE2, UNNECESSARY, 11, 11, 0);
	_objectState[13] = Object(_id, kStringBottle, kStringDefaultDescription, BOTTLE3, UNNECESSARY, 12, 12, 0);
	_objectState[14] = Object(_id, kStringBottle, kStringDefaultDescription, BOTTLE4, UNNECESSARY, 13, 13, 0);
	_objectState[15] = Object(_id, kStringBox, kStringDefaultDescription, BOX, UNNECESSARY, 14, 14, 0);
	_objectState[16] = Object(_id, kStringFace, kStringDefaultDescription, FACES, UNNECESSARY, 15, 15, 0);
	_objectState[17] = Object(_id, kStringSeller, kStringDefaultDescription, SELLER, TALK, 16, 16, 0);
}

void Kiosk::onEntrance() {
	static StringId dialEntry[2] = {
		kStringGoodEvening,
		kStringHello
	};

	if (!hasSeen()) {
		_gm->dialog(2, _gm->_dials, dialEntry, 0);
		_vm->renderImage(6);
		_vm->playSound(kAudioKiosk);
		_gm->wait(8);
		_vm->renderImage(6 + 128);
		_gm->reply(kStringScaredMe, 1, 1 +128);
		_gm->say(kStringHowSo);
		_gm->reply(kStringDisguise, 1, 1 +128);
		_gm->say(kStringWhatDisguise);
		_gm->reply(kStringStopPretending, 1, 1 +128);
		_gm->reply(kStringYouDisguised, 1, 1 +128);
		_gm->say(kStringIAmHorstHummel);
		_gm->reply(kStringGiveItUp, 1, 1 +128);
		_gm->reply(kStringGestures, 1, 1 +128);
		_gm->reply(kStringMovesDifferently, 1, 1 +128);
		_gm->say(kStringHeIsRobot);
		_gm->reply(kStringYouAreCrazy, 1, 1 +128);
		_gm->say(kStringYouIdiot);
		_gm->reply(kStringShutUp, 1, 1 +128);
		_gm->drawStatus();
		_gm->drawInventory();
		_gm->drawMapExits();
		_gm->drawCommandBox();
		setRoomSeen(true);
	}
}

void Kiosk::animation() {
}

bool Kiosk::interact(Action verb, Object &obj1, Object &obj2) {
	static StringId dialPrice[2] = {
		kStringWillTakeIt,
		kStringTooExpensive
	};
	static StringId dialSay[3] = {
		kStringWouldBuy,
		kStringMeHorstHummel,
		kStringHaveMusicChip
	};
	static StringId dialSeller[16][3] = {
		{kStringGreatMask, kStringThreeYears, kNoString},
		{kStringStrongDrink, kNoString, kNoString},
		{kStringMusicDevice, kNoString, kNoString},
		{kStringArtusToothbrush, kStringSellInBulk, kNoString},
		{kStringRarityBooks, kNoString, kNoString},
		{kStringEncyclopedia, kStringLargestDictionary, kStringOver400Words},
		{kStringNotSale, kNoString, kNoString},
		{kStringGaveOne, kStringExcited, kNoString},
		{kStringFromGame, kNoString, kNoString},
		{kStringRobust, kNoString, kNoString},
		{kStringCheapSwill, kNoString, kNoString},
		{kStringCheapSwill, kNoString, kNoString},
		{kStringCheapSwill, kNoString, kNoString},
		{kStringCheapSwill, kNoString, kNoString},
		{kStringStickers, kNoString, kNoString},
		{kStringDishes, kStringUgly, kStringSellsWell}
	};

	if (verb == ACTION_TAKE && !(obj1._type & CARRIED) &&
		obj1._id >= BOTTLE && obj1._id <= TOOTHBRUSH) {
		int price;
		switch (obj1._id) {
		case BOTTLE:
			price = 30;
			break;
		case PLAYER:
			price = 50;
			break;
		case TOOTHBRUSH:
			price = 5;
			break;
		default:
			break;
		}
		Common::String format = _vm->getGameString(kStringThatCosts);
		Common::String cost = Common::String::format(format.c_str(), price);
		_vm->renderMessage(cost, kMessageTop);
		_gm->reply(cost.c_str(), 1, 1 +128);

		if (_gm->_state._money < price)
			_gm->say(dialPrice[1]);
		else if (_gm->dialog(2, _gm->_dials, dialPrice, 0) == 0) {
			_gm->takeObject(obj1);
			_gm->takeMoney(-price);
		}
		_gm->drawStatus();
		_gm->drawInventory();
		_gm->drawMapExits();
		_gm->drawCommandBox();
	}
	else if (verb == ACTION_LOOK && obj1._id >= BMASK && obj1._id <= FACES) {
		for(int i = 0; i < 3; i++) {
			_gm->reply(dialSeller[obj1._id - BMASK][i], 1, 1 + 128);
		}
	}
	else if (verb == ACTION_TALK && obj1._id >= SELLER) {
		int i = 2;
		if (getObject(9)->_type & CARRIED)
			i++;
		switch (_gm->dialog(i, _gm->_dials, dialSay, 0)) {
		case 0:
			_gm->reply(kStringTakeALook, 1, 1 + 128);
			break;
		case 1:
			_gm->reply(kStringNonsense, 1, 1 + 128);
			break;
		case 2:
			_gm->reply(kStringImSorry, 1, 1 + 128);
			break;
		}
		_gm->drawStatus();
		_gm->drawInventory();
		_gm->drawMapExits();
		_gm->drawCommandBox();
	}
	else 
		return false;
	return true;
}

CulturePalace::CulturePalace(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 20;
	_id = CULTURE_PALACE;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringEntrance, kStringDefaultDescription, NULLOBJECT, EXIT, 1, 1, 0, CHECKOUT, 6);
	_objectState[1] = Object(_id, kStringCulturePalace, kStringFascinating, NULLOBJECT, NULLTYPE, 0, 0, 0);
	_objectState[2] = Object(_id, kStringTaxis, kStringTaxisDescription, NULLOBJECT, NULLTYPE, 3, 3, 0);
	_objectState[3] = Object(_id, kStringAxacussan, kStringDefaultDescription, AXACUSSER, TALK, 4, 4, 0);
	_objectState[4] = Object(_id, kStringParticipationCard, kStringDefaultDescription, CARD, TAKE, 255, 255, 0);
}

void CulturePalace::onEntrance() {
	setRoomSeen(true);
}

void CulturePalace::animation() {
	static int i;
	if (isSectionVisible(4))
		setSectionVisible(4, kShownFalse);
	else
		setSectionVisible(4, kShownTrue);
	i--;
	if (i <= 0) {
		if (isSectionVisible(3)) {
			setSectionVisible(3, kShownFalse);
			i = 4;
		} else {
			setSectionVisible(3, kShownTrue);
			i = 10;
		}
	}
	_gm->setAnimationTimer(2);
}

void CulturePalace::notEnoughMoney() {
	_gm->reply(kStringWhat, 2, 1);
	_gm->reply(kStringNotInformed, 2, 1);
	_vm->renderImage(1 + 128);
	setSectionVisible(2, kShownFalse);
}

bool CulturePalace::interact(Action verb, Object &obj1, Object &obj2) {
	static StringId dial1[3] = {
		kStringHorstHummel,
		kStringNiceWeather,
		kStringTellTicket,
	};
	static byte dials1[] = {1, 1, 2};

	static StringId dial2[2] = {
		kStringHereIsXa,
		kStringYouAreCrazy
	};
	static StringId dial3[4] = {
		kString500Xa,
		kString1000Xa,
		kString5000Xa,
		kString10000Xa
	};

	int e;
	if (verb == ACTION_TALK && obj1._id == AXACUSSER) {
		if (_shown[kMaxSection - 3]) {
			_vm->renderImage(1);
			_gm->reply(kStringThankYou, 2, 1);
		} else if (_shown[kMaxSection - 2]) {
			_vm->renderImage(1);
			_gm->reply(kStringWhatYouOffer, 2, 1);
		} else {
			_gm->say(kStringHello2);
			_vm->renderImage(1);
			_gm->reply(kStringWhatYouWant, 2, 1);
			addSentence(1, 1);
			switch (_gm->dialog(3, dials1, dial1, 1)) {
			case 0:
				_gm->reply(kStringWhoAreYou, 2, 1);
				_gm->say(kStringHorstHummel2);
				_gm->reply(kStringNeverHeard, 2, 1);
				_gm->say(kStringYouDontKnow);
				_gm->say(kStringImOnTV);
				_gm->reply(kStringIDontKnow, 2, 1);
				_gm->say(kStringFunny);
				break;
			case 1:
				_gm->reply(kStringAha, 2, 1);
				break;
			case 2:
				_gm->reply(kStringICan, 2, 1);
				_gm->say(kStringFromWhom);
				_gm->reply(kStringCost, 2, 1);
				if(!_gm->_state._money)
					addSentence(2, 1);
				else if (_gm->dialog(2, _gm->_dials, dial2, 0)) {
					_gm->reply(kStringAsYouSay, 2, 1);
					addSentence(2, 1);
				} else {
					_gm->takeMoney(-1);
					_gm->reply(kStringGetCard, 2, 1);
					_gm->reply(kStringOnlyParticipation, 2, 1);
					_gm->say(kStringWhatForIt);
					_gm->reply(kStringMakeOffer, 2, 1);
					_shown[kMaxSection - 2] = true;
				}
				break;
			}
		}
		_vm->renderImage(1 + 128);
		setSectionVisible(2, kShownFalse);
		_gm->drawStatus();
		_gm->drawInventory();
		_gm->drawMapExits();
		_gm->drawCommandBox();
	}
	else if (verb == ACTION_GIVE && obj2._id == AXACUSSER && _shown[kMaxSection - 2]) {
		_vm->renderImage(1);
		if (obj1._id != MONEY)
			notEnoughMoney();
		else {
			if (_gm->_state._money >= 10000) {
				if ((e = _gm->dialog(4, _gm->_dials, dial3, 0)) >= 2) {
					_gm->reply(kStringGoodOffer, 2, 1);
					_vm->playSound(kAudioSuccess);
					_gm->reply(kStringGiveCard, 2, 1);
					if (e == 2)
						_gm->takeMoney(-5000);
					else
						_gm->takeMoney(-10000);
					_gm->takeObject(*getObject(4));
					_vm->renderImage(1 + 128);
					setSectionVisible(2, false);
					_gm->reply(kStringIdiot, 0, 0);
					_shown[kMaxSection - 2] = false;
					_shown[kMaxSection - 3] = true;
					_gm->_rooms[CHECKOUT]->addSentence(1,1);
					_gm->drawStatus();
					_gm->drawInventory();
					_gm->drawMapExits();
					_gm->drawCommandBox();
				} else {
					notEnoughMoney();
					_gm->drawStatus();
					_gm->drawInventory();
					_gm->drawMapExits();
					_gm->drawCommandBox();
				}
			}
			else 
				notEnoughMoney();
		}
	}
	else 
		return false;
	return true;
}

Checkout::Checkout(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 21;
	_id = CHECKOUT;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringEntrance, kStringDefaultDescription, KP_ENTRANCE, EXIT, 0, 0, 0, NULLROOM, 3);
	_objectState[1] = Object(_id, kStringExit, kStringDefaultDescription, NULLOBJECT, EXIT, 255, 255, 0, CULTURE_PALACE, 22);
	_objectState[2] = Object(_id, kStringAxacussian, kStringDefaultDescription, AXACUSSER, TALK, 1, 1, 0);
}

void Checkout::onEntrance() {
	if (!_shown[kMaxSection - 3]) {
		_shown[kMaxSection - 3] = true;
		_gm->reply(kStringAtMusicContest, 1, 1 + 128);
		_gm->say(kStringNoImitation);
		_gm->reply(kStringGoodJoke, 1, 1 + 128);
		_gm->say(kStringIAmHorstHummel);
		_gm->reply(kStringCommon, 1, 1 + 128);
		_gm->say(kStringIWillProof);
		_gm->say(kStringIWillPerform);
		_gm->drawStatus();
		_gm->drawInventory();
		_gm->drawMapExits();
		_gm->drawCommandBox();
	}
	setRoomSeen(true);
}

void Checkout::animation() {
}

bool Checkout::interact(Action verb, Object &obj1, Object &obj2) {
	static StringId dialCheckout1[3] = {
		kStringCheckout1,
		kStringCheckout2,
		kStringCheckout3
	};
	static StringId dialCheckout2[2] = {
		kStringYes,
		kStringNo
	};
	static StringId dialStage1[3] = {
		kStringCheckout4,
		kStringCheckout5,
		kStringCheckout6
	};
	static StringId dialStage2[2] = {
		kStringCheckout7,
		kStringCheckout8
	};
	static StringId dialStage3[3] = {
		kStringCheckout9,
		kStringCheckout10,
		kStringCheckout11
	};
	static StringId dialStage4[3] = {
		kStringCheckout12,
		kStringCheckout13
	};
	if (verb == ACTION_WALK && obj1._id == KP_ENTRANCE) {
		if (_shown[kMaxSection - 4]) {
			_vm->renderImage(2);
			_gm->reply(kStringCheckout14, 0, 0);
			_vm->renderImage(2 + 128);
		} else if (_shown[kMaxSection - 2] == 0) {
			_vm->renderImage(2);
			//_gm->reply("atnuhh", 0, 0);
			_gm->reply(kStringCheckout15, 0, 0);
			_vm->renderImage(2 + 128);
			if (_gm->_rooms[CULTURE_PALACE]->getObject(4)->_type & CARRIED) {
				_gm->say(kStringCheckout16);
				_gm->reply(kStringCheckout17, 0, 0);
			} else
				_gm->say(kStringCheckout18);
			_gm->drawGUI();
		} else if (_shown[kMaxSection - 2] == 1) {
			_gm->reply(kStringCheckout19, 1, 1 + 128);
		} else {
			if (_gm->_state._tipsy) {
				_vm->setCurrentImage(22);
				_vm->renderImage(0);
				if (_shown[kMaxSection - 5] && _gm->_state._admission >= 2)
					appearance();
				else {
					_gm->dialog(3, _gm->_dials, dialStage1, 0);
					_gm->dialog(2, _gm->_dials, dialStage2, 0);
					_vm->renderMessage(kStringCheckout20, 100, 70);
					_gm->waitOnInput(_gm->_messageDuration);
					_vm->removeMessage();
					_vm->renderMessage(kStringCheckout21, 200, 40);
					_gm->waitOnInput(_gm->_messageDuration);
					_vm->removeMessage();
					_gm->say(kStringCheckout22);
					_gm->dialog(3, _gm->_dials, dialStage3, 0);
					_vm->renderMessage(kStringCheckout23, 120, 70);
					_gm->waitOnInput(_gm->_messageDuration);
					_vm->removeMessage();
					_gm->say(kStringCheckout24);
					_vm->renderMessage(kStringCheckout25, 40, 100);
					_gm->waitOnInput(_gm->_messageDuration);
					_vm->removeMessage();
					_gm->dialog(2, _gm->_dials, dialStage4, 0);
					_vm->playSound(kAudioStage1);
					while(_vm->_sound->isPlaying())
						_gm->wait(1);
					_vm->playSound(kAudioStage1);
					while(_vm->_sound->isPlaying())
						_gm->wait(1);
					_vm->playSound(kAudioStage2);
					while(_vm->_sound->isPlaying())
						_gm->wait(1);
					_vm->playSound(kAudioStage2);
					_vm->renderMessage(kStringCheckout26, 250, 80);
					while(_vm->_sound->isPlaying())
						_gm->wait(1);
					_vm->playSound(kAudioStage1);
					while(_vm->_sound->isPlaying())
						_gm->wait(1);
					_vm->removeMessage();
					_vm->playSound(kAudioStage2);
					_vm->renderMessage(kStringCheckout27, 140, 60);
					while(_vm->_sound->isPlaying())
						_gm->wait(1);
					_vm->playSound(kAudioStage2);
					while(_vm->_sound->isPlaying())
						_gm->wait(1);
					_gm->wait(2);
					_vm->removeMessage();
					_vm->playSound(kAudioStage2);
					_vm->renderMessage(kStringCheckout26, 180, 50);
					_gm->waitOnInput(_gm->_messageDuration);
					_vm->removeMessage();
					_vm->renderMessage(kStringCheckout28, 50, 110);
					_gm->waitOnInput(_gm->_messageDuration);
					_vm->removeMessage();
					_gm->say(kStringCheckout29);
					_vm->renderRoom(*this);
					if (_shown[kMaxSection - 5])
						_vm->renderMessage(kStringCheckout30);
					else
						_vm->renderMessage(kStringCheckout31);
					_shown[kMaxSection - 4] = true;
					_gm->drawGUI();
				}
			} else {
				_vm->renderMessage(kStringCheckout32);
				_gm->waitOnInput(_gm->_messageDuration);
				_vm->removeMessage();
				_vm->renderMessage(kStringCheckout33);
			}
		}
	} else if (verb == ACTION_GIVE && obj1._id == CHIP && obj2._id == AXACUSSER &&
			_shown[kMaxSection - 2] == 1) {
		_gm->_inventory.remove(obj1);
		_shown[kMaxSection - 5] = true;
		_gm->reply(kStringCheckout37, 1, 1 + 128);
		_shown[kMaxSection - 2] = 2;
		_gm->drawGUI();
	} else if (verb == ACTION_GIVE && obj1._id == CARD && obj2._id == AXACUSSER) {
		_gm->_inventory.remove(*_gm->_rooms[CULTURE_PALACE]->getObject(4));
		_gm->reply(kStringCheckout34, 1, 1 + 128);
		_gm->reply(kStringCheckout35, 1, 1 + 128);
		if (_gm->dialog(2, _gm->_dials, dialCheckout2, 0) == 1) {
			_gm->reply(kStringCheckout36, 1, 1 + 128);
			_shown[kMaxSection - 2] = 1;
		} else {
			_gm->reply(kStringCheckout37, 1, 1 + 128);
			_shown[kMaxSection - 2] = 2;
		}
		_gm->drawGUI();
	} else if (verb == ACTION_TALK && obj1._id == AXACUSSER) {
		if (_shown[kMaxSection - 4]) {
			_gm->say(kStringCheckout38);
			_gm->reply(kStringCheckout39, 1, 1 + 128);
			_gm->drawGUI();
		} else {
			switch (_shown[kMaxSection - 2]) {
			case 0:
				addSentence(2, 1);
				switch (_gm->dialog(3, _gm->_dials, dialCheckout1, 1)) {
				case 0:
					_gm->reply(kStringCheckout40, 1, 1 + 128);
					_gm->say(kStringNo);
					_gm->reply(kStringCheckout41, 1, 1 + 128);
					_gm->say(kStringCheckout42);
					break;
				case 1:
					_gm->reply(kStringCheckout43, 1, 1 + 128);
					if (_gm->_rooms[CULTURE_PALACE]->getObject(4)->_type & CARRIED) {
						_gm->say(kStringCheckout44);
						return interact(ACTION_GIVE,
								*_gm->_rooms[CULTURE_PALACE]->getObject(4),
								*_gm->_rooms[CHECKOUT]->getObject(2));
					} else {
						_gm->say(kStringNo);
						_gm->reply(kStringCheckout45, 1, 1 + 128);
						_gm->say(kStringCheckout46);
					}
					break;
				case 2:
					_gm->reply(kStringCheckout47, 1, 1 + 128);
					break;
				}
				_gm->drawGUI();
				break;
			case 1:
				_gm->reply(kStringCheckout48, 1, 1 + 128);
				break;
			case 2:
				_gm->reply(kStringCheckout49, 1, 1 + 128);
				break;
			}
		}
	} else 
		return false;
	return true;
}

void Checkout::appearance() {
	int xp = 0;
	_gm->playCD();
	_vm->removeMessage();
	_vm->playSound(kAudioAppearance1);
	while (_vm->_sound->isPlaying())
		_gm->wait(1);
	_vm->paletteFadeOut();

	_vm->setCurrentImage(39);
	_vm->renderImage(0);
	_vm->paletteFadeIn();
	_gm->wait(50);
	_vm->setCurrentImage(40);
	_vm->renderImage(0);
	for (int i = 1; i < 11; i++) {
		_gm->wait(3);
		_vm->renderImage(i);
	}
	_vm->playSound(kAudioAppearance2);
	_gm->wait(30);
	_vm->renderImage(11);
	_vm->playSound(kAudioAppearance2);
	_gm->wait(60);

	_vm->_system->fillScreen(kColorBlack);
	_vm->setCurrentImage(42);
	_vm->renderImage(0);
	_vm->renderImage(11);
	_gm->talk(1, 1 + 128, 0, kMessageLeft, kStringAppearance1);
	_gm->talk(1, 1 + 128, 0, kMessageLeft, kStringAppearance2);
	_vm->renderImage(4);
	_gm->wait(3);
	_vm->renderImage(6);
	_gm->talk(8, 6, 0, kMessageLeft, kStringAppearance3);
	_gm->talk(12, 13, 0, kMessageCenter, kStringAppearance4);
	_gm->talk(12, 13, 0, kMessageCenter, kStringAppearance5);
	_gm->talk(3, 3 + 128, 0, kMessageRight, kStringAppearance6);
	_gm->talk(12, 13, 0, kMessageCenter, kStringAppearance7);
	_gm->talk(8, 6, 0, kMessageLeft, kStringAppearance8);
	_gm->talk(12, 13, 4, kMessageCenter, kStringAppearance9);
	_vm->renderImage(4);
	_gm->talkRest(12, 13, 1);
	_vm->renderImage(4 + 128);
	_gm->talkRest(12, 13, 4);
	_vm->renderImage(4);
	_gm->talkRest(12, 13, 1);
	_vm->renderImage(6);
	_gm->talkRest(12, 13, _gm->_restTime + 6);
	_vm->removeMessage();
	_gm->talk(3, 3 + 128, 0, kMessageRight, kStringAppearance10);
	_gm->talk(12, 13, 0, kMessageCenter, kStringAppearance11);
	_gm->talk(12, 13, 0, kMessageCenter, kStringAppearance12);
	_gm->talk(3, 3 + 128, 0, kMessageRight, kStringAppearance13);
	_gm->talk(12, 13, 0, kMessageCenter, kStringAppearance14);
	_gm->talk(12, 13, 0, kMessageCenter, kStringAppearance15);
	_gm->talk(3, 3 + 128, 0, kMessageRight, kStringAppearance16);
	_gm->talk(12, 13, 2, kMessageCenter, kStringAppearance17);
	_vm->renderImage(4);
	_gm->talkRest(12, 13, 1);
	_vm->renderImage(4 + 128);
	_gm->talkRest(12, 13, 4);
	_vm->renderImage(4);
	_gm->talkRest(12, 13, 1);
	_vm->renderImage(6);
	_gm->talkRest(12, 13, _gm->_restTime + 6);
	_vm->removeMessage();
	_gm->talk(8, 6, 0, kMessageLeft, kStringAppearance18);
	_gm->talk(12, 13, 1, kMessageCenter, kStringAppearance19);
	_vm->renderImage(4);
	_gm->talkRest(12, 13, 1);
	_vm->renderImage(4 + 128);
	_gm->talkRest(12, 13, 4);
	_vm->renderImage(4);
	_gm->talkRest(12, 13, 1);
	_vm->renderImage(6);
	_gm->talkRest(12, 13, _gm->_restTime + 6);
	_vm->removeMessage();
	_gm->talk(8, 6, 0, kMessageLeft, kStringAppearance20);
	_gm->talk(12, 13, 1, kMessageCenter, kStringAppearance21);
	_vm->renderImage(17);
	_gm->wait(2);
	_vm->renderImage(18);
	_gm->wait(2);
	_vm->renderImage(19);
	_gm->wait(2);
	_vm->renderImage(20);
	_gm->wait(3);
	_vm->renderImage(21);
	_vm->renderImage(19);
	_gm->wait(1);
	_vm->renderImage(21+128);
	_vm->renderImage(22);
	_vm->renderImage(18);
	_gm->wait(1);
	_vm->renderImage(22+128);
	_vm->renderImage(23);
	_gm->wait(1);
	_vm->renderImage(23+128);
	_vm->renderImage(24);
	_vm->renderImage(17);
	_gm->wait(1);
	_vm->renderImage(24+128);
	_vm->renderImage(25);
	_gm->wait(1);
	_vm->renderImage(25+128);
	_vm->renderImage(32);
	_vm->renderImage(11);
	_vm->renderImage(26);
	_vm->playSound(kAudioAppearance3);
	_gm->wait(2);
	_vm->renderImage(32+128);
	_vm->renderImage(33);
	_vm->renderImage(27);
	_gm->wait(2);
	_vm->renderImage(33+128);
	_vm->renderImage(34);
	_vm->renderImage(28);
	_gm->wait(2);
	_vm->renderImage(29);
	_gm->wait(2);
	_vm->renderImage(30);
	_gm->wait(2);
	_vm->renderImage(31);
	_gm->wait(2);
	_gm->wait(50);
	_vm->paletteFadeOut();

	_vm->_system->fillScreen(kColorBlack);
	_vm->paletteFadeIn();
	_vm->renderMessage(kStringAppearance22);
	_gm->waitOnInput(_gm->_messageDuration);
	_vm->removeMessage();
	_vm->renderMessage(kStringAppearance23);
	_gm->waitOnInput(_gm->_messageDuration);
	_vm->removeMessage();
	_vm->renderMessage(kStringAppearance24);
	_gm->waitOnInput(_gm->_messageDuration);
	_vm->removeMessage();
	_vm->renderMessage(kStringAppearance25);
	_gm->waitOnInput(_gm->_messageDuration);
	_vm->removeMessage();
	_vm->renderMessage(kStringAppearance26);
	_gm->waitOnInput(_gm->_messageDuration);
	_vm->removeMessage();
	_vm->renderMessage(kStringAppearance27);
	_gm->waitOnInput(_gm->_messageDuration);
	_vm->removeMessage();
	_vm->setCurrentImage(44);
	_vm->renderImage(0);
	_vm->paletteFadeIn();

	MS2Image *image = _vm->_screen->getCurrentImage();

	int i = 0;
	do {
		_vm->renderImage(1);
		_gm->wait(1);
		_vm->renderImage(1 + 128);
		image->_section[1].x1 += xp;
		image->_section[1].x2 += xp;
		image->_section[1].y1 -= 2;
		image->_section[1].y2 -= 2;
		i++;
		if (i == 6) {
			i = 0;
			xp++;
		}
	} while (image->_section[1].y1 < 200);
	_vm->paletteFadeOut();
	_vm->_system->fillScreen(kColorBlack);
	_vm->paletteFadeIn();
	_vm->renderMessage(kStringAppearance28);
	_gm->waitOnInput(_gm->_messageDuration);
	_vm->removeMessage();
	_vm->renderMessage(kStringAppearance29);
	_gm->waitOnInput(_gm->_messageDuration);
	_vm->removeMessage();
	_vm->renderMessage(kStringAppearance30);
	_gm->waitOnInput(_gm->_messageDuration);
	_vm->removeMessage();
	_vm->renderMessage(kStringAppearance31);
	_gm->waitOnInput(_gm->_messageDuration);
	_vm->removeMessage();
	_gm->changeRoom(SHIP);
	_gm->_state._dark = true;
	_vm->_screen->setViewportBrightness(1);
	_vm->paletteBrightness();
	_gm->drawGUI();
}

City1::City1(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 23;
	_id = CITY1;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringSign, kStringSign1Description, NULLOBJECT, NULLTYPE, 2, 2, 0);
	_objectState[1] = Object(_id, kStringSign, kStringSign2Description, NULLOBJECT, NULLTYPE, 3, 3, 0);
	_objectState[2] = Object(_id, kStringDoor, kStringDefaultDescription, DOOR_L, EXIT | OPENABLE, 0, 0, 1, ELEVATOR, 10);
	_objectState[3] = Object(_id, kStringDoor, kStringDefaultDescription, DOOR_R, EXIT | OPENABLE, 1, 1, 2, ELEVATOR, 14);
}

void City1::onEntrance() {
	setRoomSeen(true);
}

void City1::animation() {
}

bool City1::interact(Action verb, Object &obj1, Object &obj2) {
	if (verb == ACTION_OPEN && obj1._id == DOOR_L) {
		_objectState[0]._click = 255;
	} else if (verb == ACTION_CLOSE && obj1._id == DOOR_L) {
		_objectState[0]._click = 2;
	} else if (verb == ACTION_OPEN && obj1._id == DOOR_R) {
		_objectState[1]._click = 255;
	} else if (verb == ACTION_CLOSE && obj1._id == DOOR_R) {
		_objectState[1]._click = 3;
	} else if (verb == ACTION_WALK && obj1._id == DOOR_L) {
		_gm->_state._elevatorNumber = 1;
	} else if (verb == ACTION_WALK && obj1._id == DOOR_R) {
		_gm->_state._elevatorNumber = 2;
	}
	_gm->_rooms[ELEVATOR]->getObject(5)->_exitRoom = CITY1;
	_gm->_state._elevatorE = 0;
	return false;
}

City2::City2(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 23;
	_id = CITY2;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringSign, kStringSign3Description, NULLOBJECT, NULLTYPE, 2, 2, 0);
	_objectState[1] = Object(_id, kStringSign, kStringSign4Description, NULLOBJECT, NULLTYPE, 3, 3, 0);
	_objectState[2] = Object(_id, kStringDoor, kStringDefaultDescription, DOOR_L, EXIT | OPENABLE, 0, 0, 1, ELEVATOR, 10);
	_objectState[3] = Object(_id, kStringDoor, kStringDefaultDescription, DOOR_R, EXIT | OPENABLE, 1, 1, 2, ELEVATOR, 14);
}

void City2::onEntrance() {
	setRoomSeen(true);
}

void City2::animation() {
}

bool City2::interact(Action verb, Object &obj1, Object &obj2) {
	if (verb == ACTION_OPEN && obj1._id == DOOR_L) {
		_objectState[0]._click = 255;
	} else if (verb == ACTION_CLOSE && obj1._id == DOOR_L) {
		_objectState[0]._click = 2;
	} else if (verb == ACTION_OPEN && obj1._id == DOOR_R) {
		_objectState[1]._click = 255;
	} else if (verb == ACTION_CLOSE && obj1._id == DOOR_R) {
		_objectState[1]._click = 3;
	} else if (verb == ACTION_WALK && obj1._id == DOOR_L) {
		_gm->_state._elevatorNumber = 3;
	} else if (verb == ACTION_WALK && obj1._id == DOOR_R) {
		_gm->_state._elevatorNumber = 4;
	}
	_gm->_rooms[ELEVATOR]->getObject(5)->_exitRoom = CITY2;
	_gm->_state._elevatorE = 0;
	return false;
}

Elevator::Elevator(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 24;
	_id = ELEVATOR;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringSlot, kStringDefaultDescription, SLOT, COMBINABLE, 0, 0, 0);
	_objectState[1] = Object(_id, kStringBell, kStringDefaultDescription, BELL, PRESS, 1, 1, 0);
	_objectState[2] = Object(_id, kStringDisplay, kStringDefaultDescription, DISPLAY, NULLTYPE, 2, 2, 0);
	_objectState[3] = Object(_id, kStringKeypad, kStringKeypadDescription, KEYPAD, PRESS, 3, 3, 0);
	_objectState[4] = Object(_id, kStringDoor, kStringDefaultDescription, NULLOBJECT, EXIT | OPENABLE | CLOSED, 4, 4, 0, APARTMENT, 3);
	_objectState[5] = Object(_id, kStringExit, kStringDefaultDescription, NULLOBJECT, EXIT | OPENABLE | OPENED | CLOSED, 255, 255, 0, CITY1, 22);
}

void Elevator::onEntrance() {
	if (_gm->_state._elevatorE)
		_objectState[5]._type &= ~OPENED;
	else
		_objectState[5]._type |= OPENED;
	setRoomSeen(true);
}

void Elevator::animation() {
}

bool Elevator::interact(Action verb, Object &obj1, Object &obj2) {
	static StringId dialLuke1[4] = {
		kStringElevator2,
		kStringElevator3,
		kStringElevator4,
		kStringElevator5
	};
	static byte dialsLuke1[3] = {2,1,1};
	static StringId dialBoss1[2] = {
		kStringElevator6,
		kStringElevator7
	};
	Common::String input;

	if (verb == ACTION_LOOK && obj1._id == DISPLAY) {
		Common::String format = _vm->getGameString(kStringElevator1);
		Common::String display = 
			Common::String::format(format.c_str(), _gm->_state._elevatorE);
		_vm->renderMessage(display);
	} else if (verb == ACTION_PRESS && obj1._id == BELL) {
		_vm->renderImage(8);
		_vm->playSound(kAudioElevatorBell);
		while(_vm->_sound->isPlaying())
			_gm->wait(1);
		_vm->renderImage(8 + 128);
		if (_gm->_state._elevatorNumber == 4 && _gm->_state._elevatorE == 4 && !_gm->_state._toMuseum) {
			_gm->wait(18);
			_vm->renderImage(1);
			_gm->wait(3);
			_vm->renderImage(2);
			setSectionVisible(1, kShownFalse);
			_gm->wait(3);
			_vm->renderImage(3);
			setSectionVisible(2, kShownFalse);
			_gm->reply(kStringWhatYouWant, 4, 3);
			switch (_gm->dialog(3, dialsLuke1, dialLuke1, 1)) {
			case 0:
				_gm->reply(kStringElevator11, 4, 3);
				_vm->renderImage(2);
				setSectionVisible(3, kShownFalse);
				_gm->wait(3);
				_vm->renderImage(1);
				setSectionVisible(2, kShownFalse);
				_gm->wait(3);
				_vm->renderImage(1 + 128);
				_vm->renderMessage(kStringElevator12);
				_gm->waitOnInput(_gm->_messageDuration);
				_vm->removeMessage();
				_vm->renderImage(1);
				_gm->wait(3);
				_vm->renderImage(2);
				setSectionVisible(1, kShownFalse);
				_gm->wait(3);
				_vm->renderImage(3);
				setSectionVisible(2, kShownFalse);
				_gm->reply(kStringElevator13, 4, 3);
				setSectionVisible(3, kShownFalse);
				setSectionVisible(4, kShownFalse);
				_vm->paletteFadeOut();
				_vm->_system->fillScreen(kColorBlack);
				_vm->paletteFadeIn();
				_vm->renderMessage(kStringElevator14);
				_gm->waitOnInput(_gm->_messageDuration);
				_vm->removeMessage();
				_vm->paletteFadeOut();
				_vm->paletteFadeOut();
				_vm->setCurrentImage(26);
				_vm->renderImage(0);
				_vm->paletteFadeIn();
				_gm->reply(kStringElevator15, 1, 1 + 128);
				_gm->say(kStringYes);
				_gm->reply(kStringElevator16, 1, 1 + 128);
				_gm->reply(kStringElevator17, 1, 1 + 128);
				if (_gm->dialog(2, _gm->_dials, dialBoss1, 0)) {
					_gm->reply(kStringElevator18, 1, 1 + 128);
					_gm->reply(kStringElevator19, 1, 1 + 128);
					_gm->say(kStringElevator20);
				}
				_gm->reply(kStringElevator21, 1, 1 + 128);
				_gm->reply(kStringElevator22, 1, 1 + 128);
				_gm->reply(kStringElevator23, 1, 1 + 128);
				_gm->reply(kStringElevator24, 1, 1 + 128);
				_gm->reply(kStringElevator25, 1, 1 + 128);
				_gm->reply(kStringElevator26, 1, 1 + 128);
				_gm->reply(kStringElevator27, 1, 1 + 128);
				_gm->reply(kStringElevator28, 1, 1 + 128);
				jobDescription();
				return true;
			case 1:
				_gm->reply(kStringElevator59, 4, 3);
				_vm->renderImage(2);
				setSectionVisible(3, kShownFalse);
				setSectionVisible(4, kShownFalse);
				_gm->wait(3);
				_vm->renderImage(1);
				setSectionVisible(2, kShownFalse);
				_vm->renderImage(1 + 128);
				break;
			case 2:
				_gm->reply(kStringElevator60, 4, 3);
				_vm->renderImage(2);
				setSectionVisible(3, kShownFalse);
				setSectionVisible(4, kShownFalse);
				_gm->wait(3);
				_vm->renderImage(1);
				setSectionVisible(2, kShownFalse);
				_vm->renderImage(1 + 128);
				break;
			}
			_gm->drawGUI();
		}
		else
			_vm->renderMessage(kStringElevator61);
	} else if ((verb == ACTION_USE || verb == ACTION_PRESS) && obj1._id == KEYPAD) {
		_vm->renderMessage(kStringElevator62);
		do {
			_gm->edit(input, 237, 66, 2);
		} while ((_gm->_key.keycode != Common::KEYCODE_RETURN) && 
				 (_gm->_key.keycode != Common::KEYCODE_ESCAPE) && !_vm->shouldQuit());
		_vm->removeMessage();
		if (_gm->_key.keycode == Common::KEYCODE_RETURN && input[0] != 0) {
			for (unsigned i = 0; i < input.size(); i++) {
				if (input[i] < '0' || input[i] > '9') {
					_vm->renderMessage(kStringElevator63);
					return true;
				}
			}
			int64 number = input.asUint64();
			if (number > 60)
				_vm->renderMessage(kStringElevator63);
			else if (number != _gm->_state._elevatorE) {
				if (isSectionVisible(6)) {
					_vm->renderImage(6 + 128);
					_objectState[4]._type &= ~OPENED;
					_vm->playSound(kAudioElevator1);
				}
				_vm->renderMessage(kStringElevator64);
				_gm->_state._elevatorE = number;
				if (number)
					_objectState[5]._type &= ~OPENED;
				else
					_objectState[5]._type |= OPENED;
			}
		}
	} else if (verb == ACTION_USE && Object::combine(obj1, obj2, ID_CARD, SLOT)) {
		if (_gm->_state._elevatorNumber == 1 && _gm->_state._elevatorE == 32) {
			_vm->renderImage(6);
			_objectState[4]._type |= OPENED;
			_vm->playSound(kAudioTaxiOpen);
		}
		else
			_vm->renderMessage(kStringElevator65);
	} else
		return false;
	return true;
}

void Elevator::jobDescription() {
	static StringId dialBoss2[5] = {
		kStringElevator8,
		kStringElevator9,
		kStringElevator10
	};
	byte dialsBoss2[4] = {1,1,2,1};

	_gm->reply(kStringElevator29, 1, 1 + 128);
	_gm->reply(kStringElevator30, 1, 1 + 128);
	_gm->reply(kStringElevator31, 1, 1 + 128);
	_gm->reply(kStringElevator32, 1, 1 + 128);
	_vm->setCurrentImage(30);
	_vm->renderImage(0);
	_gm->waitOnInput(72);
	_gm->reply(kStringElevator33, 0, 0);
	_gm->reply(kStringElevator34, 0, 0);
	_gm->reply(kStringElevator35, 0, 0);
	_gm->reply(kStringElevator36, 0, 0);
	_gm->reply(kStringElevator37, 0, 0);
	_gm->reply(kStringElevator38, 0, 0);
	_gm->reply(kStringElevator39, 0, 0);
	_gm->reply(kStringElevator40, 0, 0);
	_gm->reply(kStringElevator41, 0, 0);
	_gm->reply(kStringElevator42, 0, 0);
	_gm->reply(kStringElevator43, 0, 0);
	_gm->reply(kStringElevator44, 0, 0);
	_gm->reply(kStringElevator45, 0, 0);
	_gm->reply(kStringElevator46, 0, 0);
	_gm->reply(kStringElevator47, 0, 0);
	_gm->reply(kStringElevator48, 0, 0);
	_vm->setCurrentImage(26);
	_vm->_system->fillScreen(kColorBlack);
	_vm->renderImage(0);
	_gm->reply(kStringElevator49, 1, 1 + 128);
	int e;
	do {
		addSentence(0, 2);
		switch (e = _gm->dialog(4, dialsBoss2, dialBoss2, 2)) {
		case 0:
			_gm->reply(kStringElevator50, 1, 1 + 128);
			jobDescription();
			return;
		case 1:
			_gm->reply(kStringElevator51, 1, 1 + 128);
			break;
		case 2:
			_gm->reply(kStringElevator52, 1, 1 + 128);
			break;
		}
		if (e == 1 || e == 2)
			_gm->reply(kStringElevator53, 1, 1 + 128);
	} while (e != 3);
	_gm->reply(kStringElevator54, 1, 1 + 128);
	_vm->paletteFadeOut();
	_vm->_system->fillScreen(kColorBlack);
	_vm->paletteFadeIn();
	_vm->renderMessage(kStringElevator55);
	_gm->waitOnInput(_gm->_messageDuration);
	_vm->removeMessage();
	_vm->paletteFadeOut();
	_vm->saveGame(kSleepAutosaveSlot, "autosave");
	_gm->_inventory.clear();
	_gm->takeObject(*_gm->_rooms[INTRO]->getObject(3));
	_gm->takeObject(*_gm->_rooms[INTRO]->getObject(5));
	_gm->takeObject(*_gm->_rooms[INTRO]->getObject(6));
	_gm->takeObject(*_gm->_rooms[INTRO]->getObject(8));
	_vm->setCurrentImage(29);
	_gm->changeRoom(MUSEUM);
	_vm->renderImage(0);
	_vm->paletteFadeIn();
	_vm->renderMessage(kStringElevator56);
	_gm->waitOnInput(_gm->_messageDuration);
	_vm->removeMessage();
	_vm->renderMessage(kStringElevator57);
	_gm->waitOnInput(_gm->_messageDuration);
	_vm->removeMessage();
	_vm->renderMessage(kStringElevator58);
	_gm->drawGUI();
	_gm->_state._startTime = g_system->getMillis() - 2390000;
	_gm->_state._tipsy = false;
	_gm->_state._toMuseum = true;
}

Apartment::Apartment(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 25;
	_id = APARTMENT;
	_shown[0] = kShownTrue;
	_shown[1] = kShownFalse;
	_shown[2] = kShownFalse;
	_shown[3] = kShownTrue;

	_objectState[0] = Object(_id, kStringChip, kStringChipDescription, CHIP, TAKE | COMBINABLE, 255, 255, 1);
	_objectState[1] = Object(_id, kStringHatch, kStringHatchDescription, HATCH, OPENABLE | CLOSED | COMBINABLE, 0, 1, 1);
	_objectState[2] = Object(_id, kStringDefaultDescription, kStringDefaultDescription, NULLOBJECT, NULLTYPE, 255, 255, 0);
	_objectState[3] = Object(_id, kStringMusicSystem, kStringMusicSystemDescription, MUSIC_SYSTEM, COMBINABLE, 4, 4, 0);
	_objectState[4] = Object(_id, kStringSpeakers, kStringSpeakersDescription, NULLOBJECT, NULLTYPE, 5, 5, 0);
	_objectState[5] = Object(_id, kStringPencils, kStringPencilsDescription, NULLOBJECT, UNNECESSARY, 6, 6, 0);
	_objectState[6] = Object(_id, kStringMetalBlocks, kStringMetalBlocksDescription, MAGNET, TAKE | COMBINABLE, 10, 10, 3 + 128);
	_objectState[7] = Object(_id, kStringImage, kStringImageDescription, NULLOBJECT, UNNECESSARY, 7, 7, 0);
	_objectState[8] = Object(_id, kStringCabinet, kStringCabinetDescription, CABINET, OPENABLE | CLOSED, 8, 8, 0);
	_objectState[9] = Object(_id, kStringChair, kStringDefaultDescription, NULLOBJECT, NULLTYPE, 9, 9, 0);
	_objectState[10] = Object(_id, kStringElevator, kStringDefaultDescription, NULLOBJECT, EXIT, 255, 255, 0, ELEVATOR, 22);
	_objectState[11] = Object(_id, kStringUnderBed, kStringUnderBedDescription, UNDER_BED, NULLTYPE, 11, 11, 0);
	_objectState[12] = Object(_id, kStringKey, kStringKeyDescription, KEY, TAKE | COMBINABLE, 255, 255, 0);
}

void Apartment::onEntrance() {
	setRoomSeen(true);
}

void Apartment::animation() {
}

bool Apartment::interact(Action verb, Object &obj1, Object &obj2) {
	if (verb == ACTION_USE && Object::combine(obj1, obj2, ROD, UNDER_BED)) {
		if (_objectState[12]._type & CARRIED)
			_vm->renderMessage(kStringApartment1);
		else if (_shown[kMaxSection - 1]) {
			_vm->renderMessage(kStringApartment2);
			_gm->takeObject(_objectState[12]);
			_vm->playSound(kAudioSuccess);
		} else {
			_vm->renderMessage(kStringApartment3);
			_gm->waitOnInput(_gm->_messageDuration);
			_vm->removeMessage();
			_vm->renderMessage(kStringApartment4);
		}
	} else if (verb == ACTION_USE && Object::combine(obj1, obj2, KEY, HATCH)) {
		if (_objectState[1]._type & OPENED)
			_vm->renderMessage(kStringApartment5);
		else {
			_vm->renderImage(1);
			_vm->playSound(kAudioTaxiOpen);
			_objectState[1]._type |= OPENED;
			_objectState[1]._click = 1;
			if (!(_objectState[0]._type & CARRIED)) {
				_vm->renderImage(2);
				_objectState[0]._click = 2;
			}
		}
	} else if (verb == ACTION_CLOSE && obj1._id == HATCH && obj1._type & OPENED) {
		_vm->renderImage(1 + 128);
		setSectionVisible(2, false);
		_vm->playSound(kAudioElevator1);
		obj1._type &= ~OPENED;
		obj1._click = 0;
		if (!(_objectState[0]._type & CARRIED))
			_objectState[0]._click = 255;
	} else if (verb == ACTION_TAKE && obj1._id == CHIP && !(obj1._type & CARRIED)) {
		setSectionVisible(2, kShownFalse);
		return false;
	} else if (verb == ACTION_USE && Object::combine(obj1, obj2, KEY, CABINET)) {
		_vm->renderMessage(kStringApartment6);
	} else if (verb == ACTION_USE && Object::combine(obj1, obj2, CHIP, MUSIC_SYSTEM)) {
		_vm->renderMessage(kStringApartment7);
	} else if (verb == ACTION_PRESS && obj1._id == MUSIC_SYSTEM) {
		_vm->renderMessage(kStringApartment8);
	} else
		return false;
	return true;
}

Ship::Ship(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 45;
	_id = SHIP;
	_shown[0] = kShownTrue;
	_shown[1] = kShownFalse;
	_shown[2] = kShownFalse;
	_shown[3] = kShownFalse;
	_shown[4] = kShownFalse;
	_shown[5] = kShownFalse;
	_shown[6] = kShownFalse;
	_shown[7] = kShownFalse;
	_shown[8] = kShownTrue;

	_objectState[0] = Object(_id, kStringSwitch, kStringDefaultDescription, SWITCH, PRESS | COMBINABLE, 0, 0, 0);
	_objectState[1] = Object(_id, kStringHandle, kStringDefaultDescription, HANDLE, NULLTYPE, 255, 255, 0);
	_objectState[2] = Object(_id, kStringHatch2, kStringDefaultDescription, NULLOBJECT, NULLTYPE, 255, 255, 0, SHIP, 19);
	_objectState[3] = Object(_id, kStringSwitch, kStringDefaultDescription, DOOR_SWITCH, PRESS | COMBINABLE, 255, 255, 0);
	_objectState[4] = Object(_id, kStringSpaceSuit, kStringSpaceSuitDescription, SUIT, TAKE, 255, 255, 1);
	_objectState[5] = Object(_id, kStringCable, kStringCableDescription1, RCABLE, COMBINABLE, 255, 255, 0);
	_objectState[6] = Object(_id, kStringCable, kStringCableDescription2, CABLE, TAKE | COMBINABLE, 255, 255, 8 + 128);
}

void Ship::onEntrance() {
	setRoomSeen(true);
}

void Ship::animation() {
}

bool Ship::interact(Action verb, Object &obj1, Object &obj2) {
	if (verb == ACTION_PRESS && obj1._id == SWITCH) {
		if (_gm->_state._dark) {
			_gm->_state._dark = false;
			_vm->_screen->setViewportBrightness(255);
			_objectState[1]._click = 1;
			_objectState[2]._click = 2;
			_objectState[3]._click = 3;
			_objectState[5]._click = 4;
			_objectState[6]._click = 5;
			_objectState[2]._type = EXIT | OPENABLE | CLOSED;
			_gm->drawMapExits();
		}
	} else if (verb == ACTION_PRESS && obj1._id == DOOR_SWITCH &&
			!isSectionVisible(7)) {
		_vm->renderImage(3);
		_gm->wait(2);
		_vm->renderImage(6);
		_vm->playSound(kAudioShip1);
		while (_vm->_sound->isPlaying())
			_gm->wait(1);
		_vm->renderImage(6 + 128);
		_vm->renderImage(7);
		_objectState[3]._description = kStringShip1;
		_objectState[2]._description = kStringShip2;
	} else if (verb == ACTION_PULL && obj1._id == HANDLE) {
		_vm->renderImage(2);
		_vm->playSound(kAudioTaxiOpen);
		obj1._click = 255;
		_objectState[4]._click = 9;
	} else if (verb == ACTION_TAKE && obj1._id == SUIT && !(obj1._type & CARRIED)) {
		setSectionVisible(2, kShownFalse);
		_gm->takeObject(obj1);
	} else if (verb == ACTION_USE && obj1._id == SUIT) {
		if (!(obj1._type & CARRIED)) {
			setSectionVisible(2, kShownFalse);
			_gm->takeObject(obj1);
		}
		if ((_shown[kMaxSection - 1] = !_shown[kMaxSection - 1]))
			_vm->renderMessage(kStringShip3);
		else
			_vm->renderMessage(kStringShip4);
	} else if (verb == ACTION_USE && Object::combine(obj1, obj2, CABLE, RCABLE)) {
		_objectState[6]._description = kStringDefaultDescription;
		if (_objectState[6]._click == 6)
			_vm->renderImage(8 + 128);
		if (_objectState[6]._type & CARRIED)
			_gm->_inventory.remove(_objectState[6]);
		if (isSectionVisible(11) || isSectionVisible(10))
			_vm->renderMessage(kStringShip5);
		else if (isSectionVisible(9)) {
			_vm->renderImage(9 + 128);
			_vm->renderImage(11);
			if (!_shown[kMaxSection - 1])
				kill();
			_objectState[6]._click = 8;
			_gm->wait(2);
			_vm->renderImage(4);
			_vm->playSound(kAudioShip2);
			_gm->wait(3);
			_vm->renderImage(5);
			_objectState[2]._type |= OPENED;
			_objectState[2]._description = kStringDefaultDescription;
		} else {
			_vm->renderImage(10);
			if (!_shown[kMaxSection - 1])
				kill();
			_objectState[6]._click = 7;
		}
	} else if (verb == ACTION_USE && Object::combine(obj1, obj2, CABLE, DOOR_SWITCH) && isSectionVisible(7)) {
		_objectState[6]._description = kStringDefaultDescription;
		if (_objectState[6]._click == 5)
			_vm->renderImage(8 + 128);
		if (_objectState[6]._type & CARRIED)
			_gm->_inventory.remove(_objectState[6]);
		if (isSectionVisible(11) || isSectionVisible(9))
			_vm->renderMessage(kStringShip5);
		else if (isSectionVisible(10)) {
			_vm->renderImage(10 + 128);
			_vm->renderImage(11);
			if (!_shown[kMaxSection - 1])
				kill();
			_objectState[6]._click = 8;
			_gm->wait(2);
			_vm->renderImage(4);
			_vm->playSound(kAudioShip2);
			_gm->wait(3);
			_vm->renderImage(5);
			_objectState[2]._type |= OPENED;
			_objectState[2]._description = kStringDefaultDescription;
		} else {
			_vm->renderImage(9);
			_objectState[6]._click = 6;
		}
	} else if (verb == ACTION_TAKE && obj1._id == CABLE && obj1._type & CARRIED) {
		if (isSectionVisible(8)) {
			obj1._description = kStringDefaultDescription;
			_gm->takeObject(obj1);
		} else
			_vm->renderMessage(kStringShip6);
	} else if (verb == ACTION_WALK && obj1._type == (EXIT | OPENABLE | CLOSED | OPENED)) {
		_vm->setCurrentImage(46);
		_vm->renderImage(0);
		_gm->wait(16);
		_vm->renderMessage(kStringShip7, kMessageRight);
		_gm->waitOnInput(_gm->_messageDuration);
		_vm->removeMessage();
		_gm->reply(kStringShip8, 1, 1 + 128);
		_vm->renderMessage(kStringShip9, kMessageRight);
		_gm->waitOnInput(_gm->_messageDuration);
		_vm->removeMessage();
		_gm->reply(kStringShip10, 1, 1 + 128);
		_gm->reply(kStringShip11, 1, 1 + 128);
		_gm->reply(kStringShip12, 1, 1 + 128);
		_gm->reply(kStringShip13, 1, 1 + 128);
		_vm->renderMessage(kStringShip14, kMessageRight);
		_gm->waitOnInput(_gm->_messageDuration);
		_vm->removeMessage();
		_gm->reply(kStringShip15, 1, 1 + 128);
		_vm->renderMessage(kStringShip16, kMessageRight);
		_gm->waitOnInput(_gm->_messageDuration);
		_vm->removeMessage();
		_gm->reply(kStringAha, 1, 1 + 128);
		_gm->reply(kStringShip17, 1, 1 + 128);
		_vm->renderMessage(kStringShip18, kMessageRight);
		_gm->waitOnInput(_gm->_messageDuration);
		_vm->removeMessage();
		_gm->reply(kStringShip19, 1, 1 + 128);
		_gm->wait(16);
		CursorMan.showMouse(false);
		_vm->renderImage(2);
		for (int i = 3; i < 12; i++) {
			_vm->renderImage(i);
			if (i == 9)
				_vm->playSound(kAudioShip3);
			_gm->wait(2);
			_vm->renderImage(i + 128);
		}
		_vm->renderImage(12);
		_gm->wait(18);
		// TODO some palette stuff
		_vm->renderImage(13);
		_vm->playSound(kMusicMadMonkeys);
	} else
		return false;
	return true;
}

void Ship::kill() {
	_vm->playSound(kAudioShipDeath);
	while (_vm->_sound->isPlaying())
		_gm->wait(1);
	_gm->dead(kStringShip0);
}

Pyramid::Pyramid(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 8;
	_id = PYRAMID;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringRope, kStringDefaultDescription, ROPE, TAKE | COMBINABLE, 255, 255, 1 + 128);
	_objectState[1] = Object(_id, kStringSign, kStringSignDescription, SIGN, COMBINABLE, 25, 25, 0);
	_objectState[2] = Object(_id, kStringEntrance, kStringEntrance1Description, PYRA_ENTRANCE, EXIT, 27, 27, 0, PYR_ENTRANCE, 7);
	_objectState[3] = Object(_id, kStringPyramid, kStringPyramidDescription, NULLOBJECT, NULLTYPE, 26, 26, 0);
	_objectState[4] = Object(_id, kStringSun, kStringSunDescription, SUN, NULLTYPE, 28, 28, 0);
	_objectState[5] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE1, COMBINABLE, 0, 0, 0);
	_objectState[6] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE2, COMBINABLE, 1, 0, 0);
	_objectState[7] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE3, COMBINABLE, 2, 0, 0);
	_objectState[8] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE4, COMBINABLE, 3, 0, 0);
	_objectState[9] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE5, COMBINABLE, 4, 0, 0);
	_objectState[10] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE6, COMBINABLE, 5, 0, 0);
	_objectState[11] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE7, COMBINABLE, 6, 0, 0);
	_objectState[12] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE8, COMBINABLE, 7, 0, 0);
	_objectState[13] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE9, COMBINABLE, 8, 0, 0);
	_objectState[14] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE10, COMBINABLE, 9, 0, 0);
	_objectState[15] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE11, COMBINABLE, 10, 0, 0);
	_objectState[16] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE12, COMBINABLE, 11, 0, 0);
	_objectState[17] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE13, COMBINABLE, 12, 0, 0);
	_objectState[18] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE14, COMBINABLE, 13, 0, 0);
	_objectState[19] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE15, COMBINABLE, 14, 0, 0);
	_objectState[20] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE16, COMBINABLE, 15, 0, 0);
	_objectState[21] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE17, COMBINABLE, 16, 0, 0);
	_objectState[22] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE18, COMBINABLE, 17, 0, 0);
	_objectState[23] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE19, COMBINABLE, 18, 0, 0);
	_objectState[24] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE20, COMBINABLE, 19, 0, 0);
	_objectState[25] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE21, COMBINABLE, 20, 0, 0);
	_objectState[26] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE22, COMBINABLE, 21, 0, 0);
	_objectState[27] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE23, COMBINABLE, 22, 0, 0);
	_objectState[28] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE24, COMBINABLE, 23, 0, 0);
	_objectState[29] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE25, COMBINABLE, 24, 0, 0);
}

void Pyramid::onEntrance() {
	setRoomSeen(true);
}

void Pyramid::animation() {
}

bool Pyramid::interact(Action verb, Object &obj1, Object &obj2) {
	Object *rope, *hole;
	rope = hole = nullptr;

	if (obj1._id == ROPE)
		rope = &obj1;
	if (obj2._id == ROPE)
		rope = &obj2;
	if (obj1._id >= HOLE1 && obj1._id <= HOLE25)
		hole = &obj1;
	if (obj2._id >= HOLE1 && obj2._id <= HOLE25)
		hole = &obj2;

	if (verb == ACTION_WALK && obj1._id == PYRA_ENTRANCE) {
		_gm->_state._eventCallback = kPyramidEndFn;
		_gm->_state._eventTime = g_system->getMillis() + 3600000; //hour
		_gm->_state._pyraS = 4;
		_gm->_state._pyraZ = 10;
		_gm->_state._pyraDirection = 0;
		_gm->passageConstruction();
		return false;
	} else if (verb == ACTION_USE && Object::combine(obj1, obj2, ROPE, SIGN)) {
		if (rope->_type & CARRIED) {
			_vm->renderImage(1);
			_gm->_inventory.remove(*rope);
			_objectState[0]._click = 29;
		}
		else
			_vm->renderMessage(kStringPyramid1);
	} else if (verb == ACTION_USE && hole != nullptr && rope != nullptr) {
		if (rope->_type & CARRIED)
			_vm->renderMessage(kStringPyramid2);
		else {
			int number = hole->_id - HOLE1;
			int start = number / 5 * 5 + 2;
			for (int i = 1; i <= 26; i++)
				_vm->renderImage(i + 128);
			for (int i = start; i <= start + number % 5; i++)
				_vm->renderImage(i);
			_objectState[0]._click = 30;
			if (number == 18) {
				_gm->_rooms[HOLE_ROOM]->setSectionVisible(16, kShownTrue);
				_gm->_rooms[HOLE_ROOM]->getObject(2)->_click = 5;
				_gm->_rooms[HOLE_ROOM]->getObject(3)->_type = EXIT;
			} else {
				_gm->_rooms[HOLE_ROOM]->setSectionVisible(16, kShownFalse);
				_gm->_rooms[HOLE_ROOM]->getObject(2)->_click = 255;
				_gm->_rooms[HOLE_ROOM]->getObject(3)->_type = NULLTYPE;
			}
		}
	} else if (verb == ACTION_PULL && rope != nullptr && !(rope->_type & CARRIED) &&
			!isSectionVisible(1)) {
		for (int i = 2; i <= 26; i++)
			_vm->renderImage(i + 128);
		_vm->renderImage(1);
		_objectState[0]._click = 29;
		_gm->_rooms[HOLE_ROOM]->setSectionVisible(16, kShownFalse);
		_gm->_rooms[HOLE_ROOM]->getObject(2)->_click = 255;
		_gm->_rooms[HOLE_ROOM]->getObject(3)->_type = NULLTYPE;
	} else if (verb == ACTION_TAKE && rope != nullptr && !(rope->_type & CARRIED)) {
		for (int i = 2; i <= 26; i++)
			_vm->renderImage(i + 128);
		_gm->takeObject(*rope);
		_gm->_rooms[HOLE_ROOM]->setSectionVisible(16, kShownFalse);
		_gm->_rooms[HOLE_ROOM]->getObject(2)->_click = 255;
		_gm->_rooms[HOLE_ROOM]->getObject(3)->_type = NULLTYPE;
	} else if (verb == ACTION_WALK && obj1._id == SUN) {
		_vm->renderMessage(kStringPyramid3);
	} else
		return false;
	return true;
}

PyrEntrance::PyrEntrance(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 9;
	_id = PYR_ENTRANCE;
	_shown[0] = kShownTrue;
	_shown[1] = kShownFalse;
	_shown[2] = kShownFalse;
	_shown[3] = kShownFalse;
	_shown[4] = kShownTrue;
	_shown[5] = kShownFalse;
	_shown[6] = kShownFalse;
	_shown[7] = kShownFalse;
	_shown[8] = kShownTrue;

	_objectState[0] = Object(_id, kStringSign, kStringSign5Description, SIGN, NULLTYPE, 255, 255, 0);
	_objectState[1] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 1, 1, 0, PYR_ENTRANCE, 14);
	_objectState[2] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 2, 2, 0, PYR_ENTRANCE, 10);
	_objectState[3] = Object(_id, kStringCorridor, kStringDefaultDescription, CORRIDOR, EXIT, 0, 0, 0, PYR_ENTRANCE, 2);
}

void PyrEntrance::onEntrance() {
	if (_gm->_state._pyraS != 8 || _gm->_state._pyraZ != 5)
		_waitTime = 0;
}

void PyrEntrance::animation() {
	if (_gm->_state._pyraS == 8 && _gm->_state._pyraZ == 5) {
		if (_waitTime == 700) { // around 1 minute
			_vm->renderMessage(kStringPyramid4);
			_gm->waitOnInput(_gm->_messageDuration);
			_vm->removeMessage();
			_gm->_state._pyraZ++;
			_gm->_state._pyraDirection = 0;
			_gm->changeRoom(FLOORDOOR);
			_vm->setCurrentImage(14);
			_vm->renderRoom(*_gm->_rooms[FLOORDOOR]);
			_gm->drawMapExits();
			_gm->wait(3);
			_vm->renderImage(5);
			_gm->wait(3);
			_vm->renderImage(6);
			_gm->_rooms[FLOORDOOR]->setSectionVisible(5, kShownFalse);
			_gm->wait(3);
			_vm->renderImage(7);
			_gm->_rooms[FLOORDOOR]->setSectionVisible(6, kShownFalse);
			_gm->wait(3);
			_vm->renderImage(8);
			_gm->_rooms[FLOORDOOR]->setSectionVisible(7, kShownFalse);
			_gm->wait(3);
			_vm->renderImage(9);
			_gm->_rooms[FLOORDOOR]->setSectionVisible(8, kShownFalse);
			_vm->playSound(kAudioShip3);
			_gm->screenShake();
			_gm->_rooms[FLOORDOOR]->setSectionVisible(kMaxSection - 1, kShownTrue);
		}
		else {
			_waitTime++;
			_gm->setAnimationTimer(1);
		}
	}
	else
		_gm->setAnimationTimer(kMaxTimerValue);
}

bool PyrEntrance::interact(Action verb, Object &obj1, Object &obj2) {
	static RoomEntry roomTab[29] = {
		{2, 8, 6, 0, FLOORDOOR},
		{0, 8, 4, 2, FLOORDOOR_U},
		{0, 4, 11, 2, PYRAMID},
		{0, 0, 2, 1, UPSTAIRS1},
		{1, 1, 2, 3, DOWNSTAIRS1},
		{0, 5, 8, 3, BOTTOM_RIGHT_DOOR},
		{0, 4, 8, 1, BOTTOM_LEFT_DOOR},
		{1, 5, 8, 3, UPPER_DOOR},
		{1, 4, 8, 1, UPPER_DOOR},
		{0, 4, 8, 0, UPSTAIRS2},
		{1, 4, 7, 2, DOWNSTAIRS2},
		{1, 6, 6, 2, PUZZLE_FRONT},
		{1, 6, 7, 0, PUZZLE_BEHIND},
		{0, 3, 6, 0, FORMULA1_N},
		{0, 3, 7, 0, FORMULA1_F},
		{0, 4, 6, 0, FORMULA2_N},
		{0, 4, 7, 0, FORMULA2_F},
		{0, 8, 9, 2, TOMATO_N},
		{0, 8, 8, 2, TOMATO_F},
		{1, 4, 2, 0, MONSTER_F},
		{1, 10, 8, 0, MONSTER_F},
		{1, 4, 1, 0, MONSTER1_N},
		{1, 10, 7, 0, MONSTER2_N},
		{0, 2, 4, 2, DOWNSTAIRS3},
		{1, 2, 5, 0, UPSTAIRS3},
		{1, 2, 5, 3, LCORRIDOR1},
		{1, 1, 5, 1, LCORRIDOR2},
		{1, 1, 5, 3, HOLE_ROOM},
		{0, 7, 4, 0, BST_DOOR}
	};
	if (!_gm->move(verb, obj1))
		return false;
	if (_gm->_rooms[FLOORDOOR]->isSectionVisible(kMaxSection - 1))
		roomTab[0]._e = 1;
	else
		roomTab[0]._e = 2;
	for (int i = 0; i < 29; i++) {
		if (_gm->_state._pyraE == roomTab[i]._e &&
			_gm->_state._pyraS == roomTab[i]._s &&
			_gm->_state._pyraZ == roomTab[i]._z &&
			_gm->_state._pyraDirection == roomTab[i]._r) {
			_gm->changeRoom(roomTab[i]._exitRoom);
			return true;
		}
	}
	_gm->passageConstruction();
	_gm->_newRoom = true;
	return true;
}

Upstairs1::Upstairs1(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 10;
	_id = UPSTAIRS1;
	_shown[0] = kShownTrue;
	_shown[1] = kShownTrue;
	_shown[2] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 1, 1, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 2, 2, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringCorridor, kStringDefaultDescription, CORRIDOR, EXIT, 0, 0, 0, PYR_ENTRANCE, 2);
}

void Upstairs1::onEntrance() {
	setRoomSeen(true);
}

void Upstairs1::animation() {
}

bool Upstairs1::interact(Action verb, Object &obj1, Object &obj2) {
	if (!_gm->move(verb, obj1))
		return false;
	if (obj1._id == CORRIDOR) {
		if (_gm->_state._pyraE)
			_gm->_state._pyraE = 0;
		else
			_gm->_state._pyraE = 1;
	}
	_gm->passageConstruction();
	_gm->_newRoom = true;
	return true;
}

Downstairs1::Downstairs1(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = DOWNSTAIRS1;
	_shown[0] = kShownTrue;
	_shown[1] = kShownTrue;
	_shown[3] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 1, 1, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 2, 2, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringCorridor, kStringDefaultDescription, CORRIDOR, EXIT, 3, 3, 0, PYR_ENTRANCE, 2);
}

void Downstairs1::onEntrance() {
	setRoomSeen(true);
}

void Downstairs1::animation() {
}

bool Downstairs1::interact(Action verb, Object &obj1, Object &obj2) {
	if (!_gm->move(verb, obj1))
		return false;
	if (obj1._id == CORRIDOR) {
		if (_gm->_state._pyraE)
			_gm->_state._pyraE = 0;
		else
			_gm->_state._pyraE = 1;
	}
	_gm->passageConstruction();
	_gm->_newRoom = true;
	return true;
}

BottomRightDoor::BottomRightDoor(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 12;
	_id = BOTTOM_RIGHT_DOOR;
	_shown[0] = kShownTrue;
	_shown[19] = kShownTrue;
	_shown[23] = kShownTrue;
	_shown[29] = kShownTrue;
	_shown[30] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 1, 1, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 2, 2, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringDoor, kStringMassive, DOOR, EXIT | OPENABLE | CLOSED, 0, 0, 0, PYR_ENTRANCE, 2);
	_objectState[3] = Object(_id, kStringButton, kStringDefaultDescription, BUTTON, PRESS, 19, 19, 0);
}

void BottomRightDoor::onEntrance() {
	setRoomSeen(true);
}

void BottomRightDoor::animation() {
}

bool BottomRightDoor::interact(Action verb, Object &obj1, Object &obj2) {
	if (_gm->move(verb, obj1)) {
		_gm->passageConstruction();
		_gm->_newRoom = true;
	}
	else if (verb == ACTION_PRESS && obj1._id == BUTTON) {
		if (isSectionVisible(22)) {
			_vm->renderImage(21);
			setSectionVisible(22, kShownFalse);
			_gm->wait(2);
			_vm->renderImage(20);
			setSectionVisible(21, kShownFalse);
			_gm->wait(2);
			_vm->renderImage(19);
			setSectionVisible(20, kShownFalse);
			_objectState[2]._type = EXIT | OPENABLE | CLOSED;
			_objectState[2]._id = DOOR;
			_objectState[2]._description = kStringMassive;
			_gm->_rooms[BOTTOM_LEFT_DOOR]->setSectionVisible(22, kShownFalse);
			_gm->_rooms[BOTTOM_LEFT_DOOR]->setSectionVisible(21, kShownFalse);
			_gm->_rooms[BOTTOM_LEFT_DOOR]->setSectionVisible(20, kShownFalse);
			_gm->_rooms[BOTTOM_LEFT_DOOR]->setSectionVisible(19, kShownTrue);
			_gm->_rooms[BOTTOM_LEFT_DOOR]->setSectionVisible(27, kShownFalse);
			_gm->_rooms[BOTTOM_LEFT_DOOR]->setSectionVisible(28, kShownFalse);
			_gm->_rooms[BOTTOM_LEFT_DOOR]->setSectionVisible(24, kShownFalse);
			_gm->_rooms[BOTTOM_LEFT_DOOR]->getObject(2)->_type = EXIT | OPENABLE | CLOSED;
			_gm->_rooms[BOTTOM_LEFT_DOOR]->getObject(2)->_id = DOOR;
			_gm->_rooms[BOTTOM_LEFT_DOOR]->getObject(2)->_description = kStringMassive;
			_gm->_rooms[UPPER_DOOR]->setSectionVisible(26, kShownTrue);
			_gm->_rooms[UPPER_DOOR]->setSectionVisible(27, kShownTrue);
			_gm->_rooms[UPPER_DOOR]->setSectionVisible(19, kShownFalse);
			_gm->_rooms[UPPER_DOOR]->setSectionVisible(25, kShownFalse);
			_gm->_rooms[UPPER_DOOR]->getObject(2)->_type = EXIT;
			_gm->_rooms[UPPER_DOOR]->getObject(2)->_id = CORRIDOR;
			_gm->_rooms[UPPER_DOOR]->getObject(2)->_description = kStringDefaultDescription;
		} else {
			_vm->renderImage(20);
			setSectionVisible(19, kShownFalse);
			_gm->wait(2);
			_vm->renderImage(21);
			_gm->wait(2);
			_vm->renderImage(22);
			_objectState[2]._type = EXIT;
			_objectState[2]._id = CORRIDOR;
			_objectState[2]._description = kStringDefaultDescription;
			_gm->_rooms[BOTTOM_LEFT_DOOR]->setSectionVisible(20, kShownTrue);
			_gm->_rooms[BOTTOM_LEFT_DOOR]->setSectionVisible(21, kShownTrue);
			_gm->_rooms[BOTTOM_LEFT_DOOR]->setSectionVisible(22, kShownTrue);
			_gm->_rooms[BOTTOM_LEFT_DOOR]->setSectionVisible(19, kShownFalse);
			_gm->_rooms[BOTTOM_LEFT_DOOR]->setSectionVisible(27, kShownTrue);
			_gm->_rooms[BOTTOM_LEFT_DOOR]->setSectionVisible(28, kShownTrue);
			_gm->_rooms[BOTTOM_LEFT_DOOR]->setSectionVisible(24, kShownTrue);
			_gm->_rooms[BOTTOM_LEFT_DOOR]->getObject(2)->_type = EXIT;
			_gm->_rooms[BOTTOM_LEFT_DOOR]->getObject(2)->_id = CORRIDOR;
			_gm->_rooms[BOTTOM_LEFT_DOOR]->getObject(2)->_description = kStringDefaultDescription;
			_gm->_rooms[UPPER_DOOR]->setSectionVisible(26, kShownFalse);
			_gm->_rooms[UPPER_DOOR]->setSectionVisible(27, kShownFalse);
			_gm->_rooms[UPPER_DOOR]->setSectionVisible(19, kShownTrue);
			_gm->_rooms[UPPER_DOOR]->setSectionVisible(25, kShownTrue);
			_gm->_rooms[UPPER_DOOR]->getObject(2)->_type = EXIT | OPENABLE | CLOSED;
			_gm->_rooms[UPPER_DOOR]->getObject(2)->_id = DOOR;
			_gm->_rooms[UPPER_DOOR]->getObject(2)->_description = kStringMassive;
		}
		_vm->playSound(kAudioShip3);
		_gm->screenShake();
	}
	else
		return false;
	return true;
}

BottomLeftDoor::BottomLeftDoor(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = BOTTOM_LEFT_DOOR;
	_shown[0] = kShownTrue;
	_shown[19] = kShownTrue;
	_shown[29] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 1, 1, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 2, 2, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringDoor, kStringMassive, DOOR, EXIT | OPENABLE | CLOSED, 0, 0, 0, PYR_ENTRANCE, 2);
}

void BottomLeftDoor::onEntrance() {
	setRoomSeen(true);
}

void BottomLeftDoor::animation() {
}

bool BottomLeftDoor::interact(Action verb, Object &obj1, Object &obj2) {
	if (_gm->move(verb, obj1)) {
		if (verb == ACTION_WALK && obj1._id == G_LEFT)
			_gm->changeRoom(UPSTAIRS2);
		else
			_gm->passageConstruction();
		_gm->_newRoom = true;
	}
	else
		return false;
	return true;
}

Upstairs2::Upstairs2(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 10;
	_id = UPSTAIRS2;
	_shown[0] = kShownTrue;
	_shown[2] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 1, 1, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 2, 2, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringCorridor, kStringDefaultDescription, CORRIDOR, EXIT, 0, 0, 0, PYR_ENTRANCE, 2);
}

void Upstairs2::onEntrance() {
	setRoomSeen(true);
}

void Upstairs2::animation() {
}

bool Upstairs2::interact(Action verb, Object &obj1, Object &obj2) {
	if (!_gm->move(verb, obj1))
		return false;
	if (obj1._id == CORRIDOR) {
		if (_gm->_state._pyraE)
			_gm->_state._pyraE = 0;
		else
			_gm->_state._pyraE = 1;
	}
	if (obj1._id == G_RIGHT)
		_gm->changeRoom(BOTTOM_LEFT_DOOR);
	else
		_gm->passageConstruction();
	_gm->_newRoom = true;
	return true;
}

Downstairs2::Downstairs2(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 10;
	_id = DOWNSTAIRS2;
	_shown[0] = kShownTrue;
	_shown[1] = kShownTrue;
	_shown[3] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 1, 1, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 2, 2, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringCorridor, kStringDefaultDescription, CORRIDOR, EXIT, 3, 3, 0, PYR_ENTRANCE, 2);
}

void Downstairs2::onEntrance() {
	setRoomSeen(true);
}

void Downstairs2::animation() {
}

bool Downstairs2::interact(Action verb, Object &obj1, Object &obj2) {
	if (!_gm->move(verb, obj1))
		return false;
	if (obj1._id == CORRIDOR) {
		if (_gm->_state._pyraE)
			_gm->_state._pyraE = 0;
		else
			_gm->_state._pyraE = 1;
	}
	_gm->passageConstruction();
	_gm->_newRoom = true;
	return true;
	return true;
}

UpperDoor::UpperDoor(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 12;
	_id = UPPER_DOOR;
	_shown[0] = kShownTrue;
	_shown[26] = kShownTrue;
	_shown[27] = kShownTrue;
	_shown[30] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 1, 1, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 2, 2, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringDoor, kStringDefaultDescription, CORRIDOR, EXIT, 0, 0, 0, PYR_ENTRANCE, 2);
}

void UpperDoor::onEntrance() {
	setRoomSeen(true);
}

void UpperDoor::animation() {
}

bool UpperDoor::interact(Action verb, Object &obj1, Object &obj2) {
	if (!_gm->move(verb, obj1))
		return false;
	_gm->passageConstruction();
	_gm->_newRoom = true;
	return true;
}

PuzzleFront::PuzzleFront(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 12;
	_id = PUZZLE_FRONT;
	_shown[0] = kShownTrue;
	_shown[1] = kShownTrue;
	_shown[2] = kShownTrue;
	_shown[3] = kShownTrue;
	_shown[4] = kShownTrue;
	_shown[5] = kShownTrue;
	_shown[6] = kShownTrue;
	_shown[7] = kShownTrue;
	_shown[8] = kShownTrue;
	_shown[9] = kShownTrue;
	_shown[10] = kShownTrue;
	_shown[11] = kShownTrue;
	_shown[12] = kShownTrue;
	_shown[13] = kShownTrue;
	_shown[14] = kShownTrue;
	_shown[15] = kShownTrue;
	_shown[30] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 1, 1, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 2, 2, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringDoor, kStringMassive, DOOR, EXIT | OPENABLE | CLOSED, 255, 255, 0, PYR_ENTRANCE, 2);
	_objectState[3] = Object(_id, kStringPiece, kStringDefaultDescription, PART0, PRESS, 3, 3, 0);
	_objectState[4] = Object(_id, kStringPiece, kStringDefaultDescription, PART1, PRESS, 4, 4, 0);
	_objectState[5] = Object(_id, kStringPiece, kStringDefaultDescription, PART2, PRESS, 5, 5, 0);
	_objectState[6] = Object(_id, kStringPiece, kStringDefaultDescription, PART3, PRESS, 6, 6, 0);
	_objectState[7] = Object(_id, kStringPiece, kStringDefaultDescription, PART4, PRESS, 7, 7, 0);
	_objectState[8] = Object(_id, kStringPiece, kStringDefaultDescription, PART5, PRESS, 8, 8, 0);
	_objectState[9] = Object(_id, kStringPiece, kStringDefaultDescription, PART6, PRESS, 9, 9, 0);
	_objectState[10] = Object(_id, kStringPiece, kStringDefaultDescription, PART7, PRESS, 10, 10, 0);
	_objectState[11] = Object(_id, kStringPiece, kStringDefaultDescription, PART8, PRESS, 11, 11, 0);
	_objectState[12] = Object(_id, kStringPiece, kStringDefaultDescription, PART9, PRESS, 12, 12, 0);
	_objectState[13] = Object(_id, kStringPiece, kStringDefaultDescription, PART10, PRESS, 13, 13, 0);
	_objectState[14] = Object(_id, kStringPiece, kStringDefaultDescription, PART11, PRESS, 14, 14, 0);
	_objectState[15] = Object(_id, kStringPiece, kStringDefaultDescription, PART12, PRESS, 15, 15, 0);
	_objectState[16] = Object(_id, kStringPiece, kStringDefaultDescription, PART13, PRESS, 16, 16, 0);
	_objectState[17] = Object(_id, kStringPiece, kStringDefaultDescription, PART14, PRESS, 17, 17, 0);
	_objectState[18] = Object(_id, kStringPiece, kStringDefaultDescription, PART15, PRESS, 18, 18, 0);
}

void PuzzleFront::onEntrance() {
	setRoomSeen(true);
}

void PuzzleFront::animation() {
}

bool PuzzleFront::interact(Action verb, Object &obj1, Object &obj2) {
	MS2Image *image = _vm->_screen->getCurrentImage();
	if (_gm->move(verb, obj1)) {
		_gm->passageConstruction();
		_gm->_newRoom = true;
	} else if (verb == ACTION_PRESS && obj1._id >= PART0 && obj1._id <= PART15) {
		int pos = obj1._id - PART0;
		int newPos = 0;
		if (pos > 3  && _gm->_puzzleField[pos - 4] == 255)
			newPos = pos - 4;
		else if (pos < 12 && _gm->_puzzleField[pos + 4] == 255)
			newPos = pos + 4;
		else if ((pos % 4) > 0 && _gm->_puzzleField[pos - 1] == 255)
			newPos = pos - 1;
		else if ((pos % 4) < 3 && _gm->_puzzleField[pos + 1] == 255)
			newPos = pos + 1;
		else {
			_vm->renderMessage(kStringPyramid5);
			return true;
		}
		int a = _gm->_puzzleField[pos] + 1;
		_vm->renderImage(a + 128);
		image->_section[a].x1 = 95 + (newPos % 4) * 33;
		image->_section[a].x2 = image->_section[a].x1 + 31;
		image->_section[a].y1 = 24 + (newPos / 4) * 25;
		image->_section[a].y2 = image->_section[a].y1 + 23;
		_vm->renderImage(a);
		_vm->playSound(kAudioTaxiOpen);
		_gm->_state._puzzleTab[a - 1] = newPos;
		_gm->_puzzleField[pos] = 255;
		_gm->_puzzleField[newPos] = a - 1;
		_objectState[pos + 3]._click = 255;
		_objectState[newPos + 3]._click = newPos + 3;
		for (int i = 0; i < 15; i++)
			if (_gm->_state._puzzleTab[i] != i)
				return true;
		
		_gm->wait(2);
		_vm->renderImage(16);
		for (int i = 1; i < 16; i++)
			setSectionVisible(i, kShownFalse);
		_gm->wait(2);
		_vm->renderImage(17);
		_gm->wait(2);
		_vm->renderImage(18);
		_objectState[2]._type = EXIT;
		_objectState[2]._id = CORRIDOR;
		_objectState[2]._description = kStringDefaultDescription;
		_objectState[2]._click = 0;
		
		_gm->_rooms[PUZZLE_BEHIND]->setSectionVisible(31, kShownFalse);
		_gm->_rooms[PUZZLE_BEHIND]->setSectionVisible(26, kShownTrue);
		_gm->_rooms[PUZZLE_BEHIND]->setSectionVisible(27, kShownTrue);
		_gm->_rooms[PUZZLE_BEHIND]->setSectionVisible(29, kShownTrue);
		_gm->_rooms[PUZZLE_BEHIND]->getObject(2)->_type = EXIT;
		_gm->_rooms[PUZZLE_BEHIND]->getObject(2)->_id = CORRIDOR;
		_gm->_rooms[PUZZLE_BEHIND]->getObject(2)->_description = kStringDefaultDescription;
		_vm->playSound(kAudioShip3);
		_gm->screenShake();
	}
	else
		return false;
	return true;
}

PuzzleBehind::PuzzleBehind(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 12;
	_id = PUZZLE_BEHIND;
	_shown[0] = kShownTrue;
	_shown[29] = kShownTrue;
	_shown[31] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 1, 1, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 2, 2, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringDoor, kStringMassive, DOOR, EXIT | OPENABLE | CLOSED, 0, 0, 0, PYR_ENTRANCE, 2);
}

void PuzzleBehind::onEntrance() {
	setRoomSeen(true);
}

void PuzzleBehind::animation() {
}

bool PuzzleBehind::interact(Action verb, Object &obj1, Object &obj2) {
	if (!_gm->move(verb, obj1))
		return false;
	_gm->passageConstruction();
	_gm->_newRoom = true;
	return true;
}

Formula1F::Formula1F(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 14;
	_id = FORMULA1_F;
	_shown[0] = kShownTrue;
	_shown[2] = kShownTrue;
	_shown[15] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 12, 12, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 11, 11, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringInscription, kStringInscriptionDescription, DOOR, EXIT, 1, 1, 0, PYR_ENTRANCE, 2);
}

void Formula1F::onEntrance() {
	setRoomSeen(true);
}

void Formula1F::animation() {
}

bool Formula1F::interact(Action verb, Object &obj1, Object &obj2) {
	if (!_gm->move(verb, obj1))
		return false;
	if (obj1._id == CORRIDOR)
		_gm->changeRoom(FORMULA1_N);
	else
		_gm->passageConstruction();
	_gm->_newRoom = true;
	return true;
}

Formula1N::Formula1N(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 13;
	_id = FORMULA1_N;
	_shown[0] = kShownTrue;
	_shown[5] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 8, 8, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 7, 7, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringInscription, kStringInscriptionDescription, NULLOBJECT, NULLTYPE, 2, 2, 0);
}

void Formula1N::onEntrance() {
	setRoomSeen(true);
}

void Formula1N::animation() {
}

bool Formula1N::interact(Action verb, Object &obj1, Object &obj2) {
	if (!_gm->move(verb, obj1))
		return false;
	_gm->passageConstruction();
	_gm->_newRoom = true;
	return true;
}

Formula2F::Formula2F(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 14;
	_id = FORMULA2_F;
	_shown[0] = kShownTrue;
	_shown[3] = kShownTrue;
	_shown[14] = kShownTrue;
	_shown[15] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 12, 12, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 11, 11, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringInscription, kStringInscriptionDescription, DOOR, EXIT, 2, 2, 0, PYR_ENTRANCE, 2);
}

void Formula2F::onEntrance() {
	setRoomSeen(true);
}

void Formula2F::animation() {
}

bool Formula2F::interact(Action verb, Object &obj1, Object &obj2) {
	if (!_gm->move(verb, obj1))
		return false;
	if (obj1._id == CORRIDOR)
		_gm->changeRoom(FORMULA2_N);
	else
		_gm->passageConstruction();
	_gm->_newRoom = true;
	return true;
}

Formula2N::Formula2N(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 13;
	_id = FORMULA2_N;
	_shown[0] = kShownTrue;
	_shown[6] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 8, 8, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 7, 7, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringInscription, kStringInscriptionDescription, NULLOBJECT, NULLTYPE, 3, 3, 0);
}

void Formula2N::onEntrance() {
	setRoomSeen(true);
}

void Formula2N::animation() {
}

bool Formula2N::interact(Action verb, Object &obj1, Object &obj2) {
	if (!_gm->move(verb, obj1))
		return false;
	_gm->passageConstruction();
	_gm->_newRoom = true;
	return true;
}

TomatoF::TomatoF(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 14;
	_id = TOMATO_F;
	_shown[0] = kShownTrue;
	_shown[4] = kShownTrue;
	_shown[15] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 12, 12, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 11, 11, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringTomato, kStringFunnyDescription, CORRIDOR, EXIT, 3, 3, 0, PYR_ENTRANCE, 2);
}

void TomatoF::onEntrance() {
	setRoomSeen(true);
}

void TomatoF::animation() {
}

bool TomatoF::interact(Action verb, Object &obj1, Object &obj2) {
	if (!_gm->move(verb, obj1))
		return false;
	if (obj1._id == CORRIDOR)
		_gm->changeRoom(TOMATO_N);
	else
		_gm->passageConstruction();
	_gm->_newRoom = true;
	return true;
}

TomatoN::TomatoN(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 13;
	_id = TOMATO_N;
	_shown[0] = kShownTrue;
	_shown[7] = kShownTrue;
	_shown[8] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 8, 8, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 7, 7, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringKnife1, kStringKnife1Description, TKNIFE, TAKE | COMBINABLE, 5, 5, 7);
	_objectState[3] = Object(_id, kStringTomato, kStringFunnyDescription, NULLOBJECT, UNNECESSARY, 4, 4, 0);
}

void TomatoN::onEntrance() {
	setRoomSeen(true);
}

void TomatoN::animation() {
}

bool TomatoN::interact(Action verb, Object &obj1, Object &obj2) {
	if (_gm->move(verb, obj1)) {
		_gm->passageConstruction();
		_gm->_newRoom = true;
	} else if (verb == ACTION_TAKE && obj1._id == TKNIFE) {
		_gm->takeObject(obj1);
		setSectionVisible(8, kShownFalse);
	} else
		return false;
	return true;
}

MonsterF::MonsterF(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 14;
	_id = MONSTER_F;
	_shown[0] = kShownTrue;
	_shown[1] = kShownTrue;
	_shown[14] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 12, 12, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 11, 11, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringMonster, kStringMonsterDescription, CORRIDOR, EXIT, 0, 0, 0, PYR_ENTRANCE, 2);
}

void MonsterF::onEntrance() {
	setRoomSeen(true);
}

void MonsterF::animation() {
}

bool MonsterF::interact(Action verb, Object &obj1, Object &obj2) {
	if (!_gm->move(verb, obj1))
		return false;
	if (obj1._id == CORRIDOR) {
		if (_gm->_state._pyraS == 4)
			_gm->changeRoom(MONSTER1_N);
		else
			_gm->changeRoom(MONSTER2_N);
	} else
		_gm->passageConstruction();
	_gm->_newRoom = true;
	return true;
}

Monster1N::Monster1N(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 13;
	_id = MONSTER1_N;
	_shown[0] = kShownTrue;
	_shown[1] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 8, 8, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 7, 7, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringRope, kStringRopeDescription, ROPE, TAKE | COMBINABLE, 255, 255, 2);
	_objectState[3] = Object(_id, kStringEyes, kStringDefaultDescription, NULLOBJECT, NULLTYPE, 6, 6, 0);
	_objectState[4] = Object(_id, kStringMouth, kStringDefaultDescription, MOUTH, NULLTYPE, 1, 1, 0);
	_objectState[5] = Object(_id, kStringMonster, kStringMonster1Description, MONSTER, NULLTYPE, 0, 0, 0);
}

void Monster1N::onEntrance() {
	setRoomSeen(true);
}

void Monster1N::animation() {
}

bool Monster1N::interact(Action verb, Object &obj1, Object &obj2) {
	if (_gm->move(verb, obj1)) {
		_gm->passageConstruction();
		_gm->_newRoom = true;
	} else if ((verb == ACTION_OPEN || verb == ACTION_PULL) && obj1._id == MOUTH) {
		if (obj1._type & OPENED)
			_vm->renderMessage(kStringGenericInteract8);
		else {
			obj1._type |= OPENED;
			if (isSectionVisible(kMaxSection - 1))
				_vm->renderImage(2);
			else {
				_objectState[2]._click = 1;
				_vm->renderImage(4);
				setSectionVisible(2, kShownFalse);
			}
			_vm->playSound(kAudioTaxiOpen);
		}
	} else if (verb == ACTION_CLOSE && obj1._id == MOUTH) {
		if (obj1._type & OPENED) {
			_vm->renderImage(1);
			setSectionVisible(2, kShownFalse);
			obj1._type &= ~OPENED;
			_vm->playSound(kAudioElevator1);
		} else
			_vm->renderMessage(kStringGenericInteract11);
	} else if (verb == ACTION_TAKE && (obj1._id == ROPE || obj1._id == NOTE)) {
		_gm->takeObject(obj1);
		setSectionVisible(3, kShownFalse);
		setSectionVisible(4, kShownFalse);
		setSectionVisible(kMaxSection - 1, kShownTrue);
	} else if (verb == ACTION_USE && Object::combine(obj1, obj2, TKNIFE, MONSTER)) {
		_vm->renderMessage(kStringPyramid6);
	} else
		return false;
	return true;
}

Monster2N::Monster2N(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 13;
	_id = MONSTER2_N;
	_shown[0] = kShownTrue;
	_shown[1] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 8, 8, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 7, 7, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringNote, kStringNoteDescription, NOTE, TAKE | COMBINABLE, 255, 255, 2);
	_objectState[3] = Object(_id, kStringEyes, kStringDefaultDescription, NULLOBJECT, NULLTYPE, 6, 6, 0);
	_objectState[4] = Object(_id, kStringMouth, kStringDefaultDescription, MOUTH, NULLTYPE, 1, 1, 0);
	_objectState[5] = Object(_id, kStringMonster, kStringMonster1Description, MONSTER, NULLTYPE, 0, 0, 0);
}

void Monster2N::onEntrance() {
	setRoomSeen(true);
}

void Monster2N::animation() {
}

bool Monster2N::interact(Action verb, Object &obj1, Object &obj2) {
	if (_gm->move(verb, obj1)) {
		_gm->passageConstruction();
		_gm->_newRoom = true;
	} else if ((verb == ACTION_OPEN || verb == ACTION_PULL) && obj1._id == MOUTH) {
		if (obj1._type & OPENED)
			_vm->renderMessage(kStringGenericInteract8);
		else {
			obj1._type |= OPENED;
			if (isSectionVisible(kMaxSection - 1))
				_vm->renderImage(2);
			else {
				_objectState[2]._click = 1;
				_vm->renderImage(3);
				setSectionVisible(2, kShownFalse);
			}
			_vm->playSound(kAudioTaxiOpen);
		}
	} else if (verb == ACTION_CLOSE && obj1._id == MOUTH) {
		if (obj1._type & OPENED) {
			_vm->renderImage(1);
			setSectionVisible(2, kShownFalse);
			obj1._type &= ~OPENED;
			_vm->playSound(kAudioElevator1);
		} else
			_vm->renderMessage(kStringGenericInteract11);
	} else if (verb == ACTION_TAKE && (obj1._id == ROPE || obj1._id == NOTE)) {
		_gm->takeObject(obj1);
		setSectionVisible(3, kShownFalse);
		setSectionVisible(4, kShownFalse);
		setSectionVisible(kMaxSection - 1, kShownTrue);
	} else if (verb == ACTION_USE && Object::combine(obj1, obj2, TKNIFE, MONSTER)) {
		_vm->renderMessage(kStringPyramid6);
	} else
		return false;
	return true;
}

Upstairs3::Upstairs3(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 10;
	_id = UPSTAIRS3;
	_shown[0] = kShownTrue;
	_shown[1] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 1, 1, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 2, 2, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringCorridor, kStringDefaultDescription, CORRIDOR, EXIT, 0, 0, 0, PYR_ENTRANCE, 2);
}

void Upstairs3::onEntrance() {
	setRoomSeen(true);
}

void Upstairs3::animation() {
}

bool Upstairs3::interact(Action verb, Object &obj1, Object &obj2) {
	if (!_gm->move(verb, obj1))
		return false;
	if (obj1._id == CORRIDOR) {
		if (_gm->_state._pyraE)
			_gm->_state._pyraE = 0;
		else
			_gm->_state._pyraE = 1;
	}
	if (obj1._id == G_LEFT)
		_gm->changeRoom(LCORRIDOR1);
	else
		_gm->passageConstruction();
	_gm->_newRoom = true;
	return true;
}

Downstairs3::Downstairs3(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 10;
	_id = DOWNSTAIRS3;
	_shown[0] = kShownTrue;
	_shown[2] = kShownTrue;
	_shown[3] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 1, 1, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 2, 2, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringCorridor, kStringDefaultDescription, CORRIDOR, EXIT, 3, 3, 0, PYR_ENTRANCE, 2);
}

void Downstairs3::onEntrance() {
	setRoomSeen(true);
}

void Downstairs3::animation() {
}

bool Downstairs3::interact(Action verb, Object &obj1, Object &obj2) {
	if (!_gm->move(verb, obj1))
		return false;
	if (obj1._id == CORRIDOR) {
		if (_gm->_state._pyraE)
			_gm->_state._pyraE = 0;
		else
			_gm->_state._pyraE = 1;
	}
	_gm->passageConstruction();
	_gm->_newRoom = true;
	return true;
}

LCorridor1::LCorridor1(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 9;
	_id = LCORRIDOR1;
	_shown[0] = kShownTrue;
	_shown[2] = kShownTrue;
	_shown[17] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 1, 1, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 2, 2, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringCorridor, kStringDefaultDescription, CORRIDOR, EXIT, 6, 6, 0, PYR_ENTRANCE, 2);
}

void LCorridor1::onEntrance() {
	setRoomSeen(true);
}

void LCorridor1::animation() {
}

bool LCorridor1::interact(Action verb, Object &obj1, Object &obj2) {
	if (!_gm->move(verb, obj1))
		return false;
	if (obj1._id == G_RIGHT)
		_gm->changeRoom(UPSTAIRS3);
	else if (obj1._id == CORRIDOR)
		_gm->changeRoom(HOLE_ROOM);
	else
		_gm->passageConstruction();
	_gm->_newRoom = true;
	return true;
}

LCorridor2::LCorridor2(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 9;
	_id = LCORRIDOR2;
	_shown[0] = kShownTrue;
	_shown[17] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 1, 1, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 2, 2, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringCorridor, kStringDefaultDescription, CORRIDOR, EXIT, 6, 6, 0, PYR_ENTRANCE, 2);
}

void LCorridor2::onEntrance() {
	setRoomSeen(true);
}

void LCorridor2::animation() {
}

bool LCorridor2::interact(Action verb, Object &obj1, Object &obj2) {
	if (!_gm->move(verb, obj1))
		return false;
	_gm->passageConstruction();
	_gm->_newRoom = true;
	return true;
}

HoleRoom::HoleRoom(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 9;
	_id = HOLE_ROOM;
	_shown[0] = kShownTrue;
	_shown[8] = kShownTrue;
	_shown[15] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 1, 1, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 2, 2, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringRope, kStringDefaultDescription, ROPE, NULLTYPE, 255, 255, 0);
	_objectState[3] = Object(_id, kStringOpening, kStringOpeningDescription1, HOLE, EXIT, 3, 3, 0, IN_HOLE, 12);
	_objectState[4] = Object(_id, kStringOpening, kStringOpeningDescription2, NULLOBJECT, NULLTYPE, 4, 4, 0);
}

void HoleRoom::onEntrance() {
	setRoomSeen(true);
}

void HoleRoom::animation() {
}

bool HoleRoom::interact(Action verb, Object &obj1, Object &obj2) {
	if (_gm->move(verb, obj1)) {
		_gm->passageConstruction();
		_gm->_newRoom = true;
	} else if (verb == ACTION_USE && 
			   (Object::combine(obj1, obj2, ROPE, G_RIGHT) || 
				Object::combine(obj1, obj2, ROPE, G_LEFT))) {
		_vm->renderMessage(kStringPyramid7);
	} else if (verb == ACTION_USE && 
			   (Object::combine(obj1, obj2, TKNIFE, G_RIGHT) || 
				Object::combine(obj1, obj2, TKNIFE, G_LEFT))) {
		_vm->renderMessage(kStringPyramid8);
	} else if (verb == ACTION_TAKE && obj1._id == ROPE && !(obj1._type & CARRIED)) {
		_vm->renderMessage(kStringPyramid9);
	} else if (verb == ACTION_WALK && obj1._id == HOLE && !isSectionVisible(16)) {
		_vm->renderMessage(kStringPyramid10);
	} else if (verb == ACTION_USE && obj1._id == ROPE && isSectionVisible(16)) {
		_gm->changeRoom(IN_HOLE);
		_gm->_newRoom = true;
	} else
		return false;
	return true;
}

InHole::InHole(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 6;
	_id = IN_HOLE;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringRope, kStringDefaultDescription, ROPE, EXIT, 0, 0, 0, HOLE_ROOM, 2);
	_objectState[1] = Object(_id, kStringNote, kStringNoteDescription1, NULLOBJECT, TAKE, 255, 255, 1 + 128);
	_objectState[2] = Object(_id, kStringSlot, kStringSlotDescription3, SLOT, COMBINABLE, 1, 1, 0);
}

void InHole::onEntrance() {
	setRoomSeen(true);
}

void InHole::animation() {
}

bool InHole::interact(Action verb, Object &obj1, Object &obj2) {
	if (verb == ACTION_USE && Object::combine(obj1, obj2, TKNIFE, SLOT)) {
		if (isSectionVisible(kMaxSection - 1))
			_vm->renderMessage(kStringPyramid11);
		else {
			_vm->renderImage(1);
			_objectState[1]._click = 2;
			setSectionVisible(kMaxSection - 1, kShownTrue);
		}
	} else if (verb == ACTION_TAKE && obj1._id == ROPE) {
		_vm->renderMessage(kStringPyramid9);
	} else if (verb == ACTION_USE && obj1._id == ROPE) {
		_gm->changeRoom(HOLE_ROOM);
		_gm->_newRoom = true;
	} else
		return false;
	return true;
}

Floordoor::Floordoor(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 14;
	_id = FLOORDOOR;
	_shown[0] = kShownTrue;
	_shown[14] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 12, 12, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 11, 11, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringKnife1, kStringDefaultDescription, TKNIFE, TAKE | COMBINABLE, 255, 255, 9);
	_objectState[3] = Object(_id, kStringRope, kStringDefaultDescription, ROPE, TAKE, 255, 255, 11+128);
	_objectState[4] = Object(_id, kStringOpening, kStringOpeningDescription3, HOLE, EXIT, 4, 4, 0, FLOORDOOR_U, 12);
	_objectState[5] = Object(_id, kStringStones, kStringDefaultDescription, STONES, COMBINABLE, 5, 5, 0);
}

void Floordoor::onEntrance() {
	setRoomSeen(true);
}

void Floordoor::animation() {
}

bool Floordoor::interact(Action verb, Object &obj1, Object &obj2) {
	if (_gm->move(verb, obj1)) {
		_gm->passageConstruction();
		_gm->_newRoom = true;
	} else if (verb == ACTION_WALK && obj1._id == HOLE) {
		if (isSectionVisible(11)) {
			_gm->_state._pyraZ = 4;
			_gm->_state._pyraDirection = 2;
			_gm->_state._pyraE = 0;
			return false;
		} else
			_vm->renderMessage(kStringPyramid12);
	} else if (verb == ACTION_USE && obj1._id == ROPE && isSectionVisible(11)) {
		_gm->_state._pyraZ = 4;
		_gm->_state._pyraDirection = 2;
		_gm->_state._pyraE = 0 ;
		_gm->changeRoom(FLOORDOOR_U);
		_gm->_newRoom = true;
	} else if (verb == ACTION_USE && Object::combine(obj1, obj2, TKNIFE, STONES)) {
		_vm->renderImage(10);
		if (obj1._id == TKNIFE)
			_gm->_inventory.remove(obj1);
		else
			_gm->_inventory.remove(obj2);
		_objectState[2]._click = 6;
	} else if (verb == ACTION_USE && 
			   (Object::combine(obj1, obj2, TKNIFE, G_RIGHT) ||
				Object::combine(obj1, obj2, TKNIFE, G_LEFT))) {
		_vm->renderMessage(kStringPyramid8);
	} else if (verb == ACTION_USE && Object::combine(obj1, obj2, ROPE, TKNIFE) &&
					isSectionVisible(10)) {
		_vm->renderImage(11);
		setSectionVisible(10, kShownFalse);
		if (obj1._id == ROPE)
			_gm->_inventory.remove(obj1);
		else
			_gm->_inventory.remove(obj2);
		_objectState[2]._click = 255;
		_objectState[3]._click = 7;
		_objectState[3]._type &= ~COMBINABLE;
	} else if (verb == ACTION_USE && 
			   (Object::combine(obj1, obj2, ROPE, G_RIGHT) ||
				Object::combine(obj1, obj2, ROPE, G_LEFT)  ||
				Object::combine(obj1, obj2, ROPE, STONES))) {
		_vm->renderMessage(kStringPyramid7);
	} else if (verb == ACTION_TAKE && obj1._id == ROPE && !(obj1._type & CARRIED)) {
		_gm->takeObject(obj1);
		obj1._type |= COMBINABLE;
		_vm->renderImage(9);
		_vm->renderImage(10);
		_objectState[2]._click = 6;
	} else if (verb == ACTION_TAKE && obj1._id == TKNIFE && !(obj1._type & CARRIED)) {
		_gm->takeObject(obj1);
		setSectionVisible(10, kShownFalse);
	} else
		return false;
	return true;
}

FloordoorU::FloordoorU(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 14;
	_id = FLOORDOOR_U;
	_shown[0] = kShownTrue;
	_shown[12] = kShownTrue;
	_shown[13] = kShownTrue;
	_shown[15] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 12, 12, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 11, 11, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringRope, kStringDefaultDescription, ROPE, NULLTYPE, 10, 10, 0, FLOORDOOR, 2);
	_objectState[3] = Object(_id, kStringOpening, kStringDefaultDescription, HOLE, EXIT, 8, 8, 0, FLOORDOOR, 2);
	_objectState[4] = Object(_id, kStringPlate, kStringDefaultDescription, NULLOBJECT, NULLTYPE, 9, 9, 0);
}

void FloordoorU::onEntrance() {
	setRoomSeen(true);
}

void FloordoorU::animation() {
}

bool FloordoorU::interact(Action verb, Object &obj1, Object &obj2) {
	if (_gm->move(verb, obj1)) {
		_gm->passageConstruction();
		_gm->_newRoom = true;
	} else if (verb == ACTION_WALK && obj1._id == HOLE) {
		_gm->_state._pyraZ = 6;
		_gm->_state._pyraDirection = 0;
		_gm->_state._pyraE = 1;
		return false;
	} else if (verb == ACTION_USE && obj1._id == ROPE) {
		_gm->_state._pyraZ = 6;
		_gm->_state._pyraDirection = 0;
		_gm->_state._pyraE = 1;
		_gm->changeRoom(FLOORDOOR);
		_gm->_newRoom = true;
	} else if (verb == ACTION_TAKE && obj1._id == ROPE) {
		_vm->renderMessage(kStringPyramid9);
	} else
		return false;
	return true;
}

BstDoor::BstDoor(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 15;
	_id = BST_DOOR;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringRight, kStringDefaultDescription, G_RIGHT, EXIT, 18, 18, 0, PYR_ENTRANCE, 14);
	_objectState[1] = Object(_id, kStringLeft, kStringDefaultDescription, G_LEFT, EXIT, 17, 17, 0, PYR_ENTRANCE, 10);
	_objectState[2] = Object(_id, kStringLetter, kStringDefaultDescription, BST1, PRESS, 0, 0, 0);
	_objectState[3] = Object(_id, kStringLetter, kStringDefaultDescription, BST2, PRESS, 1, 0, 0);
	_objectState[4] = Object(_id, kStringLetter, kStringDefaultDescription, BST3, PRESS, 2, 0, 0);
	_objectState[5] = Object(_id, kStringLetter, kStringDefaultDescription, BST4, PRESS, 3, 0, 0);
	_objectState[6] = Object(_id, kStringLetter, kStringDefaultDescription, BST5, PRESS, 4, 0, 0);
	_objectState[7] = Object(_id, kStringLetter, kStringDefaultDescription, BST6, PRESS, 5, 0, 0);
	_objectState[8] = Object(_id, kStringLetter, kStringDefaultDescription, BST7, PRESS, 6, 0, 0);
	_objectState[9] = Object(_id, kStringLetter, kStringDefaultDescription, BST8, PRESS, 7, 0, 0);
	_objectState[10] = Object(_id, kStringLetter, kStringDefaultDescription, BST9, PRESS, 8, 0, 0);
	_objectState[11] = Object(_id, kStringLetter, kStringDefaultDescription, BST10, PRESS, 9, 0, 0);
	_objectState[12] = Object(_id, kStringLetter, kStringDefaultDescription, BST11, PRESS, 10, 0, 0);
	_objectState[13] = Object(_id, kStringLetter, kStringDefaultDescription, BST12, PRESS, 11, 0, 0);
	_objectState[14] = Object(_id, kStringLetter, kStringDefaultDescription, BST13, PRESS, 12, 0, 0);
	_objectState[15] = Object(_id, kStringLetter, kStringDefaultDescription, BST14, PRESS, 13, 0, 0);
	_objectState[16] = Object(_id, kStringLetter, kStringDefaultDescription, BST15, PRESS, 14, 0, 0);
	_objectState[17] = Object(_id, kStringLetter, kStringDefaultDescription, BST16, PRESS, 15, 0, 0);
	_objectState[18] = Object(_id, kStringDoor, kStringMassive, DOOR, EXIT | OPENABLE | CLOSED, 16, 16, 0, HALL, 2);
}

void BstDoor::onEntrance() {
	setRoomSeen(true);
}

void BstDoor::animation() {
}

bool BstDoor::interact(Action verb, Object &obj1, Object &obj2) {
	static char password[16] = {0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0};
	if (_gm->move(verb, obj1)) {
		_gm->passageConstruction();
		_gm->_newRoom = true;
	} else if (verb == ACTION_PRESS && obj1._id >= BST1 && obj1._id <= BST16) {
		int number = obj1._id - (BST1 - 1);
		if (isSectionVisible(number))
			_vm->renderImage(number + 128);
		else
			_vm->renderImage(number);
		_vm->playSound(kAudioTaxiOpen);
		for (number = 1; number <= 16; number++) {
			if (isSectionVisible(number) != password[number - 1])
				return true;
		}
		_gm->wait(2);
		_vm->renderImage(17);
		for (number = 1; number <= 16; number++) {
			setSectionVisible(number, kShownFalse);
			_objectState[number + 1]._click = 255;
		}
		_gm->wait(2);
		_vm->renderImage(18);
		_gm->wait(2);
		_vm->renderImage(19);
		_objectState[18]._type = EXIT;
		_objectState[18]._description = kStringDefaultDescription;
		_vm->playSound(kAudioShip3);
		_gm->screenShake();
	} else
		return false;
	return true;
}

Hall::Hall(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 16;
	_id = HALL;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringCoffin, kStringDefaultDescription, NULLOBJECT, EXIT, 0, 0, 0, COFFIN_ROOM, 2);
	_objectState[1] = Object(_id, kStringExit, kStringDefaultDescription, CORRIDOR, EXIT, 255, 255, 0, PYR_ENTRANCE, 22);
}

void Hall::onEntrance() {
	setRoomSeen(true);
}

void Hall::animation() {
}

bool Hall::interact(Action verb, Object &obj1, Object &obj2) {
	if (verb == ACTION_WALK && obj1._id == CORRIDOR) {
		_gm->_state._pyraDirection = 2;
		_gm->passageConstruction();
	}
	return false;
}

CoffinRoom::CoffinRoom(Supernova2Engine *vm, GameManager *gm) {
	_vm = vm;
	_gm = gm;

	_fileNumber = 17;
	_id = COFFIN_ROOM;
	_shown[0] = kShownTrue;

	_objectState[0] = Object(_id, kStringExit, kStringDefaultDescription, NULLOBJECT, EXIT, 255, 255, 0, HALL, 22);
	_objectState[1] = Object(_id, kStringCoffin, kStringCreepy, COFFIN, NULLTYPE, 0, 0, 0);
	_objectState[2] = Object(_id, kStringToothbrush, kStringToothbrushDescription1, NULLOBJECT, NULLTYPE, 1, 1, 0);
	_objectState[3] = Object(_id, kStringToothpaste, kStringToothbrushDescription1, NULLOBJECT, NULLTYPE, 2, 2, 0);
	_objectState[4] = Object(_id, kStringBall, kStringBallDescription, L_BALL, NULLTYPE, 3, 3, 0);
	_objectState[5] = Object(_id, kStringBall, kStringBallDescription, R_BALL, NULLTYPE, 3, 3, 0);
}

void CoffinRoom::onEntrance() {
	setRoomSeen(true);
}

void CoffinRoom::animation() {
}

bool CoffinRoom::interact(Action verb, Object &obj1, Object &obj2) {
	char change;
	if (verb == ACTION_OPEN && obj1._id == COFFIN && !isSectionVisible(1)) {
		_vm->renderMessage(kStringPyramid13);
	} else if (verb == ACTION_PRESS && (obj1._id == L_BALL || obj1._id == R_BALL)) {
		if (obj1._id == L_BALL) {
			if ((change = !isSectionVisible(2))) {
				_vm->renderImage(2);
				_vm->playSound(kAudioTaxiOpen);
			} else
				_vm->renderMessage(kStringPyramid14);
		} else {
			if ((change = !isSectionVisible(3))) {
				_vm->renderImage(3);
				_vm->playSound(kAudioTaxiOpen);
			} else
				_vm->renderMessage(kStringPyramid14);
		} if (change) {
			if (isSectionVisible(2) && isSectionVisible(3)) {
				_vm->playSound(kAudioShip2);
				_vm->renderImage(4);
				_gm->wait(4);
				_vm->renderImage(5);
				_gm->wait(4);
				_vm->renderImage(5);
				_gm->wait(4);
				_vm->renderImage(5);
				_gm->wait(4);
				_vm->renderImage(1);
				setSectionVisible(4, kShownFalse);
				setSectionVisible(5, kShownFalse);
				setSectionVisible(6, kShownFalse);
				setSectionVisible(7, kShownFalse);
			}
			else
				_vm->renderMessage(kStringPyramid15);
		}
	} else if (verb == ACTION_LOOK && obj1._id == COFFIN && isSectionVisible(1)) {
		_gm->changeRoom(MASK);
		_gm->_newRoom = true;
	} else
		return false;
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
