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

#ifndef COMMON_STRING_H
#define COMMON_STRING_H

#include "scummsys.h"
#include "common/list.h"

namespace ScummVM {

/*
 TODO
 Add a class ConstString which is a light weight base class of String.
 It will be immutable, and *not* copy the char pointer it gets when created.
 Only constructor: ConstString(const char *ptr)
 Then whenever you now use "const String &" in a parameter, use "const ConstString &"
 instead (mayhaps make a typedef even). Thus, parameters are passed w/o 
 causing a free/malloc. Then only for permenant storage, when we assign it to a
 real String object, will a malloc be issued (to this end, make String aware of
 ConstString ?!?
*/

class ConstString {
protected:
	char	*_str;
	int		_len;

public:
	ConstString() : _str(0), _len(0) {}
	ConstString(const char *str) : _str((char*)str) { _len = str ? strlen(str) : 0;}
	virtual ~ConstString() {}
	
	bool operator ==(const ConstString& x) const;
	bool operator ==(const char* x) const;
	bool operator !=(const ConstString& x) const;
	bool operator !=(const char* x) const;
	bool operator <(const ConstString& x) const;
	bool operator <=(const ConstString& x) const;
	bool operator >(const ConstString& x) const;
	bool operator >=(const ConstString& x) const;

	const char *c_str() const		{ return _str; }
	int size() const				{ return _len; }

	bool isEmpty() const	{ return (_len == 0); }
};

class String : public ConstString {
protected:
	int		*_refCount;
	int		_capacity;

public:
	String() : _capacity(0) { _refCount = new int(1); }
	String(const char *str);
	String(const String &str);
	virtual ~String();
	
	String& operator  =(const char* str);
	String& operator  =(const String& str);
	String& operator +=(const char* str);
	String& operator +=(const String& str);
	String& operator +=(char c);

	void deleteLastChar();
	void clear();

protected:
	void ensureCapacity(int new_len, bool keep_old);
	void decRefCount();
};

// Some useful additional comparision operators for Strings
bool operator == (const char* x, const ConstString& y);
bool operator != (const char* x, const ConstString& y);

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
