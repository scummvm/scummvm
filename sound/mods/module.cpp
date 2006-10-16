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
#include "common/file.h"
#include "common/stream.h"

#include "sound/mods/module.h"

namespace Modules {

bool Module::load(const char *fn) {
	Common::File f;

	if (!f.open(fn))
		return false;

	int bufsz = f.size();
	byte *buf = new byte[bufsz];

	int r = f.read(buf, bufsz);
	assert(r);

	f.close();

	Common::MemoryReadStream st(buf, bufsz);

	st.read(songname, 20);
	songname[0] = '\0';

	for (int i = 0; i < 31; ++i) {
		st.read(sample[i].name, 22);
		sample[i].name[22] = '\0';
		sample[i].len = 2 * st.readUint16BE();

		sample[i].finetune = st.readByte();
		assert(sample[i].finetune < 0x10);

		sample[i].vol = st.readByte();
		sample[i].repeat = 2 * st.readUint16BE();
		sample[i].replen = 2 * st.readUint16BE();

		//printf("\"%s\"\tlen: %d\tfinetune: %d\tvol: %d\trepeat: %d\treplen: %d\n",
		//       sample[i].name, sample[i].len, sample[i].finetune, sample[i].vol, sample[i].repeat, sample[i].replen);

	}

	songlen = 2 * st.readByte();
	undef = 2 * st.readByte();

	st.read(songpos, 128);
	st.read(sig, 4);	// Should be "M.K."
	assert(0 == memcmp(sig, "M.K.", 4));

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
/*
        const char *notename;

        switch (pattern[i][j][k].period) {
          case 856: notename = "C-1"; break;
          case 808: notename = "C#1"; break;
          case 762: notename = "D-1"; break;
          case 720: notename = "D#1"; break;
          case 678: notename = "E-1"; break;
          case 640: notename = "E#1"; break;
          case 604: notename = "F-1"; break;
          case 570: notename = "F#1"; break;
          case 538: notename = "A-1"; break;
          case 508: notename = "A#1"; break;
          case 480: notename = "B-1"; break;
          case 453: notename = "B#1"; break;
          case 428: notename = "C-2"; break;
          case 404: notename = "C#2"; break;
          case 381: notename = "D-2"; break;
          case 360: notename = "D#2"; break;
          case 339: notename = "E-2"; break;
          case 320: notename = "E#2"; break;
          case 302: notename = "F-2"; break;
          case 285: notename = "F#2"; break;
          case 269: notename = "A-2"; break;
          case 254: notename = "A#2"; break;
          case 240: notename = "B-2"; break;
          case 226: notename = "B#2"; break;
          case 214: notename = "C-3"; break;
          case 202: notename = "C#3"; break;
          case 190: notename = "D-3"; break;
          case 180: notename = "D#3"; break;
          case 170: notename = "E-3"; break;
          case 160: notename = "E#3"; break;
          case 151: notename = "F-3"; break;
          case 143: notename = "F#3"; break;
          case 135: notename = "A-3"; break;
          case 127: notename = "A#3"; break;
          case 120: notename = "B-3"; break;
          case 113: notename = "B#3"; break;
          case   0: notename = "   "; break;
          default:  notename = "???"; break;
        }

        if (k > 0) printf("  |  ");

        if (pattern[i][j][k].sample)
          printf("%2d %s %3X", pattern[i][j][k].sample, notename, pattern[i][j][k].effect);
        else
          printf("   %s %3X", notename, pattern[i][j][k].effect);
*/
			}
			//printf("\n");
		}
	}

	for (int i = 0; i < 31; ++i) {
		if (!sample[i].len)
			sample[i].data = 0;
		else {
			sample[i].data = new byte[sample[i].len];
			st.read((byte *)sample[i].data, sample[i].len);
		}
	}

	assert(st.eos());

	delete[] buf;

	return true;
}

Module::Module() {
	pattern = NULL;
	for (int i = 0; i < 31; ++i) {
		sample[i].data = NULL;
	}
}

Module::~Module() {
	delete[] pattern;
	for (int i = 0; i < 31; ++i) {
		delete[] sample[i].data;
	}
}

} // End of namespace Modules
