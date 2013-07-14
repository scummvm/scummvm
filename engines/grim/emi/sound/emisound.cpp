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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "gui/error.h"

#include "common/stream.h"
#include "common/mutex.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/mixer.h"
#include "engines/grim/sound.h"
#include "engines/grim/grim.h"
#include "engines/grim/resource.h"
#include "engines/grim/textsplit.h"
#include "engines/grim/emi/sound/emisound.h"
#include "engines/grim/emi/sound/track.h"
#include "engines/grim/emi/sound/mp3track.h"
#include "engines/grim/emi/sound/scxtrack.h"
#include "engines/grim/emi/sound/vimatrack.h"

#define NUM_CHANNELS 32

namespace Grim {

class SoundTrack;

EMISound::EMISound() {
	_channels = new SoundTrack*[NUM_CHANNELS];
	for (int i = 0; i < NUM_CHANNELS; i++) {
		_channels[i] = NULL;
	}
	_music = NULL;
	initMusicTable();
}

EMISound::~EMISound() {
	for (int i = 0; i < NUM_CHANNELS; i++) {
		freeChannel(i);
	}
	delete _music;
	delete[] _channels;
	delete[] _musicTable;
}

int32 EMISound::getFreeChannel() {
	for (int i = 0; i < NUM_CHANNELS; i++) {
		if (_channels[i] == NULL)
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
	_channels[channel] = NULL;
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
	return (62.5 / 60.0) * g_system->getMixer()->getSoundElapsedTime(*_channels[channel]->getHandle()); //16 ms is 62.5 Hz
}

void EMISound::setVolume(const char *soundName, int volume) {
	int32 channel = getChannelByName(soundName);
	assert(channel != -1);
	g_system->getMixer()->setChannelVolume(*_channels[channel]->getHandle(), volume);
}

void EMISound::setPan(const char *soundName, int pan) {
	warning("EMI doesn't support sound-panning yet, %s", soundName);
}

void EMISound::setMusicState(int stateId) {
	if (_music) {
		delete _music;
		_music = NULL;
	}
	if (stateId == 0)
		return;
	if (_musicTable == NULL) {
		warning("No music table loaded");
		return;
	}
	if (_musicTable[stateId]._id != stateId) {
		warning("Attempted to play track #%d, not found in music table!", stateId);
		return;
	}
	Common::String filename;
	if (g_grim->getGamePlatform() == Common::kPlatformPS2) {
		warning("PS2 doesn't have musictable yet %d ignored, just playing 1195.SCX", stateId);
		// So, we just rig up the menu-song hardcoded for now, as a test of the SCX-code.
		filename = "1195.SCX";
		_music = new SCXTrack(Audio::Mixer::kMusicSoundType);
	} else {
		filename = _musicTable[stateId]._filename;
		_music = new MP3Track(Audio::Mixer::kMusicSoundType);
	}
	warning("Loading music: %s", filename.c_str());
	Common::SeekableReadStream *str = g_resourceloader->openNewStreamFile(_musicPrefix + filename);

	if (_music->openSound(filename, str))
		_music->play();
}

uint32 EMISound::getMsPos(int stateId) {
	if (!_music || !_music->getHandle())
		return 0;
	return g_system->getMixer()->getSoundElapsedTime(*_music->getHandle());
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
		return NULL;
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
	const char *errorMessage = 0;
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
		_musicTable = NULL;
		_musicPrefix = "";
	} else {
		_musicTable = NULL;
		_musicTable = initMusicTableRetail(_musicTable, "Textures/FullMonkeyMap1.imt");
		if (_musicTable == NULL) {
			tableLoadErrorDialog("Textures/FullMonkeyMap1.imt");
		}
		_musicTable = initMusicTableRetail(_musicTable, "Textures/FullMonkeyMap2.imt");
		if (_musicTable == NULL) {
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
	if (_music)
		_music->pause();
	_stateStack.push(_music);
	_music = NULL;
}

void EMISound::popStateFromStack() {
	delete _music;

	//even pop state from stack if music isn't set
	_music = _stateStack.pop();

	if (_music) {
		_music->pause();
	}
}

void EMISound::flushStack() {
	while (!_stateStack.empty()) {
		SoundTrack *temp = _stateStack.pop();
		delete temp;
	}
}

} // end of namespace Grim
