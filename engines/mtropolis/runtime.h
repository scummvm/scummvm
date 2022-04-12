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

#ifndef MTROPOLIS_RUNTIME_H
#define MTROPOLIS_RUNTIME_H

#include "common/array.h"
#include "common/platform.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

#include "mtropolis/data.h"
#include "mtropolis/vthread.h"

namespace MTropolis {

class Project;
class PlugIn;
class Modifier;
struct IModifierFactory;
struct IPlugInModifierFactory;
struct IPlugInModifierFactoryAndDataFactory;
struct ModifierLoaderContext;

struct Point16 {
	int16 x;
	int16 y;

	bool load(const Data::Point &point);

	inline bool operator==(const Point16& other) const {
		return x == other.x && y == other.y;
	}

	inline bool operator!=(const Point16 &other) const {
		return !((*this) == other);
	}
};

struct Rect16 {
	int16 top;
	int16 left;
	int16 bottom;
	int16 right;

	bool load(const Data::Rect &rect);

	inline bool operator==(const Rect16 &other) const {
		return top == other.top && left == other.left && bottom == other.bottom && right == other.right;
	}

	inline bool operator!=(const Rect16 &other) const {
		return !((*this) == other);
	}
};

struct IntRange {
	int32 min;
	int32 max;

	bool load(const Data::IntRange &range);

	inline bool operator==(const IntRange &other) const {
		return min == other.min && max == other.max;
	}

	inline bool operator!=(const IntRange &other) const {
		return !((*this) == other);
	}
};

struct Label {
	uint32 superGroupID;
	uint32 id;

	bool load(const Data::Label &label);

	inline bool operator==(const Label &other) const {
		return superGroupID == other.superGroupID && id == other.id;
	}

	inline bool operator!=(const Label &other) const {
		return !((*this) == other);
	}
};

struct Event {
	uint32 eventType;
	uint32 eventInfo;

	bool load(const Data::Event &data);

	inline bool operator==(const Event &other) const {
		return eventType == other.eventType && eventInfo == other.eventInfo;
	}

	inline bool operator!=(const Event &other) const {
		return !((*this) == other);
	}
};

struct VarReference {
	uint32 guid;
	Common::String *source;

	inline bool operator==(const VarReference &other) const {
		return guid == other.guid && (*source) == (*other.source);
	}

	inline bool operator!=(const VarReference &other) const {
		return !((*this) == other);
	}
};

struct AngleMagVector {
	double angleRadians;
	double magnitude;

	inline bool operator==(const AngleMagVector &other) const {
		return angleRadians == other.angleRadians && magnitude == other.magnitude;
	}

	inline bool operator!=(const AngleMagVector &other) const {
		return !((*this) == other);
	}
};

struct ColorRGB8 {
	uint8 r;
	uint8 g;
	uint8 b;

	bool load(const Data::ColorRGB16 &color);
};

struct MessageFlags {
	MessageFlags();

	bool relay : 1;
	bool cascade : 1;
	bool immediate : 1;
};

struct DynamicValue {
	enum Type {
		kTypeNull,
		kTypeInteger,
		kTypeFloat,
		kTypePoint,
		KTypeIntegerRange,
		kTypeBoolean,
		kTypeVector,
		kTypeLabel,
		kTypeEvent,
		kTypeVariableReference,
		kTypeIncomingData,
		kTypeString,
	};

	DynamicValue();
	DynamicValue(const DynamicValue &other);
	~DynamicValue();

	bool load(const Data::InternalTypeTaggedValue &data, const Common::String &varSource, const Common::String &varString);
	bool load(const Data::PlugInTypeTaggedValue &data);

	Type getType() const;

	const int32 &getInt() const;
	const double &getFloat() const;
	const Point16 &getPoint() const;
	const IntRange &getIntRange() const;
	const AngleMagVector &getVector() const;
	const Label &getLabel() const;
	const Event &getEvent() const;
	const VarReference &getVarReference() const;
	const Common::String &getString() const;
	const bool &getBool() const;

	DynamicValue &operator=(const DynamicValue &other);

	bool operator==(const DynamicValue &other) const;
	inline bool operator!=(const DynamicValue& other) const {
		return !((*this) == other);
	}

private:
	union ValueUnion {
		double asFloat;
		int32 asInt;
		IntRange asIntRange;
		AngleMagVector asVector;
		Label asLabel;
		VarReference asVarReference;
		Event asEvent;
		Point16 asPoint;
		bool asBool;
	};

