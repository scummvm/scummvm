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
#include "common/rect.h"
#include "common/xpfloat.h"

#include "mtropolis/core.h"

// This contains defs related to parsing of mTropolis stored data into structured data objects.
// This is separated from asset construction for a number of reasons, mainly that data parsing has
// several quirky parses, and there are a lot of fields where, due to platform-specific byte
// swaps, we know the size of the value but don't know what it means.
namespace MTropolis {

class PlugIn;

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

enum TextAlignmentCode {
	kTextAlignmentCodeLeft = 0,
	kTextAlignmentCodeCenter = 1,
	kTextAlignmentCodeRight = 0xffff,
};


namespace DataObjectTypes {

enum DataObjectType {
	kUnknown								= 0,

	kProjectLabelMap						= 0x22,
	kProjectCatalog							= 0x3e8,
	kStreamHeader							= 0x3e9,
	kProjectHeader							= 0x3ea,
	kPresentationSettings					= 0x3ec,

	kAssetCatalog							= 0xd,
	kGlobalObjectInfo						= 0x17,
	kUnknown19								= 0x19,

	kProjectStructuralDef					= 0x2,
	kSectionStructuralDef					= 0x3,
	kSubsectionStructuralDef				= 0x21,

	kGraphicElement							= 0x8,
	kMovieElement							= 0x5,
	kMToonElement							= 0x6,		// NYI
	kImageElement							= 0x7,
	kSoundElement							= 0xa,
	kTextLabelElement						= 0x15,

	kAliasModifier							= 0x27,
	kChangeSceneModifier					= 0x136,
	kReturnModifier							= 0x140,	// NYI
	kSoundEffectModifier					= 0x1a4,
	kDragMotionModifier						= 0x208,
	kPathMotionModifierV1					= 0x21c,	// NYI - Obsolete version
	kPathMotionModifierV2					= 0x21b,
	kVectorMotionModifier					= 0x226,
	kSceneTransitionModifier				= 0x26c,
	kElementTransitionModifier				= 0x276,
	kSharedSceneModifier					= 0x29a,	// NYI
	kIfMessengerModifier					= 0x2bc,
	kBehaviorModifier						= 0x2c6,
	kMessengerModifier						= 0x2da,
	kSetModifier							= 0x2df,
	kTimerMessengerModifier					= 0x2e4,
	kCollisionDetectionMessengerModifier	= 0x2ee,
	kBoundaryDetectionMessengerModifier		= 0x2f8,
	kKeyboardMessengerModifier				= 0x302,
	kTextStyleModifier						= 0x32a,
	kGraphicModifier						= 0x334,
	kImageEffectModifier					= 0x384,	// NYI
	kMiniscriptModifier						= 0x3c0,
	kCursorModifierV1						= 0x3ca,	// NYI - Obsolete version
	kGradientModifier						= 0x4b0,	// NYI
	kColorTableModifier						= 0x4c4,	// NYI
	kSaveAndRestoreModifier					= 0x4d8,

	kCompoundVariableModifier				= 0x2c7,
	kBooleanVariableModifier				= 0x321,
	kIntegerVariableModifier				= 0x322,
	kIntegerRangeVariableModifier			= 0x324,
	kVectorVariableModifier					= 0x327,
	kPointVariableModifier					= 0x326,
	kFloatingPointVariableModifier			= 0x328,
	kStringVariableModifier					= 0x329,
	kDebris									= 0xfffffffe,	// Deleted modifier in alias list
	kPlugInModifier							= 0xffffffff,

	kMovieAsset								= 0x10,
	kAudioAsset								= 0x11,
	kColorTableAsset						= 0x1e,
	kImageAsset								= 0xe,
	kMToonAsset								= 0xf,
	kTextAsset								= 0x1f,

