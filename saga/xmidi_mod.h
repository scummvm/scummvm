/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
/*
 Description:   
 
    XMIDI conversion routines - module header

 Notes: 
*/

#ifndef SAGA_XMIDI_MOD_H_
#define SAGA_XMIDI_MOD_H_

namespace Saga {

#define MIDI_STD_VELOCITY 0x7F
#define MIDI_STD_TEMPO    0x0007A120L	/* 500000 */

/* MIDI Events 
\*--------------------------------------------------------------------------*/
enum R_MIDI_EVENTS {
	MIDI_NOTE_ON = 0x90,
	MIDI_NOTE_OFF = 0x80,
	MIDI_AFTERTOUCH = 0xA0,
	MIDI_CONTROLCHANGE = 0xB0,
	MIDI_PROGRAMCHANGE = 0xC0,
	MIDI_CHANNELPRESSURE = 0xD0,
	MIDI_PITCHWHEEL = 0xE0,
	MIDI_SYSTEMEXCLUSIVE = 0xF0,

	MIDI_NONMIDI = 0xFF,

	MIDI_SYSEX_SEQNUM = 0x00,
	MIDI_SYSEX_TEXT = 0x01,
	MIDI_SYSEX_COPYRIGHT = 0x02,
	MIDI_SYSEX_SEQNAME = 0x03,
	MIDI_SYSEX_INSTRUMENT = 0x04,
	MIDI_SYSEX_LYRIC = 0x05,
	MIDI_SYSEX_MARKER = 0x06,
	MIDI_SYSEX_CUEPOINT = 0x07,

	MIDI_SYSEX_CHANNEL = 0x20,
	MIDI_SYSEX_PORTNUM = 0x21,
	MIDI_SYSEX_TRACKEND = 0x2F,
	MIDI_SYSEX_TEMPO = 0x51,
	MIDI_SYSEX_TIMESIG = 0x58,

	MIDI_SYSEX_PROPRIETARY = 0x7F
};

enum MIDI_EVENT_LENGTHS {
	MIDI_NOTE_ON_LEN = 3,
	MIDI_NOTE_OFF_LEN = 3,
	MIDI_AFTERTOUCH_LEN = 3,
	MIDI_CONTROLCHANGE_LEN = 3,
	MIDI_PITCHWHEEL_LEN = 3,

	MIDI_PROGRAMCHANGE_LEN = 2,
	MIDI_CHANNELPRESSURE_LEN = 2,

	MIDI_SYSEX_TRACKEND_LEN = 3,
	MIDI_SYSEX_TEMPO_LEN = 6,
	MIDI_SYSEX_TIMESIG_LEN = 7
};

typedef struct XMIDIEVENT_tag {
	struct XMIDIEVENT_tag *prev_event;
	struct XMIDIEVENT_tag *next_event;

	size_t smf_size;	/* Size of event in SMF format */

	ulong delta_time;

	uchar event;
	uchar channel;
	uchar sysex_op;
	uchar op1;
	uchar op2;
	uchar op3;
	uchar op4;
	uchar pad;
} XMIDIEVENT;

typedef struct XMIDIEVENT_LIST_tag {
	XMIDIEVENT *head;
	XMIDIEVENT *tail;
	int smf_size;
} XMIDIEVENT_LIST;

int XMIDI_Read(const uchar *XMI_img, XMIDIEVENT_LIST *event_list);
int XMIDI_Free(XMIDIEVENT_LIST *event_list);

}				// End of namespace Saga

#endif				/* SAGA_XMIDI_MOD_H_ */
