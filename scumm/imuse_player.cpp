/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
#include "scumm/scumm.h"
#include "sound/mididrv.h"
#include "common/util.h"
#include "common/engine.h"
#include "imuse_internal.h"



////////////////////////////////////////
//
//  Helper functions
//
////////////////////////////////////////

static uint32 get_delta_time(byte **s) {
	byte *d = *s, b;
	uint32 time = 0;
	do {
		b = *d++;
		time = (time << 7) | (b & 0x7F);
	} while (b & 0x80);
	*s = d;
	return time;
}

static uint read_word(byte *a) {
	return (a[0] << 8) + a[1];
}

static void skip_midi_cmd(byte **song_ptr) {
	byte *s, code;

	const byte num_skip[] = {
		2, 2, 2, 2, 1, 1, 2
	};

	s = *song_ptr;

	code = *s++;

	if (code < 0x80) {
		s = NULL;
	} else if (code < 0xF0) {
		s += num_skip[(code & 0x70) >> 4];
	} else {
		if (code == 0xF0 || code == 0xF7 || code == 0xFF && *s++ != 0x2F) {
			s += get_delta_time(&s);
		} else {
			s = NULL;
		}
	}
	*song_ptr = s;
}

static int is_note_cmd(byte **a, IsNoteCmdData * isnote) {
	byte *s = *a;
	byte code;

	code = *s++;

	switch (code >> 4) {
	case 8: // Key Off
		isnote->chan = code & 0xF;
		isnote->note = *s++;
		isnote->vel = *s++;
		*a = s;
		return 1;
	case 9: // Key On
		isnote->chan = code & 0xF;
		isnote->note = *s++;
		isnote->vel = *s++;
		*a = s;
		if (isnote->vel)
			return 2;
		return 1;
	case 0xA:
	case 0xB:
	case 0xE:
		s++;
	case 0xC:
	case 0xD:
		s++;
		break;
	case 0xF:
		if (code == 0xF0 || code == 0xF7 || code == 0xFF && *s++ != 0x2F) {
			s += get_delta_time(&s);
			break;
		}
		return -1;
	default:
		return -1;
	}
	*a = s;
	return 0;
}



//////////////////////////////////////////////////
//
// IMuse Player implementation
//
//////////////////////////////////////////////////

bool Player::is_fading_out() {
	int i;
	for (i = 0; i < ARRAYSIZE(_parameterFaders); ++i) {
		if (_parameterFaders[i].param == ParameterFader::pfVolume &&
		    _parameterFaders[i].end == 0)
		{
			return true;
		}
	}
	return false;
}

void Player::clear() {
	uninit_seq();
	cancel_volume_fade();
	uninit_parts();
	_se->ImFireAllTriggers (_id);
	_active = false;
	_ticks_per_beat = TICKS_PER_BEAT;
	_midi = NULL;
}

bool Player::startSound (int sound, MidiDriver *midi) {
	void *mdhd;
	int i;

	mdhd = _se->findTag(sound, MDHD_TAG, 0);
	if (mdhd == NULL) {
		mdhd = _se->findTag(sound, MDPG_TAG, 0);
		if (mdhd == NULL) {
				warning("P::startSound failed: Couldn't find %s", MDHD_TAG);
				return false;
		}
	}

	_mt32emulate = _se->isMT32(sound);
	_isGM = _se->isGM(sound);

	_parts = NULL;
	_active = true;
	_midi = midi;
	_id = sound;
	_priority = 0x80;
	_volume = 0x7F;
	_vol_chan = 0xFFFF;
	_vol_eff = (_se->get_channel_volume(0xFFFF) << 7) >> 7;
	_pan = 0;
	_transpose = 0;
	_detune = 0;

	for (i = 0; i < ARRAYSIZE(_parameterFaders); ++i)
		_parameterFaders[i].init();

	hook_clear();
	if (start_seq_sound(sound) != 0) {
		_active = false;
		_midi = NULL;
		return false;
	}
	return true;
}

void Player::hook_clear() {
	memset(&_hook, 0, sizeof(_hook));
}

