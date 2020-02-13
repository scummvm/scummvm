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

#ifndef PRINCE_MUSIC_H
#define PRINCE_MUSIC_H

#include "audio/midiplayer.h"
#include "common/memstream.h"

namespace Prince {

class PrinceEngine;

class MusicPlayer: public Audio::MidiPlayer {
private:
	PrinceEngine *_vm;
	byte *_data;
	int _dataSize;
	bool _isGM;

	// Start MIDI File
	void sndMidiStart();

	// Stop MIDI File
	void sndMidiStop();
public:
	MusicPlayer(PrinceEngine *vm);
	~MusicPlayer() override;

	void loadMidi(const char *);
	void killMidi();

	void send(uint32 b) override;
	void sendToChannel(byte channel, uint32 b) override;

	static const char *_musTable[];
	static const uint8 _musRoomTable[];
};

} // End of namespace Prince

#endif
