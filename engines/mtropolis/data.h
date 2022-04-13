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

	kChangeSceneModifier                 = 0x136,
	kReturnModifier                      = 0x140,	// NYI
	kSoundEffectModifier                 = 0x1a4,	// NYI
	kDragMotionModifier                  = 0x208,
	kPathMotionModifierV1                = 0x21c,	// NYI - Obsolete version
	kPathMotionModifierV2                = 0x21b,	// NYI
	kVectorMotionModifier                = 0x226,
	kSceneTransitionModifier             = 0x26c,
	kElementTransitionModifier           = 0x276,
	kSharedSceneModifier                 = 0x29a,	// NYI
	kIfMessengerModifier                 = 0x2bc,
	kBehaviorModifier                    = 0x2c6,
	kMessengerModifier                   = 0x2da,
	kSetModifier                         = 0x2df,
	kTimerMessengerModifier              = 0x2e4,
	kCollisionDetectionMessengerModifier = 0x2ee,
	kBoundaryDetectionMessengerModifier  = 0x2f8,
	kKeyboardMessengerModifier           = 0x302,
	kTextStyleModifier                   = 0x32a,
	kGraphicModifier                     = 0x334,
	kImageEffectModifier                 = 0x384,	// NYI
	kMiniscriptModifier                  = 0x3c0,
	kCursorModifierV1                    = 0x3ca,	// NYI - Obsolete version
	kGradientModifier                    = 0x4b0,	// NYI
	kColorTableModifier                  = 0x4c4,	// NYI
	kSaveAndRestoreModifier              = 0x4d8,	// NYI

	kCompoundVariableModifier            = 0x2c7,
	kBooleanVariableModifier             = 0x321,
	kIntegerVariableModifier             = 0x322,
	kIntegerRangeVariableModifier        = 0x324,
	kVectorVariableModifier              = 0x327,
	kPointVariableModifier               = 0x326,
	kFloatingPointVariableModifier       = 0x328,
	kStringVariableModifier              = 0x329,

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

struct ColorRGB16 {
	bool load(DataReader &reader);

	uint16 red;
	uint16 green;
	uint16 blue;
};

struct IntRange {
	bool load(DataReader &reader);

	int32 min;
	int32 max;
};

struct XPFloat {
	bool load(DataReader &reader);
	double toDouble() const;

	uint64 mantissa;
	uint16 signAndExponent;
};

struct XPFloatVector {
	bool load(DataReader &reader);

	XPFloat angleRadians;
	XPFloat magnitude;
};

struct Label {
	bool load(DataReader &reader);

	uint32 superGroupID;
	uint32 labelID;
};

// mTropolis uses two separate type-tagged value formats.
//
// InternalTypeTaggedValue is used by internal modifiers for messenger payloads and set modifiers
// and seems to match Miniscript ops too.
// InternalTypeTaggedValue is always 44 bytes in size and stores string data elsewhere in the containing structure.
//
// PlugInTypeTaggedValue is used by plug-ins and is fully self-contained.
//
// If you change something here, remember to update DynamicValue::load
struct InternalTypeTaggedValue {
	enum TypeCode {
		kNull = 0x00,
		kInteger = 0x01,
		kString = 0x0d, // String data is stored externally from the value
		kPoint = 0x10,
		kIntegerRange = 0x11,
		kFloat = 0x15,
		kBool = 0x1a,
		kIncomingData = 0x1b,
		kVariableReference = 0x1c,
		kLabel = 0x1d,
	};

	struct VariableReference {
		uint32 unknown;
		uint32 guid;
	};

	union ValueUnion {
		uint8 asBool;
		XPFloat asFloat;
		int32 asInteger;
		IntRange asIntegerRange;
		VariableReference asVariableReference;
		Label asLabel;
		Point asPoint;
	};

	uint16 type;
	ValueUnion value;

	bool load(DataReader &reader);
};

struct PlugInTypeTaggedValue : public Common::NonCopyable {
	enum TypeCode {
		kNull = 0x00,
		kInteger = 0x01,
		kPoint = 0xa,
		kIntegerRange = 0xb,
		kFloat = 0xf,
		kBoolean = 0x14,
		kEvent = 0x17,
		kLabel = 0x64,
		kString = 0x66,
		kIncomingData = 0x6e,
		kVariableReference = 0x73,	// Has extra data
	};

	union ValueUnion {
		int32 asInt;
		Point asPoint;
		IntRange asIntRange;
		XPFloat asFloat;
		uint16 asBoolean;
		Event asEvent;
		Label asLabel;
		uint32 asVarRefGUID;
	};

