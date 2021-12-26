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

#ifndef BACKENDS_MIXER_MAXMOD_H
#define BACKENDS_MIXER_MAXMOD_H

#include "backends/mixer/mixer.h"

#include <mm_types.h>

/**
 * MaxMod mixer manager. It wraps the actual implementation
 * of the Audio:Mixer used by the engine, and sets up
 * MaxMod and the callback for the audio mixer implementation.
 */
class MaxModMixerManager : public MixerManager {
public:
	MaxModMixerManager(int freq, int bufSize);
	virtual ~MaxModMixerManager();

	/**
	 * Initialize and setups the mixer
	 */
	virtual void init();

	/**
	 * Pauses the audio system
	 */
	virtual void suspendAudio();

	/**
	 * Resumes the audio system
	 */
	virtual int resumeAudio();

	/**
	 * Updates the audio system
	 */
	void updateAudio();

protected:
	mm_stream _stream;
	int _freq, _bufSize;
};

#endif
