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

#include "audio/midiparser.h"
#include "backends/audiocd/audiocd.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "twine/hqrdepack.h"
#include "twine/music.h"
#include "twine/resources.h"
#include "twine/twine.h"
#include "twine/xmidi.h"

namespace TwinE {

/** MP3 music folder */
#define MUSIC_FOLDER "music"
/**
 * LBA1 default number of tracks
 * <pre>
 *  TRACK 01 MODE1/2352
 *    INDEX 01 00:00:00
 *  TRACK 02 AUDIO
 *    INDEX 01 10:47:52
 *  TRACK 03 AUDIO
 *    INDEX 01 14:02:01
 *  TRACK 04 AUDIO
 *    INDEX 01 17:02:19
 *  TRACK 05 AUDIO
 *    INDEX 01 19:34:45
 *  TRACK 06 AUDIO
 *    INDEX 01 22:22:34
 *  TRACK 07 AUDIO
 *    INDEX 01 25:09:32
 *  TRACK 08 AUDIO
 *    INDEX 01 26:47:72
 *  TRACK 09 AUDIO
 *    INDEX 01 30:29:07
 *  TRACK 10 AUDIO
 *    INDEX 01 32:04:62
 * </pre>
 */
#define NUM_CD_TRACKS 10
/** Number of miliseconds to fade music */
#define FADE_MS 500

void Music::musicVolume(int32 volume) {
	_engine->_system->getMixer()->setVolumeForSoundType(Audio::Mixer::SoundType::kMusicSoundType, volume);
}

void Music::musicFadeIn() {
	int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::SoundType::kMusicSoundType);
#if 0 // TODO
	Mix_FadeInMusic(current_track, 1, FADE_MS);
#endif
	musicVolume(volume);
}

void Music::musicFadeOut() {
	int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::SoundType::kMusicSoundType);
#if 0 // TODO
	while (!Mix_FadeOutMusic(FADE_MS) && Mix_PlayingMusic()) {
		SDL_Delay(100);
	}
	Mix_HaltMusic();
	Mix_RewindMusic();
#endif
	musicVolume(volume);
}

void Music::playTrackMusicCd(int32 track) {
	if (!_engine->cfgfile.UseCD) {
		return;
	}
	AudioCDManager *cdrom = g_system->getAudioCDManager();
	cdrom->play(track, 1, 0, 0);
}

void Music::stopTrackMusicCd() {
	if (!_engine->cfgfile.UseCD) {
		return;
	}

	AudioCDManager *cdrom = g_system->getAudioCDManager();
	cdrom->stop();
}

void Music::playTrackMusic(int32 track) {
	if (!_engine->cfgfile.Sound) {
		return;
	}

	if (track == currentMusic)
		return;
	currentMusic = track;

	stopMusic();
	playTrackMusicCd(track);
}

void Music::stopTrackMusic() {
	if (!_engine->cfgfile.Sound) {
		return;
	}

	musicFadeOut();
	stopTrackMusicCd();
}

void Music::playMidiMusic(int32 midiIdx, int32 loop) {

	if (!_engine->cfgfile.Sound) {
		return;
	}

	if (midiIdx == currentMusic) {
		return;
	}

	stopMusic();
	currentMusic = midiIdx;

	char filename[256];
	if (_engine->cfgfile.MidiType == MIDIFILE_DOS)
		snprintf(filename, sizeof(filename), "%s", Resources::HQR_MIDI_MI_DOS_FILE);
	else
		snprintf(filename, sizeof(filename), "%s", Resources::HQR_MIDI_MI_WIN_FILE);

	if (midiPtr) {
		musicFadeOut();
		stopMidiMusic();
	}

	int32 midiSize = _engine->_hqrdepack->hqrGetallocEntry(&midiPtr, filename, midiIdx);

	if (_engine->cfgfile.Sound == 1 && _engine->cfgfile.MidiType == 0) {
		uint8 *dos_midi_ptr;
		midiSize = convert_to_midi(midiPtr, midiSize, &dos_midi_ptr);
		free(midiPtr);
		midiPtr = dos_midi_ptr;
	}

#if 0
	SDL_RWops *rw = SDL_RWFromMem(midiPtr, midiSize);
	current_track = Mix_LoadMUS_RW(rw, 0);

	musicFadeIn();

	if (Mix_PlayMusic(current_track, loop) == -1)
		warning("Error while playing music: %d \n", midiIdx);
#endif
}

void Music::stopMidiMusic() {
	if (!_engine->cfgfile.Sound) {
		return;
	}

#if 0 // TODO
	if (current_track != NULL) {
		Mix_FreeMusic(current_track);
		current_track = NULL;
		if (midiPtr != NULL)
			free(midiPtr);
	}
#endif
}

int Music::initCdrom() {
	if (!_engine->cfgfile.Sound) {
		return 0;
	}
#if 0 // TODO: mgerhardy
	AudioCDManager* cdrom = g_system->getAudioCDManager();
	if (cdrom->numtracks == NUM_CD_TRACKS) {
		_engine->cdDir = "LBA";
		_engine->cfgfile.UseCD = 1;
		return 1;
	}
#endif
	// not found the right CD
	_engine->cfgfile.UseCD = 0;
	return 0;
}

void Music::stopMusic() {
	stopTrackMusic();
	stopMidiMusic();
}

} // namespace TwinE