int Player::start_seq_sound(int sound) {
	byte *ptr, *track_ptr;

	_song_index = sound;
	_timer_counter = 0;
	_loop_to_beat = 1;
	_loop_from_beat = 1;
	_track_index = 0;
	_loop_counter = 0;
	_loop_to_tick = 0;
	_loop_from_tick = 0;

	set_tempo(500000);
	set_speed(128);
	ptr = _se->findTag(sound, "MTrk", _track_index);
	if (ptr == NULL)
		return -1;

	track_ptr = ptr;
	_cur_pos = _next_pos = get_delta_time(&track_ptr);
	_song_offset = track_ptr - ptr;

	_tick_index = _cur_pos;
	_beat_index = 1;

	if (_tick_index >= _ticks_per_beat) {
		_beat_index += _tick_index / _ticks_per_beat;
		_tick_index %= _ticks_per_beat;
	}

	return 0;
}

void Player::set_tempo(uint32 b) {
	uint32 i, j;

	i = _midi->getBaseTempo();

	j = _tempo = b;
	j = j * 100 / _se->_tempoFactor;

	while (i & 0xFFFF0000 || j & 0xFFFF0000) {
		i >>= 1;
		j >>= 1;
	}

	_tempo_eff = (i << 16) / j;

	set_speed(_speed);
}

void Player::cancel_volume_fade() {
	int i;
	for (i = 0; i < ARRAYSIZE(_parameterFaders); ++i) {
		if (_parameterFaders[i].param == ParameterFader::pfVolume) {
			_parameterFaders[i].param = 0;
			break;
		}
	}
}

void Player::uninit_parts() {
	if (_parts && _parts->_player != this)
		error("asd");
	while (_parts)
		_parts->uninit();

	// In case another player is waiting to allocate parts
	if (_midi)
		_se->reallocateMidiChannels (_midi);
}

void Player::uninit_seq() {
	_abort = true;
}

void Player::set_speed(byte speed) {
	_speed = speed;
	_timer_speed = (_tempo_eff * speed >> 7);
}

byte *Player::parse_midi(byte *s) {
	byte cmd, chan, note, velocity, control;
	uint value;
	Part *part;

	cmd = *s++;

	chan = cmd & 0xF;

	switch (cmd >> 4) {
	case 0x8: // Key Off
		note = *s++;
		if (!_scanning) {
			key_off(chan, note);
		} else {
			clear_active_note(chan, note);
		}
		s++; // Skip velocity
		break;

	case 0x9: // Key On
		note = *s++;
		velocity = *s++;
		if (velocity) {
			if (!_scanning)
				key_on(chan, note, velocity);
			else
				set_active_note(chan, note);
		} else {
			if (!_scanning)
				key_off(chan, note);
			else
				clear_active_note(chan, note);
		}
		break;

	case 0xA: // Aftertouch
		s += 2;
		break;

	case 0xB: // Control Change
		control = *s++;
		value = *s++;
		part = get_part(chan);
		if (!part)
			break;

		switch (control) {
		case 1: // Modulation Wheel
			part->set_modwheel(value);
			break;
		case 7: // Volume
			part->set_vol(value);
			break;
		case 10: // Pan Position
			part->set_pan(value - 0x40);
			break;
		case 16: // Pitchbend Factor (non-standard)
			part->set_pitchbend_factor(value);
			break;
		case 17: // GP Slider 2
			part->set_detune(value - 0x40);
			break;
		case 18: // GP Slider 3
			part->set_pri(value - 0x40);
			_se->reallocateMidiChannels (_midi);
			break;
		case 64: // Sustain Pedal
			part->set_pedal(value != 0);
			break;
		case 91: // Effects Level
			part->set_effect_level(value);
			break;
		case 93: // Chorus Level
			part->set_chorus(value);
			break;
		default:
			warning("parse_midi: invalid control %d", control);
		}
		break;

	case 0xC: // Program Change
		value = *s++;
		part = get_part(chan);
		if (part) {
			if (_isGM) {
				if (value < 128)
					part->set_program(value);
			} else {
				if (value < 32)
					part->load_global_instrument(value);
			}
		}
		break;

	case 0xD: // Channel Pressure
		s++;
		break;

	case 0xE: // Pitch Bend
		part = get_part(chan);
		if (part)
			part->set_pitchbend(((s[1] << 7) | s[0]) - 0x2000);
		s += 2;
		break;

	case 0xF:
		if (chan == 0) {
			uint size = get_delta_time(&s);
			parse_sysex(s, size);
			s += size;
		} else if (chan == 0xF) {
			cmd = *s++;
			if (cmd == 47)
				goto Error; // End of song
			if (cmd == 81) {
				set_tempo((s[1] << 16) | (s[2] << 8) | s[3]);
				s += 4;
				break;
			}
			s += get_delta_time(&s);
		} else if (chan == 0x7) {
			s += get_delta_time(&s);
		} else {
			goto Error;
		}
		break;

	default:
	Error:;
		if (!_scanning)
			clear();
		return NULL;
	}
	return s;
}