	void clear();
	void initFromOther(const DynamicValue &other);

	Type _type;
	ValueUnion _value;
	Common::String _str;
};

struct MessengerSendSpec {
	MessengerSendSpec();
	bool load(const Data::Event &dataEvent, uint32 dataMessageFlags, const Data::InternalTypeTaggedValue &dataLocator, const Common::String &dataWithSource, const Common::String &dataWithString, uint32 dataDestination);
	bool load(const Data::PlugInTypeTaggedValue &dataEvent, const MessageFlags &dataMessageFlags, const Data::PlugInTypeTaggedValue &dataWith, uint32 dataDestination);

	Event send;
	MessageFlags messageFlags;
	DynamicValue with;
	uint32 destination; // May be a MessageDestination or GUID
};

enum MessageDestination {
	kMessageDestSharedScene = 0x65,
	kMessageDestScene = 0x66,
	kMessageDestSection = 0x67,
	kMessageDestProject = 0x68,
	kMessageDestActiveScene = 0x69,
	kMessageDestElementsParent = 0x6a,
	kMessageDestChildren = 0x6b,
	kMessageDestModifiersParent = 0x6c,
	kMessageDestSubsection = 0x6d,

	kMessageDestElement = 0xc9,
	kMessageDestSourcesParent = 0xcf,

	kMessageDestBehavior = 0xd4,
	kMessageDestNextElement = 0xd1,
	kMessageDestPrevElement = 0xd2,
	kMessageDestBehaviorsParent = 0xd3,
};

struct SegmentDescription {
	int volumeID;
	Common::String filePath;
	Common::SeekableReadStream *stream;
};

struct IPlugInModifierRegistrar {
	virtual void registerPlugInModifier(const char *name, const Data::IPlugInModifierDataFactory *loader, const IPlugInModifierFactory *factory) = 0;
	void registerPlugInModifier(const char *name, const IPlugInModifierFactoryAndDataFactory *loaderFactory);
};

class PlugIn {
public:
	virtual ~PlugIn();

	virtual void registerModifiers(IPlugInModifierRegistrar *registrar) const = 0;
};

struct ProjectResources {
	virtual ~ProjectResources();
};

class ProjectDescription {

public:
	ProjectDescription(); 
	~ProjectDescription();

	void addSegment(int volumeID, const char *filePath);
	void addSegment(int volumeID, Common::SeekableReadStream *stream);
	const Common::Array<SegmentDescription> &getSegments() const;

	void addPlugIn(const Common::SharedPtr<PlugIn> &plugIn);
	const Common::Array<Common::SharedPtr<PlugIn> > &getPlugIns() const;

	void setResources(const Common::SharedPtr<ProjectResources> &resources);
	const Common::SharedPtr<ProjectResources> &getResources() const;

private:
	Common::Array<SegmentDescription> _segments;
	Common::Array<Common::SharedPtr<PlugIn> > _plugIns;
	Common::SharedPtr<ProjectResources> _resources;
};

struct VolumeState {
	Common::String name;
	int volumeID;
	bool isMounted;
};

class Runtime {
public:
	Runtime();

	void runFrame();
	void queueProject(const Common::SharedPtr<ProjectDescription> &desc);

	void addVolume(int volumeID, const char *name, bool isMounted);

private:
	Common::Array<VolumeState> _volumes;
	Common::SharedPtr<ProjectDescription> _queuedProjectDesc;
	Common::SharedPtr<Project> _project;
	Common::ScopedPtr<VThread> _vthread;
};

struct IModifierContainer {
	virtual const Common::Array<Common::SharedPtr<Modifier> > &getModifiers() const = 0;
	virtual void appendModifier(const Common::SharedPtr<Modifier> &modifier) = 0;
};

class SimpleModifierContainer : public IModifierContainer {

	const Common::Array<Common::SharedPtr<Modifier> > &getModifiers() const;
	void appendModifier(const Common::SharedPtr<Modifier> &modifier) override;

private:
	Common::Array<Common::SharedPtr<Modifier> > _modifiers;
};

class Structural : public IModifierContainer {

public:
	virtual ~Structural();

	const Common::Array<Common::SharedPtr<Structural> > &getChildren() const;

