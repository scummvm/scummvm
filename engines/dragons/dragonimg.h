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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef SCUMMVM_DRAGONIMG_H
#define SCUMMVM_DRAGONIMG_H

#include "common/system.h"

namespace Dragons {

struct IMG {
	uint16 field_0;
	uint16 field_2;
	uint16 field_4;
	uint16 field_6;
	uint16 field_8;
	uint16 field_a;
	uint16 field_c;
	uint16 field_e;
	byte *data;
};

class BigfileArchive;

class DragonIMG {
private:
	int16 _count;
	IMG *_imgObjects;
	byte *_imgData;
public:
	DragonIMG(BigfileArchive *bigfileArchive);
	~DragonIMG();
	IMG *getIMG(uint32 iptId);
};

} // End of namespace Dragons

#endif //SCUMMVM_DRAGONIMG_H
