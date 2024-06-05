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

#include "ultima/ultima8/audio/audio_mixer.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/audio/u8_music_process.h"
#include "ultima/ultima8/audio/cru_music_process.h"
#include "ultima/ultima8/audio/audio_channel.h"
#include "ultima/ultima8/audio/midi_player.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {

AudioMixer *AudioMixer::_audioMixer = nullptr;

static const uint32 SAMPLE_RATE = 22050;
static const int BASE_CHANNEL_COUNT = 16;
static const int AMBIENT_CHANNEL_COUNT = 4;
static const int TOTAL_CHANNEL_COUNT = BASE_CHANNEL_COUNT + AMBIENT_CHANNEL_COUNT;

AudioMixer::AudioMixer(Audio::Mixer *mixer) : _mixer(mixer), _midiPlayer(nullptr) {
	_audioMixer = this;

	_channels.resize(TOTAL_CHANNEL_COUNT);
	for (int idx = 0; idx < TOTAL_CHANNEL_COUNT; ++idx)
		_channels[idx] = new AudioChannel(_mixer, SAMPLE_RATE, true);

	debug(1, "Creating AudioMixer...");
}

void AudioMixer::createProcesses() {
	Kernel *kernel = Kernel::get_instance();

	// Create the Audio Process
	kernel->addProcess(new AudioProcess());

	// Create the Music Process
	if (GAME_IS_U8) {
		kernel->addProcess(new U8MusicProcess(_midiPlayer));
	} else if (GAME_IS_CRUSADER) {
		kernel->addProcess(new CruMusicProcess());
	}
}

AudioMixer::~AudioMixer(void) {
	_audioMixer = nullptr;

	debug(1, "Destroying AudioMixer...");

	closeMidiOutput();

	for (int idx = 0; idx < TOTAL_CHANNEL_COUNT; ++idx)
		delete _channels[idx];
}


void AudioMixer::reset() {
	_mixer->stopAll();
}

int AudioMixer::playSample(AudioSample *sample, int loop, int priority, bool isSpeech, uint32 pitch_shift, byte volume, int8 balance, bool ambient) {
	int lowest = -1;
	int lowprior = 65536;

	int i;
	const int minchan = (ambient ? BASE_CHANNEL_COUNT : 0);
	const int maxchan = (ambient ? TOTAL_CHANNEL_COUNT : BASE_CHANNEL_COUNT);
	for (i = minchan; i < maxchan; i++) {
		if (!_channels[i]->isPlaying()) {
			lowest = i;
			break;
		}
		else if (_channels[i]->getPriority() < priority) {
			lowprior = _channels[i]->getPriority();
			lowest = i;
		}
	}

	if (i != maxchan || lowprior < priority)
		_channels[lowest]->playSample(sample, loop, priority, isSpeech, pitch_shift, volume, balance);
	else
		lowest = -1;

	return lowest;
}

bool AudioMixer::isPlaying(int chan) {
	if (chan >= TOTAL_CHANNEL_COUNT || chan < 0)
		return false;

	bool playing = _channels[chan]->isPlaying();

	return playing;
}

void AudioMixer::stopSample(int chan) {
	if (chan >= TOTAL_CHANNEL_COUNT || chan < 0)
		return;

	_channels[chan]->stop();
}

void AudioMixer::setPaused(int chan, bool paused) {
	if (chan >= TOTAL_CHANNEL_COUNT || chan < 0)
		return;

	_channels[chan]->setPaused(paused);
}


void AudioMixer::setVolume(int chan, byte volume, int8 balance) {
	if (chan >= TOTAL_CHANNEL_COUNT || chan < 0)
		return;

	_channels[chan]->setVolume(volume, balance);
}

void AudioMixer::openMidiOutput() {
	_midiPlayer = new MidiPlayer();
}

void AudioMixer::closeMidiOutput() {
	delete _midiPlayer;
	_midiPlayer = nullptr;
}

} // End of namespace Ultima8
} // End of namespace Ultima
