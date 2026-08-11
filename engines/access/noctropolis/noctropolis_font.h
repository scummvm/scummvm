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

#ifndef ACCESS_NOCTROPOLIS_NOCTROPOLIS_FONT_H
#define ACCESS_NOCTROPOLIS_NOCTROPOLIS_FONT_H

#include "access/font.h"

namespace Access {

namespace Noctropolis {

const extern uint16 CHALETEU_OFFSETS[];
const extern byte CHALETEU_DATA[];
const extern uint16 SYSTEMEU_OFFSETS[];
const extern byte SYSTEMEU_DATA[];
const extern uint16 SML3X5_OFFSETS[];
const extern byte SML3X5_DATA[];
const extern uint16 NAPLES12_OFFSETS[];
const extern byte NAPLES12_DATA[];
const extern uint16 GOTHICEU_OFFSETS[];
const extern byte GOTHICEU_DATA[];
const extern uint16 CHALETSE_OFFSETS[];
const extern byte CHALETSE_DATA[];
const extern uint16 COMICSEU_OFFSETS[];
const extern byte COMICSEU_DATA[];

class NoctropolisFont : public Font {
public:
	NoctropolisFont(uint16 bytes, int16 height, byte firstchar, byte lastchar, const uint16 *offsets, const byte *data);

private:
	void load(const uint16 *offsets, const byte *data);
	const byte *getCharDataPtr(byte ch, const uint16 *offsets, const byte *data) const;
	uint getCharBitOffset(byte ch, const uint16 *offsets) const;
	uint getCharWidth(byte ch, const uint16 *offsets) const;
	void makeCharSurface(Graphics::Surface &surface, byte ch, const uint16 *offsets, const byte *data) const;

	byte _lastChar;
	uint16 _bytes;
};

} // end namespace Noctropolis

} // end namespace Access

#endif // ACCESS_NOCTROPOLIS_NOCTROPOLIS_FONT_H