void Player::parse_sysex(byte *p, uint len) {
	byte code;
	byte a;
	uint b;
	byte buf[128];
	Part *part;

	// Check SysEx manufacturer.
	// Roland is 0x41
	a = *p++;
	--len;
	if (a != IMUSE_SYSEX_ID) {
		if (a == ROLAND_SYSEX_ID) {
			// Roland custom instrument definition.
			part = get_part (p[0] & 0x0F);
			if (part) {
				part->_instrument.roland (p - 1);
				if (part->clearToTransmit())
					part->_instrument.send (part->_mc);
			}
		} else {
			warning ("Unknown SysEx manufacturer 0x%02X", (int) a);
		}
		return;
	}
	--len;

	// Too big?
	if (len >= sizeof(buf) * 2)
		return;

#ifdef IMUSE_DEBUG
	for (a = 0; a < len + 1 && a < 19; ++a) {
		sprintf ((char *)&buf[a*3], " %02X", p[a]);
	} // next for
	if (a < len + 1) {
		buf[a*3] = buf[a*3+1] = buf[a*3+2] = '.';
		++a;
	} // end if
	buf[a*3] = '\0';
	debug (0, "[%02d] SysEx:%s", _id, buf);
#endif

	switch (code = *p++) {
	case 0:
		if (g_scumm->_gameId != GID_SAMNMAX) {
			// There are 17 bytes of useful information beyond
			// what we've read so far. All we know about them is
			// as follows:
			//   BYTE 00: Channel #
			//   BYTE 02: BIT 01 (0x01): Part on? (1 = yes)
			//   BYTE 05: Volume (upper 4 bits) [guessing]
			//   BYTE 06: Volume (lower 4 bits) [guessing]
			//   BYTE 09: BIT 04 (0x08): Percussion? (1 = yes)
			//   BYTE 15: Program (upper 4 bits)
			//   BYTE 16: Program (lower 4 bits)
			part = get_part (p[0] & 0x0F);
			if (part) {
				part->set_onoff (p[2] & 0x01);
				part->set_vol ((p[5] & 0x0F) << 4 | (p[6] & 0x0F));
				part->_percussion = _isGM ? ((p[9] & 0x08) > 0) : false;
				if (part->_percussion) {
					if (part->_mc) {
						part->off();
						_se->reallocateMidiChannels (_midi);
					}
				} else {
					// Even in cases where a program does not seem to be specified,
					// i.e. bytes 15 and 16 are 0, we send a program change because
					// 0 is a valid program number. MI2 tests show that in such
					// cases, a regular program change message always seems to follow
					// anyway.
					if (_isGM)
						part->_instrument.program ((p[15] & 0x0F) << 4 | (p[16] & 0x0F), _mt32emulate);
					part->sendAll();
				}
			}
		} else {
			// Sam & Max: Trigger Event
			// Triggers are set by doCommand (ImSetTrigger).
			// When a SysEx marker is encountered whose sound
			// ID and marker ID match what was set by ImSetTrigger,
			// something magical is supposed to happen....
			for (a = 0; a < 16; ++a) {
				if (_se->_snm_triggers [a].sound == _id &&
				    _se->_snm_triggers [a].id == *p)
				{
					_se->_snm_triggers [a].sound = _se->_snm_triggers [a].id = 0;
					_se->doCommand (_se->_snm_triggers [a].command [0],
					                 _se->_snm_triggers [a].command [1],
					                 _se->_snm_triggers [a].command [2],
					                 _se->_snm_triggers [a].command [3],
					                 0, 0, 0, 0);
					break;
				}
			}
		} // end if
		break;

	case 1:
		// This SysEx is used in Sam & Max for maybe_jump.
		if (_scanning)
			break;
		maybe_jump (p[0], p[1] - 1, (read_word (p + 2) - 1) * 4 + p[4], ((p[5] * _ticks_per_beat) >> 2) + p[6]);
		break;

	case 2: // Start of song. Ignore for now.
		break;

	case 16: // Adlib instrument definition (Part)
		a = *p++ & 0x0F;
		++p; // Skip hardware type
		part = get_part(a);
		if (part) {
			if (len == 63) {
				decode_sysex_bytes(p, buf, len - 3);
				part->set_instrument((byte *) buf);
			} else {
				// SPK tracks have len == 49 here, and are not supported
				part->set_program (254); // Must be invalid, but not 255 (which is reserved)
			}
		}
		break;

	case 17: // Adlib instrument definition (Global)
		p += 2; // Skip hardware type and... whatever came right before it
		a = *p++;
		decode_sysex_bytes(p, buf, len - 4);
		_se->setGlobalAdlibInstrument (a, buf);
		break;

	case 33: // Parameter adjust
		a = *p++ & 0x0F;
		++p; // Skip hardware type
		decode_sysex_bytes(p, buf, len - 3);
		part = get_part(a);
		if (part)
			part->set_param(read_word(buf), read_word(buf + 2));
		break;

	case 48: // Hook - jump
		if (_scanning)
			break;
		decode_sysex_bytes(p + 1, buf, len - 2);
		maybe_jump (buf[0], read_word (buf + 1), read_word (buf + 3), read_word (buf + 5));
		break;

	case 49: // Hook - global transpose
		decode_sysex_bytes(p + 1, buf, len - 2);
		maybe_set_transpose(buf);
		break;

	case 50: // Hook - part on/off
		buf[0] = *p++ & 0x0F;
		decode_sysex_bytes(p, buf + 1, len - 2);
		maybe_part_onoff(buf);
		break;

	case 51: // Hook - set volume
		buf[0] = *p++ & 0x0F;
		decode_sysex_bytes(p, buf + 1, len - 2);
		maybe_set_volume(buf);
		break;

	case 52: // Hook - set program
		buf[0] = *p++ & 0x0F;
		decode_sysex_bytes(p, buf + 1, len - 2);
		maybe_set_program(buf);
		break;

	case 53: // Hook - set transpose
		buf[0] = *p++ & 0x0F;
		decode_sysex_bytes(p, buf + 1, len - 2);
		maybe_set_transpose_part(buf);
		break;

	case 64: // Marker
		p++;
		len -= 2;
		while (len--) {
			_se->handle_marker(_id, *p++);
		}
		break;

	case 80: // Loop
		decode_sysex_bytes(p + 1, buf, len - 2);
		set_loop(read_word(buf),
						 read_word(buf + 2), read_word(buf + 4), read_word(buf + 6), read_word(buf + 8)
			);
		break;

	case 81: // End loop
		clear_loop();
		break;

	case 96: // Set instrument
		part = get_part(p[0] & 0x0F);
		b = (p[1] & 0x0F) << 12 | (p[2] & 0x0F) << 8 | (p[4] & 0x0F) << 4 | (p[4] & 0x0F);
		if (part)
			part->set_instrument(b);
		break;

	default:
		warning ("Unknown SysEx command %d", (int) code);
	}
}

