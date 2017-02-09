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

#ifndef TITANIC_WAVE_FILE_H
#define TITANIC_WAVE_FILE_H

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "titanic/sound/audio_buffer.h"
#include "titanic/support/string.h"
#include "titanic/true_talk/dialogue_file.h"

namespace Titanic {

enum LoadMode { LOADMODE_AUDIO_BUFFER = 1, LOADMODE_SCUMMVM = 2 };

class QSoundManager;

class CWaveFile {
private:
	/**
	 * Handles setup of fields shared by the constructors
	 */
	void setup();
public:
	QSoundManager *_soundManager;
	byte *_rawData;
	Audio::SeekableAudioStream *_audioStream;
	Audio::SoundHandle _soundHandle;
	Audio::Mixer::SoundType _soundType;

	LoadMode _loadMode;
	int _field4;
	int _field14;
	uint _dataSize;
	CAudioBuffer *_audioBuffer;
	DisposeAfterUse::Flag _disposeAudioBuffer;
	int _channel;
public:
	CWaveFile();
	CWaveFile(QSoundManager *owner);
	~CWaveFile();

	/**
	 * Returns the duration of the wave file
	 * @returns	Total ticks. Not really sure how ticks
	 * map to real time
	 */
	uint getDurationTicks() const;

	/**
	 * Return the size of the wave file
	 */
	uint size() const { return _dataSize; }

	/**
	 * Tries to load the specified wave file sound
	 */
	bool loadSound(const CString &name);

	/**
	 * Tries to load speech from a specified dialogue file
	 */
	bool loadSpeech(CDialogueFile *dialogueFile, int speechIndex);

	/**
	 * Tries to load the specified music wave file
	 */
	bool loadMusic(const CString &name);

	/**
	 * Tries to load the specified audio buffer
	 */
	bool loadMusic(CAudioBuffer *buffer, DisposeAfterUse::Flag disposeAfterUse);

	/**
	 * Returns true if the wave file has data loaded
	 */
	bool isLoaded() const { return _audioStream != nullptr; }

	/**
	 * Return the frequency of the loaded wave file
	 */
	uint getFrequency() const;

	/**
	 * Resets the music stream
	 */
	void reset();

	/**
	 * Lock sound data for access
	 */
	const byte *lock();

	/**
	 * Unlock sound data after a prior call to lock
	 */
	void unlock(const byte *ptr);
};

} // End of namespace Titanic

#endif /* TITANIC_WAVE_FILE_H */
