/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "scumm.h"
#include "gmidi.h"

void MidiSoundDriver::midiSetDriver(MidiDriver *driver)
{
	_md = driver;

	int result = _md->open(MidiDriver::MO_SIMPLE);
	if (result)
		error("MidiSoundDriver::error = %s", MidiDriver::get_error_name(result));
}

/****************** Common midi code (SCUMM specific) *****************/
void MidiSoundDriver::midiPitchBend(byte chan, int16 pitchbend)
{
	uint16 tmp;

	if (_midi_pitchbend_last[chan] != pitchbend) {
		_midi_pitchbend_last[chan] = pitchbend;
		tmp = (pitchbend << 2) + 0x2000;
		_md->send(((tmp >> 7) & 0x7F) << 16 | (tmp & 0x7F) << 8 | 0xE0 | chan);
	}
}

void MidiSoundDriver::midiVolume(byte chan, byte volume)
{
	if (_midi_volume_last[chan] != volume) {
		_midi_volume_last[chan] = volume;
		_md->send(volume << 16 | 7 << 8 | 0xB0 | chan);
	}
}
void MidiSoundDriver::midiPedal(byte chan, bool pedal)
{
	if (_midi_pedal_last[chan] != pedal) {
		_midi_pedal_last[chan] = pedal;
		_md->send(pedal << 16 | 64 << 8 | 0xB0 | chan);
	}
}

void MidiSoundDriver::midiModWheel(byte chan, byte modwheel)
{
	if (_midi_modwheel_last[chan] != modwheel) {
		_midi_modwheel_last[chan] = modwheel;
		_md->send(modwheel << 16 | 1 << 8 | 0xB0 | chan);
	}
}

void MidiSoundDriver::midiEffectLevel(byte chan, byte level)
{
	if (_midi_effectlevel_last[chan] != level) {
		_midi_effectlevel_last[chan] = level;
		_md->send(level << 16 | 91 << 8 | 0xB0 | chan);
	}
}

void MidiSoundDriver::midiChorus(byte chan, byte chorus)
{
	if (_midi_chorus_last[chan] != chorus) {
		_midi_chorus_last[chan] = chorus;
		_md->send(chorus << 16 | 93 << 8 | 0xB0 | chan);
	}
}

void MidiSoundDriver::midiControl0(byte chan, byte value)
{
	_md->send(value << 16 | 0 << 8 | 0xB0 | chan);
}

void MidiSoundDriver::midiProgram(byte chan, byte program)
{
	if ((chan + 1) != 10) {				/* Ignore percussion prededed by patch change */
		if (_se->_mt32emulate)
			program = mt32_to_gmidi[program];

		_md->send(program << 8 | 0xC0 | chan);
	}
}

void MidiSoundDriver::midiPan(byte chan, int8 pan)
{
	if (_midi_pan_last[chan] != pan) {
		_midi_pan_last[chan] = pan;
		_md->send(((pan - 64) & 0x7F) << 16 | 10 << 8 | 0xB0 | chan);
	}
}

void MidiSoundDriver::midiNoteOn(byte chan, byte note, byte velocity)
{
	_md->send(velocity << 16 | note << 8 | 0x90 | chan);
}

void MidiSoundDriver::midiNoteOff(byte chan, byte note)
{
	_md->send(note << 8 | 0x80 | chan);
}

void MidiSoundDriver::midiSilence(byte chan)
{
	_md->send((64 << 8) | 0xB0 | chan);
	_md->send((123 << 8) | 0xB0 | chan);
}


void MidiSoundDriver::part_key_on(Part *part, byte note, byte velocity)
{
	MidiChannelGM *mc = part->_mc->gm();

	if (mc) {
		mc->_actives[note >> 4] |= (1 << (note & 0xF));
		midiNoteOn(mc->_chan, note, velocity);
	} else if (part->_percussion) {
		midiVolume(SPECIAL_CHANNEL, part->_vol_eff);
		midiProgram(SPECIAL_CHANNEL, part->_bank);
		midiNoteOn(SPECIAL_CHANNEL, note, velocity);
	}
}

void MidiSoundDriver::part_key_off(Part *part, byte note)
{
	MidiChannelGM *mc = part->_mc->gm();

	if (mc) {
		mc->_actives[note >> 4] &= ~(1 << (note & 0xF));
		midiNoteOff(mc->_chan, note);
	} else if (part->_percussion) {
		midiNoteOff(SPECIAL_CHANNEL, note);
	}
}

