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
 */

#ifndef DUNE_SOUND_EFFECT_H
#define DUNE_SOUND_EFFECT_H

#include "audio/audiostream.h"
#include "common/array.h"
#include "common/mutex.h"

namespace Dune {

/** DNSDB's sample/silence/repeat program, separate from speech mouth cues. */
class SoundEffectStream : public Audio::AudioStream {
public:
	bool load(const byte *data, uint32 size);
	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override { return false; }
	int getRate() const override { return _rate; }
	bool endOfData() const override;
	// AC30/DNSDB entry 0C: end the innermost active loop AFTER this iteration.
	// If still in its attack (before block 6), there is no active loop to end.
	void finishLoop();

private:
	struct Block {
		byte kind = 0;
		uint rate = 0;
		uint16 repeats = 0;
		Common::Array<int16> samples;
	};
	struct Loop {
		uint start;
		uint16 remaining;
	};
	Common::Array<Block> _blocks;
	Common::Array<Loop> _loops;
	uint _current = 0;
	uint64 _position = 0;
	uint _rate = 0;
	mutable Common::Mutex _mutex;
	void advanceBlock();
};

SoundEffectStream *makeSoundEffectStream(const byte *data, uint32 size);

} // namespace Dune

#endif
