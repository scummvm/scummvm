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

#include "common/random.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/translation.h"

#include "gui/dialog.h"
#include "gui/imagealbum-dialog.h"

#include "graphics/palette.h"

#include "image/bmp.h"
#include "image/pict.h"

#include "mtropolis/miniscript.h"
#include "mtropolis/plugin/standard.h"
#include "mtropolis/plugins.h"

#include "mtropolis/miniscript.h"

namespace MTropolis {

namespace Standard {

CursorModifier::CursorModifier() : _cursorID(0) {
}

bool CursorModifier::respondsToEvent(const Event &evt) const {
	return _applyWhen.respondsTo(evt) || _removeWhen.respondsTo(evt);
}

VThreadState CursorModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	// As with mTropolis, this doesn't support stacking cursors
 	if (_applyWhen.respondsTo(msg->getEvent())) {
		runtime->setModifierCursorOverride(_cursorID);
	}
	if (_removeWhen.respondsTo(msg->getEvent())) {
		// This doesn't call "disable" because the behavior is actually different.
		// Disabling a cursor modifier doesn't seem to remove it.
		runtime->clearModifierCursorOverride();
	}
	return kVThreadReturn;
}

void CursorModifier::disable(Runtime *runtime) {
}

bool CursorModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::CursorModifier &data) {
	if (data.applyWhen.type != Data::PlugInTypeTaggedValue::kEvent || data.cursorIDAsLabel.type != Data::PlugInTypeTaggedValue::kLabel)
		return false;

	if (!_applyWhen.load(data.applyWhen.value.asEvent))
		return false;

	if (data.haveRemoveWhen) {
		if (!_removeWhen.load(data.removeWhen.value.asEvent))
			return false;
	}

	if (data.cursorIDAsLabel.type != Data::PlugInTypeTaggedValue::kLabel)
		return false;

	_cursorID = data.cursorIDAsLabel.value.asLabel.labelID;

	return true;
}

Common::SharedPtr<Modifier> CursorModifier::shallowClone() const {
	Common::SharedPtr<CursorModifier> clone(new CursorModifier(*this));
	return clone;
}

const char *CursorModifier::getDefaultName() const {
	return "Cursor Modifier";
}

STransCtModifier::STransCtModifier() : _transitionType(0), _transitionDirection(0), _steps(0), _duration(0), _fullScreen(false) {
}

bool STransCtModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::STransCtModifier &data) {
	if (data.enableWhen.type != Data::PlugInTypeTaggedValue::kEvent ||
		data.disableWhen.type != Data::PlugInTypeTaggedValue::kEvent ||
		data.transitionType.type != Data::PlugInTypeTaggedValue::kInteger ||
		data.transitionDirection.type != Data::PlugInTypeTaggedValue::kInteger ||
		data.steps.type != Data::PlugInTypeTaggedValue::kInteger ||
		data.duration.type != Data::PlugInTypeTaggedValue::kInteger ||
		data.fullScreen.type != Data::PlugInTypeTaggedValue::kBoolean)
		return false;

	if (!_enableWhen.load(data.enableWhen.value.asEvent) || !_disableWhen.load(data.disableWhen.value.asEvent))
		return false;

	_transitionType = data.transitionType.value.asInt;
	_transitionDirection = data.transitionDirection.value.asInt;
	_steps = data.steps.value.asInt;
	_duration = data.duration.value.asInt;
	_fullScreen = data.fullScreen.value.asBoolean;

	return true;
}

bool STransCtModifier::respondsToEvent(const Event &evt) const {
	return _enableWhen.respondsTo(evt) || _disableWhen.respondsTo(evt);
}

VThreadState STransCtModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_enableWhen.respondsTo(msg->getEvent())) {
		SceneTransitionEffect effect;
		effect._duration = _duration / 10;
		effect._steps = _steps;

		if (SceneTransitionTypes::loadFromData(effect._transitionType, _transitionType) && SceneTransitionDirections::loadFromData(effect._transitionDirection, _transitionDirection)) {
			// Duration doesn't seem to affect wipe transitions for some reason.
			// In Obsidian, this mostly effects 180-degree turns.
			// Good place to test this is in the corners of the Bureau library, where it's 0,
			// but some cases where it is set (e.g. the Spider control room) have the same duration anyway.
			if (effect._transitionType == SceneTransitionTypes::kWipe)
				effect._duration = 500;

			runtime->setSceneTransitionEffect(false, &effect);
		} else {
			warning("Source-scene transition had invalid data");
		}
	}
	if (_disableWhen.respondsTo(msg->getEvent()))
		disable(runtime);

	return kVThreadReturn;
}

void STransCtModifier::disable(Runtime *runtime) {
	runtime->setSceneTransitionEffect(false, nullptr);
}

bool STransCtModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "rate") {
		if (_duration <= (kMaxDuration / 100))
			result.setInt(100);
		else if (_duration >= kMaxDuration)
			result.setInt(1);
		else
			result.setInt((kMaxDuration + (_duration / 2)) / _duration);
		return true;
	} else if (attrib == "steps") {
		result.setInt(_steps);
		return true;
	}

	return Modifier::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome STransCtModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "rate") {
		DynamicValueWriteFuncHelper<STransCtModifier, &STransCtModifier::scriptSetRate, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "steps") {
		DynamicValueWriteFuncHelper<STransCtModifier, &STransCtModifier::scriptSetSteps, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return Modifier::writeRefAttribute(thread, result, attrib);
}


Common::SharedPtr<Modifier> STransCtModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new STransCtModifier(*this));
}

const char *STransCtModifier::getDefaultName() const {
	return "STransCt";	// Probably wrong
}

