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

#ifndef MTROPOLIS_DATA_H
#define MTROPOLIS_DATA_H

#include "common/array.h"
#include "common/error.h"
#include "common/ptr.h"
#include "common/stream.h"

namespace MTropolis {

namespace Data {

enum ProjectFormat {
	kProjectFormatUnknown,

	kProjectFormatMacintosh,
	kProjectFormatWindows,
	kProjectFormatNeutral,
};

enum DataReadErrorCode {
	kDataReadErrorNone = 0,

	kDataReadErrorUnsupportedRevision,
	kDataReadErrorReadFailed,
	kDataReadErrorUnrecognized,
};

namespace DataObjectTypes {

enum DataObjectType {
	kUnknown = 0,

	kProjectCatalog       = 0x3e8,
	kStreamHeader         = 0x3e9,
	kProjectHeader        = 0x3ea,
	kPresentationSettings = 0x3ec,

	kAssetCatalog         = 0xd,
};

} // End of namespace DataObjectTypes

class DataReader {

public:
	DataReader(Common::SeekableReadStreamEndian &stream, ProjectFormat projectFormat);

	bool readU8(uint8 &value);
	bool readU16(uint16 &value);
	bool readU32(uint32 &value);
	bool readU64(uint64 &value);
	bool readS8(int8 &value);
	bool readS16(int16 &value);
	bool readS32(int32 &value);
	bool readS64(int64 &value);
	bool readF32(float &value);
	bool readF64(double &value);
	bool read(void *dest, size_t size);

	// Reads a terminated string where "length" is the number of characters including a null terminator
	bool readTerminatedStr(Common::String &value, size_t length);

	bool readNonTerminatedStr(Common::String &value, size_t length);

	template<size_t TSize>
	bool readChars(char (&arr)[TSize]);

	template<size_t TSize>
	bool readBytes(uint8 (&arr)[TSize]);

	bool skip(size_t count);

	ProjectFormat getProjectFormat() const;

private:
	Common::SeekableReadStreamEndian &_stream;
	ProjectFormat _projectFormat;
};

struct Rect {
	bool load(DataReader &reader);

	int16 top;
	int16 left;
	int16 bottom;
	int16 right;
};

struct Point {
	bool load(DataReader &reader);

	int16 x;
	int16 y;
};

struct Event {
	bool load(DataReader &reader);

	uint32 eventID;
	uint32 eventInfo;
};

class DataObject : public Common::NonCopyable {

public:
	DataObject();
	virtual ~DataObject();
	DataReadErrorCode load(DataObjectTypes::DataObjectType type, uint16 revision, DataReader &reader);

	uint16 getRevision() const;
	DataObjectTypes::DataObjectType getType() const;

protected:
	virtual DataReadErrorCode load(DataReader &reader) = 0;

	DataObjectTypes::DataObjectType _type;
	uint16 _revision;
};

class ProjectHeader : public DataObject {

public:
	uint32 persistFlags;
	uint32 sizeIncludingTag;
	uint16 unknown1;
	uint32 catalogFilePosition;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct PresentationSettings : public DataObject {

public:
	DataReadErrorCode load(DataReader &reader) override;

	uint32 persistFlags;
	uint32 sizeIncludingTag;
	uint8 unknown1[2];
	Point dimensions;
	uint16 bitsPerPixel;
	uint16 unknown4;
};

struct AssetCatalog : public DataObject {
	DataReadErrorCode load(DataReader &reader) override;

	enum {
		kFlag1Deleted = 1,
		kFlag1LimitOnePerSegment = 2,
	};

	struct AssetInfo {
		uint32 flags1;
		uint16 nameLength;
		uint16 alwaysZero;
		uint32 unknown1;     // Possibly scene ID
		uint32 filePosition; // Contains a static value in Obsidian
		uint32 assetType;
		uint32 flags2;
		Common::String name;
	};

	uint32 persistFlags;
	uint32 totalNameSizePlus22;
	uint8 unknown1[4];
	uint32 numAssets;
	Common::Array<AssetInfo> assets;
};

class ProjectCatalog : public DataObject {

public:
	struct StreamDesc {
		char streamType[25];
		uint16 segmentIndexPlusOne;
		uint32 size;
		uint32 pos;
	};

	struct SegmentDesc {
		uint32 segmentID;
		Common::String label;
		Common::String exportedPath;
	};

	uint32 persistFlags;
	uint32 sizeOfStreamAndSegmentDescs;
	uint16 unknown1;
	uint16 unknown2;
	uint32 unknown3;

	Common::Array<SegmentDesc> segments;
	Common::Array<StreamDesc> streams;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

class StreamHeader : public DataObject {

public:
	uint32 marker;
	uint32 sizeIncludingTag;
	char name[17];
	uint8 projectID[2];
	uint8 unknown1[4]; // Seems to be consistent across builds
	uint16 unknown2;   // 0

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

DataReadErrorCode loadDataObject(DataReader &reader, Common::SharedPtr<DataObject> &outObject);

template<size_t TSize>
inline bool DataReader::readBytes(uint8(&arr)[TSize]) {
	return this->read(arr, TSize);
}

template<size_t TSize>
inline bool DataReader::readChars(char (&arr)[TSize]) {
	return this->read(arr, TSize);
}

} // End of namespace Data

} // End of namespace MTropolis

#endif /* MTROPOLIS_DATA_H */
