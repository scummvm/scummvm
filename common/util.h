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

#ifndef ABS
#define ABS(x) ((x)>=0?(x):-(x))
#endif

#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif


int RGBMatch(byte *palette, int r, int g, int b);
int Blend(int src, int dst, byte *palette);
void ClearBlendCache(byte *palette, int weight);

/*
 * Print hexdump of the data passed in, 8 bytes a row
 */
void hexdump(const byte * data, int len);


namespace ScummVM {

template <class T>
class List {
protected:
	int		_capacity;
	int		_size;
	T		*_data;

public:
	List<T>() : _capacity(0), _size(0), _data(0) {}
	List<T>(const List<T>& list) : _capacity(0), _size(0), _data(0)
	{
		_size = list._size;
		_capacity = _size + 32;
		_data = new T[_capacity];
		for (int i = 0; i < _size; i++)
			_data[i] = list._data[i];
	}
	
	~List<T>()
	{
		if (_data)
			delete [] _data;
	}
	
	void push_back(const T& str)
	{
		ensureCapacity(_size + 1);
		_data[_size++] = str;
	}
	
	// TODO: insert, remove, ...
	
	T& operator [](int idx)
	{
		assert(idx >= 0 && idx < _size);
		return _data[idx];
	}

	const T& operator [](int idx) const
	{
		assert(idx >= 0 && idx < _size);
		return _data[idx];
	}

	List<T>& operator  =(const List<T>& list)
	{
		if (_data)
			delete [] _data;
		_size = list._size;
		_capacity = _size + 32;
		_data = new T[_capacity];
		for (int i = 0; i < _size; i++)
			_data[i] = list._data[i];

		return *this;
	}

	int size() const	{ return _size; }

	void clear()
	{
		if (_data) {
			delete [] _data;
			_data = 0;
		}
		_size = 0;
		_capacity = 0;
	}
	
	bool isEmpty() const	{ return (_size == 0); }

protected:
	void ensureCapacity(int new_len)
	{
		if (new_len <= _capacity)
			return;
	
		T *old_data = _data;
		_capacity = new_len + 32;
		_data = new T[_capacity];
	
		if (old_data) {
			// Copy old data
			for (int i = 0; i < _size; i++)
				_data[i] = old_data[i];
			delete [] old_data;
		}
	}
};


class String {
protected:
	int		*_refCount;
	int		_capacity;
	int		_len;
	char	*_str;
public:
	String() : _capacity(0), _len(0), _str(0) { _refCount = new int(1); }
	String(const char *str);
	String(const String &str);
	~String();
	
	String& operator  =(const char* str);
	String& operator  =(const String& str);
	String& operator +=(const char* str);
	String& operator +=(const String& str);
	String& operator +=(char c);

	bool operator ==(const String& x);
	bool operator ==(const char* x);
	bool operator !=(const String& x);
	bool operator !=(const char* x);

//	operator char *()				{ return _str; }
	operator const char *()	const	{ return _str; }
	const char *c_str() const		{ return _str; }
	int size() const				{ return _len; }
	
	void deleteLastChar();
	void clear();

	bool isEmpty() const	{ return (_len == 0); }

protected:
	void ensureCapacity(int new_len, bool keep_old);
	void decRefCount();
};

// Some useful additional comparision operators for Strings
bool operator == (const char* x, const String& y);
bool operator != (const char* x, const String& y);

class StringList : public List<String> {
public:
	void push_back(const char* str)
	{
		ensureCapacity(_size + 1);
		_data[_size] = str;
		_size++;
	}
};

};	// End of namespace ScummVM

#endif
