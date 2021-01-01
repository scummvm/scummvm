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

#ifndef ULTIMA8_AUDIO_AUDIOCHANNEL_H
#define ULTIMA8_AUDIO_AUDIOCHANNEL_H

#include "audio/mixer.h"

namespace Ultima {
namespace Ultima8 {

class AudioSample;

class AudioChannel {
private:
	// We have:
	// 1x decompressor size
	// 2x frame size
	Common::Array<byte> _playData;

	Audio::SoundHandle _soundHandle;
	Audio::Mixer *_mixer;
	uint32          _decompressorSize;  // Persistent data for the decompressor
	uint32          _frameSize;         //

	int32           _loop;
	AudioSample     *_sample;

	// Info for sampling
	uint32          _frameEvenOdd;  // which buffer is 'frame0'
	int             _lVol, _rVol;   // 0-256
	uint32          _pitchShift;    // AudioProcess::PITCH_SHIFT_NONE = no shift
	int             _priority;      // anything.
	bool            _paused;        // true/false
private:
	/**
	 * Decompresses the next frame of sample data
	 */
	void decompressNextFrame();
public:
	AudioChannel(Audio::Mixer *mixer, uint32 sampleRate, bool stereo);
	~AudioChannel(void);

	void stop();

	void playSample(AudioSample *sample, int loop, int priority, bool paused,
		uint32 pitchShift, int lvol, int rvol);

	void playMusicStream(Audio::AudioStream *stream);

	bool isPlaying();

	void setPitchShift(int pitchShift) {
		_pitchShift = pitchShift;
	}
	uint32 getPitchShift() const {
		return _pitchShift;
	}

	void setLoop(int loop) {
		_loop = loop;
	}
	int32 getLoop() const {
		return _loop;
	}

	void setVolume(int lvol, int rvol) {
		_lVol = lvol;
		_rVol = rvol;
		_mixer->setChannelVolume(_soundHandle, (rvol + lvol) / 2);
		_mixer->setChannelBalance(_soundHandle, (rvol - lvol) / 2);
	}
	void getVolume(int &lvol, int &rvol) const {
		lvol = _lVol;
		rvol = _rVol;
	}

	void setPriority(int priority) {
		_priority = priority;
	}
	int getPriority() const {
		return _priority;
	}

	void setPaused(bool paused);

	bool isPaused() const {
		return _paused;
	}
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