MiniscriptInstructionOutcome STransCtModifier::scriptSetRate(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	if (asInteger < 1)
		asInteger = 1;
	else if (asInteger > 100)
		asInteger = 100;

	if (asInteger == 100)
		_duration = 0;
	else
		_duration = kMaxDuration / asInteger;

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome STransCtModifier::scriptSetSteps(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	if (asInteger < 4)
		asInteger = 4;
	else if (asInteger > 256)
		asInteger = 100;

	_steps = asInteger;

	return kMiniscriptInstructionOutcomeContinue;
}

MediaCueMessengerModifier::CueSourceUnion::CueSourceUnion() : asUnset(0) {
}

MediaCueMessengerModifier::CueSourceUnion::~CueSourceUnion() {
}

template<class T, T (MediaCueMessengerModifier::CueSourceUnion::*TMember)>
void MediaCueMessengerModifier::CueSourceUnion::construct(const T &value) {
	T *field = &(this->*TMember);
	new (field) T(value);
}

template<class T, T (MediaCueMessengerModifier::CueSourceUnion::*TMember)>
void MediaCueMessengerModifier::CueSourceUnion::destruct() {
	T *field = &(this->*TMember);
	field->~T();
}

MediaCueMessengerModifier::MediaCueMessengerModifier() : _isActive(false), _cueSourceType(kCueSourceInvalid) {
	_mediaCue.sourceModifier = this;
}

MediaCueMessengerModifier::MediaCueMessengerModifier(const MediaCueMessengerModifier &other)
	: Modifier(other), _cueSourceType(other._cueSourceType), _cueSourceModifier(other._cueSourceModifier), _enableWhen(other._enableWhen), _disableWhen(other._disableWhen), _mediaCue(other._mediaCue), _isActive(other._isActive) {
	_cueSource.destruct<uint64, &CueSourceUnion::asUnset>();

	switch (_cueSourceType) {
	case kCueSourceInteger:
		_cueSource.construct<int32, &CueSourceUnion::asInt>(other._cueSource.asInt);
		break;
	case kCueSourceIntegerRange:
		_cueSource.construct<IntRange, &CueSourceUnion::asIntRange>(other._cueSource.asIntRange);
		break;
	case kCueSourceVariableReference:
		_cueSource.construct<uint32, &CueSourceUnion::asVarRefGUID>(other._cueSource.asVarRefGUID);
		break;
	case kCueSourceLabel:
		_cueSource.construct<Label, &CueSourceUnion::asLabel>(other._cueSource.asLabel);
		break;
	case kCueSourceString:
		_cueSource.construct<Common::String, &CueSourceUnion::asString>(other._cueSource.asString);
		break;
	default:
		_cueSource.construct<uint64, &CueSourceUnion::asUnset>(0);
		break;
	}
}

MediaCueMessengerModifier::~MediaCueMessengerModifier() {
	destructCueSource();
}

void MediaCueMessengerModifier::destructCueSource() {
	switch (_cueSourceType) {
	case kCueSourceInteger:
		_cueSource.destruct<int32, &CueSourceUnion::asInt>();
		break;
	case kCueSourceIntegerRange:
		_cueSource.destruct<IntRange, &CueSourceUnion::asIntRange>();
		break;
	case kCueSourceVariableReference:
		_cueSource.destruct<uint32, &CueSourceUnion::asVarRefGUID>();
		break;
	case kCueSourceLabel:
		_cueSource.destruct<Label, &CueSourceUnion::asLabel>();
		break;
	case kCueSourceString:
		_cueSource.destruct<Common::String, &CueSourceUnion::asString>();
		break;
	default:
		_cueSource.destruct<uint64, &CueSourceUnion::asUnset>();
		break;
	}
}

bool MediaCueMessengerModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::MediaCueMessengerModifier &data) {
	if (data.enableWhen.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	if (!_enableWhen.load(data.enableWhen.value.asEvent))
		return false;

	if (data.disableWhen.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	if (!_disableWhen.load(data.disableWhen.value.asEvent))
		return false;

	if (data.triggerTiming.type != Data::PlugInTypeTaggedValue::kInteger)
		return false;

	_mediaCue.triggerTiming = static_cast<MediaCueState::TriggerTiming>(data.triggerTiming.value.asInt);

	if (data.nonStandardMessageFlags.type != Data::PlugInTypeTaggedValue::kInteger)
		return false;

	int32 msgFlags = data.nonStandardMessageFlags.value.asInt;

	MessageFlags messageFlags;
	messageFlags.immediate = ((msgFlags & Data::Standard::MediaCueMessengerModifier::kMessageFlagImmediate) != 0);
	messageFlags.cascade = ((msgFlags & Data::Standard::MediaCueMessengerModifier::kMessageFlagCascade) != 0);
	messageFlags.relay = ((msgFlags & Data::Standard::MediaCueMessengerModifier::kMessageFlagRelay) != 0);
	if (!_mediaCue.send.load(data.sendEvent, messageFlags, data.with, data.destination))
		return false;

	destructCueSource();

	switch (data.executeAt.type) {
	case Data::PlugInTypeTaggedValue::kInteger:
		_cueSource.construct<int32, &CueSourceUnion::asInt>(data.executeAt.value.asInt);
		_cueSourceType = kCueSourceInteger;
		break;
	case Data::PlugInTypeTaggedValue::kIntegerRange: {
			IntRange intRange;
			if (!intRange.load(data.executeAt.value.asIntRange))
				return false;

			_cueSource.construct<IntRange, &CueSourceUnion::asIntRange>(intRange);
			_cueSourceType = kCueSourceIntegerRange;
		} break;
	case Data::PlugInTypeTaggedValue::kVariableReference:
		_cueSource.construct<uint32, &CueSourceUnion::asVarRefGUID>(data.executeAt.value.asVarRefGUID);
		_cueSourceType = kCueSourceVariableReference;
		break;
	case Data::PlugInTypeTaggedValue::kLabel: {
			Label label;
			if (!label.load(data.executeAt.value.asLabel))
				return false;

			_cueSource.construct<Label, &CueSourceUnion::asLabel>(label);
			_cueSourceType = kCueSourceLabel;
		} break;
	case Data::PlugInTypeTaggedValue::kString:
		_cueSource.construct<Common::String, &CueSourceUnion::asString>(data.executeAt.value.asString);
		_cueSourceType = kCueSourceString;
		break;
	default:
		_cueSource.construct<uint64, &CueSourceUnion::asUnset>(0);
		_cueSourceType = kCueSourceInvalid;
		return false;
	}

	return true;
}

bool MediaCueMessengerModifier::respondsToEvent(const Event &evt) const {
	return _enableWhen.respondsTo(evt) || _disableWhen.respondsTo(evt);
}

VThreadState MediaCueMessengerModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_enableWhen.respondsTo(msg->getEvent())) {
		Structural *owner = findStructuralOwner();
		if (owner && owner->isElement()) {

			Element *element = static_cast<Element *>(owner);

			switch (_cueSourceType) {
			case kCueSourceInteger:
				_mediaCue.minTime = _mediaCue.maxTime = _cueSource.asInt;
				break;
			case kCueSourceIntegerRange:
				_mediaCue.minTime = _cueSource.asIntRange.min;
				_mediaCue.maxTime = _cueSource.asIntRange.max;
				break;
			case kCueSourceLabel: {
					int32 resolved = 0;
					if (element->resolveMediaMarkerLabel(_cueSource.asLabel, resolved))
						_mediaCue.minTime = _mediaCue.maxTime = resolved;
					else {
						warning("Failed to resolve media cue marker label");
						return kVThreadError;
					}
				} break;
			case kCueSourceVariableReference: {
					Modifier *modifier = _cueSourceModifier.lock().get();
					if (!modifier->isVariable()) {
						warning("Media cue source variable couldn't be resolved");
						return kVThreadReturn;
					}

					DynamicValue value;
					static_cast<VariableModifier *>(modifier)->varGetValue(value);

					switch (value.getType()) {
					case DynamicValueTypes::kInteger:
						_mediaCue.minTime = _mediaCue.maxTime = value.getInt();
						break;
					case DynamicValueTypes::kIntegerRange:
						_mediaCue.minTime = value.getIntRange().min;
						_mediaCue.maxTime = value.getIntRange().max;
						break;
					case DynamicValueTypes::kFloat:
						_mediaCue.minTime = _mediaCue.maxTime = static_cast<int32>(round(value.getFloat()));
						break;
					default:
						warning("Media cue variable was not a usable type");
						return kVThreadError;
					}

				} break;
			default:
				assert(false);	// Something wasn't handled in the loader
				return kVThreadReturn;
			}

			element->addMediaCue(&_mediaCue);
			_isActive = true;
		}
	}
	if (_disableWhen.respondsTo(msg->getEvent())) {
		disable(runtime);
	}

	return kVThreadReturn;
}

void MediaCueMessengerModifier::disable(Runtime *runtime) {
	if (_isActive) {
		Structural *owner = findStructuralOwner();
		if (owner && owner->isElement())
			static_cast<Element *>(owner)->removeMediaCue(&_mediaCue);

		_isActive = false;
	}
}

Modifier *MediaCueMessengerModifier::getMediaCueModifier() {
	return this;
}

Common::WeakPtr<Modifier> MediaCueMessengerModifier::getMediaCueTriggerSource() const {
	return _cueSourceModifier;
}

