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

#ifndef UTIL_H
#define UTIL_H

#include "scummsys.h"


int RGBMatch(byte *palette, int r, int g, int b);
int Blend(int src, int dst, byte *palette);
void ClearBlendCache(byte *palette, int weight);


class String {
protected:
	int		_capacity;
	int		_len;
	char	*_str;
public:
	String() : _capacity(0), _len(0), _str(0) {}
	String(const char *str);
	String(const String &str);
	~String();
	
	String& operator  =(const char* str);
	String& operator  =(const String& str);
	String& operator +=(const char* str);
	String& operator +=(const String& str);
	String& operator +=(char c);

//	operator char *()				{ return _str; }
	operator const char *()	const	{ return _str; }
	const char *c_str() const		{ return _str; }
	int size() const				{ return _len; }

protected:
	void ensureCapacity(int new_len, bool keep_old);
};

class StringList {
protected:
	int		_capacity;
	int		_size;
	String	**_data;
public:
	StringList() : _capacity(0), _size(0), _data(0) {}
	StringList(const StringList& list);
	~StringList();
	
	void push_back(const char* str);
	void push_back(const String& str);
	
	// TODO: insert, remove, ...
	
	String& operator [](int idx);
	const String& operator [](int idx) const;

	int size() const	{ return _size; }

protected:
	void ensureCapacity(int new_len);
};


#endif
