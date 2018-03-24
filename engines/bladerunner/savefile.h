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

#ifndef BLADERUNNER_SAVEFILE_H
#define BLADERUNNER_SAVEFILE_H

#include "common/memstream.h"
#include "common/types.h"

namespace Common {
class OutSaveFile;
class String;
struct Rect;
}

namespace BladeRunner {

class Vector2;
class Vector3;
class BoundingBox;

class SaveFileWriteStream : public Common::MemoryWriteStreamDynamic {
public:
	SaveFileWriteStream();

	void debug(char *p);

	void padBytes(int count);

	void writeInt(int v);
	void writeFloat(int v);
	void writeBool(bool v);
	void writeStringSz(const Common::String &s, int sz);
	void writeVector2(const Vector2 &v);
	void writeVector3(const Vector3 &v);
	void writeRect(const Common::Rect &v);
	void writeBoundingBox(const BoundingBox &v);
};

class SaveFileReadStream : public Common::MemoryReadStream {
public:
	SaveFileReadStream(const byte *dataPtr, uint32 dataSize);

	int readInt();
	float readFloat();
	bool readBool();
	Common::String readStringSz(int sz);
	Vector2 readVector2();
	Vector3 readVector3();
	Common::Rect readRect();
	BoundingBox readBoundingBox();
};

} // End of namespace BladeRunner

#endif
