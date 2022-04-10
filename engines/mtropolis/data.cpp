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
#include "common/debug.h"

#include <float.h>

namespace MTropolis {

namespace Data {


DataReader::DataReader(Common::SeekableReadStreamEndian &stream, ProjectFormat projectFormat) : _stream(stream), _projectFormat(projectFormat) {
}

bool DataReader::readU8(uint8 &value) {
	value = _stream.readByte();
	return checkErrorAndReset();
}

bool DataReader::readU16(uint16 &value) {
	value = _stream.readUint16();
	return checkErrorAndReset();
}

bool DataReader::readU32(uint32 &value) {
	value = _stream.readUint32();
	return checkErrorAndReset();
}

bool DataReader::readU64(uint64 &value) {
	value = _stream.readUint64();
	return checkErrorAndReset();
}

bool DataReader::readS8(int8 &value) {
	value = _stream.readSByte();
	return checkErrorAndReset();
}

bool DataReader::readS16(int16 &value) {
	value = _stream.readSint16();
	return checkErrorAndReset();
}

bool DataReader::readS32(int32 &value) {
	value = _stream.readSint32();
	return checkErrorAndReset();
}

bool DataReader::readS64(int64 &value) {
	value = _stream.readSint64();
	return checkErrorAndReset();
}

bool DataReader::readF32(float &value) {
	value = _stream.readFloat();
	return checkErrorAndReset();
}

bool DataReader::readF64(double &value) {
	value = _stream.readDouble();
	return checkErrorAndReset();
}

bool DataReader::readF80(double &value) {
	uint16_t signAndExponent;
	uint64_t mantissa;
	if (isBigEndian()) {
		if (!readU16(signAndExponent) || !readU64(mantissa))
			return false;
	} else {
		if (!readU64(mantissa) || !readU16(signAndExponent))
			return false;
	}

	uint8_t sign = (signAndExponent >> 15) & 1;
	int16_t exponent = signAndExponent & 0x7fff;

	// Eliminate implicit 1 and truncate from 63 to 47 bits
	mantissa &= 0x7fffffffffffffffULL;
	mantissa >>= 16;

	if (mantissa != 0 || exponent != 0) {
		// Adjust exponent
		exponent = exponent - 15360;
		if (exponent > 2046) {
			// Too big, set to largest finite magnitude
			exponent = 2046;
			mantissa = 0xFFFFFFFFFFFFFUL;
		} else if (exponent < 0) {
			// Subnormal number
			mantissa |= 0x1000000000000ULL;
			if (exponent <= -48) {
				mantissa = 0;
				exponent = 0;
			} else {
				mantissa >>= (-exponent);
				exponent = 0;
			}
		}
	}

	uint64_t recombined = (static_cast<uint64_t>(sign) << 63) | (static_cast<uint64_t>(exponent) << 52) | (static_cast<uint64_t>(mantissa) << 5);
	memcpy(&value, &recombined, 8);

	return true;
}

bool DataReader::read(void *dest, size_t size) {
	while (size > 0) {
		uint32 thisChunkSize = UINT32_MAX;
		if (size < thisChunkSize) {
			thisChunkSize = static_cast<uint32>(size);
		}

		if (_stream.read(dest, thisChunkSize) != thisChunkSize) {
			checkErrorAndReset();
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
			checkErrorAndReset();
			return false;
		}

		count -= static_cast<size_t>(thisChunkSize);
	}
	return true;
}

ProjectFormat DataReader::getProjectFormat() const {
	return _projectFormat;
}

bool DataReader::isBigEndian() const {
	return _stream.isBE();
}

bool DataReader::checkErrorAndReset() {
	const bool isFault = _stream.err() || _stream.eos();
	if (isFault) {
		_stream.clearErr();
		return false;
	}

	return true;
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

		if (_revision >= 3 && reader.getProjectFormat() == Data::kProjectFormatWindows && !reader.skip(8)) {
			return kDataReadErrorReadFailed;
		}

		if (!reader.readU32(streamDesc.pos) || !reader.readU32(streamDesc.size)) {
			return kDataReadErrorReadFailed;
		}

		if (_revision >= 3 && reader.getProjectFormat() == Data::kProjectFormatMacintosh && !reader.skip(8)) {
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

	if (!reader.readU32(modifierFlags) || !reader.readU32(sizeIncludingTag)
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

bool MiniscriptProgram::load(DataReader &reader) {
	projectFormat = reader.getProjectFormat();
	isBigEndian = reader.isBigEndian();

	if (!reader.readU32(unknown1) || !reader.readU32(sizeOfInstructions) || !reader.readU32(numOfInstructions) || !reader.readU32(numLocalRefs) || !reader.readU32(numAttributes))
		return false;

	if (sizeOfInstructions > 0) {
		bytecode.resize(sizeOfInstructions);
		if (!reader.read(&bytecode[0], sizeOfInstructions))
			return false;
	}

	if (numLocalRefs > 0) {
		localRefs.resize(numLocalRefs);
		for (size_t i = 0; i < numLocalRefs; i++) {
			LocalRef &localRef = localRefs[i];
			if (!reader.readU32(localRef.guid) || !reader.readU8(localRef.lengthOfName) || !reader.readU8(localRef.unknown2))
				return false;

			if (localRef.lengthOfName > 0 && !reader.readTerminatedStr(localRef.name, localRef.lengthOfName))
				return false;
		}
	}

	if (numAttributes > 0) {
		attributes.resize(numAttributes);
		for (size_t i = 0; i < numAttributes; i++) {
			Attribute &attrib = attributes[i];
			if (!reader.readU8(attrib.lengthOfName) || !reader.readU8(attrib.unknown3))
				return false;

			if (attrib.lengthOfName > 0 && !reader.readTerminatedStr(attrib.name, attrib.lengthOfName))
				return false;
		}
	}

	return true;
}

bool TypicalModifierHeader::load(DataReader& reader) {
	if (!reader.readU32(modifierFlags) || !reader.readU32(sizeIncludingTag) || !reader.readU32(guid)
		|| !reader.readBytes(unknown3) || !reader.readU32(unknown4) || !editorLayoutPosition.load(reader)
		|| !reader.readU16(lengthOfName))
		return false;

	if (lengthOfName > 0 && !reader.readTerminatedStr(name, lengthOfName))
		return false;

	return true;
}

DataReadErrorCode MiniscriptModifier::load(DataReader &reader) {
	if (_revision != 0x3eb)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader) || !enableWhen.load(reader) || !reader.readBytes(unknown6) || !reader.readU8(unknown7) || !program.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode MessengerModifier::load(DataReader& reader) {
	if (_revision != 0x3ea)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader))
		return kDataReadErrorReadFailed;

	// Unlike most cases, the "when" event is split in half in this case
	if (!reader.readU32(messageFlags) || !reader.readU32(when.eventID) || !send.load(reader) || !reader.readU16(unknown14) || !reader.readU32(destination)
		|| !reader.readBytes(unknown11) || !reader.readU16(with) || !reader.readBytes(unknown15) || !reader.readU32(withSourceGUID)
		|| !reader.readBytes(unknown12) || !reader.readU32(when.eventInfo) || !reader.readU8(withSourceLength) || !reader.readU8(unknown13))
		return kDataReadErrorReadFailed;

	if (withSourceLength > 0 && !reader.readNonTerminatedStr(withSourceName, withSourceLength))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode IfMessengerModifier::load(DataReader &reader) {
	if (_revision != 0x3ea)
		return kDataReadErrorReadFailed;

	if (!modHeader.load(reader) || !reader.readU32(messageFlags) || !when.load(reader) || !send.load(reader) ||
		!reader.readU16(unknown6) || !reader.readU32(destination) || !reader.readBytes(unknown7) || !reader.readU16(with)
		|| !reader.readBytes(unknown8) || !reader.readU32(withSourceGUID) || !reader.readBytes(unknown9) || !reader.readU8(withSourceLength) || !reader.readU8(unknown10))
		return kDataReadErrorReadFailed;

	if (withSourceLength > 0 && !reader.readNonTerminatedStr(withSource, withSourceLength))
		return kDataReadErrorReadFailed;

	if (!program.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode KeyboardMessengerModifier::load(DataReader &reader) {
	if (_revision != 0x3eb)
		return kDataReadErrorReadFailed;

	if (!modHeader.load(reader) || !reader.readU32(messageFlagsAndKeyStates) || !reader.readU16(unknown2)
		|| !reader.readU16(keyModifiers) || !reader.readU8(keycode) || !reader.readBytes(unknown4)
		|| !message.load(reader) || !reader.readU16(unknown7) || !reader.readU32(destination)
		|| !reader.readBytes(unknown9) || !reader.readU16(with) || !reader.readBytes(unknown11)
		|| !reader.readU32(withSourceGUID) || !reader.readBytes(unknown13) || !reader.readU8(withSourceLength)
		|| !reader.readU8(unknown14))
		return kDataReadErrorReadFailed;

	if (withSourceLength > 0 && !reader.readNonTerminatedStr(withSource, withSourceLength))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode BooleanVariableModifier::load(DataReader &reader) {
	if (_revision != 0x3e8)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader) || !reader.readU8(value) || !reader.readU8(unknown5))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode PointVariableModifier::load(DataReader &reader) {
	if (_revision != 0x3e8)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader) || !reader.readBytes(unknown5) || !value.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

PlugInModifierData::~PlugInModifierData() {
}

DataReadErrorCode PlugInModifier::load(DataReader &reader) {
	if (_revision != 0x03e9)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(modifierFlags) || !reader.readU32(codedSize) || !reader.read(modifierName, 16)
		|| !reader.readU32(guid) || !reader.readBytes(unknown2) || !reader.readU16(plugInRevision)
		|| !reader.readU32(unknown4) || !editorLayoutPosition.load(reader) || !reader.readU16(lengthOfName))
		return kDataReadErrorReadFailed;

	if (lengthOfName > 0 && !reader.readTerminatedStr(name, lengthOfName))
		return kDataReadErrorReadFailed;

	// Terminate modifier name safely
	modifierName[16] = 0;

	subObjectSize = codedSize;
	if (reader.getProjectFormat() == kProjectFormatWindows) {
		// This makes no sense but it's how it's stored...
		if (subObjectSize < lengthOfName * 256u)
			return kDataReadErrorReadFailed;
		subObjectSize -= lengthOfName * 256u;
	} else {
		if (subObjectSize < lengthOfName)
			return kDataReadErrorReadFailed;
		subObjectSize -= lengthOfName;
	}

	if (subObjectSize < 52)
		return kDataReadErrorReadFailed;
	subObjectSize -= 52;

	return kDataReadErrorNone;
}

DataReadErrorCode Debris::load(DataReader &reader) {
	if (_revision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(persistFlags) || !reader.readU32(sizeIncludingTag))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

const IPlugInModifierDataFactory *PlugInModifierRegistry::findLoader(const char *modifierName) const {
	Common::HashMap<Common::String, const IPlugInModifierDataFactory *>::const_iterator it = _loaders.find(modifierName);
	if (it == _loaders.end())
		return nullptr;
	return it->_value;
}

void PlugInModifierRegistry::registerLoader(const char *modifierName, const IPlugInModifierDataFactory *loader) {
	_loaders[modifierName] = loader;
}

DataReadErrorCode loadDataObject(const PlugInModifierRegistry &registry, DataReader &reader, Common::SharedPtr<DataObject> &outObject) {
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
	case DataObjectTypes::kMiniscriptModifier:
		dataObject = new MiniscriptModifier();
		break;
	case DataObjectTypes::kMessengerModifier:
		dataObject = new MessengerModifier();
		break;
	case DataObjectTypes::kIfMessengerModifier:
		dataObject = new IfMessengerModifier();
		break;
	case DataObjectTypes::kBooleanVariableModifier:
		dataObject = new BooleanVariableModifier();
		break;
	case DataObjectTypes::kPointVariableModifier:
		dataObject = new PointVariableModifier();
		break;
	case DataObjectTypes::kDebris:
		dataObject = new Debris();
		break;
	case DataObjectTypes::kPlugInModifier:
		dataObject = new PlugInModifier();
		break;
	case DataObjectTypes::kKeyboardMessengerModifier:
		dataObject = new KeyboardMessengerModifier();
		break;
	default:
		warning("Unrecognized data object type %x", static_cast<int>(type));
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

	if (type == DataObjectTypes::kPlugInModifier) {
		const IPlugInModifierDataFactory *plugInLoader = registry.findLoader(static_cast<const PlugInModifier *>(dataObject)->modifierName);
		if (!plugInLoader) {
			warning("Unrecognized plug-in modifier type %s", static_cast<const PlugInModifier *>(dataObject)->modifierName);
			outObject.reset();
			return kDataReadErrorPlugInNotFound;
		}

		Common::SharedPtr<PlugInModifierData> plugInModifierData(plugInLoader->createModifierData());
		errorCode = plugInModifierData->load(*static_cast<const PlugInModifier *>(dataObject), reader);
		if (errorCode != kDataReadErrorNone) {
			outObject.reset();
			return errorCode;
		}

		static_cast<PlugInModifier *>(dataObject)->plugInData = plugInModifierData;
	}

	outObject = sharedPtr;
	return errorCode;
}

} // End of namespace Data

} // End of namespace MTropolis