int MidiSoundDriver::midi_driver_thread(void *param) {
	MidiSoundDriver *mid = (MidiSoundDriver*) param;
	int old_time, cur_time;

	old_time = mid->_system->get_msecs();

	for(;;) {
		mid->_system->delay_msecs(10);

		cur_time = mid->_system->get_msecs();
		while (old_time < cur_time) {
			old_time += 10;
			mid->_se->on_timer();
		}
	}
}

void MidiSoundDriver::init(SoundEngine *eng, OSystem *syst)
{
	int i;
	MidiChannelGM *mc;

	_system = syst;

	/* Install the on_timer thread */
	syst->create_thread(midi_driver_thread, this);
	_se = eng;

	for (i = 0, mc = _midi_channels; i != ARRAYSIZE(_midi_channels); i++, mc++)
		mc->_chan = i;
}

void MidiSoundDriver::update_pris()
{
	Part *part, *hipart;
	int i;
	byte hipri, lopri;
	MidiChannelGM *mc, *lomc;

	while (true) {
		hipri = 0;
		hipart = NULL;
		for (i = 32, part = _se->parts_ptr(); i; i--, part++) {
			if (part->_player && !part->_percussion && part->_on && !part->_mc
					&& part->_pri_eff >= hipri) {
				hipri = part->_pri_eff;
				hipart = part;
			}
		}

		if (!hipart)
			return;

		lopri = 255;
		lomc = NULL;
		for (i = ARRAYSIZE(_midi_channels), mc = _midi_channels;; mc++) {
			if (!mc->_part) {
				lomc = mc;
				break;
			}
			if (mc->_part->_pri_eff <= lopri) {
				lopri = mc->_part->_pri_eff;
				lomc = mc;
			}

			if (!--i) {
				if (lopri >= hipri)
					return;
				lomc->_part->off();
				break;
			}
		}

		hipart->_mc = lomc;
		lomc->_part = hipart;
		hipart->changed(pcAll);
	}
}

int MidiSoundDriver::part_update_active(Part *part, uint16 *active)
{
	int i, j;
	uint16 *act, mask, bits;
	int count = 0;

	bits = 1 << part->_chan;

	act = part->_mc->gm()->_actives;

	for (i = 8; i; i--) {
		mask = *act++;
		if (mask) {
			for (j = 16; j; j--, mask >>= 1, active++) {
				if (mask & 1 && !(*active & bits)) {
					*active |= bits;
					count++;
				}
			}
		} else {
			active += 16;
		}
	}
	return count;
}

void MidiSoundDriver::part_changed(Part *part, byte what)
{
	MidiChannelGM *mc;

	/* Mark for re-schedule if program changed when in pre-state */
	if (what & pcProgram && part->_percussion) {
		part->_percussion = false;
		update_pris();
	}

	if (!(mc = part->_mc->gm()))
		return;

	if (what & pcMod)
		midiPitchBend(mc->_chan,
									clamp(part->_pitchbend + part->_detune_eff +
												(part->_transpose_eff << 7), -2048, 2047));

	if (what & pcVolume)
		midiVolume(mc->_chan, part->_vol_eff);

	if (what & pcPedal)
		midiPedal(mc->_chan, part->_pedal);

	if (what & pcModwheel)
		midiModWheel(mc->_chan, part->_modwheel);

	if (what & pcPan)
		midiPan(mc->_chan, part->_pan_eff);

	if (what & pcEffectLevel)
		midiEffectLevel(mc->_chan, part->_effect_level);

	if (what & pcProgram) {
		if (part->_bank) {
			midiControl0(mc->_chan, part->_bank);
			midiProgram(mc->_chan, part->_program);
			midiControl0(mc->_chan, 0);
		} else {
			midiProgram(mc->_chan, part->_program);
		}
	}

	if (what & pcChorus)
		midiChorus(mc->_chan, part->_effect_level);
}


void MidiSoundDriver::part_off(Part *part)
{
	MidiChannelGM *mc = part->_mc->gm();
	if (mc) {
		part->_mc = NULL;
		mc->_part = NULL;
		memset(mc->_actives, 0, sizeof(mc->_actives));
		midiSilence(mc->_chan);
	}
}
