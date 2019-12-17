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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/audio/audio_mixer.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/audio/audio_channel.h"
#include "ultima/ultima8/audio/midi/midi_driver.h"
#include "ultima/ultima8/conf/setting_manager.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "audio/decoders/raw.h"

namespace Ultima {
namespace Ultima8 {
namespace Pentagram {

AudioMixer *AudioMixer::the_audio_mixer = 0;

AudioMixer::AudioMixer(Audio::Mixer *mixer) : _mixer(mixer), _midiDriver(0) {
	the_audio_mixer = this;
	
	_channels.resize(CHANNEL_COUNT);
	for (int idx = 0; idx < CHANNEL_COUNT; ++idx)
		_channels[idx] = new AudioChannel(_mixer, SAMPLE_RATE, true);

	con->Print(MM_INFO, "Creating AudioMixer...\n");
}

void AudioMixer::createProcesses() {
	Kernel *kernel = Kernel::get_instance();

	// Create the Audio Process
	kernel->addProcess(new AudioProcess());

#ifdef TODO
	// Create the Music Process
	kernel->addProcess(new MusicProcess(midi_driver));
#endif
}

AudioMixer::~AudioMixer(void) {
	con->Print(MM_INFO, "Destroying AudioMixer...\n");

	closeMidiOutput();

	for (int idx = 0; idx < CHANNEL_COUNT; ++idx)
		delete _channels[idx];
}

void AudioMixer::Lock() {
	// No implementation
}

void AudioMixer::Unlock() {
	// No implementation
}

void AudioMixer::reset() {
	_mixer->stopAll();
	Unlock();
}

int AudioMixer::playSample(AudioSample *sample, int loop, int priority, bool paused, uint32 pitch_shift_, int lvol, int rvol) {
	int lowest = -1;
	int lowprior = 65536;

	// Lock the audio
	Lock();

	int i;
	for (i = 0; i < CHANNEL_COUNT; i++) {
		if (!_channels[i]->isPlaying()) {
			lowest = i;
			break;
		}
		else if (_channels[i]->getPriority() < priority) {
			lowprior = _channels[i]->getPriority();
			lowest = i;
		}
	}

	if (i != CHANNEL_COUNT || lowprior < priority)
		_channels[lowest]->playSample(sample, loop, priority, paused, pitch_shift_, lvol, rvol);
	else
		lowest = -1;

	// Unlock
	Unlock();

	return lowest;
}

bool AudioMixer::isPlaying(int chan) {
	if (chan >= CHANNEL_COUNT || chan < 0)
		return false;

	Lock();

	bool playing = _channels[chan]->isPlaying();

	Unlock();

	return playing;
}

void AudioMixer::stopSample(int chan) {
	if (chan >= CHANNEL_COUNT || chan < 0)
		return;

	Lock();

	_channels[chan]->stop();

	Unlock();
}

void AudioMixer::setPaused(int chan, bool paused) {
	if (chan >= CHANNEL_COUNT || chan < 0)
		return;

	Lock();

	_channels[chan]->setPaused(paused);

	Unlock();
}

bool AudioMixer::isPaused(int chan) {
	if (chan >= CHANNEL_COUNT|| chan < 0)
		return false;

	Lock();

	bool ret = _channels[chan]->isPaused();

	Unlock();

	return ret;
}

void AudioMixer::setVolume(int chan, int lvol, int rvol) {
	if (chan >= CHANNEL_COUNT || chan < 0) return;

	Lock();

	_channels[chan]->setVolume(lvol, rvol);

	Unlock();
}

void AudioMixer::getVolume(int chan, int &lvol, int &rvol) {
	if (chan >= CHANNEL_COUNT || chan < 0) return;

	Lock();

	_channels[chan]->getVolume(lvol, rvol);

	Unlock();
}

void AudioMixer::openMidiOutput() {
/*
	if (midi_driver) return;
	if (!audio_ok) return;

	MidiDriver *new_driver = 0;
	con->Print(MM_INFO, "Initializing MidiDriver...\n");

	SettingManager *settingman = SettingManager::get_instance();

	// First thing attempt to find the Midi driver as specified in the config
	std::string desired_driver;
	settingman->setDefault("midi_driver", "default");
	settingman->get("midi_driver", desired_driver);

	// Has the config file specified disabled midi?
	if (audio_ok) new_driver = MidiDriver::createInstance(desired_driver, sample_rate, stereo);

	// If the driver is a 'sample' producer we need to hook it to SDL
	if (new_driver) {
		Lock();
		midi_driver = new_driver;
		Unlock();
		midi_driver->setGlobalVolume(midi_volume);
	}
*/
}

void AudioMixer::closeMidiOutput() {
/*
	if (!midi_driver) return;
	con->Print(MM_INFO, "Destroying MidiDriver...\n");

	midi_driver->destroyMidiDriver();

	Lock();
	delete midi_driver;
	midi_driver = 0;
	Unlock();
*/
}

} // End of namespace Pentagram
} // End of namespace Ultima8
} // End of namespace Ultima