Common::SharedPtr<Modifier> MediaCueMessengerModifier::shallowClone() const {
	Common::SharedPtr<MediaCueMessengerModifier> clone(new MediaCueMessengerModifier(*this));
	clone->_isActive = false;
	clone->_mediaCue.sourceModifier = clone.get();
	clone->_mediaCue.incomingData = DynamicValue();
	return clone;
}

const char *MediaCueMessengerModifier::getDefaultName() const {
	return "Media Cue Messenger";
}

void MediaCueMessengerModifier::linkInternalReferences(ObjectLinkingScope *scope) {
	if (_cueSourceType == kCueSourceVariableReference) {
		Common::WeakPtr<RuntimeObject> obj = scope->resolve(_cueSource.asVarRefGUID);
		RuntimeObject *objPtr = obj.lock().get();
		if (objPtr && objPtr->isModifier())
			_cueSourceModifier = obj.staticCast<Modifier>();
	}

	_mediaCue.send.linkInternalReferences(scope);
}

void MediaCueMessengerModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	visitor->visitWeakModifierRef(_cueSourceModifier);
	_mediaCue.send.visitInternalReferences(visitor);
}

ObjectReferenceVariableModifier::ObjectReferenceVariableModifier() : VariableModifier(Common::SharedPtr<VariableStorage>(new ObjectReferenceVariableStorage())) {
}

bool ObjectReferenceVariableModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::ObjectReferenceVariableModifier &data) {
	if (data.setToSourceParentWhen.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	if (!_setToSourceParentWhen.load(data.setToSourceParentWhen.value.asEvent))
		return false;

	ObjectReferenceVariableStorage *storage = static_cast<ObjectReferenceVariableStorage *>(_storage.get());

	if (data.objectPath.type == Data::PlugInTypeTaggedValue::kString)
		storage->_objectPath = data.objectPath.value.asString;
	else if (data.objectPath.type != Data::PlugInTypeTaggedValue::kNull)
		return false;

	storage->_object.reset();

	return true;
}

// Object reference variables are somewhat unusual in that they don't store a simple value,
// they instead have "object" and "path" attributes AND as a value, they resolve to the
// modifier itself.
bool ObjectReferenceVariableModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	ObjectReferenceVariableStorage *storage = static_cast<ObjectReferenceVariableStorage *>(_storage.get());

	if (attrib == "path") {
		result.setString(storage->_objectPath);
		return true;
	}
	if (attrib == "object") {
		if (storage->_object.object.expired())
			resolve(thread->getRuntime());

		if (storage->_object.object.expired())
			result.clear();
		else
			result.setObject(storage->_object);
		return true;
	}

	return VariableModifier::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome ObjectReferenceVariableModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "path") {
		DynamicValueWriteFuncHelper<ObjectReferenceVariableModifier, &ObjectReferenceVariableModifier::scriptSetPath, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "object") {
		result.pod.ptrOrOffset = 0;
		result.pod.objectRef = this;
		result.pod.ifc = DynamicValueWriteInterfaceGlue<ObjectWriteInterface>::getInstance();
		return kMiniscriptInstructionOutcomeContinue;
	}

	return VariableModifier::writeRefAttribute(thread, result, attrib);
}

bool ObjectReferenceVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	switch (value.getType()) {
	case DynamicValueTypes::kNull:
	case DynamicValueTypes::kObject:
		return scriptSetObject(thread, value) == kMiniscriptInstructionOutcomeContinue;
	case DynamicValueTypes::kString:
		return scriptSetPath(thread, value) == kMiniscriptInstructionOutcomeContinue;
	default:
		return false;
	}
}

void ObjectReferenceVariableModifier::varGetValue(DynamicValue &dest) const {
	dest.setObject(getSelfReference());
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void ObjectReferenceVariableModifier::debugInspect(IDebugInspectionReport *report) const {
	VariableModifier::debugInspect(report);

	ObjectReferenceVariableStorage *storage = static_cast<ObjectReferenceVariableStorage *>(_storage.get());

	report->declareDynamic("path", storage->_objectPath);
	report->declareDynamic("fullPath", storage->_fullPath);
}
#endif

Common::SharedPtr<Modifier> ObjectReferenceVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ObjectReferenceVariableModifier(*this));
}

const char *ObjectReferenceVariableModifier::getDefaultName() const {
	return "Object Reference Variable";
}

MiniscriptInstructionOutcome ObjectReferenceVariableModifier::scriptSetPath(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kString)
		return kMiniscriptInstructionOutcomeFailed;

	ObjectReferenceVariableStorage *storage = static_cast<ObjectReferenceVariableStorage *>(_storage.get());

	storage->_objectPath = value.getString();
	storage->_object.reset();

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome ObjectReferenceVariableModifier::scriptSetObject(MiniscriptThread *thread, const DynamicValue &value) {
	ObjectReferenceVariableStorage *storage = static_cast<ObjectReferenceVariableStorage *>(_storage.get());

	if (value.getType() == DynamicValueTypes::kNull) {
		storage->_object.reset();
		storage->_objectPath.clear();
		storage->_fullPath.clear();

		return kMiniscriptInstructionOutcomeContinue;
	} else if (value.getType() == DynamicValueTypes::kObject) {
		Common::SharedPtr<RuntimeObject> obj = value.getObject().object.lock();
		if (!obj)
			return scriptSetObject(thread, DynamicValue());

		if (!computeObjectPath(obj.get(), storage->_fullPath))
			return scriptSetObject(thread, DynamicValue());

		storage->_objectPath = storage->_fullPath;
		storage->_object.object = obj;

		return kMiniscriptInstructionOutcomeContinue;
	} else
		return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome ObjectReferenceVariableModifier::scriptObjectRefAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, const Common::String &attrib) {
	ObjectReferenceVariableStorage *storage = static_cast<ObjectReferenceVariableStorage *>(_storage.get());

	resolve(thread->getRuntime());

	if (storage->_object.object.expired()) {
		thread->error("Attempted to reference an attribute of an object variable object, but the reference is dead");
		return kMiniscriptInstructionOutcomeFailed;
	}

	return storage->_object.object.lock()->writeRefAttribute(thread, proxy, attrib);
}

MiniscriptInstructionOutcome ObjectReferenceVariableModifier::scriptObjectRefAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, const Common::String &attrib, const DynamicValue &index) {
	ObjectReferenceVariableStorage *storage = static_cast<ObjectReferenceVariableStorage *>(_storage.get());

	resolve(thread->getRuntime());

	if (storage->_object.object.expired()) {
		thread->error("Attempted to reference an attribute of an object variable object, but the reference is dead");
		return kMiniscriptInstructionOutcomeFailed;
	}

	return storage->_object.object.lock()->writeRefAttributeIndexed(thread, proxy, attrib, index);
}

void ObjectReferenceVariableModifier::resolve(Runtime *runtime) {
	ObjectReferenceVariableStorage *storage = static_cast<ObjectReferenceVariableStorage *>(_storage.get());

	if (!storage->_object.object.expired())
		return;

	storage->_fullPath.clear();
	storage->_object.reset();

	if (storage->_objectPath.size() == 0)
		return;

	if (storage->_objectPath[0] == '/')
		resolveAbsolutePath(runtime);
	else if (storage->_objectPath[0] == '.')
		resolveRelativePath(runtime, this, storage->_objectPath, 0);
	else
		warning("Object reference variable had an unknown path format");

	if (!storage->_object.object.expired()) {
		if (!computeObjectPath(storage->_object.object.lock().get(), storage->_fullPath)) {
			storage->_object.reset();
		}
	}
}

