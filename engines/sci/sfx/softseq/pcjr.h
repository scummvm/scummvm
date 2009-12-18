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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sci/sfx/softseq/mididriver.h"

namespace Sci {

class MidiDriver_PCJr : public MidiDriver_Emulated {
public:
	friend class MidiPlayer_PCJr;

	enum {
		kMaxChannels = 3
	};

	MidiDriver_PCJr(Audio::Mixer *mixer) : MidiDriver_Emulated(mixer) { }
	~MidiDriver_PCJr() { }

	// MidiDriver
	int open() { return open(kMaxChannels); }
	void close();
	void send(uint32 b);
	MidiChannel *allocateChannel() { return NULL; }
	MidiChannel *getPercussionChannel() { return NULL; }

	// AudioStream
	bool isStereo() const { return false; }
	int getRate() const { return _mixer->getOutputRate(); }

	// MidiDriver_Emulated
	void generateSamples(int16 *buf, int len);

	int open(int channels);
private:
	int _channels_nr;
	int _global_volume; // Base volume
	int _volumes[kMaxChannels];
	int _notes[kMaxChannels]; // Current halftone, or 0 if off
	int _freq_count[kMaxChannels];
	int _channel_assigner;
	int _channels_assigned;
	int _chan_nrs[kMaxChannels];
};

class MidiPlayer_PCJr : public MidiPlayer {
public:
	MidiPlayer_PCJr() { _driver = new MidiDriver_PCJr(g_system->getMixer()); }
	int open(ResourceManager *resMan) { return static_cast<MidiDriver_PCJr *>(_driver)->open(getPolyphony()); }
	int getPlayMask() const { return 0x10; }
	int getPolyphony() const { return 3; }
	bool hasRhythmChannel() const { return false; }
	void setVolume(byte volume) { static_cast<MidiDriver_PCJr *>(_driver)->_global_volume = volume; }
};

class MidiPlayer_PCSpeaker : public MidiPlayer_PCJr {
public:
	int getPlayMask() const { return 0x20; }
	int getPolyphony() const { return 1; }
};

} // End of namespace Sci