	kAssetDataChunk							= 0xffff,
};

bool isValidSceneRootElement(DataObjectType type);
bool isVisualElement(DataObjectType type);
bool isNonVisualElement(DataObjectType type);
bool isStructural(DataObjectType type);
bool isElement(DataObjectType type);
bool isModifier(DataObjectType type);
bool isAsset(DataObjectType type);

} // End of namespace DataObjectTypes

namespace StructuralFlags {
	enum StructuralFlags {
		kHasModifiers = 0x1,
		kHasChildren = 0x4,
		kNoMoreSiblings = 0x8,
	};
} // End of namespace StructuralFlags

class DataReader {
public:
	DataReader(int64 globalPosition, Common::SeekableReadStreamEndian &stream, ProjectFormat projectFormat);

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
	bool readPlatformFloat(Common::XPFloat &value);

	bool read(void *dest, size_t size);

	// Reads a terminated string where "length" is the number of characters including a null terminator
	bool readTerminatedStr(Common::String &value, size_t length);

	bool readNonTerminatedStr(Common::String &value, size_t length);

	template<size_t TSize>
	bool readChars(char (&arr)[TSize]);

	template<size_t TSize>
	bool readBytes(uint8 (&arr)[TSize]);

	bool seek(int64 pos);
	bool skip(size_t count);

	int64 tell() const;
	inline int64 tellGlobal() const { return _globalPosition + tell(); }

	ProjectFormat getProjectFormat() const;
	bool isBigEndian() const;

private:
	bool checkErrorAndReset();

	Common::SeekableReadStreamEndian &_stream;
	ProjectFormat _projectFormat;
	int64 _globalPosition;
};

struct Rect {
	bool load(DataReader &reader);
	static Rect createDefault();

	bool toScummVMRect(Common::Rect &outRect) const;
	bool toScummVMRectUnchecked(Common::Rect &outRect) const;

	int16 top;
	int16 left;
	int16 bottom;
	int16 right;
};

struct Point {
	bool load(DataReader &reader);
	bool toScummVMPoint(Common::Point &outPoint) const;

	static Point createDefault();

	int16 x;
	int16 y;
};

struct Event {
	bool load(DataReader &reader);
	static Event createDefault();

	uint32 eventID;
	uint32 eventInfo;
};

struct ColorRGB16 {
	ColorRGB16();

	bool load(DataReader &reader);

	uint16 red;
	uint16 green;
	uint16 blue;
};

struct IntRange {
	bool load(DataReader &reader);
	static IntRange createDefault();

	int32 min;
	int32 max;
};

struct XPFloatVector {
	bool load(DataReader &reader);

	Common::XPFloat angleRadians;
	Common::XPFloat magnitude;
};

struct XPFloatPOD {
	uint16 signAndExponent;
	uint64 mantissa;

	bool load(DataReader &reader);
	Common::XPFloat toXPFloat() const;
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
// InternalTypeTaggedValue is always 46 bytes in size and stores string data elsewhere in the containing structure.
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
		XPFloatPOD asFloat;
		int32 asInteger;
		IntRange asIntegerRange;
		VariableReference asVariableReference;
		Label asLabel;
		Point asPoint;
	};

	InternalTypeTaggedValue();

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
		XPFloatPOD asFloat;
		uint16 asBoolean;
		Event asEvent;
		Label asLabel;
		uint32 asVarRefGUID;
	};

	PlugInTypeTaggedValue();

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

struct ProjectHeader : public DataObject {
	ProjectHeader();

