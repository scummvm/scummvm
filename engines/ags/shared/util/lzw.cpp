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
// LZW compression -- the LZW/GIF patent has expired, so we can use it now!!!
//
//=============================================================================

#include "ags/shared/util/lzw.h"
#include "ags/shared/ac/common.h" // quit
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
	p = &root[(unsigned char)_G(lzbuffer)[i]];
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
	char buf[17];

	_G(lzbuffer) = (char *)malloc(N + F + (N + 1 + N + N + 256) * sizeof(int));       // 28.5 k !
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
		_G(lzbuffer)[i + F] = ch;
		i = (i + 1) & (N - 1);
	}

	run = len;

	do {
		ch = lzw_in->ReadByte();
		if (i >= N - F) {
			_delete(i + F - N);
			_G(lzbuffer)[i + F] = _G(lzbuffer)[i + F - N] = ch;
		} else {
			_delete(i + F);
			_G(lzbuffer)[i + F] = ch;
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
				*(short *)(buf + size) = ((match - 3) << 12) | ((i - _G(pos) - 1) & (N - 1));
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

void myputc(int ccc, Stream *out) {
	if (_G(maxsize) > 0) {
		_G(putbytes)++;
		if (_G(putbytes) > _G(maxsize))
			return;
	}

	_G(outbytes)++;
	out->WriteInt8(ccc);
}

bool lzwexpand(Stream *lzw_in, Stream *out, size_t out_size) {
	int bits, ch, i, j, len, mask;
	char *lzbuffer;
	_G(outbytes) = 0; _G(putbytes) = 0;
	_G(maxsize) = out_size;

	lzbuffer = (char *)malloc(N);
	if (lzbuffer == nullptr) {
		return false;
	}
	i = N - F;

	// this end condition just checks for EOF, which is no good to us
	while ((bits = lzw_in->ReadByte()) != -1) {
		for (mask = 0x01; mask & 0xFF; mask <<= 1) {
			if (bits & mask) {
				short jshort = 0;
				jshort = lzw_in->ReadInt16();
				j = jshort;

				len = ((j >> 12) & 15) + 3;
				j = (i - j - 1) & (N - 1);

				while (len--) {
					myputc(lzbuffer[i] = lzbuffer[j], out);
					j = (j + 1) & (N - 1);
					i = (i + 1) & (N - 1);
				}
			} else {
				ch = lzw_in->ReadByte();
				myputc(lzbuffer[i] = ch, out);
				i = (i + 1) & (N - 1);
			}

			if ((_G(putbytes) >= _G(maxsize)) && (_G(maxsize) > 0))
				break;

			if ((lzw_in->EOS()) && (_G(maxsize) > 0)) {
				free(lzbuffer);
				return false;
			}
		}                           // end for mask

		if ((_G(putbytes) >= _G(maxsize)) && (_G(maxsize) > 0))
			break;
	}

	free(lzbuffer);
	return true;
}

} // namespace AGS3
