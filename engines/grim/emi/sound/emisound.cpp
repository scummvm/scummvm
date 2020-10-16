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

#include "gui/error.h"

#include "common/stream.h"
#include "common/mutex.h"
#include "common/timer.h"
#include "common/translation.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/mixer.h"
#include "engines/grim/debug.h"
#include "engines/grim/sound.h"
#include "engines/grim/grim.h"
#include "engines/grim/resource.h"
#include "engines/grim/savegame.h"
#include "engines/grim/textsplit.h"
#include "engines/grim/emi/sound/emisound.h"
#include "engines/grim/emi/sound/track.h"
#include "engines/grim/emi/sound/aifftrack.h"
#include "engines/grim/emi/sound/mp3track.h"
#include "engines/grim/emi/sound/scxtrack.h"
#include "engines/grim/emi/sound/vimatrack.h"
#include "engines/grim/movie/codecs/vima.h"

namespace Grim {

EMISound *g_emiSound = nullptr;

extern uint16 imuseDestTable[];

MusicEntry emiPS2MusicTable[] = {
	{ 0, 0, 0, 127, 0, "", "", "" },
	{ 0, 0, 1, 127, 1, "state", "", "1115.scx" },
	{ 0, 0, 2, 127, 2, "state", "", "1170.scx" },
	{ 0, 0, 2, 127, 3, "state", "", "1170.scx" },
	{ 0, 0, 2, 127, 4, "state", "", "1170.scx" },
	{ 0, 0, 3, 127, 5, "state", "", "1165.scx" },
	{ 0, 0, 4, 127, 6, "state", "", "1145.scx" },
	{ 0, 0, 4, 127, 7, "state", "", "1145.scx" },
	{ 0, 0, 1, 127, 8, "state", "", "1115.scx" },
	{ 0, 0, 1, 127, 9, "state", "", "1115.scx" },
	{ 0, 0, 0, 127, 10, "episode", "", "7200.scx" },
	{ 0, 0, 0, 127, 11, "episode", "", "1210.scx" },
	{ 0, 0, 0, 127, 12, "state", "", "1180.scx" },
	{ 0, 0, 0, 127, 13, "state", "", "1110.scx" },
	{ 0, 0, 1, 127, 14, "state", "", "1115.scx" },
	{ 0, 0, 0, 127, 15, "state", "", "1105.scx" },
	{ 0, 0, 4, 127, 16, "state", "", "1145.scx" },
	{ 0, 0, 0, 127, 17, "state", "", "1150.scx" },
	{ 0, 0, 0, 127, 18, "state", "", "1100.scx" },
	{ 0, 0, 5, 127, 19, "state", "", "1120.scx" },
	{ 0, 0, 5, 127, 20, "state", "", "1120.scx" },
	{ 0, 0, 5, 127, 21, "state", "", "1120.scx" },
	{ 0, 0, 3, 127, 22, "state", "", "1165.scx" },
	{ 0, 0, 0, 127, 23, "state", "", "1155.scx" },
	{ 0, 0, 0, 127, 24, "state", "", "1160.scx" },
	{ 0, 0, 0, 127, 25, "state", "", "1140.scx" },
	{ 0, 0, 0, 127, 26, "state", "", "1140.scx" },
	{ 0, 0, 2, 127, 27, "state", "", "1170.scx" },
	{ 0, 0, 2, 127, 28, "state", "", "1175.scx" },
	{ 0, 0, 0, 127, 29, "episode", "", "1205.scx" },
	{ 0, 0, 0, 127, 30, "state", "", "1000.scx" },
	{ 0, 0, 0, 127, 31, "state", "", "1185.scx" },
	{ 0, 0, 0, 127, 32, "state", "", "2127.scx" },
	{ 0, 0, 0, 127, 33, "state", "", "2119.scx" },
	{ 0, 0, 0, 127, 34, "episode", "", "2208.scx" },
	{ 0, 0, 0, 127, 35, "state", "", "2195.scx" },
	{ 0, 0, 0, 127, 36, "state", "", "2190.scx" },
	{ 0, 0, 0, 127, 37, "state", "", "2185.scx" },
	{ 0, 0, 1, 127, 38, "state", "", "2175.scx" },
	{ 0, 0, 0, 127, 39, "state", "", "2170.scx" },
	{ 0, 0, 0, 127, 40, "state", "", "2165.scx" },
	{ 0, 0, 0, 127, 41, "state", "", "2160.scx" },
	{ 0, 0, 0, 127, 42, "state", "", "2155.scx" },
	{ 0, 0, 0, 127, 43, "state", "", "2120.scx" },
	{ 0, 0, 0, 127, 44, "state", "", "2150.scx" },
	{ 0, 0, 0, 127, 45, "state", "", "2145.scx" },
	{ 0, 0, 2, 127, 46, "state", "", "2105.scx" },
	{ 0, 0, 0, 127, 47, "state", "", "2115.scx" },
	{ 0, 0, 0, 127, 48, "state", "", "2125.scx" },
	{ 0, 0, 0, 127, 49, "state", "", "2130.scx" },
	{ 0, 0, 0, 127, 50, "state", "", "2100.scx" },
	{ 0, 0, 0, 127, 51, "state", "", "2140.scx" },
	{ 0, 0, 0, 127, 52, "episode", "", "2200.scx" },
	{ 0, 0, 0, 127, 53, "state", "", "2116.scx" },
	{ 0, 0, 0, 127, 54, "episode", "", "2207.scx" },
	{ 0, 0, 0, 127, 55, "state", "", "2107.scx" },
	{ 0, 0, 0, 127, 56, "episode", "", "2215.scx" },
	{ 0, 0, 0, 127, 57, "episode", "", "2220.scx" },
	{ 0, 0, 0, 127, 58, "episode", "", "2225.scx" },
	{ 0, 0, 0, 127, 59, "episode", "", "2210.scx" },
	{ 0, 0, 0, 127, 60, "state", "", "2135.scx" },
	{ 0, 0, 2, 127, 61, "state", "", "2105.scx" },
	{ 0, 0, 0, 127, 62, "state", "", "2108.scx" },
	{ 0, 0, 0, 127, 63, "state", "", "2117.scx" },
	{ 0, 0, 0, 127, 64, "state", "", "2118.scx" },
	{ 0, 0, 1, 127, 65, "state", "", "2175.scx" },
	{ 0, 0, 0, 127, 66, "state", "", "4120.scx" },
	{ 0, 0, 1, 127, 67, "state", "", "3100.scx" },
	{ 0, 0, 0, 127, 68, "state", "", "4115.scx" },
	{ 0, 0, 2, 127, 69, "state", "", "4100.scx" },
	{ 0, 0, 0, 127, 70, "state", "", "3150.scx" },
	{ 0, 0, 0, 127, 71, "state", "", "3145.scx" },
	{ 0, 0, 0, 127, 72, "state", "", "4110.scx" },
	{ 0, 0, 0, 127, 73, "state", "", "3140.scx" },
	{ 0, 0, 3, 127, 74, "state", "", "3135.scx" },
	{ 0, 0, 3, 127, 75, "state", "", "3120.scx" },
	{ 0, 0, 4, 127, 76, "state", "", "3130.scx" },
	{ 0, 0, 4, 127, 77, "state", "", "3115.scx" },
	{ 0, 0, 1, 127, 78, "state", "", "3100.scx" },
	{ 0, 0, 5, 127, 79, "state", "", "3125.scx" },
	{ 0, 0, 5, 127, 80, "state", "", "3110.scx" },
	{ 0, 0, 6, 127, 81, "state", "", "3105.scx" },
	{ 0, 0, 0, 127, 82, "episode", "", "3210.scx" },
	{ 0, 0, 0, 127, 83, "episode", "", "3200.scx" },
	{ 0, 0, 0, 127, 84, "episode", "", "3205.scx" },
	{ 0, 0, 0, 127, 85, "state", "", "3147.scx" },
	{ 0, 0, 0, 127, 86, "episode", "", "4215.scx" },
	{ 0, 0, 0, 127, 87, "state", "", "4105.scx" },
	{ 0, 0, 6, 127, 88, "state", "", "3106.scx" },
	{ 0, 0, 6, 127, 89, "state", "", "3107.scx" },
	{ 0, 0, 2, 127, 90, "state", "", "4100.scx" },
	{ 0, 0, 1, 127, 91, "state", "", "5145.scx" },
	{ 0, 0, 2, 127, 92, "state", "", "5140.scx" },
	{ 0, 0, 2, 127, 93, "state", "", "5140.scx" },
	{ 0, 0, 3, 127, 94, "state", "", "5135.scx" },
	{ 0, 0, 3, 127, 95, "state", "", "5135.scx" },
	{ 0, 0, 3, 127, 96, "state", "", "5135.scx" },
	{ 0, 0, 0, 127, 97, "state", "", "5170.scx" },
	{ 0, 0, 0, 127, 98, "episode", "", "5205.scx" },
	{ 0, 0, 0, 127, 99, "state", "", "5120.scx" },
	{ 0, 0, 0, 127, 100, "episode", "", "5215.scx" },
	{ 0, 0, 0, 127, 101, "episode", "", "5230.scx" },
	{ 0, 0, 0, 127, 102, "episode", "", "5225.scx" },
	{ 0, 0, 0, 127, 103, "state", "", "5117.scx" },
	{ 0, 0, 0, 127, 104, "state", "", "5115.scx" },
	{ 0, 0, 0, 127, 105, "episode", "", "5220.scx" },
	{ 0, 0, 0, 127, 106, "state", "", "6105.scx" },
	{ 0, 0, 0, 127, 107, "state", "", "6100.scx" },
	{ 0, 0, 0, 127, 108, "state", "", "5165.scx" },
	{ 0, 0, 0, 127, 109, "state", "", "5160.scx" },
	{ 0, 0, 0, 127, 110, "episode", "", "5200.scx" },
	{ 0, 0, 2, 127, 111, "state", "", "5140.scx" },
	{ 0, 0, 3, 127, 112, "state", "", "5135.scx" },
	{ 0, 0, 0, 127, 113, "state", "", "5155.scx" },
	{ 0, 0, 0, 127, 114, "state", "", "5150.scx" },
	{ 0, 0, 0, 127, 115, "state", "", "5130.scx" },
	{ 0, 0, 0, 127, 116, "state", "", "5125.scx" },
	{ 0, 0, 0, 127, 117, "state", "", "5110.scx" },
	{ 0, 0, 1, 127, 118, "state", "", "5105.scx" },
	{ 0, 0, 0, 127, 119, "state", "", "5100.scx" },
	{ 0, 0, 0, 127, 120, "state", "", "6110.scx" },
	{ 0, 0, 0, 127, 121, "state", "", "5106.scx" },
	{ 0, 0, 0, 127, 122, "episode", "", "7210.scx" },
	{ 0, 0, 0, 127, 123, "episode", "", "1200.scx" },
	{ 0, 0, 0, 127, 124, "state", "", "1195.scx" },
	{ 0, 0, 0, 127, 125, "episode", "", "1215.scx" }
};

void EMISound::timerHandler(void *refCon) {
	EMISound *emiSound = (EMISound *)refCon;
	emiSound->callback();
}

EMISound::EMISound(int fps) {
	_curMusicState = -1;
	_numMusicStates = 0;
	_musicTrack = nullptr;
	_curTrackId = 0;
	_callbackFps = fps;
	vimaInit(imuseDestTable);
	initMusicTable();
	g_system->getTimerManager()->installTimerProc(timerHandler, 1000000 / _callbackFps, this, "emiSoundCallback");
}

EMISound::~EMISound() {
	g_system->getTimerManager()->removeTimerProc(timerHandler);
	freePlayingSounds();
	freeLoadedSounds();
	delete _musicTrack;
	if (g_grim->getGamePlatform() != Common::kPlatformPS2) {
		delete[] _musicTable;
	}
}

EMISound::TrackList::iterator EMISound::getPlayingTrackByName(const Common::String &name) {
	for (TrackList::iterator it = _playingTracks.begin(); it != _playingTracks.end(); ++it) {
		if ((*it)->getSoundName() == name) {
			return it;
		}
	}
	return _playingTracks.end();
}

void EMISound::freePlayingSounds() {
	for (TrackList::iterator it = _playingTracks.begin(); it != _playingTracks.end(); ++it) {
		delete (*it);
	}
	_playingTracks.clear();
}

void EMISound::freeLoadedSounds() {
	for (TrackMap::iterator it = _preloadedTrackMap.begin(); it != _preloadedTrackMap.end(); ++it) {
		delete it->_value;
	}
	_preloadedTrackMap.clear();
}

bool EMISound::startVoice(const Common::String &soundName, int volume, int pan) {
	return startSound(soundName, Audio::Mixer::kSpeechSoundType, volume, pan);
}

bool EMISound::startSfx(const Common::String &soundName, int volume, int pan) {
	return startSound(soundName, Audio::Mixer::kSFXSoundType, volume, pan);
}

bool EMISound::startSfxFrom(const Common::String &soundName, const Math::Vector3d &pos, int volume) {
	return startSoundFrom(soundName, Audio::Mixer::kSFXSoundType, pos, volume);
}

bool EMISound::startSound(const Common::String &soundName, Audio::Mixer::SoundType soundType, int volume, int pan) {
	Common::StackLock lock(_mutex);
	SoundTrack *track = initTrack(soundName, soundType);
	if (track) {
		track->setBalance(pan * 2 - 127);
		track->setVolume(volume);
		track->play();
		_playingTracks.push_back(track);
		return true;
	}
	return false;
}

bool EMISound::startSoundFrom(const Common::String &soundName, Audio::Mixer::SoundType soundType, const Math::Vector3d &pos, int volume) {
	Common::StackLock lock(_mutex);
	SoundTrack *track = initTrack(soundName, soundType);
	if (track) {
		track->setVolume(volume);
		track->setPosition(true, pos);
		track->play();
		_playingTracks.push_back(track);
		return true;
	}
	return false;
}

bool EMISound::getSoundStatus(const Common::String &soundName) {
	TrackList::iterator it = getPlayingTrackByName(soundName);

	if (it == _playingTracks.end())  // We have no such sound.
		return false;

	return (*it)->isPlaying();
}

void EMISound::stopSound(const Common::String &soundName) {
	Common::StackLock lock(_mutex);
	TrackList::iterator it = getPlayingTrackByName(soundName);
	if (it == _playingTracks.end()) {
		warning("Sound track '%s' could not be found to stop", soundName.c_str());
	} else {
		delete (*it);
		_playingTracks.erase(it);
	}
}

int32 EMISound::getPosIn16msTicks(const Common::String &soundName) {
	TrackList::iterator it = getPlayingTrackByName(soundName);
	if (it == _playingTracks.end()) {
		warning("Sound track '%s' could not be found to get ticks", soundName.c_str());
		return 0;
	} else {
		return (*it)->getPos().msecs() / 16;
	}
}

void EMISound::setVolume(const Common::String &soundName, int volume) {
	Common::StackLock lock(_mutex);
	TrackList::iterator it = getPlayingTrackByName(soundName);
	if (it == _playingTracks.end()) {
		warning("Sound track '%s' could not be found to set volume", soundName.c_str());
	} else {
		(*it)->setVolume(volume);
	}
}

void EMISound::setPan(const Common::String &soundName, int pan) {
	Common::StackLock lock(_mutex);
	TrackList::iterator it = getPlayingTrackByName(soundName);
	if (it == _playingTracks.end()) {
		warning("Sound track '%s' could not be found to set pan", soundName.c_str());
	} else {
		(*it)->setBalance(pan * 2 - 127);
	}
}

bool EMISound::loadSfx(const Common::String &soundName, int &id) {
	Common::StackLock lock(_mutex);
	SoundTrack *track = initTrack(soundName, Audio::Mixer::kSFXSoundType);
	if (track) {
		id = _curTrackId++;
		_preloadedTrackMap[id] = track;
		return true;
	} else {
		return false;
	}
}

void EMISound::playLoadedSound(int id, bool looping) {
	Common::StackLock lock(_mutex);
	TrackMap::iterator it = _preloadedTrackMap.find(id);
	if (it != _preloadedTrackMap.end()) {
		it->_value->setLooping(looping);
		it->_value->setPosition(false);
		it->_value->play();
	} else {
		warning("EMISound::playLoadedSound called with invalid sound id");
	}
}

void EMISound::playLoadedSoundFrom(int id, const Math::Vector3d &pos, bool looping) {
	Common::StackLock lock(_mutex);
	TrackMap::iterator it = _preloadedTrackMap.find(id);
	if (it != _preloadedTrackMap.end()) {
		it->_value->setLooping(looping);
		it->_value->setPosition(true, pos);
		it->_value->play();
	}
	else {
		warning("EMISound::playLoadedSoundFrom called with invalid sound id");
	}
}

void EMISound::setLoadedSoundLooping(int id, bool looping) {
	Common::StackLock lock(_mutex);
	TrackMap::iterator it = _preloadedTrackMap.find(id);
	if (it != _preloadedTrackMap.end()) {
		it->_value->setLooping(looping);
	} else {
		warning("EMISound::setLoadedSoundLooping called with invalid sound id");
	}
}

void EMISound::stopLoadedSound(int id) {
	Common::StackLock lock(_mutex);
	TrackMap::iterator it = _preloadedTrackMap.find(id);
	if (it != _preloadedTrackMap.end()) {
		it->_value->stop();
	} else {
		warning("EMISound::stopLoadedSound called with invalid sound id");
	}
}

void EMISound::freeLoadedSound(int id) {
	Common::StackLock lock(_mutex);
	TrackMap::iterator it = _preloadedTrackMap.find(id);
	if (it != _preloadedTrackMap.end()) {
		delete it->_value;
		_preloadedTrackMap.erase(it);
	} else {
		warning("EMISound::freeLoadedSound called with invalid sound id");
	}
}

void EMISound::setLoadedSoundVolume(int id, int volume) {
	Common::StackLock lock(_mutex);
	TrackMap::iterator it = _preloadedTrackMap.find(id);
	if (it != _preloadedTrackMap.end()) {
		it->_value->setVolume(volume);
	} else {
		warning("EMISound::setLoadedSoundVolume called with invalid sound id");
	}
}

void EMISound::setLoadedSoundPan(int id, int pan) {
	Common::StackLock lock(_mutex);
	TrackMap::iterator it = _preloadedTrackMap.find(id);
	if (it != _preloadedTrackMap.end()) {
		it->_value->setBalance(pan * 2 - 127);
	} else {
		warning("EMISound::setLoadedSoundPan called with invalid sound id");
	}
}

void EMISound::setLoadedSoundPosition(int id, const Math::Vector3d &pos) {
	Common::StackLock lock(_mutex);
	TrackMap::iterator it = _preloadedTrackMap.find(id);
	if (it != _preloadedTrackMap.end()) {
		it->_value->setPosition(true, pos);
	} else {
		warning("EMISound::setLoadedSoundPosition called with invalid sound id");
	}
}

bool EMISound::getLoadedSoundStatus(int id) {
	Common::StackLock lock(_mutex);
	TrackMap::iterator it = _preloadedTrackMap.find(id);
	if (it != _preloadedTrackMap.end()) {
		return it->_value->isPlaying();
	}
	warning("EMISound::getLoadedSoundStatus called with invalid sound id");
	return false;
}

int EMISound::getLoadedSoundVolume(int id) {
	Common::StackLock lock(_mutex);
	TrackMap::iterator it = _preloadedTrackMap.find(id);
	if (it != _preloadedTrackMap.end()) {
		return it->_value->getVolume();
	}
	warning("EMISound::getLoadedSoundVolume called with invalid sound id");
	return false;
}

SoundTrack *EMISound::initTrack(const Common::String &soundName, Audio::Mixer::SoundType soundType, const Audio::Timestamp *start) const {
	SoundTrack *track;
	Common::String soundNameLower(soundName);
	soundNameLower.toLowercase();
	if (soundNameLower.hasSuffix(".scx")) {
		track = new SCXTrack(soundType);
	} else if (soundNameLower.hasSuffix(".m4b") || soundNameLower.hasSuffix(".lab")) {
		track = new MP3Track(soundType);
	} else if (soundNameLower.hasSuffix(".aif")) {
		track = new AIFFTrack(soundType);
	} else {
		track = new VimaTrack();
	}

	Common::String filename;
	if (soundType == Audio::Mixer::kMusicSoundType) {
		filename = _musicPrefix + soundName;
	} else {
		filename = soundName;
	}

	if (track->openSound(filename, soundName, start)) {
		return track;
	}
	return nullptr;
}

bool EMISound::stateHasLooped(int stateId) {
	if (stateId == _curMusicState) {
		if (_curMusicState != 0 && _musicTrack) {
			return _musicTrack->hasLooped();
		}
	} else {
		warning("EMISound::stateHasLooped called for a different music state than the current one");
	}
	return false;
}

bool EMISound::stateHasEnded(int stateId) {
	if (stateId == _curMusicState) {
		if (_curMusicState != 0 && _musicTrack) {
			return !_musicTrack->isPlaying();
		}
	}
	return true;
}

void EMISound::setMusicState(int stateId) {
	Common::StackLock lock(_mutex);
	// The demo calls ImSetState with state id 1000, which exceeds the number of states in the
	// music table.
	if (stateId >= _numMusicStates)
		stateId = 0;
	if (stateId == _curMusicState)
		return;

	if (_musicTable == nullptr) {
		Debug::debug(Debug::Sound, "No music table loaded");
		return;
	}

	Common::String soundName = _musicTable[stateId]._filename;
	int sync = _musicTable[stateId]._sync;
	Audio::Timestamp musicPos;
	int prevSync = -1;
	if (_musicTrack) {
		if (_musicTrack->isPlaying()) {
			musicPos = _musicTrack->getPos();
			prevSync = _musicTrack->getSync();
			if (sync == prevSync && soundName == _musicTrack->getSoundName()) {
				// If the previous music track is the same track as the new one, we'll just
				// keep playing the previous track. This happens in the PS2 version where they
				// removed some of the music variations, but kept the states associated with
				// those.
				_curMusicState = stateId;
				return;
			}
			_musicTrack->fadeOut();
			_playingTracks.push_back(_musicTrack);
			_musicTrack = nullptr;
		}
	}

	bool fadeMusicIn = false;
	for (TrackList::iterator it = _playingTracks.begin(); it != _playingTracks.end(); ++it) {
		if ((*it)->isPlaying() && (*it)->getSoundType() == Audio::Mixer::kMusicSoundType) {
			fadeMusicIn = true;
			break;
		}
	}
	if (!fadeMusicIn) {
		for (uint i = 0; i < _stateStack.size(); ++i) {
			if (_stateStack[i]._track && _stateStack[i]._track->isPlaying() && !_stateStack[i]._track->isPaused()) {
				fadeMusicIn = true;
				break;
			}
		}
	}

	if (stateId == 0) {
		_curMusicState = 0;
		return;
	}
	if (_musicTable[stateId]._id != stateId) {
		Debug::debug(Debug::Sound, "Attempted to play track #%d, not found in music table!", stateId);
		return;
	}
	_curMusicState = stateId;

	Audio::Timestamp *start = nullptr;
	if (prevSync != 0 && sync != 0 && prevSync == sync)
		start = &musicPos;

	Debug::debug(Debug::Sound, "Loading music: %s", soundName.c_str());
	SoundTrack *music = initTrack(soundName, Audio::Mixer::kMusicSoundType, start);
	if (music) {
		music->play();
		music->setSync(sync);
		if (fadeMusicIn) {
			music->setFade(0.0f);
			music->fadeIn();
		}
		_musicTrack = music;
	}
}

uint32 EMISound::getMsPos(int stateId) {
	if (!_musicTrack) {
		Debug::debug(Debug::Sound, "EMISound::getMsPos: Music track is null", stateId);
		return 0;
	}
	return _musicTrack->getPos().msecs();
}

MusicEntry *EMISound::initMusicTableDemo(const Common::String &filename) {
	Common::SeekableReadStream *data = g_resourceloader->openNewStreamFile(filename);

	if (!data)
		error("Couldn't open %s", filename.c_str());
	// FIXME, for now we use a fixed-size table, as I haven't looked at the retail-data yet.
	_numMusicStates = 15;
	MusicEntry *musicTable = new MusicEntry[_numMusicStates];
	for (int i = 0; i < 15; ++i) {
		musicTable->_x = 0;
		musicTable->_y = 0;
		musicTable->_sync = 0;
		musicTable->_trim = 0;
		musicTable->_id = i;
	}

	TextSplitter *ts = new TextSplitter(filename, data);
	int id, x, y, sync;
	char musicfilename[64];
	char name[64];
	while (!ts->isEof()) {
		while (!ts->checkString("*/")) {
			while (!ts->checkString(".cuebutton"))
				ts->nextLine();

			ts->scanString(".cuebutton id %d x %d y %d sync %d \"%[^\"]64s", 5, &id, &x, &y, &sync, name);
			ts->scanString(".playfile \"%[^\"]64s", 1, musicfilename);
			musicTable[id]._id = id;
			musicTable[id]._x = x;
			musicTable[id]._y = y;
			musicTable[id]._sync = sync;
			musicTable[id]._name = name;
			musicTable[id]._filename = musicfilename;
		}
		ts->nextLine();
	}
	delete ts;
	delete data;
	return musicTable;
}

void EMISound::initMusicTableRetail(MusicEntry *musicTable, const Common::String filename) {
	Common::SeekableReadStream *data = g_resourceloader->openNewStreamFile(filename);

	// Remember to check, in case we forgot to copy over those files from the CDs.
	if (!data) {
		warning("Couldn't open %s", filename.c_str());
		return;
	}

	TextSplitter *ts = new TextSplitter(filename, data);
	int id, x, y, sync, trim;
	char musicfilename[64];
	char type[16];
	// Every block is followed by 3 lines of commenting/uncommenting, except the last.
	while (!ts->isEof()) {
		while (!ts->checkString("*/")) {
			while (!ts->checkString(".cuebutton"))
				ts->nextLine();

			ts->scanString(".cuebutton id %d x %d y %d sync %d type %16s", 5, &id, &x, &y, &sync, type);
			ts->scanString(".playfile trim %d \"%[^\"]64s", 2, &trim, musicfilename);
			if (musicfilename[1] == '\\')
				musicfilename[1] = '/';
			musicTable[id]._id = id;
			musicTable[id]._x = x;
			musicTable[id]._y = y;
			musicTable[id]._sync = sync;
			musicTable[id]._type = type;
			musicTable[id]._name = "";
			musicTable[id]._trim = trim;
			musicTable[id]._filename = musicfilename;
		}
		ts->nextLine();
	}
	delete ts;
	delete data;
}

void tableLoadErrorDialog() {
	Common::U32String errorMessage = _("ERROR: Not enough music tracks found!\n"
	"Escape from Monkey Island has two versions of FullMonkeyMap.imt,\n"
	"you need to copy both files from both CDs to Textures/, and rename\n"
	"them as follows to get music-support in-game: \n"
	"CD 1: \"FullMonkeyMap.imt\" -> \"FullMonkeyMap1.imt\"\n"
	"CD 2: \"FullMonkeyMap.imt\" -> \"FullMonkeyMap2.imt\"\n"
	"\n"
	"Alternatively, a Steam or GOG copy has a combined FullMonkeyMap.imt");
	GUI::displayErrorDialog(errorMessage);
}

void EMISound::initMusicTable() {
	if (g_grim->getGameFlags() & ADGF_DEMO) {
		_musicTable = initMusicTableDemo("Music/FullMonkeyMap.imt");
		_musicPrefix = "Music/";
	} else if (g_grim->getGamePlatform() == Common::kPlatformPS2) {
		_musicTable = emiPS2MusicTable;
		_numMusicStates = ARRAYSIZE(emiPS2MusicTable);
		_musicPrefix = "";
	} else {
		MusicEntry *musicTable = new MusicEntry[126];
		for (int i = 0; i < 126; ++i) {
			musicTable[i]._x = 0;
			musicTable[i]._y = 0;
			musicTable[i]._sync = 0;
			musicTable[i]._trim = 0;
			musicTable[i]._id = i;
		}

		initMusicTableRetail(musicTable, "Textures/FullMonkeyMap1.imt");
		initMusicTableRetail(musicTable, "Textures/FullMonkeyMap2.imt");
		initMusicTableRetail(musicTable, "Textures/FullMonkeyMap.imt");

		/* There seem to be 69+60 music tracks, for a total of 125 unique tracks. */
		int numTracks = 0;
		for (int i = 0; i < 126; i++) {
			if (!musicTable[i]._filename.empty()) {
				numTracks++;
			}
		}

		warning("Found %d music tracks, expected at least 100", numTracks);
		if (numTracks < 100) {
			delete[] musicTable;
			_numMusicStates = 0;
			_musicTable = nullptr;
			tableLoadErrorDialog();
		} else {
			_numMusicStates = 126;
			_musicTable = musicTable;
			_musicPrefix = "Textures/spago/"; // Default to high-quality music.
		}
	}
}

void EMISound::selectMusicSet(int setId) {
	if (g_grim->getGamePlatform() == Common::kPlatformPS2) {
		assert(setId == 0);
		_musicPrefix = "";
		return;
	}
	if (setId == 0) {
		_musicPrefix = "Textures/spago/";
	} else if (setId == 1) {
		_musicPrefix = "Textures/mego/";
	} else {
		error("EMISound::selectMusicSet - Unknown setId %d", setId);
	}

	// Immediately switch all currently active music tracks to the new quality.
	for (TrackList::iterator it = _playingTracks.begin(); it != _playingTracks.end(); ++it) {
		SoundTrack *track = (*it);
		if (track && track->getSoundType() == Audio::Mixer::kMusicSoundType) {
			(*it) = restartTrack(track);
			delete track;
		}
	}
	for (uint32 i = 0; i < _stateStack.size(); ++i) {
		SoundTrack *track = _stateStack[i]._track;
		if (track) {
			_stateStack[i]._track = restartTrack(track);
			delete track;
		}
	}
}

SoundTrack *EMISound::restartTrack(SoundTrack *track) {
	Audio::Timestamp pos = track->getPos();
	SoundTrack *newTrack = initTrack(track->getSoundName(), track->getSoundType(), &pos);
	if (newTrack) {
		newTrack->setVolume(track->getVolume());
		newTrack->setBalance(track->getBalance());
		newTrack->setFadeMode(track->getFadeMode());
		newTrack->setFade(track->getFade());
		if (track->isPlaying()) {
			newTrack->play();
		}
		if (track->isPaused()) {
			newTrack->pause();
		}
	}
	return newTrack;
}

void EMISound::pushStateToStack() {
	Common::StackLock lock(_mutex);
	if (_musicTrack) {
		_musicTrack->fadeOut();
		StackEntry entry = { _curMusicState, _musicTrack };
		_stateStack.push(entry);
		_musicTrack = nullptr;
	} else {
		StackEntry entry = { _curMusicState, nullptr };
		_stateStack.push(entry);
	}
	_curMusicState = 0;
}

void EMISound::popStateFromStack() {
	Common::StackLock lock(_mutex);
	if (_musicTrack) {
		_musicTrack->fadeOut();
		_playingTracks.push_back(_musicTrack);
	}

	//even pop state from stack if music isn't set
	StackEntry entry = _stateStack.pop();
	SoundTrack *track = entry._track;

	_musicTrack = track;
	_curMusicState = entry._state;

	if (track) {
		if (track->isPaused()) {
			track->pause();
		}
		track->fadeIn();
	}
}

void EMISound::flushStack() {
	Common::StackLock lock(_mutex);
	while (!_stateStack.empty()) {
		SoundTrack *temp = _stateStack.pop()._track;
		delete temp;
	}
}

void EMISound::pause(bool paused) {
	Common::StackLock lock(_mutex);

	for (TrackList::iterator it = _playingTracks.begin(); it != _playingTracks.end(); ++it) {
		SoundTrack *track = (*it);
		if (paused == track->isPaused())
			continue;

		// Do not pause music.
		if (track == _musicTrack)
			continue;

		track->pause();
	}

	for (TrackMap::iterator it = _preloadedTrackMap.begin(); it != _preloadedTrackMap.end(); ++it) {
		SoundTrack *track = (*it)._value;
		if (!track->isPlaying() || paused == track->isPaused())
			continue;

		track->pause();
	}
}

void EMISound::callback() {
	Common::StackLock lock(_mutex);

	if (_musicTrack) {
		updateTrack(_musicTrack);
	}

	for (uint i = 0; i < _stateStack.size(); ++i) {
		SoundTrack *track = _stateStack[i]._track;
		if (track == nullptr || track->isPaused() || !track->isPlaying())
			continue;

		updateTrack(track);
		if (track->getFadeMode() == SoundTrack::FadeOut && track->getFade() == 0.0f) {
			track->pause();
		}
	}

	for (TrackList::iterator it = _playingTracks.begin(); it != _playingTracks.end(); ++it) {
		SoundTrack *track = (*it);
		if (track->isPaused() || !track->isPlaying())
			continue;

		updateTrack(track);
		if (track->getFadeMode() == SoundTrack::FadeOut && track->getFade() == 0.0f) {
			track->stop();
		}
	}
}

void EMISound::updateTrack(SoundTrack *track) {
	if (track->getFadeMode() != SoundTrack::FadeNone) {
		float fadeStep = 0.5f / _callbackFps;
		float fade = track->getFade();
		if (track->getFadeMode() == SoundTrack::FadeIn) {
			fade += fadeStep;
			if (fade > 1.0f)
				fade = 1.0f;
			track->setFade(fade);
		}
		else {
			fade -= fadeStep;
			if (fade < 0.0f)
				fade = 0.0f;
			track->setFade(fade);
		}
	}
}

void EMISound::flushTracks() {
	Common::StackLock lock(_mutex);
	for (TrackList::iterator it = _playingTracks.begin(); it != _playingTracks.end(); ++it) {
		SoundTrack *track = (*it);
		if (!track->isPlaying()) {
			delete track;
			it = _playingTracks.erase(it);
		}
	}
}

void EMISound::restoreState(SaveGame *savedState) {
	Common::StackLock lock(_mutex);
	// Clear any current music
	flushStack();
	setMusicState(0);
	freePlayingSounds();
	freeLoadedSounds();
	delete _musicTrack;
	_musicTrack = nullptr;
	// Actually load:
	savedState->beginSection('SOUN');
	_musicPrefix = savedState->readString();
	if (savedState->saveMinorVersion() >= 21) {
		_curMusicState = savedState->readLESint32();
	}

	// Stack:
	uint32 stackSize = savedState->readLEUint32();
	for (uint32 i = 0; i < stackSize; i++) {
		SoundTrack *track = nullptr;
		int state = 0;
		if (savedState->saveMinorVersion() >= 21) {
			state = savedState->readLESint32();
			bool hasTrack = savedState->readBool();
			if (hasTrack) {
				track = restoreTrack(savedState);
			}
		} else {
			Common::String soundName = savedState->readString();
			track = initTrack(soundName, Audio::Mixer::kMusicSoundType);
			if (track) {
				track->play();
				track->pause();
			}
		}
		StackEntry entry = { state, track };
		_stateStack.push(entry);
	}

	// Music:
	if (savedState->saveMinorVersion() < 21) {
		uint32 hasActiveTrack = savedState->readLEUint32();
		if (hasActiveTrack) {
			Common::String soundName = savedState->readString();
			_musicTrack = initTrack(soundName, Audio::Mixer::kMusicSoundType);
			if (_musicTrack) {
				_musicTrack->play();
			} else {
				error("Couldn't reopen %s", soundName.c_str());
			}
		}
	} else if (savedState->saveMinorVersion() >= 21) {
		bool musicActive = savedState->readBool();
		if (musicActive) {
			_musicTrack = restoreTrack(savedState);
		}
	}

	// Effects and voices:
	uint32 numTracks = savedState->readLEUint32();
	for (uint32 i = 0; i < numTracks; i++) {
		bool channelIsActive = true;
		if (savedState->saveMinorVersion() < 21) {
			channelIsActive = (savedState->readLESint32() != 0);
		}
		if (channelIsActive) {
			SoundTrack *track = restoreTrack(savedState);
			_playingTracks.push_back(track);
		}
	}

	// Preloaded sounds:
	if (savedState->saveMinorVersion() >= 21) {
		_curTrackId = savedState->readLESint32();
		uint32 numLoaded = savedState->readLEUint32();
		for (uint32 i = 0; i < numLoaded; ++i) {
			int id = savedState->readLESint32();
			_preloadedTrackMap[id] = restoreTrack(savedState);
		}
	}

	savedState->endSection();
}

void EMISound::saveState(SaveGame *savedState) {
	Common::StackLock lock(_mutex);
	savedState->beginSection('SOUN');
	savedState->writeString(_musicPrefix);
	savedState->writeLESint32(_curMusicState);

	// Stack:
	uint32 stackSize = _stateStack.size();
	savedState->writeLEUint32(stackSize);
	for (uint32 i = 0; i < stackSize; i++) {
		savedState->writeLESint32(_stateStack[i]._state);
		if (!_stateStack[i]._track) {
			savedState->writeBool(false);
		} else {
			savedState->writeBool(true);
			saveTrack(_stateStack[i]._track, savedState);
		}
	}

	// Music:
	savedState->writeBool(_musicTrack != nullptr);
	if (_musicTrack) {
		saveTrack(_musicTrack, savedState);
	}

	// Effects and voices:
	savedState->writeLEUint32(_playingTracks.size());
	for (TrackList::iterator it = _playingTracks.begin(); it != _playingTracks.end(); ++it) {
		saveTrack((*it), savedState);
	}

	// Preloaded sounds:
	savedState->writeLESint32(_curTrackId);
	uint32 numLoaded = _preloadedTrackMap.size();
	savedState->writeLEUint32(numLoaded);
	for (TrackMap::iterator it = _preloadedTrackMap.begin(); it != _preloadedTrackMap.end(); ++it) {
		savedState->writeLESint32(it->_key);
		saveTrack(it->_value, savedState);
	}

	savedState->endSection();
}

void EMISound::saveTrack(SoundTrack *track, SaveGame *savedState) {
	savedState->writeString(track->getSoundName());
	savedState->writeLEUint32(track->getVolume());
	savedState->writeLEUint32(track->getBalance());
	savedState->writeLEUint32(track->getPos().msecs());
	savedState->writeBool(track->isPlaying());
	savedState->writeBool(track->isPaused());
	savedState->writeLESint32((int)track->getSoundType());
	savedState->writeLESint32((int)track->getFadeMode());
	savedState->writeFloat(track->getFade());
	savedState->writeLESint32(track->getSync());
	savedState->writeBool(track->isLooping());
	savedState->writeBool(track->isPositioned());
	savedState->writeVector3d(track->getWorldPos());
}

SoundTrack *EMISound::restoreTrack(SaveGame *savedState) {
	Common::String soundName = savedState->readString();
	int volume = savedState->readLESint32();
	int balance = savedState->readLESint32();
	Audio::Timestamp pos(savedState->readLESint32());
	bool playing = savedState->readBool();
	if (savedState->saveMinorVersion() < 21) {
		SoundTrack *track = initTrack(soundName, Audio::Mixer::kSpeechSoundType);
		if (track)
			track->play();
		return track;
	}
	bool paused = savedState->readBool();
	Audio::Mixer::SoundType soundType = (Audio::Mixer::SoundType)savedState->readLESint32();
	SoundTrack::FadeMode fadeMode = (SoundTrack::FadeMode)savedState->readLESint32();
	float fade = savedState->readFloat();
	int sync = savedState->readLESint32();
	bool looping = savedState->saveMinorVersion() >= 21 ? savedState->readBool() : false;
	bool positioned = false;
	Math::Vector3d worldPos;
	if (savedState->saveMinorVersion() >= 23) {
		positioned = savedState->readBool();
		worldPos = savedState->readVector3d();
	}

	SoundTrack *track = initTrack(soundName, soundType, &pos);
	track->setVolume(volume);
	track->setBalance(balance);
	track->setPosition(positioned, worldPos);
	track->setLooping(looping);
	track->setFadeMode(fadeMode);
	track->setFade(fade);
	track->setSync(sync);
	if (playing)
		track->play();
	if (paused)
		track->pause();
	return track;
}

void EMISound::updateSoundPositions() {
	Common::StackLock lock(_mutex);

	for (TrackList::iterator it = _playingTracks.begin(); it != _playingTracks.end(); ++it) {
		SoundTrack *track = (*it);
		track->updatePosition();
	}

	for (TrackMap::iterator it = _preloadedTrackMap.begin(); it != _preloadedTrackMap.end(); ++it) {
		SoundTrack *track = (*it)._value;
		track->updatePosition();
	}
}

} // end of namespace Grim
