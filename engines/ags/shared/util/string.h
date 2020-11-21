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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

//=============================================================================
//
// String class with simple memory management and copy-on-write behavior.
//
// String objects do reference counting and share data buffer on assignment.
// The reallocation and copying is done only when the string is modified.
//
// The copying of memory inside buffer is reduced to minimum. If the string is
// truncated, it is not aligned to buffer head each time, instead the c-str
// pointer is advanced, or null-terminator is put on the new place. Similarly,
// when string is enlarged and new characters are prepended or appended, only
// c-str pointer and null-terminator's position are changed, if there's enough
// space before and after meaningful string data.
//
// The class provides means to reserve large amount of buffer space before
// making modifications, as well as compacting buffer to minimal size.
//
// For all methods that expect C-string as parameter - if the null pointer is
// passed in place of C-string it is treated in all aspects as a valid empty
// string.
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_STRING_H
#define AGS_SHARED_UTIL_STRING_H

#include <stdarg.h>
#include <vector>
#include "core/platform.h"
#include "core/types.h"
#include "debug/assert.h"

namespace AGS {
namespace Common {

class Stream;

class String {
public:
	// Standard constructor: intialize empty string
	String();
	// Copy constructor
	String(const String &);
	// Initialize with C-string
	String(const char *cstr);
	// Initialize by copying up to N chars from C-string
	String(const char *cstr, size_t length);
	// Initialize by filling N chars with certain value
	String(char c, size_t count);
	~String();

	// Get underlying C-string for reading; this method guarantees valid C-string
	inline const char *GetCStr() const {
		return _cstr ? _cstr : "";
	}
	// Get C-string or nullptr
	inline const char *GetNullableCStr() const {
		return _cstr ? _cstr : nullptr;
	}
	// Get character count
	inline size_t GetLength() const {
		return _len;
	}
	// Know if the string is empty (has no meaningful characters)
	inline bool IsEmpty() const {
		return _len == 0;
	}

	// Those getters are for tests only, hence if AGS_PLATFORM_DEBUG
#if AGS_PLATFORM_DEBUG
	inline const char *GetBuffer() const {
		return _buf;
	}

	inline size_t GetCapacity() const {
		return _bufHead ? _bufHead->Capacity : 0;
	}

	inline size_t GetRefCount() const {
		return _bufHead ? _bufHead->RefCount : 0;
	}
#endif

	// Read() method implies that string length is initially unknown.
	// max_chars parameter determine the buffer size limit.
	// If stop_at_limit flag is set, it will read only up to the max_chars.
	// Otherwise (by default) hitting the limit won't stop stream reading;
	// the data will be read until null-terminator or EOS is met, and buffer
	// will contain only leftmost part of the longer string that fits in.
	// This method is better fit for reading from binary streams.
	void    Read(Stream *in, size_t max_chars = 5 * 1024 * 1024, bool stop_at_limit = false);
	// ReadCount() reads up to N characters from stream, ignoring null-
	// terminator. This method is better fit for reading from text
	// streams, or when the length of string is known beforehand.
	void    ReadCount(Stream *in, size_t count);
	// Write() puts the null-terminated string into the stream.
	void    Write(Stream *out) const;
	// WriteCount() writes N characters to stream, filling the remaining
	// space with null-terminators when needed.
	void    WriteCount(Stream *out, size_t count) const;

	static void WriteString(const char *cstr, Stream *out);

	//-------------------------------------------------------------------------
	// String analysis methods
	//-------------------------------------------------------------------------

	// Compares with given C-string
	int     Compare(const char *cstr) const;
	int     CompareNoCase(const char *cstr) const;
	// Compares the leftmost part of this string with given C-string
	int     CompareLeft(const char *cstr, size_t count = -1) const;
	int     CompareLeftNoCase(const char *cstr, size_t count = -1) const;
	// Compares any part of this string with given C-string
	int     CompareMid(const char *cstr, size_t from, size_t count = -1) const;
	int     CompareMidNoCase(const char *cstr, size_t from, size_t count = -1) const;
	// Compares the rightmost part of this string with given C-string
	int     CompareRight(const char *cstr, size_t count = -1) const;
	int     CompareRightNoCase(const char *cstr, size_t count = -1) const;

	// These functions search for character or substring inside this string
	// and return the index of the (first) character, or -1 if nothing found.
	size_t  FindChar(char c, size_t from = 0) const;
	size_t  FindCharReverse(char c, size_t from = -1) const;
	size_t  FindString(const char *cstr, size_t from = 0) const;

