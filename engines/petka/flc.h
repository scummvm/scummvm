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

#ifndef PETKA_FLC_H
#define PETKA_FLC_H

#include "video/flic_decoder.h"

namespace Petka {

class FlicDecoder : public Video::FlicDecoder {
public:
	void load(Common::SeekableReadStream *flcStream, Common::SeekableReadStream *mskStream);

	void setFrame(int frame);

	uint getDelay() const;
	const Common::Rect &getBounds() const;
	const Common::Array<Common::Rect> &getMskRects() const;
	const Graphics::Surface *getCurrentFrame() const;
	uint32 getTransColor(const Graphics::PixelFormat &fmt) const;

protected:
	class FlicVideoTrack : public Video::FlicDecoder::FlicVideoTrack {
	public:
		FlicVideoTrack(Common::SeekableReadStream *stream, uint16 frameCount, uint16 width, uint16 height, bool skipHeader = false);

		const Graphics::Surface *decodeNextFrame() override;

		bool loadMsk(Common::SeekableReadStream &stream);

		uint getDelay() const;
		const Common::Rect &getBounds() const;
		const Common::Array<Common::Rect> &getMskRects() const;
		const Graphics::Surface *getSurface() const;

	private:
		Common::Rect _bounds;
		Common::Array<Common::Array<Common::Rect> > _msk;
	};
};

} // End of namespace Petka

#endif
