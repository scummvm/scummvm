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
#ifndef DRAGONS_DRAGONImg_H
#define DRAGONS_DRAGONImg_H

#include "common/system.h"

namespace Dragons {

struct Img {
	uint16 x;
	uint16 y;
	uint16 w;
	uint16 h;
	uint16 layerNum;
	uint16 field_a;
	uint16 field_c;
	uint16 field_e;
	byte *data;
};

class BigfileArchive;

class DragonImg {
private:
	uint16 _count;
	Img *_imgObjects;
	byte *_imgData;

public:
	DragonImg(BigfileArchive *bigfileArchive);
	~DragonImg();
	Img *getImg(uint32 iptId);
};

} // End of namespace Dragons

#endif //DRAGONS_DRAGONImg_H
