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

#include <stdafx.h>
#include "blitter.h"
#include "chunck.h"

#include <assert.h>
#include <string.h> // for memcpy

#ifndef min
#define min(x, y) ((x) > (y) ? (y) : (x))
#endif

Blitter::Blitter(char * ptr, const Point & dstsize, const Rect & src) : 
			_ptr(ptr), 
			_clip(dstsize), 
			_src(src),
			_cur(src.left(), src.top()),
			_outside(false) {
#ifdef DEBUG_CLIPPER
	_clipped = 0;
	_clippedBlock = 0;
#endif
	assert(_ptr);
	assert(_clip.getX() > 0 && _clip.getY() > 0);
	assert(_src.width() > 0 && _src.height() > 0);
	assert(_src.left() < _clip.getX() && _src.right() <= _clip.getX());
	assert(_src.top() < _clip.getY() && _src.bottom() <= _clip.getY());
	_offset = _ptr + _clip.getX() * _cur.getY()  + _cur.getX();
}

Blitter::~Blitter() {
#ifdef DEBUG_CLIPPER
	if(_clipped || _clippedBlock) {
		debug(3, "blitter clipped %d pixels and %d blocks", _clipped, _clippedBlock);
	}
#endif
}

void Blitter::advance(int x, int y) {
	if(y != 0) {
		_cur.set(_src.left() + x, _cur.getY() + y);
	} else {
		_cur.getX() += x;
		if(_cur.getX() >= _src.right()) {
			_cur.set(_src.left(), _cur.getY()+1);
		}
	}
	_offset = _ptr + _clip.getX() * _cur.getY()  + _cur.getX();
	_outside = ! _src.isInside(_cur); 
}

void Blitter::advanceBlock(int x, int y) {
	advance(x*4, y*4);
}

void Blitter::put(char data) {
	if(!_outside) {
		*_offset = data;
		advance();
	}
#ifdef DEBUG_CLIPPER
	else	_clipped ++;
#endif
}

void Blitter::put(char data, unsigned int len) {
	while(len) {
		if(_outside) {
#ifdef DEBUG_CLIPPER
			_clipped += len;
#endif
			break;
		}
		int l = min((int)len, min(_clip.getX() - _cur.getX(), _src.right() - _cur.getX()));
		len -= l;
		memset(_offset, data, l);
		advance(l);
	}
}

void Blitter::blit(char * ptr, unsigned int len) {
	while(len) {
		if(_outside) {
#ifdef DEBUG_CLIPPER
			_clipped += len;
#endif
			break;
		}
		int l = min((int)len, min(_clip.getX() - _cur.getX(), _src.right() - _cur.getX()));
		len -= l;
		memcpy(_offset, ptr, l);
		ptr += l;
		advance(l);
	}
}

void Blitter::blit(Chunck & src, unsigned int len) {
	while(len) {
		if(_outside) {
#ifdef DEBUG_CLIPPER
			_clipped += len;
#endif
			break;
		}
		int l = min((int)len, min(_clip.getX() -_cur.getX(), _src.right() - _cur.getX()));
		len -= l;
		src.read(_offset, l);
		advance(l);
	}
}

void Blitter::putBlock(unsigned int data) {
	if(_cur.getX() + 3 < _src.right() && _cur.getY() + 3 < _src.bottom()) { // This is clipping
		assert((_clip.getX() & 3) == 0);
		unsigned int * dst = (unsigned int *)_offset;
		int line_size = _clip.getX() >> 2;

		*dst = data; dst += line_size;
		*dst = data; dst += line_size;
		*dst = data; dst += line_size;
		*dst = data;

#ifdef DEBUG_CLIPPER
	} else {
		_clippedBlock ++;
#endif
	}
	advanceBlock();
}

void Blitter::putBlock(unsigned int d1, unsigned int d2, unsigned int d3, unsigned int d4) {
	if(_cur.getX() + 3 < _src.right() && _cur.getY() + 3 < _src.bottom()) { // This is clipping
		assert((_clip.getX() & 3) == 0);
		unsigned int * dst = (unsigned int *)_offset;
		int line_size = _clip.getX() >> 2;

		*dst = d4; dst += line_size;
		*dst = d3; dst += line_size;
		*dst = d2; dst += line_size;
		*dst = d1;

#ifdef DEBUG_CLIPPER
	} else {
		_clippedBlock ++;
#endif
	}
	advanceBlock();
}

void Blitter::putBlock(unsigned char * data) {
	if(_cur.getX() + 3 < _src.right() && _cur.getY() + 3 < _src.bottom()) { // This is clipping
		assert((_clip.getX() & 3) == 0);
		unsigned int * dst =  (unsigned int *)_offset;
		int line_size = _clip.getX() >> 2;
		unsigned int * src =  (unsigned int *)data;
		*dst = *src++; dst += line_size; 
		*dst = *src++; dst += line_size;
		*dst = *src++; dst += line_size;
		*dst = *src++;
#ifdef DEBUG_CLIPPER
	} else {
		_clippedBlock ++;
#endif
	}
	advanceBlock();
}

void Blitter::putBlock(Chunck & src) {
	if(_cur.getX() + 3 < _src.right() && _cur.getY() + 3 < _src.bottom()) { // This is clipping
		assert((_clip.getX() & 3) == 0);
		unsigned int * dst =  (unsigned int *)_offset;
		int line_size = _clip.getX() >> 2;
		*dst = src.getDword(); dst += line_size;
		*dst = src.getDword(); dst += line_size;
		*dst = src.getDword(); dst += line_size;
		*dst = src.getDword();
#ifdef DEBUG_CLIPPER
	} else {
		_clippedBlock ++;
#endif
	}
	advanceBlock();
}

void Blitter::blockCopy(int offset) {
	if(_cur.getX() + 3 < _src.right() && _cur.getY() + 3 < _src.bottom()) {// This is clipping
		char  * dst = _offset;
		*((unsigned int *)dst) = *((unsigned int *)(dst + offset));
		dst += _clip.getX();
		*((unsigned int *)dst) = *((unsigned int *)(dst + offset));
		dst += _clip.getX();
		*((unsigned int *)dst) = *((unsigned int *)(dst + offset));
		dst += _clip.getX();
		*((unsigned int *)dst) = *((unsigned int *)(dst + offset));
#ifdef DEBUG_CLIPPER
	} else {
		_clippedBlock ++;
#endif
	}
	advanceBlock();
}
