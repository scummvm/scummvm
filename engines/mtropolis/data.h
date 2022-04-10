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
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/ptr.h"
#include "common/stream.h"

// This contains defs related to parsing of mTropolis stored data into structured data objects.
namespace MTropolis {

namespace Data {

struct PlugInModifier;
struct PlugInModifierData;

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
	kDataReadErrorPlugInNotFound,
};

enum ModifierFlags {
	kModifierFlagLast = 0x2,
};


namespace DataObjectTypes {

enum DataObjectType {
	kUnknown                             = 0,

	kProjectCatalog                      = 0x3e8,
	kStreamHeader                        = 0x3e9,
	kProjectHeader                       = 0x3ea,
	kPresentationSettings                = 0x3ec,

	kAssetCatalog                        = 0xd,
    kGlobalObjectInfo                    = 0x17,
	kUnknown19                           = 0x19,
	kProjectLabelMap                     = 0x22,	// NYI

	kProjectStructuralDef                = 0x2,		// NYI
	kSectionStructuralDef                = 0x3,		// NYI
	kSceneStructuralDef                  = 0x8,
	kSubsectionStructuralDef             = 0x21,	// NYI
	kMovieStructuralDef                  = 0x5,		// NYI
	kMToonStructuralDef                  = 0x6,		// NYI
	kGraphicStructuralDef                = 0x7,		// NYI
	kSoundStructuralDef                  = 0xa,		// NYI

	kTextLabelElement                    = 0x15,	// NYI

	kAlias                               = 0x27,	// NYI

	kMovieAsset                          = 0x10,	// NYI
	kSoundAsset                          = 0x11,	// NYI
	kColorTableAsset                     = 0x1e,	// NYI
	kImageAsset                          = 0xe,		// NYI
	kMToonAsset                          = 0xf,		// NYI

	kSoundEffectModifier                 = 0x1a4,	// NYI
	kChangeSceneModifier                 = 0x136,	// NYI
	kReturnModifier                      = 0x140,	// NYI
	kDragMotionModifier                  = 0x208,	// NYI
	kVectorMotionModifier                = 0x226,	// NYI
	kPathMotionModifierV1                = 0x21c,	// NYI - Obsolete version
	kPathMotionModifierV2                = 0x21b,	// NYI
	kSceneTransitionModifier             = 0x26c,	// NYI
	kElementTransitionModifier           = 0x276,	// NYI
	kSharedSceneModifier                 = 0x29a,	// NYI
	kIfMessengerModifier                 = 0x2bc,
	kBehaviorModifier                    = 0x2c6,
	kMessengerModifier                   = 0x2da,
	kSetModifier                         = 0x2df,	// NYI
	kCollisionDetectionMessengerModifier = 0x2ee,	// NYI
	kBoundaryDetectionMessengerModifier  = 0x2f8,	// NYI
	kKeyboardMessengerModifier           = 0x302,
	kTextStyleModifier                   = 0x32a,	// NYI
	kGraphicModifier                     = 0x334,	// NYI
	kImageEffectModifier                 = 0x384,	// NYI
	kMiniscriptModifier                  = 0x3c0,
	kCursorModifierV1                    = 0x3ca,	// NYI - Obsolete version
	kGradientModifier                    = 0x4b0,	// NYI
	kColorTableModifier                  = 0x4c4,	// NYI
	kSaveAndRestoreModifier              = 0x4d8,	// NYI

	kCompoundVariableModifier            = 0x2c7,	// NYI
	kBooleanVariableModifier             = 0x321,
	kIntegerVariableModifier             = 0x322,	// NYI
	kIntegerRangeVariableModifier        = 0x324,	// NYI
	kVectorVariableModifier              = 0x327,	// NYI
	kFloatingPointVariableModifier       = 0x328,	// NYI
	kPointVariableModifier               = 0x326,
	kStringVariableModifier              = 0x329,	// NYI

