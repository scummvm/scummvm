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

#ifndef GRIM_SAVEGAME_H
#define GRIM_SAVEGAME_H

#include "common/savefile.h"

#include "math/mathfwd.h"

namespace Grim {

class Color;

class SaveGame {
public:
	static SaveGame *openForLoading(const Common::String &filename);
	static SaveGame *openForSaving(const Common::String &filename);
	~SaveGame();

	/**
	 * Major savegame version.
	 * If a savegame has a different major version than SAVEGAME_MAJOR_VERSION
	 * it cannot be loaded.
	 */
	static uint SAVEGAME_MAJOR_VERSION;
	/**
	 * Minor savegame version.
	 * A savegame with a different minor version than SAVEGAME_MINOR_VERSION and
	 * the same major version than SAVEGAME_MAJOR_VERSION is compatible with
	 * the current loading code.
	 */
	static uint SAVEGAME_MINOR_VERSION;

	bool isCompatible() const;

	uint saveMajorVersion() const;
	uint saveMinorVersion() const;
	uint32 beginSection(uint32 sectionTag);
	void endSection();
	void read(void *data, int size);
	void write(const void *data, int size);
	uint64 readLEUint64();
	uint32 readLEUint32();
	uint16 readLEUint16();
	int32 readLESint32();
	bool readBool();
	byte readByte();
	void writeLEUint64(uint64 data);
	void writeLEUint32(uint32 data);
	void writeLEUint16(uint16 data);
	void writeLESint32(int32 data);
	void writeBool(bool data);
	void writeByte(byte data);
	void writeString(const Common::String &string);

	void writeVector3d(const Math::Vector3d &vec);
	void writeColor(const Grim::Color &color);
	void writeFloat(float data);
	Math::Vector3d readVector3d();
	Grim::Color readColor();
	float readFloat();
	Common::String readString();

	void checkAlloc(int size);

protected:
	SaveGame();

	uint _majorVersion;
	uint _minorVersion;
	bool _saving;
	Common::InSaveFile *_inSaveFile;
	Common::OutSaveFile *_outSaveFile;
	uint32 _currentSection;
	uint32 _sectionSize;
	uint32 _sectionAlloc;
	uint32 _sectionPtr;
	byte *_sectionBuffer;

	static const int _allocAmmount = 1048576;
};

} // end of namespace Grim

#endif
