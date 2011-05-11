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
 */

// Music class

#ifndef M4_MIDI_H
#define M4_MIDI_H

#include "audio/midiplayer.h"

namespace M4 {

class MidiPlayer : public Audio::MidiPlayer {
public:
	MidiPlayer(MadsM4Engine *vm);

	void playMusic(const char *name, int32 vol, bool loop, int32 trigger, int32 scene);

	void setGM(bool isGM) { _isGM = isGM; }

	// MidiDriver_BASE interface implementation
	virtual void send(uint32 b);

protected:
	MadsM4Engine *_vm;

	bool _isGM;

	bool _randomLoop;

	byte *_musicData;
	uint16 *_buf;
	size_t _musicDataSize;

	byte *convertHMPtoSMF(byte *data, uint32 inSize, uint32 &outSize);
};

} // End of namespace M4

#endif

