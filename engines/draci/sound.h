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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/engines/draci/barchive.h $
 * $Id: barchive.h 44802 2009-10-08 21:28:57Z fingolfin $
 *
 */

#ifndef DRACI_SOUND_H
#define DRACI_SOUND_H

#include "common/str.h"
#include "common/file.h"
#include "sound/mixer.h"

namespace Draci {

/**
 *  Represents individual files inside the archive.
 */
struct SoundSample {
	uint _offset;
	uint _length;
	uint _frequency;
	byte* _data;

	void close(void) {
		delete[] _data;
		_data = NULL;
	}
};

class SoundArchive {
public:
	SoundArchive(const Common::String &path, uint defaultFreq) :
	_path(), _samples(NULL), _sampleCount(0), _defaultFreq(defaultFreq), _opened(false), _f(NULL) {
		openArchive(path);
	}

	~SoundArchive() { closeArchive(); }

	void closeArchive();
	void openArchive(const Common::String &path);
	uint size() const { return _sampleCount; }

	/**
	 * Checks whether there is an archive opened. Should be called before reading
	 * from the archive to check whether openArchive() succeeded.
	 */
	bool isOpen() const { return _opened; }

	void clearCache();

	SoundSample *getSample(int i, uint freq);

private:
	Common::String _path;    ///< Path to file
	SoundSample *_samples;          ///< Internal array of files
	uint _sampleCount;         ///< Number of files in archive
	uint _defaultFreq;	///< The default sampling frequency of the archived samples
	bool _opened;            ///< True if the archive is opened, false otherwise
	Common::File *_f;	///< Opened file
};

#define SOUND_HANDLES 10

enum sndHandleType {
	kFreeHandle,
	kEffectHandle,
	kVoiceHandle
};

struct SndHandle {
	Audio::SoundHandle handle;
	sndHandleType type;
};

// Taken from engines/saga/sound.h and simplified (in particular, removed
// decompression until we support compressed files too).
class Sound {
public:

	Sound(Audio::Mixer *mixer);
	~Sound() {}

	void playSound(const SoundSample *buffer, int volume, bool loop);
	void pauseSound();
	void resumeSound();
	void stopSound();
	bool isMutedSound() const { return _muteSound; }

	void playVoice(const SoundSample *buffer);
	void pauseVoice();
	void resumeVoice();
	void stopVoice();
	bool isMutedVoice() const { return _muteVoice; }

	void stopAll();

	void setVolume();

	bool showSubtitles() const { return _showSubtitles; }
	int talkSpeed() const { return _talkSpeed; }

 private:

	void playSoundBuffer(Audio::SoundHandle *handle, const SoundSample &buffer, int volume,
				sndHandleType handleType, bool loop);

	SndHandle *getHandle();

	Audio::Mixer *_mixer;

	bool _muteSound;
	bool _muteVoice;
	bool _showSubtitles;
	int _talkSpeed;

	SndHandle _handles[SOUND_HANDLES];
};

} // End of namespace Draci

#endif // DRACI_SOUND_H
