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
class Modifier;

struct SegmentDescription {
	int volumeID;
	Common::String filePath;
};

class ProjectDescription {

public:
	ProjectDescription(); 
	~ProjectDescription();

	void addSegment(int volumeID, const char *filePath);
	const Common::Array<SegmentDescription> &getSegments() const;

private:
	Common::Array<SegmentDescription> _segments;
};

struct VolumeState {
	Common::String name;
	int volumeID;
	bool isMounted;
};

struct Event {
	Event();

	uint32 eventType;
	uint32 eventInfo;

	bool load(const Data::Event &data);
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
	virtual Common::Array<Common::SharedPtr<Modifier> > &getModifiers() = 0;
	const Common::Array<Common::SharedPtr<Modifier> > &getModifiers() const;
};

class SimpleModifierContainer : public IModifierContainer {

	Common::Array<Common::SharedPtr<Modifier> > &getModifiers() override;

private:
	Common::Array<Common::SharedPtr<Modifier> > _modifiers;
};

class Structural : public IModifierContainer {

public:
	virtual ~Structural();

	const Common::Array<Common::SharedPtr<Structural> > &getChildren() const;

	Common::Array<Common::SharedPtr<Modifier> > &getModifiers() override;

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

class Project : public Structural {

public:
	Project();
	~Project();

	void loadFromDescription(const ProjectDescription &desc);

private:
	struct Segment {
		SegmentDescription desc;
		Common::SharedPtr<Common::SeekableReadStream> stream;
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
};

class Section : public Structural {
};

class Subsection : public Structural {
};

class Scene : public Structural {
};

class Modifier {
public:
	virtual ~Modifier();
};

void loadRuntimeContextualObject(ChildLoaderStack &stack, const Data::DataObject &dataObject);

} // End of namespace MTropolis

#endif
