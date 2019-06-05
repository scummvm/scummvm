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
			if (_shown[kMaxSection - 2]) {
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
	}
	else 
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
	_objectState[5] = Object(_id, kStringEntrance, kStringDefaultDescription, NULLOBJECT, EXIT | OPENABLE | OPENED | CLOSED, 4, 4, 0, APARTMENT, 3);
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
	char input[3];

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