void ObjectReferenceVariableModifier::resolveRelativePath(Runtime *runtime, RuntimeObject *obj, const Common::String &path, size_t startPos) {
	ObjectReferenceVariableStorage *storage = static_cast<ObjectReferenceVariableStorage *>(_storage.get());

	bool haveNextLevel = true;
	size_t nextLevelPos = startPos;

	while (haveNextLevel) {
		startPos = nextLevelPos;
		size_t endPos = path.find('/', startPos);
		if (endPos == Common::String::npos) {
			haveNextLevel = false;
			endPos = path.size();
		} else {
			nextLevelPos = endPos + 1;
		}

		Common::String levelName = path.substr(startPos, endPos - startPos);

		// This is technically more forgiving than mTropolis, which only allows ".." chains at the start of the path
		// Adjust this if it turns out to be a problem...
		if (levelName == "..") {
			obj = getObjectParent(obj);
			if (obj == nullptr)
				return;

			continue;
		}

		const Common::Array<Common::SharedPtr<Modifier> > *modifierChildren = nullptr;
		const Common::Array<Common::SharedPtr<Structural> > *structuralChildren = nullptr;

		if (obj->isStructural()) {
			Structural *structural = static_cast<Structural *>(obj);

			if (structural->getSceneLoadState() == Structural::SceneLoadState::kSceneNotLoaded)
				runtime->hotLoadScene(structural);

			modifierChildren = &structural->getModifiers();
			structuralChildren = &structural->getChildren();
		} else if (obj->isModifier()) {
			Modifier *modifier = static_cast<Modifier *>(obj);
			IModifierContainer *childContainer = modifier->getChildContainer();
			if (childContainer)
				modifierChildren = &childContainer->getModifiers();
		}

		bool foundMatch = false;
		if (modifierChildren) {
			for (const Common::SharedPtr<Modifier> &modifier : *modifierChildren) {
				if (caseInsensitiveEqual(levelName, modifier->getName())) {
					foundMatch = true;
					obj = modifier.get();
					break;
				}
			}
		}
		if (structuralChildren && !foundMatch) {
			for (const Common::SharedPtr<Structural> &structural : *structuralChildren) {
				if (caseInsensitiveEqual(levelName, structural->getName())) {
					foundMatch = true;
					obj = structural.get();
					break;
				}
			}
		}

		if (!foundMatch)
			return;
	}

	storage->_object.object = obj->getSelfReference();
}

void ObjectReferenceVariableModifier::resolveAbsolutePath(Runtime *runtime) {
	ObjectReferenceVariableStorage *storage = static_cast<ObjectReferenceVariableStorage *>(_storage.get());

	assert(storage->_objectPath[0] == '/');

	RuntimeObject *project = this;
	for (;;) {
		RuntimeObject *parent = getObjectParent(project);
		if (!parent)
			break;
		project = parent;
	}

	if (!project->isProject())
		return; // Some sort of detached object

	size_t prefixEnd = 0;

	bool foundPrefix = false;

	if (runtime->getHacks().ignoreMismatchedProjectNameInObjectLookups) {
		size_t slashOffset = storage->_objectPath.findFirstOf('/', 1);
		if (slashOffset != Common::String::npos) {
			prefixEnd = slashOffset;
			foundPrefix = true;
		}
	} else {
		Common::String projectPrefixes[2] = {
			"/" + static_cast<Structural *>(project)->getName(),
			"/<project>"};

		for (const Common::String &prefix : projectPrefixes) {
			if (storage->_objectPath.size() >= prefix.size() && caseInsensitiveEqual(storage->_objectPath.substr(0, prefix.size()), prefix)) {
				prefixEnd = prefix.size();
				foundPrefix = true;
				break;
			}
		}
	}

	if (!foundPrefix)
		return;

	// If the object path is longer, then there must be a slash separator, otherwise this doesn't match the project
	if (prefixEnd == storage->_objectPath.size()) {
		storage->_object = ObjectReference(project->getSelfReference());
		return;
	}

	if (storage->_objectPath[prefixEnd] != '/')
		return;

	return resolveRelativePath(runtime, project, storage->_objectPath, prefixEnd + 1);
}

bool ObjectReferenceVariableModifier::computeObjectPath(RuntimeObject *obj, Common::String &outPath) {
	Common::String pathForThis = "/";

	if (obj->isStructural()) {
		Structural *structural = static_cast<Structural *>(obj);
		pathForThis += structural->getName();
	} else if (obj->isModifier()) {
		Modifier *modifier = static_cast<Modifier *>(obj);
		pathForThis += modifier->getName();
	}

	RuntimeObject *parent = getObjectParent(obj);

	if (parent) {
		Common::String pathForParent;
		if (!computeObjectPath(parent, pathForParent))
			return false;

		outPath = pathForParent + pathForThis;
	} else
		outPath = pathForThis;

	return true;
}

RuntimeObject *ObjectReferenceVariableModifier::getObjectParent(RuntimeObject *obj) {
	if (obj->isStructural()) {
		Structural *structural = static_cast<Structural *>(obj);
		return structural->getParent();
	} else if (obj->isModifier()) {
		Modifier *modifier = static_cast<Modifier *>(obj);
		return modifier->getParent().lock().get();
	}
	return nullptr;
}

MiniscriptInstructionOutcome ObjectReferenceVariableModifier::ObjectWriteInterface::write(MiniscriptThread *thread, const DynamicValue &value, void *objectRef, uintptr ptrOrOffset) {
	return static_cast<ObjectReferenceVariableModifier *>(objectRef)->scriptSetObject(thread, value);
}

MiniscriptInstructionOutcome ObjectReferenceVariableModifier::ObjectWriteInterface::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) {
	return static_cast<ObjectReferenceVariableModifier *>(objectRef)->scriptObjectRefAttrib(thread, proxy, attrib);
}

MiniscriptInstructionOutcome ObjectReferenceVariableModifier::ObjectWriteInterface::refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) {
	return static_cast<ObjectReferenceVariableModifier *>(objectRef)->scriptObjectRefAttribIndexed(thread, proxy, attrib, index);
}

ObjectReferenceVariableStorage::SaveLoad::SaveLoad(ObjectReferenceVariableStorage *storage) : _storage(storage) {
	_objectPath = _storage->_objectPath;
}




ObjectReferenceVariableStorage::ObjectReferenceVariableStorage() {
}

Common::SharedPtr<ModifierSaveLoad> ObjectReferenceVariableStorage::getSaveLoad(Runtime *runtime) {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
}

Common::SharedPtr<VariableStorage> ObjectReferenceVariableStorage::clone() const {
	return Common::SharedPtr<VariableStorage>(new ObjectReferenceVariableStorage(*this));
}

void ObjectReferenceVariableStorage::SaveLoad::commitLoad() const {
	_storage->_object.reset();
	_storage->_fullPath.clear();
	_storage->_objectPath = _objectPath;
}

void ObjectReferenceVariableStorage::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	stream->writeUint32BE(_objectPath.size());
	stream->writeString(_objectPath);
}

bool ObjectReferenceVariableStorage::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
	uint32 stringLen = stream->readUint32BE();
	if (stream->err())
		return false;

	_objectPath.clear();

	if (stringLen) {
		Common::Array<char> strChars;
		strChars.resize(stringLen);
		stream->read(&strChars[0], stringLen);
		if (stream->err())
			return false;

		_objectPath = Common::String(&strChars[0], stringLen);
	}

	return true;
}

