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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SCUMM_INSANE_MORTIMER_MOVIE_H
#define SCUMM_INSANE_MORTIMER_MOVIE_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

namespace Common {
class SeekableReadStream;
}

namespace Scumm {

class SmushDeltaBlocksDecoder;

struct MortimerSprite {
	int x, y, width, height;
	Common::Array<byte> pixels;
};

struct MortimerSound {
	uint frame = 0;
	uint rate = 11025;
	byte volume = 255;
	int8 balance = 0;
	bool music = false;
	bool speech = true;
	Common::Array<byte> samples;
};

bool loadMortimerSprites(Common::SeekableReadStream &stream, Common::Array<MortimerSprite> &sprites, byte *palette = nullptr);
bool loadMortimerSound(Common::SeekableReadStream &stream, MortimerSound &sound);

// Mortimer's SMUSH movie reader. Gameplay supplies the IACT and SKIP rules.
class MortimerMovie {
public:
	enum { kScreenWidth = 640, kScreenHeight = 480 };
	struct Caption {
		int16 x, y;
		uint16 id;
	};

	MortimerMovie();
	virtual ~MortimerMovie();
	bool load(Common::SeekableReadStream &stream, bool doubled = true);
	virtual void reset();
	// Suppress compositing while seeking to a frame that replaces the image.
	virtual bool advance(bool renderFrame = true);
	void setBackground(const byte *pixels);

	const byte *pixels() const { return _pixels.begin(); }
	const byte *palette() const { return _palette; }
	uint frame() const { return _nextFrame ? _nextFrame - 1 : 0; }
	uint frameCount() const { return _frames.size(); }
	uint frameRate() const { return _frameRate; }
	const Common::Array<MortimerSound> &sounds() const { return _sounds; }
	const Common::Array<Caption> &captions() const { return _captions; }
	const Common::String &errorMessage() const { return _error; }

protected:
	virtual void handleAction(const byte *data, uint32 size);
	virtual bool skipObject(uint16 a, uint16 b) const { return false; }
	virtual int objectScale(byte codec) const;
	void render();

private:
	struct Frame {
		uint32 offset, size;
	};
	struct Object {
		uint32 offset = 0, size = 0;
		uint16 skipA = 0xffff, skipB = 0;
		int x = 0, y = 0, width = 0, height = 0, scale = 1;
		byte codec = 0;
		bool transparent = false;
		byte remap[256] = {};
		Common::Array<byte> pixels;
	};

	bool fail(const char *message);
	void handlePalette(const byte *data, uint32 size);
	bool decodeObject(Object &object);

	Common::Array<byte> _data, _pixels, _previous, _scratch, _background;
	Common::Array<Frame> _frames;
	Common::Array<Object> _objects;
	Common::Array<MortimerSound> _sounds;
	Common::Array<Caption> _captions;
	Object _storedObject;
	SmushDeltaBlocksDecoder *_decoder;
	Common::String _error;
	byte _palette[768], _initialPalette[768], _remap[256];
	int16 _paletteDelta[768];
	int32 _shiftedPalette[768];
	int _decoderWidth, _decoderHeight;
	bool _doubled, _initialDoubled, _storeNext;
	uint _nextFrame, _frameRate;
};

} // End of namespace Scumm

#endif
