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

#include "twine/audio/music.h"
#include "audio/audiostream.h"
#include "audio/midiparser.h"
#include "audio/midiplayer.h"
#include "backends/audiocd/audiocd.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "twine/detection.h"
#include "twine/resources/hqr.h"
#include "twine/resources/resources.h"
#include "twine/twine.h"

namespace TwinE {

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

void TwinEMidiPlayer::play(byte *buf, int size, bool loop) {
	if (_parser == nullptr) {
		if (_engine->_cfgfile.MidiType == MIDIFILE_DOS) {
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

	_isLooping = loop;
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

static const char *musicTracksLba2[] = {
	"",
	"TADPCM1",
	"TADPCM2",
	"TADPCM3",
	"TADPCM4",
	"TADPCM5",
	"JADPCM01",
	"",	// Track6.wav
	"JADPCM02",
	"JADPCM03",
	"JADPCM04",
	"JADPCM05",
	"JADPCM06",
	"JADPCM07",
	"JADPCM08",
	"JADPCM09",
	"JADPCM10",
	"JADPCM11",
	"JADPCM12",
	"JADPCM13",
	"JADPCM14",
	"JADPCM15",
	"JADPCM16",
	"JADPCM17",
	"JADPCM18",
	"LOGADPCM"
};

bool Music::playTrackMusicCd(int32 track) {
	if (!_engine->_cfgfile.UseCD) {
		return false;
	}

	if (_engine->isLBA2()) {
		const char *basename = musicTracksLba2[track];
		Audio::SeekableAudioStream *stream = Audio::SeekableAudioStream::openStreamFile(basename);
		if (stream != nullptr) {
			const int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
			_engine->_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, &_midiHandle,
						Audio::makeLoopingAudioStream(stream, 1), volume);
			debug(3, "Play audio track %s for track id %i", basename, track);
			return true;
		}
		debug(3, "Failed to find a supported format for audio track: %s", basename);
		// TODO: are there versions with real audio cd?
		// us release starting at track 0
		// other releases at track 6
		return false;
	}

	AudioCDManager *cdrom = g_system->getAudioCDManager();
	if (_engine->isDotEmuEnhanced() || _engine->isLba1Classic()) {
		track += 1;
	}
	return cdrom->play(track, 1, 0, 0);
}

void Music::stopTrackMusicCd() {
	AudioCDManager *cdrom = g_system->getAudioCDManager();
	cdrom->stop();
}

bool Music::playTrackMusic(int32 track) {
	if (track == -1) {
		stopMusic();
		return true;
	}
	if (!_engine->_cfgfile.Sound) {
		return false;
	}

	if (track == currentMusic) {
		return true;
	}

	stopMusic();
	if (playTrackMusicCd(track)) {
		currentMusic = track;
		debug("Play cd music track %i", track);
		return true;
	}
	if (playMidiMusic(track)) {
		currentMusic = track;
		debug("Play midi music track %i", track);
		return true;
	}
	warning("Failed to play track %i", track);
	return false;
}

void Music::stopTrackMusic() {
	if (!_engine->_cfgfile.Sound) {
		return;
	}

	musicFadeOut();
	stopTrackMusicCd();
}

bool Music::playMidiMusic(int32 midiIdx, int32 loop) {
	if (!_engine->_cfgfile.Sound || _engine->_cfgfile.MidiType == MIDIFILE_NONE) {
		debug("midi disabled - skip playing %i", midiIdx);
		return false;
	}

	if (midiIdx == currentMusic) {
		debug("already playing %i", midiIdx);
		return true;
	}

	stopMusic();
	currentMusic = midiIdx;

	const char *filename;
	if (_engine->_cfgfile.MidiType == MIDIFILE_DOS) {
		filename = Resources::HQR_MIDI_MI_DOS_FILE;
	} else {
		filename = Resources::HQR_MIDI_MI_WIN_FILE;
	}

	if (midiPtr) {
		musicFadeOut();
		stopMidiMusic();
	}

	if (_engine->isDotEmuEnhanced() || _engine->isLba1Classic()) {
		const Common::String &trackName = Common::String::format("lba1-%02i", midiIdx + 1);
		Audio::SeekableAudioStream *stream = Audio::SeekableAudioStream::openStreamFile(trackName);
		if (stream != nullptr) {
			const int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
			_engine->_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, &_midiHandle,
						Audio::makeLoopingAudioStream(stream, loop), volume);
			return true;
		}
	}

	int32 midiSize = HQR::getAllocEntry(&midiPtr, filename, midiIdx);
	if (midiSize == 0) {
		debug("Could not find midi file for index %i", midiIdx);
		return false;
	}
	debug("Play midi file for index %i", midiIdx);
	_midiPlayer.play(midiPtr, midiSize, loop == 0 || loop > 1);
	return true;
}

void Music::stopMidiMusic() {
	if (_engine->isDotEmuEnhanced()) {
		_engine->_system->getMixer()->stopHandle(_midiHandle);
	}

	_midiPlayer.stop();
	free(midiPtr);
	midiPtr = nullptr;
}

bool Music::initCdrom() {
	AudioCDManager* cdrom = g_system->getAudioCDManager();
	_engine->_cfgfile.UseCD = cdrom->open();
	return _engine->_cfgfile.UseCD;
}

void Music::stopMusic() {
	stopTrackMusic();
	stopMidiMusic();
	currentMusic = -1;
}

} // namespace TwinE