ListVariableModifier::ListVariableModifier() : VariableModifier(Common::SharedPtr<VariableStorage>(new ListVariableStorage())) {
}

bool ListVariableModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::ListVariableModifier &data) {
	ListVariableStorage *storage = static_cast<ListVariableStorage *>(_storage.get());

	bool loadData = true;

	storage->_preferredContentType = DynamicValueTypes::kInvalid;
	switch (data.contentsType) {
	case Data::Standard::ListVariableModifier::kContentsTypeInteger:
		storage->_preferredContentType = DynamicValueTypes::kInteger;
		break;
	case Data::Standard::ListVariableModifier::kContentsTypePoint:
		storage->_preferredContentType = DynamicValueTypes::kPoint;
		break;
	case Data::Standard::ListVariableModifier::kContentsTypeRange:
		storage->_preferredContentType = DynamicValueTypes::kIntegerRange;
		break;
	case Data::Standard::ListVariableModifier::kContentsTypeFloat:
		storage->_preferredContentType = DynamicValueTypes::kFloat;
		break;
	case Data::Standard::ListVariableModifier::kContentsTypeString:
		storage->_preferredContentType = DynamicValueTypes::kString;
		break;
	case Data::Standard::ListVariableModifier::kContentsTypeObject:
		storage->_preferredContentType = DynamicValueTypes::kObject;
		if (data.persistentValuesGarbled) {
			// Ignore and let the game fix it
		} else {
			warning("Loading object reference lists from data is not implemented");
		}
		loadData = false;
		break;
	case Data::Standard::ListVariableModifier::kContentsTypeVector:
		storage->_preferredContentType = DynamicValueTypes::kVector;
		break;
	case Data::Standard::ListVariableModifier::kContentsTypeBoolean:
		storage->_preferredContentType = DynamicValueTypes::kBoolean;
		break;
	default:
		warning("Unknown list data type");
		return false;
	}

	storage->_list->forceType(storage->_preferredContentType);

	if (loadData) {
		if (!data.havePersistentData || data.numValues == 0)
			return true;

		for (size_t i = 0; i < data.numValues; i++) {
			DynamicValue dynValue;
			if (!dynValue.loadConstant(data.values[i]))
				return false;

			if (dynValue.getType() != storage->_preferredContentType) {
				warning("List mod initialization element had the wrong type");
				return false;
			}

			if (!storage->_list->setAtIndex(i, dynValue)) {
				warning("Failed to initialize list modifier, value was rejected");
				return false;
			}
		}
	}

	return true;
}

bool ListVariableModifier::isListVariable() const {
	return true;
}

bool ListVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	ListVariableStorage *storage = static_cast<ListVariableStorage *>(_storage.get());

	if (value.getType() == DynamicValueTypes::kList) {
		// Source value is a list.  In this case, it must be convertible, or an error occurs.
		Common::SharedPtr<DynamicList> sourceList = value.getList();
		Common::SharedPtr<DynamicList> newList(new DynamicList());

		for (size_t i = 0; i < sourceList->getSize(); i++) {
			DynamicValue sourceElement;
			sourceList->getAtIndex(i, sourceElement);

			DynamicValue convertedElement;

			if (!sourceElement.convertToType(storage->_preferredContentType, convertedElement)) {
				thread->error("Failed to convert list when assigning to a list variable");
				return false;
			}

			newList->setAtIndex(i, convertedElement);
		}

		storage->_list = newList;
	} else if (value.getType() == DynamicValueTypes::kObject) {
		// Source value is an object.  In this case, it must be another list, otherwise this fails without an error.
		RuntimeObject *obj = value.getObject().object.lock().get();
		if (obj && obj->isModifier() && static_cast<Modifier *>(obj)->isVariable() && static_cast<VariableModifier *>(obj)->isListVariable()) {
			Common::SharedPtr<DynamicList> sourceList = static_cast<ListVariableStorage *>(static_cast<ListVariableModifier *>(obj)->_storage.get())->_list;
			Common::SharedPtr<DynamicList> newList(new DynamicList());

			bool failed = false;
			for (size_t i = 0; i < sourceList->getSize(); i++) {
				DynamicValue sourceElement;
				sourceList->getAtIndex(i, sourceElement);

				DynamicValue convertedElement;

				if (!sourceElement.convertToType(storage->_preferredContentType, convertedElement)) {
					warning("Failed to convert list when assigning to a list variable.  (Non-fatal since it was directly assigned.)");
					failed = true;
					break;
				}

				newList->setAtIndex(i, convertedElement);
			}

			if (!failed)
				storage->_list = newList;
		}
	} else {
		// Source value is a non-list.  In this case, it must be exactly the correct type, except for numbers.

		DynamicValue convertedValue;
		if (value.convertToType(storage->_preferredContentType, convertedValue)) {
			Common::SharedPtr<DynamicList> newList(new DynamicList());
			newList->setAtIndex(0, convertedValue);
			storage->_list = newList;
		} else {
			thread->error("Can't assign incompatible value type to a list variable");
			return false;
		}
	}

	return true;
}

void ListVariableModifier::varGetValue(DynamicValue &dest) const {
	dest.setObject(this->getSelfReference());
}

bool ListVariableModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	ListVariableStorage *storage = static_cast<ListVariableStorage *>(_storage.get());

	if (attrib == "count") {
		result.setInt(storage->_list->getSize());
		return true;
	} else if (attrib == "random") {
		if (storage->_list->getSize() == 0)
			return false;

		size_t index = thread->getRuntime()->getRandom()->getRandomNumber(storage->_list->getSize() - 1);
		return storage->_list->getAtIndex(index, result);
	} else if (attrib == "shuffle") {
		storage->_list = storage->_list->clone();

		Common::RandomSource *rng = thread->getRuntime()->getRandom();

		size_t listSize = storage->_list->getSize();
		for (size_t i = 1; i < listSize; i++) {
			size_t sourceIndex = i - 1;
			size_t destIndex = sourceIndex + rng->getRandomNumber(static_cast<uint>(listSize - i));
			if (sourceIndex != destIndex) {
				DynamicValue srcValue;
				DynamicValue destValue;
				(void)storage->_list->getAtIndex(sourceIndex, srcValue);
				(void)storage->_list->getAtIndex(destIndex, destValue);

				(void)storage->_list->setAtIndex(destIndex, srcValue);
				(void)storage->_list->setAtIndex(sourceIndex, destValue);
			}
		}

		result.setInt(listSize);
		return true;
	}

	return Modifier::readAttribute(thread, result, attrib);
}

bool ListVariableModifier::readAttributeIndexed(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib, const DynamicValue &index) {
	ListVariableStorage *storage = static_cast<ListVariableStorage *>(_storage.get());

	if (attrib == "value") {
		size_t realIndex = 0;
		return storage->_list->dynamicValueToIndex(realIndex, index) && storage->_list->getAtIndex(realIndex, result);
	} else if (attrib == "delete") {
		size_t realIndex = 0;
		if (!storage->_list->dynamicValueToIndex(realIndex, index))
			return false;
		if (!storage->_list->getAtIndex(realIndex, result))
			return false;

		storage->_list = storage->_list->clone();
		storage->_list->deleteAtIndex(realIndex);

		return true;
	}

	return Modifier::readAttributeIndexed(thread, result, attrib, index);
}

MiniscriptInstructionOutcome ListVariableModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	if (attrib == "count") {
		DynamicValueWriteFuncHelper<ListVariableModifier, &ListVariableModifier::scriptSetCount, true>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return VariableModifier::writeRefAttribute(thread, writeProxy, attrib);
}

