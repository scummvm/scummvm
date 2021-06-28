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
#include "saga2/shorten.h"
#include "saga2/hresmgr.h"

namespace Saga2 {

Common::SeekableReadStream *loadResourceToStream(hResContext *con, uint32 id, const char desc[]);

extern hResContext *voiceRes;

class SoundQueue {
private:
	Common::Queue<soundSegment> _voiceQueue;
	Common::Queue<soundSegment> _sfxQueue;
	Common::Queue<soundSegment> _bgmQueue;
	Audio::SoundHandle _speechSoundHandle;

public:
	void pushVoice(soundSegment s) {
		_voiceQueue.push(s);
	}

	void pushVoice(soundSegment s[]) {
		soundSegment *p = s;
		while (*p) {
			_voiceQueue.push(*p);
			p++;
		}
	}

	void playNext() {
		if (_voiceQueue.size()) {
			soundSegment s = _voiceQueue.pop();
			Common::SeekableReadStream *stream = loadResourceToStream(voiceRes, s, "voice data");

			Audio::AudioStream *aud = makeShortenStream(*stream);

			g_system->getMixer()->playStream(Audio::Mixer::kSpeechSoundType, &_speechSoundHandle, aud);

			delete stream;
		}
	}

	bool isPlaying() {
		return g_system->getMixer()->isSoundHandleActive(_speechSoundHandle);
	}

	int getSize() {
		return _voiceQueue.size();
	}
};

} // end of namespace Saga2

#endif
