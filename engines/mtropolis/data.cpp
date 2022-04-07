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

#include "mtropolis/data.h"

namespace MTropolis {

namespace Data {


DataReader::DataReader(Common::SeekableReadStreamEndian &stream, ProjectFormat projectFormat) : _stream(stream), _projectFormat(projectFormat) {
}

bool DataReader::readU8(uint8 &value) {
	value = _stream.readByte();
	return !_stream.err();
}

bool DataReader::readU16(uint16 &value) {
	value = _stream.readUint16();
	return !_stream.err();
}

bool DataReader::readU32(uint32 &value) {
	value = _stream.readUint32();
	return !_stream.err();
}

bool DataReader::readU64(uint64 &value) {
	value = _stream.readUint64();
	return !_stream.err();
}

bool DataReader::readS8(int8 &value) {
	value = _stream.readSByte();
	return !_stream.err();
}

bool DataReader::readS16(int16 &value) {
	value = _stream.readSint16();
	return !_stream.err();
}

bool DataReader::readS32(int32 &value) {
	value = _stream.readSint32();
	return !_stream.err();
}

bool DataReader::readS64(int64 &value) {
	value = _stream.readSint64();
	return !_stream.err();
}

bool DataReader::readF32(float &value) {
	value = _stream.readFloat();
	return !_stream.err();
}

bool DataReader::readF64(double &value) {
	value = _stream.readDouble();
	return !_stream.err();
}

bool DataReader::read(void *dest, size_t size) {
	while (size > 0) {
		uint32 thisChunkSize = UINT32_MAX;
		if (size < thisChunkSize) {
			thisChunkSize = static_cast<uint32>(size);
		}

		_stream.read(dest, thisChunkSize);
		if (_stream.err()) {
			return false;
		}

		dest = static_cast<char *>(dest) + thisChunkSize;
		size -= thisChunkSize;
	}

	return true;
}

bool DataReader::readTerminatedStr(Common::String& value, size_t size) {
	if (size > 0) {
		Common::Array<char> strChars;
		strChars.resize(size);
		if (!this->read(&strChars[0], size)) {
			return false;
		}
		if (strChars[size - 1] != 0) {
			return false;
		}
		value = Common::String(&strChars[0], size - 1);
	} else {
		value.clear();
	}

	return true;
}

bool DataReader::readNonTerminatedStr(Common::String &value, size_t size) {
	if (size > 0) {
		Common::Array<char> strChars;
		strChars.resize(size);
		if (!this->read(&strChars[0], size)) {
			return false;
		}
		value = Common::String(&strChars[0], size);
	} else {
		value.clear();
	}

	return true;
}

bool DataReader::skip(size_t count) {
	while (count > 0) {
		uint64 thisChunkSize = INT64_MAX;
		if (count < thisChunkSize) {
			thisChunkSize = static_cast<uint64>(count);
		}

		if (!_stream.seek(static_cast<int64>(count), SEEK_CUR)) {
			return false;
		}

		count -= static_cast<size_t>(thisChunkSize);
	}
	return true;
}

ProjectFormat DataReader::getProjectFormat() const {
	return _projectFormat;
}

bool Rect::load(DataReader &reader) {
	if (reader.getProjectFormat() == kProjectFormatMacintosh)
		return reader.readS16(top) && reader.readS16(left) && reader.readS16(bottom) && reader.readS16(right);
	else if (reader.getProjectFormat() == kProjectFormatWindows)
		return reader.readS16(left) && reader.readS16(top) && reader.readS16(right) && reader.readS16(bottom);
	else
		return false;
}

bool Point::load(DataReader &reader) {
	if (reader.getProjectFormat() == kProjectFormatMacintosh)
		return reader.readS16(y) && reader.readS16(x);
	else if (reader.getProjectFormat() == kProjectFormatWindows)
		return reader.readS16(x) && reader.readS16(y);
	else
		return false;
}

bool Event::load(DataReader& reader) {
	return reader.readU32(eventID) && reader.readU32(eventInfo);
}

DataObject::DataObject() : _type(DataObjectTypes::kUnknown), _revision(0) {
}

DataObject::~DataObject() {
}

DataReadErrorCode DataObject::load(DataObjectTypes::DataObjectType type, uint16 revision, DataReader &reader) {
	_type = type;
	_revision = revision;
	return this->load(reader);
}

uint16 DataObject::getRevision() const {
	return _revision;
}

DataObjectTypes::DataObjectType DataObject::getType() const {
	return _type;
}

DataReadErrorCode ProjectHeader::load(DataReader &reader) {
	if (_revision != 0) {
		return kDataReadErrorUnsupportedRevision;
	}

	if (!reader.readU32(persistFlags) || !reader.readU32(sizeIncludingTag) || !reader.readU16(unknown1) || !reader.readU32(catalogFilePosition)) {
		return kDataReadErrorReadFailed;
	}

	return kDataReadErrorNone;
}

DataReadErrorCode PresentationSettings::load(DataReader &reader) {

	if (_revision != 2)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(persistFlags) ||
		!reader.readU32(sizeIncludingTag) ||
		!reader.readBytes(unknown1) ||
		!dimensions.load(reader) ||
		!reader.readU16(bitsPerPixel) ||
		!reader.readU16(unknown4))
		return kDataReadErrorReadFailed;