MiniscriptInstructionOutcome ListVariableModifier::writeRefAttributeIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib, const DynamicValue &index) {
	ListVariableStorage *storage = static_cast<ListVariableStorage *>(_storage.get());

	if (attrib == "value") {
		size_t realIndex = 0;
		if (!storage->_list->dynamicValueToIndex(realIndex, index))
			return kMiniscriptInstructionOutcomeFailed;

		storage->_list->createWriteProxyForIndex(realIndex, writeProxy);
		writeProxy.containerList = storage->_list;
		return kMiniscriptInstructionOutcomeContinue;
	}
	return kMiniscriptInstructionOutcomeFailed;
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void ListVariableModifier::debugInspect(IDebugInspectionReport *report) const {
	VariableModifier::debugInspect(report);

	ListVariableStorage *storage = static_cast<ListVariableStorage *>(_storage.get());

	size_t listSize = storage->_list->getSize();

	for (size_t i = 0; i < listSize; i++) {
		int cardinal = i + 1;
		switch (storage->_list->getType()) {
		case DynamicValueTypes::kInteger:
			report->declareLoose(Common::String::format("[%i] = %i", cardinal, storage->_list->getInt()[i]));
			break;
		case DynamicValueTypes::kFloat:
			report->declareLoose(Common::String::format("[%i] = %g", cardinal, storage->_list->getFloat()[i]));
			break;
		case DynamicValueTypes::kPoint:
			report->declareLoose(Common::String::format("[%i] = ", cardinal) + pointToString(storage->_list->getPoint()[i]));
			break;
		case DynamicValueTypes::kIntegerRange:
			report->declareLoose(Common::String::format("[%i] = ", cardinal) + storage->_list->getIntRange()[i].toString());
			break;
		case DynamicValueTypes::kBoolean:
			report->declareLoose(Common::String::format("[%i] = %s", cardinal, storage->_list->getBool()[i] ? "true" : "false"));
			break;
		case DynamicValueTypes::kVector:
			report->declareLoose(Common::String::format("[%i] = ", cardinal) + storage->_list->getVector()[i].toString());
			break;
		case DynamicValueTypes::kLabel:
			report->declareLoose(Common::String::format("[%i] = Label?", cardinal));
			break;
		case DynamicValueTypes::kEvent:
			report->declareLoose(Common::String::format("[%i] = Event?", cardinal));
			break;
		case DynamicValueTypes::kString:
			report->declareLoose(Common::String::format("[%i] = ", cardinal) + storage->_list->getString()[i]);
			break;
		case DynamicValueTypes::kList:
			report->declareLoose(Common::String::format("[%i] = List", cardinal));
			break;
		case DynamicValueTypes::kObject: {
				RuntimeObject *obj = storage->_list->getObjectReference()[i].object.lock().get();

				if (obj)
					report->declareLoose(Common::String::format("[%i] = Object %x", cardinal, static_cast<uint>(obj->getRuntimeGUID())));
				else
					report->declareLoose(Common::String::format("[%i] = Object (Invalid)", cardinal));
			} break;
		default:
			report->declareLoose(Common::String::format("[%i] = <BAD TYPE>", cardinal));
			break;
		}
	}
}
#endif

MiniscriptInstructionOutcome ListVariableModifier::scriptSetCount(MiniscriptThread *thread, const DynamicValue &value) {
	ListVariableStorage *storage = static_cast<ListVariableStorage *>(_storage.get());

	int32 asInteger = 0;
	if (!value.roundToInt(asInteger)) {
		thread->error("Tried to set a list variable count to something other than an integer");
		return kMiniscriptInstructionOutcomeFailed;
	}

	if (asInteger < 0) {
		thread->error("Tried to set a list variable count to a negative value");
		return kMiniscriptInstructionOutcomeFailed;
	}

	size_t newSize = asInteger;
	if (newSize > storage->_list->getSize()) {
		storage->_list->expandToMinimumSize(newSize);
	} else if (newSize < storage->_list->getSize()) {
		storage->_list->truncateToSize(newSize);
	}

	return kMiniscriptInstructionOutcomeContinue;
}

Common::SharedPtr<Modifier> ListVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ListVariableModifier(*this));
}

const char *ListVariableModifier::getDefaultName() const {
	return "List Variable";
}

ListVariableStorage::ListVariableStorage() : _preferredContentType(DynamicValueTypes::kInteger), _list(new DynamicList()) {
}

Common::SharedPtr<ModifierSaveLoad> ListVariableStorage::getSaveLoad(Runtime *runtime) {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
}

Common::SharedPtr<VariableStorage> ListVariableStorage::clone() const {
	ListVariableStorage *newInstance = new ListVariableStorage();
	newInstance->_list = Common::SharedPtr<DynamicList>(new DynamicList(*_list));
	newInstance->_preferredContentType = _preferredContentType;
	return Common::SharedPtr<VariableStorage>(newInstance);
}

ListVariableStorage::SaveLoad::SaveLoad(ListVariableStorage *storage) : _storage(storage), _list(storage->_list) {
}

void ListVariableStorage::SaveLoad::commitLoad() const {
	// We don't support deserializing object references (yet?), so just leave the existing values.
	// In Obsidian at least, this doesn't matter.
	if (_list->getType() != DynamicValueTypes::kObject)
		_storage->_list = _list;
}

void ListVariableStorage::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	recursiveWriteList(_list.get(), stream);
}

bool ListVariableStorage::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
	Common::SharedPtr<DynamicList> list = recursiveReadList(stream);
	if (list) {
		_list = list;
		return true;
	} else {
		return false;
	}
}

void ListVariableStorage::SaveLoad::recursiveWriteList(DynamicList *list, Common::WriteStream *stream) {
	stream->writeUint32BE(list->getType());
	stream->writeUint32BE(list->getSize());

	size_t listSize = list->getSize();
	for (size_t i = 0; i < listSize; i++) {
		switch (list->getType()) {
		case DynamicValueTypes::kInteger:
			stream->writeSint32BE(list->getInt()[i]);
			break;
		case DynamicValueTypes::kPoint: {
				const Common::Point &pt = list->getPoint()[i];
				stream->writeSint16BE(pt.x);
				stream->writeSint16BE(pt.y);
			}
			break;
		case DynamicValueTypes::kIntegerRange: {
				const IntRange &range = list->getIntRange()[i];
				stream->writeSint32BE(range.min);
				stream->writeSint32BE(range.max);
			} break;
		case DynamicValueTypes::kFloat:
			stream->writeDoubleBE(list->getFloat()[i]);
			break;
		case DynamicValueTypes::kString: {
				const Common::String &str = list->getString()[i];
				stream->writeUint32BE(str.size());
				stream->writeString(str);
			} break;
		case DynamicValueTypes::kVector: {
				const AngleMagVector &vec = list->getVector()[i];
				stream->writeDoubleBE(vec.angleDegrees);
				stream->writeDoubleBE(vec.magnitude);
			} break;
		case DynamicValueTypes::kBoolean:
			stream->writeByte(list->getBool()[i] ? 1 : 0);
			break;
		case DynamicValueTypes::kObject:
			break;
		default:
			error("Can't figure out how to write a saved variable");
			break;
		}
	}
}

