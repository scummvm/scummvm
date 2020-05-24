/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/unicode.h"
#include "glk/unicode_gen.h"
#include "common/textconsole.h"

namespace Glk {

size_t strlen_uni(const uint32 *s) {
	size_t len = 0;
	while (*s++)
		++len;
	return len;
}

uint bufferChangeCase(uint32 *buf, uint len, uint numchars, BufferChangeCase destcase,
						BufferChangeCond cond, int changerest) {
	uint ix, jx;
	uint32 *outbuf;
	uint32 *newoutbuf;
	uint outcount;
	int dest_block_rest = 0, dest_block_first = 0;
	int dest_spec_rest = 0, dest_spec_first = 0;

	switch (cond) {
	case COND_ALL:
		dest_spec_rest = destcase;
		dest_spec_first = destcase;
		break;
	case COND_LINESTART:
		if (changerest)
			dest_spec_rest = CASE_LOWER;
		else
			dest_spec_rest = CASE_IDENT;
		dest_spec_first = destcase;
		break;
	default:
		break;
	}

	dest_block_rest = dest_spec_rest;
	if (dest_block_rest == CASE_TITLE)
		dest_block_rest = CASE_UPPER;
	dest_block_first = dest_spec_first;
	if (dest_block_first == CASE_TITLE)
		dest_block_first = CASE_UPPER;

	newoutbuf = nullptr;
	outcount = 0;
	outbuf = buf;

	for (ix = 0; ix < numchars; ix++) {
		int target;
		int isfirst;
		uint res;
		uint *special;
		uint *ptr;
		uint speccount;
		uint32 ch = buf[ix];

		isfirst = (ix == 0);

		target = (isfirst ? dest_block_first : dest_block_rest);

		if (target == CASE_IDENT) {
			res = ch;
		} else {
			gli_case_block_t *block;

			GET_CASE_BLOCK(ch, &block);
			if (!block)
				res = ch;
			else
				res = block[ch & 0xFF][target];
		}

		if (res != 0xFFFFFFFF || res == ch) {
			// simple case
			if (outcount < len)
				outbuf[outcount] = res;
			outcount++;
			continue;
		}

		target = (isfirst ? dest_spec_first : dest_spec_rest);

		// complicated cases
		GET_CASE_SPECIAL(ch, &special);
		if (!special) {
			warning("inconsistency in cgunigen.c");
			continue;
		}
		ptr = &unigen_special_array[special[target]];
		speccount = *(ptr++);

		if (speccount == 1) {
			// simple after all
			if (outcount < len)
				outbuf[outcount] = ptr[0];
			outcount++;
			continue;
		}

		// Now we have to allocate a new buffer, if we haven't already.
		if (!newoutbuf) {
			newoutbuf = new uint32[len + 1];
			if (!newoutbuf)
				return 0;
			if (outcount)
				memcpy(newoutbuf, buf, outcount * sizeof(uint32));
			outbuf = newoutbuf;
		}

		for (jx = 0; jx < speccount; jx++) {
			if (outcount < len)
				outbuf[outcount] = ptr[jx];
			outcount++;
		}
	}

	if (newoutbuf) {
		uint finallen = outcount;
		if (finallen > len)
			finallen = len;
		if (finallen)
			memcpy(buf, newoutbuf, finallen * sizeof(uint));
		delete[] newoutbuf;
	}

	return outcount;
}

} // End of namespace Glk
