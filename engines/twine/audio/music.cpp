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

TwinEMidiPlayer::TwinEMidiPlayer(TwinEEngine *engine) : _engine(engine) {
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
	debug("play midi with volume: %i", getVolume());

	_isLooping = loop;
	_isPlaying = true;
}

Music::Music(TwinEEngine *engine) : _engine(engine), _midiPlayer(engine) {
}

int32 Music::getLengthTrackCDR(int track) const {
	// TODO:
	return -1;
}

bool Music::playMidi(int32 midiIdx) {
	const int32 loop = 1;
	if (_engine->isDotEmuEnhanced() || _engine->isLba1Classic()) {
		// the midi tracks are stored in the lba1-xx files and the adeline logo jingle
		// is in lba1-32.xx - while the midiIdx is 31
		const int32 trackOffset = 1;
		Common::Path trackName(Common::String::format("lba1-%02i", midiIdx + trackOffset));
		Audio::SeekableAudioStream *stream = Audio::SeekableAudioStream::openStreamFile(trackName);
		if (stream != nullptr) {
			const int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
			_engine->_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, &_midiHandle,
													 Audio::makeLoopingAudioStream(stream, loop), volume);
			debug("Play midi music track %i", midiIdx);
			return true;
		}
	}

	const char *filename;
	if (_engine->_cfgfile.MidiType == MIDIFILE_DOS) {
		filename = Resources::HQR_MIDI_MI_DOS_FILE;
	} else if (_engine->_cfgfile.MidiType == MIDIFILE_WIN) {
		filename = Resources::HQR_MIDI_MI_WIN_FILE;
	} else {
		debug("midi disabled - skip playing %i", midiIdx);
		return false;
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

bool Music::playTrackCDR(int32 track) {
	if (_engine->isLBA2()) {
		static const char *musicTracksLba2[] = {
			"",
			"TADPCM1",
			"TADPCM2",
			"TADPCM3",
			"TADPCM4",
			"TADPCM5",
			"JADPCM01",
			"", // Track6.wav
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
			"LOGADPCM"};

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
	return cdrom->play(track + 1, 1, 0, 0);
}

bool Music::initCdrom() {
	AudioCDManager *cdrom = g_system->getAudioCDManager();
	return cdrom->open();
}

void Music::stopMusic() {
	stopMusicCD();
	stopMusicMidi();
}

void Music::stopMusicCD() {
	AudioCDManager *cdrom = g_system->getAudioCDManager();
	cdrom->stop();
}

void Music::fadeMusicMidi(uint32 time) {
	// TODO implement fade out
	stopMusicMidi();
}

void Music::stopMusicMidi() {
	if (_engine->isDotEmuEnhanced() || _engine->isLba1Classic() || _engine->isLBA2()) {
		_engine->_system->getMixer()->stopHandle(_midiHandle);
	}

	_midiPlayer.stop();
	free(midiPtr);
	midiPtr = nullptr;
	numXmi = -1;
}

bool Music::playMusic(int32 track) {
	if (track == -1) {
		stopMusic();
		return true;
	}
	if (!_engine->_cfgfile.Sound) {
		return false;
	}

	if (_engine->isCDROM()) {
		if (_flagVoiceCD || track < 1 || track > 9) {
			if (playMidiFile(track)) {
				return true;
			}
		} else {
			if (playCdTrack(track)) {
				return true;
			}
		}
	} else {
		if (playMidiFile(track)) {
			return true;
		}
	}
	warning("Failed to play track %i", track);
	return false;
}

bool Music::playMidiFile(int32 midiIdx) {
	if (!_engine->_cfgfile.Sound) {
		debug("sound disabled - skip playing %i", midiIdx);
		return false;
	}

	if (_engine->isCDROM()) {
		stopMusicCD();
	}

	if (midiIdx != numXmi || !isMidiPlaying()) {
		stopMusicMidi();
		numXmi = midiIdx;
		if (!playMidi(midiIdx)) {
			return false;
		}
		// volumeMidi(100);
	}
	return true;
}

int32 Music::getMusicCD() {
	AudioCDManager *cdrom = g_system->getAudioCDManager();
	// if (_engine->_system->getMillis() > endMusicCD) {
	if (!cdrom->isPlaying()) {
		currentMusicCD = -1;
	}
	return currentMusicCD;
}

bool Music::playCdTrack(int32 track) {
	fadeMusicMidi(1);
	numXmi = -1;

	if (track != getMusicCD()) {
		stopMusicCD();
		// TODO: endMusicCD = _engine->toSeconds(getLengthTrackCDR(track)) / 75 + _engine->toSeconds(1);
		if (playTrackCDR(track)) {
			// TODO: endMusicCD += _engine->_system->getMillis();
			debug("Play cd music track %i", track);
			currentMusicCD = track;
		}
	}
	return true;
}

void Music::playAllMusic(int num) {
	if (num != numXmi || !isMidiPlaying()) {
		stopMusicMidi();
		numXmi = num;
		playMidi(num);
		// volumeMidi(100);
	}
	if (num != getMusicCD()) {
		stopMusicCD();
		// TODO: endMusicCD = _engine->toSeconds(getLengthTrackCDR(num)) / 75 + _engine->toSeconds(1);
		if (playTrackCDR(num)) {
			// TODO: endMusicCD += _engine->_system->getMillis();
			currentMusicCD = num;
		}
	}
}

bool Music::isMidiPlaying() const {
	if (_engine->isDotEmuEnhanced() || _engine->isLba1Classic()) {
		return _engine->_system->getMixer()->isSoundHandleActive(_midiHandle);
	}

	return _midiPlayer.isPlaying();
}

void Music::musicVolume(int32 volume) {
	_engine->_system->getMixer()->setVolumeForSoundType(Audio::Mixer::SoundType::kMusicSoundType, volume);
	_midiPlayer.setVolume(volume);
}

} // namespace TwinE