Common::SharedPtr<DynamicList> ListVariableStorage::SaveLoad::recursiveReadList(Common::ReadStream *stream) {
	Common::SharedPtr<DynamicList> list;
	list.reset(new DynamicList());

	uint32 typeCode = stream->readUint32BE();
	uint32 size = stream->readUint32BE();

	if (stream->err())
		return nullptr;

	list->forceType(static_cast<DynamicValueTypes::DynamicValueType>(typeCode));

	for (size_t i = 0; i < size; i++) {
		DynamicValue val;

		switch (typeCode) {
		case DynamicValueTypes::kInteger: {
				int32 i32 = stream->readSint32BE();
				val.setInt(i32);
			} break;
		case DynamicValueTypes::kPoint: {
				Common::Point pt;
				pt.x = stream->readSint16BE();
				pt.y = stream->readSint16BE();
				val.setPoint(pt);
			} break;
		case DynamicValueTypes::kIntegerRange: {
				IntRange range;
				range.min = stream->readSint32BE();
				range.max = stream->readSint32BE();
				val.setIntRange(range);
			} break;
		case DynamicValueTypes::kFloat: {
				double f;
				f = stream->readDoubleBE();
				val.setFloat(f);
			} break;
		case DynamicValueTypes::kString: {
				uint32 strLen = stream->readUint32BE();
				if (stream->err())
					return nullptr;

				Common::String str;
				if (strLen > 0) {
					Common::Array<char> chars;
					chars.resize(strLen);
					stream->read(&chars[0], strLen);
					str = Common::String(&chars[0], strLen);
				}
				val.setString(str);
			} break;
		case DynamicValueTypes::kVector: {
				AngleMagVector vec;
				vec.angleDegrees = stream->readDoubleBE();
				vec.magnitude = stream->readDoubleBE();
				val.setVector(vec);
			} break;
		case DynamicValueTypes::kBoolean: {
				byte b = stream->readByte();
				val.setBool(b != 0);
			} break;
		case DynamicValueTypes::kObject: {
				val.setObject(Common::WeakPtr<RuntimeObject>());
			} break;
		default:
			error("Can't figure out how to write a saved variable");
			break;
		}

		if (stream->err())
			return nullptr;

		list->setAtIndex(i, val);
	}

	return list;
}

bool SysInfoModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::SysInfoModifier &data) {
	return true;
}

bool SysInfoModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "bitdepth") {
		ColorDepthMode colorDepth = thread->getRuntime()->getFakeColorDepth();
		switch (colorDepth) {
		case kColorDepthMode1Bit:
			result.setInt(1);
			break;
		case kColorDepthMode2Bit:
			result.setInt(2);
			break;
		case kColorDepthMode4Bit:
			result.setInt(4);
			break;
		case kColorDepthMode8Bit:
			result.setInt(8);
			break;
		case kColorDepthMode16Bit:
			result.setInt(16);
			break;
		case kColorDepthMode32Bit:
			result.setInt(32);
			break;
		default:
			return false;
		}

		return true;
	} else if (attrib == "screensize") {
		uint16 width, height;

		thread->getRuntime()->getDisplayResolution(width, height);

		Common::Point hacksSize = thread->getRuntime()->getHacks().reportDisplaySize;
		if (hacksSize.x != 0)
			width = hacksSize.x;
		if (hacksSize.y != 0)
			height = hacksSize.y;

		result.setPoint(Common::Point(width, height));
		return true;
	} else if (attrib == "currentram") {
		result.setInt(256 * 1024 * 1024);
		return true;
	} else if (attrib == "architecture") {
		ProjectPlatform platform = thread->getRuntime()->getProject()->getPlatform();

		if (platform == kProjectPlatformWindows)
			result.setString("80x86");
		else if (platform == kProjectPlatformMacintosh)
			result.setString("PowerPC"); // MC680x0 for 68k
		else {
			thread->error("Couldn't resolve architecture");
			return false;
		}

		return true;
	} else if (attrib == "sysversion") {
		ProjectPlatform platform = thread->getRuntime()->getProject()->getPlatform();

		if (platform == kProjectPlatformMacintosh)
			result.setString("9.0.4");
		else if (platform == kProjectPlatformWindows)
			result.setString("4.0");	// Windows version?  MindGym checks for < 4
		else {
			thread->error("Couldn't resolve architecture");
			return false;
		}

		return true;
	} else if (attrib == "processor" || attrib == "nativecpu") {
		ProjectPlatform platform = thread->getRuntime()->getProject()->getPlatform();

		if (platform == kProjectPlatformMacintosh)
			result.setString("604");		// PowerPC 604
		else if (platform == kProjectPlatformWindows)
			result.setString("Pentium");
		else {
			thread->error("Couldn't resolve architecture");
			return false;
		}

		return true;
	}

	return false;
}


Common::SharedPtr<Modifier> SysInfoModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new SysInfoModifier(*this));
}

const char *SysInfoModifier::getDefaultName() const {
	return "SysInfo Modifier";
}

PanningModifier::PanningModifier() {
}

PanningModifier::~PanningModifier() {
}

bool PanningModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::PanningModifier &data) {
	return true;
}

bool PanningModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState PanningModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void PanningModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void PanningModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> PanningModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new PanningModifier(*this));
}

const char *PanningModifier::getDefaultName() const {
	return "Panning Modifier"; // ???
}

FadeModifier::FadeModifier() {
}

FadeModifier::~FadeModifier() {
}

bool FadeModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::FadeModifier &data) {
	return true;
}

void FadeModifier::disable(Runtime *runtime) {
}

Common::SharedPtr<Modifier> FadeModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new FadeModifier(*this));
}

const char *FadeModifier::getDefaultName() const {
	return "Fade Modifier"; // ???
}

class PrintModifierImageSupplier : public GUI::ImageAlbumImageSupplier {
public:
	PrintModifierImageSupplier(const Common::String &inputPath, bool isMacVersion);

	bool loadImageSlot(uint slot, const Graphics::Surface *&outSurface, bool &outHasPalette, Graphics::Palette &outPalette, GUI::ImageAlbumImageMetadata &outMetadata) override;
	void releaseImageSlot(uint slot) override;
	uint getNumSlots() const override;
	Common::U32String getDefaultFileNameForSlot(uint slot) const override;
	bool getFileFormatForImageSlot(uint slot, Common::FormatInfo::FormatID &outFormat) const override;
	Common::SeekableReadStream *createReadStreamForSlot(uint slot) override;

private:
	Common::String _path;

	Common::SharedPtr<Image::ImageDecoder> _decoder;
	bool _isMacVersion;
};

PrintModifierImageSupplier::PrintModifierImageSupplier(const Common::String &inputPath, bool isMacVersion) : _path(inputPath), _isMacVersion(isMacVersion) {
	if (isMacVersion)
		_decoder.reset(new Image::PICTDecoder());
	else
		_decoder.reset(new Image::BitmapDecoder());
}

bool PrintModifierImageSupplier::loadImageSlot(uint slot, const Graphics::Surface *&outSurface, bool &outHasPalette, Graphics::Palette &outPalette, GUI::ImageAlbumImageMetadata &outMetadata) {
	Common::ScopedPtr<Common::SeekableReadStream> dataStream(createReadStreamForSlot(slot));

	if (!dataStream)
		return false;

	if (!_decoder->loadStream(*dataStream)) {
		warning("Failed to decode print file");
		return false;
	}

	dataStream.reset();

	outSurface = _decoder->getSurface();
	outHasPalette = _decoder->hasPalette();

	if (_decoder->hasPalette())
		outPalette.set(_decoder->getPalette(), 0, _decoder->getPaletteColorCount());

	outMetadata = GUI::ImageAlbumImageMetadata();
	outMetadata._orientation = GUI::kImageAlbumImageOrientationLandscape;
	outMetadata._viewTransformation = GUI::kImageAlbumViewTransformationRotate90CW;

	return true;
}

