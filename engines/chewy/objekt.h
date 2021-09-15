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

#ifndef CHEWY_OBJEKT_H
#define CHEWY_OBJEKT_H

#include "common/stream.h"

namespace Chewy {

struct RoomMovObjekt {
	int16 RoomNr;

	int16 X;
	int16 Y;
	uint8 XOff;
	uint8 YOff;
	int16 TxtNr;
	int16 NeuObj;
	int16 ActionObj;
	uint8 ZustandAk;
	uint8 ZustandOff;
	uint8 ZustandFlipFlop;
	uint8 AutoMov;
	uint8 AniFlag;
	uint8 Del;
	uint8 Attribut;
	uint8 HeldHide;
	int16 ZEbene;

	bool load(Common::SeekableReadStream *src);
	static size_t size() { return 24; }
};

struct IibDateiHeader {
	char Id[4];
	char Tafname[14];
	uint32 Size;

	bool load(Common::SeekableReadStream *src);
};

struct RoomStaticInventar {
	int16 RoomNr;
	int16 X;
	int16 Y;
	uint8 XOff;
	uint8 YOff;
	int16 InvNr;
	int16 TxtNr;
	uint8 HideSib;
	uint8 Dummy;
	uint8 ZustandAk;
	uint8 ZustandOff;
	uint8 ZustandFlipFlop;
	uint8 AutoMov;
	uint8 AniFlag;
	uint8 HeldHide;
	int16 StaticAk;
	int16 StaticOff;

	bool load(Common::SeekableReadStream *src);
};

struct SibDateiHeader {
	char Id[4];
	int16 Anz;

	bool load(Common::SeekableReadStream *src);
};

struct RoomExit {
	int16 RoomNr;
	int16 X;
	int16 Y;
	uint8 XOff;
	uint8 YOff;
	int16 Exit;
	uint8 ExitMov;
	uint8 AutoMov;
	uint8 Attribut;
	uint8 dummy;

	bool load(Common::SeekableReadStream *src);
};

struct EibDateiHeader {
	char Id[4];
	int16 Anz;

	bool load(Common::SeekableReadStream *src);
};

#define OBJZU_AUF 0
#define OBJZU_ZU 1
#define OBJZU_AN 2
#define OBJZU_AUS 3
#define OBJZU_VERSCHLOSSEN 5
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

} // namespace Chewy

#endif
