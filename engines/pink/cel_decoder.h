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

#ifndef PINK_CEL_DECODER_H
#define PINK_CEL_DECODER_H

#include "video/flic_decoder.h"

namespace Pink {

class CelDecoder : public Video::FlicDecoder {
public:
	virtual bool loadStream(Common::SeekableReadStream *stream);

	int32 getX();
	int32 getY();
	uint16 getTransparentColourIndex();

	Common::Point getCenter();
	Common::Rect &getRectangle();
	const Graphics::Surface *getCurrentFrame();
	void skipFrame();

	void setX(int32 x);
	void setY(int32 y);

protected:
	class CelVideoTrack : public FlicVideoTrack {
	public:
		CelVideoTrack(Common::SeekableReadStream *stream, uint16 frameCount, uint16 width, uint16 height, bool skipHeader = false);
		virtual void readHeader();

		int32 getX() const;
		int32 getY() const;
		uint16 getTransparentColourIndex();

		Common::Point getCenter();
		Common::Rect &getRect();
		const Graphics::Surface *getCurrentFrame();

		void setX(int32 x);
		void setY(int32 y);

		void skipFrame();
	private:
		const Graphics::Surface *decodeNextFrame();
		void readPrefixChunk();

		Common::Point _center;
		Common::Rect _rect;
		byte _transparentColourIndex;
	};
};

}

#endif
