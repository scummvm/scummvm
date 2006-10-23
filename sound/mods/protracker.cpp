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
#include "sound/mods/module.h"

#include "sound/audiostream.h"

namespace Modules {

class SoundBuffer {
private:
	int _capacity;
	int _size;
	int16 *_data;

public:
	SoundBuffer() {
		_size = 0;
		_capacity = 8192;
		_data = (int16 *)malloc(_capacity * sizeof(int16));
		assert(_data);
	}

	~SoundBuffer() {
		free(_data);
	}

	int size() {
		return _size;
	}

	int16 *getEnd() {
		return _data + _size;
	}

	void ensureCapacity(int len) {
		if (_size + len > _capacity) {
			do {
				_capacity *= 2;
			} while (_size + len > _capacity);

			_data = (int16 *)realloc(_data, _capacity * sizeof(int16));
			assert(_data);
		}
	}

	void finish(int len) {
		_size += len;
	}

	void pop(int16 *dest, int len) {
		assert(_size >= len);
		memcpy(dest, _data, len * sizeof(int16));
		memmove(_data, _data + len, (_size - len) * sizeof(int16));
		_size -= len;
	}
};

class ProtrackerStream : public ::Audio::AudioStream {
private:
	Module *_module;

	int _rate;
	SoundBuffer *_buf;
	double _generatedSamplesOverflow;

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
	ProtrackerStream(Common::ReadStream *stream, int rate);

	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return false; }
	bool endOfData() const { return false; }

	int getRate() const { return _rate; }
	
private:
	void generateSound();

	void updateRow();
	void updateEffects();
};

ProtrackerStream::ProtrackerStream(Common::ReadStream *stream, int rate) {
	_module = new Module();
	bool result = _module->load(*stream);
	assert(result);

	_buf = new SoundBuffer();
	
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

	_generatedSamplesOverflow = 0.0;

	for (int t = 0; t < 4; t++) {
		_track[t].sample = 0;
		_track[t].period = 0;
		_track[t].offset = 0.0;

		_track[t].vibrato = 0;
	}	
}

void ProtrackerStream::generateSound() {
	_generatedSamplesOverflow += 5.0 * _rate / (2.0 * _bpm);
	int samples = (int)floor(_generatedSamplesOverflow);
	_generatedSamplesOverflow -= samples;

	_buf->ensureCapacity(samples);

	int16 *p = _buf->getEnd();
	for (int i = 0; i < samples; i++)
		p[i] = 0;

	for (int track = 0; track < 4; track++) {
		if (_track[track].sample > 0) {
			p = _buf->getEnd();

			double frequency =
			    (7093789.2 / 2.0) / (_track[track].period +
			    _track[track].vibrato);

			double rate = frequency / _rate;
			double offset = _track[track].offset;
			int sample = _track[track].sample - 1;
			int slen = _module->sample[sample].len;
			int8 *data = _module->sample[sample].data;

			static bool did_warn_about_finetune = false;
			if (!did_warn_about_finetune && _module->sample[sample].finetune != 0) {
				did_warn_about_finetune = true;
				warning("Finetuning not implemented!");
			}

			if (_module->sample[sample].replen > 2) {
				int neededSamples = samples;

				while ((int)(offset + neededSamples * rate) >=
				    (_module->sample[sample].repeat + _module->sample[sample].replen)) {
					/* The repeat length is the limiting factor */

					int end =
					    (int)((_module->sample[sample].
						repeat + _module->sample[sample].
						replen - offset) / rate);

					for (int i = 0; i < end; i++)
						*p++ +=
						    _track[track].vol * data[(int)(offset + rate * i)];
					_track[track].offset =
					    _module->sample[sample].repeat;
					offset = _track[track].offset;
					neededSamples -= end;
				}
				if (neededSamples > 0) {
					/* The requested number of samples is the limiting factor, not the repeat length */

					for (int i = 0; i < neededSamples; i++)
						*p++ +=
						    _track[track].vol * data[(int)(offset + rate * i)];
					_track[track].offset +=
					    rate * neededSamples;
				}
			} else {
				if (offset < slen) {
					if ((int)(offset + samples * rate) >=
					    slen) {
						/* The end of the sample is the limiting factor */

						int end = (int)((slen - offset) / rate);
						for (int i = 0; i < end; i++)
							*p++ += _track[track].vol *
							    data[(int)(offset + rate * i)];
						_track[track].offset = slen;
					} else {
						/* The requested number of samples is the limiting factor, not the sample */

						for (int i = 0; i < samples; i++)
							*p++ += _track[track].vol *
							    data[(int)(offset + rate * i)];
						_track[track].offset += rate * samples;
					}
				}
			}
		}
	}

	_buf->finish(samples);
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

int ProtrackerStream::readBuffer(int16 *buffer, const int numSamples) {
	while (_buf->size() < numSamples) {
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

		generateSound();
	}

	_buf->pop(buffer, numSamples);

	return numSamples;
}

} // End of namespace Modules

namespace Audio {

AudioStream *makeProtrackerStream(Common::ReadStream *stream, int rate) {
	return new Modules::ProtrackerStream(stream, rate);
}

} // End of namespace Audio
