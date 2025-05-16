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
#ifndef TOT_SOUNDMAN_H
#define TOT_SOUNDMAN_H

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "audio/midiplayer.h"
#include "audio/mixer.h"
#include "common/file.h"

namespace Tot {
const int kSfxId = 1;
class MusicPlayer : public Audio::MidiPlayer {

private:
	byte *_data;
	int _dataSize;

	// Start MIDI File
	void sndMidiStart(bool loop);

	// Stop MIDI File
	void sndMidiStop();

public:
	MusicPlayer();
	~MusicPlayer() override;

	void playMidi(const char *fileName, bool loop);
	void playMidi(byte *data, int size, bool loop);
	void killMidi();
};

class SoundManager {
public:
	SoundManager(Audio::Mixer *mixer);
	~SoundManager();
	void loadVoc(Common::String fileName, long posinicio, int16 tamvocleer);
	void autoPlayVoc();
	void playVoc();
	void stopVoc();
	bool isVocPlaying();
	void playMidi(const char *fileName, bool loop);
	void playMidi(byte *data, int size, bool loop);
	void toggleMusic();
	void beep(int32 frequency, int32 ms);
	void waitForSoundEnd();
	void setSfxVolume(int volume);
	void setMusicVolume(int volume);
	void setSfxBalance(bool left, bool right);

private:
	Audio::SoundHandle _soundHandle;
	Audio::SoundHandle _speakerHandle;
	Audio::Mixer *_mixer;
	MusicPlayer *_musicPlayer;
	Common::SeekableReadStream *_lastSrcStream = nullptr;
	Audio::SeekableAudioStream *_audioStream = nullptr;
};

} // End of namespace Tot
#endif
