/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "util.h"

// 8-bit alpha blending routines
int BlendCache[256][256];

int RGBMatch(byte *palette, int r, int g, int b)
{
	int i, bestidx = 0, besterr = 0xFFFFFF;
	int error = 0;

	for (i = 0;i < 256;i++) {
		byte *pal = palette + (i * 3);
		int r_diff = r - (int)*pal++; 
		int g_diff = g - (int)*pal++; 
		int b_diff = b - (int)*pal++; 
		r_diff *= r_diff; g_diff *= g_diff; b_diff *= b_diff;

		error = r_diff + g_diff + b_diff;
		if (error < besterr) {
			besterr = error;
			bestidx = i;
		}
	}
	return bestidx;
}

int Blend(int src, int dst, byte *palette)
{
	int r, g, b;
	int alpha = 128;	// Level of transparency [0-256]
	byte *srcpal = palette + (dst  * 3);
	byte *dstpal = palette + (src * 3);

	if (BlendCache[dst][src] > -1)
		return BlendCache[dst][src];

	r =  (*srcpal++ * alpha);
    r += (*dstpal++ * (256-alpha));
    r /= 256;

    g =  (*srcpal++ * alpha);
    g += (*dstpal++ * (256-alpha));
    g /= 256;

    b =  (*srcpal++ * alpha);
    b += (*dstpal++  * (256-alpha));
    b /= 256;
       
	return (BlendCache[dst][src] = RGBMatch(palette, r , g , b ));
}

void ClearBlendCache(byte *palette, int weight)
{
	for (int i = 0; i < 256; i++)
		for (int j = 0 ; j < 256 ; j++)			
//			BlendCache[i][j] = i;	// No alphablending
//			BlendCache[i][j] = j;	// 100% translucent
			BlendCache[i][j] = -1;	// Enable alphablending
}


#pragma mark -


String::String(const char *str)
{
	_capacity = _len = strlen(str);
	_str = (char *)calloc(1, _capacity+1);
	memcpy(_str, str, _len+1);
}

String::String(const String &str)
{
	_capacity = str._capacity;
	_len = str._len;
	_str = (char *)calloc(1, _capacity+1);
	memcpy(_str, str._str, _len+1);
}

String::~String()
{
	if (_str)
		free(_str);
}

String& String::operator  =(const char* str)
{
	int len = strlen(str);
	ensureCapacity(len, false);
	
	_len = len;
	memcpy(_str, str, _len + 1);

	return *this;
}

String& String::operator  =(const String& str)
{
	int len = str._len;
	ensureCapacity(len, false);
	
	_len = len;
	memcpy(_str, str._str, _len + 1);

	return *this;
}

String& String::operator +=(const char* str)
{
	int len = strlen(str);
	ensureCapacity(_len + len, true);
	
	memcpy(_str + _len, str, len + 1);
	_len += len;

	return *this;
}

String& String::operator +=(const String& str)
{
	int len = str._len;
	ensureCapacity(_len + len, true);
	
	memcpy(_str + _len, str._str, len + 1);
	_len += len;

	return *this;
}

String& String::operator +=(char c)
{
	int len = _len + 1;
	ensureCapacity(len, true);
	
	_str[_len++] = c;
	_str[_len] = 0;

	return *this;
}

void String::clear()
{
	_len = 0;
	if (_str)
		_str[0] = 0;
}

void String::ensureCapacity(int new_len, bool keep_old)
{
	if (new_len <= _capacity)
		return;

	char	*old_str = _str;
	_capacity = new_len + 32;
	_str = (char *)calloc(1, _capacity+1);

	if (old_str) {
		if (keep_old)
			memcpy(_str, old_str, _len+1);
		free(old_str);
	}
}


#pragma mark -


StringList::StringList(const StringList& list)
	: _capacity(0), _size(0), _data(0)
{
	printf("EEEEK! StringList copy constructor called!\n");
	assert(0);
}

StringList::~StringList()
{
	if (_data) {
		for (int i = 0; i < _capacity; i++)
			if (_data[_size])
				delete _data[_size];
		free(_data);
	}
}


void StringList::push_back(const char* str)
{
	ensureCapacity(_size + 1);
	
	if (!_data[_size])
		_data[_size] = new String(str);
	else
		*_data[_size] = str;

	_size++;
}

void StringList::push_back(const String& str)
{
	ensureCapacity(_size + 1);
	
	if (!_data[_size])
		_data[_size] = new String(str);
	else
		*_data[_size] = str;

	_size++;
}

String& StringList::operator [](int idx)
{
	assert(idx >= 0 && idx < _size);
	return *_data[idx];
}

const String& StringList::operator [](int idx) const
{
	assert(idx >= 0 && idx < _size);
	return *_data[idx];
}

void String::clear()
{
	_len = 0;
}

void StringList::ensureCapacity(int new_len)
{
	if (new_len <= _capacity)
		return;

	String	**old_data = _data;
	_capacity = new_len + 32;
	_data = (String **)calloc(sizeof(String*), _capacity);

	if (old_data) {
		memcpy(_data, old_data, _size * sizeof(String*));
		free(old_data);
	}
}
