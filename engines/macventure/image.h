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

/*
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#ifndef MACVENTURE_IMAGE_H
#define MACVENTURE_IMAGE_H

#include "macventure/macventure.h"
#include "macventure/container.h"
#include "common/bitstream.h"

namespace MacVenture {

typedef uint32 ObjID;
class Container;


enum BlitMode {
	kBlitDirect = 0,
	kBlitBIC = 1,
	kBlitOR = 2,
	kBlitXOR = 3
};

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
	ImageAsset(ObjID original, Container *container);
	~ImageAsset();

	void blitInto(Graphics::ManagedSurface *target, int x, int y, BlitMode mode);

	bool isPointInside(Common::Point point);
	bool isRectInside(Common::Rect rect);

	int getWidth();
	int getHeight();

private:
	void decodePPIC(ObjID id, Common::Array<byte> &data, uint &bitHeight, uint &bitWidth, uint &rowBytes);

	void decodePPIC0(Common::BitStream32BEMSB &stream, Common::Array<byte> &data, uint bitHeight, uint bitWidth, uint rowBytes);
	void decodePPIC1(Common::BitStream32BEMSB &stream, Common::Array<byte> &data, uint bitHeight, uint bitWidth, uint rowBytes);
	void decodePPIC2(Common::BitStream32BEMSB &stream, Common::Array<byte> &data, uint bitHeight, uint bitWidth, uint rowBytes);
	void decodePPIC3(Common::BitStream32BEMSB &stream, Common::Array<byte> &data, uint bitHeight, uint bitWidth, uint rowBytes);

	void decodeHuffGraphic(const PPICHuff &huff, Common::BitStream32BEMSB &stream, Common::Array<byte> &data, uint bitHeight, uint bitWidth, uint rowBytes);
	byte walkHuff(const PPICHuff &huff, Common::BitStream32BEMSB &stream);

	void blitDirect(Graphics::ManagedSurface *target, int ox, int oy, const Common::Array<byte> &data, uint bitHeight, uint bitWidth, uint rowBytes);
	void blitBIC(Graphics::ManagedSurface *target, int ox, int oy, const Common::Array<byte> &data, uint bitHeight, uint bitWidth, uint rowBytes);
	void blitOR(Graphics::ManagedSurface *target, int ox, int oy, const Common::Array<byte> &data, uint bitHeight, uint bitWidth, uint rowBytes);
	void blitXOR(Graphics::ManagedSurface *target, int ox, int oy, const Common::Array<byte> &data, uint bitHeight, uint bitWidth, uint rowBytes);

	void calculateSectionToDraw(Graphics::ManagedSurface *target, int &ox, int &oy, uint bitWidth, uint bitHeight, uint &sx, uint &sy, uint &w, uint &h);
	void calculateSectionInDirection(uint targetWhole, uint originWhole, int &originPosition, uint &startPosition, uint &blittedWhole);

private:
	ObjID _id;
	ObjID _mask;
	Container *_container;

	uint16 _walkRepeat;
	uint16 _walkLast;

	Common::Array<byte> _imgData;
	uint16 _imgRowBytes;
	uint16 _imgBitWidth;
	uint16 _imgBitHeight;

	Common::Array<byte> _maskData;
	uint16 _maskRowBytes;
	uint16 _maskBitWidth;
	uint16 _maskBitHeight;
};

} // End of namespace MacVenture

#endif
