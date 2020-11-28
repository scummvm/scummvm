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

#include "ags/shared/util/math.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/string.h"
#include "ags/shared/util/string_compat.h"
#include "ags/lib/std/algorithm.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

String::String()
	: _cstr(nullptr)
	, _len(0)
	, _buf(nullptr) {
}

String::String(const String &str)
	: _cstr(nullptr)
	, _len(0)
	, _buf(nullptr) {
	*this = str;
}

String::String(const char *cstr)
	: _cstr(nullptr)
	, _len(0)
	, _buf(nullptr) {
	*this = cstr;
}

String::String(const Common::String &s) : _cstr(nullptr), _len(0), _buf(nullptr) {
	*this = s.c_str();
}

String::String(const char *cstr, size_t length)
	: _cstr(nullptr)
	, _len(0)
	, _buf(nullptr) {
	SetString(cstr, length);
}

String::String(char c, size_t count)
	: _cstr(nullptr)
	, _len(0)
	, _buf(nullptr) {
	FillString(c, count);
}

String::~String() {
	Free();
}

void String::Read(Stream *in, size_t max_chars, bool stop_at_limit) {
	Empty();
	if (!in) {
		return;
	}
	if (max_chars == 0 && stop_at_limit) {
		return;
	}

	char buffer[1024];
	char *read_ptr = buffer;
	size_t read_size = 0;
	int ichar;
	do {
		ichar = in->ReadByte();
		read_size++;
		if (read_size > max_chars) {
			continue;
		}
		*read_ptr = (char)(ichar >= 0 ? ichar : 0);
		if (!*read_ptr || ((read_ptr - buffer) == (sizeof(buffer) - 1 - 1))) {
			buffer[sizeof(buffer) - 1] = 0;
			Append(buffer);
			read_ptr = buffer;
		} else {
			read_ptr++;
		}
	} while (ichar > 0 && !(stop_at_limit && read_size == max_chars));
}

void String::ReadCount(Stream *in, size_t count) {
	Empty();
	if (in && count > 0) {
		ReserveAndShift(false, count);
		count = in->Read(_cstr, count);
		_cstr[count] = 0;
		_len = strlen(_cstr);
	}
}

void String::Write(Stream *out) const {
	if (out) {
		out->Write(GetCStr(), GetLength() + 1);
	}
}

void String::WriteCount(Stream *out, size_t count) const {
	if (out) {
		size_t str_out_len = Math::Min(count - 1, GetLength());
		if (str_out_len > 0)
			out->Write(GetCStr(), str_out_len);
		size_t null_out_len = count - str_out_len;
		if (null_out_len > 0)
			out->WriteByteCount(0, null_out_len);
	}
}

/* static */ void String::WriteString(const char *cstr, Stream *out) {
	if (out) {
		cstr = cstr ? cstr : "";
		out->Write(cstr, strlen(cstr) + 1);
	}
}

int String::Compare(const char *cstr) const {
	return strcmp(GetCStr(), cstr ? cstr : "");
}

int String::CompareNoCase(const char *cstr) const {
	return ags_stricmp(GetCStr(), cstr ? cstr : "");
}

int String::CompareLeft(const char *cstr, size_t count) const {
	cstr = cstr ? cstr : "";
	return strncmp(GetCStr(), cstr, count != npos ? count : strlen(cstr));
}

int String::CompareLeftNoCase(const char *cstr, size_t count) const {
	cstr = cstr ? cstr : "";
	return ags_strnicmp(GetCStr(), cstr, count != npos ? count : strlen(cstr));
}

int String::CompareMid(const char *cstr, size_t from, size_t count) const {
	cstr = cstr ? cstr : "";
	from = Math::Min(from, GetLength());
	return strncmp(GetCStr() + from, cstr, count != npos ? count : strlen(cstr));
}

int String::CompareMidNoCase(const char *cstr, size_t from, size_t count) const {
	cstr = cstr ? cstr : "";
	from = Math::Min(from, GetLength());
	return ags_strnicmp(GetCStr() + from, cstr, count != npos ? count : strlen(cstr));
}

int String::CompareRight(const char *cstr, size_t count) const {
	cstr = cstr ? cstr : "";
	count = count != npos ? count : strlen(cstr);
	size_t off = Math::Min(GetLength(), count);
	return strncmp(GetCStr() + GetLength() - off, cstr, count);
}

int String::CompareRightNoCase(const char *cstr, size_t count) const {
	cstr = cstr ? cstr : "";
	count = count != npos ? count : strlen(cstr);
	size_t off = Math::Min(GetLength(), count);
	return ags_strnicmp(GetCStr() + GetLength() - off, cstr, count);
}

