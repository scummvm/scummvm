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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_AUDQUEUE_H
#define SAGA2_AUDQUEUE_H

#include "audio/mixer.h"
#include "saga2/saga2.h"
#include "saga2/fta.h"
#include "saga2/shorten.h"
#include "saga2/hresmgr.h"

namespace Saga2 {

extern hResContext *voiceRes;
extern hResContext *soundRes;
extern hResContext *musicRes;

class SoundQueue {
private:
	Common::Queue<soundSegment> _speechQueue;
	Common::Queue<soundSegment> _sfxQueue;
	Common::Queue<soundSegment> _bgmQueue;
	Audio::SoundHandle _speechSoundHandle;
	Audio::SoundHandle _sfxSoundHandle;
	Audio::SoundHandle _bgmSoundHandle;

	soundSegment _currentSpeech;

public:
	SoundQueue() {
		_currentSpeech = 0;
	}

	void pushVoice(soundSegment s) {
		_speechQueue.push(s);
	}

	void pushVoice(soundSegment s[]) {
		soundSegment *p = s;
		while (*p) {
			_speechQueue.push(*p);
			p++;
		}
	}

	void pushSound(soundSegment s) {
		_sfxQueue.push(s);
	}

	void pushMusic(soundSegment s) {
		_bgmQueue.push(s);
	}

	void playNext() {
		if (_speechQueue.size()) {
			soundSegment s = _speechQueue.pop();
			_currentSpeech = s;
			playSpeech(s);
		}

		if (_sfxQueue.size()) {
			soundSegment s = _sfxQueue.pop();
			playSound(s);
		}
	}

	void playSpeech(soundSegment s) {
		Common::SeekableReadStream *stream = loadResourceToStream(voiceRes, s, "voice data");

		Audio::AudioStream *aud = makeShortenStream(*stream);

		g_system->getMixer()->playStream(Audio::Mixer::kSpeechSoundType, &_speechSoundHandle, aud);

		delete stream;
	}

	void playSound(soundSegment s) {
		warning("STUB: SoundQueue::playSound");

#if 0
		Common::SeekableReadStream *stream = loadResourceToStream(soundRes, s, "voice data");

		Audio::AudioStream *aud = makeShortenStream(*stream);

		g_system->getMixer()->playStream(Audio::Mixer::kSpeechSoundType, &_sfxSoundHandle, aud);

		delete stream;
#endif
	}

	void playMusic(soundSegment s) {
		warning("STUB: SoundQueue::playMusic");

#if 0
		Common::SeekableReadStream *stream = loadResourceToStream(musicRes, s, "voice data");

		Audio::AudioStream *aud = makeShortenStream(*stream);

		g_system->getMixer()->playStream(Audio::Mixer::kSpeechSoundType, &_bgmSoundHandle, aud);

		delete stream;
#endif
	}

	bool isSpeechPlaying() {
		return g_system->getMixer()->isSoundHandleActive(_speechSoundHandle);
	}

	bool isSpeechPlaying(soundSegment s) {
		debugC(2, kDebugSound, "STUB: Sound: isSpeechPlaying(%d) vs %d", s, _currentSpeech);
		return isSpeechPlaying() && _currentSpeech <= s;
	}

	bool isSoundPlaying() {
		return g_system->getMixer()->isSoundHandleActive(_sfxSoundHandle);
	}

	bool isMusicPlaying() {
		return g_system->getMixer()->isSoundHandleActive(_bgmSoundHandle);
	}

	bool isPlaying() {
		return isSpeechPlaying() || isSoundPlaying() || isMusicPlaying();
	}

	int getSize() {
		return _speechQueue.size() + _sfxQueue.size() + _bgmQueue.size();
	}
};

} // end of namespace Saga2

#endif