	uint16 type;
	ValueUnion value;

	Common::String str;
	Common::Array<uint8> extraData;

	bool load(DataReader &reader);
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
	InternalTypeTaggedValue with;
	uint8 withSourceLength;
	uint8 withStringLength;

	Common::String withSource;
	Common::String withString;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct SetModifier : public DataObject {
	TypicalModifierHeader modHeader;

	uint8 unknown1[4];
	Event executeWhen;
	InternalTypeTaggedValue source;
	InternalTypeTaggedValue target;
	uint8 unknown3;
	uint8 sourceNameLength;
	uint8 targetNameLength;
	uint8 sourceStringLength;
	uint8 targetStringLength;
	uint8 unknown4;

	Common::String sourceName;
	Common::String targetName;
	Common::String sourceString;
	Common::String targetString;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct ChangeSceneModifier : public DataObject {
	TypicalModifierHeader modHeader;

	enum ChangeSceneFlags {
		kChangeSceneFlagNextScene       = 0x80000000,
		kChangeSceneFlagPrevScene       = 0x40000000,
		kChangeSceneFlagSpecificScene   = 0x20000000,
		kChangeSceneFlagAddToReturnList = 0x10000000,
		kChangeSceneFlagAddToDestList   = 0x08000000,
		kChangeSceneFlagWrapAround      = 0x04000000,
	};

	uint32 changeSceneFlags;
	Event executeWhen;
	uint32 targetSectionGUID;
	uint32 targetSubsectionGUID;
	uint32 targetSceneGUID;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct DragMotionModifier : public DataObject {
	TypicalModifierHeader modHeader;

	Event enableWhen;
	Event disableWhen;

	struct WinPart {
		uint8_t unknown2;
		uint8_t constrainHorizontal;
		uint8_t constrainVertical;
		uint8_t constrainToParent;
	};

	struct MacPart {
		uint8_t flags;
		uint8_t unknown3;

		enum Flags {
			kConstrainToParent = 0x10,
			kConstrainVertical = 0x20,
			kConstrainHorizontal = 0x40,
		};
	};

	union PlatformPart {
		WinPart win;
		MacPart mac;
	};

	PlatformPart platform;

	bool haveMacPart;
	bool haveWinPart;
	Rect constraintMargin;
	uint16_t unknown1;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct VectorMotionModifier : public DataObject {
	TypicalModifierHeader modHeader;

	Event enableWhen;
	Event disableWhen;
	InternalTypeTaggedValue vec;
	uint16 unknown1;
	uint8 vecSourceLength;
	uint8 vecStringLength;

	Common::String vecSource;
	Common::String vecString;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct SceneTransitionModifier : public DataObject {
	TypicalModifierHeader modHeader;

	Event enableWhen;
	Event disableWhen;
	uint16 transitionType;
	uint16 direction;
	uint16 unknown3;
	uint16 steps;
	uint32 duration;
	uint8 unknown5[2];

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct ElementTransitionModifier : public DataObject {
	TypicalModifierHeader modHeader;

	Event enableWhen;
	Event disableWhen;
	uint16 revealType;
	uint16 transitionType;
	uint16 unknown3;
	uint16 unknown4;
	uint16 steps;
	uint16 rate;

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
	InternalTypeTaggedValue with;
	uint8 unknown9[10];
	uint8 withSourceLength;
	uint8 withStringLength;
	MiniscriptProgram program;

	Common::String withSource;
	Common::String withString;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct TimerMessengerModifier : public DataObject {
	TypicalModifierHeader modHeader;

	enum TimerFlags {
		kTimerFlagLooping = 0x10000000,
	};

	uint32 messageAndTimerFlags;
	Event executeWhen;
	Event send;
	Event terminateWhen;
	uint16 unknown2;
	uint32 destination;
	uint8 unknown4[10];
	InternalTypeTaggedValue with;
	uint8 unknown5;
	uint8 minutes;
	uint8 seconds;
	uint8 hundredthsOfSeconds;
	uint32 unknown6;
	uint32 unknown7;
	uint8 unknown8[10];
	uint8 withSourceLength;
	uint8 withStringLength;

	Common::String withSource;
	Common::String withString;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct BoundaryDetectionMessengerModifier : public DataObject {
	enum Flags {
		kDetectTopEdge = 0x1000,
		kDetectBottomEdge = 0x0800,
		kDetectLeftEdge = 0x0400,
		kDetectRightEdge = 0x0200,
		kDetectExiting = 0x0100, // Off = once exited
		kWhileDetected = 0x0080, // Off = on first detected
	};

	TypicalModifierHeader modHeader;
	uint16 messageFlagsHigh;
	Event enableWhen;
	Event disableWhen;
	Event send;
	uint16 unknown2;
	uint32 destination;
	uint8 unknown3[10];
	InternalTypeTaggedValue with;
	uint8 withSourceLength;
	uint8 withStringLength;

	Common::String withSource;
	Common::String withString;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct CollisionDetectionMessengerModifier : public DataObject {
	enum ModifierFlags {
		kDetectLayerInFront = 0x10000000,
		kDetectLayerBehind = 0x08000000,
		kSendToCollidingElement = 0x02000000,
		kSendToOnlyFirstCollidingElement = 0x00200000,

		kDetectionModeMask = 0x01c00000,
		kDetectionModeFirstContact = 0x01400000,
		kDetectionModeWhileInContact = 0x01000000,
		kDetectionModeExiting = 0x00800000,

		kNoCollideWithParent = 0x00100000,
	};

	TypicalModifierHeader modHeader;
	uint32 messageAndModifierFlags;
	Event enableWhen;
	Event disableWhen;
	Event send;
	uint16 unknown2;
	uint32 destination;
	uint8 unknown3[10];
	InternalTypeTaggedValue with;
	uint8 withSourceLength;
	uint8 withStringLength;

	Common::String withSource;
	Common::String withString;

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
	InternalTypeTaggedValue with;
	uint8 withSourceLength;
	uint8 withStringLength;

	Common::String withSource;
	Common::String withString;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct TextStyleModifier : public DataObject {
	TypicalModifierHeader modHeader;

	TypicalModifierHeader m_modHeader;
	uint8 unknown1[4];
	uint16 macFontID;
	uint8 flags;
	uint8 unknown2;
	uint16 size;
	ColorRGB16 textColor;		// Appears to not actually be used
	ColorRGB16 backgroundColor; // Appears to not actually be used
	uint16 alignment;
	uint16 unknown3;
	Event applyWhen;
	Event removeWhen;
	uint16_t lengthOfFontFamilyName;

	Common::String fontFamilyName;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct GraphicModifier : public DataObject {
	TypicalModifierHeader modHeader;

	uint16 unknown1;
	Event applyWhen;
	Event removeWhen;
	uint8 unknown2[2];
	uint16 inkMode;
	uint16 shape;

	struct MacPart {
		uint8 unknown4_1[6];
		uint8 unknown4_2[26];
	};

	struct WinPart {
		uint8 unknown5_1[4];
		uint8 unknown5_2[22];
	};

	union PlatformPart {
		MacPart mac;
		WinPart win;
	};

	bool haveMacPart;
	bool haveWinPart;
	PlatformPart platform;

	ColorRGB16 foreColor;
	ColorRGB16 backColor;
	uint16 borderSize;
	ColorRGB16 borderColor;
	uint16 shadowSize;
	ColorRGB16 shadowColor;

	uint16 numPolygonPoints;
	uint8 unknown6[8];

	Common::Array<Point> polyPoints;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct CompoundVariableModifier : public DataObject {
	// This doesn't follow the usual modifier header layout
	uint32 modifierFlags;
	uint32 sizeIncludingTag;
	uint8 unknown1[2];	// Extra field
	uint32 guid;
	uint8 unknown4[6];
	uint32 unknown5;
	Point editorLayoutPosition;
	uint16 lengthOfName;
	uint16 numChildren;
	uint8 unknown7[4];

	Common::String name;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct BooleanVariableModifier : public DataObject {
	TypicalModifierHeader modHeader;
	uint8 value;
	uint8 unknown5;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct IntegerVariableModifier : public DataObject {
	TypicalModifierHeader modHeader;
	uint8 unknown1[4];
	int32 value;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct IntegerRangeVariableModifier : public DataObject {
	TypicalModifierHeader modHeader;
	uint8 unknown1[4];
	IntRange range;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct VectorVariableModifier : public DataObject {
	TypicalModifierHeader modHeader;
	uint8 unknown1[4];
	XPFloatVector vector;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct PointVariableModifier : public DataObject {
	TypicalModifierHeader modHeader;

	uint8 unknown5[4];
	Point value;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct FloatingPointVariableModifier : public DataObject {
	TypicalModifierHeader modHeader;
	uint8 unknown1[4];
	XPFloat value;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct StringVariableModifier : public DataObject {
	TypicalModifierHeader modHeader;
	uint32 lengthOfString;
	uint8 unknown1[4];
	Common::String value;

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