	// Section methods treat string as a sequence of 'fields', separated by
	// special character. They search for a substring consisting of all such
	// 'fields' from the 'first' to the 'last', inclusive; the bounding
	// separators are optionally included too.
	// Section indexes are zero-based. The first (0th) section is always
	// located before the first separator and the last section is always
	// located after the last separator, meaning that if the outermost
	// character in string is separator char, there's still an empty trailing
	// field beyond that.
	// This also means that there's always at least one section in any string,
	// even if there are no separating chars.
	bool    FindSection(char separator, size_t first, size_t last, bool exclude_first_sep, bool exclude_last_sep,
	                    size_t &from, size_t &to) const;

	// Get Nth character with bounds check (as opposed to subscript operator)
	inline char GetAt(size_t index) const {
		return (index < _len) ? _cstr[index] : 0;
	}
	inline char GetLast() const {
		return (_len > 0) ? _cstr[_len - 1] : 0;
	}

	//-------------------------------------------------------------------------
	// Value cast methods
	//-------------------------------------------------------------------------

	int     ToInt() const;

	//-------------------------------------------------------------------------
	// Factory methods
	//-------------------------------------------------------------------------

	// Wraps the given string buffer without owning it, won't count references,
	// won't delete it at destruction. Can be used with string literals.
	static String Wrapper(const char *cstr);

	static String FromFormat(const char *fcstr, ...);
	static String FromFormatV(const char *fcstr, va_list argptr);
	// Reads stream until null-terminator or EOS
	static String FromStream(Stream *in, size_t max_chars = 5 * 1024 * 1024, bool stop_at_limit = false);
	// Reads up to N chars from stream
	static String FromStreamCount(Stream *in, size_t count);

	// Creates a lowercased copy of the string
	String  Lower() const;
	// Creates an uppercased copy of the string
	String  Upper() const;

	// Extract N leftmost characters as a new string
	String  Left(size_t count) const;
	// Extract up to N characters starting from given index
	String  Mid(size_t from, size_t count = -1) const;
	// Extract N rightmost characters
	String  Right(size_t count) const;

	// Extract leftmost part, separated by the given char; if no separator was
	// found returns the whole string
	String  LeftSection(char separator, bool exclude_separator = true) const;
	// Extract rightmost part, separated by the given char; if no separator was
	// found returns the whole string
	String  RightSection(char separator, bool exclude_separator = true) const;
	// Extract the range of Xth to Yth fields, separated by the given character
	String  Section(char separator, size_t first, size_t last,
	                bool exclude_first_sep = true, bool exclude_last_sep = true) const;
	// Splits the string into segments divided by the instances of a given character,
	// including empty segments e.g. if separators follow each other;
	// returns at least one segment (equal to full string if no separator was found)
	std::vector<String> Split(char separator) const;

	//-------------------------------------------------------------------------
	// String modification methods
	//-------------------------------------------------------------------------

	// Ensure string has at least space to store N chars;
	// this does not change string contents, nor length
	void    Reserve(size_t max_length);
	// Ensure string has at least space to store N additional chars
	void    ReserveMore(size_t more_length);
	// Make string's buffer as small as possible to hold current data
	void    Compact();

