/*
Copyright (C) 2003-2004 The Pentagram Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// istring.h -- case insensitive stl strings

#ifndef ULTIMA8_MISC_ISTRING_H
#define ULTIMA8_MISC_ISTRING_H

#include "ultima8/std/misc.h"
#include "ultima8/std/string.h"

#ifdef strcasecmp
#undef strcasecmp
#endif

#ifdef strncasecmp
#undef strncasecmp
#endif

namespace Ultima8 {
namespace Pentagram {

extern int strcasecmp(const char *s1, const char *s2);
extern int strncasecmp(const char *s1, const char *s2, uint32 length);

//! Safe strcpy with size checking
void strcpy_s(char *dest, size_t size, const char *src);
//! Safe strcpy with size checking from dest array size
template<size_t size> inline void strcpy_s(char (& dest)[size], const char *src) {
	strcpy_s(dest, size, src);
}

//! Safe strcat with size checking
inline char *strcat_s(char *dest, size_t size, const char *src) {
	size_t cur = std::strlen(dest);
	if (cur < size) strcpy_s(dest + cur, size - cur, src);
	return dest;
}
//! Safe strcat with size checking from dest array size
template<size_t size> inline char (&strcat_s(char (& dest)[size], const char *src))[size]  {
	size_t cur = std::strlen(dest);
	if (cur < size) strcpy_s(dest + cur, size - cur, src);
	return dest;
}

#if 0

//! STL char_traits for case insensitive comparisons
struct ichar_traits : public std::char_traits<char> {
	//! Case insensitive check for 2 chars being equal
	static bool eq(const char_type &c1, const char_type &c2);

	//! Case insensitive check for the first char being less
	//! than the second char
	static bool lt(const char_type &c1, const char_type &c2);

	//! Case insensitive comparison for 2 c_strings - based off Q_strncasecmp
	static int compare(const char_type *s1, const char_type *s2, size_t length);
};

template<class _Elem,
         class _Traits = std::char_traits<_Elem>,
         class _Ax = std::allocator<_Elem> >
class pent_string
	: public std::basic_string<_Elem, _Traits, _Ax> {
	// null-terminated transparent array of elements
public:
	typedef pent_string<_Elem, _Traits, _Ax> _Myt;
	typedef std::basic_string<_Elem, _Traits, _Ax> _Mybase;
	typedef _Ax _Alloc;
	typedef typename _Alloc::size_type size_type;
	typedef typename _Alloc::difference_type difference_type;
	typedef typename _Alloc::pointer _Tptr;
	typedef typename _Alloc::const_pointer _Ctptr;
	typedef _Tptr pointer;
	typedef _Ctptr const_pointer;
	typedef typename _Alloc::reference reference;
	typedef typename _Alloc::const_reference const_reference;
	typedef typename _Alloc::value_type value_type;

	typedef ichar_traits traits_type;

	pent_string()
		: _Mybase() {
	}

	explicit pent_string(const _Alloc &_Al)
		: _Mybase(_Al) {
	}

	pent_string(const _Mybase &_Right)
		: _Mybase(_Right) {
		// construct by copying _Right
	}

	pent_string(const _Mybase &_Right, size_type _Roff,
	            size_type _Count = _Mybase::npos)
		: _Mybase(_Right, _Roff, _Count) {
		// construct from _Right [_Roff, _Roff + _Count)
	}

	pent_string(const _Mybase &_Right, size_type _Roff, size_type _Count,
	            const _Alloc &_Al)
		: _Mybase(_Right, _Roff, _Count, _Al) {
		// construct from _Right [_Roff, _Roff + _Count) with allocator
	}

	pent_string(const _Elem *_Ptr, size_type _Count)
		: _Mybase(_Ptr, _Count) {
		// construct from [_Ptr, _Ptr + _Count)
	}

	pent_string(const _Elem *_Ptr, size_type _Count, const _Alloc &_Al)
		: _Mybase(_Ptr, _Count, _Al) {
		// construct from [_Ptr, _Ptr + _Count) with allocator
	}

	pent_string(const _Elem *_Ptr)
		: _Mybase(_Ptr) {
		// construct from [_Ptr, <null>)
	}

	pent_string(const _Elem *_Ptr, const _Alloc &_Al)
		: _Mybase(_Ptr, _Al) {
		// construct from [_Ptr, <null>) with allocator
	}

	pent_string(size_type _Count, _Elem _Ch)
		: _Mybase(_Count, _Ch) {
		// construct from _Count * _Ch
	}

	pent_string(size_type _Count, _Elem _Ch, const _Alloc &_Al)
		: _Mybase(_Count, _Ch, _Al) {
		// construct from _Count * _Ch with allocator
	}

	template<class _It>
	pent_string(_It _First, _It _Last)
		: _Mybase(_First, _Last) {
		// construct from [_First, _Last)
	}

	template<class _It>
	pent_string(_It _First, _It _Last, const _Alloc &_Al)
		: _Mybase(_First, _Last, _Al) {
		// construct from [_First, _Last) with allocator
	}

	//
	// Overloaded operators (will return the correct type)
	//

	_Myt &operator=(const _Myt &_Right) {
		// assign _Right
		this->assign(_Right);
		return (*this);
	}

	_Myt &operator=(const _Elem *_Ptr) {
		// assign [_Ptr, <null>)
		this->assign(_Ptr);
		return (*this);
	}

	_Myt &operator=(_Elem _Ch) {
		// assign 1 * _Ch
		this->assign(1, _Ch);
		return (*this);
	}

	_Myt &operator+=(const _Myt &_Right) {
		// append _Right
		this->append(_Right);
		return (*this);
	}

	_Myt &operator+=(const _Elem *_Ptr) {
		// append [_Ptr, <null>)
		this->append(_Ptr);
		return (*this);
	}

	_Myt &operator+=(_Elem _Ch) {
		// append 1 * _Ch
		this->append(static_cast<size_type>(1), _Ch);
		return (*this);
	}

	// New case insensitive compare functions

	int compare(const _Mybase &_Right) const {
		// compare [0, _Mysize) with _Right
		return (compare(0, this->size(), _Right.data(), _Right.size()));
	}

	int compare(size_type _Off, size_type _N0,
	            const _Mybase &_Right) const {
		// compare [_Off, _Off + _N0) with _Right
		return (compare(_Off, _N0, _Right, 0, _Mybase::npos));
	}

	int compare(size_type _Off, size_type _N0, const _Myt &_Right,
	            size_type _Roff, size_type _Count) const {
		// compare [_Off, _Off + _N0) with _Right [_Roff, _Roff + _Count)
		if (_Right.size() < _Roff)
			return 0; //_String_base::_Xran();  // _Off off end
		if (_Right.size() - _Roff < _Count)
			_Count = _Right.size() - _Roff; // trim _Count to size
		return (compare(_Off, _N0, _Right.data() + _Roff, _Count));
	}

	int compare(const _Elem *_Ptr) const {
		// compare [0, _Mysize) with [_Ptr, <null>)
		return (compare(0, this->size(), _Ptr, _Traits::length(_Ptr)));
	}

	int compare(size_type _Off, size_type _N0, const _Elem *_Ptr) const {
		// compare [_Off, _Off + _N0) with [_Ptr, <null>)
		return (compare(_Off, _N0, _Ptr, _Traits::length(_Ptr)));
	}

	int compare(size_type _Off, size_type _N0, const _Elem *_Ptr,
	            size_type _Count) const {
		// compare [_Off, _Off + _N0) with [_Ptr, _Ptr + _Count)
		if (this->size() < _Off)
			return 0; //_String_base::_Xran();  // _Off off end
		if (this->size() - _Off < _N0)
			_N0 = this->size() - _Off;  // trim _N0 to size

		size_type _Ans = _N0 == 0 ? 0
		                 : ichar_traits::compare(this->data() + _Off, _Ptr,
		                         _N0 < _Count ? _N0 : _Count);
		return (_Ans != 0 ? static_cast<int>(_Ans) : _N0 < _Count ? -1
		        : _N0 == _Count ? 0 : +1);
	}
};

//! STL case insensitve string
//template pent_string<char>;
typedef pent_string<char> istring;

inline pent_string<char> operator+(
    const pent_string<char> &_Left,
    const pent_string<char> &_Right) {
	// return string + string
	return (pent_string<char>(_Left) += _Right);
}

inline pent_string<char> operator+(
    const pent_string<char> &_Left,
    const std::basic_string<char> &_Right) {
	// return string + string
	return (pent_string<char>(_Left) += _Right);
}

inline pent_string<char> operator+(
    const std::basic_string<char> &_Left,
    const pent_string<char> &_Right) {
	// return string + string
	return (pent_string<char>(_Left) += _Right);
}

inline pent_string<char> operator+(
    const char *_Left,
    const pent_string<char> &_Right) {
	// return NTCS + string
	return (pent_string<char>(_Left) += _Right);
}

inline pent_string<char> operator+(
    const char _Left,
    const pent_string<char> &_Right) {
	// return character + string
	return (pent_string<char>(1, _Left) += _Right);
}

inline pent_string<char> operator+(
    const pent_string<char> &_Left,
    const char *_Right) {
	// return string + NTCS
	return (pent_string<char>(_Left) += _Right);
}

inline pent_string<char> operator+(
    const pent_string<char> &_Left,
    const char _Right) {
	// return string + character
	return (pent_string<char>(_Left) += _Right);
}


inline bool operator==(
    const pent_string<char> &_Left,
    const pent_string<char> &_Right) {
	// test for string equality
	return (_Left.compare(_Right) == 0);
}

inline bool operator==(
    const pent_string<char> &_Left,
    const std::basic_string<char> &_Right) {
	// test for string equality
	return (_Left.compare(_Right) == 0);
}

inline bool operator==(
    const std::basic_string<char> &_Left,
    const pent_string<char> &_Right) {
	// test for string equality
	return (_Right.compare(_Left) == 0);
}

inline bool operator==(
    const char *_Left,
    const pent_string<char> &_Right) {
	// test for NTCS vs. string equality
	return (_Right.compare(_Left) == 0);
}

inline bool operator==(
    const pent_string<char> &_Left,
    const char *_Right) {
	// test for string vs. NTCS equality
	return (_Left.compare(_Right) == 0);
}

inline bool operator!=(
    const pent_string<char> &_Left,
    const pent_string<char> &_Right) {
	// test for string inequality
	return (!(_Left == _Right));
}

inline bool operator!=(
    const pent_string<char> &_Left,
    const std::basic_string<char> &_Right) {
	// test for string inequality
	return (!(_Left == _Right));
}

inline bool operator!=(
    const std::basic_string<char> &_Left,
    const pent_string<char> &_Right) {
	// test for string inequality
	return (!(_Left == _Right));
}

inline bool operator!=(
    const char *_Left,
    const pent_string<char> &_Right) {
	// test for NTCS vs. string inequality
	return (!(_Left == _Right));
}

inline bool operator!=(
    const pent_string<char> &_Left,
    const char *_Right) {
	// test for string vs. NTCS inequality
	return (!(_Left == _Right));
}



inline bool operator<(
    const pent_string<char> &_Left,
    const pent_string<char> &_Right) {
	// test if string < string
	return (_Left.compare(_Right) < 0);
}

inline bool operator<(
    const pent_string<char> &_Left,
    const std::basic_string<char> &_Right) {
	// test if string < string
	return (_Left.compare(_Right) < 0);
}

inline bool operator<(
    const std::basic_string<char> &_Left,
    const pent_string<char> &_Right) {
	// test if string < string
	return (_Right.compare(_Left) > 0);
}

inline bool operator<(
    const char *_Left,
    const pent_string<char> &_Right) {
	// test if NTCS < string
	return (_Right.compare(_Left) > 0);
}

inline bool operator<(
    const pent_string<char> &_Left,
    const char *_Right) {
	// test if string < NTCS
	return (_Left.compare(_Right) < 0);
}



inline bool operator>(
    const pent_string<char> &_Left,
    const pent_string<char> &_Right) {
	// test if string > string
	return (_Right < _Left);
}

inline bool operator>(
    const pent_string<char> &_Left,
    const std::basic_string<char> &_Right) {
	// test if string > string
	return (_Right < _Left);
}

inline bool operator>(
    const std::basic_string<char> &_Left,
    const pent_string<char> &_Right) {
	// test if string > string
	return (_Right < _Left);
}

inline bool operator>(
    const char *_Left,
    const pent_string<char> &_Right) {
	// test if NTCS > string
	return (_Right < _Left);
}

inline bool operator>(
    const pent_string<char> &_Left,
    const char *_Right) {
	// test if string > NTCS
	return (_Right < _Left);
}


inline bool operator<=(
    const pent_string<char> &_Left,
    const pent_string<char> &_Right) {
	// test if string <= string
	return (!(_Right < _Left));
}

inline bool operator<=(
    const pent_string<char> &_Left,
    const std::basic_string<char> &_Right) {
	// test if string <= string
	return (!(_Right < _Left));
}

inline bool operator<=(
    const std::basic_string<char> &_Left,
    const pent_string<char> &_Right) {
	// test if string <= string
	return (!(_Right < _Left));
}

inline bool operator<=(
    const char *_Left,
    const pent_string<char> &_Right) {
	// test if NTCS <= string
	return (!(_Right < _Left));
}

inline bool operator<=(
    const pent_string<char> &_Left,
    const char *_Right) {
	// test if string <= NTCS
	return (!(_Right < _Left));
}


inline bool operator>=(
    const pent_string<char> &_Left,
    const pent_string<char> &_Right) {
	// test if string >= string
	return (!(_Left < _Right));
}

inline bool operator>=(
    const pent_string<char> &_Left,
    const std::basic_string<char> &_Right) {
	// test if string >= string
	return (!(_Left < _Right));
}

inline bool operator>=(
    const std::basic_string<char> &_Left,
    const pent_string<char> &_Right) {
	// test if string >= string
	return (!(_Left < _Right));
}

inline bool operator>=(
    const char *_Left,
    const pent_string<char> &_Right) {
	// test if NTCS >= string
	return (!(_Left < _Right));
}

inline bool operator>=(
    const pent_string<char> &_Left,
    const char *_Right) {
	// test if string >= NTCS
	return (!(_Left < _Right));
}

#else
typedef std::string istring;
#endif

} // End of namespace Pentagram
} // End of namespace Ultima8

#endif
