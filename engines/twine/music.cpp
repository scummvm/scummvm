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

#include "twine/music.h"
#include "audio/midiparser.h"
#include "audio/midiplayer.h"
#include "backends/audiocd/audiocd.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "twine/hqrdepack.h"
#include "twine/resources.h"
#include "twine/twine.h"

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

TwinEMidiPlayer::TwinEMidiPlayer(TwinEEngine* engine) : _engine(engine) {
	MidiPlayer::createDriver();

	int ret = _driver->open();
	if (ret == 0) {
		if (_nativeMT32) {
			_driver->sendMT32Reset();
		} else {
			_driver->sendGMReset();
		}
		_driver->setTimerCallback(this, &timerCallback);
	}
}

void TwinEMidiPlayer::play(byte *buf, int size) {
	if (_parser == nullptr) {
		if (_engine->cfgfile.MidiType == MIDIFILE_DOS) {
			_parser = MidiParser::createParser_XMIDI();
		} else {
			_parser = MidiParser::createParser_SMF();
		}
	}

	if (!_parser->loadMusic(buf, size)) {
		warning("Failed to load midi music");
		return;
	}
	_parser->setTrack(0);
	_parser->setMidiDriver(this);
	_parser->setTimerRate(_driver->getBaseTempo());
	_parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);

	syncVolume();

	// All the tracks are supposed to loop
	_isLooping = true;
	_isPlaying = true;
}

Music::Music(TwinEEngine *engine) : _engine(engine), _midiPlayer(engine) {
}

void Music::musicVolume(int32 volume) {
	_engine->_system->getMixer()->setVolumeForSoundType(Audio::Mixer::SoundType::kMusicSoundType, volume);
	_midiPlayer.setVolume(volume);
}

void Music::musicFadeIn() {
	int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::SoundType::kMusicSoundType);
	// TODO implement fade in
	musicVolume(volume);
}

void Music::musicFadeOut() {
	int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::SoundType::kMusicSoundType);
	// TODO implement fade out
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

	if (track == currentMusic) {
		return;
	}
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
	if (!_engine->cfgfile.Sound || _engine->cfgfile.MidiType == MIDIFILE_NONE) {
		return;
	}

	if (midiIdx == currentMusic) {
		return;
	}

	stopMusic();
	currentMusic = midiIdx;

	const char *filename;
	if (_engine->cfgfile.MidiType == MIDIFILE_DOS) {
		filename = Resources::HQR_MIDI_MI_DOS_FILE;
	} else {
		filename = Resources::HQR_MIDI_MI_WIN_FILE;
	}

	if (midiPtr) {
		musicFadeOut();
		stopMidiMusic();
	}

	int32 midiSize = _engine->_hqrdepack->hqrGetallocEntry(&midiPtr, filename, midiIdx);
	_midiPlayer.play(midiPtr, midiSize);
}

void Music::stopMidiMusic() {
	if (!_engine->cfgfile.Sound) {
		return;
	}

	_midiPlayer.stop();
	free(midiPtr);
	midiPtr = nullptr;
}

bool Music::initCdrom() {
	if (!_engine->cfgfile.Sound) {
		return false;
	}
#if 0 // TODO: mgerhardy
	AudioCDManager* cdrom = g_system->getAudioCDManager();
	if (cdrom->numtracks == NUM_CD_TRACKS) {
		_engine->cdDir = "LBA";
		_engine->cfgfile.UseCD = 1;
		return true;
	}
#endif
	// not found the right CD
	_engine->cfgfile.UseCD = 0;
	return false;
}

void Music::stopMusic() {
	stopTrackMusic();
	stopMidiMusic();
}

} // namespace TwinE
