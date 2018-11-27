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

#ifndef GLK_FROTZ_QUETZAL
#define GLK_FROTZ_QUETZAL

#include "glk/glk_types.h"
#include "glk/frotz/frotz_types.h"

namespace Glk {
namespace Frotz {

enum QueztalTag {
	ID_FORM = MKTAG('F', 'O', 'R', 'M'),
	ID_IFZS = MKTAG('I', 'F', 'Z', 'S'),
	ID_IFhd = MKTAG('I', 'F', 'h', 'd'),
	ID_UMem = MKTAG('U', 'M', 'e', 'm'),
	ID_CMem = MKTAG('C', 'M', 'e', 'm'),
	ID_Stks = MKTAG('S', 't', 'k', 's'),
	ID_ANNO = MKTAG('A', 'N', 'N', 'O'),
	ID_SCVM = MKTAG('S', 'C', 'V', 'M')
};

class Processor;

class Quetzal {
private:
	Common::SeekableReadStream *_storyFile;
	Common::WriteStream *_out;
	zword frames[STACK_SIZE / 4 + 1];
private:
	/**
	 * Read a 16-bit value from the file
	 */
	bool read_word(Common::ReadStream *f, zword *result);

	/**
	 * Read  32-bit value from the file
	 */
	bool read_long(Common::ReadStream *f, uint *result);

	void write_byte(zbyte b) { _out->writeByte(b); }
	void write_bytx(zword b) { _out->writeByte(b & 0xFF); }
	void write_word(zword w) { _out->writeUint16BE(w); }
	void write_long(uint l) { _out->writeUint32BE(l); }
	void write_run(zword run) { write_byte(0); write_byte(run); }
	void write_chnk(QueztalTag id, zword len) {
		_out->writeUint32BE(id);
		_out->writeUint32BE(len);
	}
public:
	/**
	 * Constructor
	 */
	Quetzal(Common::SeekableReadStream *storyFile) : _storyFile(storyFile) {}

	/*
	 * Save a game using Quetzal format.
	 * @param svf	Savegame file
	 * @param proc	Pointer to the Frotz processor
	 * @param desc	Savegame description
	 * @returns		Returns true if OK, false if failed
	 */
	bool save(Common::WriteStream *svf, Processor *proc, const Common::String &desc);

	/**
	 * Restore a saved game using Quetzal format
	 * @param svf	Savegame file
	 * @param proc	Pointer to the Frotz processor
	 * @returns		Return 2 if OK, 0 if an error occurred before any damage was done,
	 *				-1 on a fatal error
	 */
	int restore(Common::SeekableReadStream *svf, Processor *proc);
};

} // End of namespace Frotz
} // End of namespace Glk

#endif
