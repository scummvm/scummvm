/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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

#ifndef DECODER_H
#define DECODER_H

#include "config.h"

#include "common/rect.h"

using ScummVM::Point;
using ScummVM::Rect;

/*!	@brief base class for codec decompression.

	This class provides an interface for codec decompression.

*/
class Decoder {
private:
	Rect _r;			//!< current size of the frame object to decode
	Point _p;			//!< position of the frame object to decode
protected:
	const Rect & getRect() const{ return _r; }
	const Point & getSize() const { return _p; }
public:
	Decoder() {};
	virtual ~Decoder() {};
	virtual bool initSize(const Point &p, const Rect &r) { _p = p; _r = r; return true; };
	virtual bool decode(byte *dst, const byte *src, int length) = 0;
};

#endif
