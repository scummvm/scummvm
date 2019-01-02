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

#include "glk/frotz/quetzal.h"
#include "glk/frotz/processor.h"
#include "common/memstream.h"

namespace Glk {
namespace Frotz {

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


bool Quetzal::read_word(Common::ReadStream *f, zword *result) {
	*result = f->readUint16BE();
	return true;
}

bool Quetzal::read_long(Common::ReadStream *f, uint *result) {
	*result = f->readUint32BE();
	return true;
}

bool Quetzal::save(Common::WriteStream *svf, Processor *proc, const Common::String &desc) {
	Processor &p = *proc;
	uint ifzslen = 0, cmemlen = 0, stkslen = 0, descLen = 0;
	uint pc;
	zword i, j, n;
	zword nvars, nargs, nstk;
	zbyte var;
	long cmempos, stkspos;
	int c;

	// Set a temporary memory stream for writing out the data. This is needed, since we need to
	// do some seeking within it at the end to fill out totals before properly writing it all out
	Common::MemoryWriteStreamDynamic saveData(DisposeAfterUse::YES);
	_out = &saveData;

	// Write `IFZS' header.
	write_chnk(ID_FORM, 0);
	write_long(ID_IFZS);

	// Write `IFhd' chunk
	pc = p.getPC();
	write_chnk(ID_IFhd, 13);
	write_word(p.h_release);
	for (i = H_SERIAL; i<H_SERIAL + 6; ++i)
		write_byte(p[i]);

	write_word(p.h_checksum);
	write_long(pc << 8);		// Includes pad

	// Write 'ANNO' chunk
	descLen = desc.size() + 1;
	write_chnk(ID_ANNO, descLen);
	saveData.write(desc.c_str(), desc.size());
	write_byte(0);
	if ((desc.size() % 2) == 0) {
		write_byte(0);
		++descLen;
	}

	// Write `CMem' chunk.
	cmempos = saveData.pos();
	write_chnk(ID_CMem, 0);
	_storyFile->seek(0);

	// j holds current run length.
	for (i = 0, j = 0, cmemlen = 0; i < p.h_dynamic_size; ++i) {
		c = _storyFile->readByte();
		c ^= p[i];

		if (c == 0) {
			// It's a run of equal bytes
			++j;
		} else {
			// Write out any run there may be.
			if (j > 0) {
				for (; j > 0x100; j -= 0x100) {
					write_run(0xFF);
					cmemlen += 2;
				}
				write_run(j - 1);
				cmemlen += 2;
				j = 0;
			}

			// Any runs are now written. Write this (nonzero) byte
			write_byte((zbyte)c);
			++cmemlen;
		}
	}

	// Reached end of dynamic memory. We ignore any unwritten run there may be at this point.
	if (cmemlen & 1)
		// Chunk length must be even.
		write_byte(0);

	// Write `Stks' chunk. You are not expected to understand this. ;)
	stkspos = saveData.pos();
	write_chnk(ID_Stks, 0);

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
			write_byte(0);
		nstk = STACK_SIZE - frames[n];
		write_word(nstk);
		for (j = STACK_SIZE - 1; j >= frames[n]; --j)
			write_word(p._stack[j]);
		stkslen = 8 + 2 * nstk;
	}