	const Common::Array<Common::SharedPtr<Modifier> > &getModifiers() const override;
	void appendModifier(const Common::SharedPtr<Modifier> &modifier) override;

private:
	Common::Array<Common::SharedPtr<Structural> > _children;
	Common::Array<Common::SharedPtr<Modifier> > _modifiers;
};

struct ProjectPresentationSettings {
	ProjectPresentationSettings();

	uint16 width;
	uint16 height;
	uint32 bitsPerPixel;
};

class AssetInfo {

public:
	AssetInfo();
	virtual ~AssetInfo();

	void setSegmentIndex(int segmentIndex);
	int getSegmentIndex() const;

private:
	int _segmentIndex;
};

struct ChildLoaderContext {
	enum Type {
		kTypeModifierList,
		kTypeStructuralList,
	};

	union ContainerUnion {
		IModifierContainer *modifierContainer;
	};

	uint remainingCount;
	Type type;
	ContainerUnion containerUnion;
};

struct ChildLoaderStack {
	Common::Array<ChildLoaderContext> contexts;
};

class ProjectPlugInRegistry : public IPlugInModifierRegistrar  {
public:
	ProjectPlugInRegistry();

	void registerPlugInModifier(const char *name, const Data::IPlugInModifierDataFactory *dataFactory, const IPlugInModifierFactory *factory) override;

	const Data::PlugInModifierRegistry &getDataLoaderRegistry() const;
	const IPlugInModifierFactory *findPlugInModifierFactory(const char *name) const;

private:
	Data::PlugInModifierRegistry _dataLoaderRegistry;
	Common::HashMap<Common::String, const IPlugInModifierFactory *> _factoryRegistry;
};

class Project : public Structural {

public:
	Project();
	~Project();

	void loadFromDescription(const ProjectDescription &desc);

private:
	struct Segment {
		SegmentDescription desc;
		Common::SharedPtr<Common::SeekableReadStream> rcStream;
		Common::SeekableReadStream *weakStream;
	};

	enum StreamType {
		kStreamTypeUnknown,

		kStreamTypeAsset,
		kStreamTypeBoot,
		kStreamTypeScene,
	};

	struct StreamDesc {
		StreamType streamType;
		uint16 segmentIndex;
		uint32 size;
		uint32 pos;
	};

	struct AssetDesc {
		uint32 typeCode;
		size_t id;
		Common::String name;

		// If the asset is live, this will be its asset info
		Common::SharedPtr<AssetInfo> assetInfo;
	};

	void openSegmentStream(int segmentIndex);
	void loadBootStream(size_t streamIndex);

	void loadPresentationSettings(const Data::PresentationSettings &presentationSettings);
	void loadAssetCatalog(const Data::AssetCatalog &assetCatalog);
	void loadGlobalObjectInfo(ChildLoaderStack &loaderStack, const Data::GlobalObjectInfo &globalObjectInfo);
	void loadContextualObject(ChildLoaderStack &stack, const Data::DataObject &dataObject);
	Common::SharedPtr<Modifier> loadModifierObject(ModifierLoaderContext &loaderContext, const Data::DataObject &dataObject);

	Common::Array<Segment> _segments;
	Common::Array<StreamDesc> _streams;
	Data::ProjectFormat _projectFormat;
	bool _isBigEndian;

	Common::Array<AssetDesc *> _assetsByID;
	Common::Array<AssetDesc> _realAssets;

	Common::HashMap<Common::String, size_t> _assetNameToID;

	ProjectPresentationSettings _presentationSettings;

	bool _haveGlobalObjectInfo;
	SimpleModifierContainer _globalModifiers;

	ProjectPlugInRegistry _plugInRegistry;

	Common::Array<Common::SharedPtr<PlugIn> > _plugIns;
	Common::SharedPtr<ProjectResources> _resources;
};

class Section : public Structural {
};

class Subsection : public Structural {
};

class Scene : public Structural {
};

struct ModifierFlags {
	ModifierFlags();
	bool load(const uint32 dataModifierFlags);

	bool isLastModifier : 1;
};

class Modifier {
public:
	Modifier();
	virtual ~Modifier();

protected:
	uint32 _guid;
	Common::String _name;
	ModifierFlags _modifierFlags;

	bool loadTypicalHeader(const Data::TypicalModifierHeader &typicalHeader);
};

} // End of namespace MTropolis

#endif
