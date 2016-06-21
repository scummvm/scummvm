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

#ifndef MACVENTURE_IMAGE_H
#define MACVENTURE_IMAGE_H

#include "macventure/macventure.h"
#include "macventure/container.h"

namespace MacVenture {

typedef uint32 ObjID;
class Container;

enum GraphicsEncoding {
	kPPIC0 = 0,
	kPPIC1 = 1,
	kPPIC2 = 2,
	kPPIC3 = 3
};

struct PPICHuff {
	uint16 masks[17];
	uint16 lens[17];
	uint8 symbols[17];
};

class ImageAsset {
public:
	ImageAsset(ObjID id, Container *container); 
	~ImageAsset();

	void blit(Graphics::ManagedSurface *target);

private:
	void decodePPIC();

	void decodePPIC0(Common::BitStream &stream); 
	void decodePPIC1(Common::BitStream &stream); 
	void decodePPIC2(Common::BitStream &stream);
	void decodePPIC3(Common::BitStream &stream);

	void decodeHuffGraphic(const PPICHuff &huff, Common::BitStream &stream); 
	byte walkHuff(const PPICHuff &huff, Common::BitStream &stream);

private:
	ObjID _id;
	Container *_container;

	uint16 _walkRepeat;
	uint16 _walkLast;

	uint16 _rowBytes;
	uint16 _bitWidth;
	uint16 _bitHeight;

	byte* _data;

	Graphics::ManagedSurface *_surface;
	Graphics::ManagedSurface *_mask;
};

} // End of namespace MacVenture

#endif
