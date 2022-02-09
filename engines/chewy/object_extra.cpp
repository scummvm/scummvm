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

bool IibDateiHeader::load(Common::SeekableReadStream *src) {
	src->read(Id, 4);
	src->read(Tafname, 14);
	Size = src->readUint32LE();

	return true;
}

bool RoomStaticInventar::load(Common::SeekableReadStream *src) {
	Common::Serializer s(src, nullptr);
	synchronize(s);
	return true;
}

void RoomStaticInventar::synchronize(Common::Serializer &s) {
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

bool SibDateiHeader::load(Common::SeekableReadStream *src) {
	src->read(Id, 4);
	Anz = src->readUint16LE();

	return true;
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

bool EibDateiHeader::load(Common::SeekableReadStream *src) {
	src->read(Id, 4);
	Anz = src->readSint16LE();

	return true;
}

} // namespace Chewy