	// Write out the rest of the stack frames.
	for (i = n; i > 0; --i) {
		zword *pf = p._stack + frames[i] - 4;	// Points to call frame
		nvars = (pf[0] & 0x0F00) >> 8;
		nargs = pf[0] & 0x00FF;
		nstk = frames[i] - frames[i - 1] - nvars - 4;
		pc = ((uint)pf[3] << 9) | pf[2];

		// Check type of call
		switch (pf[0] & 0xF000)	{
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
		write_long(pc);
		write_byte(var);
		write_byte(nargs);
		write_word(nstk);

		// Write the variables and eval stack
		for (j = 0, --pf; j<nvars + nstk; ++j, --pf)
			write_word(*pf);

		// Calculate length written thus far
		stkslen += 8 + 2 * (nvars + nstk);
	}

	// Fill in variable chunk lengths
	ifzslen = 4 * 8 + 4 + 14 + cmemlen + stkslen + descLen;
	if (cmemlen & 1)
		++ifzslen;

	saveData.seek(4);
	saveData.writeUint32BE(ifzslen);
	saveData.seek(cmempos + 4);
	saveData.writeUint32BE(cmemlen);
	saveData.seek(stkspos + 4);
	saveData.writeUint32BE(stkslen);

	// Write the save data out
	svf->write(saveData.getData(), saveData.size());

	// After all that, still nothing went wrong!
	return true;
}


int Quetzal::restore(Common::SeekableReadStream *svf, Processor *proc) {
	Processor &p = *proc;
	uint ifzslen, currlen, tmpl;
	uint pc;
	zword i, tmpw;
	int fatal = 0;	// Set to -1 when errors must be fatal.
	zbyte skip, progress = GOT_NONE;
	int x, y;

	// Check it's really an `IFZS' file.
	tmpl = svf->readUint32BE();
	ifzslen = svf->readUint32BE();
	currlen = svf->readUint32BE();
	if (tmpl != ID_FORM || currlen != ID_IFZS) {
		p.print_string("This is not a saved game file!\n");
		return 0;
	}
	if ((ifzslen & 1) || ifzslen<4)
		// Sanity checks
		return 0;
	ifzslen -= 4;

	// Read each chunk and process it
	while (ifzslen > 0) {
		// Read chunk header
		if (ifzslen < 8)
			// Couldn't contain a chunk
			return 0;

		tmpl = svf->readUint32BE();
		currlen = svf->readUint32BE();
		ifzslen -= 8;	// Reduce remaining by size of header

		// Handle chunk body
		if (ifzslen < currlen)
			// Chunk goes past EOF?!
			return 0;
		skip = currlen & 1;
		ifzslen -= currlen + (uint)skip;

		switch (tmpl) {
		// `IFhd' header chunk; must be first in file
		case ID_IFhd:
			if (progress & GOT_HEADER) {
				p.print_string("Save file has two IFZS chunks!\n");
				return fatal;
			}
			progress |= GOT_HEADER;
			if (currlen < 13)
				return fatal;

			tmpw = svf->readUint16BE();
			if (tmpw != p.h_release)
				progress = GOT_ERROR;

			for (i = H_SERIAL; i < H_SERIAL + 6; ++i) {
				x = svf->readByte();
				if (x != p[i])
					progress = GOT_ERROR;
			}

			tmpw = svf->readUint16BE();
			if (tmpw != p.h_checksum)
				progress = GOT_ERROR;

			if (progress & GOT_ERROR) {
				p.print_string("File was not saved from this story!\n");
				return fatal;
			}

			x = svf->readByte();
			pc = (uint)x << 16;
			x = svf->readByte();
			pc |= (uint)x << 8;
			x = svf->readByte();
			pc |= (uint)x;

			fatal = -1;		// Setting PC means errors must be fatal
			p.setPC(pc);

			svf->skip(currlen - 13);	// Skip rest of chunk
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

				svf->skip(6);
				tmpw = svf->readUint16BE();

				if (tmpw > STACK_SIZE) {
					p.print_string("Save-file has too much stack (and I can't cope).\n");
					return fatal;
				}

				currlen -= 8;
				if (currlen < (uint)tmpw * 2)
					return fatal;
				for (i = 0; i < tmpw; ++i)
					*--p._sp = svf->readUint16BE();
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
				tmpl = svf->readUint32BE();
				y = (int)(tmpl & 0x0F);		// Number of formals
				tmpw = y << 8;

				// Read result variable
				x = svf->readByte();

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
				x = svf->readByte();
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
				tmpw = svf->readUint16BE();

				tmpw += y;	// Amount of stack + number of locals
				if (p._sp - p._stack <= tmpw) {
					p.print_string("Save-file has too much stack (and I can't cope).\n");
					return fatal;
				}
				if (currlen < (uint)tmpw * 2)
					return fatal;
				
				for (i = 0; i < tmpw; ++i)
					--*p._sp = svf->readUint16BE();
				currlen -= tmpw * 2;
			}

			// End of `Stks' processing...
			break;

		// Any more special chunk types must go in HERE or ABOVE
		// `CMem' compressed memory chunk; uncompress it
		case ID_CMem:
			if (!(progress & GOT_MEMORY)) {
				_storyFile->seek(0);
				
				i = 0;	// Bytes written to data area
				for (; currlen > 0; --currlen) {
					x = svf->readByte();
					if (x == 0) {
						// Start of run
						// Check for bogus run
						if (currlen < 2) {
							p.print_string("File contains bogus `CMem' chunk.\n");
							svf->skip(currlen);

							currlen = 1;
							i = 0xFFFF;
							break; // Keep going; may be a `UMem' too
						}

						// Copy story file to memory during the run
						--currlen;
						x = svf->readByte();
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
						svf->skip(currlen);
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

			// fall through

		case ID_UMem:
			if (!(progress & GOT_MEMORY)) {
				// Must be exactly the right size
				if (currlen == p.h_dynamic_size) {
					if (svf->read(p.zmp, currlen) == currlen) {
						progress |= GOT_MEMORY;	// Only on success
						break;
					}
				} else {
					p.print_string("`UMem' chunk wrong size!\n");
				}
				
				// Fall into default action (skip chunk) on errors
			}

			// fall through

		default:
			svf->seek(currlen, SEEK_CUR);		// Skip chunk
			break;
		}

		if (skip)
			svf->skip(1);						// Skip pad byte
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

} // End of namespace Frotz
} // End of namespace Glk
