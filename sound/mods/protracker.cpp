/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 * Based on code by madmoose
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

#include "common/stdafx.h"

#include "sound/mods/protracker.h"
#include "sound/mods/paula.h"
#include "sound/mods/module.h"

#include "sound/audiostream.h"

namespace Modules {

class ProtrackerStream : public ::Audio::Paula {
private:
	Module *_module;

	int _rate;

	int _tick;
	int _row;
	int _pos;

	int _patternDelay;

	int _speed;
	int _bpm;

	// For effect 0xB - Jump To Pattern;
	bool _hasJumpToPattern;
	int _jumpToPattern;

	// For effect 0xD - PatternBreak;
	bool _hasPatternBreak;
	int _skiprow;

	// For effect 0xE6 - Pattern Loop
	bool _hasPatternLoop;
	int _patternLoopCount;
	int _patternLoopRow;

	struct {
		byte sample;
		uint16 period;
		double offset;

		byte vol;

		// For effect 0x3 - Porta to note
		uint16 portaToNote;
		byte portaToNoteSpeed;

		// For effect 0x4 - Vibrato
		int vibrato;
		byte vibratoPos;
		byte vibratoSpeed;
		byte vibratoDepth;
	} _track[4];

public:
	ProtrackerStream(Common::ReadStream *stream, int rate, bool stereo);

private:
	void startPlay() { _playing = true; _end = false; }
	void interrupt();

