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
#include "common/system.h"

#include "sound/mods/protracker.h"

namespace Modules {

#define DUMP 1

void ProtrackerPlayer::init(OSystem *system) {
	_system = system;

	_buf = new SoundBuffer();

	_system->setSoundCallback(&audioCallback, this);
}

void ProtrackerPlayer::start() {
	if (_module) {
		_tick = _row = _pos = 0;
		_hasJumpToPattern = false;
		_hasPatternBreak = false;
		_hasPatternLoop = false;
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

		//_system->startAudio();
	}
}

void ProtrackerPlayer::pause() {
}

void ProtrackerPlayer::stop() {
	//_system->stopAudio();
}

void ProtrackerPlayer::loadModule(const char *fn) {
	if (_module)
		delete _module;

	_module = new Module();
	_module->load(fn);
}

void ProtrackerPlayer::generateSound() {
	_generatedSamplesOverflow += 5.0 * 44100.0 / (2.0 * _bpm);
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

			double rate = frequency / 44100.0;
			double offset = _track[track].offset;
			int sample = _track[track].sample - 1;
			int slen = _module->sample[sample].len;
			byte *data = _module->sample[sample].data;

			static bool did_warn_about_finetune = false;
			if (!did_warn_about_finetune && _module->sample[sample].finetune != 0) {
				did_warn_about_finetune = true;
				warning("Finetuning not implemented!");
			}

			if (_module->sample[sample].replen > 2) {
				int neededSamples = samples;

				//printf("%d %d >= %d\n", (int)offset, (int)(offset + neededSamples*rate), _module->sample[sample].repeat + _module->sample[sample].replen);

				while ((int)(offset + neededSamples * rate) >=
				    (_module->sample[sample].repeat + _module->sample[sample].replen)) {
					//puts("/* The repeat length is the limiting factor */");
					//printf("case 1: period: %d\trate: %g\toffset: %g\tsample: %d\tslen: %d\tvol: %d\n", _track[track].period, rate, _track[track].offset, _track[track].sample, slen, _track[track].vol);

					int end =
					    (int)((_module->sample[sample].
						repeat + _module->sample[sample].
						replen - offset) / rate);

					if ((int)(offset + rate * end) > slen)
						warning("!!!!!!!!!");
					//printf("writing %d-%d max %d\n", (int)offset, (int)(offset + rate*end), slen);

					for (int i = 0; i < end; i++)
						*p++ +=
						    _track[track].vol * data[(int)(offset + rate * i)];
					_track[track].offset =
					    _module->sample[sample].repeat;
					offset = _track[track].offset;
					neededSamples -= end;
				}
				if (neededSamples > 0) {
					//puts("/* The requested number of samples is the limiting factor, not the repeat length */");
					//printf("case 2: period: %d\trate: %g\toffset: %g\tsample: %d\tslen: %d\tvol: %d\n", _track[track].period, rate, _track[track].offset, _track[track].sample, slen, _track[track].vol);

					//if ((int)(offset + rate*neededSamples) > slen) puts("!!!!!!!!!");
					//printf("writing %d-%d max %d\n", (int)offset, (int)(offset + rate*neededSamples), slen);
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
						//printf("case 3: period: %d\trate: %g\toffset: %g\tsample: %d\tslen: %d\tvol: %d\n", _track[track].period, rate, _track[track].offset, _track[track].sample, slen, _track[track].vol);

						int end = (int)((slen - offset) / rate);
						for (int i = 0; i < end; i++)
							*p++ += _track[track].vol *
							    data[(int)(offset + rate * i)];
						_track[track].offset = slen;
					} else {
						/* The requested number of samples is the limiting factor, not the sample */
						//printf("case 4: period: %d\trate: %g\toffset: %g\tsample: %d\tslen: %d\tvol: %d\n", _track[track].period, rate, _track[track].offset, _track[track].sample, slen, _track[track].vol);

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

void ProtrackerPlayer::updateRow() {
	//printf("ProtrackerPlayer::updateRow(): tick: %d\tpos: %d->%d\trow: %d\n", _tick, _pos, _module->songpos[_pos], _row);

#ifdef DUMP
	printf("%3d:%3d:%2d: ", _pos, _module->songpos[_pos], _row);
#endif
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

#ifdef DUMP
		const char *notename;

		switch (note.period) {
		case 856:
			notename = "C-1";
			break;
		case 808:
			notename = "C#1";
			break;
		case 762:
			notename = "D-1";
			break;
		case 720:
			notename = "D#1";
			break;
		case 678:
			notename = "E-1";
			break;
		case 640:
			notename = "E#1";
			break;
		case 604:
			notename = "F-1";
			break;
		case 570:
			notename = "F#1";
			break;
		case 538:
			notename = "A-1";
			break;
		case 508:
			notename = "A#1";
			break;
		case 480:
			notename = "B-1";
			break;
		case 453:
			notename = "B#1";
			break;
		case 428:
			notename = "C-2";
			break;
		case 404:
			notename = "C#2";
			break;
		case 381:
			notename = "D-2";
			break;
		case 360:
			notename = "D#2";
			break;
		case 339:
			notename = "E-2";
			break;
		case 320:
			notename = "E#2";
			break;
		case 302:
			notename = "F-2";
			break;
		case 285:
			notename = "F#2";
			break;
		case 269:
			notename = "A-2";
			break;
		case 254:
			notename = "A#2";
			break;
		case 240:
			notename = "B-2";
			break;
		case 226:
			notename = "B#2";
			break;
		case 214:
			notename = "C-3";
			break;
		case 202:
			notename = "C#3";
			break;
		case 190:
			notename = "D-3";
			break;
		case 180:
			notename = "D#3";
			break;
		case 170:
			notename = "E-3";
			break;
		case 160:
			notename = "E#3";
			break;
		case 151:
			notename = "F-3";
			break;
		case 143:
			notename = "F#3";
			break;
		case 135:
			notename = "A-3";
			break;
		case 127:
			notename = "A#3";
			break;
		case 120:
			notename = "B-3";
			break;
		case 113:
			notename = "B#3";
			break;
		case 0:
			notename = "   ";
			break;
		default:
			notename = "???";
			break;
		}

		if (track > 0)
			printf("  |  ");

		if (note.sample)
			printf("%2d %s %3X", note.sample, notename,
			    note.effect);
		else
			printf("   %s %3X", notename, note.effect);
#endif

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
				printf("Unimplemented effect %X\n",
				    note.effect);
			}
			break;

		case 0xF:
			if (exy < 0x20) {
				_speed = exy;
				//printf("Speed: %x\n", _speed);
			} else {
				_bpm = exy;
				//printf("BPM: %x\n", _bpm);
			}
			break;
		default:
			printf("Unimplemented effect %X\n", note.effect);
		}
	}
#ifdef DUMP
	putchar('\n');
	fflush(NULL);
#endif
}

void ProtrackerPlayer::updateEffects() {
	//printf("ProtrackerPlayer::updateEffects(): tick: %d\tpos: %d->%d\trow: %d\n", _tick, _pos, _module->songpos[_pos], _row);

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

		//printf("track %d: period: %3d\tsample: %2d\teffect: %x\n", track, note.period, note.sample, note.effect);

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

void ProtrackerPlayer::mix(byte *buf0, int len) {
	int16 *buf = (int16 *)buf0;
	len /= 2;

	while (_buf->size() < len) {
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

	_buf->pop(buf, len);
}

void ProtrackerPlayer::audioCallback(void *param, byte *buf, int len) {
	((ProtrackerPlayer *)param)->mix(buf, len);
}

} // End of namespace Modules

