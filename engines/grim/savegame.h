/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
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

	static int SAVEGAME_VERSION;

	int saveVersion() const;
	uint32 beginSection(uint32 sectionTag);
	void endSection();
	uint32 getBufferPos();
	void read(void *data, int size);
	void write(const void *data, int size);
	uint32 readLEUint32();
	uint16 readLEUint16();
	int32 readLESint32();
	bool readBool();
	byte readByte();
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

	int _version;
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
