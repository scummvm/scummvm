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

class SaveFile {
	Common::OutSaveFile              *_saveFile;
	Common::MemoryWriteStreamDynamic  _stream;
public:
	SaveFile(Common::OutSaveFile *saveFile);

	// bool err();
	void finalize();

	void padBytes(int count);
	void write(bool v);
	void write(int v);
	void write(uint32 v);
	void write(byte  v);
	void write(float v);
	void debug(char *p);
	void write(char *p, int sz);
	void write(Common::String &s, int sz);
	void write(const Vector2 &v);
	void write(const Vector3 &v);
	void write(const Common::Rect &v);
	void write(const BoundingBox &v);
};

} // End of namespace BladeRunner

#endif
