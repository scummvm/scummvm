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

#ifndef NANCY_UTIL_H
#define NANCY_UTIL_H

#include "common/rect.h"
#include "common/serializer.h"

namespace Nancy {

void readRect(Common::SeekableReadStream &stream, Common::Rect &inRect);
void readRect(Common::Serializer &stream, Common::Rect &inRect, Common::Serializer::Version minVersion = 0, Common::Serializer::Version maxVersion = Common::Serializer::kLastVersion);
void readRectArray(Common::SeekableReadStream &stream, Common::Array<Common::Rect> &inArray, uint num);
void readRectArray(Common::Serializer &stream, Common::Array<Common::Rect> &inArray, uint num, Common::Serializer::Version minVersion = 0, Common::Serializer::Version maxVersion = Common::Serializer::kLastVersion);

void readFilename(Common::SeekableReadStream &stream, Common::String &inString);
void readFilename(Common::Serializer &stream, Common::String &inString, Common::Serializer::Version minVersion = 0, Common::Serializer::Version maxVersion = Common::Serializer::kLastVersion);
void readFilenameArray(Common::SeekableReadStream &stream, Common::Array<Common::String> &inArray, uint num);
void readFilenameArray(Common::Serializer &stream, Common::Array<Common::String> &inArray, uint num, Common::Serializer::Version minVersion = 0, Common::Serializer::Version maxVersion = Common::Serializer::kLastVersion);

// Abstract base class used for loading data that would take too much time in a single frame
class DeferredLoader {
public:
	DeferredLoader() {}
	virtual ~DeferredLoader() {}

	// Calls loadInner() one or many times, until its allotted time is done
	bool load(uint32 endTime);

protected:
	// Contains the actual loading logic, split up into tasks that are as small as possible
	virtual bool loadInner() = 0;
};

} // End of namespace Nancy

#endif // NANCY_UTIL_H
