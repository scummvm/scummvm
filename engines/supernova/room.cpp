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

#include "supernova/screen.h"
#include "supernova/supernova.h"
#include "supernova/game-manager.h"

namespace Supernova {

Room::Room() {
	_seen = false;
	_fileNumber = 0;
	_id = NULLROOM;
	_vm = nullptr;

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
		_objectState[i]._name = in->readSint32LE();
		_objectState[i]._description = in->readSint32LE();
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
		_shown[section - kSectionInvert] = visible ? kShownFalse : kShownTrue;
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

}
