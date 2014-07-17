/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef GRIM_SOUNDTRACK_H
#define GRIM_SOUNDTRACK_H

#include "audio/mixer.h"
#include "audio/timestamp.h"

namespace Common {
	class String;
}

namespace Audio {
	class AudioStream;
	class SoundHandle;
}

namespace Grim {

/**
 * @class Super-class for the different codecs used in EMI
 */
class SoundTrack {
public:
	enum FadeMode {
		FadeNone,
		FadeIn,
		FadeOut
	};
protected:
	Common::String _soundName;
	Audio::AudioStream *_stream;
	Audio::SoundHandle *_handle;
	Audio::Mixer::SoundType _soundType;
	DisposeAfterUse::Flag _disposeAfterPlaying;
	bool _paused;
	FadeMode _fadeMode;
	float _fade;
	int _balance;
	int _volume;
	int _sync;
public:
	SoundTrack();
	virtual ~SoundTrack();
	virtual bool openSound(const Common::String &filename, const Common::String &voiceName, const Audio::Timestamp *start = nullptr) = 0;
	virtual bool isPlaying() = 0;
	virtual bool play();
	virtual void pause();
	virtual void stop();
	void fadeIn() { _fadeMode = FadeIn; }
	void fadeOut() { _fadeMode = FadeOut; }
	void setFade(float fade) { _fade = fade; }
	float getFade() const { return _fade; }
	FadeMode getFadeMode() const { return _fadeMode; }
	void setBalance(int balance);
	void setVolume(int volume);
	void setSync(int sync) { _sync = sync; }
	virtual int getVolume() { return _volume; };
	int getSync() const { return _sync; }
	virtual Audio::Timestamp getPos() = 0;
	Audio::SoundHandle *getHandle() { return _handle; }
	Common::String getSoundName();
	void setSoundName(const Common::String &name);
	virtual bool hasLooped() { return false; }
	bool isPaused() const { return _paused; }
};

}

#endif