void Player::decode_sysex_bytes(byte *src, byte *dst, int len) {
	while (len >= 0) {
		*dst++ = (src[0] << 4) | (src[1] & 0xF);
		src += 2;
		len -= 2;
	}
}

void Player::maybe_jump (byte cmd, uint track, uint beat, uint tick) {
	// Is this the hook I'm waiting for?
	if (cmd && _hook._jump[0] != cmd)
		return;

	// Reset hook?
	if (cmd != 0 && cmd < 0x80) {
		_hook._jump[0] = _hook._jump[1];
		_hook._jump[1] = 0;
	}

	jump (track, beat, tick);
}

void Player::maybe_set_transpose(byte *data) {
	byte cmd;

	cmd = data[0];

	// Is this the hook I'm waiting for?
	if (cmd && _hook._transpose != cmd)
		return;

	// Reset hook?
	if (cmd != 0 && cmd < 0x80)
		_hook._transpose = 0;

	set_transpose(data[1], (int8)data[2]);
}

void Player::maybe_part_onoff(byte *data) {
	byte cmd, *p;
	uint chan;
	Part *part;

	cmd = data[1];
	chan = data[0];

	p = &_hook._part_onoff[chan];

	// Is this the hook I'm waiting for?
	if (cmd && *p != cmd)
		return;

	if (cmd != 0 && cmd < 0x80)
		*p = 0;

	part = get_part(chan);
	if (part)
		part->set_onoff(data[2] != 0);
}

