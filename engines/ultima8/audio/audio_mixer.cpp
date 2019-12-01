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

#include "ultima8/misc/pent_include.h"
#include "ultima8/audio/audio_mixer.h"
#include "ultima8/conf/setting_manager.h"
#include "ultima8/kernel/kernel.h"

#include "ultima8/audio/audio_process.h"
#include "ultima8/audio/music_process.h"
#include "ultima8/audio/AudioChannel.h"
#include "ultima8/audio/midi/MidiDriver.h"

//include SDL.h

namespace Ultima8 {
namespace Pentagram {

AudioMixer *AudioMixer::the_audio_mixer = 0;

AudioMixer::AudioMixer(int sample_rate_, bool stereo_, int num_channels_) :
	audio_ok(false),
	sample_rate(sample_rate_), stereo(stereo_),
	midi_driver(0), midi_volume(255),
	num_channels(num_channels_), channels(0) {
	the_audio_mixer = this;

	con.Print(MM_INFO, "Creating AudioMixer...\n");

	SDL_AudioSpec desired, obtained;

	desired.format = AUDIO_S16SYS;
	desired.freq = sample_rate_;
	desired.channels = stereo_ ? 2 : 1;
	desired.samples = 1024;
	desired.callback = sdlAudioCallback;
	desired.userdata = reinterpret_cast<void *>(this);

#ifdef UNDER_CE
	desired.freq = 11025;
	desired.channels = 1;
#endif

	// Open SDL Audio (even though we may not need it)
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	int ret = SDL_OpenAudio(&desired, &obtained);
	audio_ok = (ret == 0);

	if (audio_ok) {
		pout << "Audio opened using format: " << obtained.freq << " Hz " << (int) obtained.channels << " Channels" <<  std::endl;
		// Lock the audio
		Lock();

		sample_rate = obtained.freq;
		stereo = obtained.channels == 2;

		channels = new AudioChannel*[num_channels];
		for (int i = 0; i < num_channels; i++)
			channels[i] = new AudioChannel(sample_rate, stereo);

		// Unlock it
		Unlock();

		// GO GO GO!
		SDL_PauseAudio(0);
	}
}

void AudioMixer::createProcesses() {
	Kernel *kernel = Kernel::get_instance();

	// Create the Audio Process
	kernel->addProcess(new AudioProcess());

	// Create the Music Process
	kernel->addProcess(new MusicProcess(midi_driver));
}

AudioMixer::~AudioMixer(void) {
	con.Print(MM_INFO, "Destroying AudioMixer...\n");

	closeMidiOutput();

	SDL_CloseAudio();

	the_audio_mixer = 0;

	if (channels) for (int i = 0; i < num_channels; i++) delete channels[i];
	delete [] channels;
}

void AudioMixer::Lock() {
	SDL_LockAudio();
}

void AudioMixer::Unlock() {
	SDL_UnlockAudio();
}

void AudioMixer::reset() {
	if (!audio_ok) return;

	con.Print(MM_INFO, "Resetting AudioMixer...\n");

	Lock();

	if (midi_driver) {
		for (int i = 0; i < midi_driver->maxSequences(); i++) {
			midi_driver->finishSequence(i);
		}
	}

	if (channels) for (int i = 0; i < num_channels; i++) channels[i]->stop();

	Unlock();
}

int AudioMixer::playSample(AudioSample *sample, int loop, int priority, bool paused, uint32 pitch_shift_, int lvol, int rvol) {
	if (!audio_ok || !channels) return -1;

	int lowest = -1;
	int lowprior = 65536;

	// Lock the audio
	Lock();

	int i;
	for (i = 0; i < num_channels; i++) {
		if (!channels[i]->isPlaying()) {
			lowest = i;
			break;
		} else if (channels[i]->getPriority() < priority) {
			lowprior = channels[i]->getPriority();
			lowest = i;
		}
	}

	if (i != num_channels || lowprior < priority)
		channels[lowest]->playSample(sample, loop, priority, paused, pitch_shift_, lvol, rvol);
	else
		lowest = -1;

	// Unlock
	Unlock();

	return lowest;
}

bool AudioMixer::isPlaying(int chan) {
	if (chan > num_channels || chan < 0 || !channels || !audio_ok) return 0;

	Lock();

	bool playing = channels[chan]->isPlaying();

	Unlock();

	return playing;
}

void AudioMixer::stopSample(int chan) {
	if (chan > num_channels || chan < 0 || !channels || !audio_ok) return;

	Lock();

	channels[chan]->stop();

	Unlock();
}

void AudioMixer::setPaused(int chan, bool paused) {
	if (chan > num_channels || chan < 0 || !channels || !audio_ok) return;

	Lock();

	channels[chan]->setPaused(paused);

	Unlock();
}

bool AudioMixer::isPaused(int chan) {
	if (chan > num_channels || chan < 0 || !channels || !audio_ok) return false;

	Lock();

	bool ret = channels[chan]->isPaused();

	Unlock();

	return ret;
}

void AudioMixer::setVolume(int chan, int lvol, int rvol) {
	if (chan > num_channels || chan < 0 || !channels || !audio_ok) return;

	Lock();

	channels[chan]->setVolume(lvol, rvol);

	Unlock();
}

void AudioMixer::getVolume(int chan, int &lvol, int &rvol) {
	if (chan > num_channels || chan < 0 || !channels || !audio_ok) return;

	Lock();

	channels[chan]->getVolume(lvol, rvol);

	Unlock();
}


void AudioMixer::sdlAudioCallback(void *userdata, Uint8 *stream, int len) {
	AudioMixer *mixer = reinterpret_cast<AudioMixer *>(userdata);

	mixer->MixAudio(reinterpret_cast<int16 *>(stream), len);
}

void AudioMixer::MixAudio(int16 *stream, uint32 bytes) {
	if (!audio_ok) return;

	if (midi_driver && midi_driver->isSampleProducer())
		midi_driver->produceSamples(stream, bytes);

	if (channels) for (int i = 0; i < num_channels; i++)
			if (channels[i]->isPlaying()) channels[i]->resampleAndMix(stream, bytes);
}

void AudioMixer::openMidiOutput() {
	if (midi_driver) return;
	if (!audio_ok) return;

	MidiDriver *new_driver = 0;
	con.Print(MM_INFO, "Initializing MidiDriver...\n");

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
}

void AudioMixer::closeMidiOutput() {
	if (!midi_driver) return;
	con.Print(MM_INFO, "Destroying MidiDriver...\n");

	midi_driver->destroyMidiDriver();

	Lock();
	delete midi_driver;
	midi_driver = 0;
	Unlock();
}

} // End of namespace Pentagram
} // End of namespace Ultima8
