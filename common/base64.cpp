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

#include "common/base64.h"
#include "common/array.h"
#include "common/memstream.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/types.h"

namespace Common {

static const char *encodingTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

bool b64Validate(String &string) {
	bool paddingStarted = false;
	// Base64 encoded strings uses 4 characters to encode 3 bytes.
	// and thus, a fully encoded string are always divisable by 4.
	// A padding character (=) may be used to ensure divisibility.
	if ((strlen(string.c_str()) % 4) > 0)
		return false;

	String encodingStr(encodingTable);

	// It must also use characters defined in the encoding table,
	for (char c : string) {
		if (!encodingStr.contains(c)) {
			// or the padding character (=).
			if (c != '=')
				return false;
		}

		if (paddingStarted && c != '=') {
			// Excess data after padding are not allowed.
			return false;
		}

		if (c == '=' && !paddingStarted) {
			paddingStarted = true;
		}
	}

	return true;
}

String b64EncodeString(String &string) {
	String out;
	int32 val = 0, valb = -6;
	for (uint8 c : string) {
		val = (val << 8) + c;
		valb += 8;
		while (valb >= 0) {
			out += (encodingTable[(val >> valb) & 0x3F]);
			valb -= 6;
		}
	}
	if (valb > -6)
		out += (encodingTable[((val << 8) >> (valb + 8)) & 0x3F]);
	while (out.size() % 4)
		out += "=";
	return out;
}

String b64EncodeStream(ReadStream &stream) {
	String out;
	int32 val = 0, valb = -6;
	while (true) {
		uint8 b = stream.readByte();
		if (stream.eos())
			break;
		val = (val << 8) + b;
		valb += 8;
		while (valb >= 0) {
			out += (encodingTable[(val >> valb) & 0x3F]);
			valb -= 6;
		}
	}
	if (valb > -6)
		out += (encodingTable[((val << 8) >> (valb + 8)) & 0x3F]);
	while (out.size() % 4)
		out += "=";
	return out;
}

String b64EncodeData(void *dataPtr, size_t dataSize) {
	String out;
	int32 val = 0, valb = -6;
	uint8 *p = (uint8 *)dataPtr;

	for (size_t i = 0; i < dataSize; i++) {
		uint8 b = p[i];
		val = (val << 8) + b;
		valb += 8;
		while (valb >= 0) {
			out += (encodingTable[(val >> valb) & 0x3F]);
			valb -= 6;
		}
	}
	if (valb > -6)
		out += (encodingTable[((val << 8) >> (valb + 8)) & 0x3F]);
	while (out.size() % 4)
		out += "=";
	return out;
}

String b64DecodeString(String &string) {
	String out;
	Common::Array<int> T(256, -1);

	if (!b64Validate(string))
		// Return empty string.
		return out;

	for (int i = 0; i < 64; i++) {
		T[encodingTable[i]] = i;
	}

	int val = 0, valb = -8;
	for (char c : string) {
		if (T[c] == -1)
			break;
		val = (val << 6) + T[c];
		valb += 6;
		if (valb >= 0) {
			out += (char((val >> valb) & 0xFF));
			valb -= 8;
		}
	}

	return out;
}

MemoryReadStream *b64DecodeStream(String &string, uint32 outputLength) {
	char *out = (char *)malloc(outputLength);
	Common::Array<int> T(256, -1);

	if (!b64Validate(string)) {
		// Return nothing.
		free(out);
		return nullptr;
	}

	for (int i = 0; i < 64; i++) {
		T[encodingTable[i]] = i;
	}

	int val = 0, valb = -8, valc = 0;
	for (char c : string) {
		if (T[c] == -1)
			break;
		val = (val << 6) + T[c];
		valb += 6;
		if (valb >= 0) {
			out[valc++] = (char((val >> valb) & 0xFF));
			valb -= 8;
		}
	}

	return new MemoryReadStream((const byte *)out, outputLength, DisposeAfterUse::YES);
}

bool b64DecodeData(String &string, void *dataPtr) {
	uint8 *p = (uint8 *)dataPtr;
	Common::Array<int> T(256, -1);

	if (!b64Validate(string))
		return false;

	for (int i = 0; i < 64; i++) {
		T[encodingTable[i]] = i;
	}

	int val = 0, valb = -8, valc = 0;
	for (char c : string) {
		if (T[c] == -1)
			break;
		val = (val << 6) + T[c];
		valb += 6;
		if (valb >= 0) {
			p[valc++] = (char((val >> valb) & 0xFF));
			valb -= 8;
		}
	}

	return true;
}

} // End of namespace Common