void PrintModifierImageSupplier::releaseImageSlot(uint slot) {
	_decoder->destroy();
}

uint PrintModifierImageSupplier::getNumSlots() const {
	return 1;
}

Common::U32String PrintModifierImageSupplier::getDefaultFileNameForSlot(uint slot) const {
	Common::String filename = _path;

	size_t lastColonPos = filename.findLastOf(':');

	if (lastColonPos != Common::String::npos)
		filename = filename.substr(lastColonPos + 1);

	size_t lastDotPos = filename.findLastOf('.');
	if (lastDotPos != Common::String::npos)
		filename = filename.substr(0, lastDotPos);

	if (_isMacVersion)
		filename += Common::U32String(".pict");
	else
		filename += Common::U32String(".bmp");

	return filename.decode(Common::kASCII);
}

bool PrintModifierImageSupplier::getFileFormatForImageSlot(uint slot, Common::FormatInfo::FormatID &outFormat) const {
	if (slot != 0)
		return false;

	if (_isMacVersion)
		outFormat = Common::FormatInfo::kPICT;
	else
		outFormat = Common::FormatInfo::kBMP;

	return true;
}

Common::SeekableReadStream *PrintModifierImageSupplier::createReadStreamForSlot(uint slot) {
	if (slot != 0)
		return nullptr;

	size_t lastColonPos = _path.findLastOf(':');
	Common::String filename;

	if (lastColonPos == Common::String::npos)
		filename = _path;
	else
		filename = _path.substr(lastColonPos + 1);

	Common::Path path(Common::String("MPZ_MTI/") + filename);

	if (_isMacVersion) {
		// Color images have res fork data so we must load from the data fork
		return Common::MacResManager::openFileOrDataFork(path);
	} else {
		// Win versions are just files
		Common::File *f = new Common::File();

		if (!f->open(path)) {
			delete f;
			return nullptr;
		}
		return f;
	}
}

PrintModifier::PrintModifier() {
}

PrintModifier::~PrintModifier() {
}

bool PrintModifier::respondsToEvent(const Event &evt) const {
	return _executeWhen.respondsTo(evt);
}

VThreadState PrintModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_executeWhen.respondsTo(msg->getEvent())) {
		PrintModifierImageSupplier imageSupplier(_filePath, runtime->getProject()->getPlatform() == kProjectPlatformMacintosh);

		Common::ScopedPtr<GUI::Dialog> dialog(GUI::createImageAlbumDialog(_("Image Viewer"), &imageSupplier, 0));
		dialog->runModal();
	}

	return kVThreadReturn;
}

void PrintModifier::disable(Runtime *runtime) {
}

MiniscriptInstructionOutcome PrintModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	if (attrib == "showdialog") {
		// This is only ever set to "false"
		DynamicValueWriteDiscardHelper::create(writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "filepath") {
		DynamicValueWriteStringHelper::create(&_filePath, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return Modifier::writeRefAttribute(thread, writeProxy, attrib);
}

bool PrintModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::PrintModifier &data) {
	if (data.executeWhen.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	if (data.filePath.type != Data::PlugInTypeTaggedValue::kString)
		return false;

	_filePath = data.filePath.value.asString;

	if (!_executeWhen.load(data.executeWhen.value.asEvent))
		return false;

	return true;
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void PrintModifier::debugInspect(IDebugInspectionReport *report) const {
}
#endif

Common::SharedPtr<Modifier> PrintModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new PrintModifier(*this));
}

const char *PrintModifier::getDefaultName() const {
	return "Print Modifier";
}

NavigateModifier::NavigateModifier() {
}

NavigateModifier::~NavigateModifier() {
}

bool NavigateModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::NavigateModifier &data) {
	return true;
}

bool NavigateModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState NavigateModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void NavigateModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void NavigateModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> NavigateModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new NavigateModifier(*this));
}

const char *NavigateModifier::getDefaultName() const {
	return "Navigate Modifier"; // ???
}

OpenTitleModifier::OpenTitleModifier()
	: _addToReturnList(false){
}

OpenTitleModifier::~OpenTitleModifier() {
}

bool OpenTitleModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::OpenTitleModifier &data) {
	if (data.executeWhen.type != Data::PlugInTypeTaggedValue::kEvent || data.pathOrUrl.type != Data::PlugInTypeTaggedValue::kString || data.addToReturnList.type != Data::PlugInTypeTaggedValue::kInteger)
		return false;

	if (!_executeWhen.load(data.executeWhen.value.asEvent))
		return false;

	_pathOrUrl = data.pathOrUrl.value.asString;

	_addToReturnList = static_cast<bool>(data.addToReturnList.value.asInt);

	return true;
}

bool OpenTitleModifier::respondsToEvent(const Event &evt) const {
	return _executeWhen.respondsTo(evt);
}

VThreadState OpenTitleModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
#ifdef MTROPOLIS_DEBUG_ENABLE
	if (Debugger *debugger = runtime->debugGetDebugger())
		debugger->notify(kDebugSeverityWarning, "Open Title modifier was executed, which isn't implemented yet");
#endif
	return kVThreadReturn;
}

void OpenTitleModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void OpenTitleModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> OpenTitleModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new OpenTitleModifier(*this));
}

const char *OpenTitleModifier::getDefaultName() const {
	return "Open Title Modifier"; // ???
}

StandardPlugInHacks::StandardPlugInHacks() : allowGarbledListModData(false) {
}

StandardPlugIn::StandardPlugIn()
	: _cursorModifierFactory(this)
	, _sTransCtModifierFactory(this)
	, _mediaCueModifierFactory(this)
	, _objRefVarModifierFactory(this)
	, _listVarModifierFactory(this)
	, _sysInfoModifierFactory(this)
	, _panningModifierFactory(this)
	, _fadeModifierFactory(this)
	, _printModifierFactory(this)
	, _navigateModifierFactory(this)
	, _openTitleModifierFactory(this) {
}

StandardPlugIn::~StandardPlugIn() {
}

void StandardPlugIn::registerModifiers(IPlugInModifierRegistrar *registrar) const {
	registrar->registerPlugInModifier("CursorMod", &_cursorModifierFactory);
	registrar->registerPlugInModifier("STransCt", &_sTransCtModifierFactory);
	registrar->registerPlugInModifier("MediaCue", &_mediaCueModifierFactory);
	registrar->registerPlugInModifier("ObjRefP", &_objRefVarModifierFactory);
	registrar->registerPlugInModifier("ListMod", &_listVarModifierFactory);
	registrar->registerPlugInModifier("SysInfo", &_sysInfoModifierFactory);
	registrar->registerPlugInModifier("Print", &_printModifierFactory);

	registrar->registerPlugInModifier("panning", &_panningModifierFactory);
	registrar->registerPlugInModifier("fade", &_fadeModifierFactory);
	registrar->registerPlugInModifier("Navigate", &_navigateModifierFactory);
	registrar->registerPlugInModifier("OpenTitle", &_openTitleModifierFactory);
}

const StandardPlugInHacks &StandardPlugIn::getHacks() const {
	return _hacks;
}

StandardPlugInHacks &StandardPlugIn::getHacks() {
	return _hacks;
}

} // End of namespace Standard

namespace PlugIns {

Common::SharedPtr<PlugIn> createStandard() {
	return Common::SharedPtr<PlugIn>(new Standard::StandardPlugIn());
}

} // End of namespace PlugIns

} // End of namespace MTropolis
