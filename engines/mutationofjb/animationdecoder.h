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

#ifndef MUTATIONOFJB_ANIMATIONDECODER_H
#define MUTATIONOFJB_ANIMATIONDECODER_H

#include "common/rect.h"
#include "common/scummsys.h"
#include "common/str.h"

#include "graphics/surface.h"
#include "mutationofjb/encryptedfile.h"

namespace Common {
class SeekableReadStream;
}

namespace MutationOfJB {

enum {
	PALETTE_COLORS = 256,
	PALETTE_SIZE = PALETTE_COLORS * 3,
	IMAGE_WIDTH = 320,
	IMAGE_HEIGHT = 200
};

class AnimationDecoderCallback {
public:
	virtual void onFrame(int frameNo, Graphics::Surface &surface) = 0;
	virtual void onPaletteUpdated(byte palette[PALETTE_SIZE]) = 0;
	virtual ~AnimationDecoderCallback() {}
};

class AnimationDecoder {
public:
	AnimationDecoder(const Common::String &fileName);
	AnimationDecoder(const Common::String &fileName, const Graphics::Surface &outSurface);
	~AnimationDecoder();
	bool decode(AnimationDecoderCallback *callback);

	/**
	 * Enables partial decoding mode.
	 *
	 * @param fromFrame Frame to start decoding on (inclusive).
	 * @param toFrame Frame to end decoding on (inclusive).
	 * @param area Output surface will be confined to this area.
	 * @param threshold Source pixels with color index above this threshold will not be replaced.
	 */
	void setPartialMode(int fromFrame, int toFrame, const Common::Rect area = Common::Rect(), uint8 threshold = 0xFF);

private:
	void loadPalette(Common::SeekableReadStream &stream);
	void loadFullFrame(EncryptedFile &file, uint32 size);
	void loadDiffFrame(EncryptedFile &file, uint32 size);

	Common::String _fileName;
	Graphics::Surface _surface;
	bool _owningSurface;
	byte _palette[PALETTE_SIZE];
	int _fromFrame;
	int _toFrame;
	Common::Rect _area;
	uint8 _threshold;
};

}

#endif
