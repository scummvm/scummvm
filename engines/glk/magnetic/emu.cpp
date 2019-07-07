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

#include "glk/magnetic/magnetic.h"

namespace Glk {
namespace Magnetic {

//static const char *no_hints = "[Hints are not available.]\n";
//static const char *not_supported = "[This function is not supported.]\n";

int Magnetic::ms_init(bool restarting) {
	byte header[42];
	uint32 i, j, dict_size, string2_size, code_size, dec;

	ms_stop();

	if (restarting) {
		if (!restart)
			return 0;
		else {
			memcpy(code, restart, undo_size);
			undo_stat[0] = undo_stat[1] = 0;
			ms_showpic(0, 0);
		}
	} else {
		undo_stat[0] = undo_stat[1] = 0;

		if (_gameFile.read(header, 42) != 42 || READ_BE_UINT32(header) != MKTAG('M', 'a', 'S', 'c')
			|| READ_LE_UINT32(header + 8) != 42)
			return 0;

		ms_freemem();
		version = header[13];
		code_size = READ_LE_UINT32(header + 14);
		string_size = READ_LE_UINT32(header + 18);
		string2_size = READ_LE_UINT32(header + 22);
		dict_size = READ_LE_UINT32(header + 26);
		undo_size = READ_LE_UINT32(header + 34);
		undo_pc = READ_LE_UINT32(header + 38);

		if ((version < 4) && (code_size < 65536))
			mem_size = 65536;
		else
			mem_size = code_size;

		sd = (byte)((dict_size != 0L) ? 1 : 0);			// if (sd) => separate dict

		if (!(code = new byte[mem_size]) || !(string2 = new byte[string2_size]) ||
			!(restart = new byte[undo_size]) || (sd &&
				!(dict = new byte[dict_size]))) {
			ms_freemem();
			return 0;
		}
		if (string_size > MAX_STRING_SIZE) {
			if (!(string = new byte[MAX_STRING_SIZE]) ||
				!(string3 = new byte[string_size - MAX_STRING_SIZE])) {
				ms_freemem();
				return 0;
			}
		} else {
			if (!(string = new byte[string_size])) {
				ms_freemem();
				return 0;
			}
		}
		if (!(undo[0] = new byte[undo_size]) || !(undo[1] = new byte[undo_size])) {
			ms_freemem();
			return 0;
		}
		if (_gameFile.read(code, code_size) != code_size) {
			ms_freemem();
			return 0;
		}

		memcpy(restart, code, undo_size);	// fast restarts
		if (string_size > MAX_STRING_SIZE) {
			if (_gameFile.read(string, MAX_STRING_SIZE) != MAX_STRING_SIZE) {
				ms_freemem();
				return 0;
			}
			if (_gameFile.read(string3, string_size - MAX_STRING_SIZE) != (string_size - MAX_STRING_SIZE)) {
				ms_freemem();
				return 0;
			}
		} else {
			if (_gameFile.read(string, string_size) != string_size) {
				ms_freemem();
				return 0;
			}
		}
		if (_gameFile.read(string2, string2_size) != string2_size) {
			ms_freemem();
			return 0;
		}
		if (sd && _gameFile.read(dict, dict_size) != dict_size) {
			ms_freemem();
			return 0;
		}

		dec = READ_LE_UINT32(header + 30);
		if (dec >= string_size) {
			decode_table = string2 + dec - string_size;
		} else {
			if (dec >= MAX_STRING_SIZE)
				decode_table = string3 + dec - MAX_STRING_SIZE;
			else
				decode_table = string + dec;
		}
	}

	for (i = 0; i < 8; i++)
		dreg[i] = areg[i] = 0;
	write_reg(8 + 7, 2, 0xfffe);	// Stack-pointer, -2 due to MS-DOS segments
	pc = 0;
	zflag = nflag = cflag = vflag = 0;
	i_count = 0;
	running = 1;

	if (restarting)
		return (byte)(gfx_buf ? 2 : 1);		// Restarted

	if (version == 4) {
		// Try loading a hint file
		if (_hintFile.isOpen()) {
			_hintFile.seek(0);

			if (_hintFile.readUint32BE() == MKTAG('M', 'a', 'H', 't')) {
				uint16 blkcnt, elcnt, ntype, elsize, conidx;

				// Allocate memory for hints
				hints = new ms_hint[MAX_HINTS];
				hint_contents = new byte[MAX_HCONTENTS];

				if ((hints != 0) && (hint_contents != 0)) {
					// Read number of blocks
					blkcnt = _hintFile.readUint16LE();

					conidx = 0;
					for (i = 0; i < blkcnt; i++) {
						// Read number of elements
						elcnt = _hintFile.readUint16LE();
						hints[i].elcount = elcnt;

						// Read node type
						ntype = _hintFile.readUint16LE();
						hints[i].nodetype = ntype;
						hints[i].content = hint_contents + conidx;

						for (j = 0; j < elcnt; j++) {
							elsize = _hintFile.readUint16LE();
							if (_hintFile.read(hint_contents + conidx, elsize) != elsize || _hintFile.eos())
								return 0;
							hint_contents[conidx + elsize - 1] = '\0';

							conidx += elsize;
						}

						// Do we need a jump table?
						if (ntype == 1) {
							for (j = 0; j < elcnt; j++) {
								hints[i].links[j] = _hintFile.readUint16LE();
							}
						}

						// Read the parent block
						hints[i].parent = _hintFile.readUint16LE();

					}
				} else {
					delete[] hints;
					delete[] hint_contents;
					hints = nullptr;
					hint_contents = nullptr;
				}
			}
		}

		// Try loading a music file
		if (_sndFile.isOpen() && _sndFile.size() >= 8) {
			_sndFile.seek(0);

			if (_sndFile.readUint32BE() != MKTAG('M', 'a', 'S', 'd'))
				return 0;

			init_snd(_sndFile.readUint32LE());
		}
	}

	if (!_gfxFile.isOpen() || _gfxFile.size() < 8)
		return 1;
	_gfxFile.seek(0);
	uint tag = _gfxFile.readUint32BE();

	if (version < 4 && tag == MKTAG('M', 'a', 'P', 'i'))
		return init_gfx1(_gfxFile.readUint32LE() - 8);
	else if (version == 4 && tag == MKTAG('M', 'a', 'P', '2'))
		return init_gfx2(_gfxFile.readUint16LE());

	return 1;
}

void Magnetic::ms_freemem() {
	delete[] code;
	delete[] string;
	delete[] string2;
	delete[] string3;
	delete[] dict;
	delete[] undo[0];
	delete[] undo[1];
	delete[] restart;
	code = string = string2 = string3 = dict = nullptr;
	undo[0] = undo[1] = restart = nullptr;

	delete[] gfx_data;
	delete[] gfx_buf;
	delete[] gfx2_hdr;
	delete[] gfx2_buf;
	gfx_data = gfx_buf = gfx2_hdr = gfx2_buf = 0;

	gfx_fp.close();

	gfx2_name.clear();
	gfx_ver = 0;
	gfxtable = table_dist = 0;

	pos_table_size = 0;
	command_index = 0;
	anim_repeat = 0;
	pos_table_index = -1;
	pos_table_max = -1;

	lastchar = 0;
	delete[] hints;
	delete[] hint_contents;
	hints = nullptr;
	hint_contents = nullptr;

	delete[] snd_hdr;
	delete[] snd_buf;
	snd_hdr = nullptr;
	snd_hsize = 0;
	snd_buf = nullptr;
}

} // End of namespace Magnetic
} // End of namespace Glk
