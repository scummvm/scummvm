/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//=============================================================================
//
// LZW compression.
//
//=============================================================================

#include "ags/shared/util/lzw.h"
#include "ags/shared/ac/common.h" // quit
#include "ags/shared/util/bbop.h"
#include "ags/shared/util/memory.h"
#include "ags/shared/util/stream.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

#ifdef _MANAGED
// ensure this doesn't get compiled to .NET IL
#pragma unmanaged
#endif

int insert(int, int);
void _delete(int);

#define N 4096
#define F 16
#define THRESHOLD 3
#define min(xx,yy) ((yy<xx) ? yy : xx)

#define dad (_G(node)+1)
#define lson (_G(node)+1+N)
#define rson (_G(node)+1+N+N)
#define root (_G(node)+1+N+N+N)
#define NIL -1

int insert(int i, int run) {
	int c, j, k, l, n, match;
	int *p;

	c = NIL;

	k = l = 1;
	match = THRESHOLD - 1;
	p = &root[_G(lzbuffer)[i]];
	lson[i] = rson[i] = NIL;
	while ((j = *p) != NIL) {
		for (n = min(k, l); n < run && (c = (_G(lzbuffer)[j + n] - _G(lzbuffer)[i + n])) == 0; n++);

		if (n > match) {
			match = n;
			_G(pos) = j;
		}

		if (c < 0) {
			p = &lson[j];
			k = n;
		} else if (c > 0) {
			p = &rson[j];
			l = n;
		} else {
			dad[j] = NIL;
			dad[lson[j]] = lson + i - _G(node);
			dad[rson[j]] = rson + i - _G(node);
			lson[i] = lson[j];
			rson[i] = rson[j];
			break;
		}
	}

	dad[i] = p - _G(node);
	*p = i;
	return match;
}

void _delete(int z) {
	int j;

	if (dad[z] != NIL) {
		if (rson[z] == NIL)
			j = lson[z];
		else if (lson[z] == NIL)
			j = rson[z];
		else {
			j = lson[z];
			if (rson[j] != NIL) {
				do {
					j = rson[j];
				} while (rson[j] != NIL);

				_G(node)[dad[j]] = lson[j];
				dad[lson[j]] = dad[j];
				lson[j] = lson[z];
				dad[lson[z]] = lson + j - _G(node);
			}

			rson[j] = rson[z];
			dad[rson[z]] = rson + j - _G(node);
		}

		dad[j] = dad[z];
		_G(node)[dad[z]] = j;
		dad[z] = NIL;
	}
}

bool lzwcompress(Stream *lzw_in, Stream *out) {
	int ch, i, run, len, match, size, mask;
	uint8_t buf[17];

	_G(lzbuffer) = (uint8_t *)malloc(N + F + (N + 1 + N + N + 256) * sizeof(int));       // 28.5 k !
	if (_G(lzbuffer) == nullptr) {
		return false;
	}

	_G(node) = (int *)(_G(lzbuffer) + N + F);
	for (i = 0; i < 256; i++)
		root[i] = NIL;

	for (i = NIL; i < N; i++)
		dad[i] = NIL;

	size = mask = 1;
	buf[0] = 0;
	i = N - F - F;

	for (len = 0; len < F && (ch = lzw_in->ReadByte()) != -1; len++) {
		_G(lzbuffer)[i + F] = static_cast<uint8_t>(ch);
		i = (i + 1) & (N - 1);
	}

	run = len;

	do {
		ch = lzw_in->ReadByte();
		if (i >= N - F) {
			_delete(i + F - N);
			_G(lzbuffer)[i + F] = _G(lzbuffer)[i + F - N] = static_cast<uint8_t>(ch);
		} else {
			_delete(i + F);
			_G(lzbuffer)[i + F] = static_cast<uint8_t>(ch);
		}

		match = insert(i, run);
		if (ch == -1) {
			run--;
			len--;
		}

		if (len++ >= run) {
			if (match >= THRESHOLD) {
				buf[0] |= mask;
				// possible fix: change int* to short* ??
				*(short *)(buf + size) = static_cast<short>(((match - 3) << 12) | ((i - _G(pos) - 1) & (N - 1)));
				size += 2;
				len -= match;
			} else {
				buf[size++] = _G(lzbuffer)[i];
				len--;
			}

			if (!((mask += mask) & 0xFF)) {
				out->Write(buf, size);
				_G(outbytes) += size;
				size = mask = 1;
				buf[0] = 0;
			}
		}
		i = (i + 1) & (N - 1);
	} while (len > 0);

	if (size > 1) {
		out->Write(buf, size);
		_G(outbytes) += size;
	}

	free(_G(lzbuffer));
	return true;
}

bool lzwexpand(const uint8_t *src, size_t src_sz, uint8_t *dst, size_t dst_sz) {
	int bits, ch, i, j, len, mask;
	uint8_t *dst_ptr = dst;
	const uint8_t *src_ptr = src;

	if (dst_sz == 0)
		return false; // nowhere to expand to

	_G(lzbuffer) = (uint8_t *)malloc(N);
	if (_G(lzbuffer) == nullptr) {
		return false;  // not enough memory
	}
	i = N - F;

	// Read from the src and expand, until either src or dst runs out of space
	while ((static_cast<size_t>(src_ptr - src) < src_sz) &&
		   (static_cast<size_t>(dst_ptr - dst) < dst_sz)) {
		bits = *(src_ptr++);
		for (mask = 0x01; mask & 0xFF; mask <<= 1) {
			if (bits & mask) {
				if (static_cast<size_t>(src_ptr - src) > (src_sz - sizeof(int16_t)))
					break;

				short jshort = 0;
				jshort = Memory::ReadInt16LE(src_ptr);
				src_ptr += sizeof(int16_t);
				j = jshort;

				len = ((j >> 12) & 15) + 3;
				j = (i - j - 1) & (N - 1);

				if (static_cast<size_t>(dst_ptr - dst) > (dst_sz - len))
					break; // not enough dest buffer

				while (len--) {
					*(dst_ptr++) = (_G(lzbuffer)[i] = _G(lzbuffer)[j]);
					j = (j + 1) & (N - 1);
					i = (i + 1) & (N - 1);
				}
			} else {
				ch = *(src_ptr++);
				*(dst_ptr++) = (_G(lzbuffer)[i] = static_cast<uint8_t>(ch));
				i = (i + 1) & (N - 1);
			}

			if ((static_cast<size_t>(dst_ptr - dst) >= dst_sz) ||
				(static_cast<size_t>(src_ptr - src) >= src_sz)) {
				break; // not enough dest buffer for the next pass
			}
		} // end for mask

	}

	free(_G(lzbuffer));
	return static_cast<size_t>(src_ptr - src) == src_sz;
}

} // namespace AGS3
