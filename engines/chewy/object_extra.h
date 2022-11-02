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

#ifndef CHEWY_OBJECT_EXTRA_H
#define CHEWY_OBJECT_EXTRA_H

#include "common/serializer.h"
#include "common/stream.h"

namespace Chewy {

#define OBJZU_AUF 0
#define OBJZU_ZU 1
#define OBJZU_AN 2
#define OBJZU_AUS 3
#define OBJZU_LOCKED 5
#define OBJZU_GEOEFFNET 6
#define OBJZU_NASS 7
#define OBJZU_TROCKEN 8
#define OBJZU_OELIG 9
#define OBJZU_HEISS 10
#define OBJZU_KALT 11
#define OBJZU_LEER 12
#define OBJZU_VOLL 13
#define OBJZU_UNDICHT 14
#define OBJZU_KLEMMT 15
#define SIB_GET_INV 16

struct RoomMovObject {
	int16 RoomNr = 0;

	int16 X = 0;
	int16 Y = 0;
	uint8 XOff = 0;
	uint8 YOff = 0;
	int16 TxtNr = 0;
	int16 NeuObj = 0;
	int16 ActionObj = 0;
	uint8 ZustandAk = 0;
	uint8 ZustandOff = 0;
	uint8 ZustandFlipFlop = 0;
	uint8 AutoMov = 0;
	uint8 AniFlag = 0;
	uint8 Del = 0;
	uint8 Attribut = 0;
	uint8 HeldHide = 0;
	int16 ZEbene = 0;

	void synchronize(Common::Serializer &s);
	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE() { return 24; }
};

struct IibFileHeader {
	char Id[4];
	char Tafname[14];
	uint32 Size;

	bool load(Common::SeekableReadStream *src);
	IibFileHeader();
};

struct RoomStaticInventory {
	int16 RoomNr = 0;
	int16 X = 0;
	int16 Y = 0;
	uint8 XOff = 0;
	uint8 YOff = 0;
	int16 InvNr = 0;
	int16 TxtNr = 0;
	uint8 HideSib = 0;
	uint8 Dummy = 0;
	uint8 ZustandAk = 0;
	uint8 ZustandOff = 0;
	uint8 ZustandFlipFlop = 0;
	uint8 AutoMov = 0;
	uint8 AniFlag = 0;
	uint8 HeldHide = 0;
	int16 StaticAk = 0;
	int16 StaticOff = 0;

	void synchronize(Common::Serializer &s);
	bool load(Common::SeekableReadStream *src);
};

struct SibFileHeader {
	char _id[4];
	int16 _nr;

	bool load(Common::SeekableReadStream *src);

	SibFileHeader();
};

struct RoomExit {
	int16 RoomNr = 0;
	int16 X = 0;
	int16 Y = 0;
	uint8 XOff = 0;
	uint8 YOff = 0;
	int16 Exit = 0;
	uint8 ExitMov = 0;
	uint8 AutoMov = 0;
	uint8 Attribut = 0;
	uint8 dummy = 0;

	void synchronize(Common::Serializer &s);
	bool load(Common::SeekableReadStream *src);
};

struct EibFileHeader {
	char _id[4];
	int16 _nr;

	bool load(Common::SeekableReadStream *src);
	EibFileHeader();
};

} // namespace Chewy

#endif
