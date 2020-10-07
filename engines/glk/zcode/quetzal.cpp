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

#include "glk/zcode/quetzal.h"
#include "glk/zcode/processor.h"
#include "common/memstream.h"

namespace Glk {
namespace ZCode {

/**
 * Various parsing states within restoration.
 */
enum ParseState {
	GOT_HEADER = 0x01,
	GOT_STACK  = 0x02,
	GOT_MEMORY = 0x04,
	GOT_NONE   = 0x00,
	GOT_ALL    = 0x07,
	GOT_ERROR  = 0x80
};

#define WRITE_RUN(RUN) ws.writeByte(0); ws.writeByte((byte)(RUN))

bool Quetzal::save(Common::WriteStream *svf, Processor *proc, const Common::String &desc) {
	Processor &p = *proc;
	offset_t pc;
	zword i, j, n;
	zword nvars, nargs, nstk;
	zbyte var;
	int c;

	// Reset Quetzal writer
	_writer.clear();

	// Write `IFhd' chunk
	{
		Common::WriteStream &ws = _writer.add(ID_IFhd);
		pc = p.getPC();
		ws.writeUint16BE(p.h_release);
		ws.write(&p[H_SERIAL], 6);
		ws.writeUint16BE(p.h_checksum);

		ws.writeByte((pc >> 16) & 0xff);
		ws.writeByte((pc >> 8) & 0xff);
		ws.writeByte(pc & 0xff);
	}

	// Write `CMem' chunk.
	{
		Common::WriteStream &ws = _writer.add(ID_CMem);
		_storyFile->seek(0);

		// j holds current run length.
		for (i = 0, j = 0; i < p.h_dynamic_size; ++i) {
			c = _storyFile->readByte();
			c ^= p[i];

			if (c == 0) {
				// It's a run of equal bytes
				++j;
			} else {
				// Write out any run there may be.
				if (j > 0) {
					for (; j > 0x100; j -= 0x100) {
						WRITE_RUN(0xFF);
					}
					WRITE_RUN(j - 1);
					j = 0;
				}

				// Any runs are now written. Write this (nonzero) byte
				ws.writeByte(c);
			}
		}
	}

	// Write `Stks' chunk. You are not expected to understand this. ;)
	{
		Common::WriteStream &ws = _writer.add(ID_Stks);

		// We construct a list of frame indices, most recent first, in `frames'.
		// These indices are the offsets into the `stack' array of the word before
		// the first word pushed in each frame.
		frames[0] = p._sp - p._stack;	// The frame we'd get by doing a call now.
		for (i = p._fp - p._stack + 4, n = 0; i < STACK_SIZE + 4; i = p._stack[i - 3] + 5)
			frames[++n] = i;

		// All versions other than V6 can use evaluation stack outside a function
		// context. We write a faked stack frame (most fields zero) to cater for this.
		if (p.h_version != V6) {
			for (i = 0; i < 6; ++i)
				ws.writeByte(0);
			nstk = STACK_SIZE - frames[n];
			ws.writeUint16BE(nstk);
			for (j = STACK_SIZE - 1; j >= frames[n]; --j)
				ws.writeUint16BE(p._stack[j]);
		}

		// Write out the rest of the stack frames.
		for (i = n; i > 0; --i) {
			zword *pf = p._stack + frames[i] - 4;	// Points to call frame
			nvars = (pf[0] & 0x0F00) >> 8;
			nargs = pf[0] & 0x00FF;
			nstk = frames[i] - frames[i - 1] - nvars - 4;
			pc = ((uint)pf[3] << 9) | pf[2];

			// Check type of call
			switch (pf[0] & 0xF000) {
			case 0x0000:
				// Function
				var = p[pc];
				pc = ((pc + 1) << 8) | nvars;
				break;

			case 0x1000:
				// Procedure
				var = 0;
				pc = (pc << 8) | 0x10 | nvars;	// Set procedure flag
				break;

			default:
				p.runtimeError(ERR_SAVE_IN_INTER);
				return 0;
			}
			if (nargs != 0)
				nargs = (1 << nargs) - 1;	// Make args into bitmap

			// Write the main part of the frame...
			ws.writeUint32BE(pc);
			ws.writeByte(var);
			ws.writeByte(nargs);
			ws.writeUint16BE(nstk);

			// Write the variables and eval stack
			for (j = 0, --pf; j<nvars + nstk; ++j, --pf)
				ws.writeUint16BE(*pf);
		}
	}

	// Write the save data out
	_writer.save(svf, desc, ID_IFZS);

	// After all that, still nothing went wrong!
	return true;
}

int Quetzal::restore(Common::SeekableReadStream *sv, Processor *proc) {
	Processor &p = *proc;
	uint tmpl, currlen;
	offset_t pc;
	zword tmpw;
	int fatal = 0;	// Set to -1 when errors must be fatal.
	zbyte progress = GOT_NONE;
	int i, x, y;

	// Load the savefile for reading
	if (!_reader.open(sv, ID_IFZS)) {
		p.print_string("This is not a saved game file!\n");
		return 0;
	}

	// Read each chunk and process it
	for (QuetzalReader::Iterator it = _reader.begin(); it != _reader.end(); ++it) {
		Common::SeekableReadStream *s = it.getStream();
		currlen = (*it)._size;

		switch ((*it)._id) {
		// `IFhd' header chunk; must be first in file
		case ID_IFhd:
			if (progress & GOT_HEADER) {
				p.print_string("Save file has two IFZS chunks!\n");
				return fatal;
			}
			progress |= GOT_HEADER;
			if (currlen < 13)
				return fatal;

			tmpw = s->readUint16BE();
			if (tmpw != p.h_release)
				progress = GOT_ERROR;

			for (int idx = H_SERIAL; idx < H_SERIAL + 6; ++idx) {
				x = s->readByte();
				if (x != p[idx])
					progress = GOT_ERROR;
			}

			tmpw = s->readUint16BE();
			if (tmpw != p.h_checksum)
				progress = GOT_ERROR;

			if (progress & GOT_ERROR) {
				p.print_string("File was not saved from this story!\n");
				return fatal;
			}

			x = s->readByte();
			pc = (uint)x << 16;
			x = s->readByte();
			pc |= (uint)x << 8;
			x = s->readByte();
			pc |= (uint)x;

			fatal = -1;		// Setting PC means errors must be fatal
			p.setPC(pc);
			break;

		// `Stks' stacks chunk; restoring this is quite complex. ;)
		case ID_Stks:
			if (progress & GOT_STACK) {
				p.print_string("File contains two stack chunks!\n");
				break;
			}
			progress |= GOT_STACK;

			fatal = -1;		// Setting SP means errors must be fatal
			p._sp = p._stack + STACK_SIZE;

			// All versions other than V6 may use evaluation stack outside any function context.
			// As a result a faked function context will be present in the file here. We skip
			// this context, but load the associated stack onto the stack proper...
			if (p.h_version != V6) {
				if (currlen < 8)
					return fatal;

				s->skip(6);
				tmpw = s->readUint16BE();

				if (tmpw > STACK_SIZE) {
					p.print_string("Save-file has too much stack (and I can't cope).\n");
					return fatal;
				}

				currlen -= 8;
				if (currlen < (uint)tmpw * 2)
					return fatal;
				for (i = 0; i < tmpw; ++i)
					*--p._sp = s->readUint16BE();
				currlen -= tmpw * 2;
			}

			// We now proceed to load the main block of stack frames
			for (p._fp = p._stack + STACK_SIZE, p._frameCount = 0;
					currlen > 0; currlen -= 8, ++p._frameCount) {
				if (currlen < 8)				return fatal;
				if (p._sp - p._stack < 4) {
					// No space for frame
					p.print_string("Save-file has too much stack (and I can't cope).\n");
					return fatal;
				}

				// Read PC, procedure flag and formal param count
				tmpl = s->readUint32BE();
				y = (int)(tmpl & 0x0F);		// Number of formals
				tmpw = y << 8;

				// Read result variable
				x = s->readByte();

				// Check the procedure flag...
				if (tmpl & 0x10) {
					tmpw |= 0x1000;		// It's a procedure
					tmpl >>= 8;			// Shift to get PC value
				} else {
					// Functions have type 0, so no need to or anything
					tmpl >>= 8;			// Shift to get PC value
					--tmpl;				// Point at result byte. */

					// Sanity check on result variable...
					if (p[tmpl] != (zbyte)x) {
						p.print_string("Save-file has wrong variable number on stack (possibly wrong game version?)\n");
						return fatal;
					}
				}

				*--p._sp = (zword)(tmpl >> 9);		// High part of PC
				*--p._sp = (zword)(tmpl & 0x1FF);	// Low part of PC
				*--p._sp = (zword)(p._fp - p._stack - 1);	// FP

				// Read and process argument mask
				x = s->readByte();
				++x;		// Should now be a power of 2
				for (i = 0; i<8; ++i)
					if (x & (1 << i))
						break;
				if (x ^ (1 << i)) {
					// Not a power of 2
					p.print_string("Save-file uses incomplete argument lists (which I can't handle)\n");
					return fatal;
				}

				*--p._sp = tmpw | i;
				p._fp = p._sp;	// FP for next frame

				// Read amount of eval stack used
				tmpw = s->readUint16BE();

				tmpw += y;	// Amount of stack + number of locals
				if (p._sp - p._stack <= tmpw) {
					p.print_string("Save-file has too much stack (and I can't cope).\n");
					return fatal;
				}
				if (currlen < (uint)tmpw * 2)
					return fatal;
				
				for (i = 0; i < tmpw; ++i)
					*--p._sp = s->readUint16BE();
				currlen -= tmpw * 2;
			}

			// End of `Stks' processing...
			break;

		// `CMem' compressed memory chunk; uncompress it
		case ID_CMem:
			if (!(progress & GOT_MEMORY)) {
				_storyFile->seek(0);
				
				i = 0;	// Bytes written to data area
				for (; currlen > 0; --currlen) {
					x = s->readByte();
					if (x == 0) {
						// Start of run
						// Check for bogus run
						if (currlen < 2) {
							p.print_string("File contains bogus `CMem' chunk.\n");
							s->skip(currlen);

							currlen = 1;
							i = 0xFFFF;
							break; // Keep going; may be a `UMem' too
						}

						// Copy story file to memory during the run
						--currlen;
						x = s->readByte();
						for (; x >= 0 && i < p.h_dynamic_size; --x, ++i)
							p[i] = _storyFile->readByte();
					} else {
						// Not a run
						y = _storyFile->readByte();
						p[i] = (zbyte)(x ^ y);
						++i;
					}

					// Make sure we don't load too much
					if (i > p.h_dynamic_size) {
						p.print_string("warning: `CMem' chunk too long!\n");
						s->skip(currlen);
						break;	// Keep going; there may be a `UMem' too
					}
				}

				// If chunk is short, assume a run
				for (; i < p.h_dynamic_size; ++i)
					p[i] = _storyFile->readByte();

				if (currlen == 0)
					progress |= GOT_MEMORY;		// Only if succeeded
				break;
			}
			break;

		// 'UMem' Uncompressed memory chunk
		case ID_UMem:
			if (!(progress & GOT_MEMORY)) {
				// Must be exactly the right size
				if (currlen == p.h_dynamic_size) {
					if (s->read(p.zmp, currlen) == currlen) {
						progress |= GOT_MEMORY;	// Only on success
						break;
					}
				} else {
					p.print_string("`UMem' chunk wrong size!\n");
				}
				
				// Fall into default action (skip chunk) on errors
			}
			break;

		default:
			break;
		}

		delete s;
	}

	// We've reached the end of the file. For the restoration to have been a
	// success, we must have had one of each of the required chunks.
	if (!(progress & GOT_HEADER))
		p.print_string("error: no valid header (`IFhd') chunk in file.\n");
	if (!(progress & GOT_STACK))
		p.print_string("error: no valid stack (`Stks') chunk in file.\n");
	if (!(progress & GOT_MEMORY))
		p.print_string("error: no valid memory (`CMem' or `UMem') chunk in file.\n");

	return (progress == GOT_ALL ? 2 : fatal);
}

} // End of namespace ZCode
} // End of namespace Glk
