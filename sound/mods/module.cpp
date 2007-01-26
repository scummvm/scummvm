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

#include "sound/mods/module.h"

#include "common/util.h"

namespace Modules {

bool Module::load(Common::ReadStream &st) {
	st.read(songname, 20);
	songname[20] = '\0';

	// FIXME: We define sample to have 32 entries,
	// yet we only setup 31 of these -- is this on
	// purpose, or an off-by-one error? This should
	// be clarified by either adding a comment explaining
	// this odditiy, or by fixing the off-by-one-bug.
	for (int i = 0; i < 31; ++i) {
		st.read(sample[i].name, 22);
		sample[i].name[22] = '\0';
		sample[i].len = 2 * st.readUint16BE();

		sample[i].finetune = st.readByte();
		assert(sample[i].finetune < 0x10);

		sample[i].vol = st.readByte();
		sample[i].repeat = 2 * st.readUint16BE();
		sample[i].replen = 2 * st.readUint16BE();
	}

	songlen = 2 * st.readByte();
	undef = 2 * st.readByte();

	st.read(songpos, 128);
	st.read(sig, 4);
	if (memcmp(sig, "M.K.", 4)) {
		warning("Expected 'M.K.' in protracker module");
		return false;
	}

	int maxpattern = 0;
	for (int i = 0; i < 128; ++i)
		if (maxpattern < songpos[i])
			maxpattern = songpos[i];

	pattern = new pattern_t[maxpattern + 1];

	for (int i = 0; i <= maxpattern; ++i) {
		for (int j = 0; j < 64; ++j) {
			for (int k = 0; k < 4; ++k) {
				uint32 note = st.readUint32BE();
				pattern[i][j][k].sample = (note & 0xf0000000) >> 24 | (note & 0x0000f000) >> 12;
				pattern[i][j][k].period = (note >> 16) & 0xfff;
				pattern[i][j][k].effect = note & 0xfff;
			}
		}
	}

	for (int i = 0; i < 31; ++i) {
		if (!sample[i].len)
			sample[i].data = 0;
		else {
			sample[i].data = new int8[sample[i].len];
			st.read((byte *)sample[i].data, sample[i].len);
		}
	}

	if (!st.eos())
		warning("Expected EOS on module stream");

	return true;
}

Module::Module() {
	pattern = 0;
	for (int i = 0; i < 31; ++i) {
		sample[i].data = 0;
	}
}

Module::~Module() {
	delete[] pattern;
	for (int i = 0; i < 31; ++i) {
		delete[] sample[i].data;
	}
}

} // End of namespace Modules
