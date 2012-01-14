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
//#include "engines/grim/imuse/imuse.h"
#include "engines/grim/sound.h"
#include "engines/grim/grim.h"
#include "engines/grim/resource.h"
#include "engines/grim/emisound/emisound.h"
#include "engines/grim/emisound/track.h"
#include "engines/grim/emisound/vimatrack.h"

#define NUM_CHANNELS 32

namespace Grim {

class SoundTrack;

EMISound::EMISound() {
	_channels = new SoundTrack*[NUM_CHANNELS];
	for (int i = 0; i < NUM_CHANNELS; i++) {
		_channels[i] = NULL;
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

	return _channels[channel]->openSound(soundName, str);
}

bool EMISound::getSoundStatus(const char *soundName) {
	int32 channel = getChannelByName(soundName);
	
	if (channel == -1)	// We have no such sound.
		return false;
	
	return g_system->getMixer()->isSoundHandleActive(*_channels[channel]->_handle) && _channels[channel]->isPlaying();	
}

void EMISound::stopSound(const char *soundName) {
	int32 channel = getChannelByName(soundName);
	assert(channel != -1);
	g_system->getMixer()->stopHandle(*_channels[channel]->_handle);
	freeChannel(channel);
}

int32 EMISound::getPosIn60HzTicks(const char *soundName) {	
	int32 channel = getChannelByName(soundName);
	assert(channel != -1);
	return g_system->getMixer()->getSoundElapsedTime(*_channels[channel]->_handle);
}
	
void EMISound::setVolume(const char *soundName, int volume) {
	int32 channel = getChannelByName(soundName);
	assert(channel != -1);
	g_system->getMixer()->setChannelVolume(*_channels[channel]->_handle, volume);
}

void EMISound::setPan(const char *soundName, int pan) {
	warning("EMI doesn't support sound-panning yet, %s", soundName);
}

} // end of namespace Grim