	kDebris                              = 0xfffffffe,	// Deleted object
	kPlugInModifier                      = 0xffffffff,
	kAssetDataChunk                      = 0xffff,
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
	bool readF80(double &value);

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
	bool isBigEndian() const;

private:
	bool checkErrorAndReset();

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
	uint32 persistFlags;
	uint32 sizeIncludingTag;
	uint8 unknown1[2];
	Point dimensions;
	uint16 bitsPerPixel;
	uint16 unknown4;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct AssetCatalog : public DataObject {
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

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct Unknown19 : public DataObject {
	uint32 persistFlags;
	uint32 sizeIncludingTag;
	uint8 unknown1[2];

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct GlobalObjectInfo : public DataObject {
	DataReadErrorCode load(DataReader &reader) override;

	uint32 persistFlags;
	uint32 sizeIncludingTag;
	uint16 numGlobalModifiers;
	uint8 unknown1[4];
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

struct StreamHeader : public DataObject {

	uint32 marker;
	uint32 sizeIncludingTag;
	char name[17];
	uint8 projectID[2];
	uint8 unknown1[4]; // Seems to be consistent across builds
	uint16 unknown2;   // 0

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct BehaviorModifier : public DataObject {

	uint32 modifierFlags;
	uint32 sizeIncludingTag;
	uint8 unknown2[2];
	uint32 guid;
	uint32 unknown4;
	uint16 unknown5;
	uint32 unknown6;
	Point editorLayoutPosition;
	uint16 lengthOfName;
	uint16 numChildren;
	uint32 flags;
	Event enableWhen;
	Event disableWhen;
	uint8 unknown7[2];

	Common::String name;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct MiniscriptProgram {
	struct LocalRef {
		uint32 guid;
		uint8 lengthOfName;
		uint8 unknown2;

		Common::String name;
	};

	struct Attribute {
		uint8 lengthOfName;
		uint8 unknown3;

		Common::String name;
	};

	uint32 unknown1;
	uint32 sizeOfInstructions;
	uint32 numOfInstructions;
	uint32 numLocalRefs;
	uint32 numAttributes;

	Common::Array<uint8> bytecode;
	Common::Array<LocalRef> localRefs;
	Common::Array<Attribute> attributes;

	ProjectFormat projectFormat;
	bool isBigEndian;

	bool load(DataReader &reader);
};

// Header used for most modifiers, but not all
struct TypicalModifierHeader {
	uint32 modifierFlags;
	uint32 sizeIncludingTag;
	uint32 guid;
	uint8 unknown3[6];
	uint32 unknown4;
	Point editorLayoutPosition;
	uint16 lengthOfName;

	Common::String name;

	bool load(DataReader &reader);
};

struct MiniscriptModifier : public DataObject {

	TypicalModifierHeader modHeader;
	Event enableWhen;
	uint8 unknown6[11];
	uint8 unknown7;

	MiniscriptProgram program;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

enum MessageFlags {
	kMessageFlagNoRelay = 0x20000000,
	kMessageFlagNoCascade = 0x40000000,
	kMessageFlagNoImmediate = 0x80000000,
};

struct MessengerModifier : public DataObject {
	TypicalModifierHeader modHeader;

	uint32 messageFlags;
	Event send;
	Event when;
	uint16 unknown14;
	uint32 destination;
	uint8 unknown11[10];
	uint16 with;
	uint8 unknown15[4];
	uint32 withSourceGUID;
	uint8 unknown12[36];
	uint8 withSourceLength;
	uint8 unknown13;

	Common::String withSourceName;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct IfMessengerModifier : public DataObject {
	TypicalModifierHeader modHeader;

	uint32 messageFlags;
	Event send;
	Event when;
	uint16 unknown6;
	uint32 destination;
	uint8 unknown7[10];
	uint16 with;
	uint8 unknown8[4];
	uint32 withSourceGUID;
	uint8 unknown9[46];
	uint8 withSourceLength;
	uint8 unknown10;
	MiniscriptProgram program;

	Common::String withSource;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct KeyboardMessengerModifier : public DataObject {
	enum KeyStateFlags {
		kOnDown = 0x10000000,
		kOnUp = 0x4000000,
		kOnRepeat = 0x8000000,

		kKeyStateMask = (kOnDown | kOnUp | kOnRepeat),
	};

	enum KeyModifiers {
		kControl = 0x1000,
		kCommand = 0x0100,
		kOption = 0x0800,
	};

	enum KeyCodes {
		kAny = 0x00,
		kHome = 0x01,
		kEnter = 0x03,
		kEnd = 0x04,
		kHelp = 0x05,
		kBackspace = 0x08,
		kTab = 0x09,
		kPageUp = 0x0b,
		kPageDown = 0x0c,
		kReturn = 0x0d,
		kEscape = 0x1b,
		kArrowLeft = 0x1c,
		kArrowRight = 0x1d,
		kArrowUp = 0x1e,
		kArrowDown = 0x1f,
		kDelete = 0x7f,
	};

	TypicalModifierHeader modHeader;
	uint32 messageFlagsAndKeyStates;
	uint16 unknown2;
	uint16 keyModifiers;
	uint8 keycode;
	uint8 unknown4[7];
	Event message;
	uint16 unknown7;
	uint32 destination;
	uint8 unknown9[10];
	uint16 with;
	uint8 unknown11[4];
	uint32 withSourceGUID;
	uint8 unknown13[36];
	uint8 withSourceLength;
	uint8 unknown14;

	Common::String withSource;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct BooleanVariableModifier final : public DataObject {
	TypicalModifierHeader modHeader;
	uint8_t value;
	uint8_t unknown5;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct PointVariableModifier final : public DataObject {
	TypicalModifierHeader modHeader;

	uint8_t unknown5[4];
	Point value;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct PlugInModifierData {
	virtual ~PlugInModifierData();
	virtual DataReadErrorCode load(const PlugInModifier &prefix, DataReader &reader) = 0;
};

struct PlugInModifier : public DataObject {
	uint32 modifierFlags;
	uint32 codedSize;	// Total size on Mac but (size + (name length * 255)) on Windows for some reason
	char modifierName[17];
	uint32 guid;
	uint8 unknown2[6];
	uint16 plugInRevision;
	uint32 unknown4;
	Point editorLayoutPosition;
	uint16 lengthOfName;

	Common::String name;

	uint32 subObjectSize;

	Common::SharedPtr<PlugInModifierData> plugInData;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct Debris : public DataObject {
	uint32 persistFlags;
	uint32 sizeIncludingTag;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct IPlugInModifierDataFactory {
	virtual Common::SharedPtr<Data::PlugInModifierData> createModifierData() const = 0;
};

class PlugInModifierRegistry {
public:
	const IPlugInModifierDataFactory *findLoader(const char *modifierName) const;
	void registerLoader(const char *modifierName, const IPlugInModifierDataFactory *loader);

private:
	Common::HashMap<Common::String, const IPlugInModifierDataFactory *> _loaders;
};

DataReadErrorCode loadDataObject(const PlugInModifierRegistry &registry, DataReader &reader, Common::SharedPtr<DataObject> &outObject);

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
