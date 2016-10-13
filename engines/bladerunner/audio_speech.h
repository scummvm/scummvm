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

#ifndef BLADERUNNER_AUDIO_SPEECH_H
#define BLADERUNNER_AUDIO_SPEECH_H

#include "audio/mixer.h"

namespace BladeRunner {

class BladeRunnerEngine;

class AudioSpeech {
private:
	BladeRunnerEngine  *_vm;
	int                 _volume;
	bool                _isMaybeActive;
	Audio::SoundHandle  _soundHandle;
	byte               *_data;

public:
	AudioSpeech(BladeRunnerEngine *vm);
	~AudioSpeech();

	bool playSpeech(const char *name, int balance = 0);
	void stopSpeech();
	bool isPlaying();
	void setVolume(int volume) { _volume = volume; }
};

} // End of namespace BladeRunner

#endif
