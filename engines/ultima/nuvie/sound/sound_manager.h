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

#ifndef NUVIE_SOUND_SOUND_MANAGER_H
#define NUVIE_SOUND_SOUND_MANAGER_H

//priorities:
//todo:
//-sample loading partially implemented, will do later (now is 21/01/04)
//-make songs fade in & out - add query/callback for end of song so that they can cycle
//-make samples sound from mapwindow
//-make samples fade in & out according to distance
//-try and use original .m files

#include "mididrv_m_adlib.h"
#include "mididrv_m_mt32.h"
#include "midiparser_m.h"

#include "ultima/nuvie/sound/sound.h"
#include "ultima/nuvie/sound/song.h"
#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/files/nuvie_io_file.h"
#include "ultima/nuvie/sound/sfx.h"

#include "audio/mixer.h"
#include "audio/mididrv.h"
#include "common/mutex.h"

namespace Ultima {
namespace Nuvie {

#define SFX_PLAY_ASYNC true
#define SFX_PLAY_SYNC false

class SfxManager;
class CEmuopl;

struct SoundManagerSfx {
	SfxIdType sfx_id;
	Audio::SoundHandle handle;
} ;

class SoundManager {
private:
	struct SongMT32InstrumentMapping {
		char midiDatId;
		const char *filename;
		MInstrumentAssignment instrumentMapping[16];
	};

	const static SongMT32InstrumentMapping DEFAULT_MT32_INSTRUMENT_MAPPING[12];

public:
	SoundManager(Audio::Mixer *mixer);
	~SoundManager();

	bool nuvieStartup(const Configuration *config);
	bool initAudio();
	void update_map_sfx(); //updates the active sounds
	void update(); // at the moment this just changes songs if required

	void musicPlayFrom(string group);

	void musicPause();
	void musicPlay();
	void musicPlay(const char *filename, uint16 song_num = 0);

	void musicStop(); // SB-X
	Audio::SoundHandle playTownsSound(const Common::Path &filename, uint16 sample_num);
	bool isSoundPLaying(Audio::SoundHandle handle);

	bool playSfx(uint16 sfx_id, bool async = false);

	void syncSoundSettings();

	bool is_audio_enabled() {
		return audio_enabled;
	}
	void set_audio_enabled(bool val);
	bool is_music_enabled() {
		return music_enabled;
	}
	void set_music_enabled(bool val);
	bool is_speech_enabled() {
		return speech_enabled;
	}
	void set_speech_enabled(bool val);
	bool is_sfx_enabled() {
		return sfx_enabled;
	}
	void set_sfx_enabled(bool val) {
		sfx_enabled = val;
	}
	uint8 get_sfx_volume() {
		return sfx_volume;
	}
	void set_sfx_volume(uint8 val) {
		sfx_volume = val;
	}
	uint8 get_music_volume() {
		return music_volume;
	}
	void set_music_volume(uint8 val) {
		music_volume = val;
	}
	Sound *get_m_pCurrentSong() {
		return m_pCurrentSong;
	}

	bool stop_music_on_group_change;

private:
	bool LoadCustomSongs(const Common::Path &scriptname);
	bool LoadNativeU6Songs();
	bool loadSong(Song *song, const Common::Path &filename, const char *fileId);
	bool loadSong(Song *song, const Common::Path &filename, const char *fileId, const char *title);
	bool groupAddSong(const char *group, Song *song);

	//bool LoadObjectSamples(string sound_dir);
	//bool LoadTileSamples(string sound_dir);
	bool LoadSfxManager(string sfx_style);

	Sound *SongExists(const string &name); //have we loaded this sound before?
	Sound *SampleExists(const string &name); //have we loaded this sound before?


	Sound *RequestTileSound(int id);
	Sound *RequestObjectSound(int id);
	Sound *RequestSong(const string &group); //request a song from this group

	uint16 RequestObjectSfxId(uint16 obj_n);

	typedef Common::HashMap<int, SoundCollection *> IntCollectionMap;
	typedef Common::HashMap<Common::String, SoundCollection *> StringCollectionMap;
	IntCollectionMap m_TileSampleMap;
	IntCollectionMap m_ObjectSampleMap;
	StringCollectionMap m_MusicMap;
	list<Sound *> m_Songs;
	list<Sound *> m_Samples;
	const Configuration *m_Config;

	//state info:
	string m_CurrentGroup;
	Sound *m_pCurrentSong;
	list<SoundManagerSfx> m_ActiveSounds;
	bool audio_enabled;
	bool music_enabled;
	bool speech_enabled;
	bool sfx_enabled;

	uint8 music_volume;
	uint8 sfx_volume;

	Audio::Mixer *_mixer;
	SfxManager *m_SfxManager;

	CEmuopl *opl;

	MidiDriver_Multisource *_midiDriver;
	MidiDriver_M_MT32 *_mt32MidiDriver;
	MidiParser_M *_midiParser;
	MusicType _deviceType;
	byte *_musicData;
	const SongMT32InstrumentMapping *_mt32InstrumentMapping;
	Common::Mutex _musicMutex;

	int game_type; //FIXME there's a nuvie_game_t, but almost everything uses int game_type (or gametype)

public:
	static bool g_MusicFinished;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
