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

#ifndef TESORO_CORE_ANIMBANK_H
#define TESORO_CORE_ANIMBANK_H

#include "common/array.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "tesoro/core/animimage.h"

namespace Tesoro {

/** One step of an animation: how long it lasts, which image it shows and optional point data. */
struct AnimSequence {
	int16 imageX;			// where the image is drawn, relative to the object
	int16 imageY;
	int16 x;				// movement of the object at this step, relative to the first sequence
	int16 y;
	uint16 duration;
	uint16 type;			// 2: dword entries, 4: point entries, otherwise none
	int16 hitX;				// hit rectangle: left, top, width, height
	int16 hitY;
	int16 hitW;
	int16 hitH;
	int16 imageIndex;		// negative: no image
	uint16 firstDescent;	// index of the first entry whose x is greater than the next one's
	Common::Array<uint32> entries;	// type 2: raw dwords; type 4: x | y << 16
	Common::Array<uint16> scales;	// type 4: one per entry

	bool hasImage() const { return imageIndex >= 0; }
};

/** The data of an animation file: sequences and their images. Shared by every player made from it. */
class AnimBank {
public:
	/** Image loading modes. */
	enum { kLoadNow = 0, kLoadDeferred = 2 };

	/** Reads the animation stored at an archive entry offset. In deferred mode the images are skipped. */
	bool load(Common::SeekableReadStream &dat, uint32 entryOffset, int mode, bool is555);

	// /** Loads the images skipped by a deferred load. */
	// bool loadDeferredImages(Common::SeekableReadStream &dat, bool is555);

	// /** Multiplies the brightness of all literal images by factor / 256, in place. */
	// void scaleBrightness(uint16 factor, bool is555);

	bool hasHeaderFlag() const { return _headerFlag; }
	uint32 headerDword(uint i) const { return _header[i]; }
	uint32 sequenceCount() const { return _sequences.size(); }
	const AnimSequence &sequence(uint i) const { return _sequences[i]; }
	uint32 imageCount() const { return _images.size(); }
	const AnimImage &image(uint i) const { return _images[i]; }
	AnimImage &mutableImage(uint i) { return _images[i]; }
	int mode() const { return _mode; }


private:
	int _mode = kLoadNow;
	bool _headerFlag = false;
	uint32 _header[3] = {};
	Common::Array<AnimSequence> _sequences;
	Common::Array<AnimImage> _images;
	Common::Array<uint32> _imagePositions;	// stream position of each image
};

} // End of namespace Tesoro

#endif
