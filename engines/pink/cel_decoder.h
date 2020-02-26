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
	bool loadStream(Common::SeekableReadStream *stream) override;

	uint16 getTransparentColourIndex() const;

	Common::Point getCenter() const;
	const Graphics::Surface *getCurrentFrame() const;
	void skipFrame();

	void setEndOfTrack();

protected:
	class CelVideoTrack : public FlicVideoTrack {
	public:
		CelVideoTrack(Common::SeekableReadStream *stream, uint16 frameCount, uint16 width, uint16 height, bool skipHeader = false);
		void readHeader() override;

		uint16 getTransparentColourIndex() const;

		// Hack. Pink needs so that Track needed an update after lastFrame delay ends
		void setEndOfTrack() { _curFrame = _frameCount; }
		bool endOfTrack() const override { return getCurFrame() >= getFrameCount(); }

		Common::Point getCenter() const;
		const Graphics::Surface *getCurrentFrame() const;

		void skipFrame();

		bool rewind() override;

	private:
		const Graphics::Surface *decodeNextFrame() override;
		void readPrefixChunk();

		Common::Point _center;
		byte _transparentColourIndex;
	};
};

}

#endif