	uint32 persistFlags;
	uint32 sizeIncludingTag;
	uint16 unknown1;
	uint32 catalogFilePosition;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct ProjectLabelMap : public DataObject {
	ProjectLabelMap();
	~ProjectLabelMap();

	struct LabelTree {
		LabelTree();
		~LabelTree();

		enum {
			kExpandedInEditor = 0x80000000,
		};

		uint32 nameLength;
		uint32 isGroup;
		uint32 id;
		uint32 unknown1;
		uint32 flags;

		Common::String name;

		uint32 numChildren;
		LabelTree *children;
	};

	struct SuperGroup {
		SuperGroup();
		~SuperGroup();

		uint32 nameLength;
		uint32 id;
		uint32 unknown2;
		Common::String name;

		uint32 numChildren;
		LabelTree *tree;
	};

	uint32 persistFlags;
	uint32 unknown1; // Always 0x16
	uint32 numSuperGroups;
	uint32 nextAvailableID;

	SuperGroup *superGroups;

private:
	static DataReadErrorCode loadSuperGroup(SuperGroup &sg, DataReader &reader);
	static DataReadErrorCode loadLabelTree(LabelTree &lt, DataReader &reader);

	DataReadErrorCode load(DataReader &reader) override;
};

struct PresentationSettings : public DataObject {
	PresentationSettings();

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

	struct AssetInfoRev4Fields {
		AssetInfoRev4Fields();

		uint32 assetType;
		uint32 flags2;
	};

	struct AssetInfo {
		AssetInfo();

		uint32 flags1;
		uint16 nameLength;
		uint16 alwaysZero;
		uint32 unknown1;	 // Possibly scene ID
		uint32 filePosition; // Contains a static value in Obsidian

		AssetInfoRev4Fields rev4Fields;

		Common::String name;
	};

	AssetCatalog();

	uint32 persistFlags;
	uint32 totalNameSizePlus22;
	uint8 unknown1[4];
	uint32 numAssets;
	bool haveRev4Fields;

