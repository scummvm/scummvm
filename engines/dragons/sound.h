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
 */
#ifndef DRAGONS_SOUND_H
#define DRAGONS_SOUND_H

#include "common/scummsys.h"
#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "midimusicplayer.h"


namespace Dragons {

class DragonsEngine;
class BigfileArchive;
class DragonRMS;
class VabSound;
struct SpeechLocation;

typedef struct Voice {
	int16 soundID;
	Audio::SoundHandle handle;

	Voice() {
		soundID = -1;
	}
} Voice;

#define NUM_VOICES 25

class SoundManager {
public:
	SoundManager(DragonsEngine *vm, BigfileArchive* bigFileArchive, DragonRMS *dragonRms);
	~SoundManager();

	void loadMsf(uint32 sceneId);
	void playOrStopSound(uint16 soundId);

	void playMusic(int16 song);
	void playSpeech(uint32 textIndex);
	bool isSpeechPlaying();
	void resumeMusic();
	void syncSoundSettings();

public:
	uint16 _dat_8006bb60_sound_related;

private:
	DragonsEngine *_vm;
	BigfileArchive *_bigFileArchive;
	DragonRMS *_dragonRMS;

	uint8 _speechVolume;
	uint8 _sfxVolume;
	uint8 _musicVolume;

	uint8 _sfxVolumeTbl[0x780];

	VabSound* _vabMusx;
	VabSound* _vabMsf;
	VabSound* _vabGlob;

	Audio::SoundHandle _speechHandle;
	MidiMusicPlayer *_midiPlayer;

	Voice _voice[NUM_VOICES];
	int16 _currentSong;

private:
	void SomeInitSound_FUN_8003f64c();

	void initVabData();

	void playSound(uint16 soundId, uint16 i);

	void stopSound(uint16 id, uint16 i);

	uint16 getVabFromSoundId(uint16 id);

	VabSound * loadVab(const char *headerFilename, const char *bodyFilename);

	bool getSpeechLocation(uint32 talkId, struct SpeechLocation *location);
	bool isVoicePlaying(uint16 soundID);
	Audio::SoundHandle *getVoiceHandle(uint16 soundID);
	void stopVoicePlaying(uint16 soundID);
	void stopAllVoices();

private:
	class PSXAudioTrack {
	private:
		struct ADPCMStatus {
			int16 sample[2];
		} _adpcmStatus[2];

	public:
		PSXAudioTrack();

		Audio::QueuingAudioStream *createNewAudioStream(Common::File *fd, uint16 sectorStart, int8 startOffset, uint16 sectorEnd);

	private:
		void queueAudioFromSector(Audio::QueuingAudioStream *audStream, Common::SeekableReadStream *sector);
	};
};

} // End of namespace Dragons

#endif //DRAGONS_SOUND_H
