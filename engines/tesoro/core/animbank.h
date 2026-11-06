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
struct AnimStep {
	int16 imageX;			// where the image is drawn, relative to the object
	int16 imageY;
	int16 x;				// movement of the object at this step, relative to the first step
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

/** The data of an animation file: steps and their images. Shared by every player made from it. */
class AnimBank {
public:
	/** Image loading modes. */
	enum { kLoadNow = 0, kLoadDeferred = 2 };

	/** Reads the animation stored at an archive entry offset. In deferred mode the images are skipped. */
	bool load(Common::SeekableReadStream &dat, uint32 entryOffset, int mode, bool is555);

	bool hasHeaderFlag() const { return _headerFlag; }
	uint32 headerDword(uint i) const { return _header[i]; }
	uint32 stepCount() const { return _steps.size(); }
	const AnimStep &step(uint i) const { return _steps[i]; }
	uint32 imageCount() const { return _images.size(); }
	const AnimImage &image(uint i) const { return _images[i]; }
	AnimImage &mutableImage(uint i) { return _images[i]; }
	int mode() const { return _mode; }


private:
	int _mode = kLoadNow;
	bool _headerFlag = false;
	uint32 _header[3] = {};
	Common::Array<AnimStep> _steps;
	Common::Array<AnimImage> _images;
	Common::Array<uint32> _imagePositions;	// stream position of each image
};


/**
 * Wrapper over a bank with playback state. Multiple players can share the same bank.
 * Copies share the bank and keep their own play state.
 * */
class AnimPlayer {
public:
	enum Flags {
		kPlaying = 1,
		kPaused = 2,
		kLoop = 4,
		kHeaderFlag = 8
	};

	bool load(Common::SeekableReadStream &dat, uint32 entryOffset, int mode, bool is555);

	/**
	 * Plays the steps first..last. A last of -1 extends the range up to the next step without image.
	 * sumDurations totals their durations. now is the game clock.
	 */
	void setRange(uint16 first, int last, bool loop, bool sumDurations, int32 now);

	/** Moves the play head to the step the game time has reached and updates the movement. */
	void advance(int32 now);

	void setFlags(uint32 flags) { _flags = flags; }

	/** Puts the play head on a step without touching anything else. */
	void setCurrent(uint16 step) { _current = step; }

	/** Forgets the movement computed by advance(). */
	void clearMovement() { _movementX = _movementY = 0; }
	void setMovement(int16 x, int16 y) { _movementX = x; _movementY = y; }

	AnimBank *bank() const { return _bank.get(); }
	uint32 flags() const { return _flags; }
	uint16 first() const { return _first; }
	uint16 last() const { return _last; }
	uint16 current() const { return _current; }
	int16 movementX() const { return _movementX; }
	int16 movementY() const { return _movementY; }

private:
	Common::SharedPtr<AnimBank> _bank;
	int32 _startTime = 0;
	uint32 _totalDuration = 0;
	uint32 _flags = 0;
	uint16 _first = 0;
	uint16 _last = 0;
	uint16 _current = 0;		// step being played
	int16 _loopX = 0;			// movement gathered by completed loops
	int16 _loopY = 0;
	int16 _movementX = 0;		// where the current step puts the object
	int16 _movementY = 0;
};

} // End of namespace Tesoro

#endif