	Common::Array<AssetInfo> assets;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct Unknown19 : public DataObject {
	Unknown19();

	uint32 persistFlags;
	uint32 sizeIncludingTag;
	uint8 unknown1[2];

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct StructuralDef : public DataObject {
	StructuralDef();

	uint32 structuralFlags;
};

struct ProjectStructuralDef : public DataObject {
	ProjectStructuralDef();

	uint32 unknown1; // Seems to always be 0x16 or 0x9
	uint32 sizeIncludingTag;
	uint32 guid;
	uint32 otherFlags;
	uint16 lengthOfName;

	Common::String name;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct SectionStructuralDef : public StructuralDef {
	SectionStructuralDef();

	uint32 sizeIncludingTag;
	uint32 guid;
	uint16 lengthOfName;
	uint32 otherFlags;
	uint16 unknown4;
	uint16 sectionID;
	uint32 segmentID;

	Common::String name;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct SubsectionStructuralDef : public StructuralDef {
	SubsectionStructuralDef();

	uint32 structuralFlags;
	uint32 sizeIncludingTag;
	uint32 guid;
	uint16 lengthOfName;
	uint32 otherFlags;
	uint16 sectionID;

	Common::String name;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

namespace ElementFlags {

enum ElementFlags {
	kNotDirectToScreen	= 0x00001000,
	kHidden				= 0x00008000,
	kPaused				= 0x00010000,
	kExpandedInEditor	= 0x00800000,
	kCacheBitmap		= 0x02000000,
	kSelectedInEditor	= 0x10000000,
};

} // End of namespace ElementFlags

namespace AnimationFlags {

enum AnimationFlags {
	kAlternate			= 0x10000000,
	kLoop				= 0x08000000,
	kPlayEveryFrame		= 0x02000000,
};

} // End of namespace AnimationFlags

namespace SceneTransitionTypes {

enum SceneTransitionType {
	kNone = 0,
	kPatternDissolve = 0x0406,
	kRandomDissolve = 0x0410, // No steps
	kFade = 0x041a,
	kSlide = 0x03e8, // Directional
	kPush = 0x03f2,  // Directional
	kZoom = 0x03fc,
	kWipe = 0x0424, // Directional
};

} // End of namespace SceneTransitionTypes

namespace SceneTransitionDirections {

enum SceneTransitionDirection {
	kUp = 0x384,
	kDown = 0x385,
	kLeft = 0x386,
	kRight = 0x387,
};

} // End of namespace SceneTransitionDirections

struct GraphicElement : public StructuralDef {
	GraphicElement();

	// Possible element flags: NotDirectToScreen, CacheBitmap, Hidden
	uint32 sizeIncludingTag;
	uint32 guid;
	uint16 lengthOfName;
	uint32 elementFlags;
	uint16 layer;
	uint16 sectionID;
	Rect rect1;
	Rect rect2;
	uint32 streamLocator; // 1-based index, sometimes observed with 0x10000000 flag set, not sure of the meaning
	uint8 unknown11[4];

	Common::String name;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct ImageElement : public StructuralDef {
	ImageElement();

	// Possible element flags: NotDirectToScreen, CacheBitmap, Hidden
	uint32 sizeIncludingTag;
	uint32 guid;
	uint16 lengthOfName;
	uint32 elementFlags;
	uint16 layer;
	uint16 sectionID;
	Rect rect1;
	Rect rect2;
	uint32 imageAssetID;
	uint32 streamLocator;
	uint8 unknown7[4];

	Common::String name;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct TextLabelElement : public StructuralDef {
	TextLabelElement();

	// Possible element flags: NotDirectToScreen, CacheBitmap, Hidden
	uint32 sizeIncludingTag;
	uint32 guid;
	uint16 lengthOfName;
	uint32 elementFlags;
	uint16 layer;
	uint16 sectionID;
	Rect rect1;
	Rect rect2;
	uint32 assetID;

	struct MacPart {
		uint8 unknown2[30];
	};

	struct WinPart {
		uint8 unknown3[2];
		uint8 unknown4[8];
	};

	union PlatformPart {
		MacPart mac;
		WinPart win;
	};

	bool haveMacPart;
	bool haveWinPart;
	PlatformPart platform;

	Common::String name;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct SoundElement : public StructuralDef {
	enum SoundFlags {
		kPaused = 0x40000000,
		kLoop = 0x80000000,
	};

	SoundElement();

	// Possible element flags: Loop, Paused
	uint32 sizeIncludingTag;
	uint32 guid;
	uint16 lengthOfName;
	uint32 elementFlags;
	uint32 soundFlags;
	uint16 unknown2;
	uint8 unknown3[2];
	uint16 rightVolume;
	uint16 leftVolume;
	int16 balance;
	uint32 assetID;
	uint8 unknown5[8];

	Common::String name;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct MovieElement : public StructuralDef {
	MovieElement();

	// Possible flags: NotDirectToScreen, CacheBitmap, Hidden, Loop, Loop + Alternate, Paused
	uint32 sizeIncludingTag;
	uint32 guid;
	uint16 lengthOfName;
	uint32 elementFlags;
	uint16 layer;
	uint8 unknown3[44];
	uint16 sectionID;
	uint8 unknown5[2];
	Rect rect1;
	Rect rect2;
	uint32 assetID;
	uint32 unknown7;
	uint16 volume;
	uint32 animationFlags;
	uint8 unknown10[4];
	uint8 unknown11[4];
	uint32 streamLocator;
	uint8 unknown13[4];

	Common::String name;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct MToonElement : public StructuralDef {
	MToonElement();

	// Possible flags: NotDirectToScreen, CacheBitmap, Hidden, Loop, Paused, PlayEveryFrame (inverted as "Maintain Rate")
	uint32 sizeIncludingTag;
	uint32 guid;
	uint16 lengthOfName;
	uint32 elementFlags;
	uint16 layer;
	uint32 animationFlags;
	uint8 unknown4[4];
	uint16 sectionID;
	Rect rect1;
	Rect rect2;
	uint32 assetID;
	uint32 rateTimes100000;
	uint32 streamLocator;
	uint32 unknown6;

	Common::String name;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct GlobalObjectInfo : public DataObject {
	GlobalObjectInfo();

	uint32 persistFlags;
	uint32 sizeIncludingTag;
	uint16 numGlobalModifiers;
	uint8 unknown1[4];

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

class ProjectCatalog : public DataObject {
public:
	struct StreamDesc {
		StreamDesc();

		char streamType[25];
		uint16 segmentIndexPlusOne;
		uint32 size;
		uint32 pos;
	};

	struct SegmentDesc {
		SegmentDesc();

		uint32 segmentID;
		Common::String label;
		Common::String exportedPath;
	};

	ProjectCatalog();

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
	StreamHeader();

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
	enum BehaviorFlags {
		kBehaviorFlagSwitchable = 1,
	};

	BehaviorModifier();

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
	uint32 behaviorFlags;
	Event enableWhen;
	Event disableWhen;
	uint8 unknown7[2];

	Common::String name;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct MiniscriptProgram {
	struct LocalRef {
		LocalRef();

		uint32 guid;
		uint8 lengthOfName;
		uint8 unknown2;

		Common::String name;
	};

	struct Attribute {
		Attribute();

		uint8 lengthOfName;
		uint8 unknown3;

		Common::String name;
	};

	MiniscriptProgram();

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
	TypicalModifierHeader();

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
	MiniscriptModifier();

	TypicalModifierHeader modHeader;
	Event enableWhen;
	uint8 unknown6[11];
	uint8 unknown7;

	MiniscriptProgram program;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};


struct SaveAndRestoreModifier : public DataObject {
	SaveAndRestoreModifier();

	TypicalModifierHeader modHeader;
	uint8 unknown1[4];
	Event saveWhen;
	Event restoreWhen;
	InternalTypeTaggedValue saveOrRestoreValue;

	uint8 unknown5[8];

	uint8 lengthOfFilePath;
	uint8 lengthOfFileName;
	uint8 lengthOfVariableName;
	uint8 lengthOfVariableString;

	Common::String varName;
	Common::String varString;
	Common::String filePath;
	Common::String fileName;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

enum MessageFlags {
	kMessageFlagNoRelay = 0x20000000,
	kMessageFlagNoCascade = 0x40000000,
	kMessageFlagNoImmediate = 0x80000000,
};


struct MessengerModifier : public DataObject {
	MessengerModifier();

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
	SetModifier();

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

struct AliasModifier : public DataObject {
	AliasModifier();

	uint32 modifierFlags;
	uint32 sizeIncludingTag;
	uint16 aliasIndexPlusOne;
	uint32 unknown1;
	uint32 unknown2;
	uint16 lengthOfName;
	uint32 guid;
	Point editorLayoutPosition;

	Common::String name;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct ChangeSceneModifier : public DataObject {
	enum ChangeSceneFlags {
		kChangeSceneFlagNextScene			= 0x80000000,
		kChangeSceneFlagPrevScene			= 0x40000000,
		kChangeSceneFlagSpecificScene		= 0x20000000,
		kChangeSceneFlagAddToReturnList		= 0x10000000,
		kChangeSceneFlagAddToDestList		= 0x08000000,
		kChangeSceneFlagWrapAround			= 0x04000000,
	};

	ChangeSceneModifier();

	TypicalModifierHeader modHeader;
	uint32 changeSceneFlags;
	Event executeWhen;
	uint32 targetSectionGUID;
	uint32 targetSubsectionGUID;
	uint32 targetSceneGUID;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct SoundEffectModifier : public DataObject {
	SoundEffectModifier();

	static const uint32 kSpecialAssetIDSystemBeep = 0xffffffffu;

	TypicalModifierHeader modHeader;
	uint8 unknown1[4];
	Event executeWhen;
	Event terminateWhen;
	uint32 unknown2;
	uint8 unknown3[4];
	uint32 assetID;
	uint8 unknown5[4];

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct PathMotionModifierV2 : public DataObject {
	struct PointDef {
		PointDef();

		enum FrameFlags {
			kFrameFlagPlaySequentially = 1,
		};

		Point point;
		uint32 frame;
		uint32 frameFlags;
		uint32 messageFlags;
		Event send;
		uint16 unknown11;
		uint32 destination;
		uint8 unknown13[10];
		InternalTypeTaggedValue with;
		uint8 withSourceLength;
		uint8 withStringLength;

		Common::String withSource;
		Common::String withString;

		bool load(DataReader &reader);
	};

	enum Flags {
		kFlagReverse = 0x00100000,
		kFlagLoop = 0x10000000,
		kFlagAlternate = 0x02000000,
		kFlagStartAtBeginning = 0x08000000,
	};

	PathMotionModifierV2();

	TypicalModifierHeader modHeader;
	uint32 flags;

	Event executeWhen;
	Event terminateWhen;

	uint8 unknown2[2];
	uint16 numPoints;
	uint8 unknown3[4];
	uint32 frameDurationTimes10Million;
	uint8 unknown5[4];
	uint32 unknown6;

	Common::Array<PointDef> points;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct DragMotionModifier : public DataObject {
	DragMotionModifier();

	TypicalModifierHeader modHeader;

	Event enableWhen;
	Event disableWhen;

	struct WinPart {
		uint8 unknown2;
		uint8 constrainHorizontal;
		uint8 constrainVertical;
		uint8 constrainToParent;
	};

	struct MacPart {
		uint8 flags;
		uint8 unknown3;

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
	uint16 unknown1;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct VectorMotionModifier : public DataObject {
	VectorMotionModifier();

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
	SceneTransitionModifier();

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
	ElementTransitionModifier();

	enum TransitionType {
		kTransitionTypeRectangularIris = 0x03e8,
		kTransitionTypeOvalIris = 0x03f2,
		kTransitionTypeZoom = 0x044c,
		kTransitionTypeFade = 0x2328,
	};

	enum RevealType {
		kRevealTypeReveal = 0,
		kRevealTypeConceal = 1,
	};

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
	IfMessengerModifier();

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
	TimerMessengerModifier();

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
	BoundaryDetectionMessengerModifier();

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
	CollisionDetectionMessengerModifier();

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

	KeyboardMessengerModifier();

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
	TextStyleModifier();

	TypicalModifierHeader modHeader;

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
	uint16 lengthOfFontFamilyName;

	Common::String fontFamilyName;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct GraphicModifier : public DataObject {
	GraphicModifier();

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
	CompoundVariableModifier();

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
	BooleanVariableModifier();

	TypicalModifierHeader modHeader;
	uint8 value;
	uint8 unknown5;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct IntegerVariableModifier : public DataObject {
	IntegerVariableModifier();

	TypicalModifierHeader modHeader;
	uint8 unknown1[4];
	int32 value;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct IntegerRangeVariableModifier : public DataObject {
	IntegerRangeVariableModifier();

	TypicalModifierHeader modHeader;
	uint8 unknown1[4];
	IntRange range;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct VectorVariableModifier : public DataObject {
	VectorVariableModifier();

	TypicalModifierHeader modHeader;
	uint8 unknown1[4];
	XPFloatVector vector;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct PointVariableModifier : public DataObject {
	PointVariableModifier();

	TypicalModifierHeader modHeader;

	uint8 unknown5[4];
	Point value;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct FloatingPointVariableModifier : public DataObject {
	FloatingPointVariableModifier();

	TypicalModifierHeader modHeader;
	uint8 unknown1[4];
	Common::XPFloat value;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct StringVariableModifier : public DataObject {
	StringVariableModifier();

	TypicalModifierHeader modHeader;
	uint32 lengthOfString;
	uint8 unknown1[4];
	Common::String value;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct PlugInModifierData {
	virtual ~PlugInModifierData();
	virtual DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) = 0;
};

struct PlugInModifier : public DataObject {
	PlugInModifier();

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
	Debris();

	uint32 persistFlags;
	uint32 sizeIncludingTag;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct ColorTableAsset : public DataObject {
	ColorTableAsset();

	uint32 persistFlags;
	uint32 sizeIncludingTag;
	uint8 unknown1[4];
	uint32 assetID;
	uint32 unknown2; // Usually zero-fill but sometimes contains 0xb

	ColorRGB16 colors[256];

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct MovieAsset : public DataObject {
	struct MacPart {
		uint8 unknown5_1[66];
		uint8 unknown6[12];
	};

	struct WinPart {
		uint8 unknown3_1[32];
		uint8 unknown4[12];
		uint8 unknown7[12];
	};

	union PlatformPart {
		MacPart mac;
		WinPart win;
	};

	MovieAsset();

	uint32 persistFlags;
	uint32 assetAndDataCombinedSize;
	uint8 unknown1[4];
	uint32 assetID;
	uint8 unknown1_1[4];
	uint16 extFileNameLength;

	uint32 movieDataPos;
	uint32 moovAtomPos;
	uint32 movieDataSize;

	bool haveMacPart;
	bool haveWinPart;
	PlatformPart platform;

	Common::String extFileName;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct AudioAsset : public DataObject {
	struct MacPart {
		uint8 unknown4[4];
		uint8 unknown5[5];
		uint8 unknown6[3];
		uint8 unknown8[20];
	};

	struct WinPart {
		uint8 unknown9[3];
		uint8 unknown10[3];
		uint8 unknown11[18];
		uint8 unknown12_1[2];
	};

	union PlatformPart {
		MacPart mac;
		WinPart win;
	};

	struct CuePoint {
		uint8 unknown13[2];
		uint32 unknown14;
		uint32 position;
		uint32 cuePointID;
	};

	AudioAsset();

	uint32 persistFlags;
	uint32 assetAndDataCombinedSize;
	uint8 unknown2[4];
	uint32 assetID;
	uint8 unknown3[20];
	uint16 sampleRate1;
	uint8 bitsPerSample;
	uint8 encoding1;
	uint8 channels;
	uint8 codedDuration[4];
	uint16 sampleRate2;
	uint32 cuePointDataSize;
	uint16 numCuePoints;
	uint8 unknown14[4];
	uint32 filePosition;
	uint32 size;

	Common::Array<CuePoint> cuePoints;

	bool haveMacPart;
	bool haveWinPart;
	bool isBigEndian;
	PlatformPart platform;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct ImageAsset : public DataObject {
	struct MacPart {
		uint8 unknown7[44];
	};

	struct WinPart {
		uint8 unknown8[10];
	};

	union PlatformPart {
		WinPart win;
		MacPart mac;
	};

	ImageAsset();

	uint32 persistFlags;
	uint32 unknown1;
	uint8 unknown2[4];
	uint32 assetID;
	uint32 unknown3;

	Rect rect1;
	uint32 hdpiFixed;
	uint32 vdpiFixed;
	uint16 bitsPerPixel;
	uint8 unknown4[2];
	uint8 unknown5[4];
	uint8 unknown6[8];
	Rect rect2;
	uint32 filePosition;
	uint32 size;

	bool haveMacPart;
	bool haveWinPart;
	PlatformPart platform;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct MToonAsset : public DataObject {
	struct MacPart {
		uint8 unknown10[88];
	};

	struct WinPart {
		uint8 unknown11[54];
	};

	union PlatformUnion {
		MacPart mac;
		WinPart win;
	};

	struct FrameDef {
		struct MacPart {
			uint8 unknown17[4];
		};

		struct WinPart {
			uint8 unknown18[2];
		};

		union PlatformUnion {
			MacPart mac;
			WinPart win;
		};

		FrameDef();

		uint8 unknown12[4];
		Rect rect1;
		uint32 dataOffset;
		uint8 unknown13[2];
		uint32 compressedSize;
		uint8 unknown14;
		uint8 keyframeFlag;
		uint8 platformBit;
		uint8 unknown15;
		Rect rect2;
		uint32 hdpiFixed;
		uint32 vdpiFixed;
		uint16 bitsPerPixel;
		uint32 unknown16;
		uint16 decompressedBytesPerRow;
		uint32 decompressedSize;

		PlatformUnion platform;
	};

	struct FrameRangeDef {
		FrameRangeDef();

		uint32 startFrame;
		uint32 endFrame;
		uint8 lengthOfName;
		uint8 unknown14;

		Common::String name; // Null terminated
	};

	enum {
		kEncodingFlag_TemporalCompression = 0x80,
		kEncodingFlag_HasRanges = 0x20000000,
		kEncodingFlag_Trimming = 0x08,
	};

	struct FrameRangePart {
		FrameRangePart();

		uint32 tag;
		uint32 sizeIncludingTag;

		uint32 numFrameRanges;
		Common::Array<FrameRangeDef> frameRanges;
	};

	MToonAsset();

	uint32 marker;
	uint8 unknown1[8];
	uint32 assetID;

	bool haveMacPart;
	bool haveWinPart;
	PlatformUnion platform;

	uint32 frameDataPosition;
	uint32 sizeOfFrameData;

	// mToon data
	uint32 mtoonHeader[2];
	uint16 version;
	uint8 unknown2[4];
	uint32 encodingFlags;
	Rect rect;

	uint16 numFrames;
	uint8 unknown3[14];
	uint16 bitsPerPixel;
	uint32 codecID;
	uint8 unknown4_1[8];
	uint32 codecDataSize;
	uint8 unknown4_2[4];

	Common::Array<FrameDef> frames;

	// Codec data appears to be a 16-byte header followed by a QuickTime sample description
	// Note that the sample description is partly useless because frames can have different sizes
	// and the sample desc is only for the last frame!
	//
	// The 16-byte header is:
	// uint32be size of codec data
	// char[4] codec ID
	// byte[8] unknown (all 0?)
	Common::Array<uint8> codecData;

	FrameRangePart frameRangesPart;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct TextAsset : public DataObject {
	struct MacFormattingSpan {
		uint8 unknown9[2];
		uint16 spanStart;
		uint8 unknown10[4];
		uint16 fontID;
		uint8 fontFlags;
		uint8 unknown11[1];
		uint16 size;
		uint8 unknown12[6];
	};

	struct MacPart {
		uint8 unknown3[44];
	};

	struct WinPart {
		uint8 unknown4[10];
	};

	union PlatformPart {
		MacPart mac;
		WinPart win;
	};

	TextAsset();

	uint32 persistFlags;
	uint32 sizeIncludingTag;
	uint32 unknown1;
	uint32 assetID;
	uint32 unknown2;
	Rect bitmapRect;
	uint32 hdpi;
	uint32 vdpi;
	uint16 unknown5;
	uint8 pitchBigEndian[2];
	uint32 unknown6;

	uint32 bitmapSize;
	uint8 unknown7[20];
	uint32 textSize;
	uint8 unknown8[8];
	uint16 alignment;
	uint16 isBitmap;

	bool haveMacPart;
	bool haveWinPart;
	PlatformPart platform;

	Common::String text;
	Common::Array<uint8> bitmapData;

	bool isBottomUp;

	Common::Array<MacFormattingSpan> macFormattingSpans;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct AssetDataChunk : public DataObject {
	AssetDataChunk();

	uint32 unknown1;
	uint32 sizeIncludingTag;
	int64 filePosition;

protected:
	DataReadErrorCode load(DataReader &reader) override;
};

struct IPlugInModifierDataFactory : public IInterfaceBase {
	virtual Common::SharedPtr<Data::PlugInModifierData> createModifierData() const = 0;
	virtual PlugIn &getPlugIn() const = 0;
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