void Player::maybe_set_volume(byte *data) {
	byte cmd;
	byte *p;
	uint chan;
	Part *part;

	cmd = data[1];
	chan = data[0];

	p = &_hook._part_volume[chan];

	// Is this the hook I'm waiting for?
	if (cmd && *p != cmd)
		return;

	// Reset hook?
	if (cmd != 0 && cmd < 0x80)
		*p = 0;

	part = get_part(chan);
	if (part)
		part->set_vol(data[2]);
}

void Player::maybe_set_program(byte *data) {
	byte cmd;
	byte *p;
	uint chan;
	Part *part;

	cmd = data[1];
	chan = data[0];

	// Is this the hook I'm waiting for?
	p = &_hook._part_program[chan];

	if (cmd && *p != cmd)
		return;

	if (cmd != 0 && cmd < 0x80)
		*p = 0;

	part = get_part(chan);
	if (part)
		part->set_program(data[2]);
}

void Player::maybe_set_transpose_part(byte *data) {
	byte cmd;
	byte *p;
	uint chan;

	cmd = data[1];
	chan = data[0];

	// Is this the hook I'm waiting for?
	p = &_hook._part_transpose[chan];

	if (cmd && *p != cmd)
		return;

	// Reset hook?
	if (cmd != 0 && cmd < 0x80)
		*p = 0;

	part_set_transpose(chan, data[2], (int8)data[3]);
}

int Player::set_transpose(byte relative, int b) {
	Part *part;

	if (b > 24 || b < -24 || relative > 1)
		return -1;
	if (relative)
		b = transpose_clamp(_transpose + b, -7, 7);

	_transpose = b;

	for (part = _parts; part; part = part->_next) {
		part->set_transpose(part->_transpose);
	}

	return 0;
}

void Player::clear_active_notes() {
	memset(_se->_active_notes, 0, sizeof(_se->_active_notes));
}

void Player::clear_active_note(int chan, byte note) {
	_se->_active_notes[note] &= ~(1 << chan);
}

void Player::set_active_note(int chan, byte note) {
	_se->_active_notes[note] |= (1 << chan);
}

void Player::part_set_transpose(uint8 chan, byte relative, int8 b) {
	Part *part;

	if (b > 24 || b < -24)
		return;

	part = get_part(chan);
	if (!part)
		return;
	if (relative)
		b = transpose_clamp(b + part->_transpose, -7, 7);
	part->set_transpose(b);
}

void Player::key_on(uint8 chan, uint8 note, uint8 velocity) {
	Part *part;

	part = get_part(chan);
	if (!part || !part->_on)
		return;

	part->key_on(note, velocity);
}

void Player::key_off(uint8 chan, uint8 note) {
	Part *part;

	for (part = _parts; part; part = part->_next) {
		if (part->_chan == (byte)chan && part->_on)
			part->key_off(note);
	}
}

bool Player::jump(uint track, uint beat, uint tick) {
	byte *mtrk, *cur_mtrk, *scanpos;
	uint32 topos, curpos, track_offs;

	if (!_active)
		return false;

	mtrk = _se->findTag(_song_index, "MTrk", track);
	if (!mtrk)
		return false;

	cur_mtrk = _se->findTag(_song_index, "MTrk", _track_index);
	if (!cur_mtrk)
		return false;

	if (beat == 0)
		beat = 1;

	topos = (beat - 1) * _ticks_per_beat + tick;

	if (track == _track_index && topos >= _next_pos) {
		scanpos = _song_offset + mtrk;
		curpos = _next_pos;
	} else {
		scanpos = mtrk;
		curpos = get_delta_time(&scanpos);
	}

	while (curpos < topos) {
		skip_midi_cmd(&scanpos);
		if (!scanpos)
			return false;
		curpos += get_delta_time(&scanpos);
	}

	track_offs = scanpos - mtrk;

	turn_off_pedals();

	find_sustaining_notes(cur_mtrk + _song_offset, mtrk + track_offs, curpos - topos);

	_beat_index = beat;
	_tick_index = tick;
	_cur_pos = topos;
	_next_pos = curpos;
	_timer_counter = 0;
	_song_offset = track_offs;
	if (track != _track_index) {
		_track_index = track;
		_loop_counter = 0;
	}
	_abort = true;
	return true;
}