	void updateRow();
	void updateEffects();

};

ProtrackerStream::ProtrackerStream(Common::ReadStream *stream, int rate, bool stereo) :
		Paula(stereo, rate, rate/50) {
	_module = new Module();
	bool result = _module->load(*stream);
	assert(result);

	_rate = rate;
	_tick = _row = _pos = 0;
	_hasJumpToPattern = false;
	_hasPatternBreak = false;
	_hasPatternLoop = false;
	_patternDelay = 0;
	_patternLoopCount = 0;
	_patternLoopRow = 0;
	_speed = 6;
	_bpm = 125;

	for (int t = 0; t < 4; t++) {
		_track[t].sample = 0;
		_track[t].period = 0;
		_track[t].offset = 0.0;
		_track[t].vibrato = 0;
	}	

	startPlay();
}

void ProtrackerStream::updateRow() {
	for (int track = 0; track < 4; track++) {
		_track[track].vibrato = 0;
		note_t note =
		    _module->pattern[_module->songpos[_pos]][_row][track];

		int effect = note.effect >> 8;

		if (note.sample) {
			if (_track[track].sample != note.sample) {
				_track[track].vibratoPos = 0;
			}
			_track[track].sample = note.sample;
			_track[track].vol = _module->sample[note.sample - 1].vol;
		}
		if (note.period) {
			if (effect != 3 && effect != 5) {
				_track[track].period = note.period;
				_track[track].offset = 0.0;
			}
		}

		int exy = note.effect & 0xff;
		int ex = (note.effect >> 4) & 0xf;
		int ey = note.effect & 0xf;

		switch (effect) {
		case 0x0:
			break;
		case 0x1:
			break;
		case 0x2:
			break;
		case 0x3:
			if (note.period)
				_track[track].portaToNote = note.period;

			if (exy)
				_track[track].portaToNoteSpeed = exy;
			break;
		case 0x4:
			if (ex || ey) {
				_track[track].vibratoSpeed = ex;
				_track[track].vibratoDepth = ey;
			}
			break;
		case 0x5:
			break;
		case 0xA:
			break;
		case 0xB:
			_hasJumpToPattern = true;
			_jumpToPattern = exy;
			break;
		case 0xC:
			_track[track].vol = exy;
			break;
		case 0xD:
			_hasPatternBreak = true;
			_skiprow = ex * 10 + ey;
			break;

		case 0xE:
			switch (ex) {
			case 0x6:
				if (ey == 0) {
					_patternLoopRow = _row;
				} else {
					_patternLoopCount++;
					if (_patternLoopCount <= ey)
						_hasPatternLoop = true;
					else
						_patternLoopCount = 0;
				}
				break;
			case 0x9:
				break;	// Retrigger note
			default:
				warning("Unimplemented effect %X\n", note.effect);
			}
			break;

		case 0xF:
			if (exy < 0x20) {
				_speed = exy;
			} else {
				_bpm = exy;
				setInterruptFreq((int) (getRate() / (_bpm * 0.4)));
			}
			break;
		default:
			warning("Unimplemented effect %X\n", note.effect);
		}
	}
}

void ProtrackerStream::updateEffects() {

	static const int16 sinetable[64] = {
		   0,   24,   49,   74,   97,  120,  141,  161,
		 180,  197,  212,  224,  235,  244,  250,  253,
		 255,  253,  250,  244,  235,  224,  212,  197,
		 180,  161,  141,  120,   97,   74,   49,   24,
		   0,  -24,  -49,  -74,  -97, -120, -141, -161,
		-180, -197, -212, -224, -235, -244, -250, -253,
		-255, -253, -250, -244, -235, -224, -212, -197,
		-180, -161, -141, -120,  -97,  -74,  -49,  -24
	};

	for (int track = 0; track < 4; track++) {
		_track[track].vibrato = 0;

		note_t note =
		    _module->pattern[_module->songpos[_pos]][_row][track];

		int effect = note.effect >> 8;

		int exy = note.effect & 0xff;
		int ex = (note.effect >> 4) & 0xf;
		int ey = (note.effect) & 0xf;

		int vol;
		switch (effect) {
		case 0x0:
			break;
		case 0x1:
			_track[track].period -= exy;
			break;
		case 0x2:
			_track[track].period += exy;
			break;
		case 0x3:
			if (_track[track].portaToNote && _track[track].portaToNoteSpeed) {
				if (_track[track].period < _track[track].portaToNote) {
					_track[track].period += _track[track].portaToNoteSpeed;
					if (_track[track].period > _track[track].portaToNote)
						_track[track].period = _track[track].portaToNote;
				} else if (_track[track].period > _track[track].portaToNote) {
					_track[track].period -= _track[track].portaToNoteSpeed;
					if (_track[track].period < _track[track].portaToNote)
						_track[track].period = _track[track].portaToNote;
				}
			}
			break;
		case 0x4:
			_track[track].vibrato =
			    (_track[track].vibratoDepth * sinetable[_track[track].vibratoPos]) / 128;
			_track[track].vibratoPos += _track[track].vibratoSpeed;
			_track[track].vibratoPos %= 64;
			break;

		case 0x5:
			if (_track[track].portaToNote
			    && _track[track].portaToNoteSpeed) {
				if (_track[track].period < _track[track].portaToNote) {
					_track[track].period += _track[track].portaToNoteSpeed;
					if (_track[track].period > _track[track].portaToNote)
						_track[track].period = _track[track].portaToNote;
				} else if (_track[track].period > _track[track].portaToNote) {
					_track[track].period -= _track[track].portaToNoteSpeed;
					if (_track[track].period < _track[track].portaToNote)
						_track[track].period = _track[track].portaToNote;
				}
			}

			vol = _track[track].vol;
			if (ex == 0)
				vol -= ey;
			else if (ey == 0)
				vol += ex;

			if (vol < 0)
				vol = 0;
			else if (vol > 64)
				vol = 64;

			_track[track].vol = vol;

			break;

		case 0x6:
			_track[track].vibrato =
			    (_track[track].vibratoDepth * sinetable[_track[track].vibratoPos]) / 128;
			_track[track].vibratoPos += _track[track].vibratoSpeed;
			_track[track].vibratoPos %= 64;

			vol = _track[track].vol;
			if (ex == 0)
				vol -= ey;
			else if (ey == 0)
				vol += ex;

			if (vol < 0)
				vol = 0;
			else if (vol > 64)
				vol = 64;

			_track[track].vol = vol;

			break;

		case 0xA:
			vol = _track[track].vol;
			if (ex == 0)
				vol -= ey;
			else if (ey == 0)
				vol += ex;

			if (vol < 0)
				vol = 0;
			else if (vol > 64)
				vol = 64;

			_track[track].vol = vol;

			break;

		case 0xE:
			switch (ex) {
			case 0x6:
				break;	// Pattern loop
			case 0x9:	// Retrigger note
				if (ey && _tick % ey == 0)
					_track[track].offset = 0.0;
				break;
			}
			break;
		}
	}
}

void ProtrackerStream::interrupt(void) {
	int track;

	for (track = 0; track < 4; track++)
		_track[track].offset = _voice[track].offset;

	if (_tick == 0) {
		if (_hasJumpToPattern) {
			_hasJumpToPattern = false;
			_pos = _jumpToPattern;
			_row = 0;
		} else if (_hasPatternBreak) {
			_hasPatternBreak = false;
			_row = _skiprow;
			_pos = (_pos + 1) % _module->songlen;
			_patternLoopRow = 0;
		} else if (_hasPatternLoop) {
			_hasPatternLoop = false;
			_row = _patternLoopRow;
		}
		if (_row >= 64) {
			_row = 0;
			_pos = (_pos + 1) % _module->songlen;
			_patternLoopRow = 0;
		}

		if (_patternDelay == 0) {
			updateRow();
		} else {
			_patternDelay--;
		}
	} else
		updateEffects();

	_tick = (_tick + 1) % _speed;
	if (_tick == 0)
		_row++;

	for (track = 0; track < 4; track++) {
		_voice[track].period = _track[track].period + _track[track].vibrato;
		_voice[track].volume = _track[track].vol;
		if (_track[track].sample) {
			sample_t &sample = _module->sample[_track[track].sample - 1];
			_voice[track].data = sample.data;
			_voice[track].dataRepeat = sample.replen > 2 ? sample.data + sample.repeat : 0;
			_voice[track].length = sample.len;
			_voice[track].lengthRepeat = sample.replen;
			_voice[track].offset = _track[track].offset;
			_track[track].sample = 0;
		}
	}
}

} // End of namespace Modules

namespace Audio {

AudioStream *makeProtrackerStream(Common::ReadStream *stream, int rate, bool stereo) {
	return new Modules::ProtrackerStream(stream, rate, stereo);
}

} // End of namespace Audio
