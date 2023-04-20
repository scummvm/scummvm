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

#ifndef DIRECTOR_CASTMEMBER_BITMAP_H
#define DIRECTOR_CASTMEMBER_BITMAP_H

#include "director/castmember/castmember.h"

namespace Image {
class ImageDecoder;
}

namespace Director {

class BitmapCastMember : public CastMember {
public:
	BitmapCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint32 castTag, uint16 version, uint8 flags1 = 0);
	BitmapCastMember(Cast *cast, uint16 castId, Image::ImageDecoder *img, uint8 flags1 = 0);
	~BitmapCastMember();
	Graphics::MacWidget *createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) override;

	bool isModified() override;
	void createMatte(Common::Rect &bbox);
	Graphics::Surface *getMatte(Common::Rect &bbox);
	void copyStretchImg(Graphics::Surface *surface, const Common::Rect &bbox, const byte *pal = 0);

	bool hasField(int field) override;
	Datum getField(int field) override;
	bool setField(int field, const Datum &value) override;

	Common::String formatInfo() override;

	PictureReference *getPicture() const;
	void setPicture(PictureReference &picture);
	void setPicture(Image::ImageDecoder &image, bool adjustSize);

	Picture *_picture = nullptr;
	Graphics::Surface *_ditheredImg;
	Graphics::FloodFill *_matte;

	uint16 _pitch;
	uint16 _regX;
	uint16 _regY;
	uint16 _flags2;
	uint16 _bytes;
	int _clut;
	int _ditheredTargetClut;

	uint16 _bitsPerPixel;

	uint32 _tag;
	bool _noMatte;
	bool _external;
};

} // End of namespace Director

#endif
