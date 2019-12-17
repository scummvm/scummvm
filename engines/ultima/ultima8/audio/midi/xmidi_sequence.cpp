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

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/audio/midi/xmidi_sequence.h"
#include "ultima/ultima8/audio/midi/xmidi_sequence_handler.h"
#include "ultima/ultima8/audio/midi/xmidi_file.h"

namespace Ultima {
namespace Ultima8 {

// Define this to stop the Midisequencer from attempting to
// catch up time if it has missed over 1200 ticks or 1/5th of a second
// This is useful for when debugging, since the Sequencer will not attempt
// to play hundreds of events at the same time if execution is broken, and
// later resumed.
#define XMIDISEQUENCER_NO_CATCHUP_WAIT_OVER 1200

const uint16    XMidiSequence::ChannelShadow::centre_value = 0x2000;
const uint8     XMidiSequence::ChannelShadow::fine_value = centre_value & 127;
const uint8     XMidiSequence::ChannelShadow::coarse_value = centre_value >> 7;
const uint16    XMidiSequence::ChannelShadow::combined_value = (coarse_value << 8) | fine_value;

XMidiSequence::XMidiSequence(XMidiSequenceHandler *Handler, uint16 seq_id, XMidiEventList *events,
                             bool Repeat, int volume, int branch) :
	handler(Handler), sequence_id(seq_id), evntlist(events), event(0),
	repeat(Repeat),  notes_on(), last_tick(0), loop_num(-1), vol_multi(volume),
	paused(false), speed(100) {
	std::memset(loop_event, 0, XMIDI_MAX_FOR_LOOP_COUNT * sizeof(int));
	std::memset(loop_count, 0, XMIDI_MAX_FOR_LOOP_COUNT * sizeof(int));
	event = evntlist->events;

	for (int i = 0; i < 16; i++) {
		shadows[i].reset();
		handler->sequenceSendEvent(sequence_id, i | (MIDI_STATUS_CONTROLLER << 4) | (XMIDI_CONTROLLER_BANK_CHANGE << 8));
	}

	// Jump to branch
	XMidiEvent *brnch = events->findBranchEvent(branch);
	if (brnch) {
		last_tick = brnch->time;
		event = brnch;

		XMidiEvent  *event_ = evntlist->events;
		while (event_ != brnch) {
			updateShadowForEvent(event_);
			event_ = event_->next;
		}
		for (int i = 0; i < 16; i++) gainChannel(i);
	}

	initClock();
}

XMidiSequence::~XMidiSequence() {
	// Handle note off's here
	while (XMidiEvent *note = notes_on.Pop())
		handler->sequenceSendEvent(sequence_id, note->status + (note->data[0] << 8));

	for (int i = 0; i < 16; i++) {
		shadows[i].reset();
		applyShadow(i);
	}

	// 'Release' it
	evntlist->decerementCounter();
}

void XMidiSequence::ChannelShadow::reset() {
	pitchWheel = combined_value;

	program = -1;

	// Bank Select
	bank[0] = 0;
	bank[1] = 0;

	// Modulation Wheel
	modWheel[0] = coarse_value;
	modWheel[1] = fine_value;

	// Foot pedal
	footpedal[0] = 0;
	footpedal[1] = 0;

	// Volume
	volumes[0] = coarse_value;
	volumes[1] = fine_value;

	// Pan
	pan[0] = coarse_value;
	pan[1] = fine_value;

	// Balance
	balance[0] = coarse_value;
	balance[1] = fine_value;

	// Expression
	expression[0] = 127;
	expression[1] = 0;

	// sustain
	sustain = 0;

	// Effects (Reverb)
	effects = 0;

	// Chorus
	chorus = 0;

	// Xmidi Bank
	xbank = 0;
}

int XMidiSequence::playEvent() {
	XMidiEvent *note;

	// Handle note off's here
	while ((note = notes_on.PopTime(getRealTime())) != 0)
		handler->sequenceSendEvent(sequence_id, note->status + (note->data[0] << 8));

	// No events left, but we still have notes on, so say we are still playing, if not report we've finished
	if (!event) {
		if (notes_on.GetNotes()) return 1;
		else return -1;
	}

	// Effectively paused, so indicate it
	if (speed <= 0 || paused) return 1;

	// Play all waiting notes;
	int32 aim = ((event->time - last_tick) * 5000) / speed;
	int32 diff = aim - getTime();

	if (diff > 5) return 1;

	addOffset(aim);

	last_tick = event->time;

#ifdef XMIDISEQUENCER_NO_CATCHUP_WAIT_OVER
	if (diff < -XMIDISEQUENCER_NO_CATCHUP_WAIT_OVER) addOffset(-diff);
#endif

	// Handle note off's here too
	while ((note = notes_on.PopTime(getRealTime())) != 0)
		handler->sequenceSendEvent(sequence_id, note->status + (note->data[0] << 8));

	// XMidi For Loop
	if ((event->status >> 4) == MIDI_STATUS_CONTROLLER && event->data[0] == XMIDI_CONTROLLER_FOR_LOOP) {
		if (loop_num < XMIDI_MAX_FOR_LOOP_COUNT) loop_num++;

		loop_count[loop_num] = event->data[1];
		loop_event[loop_num] = event;

	}   // XMidi Next/Break
	else if ((event->status >> 4) == MIDI_STATUS_CONTROLLER && event->data[0] == XMIDI_CONTROLLER_NEXT_BREAK) {
		if (loop_num != -1) {
			if (event->data[1] < 64) {
				loop_num--;
			}
		} else {
			// See if we can find the branch index
			// If we can, jump to that
			XMidiEvent *branch = evntlist->findBranchEvent(126);

			if (branch) {
				loop_num = 0;
				loop_count[loop_num] = 1;
				loop_event[loop_num] = branch;
			}
		}
		event = NULL;
	}   // XMidi Callback Trigger
	else if ((event->status >> 4) == MIDI_STATUS_CONTROLLER && event->data[0] == XMIDI_CONTROLLER_CALLBACK_TRIG) {
		handler->handleCallbackTrigger(sequence_id, event->data[1]);
	}   // Not SysEx
	else if (event->status < 0xF0) {
		sendEvent();
	}
	// SysEx gets sent immediately
	else if (event->status != 0xFF) {
		handler->sequenceSendSysEx(sequence_id, event->status,
		                           event->ex.sysex_data.buffer, event->ex.sysex_data.len);
	}

	// If we've got another note, play that next
	if (event) event = event->next;

	// Now, handle what to do when we are at the end
	if (!event) {
		// If we have for loop events, follow them
		if (loop_num != -1) {
			event = loop_event[loop_num]->next;
			last_tick = loop_event[loop_num]->time;

			if (loop_count[loop_num])
				if (!--loop_count[loop_num])
					loop_num--;
		}
		// Or, if we are repeating, but there hasn't been any for loop events,
		// repeat from the start
		else if (repeat) {
			event = evntlist->events;
			if (last_tick == 0) return 1;
			last_tick = 0;
		}
		// If we are not repeating, then return saying we are end
		else {
			if (notes_on.GetNotes()) return 1;
			return -1;
		}
	}

	if (!event) {
		if (notes_on.GetNotes()) return 1;
		else return -1;
	}

	aim = ((event->time - last_tick) * 5000) / speed;
	diff = aim - getTime();

	if (diff < 0) return 0; // Next event is ready now!
	return 1;
}

int32 XMidiSequence::timeTillNext() {
	int32 sixthoToNext = 0x7FFFFFFF; // Int max

	// Time remaining on notes currently being played
	XMidiEvent *note;
	note = notes_on.GetNotes();
	if (note) {
		int32 diff = note->ex.note_on.note_time - getRealTime();
		if (diff < sixthoToNext) sixthoToNext = diff;
	}

	// Time till the next event, if we are playing
	if (speed > 0 && event && !paused) {
		int32 aim = ((event->time - last_tick) * 5000) / speed;
		int32 diff = aim - getTime();

		if (diff < sixthoToNext) sixthoToNext = diff;
	}
	return sixthoToNext / 6;
}

void XMidiSequence::updateShadowForEvent(XMidiEvent *event_) {
	unsigned int chan = event_->status & 0xF;
	unsigned int type = event_->status >> 4;
	uint32 data = event_->data[0] | (event_->data[1] << 8);

	// Shouldn't be required. XMidi should automatically detect all anyway
	//evntlist->chan_mask |= 1 << chan;

	// Update the shadows here

	if (type == MIDI_STATUS_CONTROLLER) {
		// Channel volume
		if (event_->data[0] == 7) {
			shadows[chan].volumes[0] = event_->data[1];
		} else if (event_->data[0] == 39) {
			shadows[chan].volumes[1] = event_->data[1];
		}
		// Bank
		else if (event_->data[0] == 0 || event_->data[0] == 32) {
			shadows[chan].bank[event_->data[0] / 32] = event_->data[1];
		}
		// modWheel
		else if (event_->data[0] == 1 || event_->data[0] == 33) {
			shadows[chan].modWheel[event_->data[0] / 32] = event_->data[1];
		}
		// footpedal
		else if (event_->data[0] == 4 || event_->data[0] == 36) {
			shadows[chan].footpedal[event_->data[0] / 32] = event_->data[1];
		}
		// pan
		else if (event_->data[0] == 9 || event_->data[0] == 41) {
			shadows[chan].pan[event_->data[0] / 32] = event_->data[1];
		}
		// balance
		else if (event_->data[0] == 10 || event_->data[0] == 42) {
			shadows[chan].balance[event_->data[0] / 32] = event_->data[1];
		}
		// expression
		else if (event_->data[0] == 11 || event_->data[0] == 43) {
			shadows[chan].expression[event_->data[0] / 32] = event_->data[1];
		}
		// sustain
		else if (event_->data[0] == 64) {
			shadows[chan].effects = event_->data[1];
		}
		// effect
		else if (event_->data[0] == 91) {
			shadows[chan].effects = event_->data[1];
		}
		// chorus
		else if (event_->data[0] == 93) {
			shadows[chan].chorus = event_->data[1];
		}
		// XMidi bank
		else if (event_->data[0] == XMIDI_CONTROLLER_BANK_CHANGE) {
			shadows[chan].xbank = event_->data[1];
		}
	} else if (type == MIDI_STATUS_PROG_CHANGE) {
		shadows[chan].program = data;
	} else if (type == MIDI_STATUS_PITCH_WHEEL) {
		shadows[chan].pitchWheel = data;
	}
}

void XMidiSequence::sendEvent() {
	//unsigned int chan = event->status & 0xF;
	unsigned int type = event->status >> 4;
	uint32 data = event->data[0] | (event->data[1] << 8);

	// Shouldn't be required. XMidi should automatically detect all anyway
	//evntlist->chan_mask |= 1 << chan;

	// Update the shadows here
	updateShadowForEvent(event);

	if (type == MIDI_STATUS_CONTROLLER) {
		// Channel volume
		if (event->data[0] == 7) {
			data = event->data[0] | (((event->data[1] * vol_multi) / 0xFF) << 8);
		}
	} else if (type == MIDI_STATUS_AFTERTOUCH) {
		notes_on.SetAftertouch(event);
	}

	if ((type != MIDI_STATUS_NOTE_ON || event->data[1]) && type != MIDI_STATUS_NOTE_OFF) {

		if (type == MIDI_STATUS_NOTE_ON) {

			if (!event->data[1]) return;

			notes_on.Remove(event);

			handler->sequenceSendEvent(sequence_id, event->status | (data << 8));
			event->ex.note_on.actualvel = event->data[1];

			notes_on.Push(event, ((event->ex.note_on.duration - 1) * 5000 / speed) + getStart());
		}
		// Only send IF the channel has been marked enabled
		else
			handler->sequenceSendEvent(sequence_id, event->status | (data << 8));
	}
}

#define SendController(ctrl,name) \
	handler->sequenceSendEvent(sequence_id, i | (MIDI_STATUS_CONTROLLER << 4) | (ctrl << 8) | (shadows[i].name[0] << 16)); \
	handler->sequenceSendEvent(sequence_id, i | (MIDI_STATUS_CONTROLLER << 4) | ((ctrl+32) << 8) | (shadows[i].name[1] << 16));

void XMidiSequence::applyShadow(int i) {
	// Pitch Wheel
	handler->sequenceSendEvent(sequence_id, i | (MIDI_STATUS_PITCH_WHEEL << 4) | (shadows[i].pitchWheel << 8));

	// Modulation Wheel
	SendController(1, modWheel);

	// Footpedal
	SendController(4, footpedal);

	// Volume
	handler->sequenceSendEvent(sequence_id, i | (MIDI_STATUS_CONTROLLER << 4) | (7 << 8) | (((shadows[i].volumes[0]*vol_multi) / 0xFF) << 16));
	handler->sequenceSendEvent(sequence_id, i | (MIDI_STATUS_CONTROLLER << 4) | (39 << 8) | (shadows[i].volumes[1] << 16));

	// Pan
	SendController(9, pan);

	// Balance
	SendController(10, balance);

	// expression
	SendController(11, expression);

	// Sustain
	handler->sequenceSendEvent(sequence_id, i | (MIDI_STATUS_CONTROLLER << 4) | (64 << 8) | (shadows[i].sustain << 16));

	// Effects (Reverb)
	handler->sequenceSendEvent(sequence_id, i | (MIDI_STATUS_CONTROLLER << 4) | (91 << 8) | (shadows[i].effects << 16));

	// Chorus
	handler->sequenceSendEvent(sequence_id, i | (MIDI_STATUS_CONTROLLER << 4) | (93 << 8) | (shadows[i].chorus << 16));

	// XMidi Bank
	handler->sequenceSendEvent(sequence_id, i | (MIDI_STATUS_CONTROLLER << 4) | (XMIDI_CONTROLLER_BANK_CHANGE << 8) | (shadows[i].xbank << 16));

	// Bank Select
	if (shadows[i].program != -1) handler->sequenceSendEvent(sequence_id, i | (MIDI_STATUS_PROG_CHANGE << 4) | (shadows[i].program << 8));
	SendController(0, bank);
}

void XMidiSequence::setVolume(int new_volume) {
	vol_multi = new_volume;
	new_volume = -1;

	// Only update used channels
	for (int i = 0; i < 16; i++) if (evntlist->chan_mask & (1 << i)) {
			uint32 message = i;
			message |= MIDI_STATUS_CONTROLLER << 4;
			message |= 7 << 8;
			message |= ((shadows[i].volumes[0] * vol_multi) / 0xFF) << 16;
			handler->sequenceSendEvent(sequence_id, message);
		}
}

void XMidiSequence::loseChannel(int i) {
	// If the channel matches, send a note off for any note
	XMidiEvent *note = notes_on.GetNotes();
	while (note) {
		if ((note->status & 0xF) == i)
			handler->sequenceSendEvent(sequence_id, note->status + (note->data[0] << 8));
		note = note->ex.note_on.next_note;
	}

}

void XMidiSequence::gainChannel(int i) {
	applyShadow(i);

	// If the channel matches, send a note on for any note
	XMidiEvent *note = notes_on.GetNotes();
	while (note) {
		if ((note->status & 0xF) == i)
			handler->sequenceSendEvent(sequence_id, note->status | (note->data[0] << 8) | (note->ex.note_on.actualvel << 16));
		note = note->ex.note_on.next_note;
	}
}

void XMidiSequence::pause() {
	paused = true;
	for (int i = 0; i < 16; i++)
		if (evntlist->chan_mask & (1 << i))
			loseChannel(i);
}

void XMidiSequence::unpause() {
	paused = false;
	for (int i = 0; i < 16; i++)
		if (evntlist->chan_mask & (1 << i))
			applyShadow(i);
}

int XMidiSequence::countNotesOn(int chan) {
	if (paused) return 0;

	int ret = 0;
	XMidiEvent *note = notes_on.GetNotes();
	while (note) {
		if ((note->status & 0xF) == chan)
			ret++;
		note = note->ex.note_on.next_note;
	}
	return ret;
}

} // End of namespace Ultima8
} // End of namespace Ultima
