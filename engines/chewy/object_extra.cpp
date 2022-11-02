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

#include "chewy/object_extra.h"

namespace Chewy {

bool RoomMovObject::load(Common::SeekableReadStream *src) {
	Common::Serializer s(src, nullptr);
	synchronize(s);
	return true;
}

void RoomMovObject::synchronize(Common::Serializer &s) {	
	s.syncAsSint16LE(RoomNr);

	s.syncAsSint16LE(X);
	s.syncAsSint16LE(Y);
	s.syncAsByte(XOff);
	s.syncAsByte(YOff);
	s.syncAsSint16LE(TxtNr);
	s.syncAsSint16LE(NeuObj);
	s.syncAsSint16LE(ActionObj);
	s.syncAsByte(ZustandAk);
	s.syncAsByte(ZustandOff);
	s.syncAsByte(ZustandFlipFlop);
	s.syncAsByte(AutoMov);
	s.syncAsByte(AniFlag);
	s.syncAsByte(Del);
	s.syncAsByte(Attribut);
	s.syncAsByte(HeldHide);
	s.syncAsSint16LE(ZEbene);
}

bool IibFileHeader::load(Common::SeekableReadStream *src) {
	src->read(Id, 4);
	src->read(Tafname, 14);
	Size = src->readUint32LE();

	return true;
}

IibFileHeader::IibFileHeader() {
	memset(Id, 0, 4);
	memset(Tafname, 0, 14);
	Size = 0;
}

bool RoomStaticInventory::load(Common::SeekableReadStream *src) {
	Common::Serializer s(src, nullptr);
	synchronize(s);
	return true;
}

void RoomStaticInventory::synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(RoomNr);
	s.syncAsSint16LE(X);
	s.syncAsSint16LE(Y);
	s.syncAsByte(XOff);
	s.syncAsByte(YOff);
	s.syncAsSint16LE(InvNr);
	s.syncAsSint16LE(TxtNr);
	s.syncAsByte(HideSib);
	s.syncAsByte(Dummy);
	s.syncAsByte(ZustandAk);
	s.syncAsByte(ZustandOff);
	s.syncAsByte(ZustandFlipFlop);
	s.syncAsByte(AutoMov);
	s.syncAsByte(AniFlag);
	s.syncAsByte(HeldHide);
	s.syncAsSint16LE(StaticAk);
	s.syncAsSint16LE(StaticOff);
}

bool SibFileHeader::load(Common::SeekableReadStream *src) {
	src->read(_id, 4);
	_nr = src->readUint16LE();

	return true;
}

SibFileHeader::SibFileHeader() {
	for (int i = 0; i < 4; ++i) {
		_id[i] = 0;
	}
	_nr = 0;
}

bool RoomExit::load(Common::SeekableReadStream *src) {
	Common::Serializer s(src, nullptr);
	synchronize(s);
	return true;
}

void RoomExit::synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(RoomNr);
	s.syncAsSint16LE(X);
	s.syncAsSint16LE(Y);
	s.syncAsByte(XOff);
	s.syncAsByte(YOff);
	s.syncAsSint16LE(Exit);
	s.syncAsByte(ExitMov);
	s.syncAsByte(AutoMov);
	s.syncAsByte(Attribut);
	s.syncAsByte(dummy);
}

bool EibFileHeader::load(Common::SeekableReadStream *src) {
	src->read(_id, 4);
	_nr = src->readSint16LE();

	return true;
}

EibFileHeader::EibFileHeader() {
	for (int i = 0; i < 4; ++i) {
		_id[i] = 0;
	}
	_nr = 0;
}

} // namespace Chewy
