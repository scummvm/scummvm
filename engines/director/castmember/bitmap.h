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
	BitmapCastMember(Cast *cast, uint16 castId, BitmapCastMember &source);
	~BitmapCastMember();

	CastMember *duplicate(Cast *cast, uint16 castId) override { return (CastMember *)(new BitmapCastMember(cast, castId, *this)); }

	Graphics::MacWidget *createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) override;

	bool isModified() override;
	void createMatte(const Common::Rect &bbox);
	Graphics::Surface *getMatte(const Common::Rect &bbox);
	Graphics::Surface *getDitherImg();

	bool hasField(int field) override;
	Datum getField(int field) override;
	void setField(int field, const Datum &value) override;

	Common::String formatInfo() override;

	void load() override;
	void unload() override;

	PictureReference *getPicture() const;
	void setPicture(PictureReference &picture);
	void setPicture(Image::ImageDecoder &image, bool adjustSize);

	Common::Point getRegistrationOffset() override;
	Common::Point getRegistrationOffset(int16 width, int16 height) override;

	CollisionTest isWithin(const Common::Rect &bbox, const Common::Point &pos, InkType ink) override;

	void writeCastData(Common::SeekableWriteStream *writeStream) override;
	uint32 writeBITDResource(Common::SeekableWriteStream *writeStream, uint32 offset);

	uint32 getCastDataSize() override;			// This is the size of the data in the 'CASt' resource
	uint32 getBITDResourceSize();

	Picture *_picture = nullptr;
	Graphics::Surface *_ditheredImg;
	Graphics::Surface *_matte;

	int _version;

	uint16 _flags2;
	uint16 _bytes;
	CastMemberID _clut;
	CastMemberID _ditheredTargetClut;

	uint32 _tag;
	bool _noMatte;
	bool _external;

	// D4 stucture:
	// uint16 _pitch;
	// _initialRect			// 2
	// _boundingRect		// 10
	// int16 _regY;			// 18
	// int16 _regX;			// 20
	// uint8 _bitsPerPixel;	// 22  when _pitch & 0x8000

	// D6+ structure
	uint16 _pitch;
	// _initialRect			// 2
	// _boundingRect		// 10  D%-
	byte _alphaThreshold; 	// 10  D7+
	   // padding
	uint16 _editVersion;	// 12  D6+
	Common::Point _scrollPoint; // 14
	int16 _regY;			// 18
	int16 _regX;			// 20
	byte _updateFlags;		// 21

	uint8 _bitsPerPixel;	// 22  when _pitch & 0x8000

	// These sit in _flags1
	enum {
		kFlagCenterRegPointD4	= 0x01,	// centerRegPoint property
		kFlagMatteD4			= 0x20,	// double check value
	};

	enum {
		kFlagDither 			= 0x01,	// Bitmap needs dithering
		kFlagRemapPalette 		= 0x02,	// Bitmap needs palette remap
		kFlagSyncPalette 		= 0x04,	// When bitmap comes from outside, sync its palette castmember
		kFlagDitherOnUpdate 	= 0x08,	// Forced dither

		// D7+
		kFlagFollowAlpha 		= 0x10,	// Alpha channel for 32-bit images must be followed
		kFlagCenterRegPoint 	= 0x20,	// centerRegPoint property
		kFlagMatte 				= 0x40,	// it is used for matte image
		kFlagNoAutoCrop 		= 0x80,	// do not automatically crop the image
	};

};

} // End of namespace Director

#endif
