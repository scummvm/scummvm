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

SaveFile::SaveFile(Common::OutSaveFile *saveFile)
	: _saveFile(saveFile),
	  _stream(DisposeAfterUse::YES)
{
}

void SaveFile::finalize() {
	_saveFile->writeUint32LE(_stream.size() + 4);
	_saveFile->write(_stream.getData(), _stream.size());
	_saveFile->finalize();
}

void SaveFile::padBytes(int count) {
	for (int i = 0; i < count; ++i) {
		_stream.writeByte(0);
	}
}

void SaveFile::write(bool v) {
	_stream.writeUint32LE(v);
}

void SaveFile::write(int v) {
	_stream.writeUint32LE(v);
}

void SaveFile::write(uint32 v) {
	_stream.writeUint32LE(v);
}

void SaveFile::write(byte v) {
	_stream.writeByte(v);
}

void SaveFile::write(float v) {
	_stream.writeFloatLE(v);
}

void SaveFile::debug(char *p) {
	_stream.write(p, strlen(p) + 1);
}

void SaveFile::write(char *p, int sz) {
	_stream.write(p, sz);
}

void SaveFile::write(Common::String &s, int sz) {
	assert(s.size() < (uint)sz);
	_stream.write(s.begin(), s.size());
	padBytes((uint)sz - s.size());
}

void SaveFile::write(const Vector2 &v) {
	_stream.writeFloatLE(v.x);
	_stream.writeFloatLE(v.y);
}

void SaveFile::write(const Vector3 &v) {
	_stream.writeFloatLE(v.x);
	_stream.writeFloatLE(v.y);
	_stream.writeFloatLE(v.z);
}

void SaveFile::write(const Common::Rect &v) {
	_stream.writeUint32LE(v.left);
	_stream.writeUint32LE(v.top);
	_stream.writeUint32LE(v.right);
	_stream.writeUint32LE(v.bottom);
}

void SaveFile::write(const BoundingBox &v) {
	float x0, y0, z0, x1, y1, z1;

	v.getXYZ(&x0, &y0, &z0, &x1, &y1, &z1);
	_stream.writeFloatLE(x0);
	_stream.writeFloatLE(y0);
	_stream.writeFloatLE(z0);
	_stream.writeFloatLE(x1);
	_stream.writeFloatLE(y1);
	_stream.writeFloatLE(z1);

	// Bounding boxes have a lot of extra data that's never actually used
	for (int i = 0; i != 96; ++i) {
		_stream.writeFloatLE(0.0f);
	}
}

} // End of namespace BladeRunner
