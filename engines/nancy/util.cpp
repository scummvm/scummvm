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
 */

#include "engines/nancy/nancy.h"
#include "engines/nancy/util.h"
#include "common/system.h"

namespace Nancy {

void readRect(Common::SeekableReadStream &stream, Common::Rect &inRect) {
	inRect.left = stream.readSint32LE();
	inRect.top = stream.readSint32LE();
	inRect.right = stream.readSint32LE();
	inRect.bottom = stream.readSint32LE();

	// TVD's rects are non-inclusive
	if (g_nancy->getGameType() > kGameTypeVampire && !inRect.isEmpty()) {
		++inRect.right;
		++inRect.bottom;
	}
}

void readRect(Common::Serializer &stream, Common::Rect &inRect, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	Common::Serializer::Version version = stream.getVersion();
	if (version >= minVersion && version <= maxVersion) {
		stream.syncAsSint32LE(inRect.left);
		stream.syncAsSint32LE(inRect.top);
		stream.syncAsSint32LE(inRect.right);
		stream.syncAsSint32LE(inRect.bottom);

		// TVD's rects are non-inclusive
		if (version > kGameTypeVampire && !inRect.isEmpty()) {
			++inRect.right;
			++inRect.bottom;
		}
	}
}

void readRectArray(Common::SeekableReadStream &stream, Common::Array<Common::Rect> &inArray, uint num, uint totalNum) {
	uint startSize = inArray.size();
	inArray.resize(num + startSize);

	for (Common::Rect *rect = &inArray[startSize]; rect != inArray.end(); ++rect) {
		readRect(stream, *rect);
	}

	if (totalNum == 0) {
		totalNum = num;
	}

	stream.skip(totalNum > num ? (totalNum - num) * 16 : 0);
}

void readRectArray(Common::Serializer &stream, Common::Array<Common::Rect> &inArray, uint num, uint totalNum, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	Common::Serializer::Version version = stream.getVersion();
	if (version >= minVersion && version <= maxVersion) {
		uint startSize = inArray.size();
		inArray.resize(num + startSize);

		for (Common::Rect *rect = &inArray[startSize]; rect != inArray.end(); ++rect) {
			stream.syncAsSint32LE(rect->left);
			stream.syncAsSint32LE(rect->top);
			stream.syncAsSint32LE(rect->right);
			stream.syncAsSint32LE(rect->bottom);

			// TVD's rects are non-inclusive
			if (version > kGameTypeVampire && !rect->isEmpty()) {
				++rect->right;
				++rect->bottom;
			}
		}

		if (totalNum == 0) {
			totalNum = num;
		}

		stream.skip(totalNum > num ? (totalNum - num) * 16 : 0);
	}
}

void readRect16(Common::SeekableReadStream &stream, Common::Rect &inRect) {
	inRect.left = stream.readSint16LE();
	inRect.top = stream.readSint16LE();
	inRect.right = stream.readSint16LE();
	inRect.bottom = stream.readSint16LE();

	// TVD's rects are non-inclusive
	if (g_nancy->getGameType() > kGameTypeVampire && !inRect.isEmpty()) {
		++inRect.right;
		++inRect.bottom;
	}
}

void readRect16(Common::Serializer &stream, Common::Rect &inRect, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	Common::Serializer::Version version = stream.getVersion();
	if (version >= minVersion && version <= maxVersion) {
		stream.syncAsSint16LE(inRect.left);
		stream.syncAsSint16LE(inRect.top);
		stream.syncAsSint16LE(inRect.right);
		stream.syncAsSint16LE(inRect.bottom);

		// TVD's rects are non-inclusive
		if (version > kGameTypeVampire && !inRect.isEmpty()) {
			++inRect.right;
			++inRect.bottom;
		}
	}
}

void readRectArray16(Common::SeekableReadStream &stream, Common::Array<Common::Rect> &inArray, uint num, uint totalNum) {
	uint startSize = inArray.size();
	inArray.resize(num + startSize);

	for (Common::Rect *rect = &inArray[startSize]; rect != inArray.end(); ++rect) {
		readRect16(stream, *rect);
	}

	if (totalNum == 0) {
		totalNum = num;
	}

	stream.skip(totalNum > num ? (totalNum - num) * 8 : 0);
}

void readRectArray16(Common::Serializer &stream, Common::Array<Common::Rect> &inArray, uint num, uint totalNum, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	Common::Serializer::Version version = stream.getVersion();
	if (version >= minVersion && version <= maxVersion) {
		uint startSize = inArray.size();
		inArray.resize(num + startSize);

		for (Common::Rect *rect = &inArray[startSize]; rect != inArray.end(); ++rect) {
			stream.syncAsSint16LE(rect->left);
			stream.syncAsSint16LE(rect->top);
			stream.syncAsSint16LE(rect->right);
			stream.syncAsSint16LE(rect->bottom);

			// TVD's rects are non-inclusive
			if (version > kGameTypeVampire && !rect->isEmpty()) {
				++rect->right;
				++rect->bottom;
			}
		}

		if (totalNum == 0) {
			totalNum = num;
		}

		stream.skip(totalNum > num ? (totalNum - num) * 8 : 0);
	}
}

void readFilename(Common::SeekableReadStream &stream, Common::String &inString) {
	char buf[33] = "";

	if (g_nancy->getGameType() <= kGameTypeNancy2) {
		// Older games only support 8-character filenames, and stored them in a 10 char buffer
		stream.read(buf, 10);
		buf[9] = '\0';
	} else {
		// Later games support 32-character filenames
		stream.read(buf, 33);
		buf[32] = '\0';
	}

	inString = buf;
}

void readFilename(Common::Serializer &stream, Common::String &inString, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	Common::Serializer::Version version = stream.getVersion();
	if (version >= minVersion && version <= maxVersion) {
		char buf[33] = "";

		if (version <= kGameTypeNancy2) {
			// Older games only support 8-character filenames, and stored them in a 10 char buffer
			stream.syncBytes((byte *)buf, 10);
			buf[9] = '\0';
		} else {
			// Later games support 32-character filenames
			stream.syncBytes((byte *)buf, 33);
			buf[32] = '\0';
		}

		inString = buf;
	}
}

void readFilenameArray(Common::SeekableReadStream &stream, Common::Array<Common::String> &inArray, uint num) {
	inArray.resize(num);
	for (Common::String &str : inArray) {
		readFilename(stream, str);
	}
}

void readFilenameArray(Common::Serializer &stream, Common::Array<Common::String> &inArray, uint num, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	Common::Serializer::Version version = stream.getVersion();
	if (version >= minVersion && version <= maxVersion) {
		inArray.resize(num);
		char buf[33] = "";

		for (Common::String &str : inArray) {
			if (version <= kGameTypeNancy2) {
				// Older games only support 8-character filenames, and stored them in a 10 char buffer
				stream.syncBytes((byte *)buf, 10);
				buf[9] = '\0';
			} else {
				// Later games support 32-character filenames
				stream.syncBytes((byte *)buf, 33);
				buf[32] = '\0';
			}

			str = buf;
		}
	}
}

void readFilenameArray(Common::SeekableReadStream &stream, Common::Array<Common::Path> &inArray, uint num) {
	inArray.resize(num);
	for (Common::Path &str : inArray) {
		readFilename(stream, str);
	}
}

void readFilenameArray(Common::Serializer &stream, Common::Array<Common::Path> &inArray, uint num, Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	Common::Serializer::Version version = stream.getVersion();
	if (version >= minVersion && version <= maxVersion) {
		inArray.resize(num);
		for (Common::Path &str : inArray) {
			readFilename(stream, str, minVersion, maxVersion);
		}
	}
}

// A text line will often be broken up into chunks separated by nulls, use
// this function to put it back together as a Common::String
void assembleTextLine(char *rawCaption, Common::String &output, uint size) {
	for (uint i = 0; i < size; ++i) {
		// A single line can be broken up into bits, look for them and
		// concatenate them when we're done
		if (rawCaption[i] != 0) {
			Common::String newBit(rawCaption + i);
			output += newBit;
			i += newBit.size();
		}
	}

	// Fix spaces at the end of the string in nancy1
	output.trim();

	// Scan the text line for doubly-closed tokens; happens in some strings in The Vampire Diaries
	uint pos = Common::String::npos;
	while (pos = output.find(">>"), pos != Common::String::npos) {
		output.replace(pos, 2, ">");
	}
}

bool DeferredLoader::load(uint32 endTime) {
	uint32 loopStartTime = g_system->getMillis();
	uint32 loopTime = 0; // Stores the loop that took the longest time to complete

	do {
		if (loadInner()) {
			return true;
		}

		uint32 loopEndTime = g_system->getMillis();
		loopTime = MAX<uint32>(loopEndTime - loopStartTime, loopTime);
		loopStartTime = loopEndTime;

		// We do this after at least one execution to avoid the case where the game runs below
		// the target fps, and thus has no spare milliseconds until the next frame. This way
		// we ensure loading actually gets done, at the expense of some lag
		if (g_system->getMillis() < endTime) {
			break;
		}
	} while (loopTime + loopStartTime < endTime);

	return false;
}

} // End of namespace Nancy
