/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef __CODEC37_H_
#define __CODEC37_H_

#include "config.h"

#ifdef DEBUG
# ifndef NO_DEBUG_CODEC37
#  define DEBUG_CODEC37
# endif
#else
# ifdef DEBUG_CODEC37
#  error DEBUG_CODEC37 defined without DEBUG
# endif
#endif

#ifdef DEBUG_CODEC37
# ifndef NO_DEBUG_CODEC37_PROCS
#  define DEBUG_CODEC37_PROC1
#  define DEBUG_CODEC37_PROC2
#  define DEBUG_CODEC37_PROC3
#  define DEBUG_CODEC37_PROC4
# endif
#endif

#include "decoder.h"

/*!	@brief ::decoder for codec 37.

*/

#define DELTA_ADD 0x3E00	// what is this 0x3E00 ?? == 320*200/4 - 128
				// It looks like it is a safe-guarding protection from bugs., but maybe not...

class Codec37Decoder : public Decoder {
private:
	int _deltaSize;
	unsigned char * _deltaBufs[2];
	unsigned char * _deltaBuf;
	short * _offsetTable;
	int _curtable;
	unsigned short _prevSeqNb;
	int _tableLastPitch;
	int _tableLastIndex;

public:
	bool initSize(const Point &, const Rect &);
	Codec37Decoder();
	void clean();
	virtual ~Codec37Decoder();
protected:
	static inline unsigned int expand(unsigned char b) {
		unsigned int r = b | (b << 8);
		return r | (r << 16);
	}
	void maketable(int, int);
	void proc1(Blitter &, Chunck &, int, int, int, int);
	void proc2(Blitter &, Chunck &, int);
	void proc3WithFDFE(Blitter &, Chunck &, int, int, int);
	void proc3WithoutFDFE(Blitter &, Chunck &, int, int, int);
	void proc4(Blitter &, Chunck &, int, int, int);
public:
	bool decode(Blitter &, Chunck &);
};

#endif
