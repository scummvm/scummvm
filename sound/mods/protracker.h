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

#ifndef __SOUND_MODS_PROTRACKER_H__
#define __SOUND_MODS_PROTRACKER_H__

#include "common/stdafx.h"
#include "common/system.h"

#include "sound/mods/module.h"

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
			memset(_data + _size, 0, len);
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

class ProtrackerPlayer {
	OSystem *_system;
	Module *_module;

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
	ProtrackerPlayer() : _system(NULL), _module(NULL) { };

	void init(OSystem *system);

	void start();
	void pause();
	void stop();

	void loadModule(const char *fn);

	void mix(byte *buf, int len);

private:
	void generateSound();

	void updateRow();
	void updateEffects();

	static void audioCallback(void *param, byte *buf, int len);
};

} // End of namespace Modules

#endif
