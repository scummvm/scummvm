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

#ifndef BLADERUNNER_SAVEFILE_H
#define BLADERUNNER_SAVEFILE_H

#include "common/array.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/types.h"

#include "graphics/surface.h"

#include "engines/savestate.h"

namespace Common {
class OutSaveFile;
class String;
struct Rect;
}

namespace BladeRunner {

class Vector2;
class Vector3;
class BoundingBox;


struct SaveFileHeader {
	uint8              _version;
	Common::String     _name;
	int                _year;
	int                _month;
	int                _day;
	int                _hour;
	int                _minute;
	uint32             _playTime;
	Graphics::Surface *_thumbnail;
};

class SaveFileManager {
private:
	static const uint32 kTag = MKTAG('B', 'R', 'S', 'V');
	// kVersion: 3 as of Feb 5th 2020 (UTC) - ScummVM development version 2.2.0git
	// kVersion: 4 as of Apr 17th 2021 - Added full-size thumbnails
	static const uint32 kVersion = 4;

public:
	// kVersion
	// ----------
	//        2:: max of 32 characters for the saved game name
	//        3:: max of 41 characters for the saved game name (this matches the original game's setting)
	static const uint32 kNameLengthV2  = 32;
	static const uint32 kNameLength    = 41;
	static const uint32 kThumbnailSize = 9600; // 80x60x16bpp

	static SaveStateList list(const MetaEngine *metaEngine, const Common::String &target);
	static SaveStateDescriptor queryMetaInfos(const MetaEngine *metaEngine, const Common::String &target, int slot);

	static Common::InSaveFile *openForLoading(const Common::String &target, int slot);
	static Common::OutSaveFile *openForSaving(const Common::String &target, int slot);

	static bool remove(const Common::String &target, int slot);

	static bool readHeader(Common::SeekableReadStream &in, SaveFileHeader &header, bool skipThumbnail = true);
	static bool writeHeader(Common::WriteStream &out, SaveFileHeader &header);

};

class SaveFileWriteStream : public Common::WriteStream {
private:
	Common::WriteStream &_s;

public:
	SaveFileWriteStream(Common::WriteStream &s);

	uint32 write(const void *dataPtr, uint32 dataSize) override { return _s.write(dataPtr, dataSize); }
	bool flush() override { return _s.flush(); }
	int64 pos() const override { return _s.pos(); }

	void debug(char *p);

	void padBytes(int count);

	void writeInt(int32 v); // this writes a 4 byte int (uses writeUint32LE)
	void writeFloat(float v);
	void writeBool(bool v);
	void writeStringSz(const Common::String &s, uint sz);
	void writeVector2(const Vector2 &v);
	void writeVector3(const Vector3 &v);
	void writeRect(const Common::Rect &v);
	void writeBoundingBox(const BoundingBox &v, bool serialized);
};

class SaveFileReadStream : public Common::SeekableReadStream {
private:
	Common::SeekableReadStream &_s;

public:
	SaveFileReadStream(Common::SeekableReadStream &s);

	bool eos() const override { return _s.eos(); }
	uint32 read(void *dataPtr, uint32 dataSize) override { return _s.read(dataPtr, dataSize); }
	int64 pos() const override { return _s.pos(); }
	int64 size() const override { return _s.size(); }
	bool seek(int64 offset, int whence = SEEK_SET) override { return _s.seek(offset, whence); }

	int32 readInt();
	float readFloat();
	bool readBool();
	Common::String readStringSz(uint sz);
	Vector2 readVector2();
	Vector3 readVector3();
	Common::Rect readRect();
	BoundingBox readBoundingBox(bool serialized);
};

} // End of namespace BladeRunner

#endif
