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


#include "engines/engine.h"
#include "scumm/players/player_v3a.h"
#include "scumm/scumm.h"

namespace Scumm {

Player_V3A::Player_V3A(ScummEngine *scumm, Audio::Mixer *mixer)
	: Paula(true, mixer->getOutputRate(), mixer->getOutputRate() / (scumm->getAmigaMusicTimerFrequency() / 4)),
	  _vm(scumm),
	  _mixer(mixer),
	  _soundHandle(),
	  _songData(nullptr),
	  _wavetableData(nullptr),
	  _wavetablePtrs(nullptr),
	  _musicTimer(0),
	  _initState(kInitStateNotReady) {

	assert(scumm);
	assert(mixer); // this one's a bit pointless, since we had to dereference it to initialize Paula
	assert((_vm->_game.id == GID_INDY3) || (_vm->_game.id == GID_LOOM));

	stopAllSounds();

	// As in the original game, the same Paula is shared between both SFX and music and plays continuously.
	// Doing them separately would require subclassing Paula and creating two instances
	// (since all of the important methods are protected)
	startPaula();

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

bool Player_V3A::init() {
	byte *ptr;
	int numInstruments;

	// Determine which sound resource contains the wavetable data and how large it is
	// This is hardcoded into each game's executable
	if (_vm->_game.id == GID_INDY3) {
		ptr = _vm->getResourceAddress(rtSound, 83);
		numInstruments = 12;
	} else if (_vm->_game.id == GID_LOOM) {
		ptr = _vm->getResourceAddress(rtSound, 79);
		numInstruments = 9;
	} else {
		error("player_v3a - unknown game");
		return false;
	}
	if (!ptr) {
		error("player_v3a - unable to load music samples resource");
		return false;
	}

	// Keep a copy of the resource data, since the original pointer may eventually go bad
	int length = READ_LE_UINT16(ptr);
	_wavetableData = new int8[length];
	if (!_wavetableData) {
		error("player_v3a - failed to allocate copy of wavetable data");
		return false;
	}
	memcpy(_wavetableData, ptr, length);

	int offset = 4;

	// Parse the header tables into a more convenient structure
	_wavetablePtrs = new InstData[numInstruments];
	for (int i = 0; i < numInstruments; i++) {

		// Each instrument defines 6 octaves
		for (int j = 0; j < 6; j++) {
			// Offset/length for intro/main component
			int dataOff = READ_BE_UINT16(_wavetableData + offset + 0);
			int dataLen = READ_BE_UINT16(_wavetableData + offset + 2);

			if (dataLen) {
				_wavetablePtrs[i].mainLen[j] = dataLen;
				_wavetablePtrs[i].mainData[j] = &_wavetableData[dataOff];
			} else {
				_wavetablePtrs[i].mainLen[j] = 0;
				_wavetablePtrs[i].mainData[j] = nullptr;
			}

			// Offset/length for looped component, if any
			dataOff = READ_BE_UINT16(ptr + offset + 4);
			dataLen = READ_BE_UINT16(ptr + offset + 6);

			if (dataLen) {
				_wavetablePtrs[i].loopLen[j] = dataLen;
				_wavetablePtrs[i].loopData[j] = &_wavetableData[dataOff];
			} else {
				_wavetablePtrs[i].loopLen[j] = 0;
				_wavetablePtrs[i].loopData[j] = nullptr;
			}

			// Octave shift for this octave
			_wavetablePtrs[i].octave[j] = READ_BE_INT16(ptr + offset + 8);
			offset += 10;
		}

		// Fadeout rate, in 1/256ths of a volume level
		_wavetablePtrs[i].volumeFade = READ_BE_INT16(ptr + offset);
		offset += 2;

		if (_vm->_game.id == GID_LOOM) {
			// Loom's sound samples aren't all in tune with each other,
			// so it stores an extra adjustment here
			_wavetablePtrs[i].pitchAdjust = READ_BE_INT16(ptr + offset);
			offset += 2;
		} else {
			_wavetablePtrs[i].pitchAdjust = 0;
		}
	}
	return true;
}

Player_V3A::~Player_V3A() {
	_mixer->stopHandle(_soundHandle);
	if (_initState == kInitStateReady) {
		delete[] _wavetableData;
		delete[] _wavetablePtrs;
	}
}

void Player_V3A::setMusicVolume (int vol) {
	_mixer->setChannelVolume(_soundHandle, CLIP<int>(vol, 0, 255));
}

void Player_V3A::stopAllSounds() {
	for (int i = 0; i < 4; i++) {
		clearVoice(i);
		_channels[i].resourceId = -1;
	}
	_curSong = -1;
	_songPtr = 0;
	_songDelay = 0;
	_songData = nullptr;
}

void Player_V3A::stopSound(int nr) {
	if (nr <= 0)
		return;

	for (int i = 0; i < 4; i++) {
		if (_channels[i].resourceId == nr) {
			clearVoice(i);
			_channels[i].resourceId = -1;
		}
	}
	if (nr == _curSong) {
		_curSong = -1;
		_songDelay = 0;
		_songPtr = 0;
		_songData = nullptr;
	}
}

void Player_V3A::startSound(int nr) {
	assert(_vm);
	int8 *data = (int8 *)_vm->getResourceAddress(rtSound, nr);
	if (!data)
		return;

	if ((_vm->_game.id != GID_INDY3) && (_vm->_game.id != GID_LOOM))
		error("player_v3a - unknown game");

	if (_initState == kInitStateNotReady)
		_initState = init() ? kInitStateReady : kInitStateFailed;

	// is this a Music resource?
	if (data[26]) {
		if (_initState == kInitStateReady) {
			stopAllSounds();
			for (int i = 0; i < 4; i++) {
				_channels[i].haltTimer = 0;
				_channels[i].resourceId = nr;
				_channels[i].priority = READ_BE_UINT16(data + 4);
			}

			// Keep a local copy of the song data
			_songData = data;
			_curSong = nr;
			_songPtr = 0;
			_songDelay = 1;

			// Start timer at 0 and increment every 30 frames (see below)
			_musicTimer = 0;
		} else {
			// debug("player_v3a - wavetable unavailable, cannot play music");
		}
	} else {
		int priority = READ_BE_UINT16(data + 4);
		int channel = READ_BE_UINT16(data + 6);
		if (_channels[channel].resourceId != -1 && _channels[channel].priority > priority)
			return;

		int chan1 = SFX_CHANNEL_MAP[channel][0];
		int chan2 = SFX_CHANNEL_MAP[channel][1];

		int offsetL = READ_BE_UINT16(data + 8);
		int offsetR = READ_BE_UINT16(data + 10);
		int lengthL = READ_BE_UINT16(data + 12);
		int lengthR = READ_BE_UINT16(data + 14);

		// Period and Volume are both stored in fixed-point
		_channels[chan1].period = READ_BE_UINT16(data + 20) << 16;
		_channels[chan2].period = READ_BE_UINT16(data + 22) << 16;
		_channels[chan1].volume = data[24] << 8;
		_channels[chan2].volume = data[25] << 8;
		_channels[chan1].loopCount = data[27];
		_channels[chan2].loopCount = data[27];

		int sweepOffset = READ_BE_UINT16(data + 16);
		if (sweepOffset) {
			// This data contains a list of offset/value pairs, processed in sequence
			// The offset points into a data structure in the original sound engine
			// Offset 0x18 sets the channel's Sweep Rate (fractional)
			// Offset 0x2C with nonzero value delays until reading the next packet
			// Offset 0x2C with zero value stops playback immediately
			// The other offsets are unknown, but they are never used

			// Indy3 always uses 0x18, 0x2C-nonzero, then 0x2C-zero
			// Loom doesn't use these at all

			for (int i = 0; i < 3; i++)
			{
				int offset = READ_BE_UINT32(data + sweepOffset + i*8 + 0);
				int value = READ_BE_INT32(data + sweepOffset + i*8 + 4);
				if (offset == 0x18)
				{
					_channels[chan1].sweepRate = value;
					_channels[chan2].sweepRate = value;
				}
				if (offset == 0x2c && value != 0)
				{
					_channels[chan1].haltTimer = value;
					_channels[chan2].haltTimer = value;
				}
			}
		} else {
			_channels[chan1].sweepRate = 0;
			_channels[chan1].haltTimer = 0;
		}

		_channels[chan1].priority = priority;
		_channels[chan2].priority = priority;
		_channels[chan1].resourceId = nr;
		_channels[chan2].resourceId = nr;

		// Start the Paula playing it
		setChannelInterrupt(chan1, true);
		setChannelInterrupt(chan2, true);
		setChannelPeriod(chan1, MAX((_channels[chan1].period >> 16) & 0xFFFF, 124));
		setChannelPeriod(chan2, MAX((_channels[chan2].period >> 16) & 0xFFFF, 124));
		setChannelVolume(chan1, MIN((_channels[chan1].volume >> 8) & 0x3F, 0x3F));
		setChannelVolume(chan2, MIN((_channels[chan2].volume >> 8) & 0x3F, 0x3F));

		// Start as looped, then generate interrupts to handle looping properly
		setChannelData(chan1, (int8 *)data + offsetL, (int8 *)data + offsetL, lengthL, lengthL);
		setChannelData(chan2, (int8 *)data + offsetR, (int8 *)data + offsetR, lengthR, lengthR);
		interruptChannel(chan1);
		interruptChannel(chan2);
	}
}

void Player_V3A::interrupt() {
	if (_vm->_game.id == GID_INDY3) {
		updateMusicIndy();
	} else if (_vm->_game.id == GID_LOOM) {
		updateMusicLoom();
	}
	updateSounds();
}

void Player_V3A::interruptChannel(byte channel) {
	// check looping
	if (_channels[channel].loopCount == -1)
		return;

	if (_channels[channel].loopCount) {
		_channels[channel].loopCount--;
		if (_channels[channel].loopCount <= 0) {
			// On the last loop, set it to no longer repeat
			setChannelInterrupt(channel, false);
			setChannelSampleStart(channel, nullptr);
			setChannelSampleLen(channel, 0);

			// If there was no music playing, mark the channel as Unused
			if (_curSong == -1)
				_channels[channel].resourceId = -1;
		}
	}
}

void Player_V3A::updateSounds() {
	for (int i = 0; i < 4; i++) {
		if (!_channels[i].loopCount)
			continue;

		setChannelVolume(i, MIN((_channels[i].volume >> 8) & 0x3F, 0x3F));
		setChannelPeriod(i, MAX((_channels[i].period >> 16) & 0xFFFF, 124));

		// Only process ones that are sweeping, since others are handled by interruptChannel above
		if (!_channels[i].sweepRate)
			continue;

		if (_channels[i].haltTimer) {
			_channels[i].haltTimer--;
			if (!_channels[i].haltTimer) {
				// Once the timer reaches zero, immediately it stop looping
				_channels[i].loopCount = 1;
				interruptChannel(i);
			}
		}
		_channels[i].period += _channels[i].sweepRate;
	}
}

void Player_V3A::updateMusicIndy() {
	// technically, musicTimer should only be incremented during playback, but that seems to cause problems
	_musicTimer++;

	if (!_songDelay || !_songData)
		return;

	for (int i = 0; i < 4; i++) {
		if (_channels[i].haltTimer)
			_channels[i].haltTimer--;

		// When a looped sample runs out, fade the volume to zero
		// Non-looped samples will be allowed to continue playing
		if (!_channels[i].haltTimer && _channels[i].loopCount) {
			_channels[i].volume -= _channels[i].fadeRate;

			// Once the volume hits zero, immediately silence it
			if (_channels[i].volume < 1) {
				_channels[i].volume = 0;
				_channels[i].loopCount = 0;
				clearVoice(i);
				setChannelInterrupt(i, false);
			} else
				setChannelVolume(i, MIN((_channels[i].volume >> 8) & 0x3F, 0x3F));
		}
	}
	if (--_songDelay)
		return;

	int8 *songData = &_songData[0x1C + _songPtr];
	while (1) {
		int code = songData[0];
		if ((code & 0xF0) == 0x80) {
			// play a note
			int instrument = songData[0] & 0xF;
			int pitch = songData[1] & 0xFF;
			int volume = (songData[2] / 2) & 0xFF;
			int duration = songData[3] & 0xFF;

			_songPtr += 4;
			songData += 4;

			// pitch 0 == global rest
			if (pitch == 0) {
				_songDelay = duration;
				return;
			}

			// Find an available sound channel
			// Indy3 starts at channel (inst & 3) and tries them in sequence
			int channel = instrument & 0x3;
			for (int i = 0; i < 4; i++) {
				if (!_channels[channel].haltTimer)
					break;
				channel = (channel + 1) & 3;
			}

			startNote(channel, instrument, pitch, volume, duration);
		} else {
			// Reached the end
			for (int i = 0; i < 4; i++) {
				// Subtle bug in the original engine - it only checks the LAST playing channel
				// (rather than checking all of them)
				if (_channels[i].loopCount)
					_songDelay = _channels[i].haltTimer;
			}
			if (_songDelay == 0) {
				if ((code & 0xFF) == 0xFB) {
					// repeat
					_songPtr = 0;
					_songDelay = 1;
				} else {
					// stop
					stopSound(_curSong);
				}
			}
		}
		if ((_songDelay) || (_curSong == -1))
			break;
	}
}

void Player_V3A::updateMusicLoom() {
	// technically, musicTimer should only be incremented during playback, but that seems to cause problems
	_musicTimer++;

	if (!_songDelay || !_songData)
		return;

	// Update all playing notes
	for (int i = 0; i < 4; i++) {
		// Mark all notes that were started during a previous update
		_channels[i].canOverride = 1;
		if (_channels[i].haltTimer)
			_channels[i].haltTimer--;

		// When a looped sample runs out, fade the volume to zero
		// Non-looped samples will be allowed to continue playing
		if (!_channels[i].haltTimer && _channels[i].loopCount) {
			_channels[i].volume -= _channels[i].fadeRate;

			// Once the volume hits zero, immediately silence it
			if (_channels[i].volume < 1) {
				_channels[i].volume = 0;
				_channels[i].loopCount = 0;
				clearVoice(i);
				setChannelInterrupt(i, false);
			} else
				setChannelVolume(i, MIN((_channels[i].volume >> 8) & 0x3F, 0x3F));
		}
	}
	if (--_songDelay)
		return;

	int8 *songData = &_songData[0x1C + _songPtr];

	// Loom uses an elaborate queue to deal with overlapping notes and limited sound channels
	int queuePos = 0;
	int queueInstrument[4];
	int queuePitch[4];
	int queueVolume[4];
	int queueDuration[4];

	while (1) {
		int code = songData[0];
		if ((code & 0xF0) == 0x80) {
			// play a note
			int instrument = songData[0] & 0xF;
			int pitch = songData[1] & 0xFF;
			int volume = (((songData[2] < 0) ? (songData[2] + 1) : songData[2]) / 2) & 0xFF;
			int duration = songData[3] & 0xFF;

			_songPtr += 4;
			songData += 4;

			// pitch 0 == global rest
			if (pitch == 0) {
				_songDelay = duration;
				break;
			}

			// Try to find an appropriate channel to use
			// Channel must be playing the same instrument, started during a previous loop, and within 6 frames of ending
			int channel;
			for (channel = 0; channel < 4; channel++)
				if ((_channels[channel].instrument == instrument) && (_channels[channel].canOverride) && (_channels[channel].haltTimer < 6))
					break;

			if (channel != 4) {
				// Channel was found, so start playing the note
				startNote(channel, instrument, pitch, volume, duration);
			} else if (queuePos < 4) {
				// No channel found - put it in a queue to process at the end
				queueInstrument[queuePos] = instrument;
				queuePitch[queuePos] = pitch;
				queueVolume[queuePos] = volume;
				queueDuration[queuePos] = duration;
				++queuePos;
			}
		} else {
			// Reached end of song
			for (int i = 0; i < 4; i++) {
				// Subtle bug in the original engine - it only checks the LAST playing channel
				// rather than checking ALL of them
				if (_channels[i].loopCount)
					_songDelay = _channels[i].haltTimer;
			}
			if (_songDelay == 0) {
				if ((code & 0xFF) == 0xFB) {
					// repeat
					_songPtr = 0;
					_songDelay = 1;
				} else {
					// stop
					stopSound(_curSong);
				}
			}
		}
		if ((_songDelay) || (_curSong == -1))
			break;
	}

	while (queuePos--) {
		// Take all of the enqueued note requests and try to fit them somewhere
		int channel;
		for (channel = 0; channel < 4; channel++) {
			// First, find a soon-to-expire channel that wasn't explicitly assigned this loop
			if ((_channels[channel].canOverride) && (_channels[channel].haltTimer < 6))
				break;
		}
		if (channel == 4) {
			// If no channel found, pick the first channel playing this instrument
			for (channel = 0; channel < 4; channel++) {
				if (_channels[channel].instrument == queueInstrument[queuePos])
					break;
			}
		}
		if (channel != 4) {
			// If we found a channel, play the note there - otherwise, it gets lost
			startNote(channel, queueInstrument[queuePos], queuePitch[queuePos], queueVolume[queuePos], queueDuration[queuePos]);
		}
	}
}

void Player_V3A::startNote(int channel, int instrument, int pitch, int volume, int duration) {
	const InstData &instData = _wavetablePtrs[instrument];
	SndChan &curChan = _channels[channel];

	// for Loom, adjust pitch
	pitch += instData.pitchAdjust;

	// and set channel precedence parameters
	curChan.instrument = instrument;
	curChan.canOverride = 0;

	// Split pitch into octave+offset, truncating as needed
	int octave = (pitch / 12) - 2;
	pitch = pitch % 12;
	if (octave < 0)
		octave = 0;
	if (octave > 5)
		octave = 5;
	int actualOctave = instData.octave[octave];

	curChan.period = NOTE_FREQS[actualOctave][pitch] << 16;
	curChan.volume = (volume & 0xFF) << 8;
	curChan.sweepRate = 0;
	curChan.fadeRate = instData.volumeFade;
	curChan.haltTimer = duration;

	// For music, pre-decrement the loop counter and skip the initial interrupt
	if (instData.loopLen[octave]) {
		curChan.loopCount = -1;
		setChannelInterrupt(channel, true);
	} else {
		curChan.loopCount = 0;
		setChannelInterrupt(channel, false);
	}

	setChannelPeriod(channel, MAX((curChan.period >> 16) & 0xFFFF, 124));
	setChannelVolume(channel, MIN((curChan.volume >> 8) & 0x3F, 0x3F));
	setChannelData(channel, instData.mainData[octave], instData.loopData[octave], instData.mainLen[octave], instData.loopLen[octave]);
}

int Player_V3A::getMusicTimer() {
	// Actual code in Amiga version returns 5+timer/28, which syncs poorly in ScummVM
	// Presumably, this was meant to help slower machines sync better

	return _musicTimer / 30;
}

int Player_V3A::getSoundStatus(int nr) const {
	if (nr == -1)
		return 0;
	if (nr == _curSong)
		return 1;
	for (int i = 0; i < 4; i++)
		if (_channels[i].resourceId == nr)
			return 1;
	return 0;
}

} // End of namespace Scumm
