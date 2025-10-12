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

#ifndef DIRECTOR_CASTMEMBER_RICHTEXT_H
#define DIRECTOR_CASTMEMBER_RICHTEXT_H

#include "graphics/pixelformat.h"

#include "director/types.h"
#include "director/castmember/castmember.h"

namespace Director {

class RichTextCastMember : public CastMember {
public:
	RichTextCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version);
	RichTextCastMember(Cast *cast, uint16 castId, RichTextCastMember &source);
	~RichTextCastMember();

	CastMember *duplicate(Cast *cast, uint16 castId) override { return (CastMember *)(new RichTextCastMember(cast, castId, *this)); }

	void load() override;

	Graphics::MacWidget *createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) override;

	bool hasField(int field) override;
	Datum getField(int field) override;
	void setField(int field, const Datum &value) override;

	Common::String getText() { return Common::String(_plainText); }
	uint32 getCastDataSize() override;
	void writeCastData(Common::SeekableWriteStream *writeStream) override;

	Common::String formatInfo() override;

private:
	Common::U32String _plainText;
	Graphics::PixelFormat _pf32;
	Picture *_picture;
	Picture *_pictureWithBg;

	// _initialRect
	// _boundingRect
	byte _antialiasFlag;
	byte _cropFlags;
	uint16 _scrollPos;
	uint16 _antialiasFontSize;  // Seems to be always 12
	uint16 _displayHeight;
	uint32 _foreColor;
	uint32 _bgColor;
};

} // End of namespace Director

#endif
