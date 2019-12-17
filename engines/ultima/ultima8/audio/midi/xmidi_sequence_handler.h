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

#ifndef ULTIMA8_AUDIO_MIDI_SEQUENCEHANDLER_H
#define ULTIMA8_AUDIO_MIDI_SEQUENCEHANDLER_H

namespace Ultima {
namespace Ultima8 {

struct XMidiEvent;

//! Abstract class for handling the playing of XMidiSequence objects
class XMidiSequenceHandler {
public:
	virtual ~XMidiSequenceHandler() { }

	//! An event sent from a sequence to play
	//! \param sequence_id The id of the sequence that is attempting to send the event
	//! \param message The Event being sent
	virtual void    sequenceSendEvent(uint16 sequence_id, uint32 message) = 0;

	//! Send a SysEx event from a Sequence
	//! \param sequence_id The id of the sequence that is attempting to send the event
	//! \param status The Status Byte of the SysEx event
	//! \param msg The SysEx data (including terminator byte)
	//! \param length The number of bytes of SysEx data to send
	virtual void    sequenceSendSysEx(uint16 sequence_id, uint8 status, const uint8 *msg, uint16 length) = 0;

	//! An event sent from a sequence to play
	//! \param sequence_id The id of the sequence requesting the tick count
	//! \return Number of ticks, in 6000ths of a second
	virtual uint32  getTickCount(uint16 sequence_id) = 0;

	//! Handle an XMIDI Callback Trigger Event
	//! \param sequence_id The id of the sequence doing the callback
	//! \param data data[1] of the XMIDI_CONTROLLER_CALLBACK_TRIG event
	virtual void    handleCallbackTrigger(uint16 sequence_id, uint8 data) = 0;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
