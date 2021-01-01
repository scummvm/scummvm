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

#ifndef ULTIMA8_AUDIO_AUDIOMIXER_H
#define ULTIMA8_AUDIO_AUDIOMIXER_H

#include "audio/mixer.h"
#include "audio/mididrv.h"
#include "common/array.h"

namespace Ultima {
namespace Ultima8 {

class MidiPlayer;
class AudioChannel;
class AudioSample;

class AudioMixer {
private:
	static AudioMixer *_audioMixer;
	Audio::Mixer *_mixer;
	MidiPlayer *_midiPlayer;
	Common::Array<AudioChannel *> _channels;

	void            Lock();
	void            Unlock();
public:
	AudioMixer(Audio::Mixer *mixer);
	~AudioMixer();

	MidiPlayer *getMidiPlayer() const {
		return _midiPlayer;
	}

	static AudioMixer  *get_instance() {
		return _audioMixer;
	}

	void            reset();
	void            createProcesses();

	int             playSample(AudioSample *sample, int loop, int priority, bool paused, uint32 pitch_shift, int lvol, int rvol, bool ambient);
	bool            isPlaying(int chan);
	void            stopSample(int chan);

	void            setPaused(int chan, bool paused);
	bool            isPaused(int chan);

	void            setVolume(int chan, int lvol, int rvol);
	void            getVolume(int chan, int &lvol, int &rvol);

	void            openMidiOutput();
	void            closeMidiOutput();
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
