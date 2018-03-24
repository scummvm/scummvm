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

#include "bladerunner/savefile.h"

#include "bladerunner/boundingbox.h"
#include "bladerunner/vector.h"

#include "common/rect.h"
#include "common/savefile.h"

namespace BladeRunner {

SaveFileWriteStream::SaveFileWriteStream()
	: MemoryWriteStreamDynamic(DisposeAfterUse::YES) {
}

void SaveFileWriteStream::debug(char *p) {
	write(p, strlen(p) + 1);
}

void SaveFileWriteStream::padBytes(int count) {
	for (int i = 0; i < count; ++i) {
		writeByte(0);
	}
}

void SaveFileWriteStream::writeInt(int v) {
	writeUint32LE(v);
}

void SaveFileWriteStream::writeFloat(int v) {
	writeFloatLE(v);
}

void SaveFileWriteStream::writeBool(bool v) {
	writeUint32LE(v);
}

void SaveFileWriteStream::writeStringSz(const Common::String &s, int sz) {
	assert(s.size() < (uint)sz);
	write(s.begin(), s.size());
	padBytes((uint)sz - s.size());
}

void SaveFileWriteStream::writeVector2(const Vector2 &v) {
	writeFloatLE(v.x);
	writeFloatLE(v.y);
}

void SaveFileWriteStream::writeVector3(const Vector3 &v) {
	writeFloatLE(v.x);
	writeFloatLE(v.y);
	writeFloatLE(v.z);
}

void SaveFileWriteStream::writeRect(const Common::Rect &v) {
	writeUint32LE(v.left);
	writeUint32LE(v.top);
	writeUint32LE(v.right);
	writeUint32LE(v.bottom);
}

void SaveFileWriteStream::writeBoundingBox(const BoundingBox &v) {
	float x0, y0, z0, x1, y1, z1;

	v.getXYZ(&x0, &y0, &z0, &x1, &y1, &z1);
	writeFloatLE(x0);
	writeFloatLE(y0);
	writeFloatLE(z0);
	writeFloatLE(x1);
	writeFloatLE(y1);
	writeFloatLE(z1);

	// Bounding boxes have a lot of extra data that's never actually used
	for (int i = 0; i != 96; ++i) {
		writeFloatLE(0.0f);
	}
}

SaveFileReadStream::SaveFileReadStream(const byte *dataPtr, uint32 dataSize)
	: MemoryReadStream(dataPtr, dataSize, DisposeAfterUse::YES) {
}

int SaveFileReadStream::readInt() {
	return readUint32LE();
}

float SaveFileReadStream::readFloat() {
	return readFloatLE();
}

bool SaveFileReadStream::readBool() {
	return readUint32LE();
}

Common::String SaveFileReadStream::readStringSz(int sz) {
	char *buf = (char *)malloc(sz);
	read(buf, sz);
	Common::String result = buf;
	free(buf);
	return result;
}

Vector2 SaveFileReadStream::readVector2() {
	Vector2 result;
	result.x = readFloatLE();
	result.y = readFloatLE();
	return result;
}

Vector3 SaveFileReadStream::readVector3() {
	Vector3 result;
	result.x = readFloatLE();
	result.y = readFloatLE();
	result.z = readFloatLE();
	return result;
}

Common::Rect SaveFileReadStream::readRect() {
	Common::Rect result;
	result.left = readUint32LE();
	result.top = readUint32LE();
	result.right = readUint32LE();
	result.bottom = readUint32LE();
	return result;
}

BoundingBox SaveFileReadStream::readBoundingBox() {
	float x0, y0, z0, x1, y1, z1;

	x0 = readFloatLE();
	y0 = readFloatLE();
	z0 = readFloatLE();
	x1 = readFloatLE();
	y1 = readFloatLE();
	z1 = readFloatLE();

	// Bounding boxes have a lot of extra data that's never actually used
	skip(384);

	return BoundingBox(x0, y0, z0, x1, y1, z1);
}



} // End of namespace BladeRunner
