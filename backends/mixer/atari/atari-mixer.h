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

#ifndef BACKENDS_MIXER_ATARI_H
#define BACKENDS_MIXER_ATARI_H

#include "backends/mixer/mixer.h"
#include "common/events.h"

/**
 *  Atari XBIOS based audio mixer.
 */

class AtariMixerManager : public MixerManager, Common::EventObserver {
public:
	AtariMixerManager();
	virtual ~AtariMixerManager();

	virtual void init() override;
	void update();

	void suspendAudio() override;
	int resumeAudio() override;

	bool notifyEvent(const Common::Event &event) override;

private:
	enum PlaybackState {
		kPlaybackStopped,
		kPlayingFromPhysicalBuffer,
		kPlayingFromLogicalBuffer
	};

	void startPlayback(PlaybackState playbackState);
	void stopPlayback(PlaybackState playbackState);

	uint32 _outputRate;
	uint32 _samples = 0;
	uint8 *_samplesBuf = nullptr;

	byte *_atariSampleBuffer = nullptr;
	byte *_atariPhysicalSampleBuffer = nullptr;
	byte *_atariLogicalSampleBuffer = nullptr;
	size_t _atariSampleBufferSize;	// one buffer (logical/physical)

	PlaybackState _playbackState = kPlaybackStopped;
};

#endif
