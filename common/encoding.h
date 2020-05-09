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
 * A class, that allows conversion between different text encoding,
 * the encodings available depend on the current backend and if the
 * ScummVM is compiled with or without iconv.
 */
class Encoding {
	public:
		/**
		 * Constructs everything needed for the conversion between 2 encodings
		 * and saves the values for future use.
		 *
		 * @param to Name of the encoding the strings will be converted to
		 * @param from Name of the encoding the strings will be converted from
		 */
		Encoding(const String &to, const String &from);
		~Encoding() {};

		/**
		 * Converts string between encodings. The resulting string is ended by 
		 * a character with value 0 (C-like ending for 1 byte per character
		 * encodings, 2 zero bytes for UTF-16, 4 zero bytes for UTF-32)
		 *
		 * The result has to be freed after use.
		 *
		 * @param string String that should be converted.
		 * @param length Length of the string to convert in bytes.
		 *
		 * @return Converted string (must be freed) or nullptr if the conversion failed
		 */
		char *convert(const char *string, size_t length);

		/**
		 * Static version of the method above.
		 * Converts string between encodings. The resulting string is ended by 
		 * a character with value 0 (C-like ending for 1 byte per character
		 * encodings, 2 zero bytes for UTF-16, 4 zero bytes for UTF-32)
		 *
		 * The result has to be freed after use.
		 *
		 * @param to Name of the encoding the strings will be converted to
		 * @param from Name of the encoding the strings will be converted from
		 * @param string String that should be converted.
		 * @param length Length of the string to convert in bytes.
		 *
		 * @return Converted string (must be freed) or nullptr if the conversion failed
		 */
		static char *convert(const String &to, const String &from, const char *string, size_t length);

		/**
		 * @return The encoding, which is currently being converted from
		 */
		String getFrom() {return _from;};

		/**
		 * @param from The encoding, to convert from
		 */
		void setFrom(const String &from) {_from = from;};

		/**
		 * @return The encoding, which is currently being converted to
		 */
		String getTo() {return _to;};

		/**
		 * @param to The encoding, to convert to
		 */
		void setTo(const String &to) {_to = to;};

		/**
		 * Switches the endianity of a string.
		 *
		 * @param string Array containing the characters of a string.
		 * @param length Length of the string in bytes
		 * @param bitCount Number of bits used for each character.
		 *
		 * @return Array of characters with the opposite endianity
		 */
		static char *switchEndian(const char *string, int length, int bitCount);

		/**
		 * Computes length (in bytes) of a string in a given encoding. 
		 * The string must be zero ended. Similar to strlen
		 * (could be used instead of strlen).
		 *
		 * @param string String, which size should be computed.
		 * @param encoding Encoding of the string.
		 *
		 * @return Size of the string in bytes.
		 */
		static size_t stringLength(const char *string, const String &encoding);
	
	private:
		/** The encoding, which is currently being converted to */
		String _to;

		/** The encoding, which is currently being converted from */
		String _from;

		/**
		 * Takes care of transliteration and calls conversion
		 *
		 * The result has to be freed after use.
		 *
		 * @param to Name of the encoding the strings will be converted to
		 * @param from Name of the encoding the strings will be converted from
		 * @param string String that should be converted.
		 * @param length Length of the string to convert in bytes.
		 *
		 * @return Converted string (must be freed) or nullptr if the conversion failed
		 */
		static char *convertWithTransliteration(const String &to, const String &from, const char *string, size_t length);

		/**
		 * Calls as many conversion functions as possible or until the conversion
		 * succeeds. It first tries to use iconv, then it tries to use platform
		 * specific functions and after that it tries to use TransMan mapping.
		 *
		 * The result has to be freed after use.
		 *
		 * @param to Name of the encoding the strings will be converted to
		 * @param from Name of the encoding the strings will be converted from
		 * @param string String that should be converted.
		 * @param length Length of the string to convert in bytes.
		 *
		 * @return Converted string (must be freed) or nullptr if the conversion failed
		 */
		static char *conversion(const String &to, const String &from, const char *string, size_t length);

		/**
		 * Tries to convert the string using iconv.
		 *
		 * The result has to be freed after use.
		 *
		 * @param to Name of the encoding the strings will be converted to
		 * @param from Name of the encoding the strings will be converted from
		 * @param string String that should be converted.
		 * @param length Length of the string to convert in bytes.
		 *
		 * @return Converted string (must be freed) or nullptr if the conversion failed
		 */
		static char *convertIconv(const char *to, const char *from, const char *string, size_t length);

		/**
		 * Tries to use the TransMan to convert the string. It can convert only
		 * between UTF-32 and the current GUI charset. It also tries to convert
		 * from the current GUI charset to UTF-32 and then it calls convert() again.
		 *
		 * The result has to be freed after use.
		 *
		 * @param to Name of the encoding the strings will be converted to
		 * @param from Name of the encoding the strings will be converted from
		 * @param string String that should be converted.
		 * @param length Length of the string to convert in bytes.
		 *
		 * @return Converted string (must be freed) or nullptr if the conversion failed
		 */
		static char *convertTransManMapping(const char *to, const char *from, const char *string, size_t length);

		/**
		 * Uses conversion table to convert the string to unicode and from that
		 * to the final encoding. Important encodings, that aren't supported by
		 * all backends should go here.
		 *
		 * The result has to be freed after use.
		 *
		 * @param to Name of the encoding the strings will be converted to
		 * @param from Name of the encoding the strings will be converted from
		 * @param string String that should be converted.
		 * @param length Length of the string to convert in bytes.
		 *
		 * @return Converted string (must be freed) or nullptr if the conversion failed
		 */
		static char *convertConversionTable(const char *to, const char *from, const char *string, size_t length);

		/**
		 * Transliterates cyrillic string in iso-8859-5 encoding and returns
		 * it's ASCII (latin) form.
		 *
		 * The result has to be freed after use.
		 *
		 * @param string String that should be converted
		 *
		 * @return Transliterated string in ASCII (must be freed) or nullptr on fail.
		 */
		static char *transliterateCyrillic(const char *string);

		/**
		 * Transliterates cyrillic in UTF-32 string.
		 *
		 * The result has to be freed after use.
		 *
		 * @param string String that should be converted
		 * @param length Length of the string in bytes
		 *
		 * @return Transliterated string in UTF-32 (must be freed) or nullptr on fail.
		 */
		static uint32 *transliterateUTF32(const uint32 *string, size_t length);
};

}

#endif // COMMON_ENCODING_H