	// Append* methods add content at the string's end, increasing its length
	// Add C-string at string's end
	void    Append(const char *cstr);
	// Add single character at string's end
	void    AppendChar(char c);
	// Clip* methods decrease the string, removing defined part
	// Cuts off leftmost N characters
	void    ClipLeft(size_t count);
	// Cuts out N characters starting from given index
	void    ClipMid(size_t from, size_t count = -1);
	// Cuts off rightmost N characters
	void    ClipRight(size_t count);
	// Cuts off leftmost part, separated by the given char; if no separator was
	// found cuts whole string, leaving empty string
	void    ClipLeftSection(char separator, bool include_separator = true);
	// Cuts off rightmost part, separated by the given char; if no separator
	// was found cuts whole string, leaving empty string
	void    ClipRightSection(char separator, bool include_separator = true);
	// Cuts out the range of Xth to Yth fields separated by the given character
	void    ClipSection(char separator, size_t first, size_t last,
	                    bool include_first_sep = true, bool include_last_sep = true);
	// Sets string length to zero
	void    Empty();
	// Makes a new string by filling N chars with certain value
	void    FillString(char c, size_t count);
	// Makes a new string by putting in parameters according to format string
	void    Format(const char *fcstr, ...);
	void    FormatV(const char *fcstr, va_list argptr);
	// Decrement ref counter and deallocate data if must.
	// Free() should be called only when buffer is not needed anymore;
	// if string must be truncated to zero length, but retain the allocated
	// memory, call Empty() instead.
	void    Free();
	// Convert string to lowercase equivalent
	void    MakeLower();
	// Convert string to uppercase equivalent
	void    MakeUpper();
	// Prepend* methods add content before the string's head, increasing its length
	// Add C-string before string's head
	void    Prepend(const char *cstr);
	// Add single character before string's head
	void    PrependChar(char c);
	// Replaces all occurences of one character with another character
	void    Replace(char what, char with);
	// Replaces particular substring with another substring; new substring
	// may have different length
	void    ReplaceMid(size_t from, size_t count, const char *cstr);
	// Reverses the string
	void    Reverse();
	// Overwrite the Nth character of the string; does not change string's length
	void    SetAt(size_t index, char c);
	// Makes a new string by copying up to N chars from C-string
	void    SetString(const char *cstr, size_t length = -1);
	// For all Trim functions, if given character value is 0, all whitespace
	// characters (space, tabs, CRLF) are removed.
	// Remove heading and trailing characters from the string
	void    Trim(char c = 0);
	// Remove heading characters from the string;
	void    TrimLeft(char c = 0);
	// Remove trailing characters from the string
	void    TrimRight(char c = 0);
	// Truncate* methods decrease the string to the part of itself
	// Truncate the string to the leftmost N characters
	void    TruncateToLeft(size_t count);
	// Truncate the string to the middle N characters
	void    TruncateToMid(size_t from, size_t count = -1);
	// Truncate the string to the rightmost N characters
	void    TruncateToRight(size_t count);
	// Truncate the string to the leftmost part, separated by the given char;
	// if no separator was found leaves string unchanged
	void    TruncateToLeftSection(char separator, bool exclude_separator = true);
	// Truncate the string to the rightmost part, separated by the given char;
	// if no separator was found leaves string unchanged
	void    TruncateToRightSection(char separator, bool exclude_separator = true);
	// Truncate the string to range of Xth to Yth fields separated by the
	// given character
	void    TruncateToSection(char separator, size_t first, size_t last,
	                          bool exclude_first_sep = true, bool exclude_last_sep = true);
	// Wraps the given string buffer without owning it, won't count references,
	// won't delete it at destruction. Can be used with string literals.
	void    Wrap(const char *cstr);

	//-------------------------------------------------------------------------
	// Operators
	//-------------------------------------------------------------------------

	inline operator const char *() const {
		return GetCStr();
	}
	// Assign String by sharing data reference
	String &operator=(const String &);
	// Assign C-string by copying contents
	String &operator=(const char *cstr);
	inline char operator[](size_t index) const {
		assert(index < _len);
		return _cstr[index];
	}
	inline bool operator==(const char *cstr) const {
		return Compare(cstr) == 0;
	}
	inline bool operator!=(const char *cstr) const {
		return Compare(cstr) != 0;
	}
	inline bool operator <(const char *cstr) const {
		return Compare(cstr) < 0;
	}

private:
	// Creates new empty string with buffer enough to fit given length
	void    Create(size_t buffer_length);
	// Release string and copy data to the new buffer
	void    Copy(size_t buffer_length, size_t offset = 0);
	// Aligns data at given offset
	void    Align(size_t offset);

	// Ensure this string is a compact independent copy, with ref counter = 1
	void    BecomeUnique();
	// Ensure this string is independent, and there's enough space before
	// or after the current string data
	void    ReserveAndShift(bool left, size_t more_length);

	char    *_cstr;  // pointer to actual string data
	size_t  _len;    // valid string length, in characters, excluding null-term

	// Header of a reference-counted buffer
	struct BufHeader {
		size_t  RefCount = 0; // reference count
		size_t  Capacity = 0; // available space, in characters
	};

	// Union that groups mutually exclusive data (currently only ref counted buffer)
	union {
		char      *_buf;     // reference-counted data (raw ptr)
		BufHeader *_bufHead; // the header of a reference-counted data
	};
};

} // namespace Common
} // namespace AGS

#endif