size_t String::FindChar(char c, size_t from) const {
	if (c && from < _len) {
		const char *found_cstr = strchr(_cstr + from, c);
		return found_cstr ? found_cstr - _cstr : npos;
	}
	return npos;
}

size_t String::FindCharReverse(char c, size_t from) const {
	if (!_cstr || !c) {
		return npos;
	}

	from = Math::Min(from, _len - 1);
	const char *seek_ptr = _cstr + from;
	while (seek_ptr >= _cstr) {
		if (*seek_ptr == c) {
			return seek_ptr - _cstr;
		}
		seek_ptr--;
	}
	return npos;
}

size_t String::FindString(const char *cstr, size_t from) const {
	if (cstr && from < _len) {
		const char *found_cstr = strstr(_cstr + from, cstr);
		return found_cstr ? found_cstr - _cstr : npos;
	}
	return npos;
}

bool String::FindSection(char separator, size_t first, size_t last, bool exclude_first_sep, bool exclude_last_sep,
	size_t &from, size_t &to) const {
	if (!_cstr || !separator) {
		return false;
	}
	if (first > last) {
		return false;
	}

	size_t this_field = 0;
	size_t slice_from = 0;
	size_t slice_to = _len;
	size_t slice_at = npos;
	do {
		slice_at = FindChar(separator, slice_at + 1);
		if (slice_at == npos)
			slice_at = _len;
		// found where previous field ends
		if (this_field == last) {
			// if previous field is the last one to be included,
			// then set the section tail
			slice_to = exclude_last_sep ? slice_at : slice_at + 1;
		}
		if (slice_at != _len) {
			this_field++;
			if (this_field == first) {
				// if the new field is the first one to be included,
				// then set the section head
				slice_from = exclude_first_sep ? slice_at + 1 : slice_at;
			}
		}
	} while (slice_at < _len && this_field <= last);

	// the search is a success if at least the first field was found
	if (this_field >= first) {
		// correct the indices to stay in the [0; length] range
		assert(slice_from <= slice_to);
		from = Math::Clamp(slice_from, (size_t)0, _len);
		to = Math::Clamp(slice_to, (size_t)0, _len);
		return true;
	}
	return false;
}

int String::ToInt() const {
	return atoi(GetCStr());
}

String String::Wrapper(const char *cstr) {
	String str;
	str.Wrap(cstr);
	return str;
}

/* static */ String String::FromFormat(const char *fcstr, ...) {
	fcstr = fcstr ? fcstr : "";
	String str;
	va_list argptr;
	va_start(argptr, fcstr);
	str.FormatV(fcstr, argptr);
	va_end(argptr);
	return str;
}

/* static */ String String::FromFormatV(const char *fcstr, va_list argptr) {
	String str;
	str.FormatV(fcstr, argptr);
	return str;
}

/* static */ String String::FromStream(Stream *in, size_t max_chars, bool stop_at_limit) {
	String str;
	str.Read(in, max_chars, stop_at_limit);
	return str;
}

/* static */ String String::FromStreamCount(Stream *in, size_t count) {
	String str;
	str.ReadCount(in, count);
	return str;
}

String String::Lower() const {
	String str = *this;
	str.MakeLower();
	return str;
}

String String::Upper() const {
	String str = *this;
	str.MakeUpper();
	return str;
}

String String::Left(size_t count) const {
	count = Math::Min(count, GetLength());
	return count == GetLength() ? *this : String(GetCStr(), count);
}

String String::Mid(size_t from, size_t count) const {
	Math::ClampLength(from, count, (size_t)0, GetLength());
	return count == GetLength() ? *this : String(GetCStr() + from, count);
}

String String::Right(size_t count) const {
	count = Math::Min(count, GetLength());
	return count == GetLength() ? *this : String(GetCStr() + GetLength() - count, count);
}

String String::LeftSection(char separator, bool exclude_separator) const {
	if (_cstr && separator) {
		size_t slice_at = FindChar(separator);
		if (slice_at != npos) {
			slice_at = exclude_separator ? slice_at : slice_at + 1;
			return Left(slice_at);
		}
	}
	return *this;
}

String String::RightSection(char separator, bool exclude_separator) const {
	if (_cstr && separator) {
		size_t slice_at = FindCharReverse(separator);
		if (slice_at != npos) {
			size_t count = exclude_separator ? _len - slice_at - 1 : _len - slice_at;
			return Right(count);
		}
	}
	return *this;
}