bool Player::set_loop(uint count, uint tobeat, uint totick, uint frombeat, uint fromtick) {
	if (tobeat + 1 >= frombeat)
		return false;

	if (tobeat == 0)
		tobeat = 1;

	_loop_counter = 0; // Because of possible interrupts
	_loop_to_beat = tobeat;
	_loop_to_tick = totick;
	_loop_from_beat = frombeat;
	_loop_from_tick = fromtick;
	_loop_counter = count;

	return true;
}

void Player::clear_loop() {
	_loop_counter = 0;
}

void Player::turn_off_pedals() {
	Part *part;

	for (part = _parts; part; part = part->_next) {
		if (part->_pedal)
			part->set_pedal(false);
	}
}

void Player::find_sustaining_notes(byte *a, byte *b, uint32 l) {
	uint32 pos;
	uint16 mask;
	uint16 *bitlist_ptr;
	SustainingNotes *sn, *next;
	IsNoteCmdData isnote;
	int j;
	uint num_active;
	uint max_off_pos;

	num_active = update_actives();

	// pos contains number of ticks since current position
	pos = _next_pos - _cur_pos;
	if ((int32)pos < 0)
		pos = 0;

	// Locate the positions where the notes are turned off.
	// Remember each note that was turned off.
	while (num_active != 0) {
		// Is note off?
		j = is_note_cmd(&a, &isnote);
		if (j == -1)
			break;
		if (j == 1) {
			mask = 1 << isnote.chan;
			bitlist_ptr = _se->_active_notes + isnote.note;
			if (*bitlist_ptr & mask) {
				*bitlist_ptr &= ~mask;
				num_active--;
				// Get a node from the free list
				if ((sn = _se->_sustain_notes_free) == NULL)
					return;
				_se->_sustain_notes_free = sn->next;

				// Insert it in the beginning of the used list
				sn->next = _se->_sustain_notes_used;
				_se->_sustain_notes_used = sn;
				sn->prev = NULL;
				if (sn->next)
					sn->next->prev = sn;

				sn->note = isnote.note;
				sn->chan = isnote.chan;
				sn->player = this;
				sn->off_pos = pos;
				sn->pos = 0;
				sn->counter = 0;
			}
		}
		pos += get_delta_time(&a);
	}

	// Find the maximum position where a note was turned off
	max_off_pos = 0;
	for (sn = _se->_sustain_notes_used; sn; sn = sn->next) {
		_se->_active_notes[sn->note] |= (1 << sn->chan);
		if (sn->off_pos > max_off_pos) {
			max_off_pos = sn->off_pos;
		}
	}

	// locate positions where notes are turned on
	pos = l;
	while (pos < max_off_pos) {
		j = is_note_cmd(&b, &isnote);
		if (j == -1)
			break;
		if (j == 2) {
			mask = 1 << isnote.chan;
			bitlist_ptr = _se->_active_notes + isnote.note;

			if (*bitlist_ptr & mask) {
				sn = _se->_sustain_notes_used;
				while (sn) {
					next = sn->next;
					if (sn->note == isnote.note && sn->chan == isnote.chan && pos < sn->off_pos) {
						*bitlist_ptr &= ~mask;
						// Unlink from the sustain list
						if (next)
							next->prev = sn->prev;
						if (sn->prev)
							sn->prev->next = next;
						else
							_se->_sustain_notes_used = next;
						// Insert into the free list
						sn->next = _se->_sustain_notes_free;
						_se->_sustain_notes_free = sn;
					}
					sn = next;
				}
			}
		}
		pos += get_delta_time(&b);
	}

	// Concatenate head and used list
	if (!_se->_sustain_notes_head) {
		_se->_sustain_notes_head = _se->_sustain_notes_used;
		_se->_sustain_notes_used = NULL;
		return;
	}
	sn = _se->_sustain_notes_head;
	while (sn->next)
		sn = sn->next;
	sn->next = _se->_sustain_notes_used;
	_se->_sustain_notes_used = NULL;
	if (sn->next)
		sn->next->prev = sn;
}

