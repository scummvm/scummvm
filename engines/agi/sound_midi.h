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

// Music class

#ifndef AGI_SOUND_MIDI_H
#define AGI_SOUND_MIDI_H

#include "agi/sound.h"

#include "audio/midiplayer.h"

namespace Agi {

class SoundGenMIDI : public SoundGen, public Audio::MidiPlayer {
public:
	SoundGenMIDI(AgiBase *vm, Audio::Mixer *pMixer);

	void play(int resnum) override;
	// We must overload stop() here to implement the pure virtual
	// stop() method of the SoundGen class.
	void stop() override { Audio::MidiPlayer::stop(); }

	// MidiDriver_BASE interface implementation
	void send(uint32 b) override;

	// Overload Audio::MidiPlayer method
	void sendToChannel(byte channel, uint32 b) override;
	void endOfTrack() override;

private:
	bool _isGM;
};

} // End of namespace Agi

#endif