	if (sizeIncludingTag != 24)
		return kDataReadErrorUnrecognized;

	return kDataReadErrorNone;
}

DataReadErrorCode AssetCatalog::load(DataReader& reader) {
	if (_revision != 4)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(persistFlags) ||
		!reader.readU32(totalNameSizePlus22) ||
		!reader.readBytes(unknown1) ||
		!reader.readU32(numAssets))
		return kDataReadErrorReadFailed;

	assets.resize(numAssets);

	for (size_t i = 0; i < numAssets; i++) {
		AssetInfo &asset = assets[i];
		if (!reader.readU32(asset.flags1) || !reader.readU16(asset.nameLength) || !reader.readU16(asset.alwaysZero) || !reader.readU32(asset.unknown1) || !reader.readU32(asset.filePosition) || !reader.readU32(asset.assetType) || !reader.readU32(asset.flags2))
			return kDataReadErrorReadFailed;

		if (!reader.readNonTerminatedStr(asset.name, asset.nameLength))
			return kDataReadErrorReadFailed;
	}

	return kDataReadErrorNone;
}

DataReadErrorCode Unknown19::load(DataReader &reader) {
	if (_revision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(persistFlags) || !reader.readU32(sizeIncludingTag) || !reader.readBytes(unknown1))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode GlobalObjectInfo::load(DataReader &reader) {
	if (_revision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(persistFlags) || !reader.readU32(sizeIncludingTag) || !reader.readU16(numGlobalModifiers) || !reader.readBytes(unknown1))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode ProjectCatalog::load(DataReader &reader) {
	if (_revision != 2 && _revision != 3) {
		return kDataReadErrorUnsupportedRevision;
	}

	uint16 numSegments;
	uint16 numStreams;

	if (!reader.readU32(persistFlags) || !reader.readU32(sizeOfStreamAndSegmentDescs)
		|| !reader.readU16(numStreams) || !reader.readU16(unknown1)
		|| !reader.readU16(unknown2) || !reader.readU16(numSegments)) {
		return kDataReadErrorReadFailed;
	}

	streams.resize(numStreams);
	segments.resize(numSegments);

	for (size_t i = 0; i < numStreams; i++) {
		StreamDesc &streamDesc = streams[i];

		streamDesc.streamType[24] = 0;
		if (!reader.read(streamDesc.streamType, 24) || !reader.readU16(streamDesc.segmentIndexPlusOne)) {
			return kDataReadErrorReadFailed;
		}

		if (_revision >= 3 && !reader.skip(8)) {
			return kDataReadErrorReadFailed;
		}

		if (!reader.readU32(streamDesc.pos) || !reader.readU32(streamDesc.size)) {
			return kDataReadErrorReadFailed;
		}
	}

	uint32 unknownSegmentPrefix = 0;
	if (!reader.readU32(unknownSegmentPrefix) || unknownSegmentPrefix != 1) {
		return kDataReadErrorUnrecognized;
	}

	for (size_t i = 0; i < numSegments; i++) {
		SegmentDesc &segDesc = segments[i];

		uint16 lengthOfLabel;
		uint16 lengthOfExportedPath;
		if (!reader.readU32(segDesc.segmentID)
			|| !reader.readU16(lengthOfLabel) || !reader.readTerminatedStr(segDesc.label, lengthOfLabel)
			|| !reader.readU16(lengthOfExportedPath) || !reader.readTerminatedStr(segDesc.exportedPath, lengthOfExportedPath)) {
			return kDataReadErrorReadFailed;
		}
	}

	return kDataReadErrorNone;
}

DataReadErrorCode StreamHeader::load(DataReader& reader) {
	if (_revision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(marker) ||
		!reader.readU32(sizeIncludingTag) ||
		!reader.read(name, 16) ||
		!reader.readBytes(projectID) ||
		!reader.readBytes(unknown1) ||
		!reader.readU16(unknown2))
		return kDataReadErrorReadFailed;

	if (sizeIncludingTag != 38)
		return kDataReadErrorUnrecognized;

	name[16] = 0;

	return kDataReadErrorNone;
}

DataReadErrorCode BehaviorModifier::load(DataReader& reader) {
	if (_revision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(unknown1) || !reader.readU32(sizeIncludingTag)
		|| !reader.readBytes(unknown2) || !reader.readU32(guid)
		|| !reader.readU32(unknown4) || !reader.readU16(unknown5)
		|| !reader.readU32(unknown6) || !editorLayoutPosition.load(reader)
		|| !reader.readU16(lengthOfName) || !reader.readU16(numChildren))
		return kDataReadErrorReadFailed;

	if (lengthOfName > 0 && !reader.readTerminatedStr(name, lengthOfName))
		return kDataReadErrorReadFailed;

	if (!reader.readU32(flags) || !enableWhen.load(reader) || !disableWhen.load(reader) || !reader.readBytes(unknown7))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode loadDataObject(DataReader& reader, Common::SharedPtr<DataObject>& outObject) {
	uint32 type;
	uint16 revision;
	if (!reader.readU32(type) || !reader.readU16(revision)) {
		return kDataReadErrorReadFailed;
	}

	DataObject *dataObject = nullptr;
	switch (type) {
	case DataObjectTypes::kProjectHeader:
		dataObject = new ProjectHeader();
		break;
	case DataObjectTypes::kProjectCatalog:
		dataObject = new ProjectCatalog();
		break;
	case DataObjectTypes::kStreamHeader:
		dataObject = new StreamHeader();
		break;
	case DataObjectTypes::kPresentationSettings:
		dataObject = new PresentationSettings();
		break;
	case DataObjectTypes::kAssetCatalog:
		dataObject = new AssetCatalog();
		break;
	case DataObjectTypes::kUnknown19:
		dataObject = new Unknown19();
		break;
	case DataObjectTypes::kGlobalObjectInfo:
		dataObject = new GlobalObjectInfo();
		break;
	case DataObjectTypes::kBehaviorModifier:
		dataObject = new BehaviorModifier();
		break;
	default:
		break;
	}

	if (dataObject == nullptr) {
		return kDataReadErrorUnrecognized;
	}

	Common::SharedPtr<DataObject> sharedPtr(dataObject);
	DataReadErrorCode errorCode = dataObject->load(static_cast<DataObjectTypes::DataObjectType>(type), revision, reader);
	if (errorCode != kDataReadErrorNone) {
		outObject.reset();
		return errorCode;
	}

	outObject = sharedPtr;
	return errorCode;
}

} // End of namespace Data

} // End of namespace MTropolis