String String::Section(char separator, size_t first, size_t last,
	bool exclude_first_sep, bool exclude_last_sep) const {
	if (!_cstr || !separator) {
		return String();
	}

	size_t slice_from;
	size_t slice_to;
	if (FindSection(separator, first, last, exclude_first_sep, exclude_last_sep,
		slice_from, slice_to)) {
		return Mid(slice_from, slice_to - slice_from);
	}
	return String();
}

std::vector<String> String::Split(char separator) const {
	std::vector<String> result;
	if (!_cstr || !separator)
		return result;
	const char *ptr = _cstr;
	while (*ptr) {
		const char *found_cstr = strchr(ptr, separator);
		if (!found_cstr) break;
		result.push_back(String(ptr, found_cstr - ptr));
		ptr = found_cstr + 1;
	}
	result.push_back(String(ptr));
	return result;
}

void String::Reserve(size_t max_length) {
	if (_bufHead) {
		if (max_length > _bufHead->Capacity) {
			// grow by 50%
			size_t grow_length = _bufHead->Capacity + (_bufHead->Capacity / 2);
			Copy(Math::Max(max_length, grow_length));
		}
	} else {
		Create(max_length);
	}
}

void String::ReserveMore(size_t more_length) {
	Reserve(GetLength() + more_length);
}

void String::Compact() {
	if (_bufHead && _bufHead->Capacity > _len) {
		Copy(_len);
	}
}

void String::Append(const char *cstr) {
	if (cstr) {
		size_t length = strlen(cstr);
		if (length > 0) {
			ReserveAndShift(false, length);
			memcpy(_cstr + _len, cstr, length);
			_len += length;
			_cstr[_len] = 0;
		}
	}
}

void String::AppendChar(char c) {
	if (c) {
		ReserveAndShift(false, 1);
		_cstr[_len++] = c;
		_cstr[_len] = 0;
	}
}

void String::ClipLeft(size_t count) {
	if (_len > 0 && count > 0) {
		count = Math::Min(count, _len);
		BecomeUnique();
		_len -= count;
		_cstr += count;
	}
}

void String::ClipMid(size_t from, size_t count) {
	if (from < _len) {
		count = Math::Min(count, _len - from);
		if (count > 0) {
			BecomeUnique();
			if (!from) {
				_len -= count;
				_cstr += count;
			} else if (from + count == _len) {
				_len -= count;
				_cstr[_len] = 0;
			} else {
				char *cstr_mid = _cstr + from;
				memmove(cstr_mid, _cstr + from + count, _len - from - count + 1);
				_len -= count;
			}
		}
	}
}

void String::ClipRight(size_t count) {
	if (count > 0) {
		count = Math::Min(count, GetLength());
		BecomeUnique();
		_len -= count;
		_cstr[_len] = 0;
	}
}

void String::ClipLeftSection(char separator, bool include_separator) {
	if (_cstr && separator) {
		size_t slice_at = FindChar(separator);
		if (slice_at != npos) {
			ClipLeft(include_separator ? slice_at + 1 : slice_at);
		} else
			Empty();
	}
}

void String::ClipRightSection(char separator, bool include_separator) {
	if (_cstr && separator) {
		size_t slice_at = FindCharReverse(separator);
		if (slice_at != npos) {
			ClipRight(include_separator ? _len - slice_at : _len - slice_at - 1);
		} else
			Empty();
	}
}

void String::ClipSection(char separator, size_t first, size_t last,
	bool include_first_sep, bool include_last_sep) {
	if (!_cstr || !separator) {
		return;
	}

	size_t slice_from;
	size_t slice_to;
	if (FindSection(separator, first, last, !include_first_sep, !include_last_sep,
		slice_from, slice_to)) {
		ClipMid(slice_from, slice_to - slice_from);
	}
}

void String::Empty() {
	if (_cstr) {
		BecomeUnique();
		_len = 0;
		_cstr[0] = 0;
	}
}

void String::FillString(char c, size_t count) {
	Empty();
	if (count > 0) {
		ReserveAndShift(false, count);
		memset(_cstr, c, count);
		_len = count;
		_cstr[count] = 0;
	}
}

void String::Format(const char *fcstr, ...) {
	va_list argptr;
	va_start(argptr, fcstr);
	FormatV(fcstr, argptr);
	va_end(argptr);
}

