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

#ifndef ULTIMA8_AUDIO_MIDI_ALSAMIDIDRIVER_H
#define ULTIMA8_AUDIO_MIDI_ALSAMIDIDRIVER_H

#ifdef USE_ALSA_MIDI

#include <alsa/asoundlib.h>
#include "ultima/ultima8/std/string.h"
#include "ultima/ultima8/audio/midi/low_level_midi_driver.h"

namespace Ultima {
namespace Ultima8 {

class ALSAMidiDriver : public LowLevelMidiDriver {
	const static MidiDriverDesc desc;
	static MidiDriver *createInstance() {
		return new ALSAMidiDriver();
	}

public:
	static const MidiDriverDesc *getDesc() {
		return &desc;
	}
	ALSAMidiDriver();

protected:
	virtual int         open();
	virtual void        close();
	virtual void        send(uint32 message);
//	virtual void     yield();
	virtual void        send_sysex(uint8 status, const uint8 *msg,
	                               uint16 length);

	std::string devname;
	bool isOpen;

	snd_seq_event_t ev;
	snd_seq_t *seq_handle;
	int seq_client, seq_port;
	int my_client, my_port;

	void send_event(int do_flush);
	int parse_addr(std::string arg, int *client, int *port);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif

#endif
