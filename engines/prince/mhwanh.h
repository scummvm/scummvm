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

#ifndef PRINCE_MHWANH_H
#define PRINCE_MHWANH_H

#include "image/image_decoder.h"
#include "image/bmp.h"

#include "graphics/surface.h"

#include "resource.h"

namespace Prince {

class MhwanhDecoder : public Image::ImageDecoder {
public:
	MhwanhDecoder();
	~MhwanhDecoder() override;

	// ImageDecoder API
	void destroy() override;
	bool loadStream(Common::SeekableReadStream &stream) override;
	Graphics::Surface *getSurface() const override { return _surface; }
	const byte *getPalette() const override { return _palette; }
	uint16 getPaletteCount() const { return kPaletteColorCount; }
	static const uint16 kPaletteColorCount = 256;

private:
	Graphics::Surface *_surface;
	byte *_palette;
};

} // End of namespace Prince

#endif