Part *Player::get_part(uint8 chan) {
	Part *part;

	part = _parts;
	while (part) {
		if (part->_chan == chan)
			return part;
		part = part->_next;
	}

	part = _se->allocate_part (_priority, _midi);
	if (!part) {
		warning("no parts available");
		return NULL;
	}

	part->_chan = chan;
	part->setup(this);

	return part;
}

uint Player::update_actives() {
	Part *part;
	uint16 *active;
	int count = 0;

	clear_active_notes();
	active = _se->_active_notes;
	for (part = _parts; part; part = part->_next) {
		if (part->_mc)
			count += part->update_actives(active);
	}
	return count;
}

void Player::set_priority(int pri) {
	Part *part;

	_priority = pri;
	for (part = _parts; part; part = part->_next) {
		part->set_pri(part->_pri);
	}
	_se->reallocateMidiChannels (_midi);
}

void Player::set_pan(int pan) {
	Part *part;

	_pan = pan;
	for (part = _parts; part; part = part->_next) {
		part->set_pan(part->_pan);
	}
}

void Player::set_detune(int detune) {
	Part *part;

	_detune = detune;
	for (part = _parts; part; part = part->_next) {
		part->set_detune(part->_detune);
	}
}

int Player::scan(uint totrack, uint tobeat, uint totick) {
	byte *mtrk, *scanptr;
	uint32 curpos, topos;
	uint32 pos;

	if (!_active)
		return -1;

	mtrk = _se->findTag(_song_index, "MTrk", totrack);
	if (!mtrk)
		return -1;

	if (tobeat == 0)
		tobeat++;

	turn_off_parts();
	clear_active_notes();
	scanptr = mtrk;
	curpos = get_delta_time(&scanptr);
	_scanning = true;

	topos = (tobeat - 1) * _ticks_per_beat + totick;

	while (curpos < topos) {
		scanptr = parse_midi(scanptr);
		if (!scanptr) {
			_scanning = false;
			return -1;
		}
		curpos += get_delta_time(&scanptr);
	}
	pos = scanptr - mtrk;

	_scanning = false;
	_se->reallocateMidiChannels (_midi);
	play_active_notes();
	_beat_index = tobeat;
	_tick_index = totick;
	_cur_pos = topos;
	_next_pos = curpos;
	_timer_counter = 0;
	_song_offset = pos;
	if (_track_index != totrack) {
		_track_index = totrack;
		_loop_counter = 0;
	}
	return 0;
}

void Player::turn_off_parts() {
	Part *part;

	for (part = _parts; part; part = part->_next)
		part->off();
	_se->reallocateMidiChannels (_midi);
}

void Player::play_active_notes() {
	int i, j;
	uint mask;

	for (i = 0; i != 128; i++) {
		mask = _se->_active_notes[i];
		for (j = 0; j != 16; j++, mask >>= 1) {
			if (mask & 1) {
				key_on(j, i, 80);
			}
		}
	}
}

int Player::set_vol(byte vol) {
	Part *part;

	if (vol > 127)
		return -1;

	_volume = vol;
	_vol_eff = _se->get_channel_volume(_vol_chan) * (vol + 1) >> 7;

	for (part = _parts; part; part = part->_next) {
		part->set_vol(part->_vol);
	}

	return 0;
}

int Player::get_param(int param, byte chan) {
	switch (param) {
	case 0:
		return (byte)_priority;
	case 1:
		return (byte)_volume;
	case 2:
		return (byte)_pan;
	case 3:
		return (byte)_transpose;
	case 4:
		return (byte)_detune;
	case 5:
		return _speed;
	case 6:
		return _track_index;
	case 7:
		return _beat_index;
	case 8:
		return _tick_index;
	case 9:
		return _loop_counter;
	case 10:
		return _loop_to_beat;
	case 11:
		return _loop_to_tick;
	case 12:
		return _loop_from_beat;
	case 13:
		return _loop_from_tick;
	case 14:
	case 15:
	case 16:
	case 17:
		return query_part_param(param, chan);
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
		return _hook.query_param(param, chan);
	default:
		return -1;
	}
}

int Player::query_part_param(int param, byte chan) {
	Part *part;

	part = _parts;
	while (part) {
		if (part->_chan == chan) {
			switch (param) {
			case 14:
				return part->_on;
			case 15:
				return part->_vol;
			case 16:
				return (int) part->_instrument;
			case 17:
				return part->_transpose;
			default:
				return -1;
			}
		}
		part = part->_next;
	}
	return 129;
}

