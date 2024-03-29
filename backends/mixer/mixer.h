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

#ifndef BACKENDS_MIXER_ABSTRACT_H
#define BACKENDS_MIXER_ABSTRACT_H

#include "audio/mixer_intern.h"

/**
 * Abstract class for mixer manager. Subclasses
 * implement the real functionality.
 */
class MixerManager {
public:
	MixerManager() : _mixer(0), _audioSuspended(false) {}
	virtual ~MixerManager() { delete _mixer; }

	/**
	 * Initialize and setups the mixer
	 */
	virtual void init() = 0;

	/**
	 * Get the audio mixer implementation
	 */
	Audio::Mixer *getMixer() { return (Audio::Mixer *)_mixer; }

	// Used by LinuxMoto Port

	/**
	 * Pauses the audio system
	 */
	virtual void suspendAudio() = 0;

	/**
	 * Resumes the audio system
	 */
	virtual int resumeAudio() = 0;

	/**
	 * Returns true if this is a null device and won't output any audio.
	 */
	virtual bool isNullDevice() const { return false; }

protected:
	/** The mixer implementation */
	Audio::MixerImpl *_mixer;

	/** State of the audio system */
	bool _audioSuspended;
};

#endif