void String::FormatV(const char *fcstr, va_list argptr) {
	fcstr = fcstr ? fcstr : "";
	va_list argptr_cpy;
	va_copy(argptr_cpy, argptr);
	size_t length = vsnprintf(nullptr, 0u, fcstr, argptr);
	ReserveAndShift(false, Math::Surplus(length, GetLength()));
	vsprintf(_cstr, fcstr, argptr_cpy);
	va_end(argptr_cpy);
	_len = length;
	_cstr[_len] = 0;
}

void String::Free() {
	if (_bufHead) {
		assert(_bufHead->RefCount > 0);
		_bufHead->RefCount--;
		if (!_bufHead->RefCount) {
			delete[] _buf;
		}
	}
	_buf = nullptr;
	_cstr = nullptr;
	_len = 0;
}

void String::MakeLower() {
	if (_cstr) {
		BecomeUnique();
		ags_strlwr(_cstr);
	}
}

void String::MakeUpper() {
	if (_cstr) {
		BecomeUnique();
		ags_strupr(_cstr);
	}
}

void String::Prepend(const char *cstr) {
	if (cstr) {
		size_t length = strlen(cstr);
		if (length > 0) {
			ReserveAndShift(true, length);
			memcpy(_cstr - length, cstr, length);
			_len += length;
			_cstr -= length;
		}
	}
}

void String::PrependChar(char c) {
	if (c) {
		ReserveAndShift(true, 1);
		_len++;
		_cstr--;
		_cstr[0] = c;
	}
}

void String::Replace(char what, char with) {
	if (_cstr && what && with && what != with) {
		BecomeUnique();
		char *rep_ptr = _cstr;
		while (*rep_ptr) {
			if (*rep_ptr == what) {
				*rep_ptr = with;
			}
			rep_ptr++;
		}
	}
}

void String::ReplaceMid(size_t from, size_t count, const char *cstr) {
	if (!cstr)
		cstr = "";
	size_t length = strlen(cstr);
	Math::ClampLength(from, count, (size_t)0, GetLength());
	ReserveAndShift(false, Math::Surplus(length, count));
	memmove(_cstr + from + length, _cstr + from + count, GetLength() - (from + count) + 1);
	memcpy(_cstr + from, cstr, length);
	_len += length - count;
}

void String::Reverse() {
	if (!_cstr || GetLength() <= 1)
		return;
	for (char *fw = _cstr, *bw = _cstr + _len - 1;
		*fw; ++fw, --bw) {
		std::swap(*fw, *bw);
	}
}

void String::SetAt(size_t index, char c) {
	if (_cstr && index < GetLength() && c) {
		BecomeUnique();
		_cstr[index] = c;
	}
}

void String::SetString(const char *cstr, size_t length) {
	if (cstr) {
		length = Math::Min(length, strlen(cstr));
		if (length > 0) {
			ReserveAndShift(false, Math::Surplus(length, GetLength()));
			memcpy(_cstr, cstr, length);
			_len = length;
			_cstr[length] = 0;
		} else {
			Empty();
		}
	} else {
		Empty();
	}
}

void String::Trim(char c) {
	TrimLeft(c);
	TrimRight(c);
}

void String::TrimLeft(char c) {
	if (!_cstr || !_len) {
		return;
	}

	const char *trim_ptr = _cstr;
	for (;;) {
		auto t = *trim_ptr;
		if (t == 0) {
			break;
		}
		if (c && t != c) {
			break;
		}
		if (!c && !Common::isSpace(t)) {
			break;
		}
		trim_ptr++;
	}
	size_t trimmed = trim_ptr - _cstr;
	if (trimmed > 0) {
		BecomeUnique();
		_len -= trimmed;
		_cstr += trimmed;
	}
}

void String::TrimRight(char c) {
	if (!_cstr || !_len) {
		return;
	}

	const char *trim_ptr = _cstr + _len - 1;
	for (;;) {
		if (trim_ptr < _cstr) {
			break;
		}
		auto t = *trim_ptr;
		if (c && t != c) {
			break;
		}
		if (!c && !Common::isSpace(t)) {
			break;
		}
		trim_ptr--;
	}
	size_t trimmed = (_cstr + _len - 1) - trim_ptr;
	if (trimmed > 0) {
		BecomeUnique();
		_len -= trimmed;
		_cstr[_len] = 0;
	}
}

void String::TruncateToLeft(size_t count) {
	if (_cstr) {
		count = Math::Min(count, _len);
		if (count < _len) {
			BecomeUnique();
			_len = count;
			_cstr[_len] = 0;
		}
	}
}