void Player::sequencer_timer() {
	byte *mtrk;
	uint32 counter;
	byte *song_ptr;

	// First handle any parameter transitions
	// that are occuring.
	transitionParameters();

	// Since the volume parameter can cause
	// the player to be deactivated, check
	// to make sure we're still active.
	if (!_active)
		return;

	counter = _timer_counter + _timer_speed;
	_timer_counter = counter & 0xFFFF;
	_cur_pos += counter >> 16;
	_tick_index += counter >> 16;

	if (_tick_index >= _ticks_per_beat) {
		_beat_index += _tick_index / _ticks_per_beat;
		_tick_index %= _ticks_per_beat;
	}
	if (_loop_counter && _beat_index >= _loop_from_beat && _tick_index >= _loop_from_tick) {
		_loop_counter--;
		jump(_track_index, _loop_to_beat, _loop_to_tick);
	}
	if (_next_pos <= _cur_pos) {
		mtrk = _se->findTag(_song_index, "MTrk", _track_index);
		if (!mtrk) {
			warning("Sound %d was unloaded while active", _song_index);
			clear();
		} else {
			song_ptr = mtrk + _song_offset;
			_abort = false;

			while (_next_pos <= _cur_pos) {
				song_ptr = parse_midi(song_ptr);
				if (!song_ptr || _abort)
					return;
				_next_pos += get_delta_time(&song_ptr);
				_song_offset = song_ptr - mtrk;
			}
		}
	}
}

// "time" is referenced as hundredths of a second.
// IS THAT CORRECT??
// We convert it to microseconds before prceeding
int Player::addParameterFader (int param, int target, int time) {
	int start;

	switch (param) {
	case ParameterFader::pfVolume:
		// Volume fades are handled differently.
		start = _volume;
		break;

	case ParameterFader::pfTranspose:
		// FIXME: Is this transpose? And what's the scale?
		// It's set to fade to -2400 in the tunnel of love.
		warning ("parameterTransition(3) outside Tunnel of Love?");
		start = _transpose;
		target /= 200;
		break;

	case ParameterFader::pfSpeed:
		// FIXME: Is the speed from 0-100?
		// Right now I convert it to 0-128.
		start = _speed;
		target = target * 128 / 100;
		break;

	case 127:
		// FIXME: This MIGHT fade ALL supported parameters,
		// but I'm not sure.
		return 0;

	default:
		warning ("Player::addParameterFader(): Unknown parameter %d", param);
		return 0; // Should be -1, but we'll let the script think it worked.
	}

	ParameterFader *ptr = &_parameterFaders[0];
	ParameterFader *best = 0;
	int i;
	for (i = ARRAYSIZE(_parameterFaders); i; --i, ++ptr) {
		if (ptr->param == param) {
			best = ptr;
			start = ptr->end;
			break;
		} else if (!ptr->param) {
			best = ptr;
		}
	}

	if (best) {
		best->param = param;
		best->start = start;
		best->end = target;
		best->total_time = (uint32) time * 10000;
		best->current_time = 0;
	} else {
		warning ("IMuse Player %d: Out of parameter faders", _id);
		return -1;
	}

	return 0;
}

void Player::transitionParameters() {
	uint32 advance = _midi->getBaseTempo() / 500;
	int value;

	ParameterFader *ptr = &_parameterFaders[0];
	int i;
	for (i = ARRAYSIZE(_parameterFaders); i; --i, ++ptr) {
		if (!ptr->param)
			continue;

		ptr->current_time += advance;
		if (ptr->current_time > ptr->total_time)
			ptr->current_time = ptr->total_time;
		value = (int32) ptr->start + (int32) (ptr->end - ptr->start) * (int32) ptr->current_time / (int32) ptr->total_time;

		switch (ptr->param) {
		case ParameterFader::pfVolume:
			// Volume.
			if (!value) {
				clear();
				return;
			}
			set_vol ((byte) value);
			break;

		case ParameterFader::pfSpeed:
			// Speed.
			set_speed ((byte) value);
			break;

		case ParameterFader::pfTranspose:
			// FIXME: Is this really transpose?
			set_transpose (0, value);
			break;
		}

		if (ptr->current_time >= ptr->total_time)
			ptr->param = 0;
	}
}
