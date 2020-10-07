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

#include "common/scummsys.h"
#include "common/str.h"
#include "common/system.h"


namespace Common {

/**
 * @defgroup common_encoding Text encoding
 * @ingroup common
 *
 * @brief  Functions for managing text encoding.
 *
 * @{
 */

/**
 * A class that allows conversion between different text encoding.
 * The encodings available depend on the current backend and whether
 * ScummVM is compiled with or without iconv.
 */
class Encoding {
	public:
		/**
		 * Construct everything needed for the conversion between two encodings
		 * and saves the values for future use.
		 *
		 * @param to   Name of the encoding the strings will be converted to.
		 * @param from Name of the encoding the strings will be converted from.
		 */
		Encoding(const String &to, const String &from);
		~Encoding() {};

		/**
		 * Convert a string between encodings.
		 *
		 * The resulting string is ended by a character with value 0 
		 * (C-like ending for 1 byte per character encodings, 2 zero bytes for UTF-16,
		 * 4 zero bytes for UTF-32).
		 * The result must be freed after usage using @c free(), not @c delete[].
		 *
		 * @param string String to be converted.
		 * @param length Length of the string to convert in bytes.
		 *
		 * @return Converted string (must be freed) or nullptr if the conversion failed.
		 */
		char *convert(const char *string, size_t length);

		/**
		 * Convert a string between encodings.
		 *
		 * This is a static version of the method above.
		 * The resulting string is ended by a character with value 0
		 * (C-like ending for 1 byte per character encodings, 2 zero bytes for UTF-16,
		 * 4 zero bytes for UTF-32). The result must be freed after usage.
		 *
		 * @param to     Name of the encoding the strings will be converted to.
		 * @param from   Name of the encoding the strings will be converted from.
		 * @param string String to be converted.
		 * @param length Length of the string to convert in bytes.
		 *
		 * @return Converted string (must be freed) or nullptr if the conversion failed
		 */
		static char *convert(const String &to, const String &from, const char *string, size_t length);

		static char *convert(const String &to, const String &from, const String &s) {
			return convert(to, from, s.c_str(), s.size());
		}

		static char *convert(const String &to, const U32String &s) {
			return convert(to, "UTF-32", (const char *)s.c_str(), s.size() * 4);
		}

		/**
		 * @return The encoding that is currently being converted from.
		 */
		String getFrom() {return _from;};

		/**
		 * @param from The encoding to convert from.
		 */
		void setFrom(const String &from) {_from = from;};

		/**
		 * @return The encoding that is currently being converted to.
		 */
		String getTo() {return _to;};

		/**
		 * @param to The encoding to convert to.
		 */
		void setTo(const String &to) {_to = to;};

		/**
		 * Switch the endianness of a string.
		 *
		 * @param string   Array containing the characters of a string.
		 * @param length   Length of the string in bytes.
		 * @param bitCount Number of bits used for each character.
		 *
		 * @return Array of characters with the opposite endianness.
		 */
		static char *switchEndian(const char *string, int length, int bitCount);

		/**
		 * Compute the length (in bytes) of a string in a given encoding.
		 *
		 * The string must be zero ended. Similar to @c strlen.
		 *
		 * @note  This function must be used instead of @c strlen for some encodings
		 *        (such as UTF-16 and UTF-32) because @c strlen does not support
		 *        multi-byte encodings, with some exceptions (such as UTF-8).
		 *
		 * @param string   The string whose size to compute.
		 * @param encoding Encoding of the string.
		 *
		 * @return Size of the string in bytes.
		 */
		static size_t stringLength(const char *string, const String &encoding);
	
	private:
		/** The encoding that is currently being converted to. */
		String _to;

		/** The encoding that is currently being converted from. */
		String _from;

		/**
		 * Take care of transliteration and call conversion.
		 *
		 * The result must be freed after usage.
		 *
		 * @param to     Name of the encoding the strings will be converted to.
		 * @param from   Name of the encoding the strings will be converted from.
		 * @param string The string to convert.
		 * @param length Length of the string to convert in bytes.
		 *
		 * @return Converted string (must be freed) or nullptr if the conversion failed.
		 */
		static char *convertWithTransliteration(const String &to, const String &from, const char *string, size_t length);

		/**
		 * Call as many conversion functions as possible, or until the conversion
		 * succeeds.
		 *
		 * It first tries to use iconv, then it tries to use platform
		 * specific functions, and after that, it tries to use TransMan mapping.
		 *
		 * The result must be freed after usage.
		 *
		 * @param to     Name of the encoding the strings will be converted to.
		 * @param from   Name of the encoding the strings will be converted from.
		 * @param string The string to convert.
		 * @param length Length of the string to convert in bytes.
		 *
		 * @return Converted string (must be freed) or nullptr if the conversion failed.
		 */
		static char *conversion(const String &to, const String &from, const char *string, size_t length);

		/**
		 * Attempt to convert the string using iconv.
		 *
		 * The result must be freed after usage.
		 *
		 * @param to     Name of the encoding the strings will be converted to.
		 * @param from   Name of the encoding the strings will be converted from.
		 * @param string The string to convert.
		 * @param length Length of the string to convert in bytes.
		 *
		 * @return Converted string (must be freed) or nullptr if the conversion failed.
		 */
		static char *convertIconv(const char *to, const char *from, const char *string, size_t length);

		/**
		 * Use a conversion table to convert the string to unicode, and from that,
		 * to the final encoding. Important encodings, that are not supported by
		 * all backends should go here.
		 *
		 * The result must be freed after usage.
		 *
		 * @param to     Name of the encoding the strings will be converted to.
		 * @param from   Name of the encoding the strings will be converted from.
		 * @param string The string to be converted.
		 * @param length Length of the string to convert in bytes.
		 *
		 * @return Converted string (must be freed) or nullptr if the conversion failed.
		 */
		static char *convertConversionTable(const char *to, const char *from, const char *string, size_t length);

		/**
		 * Transliterate a Cyrillic string in ISO-8859-5 encoding and return
		 * its ASCII (Latin) form.
		 *
		 * The result must be freed after usage.
		 *
		 * @param string The string to convert.
		 *
		 * @return Transliterated string in ASCII (must be freed) or nullptr on fail.
		 */
		static char *transliterateCyrillic(const char *string);

		/**
		 * Transliterate a Cyrillic string into a UTF-32 string.
		 *
		 * The result must be freed after usage.
		 *
		 * @param string The string to convert.
		 * @param length Length of the string in bytes.
		 *
		 * @return Transliterated string in UTF-32 (must be freed) or nullptr on fail.
		 */
		static uint32 *transliterateUTF32(const uint32 *string, size_t length);
};

/** @} */

}

#endif // COMMON_ENCODING_H
