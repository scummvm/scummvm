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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/array.h"
#include "common/scummsys.h"

#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/extern.h"
#include "trecision/trecision.h"

#define FAST_COOKIE 0xFA57F00D

namespace Trecision {

void decompress(const unsigned char *src, unsigned src_len, unsigned char *dst, unsigned dst_len) {
	unsigned short *sw = (unsigned short *)(src + src_len);
	unsigned char *d = dst;
	const unsigned char *s = src;
	unsigned short ctrl = 0, ctrl_cnt = 1;

	while (s < (const unsigned char *)sw) {
		if (!--ctrl_cnt) {
			ctrl = *--sw;
			ctrl_cnt = 16;
		} else {
			ctrl <<= 1;
		}

		if (ctrl & 0x8000) {
			unsigned foo = *--sw;
			const unsigned char *cs = d - (foo >> 4);

			switch (foo & 0xF) {
			case 0:
				*d++ = *cs++;
			case 1:
				*d++ = *cs++;
			case 2:
				*d++ = *cs++;
			case 3:
				*d++ = *cs++;
			case 4:
				*d++ = *cs++;
			case 5:
				*d++ = *cs++;
			case 6:
				*d++ = *cs++;
			case 7:
				*d++ = *cs++;
			case 8:
				*d++ = *cs++;
			case 9:
				*d++ = *cs++;
			case 10:
				*d++ = *cs++;
			case 11:
				*d++ = *cs++;
			case 12:
				*d++ = *cs++;
			case 13:
				*d++ = *cs++;
			case 14:
				*d++ = *cs++;
			case 15:
				*d++ = *cs++;

				*d++ = *cs++;
				*d++ = *cs++;
			}
		} else {
			*d++ = *s++;
		}
	}
}

uint32 DecCR(Common::String FileName, uint8 *DestArea, uint8 *DecArea) {
	ff = FastFileOpen(FileName.c_str());
	if (ff == nullptr)
		error("File not found %s", FileName.c_str());

	char *ibuf = (char *)DecArea;
	char *obuf = (char *)DestArea;

	int isize = FastFileLen(ff);

	if ((isize != FastFileRead(ff, ibuf, isize)))
		CloseSys(g_vm->_sysText[kMessageUnknownError]);
	FastFileClose(ff);

	if (*(unsigned *)ibuf != FAST_COOKIE)
		CloseSys(g_vm->_sysText[kMessageErrorReadingFile]);

	ibuf += sizeof(unsigned);
	isize -= sizeof(unsigned);

	int osize = *(unsigned *)ibuf;
	ibuf += sizeof(unsigned);
	isize -= sizeof(unsigned);

	if (isize < osize)
		decompress((unsigned char *)ibuf, isize, (unsigned char *)obuf, osize);
	else {
		isize -= sizeof(unsigned);
		memcpy(obuf, ibuf, isize);
	}

	return osize;
}

} // End of namespace Trecision
