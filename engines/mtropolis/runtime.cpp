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

#include "mtropolis/runtime.h"
#include "mtropolis/data.h"
#include "mtropolis/vthread.h"
#include "mtropolis/modifier_factory.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/substream.h"


namespace MTropolis {

bool Point16::load(const Data::Point& point) {
	x = point.x;
	y = point.y;

	return true;
}

bool Rect16::load(const Data::Rect& rect) {
	top = rect.top;
	left = rect.left;
	bottom = rect.bottom;
	right = rect.right;

	return true;
}

bool IntRange::load(const Data::IntRange& range) {
	max = range.max;
	min = range.min;

	return true;
}

bool Label::load(const Data::Label& label) {
	id = label.labelID;
	superGroupID = label.superGroupID;

	return true;
}

bool ColorRGB8::load(const Data::ColorRGB16& color) {
	this->r = (color.red * 510 + 1) / 131070;
	this->g = (color.green * 510 + 1) / 131070;
	this->b = (color.blue * 510 + 1) / 131070;

	return true;
}

MessageFlags::MessageFlags() : relay(true), cascade(true), immediate(true) {
}

DynamicValue::DynamicValue() : _type(kTypeNull) {
}

DynamicValue::DynamicValue(const DynamicValue &other) : _type(kTypeNull) {
	initFromOther(other);
}

DynamicValue::~DynamicValue() {
	clear();
}

bool DynamicValue::load(const Data::InternalTypeTaggedValue &data, const Common::String &varSource, const Common::String &varString) {
	switch (data.type) {
	case Data::InternalTypeTaggedValue::kNull:
		_type = kTypeNull;
		break;
	case Data::InternalTypeTaggedValue::kIncomingData:
		_type = kTypeIncomingData;
		break;
	case Data::InternalTypeTaggedValue::kInteger:
		_type = kTypeInteger;
		_value.asInt = data.value.asInteger;
		break;
	case Data::InternalTypeTaggedValue::kString:
		_type = kTypeString;
		_str = varString;
		break;
	case Data::InternalTypeTaggedValue::kPoint:
		_type = kTypePoint;
		if (!_value.asPoint.load(data.value.asPoint))
			return false;
		break;
	case Data::InternalTypeTaggedValue::kIntegerRange:
		_type = KTypeIntegerRange;
		if (!_value.asIntRange.load(data.value.asIntegerRange))
			return false;
		break;
	case Data::InternalTypeTaggedValue::kFloat:
		_type = kTypeFloat;
		_value.asFloat = data.value.asFloat.toDouble();
		break;
	case Data::InternalTypeTaggedValue::kBool:
		_type = kTypeBoolean;
		_value.asBool = (data.value.asBool != 0);
		break;
	case Data::InternalTypeTaggedValue::kVariableReference:
		_type = kTypeVariableReference;
		_value.asVarReference.guid = data.value.asVariableReference.guid;
		_value.asVarReference.source = &_str;
		_str = varSource;
		break;
	case Data::InternalTypeTaggedValue::kLabel:
		_type = kTypeLabel;
		if (!_value.asLabel.load(data.value.asLabel))
			return false;
		break;
	default:
		assert(false);
		return false;
	}

	return true;
}

bool DynamicValue::load(const Data::PlugInTypeTaggedValue& data) {
	switch (data.type) {
	case Data::PlugInTypeTaggedValue::kNull:
		_type = kTypeNull;
		break;
	case Data::PlugInTypeTaggedValue::kIncomingData:
		_type = kTypeIncomingData;
		break;
	case Data::PlugInTypeTaggedValue::kInteger:
		_type = kTypeInteger;
		_value.asInt = data.value.asInt;
		break;
	case Data::PlugInTypeTaggedValue::kIntegerRange:
		_type = KTypeIntegerRange;
		if (!_value.asIntRange.load(data.value.asIntRange))
			return false;
		break;
	case Data::PlugInTypeTaggedValue::kFloat:
		_type = kTypeFloat;
		_value.asFloat = data.value.asFloat.toDouble();
		break;
	case Data::PlugInTypeTaggedValue::kBoolean:
		_type = kTypeBoolean;
		_value.asBool = (data.value.asBoolean != 0);
		break;
	case Data::PlugInTypeTaggedValue::kEvent:
		_type = kTypeEvent;
		if (!_value.asEvent.load(data.value.asEvent))
			return false;
		break;
	case Data::PlugInTypeTaggedValue::kLabel:
		_type = kTypeLabel;
		if (!_value.asLabel.load(data.value.asLabel))
			return false;
		break;
	case Data::PlugInTypeTaggedValue::kString:
		_type = kTypeString;
		_str = data.str;
		break;
	case Data::PlugInTypeTaggedValue::kVariableReference:
		_type = kTypeVariableReference;
		_value.asVarReference.guid = data.value.asVarRefGUID;
		_value.asVarReference.source = &_str;
		_str.clear();	// Extra data doesn't seem to correlate to this
		break;
	default:
		assert(false);
		return false;
	}

	return true;
}


DynamicValue::Type DynamicValue::getType() const {
	return _type;
}

const int32 &DynamicValue::getInt() const {
	assert(_type == kTypeInteger);
	return _value.asInt;
}

const double &DynamicValue::getFloat() const {
	assert(_type == kTypeFloat);
	return _value.asFloat;
}

const Point16 &DynamicValue::getPoint() const {
	assert(_type == kTypePoint);
	return _value.asPoint;
}

const IntRange &DynamicValue::getIntRange() const {
	assert(_type == KTypeIntegerRange);
	return _value.asIntRange;
}

const AngleMagVector &DynamicValue::getVector() const {
	assert(_type == kTypeVector);
	return _value.asVector;
}

const Label &DynamicValue::getLabel() const {
	assert(_type == kTypeLabel);
	return _value.asLabel;
}

const Event &DynamicValue::getEvent() const {
	assert(_type == kTypeEvent);
	return _value.asEvent;
}

const VarReference &DynamicValue::getVarReference() const {
	assert(_type == kTypeVariableReference);
	return _value.asVarReference;
}

const Common::String &DynamicValue::getString() const {
	assert(_type == kTypeString);
	return _str;
}

const bool &DynamicValue::getBool() const {
	assert(_type == kTypeBoolean);
	return _value.asBool;
}

DynamicValue &DynamicValue::operator=(const DynamicValue &other) {
	if (this != &other) {
		clear();
		initFromOther(other);
	}

	return *this;
}

bool DynamicValue::operator==(const DynamicValue &other) const {
	if (_type != other._type)
		return false;

	switch (_type) {
	case kTypeNull:
		return true;
	case kTypeInteger:
		return _value.asInt == other._value.asInt;
	case kTypeFloat:
		return _value.asFloat == other._value.asFloat;
	case kTypePoint:
		return _value.asPoint == other._value.asPoint;
	case KTypeIntegerRange:
		return _value.asIntRange == other._value.asIntRange;
	case kTypeVector:
		return _value.asVector == other._value.asVector;
	case kTypeLabel:
		return _value.asLabel == other._value.asLabel;
	case kTypeEvent:
		return _value.asEvent == other._value.asEvent;
	case kTypeVariableReference:
		return _value.asVarReference == other._value.asVarReference;
	case kTypeIncomingData:
		return true;
	case kTypeString:
		return _str == other._str;
	case kTypeBoolean:
		return _value.asBool == other._value.asBool;
	default:
		break;
	}

	assert(false);
	return false;
}

void DynamicValue::clear() {
	_str.clear();
	_type = kTypeNull;
}

void DynamicValue::initFromOther(const DynamicValue& other) {
	assert(_type == kTypeNull);

	_type = other._type;

	switch (_type) {
	case kTypeNull:
	case kTypeIncomingData:
		break;
	case kTypeInteger:
		_value.asInt = other._value.asInt;
		break;
	case kTypeFloat:
		_value.asFloat = other._value.asFloat;
		break;
	case kTypePoint:
		_value.asPoint = other._value.asPoint;
		break;
	case KTypeIntegerRange:
		_value.asIntRange = other._value.asIntRange;
		break;
	case kTypeVector:
		_value.asVector = other._value.asVector;
		break;
	case kTypeLabel:
		_value.asLabel = other._value.asLabel;
		break;
	case kTypeEvent:
		_value.asEvent = other._value.asEvent;
		break;
	case kTypeVariableReference:
		_value.asVarReference = other._value.asVarReference;
		_str = other._str;
		_value.asVarReference.source = &_str;
		break;
	case kTypeString:
		_str = other._str;
		break;
	case kTypeBoolean:
		_value.asBool = other._value.asBool;
		break;
	default:
		assert(false);
		break;
	}
}

MessengerSendSpec::MessengerSendSpec() : destination(0) {
}

bool MessengerSendSpec::load(const Data::Event &dataEvent, uint32 dataMessageFlags, const Data::InternalTypeTaggedValue &dataLocator, const Common::String &dataWithSource, const Common::String &dataWithString, uint32 dataDestination) {
	messageFlags.relay = ((dataMessageFlags & 0x20000000) == 0);
	messageFlags.cascade = ((dataMessageFlags & 0x40000000) == 0);
	messageFlags.immediate = ((dataMessageFlags & 0x80000000) == 0);

	if (!this->send.load(dataEvent))
		return false;

	if (!this->with.load(dataLocator, dataWithSource, dataWithString))
		return false;

	this->destination = dataDestination;

	return true;
}

bool MessengerSendSpec::load(const Data::PlugInTypeTaggedValue &dataEvent, const MessageFlags &dataMessageFlags, const Data::PlugInTypeTaggedValue &dataWith, uint32 dataDestination) {
	if (dataEvent.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	if (!this->send.load(dataEvent.value.asEvent))
		return false;

	if (!this->with.load(dataWith))
		return false;

	this->destination = dataDestination;

	return true;
}

bool Event::load(const Data::Event &data) {
	eventType = data.eventID;
	eventInfo = data.eventInfo;

	return true;
}

void IPlugInModifierRegistrar::registerPlugInModifier(const char *name, const IPlugInModifierFactoryAndDataFactory *loaderFactory) {
	return this->registerPlugInModifier(name, loaderFactory, loaderFactory);
}

PlugIn::~PlugIn() {
}

ProjectResources::~ProjectResources() {
}

ProjectDescription::ProjectDescription() {
}

ProjectDescription::~ProjectDescription() {
}

void ProjectDescription::addSegment(int volumeID, const char *filePath) {
	SegmentDescription desc;
	desc.volumeID = volumeID;
	desc.filePath = filePath;
	desc.stream = nullptr;

	_segments.push_back(desc);
}

void ProjectDescription::addSegment(int volumeID, Common::SeekableReadStream *stream) {
	SegmentDescription desc;
	desc.volumeID = volumeID;
	desc.stream = stream;

	_segments.push_back(desc);
}

const Common::Array<SegmentDescription> &ProjectDescription::getSegments() const {
	return _segments;
}

void ProjectDescription::addPlugIn(const Common::SharedPtr<PlugIn>& plugIn) {
	_plugIns.push_back(plugIn);
}

const Common::Array<Common::SharedPtr<PlugIn> >& ProjectDescription::getPlugIns() const {
	return _plugIns;
}

void ProjectDescription::setResources(const Common::SharedPtr<ProjectResources> &resources) {
	_resources = resources;
}

const Common::SharedPtr<ProjectResources> &ProjectDescription::getResources() const {
	return _resources;
}


const Common::Array<Common::SharedPtr<Modifier> >& SimpleModifierContainer::getModifiers() const {
	return _modifiers;
}

void SimpleModifierContainer::appendModifier(const Common::SharedPtr<Modifier> &modifier) {
	_modifiers.push_back(modifier);
}

Structural::~Structural() {
}

ProjectPresentationSettings::ProjectPresentationSettings() : width(640), height(480), bitsPerPixel(8) {
}

const Common::Array<Common::SharedPtr<Structural> > &Structural::getChildren() const {
	return _children;
}

const Common::Array<Common::SharedPtr<Modifier> > &Structural::getModifiers() const {
	return _modifiers;
}

void Structural::appendModifier(const Common::SharedPtr<Modifier> &modifier) {
	_modifiers.push_back(modifier);
}

Runtime::Runtime() {
	_vthread.reset(new VThread());
}

void Runtime::runFrame() {
	VThreadState state = _vthread->step();
	if (state != kVThreadReturn) {
		// Still doing blocking tasks
		return;
	}

	if (_queuedProjectDesc) {
		Common::SharedPtr<ProjectDescription> desc = _queuedProjectDesc;
		_queuedProjectDesc.reset();

		_project.reset();
		_project.reset(new Project());

		_project->loadFromDescription(*desc);
	}
}

void Runtime::queueProject(const Common::SharedPtr<ProjectDescription> &desc) {
	_queuedProjectDesc = desc;
}

void Runtime::addVolume(int volumeID, const char *name, bool isMounted) {
	VolumeState volume;
	volume.name = name;
	volume.isMounted = isMounted;
	volume.volumeID = volumeID;

	_volumes.push_back(volume);
}

const Common::Array<Common::SharedPtr<Modifier> > &IModifierContainer::getModifiers() const {
	return const_cast<IModifierContainer &>(*this).getModifiers();
};

ProjectPlugInRegistry::ProjectPlugInRegistry() {
}

void ProjectPlugInRegistry::registerPlugInModifier(const char *name, const Data::IPlugInModifierDataFactory *loader, const IPlugInModifierFactory *factory) {
	_dataLoaderRegistry.registerLoader(name, loader);
	_factoryRegistry[name] = factory;
}

const Data::PlugInModifierRegistry& ProjectPlugInRegistry::getDataLoaderRegistry() const {
	return _dataLoaderRegistry;
}

const IPlugInModifierFactory *ProjectPlugInRegistry::findPlugInModifierFactory(const char *name) const {
	Common::HashMap<Common::String, const IPlugInModifierFactory *>::const_iterator it = _factoryRegistry.find(name);
	if (it == _factoryRegistry.end())
		return nullptr;
	return it->_value;
}

Project::Project()
	: _projectFormat(Data::kProjectFormatUnknown), _isBigEndian(false), _haveGlobalObjectInfo(false) {
}

Project::~Project() {
}

void Project::loadFromDescription(const ProjectDescription& desc) {
	_resources = desc.getResources();

	debug(1, "Loading new project...");

	const Common::Array<Common::SharedPtr<PlugIn> > &plugIns = desc.getPlugIns();

	for (Common::Array<Common::SharedPtr<PlugIn> >::const_iterator it = plugIns.begin(), itEnd = plugIns.end(); it != itEnd; ++it) {
		Common::SharedPtr<PlugIn> plugIn = (*it);

		_plugIns.push_back(plugIn);

		plugIn->registerModifiers(&_plugInRegistry);
	}

	const Data::PlugInModifierRegistry &plugInDataLoaderRegistry = _plugInRegistry.getDataLoaderRegistry();

	size_t numSegments = desc.getSegments().size();
	_segments.resize(numSegments);

	for (size_t i = 0; i < numSegments; i++) {
		_segments[i].desc = desc.getSegments()[i];
	}

	// Try to open the first segment
	openSegmentStream(0);

	Common::SeekableReadStream *baseStream = _segments[0].weakStream;
	uint16_t startValue = baseStream->readUint16LE();

	if (startValue == 1) {
		// Windows format
		_isBigEndian = false;
		_projectFormat = Data::kProjectFormatWindows;
	} else if (startValue == 0) {
		// Mac format
		_isBigEndian = true;
		_projectFormat = Data::kProjectFormatMacintosh;
	} else {
		error("Unrecognized project segment header");
	}

	Common::SeekableSubReadStreamEndian stream(baseStream, 2, baseStream->size(), _isBigEndian);
	if (stream.readUint32() != 0xaa55a5a5 || stream.readUint32() != 0 || stream.readUint32() != 14) {
		error("Unrecognized project segment header");
	}

	Data::DataReader reader(stream, _projectFormat);

	Common::SharedPtr<Data::DataObject> dataObject;
	Data::loadDataObject(_plugInRegistry.getDataLoaderRegistry(), reader, dataObject);

	if (!dataObject || dataObject->getType() != Data::DataObjectTypes::kProjectHeader) {
		error("Expected project header but found something else");
	}

	Data::loadDataObject(plugInDataLoaderRegistry, reader, dataObject);
	if (!dataObject || dataObject->getType() != Data::DataObjectTypes::kProjectCatalog) {
		error("Expected project catalog but found something else");
	}

	Data::ProjectCatalog *catalog = static_cast<Data::ProjectCatalog *>(dataObject.get());

	if (catalog->segments.size() != desc.getSegments().size()) {
		error("Project declared a different number of segments than the project description provided");
	}

	debug(1, "Catalog loaded OK, identified %i streams", static_cast<int>(catalog->streams.size()));

	_streams.resize(catalog->streams.size());
	for (size_t i = 0; i < _streams.size(); i++) {
		StreamDesc &streamDesc = _streams[i];
		const Data::ProjectCatalog::StreamDesc &srcStream = catalog->streams[i];

		if (!strcmp(srcStream.streamType, "assetStream"))
			streamDesc.streamType = kStreamTypeAsset;
		else if (!strcmp(srcStream.streamType, "bootStream"))
			streamDesc.streamType = kStreamTypeBoot;
		else if (!strcmp(srcStream.streamType, "sceneStream"))
			streamDesc.streamType = kStreamTypeScene;
		else
			streamDesc.streamType = kStreamTypeUnknown;

		streamDesc.segmentIndex = srcStream.segmentIndexPlusOne - 1;
		streamDesc.size = srcStream.size;
		streamDesc.pos = srcStream.pos;
	}

	// Locate the boot stream
	size_t bootStreamIndex = 0;
	bool foundBootStream = false;
	for (size_t i = 0; i < _streams.size(); i++) {
		if (_streams[i].streamType == kStreamTypeBoot) {
			bootStreamIndex = i;
			foundBootStream = true;
			break;
		}
	}
	if (!foundBootStream) {
		error("Failed to find boot stream");
	}

	debug(1, "Loading boot stream");

	loadBootStream(bootStreamIndex);
}

void Project::openSegmentStream(int segmentIndex) {
	if (segmentIndex < 0 || static_cast<size_t>(segmentIndex) > _segments.size()) {
		error("Invalid segment index %i", segmentIndex);
	}

	Segment &segment = _segments[segmentIndex];

	if (segment.weakStream)
		return;

	if (segment.desc.stream) {
		segment.rcStream.reset();
		segment.weakStream = segment.desc.stream;
	} else {
		Common::File *f = new Common::File();
		segment.rcStream.reset(f);
		segment.weakStream = f;

		if (!f->open(segment.desc.filePath)) {
			error("Failed to open segment file %s", segment.desc.filePath.c_str());
		}
	}
}

void Project::loadBootStream(size_t streamIndex) {
	const StreamDesc &streamDesc = _streams[streamIndex];

	size_t segmentIndex = streamDesc.segmentIndex;
	openSegmentStream(segmentIndex);

	Common::SeekableSubReadStreamEndian stream(_segments[segmentIndex].weakStream, streamDesc.pos, streamDesc.pos + streamDesc.size, _isBigEndian);
	Data::DataReader reader(stream, _projectFormat);

	ChildLoaderStack loaderStack;

	const Data::PlugInModifierRegistry &plugInDataLoaderRegistry = _plugInRegistry.getDataLoaderRegistry();

	size_t numObjectsLoaded = 0;
	for (;;) {
		Common::SharedPtr<Data::DataObject> dataObject;
		Data::loadDataObject(plugInDataLoaderRegistry, reader, dataObject);

		if (!dataObject) {
			error("Failed to load project boot data");
		}

		if (loaderStack.contexts.size() > 0) {
			loadContextualObject(loaderStack, *dataObject.get());
		} else {
			// Root-level objects
			switch (dataObject->getType()) {
			case Data::DataObjectTypes::kPresentationSettings:
				loadPresentationSettings(*static_cast<const Data::PresentationSettings *>(dataObject.get()));
				break;
			case Data::DataObjectTypes::kAssetCatalog:
				loadAssetCatalog(*static_cast<const Data::AssetCatalog *>(dataObject.get()));
				break;
			case Data::DataObjectTypes::kGlobalObjectInfo:
				loadGlobalObjectInfo(loaderStack, *static_cast<const Data::GlobalObjectInfo *>(dataObject.get()));
				break;
			case Data::DataObjectTypes::kStreamHeader:
			case Data::DataObjectTypes::kUnknown19:
				// Ignore
				break;
			default:
				error("Unexpected object type in boot stream");
			}
		}

		numObjectsLoaded++;
	}
}

void Project::loadPresentationSettings(const Data::PresentationSettings &presentationSettings) {
	_presentationSettings.bitsPerPixel = presentationSettings.bitsPerPixel;
	if (_presentationSettings.bitsPerPixel != 16) {
		error("Unsupported bit depth");
	}
	_presentationSettings.width = presentationSettings.dimensions.x;
	_presentationSettings.height = presentationSettings.dimensions.y;
}

void Project::loadAssetCatalog(const Data::AssetCatalog &assetCatalog) {
	_assetsByID.clear();
	_realAssets.clear();
	_assetNameToID.clear();

	size_t numRealAssets = 0;
	for (size_t i = 0; i < assetCatalog.assets.size(); i++) {
		const Data::AssetCatalog::AssetInfo &assetInfo = assetCatalog.assets[i];
		if ((assetInfo.flags1 & Data::AssetCatalog::kFlag1Deleted) == 0)
			numRealAssets++;
	}

	_realAssets.resize(numRealAssets);
	_assetsByID.resize(assetCatalog.assets.size() + 1);

	_assetsByID[0] = nullptr;

	numRealAssets = 0;
	for (size_t i = 0; i < assetCatalog.assets.size(); i++) {
		const Data::AssetCatalog::AssetInfo &assetInfo = assetCatalog.assets[i];
		if (assetInfo.flags1 & Data::AssetCatalog::kFlag1Deleted) {
			_assetsByID[i + 1] = nullptr;
		} else {
			AssetDesc &assetDesc = _realAssets[numRealAssets++];

			assetDesc.id = i + 1;
			assetDesc.name = assetInfo.name;
			assetDesc.typeCode = assetInfo.assetType;

			_assetsByID[assetDesc.id] = &assetDesc;
			if (!assetDesc.name.empty())
				_assetNameToID[assetDesc.name] = assetDesc.id;
		}
	}
}

void Project::loadGlobalObjectInfo(ChildLoaderStack& loaderStack, const Data::GlobalObjectInfo& globalObjectInfo) {
	if (_haveGlobalObjectInfo)
		error("Multiple global object infos");

	_haveGlobalObjectInfo = true;

	if (globalObjectInfo.numGlobalModifiers > 0) {
		ChildLoaderContext loaderContext;
		loaderContext.containerUnion.modifierContainer = &_globalModifiers;
		loaderContext.remainingCount = globalObjectInfo.numGlobalModifiers;
		loaderContext.type = ChildLoaderContext::kTypeModifierList;

		loaderStack.contexts.push_back(loaderContext);
	}
}

Common::SharedPtr<Modifier> Project::loadModifierObject(ModifierLoaderContext &loaderContext, const Data::DataObject &dataObject) {
	// Special case for debris
	if (dataObject.getType() == Data::DataObjectTypes::kDebris)
		return nullptr;

	Common::SharedPtr<Modifier> modifier;

	// Special case for plug-ins
	if (dataObject.getType() == Data::DataObjectTypes::kPlugInModifier) {
		const Data::PlugInModifier &plugInData = static_cast<const Data::PlugInModifier &>(dataObject);
		const IPlugInModifierFactory *factory = _plugInRegistry.findPlugInModifierFactory(plugInData.modifierName);
		if (!factory)
			error("Unknown or unsupported plug-in modifier type");

		modifier = factory->createModifier(loaderContext, plugInData);
	} else {
		IModifierFactory *factory = getModifierFactoryForDataObjectType(dataObject.getType());

		if (!factory)
			error("Unknown or unsupported modifier type, or non-modifier encountered where a modifier was expected");

		modifier = factory->createModifier(loaderContext, dataObject);
	}
	if (!modifier)
		error("Modifier object failed to load");

	return modifier;
}

void Project::loadContextualObject(ChildLoaderStack &stack, const Data::DataObject &dataObject) {
	ChildLoaderContext &topContext = stack.contexts.back();

	// The stack entry must always be popped before loading the object because the load process may descend into more children,
	// such as when behaviors are nested.
	switch (topContext.type) {
	case ChildLoaderContext::kTypeModifierList: {
			IModifierContainer *container = topContext.containerUnion.modifierContainer;

			if ((--topContext.remainingCount) == 0)
				stack.contexts.pop_back();

			ModifierLoaderContext loaderContext(&stack);

			container->appendModifier(loadModifierObject(loaderContext, dataObject));
		}
		break;
	}
}

ModifierFlags::ModifierFlags() : isLastModifier(false) {
}

bool ModifierFlags::load(const uint32 dataModifierFlags) {
	isLastModifier = ((dataModifierFlags & 0x2) != 0);
	return true;
}

Modifier::Modifier() : _guid(0) {
}

Modifier::~Modifier() {
}

bool Modifier::loadTypicalHeader(const Data::TypicalModifierHeader& typicalHeader) {
	if (!_modifierFlags.load(typicalHeader.modifierFlags))
		return false;

	return true;
}

} // End of namespace MTropolis
