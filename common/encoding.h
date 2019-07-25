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

#ifndef COMMON_ENCODING_H
#define COMMON_ENCODING_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#ifdef USE_ICONV
#include <iconv.h>
#else
typedef void* iconv_t;
#endif // USE_ICONV

#include "common/scummsys.h"
#include "common/str.h"

namespace Common {

class Encoding {
	public:
		Encoding(const String &to, const String &from);
		~Encoding();

		char *convert(const char *string, size_t length);
		static char *convert(const String &to, const String &from, const char *string, size_t length);

		String getFrom() {return _from;};
		void setFrom(const String &from) {_from = from;};

		String getTo() {return _to;};
		void setTo(const String &to) {_to = to;};
	
	private:
		String _to;
		String _from;

		static char *doConversion(iconv_t iconvHandle, const String &to, const String &from, const char *string, size_t length);

		iconv_t _iconvHandle;
		static char *convertIconv(iconv_t iconvHandle, const char *string, size_t length);

};

}

#endif // COMMON_ENCODING_H
