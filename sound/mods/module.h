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

#ifndef SOUND_MODS_MODULE_H
#define SOUND_MODS_MODULE_H

#include "common/file.h"

namespace Modules {

/*
 * Storing notes and patterns like this
 * wastes insane amounts of memory.
 *
 * They should be stored in memory
 * like they are in the file.
 */

struct note_t {
	byte sample;
	uint16 period;
	uint16 effect;
};

typedef note_t pattern_t[64][4];

struct sample_t {
	byte name[23];
	uint16 len;
	byte finetune;
	byte vol;
	uint16 repeat;
	uint16 replen;
	byte *data;
};

class Module {
public:
	byte songname[21];

	sample_t sample[32];

	byte songlen;
	byte undef;
	byte songpos[128];
	byte sig[4];
	pattern_t *pattern;

	Module();
	~Module();

	bool load(const char *fn);
	bool loadStream(Common::SeekableReadStream &st);
};

} // End of namespace Modules

#endif
