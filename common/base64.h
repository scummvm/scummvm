/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef COMMON_BASE64_H
#define COMMON_BASE64_H

#include "common/scummsys.h"

namespace Common {

/**
 * @defgroup common_base64 Base64 encoding & decoding.
 * @ingroup common
 *
 * @brief API for encoding and decoding Base64 strings.
 *
 * @{
 */

class MemoryReadStream;
class ReadStream;
class String;

/**
 * Validates a string to see if its a properly Base64 encoded string.
 * This gets called when using the decode functions.
 * @param[in] string	the string to validate
 * @return true on success, false if an error occurred.
 */
bool b64Validate(String &string);

/**
 * Encodes a string into a Base64 encoded string.
 * @param[in] string	the string to encode
 * @return String containing the Base64 encoded string.
 */
String b64EncodeString(String &string);

/**
 * Encodes a ReadStream into a Base64 encoded string.
 * @param[in] stream	the stream to encode
 * @return String containing the Base64 encoded string.
 */
String b64EncodeStream(ReadStream &stream);

/**
 * Encodes a data pointer into a Base64 encoded string.
 * @param[in] dataPtr	pointer to data to encode
 * @return String containing the Base64 encoded string.
 */
String b64EncodeData(void *dataPtr, size_t dataSize);

/**
 * Decodes a Base64 encoded string into a regular string.
 * @param[in] string	base64 encoded string to decode
 * @return String containing the decoded result, empty string if an error occurred.
 */
String b64DecodeString(String &string);

/**
 * Decodes a Base64 encoded string into a MemoryReadStream, its contents
 * will be disposed when deleting the stream.
 * @param[in] string	base64 encoded string to decode
 * @return MemoryReadStream pointer containing the decoded result, nullptr if an error occurred
 */
MemoryReadStream *b64DecodeStream(String &string, uint32 outputLength);

/**
 * Decodes a Base64 encoded string into the set data pointer
 * @param[in] string	base64 encoded string to decode
 * @return true on success, false if an error occurred
 */
bool b64DecodeData(String &string, void *dataPtr);

/** @} */

} // End of namespace Common

#endif
