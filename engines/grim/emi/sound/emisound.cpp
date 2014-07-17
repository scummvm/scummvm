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
#include "engines/grim/emi/sound/mp3track.h"
#include "engines/grim/emi/sound/scxtrack.h"
#include "engines/grim/emi/sound/vimatrack.h"

#define NUM_CHANNELS 32

namespace Grim {

class SoundTrack;

void EMISound::timerHandler(void *refCon) {
	EMISound *emiSound = (EMISound *)refCon;
	emiSound->callback();
}

EMISound::EMISound(int fps) {
	_channels = new SoundTrack*[NUM_CHANNELS];
	for (int i = 0; i < NUM_CHANNELS; i++) {
		_channels[i] = nullptr;
	}
	_curMusicState = -1;
	_musicChannel = -1;
	_callbackFps = fps;
	initMusicTable();
	g_system->getTimerManager()->installTimerProc(timerHandler, 1000000 / _callbackFps, this, "emiSoundCallback");
}

EMISound::~EMISound() {
	g_system->getTimerManager()->removeTimerProc(timerHandler);
	freeAllChannels();
	delete[] _channels;
	delete[] _musicTable;
}

int32 EMISound::getFreeChannel() {
	for (int i = 0; i < NUM_CHANNELS; i++) {
		if (_channels[i] == nullptr)
			return i;
	}
	return -1;
}

int32 EMISound::getChannelByName(const Common::String &name) {
	for (int i = 0; i < NUM_CHANNELS; i++) {
		if (_channels[i] && _channels[i]->getSoundName() == name)
			return i;
	}
	return -1;
}

void EMISound::freeChannel(int32 channel) {
	delete _channels[channel];
	_channels[channel] = nullptr;
}

void EMISound::freeAllChannels() {
	for (int i = 0; i < NUM_CHANNELS; i++) {
		freeChannel(i);
	}
}

bool EMISound::startVoice(const char *soundName, int volume, int pan) {
	int channel = getFreeChannel();
	assert(channel != -1);

	// TODO: This could be handled on filenames instead
	if (g_grim->getGamePlatform() == Common::kPlatformPS2)
		_channels[channel] = new SCXTrack(Audio::Mixer::kSpeechSoundType);
	else
		_channels[channel] = new VimaTrack(soundName);

	Common::SeekableReadStream *str = g_resourceloader->openNewStreamFile(soundName);

	if (str && _channels[channel]->openSound(soundName, str)) {
		_channels[channel]->play();
		return true;
	}
	return false;
}

bool EMISound::getSoundStatus(const char *soundName) {
	int32 channel = getChannelByName(soundName);

	if (channel == -1)  // We have no such sound.
		return false;

	return g_system->getMixer()->isSoundHandleActive(*_channels[channel]->getHandle()) && _channels[channel]->isPlaying();
}

void EMISound::stopSound(const char *soundName) {
	int32 channel = getChannelByName(soundName);
	assert(channel != -1);
	g_system->getMixer()->stopHandle(*_channels[channel]->getHandle());
	freeChannel(channel);
}

int32 EMISound::getPosIn16msTicks(const char *soundName) {
	int32 channel = getChannelByName(soundName);
	assert(channel != -1);
	return g_system->getMixer()->getSoundElapsedTime(*_channels[channel]->getHandle()) / 16;
}

void EMISound::setVolume(const char *soundName, int volume) {
	int32 channel = getChannelByName(soundName);
	assert(channel != -1);
	g_system->getMixer()->setChannelVolume(*_channels[channel]->getHandle(), volume);
}

void EMISound::setPan(const char *soundName, int pan) {
	int32 channel = getChannelByName(soundName);
	assert(channel != -1);
	g_system->getMixer()->setChannelBalance(*_channels[channel]->getHandle(), pan * 2 - 127);
}

SoundTrack *EMISound::createEmptyMusicTrack() const {
	SoundTrack *music;
	if (g_grim->getGamePlatform() == Common::kPlatformPS2) {
		music = new SCXTrack(Audio::Mixer::kMusicSoundType);
	} else {
		music = new MP3Track(Audio::Mixer::kMusicSoundType);
	}
	return music;
}

bool EMISound::initTrack(const Common::String &filename, SoundTrack *track, const Audio::Timestamp *start) {
	Common::SeekableReadStream *str = g_resourceloader->openNewStreamFile(_musicPrefix + filename);
	if (track->openSound(filename, str, start)) {
		return true;
	} else {
		return false;
	}
}

bool EMISound::stateHasLooped(int stateId) {
	if (stateId == _curMusicState) {
		if (_musicChannel != -1 && _channels[_musicChannel] != nullptr) {
			return _channels[_musicChannel]->hasLooped();
		}
	} else {
		warning("EMISound::stateHasLooped called for a different music state than the current one");
	}
	return false;
}

void EMISound::setMusicState(int stateId) {
	if (stateId == _curMusicState)
		return;

	Audio::Timestamp musicPos;
	int prevSync = -1;
	if (_musicChannel != -1 && _channels[_musicChannel]) {
		SoundTrack *music = _channels[_musicChannel];
		if (music->isPlaying()) {
			musicPos = music->getPos();
			prevSync = music->getSync();
		}
		music->fadeOut();
		_musicChannel = -1;
	}
	if (stateId == 0)
		return;
	if (_musicTable == nullptr) {
		Debug::debug(Debug::Sound, "No music table loaded");
		return;
	}
	if (_musicTable[stateId]._id != stateId) {
		Debug::debug(Debug::Sound, "Attempted to play track #%d, not found in music table!", stateId);
		return;
	}
	Common::String filename;
	int sync = 0;
	if (g_grim->getGamePlatform() == Common::kPlatformPS2) {
		Debug::debug(Debug::Sound, "PS2 doesn't have musictable yet %d ignored, just playing 1195.SCX", stateId);
		// So, we just rig up the menu-song hardcoded for now, as a test of the SCX-code.
		filename = "1195.SCX";
	} else {
		filename = _musicTable[stateId]._filename;
		sync = _musicTable[stateId]._sync;
	}
	_curMusicState = stateId;

	_musicChannel = getFreeChannel();
	assert(_musicChannel != -1);
	SoundTrack *music = createEmptyMusicTrack();
	_channels[_musicChannel] = music;

	Audio::Timestamp *start = nullptr;
	if (prevSync == sync)
		start = &musicPos;

	Debug::debug(Debug::Sound, "Loading music: %s", filename.c_str());
	if (initTrack(filename, music, start)) {
		music->play();
		music->setSync(sync);
		music->setFade(0.0f);
		music->fadeIn();
	}
}

uint32 EMISound::getMsPos(int stateId) {
	if (_musicChannel == -1)
		return 0;
	SoundTrack *music = _channels[_musicChannel];
	if (!music || !music->getHandle())
		return 0;
	return g_system->getMixer()->getSoundElapsedTime(*music->getHandle());
}

MusicEntry *initMusicTableDemo(const Common::String &filename) {
	Common::SeekableReadStream *data = g_resourceloader->openNewStreamFile(filename);

	if (!data)
		error("Couldn't open %s", filename.c_str());
	// FIXME, for now we use a fixed-size table, as I haven't looked at the retail-data yet.
	MusicEntry *musicTable = new MusicEntry[15];
	for (unsigned int i = 0; i < 15; i++)
		musicTable[i]._id = -1;

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

MusicEntry *initMusicTableRetail(MusicEntry *table, const Common::String &filename) {
	Common::SeekableReadStream *data = g_resourceloader->openNewStreamFile(filename);

	// Remember to check, in case we forgot to copy over those files from the CDs.
	if (!data) {
		warning("Couldn't open %s", filename.c_str());
		delete[] table;
		return nullptr;
	}
	
	MusicEntry *musicTable = table;
	if (!table) {
		musicTable = new MusicEntry[126];
		for (unsigned int i = 0; i < 126; i++) {
			musicTable[i]._id = -1;
		}
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
	return musicTable;
}

void tableLoadErrorDialog(const char *filename) {
	const char *errorMessage = nullptr;
	errorMessage =  "ERROR: Missing file for music-support.\n"
	"Escape from Monkey Island has two versions of FullMonkeyMap.imt,\n"
	"you need to copy both files from both CDs to Textures/, and rename\n"
	"them as follows to get music-support in-game: \n"
	"CD 1: \"FullMonkeyMap.imt\" -> \"FullMonkeyMap1.imt\"\n"
	"CD 2: \"FullMonkeyMap.imt\" -> \"FullMonkeyMap2.imt\"";
	GUI::displayErrorDialog(errorMessage);
	error("Missing file %s", filename);
}

void EMISound::initMusicTable() {
	if (g_grim->getGameFlags() == ADGF_DEMO) {
		_musicTable = initMusicTableDemo("Music/FullMonkeyMap.imt");
		_musicPrefix = "Music/";
	} else if (g_grim->getGamePlatform() == Common::kPlatformPS2) {
		// TODO, fill this in, data is in the binary.
		//initMusicTablePS2()
		_musicTable = nullptr;
		_musicPrefix = "";
	} else {
		_musicTable = nullptr;
		_musicTable = initMusicTableRetail(_musicTable, "Textures/FullMonkeyMap1.imt");
		if (_musicTable == nullptr) {
			tableLoadErrorDialog("Textures/FullMonkeyMap1.imt");
		}
		_musicTable = initMusicTableRetail(_musicTable, "Textures/FullMonkeyMap2.imt");
		if (_musicTable == nullptr) {
			tableLoadErrorDialog("Textures/FullMonkeyMap2.imt");
		}
		_musicPrefix = "Textures/spago/"; // Hardcode the high-quality music for now.
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
}

void EMISound::pushStateToStack() {
	if (_musicChannel != -1 && _channels[_musicChannel]) {
		_channels[_musicChannel]->fadeOut();
		_stateStack.push(_channels[_musicChannel]);
		_channels[_musicChannel] = nullptr;
	} else {
		_stateStack.push(nullptr);
	}
}

void EMISound::popStateFromStack() {
	if (_musicChannel != -1 && _channels[_musicChannel]) {
		_channels[_musicChannel]->fadeOut();
	}

	_musicChannel = getFreeChannel();
	assert(_musicChannel != -1);

	//even pop state from stack if music isn't set
	_channels[_musicChannel] = _stateStack.pop();

	if (_channels[_musicChannel]) {
		if (_channels[_musicChannel]->isPaused())
			_channels[_musicChannel]->pause();
		_channels[_musicChannel]->fadeIn();
	}
}

void EMISound::flushStack() {
	while (!_stateStack.empty()) {
		SoundTrack *temp = _stateStack.pop();
		delete temp;
	}
}

void EMISound::callback() {
	Common::StackLock lock(_mutex);

	for (uint i = 0; i < _stateStack.size(); ++i) {
		SoundTrack *track = _stateStack[i];
		if (track == nullptr || !track->getHandle() || track->isPaused())
			continue;

		updateTrack(track);
		if (track->getFadeMode() == SoundTrack::FadeOut && track->getFade() == 0.0f) {
			track->pause();
		} else {
			g_system->getMixer()->setChannelVolume(*track->getHandle(), track->getVolume() * track->getFade());
		}
	}

	for (int i = 0; i < NUM_CHANNELS; i++) {
		SoundTrack *track = _channels[i];
		if (track == nullptr || !track->getHandle() || track->isPaused())
			continue;

		updateTrack(track);
		if (track->getFadeMode() == SoundTrack::FadeOut && track->getFade() == 0.0f) {
			freeChannel(i);
		} else {
			g_system->getMixer()->setChannelVolume(*track->getHandle(), track->getVolume() * track->getFade());
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
		g_system->getMixer()->setChannelVolume(*track->getHandle(), track->getVolume() * fade);
	}
}

void EMISound::restoreState(SaveGame *savedState) {
	// Clear any current music
	flushStack();
	setMusicState(0);
	freeAllChannels();
	// Actually load:
	savedState->beginSection('SOUN');
	_musicPrefix = savedState->readString();
	// Stack:
	uint32 stackSize = savedState->readLEUint32();
	for (uint32 i = 0; i < stackSize; i++) {
		SoundTrack *track = nullptr;
		Common::String soundName = savedState->readString();
		if (!soundName.empty()) {
			track = createEmptyMusicTrack();
			if (initTrack(soundName, track)) {
				track->play();
				track->pause();
			} else {
				error("Couldn't reopen %s", soundName.c_str());
			}
		}
		_stateStack.push(track);
	}
	// Currently playing music:
	uint32 hasActiveTrack = savedState->readLEUint32();
	if (hasActiveTrack) {
		_musicChannel = getFreeChannel();
		assert(_musicChannel != -1);
		SoundTrack *music = createEmptyMusicTrack();
		_channels[_musicChannel] = music;
		Common::String soundName = savedState->readString();
		if (initTrack(soundName, music)) {
			music->play();
		} else {
			error("Couldn't reopen %s", soundName.c_str());
		}
	}
	// Channels:
	uint32 numChannels = savedState->readLEUint32();
	if (numChannels > NUM_CHANNELS) {
		error("Save game made with more channels than we have now: %d > %d", numChannels, NUM_CHANNELS);
	}
	for (uint32 i = 0; i < numChannels; i++) {
		uint32 channelIsActive = savedState->readLEUint32();
		if (channelIsActive) {
			Common::String soundName = savedState->readString();
			uint32 volume = savedState->readLEUint32();
			uint32 pan = savedState->readLEUint32();
			/*uint32 pos = */savedState->readLEUint32();
			/*bool isPlaying = */savedState->readByte();
			startVoice(soundName.c_str(), volume, pan); // FIXME: Could be music also.
		}
	}
	savedState->endSection();
}

void EMISound::saveState(SaveGame *savedState) {
	savedState->beginSection('SOUN');
	savedState->writeString(_musicPrefix);
	// Stack:
	uint32 stackSize = _stateStack.size();
	savedState->writeLEUint32(stackSize);
	// TODO: Save actual state, instead of just the file needed.
	// We'll need repeatable state first though.
	for (uint32 i = 0; i < stackSize; i++) {
		if (_stateStack[i]) {
		    savedState->writeString(_stateStack[i]->getSoundName());
		} else {
		    savedState->writeString("");
		}
	}
	// Currently playing music:
	if (_musicChannel != -1 && _channels[_musicChannel]) {
		savedState->writeLEUint32(1);
		savedState->writeString(_channels[_musicChannel]->getSoundName());
	} else {
		savedState->writeLEUint32(0);
	}
	// Channels:
	uint32 numChannels = NUM_CHANNELS;
	savedState->writeLEUint32(numChannels);
	for (uint32 i = 0; i < numChannels; i++) {
		if (!_channels[i]) {
			savedState->writeLEUint32(0);
		} else {
			savedState->writeLEUint32(1);
			savedState->writeString(_channels[i]->getSoundName());
			savedState->writeLEUint32(255); // TODO: Place-holder for volume.
			savedState->writeLEUint32(255); // TODO: Place-holder for pan.
			savedState->writeLEUint32(0); // TODO: Place-holder for position.
			savedState->writeByte(1); // isPlaying.
		}
	}
	savedState->endSection();
}

} // end of namespace Grim
