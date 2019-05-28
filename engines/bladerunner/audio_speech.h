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

#include "common/str.h"
#include "common/types.h"

namespace BladeRunner {

class BladeRunnerEngine;

class AudioSpeech {
	static const int kBufferSize = 200000;
	static const int kSpeechSamples[];

	BladeRunnerEngine *_vm;

	int   _speechVolume;
	bool  _isActive;
	int   _channel;
	byte *_data;

public:
	AudioSpeech(BladeRunnerEngine *vm);
	~AudioSpeech();

	bool playSpeech(const Common::String &name, int pan = 0);
	void stopSpeech();
	bool isPlaying() const;

	bool playSpeechLine(int actorId, int sentenceId, int volume, int a4, int priority);

	void setVolume(int volume);
	int getVolume() const;
	void playSample();

private:
	void ended();
	static void mixerChannelEnded(int channel, void *data);
};

} // End of namespace BladeRunner

#endif
