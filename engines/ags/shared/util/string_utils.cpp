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

#include "ags/shared/core/platform.h"
#include "ags/shared/util/math.h"
#include "ags/shared/util/string_utils.h"
#include "ags/shared/util/stream.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

namespace AGS {
namespace Shared {

String StrUtil::IntToString(int d) {
	return String::FromFormat("%d", d);
}

int StrUtil::StringToInt(const String &s, int def_val) {
	if (!s.GetCStr())
		return def_val;
	char *stop_ptr;
	int val = strtol(s.GetCStr(), &stop_ptr, 0);
	return (stop_ptr == s.GetCStr() + s.GetLength()) ? val : def_val;
}

StrUtil::ConversionError StrUtil::StringToInt(const String &s, int &val, int def_val) {
	val = def_val;
	if (!s.GetCStr())
		return StrUtil::kFailed;
	char *stop_ptr;
	_G(errnum) = 0;
	long lval = strtol(s.GetCStr(), &stop_ptr, 0);
	if (stop_ptr != s.GetCStr() + s.GetLength())
		return StrUtil::kFailed;
	if (lval > INT_MAX || lval < INT_MIN || _G(errnum) == AL_ERANGE)
		return StrUtil::kOutOfRange;
	val = (int)lval;
	return StrUtil::kNoError;
}

String StrUtil::Unescape(const String &s) {
	size_t at = s.FindChar('\\');
	if (at == String::npos)
		return s; // no unescaping necessary, return original string
	char *buf = new char[s.GetLength()];
	strncpy(buf, s.GetCStr(), at);
	char *pb = buf + at;
	for (const char *ptr = s.GetCStr() + at; *ptr; ++ptr) {
		if (*ptr != '\\') {
			*(pb++) = *ptr;
			continue;
		}

		char next = *(++ptr);
		switch (next) {
		case 'a':  *(pb++) = '\a'; break;
		case 'b':  *(pb++) = '\b'; break;
		case 'f':  *(pb++) = '\f'; break;
		case 'n':  *(pb++) = '\n'; break;
		case 'r':  *(pb++) = '\r'; break;
		case 't':  *(pb++) = '\t'; break;
		case 'v':  *(pb++) = '\v'; break;
		case '\\': *(pb++) = '\\'; break;
		case '\'': *(pb++) = '\''; break;
		case '\"': *(pb++) = '\"'; break;
		case '\?': *(pb++) = '\?'; break;
		default: *(pb++) = next; break;
		}
	}
	*pb = 0;
	String dst(buf);
	delete buf;
	return dst;
}

String StrUtil::ReadString(Stream *in) {
	size_t len = in->ReadInt32();
	if (len > 0)
		return String::FromStreamCount(in, len);
	return String();
}

void StrUtil::ReadString(char *cstr, Stream *in, size_t buf_limit) {
	size_t len = in->ReadInt32();
	if (buf_limit == 0) {
		in->Seek(len);
		return;
	}

	len = Math::Min(len, buf_limit - 1);
	if (len > 0)
		in->Read(cstr, len);
	cstr[len] = 0;
}

void StrUtil::ReadString(String &s, Stream *in) {
	size_t len = in->ReadInt32();
	s.ReadCount(in, len);
}

void StrUtil::ReadString(char **cstr, Stream *in) {
	size_t len = in->ReadInt32();
	*cstr = new char[len + 1];
	if (len > 0)
		in->Read(*cstr, len);
	(*cstr)[len] = 0;
}

void StrUtil::SkipString(Stream *in) {
	size_t len = in->ReadInt32();
	in->Seek(len);
}

void StrUtil::WriteString(const String &s, Stream *out) {
	size_t len = s.GetLength();
	out->WriteInt32(len);
	if (len > 0)
		out->Write(s.GetCStr(), len);
}

void StrUtil::WriteString(const char *cstr, Stream *out) {
	size_t len = strlen(cstr);
	out->WriteInt32(len);
	if (len > 0)
		out->Write(cstr, len);
}

void StrUtil::WriteString(const char *cstr, size_t len, Stream *out) {
	out->WriteInt32(len);
	if (len > 0)
		out->Write(cstr, len);
}

void StrUtil::ReadCStr(char *buf, Stream *in, size_t buf_limit) {
	if (buf_limit == 0) {
		while (in->ReadByte() > 0);
		return;
	}

	auto ptr = buf;
	auto last = buf + buf_limit - 1;
	for (;;) {
		if (ptr >= last) {
			*ptr = 0;
			while (in->ReadByte() > 0); // must still read until 0
			break;
		}

		auto ichar = in->ReadByte();
		if (ichar <= 0) {
			*ptr = 0;
			break;
		}
		*ptr = static_cast<char>(ichar);
		ptr++;
	}
}

void StrUtil::SkipCStr(Stream *in) {
	while (in->ReadByte() > 0);
}

void StrUtil::WriteCStr(const char *cstr, Stream *out) {
	size_t len = strlen(cstr);
	out->Write(cstr, len + 1);
}

void StrUtil::WriteCStr(const String &s, Stream *out) {
	out->Write(s.GetCStr(), s.GetLength() + 1);
}

void StrUtil::ReadStringMap(StringMap &map, Stream *in) {
	size_t count = in->ReadInt32();
	for (size_t i = 0; i < count; ++i) {
		String key = StrUtil::ReadString(in);
		String value = StrUtil::ReadString(in);
		map.insert(std::make_pair(key, value));
	}
}

void StrUtil::WriteStringMap(const StringMap &map, Stream *out) {
	out->WriteInt32(map.size());
	for (const auto &kv : map) {
		StrUtil::WriteString(kv._key, out);
		StrUtil::WriteString(kv._value, out);
	}
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
