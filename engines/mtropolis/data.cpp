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

#include "common/debug.h"
#include "common/memstream.h"

#include "mtropolis/data.h"

namespace MTropolis {

namespace Data {

namespace DataObjectTypes {

bool isValidSceneRootElement(DataObjectType type) {
	switch (type) {
	case kGraphicElement:
	case kMovieElement:
	case kMToonElement:
	case kImageElement:
		return true;
	default:
		return false;
	}
}

bool isVisualElement(DataObjectType type) {
	switch (type) {
	case kGraphicElement:
	case kMovieElement:
	case kMToonElement:
	case kImageElement:
	case kTextLabelElement:
		return true;
	default:
		return false;
	}
}

bool isNonVisualElement(DataObjectType type) {
	switch (type) {
	case kSoundElement:
		return true;
	default:
		return false;
	}
}

bool isElement(DataObjectType type) {
	switch (type) {
	case kGraphicElement:
	case kMovieElement:
	case kMToonElement:
	case kImageElement:
	case kTextLabelElement:
	case kSoundElement:
		return true;
	default:
		return false;
	}
}

bool isStructural(DataObjectType type) {
	switch (type) {
	case kProjectStructuralDef:
	case kSectionStructuralDef:
	case kSubsectionStructuralDef:
		return true;
	default:
		return isElement(type);
	}
}

bool isModifier(DataObjectType type) {
	switch (type) {
	case kAliasModifier:
	case kChangeSceneModifier:
	case kReturnModifier:
	case kSoundEffectModifier:
	case kDragMotionModifier:
	case kPathMotionModifierV1:
	case kPathMotionModifierV2:
	case kVectorMotionModifier:
	case kSceneTransitionModifier:
	case kElementTransitionModifier:
	case kSharedSceneModifier:
	case kIfMessengerModifier:
	case kBehaviorModifier:
	case kMessengerModifier:
	case kSetModifier:
	case kTimerMessengerModifier:
	case kCollisionDetectionMessengerModifier:
	case kBoundaryDetectionMessengerModifier:
	case kKeyboardMessengerModifier:
	case kTextStyleModifier:
	case kGraphicModifier:
	case kImageEffectModifier:
	case kMiniscriptModifier:
	case kCursorModifierV1:
	case kGradientModifier:
	case kColorTableModifier:
	case kSaveAndRestoreModifier:
	case kCompoundVariableModifier:
	case kBooleanVariableModifier:
	case kIntegerVariableModifier:
	case kIntegerRangeVariableModifier:
	case kVectorVariableModifier:
	case kPointVariableModifier:
	case kFloatingPointVariableModifier:
	case kStringVariableModifier:
	case kPlugInModifier:
	case kDebris:
		return true;
	default:
		return false;
	}
}

bool isAsset(DataObjectType type) {
	switch (type) {
	case kMovieAsset:
	case kAudioAsset:
	case kColorTableAsset:
	case kImageAsset:
	case kMToonAsset:
	case kTextAsset:
		return true;
	default:
		return false;
	}
}

} // End of namespace DataObjectTypes

DataReader::DataReader(int64 globalPosition, Common::SeekableReadStreamEndian &stream, ProjectFormat projectFormat)
	: _globalPosition(globalPosition), _stream(stream), _projectFormat(projectFormat) {
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

bool DataReader::readPlatformFloat(Common::XPFloat &value) {
	if (_projectFormat == kProjectFormatMacintosh) {
		return readU16(value.signAndExponent) && readU64(value.mantissa);
	} else if (_projectFormat == kProjectFormatWindows) {
		uint64 bits;
		if (!readU64(bits))
			return false;
		value = Common::XPFloat::fromDoubleBits(bits);

		return true;
	}

	return false;
}

bool DataReader::read(void *dest, size_t size) {
	while (size > 0) {
		uint32 thisChunkSize = 0xffffffffu;
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

bool DataReader::seek(int64 pos) {
	return _stream.seek(pos);
}

bool DataReader::skip(size_t count) {
	if (count > 0) {
		if (!_stream.seek(static_cast<int64>(count), SEEK_CUR)) {
			checkErrorAndReset();
			return false;
		}
	}
	return true;
}

int64 DataReader::tell() const {
	return _stream.pos();
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

Rect Rect::createDefault() {
	Rect rect;
	rect.top = 0;
	rect.bottom = 0;
	rect.left = 0;
	rect.right = 0;
	return rect;
}

bool Rect::toScummVMRect(Common::Rect &outRect) const {
	if (left > right || top > bottom)
		return false;
	outRect = Common::Rect(left, top, right, bottom);
	return true;
}

bool Rect::toScummVMRectUnchecked(Common::Rect &outRect) const {
	outRect.top = top;
	outRect.left = left;
	outRect.bottom = bottom;
	outRect.right = right;
	return true;
}

Point Point::createDefault() {
	Point pt;
	pt.x = 0;
	pt.y = 0;
	return pt;
}

bool Point::load(DataReader &reader) {
	if (reader.getProjectFormat() == kProjectFormatMacintosh)
		return reader.readS16(y) && reader.readS16(x);
	else if (reader.getProjectFormat() == kProjectFormatWindows)
		return reader.readS16(x) && reader.readS16(y);
	else
		return false;
}

bool Point::toScummVMPoint(Common::Point &outPoint) const {
	outPoint = Common::Point(x, y);
	return true;
}

Event Event::createDefault() {
	Event evt;
	evt.eventID = 0;
	evt.eventInfo = 0;
	return evt;
}

bool Event::load(DataReader& reader) {
	return reader.readU32(eventID) && reader.readU32(eventInfo);
}

ColorRGB16::ColorRGB16() : red(0), green(0), blue(0) {
}

bool ColorRGB16::load(DataReader& reader) {

	if (reader.getProjectFormat() == kProjectFormatMacintosh)
		return reader.readU16(red) && reader.readU16(green) && reader.readU16(blue); 
	else if (reader.getProjectFormat() == kProjectFormatWindows) {
		uint8 bgra[4];
		if (!reader.readBytes(bgra))
			return false;

		red = bgra[2] * 0x101;
		green = bgra[1] * 0x101;
		blue = bgra[0] * 0x101;

		return true;
	} else
		return false;
}

bool IntRange::load(DataReader& reader) {
	return reader.readS32(min) && reader.readS32(max);
}

IntRange IntRange::createDefault() {
	IntRange intRange;
	intRange.min = 0;
	intRange.max = 0;
	return intRange;
}

bool XPFloatVector::load(DataReader& reader) {
	return reader.readPlatformFloat(angleRadians) && reader.readPlatformFloat(magnitude);
}

bool XPFloatPOD::load(DataReader &reader) {
	Common::XPFloat xpFloat;

	if (!reader.readPlatformFloat(xpFloat))
		return false;

	signAndExponent = xpFloat.signAndExponent;
	mantissa = xpFloat.mantissa;

	return true;
}

Common::XPFloat XPFloatPOD::toXPFloat() const {
	return Common::XPFloat(signAndExponent, mantissa);
}

bool Label::load(DataReader &reader) {
	return reader.readU32(superGroupID) && reader.readU32(labelID);
}

InternalTypeTaggedValue::InternalTypeTaggedValue() : type(0) {
	memset(&this->value, 0, sizeof(this->value));
}

bool InternalTypeTaggedValue::load(DataReader &reader) {
	if (!reader.readU16(type))
		return false;

	int64 valueGlobalPos = reader.tellGlobal();

	uint8 contents[44];
	if (!reader.readBytes(contents))
		return false;

	Common::MemoryReadStreamEndian contentsStream(contents, sizeof(contents), reader.isBigEndian());

	DataReader valueReader(valueGlobalPos, contentsStream, reader.getProjectFormat());

	switch (type) {
	case kNull:
	case kIncomingData:
	case kString:	// Not a bug - string data is external!
		break;
	case kInteger:
		if (!valueReader.readS32(value.asInteger))
			return false;
		break;
	case kPoint:
		if (!value.asPoint.load(valueReader))
			return false;
		break;
	case kIntegerRange:
		if (!value.asIntegerRange.load(valueReader))
			return false;
		break;
	case kFloat:
		if (!value.asFloat.load(valueReader))
			return false;
		break;
	case kBool:
		if (!valueReader.readU8(value.asBool))
			return false;
		break;
	case kVariableReference:
		if (!valueReader.readU32(value.asVariableReference.unknown) || !valueReader.readU32(value.asVariableReference.guid))
			return false;
		break;
	case kLabel:
		if (!value.asLabel.load(valueReader))
			return false;
		break;
	default:
		warning("Unknown tagged value type %x", type);
		return false;
	}

	return true;
}



PlugInTypeTaggedValue::PlugInTypeTaggedValue() : type(kNull) {
	memset(&this->value, 0, sizeof(this->value));
}

bool PlugInTypeTaggedValue::load(DataReader &reader) {
	if (!reader.readU16(type))
		return false;

	switch (type) {
	case kNull:
	case kIncomingData:
		break;
	case kPoint:
		if (!value.asPoint.load(reader))
			return false;
		break;
	case kInteger:
		if (!reader.readS32(value.asInt))
			return false;
		break;
	case kIntegerRange:
		if (!value.asIntRange.load(reader))
			return false;
		break;
	case kFloat:
		if (!value.asFloat.load(reader))
			return false;
		break;
	case kBoolean:
		if (!reader.readU16(value.asBoolean))
			return false;
		break;
	case kEvent:
		if (!value.asEvent.load(reader))
			return false;
		break;
	case kLabel:
		// This is the opposite of internal vars...
		if (!reader.readU32(value.asLabel.labelID) || !reader.readU32(value.asLabel.superGroupID))
			return false;
		break;
	case kString: {
			uint32 length1;
			uint32 length2;
			if (!reader.readU32(length1) || !reader.readU32(length2))
				return false;
			// Usually length1 == length2 but sometimes not?
			if (!reader.readTerminatedStr(this->str, length2))
				return false;
		} break;
	case kVariableReference: {
			uint32 extraDataSize;
			if (!reader.readU32(value.asVarRefGUID) || !reader.readU32(extraDataSize))
				return false;

			if (extraDataSize > 0) {
				this->extraData.resize(extraDataSize);
				if (!reader.read(&extraData[0], extraDataSize))
					return false;
			}
		} break;
	default:
		warning("Unknown plug-in value type %x", type);
		return false;
	}

	return true;
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

ProjectLabelMap::ProjectLabelMap()
	: superGroups(nullptr), persistFlags(0), unknown1(0), numSuperGroups(0), nextAvailableID(0) {
}

ProjectLabelMap::~ProjectLabelMap() {
	if (superGroups)
		delete[] superGroups;
}

DataReadErrorCode ProjectLabelMap::load(DataReader &reader) {
	if (_revision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(persistFlags) || !reader.readU32(unknown1) || !reader.readU32(numSuperGroups) || !reader.readU32(nextAvailableID))
		return kDataReadErrorReadFailed;

	if (unknown1 != 0x16)
		return kDataReadErrorUnrecognized;

	superGroups = new SuperGroup[numSuperGroups];
	for (size_t i = 0; i < numSuperGroups; i++) {
		DataReadErrorCode subCode = loadSuperGroup(superGroups[i], reader);
		if (subCode != kDataReadErrorNone)
			return subCode;
	}

	return kDataReadErrorNone;
}

ProjectLabelMap::LabelTree::LabelTree() : nameLength(0), isGroup(0), id(0), unknown1(0), flags(0), numChildren(0), children(nullptr) {
}

ProjectLabelMap::LabelTree::~LabelTree() {
	if (children)
		delete[] children;
}

ProjectLabelMap::SuperGroup::SuperGroup()
	: nameLength(0), id(0), unknown2(0), numChildren(0), tree(nullptr) {
}

ProjectLabelMap::SuperGroup::~SuperGroup() {
	if (tree)
		delete[] tree;
}

DataReadErrorCode ProjectLabelMap::loadSuperGroup(SuperGroup &sg, DataReader &reader) {
	if (!reader.readU32(sg.nameLength) || !reader.readU32(sg.id) || !reader.readU32(sg.unknown2)
		|| !reader.readNonTerminatedStr(sg.name, sg.nameLength) || !reader.readU32(sg.numChildren))
		return kDataReadErrorReadFailed;

	if (sg.numChildren) {
		sg.tree = new LabelTree[sg.numChildren];
		for (size_t i = 0; i < sg.numChildren; i++) {
			DataReadErrorCode subCode = loadLabelTree(sg.tree[i], reader);
			if (subCode != kDataReadErrorNone)
				return subCode;
		}
	}

	return kDataReadErrorNone;
}

DataReadErrorCode ProjectLabelMap::loadLabelTree(LabelTree &lt, DataReader &reader) {
	if (!reader.readU32(lt.nameLength) || !reader.readU32(lt.isGroup) || !reader.readU32(lt.id)
		|| !reader.readU32(lt.unknown1) || !reader.readU32(lt.flags) || !reader.readNonTerminatedStr(lt.name, lt.nameLength))
		return kDataReadErrorReadFailed;

	if (lt.isGroup) {
		if (!reader.readU32(lt.numChildren))
			return kDataReadErrorReadFailed;

		if (lt.numChildren) {
			lt.children = new LabelTree[lt.numChildren];
			for (size_t i = 0; i < lt.numChildren; i++) {
				DataReadErrorCode subCode = loadLabelTree(lt.children[i], reader);
				if (subCode != kDataReadErrorNone)
					return subCode;
			}
		}
	} else
		lt.numChildren = 0;

	return kDataReadErrorNone;
}

ProjectHeader::ProjectHeader() : persistFlags(0), sizeIncludingTag(0), unknown1(0), catalogFilePosition(0) {
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

PresentationSettings::PresentationSettings()
	: persistFlags(0), dimensions(Point::createDefault()), sizeIncludingTag(0), unknown1{0, 0}, bitsPerPixel(0), unknown4(0) {
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

AssetCatalog::AssetInfoRev4Fields::AssetInfoRev4Fields() : assetType(0), flags2(0) {
}

AssetCatalog::AssetInfo::AssetInfo() : flags1(0), nameLength(0), alwaysZero(0), unknown1(0), filePosition(0) {
}

AssetCatalog::AssetCatalog() : persistFlags(0), totalNameSizePlus22(0), unknown1{0, 0, 0, 0}, numAssets(0), haveRev4Fields(false) {
}

DataReadErrorCode AssetCatalog::load(DataReader& reader) {
	if (_revision != 2 && _revision != 4)
		return kDataReadErrorUnsupportedRevision;

	haveRev4Fields = (_revision >= 4);

	if (!reader.readU32(persistFlags) ||
		!reader.readU32(totalNameSizePlus22) ||
		!reader.readBytes(unknown1) ||
		!reader.readU32(numAssets))
		return kDataReadErrorReadFailed;

	assets.resize(numAssets);

	for (size_t i = 0; i < numAssets; i++) {
		AssetInfo &asset = assets[i];
		if (!reader.readU32(asset.flags1) || !reader.readU16(asset.nameLength) || !reader.readU16(asset.alwaysZero) || !reader.readU32(asset.unknown1) || !reader.readU32(asset.filePosition))
			return kDataReadErrorReadFailed;

		if (_revision >= 4) {
			if (!reader.readU32(asset.rev4Fields.assetType) || !reader.readU32(asset.rev4Fields.flags2))
				return kDataReadErrorReadFailed;
		}

		if (!reader.readTerminatedStr(asset.name, asset.nameLength))
			return kDataReadErrorReadFailed;
	}

	return kDataReadErrorNone;
}

Unknown19::Unknown19() : persistFlags(0), sizeIncludingTag(0), unknown1{0, 0} {
}

DataReadErrorCode Unknown19::load(DataReader &reader) {
	if (_revision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(persistFlags) || !reader.readU32(sizeIncludingTag) || !reader.readBytes(unknown1))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

StructuralDef::StructuralDef() : structuralFlags(0) {
}

ProjectStructuralDef::ProjectStructuralDef() : unknown1(0), sizeIncludingTag(0), guid(0), otherFlags(0), lengthOfName(0) {
}

DataReadErrorCode ProjectStructuralDef::load(DataReader &reader) {
	if (_revision != 1 && _revision != 2)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(unknown1) || !reader.readU32(sizeIncludingTag) || !reader.readU32(guid)
		|| !reader.readU32(otherFlags) || !reader.readU16(lengthOfName) || !reader.readTerminatedStr(name, lengthOfName))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

SectionStructuralDef::SectionStructuralDef()
	: sizeIncludingTag(0), guid(0), lengthOfName(0), otherFlags(0), unknown4(0), sectionID(0), segmentID(0) {
}

DataReadErrorCode SectionStructuralDef::load(DataReader &reader) {
	if (_revision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(structuralFlags) || !reader.readU32(sizeIncludingTag) || !reader.readU32(guid)
		|| !reader.readU16(lengthOfName) || !reader.readU32(otherFlags) || !reader.readU16(unknown4)
		|| !reader.readU16(unknown4) || !reader.readU32(segmentID) || !reader.readTerminatedStr(name, lengthOfName))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

SubsectionStructuralDef::SubsectionStructuralDef()
	: structuralFlags(0), sizeIncludingTag(0), guid(0), lengthOfName(0), otherFlags(0), sectionID(0) {
}

DataReadErrorCode SubsectionStructuralDef::load(DataReader &reader) {
	if (_revision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(structuralFlags) || !reader.readU32(sizeIncludingTag) || !reader.readU32(guid)
		|| !reader.readU16(lengthOfName) || !reader.readU32(otherFlags) || !reader.readU16(sectionID)
		|| !reader.readTerminatedStr(name, lengthOfName))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

GraphicElement::GraphicElement()
	: sizeIncludingTag(0), guid(0), lengthOfName(0), elementFlags(0), layer(0), sectionID(0),
	  rect1(Rect::createDefault()), rect2(Rect::createDefault()), streamLocator(0), unknown11{0, 0, 0, 0} {
}

DataReadErrorCode GraphicElement::load(DataReader& reader) {
	if (_revision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(structuralFlags) || !reader.readU32(sizeIncludingTag) || !reader.readU32(guid)
		|| !reader.readU16(lengthOfName) || !reader.readU32(elementFlags) || !reader.readU16(layer)
		|| !reader.readU16(sectionID) || !rect1.load(reader) || !rect2.load(reader)
		|| !reader.readU32(streamLocator) || !reader.readBytes(unknown11) || !reader.readTerminatedStr(name, lengthOfName))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

ImageElement::ImageElement()
	: sizeIncludingTag(0), guid(0), lengthOfName(0), elementFlags(0), layer(0), sectionID(0),
	  rect1(Rect::createDefault()), rect2(Rect::createDefault()), imageAssetID(0), streamLocator(0), unknown7{0, 0, 0, 0} {
}

DataReadErrorCode ImageElement::load(DataReader &reader) {
	if (_revision != 2)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(structuralFlags) || !reader.readU32(sizeIncludingTag) || !reader.readU32(guid)
		|| !reader.readU16(lengthOfName) || !reader.readU32(elementFlags) || !reader.readU16(layer)
		|| !reader.readU16(sectionID) || !rect1.load(reader) || !rect2.load(reader)
		|| !reader.readU32(imageAssetID) || !reader.readU32(streamLocator) || !reader.readBytes(unknown7)
		|| !reader.readTerminatedStr(name, lengthOfName))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

TextLabelElement::TextLabelElement()
	: sizeIncludingTag(0), guid(0), lengthOfName(0), elementFlags(0), layer(0), sectionID(0),
	  rect1(Rect::createDefault()), rect2(Rect::createDefault()), assetID(0), haveMacPart(false), haveWinPart(false) {
	memset(&this->platform, 0, sizeof(this->platform));
}

DataReadErrorCode TextLabelElement::load(DataReader &reader) {
	if (reader.getProjectFormat() == kProjectFormatMacintosh) {
		if (_revision != 2)
			return kDataReadErrorUnsupportedRevision;
	} else if (reader.getProjectFormat() == kProjectFormatWindows) {
		if (_revision != 0)
			return kDataReadErrorUnsupportedRevision;
	} else
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(structuralFlags) || !reader.readU32(sizeIncludingTag) || !reader.readU32(guid)
		|| !reader.readU16(lengthOfName) || !reader.readU32(elementFlags) || !reader.readU16(layer)
		|| !reader.readU16(sectionID))
		return kDataReadErrorReadFailed;

	haveMacPart = false;
	haveWinPart = false;
	if (reader.getProjectFormat() == kProjectFormatWindows) {
		haveWinPart = true;
		if (!reader.readBytes(platform.win.unknown3))
			return kDataReadErrorReadFailed;
	}

	if (!rect1.load(reader) || !rect2.load(reader) || !reader.readU32(assetID))
		return kDataReadErrorReadFailed;

	if (reader.getProjectFormat() == kProjectFormatWindows) {
		if (!reader.readBytes(platform.win.unknown4))
			return kDataReadErrorReadFailed;
	} else if (reader.getProjectFormat() == kProjectFormatMacintosh) {
		haveMacPart = true;
		if (!reader.readBytes(platform.mac.unknown2))
			return kDataReadErrorReadFailed;
	} else
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readTerminatedStr(name, lengthOfName))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

SoundElement::SoundElement()
	: sizeIncludingTag(0), guid(0), lengthOfName(0), elementFlags(0), soundFlags(0), unknown2(0),
	  unknown3{0, 0}, rightVolume(0), leftVolume(0), balance(0), assetID(0), unknown5{0, 0, 0, 0, 0, 0, 0, 0} {
}

DataReadErrorCode SoundElement::load(DataReader& reader) {
	if (_revision != 3)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(structuralFlags) || !reader.readU32(sizeIncludingTag) || !reader.readU32(guid)
		|| !reader.readU16(lengthOfName) || !reader.readU32(elementFlags) || !reader.readU32(soundFlags)
		|| !reader.readU16(unknown2) || !reader.readBytes(unknown3) || !reader.readU16(rightVolume)
		|| !reader.readU16(leftVolume) || !reader.readS16(balance) || !reader.readU32(assetID)
		|| !reader.readBytes(unknown5) || !reader.readTerminatedStr(name, lengthOfName))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

MovieElement::MovieElement()
	: sizeIncludingTag(0), guid(0), lengthOfName(0), elementFlags(0), layer(0),
	  unknown3{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	  sectionID(0), unknown5{0, 0}, rect1(Rect::createDefault()), rect2(Rect::createDefault()), assetID(0), unknown7(0), volume(0), animationFlags(0),
	  unknown10{0, 0, 0, 0}, unknown11{0, 0, 0, 0}, streamLocator(0), unknown13{0, 0, 0, 0} {
}

DataReadErrorCode MovieElement::load(DataReader &reader) {
	if (_revision != 2)
		return kDataReadErrorUnsupportedRevision;
	
	if (!reader.readU32(structuralFlags) || !reader.readU32(sizeIncludingTag) || !reader.readU32(guid)
		|| !reader.readU16(lengthOfName) || !reader.readU32(elementFlags) || !reader.readU16(layer)
		|| !reader.readBytes(unknown3) || !reader.readU16(sectionID) || !reader.readBytes(unknown5)
		|| !rect1.load(reader) || !rect2.load(reader) || !reader.readU32(assetID)
		|| !reader.readU32(unknown7) || !reader.readU16(volume) || !reader.readU32(animationFlags)
		|| !reader.readBytes(unknown10) || !reader.readBytes(unknown11) || !reader.readU32(streamLocator)
		|| !reader.readBytes(unknown13) || !reader.readTerminatedStr(name, lengthOfName))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

MToonElement::MToonElement()
	: sizeIncludingTag(0), guid(0), lengthOfName(0), elementFlags(0), layer(0), animationFlags(0), unknown4{ 0, 0, 0, 0 },
	  sectionID(0), rect1(Rect::createDefault()), rect2(Rect::createDefault()), assetID(0), rateTimes100000(0), streamLocator(0), unknown6(0) {
}

DataReadErrorCode MToonElement::load(DataReader &reader) {
	if (_revision != 2 && _revision != 3)
		return kDataReadErrorUnsupportedRevision;
	
	if (!reader.readU32(structuralFlags) || !reader.readU32(sizeIncludingTag) || !reader.readU32(guid)
			|| !reader.readU16(lengthOfName) || !reader.readU32(elementFlags) || !reader.readU16(layer)
			|| !reader.readU32(animationFlags) || !reader.readBytes(unknown4) || !reader.readU16(sectionID)
			|| !rect1.load(reader) || !rect2.load(reader) || !reader.readU32(assetID)
			|| !reader.readU32(rateTimes100000) || !reader.readU32(streamLocator) || !reader.readU32(unknown6)
			|| !reader.readTerminatedStr(name, lengthOfName))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

GlobalObjectInfo::GlobalObjectInfo()
	: persistFlags(0), sizeIncludingTag(0), numGlobalModifiers(0), unknown1{0, 0, 0, 0} {
}

DataReadErrorCode GlobalObjectInfo::load(DataReader &reader) {
	if (_revision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(persistFlags) || !reader.readU32(sizeIncludingTag) || !reader.readU16(numGlobalModifiers)
		|| !reader.readBytes(unknown1))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

ProjectCatalog::StreamDesc::StreamDesc()
	: streamType { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	  segmentIndexPlusOne(0), size(0), pos(0) {
}

ProjectCatalog::SegmentDesc::SegmentDesc() : segmentID(0) {
}

ProjectCatalog::ProjectCatalog() : persistFlags(0), sizeOfStreamAndSegmentDescs(0), unknown1(0), unknown2(0), unknown3(0) {
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

StreamHeader::StreamHeader()
	: marker(0), sizeIncludingTag(0), unknown2(0), name{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	  projectID{0, 0}, unknown1{0, 0, 0, 0} {
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

BehaviorModifier::BehaviorModifier()
	: modifierFlags(0), sizeIncludingTag(0), unknown2{0, 0}, guid(0), unknown4(0), unknown5(0), unknown6(0),
	  editorLayoutPosition(Point::createDefault()), lengthOfName(0), numChildren(0), behaviorFlags(0),
	  enableWhen(Event::createDefault()), disableWhen(Event::createDefault()), unknown7{0, 0} {
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

	if (!reader.readU32(behaviorFlags) || !enableWhen.load(reader) || !disableWhen.load(reader) || !reader.readBytes(unknown7))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

MiniscriptProgram::LocalRef::LocalRef()
	: guid(0), lengthOfName(0), unknown2(0) {
}


MiniscriptProgram::Attribute::Attribute()
	: lengthOfName(0), unknown3(0) {
}

MiniscriptProgram::MiniscriptProgram()
	: unknown1(0), sizeOfInstructions(0), numOfInstructions(0), numLocalRefs(0), numAttributes(0),
	  projectFormat(kProjectFormatUnknown), isBigEndian(false) {
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

TypicalModifierHeader::TypicalModifierHeader()
	: modifierFlags(0), sizeIncludingTag(0), guid(0), unknown3{0, 0, 0, 0, 0, 0}, unknown4(0), editorLayoutPosition(Point::createDefault()), lengthOfName(0) {
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

MiniscriptModifier::MiniscriptModifier()
	: enableWhen(Event::createDefault()), unknown6{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, unknown7(0) {
}

DataReadErrorCode MiniscriptModifier::load(DataReader &reader) {
	if (_revision != 1003)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader) || !enableWhen.load(reader) || !reader.readBytes(unknown6) || !reader.readU8(unknown7) || !program.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

SaveAndRestoreModifier::SaveAndRestoreModifier()
	: unknown1{0, 0, 0, 0}, saveWhen(Event::createDefault()), restoreWhen(Event::createDefault()),
	  unknown5{0, 0, 0, 0, 0, 0, 0, 0}, lengthOfFilePath(0), lengthOfFileName(0), lengthOfVariableName(0), lengthOfVariableString(0) {
}

DataReadErrorCode SaveAndRestoreModifier::load(DataReader &reader) {
	if (_revision != 1001)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader) || !reader.readBytes(unknown1) || !saveWhen.load(reader) || !restoreWhen.load(reader)
		|| !saveOrRestoreValue.load(reader) || !reader.readBytes(unknown5) || !reader.readU8(lengthOfFilePath)
		|| !reader.readU8(lengthOfFileName) || !reader.readU8(lengthOfVariableName) || !reader.readU8(lengthOfVariableString)
		|| !reader.readNonTerminatedStr(varName, lengthOfVariableName) || !reader.readNonTerminatedStr(varString, lengthOfVariableString)
		|| !reader.readNonTerminatedStr(filePath, lengthOfFilePath) || !reader.readNonTerminatedStr(fileName, lengthOfFileName))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

MessengerModifier::MessengerModifier()
	: messageFlags(0), send(Event::createDefault()), when(Event::createDefault()), unknown14(0), destination(0),
	  unknown11{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, withSourceLength(0), withStringLength(0) {
}

DataReadErrorCode MessengerModifier::load(DataReader &reader) {
	if (_revision != 1002)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader))
		return kDataReadErrorReadFailed;

	// Unlike most cases, the "when" event is split in half in this case
	if (!reader.readU32(messageFlags) || !reader.readU32(when.eventID) || !send.load(reader) || !reader.readU16(unknown14) || !reader.readU32(destination)
		|| !reader.readBytes(unknown11) || !with.load(reader) || !reader.readU32(when.eventInfo) || !reader.readU8(withSourceLength) || !reader.readU8(withStringLength))
		return kDataReadErrorReadFailed;

	if (!reader.readNonTerminatedStr(withSource, withSourceLength) || !reader.readNonTerminatedStr(withString, withStringLength))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

SetModifier::SetModifier()
	: unknown1{0, 0, 0, 0}, executeWhen(Event::createDefault()), unknown3(0),
	  sourceNameLength(0), targetNameLength(0), sourceStringLength(0), targetStringLength(0), unknown4(0) {
}

DataReadErrorCode SetModifier::load(DataReader &reader) {
	if (_revision != 1000)
		return kDataReadErrorUnsupportedRevision;

	// NOTE: executeWhen is split in half and stored in 2 separate parts
	if (!modHeader.load(reader) || !reader.readBytes(unknown1) || !reader.readU32(executeWhen.eventID)
		|| !source.load(reader) || !target.load(reader) || !reader.readU32(executeWhen.eventInfo) || !reader.readU8(unknown3)
		|| !reader.readU8(sourceNameLength) || !reader.readU8(targetNameLength) || !reader.readU8(sourceStringLength)
		|| !reader.readU8(targetStringLength)  || !reader.readU8(unknown4) || !reader.readNonTerminatedStr(sourceName, sourceNameLength)
		|| !reader.readNonTerminatedStr(targetName, targetNameLength) || !reader.readNonTerminatedStr(sourceString, sourceStringLength)
		|| !reader.readNonTerminatedStr(targetString, targetStringLength))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

AliasModifier::AliasModifier()
	: modifierFlags(0), sizeIncludingTag(0), aliasIndexPlusOne(0), unknown1(0), unknown2(0), lengthOfName(0), guid(0), editorLayoutPosition(Point::createDefault()) {
}

DataReadErrorCode AliasModifier::load(DataReader& reader) {
	if (_revision != 2)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(modifierFlags)
		|| !reader.readU32(sizeIncludingTag)
		|| !reader.readU16(aliasIndexPlusOne)
		|| !reader.readU32(unknown1)
		|| !reader.readU32(unknown2)
		|| !reader.readU16(lengthOfName)
		|| !editorLayoutPosition.load(reader)
		|| !reader.readU32(guid)
		|| !reader.readTerminatedStr(name, lengthOfName))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

ChangeSceneModifier::ChangeSceneModifier()
	: executeWhen(Event::createDefault()), changeSceneFlags(0), targetSectionGUID(0), targetSubsectionGUID(0), targetSceneGUID(0) {
}

DataReadErrorCode ChangeSceneModifier::load(DataReader &reader) {
	if (_revision != 1001)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader) || !reader.readU32(changeSceneFlags) || !executeWhen.load(reader)
		|| !reader.readU32(targetSectionGUID) || !reader.readU32(targetSubsectionGUID) || !reader.readU32(targetSceneGUID))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

SoundEffectModifier::SoundEffectModifier()
	: unknown1{0, 0, 0, 0}, executeWhen(Event::createDefault()), terminateWhen(Event::createDefault()),
	  unknown2(0), unknown3{0, 0, 0, 0}, assetID(0), unknown5{0, 0, 0, 0} {
}

DataReadErrorCode SoundEffectModifier::load(DataReader &reader) {
	if (_revision != 1000)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader) || !reader.readBytes(unknown1) || !executeWhen.load(reader)
		|| !terminateWhen.load(reader) || !reader.readU32(unknown2) || !reader.readBytes(unknown3)
		|| !reader.readU32(assetID) || !reader.readBytes(unknown5))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

bool PathMotionModifierV2::PointDef::load(DataReader &reader) {
	if (!point.load(reader)
		|| !reader.readU32(frame)
		|| !reader.readU32(frameFlags)
		|| !reader.readU32(messageFlags)
		|| !send.load(reader)
		|| !reader.readU16(unknown11)
		|| !reader.readU32(destination)
		|| !reader.readBytes(unknown13)
		|| !with.load(reader)
		|| !reader.readU8(withSourceLength)
		|| !reader.readU8(withStringLength)
		|| !reader.readNonTerminatedStr(withSource, withSourceLength)
		|| !reader.readNonTerminatedStr(withString, withStringLength))
		return false;

	return true;
}


PathMotionModifierV2::PointDef::PointDef()
	: point(Point::createDefault()), frame(0), frameFlags(0), messageFlags(0), send(Event::createDefault()), unknown11(0),
	  destination(0), unknown13{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, withSourceLength(0), withStringLength(0) {
}

PathMotionModifierV2::PathMotionModifierV2()
	: flags(0), executeWhen(Event::createDefault()), terminateWhen(Event::createDefault()), unknown2{0, 0}, numPoints(0), unknown3{0, 0, 0, 0},
	  frameDurationTimes10Million(0), unknown5{0, 0, 0, 0}, unknown6(0) {
}

DataReadErrorCode PathMotionModifierV2::load(DataReader &reader) {
	if (_revision != 1001)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader)
		|| !reader.readU32(flags)
		|| !executeWhen.load(reader)
		|| !terminateWhen.load(reader)
		|| !reader.readBytes(unknown2)
		|| !reader.readU16(numPoints)
		|| !reader.readBytes(unknown3)
		|| !reader.readU32(frameDurationTimes10Million)
		|| !reader.readBytes(unknown5)
		|| !reader.readU32(unknown6))
		return kDataReadErrorReadFailed;

	points.resize(numPoints);

	for (size_t i = 0; i < numPoints; i++) {
		if (!points[i].load(reader))
			return kDataReadErrorReadFailed;
	}

	return kDataReadErrorNone;
}

DragMotionModifier::DragMotionModifier()
	: enableWhen(Event::createDefault()), disableWhen(Event::createDefault()), haveMacPart(false), haveWinPart(false),
	  constraintMargin(Rect::createDefault()), unknown1(0) {
	memset(&this->platform, 0, sizeof(this->platform));
}

DataReadErrorCode DragMotionModifier::load(DataReader &reader) {
	if (_revision != 1000)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader))
		return kDataReadErrorReadFailed;

	if (!enableWhen.load(reader) || !disableWhen.load(reader))
		return kDataReadErrorReadFailed;

	if (reader.getProjectFormat() == kProjectFormatMacintosh) {
		if (!reader.readU8(platform.mac.flags) || !reader.readU8(platform.mac.unknown3))
			return kDataReadErrorReadFailed;

		haveMacPart = true;
	} else
		haveMacPart = false;

	if (reader.getProjectFormat() == kProjectFormatWindows) {
		if (!reader.readU8(platform.win.unknown2) || !reader.readU8(platform.win.constrainHorizontal)
			|| !reader.readU8(platform.win.constrainVertical) || !reader.readU8(platform.win.constrainToParent))
			return kDataReadErrorReadFailed;

		haveWinPart = true;
	} else
		haveWinPart = false;

	if (!constraintMargin.load(reader) || !reader.readU16(unknown1))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

VectorMotionModifier::VectorMotionModifier()
	: enableWhen(Event::createDefault()), disableWhen(Event::createDefault()), unknown1(0), vecSourceLength(0), vecStringLength(0) {
}

DataReadErrorCode VectorMotionModifier::load(DataReader &reader) {
	if (_revision != 1001)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader))
		return kDataReadErrorReadFailed;

	if (!enableWhen.load(reader) || !disableWhen.load(reader) || !vec.load(reader)
		|| !reader.readU16(unknown1) || !reader.readU8(vecSourceLength) || !reader.readU8(vecStringLength)
		|| !reader.readNonTerminatedStr(vecSource, vecSourceLength)
		/*|| !reader.readNonTerminatedStr(vecString, vecStringLength)*/)	// mTropolis bug!
		return kDataReadErrorNone;

	return kDataReadErrorNone;
}

SceneTransitionModifier::SceneTransitionModifier()
	: enableWhen(Event::createDefault()), disableWhen(Event::createDefault()), transitionType(0), direction(0),
	  unknown3(0), steps(0), duration(0), unknown5{0, 0} {
}

DataReadErrorCode SceneTransitionModifier::load(DataReader &reader) {
	if (_revision != 1001)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader))
		return kDataReadErrorReadFailed;

	if (!enableWhen.load(reader) || !disableWhen.load(reader) || !reader.readU16(transitionType)
		|| !reader.readU16(direction) || !reader.readU16(unknown3) || !reader.readU16(steps)
		|| !reader.readU32(duration) || !reader.readBytes(unknown5))
		return kDataReadErrorNone;

	return kDataReadErrorNone;
}

ElementTransitionModifier::ElementTransitionModifier()
	: enableWhen(Event::createDefault()), disableWhen(Event::createDefault()), revealType(0), transitionType(0),
	  unknown3(0), unknown4(0), steps(0), rate(0) {
}

DataReadErrorCode ElementTransitionModifier::load(DataReader &reader) {
	if (_revision != 1001)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader))
		return kDataReadErrorReadFailed;

	if (!enableWhen.load(reader) || !disableWhen.load(reader) || !reader.readU16(revealType)
		|| !reader.readU16(transitionType) || !reader.readU16(unknown3) || !reader.readU16(unknown4)
		|| !reader.readU16(steps) || !reader.readU16(rate))
		return kDataReadErrorNone;

	return kDataReadErrorNone;
}

IfMessengerModifier::IfMessengerModifier()
	: messageFlags(0), send(Event::createDefault()), when(Event::createDefault()), unknown6(0), destination(0),
	  unknown7{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, unknown9{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, withSourceLength(0), withStringLength(0) {
}

DataReadErrorCode IfMessengerModifier::load(DataReader &reader) {
	if (_revision != 1002)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader) || !reader.readU32(messageFlags) || !when.load(reader) || !send.load(reader)
		|| !reader.readU16(unknown6) || !reader.readU32(destination) || !reader.readBytes(unknown7) || !with.load(reader)
		|| !reader.readBytes(unknown9) || !reader.readU8(withSourceLength) || !reader.readU8(withStringLength))
		return kDataReadErrorReadFailed;

	if (!reader.readNonTerminatedStr(withSource, withSourceLength) || !reader.readNonTerminatedStr(withString, withStringLength))
		return kDataReadErrorReadFailed;

	if (!program.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

TimerMessengerModifier::TimerMessengerModifier()
	: messageAndTimerFlags(0), executeWhen(Event::createDefault()), send(Event::createDefault()), terminateWhen(Event::createDefault()),
	  unknown2(0), destination(0), unknown4{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, unknown5(0), minutes(0), seconds(0), hundredthsOfSeconds(0),
	  unknown6(0), unknown7(0), unknown8{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, withSourceLength(0), withStringLength(0) {
}

DataReadErrorCode TimerMessengerModifier::load(DataReader &reader) {
	if (_revision != 1002)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader))
		return kDataReadErrorReadFailed;

	if (!reader.readU32(messageAndTimerFlags) || !executeWhen.load(reader) || !send.load(reader)
		|| !terminateWhen.load(reader) || !reader.readU16(unknown2) || !reader.readU32(destination)
		|| !reader.readBytes(unknown4) || !with.load(reader) || !reader.readU8(unknown5)
		|| !reader.readU8(minutes) || !reader.readU8(seconds) || !reader.readU8(hundredthsOfSeconds)
		|| !reader.readU32(unknown6) || !reader.readU32(unknown7) || !reader.readBytes(unknown8)
		|| !reader.readU8(withSourceLength) || !reader.readU8(withStringLength) || !reader.readNonTerminatedStr(withSource, withSourceLength)
		 || !reader.readNonTerminatedStr(withString, withStringLength))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

BoundaryDetectionMessengerModifier::BoundaryDetectionMessengerModifier()
	: messageFlagsHigh(0), enableWhen(Event::createDefault()), disableWhen(Event::createDefault()), send(Event::createDefault()),
	  unknown2(0), destination(0), unknown3{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, withSourceLength(0), withStringLength(0) {
}

DataReadErrorCode BoundaryDetectionMessengerModifier::load(DataReader &reader) {
	if (_revision != 1002)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader))
		return kDataReadErrorReadFailed;

	if (!reader.readU16(messageFlagsHigh) || !enableWhen.load(reader) || !disableWhen.load(reader)
		|| !send.load(reader) || !reader.readU16(unknown2) || !reader.readU32(destination)
		|| !reader.readBytes(unknown3) || !with.load(reader) || !reader.readU8(withSourceLength)
		|| !reader.readU8(withStringLength) || !reader.readNonTerminatedStr(withSource, withSourceLength) || !reader.readNonTerminatedStr(withString, withStringLength))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

CollisionDetectionMessengerModifier::CollisionDetectionMessengerModifier()
	: messageAndModifierFlags(0), enableWhen(Event::createDefault()), disableWhen(Event::createDefault()), send(Event::createDefault()),
	  unknown2(0), destination(0), unknown3{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, withSourceLength(0), withStringLength(0) {
}

DataReadErrorCode CollisionDetectionMessengerModifier::load(DataReader &reader) {
	if (_revision != 1002)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader))
		return kDataReadErrorReadFailed;

	if (!reader.readU32(messageAndModifierFlags) || !enableWhen.load(reader) || !disableWhen.load(reader)
		|| !send.load(reader) || !reader.readU16(unknown2) || !reader.readU32(destination)
		|| !reader.readBytes(unknown3) || !with.load(reader) || !reader.readU8(withSourceLength)
		|| !reader.readU8(withStringLength) || !reader.readNonTerminatedStr(withSource, withSourceLength)  || !reader.readNonTerminatedStr(withString, withStringLength))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

KeyboardMessengerModifier::KeyboardMessengerModifier()
	: messageFlagsAndKeyStates(0), unknown2(0), keyModifiers(0), keycode(0), unknown4{0, 0, 0, 0},
	  message(Event::createDefault()), unknown7(0), destination(0), unknown9{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	  withSourceLength(0), withStringLength(0) {
}

DataReadErrorCode KeyboardMessengerModifier::load(DataReader &reader) {
	if (_revision != 1003)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader) || !reader.readU32(messageFlagsAndKeyStates) || !reader.readU16(unknown2)
		|| !reader.readU16(keyModifiers) || !reader.readU8(keycode) || !reader.readBytes(unknown4)
		|| !message.load(reader) || !reader.readU16(unknown7) || !reader.readU32(destination)
		|| !reader.readBytes(unknown9) || !with.load(reader) || !reader.readU8(withSourceLength)
		|| !reader.readU8(withStringLength))
		return kDataReadErrorReadFailed;

	if (!reader.readNonTerminatedStr(withSource, withSourceLength) || !reader.readNonTerminatedStr(withString, withStringLength))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

TextStyleModifier::TextStyleModifier()
	: unknown1{0, 0, 0, 0}, macFontID(0), flags(0), unknown2(0), size(0), alignment(0), unknown3(0),
	  applyWhen(Event::createDefault()), removeWhen(Event::createDefault()), lengthOfFontFamilyName(0) {
}

DataReadErrorCode TextStyleModifier::load(DataReader &reader) {
	if (_revision != 1000)
		return kDataReadErrorUnsupportedRevision;
	
	if (!modHeader.load(reader) || !reader.readBytes(unknown1) || !reader.readU16(macFontID)
		|| !reader.readU8(flags) || !reader.readU8(unknown2) || !reader.readU16(size)
		|| !textColor.load(reader) || !backgroundColor.load(reader) || !reader.readU16(alignment)
		|| !reader.readU16(unknown3) || !applyWhen.load(reader) || !removeWhen.load(reader)
		|| !reader.readU16(lengthOfFontFamilyName) || !reader.readNonTerminatedStr(fontFamilyName, lengthOfFontFamilyName))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

GraphicModifier::GraphicModifier()
	: unknown1(0), applyWhen(Event::createDefault()), removeWhen(Event::createDefault()), unknown2{0, 0}, inkMode(0), shape(0),
	  haveMacPart(false), haveWinPart(false), borderSize(0), shadowSize(0), numPolygonPoints(0), unknown6{0, 0, 0, 0, 0, 0, 0, 0} {
	memset(&this->platform, 0, sizeof(this->platform));
}

DataReadErrorCode GraphicModifier::load(DataReader &reader) {
	if (_revision != 1001)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader) || !reader.readU16(unknown1) || !applyWhen.load(reader)
		|| !removeWhen.load(reader) || !reader.readBytes(unknown2) || !reader.readU16(inkMode)
		|| !reader.readU16(shape))
		return kDataReadErrorReadFailed;

	if (reader.getProjectFormat() == kProjectFormatMacintosh) {
		haveMacPart = true;
		if (!reader.readBytes(platform.mac.unknown4_1) || !backColor.load(reader) || !foreColor.load(reader)
			|| !reader.readU16(borderSize) || !borderColor.load(reader) || !reader.readU16(shadowSize)
			|| !shadowColor.load(reader) || !reader.readBytes(platform.mac.unknown4_2))
			return kDataReadErrorReadFailed;
	} else
		haveMacPart = false;

	if (reader.getProjectFormat() == kProjectFormatWindows) {
		haveWinPart = true;
		if (!reader.readBytes(platform.win.unknown5_1) || !backColor.load(reader) || !foreColor.load(reader)
			|| !reader.readU16(borderSize) || !borderColor.load(reader) || !reader.readU16(shadowSize)
			|| !shadowColor.load(reader) || !reader.readBytes(platform.win.unknown5_2))
			return kDataReadErrorReadFailed;
	} else
		haveWinPart = false;

	if (!reader.readU16(numPolygonPoints) || !reader.readBytes(unknown6))
		return kDataReadErrorReadFailed;

	// coverity[tainted_scalar]
	polyPoints.resize(numPolygonPoints);
	for (size_t i = 0; i < numPolygonPoints; i++) {
		if (!polyPoints[i].load(reader))
			return kDataReadErrorReadFailed;
	}

	return kDataReadErrorNone;
}

CompoundVariableModifier::CompoundVariableModifier()
	: modifierFlags(0), sizeIncludingTag(0), unknown1{0, 0}, guid(0), unknown4{0, 0, 0, 0, 0, 0}, unknown5(0),
	  editorLayoutPosition(Point::createDefault()), lengthOfName(0), numChildren(0), unknown7{0, 0, 0, 0} {
}

DataReadErrorCode CompoundVariableModifier::load(DataReader &reader) {
	if (_revision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(modifierFlags) || !reader.readU32(sizeIncludingTag) || !reader.readBytes(unknown1)
		|| !reader.readU32(guid) || !reader.readBytes(unknown4) || !reader.readU32(unknown5)
		|| !editorLayoutPosition.load(reader) || !reader.readU16(lengthOfName) || !reader.readU16(numChildren)
		|| !reader.readTerminatedStr(name, lengthOfName) || !reader.readBytes(unknown7))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

BooleanVariableModifier::BooleanVariableModifier() : value(0), unknown5(0) {
}

DataReadErrorCode BooleanVariableModifier::load(DataReader &reader) {
	if (_revision != 1000)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader) || !reader.readU8(value) || !reader.readU8(unknown5))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

IntegerVariableModifier::IntegerVariableModifier() : unknown1{0, 0, 0, 0}, value(0) {
}

DataReadErrorCode IntegerVariableModifier::load(DataReader &reader) {
	if (_revision != 1000)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader) || !reader.readBytes(unknown1) || !reader.readS32(value))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

IntegerRangeVariableModifier::IntegerRangeVariableModifier() : unknown1{0, 0, 0, 0}, range(IntRange::createDefault()) {
}

DataReadErrorCode IntegerRangeVariableModifier::load(DataReader &reader) {
	if (_revision != 1000)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader) || !reader.readBytes(unknown1) || !range.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

VectorVariableModifier::VectorVariableModifier() : unknown1{0, 0, 0, 0} {
}

DataReadErrorCode VectorVariableModifier::load(DataReader &reader) {
	if (_revision != 1000)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader) || !reader.readBytes(unknown1) || !this->vector.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

PointVariableModifier::PointVariableModifier() : unknown5{0, 0, 0, 0}, value(Point::createDefault()) {
}

DataReadErrorCode PointVariableModifier::load(DataReader &reader) {
	if (_revision != 1000)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader) || !reader.readBytes(unknown5) || !value.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

FloatingPointVariableModifier::FloatingPointVariableModifier() : unknown1{0, 0, 0, 0} {
}

DataReadErrorCode FloatingPointVariableModifier::load(DataReader &reader) {
	if (_revision != 1000)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader) || !reader.readBytes(unknown1) || !reader.readPlatformFloat(value))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

StringVariableModifier::StringVariableModifier() : lengthOfString(0), unknown1{0, 0, 0, 0} {
}

DataReadErrorCode StringVariableModifier::load(DataReader &reader) {
	if (_revision != 1000)
		return kDataReadErrorUnsupportedRevision;

	if (!modHeader.load(reader) || !reader.readU32(lengthOfString) || !reader.readBytes(unknown1) || !reader.readTerminatedStr(value, lengthOfString))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

PlugInModifierData::~PlugInModifierData() {
}

PlugInModifier::PlugInModifier()
	: modifierFlags(0), codedSize(0), modifierName{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	  guid(0), unknown2{0, 0, 0, 0, 0, 0}, plugInRevision(0), unknown4(0), editorLayoutPosition(Point::createDefault()),
	  lengthOfName(0), subObjectSize(0) {
}

DataReadErrorCode PlugInModifier::load(DataReader &reader) {
	if (_revision != 1001)
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

Debris::Debris() : persistFlags(0), sizeIncludingTag(0) {
}

DataReadErrorCode Debris::load(DataReader &reader) {
	if (_revision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(persistFlags) || !reader.readU32(sizeIncludingTag))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

ColorTableAsset::ColorTableAsset()
	: persistFlags(0), sizeIncludingTag(0), unknown1{0, 0, 0, 0}, assetID(0), unknown2(0) {
}

DataReadErrorCode ColorTableAsset::load(DataReader &reader) {
	if (_revision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(persistFlags) || !reader.readU32(sizeIncludingTag))
		return kDataReadErrorReadFailed;

	if (reader.getProjectFormat() == Data::kProjectFormatMacintosh) {
		if (sizeIncludingTag != 0x0836)
			return kDataReadErrorUnrecognized;
	} else if (reader.getProjectFormat() == Data::kProjectFormatWindows) {
		if (sizeIncludingTag != 0x0428)
			return kDataReadErrorUnrecognized;
	} else
		return kDataReadErrorUnrecognized;

	if (!reader.readBytes(unknown1) || !reader.readU32(assetID) || !reader.readU32(unknown2))
		return kDataReadErrorReadFailed;

	size_t numColors = 256;
	if (reader.getProjectFormat() == Data::kProjectFormatMacintosh) {
		if (!reader.skip(20))
			return kDataReadErrorReadFailed;

		uint8 clutHeader[8];
		if (!reader.readBytes(clutHeader))
			return kDataReadErrorReadFailed;

		uint8 cdefBytes[256 * 8];
		if (!reader.read(cdefBytes, numColors * 8))
			return kDataReadErrorReadFailed;

		for (size_t i = 0; i < numColors; i++) {
			ColorRGB16 &cdef = colors[i];

			const uint8 *rgb = cdefBytes + i * 8 + 2;
			cdef.red = (rgb[0] << 8) | rgb[1];
			cdef.green = (rgb[2] << 8) | rgb[3];
			cdef.blue = (rgb[4] << 8) | rgb[5];
		}
	} else if (reader.getProjectFormat() == Data::kProjectFormatWindows) {
		if (!reader.skip(14))
			return kDataReadErrorReadFailed;

		uint8 cdefBytes[256 * 4];
		if (!reader.read(cdefBytes, numColors * 4))
			return kDataReadErrorReadFailed;

		for (size_t i = 0; i < numColors; i++) {
			ColorRGB16 &cdef = colors[i];

			cdef.red = cdefBytes[i * 4 + 2] * 0x101;
			cdef.green = cdefBytes[i * 4 + 1] * 0x101;
			cdef.blue = cdefBytes[i * 4 + 0] * 0x101;
		}
	} else
		return kDataReadErrorUnrecognized;

	return kDataReadErrorNone;
}

MovieAsset::MovieAsset()
	: persistFlags(0), assetAndDataCombinedSize(0), unknown1{0, 0, 0, 0}, assetID(0), unknown1_1{0, 0, 0, 0}, extFileNameLength(0),
	  movieDataPos(0), moovAtomPos(0), movieDataSize(0), haveMacPart(false), haveWinPart(false) {
	memset(&this->platform, 0, sizeof(this->platform));
}

DataReadErrorCode MovieAsset::load(DataReader &reader) {
	if (_revision != 0)
		return kDataReadErrorUnsupportedRevision;

	haveMacPart = false;
	haveWinPart = false;

	if (!reader.readU32(persistFlags) || !reader.readU32(assetAndDataCombinedSize) || !reader.readBytes(unknown1)
		|| !reader.readU32(assetID) || !reader.readBytes(unknown1_1) || !reader.readU16(extFileNameLength))
		return kDataReadErrorReadFailed;

	if (reader.getProjectFormat() == Data::kProjectFormatMacintosh) {
		haveMacPart = true;

		if (!reader.readBytes(platform.mac.unknown5_1) || !reader.readU32(movieDataSize) || !reader.readBytes(platform.mac.unknown6) || !reader.readU32(moovAtomPos))
			return kDataReadErrorReadFailed;
	} else if (reader.getProjectFormat() == Data::kProjectFormatWindows) {
		haveWinPart = true;

		if (!reader.readBytes(platform.win.unknown3_1) || !reader.readU32(movieDataSize) || !reader.readBytes(platform.win.unknown4) || !reader.readU32(moovAtomPos) || !reader.readBytes(platform.win.unknown7))
			return kDataReadErrorReadFailed;
	} else
		return kDataReadErrorReadFailed;

	if (!reader.readTerminatedStr(extFileName, extFileNameLength))
		return kDataReadErrorReadFailed;

	movieDataPos = reader.tellGlobal();

	if (!reader.skip(movieDataSize))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

AudioAsset::AudioAsset()
	: persistFlags(0), assetAndDataCombinedSize(0), unknown2{0, 0, 0, 0}, assetID(0),
	  unknown3{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, sampleRate1(0), bitsPerSample(0),
	  encoding1(0), channels(0), codedDuration{0, 0, 0, 0}, sampleRate2(0), cuePointDataSize(0), numCuePoints(0),
	  unknown14{0, 0, 0, 0}, filePosition(0), size(0), haveMacPart(false), haveWinPart(false), isBigEndian(false) {
	memset(&this->platform, 0, sizeof(this->platform));
}

DataReadErrorCode AudioAsset::load(DataReader &reader) {
	if (_revision != 2)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(persistFlags) || !reader.readU32(assetAndDataCombinedSize) || !reader.readBytes(unknown2)
		|| !reader.readU32(assetID) || !reader.readBytes(unknown3))
		return kDataReadErrorReadFailed;

	haveMacPart = false;
	haveWinPart = false;
	isBigEndian = false;

	if (reader.getProjectFormat() == Data::ProjectFormat::kProjectFormatMacintosh) {
		haveMacPart = true;
		isBigEndian = true;

		if (!reader.readBytes(platform.mac.unknown4) || !reader.readU16(sampleRate1) || !reader.readBytes(platform.mac.unknown5)
			|| !reader.readU8(bitsPerSample) || !reader.readU8(encoding1) || !reader.readU8(channels)
			|| !reader.readBytes(codedDuration) || !reader.readBytes(platform.mac.unknown8)
			|| !reader.readU16(sampleRate2))
			return kDataReadErrorReadFailed;
	} else if (reader.getProjectFormat() == Data::ProjectFormat::kProjectFormatWindows) {
		haveWinPart = true;

		if (!reader.readU16(sampleRate1) || !reader.readU8(bitsPerSample) || !reader.readBytes(platform.win.unknown9)
			|| !reader.readU8(encoding1) || !reader.readU8(channels) || !reader.readBytes(codedDuration)
			|| !reader.readBytes(platform.win.unknown11) || !reader.readU16(sampleRate2) || !reader.readBytes(platform.win.unknown12_1))
			return kDataReadErrorReadFailed;
	} else
		return kDataReadErrorUnrecognized;

	if (!reader.readU32(cuePointDataSize) || !reader.readU16(numCuePoints) || !reader.readBytes(unknown14)
		|| !reader.readU32(filePosition) || !reader.readU32(size))
		return kDataReadErrorReadFailed;

	if (numCuePoints * 14u != cuePointDataSize)
		return kDataReadErrorUnrecognized;

	cuePoints.resize(numCuePoints);
	for (size_t i = 0; i < numCuePoints; i++) {
		CuePoint& cuePoint = cuePoints[i];
		if (!reader.readBytes(cuePoint.unknown13) || !reader.readU32(cuePoint.unknown14) || !reader.readU32(cuePoint.position)
			|| !reader.readU32(cuePoint.cuePointID))
			return kDataReadErrorReadFailed;
	}

	return kDataReadErrorNone;
}

ImageAsset::ImageAsset()
	: persistFlags(0), unknown1(0), unknown2{0, 0, 0, 0}, assetID(0), unknown3(0), rect1(Rect::createDefault()),
	  hdpiFixed(0), vdpiFixed(0), bitsPerPixel(0), unknown4{0, 0}, unknown5{0, 0, 0, 0}, unknown6{0, 0, 0, 0, 0, 0, 0, 0},
	  rect2(Rect::createDefault()), filePosition(0), size(0), haveMacPart(false), haveWinPart(false) {
	memset(&this->platform, 0, sizeof(this->platform));
}

DataReadErrorCode ImageAsset::load(DataReader &reader) {
	if (_revision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(persistFlags) || !reader.readU32(unknown1) || !reader.readBytes(unknown2)
		|| !reader.readU32(assetID) || !reader.readU32(unknown3))
		return kDataReadErrorReadFailed;

	haveWinPart = false;
	haveMacPart = false;

	if (reader.getProjectFormat() == kProjectFormatMacintosh) {
		haveMacPart = true;
		if (!reader.readBytes(platform.mac.unknown7))
			return kDataReadErrorReadFailed;
	} else if (reader.getProjectFormat() == kProjectFormatWindows) {
		haveWinPart = true;
		if (!reader.readBytes(platform.win.unknown8))
			return kDataReadErrorReadFailed;
	} else
		return kDataReadErrorUnrecognized;

	if (!rect1.load(reader) || !reader.readU32(hdpiFixed) || !reader.readU32(vdpiFixed) || !reader.readU16(bitsPerPixel)
		|| !reader.readBytes(unknown4) || !reader.readBytes(unknown5) || !reader.readBytes(unknown6)
		|| !rect2.load(reader) || !reader.readU32(filePosition) || !reader.readU32(size))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

MToonAsset::FrameDef::FrameDef()
	: unknown12{0, 0, 0, 0}, rect1(Rect::createDefault()), dataOffset(0), unknown13{0, 0}, compressedSize(0), unknown14(0),
	  keyframeFlag(0), platformBit(0), unknown15(0), rect2(Rect::createDefault()), hdpiFixed(0), vdpiFixed(0), bitsPerPixel(0),
	  unknown16(0), decompressedBytesPerRow(0), decompressedSize(0) {
	memset(&this->platform, 0, sizeof(this->platform));
}

MToonAsset::FrameRangeDef::FrameRangeDef() : startFrame(0), endFrame(0), lengthOfName(0), unknown14(0) {
}


MToonAsset::FrameRangePart::FrameRangePart() : tag(0), sizeIncludingTag(0), numFrameRanges(0) {
}

MToonAsset::MToonAsset()
	: marker(0), unknown1{0, 0, 0, 0, 0, 0, 0, 0}, assetID(0), haveMacPart(false), haveWinPart(false), frameDataPosition(0), sizeOfFrameData(0),
	  mtoonHeader{0, 0}, version(0), unknown2{0, 0, 0, 0}, encodingFlags(0), rect(Rect::createDefault()), numFrames(0),
	  unknown3{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, bitsPerPixel(0), codecID(0), unknown4_1{0, 0, 0, 0, 0, 0, 0, 0},
	  codecDataSize(0), unknown4_2{0, 0, 0, 0} {
	memset(&this->platform, 0, sizeof(this->platform));
}

DataReadErrorCode MToonAsset::load(DataReader &reader) {
	if (_revision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(marker) || !reader.readBytes(unknown1) || !reader.readU32(assetID))
		return kDataReadErrorReadFailed;

	haveMacPart = false;
	haveWinPart = false;

	if (reader.getProjectFormat() == kProjectFormatMacintosh) {
		haveMacPart = true;

		if (!reader.readBytes(platform.mac.unknown10))
			return kDataReadErrorReadFailed;
	} else if (reader.getProjectFormat() == kProjectFormatWindows) {
		haveWinPart = true;

		if (!reader.readBytes(platform.win.unknown11))
			return kDataReadErrorReadFailed;
	} else
		return kDataReadErrorUnrecognized;

	if (!reader.readU32(frameDataPosition) || !reader.readU32(sizeOfFrameData) || !reader.readU32(mtoonHeader[0])
		|| !reader.readU32(mtoonHeader[1]) || !reader.readU16(version) || !reader.readBytes(unknown2)
		|| !reader.readU32(encodingFlags) || !rect.load(reader) || !reader.readU16(numFrames)
		|| !reader.readBytes(unknown3) || !reader.readU16(bitsPerPixel) || !reader.readU32(codecID)
		|| !reader.readBytes(unknown4_1) || !reader.readU32(codecDataSize) || !reader.readBytes(unknown4_2))
		return kDataReadErrorReadFailed;

	if (mtoonHeader[0] != 0 || mtoonHeader[1] != 0x546f6f6e)
		return kDataReadErrorUnrecognized;

	if (numFrames > 0) {
		frames.resize(numFrames);
		for (size_t i = 0; i < numFrames; i++) {
			FrameDef &frame = frames[i];

			if (!reader.readBytes(frame.unknown12) || !frame.rect1.load(reader) || !reader.readU32(frame.dataOffset)
				|| !reader.readBytes(frame.unknown13) || !reader.readU32(frame.compressedSize) || !reader.readU8(frame.unknown14)
				|| !reader.readU8(frame.keyframeFlag) || !reader.readU8(frame.platformBit) || !reader.readU8(frame.unknown15)
				|| !frame.rect2.load(reader) || !reader.readU32(frame.hdpiFixed) || !reader.readU32(frame.vdpiFixed)
				|| !reader.readU16(frame.bitsPerPixel) || !reader.readU32(frame.unknown16) || !reader.readU16(frame.decompressedBytesPerRow))
				return kDataReadErrorReadFailed;

			if (reader.getProjectFormat() == kProjectFormatMacintosh) {
				if (!reader.readBytes(frame.platform.mac.unknown17))
					return kDataReadErrorReadFailed;
			} else if (reader.getProjectFormat() == kProjectFormatWindows) {
				if (!reader.readBytes(frame.platform.win.unknown18))
					return kDataReadErrorReadFailed;
			} else
				return kDataReadErrorUnrecognized;

			if (!reader.readU32(frame.decompressedSize))
				return kDataReadErrorReadFailed;
		}
	}

	if (codecDataSize > 0) {
		codecData.resize(codecDataSize);
		if (!reader.read(&codecData[0], codecDataSize))
			return kDataReadErrorReadFailed;
	}

	if (encodingFlags & kEncodingFlag_HasRanges) {
		if (!reader.readU32(frameRangesPart.tag) || !reader.readU32(frameRangesPart.sizeIncludingTag) || !reader.readU32(frameRangesPart.numFrameRanges))
			return kDataReadErrorReadFailed;

		if (frameRangesPart.tag != 1)
			return kDataReadErrorUnrecognized;

		if (frameRangesPart.numFrameRanges > 0) {
			frameRangesPart.frameRanges.resize(frameRangesPart.numFrameRanges);
			for (size_t i = 0; i < frameRangesPart.numFrameRanges; i++) {
				FrameRangeDef &frameRange = frameRangesPart.frameRanges[i];

				if (!reader.readU32(frameRange.startFrame) || !reader.readU32(frameRange.endFrame) || !reader.readU8(frameRange.lengthOfName) || !reader.readU8(frameRange.unknown14))
					return kDataReadErrorReadFailed;

				if (!reader.readTerminatedStr(frameRange.name, frameRange.lengthOfName))
					return kDataReadErrorReadFailed;
			}
		}
	}

	return kDataReadErrorNone;
}

TextAsset::TextAsset()
	: persistFlags(0), sizeIncludingTag(0), unknown1(0), assetID(0), unknown2(0), bitmapRect(Rect::createDefault()),
	  hdpi(0), vdpi(0), unknown5(0), pitchBigEndian{0, 0}, unknown6(0), bitmapSize(0),
	  unknown7{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, textSize(0),
	  unknown8{0, 0, 0, 0, 0, 0, 0, 0}, alignment(0), isBitmap(0), haveMacPart(false), haveWinPart(false), isBottomUp(false) {

	memset(&this->platform, 0, sizeof(this->platform));
}

DataReadErrorCode TextAsset::load(DataReader &reader) {
	if (_revision != 3)
		return kDataReadErrorReadFailed;

	if (!reader.readU32(persistFlags) || !reader.readU32(sizeIncludingTag) || !reader.readU32(unknown1)
		|| !reader.readU32(assetID) || !reader.readU32(unknown2))
		return kDataReadErrorReadFailed;

	haveMacPart = false;
	haveWinPart = false;
	if (reader.getProjectFormat() == kProjectFormatMacintosh) {
		haveMacPart = true;
		isBottomUp = false;
		if (!reader.readBytes(platform.mac.unknown3))
			return kDataReadErrorReadFailed;
	} else if (reader.getProjectFormat() == kProjectFormatWindows) {
		haveWinPart = true;
		isBottomUp = true;
		if (!reader.readBytes(platform.win.unknown4))
			return kDataReadErrorReadFailed;
	} else
		return kDataReadErrorUnrecognized;

	if (!bitmapRect.load(reader) || !reader.readU32(hdpi) || !reader.readU32(vdpi) || !reader.readU16(unknown5)
		|| !reader.readBytes(pitchBigEndian) || !reader.readU32(unknown6) || !reader.readU32(bitmapSize)
		|| !reader.readBytes(unknown7) || !reader.readU32(textSize) || !reader.readBytes(unknown8)
		|| !reader.readU16(alignment) || !reader.readU16(isBitmap))
		return kDataReadErrorReadFailed;

	if ((isBitmap & 1) == 0) {
		if (!reader.readNonTerminatedStr(text, textSize))
			return kDataReadErrorReadFailed;

		if (reader.getProjectFormat() == kProjectFormatMacintosh) {
			uint16 numFormattingSpans;
			if (!reader.readU16(numFormattingSpans))
				return kDataReadErrorReadFailed;

			macFormattingSpans.resize(numFormattingSpans);
			for (size_t i = 0; i < numFormattingSpans; i++) {
				MacFormattingSpan &span = macFormattingSpans[i];
				if (!reader.readBytes(span.unknown9) || !reader.readU16(span.spanStart) || !reader.readBytes(span.unknown10)
					|| !reader.readU16(span.fontID) || !reader.readU8(span.fontFlags) || !reader.readBytes(span.unknown11)
					|| !reader.readU16(span.size) || !reader.readBytes(span.unknown12))
					return kDataReadErrorReadFailed;
			}
		}
	} else {
		bitmapData.resize(bitmapSize);
		if (bitmapSize > 0 && !reader.read(&bitmapData[0], bitmapSize))
			return kDataReadErrorReadFailed;
	}

	return kDataReadErrorNone;
}

AssetDataChunk::AssetDataChunk() : unknown1(0), sizeIncludingTag(0), filePosition(0) {
}

DataReadErrorCode AssetDataChunk::load(DataReader &reader) {
	if (_revision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!reader.readU32(unknown1) || !reader.readU32(sizeIncludingTag) || sizeIncludingTag < 14 || !reader.skip(sizeIncludingTag - 14))
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
		warning("Failed to read data object header");
		return kDataReadErrorReadFailed;
	}

	debug(4, "Loading data object type %x", static_cast<int>(type));

	DataObject *dataObject = nullptr;
	switch (type) {
	case DataObjectTypes::kProjectLabelMap:
		dataObject = new ProjectLabelMap();
		break;
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
	case DataObjectTypes::kProjectStructuralDef:
		dataObject = new ProjectStructuralDef();
		break;
	case DataObjectTypes::kSectionStructuralDef:
		dataObject = new SectionStructuralDef();
		break;
	case DataObjectTypes::kSubsectionStructuralDef:
		dataObject = new SubsectionStructuralDef();
		break;

	case DataObjectTypes::kGraphicElement:
		dataObject = new GraphicElement();
		break;
	case DataObjectTypes::kMovieElement:
		dataObject = new MovieElement();
		break;
	case DataObjectTypes::kMToonElement:
		dataObject = new MToonElement();
		break;
	case DataObjectTypes::kImageElement:
		dataObject = new ImageElement();
		break;
	case DataObjectTypes::kSoundElement:
		dataObject = new SoundElement();
		break;
	case DataObjectTypes::kTextLabelElement:
		dataObject = new TextLabelElement();
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
	case DataObjectTypes::kSaveAndRestoreModifier:
		dataObject = new SaveAndRestoreModifier();
		break;
	case DataObjectTypes::kMessengerModifier:
		dataObject = new MessengerModifier();
		break;
	case DataObjectTypes::kSetModifier:
		dataObject = new SetModifier();
		break;
	case DataObjectTypes::kAliasModifier:
		dataObject = new AliasModifier();
		break;
	case DataObjectTypes::kChangeSceneModifier:
		dataObject = new ChangeSceneModifier();
		break;
	case DataObjectTypes::kSoundEffectModifier:
		dataObject = new SoundEffectModifier();
		break;
	case DataObjectTypes::kDragMotionModifier:
		dataObject = new DragMotionModifier();
		break;
	case DataObjectTypes::kPathMotionModifierV2:
		dataObject = new PathMotionModifierV2();
		break;
	case DataObjectTypes::kVectorMotionModifier:
		dataObject = new VectorMotionModifier();
		break;
	case DataObjectTypes::kSceneTransitionModifier:
		dataObject = new SceneTransitionModifier();
		break;
	case DataObjectTypes::kElementTransitionModifier:
		dataObject = new ElementTransitionModifier();
		break;
	case DataObjectTypes::kIfMessengerModifier:
		dataObject = new IfMessengerModifier();
		break;
	case DataObjectTypes::kCompoundVariableModifier:
		dataObject = new CompoundVariableModifier();
		break;
	case DataObjectTypes::kBooleanVariableModifier:
		dataObject = new BooleanVariableModifier();
		break;
	case DataObjectTypes::kIntegerVariableModifier:
		dataObject = new IntegerVariableModifier();
		break;
	case DataObjectTypes::kIntegerRangeVariableModifier:
		dataObject = new IntegerRangeVariableModifier();
		break;
	case DataObjectTypes::kPointVariableModifier:
		dataObject = new PointVariableModifier();
		break;
	case DataObjectTypes::kFloatingPointVariableModifier:
		dataObject = new FloatingPointVariableModifier();
		break;
	case DataObjectTypes::kVectorVariableModifier:
		dataObject = new VectorVariableModifier();
		break;
	case DataObjectTypes::kStringVariableModifier:
		dataObject = new StringVariableModifier();
		break;
	case DataObjectTypes::kDebris:
		dataObject = new Debris();
		break;
	case DataObjectTypes::kPlugInModifier:
		dataObject = new PlugInModifier();
		break;
	case DataObjectTypes::kTimerMessengerModifier:
		dataObject = new TimerMessengerModifier();
		break;
	case DataObjectTypes::kCollisionDetectionMessengerModifier:
		dataObject = new CollisionDetectionMessengerModifier();
		break;
	case DataObjectTypes::kBoundaryDetectionMessengerModifier:
		dataObject = new BoundaryDetectionMessengerModifier();
		break;
	case DataObjectTypes::kKeyboardMessengerModifier:
		dataObject = new KeyboardMessengerModifier();
		break;
	case DataObjectTypes::kTextStyleModifier:
		dataObject = new TextStyleModifier();
		break;
	case DataObjectTypes::kGraphicModifier:
		dataObject = new GraphicModifier();
		break;

	case DataObjectTypes::kColorTableAsset:
		dataObject = new ColorTableAsset();
		break;

	case DataObjectTypes::kMovieAsset:
		dataObject = new MovieAsset();
		break;

	case DataObjectTypes::kAudioAsset:
		dataObject = new AudioAsset();
		break;

	case DataObjectTypes::kImageAsset:
		dataObject = new ImageAsset();
		break;

	case DataObjectTypes::kMToonAsset:
		dataObject = new MToonAsset();
		break;

	case DataObjectTypes::kTextAsset:
		dataObject = new TextAsset();
		break;

	case DataObjectTypes::kAssetDataChunk:
		dataObject = new AssetDataChunk();
		break;

	default:
		break;
	}

	if (dataObject == nullptr) {
		warning("Unrecognized data object type %x", static_cast<int>(type));
		return kDataReadErrorUnrecognized;
	}

	Common::SharedPtr<DataObject> sharedPtr(dataObject);
	DataReadErrorCode errorCode = dataObject->load(static_cast<DataObjectTypes::DataObjectType>(type), revision, reader);
	if (errorCode != kDataReadErrorNone) {
		warning("Data object type %x failed to load", static_cast<int>(type));
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
		errorCode = plugInModifierData->load(plugInLoader->getPlugIn(), *static_cast<const PlugInModifier *>(dataObject), reader);
		if (errorCode != kDataReadErrorNone) {
			warning("Plug-in modifier failed to load");
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
