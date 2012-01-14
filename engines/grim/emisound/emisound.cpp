/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "common/stream.h"
#include "common/mutex.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/mixer.h"
#include "engines/grim/sound.h"
#include "engines/grim/grim.h"
#include "engines/grim/resource.h"
#include "engines/grim/textsplit.h"
#include "engines/grim/emisound/emisound.h"
#include "engines/grim/emisound/track.h"
#include "engines/grim/emisound/vimatrack.h"
#include "engines/grim/emisound/mp3track.h"

#define NUM_CHANNELS 32

namespace Grim {

class SoundTrack;

EMISound::EMISound() {
	_channels = new SoundTrack*[NUM_CHANNELS];
	for (int i = 0; i < NUM_CHANNELS; i++) {
		_channels[i] = NULL;
	}
	_music = NULL;
	
	if (g_grim->getGameFlags() == ADGF_DEMO) {
		initMusicTable("Music/FullMonkeyMap.imt");
	}
}

int32 EMISound::getFreeChannel() {
	for (int i = 0; i < NUM_CHANNELS; i++) {
		if (_channels[i] == NULL)
			return i;
	}
	return -1;
}
	
int32 EMISound::getChannelByName(Common::String name) {
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
	
	_channels[channel] = new VimaTrack(soundName);
	
	Common::SeekableReadStream *str = g_resourceloader->openNewStreamFile(soundName);

	if (_channels[channel]->openSound(soundName, str)) {
		_channels[channel]->play();
		return true;
	}
	return false;
}

bool EMISound::getSoundStatus(const char *soundName) {
	int32 channel = getChannelByName(soundName);
	
	if (channel == -1)	// We have no such sound.
		return false;
	
	return g_system->getMixer()->isSoundHandleActive(*_channels[channel]->getHandle()) && _channels[channel]->isPlaying();	
}

void EMISound::stopSound(const char *soundName) {
	int32 channel = getChannelByName(soundName);
	assert(channel != -1);
	g_system->getMixer()->stopHandle(*_channels[channel]->getHandle());
	freeChannel(channel);
}

int32 EMISound::getPosIn60HzTicks(const char *soundName) {	
	int32 channel = getChannelByName(soundName);
	assert(channel != -1);
	return g_system->getMixer()->getSoundElapsedTime(*_channels[channel]->getHandle());
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
	if (g_grim->getGameFlags() != ADGF_DEMO) {
		warning("EMI doesn't have music-support yet %d", stateId);
		return;
	}
	if (_music) {
		delete _music;
		_music = NULL;
	}
	Common::SeekableReadStream *str = g_resourceloader->openNewStreamFile("Music/" + _musicTable[stateId]._filename);
	_music = new MP3Track(Audio::Mixer::kMusicSoundType);
	if (_music->openSound(_musicTable[stateId]._name, str))
		_music->play();
}

void EMISound::initMusicTable(Common::String filename) {
	Common::SeekableReadStream *data = g_resourceloader->openNewStreamFile(filename);
	// FIXME, for now we use a fixed-size table, as I haven't looked at the retail-data yet.
	_musicTable = new MusicEntry[15];
	
	TextSplitter *ts = new TextSplitter(data);
	char *line;
	ts->setLineNumber(3); // Skip top-comment
	int id, x, y, sync;
	int i = 0;
	char musicfilename[64];
	char name[64];
	line = ts->getCurrentLine();
	while (ts->checkString("/*")) {
		ts->nextLine();
		ts->scanString(".cuebutton id %d x %d y %d sync %d \"%[^\"]64s", 5, &id, &x, &y, &sync, name);
		ts->scanString(".playfile \"%[^\"]64s", 1, musicfilename);
		_musicTable[id]._id = id;
		_musicTable[id]._x = x;
		_musicTable[id]._y = y;
		_musicTable[id]._sync = sync;
		_musicTable[id]._name = name;
		_musicTable[id]._filename = musicfilename;
		ts->nextLine();
	}
}
	
} // end of namespace Grim
