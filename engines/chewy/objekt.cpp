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

#include "chewy/objekt.h"

namespace Chewy {

bool RoomMovObjekt::load(Common::SeekableReadStream *src) {
	RoomNr = src->readSint16LE();

	X = src->readSint16LE();
	Y = src->readSint16LE();
	XOff = src->readByte();
	YOff = src->readByte();
	TxtNr = src->readSint16LE();
	NeuObj = src->readSint16LE();
	ActionObj = src->readSint16LE();
	ZustandAk = src->readByte();
	ZustandOff = src->readByte();
	ZustandFlipFlop = src->readByte();
	AutoMov = src->readByte();
	AniFlag = src->readByte();
	Del = src->readByte();
	Attribut = src->readByte();
	HeldHide = src->readByte();
	ZEbene = src->readSint16LE();

	return true;
}

bool IibDateiHeader::load(Common::SeekableReadStream *src) {
	src->read(Id, 4);
	src->read(Tafname, 14);
	Size = src->readUint32LE();

	return true;
}

bool RoomStaticInventar::load(Common::SeekableReadStream *src) {
	RoomNr = src->readSint16LE();
	X = src->readSint16LE();
	Y = src->readSint16LE();
	XOff = src->readByte();
	YOff = src->readByte();
	InvNr = src->readSint16LE();
	TxtNr = src->readSint16LE();
	HideSib = src->readByte();
	Dummy = src->readByte();
	ZustandAk = src->readByte();
	ZustandOff = src->readByte();
	ZustandFlipFlop = src->readByte();
	AutoMov = src->readByte();
	AniFlag = src->readByte();
	HeldHide = src->readByte();
	StaticAk = src->readSint16LE();
	StaticOff = src->readSint16LE();

	return true;
}

bool SibDateiHeader::load(Common::SeekableReadStream *src) {
	src->read(Id, 4);
	Anz = src->readUint16LE();

	return true;
}

bool RoomExit::load(Common::SeekableReadStream *src) {
	RoomNr = src->readSint16LE();
	X = src->readSint16LE();
	Y = src->readSint16LE();
	XOff = src->readByte();
	YOff = src->readByte();
	Exit = src->readSint16LE();
	ExitMov = src->readByte();
	AutoMov = src->readByte();
	Attribut = src->readByte();
	dummy = src->readByte();

	return true;
}

bool EibDateiHeader::load(Common::SeekableReadStream *src) {
	src->read(Id, 4);
	Anz = src->readSint16LE();

	return true;
}

} // namespace Chewy