void String::TruncateToMid(size_t from, size_t count) {
	if (_cstr) {
		Math::ClampLength(from, count, (size_t)0, _len);
		if (from > 0 || count < _len) {
			BecomeUnique();
			_len = count;
			_cstr += from;
			_cstr[_len] = 0;
		}
	}
}

void String::TruncateToRight(size_t count) {
	if (_cstr) {
		count = Math::Min(count, GetLength());
		if (count < _len) {
			BecomeUnique();
			_cstr += _len - count;
			_len = count;
		}
	}
}

void String::TruncateToLeftSection(char separator, bool exclude_separator) {
	if (_cstr && separator) {
		size_t slice_at = FindChar(separator);
		if (slice_at != npos) {
			TruncateToLeft(exclude_separator ? slice_at : slice_at + 1);
		}
	}
}

void String::TruncateToRightSection(char separator, bool exclude_separator) {
	if (_cstr && separator) {
		size_t slice_at = FindCharReverse(separator);
		if (slice_at != npos) {
			TruncateToRight(exclude_separator ? _len - slice_at - 1 : _len - slice_at);
		}
	}
}

void String::TruncateToSection(char separator, size_t first, size_t last,
	bool exclude_first_sep, bool exclude_last_sep) {
	if (!_cstr || !separator) {
		return;
	}

	size_t slice_from;
	size_t slice_to;
	if (FindSection(separator, first, last, exclude_first_sep, exclude_last_sep,
		slice_from, slice_to)) {
		TruncateToMid(slice_from, slice_to - slice_from);
	} else {
		Empty();
	}
}

void String::Wrap(const char *cstr) {
	Free();
	_buf = nullptr;
	// Note that String is NOT supposed to *modify* the const buffer.
	// Any non-read operation on the buffer is preceded by a call to BecomeUnique,
	// which in turn will allocate a reference-counted buffer copy.
	_cstr = const_cast<char *>(cstr);
	_len = strlen(cstr);
}

String &String::operator=(const String &str) {
	if (_cstr != str._cstr) {
		Free();
		_buf = str._buf;
		_cstr = str._cstr;
		_len = str._len;
		if (_bufHead) {
			_bufHead->RefCount++;
		}
	}
	return *this;
}

String &String::operator=(const char *cstr) {
	SetString(cstr);
	return *this;
}

void String::Create(size_t max_length) {
	_buf = new char[sizeof(String::BufHeader) + max_length + 1];
	_bufHead->RefCount = 1;
	_bufHead->Capacity = max_length;
	_len = 0;
	_cstr = _buf + sizeof(String::BufHeader);
	_cstr[_len] = 0;
}

void String::Copy(size_t max_length, size_t offset) {
	if (!_cstr) {
		return;
	}

	char *new_data = new char[sizeof(String::BufHeader) + max_length + 1];
	// remember, that _cstr may point to any address in buffer
	char *cstr_head = new_data + sizeof(String::BufHeader) + offset;
	size_t copy_length = Math::Min(_len, max_length);
	memcpy(cstr_head, _cstr, copy_length);
	Free();
	_buf = new_data;
	_bufHead->RefCount = 1;
	_bufHead->Capacity = max_length;
	_len = copy_length;
	_cstr = cstr_head;
	_cstr[_len] = 0;
}

void String::Align(size_t offset) {
	char *cstr_head = _buf + sizeof(String::BufHeader) + offset;
	memmove(cstr_head, _cstr, _len + 1);
	_cstr = cstr_head;
}

void String::BecomeUnique() {
	if (_cstr && (!_bufHead || (_bufHead && _bufHead->RefCount > 1))) {
		Copy(_len);
	}
}

void String::ReserveAndShift(bool left, size_t more_length) {
	if (_bufHead) {
		size_t total_length = _len + more_length;
		if (_bufHead->Capacity < total_length) {
			// grow by 50% or at least to total_size
			size_t grow_length = _bufHead->Capacity + (_bufHead->Capacity >> 1);
			Copy(Math::Max(total_length, grow_length), left ? more_length : 0u);
		} else if (_bufHead->RefCount > 1) {
			Copy(total_length, left ? more_length : 0u);
		} else {
			// make sure we make use of all of our space
			const char *cstr_head = _buf + sizeof(String::BufHeader);
			size_t free_space = left ?
				_cstr - cstr_head :
				(cstr_head + _bufHead->Capacity) - (_cstr + _len);
			if (free_space < more_length) {
				Align((left ?
					_cstr + (more_length - free_space) :
					_cstr - (more_length - free_space)) - cstr_head);
			}
		}
	} else {
		Create(more_length);
	}
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
