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
#include "common/file.h"
#include "common/hash-ptr.h"
#include "common/macresman.h"
#include "common/random.h"
#include "common/substream.h"
#include "common/system.h"

#include "graphics/cursorman.h"
#include "graphics/managed_surface.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "graphics/wincursor.h"
#include "graphics/maccursor.h"
#include "graphics/macgui/macfontmanager.h"

#include "audio/mixer.h"

#include "mtropolis/runtime.h"
#include "mtropolis/coroutine_manager.h"
#include "mtropolis/coroutines.h"
#include "mtropolis/data.h"
#include "mtropolis/vthread.h"
#include "mtropolis/asset_factory.h"
#include "mtropolis/element_factory.h"
#include "mtropolis/miniscript.h"
#include "mtropolis/modifier_factory.h"
#include "mtropolis/modifiers.h"
#include "mtropolis/render.h"

namespace MTropolis {

int32 displayModeToBitDepth(ColorDepthMode displayMode) {
	switch (displayMode) {
	case kColorDepthMode1Bit:
		return 1;
	case kColorDepthMode2Bit:
		return 2;
	case kColorDepthMode4Bit:
		return 4;
	case kColorDepthMode8Bit:
		return 8;
	case kColorDepthMode16Bit:
		return 16;
	case kColorDepthMode32Bit:
		return 32;
	default:
		return 0;
	}
}

ColorDepthMode bitDepthToDisplayMode(int32 bits) {
	switch (bits) {
	case 1:
		return kColorDepthMode1Bit;
	case 2:
		return kColorDepthMode2Bit;
	case 4:
		return kColorDepthMode4Bit;
	case 8:
		return kColorDepthMode8Bit;
	case 16:
		return kColorDepthMode16Bit;
	case 32:
		return kColorDepthMode32Bit;
	default:
		return kColorDepthModeInvalid;
	}
}

class MainWindow : public Window {
public:
	MainWindow(const WindowParameters &windowParams);

	void onMouseDown(int32 x, int32 y, int mouseButton) override;
	void onMouseMove(int32 x, int32 y) override;
	void onMouseUp(int32 x, int32 y, int mouseButton) override;
	void onKeyboardEvent(const Common::EventType evtType, bool repeat, const Common::KeyState &keyEvt) override;
	void onAction(MTropolis::Actions::Action action) override;

private:
	bool _mouseButtonStates[Actions::kMouseButtonCount];
};

MainWindow::MainWindow(const WindowParameters &windowParams) : Window(windowParams) {
	for (int i = 0; i < Actions::kMouseButtonCount; i++)
		_mouseButtonStates[i] = false;
}

void MainWindow::onMouseDown(int32 x, int32 y, int mouseButton) {
	if (!_mouseButtonStates[mouseButton]) {
		_mouseButtonStates[mouseButton] = true;

		if (mouseButton == Actions::kMouseButtonLeft) {
			_runtime->queueOSEvent(Common::SharedPtr<OSEvent>(new MouseInputEvent(kOSEventTypeMouseDown, x, y, static_cast<Actions::MouseButton>(mouseButton))));
		}
	}
}

void MainWindow::onMouseMove(int32 x, int32 y) {
	_runtime->queueOSEvent(Common::SharedPtr<OSEvent>(new MouseInputEvent(kOSEventTypeMouseMove, x, y, Actions::kMouseButtonLeft)));
}

void MainWindow::onMouseUp(int32 x, int32 y, int mouseButton) {
	if (_mouseButtonStates[mouseButton]) {
		_mouseButtonStates[mouseButton] = false;

		if (mouseButton == Actions::kMouseButtonLeft) {
			_runtime->queueOSEvent(Common::SharedPtr<OSEvent>(new MouseInputEvent(kOSEventTypeMouseUp, x, y, static_cast<Actions::MouseButton>(mouseButton))));
		}
	}
}

void MainWindow::onKeyboardEvent(const Common::EventType evtType, bool repeat, const Common::KeyState &keyEvt) {
	_runtime->queueOSEvent(Common::SharedPtr<OSEvent>(new KeyboardInputEvent(kOSEventTypeKeyboard, evtType, repeat, keyEvt)));
}

void MainWindow::onAction(MTropolis::Actions::Action action) {
	_runtime->queueOSEvent(Common::SharedPtr<OSEvent>(new ActionEvent(kOSEventTypeAction, action)));
}


class ModifierInnerScopeBuilder : public IStructuralReferenceVisitor {
public:
	ModifierInnerScopeBuilder(Runtime *runtime, Modifier *modifier, ObjectLinkingScope *scope);

	void visitChildStructuralRef(Common::SharedPtr<Structural> &structural) override;
	void visitChildModifierRef(Common::SharedPtr<Modifier> &modifier) override;
	void visitWeakStructuralRef(Common::WeakPtr<Structural> &structural) override;
	void visitWeakModifierRef(Common::WeakPtr<Modifier> &modifier) override;

private:
	ObjectLinkingScope *_scope;
	Modifier *_modifier;
	Runtime *_runtime;
};

ModifierInnerScopeBuilder::ModifierInnerScopeBuilder(Runtime *runtime, Modifier *modifier, ObjectLinkingScope *scope)
	: _scope(scope), _modifier(modifier), _runtime(runtime) {
}

void ModifierInnerScopeBuilder::visitChildStructuralRef(Common::SharedPtr<Structural> &structural) {
	assert(false);
}

void ModifierInnerScopeBuilder::visitChildModifierRef(Common::SharedPtr<Modifier> &modifier) {
	uint32 oldStaticGUID = modifier->getStaticGUID();

	_runtime->instantiateIfAlias(modifier, _modifier->getSelfReference());
	_scope->addObject(oldStaticGUID, modifier->getName(), modifier);
}

void ModifierInnerScopeBuilder::visitWeakStructuralRef(Common::WeakPtr<Structural> &structural) {
}

void ModifierInnerScopeBuilder::visitWeakModifierRef(Common::WeakPtr<Modifier> &modifier) {
	// Do nothing
}

class ModifierChildMaterializer : public IStructuralReferenceVisitor {
public:
	ModifierChildMaterializer(Runtime *runtime, ObjectLinkingScope *outerScope);

	void visitChildStructuralRef(Common::SharedPtr<Structural> &structural) override;
	void visitChildModifierRef(Common::SharedPtr<Modifier> &modifier) override;
	void visitWeakStructuralRef(Common::WeakPtr<Structural> &structural) override;
	void visitWeakModifierRef(Common::WeakPtr<Modifier> &modifier) override;

private:
	Runtime *_runtime;
	ObjectLinkingScope *_outerScope;
};

ModifierChildMaterializer::ModifierChildMaterializer(Runtime *runtime, ObjectLinkingScope *outerScope)
	: _runtime(runtime), _outerScope(outerScope) {
}

void ModifierChildMaterializer::visitChildStructuralRef(Common::SharedPtr<Structural> &structural) {
	assert(false);
}

void ModifierChildMaterializer::visitChildModifierRef(Common::SharedPtr<Modifier> &modifier) {
	modifier->materialize(_runtime, _outerScope);
}

void ModifierChildMaterializer::visitWeakStructuralRef(Common::WeakPtr<Structural> &structural) {
	// Do nothing
}

void ModifierChildMaterializer::visitWeakModifierRef(Common::WeakPtr<Modifier> &modifier) {
	// Do nothing
}

class ModifierChildCloner : public IStructuralReferenceVisitor {
public:
	ModifierChildCloner(Runtime *runtime, const Common::WeakPtr<RuntimeObject> &relinkParent);

	void visitChildStructuralRef(Common::SharedPtr<Structural> &structural) override;
	void visitChildModifierRef(Common::SharedPtr<Modifier> &modifier) override;
	void visitWeakStructuralRef(Common::WeakPtr<Structural> &structural) override;
	void visitWeakModifierRef(Common::WeakPtr<Modifier> &modifier) override;

private:
	Runtime *_runtime;
	Common::WeakPtr<RuntimeObject> _relinkParent;
};

ModifierChildCloner::ModifierChildCloner(Runtime *runtime, const Common::WeakPtr<RuntimeObject> &relinkParent)
	: _runtime(runtime), _relinkParent(relinkParent) {
}

void ModifierChildCloner::visitChildStructuralRef(Common::SharedPtr<Structural> &structural) {
	assert(false);
}

void ModifierChildCloner::visitChildModifierRef(Common::SharedPtr<Modifier> &modifier) {
	uint32 oldGUID = modifier->getStaticGUID();
	modifier = modifier->shallowClone();
	assert(modifier->getStaticGUID() == oldGUID);

	(void)oldGUID;

	modifier->setSelfReference(modifier);
	modifier->setParent(_relinkParent);

	ModifierChildCloner recursiveCloner(_runtime, modifier);
	modifier->visitInternalReferences(&recursiveCloner);
}

void ModifierChildCloner::visitWeakStructuralRef(Common::WeakPtr<Structural> &structural) {
	// Do nothing
}

void ModifierChildCloner::visitWeakModifierRef(Common::WeakPtr<Modifier> &modifier) {
	// Do nothing
}

class ObjectCloner : public IStructuralReferenceVisitor {
public:
	ObjectCloner(Runtime *runtime, const Common::WeakPtr<RuntimeObject> &relinkParent, Common::HashMap<RuntimeObject *, RuntimeObject *> *objectRemaps);

	void visitChildStructuralRef(Common::SharedPtr<Structural> &structural) override;
	void visitChildModifierRef(Common::SharedPtr<Modifier> &modifier) override;
	void visitWeakStructuralRef(Common::WeakPtr<Structural> &structural) override;
	void visitWeakModifierRef(Common::WeakPtr<Modifier> &modifier) override;

private:
	Runtime *_runtime;
	Common::WeakPtr<RuntimeObject> _relinkParent;

	Common::HashMap<RuntimeObject *, RuntimeObject *> *_objectRemaps;
};

ObjectCloner::ObjectCloner(Runtime *runtime, const Common::WeakPtr<RuntimeObject> &relinkParent, Common::HashMap<RuntimeObject *, RuntimeObject *> *objectRemaps)
	: _runtime(runtime), _relinkParent(relinkParent), _objectRemaps(objectRemaps) {
}

void ObjectCloner::visitChildStructuralRef(Common::SharedPtr<Structural> &structuralRef) {
	uint32 oldGUID = structuralRef->getStaticGUID();
	Common::SharedPtr<Structural> cloned = structuralRef->shallowClone();
	assert(cloned->getStaticGUID() == oldGUID);

	(void)oldGUID;

	if (_objectRemaps)
		(*_objectRemaps)[structuralRef.get()] = cloned.get();

	assert(!_relinkParent.expired() && _relinkParent.lock()->isStructural());

	cloned->setSelfReference(cloned);
	cloned->setRuntimeGUID(_runtime->allocateRuntimeGUID());
	cloned->setParent(static_cast<Structural *>(_relinkParent.lock().get()));

	ObjectCloner recursiveCloner(_runtime, cloned, _objectRemaps);
	cloned->visitInternalReferences(&recursiveCloner);

	structuralRef = cloned;
}

void ObjectCloner::visitChildModifierRef(Common::SharedPtr<Modifier> &modifierRef) {
	uint32 oldGUID = modifierRef->getStaticGUID();
	Common::SharedPtr<Modifier> cloned = modifierRef->shallowClone();
	assert(cloned->getStaticGUID() == oldGUID);

	(void)oldGUID;

	if (_objectRemaps)
		(*_objectRemaps)[modifierRef.get()] = cloned.get();

	cloned->setSelfReference(cloned);
	cloned->setRuntimeGUID(_runtime->allocateRuntimeGUID());
	cloned->setParent(_relinkParent);

	ObjectCloner recursiveCloner(_runtime, cloned, _objectRemaps);
	cloned->visitInternalReferences(&recursiveCloner);

	modifierRef = cloned;
}

void ObjectCloner::visitWeakStructuralRef(Common::WeakPtr<Structural> &structural) {
	// Do nothing
}

void ObjectCloner::visitWeakModifierRef(Common::WeakPtr<Modifier> &modifier) {
	// Do nothing
}



class ObjectRefRemapper : public IStructuralReferenceVisitor {
public:
	explicit ObjectRefRemapper(const Common::HashMap<RuntimeObject *, RuntimeObject *> &objectRemaps);

	void visitChildStructuralRef(Common::SharedPtr<Structural> &structural) override;
	void visitChildModifierRef(Common::SharedPtr<Modifier> &modifier) override;
	void visitWeakStructuralRef(Common::WeakPtr<Structural> &structural) override;
	void visitWeakModifierRef(Common::WeakPtr<Modifier> &modifier) override;

private:
	const Common::HashMap<RuntimeObject *, RuntimeObject *> &_objectRemaps;
};

ObjectRefRemapper::ObjectRefRemapper(const Common::HashMap<RuntimeObject *, RuntimeObject *> &objectRemaps) : _objectRemaps(objectRemaps) {
}

void ObjectRefRemapper::visitChildStructuralRef(Common::SharedPtr<Structural> &structural) {
	RuntimeObject *obj = structural.get();
	if (obj) {
		Common::HashMap<RuntimeObject *, RuntimeObject *> ::const_iterator it = _objectRemaps.find(obj);
		if (it != _objectRemaps.end())
			structural = it->_value->getSelfReference().lock().staticCast<Structural>();
	}
}

void ObjectRefRemapper::visitChildModifierRef(Common::SharedPtr<Modifier> &modifier) {
	RuntimeObject *obj = modifier.get();
	if (obj) {
		Common::HashMap<RuntimeObject *, RuntimeObject *>::const_iterator it = _objectRemaps.find(obj);
		if (it != _objectRemaps.end())
			modifier = it->_value->getSelfReference().lock().staticCast<Modifier>();
	}
}

void ObjectRefRemapper::visitWeakStructuralRef(Common::WeakPtr<Structural> &structural) {
	RuntimeObject *obj = structural.lock().get();
	if (obj) {
		Common::HashMap<RuntimeObject *, RuntimeObject *>::const_iterator it = _objectRemaps.find(obj);
		if (it != _objectRemaps.end())
			structural = it->_value->getSelfReference().staticCast<Structural>();
	}
}

void ObjectRefRemapper::visitWeakModifierRef(Common::WeakPtr<Modifier> &modifier) {
	RuntimeObject *obj = modifier.lock().get();
	if (obj) {
		Common::HashMap<RuntimeObject *, RuntimeObject *>::const_iterator it = _objectRemaps.find(obj);
		if (it != _objectRemaps.end())
			modifier = it->_value->getSelfReference().staticCast<Modifier>();
	}
}

char invariantToLower(char c) {
	if (c >= 'A' && c <= 'Z')
		return static_cast<char>(c - 'A' + 'a');
	return c;
}

Common::String toCaseInsensitive(const Common::String &str) {
	uint strLen = str.size();
	if (strLen == 0)
		return str;

	// TODO: Figure out how this is supposed to behave with respect to non-ASCII characters
	Common::Array<char> lowered;
	lowered.resize(strLen);

	for (uint i = 0; i < strLen; i++)
		lowered[i] = invariantToLower(str[i]);

	return Common::String(&lowered[0], strLen);
}

bool caseInsensitiveEqual(const Common::String& str1, const Common::String& str2) {
	size_t length1 = str1.size();
	size_t length2 = str2.size();
	if (length1 != length2)
		return false;

	for (size_t i = 0; i < length1; i++) {
		if (invariantToLower(str1[i]) != invariantToLower(str2[i]))
			return false;
	}

	return true;
}

size_t caseInsensitiveFind(const Common::String &strToSearch, const Common::String &stringToFind) {
	if (stringToFind.size() > strToSearch.size())
		return Common::String::npos;

	size_t lastValidStart = strToSearch.size() - stringToFind.size();
	size_t searchLength = stringToFind.size();

	for (size_t startIndex = 0; startIndex <= lastValidStart; startIndex++) {
		bool matches = true;
		for (size_t i = 0; i < searchLength; i++) {
			char ca = strToSearch[i + startIndex];
			char cb = stringToFind[i];
			if (ca != cb && invariantToLower(ca) != invariantToLower(cb)) {
				matches = false;
				break;
			}
		}

		if (matches)
			return startIndex;
	}

	return Common::String::npos;
}

bool SceneTransitionTypes::loadFromData(SceneTransitionType &transType, int32 data) {
	switch (data) {
	case Data::SceneTransitionTypes::kNone:
		transType = kNone;
		break;
	case Data::SceneTransitionTypes::kPatternDissolve:
		transType = kPatternDissolve;
		break;
	case Data::SceneTransitionTypes::kRandomDissolve:
		transType = kRandomDissolve;
		break;
	case Data::SceneTransitionTypes::kFade:
		transType = kFade;
		break;
	case Data::SceneTransitionTypes::kSlide:
		transType = kSlide;
		break;
	case Data::SceneTransitionTypes::kPush:
		transType = kPush;
		break;
	case Data::SceneTransitionTypes::kZoom:
		transType = kZoom;
		break;
	case Data::SceneTransitionTypes::kWipe:
		transType = kWipe;
		break;
	default:
		return false;
	}

	return true;
}


bool SceneTransitionDirections::loadFromData(SceneTransitionDirection &transDir, int32 data) {
	switch (data) {
	case Data::SceneTransitionDirections::kUp:
		transDir = kUp;
		break;
	case Data::SceneTransitionDirections::kDown:
		transDir = kDown;
		break;
	case Data::SceneTransitionDirections::kLeft:
		transDir = kLeft;
		break;
	case Data::SceneTransitionDirections::kRight:
		transDir = kRight;
		break;
	default:
		return false;
	}

	return true;
}

bool EventIDs::isCommand(EventID eventID) {
	switch (eventID) {
	case kElementShow:
	case kElementHide:
	case kElementSelect:
	case kElementDeselect:
	case kElementToggleSelect:
	case kElementEnableEdit:
	case kElementDisableEdit:
	case kElementUpdatedCalculated:
	case kElementScrollUp:
	case kElementScrollDown:
	case kElementScrollLeft:
	case kElementScrollRight:
	case kPreloadMedia:
	case kFlushMedia:
	case kPrerollMedia:
	case kClone:
	case kKill:
	case kPlay:
	case kStop:
	case kPause:
	case kUnpause:
	case kTogglePause:
	case kCloseProject:
	case kFlushAllMedia:
	case kAttribGet:
	case kAttribSet:
		return true;
	default:
		return false;
	}
}


MiniscriptInstructionOutcome pointWriteRefAttrib(Common::Point &point, MiniscriptThread *thread, DynamicValueWriteProxy &proxy, const Common::String &attrib) {
	if (attrib == "x") {
		DynamicValueWriteIntegerHelper<int16>::create(&point.x, proxy);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "y") {
		DynamicValueWriteIntegerHelper<int16>::create(&point.y, proxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return kMiniscriptInstructionOutcomeFailed;
}

Common::String pointToString(const Common::Point &point) {
	return Common::String::format("(%i,%i)", point.x, point.y);
}

IntRange::IntRange() : min(0), max(0) {
}

IntRange::IntRange(int32 pmin, int32 pmax) : min(pmin), max(pmax) {
}

bool IntRange::load(const Data::IntRange &range) {
	max = range.max;
	min = range.min;

	return true;
}

MiniscriptInstructionOutcome IntRange::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, const Common::String &attrib) {
	if (attrib == "start") {
		DynamicValueWriteIntegerHelper<int32>::create(&min, proxy);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "end") {
		DynamicValueWriteIntegerHelper<int32>::create(&max, proxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	thread->error("Couldn't reference int range attribute '" + attrib + "'");
	return kMiniscriptInstructionOutcomeFailed;
}

Common::String IntRange::toString() const {
	return Common::String::format("(%i thru %i)", min, max);
}

Label::Label() : superGroupID(0), id(0) {
}

Label::Label(int32 psuperGroupID, int32 pid) : superGroupID(psuperGroupID), id(pid) {
}

bool Label::load(const Data::Label &label) {
	id = label.labelID;
	superGroupID = label.superGroupID;

	return true;
}

bool ColorRGB8::load(const Data::ColorRGB16 &color) {
	this->r = (color.red * 0xff * 2 + 1) / (0xffff * 2);
	this->g = (color.green * 0xff * 2 + 1) / (0xffff * 2);
	this->b = (color.blue * 0xff * 2 + 1) / (0xffff * 2);

	return true;
}

ColorRGB8::ColorRGB8() : r(0), g(0), b(0) {
}

ColorRGB8::ColorRGB8(uint8 pr, uint8 pg, uint8 pb) : r(pr), g(pg), b(pb) {
}


MessageFlags::MessageFlags() : relay(true), cascade(true), immediate(true) {
}

DynamicValueWriteProxyPOD DynamicValueWriteProxyPOD::createDefault() {
	DynamicValueWriteProxyPOD proxy;
	proxy.ifc = nullptr;
	proxy.objectRef = nullptr;
	proxy.ptrOrOffset = 0;
	return proxy;
}

DynamicValueWriteProxy::DynamicValueWriteProxy() : pod(DynamicValueWriteProxyPOD::createDefault()) {
}

Common::Point Point16POD::toScummVMPoint() const {
	return Common::Point(x, y);
}

DynamicListContainerBase::~DynamicListContainerBase() {
}

void DynamicListDefaultSetter::defaultSet(int32 &value) {
	value = 0;
}

void DynamicListDefaultSetter::defaultSet(double &value) {
	value = 0.0;
}

void DynamicListDefaultSetter::defaultSet(Common::Point &value) {
	value.x = 0;
	value.y = 0;
}

void DynamicListDefaultSetter::defaultSet(IntRange &value) {
	value.min = 0;
	value.max = 0;
}

void DynamicListDefaultSetter::defaultSet(bool &value) {
	value = false;
}

void DynamicListDefaultSetter::defaultSet(AngleMagVector &value) {
	value.angleDegrees = 0.0;
	value.magnitude = 0.0;
}

void DynamicListDefaultSetter::defaultSet(Label &value) {
	value.id = 0;
	value.superGroupID = 0;
}

void DynamicListDefaultSetter::defaultSet(Event &value) {
	value.eventType = EventIDs::EventID::kNothing;
	value.eventInfo = 0;
}

void DynamicListDefaultSetter::defaultSet(Common::String &value) {
}

void DynamicListDefaultSetter::defaultSet(Common::SharedPtr<DynamicList> &value) {
}

void DynamicListDefaultSetter::defaultSet(ObjectReference &value) {
}

bool DynamicListValueImporter::importValue(const DynamicValue &dynValue, const int32 *&outPtr) {
	if (dynValue.getType() != DynamicValueTypes::kInteger)
		return false;
	outPtr = &dynValue.getInt();
	return true;
}

bool DynamicListValueImporter::importValue(const DynamicValue &dynValue, const double *&outPtr) {
	if (dynValue.getType() != DynamicValueTypes::kFloat)
		return false;
	outPtr = &dynValue.getFloat();
	return true;
}

bool DynamicListValueImporter::importValue(const DynamicValue &dynValue, const Common::Point *&outPtr) {
	if (dynValue.getType() != DynamicValueTypes::kPoint)
		return false;
	outPtr = &dynValue.getPoint();
	return true;
}

bool DynamicListValueImporter::importValue(const DynamicValue &dynValue, const IntRange *&outPtr) {
	if (dynValue.getType() != DynamicValueTypes::kIntegerRange)
		return false;
	outPtr = &dynValue.getIntRange();
	return true;
}

bool DynamicListValueImporter::importValue(const DynamicValue &dynValue, const bool *&outPtr) {
	if (dynValue.getType() != DynamicValueTypes::kBoolean)
		return false;
	outPtr = &dynValue.getBool();
	return true;
}

bool DynamicListValueImporter::importValue(const DynamicValue &dynValue, const AngleMagVector *&outPtr) {
	if (dynValue.getType() != DynamicValueTypes::kVector)
		return false;
	outPtr = &dynValue.getVector();
	return true;
}

bool DynamicListValueImporter::importValue(const DynamicValue &dynValue, const Label *&outPtr) {
	if (dynValue.getType() != DynamicValueTypes::kLabel)
		return false;
	outPtr = &dynValue.getLabel();
	return true;
}

bool DynamicListValueImporter::importValue(const DynamicValue &dynValue, const Event *&outPtr) {
	if (dynValue.getType() != DynamicValueTypes::kEvent)
		return false;
	outPtr = &dynValue.getEvent();
	return true;
}

bool DynamicListValueImporter::importValue(const DynamicValue &dynValue, const Common::String *&outPtr) {
	if (dynValue.getType() != DynamicValueTypes::kString)
		return false;
	outPtr = &dynValue.getString();
	return true;
}

bool DynamicListValueImporter::importValue(const DynamicValue &dynValue, const Common::SharedPtr<DynamicList> *&outPtr) {
	if (dynValue.getType() != DynamicValueTypes::kList)
		return false;
	outPtr = &dynValue.getList();
	return true;
}

bool DynamicListValueImporter::importValue(const DynamicValue &dynValue, const ObjectReference *&outPtr) {
	if (dynValue.getType() != DynamicValueTypes::kObject)
		return false;
	outPtr = &dynValue.getObject();
	return true;
}


void DynamicListValueExporter::exportValue(DynamicValue &dynValue, const int32 &value) {
	dynValue.setInt(value);
}

void DynamicListValueExporter::exportValue(DynamicValue &dynValue, const double &value) {
	dynValue.setFloat(value);
}

void DynamicListValueExporter::exportValue(DynamicValue &dynValue, const Common::Point &value) {
	dynValue.setPoint(value);
}

void DynamicListValueExporter::exportValue(DynamicValue &dynValue, const IntRange &value) {
	dynValue.setIntRange(value);
}

void DynamicListValueExporter::exportValue(DynamicValue &dynValue, const bool &value) {
	dynValue.setBool(value);
}

void DynamicListValueExporter::exportValue(DynamicValue &dynValue, const AngleMagVector &value) {
	dynValue.setVector(value);
}

void DynamicListValueExporter::exportValue(DynamicValue &dynValue, const Label &value) {
	dynValue.setLabel(value);
}

void DynamicListValueExporter::exportValue(DynamicValue &dynValue, const Event &value) {
	dynValue.setEvent(value);
}

void DynamicListValueExporter::exportValue(DynamicValue &dynValue, const Common::String &value) {
	dynValue.setString(value);
}

void DynamicListValueExporter::exportValue(DynamicValue &dynValue, const Common::SharedPtr<DynamicList> &value) {
	dynValue.setList(value);
}

void DynamicListValueExporter::exportValue(DynamicValue &dynValue, const ObjectReference &value) {
	dynValue.setObject(value);
}

DynamicListContainer<void>::DynamicListContainer() : _size(0) {
}

bool DynamicListContainer<void>::setAtIndex(size_t index, const DynamicValue &dynValue) {
	return true;
}

void DynamicListContainer<void>::truncateToSize(size_t sz) {
}

bool DynamicListContainer<void>::expandToMinimumSize(size_t sz) {
	return false;
}

bool DynamicListContainer<void>::getAtIndex(size_t index, DynamicValue &dynValue) const {
	dynValue.clear();
	return true;
}

void DynamicListContainer<void>::setFrom(const DynamicListContainerBase &other) {
	_size = other.getSize(); // ... the only thing we have anyway...
}

const void *DynamicListContainer<void>::getConstArrayPtr() const {
	return nullptr;
}

void *DynamicListContainer<void>::getArrayPtr() {
	return nullptr;
}

size_t DynamicListContainer<void>::getSize() const {
	return _size;
}

bool DynamicListContainer<void>::compareEqual(const DynamicListContainerBase &other) const {
	return true;
}

DynamicListContainerBase *DynamicListContainer<void>::clone() const {
	return new DynamicListContainer<void>(*this);
}
DynamicList::DynamicList() : _type(DynamicValueTypes::kUnspecified), _container(nullptr) {
}

DynamicList::DynamicList(const DynamicList &other) : _type(DynamicValueTypes::kUnspecified), _container(nullptr) {
	initFromOther(other);
}

DynamicList::~DynamicList() {
	destroyContainer();
}

DynamicValueTypes::DynamicValueType DynamicList::getType() const {
	return _type;
}

const Common::Array<int32> &DynamicList::getInt() const {
	assert(_type == DynamicValueTypes::kInteger);
	return *static_cast<const Common::Array<int32> *>(_container->getConstArrayPtr());
}

const Common::Array<double> &DynamicList::getFloat() const {
	assert(_type == DynamicValueTypes::kFloat);
	return *static_cast<const Common::Array<double> *>(_container->getConstArrayPtr());
}

const Common::Array<Common::Point> &DynamicList::getPoint() const {
	assert(_type == DynamicValueTypes::kPoint);
	return *static_cast<const Common::Array<Common::Point> *>(_container->getConstArrayPtr());
}

const Common::Array<IntRange> &DynamicList::getIntRange() const {
	assert(_type == DynamicValueTypes::kIntegerRange);
	return *static_cast<const Common::Array<IntRange> *>(_container->getConstArrayPtr());
}

const Common::Array<AngleMagVector> &DynamicList::getVector() const {
	assert(_type == DynamicValueTypes::kVector);
	return *static_cast<const Common::Array<AngleMagVector> *>(_container->getConstArrayPtr());
}

const Common::Array<Label> &DynamicList::getLabel() const {
	assert(_type == DynamicValueTypes::kLabel);
	return *static_cast<const Common::Array<Label> *>(_container->getConstArrayPtr());
}

const Common::Array<Event> &DynamicList::getEvent() const {
	assert(_type == DynamicValueTypes::kEvent);
	return *static_cast<const Common::Array<Event> *>(_container->getConstArrayPtr());
}

const Common::Array<Common::String> &DynamicList::getString() const {
	assert(_type == DynamicValueTypes::kString);
	return *static_cast<const Common::Array<Common::String> *>(_container->getConstArrayPtr());
}

const Common::Array<bool> &DynamicList::getBool() const {
	assert(_type == DynamicValueTypes::kBoolean);
	return *static_cast<const Common::Array<bool> *>(_container->getConstArrayPtr());
}

const Common::Array<Common::SharedPtr<DynamicList> > &DynamicList::getList() const {
	assert(_type == DynamicValueTypes::kList);
	return *static_cast<const Common::Array<Common::SharedPtr<DynamicList> > *>(_container->getConstArrayPtr());
}

const Common::Array<ObjectReference> &DynamicList::getObjectReference() const {
	assert(_type == DynamicValueTypes::kObject);
	return *static_cast<const Common::Array<ObjectReference> *>(_container->getConstArrayPtr());
}

Common::Array<int32> &DynamicList::getInt() {
	assert(_type == DynamicValueTypes::kInteger);
	return *static_cast<Common::Array<int32> *>(_container->getArrayPtr());
}

Common::Array<double> &DynamicList::getFloat() {
	assert(_type == DynamicValueTypes::kFloat);
	return *static_cast<Common::Array<double> *>(_container->getArrayPtr());
}

Common::Array<Common::Point> &DynamicList::getPoint() {
	assert(_type == DynamicValueTypes::kPoint);
	return *static_cast<Common::Array<Common::Point> *>(_container->getArrayPtr());
}

Common::Array<IntRange> &DynamicList::getIntRange() {
	assert(_type == DynamicValueTypes::kIntegerRange);
	return *static_cast<Common::Array<IntRange> *>(_container->getArrayPtr());
}

Common::Array<AngleMagVector> &DynamicList::getVector() {
	assert(_type == DynamicValueTypes::kVector);
	return *static_cast<Common::Array<AngleMagVector> *>(_container->getArrayPtr());
}

Common::Array<Label> &DynamicList::getLabel() {
	assert(_type == DynamicValueTypes::kLabel);
	return *static_cast<Common::Array<Label> *>(_container->getArrayPtr());
}

Common::Array<Event> &DynamicList::getEvent() {
	assert(_type == DynamicValueTypes::kEvent);
	return *static_cast<Common::Array<Event> *>(_container->getArrayPtr());
}

Common::Array<Common::String> &DynamicList::getString() {
	assert(_type == DynamicValueTypes::kString);
	return *static_cast<Common::Array<Common::String> *>(_container->getArrayPtr());
}

Common::Array<bool> &DynamicList::getBool() {
	assert(_type == DynamicValueTypes::kBoolean);
	return *static_cast<Common::Array<bool> *>(_container->getArrayPtr());
}

Common::Array<Common::SharedPtr<DynamicList> > &DynamicList::getList() {
	assert(_type == DynamicValueTypes::kList);
	return *static_cast<Common::Array<Common::SharedPtr<DynamicList> > *>(_container->getArrayPtr());
}

Common::Array<ObjectReference> &DynamicList::getObjectReference() {
	assert(_type == DynamicValueTypes::kObject);
	return *static_cast<Common::Array<ObjectReference> *>(_container->getArrayPtr());
}

bool DynamicList::setAtIndex(size_t index, const DynamicValue &value) {
	if (_type != value.getType()) {
		if (_container) {
			DynamicValue converted;
			if (!value.convertToType(_type, converted))
				return false;
			return setAtIndex(index, converted);
		} else {
			createContainerAndSetType(value.getType());
			return _container->setAtIndex(index, value);
		}
	} else {
		return _container->setAtIndex(index, value);
	}
}

void DynamicList::deleteAtIndex(size_t index) {
	if (_container != nullptr) {
		size_t size = _container->getSize();
		if (index < _container->getSize()) {
			for (size_t i = index + 1; i < size; i++) {
				DynamicValue valueToMove;
				_container->getAtIndex(i, valueToMove);
				_container->setAtIndex(i - 1, valueToMove);
			}

			_container->truncateToSize(size - 1);
		}
	}
}

void DynamicList::truncateToSize(size_t sz) {
	if (_container)
		_container->truncateToSize(sz);
}

void DynamicList::expandToMinimumSize(size_t sz) {
	if (_container)
		_container->expandToMinimumSize(sz);
}

bool DynamicList::getAtIndex(size_t index, DynamicValue &value) const {
	if (_container == nullptr || index >= _container->getSize())
		return false;

	return _container->getAtIndex(index, value);
}

size_t DynamicList::getSize() const {
	if (!_container)
		return 0;
	else
		return _container->getSize();
}

void DynamicList::forceType(DynamicValueTypes::DynamicValueType type) {
	if (_type != type) {
		destroyContainer();
		createContainerAndSetType(type);
	}
}

bool DynamicList::dynamicValueToIndex(size_t &outIndex, const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kFloat) {
		double rounded = floor(value.getFloat() + 0.5);
		if (!isfinite(rounded) || rounded < 1.0 || rounded > 0xffffffffu)
			return false;

		outIndex = static_cast<size_t>(rounded - 1.0);
	} else if (value.getType() == DynamicValueTypes::kInteger) {
		int32 i = value.getInt();
		if (i < 1)
			return false;
		outIndex = static_cast<size_t>(i - 1);
	}
	return true;
}

DynamicList &DynamicList::operator=(const DynamicList &other) {
	if (this != &other) {
		destroyContainer();
		initFromOther(other);
	}

	return *this;
}

bool DynamicList::operator==(const DynamicList &other) const {
	if (this == &other)
		return true;

	if (_type != other._type)
		return false;

	if (_container == nullptr)
		return other._container == nullptr;

	if (other._container == nullptr)
		return false; // (_container == nullptr)

	return _container->compareEqual(*other._container);
}

void DynamicList::swap(DynamicList &other) {
	if (this == &other)
		return;

	DynamicValueTypes::DynamicValueType tempType = _type;
	_type = other._type;
	other._type = tempType;

	DynamicListContainerBase *tempContainer = _container;
	_container = other._container;
	other._container = tempContainer;
}

Common::SharedPtr<DynamicList> DynamicList::clone() const {
	Common::SharedPtr<DynamicList> clonedList(new DynamicList());

	if (_container)
		clonedList->_container = _container->clone();
	clonedList->_type = _type;

	return clonedList;
}

void DynamicList::createWriteProxyForIndex(size_t index, DynamicValueWriteProxy &proxy) {
	proxy.pod.ifc = DynamicValueWriteInterfaceGlue<WriteProxyInterface>::getInstance();
	proxy.pod.objectRef = this;
	proxy.pod.ptrOrOffset = index;
}

bool DynamicList::createContainerAndSetType(DynamicValueTypes::DynamicValueType type) {
	switch (type) {
	case DynamicValueTypes::kInvalid:
		// FIXME: Set _container as per kNull case?
		break;
	case DynamicValueTypes::kNull:
		_container = new DynamicListContainer<void>();
		break;
	case DynamicValueTypes::kInteger:
		_container = new DynamicListContainer<int32>();
		break;
	case DynamicValueTypes::kFloat:
		_container = new DynamicListContainer<double>();
		break;
	case DynamicValueTypes::kPoint:
		_container = new DynamicListContainer<Common::Point>();
		break;
	case DynamicValueTypes::kIntegerRange:
		_container = new DynamicListContainer<IntRange>();
		break;
	case DynamicValueTypes::kBoolean:
		_container = new DynamicListContainer<bool>();
		break;
	case DynamicValueTypes::kVector:
		_container = new DynamicListContainer<AngleMagVector>();
		break;
	case DynamicValueTypes::kLabel:
		_container = new DynamicListContainer<Label>();
		break;
	case DynamicValueTypes::kEvent:
		_container = new DynamicListContainer<Event>();
		break;
	case DynamicValueTypes::kString:
		_container = new DynamicListContainer<Common::String>();
		break;
	case DynamicValueTypes::kList:
		_container = new DynamicListContainer<Common::SharedPtr<DynamicList> >();
		break;
	case DynamicValueTypes::kObject:
		_container = new DynamicListContainer<ObjectReference>();
		break;
	case DynamicValueTypes::kWriteProxy:
		// FIXME
		break;
	case DynamicValueTypes::kUnspecified:
		// FIXME: Set _container as per kNull case?
		break;
	default:
		error("List was set to an invalid type");
	}

	_type = type;

	return true;
}

void DynamicList::destroyContainer() {
	if (_container)
		delete _container;
	_container = nullptr;
	_type = DynamicValueTypes::kUnspecified;
}

void DynamicList::initFromOther(const DynamicList &other) {
	assert(_container == nullptr);
	assert(_type == DynamicValueTypes::kUnspecified);

	if (other._type != DynamicValueTypes::kUnspecified) {
		createContainerAndSetType(other._type);
		_container->setFrom(*other._container);
	}
}

MiniscriptInstructionOutcome DynamicList::WriteProxyInterface::write(MiniscriptThread *thread, const DynamicValue &value, void *objectRef, uintptr ptrOrOffset) {
	if (!static_cast<DynamicList *>(objectRef)->setAtIndex(ptrOrOffset, value))
		return kMiniscriptInstructionOutcomeFailed;
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome DynamicList::WriteProxyInterface::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) {
	DynamicList *list = static_cast<DynamicList *>(objectRef);

	if (ptrOrOffset >= list->getSize()) {
		// Only direct sets of elements may change the size of a list.  Accessing attributes of elements that aren't set is an error.
		thread->error("List attrib write dereference was out of bounds");
		return kMiniscriptInstructionOutcomeFailed;
	}

	switch (list->getType()) {
	case DynamicValueTypes::kPoint:
		return pointWriteRefAttrib(list->getPoint()[ptrOrOffset], thread, proxy, attrib);
	case DynamicValueTypes::kIntegerRange:
		return list->getIntRange()[ptrOrOffset].refAttrib(thread, proxy, attrib);
	case DynamicValueTypes::kVector:
		return list->getVector()[ptrOrOffset].refAttrib(thread, proxy, attrib);
	case DynamicValueTypes::kObject: {
			Common::SharedPtr<RuntimeObject> obj = list->getObjectReference()[ptrOrOffset].object.lock();
			proxy.containerList.reset();
			if (!obj) {
				thread->error("Attempted to reference an attribute of an invalid object reference");
				return kMiniscriptInstructionOutcomeFailed;
			}

			return obj->writeRefAttribute(thread, proxy, attrib);
		} break;
	default:
		thread->error("Couldn't reference an attribute of a list element");
		return kMiniscriptInstructionOutcomeFailed;
	}
}

MiniscriptInstructionOutcome DynamicList::WriteProxyInterface::refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) {
	DynamicList *list = static_cast<DynamicList *>(objectRef);
	switch (list->getType()) {
	case DynamicValueTypes::kList: {
			if (list->getSize() <= ptrOrOffset)
				return kMiniscriptInstructionOutcomeFailed;

			Common::SharedPtr<DynamicList> subList = list->getList()[ptrOrOffset];

			size_t subIndex = 0;
			if (!subList->dynamicValueToIndex(subIndex, index))
				return kMiniscriptInstructionOutcomeFailed;

			subList->createWriteProxyForIndex(subIndex, proxy);
			proxy.containerList = subList;
			return kMiniscriptInstructionOutcomeContinue;
		} break;
	case DynamicValueTypes::kObject: {
			if (list->getSize() <= ptrOrOffset)
				return kMiniscriptInstructionOutcomeFailed;

			Common::SharedPtr<RuntimeObject> obj = list->getObjectReference()[ptrOrOffset].object.lock();
			proxy.containerList.reset();
			if (!obj && !obj->writeRefAttributeIndexed(thread, proxy, attrib, index))
				return kMiniscriptInstructionOutcomeFailed;

			return kMiniscriptInstructionOutcomeContinue;
		} break;
	default:
		return kMiniscriptInstructionOutcomeFailed;
	}

	return kMiniscriptInstructionOutcomeFailed;
}

DynamicValue::ValueUnion::ValueUnion() : asUnset(0) {
}

DynamicValue::ValueUnion::~ValueUnion() {
}

template<class T, T(DynamicValue::ValueUnion::*TMember)>
void DynamicValue::ValueUnion::construct(const T &value) {
	T *field = &(this->*TMember);
	new (field) T(value);
}

template<class T, T(DynamicValue::ValueUnion::*TMember)>
void DynamicValue::ValueUnion::construct(T &&value) {
	T *field = &(this->*TMember);
	new (field) T(static_cast<T&&>(value));
}

template<class T, T(DynamicValue::ValueUnion::*TMember)>
void DynamicValue::ValueUnion::assign(const T &value) {
	T *field = &(this->*TMember);
	*field = value;
}

template<class T, T(DynamicValue::ValueUnion::*TMember)>
void DynamicValue::ValueUnion::assign(T &&value) {
	T *field = &(this->*TMember);
	*field = static_cast<T &&>(value);
}

template<class T, T(DynamicValue::ValueUnion::*TMember)>
void DynamicValue::ValueUnion::destruct() {
	T *field = &(this->*TMember);
	field->~T();
}

DynamicValue::DynamicValue() : _type(DynamicValueTypes::kNull) {
}

DynamicValue::DynamicValue(const DynamicValue &other) : _type(DynamicValueTypes::kNull) {
	setFromOther(other);
}

DynamicValue::~DynamicValue() {
	clear();
}

bool DynamicValue::loadConstant(const Data::InternalTypeTaggedValue &data, const Common::String &varString) {
	clear();

	switch (data.type) {
	case Data::InternalTypeTaggedValue::kNull:
		_type = DynamicValueTypes::kNull;
		break;
	case Data::InternalTypeTaggedValue::kInteger:
		_type = DynamicValueTypes::kInteger;
		_value.construct<int32, &ValueUnion::asInt>(data.value.asInteger);
		break;
	case Data::InternalTypeTaggedValue::kString:
		_type = DynamicValueTypes::kString;
		_value.construct<Common::String, &ValueUnion::asString>(varString);
		break;
	case Data::InternalTypeTaggedValue::kPoint:
		_type = DynamicValueTypes::kPoint;
		_value.construct<Common::Point, &ValueUnion::asPoint>(Common::Point(data.value.asPoint.x, data.value.asPoint.y));
		break;
	case Data::InternalTypeTaggedValue::kIntegerRange:
		_type = DynamicValueTypes::kIntegerRange;
		_value.construct<IntRange, &ValueUnion::asIntRange>(IntRange(0, 0));
		if (!_value.asIntRange.load(data.value.asIntegerRange))
			return false;
		break;
	case Data::InternalTypeTaggedValue::kFloat:
		_type = DynamicValueTypes::kFloat;
		_value.construct<double, &ValueUnion::asFloat>(data.value.asFloat.toXPFloat().toDouble());
		break;
	case Data::InternalTypeTaggedValue::kBool:
		_type = DynamicValueTypes::kBoolean;
		_value.construct<bool, &ValueUnion::asBool>(data.value.asBool != 0);
		break;
	case Data::InternalTypeTaggedValue::kLabel:
		_type = DynamicValueTypes::kLabel;
		_value.construct<Label, &ValueUnion::asLabel>(Label());
		if (!_value.asLabel.load(data.value.asLabel))
			return false;
		break;
	default:
		assert(false);
		return false;
	}

	return true;
}

bool DynamicValue::loadConstant(const Data::PlugInTypeTaggedValue &data) {
	clear();

	switch (data.type) {
	case Data::PlugInTypeTaggedValue::kNull:
		_type = DynamicValueTypes::kNull;
		break;
	case Data::PlugInTypeTaggedValue::kInteger:
		_type = DynamicValueTypes::kInteger;
		_value.construct<int32, &ValueUnion::asInt>(data.value.asInt);
		break;
	case Data::PlugInTypeTaggedValue::kIntegerRange:
		_type = DynamicValueTypes::kIntegerRange;
		_value.construct<IntRange, &ValueUnion::asIntRange>(IntRange());
		if (!_value.asIntRange.load(data.value.asIntRange))
			return false;
		break;
	case Data::PlugInTypeTaggedValue::kFloat:
		_type = DynamicValueTypes::kFloat;
		_value.construct<double, &ValueUnion::asFloat>(data.value.asFloat.toXPFloat().toDouble());
		break;
	case Data::PlugInTypeTaggedValue::kBoolean:
		_type = DynamicValueTypes::kBoolean;
		_value.construct<bool, &ValueUnion::asBool>(data.value.asBoolean != 0);
		break;
	case Data::PlugInTypeTaggedValue::kEvent:
		_type = DynamicValueTypes::kEvent;
		_value.construct<Event, &ValueUnion::asEvent>(Event());
		if (!_value.asEvent.load(data.value.asEvent))
			return false;
		break;
	case Data::PlugInTypeTaggedValue::kLabel:
		_type = DynamicValueTypes::kLabel;
		_value.construct<Label, &ValueUnion::asLabel>(Label());
		if (!_value.asLabel.load(data.value.asLabel))
			return false;
		break;
	case Data::PlugInTypeTaggedValue::kString:
		_type = DynamicValueTypes::kString;
		_value.construct<Common::String, &ValueUnion::asString>(data.value.asString);
		break;
	case Data::PlugInTypeTaggedValue::kPoint:
		_type = DynamicValueTypes::kPoint;
		_value.construct<Common::Point, &ValueUnion::asPoint>(Common::Point());
		if (!data.value.asPoint.toScummVMPoint(_value.asPoint))
			return false;
		break;
	default:
		assert(false);
		return false;
	}

	return true;
}

DynamicValueTypes::DynamicValueType DynamicValue::getType() const {
	return _type;
}

const int32 &DynamicValue::getInt() const {
	assert(_type == DynamicValueTypes::kInteger);
	return _value.asInt;
}

const double &DynamicValue::getFloat() const {
	assert(_type == DynamicValueTypes::kFloat);
	return _value.asFloat;
}

const Common::Point &DynamicValue::getPoint() const {
	assert(_type == DynamicValueTypes::kPoint);
	return _value.asPoint;
}

const IntRange &DynamicValue::getIntRange() const {
	assert(_type == DynamicValueTypes::kIntegerRange);
	return _value.asIntRange;
}

const AngleMagVector &DynamicValue::getVector() const {
	assert(_type == DynamicValueTypes::kVector);
	return _value.asVector;
}

const Label &DynamicValue::getLabel() const {
	assert(_type == DynamicValueTypes::kLabel);
	return _value.asLabel;
}

const Event &DynamicValue::getEvent() const {
	assert(_type == DynamicValueTypes::kEvent);
	return _value.asEvent;
}

const Common::String &DynamicValue::getString() const {
	assert(_type == DynamicValueTypes::kString);
	return _value.asString;
}

const bool &DynamicValue::getBool() const {
	assert(_type == DynamicValueTypes::kBoolean);
	return _value.asBool;
}

const Common::SharedPtr<DynamicList> &DynamicValue::getList() const {
	assert(_type == DynamicValueTypes::kList);
	return _value.asList;
}

const ObjectReference &DynamicValue::getObject() const {
	assert(_type == DynamicValueTypes::kObject);
	return _value.asObj;
}

const DynamicValueWriteProxy &DynamicValue::getWriteProxy() const {
	assert(_type == DynamicValueTypes::kWriteProxy);
	return _value.asWriteProxy;
}

void DynamicValue::setInt(int32 value) {
	if (_type != DynamicValueTypes::kInteger) {
		clear();
		_type = DynamicValueTypes::kInteger;
		_value.construct<int32, &ValueUnion::asInt>(value);
	} else {
		_value.assign<int32, &ValueUnion::asInt>(value);
	}
}

void DynamicValue::setFloat(double value) {
	if (_type != DynamicValueTypes::kFloat) {
		clear();
		_type = DynamicValueTypes::kFloat;
		_value.construct<double, &ValueUnion::asFloat>(value);
	} else {
		_value.assign<double, &ValueUnion::asFloat>(value);
	}
}

void DynamicValue::setPoint(const Common::Point &value) {
	if (_type != DynamicValueTypes::kPoint) {
		clear();
		_type = DynamicValueTypes::kPoint;
		_value.construct<Common::Point, &ValueUnion::asPoint>(value);
	} else {
		_value.assign<Common::Point, &ValueUnion::asPoint>(value);
	}
}

void DynamicValue::setIntRange(const IntRange &value) {
	if (_type != DynamicValueTypes::kIntegerRange) {
		clear();
		_type = DynamicValueTypes::kIntegerRange;
		_value.construct<IntRange, &ValueUnion::asIntRange>(value);
	} else {
		_value.assign<IntRange, &ValueUnion::asIntRange>(value);
	}
}

void DynamicValue::setVector(const AngleMagVector &value) {
	if (_type != DynamicValueTypes::kVector) {
		clear();
		_type = DynamicValueTypes::kVector;
		_value.construct<AngleMagVector, &ValueUnion::asVector>(value);
	} else {
		_value.assign<AngleMagVector, &ValueUnion::asVector>(value);
	}
}

void DynamicValue::setLabel(const Label &value) {
	if (_type != DynamicValueTypes::kLabel) {
		clear();
		_type = DynamicValueTypes::kLabel;
		_value.construct<Label, &ValueUnion::asLabel>(value);
	} else {
		_value.assign<Label, &ValueUnion::asLabel>(value);
	}
}

void DynamicValue::setEvent(const Event &value) {
	if (_type != DynamicValueTypes::kEvent) {
		clear();
		_type = DynamicValueTypes::kEvent;
		_value.construct<Event, &ValueUnion::asEvent>(value);
	} else {
		_value.assign<Event, &ValueUnion::asEvent>(value);
	}
}

void DynamicValue::setString(const Common::String &value) {
	if (_type != DynamicValueTypes::kString) {
		clear();
		_type = DynamicValueTypes::kString;
		_value.construct<Common::String, &ValueUnion::asString>(value);
	} else {
		_value.assign<Common::String, &ValueUnion::asString>(value);
	}
}

void DynamicValue::setBool(bool value) {
	if (_type != DynamicValueTypes::kBoolean) {
		clear();
		_type = DynamicValueTypes::kBoolean;
		_value.construct<bool, &ValueUnion::asBool>(value);
	} else {
		_value.assign<bool, &ValueUnion::asBool>(value);
	}
}

void DynamicValue::setList(const Common::SharedPtr<DynamicList> &value) {
	if (_type != DynamicValueTypes::kList) {
		clear();
		_type = DynamicValueTypes::kList;
		_value.construct<Common::SharedPtr<DynamicList>, &ValueUnion::asList>(value);
	} else {
		_value.assign<Common::SharedPtr<DynamicList>, &ValueUnion::asList>(value);
	}
}

void DynamicValue::setWriteProxy(const DynamicValueWriteProxy &writeProxy) {
	Common::SharedPtr<DynamicList> listRef = writeProxy.containerList; // Back up list ref in case this is a self-assign
	if (_type != DynamicValueTypes::kWriteProxy) {
		clear();
		_type = DynamicValueTypes::kWriteProxy;
		_value.construct<DynamicValueWriteProxy, &ValueUnion::asWriteProxy>(writeProxy);
	} else {
		_value.assign<DynamicValueWriteProxy, &ValueUnion::asWriteProxy>(writeProxy);
	}
}

bool DynamicValue::roundToInt(int32 &outInt) const {
	if (_type == DynamicValueTypes::kInteger) {
		outInt = _value.asInt;
		return true;
	} else if (_type == DynamicValueTypes::kFloat) {
		outInt = static_cast<int32>(floor(_value.asFloat + 0.5));
		return true;
	}

	return false;
}

bool DynamicValue::convertToType(DynamicValueTypes::DynamicValueType targetType, DynamicValue &result) const {
	if (_type == DynamicValueTypes::kObject && targetType != DynamicValueTypes::kObject) {
		RuntimeObject *obj = this->_value.asObj.object.lock().get();
		if (obj && obj->isModifier() && static_cast<Modifier *>(obj)->isVariable()) {
			DynamicValue varContents;
			static_cast<VariableModifier *>(obj)->varGetValue(varContents);
			return varContents.convertToTypeNoDereference(targetType, result);
		}
	}

	if (_type == DynamicValueTypes::kList && targetType != DynamicValueTypes::kList) {
		if (_value.asList.get() && _value.asList->getSize() == 1) {
			// Single-value lists are convertible
			DynamicValue firstListElement;
			(void)_value.asList->getAtIndex(0, firstListElement);

			return firstListElement.convertToType(targetType, result);
		}
	}

	return convertToTypeNoDereference(targetType, result);
}

DynamicValue DynamicValue::dereference() const {
	if (_type == DynamicValueTypes::kObject) {
		RuntimeObject *obj = this->_value.asObj.object.lock().get();
		if (obj && obj->isModifier() && static_cast<Modifier *>(obj)->isVariable()) {
			DynamicValue varContents;
			static_cast<VariableModifier *>(obj)->varGetValue(varContents);
			return varContents;
		}
	}

	if (_type == DynamicValueTypes::kList) {
		if (_value.asList.get() && _value.asList->getSize() == 1) {
			// Single-value lists are convertible
			DynamicValue firstListElement;
			(void)_value.asList->getAtIndex(0, firstListElement);

			return firstListElement;
		}
	}

	return *this;
}

bool DynamicValue::convertToTypeNoDereference(DynamicValueTypes::DynamicValueType targetType, DynamicValue &result) const {
	if (_type == targetType) {
		result = *this;
		return true;
	}

	switch (_type) {
	case DynamicValueTypes::kNull:
		if (targetType == DynamicValueTypes::kObject) {
			result.setObject(Common::WeakPtr<RuntimeObject>());
			return true;
		}
		break;
	case DynamicValueTypes::kInteger:
		return convertIntToType(targetType, result);
	case DynamicValueTypes::kFloat:
		return convertFloatToType(targetType, result);
	case DynamicValueTypes::kBoolean:
		return convertBoolToType(targetType, result);
	case DynamicValueTypes::kString:
		return convertStringToType(targetType, result);
	default:
		break;
	}

	warning("Couldn't convert dynamic value from source type");
	return false;
}

void DynamicValue::setObject(const ObjectReference &value) {
	if (_type != DynamicValueTypes::kObject) {
		clear();
		_type = DynamicValueTypes::kObject;
		_value.construct<ObjectReference, &ValueUnion::asObj>(value);
	} else {
		_value.assign<ObjectReference, &ValueUnion::asObj>(value);
	}
}

void DynamicValue::setObject(const Common::WeakPtr<RuntimeObject> &value) {
	setObject(ObjectReference(value));
}

DynamicValue &DynamicValue::operator=(const DynamicValue &other) {
	setFromOther(other);
	return *this;
}

bool DynamicValue::operator==(const DynamicValue &other) const {
	if (this == &other)
		return true;

	if (_type != other._type)
		return false;

	switch (_type) {
	case DynamicValueTypes::kNull:
		return true;
	case DynamicValueTypes::kInteger:
		return _value.asInt == other._value.asInt;
	case DynamicValueTypes::kFloat:
		return _value.asFloat == other._value.asFloat;
	case DynamicValueTypes::kPoint:
		return _value.asPoint.x == other._value.asPoint.x && _value.asPoint.y == other._value.asPoint.y;
	case DynamicValueTypes::kIntegerRange:
		return _value.asIntRange == other._value.asIntRange;
	case DynamicValueTypes::kVector:
		return _value.asVector == other._value.asVector;
	case DynamicValueTypes::kLabel:
		return _value.asLabel == other._value.asLabel;
	case DynamicValueTypes::kEvent:
		return _value.asEvent == other._value.asEvent;
	case DynamicValueTypes::kString:
		return _value.asString == other._value.asString;
	case DynamicValueTypes::kBoolean:
		return _value.asBool == other._value.asBool;
	case DynamicValueTypes::kList:
		return (*_value.asList.get()) == (*other._value.asList.get());
	case DynamicValueTypes::kObject:
		return _value.asObj == other._value.asObj;
	default:
		break;
	}

	assert(false);
	return false;
}

void DynamicValue::clear() {
	switch (_type) {
	case DynamicValueTypes::kNull:
	case DynamicValueTypes::kUnspecified:
		_value.destruct<uint64, &ValueUnion::asUnset>();
		break;
	case DynamicValueTypes::kInteger:
		_value.destruct<int32, &ValueUnion::asInt>();
		break;
	case DynamicValueTypes::kFloat:
		_value.destruct<double, &ValueUnion::asFloat>();
		break;
	case DynamicValueTypes::kPoint:
		_value.destruct<Common::Point, &ValueUnion::asPoint>();
		break;
	case DynamicValueTypes::kIntegerRange:
		_value.destruct<IntRange, &ValueUnion::asIntRange>();
		break;
	case DynamicValueTypes::kBoolean:
		_value.destruct<bool, &ValueUnion::asBool>();
		break;
	case DynamicValueTypes::kVector:
		_value.destruct<AngleMagVector, &ValueUnion::asVector>();
		break;
	case DynamicValueTypes::kLabel:
		_value.destruct<Label, &ValueUnion::asLabel>();
		break;
	case DynamicValueTypes::kEvent:
		_value.destruct<Event, &ValueUnion::asEvent>();
		break;
	case DynamicValueTypes::kString:
		_value.destruct<Common::String, &ValueUnion::asString>();
		break;
	case DynamicValueTypes::kList:
		_value.destruct<Common::SharedPtr<DynamicList>, &ValueUnion::asList>();
		break;
	case DynamicValueTypes::kObject:
		_value.destruct<ObjectReference, &ValueUnion::asObj>();
		break;
	case DynamicValueTypes::kWriteProxy:
		_value.destruct<DynamicValueWriteProxy, &ValueUnion::asWriteProxy>();
		break;
	default:
		assert(false);
		break;
	};

	_type = DynamicValueTypes::kNull;
}

bool DynamicValue::convertIntToType(DynamicValueTypes::DynamicValueType targetType, DynamicValue &result) const {
	int32 value = this->getInt();

	switch (targetType) {
	case DynamicValueTypes::kInteger:
		result.setInt(value);
		return true;
	case DynamicValueTypes::kBoolean:
		result.setBool(value != 0);
		return true;
	case DynamicValueTypes::kFloat:
		result.setFloat(value);
		return true;
	default:
		warning("Unable to implicitly convert dynamic value");
		return false;
	}
}

bool DynamicValue::convertFloatToType(DynamicValueTypes::DynamicValueType targetType, DynamicValue &result) const {
	double value = this->getFloat();

	switch (targetType) {
	case DynamicValueTypes::kInteger:
		result.setInt(static_cast<int32>(round(value)));
		return true;
	case DynamicValueTypes::kBoolean:
		result.setBool(value != 0.0);
		return true;
	case DynamicValueTypes::kFloat:
		result.setFloat(value);
		return true;
	default:
		warning("Unable to implicitly convert dynamic value");
		return false;
	}
}

bool DynamicValue::convertBoolToType(DynamicValueTypes::DynamicValueType targetType, DynamicValue &result) const {
	bool value = this->getBool();

	switch (targetType) {
	case DynamicValueTypes::kInteger:
		result.setInt(value ? 1 : 0);
		return true;
	case DynamicValueTypes::kBoolean:
		result.setBool(value);
		return true;
	case DynamicValueTypes::kFloat:
		result.setFloat(value ? 1.0 : 0.0);
		return true;
	default:
		warning("Unable to implicitly convert dynamic value");
		return false;
	}
}

bool DynamicValue::convertStringToType(DynamicValueTypes::DynamicValueType targetType, DynamicValue &result) const {
	const Common::String &value = this->getString();

	// Docs say that strings are always false but they are not convertible
	switch (targetType) {
	case DynamicValueTypes::kInteger: {
			// Passing float values is allowed, but they are truncated toward zero instead of rounded
			double floatValue = 0;
			if (sscanf(value.c_str(), "%lf", &floatValue))
				result.setInt(static_cast<int>(floatValue));
			else
				result.setInt(0);
			return true;
		} break;
	case DynamicValueTypes::kFloat: {
			double floatValue = 0;
			if (sscanf(value.c_str(), "%lf", &floatValue))
				result.setFloat(floatValue);
			else
				result.setFloat(0.0);
			return true;
		} break;
	default:
		break;
	}
	warning("Unable to implicitly convert dynamic value");
	return false;
}

void DynamicValue::setFromOther(const DynamicValue &other) {
	if (this == &other)
		return;

	// Keep the list alive until the end of this in case the other value is contained inside of this one
	Common::SharedPtr<DynamicList> listHolder;
	if (_type == DynamicValueTypes::kList)
		listHolder = _value.asList;

	switch (other._type) {
	case DynamicValueTypes::kNull:
	case DynamicValueTypes::kUnspecified:
		clear();
		_type = other._type;
		break;
	case DynamicValueTypes::kInteger:
		setInt(other._value.asInt);
		break;
	case DynamicValueTypes::kFloat:
		setFloat(other._value.asFloat);
		break;
	case DynamicValueTypes::kPoint:
		setPoint(other._value.asPoint);
		break;
	case DynamicValueTypes::kIntegerRange:
		setIntRange(other._value.asIntRange);
		break;
	case DynamicValueTypes::kVector:
		setVector(other._value.asVector);
		break;
	case DynamicValueTypes::kLabel:
		setLabel(other._value.asLabel);
		break;
	case DynamicValueTypes::kEvent:
		setEvent(other._value.asEvent);
		break;
	case DynamicValueTypes::kString:
		setString(other._value.asString);
		break;
	case DynamicValueTypes::kBoolean:
		setBool(other._value.asBool);
		break;
	case DynamicValueTypes::kList:
		setList(other._value.asList);
		break;
	case DynamicValueTypes::kObject:
		setObject(other._value.asObj);
		break;
	case DynamicValueTypes::kWriteProxy:
		setWriteProxy(other._value.asWriteProxy);
		break;
	default:
		assert(false);
		break;
	}

	assert(_type == other._type);
}


DynamicValueSource::DynamicValueSource() : _sourceType(DynamicValueSourceTypes::kInvalid) {
}

DynamicValueSource::DynamicValueSource(const DynamicValueSource &other) : _sourceType(DynamicValueSourceTypes::kInvalid) {
	initFromOther(other);
}

DynamicValueSource::DynamicValueSource(DynamicValueSource &&other) : _sourceType(DynamicValueSourceTypes::kInvalid) {
	initFromOther(static_cast<DynamicValueSource &&>(other));
}

DynamicValueSource::~DynamicValueSource() {
	destructValue();
}

DynamicValueSource &DynamicValueSource::operator=(const DynamicValueSource &other) {
	if (this == &other)
		return *this;

	destructValue();
	initFromOther(other);
	return *this;
}

DynamicValueSource &DynamicValueSource::operator=(DynamicValueSource &&other) {
	if (this == &other)
		return *this;

	destructValue();
	initFromOther(static_cast<DynamicValueSource &&>(other));
	return *this;
}

DynamicValueSourceTypes::DynamicValueSourceType DynamicValueSource::getSourceType() const {
	return _sourceType;
}

const DynamicValue &DynamicValueSource::getConstant() const {
	assert(_sourceType == DynamicValueSourceTypes::kConstant);
	return _valueUnion._constValue;
}
const VarReference &DynamicValueSource::getVarReference() const {
	assert(_sourceType == DynamicValueSourceTypes::kVariableReference);
	return _valueUnion._varReference;
}

bool DynamicValueSource::load(const Data::InternalTypeTaggedValue &data, const Common::String &varSource, const Common::String &varString) {
	destructValue();

	switch (data.type)
	{
	case Data::InternalTypeTaggedValue::kIncomingData:
		_sourceType = DynamicValueSourceTypes::kIncomingData;
		return true;
	case Data::InternalTypeTaggedValue::kVariableReference:
		_sourceType = DynamicValueSourceTypes::kVariableReference;
		new (&_valueUnion._varReference) VarReference(data.value.asVariableReference.guid, varSource);
		return true;
	default:
		_sourceType = DynamicValueSourceTypes::kConstant;
		new (&_valueUnion._constValue) DynamicValue();
		return _valueUnion._constValue.loadConstant(data, varString);
	}

	assert(false);
	return false;
}

bool DynamicValueSource::load(const Data::PlugInTypeTaggedValue &data) {
	destructValue();

	switch (data.type) {
	case Data::PlugInTypeTaggedValue::kIncomingData:
		_sourceType = DynamicValueSourceTypes::kIncomingData;
		return true;
	case Data::PlugInTypeTaggedValue::kVariableReference:
		_sourceType = DynamicValueSourceTypes::kVariableReference;
		new (&_valueUnion._varReference) VarReference(data.value.asVarRefGUID, "");
		return true;
	default:
		_sourceType = DynamicValueSourceTypes::kConstant;
		new (&_valueUnion._constValue) DynamicValue();
		return _valueUnion._constValue.loadConstant(data);
	}
}


void DynamicValueSource::linkInternalReferences(ObjectLinkingScope *scope) {
	if (_sourceType == DynamicValueSourceTypes::kVariableReference) {
		_valueUnion._varReference.linkInternalReferences(scope);
	}
}

void DynamicValueSource::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	if (_sourceType == DynamicValueSourceTypes::kVariableReference) {
		_valueUnion._varReference.visitInternalReferences(visitor);
	}
}

DynamicValue DynamicValueSource::produceValue(const DynamicValue &incomingData) const {
	switch (_sourceType) {
	case DynamicValueSourceTypes::kConstant:
		return _valueUnion._constValue;
	case DynamicValueSourceTypes::kIncomingData:
		return incomingData;
	case DynamicValueSourceTypes::kVariableReference: {
			DynamicValue result;
			result.setObject(_valueUnion._varReference.resolution);
			return result;
		} break;
	default:
		warning("Dynamic value couldn't be resolved");
		return DynamicValue();
	}
}


DynamicValueSource::ValueUnion::ValueUnion() {
}

DynamicValueSource::ValueUnion::~ValueUnion() {
}

void DynamicValueSource::destructValue() {
	switch (_sourceType) {
	case DynamicValueSourceTypes::kConstant:
		_valueUnion._constValue.~DynamicValue();
		break;
	case DynamicValueSourceTypes::kVariableReference:
		_valueUnion._varReference.~VarReference();
		break;
	default:
		break;
	}

	_sourceType = DynamicValueSourceTypes::kInvalid;
}

void DynamicValueSource::initFromOther(const DynamicValueSource &other) {
	assert(_sourceType == DynamicValueSourceTypes::kInvalid);

	switch (other._sourceType) {
	case DynamicValueSourceTypes::kConstant:
		new (&_valueUnion._constValue) DynamicValue(other._valueUnion._constValue);
		break;
	case DynamicValueSourceTypes::kVariableReference:
		new (&_valueUnion._varReference) VarReference(other._valueUnion._varReference);
		break;
	default:
		break;
	}

	_sourceType = other._sourceType;
}

void DynamicValueSource::initFromOther(DynamicValueSource &&other) {
	assert(_sourceType == DynamicValueSourceTypes::kInvalid);

	switch (other._sourceType) {
	case DynamicValueSourceTypes::kConstant:
		new (&_valueUnion._constValue) DynamicValue(static_cast<DynamicValue &&>(other._valueUnion._constValue));
		break;
	case DynamicValueSourceTypes::kVariableReference:
		new (&_valueUnion._varReference) VarReference(static_cast<VarReference &&>(other._valueUnion._varReference));
		break;
	default:
		break;
	}

	_sourceType = other._sourceType;
}

MiniscriptInstructionOutcome DynamicValueWriteStringHelper::write(MiniscriptThread *thread, const DynamicValue &value, void *objectRef, uintptr ptrOrOffset) {
	DynamicValue derefValue = value.dereference();

	Common::String &dest = *static_cast<Common::String *>(objectRef);
	switch (derefValue.getType()) {
	case DynamicValueTypes::kString:
		dest = derefValue.getString();
		return kMiniscriptInstructionOutcomeContinue;
	default:
		return kMiniscriptInstructionOutcomeFailed;
	}
}

MiniscriptInstructionOutcome DynamicValueWriteStringHelper::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) {
	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome DynamicValueWriteStringHelper::refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) {
	return kMiniscriptInstructionOutcomeFailed;
}

void DynamicValueWriteStringHelper::create(Common::String *strValue, DynamicValueWriteProxy &proxy) {
	proxy.pod.ptrOrOffset = 0;
	proxy.pod.objectRef = strValue;
	proxy.pod.ifc = DynamicValueWriteInterfaceGlue<DynamicValueWriteStringHelper>::getInstance();
}

MiniscriptInstructionOutcome DynamicValueWriteDiscardHelper::write(MiniscriptThread *thread, const DynamicValue &value, void *objectRef, uintptr ptrOrOffset) {
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome DynamicValueWriteDiscardHelper::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) {
	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome DynamicValueWriteDiscardHelper::refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) {
	return kMiniscriptInstructionOutcomeFailed;
}

void DynamicValueWriteDiscardHelper::create(DynamicValueWriteProxy &proxy) {
	proxy.pod.ptrOrOffset = 0;
	proxy.pod.objectRef = nullptr;
	proxy.pod.ifc = DynamicValueWriteInterfaceGlue<DynamicValueWriteDiscardHelper>::getInstance();
}


MiniscriptInstructionOutcome DynamicValueWritePointHelper::write(MiniscriptThread *thread, const DynamicValue &value, void *objectRef, uintptr ptrOrOffset) {
	DynamicValue derefValue = value.dereference();

	if (value.getType() != DynamicValueTypes::kPoint) {
		thread->error("Can't set point to invalid type");
		return kMiniscriptInstructionOutcomeFailed;
	}

	*static_cast<Common::Point *>(objectRef) = value.getPoint();

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome DynamicValueWritePointHelper::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) {
	if (attrib == "x") {
		DynamicValueWriteIntegerHelper<int16>::create(&static_cast<Common::Point *>(objectRef)->x, proxy);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "y") {
		DynamicValueWriteIntegerHelper<int16>::create(&static_cast<Common::Point *>(objectRef)->y, proxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	thread->error("Invalid attribute for point");
	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome DynamicValueWritePointHelper::refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) {
	return kMiniscriptInstructionOutcomeFailed;
}

void DynamicValueWritePointHelper::create(Common::Point *pointValue, DynamicValueWriteProxy &proxy) {
	proxy.pod.ptrOrOffset = 0;
	proxy.pod.objectRef = pointValue;
	proxy.pod.ifc = DynamicValueWriteInterfaceGlue<DynamicValueWritePointHelper>::getInstance();
}

MiniscriptInstructionOutcome DynamicValueWriteBoolHelper::write(MiniscriptThread *thread, const DynamicValue &value, void *objectRef, uintptr ptrOrOffset) {
	DynamicValue derefValue = value.dereference();

	bool &dest = *static_cast<bool *>(objectRef);
	switch (derefValue.getType()) {
	case DynamicValueTypes::kBoolean:
		dest = derefValue.getBool();
		return kMiniscriptInstructionOutcomeContinue;
	default:
		return kMiniscriptInstructionOutcomeFailed;
	}
}

MiniscriptInstructionOutcome DynamicValueWriteBoolHelper::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) {
	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome DynamicValueWriteBoolHelper::refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) {
	return kMiniscriptInstructionOutcomeFailed;
}

void DynamicValueWriteBoolHelper::create(bool *boolValue, DynamicValueWriteProxy &proxy) {
	proxy.pod.ptrOrOffset = 0;
	proxy.pod.objectRef = boolValue;
	proxy.pod.ifc = DynamicValueWriteInterfaceGlue<DynamicValueWriteBoolHelper>::getInstance();
}

MiniscriptInstructionOutcome DynamicValueWriteObjectHelper::write(MiniscriptThread *thread, const DynamicValue &value, void *objectRef, uintptr ptrOrOffset) {
	RuntimeObject *obj = static_cast<RuntimeObject *>(objectRef);
	if (obj->isModifier() && static_cast<Modifier *>(obj)->isVariable()) {
		VariableModifier *var = static_cast<VariableModifier *>(obj);
		if (var->varSetValue(thread, value))
			return kMiniscriptInstructionOutcomeContinue;
		else {
			thread->error("Failed to assign value to variable");
			return kMiniscriptInstructionOutcomeFailed;
		}
	}

	thread->error("Can't write to read-only object value");
	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome DynamicValueWriteObjectHelper::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) {
	return static_cast<RuntimeObject *>(objectRef)->writeRefAttribute(thread, proxy, attrib);
}

MiniscriptInstructionOutcome DynamicValueWriteObjectHelper::refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) {
	return static_cast<RuntimeObject *>(objectRef)->writeRefAttributeIndexed(thread, proxy, attrib, index);
}

void DynamicValueWriteObjectHelper::create(RuntimeObject *obj, DynamicValueWriteProxy &proxy) {
	proxy.containerList.reset();	// Object references are always anchored while threads are running, so don't need to preserve the container
	proxy.pod.ifc = DynamicValueWriteInterfaceGlue<DynamicValueWriteObjectHelper>::getInstance();
	proxy.pod.objectRef = obj;
	proxy.pod.ptrOrOffset = 0;
}

MessengerSendSpec::MessengerSendSpec() : destination(kMessageDestNone), _linkType(kLinkTypeNotYetLinked) {
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

void MessengerSendSpec::linkInternalReferences(ObjectLinkingScope *outerScope) {
	switch (destination) {
	case kMessageDestNone:
	case kMessageDestSharedScene:
	case kMessageDestScene:
	case kMessageDestSection:
	case kMessageDestProject:
	case kMessageDestActiveScene:
	case kMessageDestElementsParent:
	case kMessageDestChildren:
	case kMessageDestModifiersParent:
	case kMessageDestSubsection:
	case kMessageDestElement:
	case kMessageDestSourcesParent:
	case kMessageDestBehavior:
	case kMessageDestNextElement:
	case kMessageDestPrevElement:
	case kMessageDestBehaviorsParent:
		_linkType = kLinkTypeCoded;
		break;
	default: {
			Common::SharedPtr<RuntimeObject> resolution = outerScope->resolve(destination).lock();
			if (resolution) {
				if (resolution->isModifier()) {
					_resolvedModifierDest = resolution.staticCast<Modifier>();
					_linkType = kLinkTypeModifier;
				} else if (resolution->isStructural()) {
					_resolvedStructuralDest = resolution.staticCast<Structural>();
					_linkType = kLinkTypeStructural;
				} else {
					_linkType = kLinkTypeUnresolved;
				}
			} else {
				_linkType = kLinkTypeUnresolved;
			}
		} break;
	}

	with.linkInternalReferences(outerScope);
}

void MessengerSendSpec::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	visitor->visitWeakModifierRef(_resolvedModifierDest);
	visitor->visitWeakStructuralRef(_resolvedStructuralDest);
	visitor->visitWeakModifierRef(_resolvedVarSource);
}

void MessengerSendSpec::resolveDestination(Runtime *runtime, Modifier *sender, RuntimeObject *triggerSource, Common::WeakPtr<Structural> &outStructuralDest, Common::WeakPtr<Modifier> &outModifierDest, RuntimeObject *customDestination) const {
	outStructuralDest.reset();
	outModifierDest.reset();

	if (customDestination) {
		if (customDestination->isStructural())
			outStructuralDest = customDestination->getSelfReference().staticCast<Structural>();
		else if (customDestination->isModifier())
			outModifierDest = customDestination->getSelfReference().staticCast<Modifier>();
		else
			error("Custom destination was invalid");

		return;
	}

	if (_linkType == kLinkTypeModifier) {
		outModifierDest = _resolvedModifierDest;
	} else if (_linkType == kLinkTypeStructural) {
		outStructuralDest = _resolvedStructuralDest;
	} else if (_linkType == kLinkTypeCoded) {
		switch (destination) {
		case kMessageDestNone:
			break;
		case kMessageDestSharedScene:
			outStructuralDest = runtime->getActiveSharedScene();
			break;
		case kMessageDestScene:
			resolveHierarchyStructuralDestination(runtime, sender, outStructuralDest, outModifierDest, isSceneFilter);
			break;
		case kMessageDestSection:
			resolveHierarchyStructuralDestination(runtime, sender, outStructuralDest, outModifierDest, isSectionFilter);
			break;
		case kMessageDestProject:
			outStructuralDest = runtime->getProject()->getSelfReference().staticCast<Project>();
			break;
		case kMessageDestActiveScene:
			outStructuralDest = runtime->getActiveMainScene();
			break;
		case kMessageDestElement:
			resolveHierarchyStructuralDestination(runtime, sender, outStructuralDest, outModifierDest, isElementFilter);
			break;
		case kMessageDestModifiersParent:
			resolveVariableObjectType(sender->getParent().lock().get(), outStructuralDest, outModifierDest);
			break;
		case kMessageDestElementsParent:
			resolveHierarchyStructuralDestination(runtime, sender, outStructuralDest, outModifierDest, isElementFilter);
			if (!outStructuralDest.expired())
				outStructuralDest = outStructuralDest.lock()->getParent()->getSelfReference().staticCast<Structural>();
			break;
		case kMessageDestNextElement:
		case kMessageDestPrevElement: {
				Common::WeakPtr<Structural> elementWeak;
				Common::WeakPtr<Modifier> modifier;
				resolveHierarchyStructuralDestination(runtime, sender, elementWeak, modifier, isElementFilter);

				Common::SharedPtr<Structural> sibling;
				Common::SharedPtr<Structural> element = elementWeak.lock();
				if (element) {
					Structural *parent = element->getParent();
					if (parent) {
						const Common::Array<Common::SharedPtr<Structural> > &siblings = parent->getChildren();
						for (size_t i = 0; i < siblings.size(); i++) {
							if (siblings[i] == element) {
								if (destination == kMessageDestPrevElement) {
									if (i != 0)
										sibling = siblings[i - 1];
								} else if (destination == kMessageDestNextElement) {
									if (i != siblings.size() - 1)
										sibling = siblings[i + 1];
								}
								break;
							}
						}
					}
				}

				if (sibling)
					outStructuralDest = sibling;
			} break;
		case kMessageDestSourcesParent: {
				// This sends to the exact parent, e.g. if the source is inside of a behavior, then it sends it to the behavior.
				if (triggerSource) {
					RuntimeObject *parentObj = nullptr;
					if (triggerSource->isModifier())
						parentObj = static_cast<Modifier *>(triggerSource)->getParent().lock().get();
					else if (triggerSource->isStructural())
						parentObj = static_cast<Structural *>(triggerSource)->getParent();

					if (parentObj) {
						if (parentObj->isModifier())
							outModifierDest = parentObj->getSelfReference().staticCast<Modifier>();
						else if (parentObj->isStructural())
							outStructuralDest = parentObj->getSelfReference().staticCast<Structural>();
					}
				}
			} break;
		case kMessageDestChildren:
		case kMessageDestSubsection:
		case kMessageDestBehavior:
		case kMessageDestBehaviorsParent:
			warning("Not-yet-implemented message destination type");
			break;
		default:
			break;
		}
	} else if (_linkType == kLinkTypeNotYetLinked) {
		error("Messenger wasn't linked, programmer probably forgot to call linkInternalReferences on the send spec");
	}
}

void MessengerSendSpec::resolveVariableObjectType(RuntimeObject *obj, Common::WeakPtr<Structural> &outStructuralDest, Common::WeakPtr<Modifier> &outModifierDest) {
	if (!obj) {
		warning("Couldn't resolve messenger destination");
		return;
	}

	if (obj->isStructural())
		outStructuralDest = obj->getSelfReference().staticCast<Structural>();
	else if (obj->isModifier())
		outModifierDest = obj->getSelfReference().staticCast<Modifier>();
	else {
		warning("Messenger destination was not a valid recipient type");
		return;
	}
}

void MessengerSendSpec::sendFromMessenger(Runtime *runtime, Modifier *sender, RuntimeObject *triggerSource, const DynamicValue &incomingData, RuntimeObject *customDestination) const {
	sendFromMessengerWithCustomData(runtime, sender, triggerSource, this->with.produceValue(incomingData), customDestination);
}

void MessengerSendSpec::sendFromMessengerWithCustomData(Runtime *runtime, Modifier *sender, RuntimeObject *triggerSource, const DynamicValue &data, RuntimeObject *customDestination) const {
	Common::SharedPtr<MessageProperties> props(new MessageProperties(this->send, data, sender->getSelfReference()));

	Common::WeakPtr<Modifier> modifierDestRef;
	Common::WeakPtr<Structural> structuralDestRef;

	resolveDestination(runtime, sender, triggerSource, structuralDestRef, modifierDestRef, customDestination);

	Common::SharedPtr<Modifier> modifierDest = modifierDestRef.lock();
	Common::SharedPtr<Structural> structuralDest = structuralDestRef.lock();

	Common::SharedPtr<MessageDispatch> dispatch;
	if (structuralDest)
		dispatch.reset(new MessageDispatch(props, structuralDest.get(), messageFlags.cascade, messageFlags.relay, true));
	else if (modifierDest)
		dispatch.reset(new MessageDispatch(props, modifierDest.get(), messageFlags.cascade, messageFlags.relay, true));

	if (dispatch) {
		if (messageFlags.immediate)
			runtime->sendMessageOnVThread(dispatch);
		else
			runtime->queueMessage(dispatch);
	}
}

void MessengerSendSpec::resolveHierarchyStructuralDestination(Runtime *runtime, Modifier *sender, Common::WeakPtr<Structural> &outStructuralDest, Common::WeakPtr<Modifier> &outModifierDest, bool (*compareFunc)(Structural *structural)) const {
	RuntimeObject *obj = sender->getParent().lock().get();
	while (obj) {
		if (obj->isStructural()) {
			Structural *structural = static_cast<Structural *>(obj);
			if (compareFunc(structural)) {
				outStructuralDest = structural->getSelfReference().staticCast<Structural>();
				return;
			}

			obj = structural->getParent();
		} else if (obj->isModifier()) {
			Modifier *modifier = static_cast<Modifier *>(obj);
			obj = modifier->getParent().lock().get();
		} else {
			break;
		}
	}
}

bool MessengerSendSpec::isSceneFilter(Structural *structural) {
	Structural *parent = structural->getParent();
	return parent != nullptr && parent->isSubsection();
}

bool MessengerSendSpec::isSectionFilter(Structural *structural) {
	return structural->isSection();
}

bool MessengerSendSpec::isSubsectionFilter(Structural *structural) {
	return structural->isSubsection();
}

bool MessengerSendSpec::isElementFilter(Structural *structural) {
	return structural->isElement();
}

Event::Event() : eventType(EventIDs::kNothing), eventInfo(0) {
}

Event::Event(EventIDs::EventID peventType, uint32 peventInfo) : eventType(peventType), eventInfo(peventInfo) {
}

bool Event::respondsTo(const Event &otherEvent) const {
	return (*this) == otherEvent;
}

bool Event::load(const Data::Event &data) {
	eventType = static_cast<EventIDs::EventID>(data.eventID);
	eventInfo = data.eventInfo;

	return true;
}

VarReference::VarReference() : guid(0) {
}

VarReference::VarReference(uint32 pguid, const Common::String &psource) : guid(pguid), source(psource) {
}

bool VarReference::resolve(Structural *structuralScope, Common::WeakPtr<RuntimeObject> &outObject) const {
	if (resolveContainer(structuralScope, outObject))
		return true;

	Structural *parent = structuralScope->getParent();
	if (!parent)
		return false;

	return resolve(parent, outObject);
}

bool VarReference::resolve(Modifier *modifierScope, Common::WeakPtr<RuntimeObject> &outObject) const {
	if (resolveSingleModifier(modifierScope, outObject))
		return true;

	RuntimeObject *parent = modifierScope->getParent().lock().get();
	if (parent->isStructural())
		return resolve(static_cast<Structural *>(parent), outObject);

	if (parent->isModifier()) {
		Modifier *parentModifier = static_cast<Modifier *>(parent);
		IModifierContainer *parentContainer = parentModifier->getChildContainer();
		if (parentContainer && resolveContainer(parentContainer, outObject))
			return true;

		return resolve(parentModifier, outObject);
	}

	return false;
}

void VarReference::linkInternalReferences(ObjectLinkingScope *scope) {
	if (guid) {
		Common::WeakPtr<RuntimeObject> obj = scope->resolve(guid, source, false);
		if (obj.expired()) {
			warning("VarReference to '%s' failed to resolve a valid object", source.c_str());
		} else {
			Common::SharedPtr<RuntimeObject> objShr = obj.lock();
			if (objShr->isModifier())
				this->resolution = obj.staticCast<Modifier>();
			else
				warning("VarReference to '%s' wasn't a modifier", source.c_str());
		}
	}
}

void VarReference::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	visitor->visitWeakModifierRef(this->resolution);
}

bool VarReference::resolveContainer(IModifierContainer *modifierContainer, Common::WeakPtr<RuntimeObject> &outObject) const {
	for (const Common::SharedPtr<Modifier> &modifier : modifierContainer->getModifiers())
		if (resolveSingleModifier(modifier.get(), outObject))
			return true;

	return false;
}

bool VarReference::resolveSingleModifier(Modifier *modifier, Common::WeakPtr<RuntimeObject> &outObject) const {
	if (modifier->getStaticGUID() == guid || (source.size() > 0 && caseInsensitiveEqual(modifier->getName(), source))) {
		outObject = modifier->getSelfReference();
		return true;
	}

	return false;
}


AngleMagVector::AngleMagVector() : angleDegrees(0), magnitude(0) {
}

AngleMagVector::AngleMagVector(double pangleDegrees, double pmagnitude) : angleDegrees(pangleDegrees), magnitude(pmagnitude) {
}

MiniscriptInstructionOutcome AngleMagVector::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, const Common::String &attrib) {
	if (attrib == "angle") {
		DynamicValueWriteFloatHelper<double>::create(&angleDegrees, proxy);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "magnitude") {
		DynamicValueWriteFloatHelper<double>::create(&magnitude, proxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return kMiniscriptInstructionOutcomeFailed;
}

Common::String AngleMagVector::toString() const {
	return Common::String::format("(%g deg %g mag)", angleDegrees, magnitude);
}



SegmentDescription::SegmentDescription() : volumeID(0), stream(nullptr) {
}

void IPlugInModifierRegistrar::registerPlugInModifier(const char *name, const IPlugInModifierFactoryAndDataFactory *loaderFactory) {
	return this->registerPlugInModifier(name, loaderFactory, loaderFactory);
}

PlugIn::~PlugIn() {
}

ProjectPersistentResource::~ProjectPersistentResource() {
}

ProjectResources::~ProjectResources() {
	// We need these destroyed in reverse order exactly, and unfortunately the ScummVM Common::Array destructor
	// destroys forward
	while (persistentResources.size() > 0)
		persistentResources.pop_back();
}

CursorGraphic::~CursorGraphic() {
}

MacCursorGraphic::MacCursorGraphic(const Common::SharedPtr<Graphics::MacCursor>& macCursor) : _macCursor(macCursor) {
}

Graphics::Cursor *MacCursorGraphic::getCursor() const {
	return _macCursor.get();
}

WinCursorGraphic::WinCursorGraphic(const Common::SharedPtr<Graphics::WinCursorGroup> &winCursorGroup, Graphics::Cursor *cursor) : _winCursorGroup(winCursorGroup), _cursor(cursor) {
}

Graphics::Cursor *WinCursorGraphic::getCursor() const {
	return _cursor;
}

CursorGraphicCollection::CursorGraphicCollection() {
}

CursorGraphicCollection::~CursorGraphicCollection() {
}

void CursorGraphicCollection::addWinCursorGroup(uint32 cursorGroupID, const Common::SharedPtr<Graphics::WinCursorGroup> &cursorGroup) {
	Graphics::Cursor *selectedCursor = nullptr;
	if (cursorGroup->cursors.size() > 0) {
		// Not sure what the proper logic should be here, but the second one seems to be the one we usually want
		if (cursorGroup->cursors.size() > 1)
			selectedCursor = cursorGroup->cursors[1].cursor;
		else
			selectedCursor = cursorGroup->cursors.back().cursor;
		_cursorGraphics[cursorGroupID].reset(new WinCursorGraphic(cursorGroup, selectedCursor));
	}
}

void CursorGraphicCollection::addMacCursor(uint32 cursorID, const Common::SharedPtr<Graphics::MacCursor> &cursor) {
	_cursorGraphics[cursorID].reset(new MacCursorGraphic(cursor));
}

Common::SharedPtr<CursorGraphic> CursorGraphicCollection::getGraphicByID(uint32 id) const {
	Common::HashMap<uint32, Common::SharedPtr<CursorGraphic> >::const_iterator it = _cursorGraphics.find(id);
	if (it != _cursorGraphics.end())
		return it->_value;
	return nullptr;
}

ProjectDescription::ProjectDescription(ProjectPlatform platform, RuntimeVersion runtimeVersion, bool autoDetectVersion, Common::Archive *rootArchive, const Common::Path &projectRootDir)
	: _language(Common::EN_ANY), _platform(platform), _rootArchive(rootArchive), _projectRootDir(projectRootDir), _runtimeVersion(runtimeVersion), _isRuntimeVersionAuto(autoDetectVersion) {
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

void ProjectDescription::setCursorGraphics(const Common::SharedPtr<CursorGraphicCollection>& cursorGraphics) {
	_cursorGraphics = cursorGraphics;
}

const Common::SharedPtr<CursorGraphicCollection> &ProjectDescription::getCursorGraphics() const {
	return _cursorGraphics;
}

void ProjectDescription::setLanguage(const Common::Language &language) {
	_language = language;
}

const Common::Language &ProjectDescription::getLanguage() const {
	return _language;
}

ProjectPlatform ProjectDescription::getPlatform() const {
	return _platform;
}

RuntimeVersion ProjectDescription::getRuntimeVersion() const {
	return _runtimeVersion;
}

bool ProjectDescription::isRuntimeVersionAuto() const {
	return _isRuntimeVersionAuto;
}

Common::Archive *ProjectDescription::getRootArchive() const {
	return _rootArchive;
}

const Common::Path &ProjectDescription::getProjectRootDir() const {
	return _projectRootDir;
}

const SubtitleTables &ProjectDescription::getSubtitles() const {
	return _subtitles;
}

void ProjectDescription::setSubtitles(const SubtitleTables &subs) {
	_subtitles = subs;
}

const Common::Array<Common::SharedPtr<Modifier> >& SimpleModifierContainer::getModifiers() const {
	return _modifiers;
}

void SimpleModifierContainer::appendModifier(const Common::SharedPtr<Modifier> &modifier) {
	_modifiers.push_back(modifier);
	if (modifier)
		modifier->setParent(nullptr);
}

void SimpleModifierContainer::removeModifier(const Modifier *modifier) {
	for (Common::Array<Common::SharedPtr<Modifier> >::iterator it = _modifiers.begin(), itEnd = _modifiers.end(); it != itEnd; ++it) {
		if (it->get() == modifier) {
			_modifiers.erase(it);
			return;
		}
	}
}

void SimpleModifierContainer::clear() {
	_modifiers.clear();
}

RuntimeObject::RuntimeObject() : _guid(0), _runtimeGUID(0) {
}

RuntimeObject::~RuntimeObject() {
}

uint32 RuntimeObject::getStaticGUID() const {
	return _guid;
}

uint32 RuntimeObject::getRuntimeGUID() const {
	return _runtimeGUID;
}

void RuntimeObject::setRuntimeGUID(uint32 runtimeGUID) {
	_runtimeGUID = runtimeGUID;
}

void RuntimeObject::setSelfReference(const Common::WeakPtr<RuntimeObject> &selfReference) {
	_selfReference = selfReference;
}

const Common::WeakPtr<RuntimeObject>& RuntimeObject::getSelfReference() const {
	return _selfReference;
}

bool RuntimeObject::isStructural() const {
	return false;
}

bool RuntimeObject::isProject() const {
	return false;
}

bool RuntimeObject::isSection() const {
	return false;
}

bool RuntimeObject::isSubsection() const {
	return false;
}

bool RuntimeObject::isModifier() const {
	return false;
}

bool RuntimeObject::isElement() const {
	return false;
}

bool RuntimeObject::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	return false;
}

bool RuntimeObject::readAttributeIndexed(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib, const DynamicValue &index) {
	return false;
}

MiniscriptInstructionOutcome RuntimeObject::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	if (thread->getRuntime()->getProject()->getRuntimeVersion() < kRuntimeVersion200) {
		// Per 2.0 release notes, the following attrib writes succeed without error
		// on all objects
		const char *sunkAttribs[] = {
			"position", "width", "height", "rate", "range", "cel", "text", "volume", "timevalue",
			"mastervolume", "usertimeout", "layer", "paused", "trackenable", "trackdisable",
			"cache", "direct", "loop", "visible", "loopbackforth", "playeveryframe"
		};

		for (const char *sunkAttrib : sunkAttribs) {
			if (attrib == sunkAttrib) {
#ifdef MTROPOLIS_DEBUG_ENABLE
				if (Debugger *debugger = thread->getRuntime()->debugGetDebugger())
					debugger->notify(kDebugSeverityWarning, Common::String::format("'%s' attribute write was discarded", sunkAttrib));
#endif
				DynamicValueWriteDiscardHelper::create(writeProxy);
				return kMiniscriptInstructionOutcomeContinue;
			}
		}
	}

	if (attrib == "clone") {
		DynamicValueWriteFuncHelper<RuntimeObject, &RuntimeObject::scriptSetClone, false>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	if (attrib == "kill") {
		DynamicValueWriteFuncHelper<RuntimeObject, &RuntimeObject::scriptSetKill, false>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	if (attrib == "parent") {
		writeProxy.pod.ifc = DynamicValueWriteInterfaceGlue<ParentWriteProxyInterface>::getInstance();
		writeProxy.pod.objectRef = this;
		writeProxy.pod.ptrOrOffset = 0;
		return kMiniscriptInstructionOutcomeContinue;
	}

	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome RuntimeObject::writeRefAttributeIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib, const DynamicValue &index) {
	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome RuntimeObject::scriptSetClone(MiniscriptThread *thread, const DynamicValue &value) {
	thread->getRuntime()->queueCloneObject(this->getSelfReference());
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome RuntimeObject::scriptSetKill(MiniscriptThread *thread, const DynamicValue &value) {
	thread->getRuntime()->queueKillObject(this->getSelfReference());
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome RuntimeObject::scriptSetParent(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kObject) {
		thread->error("Object couldn't be re-parented to a non-object");
		return kMiniscriptInstructionOutcomeFailed;
	}

	thread->getRuntime()->queueChangeObjectParent(this->getSelfReference(), value.getObject().object);

	return kMiniscriptInstructionOutcomeContinue;
}

// Need special handling of "parent" property, assigns indirect the value but writes re-parent the object
MiniscriptInstructionOutcome RuntimeObject::ParentWriteProxyInterface::write(MiniscriptThread *thread, const DynamicValue &dest, void *objectRef, uintptr ptrOrOffset) {
	return static_cast<RuntimeObject *>(objectRef)->scriptSetParent(thread, dest);
}

RuntimeObject *RuntimeObject::ParentWriteProxyInterface::resolveObjectParent(RuntimeObject *obj) {
	if (obj->isStructural())
		return static_cast<Structural *>(obj)->getParent();
	else if (obj->isModifier())
		return static_cast<Modifier *>(obj)->getParent().lock().get();

	return nullptr;
}

MiniscriptInstructionOutcome RuntimeObject::ParentWriteProxyInterface::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) {
	RuntimeObject *parent = resolveObjectParent(static_cast<RuntimeObject *>(objectRef));

	if (!parent)
		return kMiniscriptInstructionOutcomeFailed;

	DynamicValueWriteProxy tempProxy;
	DynamicValueWriteObjectHelper::create(parent, tempProxy);

	return tempProxy.pod.ifc->refAttrib(thread, proxy, tempProxy.pod.objectRef, tempProxy.pod.ptrOrOffset, attrib);
}

MiniscriptInstructionOutcome RuntimeObject::ParentWriteProxyInterface::refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) {
	RuntimeObject *parent = resolveObjectParent(static_cast<RuntimeObject *>(objectRef));

	if (!parent)
		return kMiniscriptInstructionOutcomeFailed;

	DynamicValueWriteProxy tempProxy;
	DynamicValueWriteObjectHelper::create(parent, tempProxy);

	return tempProxy.pod.ifc->refAttribIndexed(thread, proxy, tempProxy.pod.objectRef, tempProxy.pod.ptrOrOffset, attrib, index);
}

MessageProperties::MessageProperties(const Event &evt, const DynamicValue &value, const Common::WeakPtr<RuntimeObject> &source)
	: _evt(evt), _value(value), _source(source) {
}

const Event &MessageProperties::getEvent() const {
	return _evt;
}

const DynamicValue& MessageProperties::getValue() const {
	return _value;
}

const Common::WeakPtr<RuntimeObject>& MessageProperties::getSource() const {
	return _source;
}

void MessageProperties::setValue(const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kList)
		_value.setList(value.getList()->clone());
	else
		_value = value;
}

WorldManagerInterface::WorldManagerInterface() : _gameMode(false), _combineRedraws(true), _postponeRedraws(false), _opInt(0) {
}

bool WorldManagerInterface::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "currentscene") {
		Common::SharedPtr<RuntimeObject> mainScene = thread->getRuntime()->getActiveMainScene();
		if (mainScene)
			result.setObject(mainScene->getSelfReference());
		else
			result.clear();
		return true;
	} else if (attrib == "monitordepth") {
		int bitDepth = displayModeToBitDepth(thread->getRuntime()->getFakeColorDepth());
		if (bitDepth <= 0)
			return false;

		result.setInt(bitDepth);
		return true;
	} else if (attrib == "gamemode") {
		result.setBool(_gameMode);
		return true;
	} else if (attrib == "combineredraws") {
		result.setBool(_combineRedraws);
		return true;
	} else if (attrib == "postponeredraws") {
		result.setBool(_postponeRedraws);
		return true;
	} else if (attrib == "clickcount") {
		result.setInt(thread->getRuntime()->getMultiClickCount());
		return true;
	}

	return RuntimeObject::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome WorldManagerInterface::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "currentscene") {
		DynamicValueWriteFuncHelper<WorldManagerInterface, &WorldManagerInterface::setCurrentScene, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "refreshcursor") {
		DynamicValueWriteFuncHelper<WorldManagerInterface, &WorldManagerInterface::setRefreshCursor, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "autoresetcursor") {
		DynamicValueWriteFuncHelper<WorldManagerInterface, &WorldManagerInterface::setAutoResetCursor, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "winsndbuffersize") {
		DynamicValueWriteFuncHelper<WorldManagerInterface, &WorldManagerInterface::setWinSndBufferSize, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "gamemode") {
		DynamicValueWriteBoolHelper::create(&_gameMode, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "combineredraws") {
		DynamicValueWriteBoolHelper::create(&_combineRedraws, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "postponeredraws") {
		DynamicValueWriteBoolHelper::create(&_postponeRedraws, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "qtpalettehack") {
		DynamicValueWriteDiscardHelper::create(result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "opint") {
		// This is used by SPQR before changing scenes in many instances.  It's not clear what it does.
		// It's usually set to 2 or 4, sometimes 7
		DynamicValueWriteIntegerHelper<int32>::create(&_opInt, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "scenefades") {
#ifdef MTROPOLIS_DEBUG_ENABLE
		if (Debugger *debugger = thread->getRuntime()->debugGetDebugger())
			debugger->notify(kDebugSeverityWarning, "'scenefades' attribute was set on WorldManager, which is implemented yet");
#endif
		DynamicValueWriteDiscardHelper::create(result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "cursor") {
		DynamicValueWriteFuncHelper<WorldManagerInterface, &WorldManagerInterface::setCursor, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}
	return RuntimeObject::writeRefAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome WorldManagerInterface::setCurrentScene(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kObject)
		return kMiniscriptInstructionOutcomeFailed;

	Common::SharedPtr<RuntimeObject> sceneObj = value.getObject().object.lock();
	if (!sceneObj) {
		thread->error("Failed to get scene reference");
		return kMiniscriptInstructionOutcomeFailed;
	}

	if (!sceneObj->isStructural()) {
		thread->error("Tried to change to a non-structural object as a scene");
		return kMiniscriptInstructionOutcomeFailed;
	}

	Structural *scene = static_cast<Structural *>(sceneObj.get());
	Structural *subsection = scene->getParent();
	if (!subsection->isSubsection()) {
		thread->error("Tried to change to a non-scene as a scene");
		return kMiniscriptInstructionOutcomeFailed;
	}

	// Note that this does NOT prevent transitioning to the same scene, which is intentional.
	// Transitioning to the current scene is allowed (and will fire Scene Ended+Scene Started events)
	bool addToReturnList = (_opInt & 0x02) != 0;
	bool addToDest = (_opInt & 0x01) != 0;

	_opInt = 0;	// Possibly inaccurate

	thread->getRuntime()->addSceneStateTransition(HighLevelSceneTransition(scene->getSelfReference().lock().staticCast<Structural>(), HighLevelSceneTransition::kTypeChangeToScene, addToDest, addToReturnList));

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome WorldManagerInterface::setRefreshCursor(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kBoolean)
		return kMiniscriptInstructionOutcomeFailed;

	if (value.getBool())
		thread->getRuntime()->forceCursorRefreshOnce();

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome WorldManagerInterface::setAutoResetCursor(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kBoolean)
		return kMiniscriptInstructionOutcomeFailed;

	thread->getRuntime()->setAutoResetCursor(value.getBool());

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome WorldManagerInterface::setWinSndBufferSize(MiniscriptThread *thread, const DynamicValue &value) {
	// Ignore
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome WorldManagerInterface::setCursor(MiniscriptThread *thread, const DynamicValue &value) {
	switch (value.getType())
	{
	case DynamicValueTypes::kNull:
		thread->getRuntime()->setCursorElement(Common::WeakPtr<VisualElement>());
		return kMiniscriptInstructionOutcomeContinue;
	case DynamicValueTypes::kObject: {
			Common::SharedPtr<RuntimeObject> obj = value.getObject().object.lock();
			if (obj && obj->isElement() && static_cast<Element *>(obj.get())->isVisual()) {
				thread->getRuntime()->setCursorElement(obj.staticCast<VisualElement>());
				return kMiniscriptInstructionOutcomeContinue;
			} else {
				thread->error("Object assigned as cursor wasn't a visual element");
				return kMiniscriptInstructionOutcomeFailed;
			}
		} break;
	default:
		thread->error("Value assigned as cursor wasn't an object");
		return kMiniscriptInstructionOutcomeFailed;
	}
}

SystemInterface::SystemInterface() : _masterVolume(kFullVolume) {
}

bool SystemInterface::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "mastervolume") {
		result.setInt(_masterVolume);
		return true;
	} else if (attrib == "monitorbitdepth") {
		int bitDepth = displayModeToBitDepth(thread->getRuntime()->getFakeColorDepth());
		if (bitDepth <= 0)
			return false;

		result.setInt(bitDepth);
		return true;
	} else if (attrib == "volumeismounted") {
		int volID = 0;
		bool isMounted = false;
		bool hasVolume = thread->getRuntime()->getVolumeState(_volumeName.c_str(), volID, isMounted);

		result.setBool(hasVolume && isMounted);
		return true;
	}

	return RuntimeObject::readAttribute(thread, result, attrib);
}

bool SystemInterface::readAttributeIndexed(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib, const DynamicValue &index) {
	if (attrib == "supportsbitdepth") {
		int32 asInteger = 0;
		if (!index.roundToInt(asInteger))
			return false;

		bool supported = false;
		ColorDepthMode mode = bitDepthToDisplayMode(asInteger);

		if (mode != kColorDepthModeInvalid)
			supported = thread->getRuntime()->isDisplayModeSupported(mode);

		result.setBool(supported);
		return true;
	}

	return RuntimeObject::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome SystemInterface::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "ejectcd") {
		DynamicValueWriteFuncHelper<SystemInterface, &SystemInterface::setEjectCD, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "gamemode") {
		DynamicValueWriteFuncHelper<SystemInterface, &SystemInterface::setGameMode, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "mastervolume") {
		DynamicValueWriteFuncHelper<SystemInterface, &SystemInterface::setMasterVolume, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "monitorbitdepth") {
		DynamicValueWriteFuncHelper<SystemInterface, &SystemInterface::setMonitorBitDepth, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "volumename") {
		DynamicValueWriteFuncHelper<SystemInterface, &SystemInterface::setVolumeName, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return RuntimeObject::writeRefAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome SystemInterface::setEjectCD(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kBoolean)
		return kMiniscriptInstructionOutcomeFailed;

	// If set to true, supposed to eject the CD.
	// Maybe we could dismount one of the runtime volumes here if we really wanted to, but ironically,
	// while volumeIsMounted supports multiple CD drives at once, ejectCD doesn't, so... do nothing?

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome SystemInterface::setGameMode(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kBoolean)
		return kMiniscriptInstructionOutcomeFailed;

	// Nothing to do here
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome SystemInterface::setMasterVolume(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	if (asInteger < 0)
		asInteger = 0;
	else if (asInteger > kFullVolume)
		asInteger = kFullVolume;

	thread->getRuntime()->setVolume(static_cast<double>(asInteger) / kFullVolume);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome SystemInterface::setMonitorBitDepth(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	const ColorDepthMode depthMode = bitDepthToDisplayMode(asInteger);
	if (depthMode != kColorDepthModeInvalid) {
		thread->getRuntime()->switchDisplayMode(thread->getRuntime()->getRealColorDepth(), depthMode);
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome SystemInterface::setVolumeName(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kString)
		return kMiniscriptInstructionOutcomeFailed;

	_volumeName = value.getString();

	return kMiniscriptInstructionOutcomeContinue;
}

StructuralHooks::~StructuralHooks() {
}

AssetManagerInterface::AssetManagerInterface() {
}

bool AssetManagerInterface::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "volumeismounted") {
		int volID = 0;
		bool isMounted = false;
		bool hasVolume = thread->getRuntime()->getVolumeState(_opString.c_str(), volID, isMounted);

		result.setBool(hasVolume && isMounted);
		return true;
	}
	return false;
}

MiniscriptInstructionOutcome AssetManagerInterface::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "opstring") {
		DynamicValueWriteStringHelper::create(&_opString, result);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "cdeject") {
		DynamicValueWriteDiscardHelper::create(result);
		return kMiniscriptInstructionOutcomeContinue;
	}
	return kMiniscriptInstructionOutcomeFailed;
}

void StructuralHooks::onCreate(Structural *structural) {
}

void StructuralHooks::onPostActivate(Structural *structural) {
}

void StructuralHooks::onSetPosition(Runtime *runtime, Structural *structural, const Common::Point &oldPt, Common::Point &pt) {
}

void StructuralHooks::onStopPlayingMToon(Structural *structural, bool &visible, bool &stopped, Graphics::ManagedSurface *lastSurf) {
}

void StructuralHooks::onHidden(Structural *structural, bool &visible) {
}

ProjectPresentationSettings::ProjectPresentationSettings() : width(640), height(480), bitsPerPixel(8) {
}

Structural::Structural() : Structural(nullptr) {
}

Structural::Structural(Runtime *runtime)
	: _parent(nullptr)
	, _paused(false)
	, _loop(false)
	, _flushPriority(0)
	, _runtime(runtime)
	, _sceneLoadState(SceneLoadState::kNotAScene) {
}

Structural::Structural(const Structural &other)
	: RuntimeObject(other), Debuggable(other), _parent(other._parent), _children(other._children), _modifiers(other._modifiers), _name(other._name), _assets(other._assets)
	, _paused(other._paused), _loop(other._loop), _flushPriority(other._flushPriority)/*, _hooks(other._hooks)*/, _runtime(other._runtime)
	, _sceneLoadState(SceneLoadState::kNotAScene) {
}

Structural::~Structural() {
}

void Structural::setHooks(const Common::SharedPtr<StructuralHooks> &hooks) {
	_hooks = hooks;
}

const Common::SharedPtr<StructuralHooks> &Structural::getHooks() const {
	return _hooks;
}

void Structural::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	for (Common::SharedPtr<Structural> &child : _children)
		visitor->visitChildStructuralRef(child);

	for (Common::SharedPtr<Modifier> &child : _modifiers)
		visitor->visitChildModifierRef(child);
}

bool Structural::isStructural() const {
	return true;
}

Structural::SceneLoadState Structural::getSceneLoadState() const {
	return _sceneLoadState;
}

void Structural::setSceneLoadState(SceneLoadState sceneLoadState) {
	_sceneLoadState = sceneLoadState;
}


bool Structural::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "name") {
		result.setString(_name);
		return true;
	} else if (attrib == "paused") {
		result.setBool(_paused);
		return true;
	} else if (attrib == "loop") {
		result.setBool(_paused);
		return true;
	} else if (attrib == "this") {
		// Yes, "this" is an attribute
		result.setObject(thread->getModifier()->getSelfReference());
		return true;
	} else if (attrib == "wm" || attrib == "worldmanager") {
		result.setObject(thread->getRuntime()->getWorldManagerInterface()->getSelfReference());
		return true;
	} else if (attrib == "assetmanager") {
		result.setObject(thread->getRuntime()->getAssetManagerInterface()->getSelfReference());
		return true;
	} else if (attrib == "system") {
		result.setObject(thread->getRuntime()->getSystemInterface()->getSelfReference());
		return true;
	} else if (attrib == "parent") {
		Structural *parent = getParent();
		if (parent)
			result.setObject(parent->getSelfReference());
		else
			result.clear();
		return true;
	} else if (attrib == "previous") {
		Structural *sibling = findPrevSibling();
		if (sibling)
			result.setObject(sibling->getSelfReference());
		else
			result.clear();
		return true;
	} else if (attrib == "next") {
		Structural *sibling = findNextSibling();
		if (sibling)
			result.setObject(sibling->getSelfReference());
		else
			result.clear();
		return true;
	} else if (attrib == "scene") {
		result.clear();

		RuntimeObject *possibleScene = this;
		while (possibleScene) {
			if (possibleScene->isModifier()) {
				possibleScene = static_cast<Modifier *>(possibleScene)->getParent().lock().get();
				continue;
			}

			if (possibleScene->isStructural()) {
				Structural *parent = static_cast<Structural *>(possibleScene)->getParent();
				if (parent->isSubsection())
					break;
				else {
					possibleScene = parent;
					continue;
				}
			}

			assert(false);
			break;
		}

		if (possibleScene)
			result.setObject(possibleScene->getSelfReference());
		else
			result.clear();
		return true;
	} else if (attrib == "section") {
		result.clear();

		RuntimeObject *possibleSection = this;
		while (possibleSection) {
			if (possibleSection->isSection())
				break;

			if (possibleSection->isModifier()) {
				possibleSection = static_cast<Modifier *>(possibleSection)->getParent().lock().get();
				continue;
			}

			if (possibleSection->isStructural()) {
				possibleSection = static_cast<Structural *>(possibleSection)->getParent();
				continue;
			}

			assert(false);
			break;
		}

		if (possibleSection)
			result.setObject(possibleSection->getSelfReference());
		else
			result.clear();
		return true;
	} else if (attrib == "subsection") {
		result.clear();

		RuntimeObject *possibleSubsection = this;
		while (possibleSubsection) {
			if (possibleSubsection->isSubsection())
				break;

			if (possibleSubsection->isModifier()) {
				possibleSubsection = static_cast<Modifier *>(possibleSubsection)->getParent().lock().get();
				continue;
			}

			if (possibleSubsection->isStructural()) {
				possibleSubsection = static_cast<Structural *>(possibleSubsection)->getParent();
				continue;
			}

			assert(false);
			break;
		}
		if (possibleSubsection)
			result.setObject(possibleSubsection->getSelfReference());
		else
			result.clear();
		return true;
	} else if (attrib == "flushpriority") {
		result.setInt(_flushPriority);
		return true;
	} else if (attrib == "firstchild") {
		// Despite documentation describing modifiers as children, "firstchild" on a structural element always returns
		// the first structural child.
		if (_children.size() == 0)
			result.clear();
		else
			result.setObject(_children[0]->getSelfReference());
		return true;
	} else if (attrib == "element") {
		result.setObject(getSelfReference());
		return true;
	} else if (attrib == "elementindex") {
		int32 elementIndex = 0;

		for (const Common::SharedPtr<Structural> &parentChild : _parent->getChildren()) {
			if (parentChild.get() == this)
				break;

			elementIndex++;
		}

		assert(static_cast<uint>(elementIndex) < _parent->getChildren().size());

		result.setInt(elementIndex + 1);
		return true;
	}

	if (RuntimeObject::readAttribute(thread, result, attrib))
		return true;

	if (_sceneLoadState == Structural::SceneLoadState::kSceneNotLoaded) {
#ifdef MTROPOLIS_DEBUG_ENABLE
		if (Debugger *debugger = thread->getRuntime()->debugGetDebugger())
			debugger->notify(kDebugSeverityError, "Hot-loading scenes is not yet implemented (readAttribute)");
#endif
	}

	// Traverse children (modifiers must be first)
	for (const Common::SharedPtr<Modifier> &modifier : _modifiers) {
		if (caseInsensitiveEqual(modifier->getName(), attrib)) {
			result.setObject(modifier);
			return true;
		}
	}

	for (const Common::SharedPtr<Structural> &child : _children) {
		if (caseInsensitiveEqual(child->getName(), attrib)) {
			result.setObject(child);
			return true;
		}
	}

	return false;
}

MiniscriptInstructionOutcome Structural::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "name") {
		DynamicValueWriteStringHelper::create(&_name, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "paused") {
		DynamicValueWriteFuncHelper<Structural, &Structural::scriptSetPaused, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "this") {
		// Yes, "this" is an attribute
		DynamicValueWriteObjectHelper::create(thread->getModifier(), result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "wm" || attrib == "worldmanager") {
		DynamicValueWriteObjectHelper::create(thread->getRuntime()->getWorldManagerInterface(), result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "assetmanager") {
		DynamicValueWriteObjectHelper::create(thread->getRuntime()->getAssetManagerInterface(), result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "system") {
		DynamicValueWriteObjectHelper::create(thread->getRuntime()->getSystemInterface(), result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "next") {
		Structural *sibling = findNextSibling();
		if (sibling) {
			DynamicValueWriteObjectHelper::create(sibling, result);
			return kMiniscriptInstructionOutcomeContinue;
		} else {
			return kMiniscriptInstructionOutcomeFailed;
		}
	} else if (attrib == "previous") {
		Structural *sibling = findPrevSibling();
		if (sibling) {
			DynamicValueWriteObjectHelper::create(sibling, result);
			return kMiniscriptInstructionOutcomeContinue;
		} else {
			return kMiniscriptInstructionOutcomeFailed;
		}
	} else if (attrib == "loop") {
		DynamicValueWriteFuncHelper<Structural, &Structural::scriptSetLoop, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "debug") {
		DynamicValueWriteFuncHelper<Structural, &Structural::scriptSetDebug, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "flushpriority") {
		DynamicValueWriteIntegerHelper<int32>::create(&_flushPriority, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "unload") {
		DynamicValueWriteFuncHelper<Structural, &Structural::scriptSetUnload, false>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	// Attempt to resolve as a child object
	// Modifiers are first, then structural
	for (const Common::SharedPtr<Modifier> &modifier : _modifiers) {
		if (caseInsensitiveEqual(modifier->getName(), attrib)) {
			DynamicValueWriteObjectHelper::create(modifier.get(), result);
			return kMiniscriptInstructionOutcomeContinue;
		}
	}

	for (const Common::SharedPtr<Structural> &child : _children) {
		if (caseInsensitiveEqual(child->getName(), attrib)) {
			DynamicValueWriteObjectHelper::create(child.get(), result);
			return kMiniscriptInstructionOutcomeContinue;
		}
	}

	return RuntimeObject::writeRefAttribute(thread, result, attrib);
}

bool Structural::readAttributeIndexed(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib, const DynamicValue &index) {
	if (attrib == "nthelement") {
		if (_sceneLoadState == Structural::SceneLoadState::kSceneNotLoaded) {
#ifdef MTROPOLIS_DEBUG_ENABLE
			if (Debugger *debugger = thread->getRuntime()->debugGetDebugger())
				debugger->notify(kDebugSeverityError, "Hot-loading scenes is not yet implemented (readAttributeIndexed)");
#endif
		}

		DynamicValue indexConverted;
		if (!index.convertToType(DynamicValueTypes::kInteger, indexConverted)) {
			thread->error("Invalid index for 'nthelement'");
			return false;
		}

		int32 indexInt = indexConverted.getInt();

		if (indexInt < 1 || static_cast<uint32>(indexInt) > _children.size()) {
			thread->error("Index out of range for 'nthelement'");
			return false;
		}

		result.setObject(_children[indexInt - 1]->getSelfReference());
		return true;
	}

	return RuntimeObject::readAttributeIndexed(thread, result, attrib, index);
}


const Common::Array<Common::SharedPtr<Structural> > &Structural::getChildren() const {
	return _children;
}

void Structural::addChild(const Common::SharedPtr<Structural>& child) {
	_children.push_back(child);
	child->setParent(this);
}

void Structural::removeAllChildren() {
	_children.clear();
}

void Structural::removeAllModifiers() {
	_modifiers.clear();
}

void Structural::removeChild(Structural* child) {
	for (size_t i = 0; i < _children.size(); i++) {
		if (_children[i].get() == child) {
			_children.remove_at(i);
			return;
		}
	}
}

void Structural::removeAllAssets() {
	_assets.clear();
}

void Structural::holdAssets(const Common::Array<Common::SharedPtr<Asset> >& assets) {
	_assets = assets;
}

Structural *Structural::getParent() const {
	return _parent;
}

Structural *Structural::findNextSibling() const {
	Structural *parent = getParent();
	if (parent) {
		const Common::Array<Common::SharedPtr<Structural> > &neighborhood = parent->getChildren();
		bool found = false;
		size_t foundIndex = 0;
		for (size_t i = 0; i < neighborhood.size(); i++) {
			if (neighborhood[i].get() == this) {
				foundIndex = i;
				found = true;
				break;
			}
		}

		if (found && foundIndex < neighborhood.size() - 1)
			return neighborhood[foundIndex + 1].get();
	}

	return nullptr;
}

Structural *Structural::findPrevSibling() const {
	Structural *parent = getParent();
	if (parent) {
		const Common::Array<Common::SharedPtr<Structural> > &neighborhood = parent->getChildren();
		bool found = false;
		size_t foundIndex = 0;
		for (size_t i = 0; i < neighborhood.size(); i++) {
			if (neighborhood[i].get() == this) {
				foundIndex = i;
				found = true;
				break;
			}
		}

		if (found && foundIndex > 0)
			return neighborhood[foundIndex - 1].get();
	}

	return nullptr;
}

Runtime *Structural::getRuntime() const {
	return _runtime;
}

void Structural::setParent(Structural *parent) {
	_parent = parent;
}

VisualElement *Structural::findScene() {
	Structural *parent = _parent;
	if (!parent)
		return nullptr;

	if (parent->isSubsection())
		return static_cast<VisualElement *>(this);
	else
		return parent->findScene();
}

const Common::String &Structural::getName() const {
	return _name;
}

const Common::Array<Common::SharedPtr<Modifier> > &Structural::getModifiers() const {
	return _modifiers;
}

void Structural::appendModifier(const Common::SharedPtr<Modifier> &modifier) {
	_modifiers.push_back(modifier);
	modifier->setParent(getSelfReference());
}

void Structural::removeModifier(const Modifier *modifier) {
	for (Common::Array<Common::SharedPtr<Modifier> >::iterator it = _modifiers.begin(), itEnd = _modifiers.end(); it != itEnd; ++it) {
		if (it->get() == modifier) {
			_modifiers.erase(it);
			return;
		}
	}
}

bool Structural::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState Structural::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	assert(false);
	return kVThreadError;
}

void Structural::materializeSelfAndDescendents(Runtime *runtime, ObjectLinkingScope *outerScope) {
	linkInternalReferences(outerScope);
	setRuntimeGUID(runtime->allocateRuntimeGUID());

	materializeDescendents(runtime, outerScope);

	_runtime = runtime;
}

void Structural::materializeDescendents(Runtime *runtime, ObjectLinkingScope *outerScope) {
	// Materialization is the step after objects are fully constructed and filled with data.
	// It does three things, recursively:
	// - Assigns all objects a new runtime GUID
	// - Clones any non-variable aliased modifiers
	// - Links any static GUIDs to in-scope visible objects
	// Objects are only ever materialized once
	//
	// Modifiers can see other modifiers but can't see sibling elements...
	// so the structural scope is effectively nested inside of the modifier scope.
	ObjectLinkingScope tempModifierScope;
	ObjectLinkingScope tempStructuralScope;
	ObjectLinkingScope *modifierScope = this->getPersistentModifierScope();
	ObjectLinkingScope *structuralScope = this->getPersistentStructuralScope();

	if (!modifierScope)
		modifierScope = &tempModifierScope;
	if (!structuralScope)
		structuralScope = &tempStructuralScope;

	modifierScope->setParent(outerScope);

	for (Common::Array<Common::SharedPtr<Modifier> >::iterator it = _modifiers.begin(), itEnd = _modifiers.end(); it != itEnd; ++it) {
		Common::SharedPtr<Modifier> &modifierRef = *it;

		runtime->instantiateIfAlias(modifierRef, getSelfReference());
		modifierScope->addObject(modifierRef->getStaticGUID(), modifierRef->getName(), modifierRef);
	}

	for (Common::Array<Common::SharedPtr<Modifier> >::const_iterator it = _modifiers.begin(), itEnd = _modifiers.end(); it != itEnd; ++it) {
		Modifier *modifier = it->get();
		modifier->materialize(runtime, modifierScope);
	}

	structuralScope->setParent(modifierScope);

	const Common::Array<Common::SharedPtr<Structural> > &children = this->getChildren();
	for (Common::Array<Common::SharedPtr<Structural> >::const_iterator it = children.begin(), itEnd = children.end(); it != itEnd; ++it) {
		Structural *child = it->get();
		structuralScope->addObject(child->getStaticGUID(), child->getName(), *it);
	}

	for (Common::Array<Common::SharedPtr<Structural> >::const_iterator it = children.begin(), itEnd = children.end(); it != itEnd; ++it) {
		Structural *child = it->get();
		child->materializeSelfAndDescendents(runtime, structuralScope);
	}
}

CORO_BEGIN_DEFINITION(Structural::StructuralConsumeCommandCoroutine)
	struct Locals {
		Common::ScopedPtr<MiniscriptThread> miniscriptThread;
		uint32 attribID;
		const Common::String *attribName;
		DynamicValueWriteProxy writeProxy;
		DynamicValue attribGetResult;
		bool quietlyDiscard;
	};

	CORO_BEGIN_FUNCTION
		CORO_IF(Event(EventIDs::kUnpause, 0).respondsTo(params->msg->getEvent()))
			if (params->self->_paused) {
				params->self->_paused = false;
				params->self->onPauseStateChanged();
			}

			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kUnpause, 0), DynamicValue(), params->self->getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, params->self, false, true, false));

			CORO_CALL(Runtime::SendMessageOnVThreadCoroutine, params->runtime, dispatch);

			CORO_RETURN;
		CORO_ELSE_IF(Event(EventIDs::kPause, 0).respondsTo(params->msg->getEvent()))
			if (params->self->_paused) {
				params->self->_paused = false;
				params->self->onPauseStateChanged();
			}

			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kPause, 0), DynamicValue(), params->self->getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, params->self, false, true, false));

			CORO_CALL(Runtime::SendMessageOnVThreadCoroutine, params->runtime, dispatch);

			CORO_RETURN;
		CORO_ELSE_IF(params->msg->getEvent().eventType == EventIDs::kAttribSet)
			locals->attribID = params->msg->getEvent().eventInfo;

			locals->attribName = params->runtime->resolveAttributeIDName(locals->attribID);

			CORO_IF(locals->attribName == nullptr)
#ifdef MTROPOLIS_DEBUG_ENABLE
				if (Debugger *debugger = params->runtime->debugGetDebugger())
					debugger->notifyFmt(kDebugSeverityError, "Attribute ID '%i' couldn't be resolved for Set Attribute message", static_cast<int>(locals->attribID));
#endif
				CORO_ERROR;
			CORO_END_IF

			locals->miniscriptThread.reset(new MiniscriptThread(params->runtime, params->msg, nullptr, nullptr, nullptr));

			CORO_AWAIT_MINISCRIPT(params->self->writeRefAttribute(locals->miniscriptThread.get(), locals->writeProxy, *locals->attribName));

			CORO_AWAIT_MINISCRIPT(locals->writeProxy.pod.ifc->write(locals->miniscriptThread.get(), params->msg->getValue(), locals->writeProxy.pod.objectRef, locals->writeProxy.pod.ptrOrOffset));

			CORO_RETURN;
		CORO_ELSE_IF(params->msg->getEvent().eventType == EventIDs::kAttribGet)
			locals->attribID = params->msg->getEvent().eventInfo;

			locals->attribName = params->runtime->resolveAttributeIDName(locals->attribID);
			CORO_IF(locals->attribName == nullptr)
#ifdef MTROPOLIS_DEBUG_ENABLE
				if (Debugger *debugger = params->runtime->debugGetDebugger())
					debugger->notifyFmt(kDebugSeverityError, "Attribute ID '%i' couldn't be resolved for Set Attribute message", static_cast<int>(locals->attribID));
#endif
				CORO_ERROR;
			CORO_END_IF

			locals->miniscriptThread.reset(new MiniscriptThread(params->runtime, params->msg, nullptr, nullptr, nullptr));

			CORO_IF(!params->self->readAttribute(locals->miniscriptThread.get(), locals->attribGetResult, *locals->attribName))
				CORO_ERROR;
			CORO_END_IF

			params->msg->setValue(locals->attribGetResult);

			CORO_RETURN;
		CORO_END_IF

		locals->quietlyDiscard = false;

		// Just ignore these
		const EventIDs::EventID ignoredIDs[] = {
			EventIDs::kPreloadMedia,
			EventIDs::kFlushMedia,
			EventIDs::kFlushAllMedia,
			EventIDs::kPrerollMedia
		};

		for (EventIDs::EventID evtID : ignoredIDs) {
			if (Event(evtID, 0).respondsTo(params->msg->getEvent())) {
				locals->quietlyDiscard = true;
				break;
			}
		}

		if (!locals->quietlyDiscard)
			warning("Command type %i was ignored", params->msg->getEvent().eventType);
	CORO_END_FUNCTION

CORO_END_DEFINITION

VThreadState Structural::asyncConsumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	runtime->getVThread().pushCoroutine<StructuralConsumeCommandCoroutine>(this, runtime, msg);
	return kVThreadReturn;
}

void Structural::activate() {
}

void Structural::deactivate() {
}

void Structural::recursiveCollectObjectsMatchingCriteria(Common::Array<Common::WeakPtr<RuntimeObject> > &results, bool (*evalFunc)(void *userData, RuntimeObject *object), void *userData, bool onlyEnabled) {
	if (evalFunc(userData, this))
		results.push_back(this->getSelfReference().lock());

	for (const Common::SharedPtr<Structural> &child : _children)
		child->recursiveCollectObjectsMatchingCriteria(results, evalFunc, userData, onlyEnabled);

	for (const Common::SharedPtr<Modifier> &modifier : _modifiers)
		modifier->recursiveCollectObjectsMatchingCriteria(results, evalFunc, userData, onlyEnabled);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
SupportStatus Structural::debugGetSupportStatus() const {
	return kSupportStatusNone;
}

const Common::String &Structural::debugGetName() const {
	return _name;
}

void Structural::debugInspect(IDebugInspectionReport *report) const {
	if (report->declareStatic("type"))
		report->declareStaticContents(debugGetTypeName());
	if (report->declareStatic("guid"))
		report->declareStaticContents(Common::String::format("%x", getStaticGUID()));
	if (report->declareStatic("runtimeID"))
		report->declareStaticContents(Common::String::format("%x", getRuntimeGUID()));
}

void Structural::debugSkipMovies() {
	for (Common::SharedPtr<Structural> &child : _children)
		child->debugSkipMovies();
}

#endif /* MTROPOLIS_DEBUG_ENABLE */

void Structural::linkInternalReferences(ObjectLinkingScope *scope) {
}

void Structural::onPauseStateChanged() {
}

ObjectLinkingScope *Structural::getPersistentStructuralScope() {
	return nullptr;
}

ObjectLinkingScope* Structural::getPersistentModifierScope() {
	return nullptr;
}

MiniscriptInstructionOutcome Structural::scriptSetPaused(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kBoolean)
		return kMiniscriptInstructionOutcomeFailed;

	const bool targetValue = value.getBool();

	if (targetValue == _paused)
		return kMiniscriptInstructionOutcomeContinue;

	_paused = targetValue;
	onPauseStateChanged();

	// Quirk: "pause" state changes during scene transitions don't fire "Paused" events.
	// This is necessary in Obsidian to prevent the rotator lever from triggering when leaving the menu
	// while at the Bureau light carousel, since the lever isn't flagged as paused but is set paused
	// via an init script, and the lever trigger is detected via the pause event.
	//
	// The event does, however, need to be sent immediately.
	if (!thread->getRuntime()->isAwaitingSceneTransition()) {
		Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(targetValue ? EventIDs::kPause : EventIDs::kUnpause, 0), DynamicValue(), getSelfReference()));
		Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
		thread->getRuntime()->sendMessageOnVThread(dispatch);
	}

	return kMiniscriptInstructionOutcomeYieldToVThread;
}

MiniscriptInstructionOutcome Structural::scriptSetLoop(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kBoolean)
		return kMiniscriptInstructionOutcomeFailed;

	_loop = value.getBool();

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome Structural::scriptSetDebug(MiniscriptThread *thread, const DynamicValue &value) {
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome Structural::scriptSetUnload(MiniscriptThread *thread, const DynamicValue &value) {
	// Doesn't matter what value this is set to, it always tries to unload.
	if (_sceneLoadState != SceneLoadState::kNotAScene)
		thread->getRuntime()->addSceneStateTransition(HighLevelSceneTransition(getSelfReference().lock().staticCast<Structural>(), HighLevelSceneTransition::kTypeRequestUnloadScene, false, false));

	return kMiniscriptInstructionOutcomeContinue;
}

VolumeState::VolumeState() : volumeID(0), isMounted(false) {
}

ObjectLinkingScope::ObjectLinkingScope() : _parent(nullptr) {
}

ObjectLinkingScope::~ObjectLinkingScope() {
}

void ObjectLinkingScope::setParent(ObjectLinkingScope *parent) {
	_parent = parent;
}

void ObjectLinkingScope::addObject(uint32 guid, const Common::String &name, const Common::WeakPtr<RuntimeObject> &object) {
	_guidToObject[guid] = object;

	if (name.size() > 0) {
		// Have to keep the first instance we find and ignore later instances.
		// Obsidian depends on this to properly resolve the scene destination when returning to the plane from the Statue
		// because there are two "sDest" variables but the first one is the correct one (and the one matching the GUID
		// link from the script that assigns to it)
		Common::WeakPtr<RuntimeObject> &objRef = _nameToObject[toCaseInsensitive(name)];

		if (objRef.expired())
			objRef = object;
	}
}

Common::WeakPtr<RuntimeObject> ObjectLinkingScope::resolve(uint32 staticGUID) const {
	if (staticGUID == 0)
		return Common::WeakPtr<RuntimeObject>();

	Common::HashMap<uint32, Common::WeakPtr<RuntimeObject> >::const_iterator it = _guidToObject.find(staticGUID);
	if (it != _guidToObject.end()) {
		return it->_value;
	} else {
		if (_parent)
			return _parent->resolve(staticGUID);

		return Common::WeakPtr<RuntimeObject>();
	}
}

Common::WeakPtr<RuntimeObject> ObjectLinkingScope::resolve(const Common::String &name, bool isNameAlreadyInsensitive) const {
	const Common::String *namePtr = &name;
	Common::String madeInsensitive;

	if (!isNameAlreadyInsensitive) {
		madeInsensitive = toCaseInsensitive(name);
		namePtr = &madeInsensitive;
	}

	Common::HashMap<Common::String, Common::WeakPtr<RuntimeObject> >::const_iterator it = _nameToObject.find(*namePtr);
	if (it != _nameToObject.end()) {
		return it->_value;
	} else {
		if (_parent)
			return _parent->resolve(*namePtr, true);

		return Common::WeakPtr<RuntimeObject>();
	}
}

Common::WeakPtr<RuntimeObject> ObjectLinkingScope::resolve(uint32 staticGUID, const Common::String &name, bool isNameAlreadyInsensitive) const {
	Common::WeakPtr<RuntimeObject> byGUIDResult = resolve(staticGUID);
	if (!byGUIDResult.expired())
		return byGUIDResult;
	else {
		Common::WeakPtr<RuntimeObject> fallback = resolve(name, isNameAlreadyInsensitive);
		if (fallback.expired()) {
			warning("Couldn't resolve static guid '%x' with name '%s'", staticGUID, name.c_str());
		}
		return fallback;
	}
}

void ObjectLinkingScope::reset() {
	_parent = nullptr;
	_guidToObject.clear(true);
}

LowLevelSceneStateTransitionAction::LowLevelSceneStateTransitionAction(const Common::SharedPtr<MessageDispatch> &msg)
	: _actionType(LowLevelSceneStateTransitionAction::kSendMessage), _msg(msg) {
}

LowLevelSceneStateTransitionAction::LowLevelSceneStateTransitionAction(ActionType actionType)
	: _actionType(actionType) {
}

LowLevelSceneStateTransitionAction::LowLevelSceneStateTransitionAction(const LowLevelSceneStateTransitionAction &other)
	: _actionType(other._actionType), _msg(other._msg), _scene(other._scene) {
}

LowLevelSceneStateTransitionAction::LowLevelSceneStateTransitionAction(const Common::SharedPtr<Structural> &scene, ActionType actionType)
	: _scene(scene), _actionType(actionType) {
}

LowLevelSceneStateTransitionAction::ActionType LowLevelSceneStateTransitionAction::getActionType() const {
	return _actionType;
}

const Common::SharedPtr<Structural>& LowLevelSceneStateTransitionAction::getScene() const {
	return _scene;
}

const Common::SharedPtr<MessageDispatch>& LowLevelSceneStateTransitionAction::getMessage() const {
	return _msg;
}

LowLevelSceneStateTransitionAction &LowLevelSceneStateTransitionAction::operator=(const LowLevelSceneStateTransitionAction &other) {
	_scene = other._scene;
	_msg = other._msg;
	_actionType = other._actionType;

	return *this;
}


HighLevelSceneTransition::HighLevelSceneTransition(const Common::SharedPtr<Structural> &hlst_scene, Type hlst_type, bool hlst_addToDestinationScene, bool hlst_addToReturnList)
	: scene(hlst_scene), type(hlst_type), addToDestinationScene(hlst_addToDestinationScene), addToReturnList(hlst_addToReturnList) {
}

ObjectParentChange::ObjectParentChange(const Common::WeakPtr<RuntimeObject> &object, const Common::WeakPtr<RuntimeObject> &newParent)
	: _object(object), _newParent(newParent) {
}

SceneTransitionEffect::SceneTransitionEffect()
	: _duration(100000), _steps(64), _transitionType(SceneTransitionTypes::kNone), _transitionDirection(SceneTransitionDirections::kUp) {
}

MessageDispatch::MessageDispatch(const Common::SharedPtr<MessageProperties> &msgProps, Structural *root, bool cascade, bool relay, bool couldBeCommand)
	: _cascade(cascade), _relay(relay), _msg(msgProps), _isCommand(false), _rootType(RootType::Structural) {
	if (couldBeCommand && EventIDs::isCommand(msgProps->getEvent().eventType)) {
		_isCommand = true;
		_rootType = RootType::Command;
	}

	_root = root->getSelfReference();
}

MessageDispatch::MessageDispatch(const Common::SharedPtr<MessageProperties> &msgProps, Modifier *root, bool cascade, bool relay, bool couldBeCommand)
	: _cascade(cascade), _relay(relay), _msg(msgProps), _isCommand(false), _rootType(RootType::Modifier) {

	// Apparently if a command message is sent to a modifier, it's handled as a message.
	// SPQR depends on this to send "Element Select" messages to pick the palette.
	_root = root->getSelfReference();
}

const Common::SharedPtr<MessageProperties> &MessageDispatch::getMsg() const {
	return _msg;
}

bool MessageDispatch::isCascade() const {
	return _cascade;
}

bool MessageDispatch::isRelay() const {
	return _relay;
}

MessageDispatch::RootType MessageDispatch::getRootType() const {
	return _rootType;
}

const Common::WeakPtr<RuntimeObject> &MessageDispatch::getRootWeakPtr() const {
	return _root;
}


RuntimeObject *MessageDispatch::getRootPropagator() const {
	return _root.lock().get();
}

KeyEventDispatch::KeyEventDispatch(const Common::SharedPtr<KeyboardInputEvent> &evt) : _dispatchIndex(0), _evt(evt) {
}

Common::Array<Common::WeakPtr<RuntimeObject> >& KeyEventDispatch::getKeyboardMessengerArray() {
	return _keyboardMessengers;
}

bool KeyEventDispatch::keyboardMessengerFilterFunc(void *userData, RuntimeObject *object) {
	if (!object->isModifier())
		return false;

	return static_cast<Modifier *>(object)->isKeyboardMessenger();
}

bool KeyEventDispatch::isTerminated() const {
	return _dispatchIndex == _keyboardMessengers.size();
}

VThreadState KeyEventDispatch::continuePropagating(Runtime *runtime) {
	KeyboardInputEvent *evt = _evt.get();

	// This is kind of messy but we have to guard against situations where a key event triggers a clone
	// which may itself contain a keyboard messenger.  (Do multiple messengers respond to keystrokes?)
	while (_dispatchIndex < _keyboardMessengers.size()) {
		Common::SharedPtr<RuntimeObject> obj = _keyboardMessengers[_dispatchIndex++].lock();
		assert(obj->isModifier());

		Modifier *modifier = static_cast<Modifier *>(obj.get());
		assert(modifier->isKeyboardMessenger());

		KeyboardMessengerModifier *msgr = static_cast<KeyboardMessengerModifier *>(modifier);
		Common::String charStr;
		if (msgr->checkKeyEventTrigger(runtime, evt->getKeyEventType(), evt->isRepeat(), evt->getKeyState(), charStr)) {
			msgr->dispatchMessage(runtime, charStr);
			return kVThreadReturn;
		}
	}

	return kVThreadReturn;
}

void ScheduledEvent::cancel() {
	if (_scheduler)
		_scheduler->removeEvent(this);

	_scheduler = nullptr;
}

uint64 ScheduledEvent::getScheduledTime() const {
	return _scheduledTime;
}

void ScheduledEvent::activate(Runtime *runtime) const {
	_activateFunc(this->_obj, runtime);
}


ScheduledEvent::ScheduledEvent(void *obj, void (*activateFunc)(void *, Runtime *runtime), uint64 scheduledTime, Scheduler *scheduler)
	: _obj(obj), _activateFunc(activateFunc), _scheduledTime(scheduledTime), _scheduler(scheduler) {
}

Scheduler::Scheduler() {
}

Scheduler::~Scheduler() {
	for (const Common::SharedPtr<ScheduledEvent> &evt : _events)
		evt->_scheduler = nullptr;

	_events.clear();
}

Common::SharedPtr<ScheduledEvent> Scheduler::getFirstEvent() const {
	if (_events.size() > 0)
		return _events[0];
	return nullptr;
}

void Scheduler::descheduleFirstEvent() {
	_events.remove_at(0);
}

void Scheduler::insertEvent(const Common::SharedPtr<ScheduledEvent> &evt) {
	uint32 t = evt->getScheduledTime();
	size_t insertionIndex = 0;
	while (insertionIndex < _events.size()) {
		if (_events[insertionIndex]->getScheduledTime() > t)
			break;
		insertionIndex++;
	}

	_events.insert_at(insertionIndex, evt);
}

void Scheduler::removeEvent(const ScheduledEvent *evt) {
	for (size_t i = 0; i < _events.size(); i++) {
		if (_events[i].get() == evt) {
			_events[i].get()->_scheduler = nullptr;
			_events.remove_at(i);
			break;
		}
	}
}

class DefaultCursor : public Graphics::Cursor {
public:
	uint16 getWidth() const override { return 16; }
	uint16 getHeight() const override { return 16; }
	uint16 getHotspotX() const override { return 1; }
	uint16 getHotspotY() const override { return 2; }
	byte getKeyColor() const override { return 0; }

	const byte *getSurface() const override { return _cursorGraphic; }

	const byte *getPalette() const override { return _cursorPalette; }
	byte getPaletteStartIndex() const override { return 0; }
	uint16 getPaletteCount() const override { return 3; }

private:
	static const byte _cursorGraphic[256];
	static const byte _cursorPalette[9];
};

const byte DefaultCursor::_cursorGraphic[256] = {
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0,
	1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
	1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 2, 1, 1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 1, 0, 1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
};

const byte DefaultCursor::_cursorPalette[9] = {
	255, 0, 255,
	0, 0, 0,
	255, 255, 255
};


class DefaultCursorGraphic : public CursorGraphic {
public:
	Graphics::Cursor *getCursor() const override;

private:
	mutable DefaultCursor _cursor;
};

Graphics::Cursor *DefaultCursorGraphic::getCursor() const {
	return &_cursor;
}


OSEvent::OSEvent(OSEventType eventType) : _eventType(eventType) {
}

OSEvent::~OSEvent() {
}

OSEventType OSEvent::getEventType() const {
	return _eventType;
}

MouseInputEvent::MouseInputEvent(OSEventType eventType, int32 x, int32 y, Actions::MouseButton button) : OSEvent(eventType), _x(x), _y(y), _button(button) {
}

int32 MouseInputEvent::getX() const {
	return _x;
}

int32 MouseInputEvent::getY() const {
	return _y;
}

Actions::MouseButton MouseInputEvent::getButton() const {
	return _button;
}


KeyboardInputEvent::KeyboardInputEvent(OSEventType osEventType, Common::EventType keyEventType, bool repeat, const Common::KeyState &keyEvt)
	: OSEvent(osEventType), _keyEventType(keyEventType), _repeat(repeat), _keyEvt(keyEvt) {
}

Common::EventType KeyboardInputEvent::getKeyEventType() const {
	return _keyEventType;
}

bool KeyboardInputEvent::isRepeat() const {
	return _repeat;
}

const Common::KeyState &KeyboardInputEvent::getKeyState() const {
	return _keyEvt;
}

ActionEvent::ActionEvent(OSEventType osEventType, Actions::Action action) : OSEvent(osEventType), _action(action) {
}

Actions::Action ActionEvent::getAction() const {
	return _action;
}

Runtime::SceneStackEntry::SceneStackEntry() {
}

Runtime::Teardown::Teardown() : onlyRemoveChildren(false) {
}

Runtime::SceneReturnListEntry::SceneReturnListEntry() : isAddToDestinationSceneTransition(false) {
}

Runtime::DispatchActionTaskData::DispatchActionTaskData() : action(Actions::kNone) {
}

Runtime::ConsumeMessageTaskData::ConsumeMessageTaskData() : consumer(nullptr) {
}

Runtime::ConsumeCommandTaskData::ConsumeCommandTaskData() : structural(nullptr) {
}

Runtime::ApplyDefaultVisibilityTaskData::ApplyDefaultVisibilityTaskData() : element(nullptr), targetVisibility(false) {
}


Runtime::UpdateMouseStateTaskData::UpdateMouseStateTaskData() : mouseDown(false) {
}

Runtime::UpdateMousePositionTaskData::UpdateMousePositionTaskData() : x(0), y(0) {
}

Runtime::CollisionCheckState::CollisionCheckState() : collider(nullptr) {
}

Runtime::BoundaryCheckState::BoundaryCheckState() : detector(nullptr), currentContacts(0), positionResolved(false) {
}

Runtime::ColliderInfo::ColliderInfo() : sceneStackDepth(0), layer(0), element(nullptr) {
}

DragMotionProperties::DragMotionProperties() : constraintDirection(kConstraintDirectionNone), constrainToParent(false) {
}

SceneTransitionHooks::~SceneTransitionHooks() {
}

void SceneTransitionHooks::onSceneTransitionSetup(Runtime *runtime, const Common::WeakPtr<Structural> &oldScene, const Common::WeakPtr<Structural> &newScene) {
}

void SceneTransitionHooks::onSceneTransitionEnded(Runtime *runtime, const Common::WeakPtr<Structural> &newScene) {
}

void SceneTransitionHooks::onProjectStarted(Runtime *runtime) {
}


Palette::Palette() {
	initDefaultPalette(1);
}

void Palette::initDefaultPalette(int version) {
	// NOTE: The "V2" palette is correct for Unit: Rebooted.
	// Is it correct for all V2 apps?
	assert(version == 1 || version == 2);
	int outColorIndex = 0;
	for (int rb = 0; rb < 6; rb++) {
		for (int rg = 0; rg < 6; rg++) {
			for (int rr = 0; rr < 6; rr++) {
				byte *color = _colors + outColorIndex * 3;
				outColorIndex++;

				if (version == 1) {
					color[0] = 255 - rr * 51;
					color[1] = 255 - rg * 51;
					color[2] = 255 - rb * 51;
				} else {
					color[2] = 255 - rr * 51;
					color[1] = 255 - rg * 51;
					color[0] = 255 - rb * 51;
				}
			}
		}
	}

	outColorIndex--;

	for (int ch = 0; ch < 4; ch++) {
		for (int ri = 0; ri < 16; ri++) {
			if (ri % 3 == 0)
				continue;

			byte *color = _colors + outColorIndex * 3;
			outColorIndex++;

			byte intensity = 255 - ri * 17;

			if (ch == 3) {
				color[0] = color[1] = color[2] = intensity;
			} else {
				color[0] = color[1] = color[2] = 0;
				color[ch] = intensity;
			}
		}
	}

	assert(outColorIndex == 255);

	if (version == 1) {
		_colors[255 * 3 + 0] = 0;
		_colors[255 * 3 + 1] = 0;
		_colors[255 * 3 + 2] = 0;
	} else {
		_colors[0 * 3 + 0] = 0;
		_colors[0 * 3 + 1] = 0;
		_colors[0 * 3 + 2] = 0;
		_colors[255 * 3 + 0] = 255;
		_colors[255 * 3 + 1] = 255;
		_colors[255 * 3 + 2] = 255;
	}
}

Palette::Palette(const ColorRGB8 *colors) {
	for (int i = 0; i < 256; i++) {
		_colors[i * 3 + 0] = colors[i].r;
		_colors[i * 3 + 1] = colors[i].g;
		_colors[i * 3 + 2] = colors[i].b;
	}
}

const byte *Palette::getPalette() const {
	return _colors;
}

Runtime::Runtime(OSystem *system, Audio::Mixer *mixer, ISaveUIProvider *saveProvider, ILoadUIProvider *loadProvider, const Common::SharedPtr<SubtitleRenderer> &subRenderer)
	: _system(system), _mixer(mixer), _saveProvider(saveProvider), _loadProvider(loadProvider),
	  _nextRuntimeGUID(1), _realDisplayMode(kColorDepthModeInvalid), _fakeDisplayMode(kColorDepthModeInvalid),
	  _displayWidth(1024), _displayHeight(768), _realTime(0), _realTimeBase(0), _playTime(0), _playTimeBase(0), _sceneTransitionState(kSceneTransitionStateNotTransitioning),
	  _lastFrameCursor(nullptr), _lastFrameMouseVisible(false), _defaultCursor(new DefaultCursorGraphic()),
	  _cachedMousePosition(Common::Point(0, 0)), _realMousePosition(Common::Point(0, 0)), _trackedMouseOutside(false),
	  _forceCursorRefreshOnce(true), _autoResetCursor(false), _haveModifierOverrideCursor(false), _haveCursorElement(false), _sceneGraphChanged(false), _isQuitting(false),
	  _collisionCheckTime(0), /*_elementCursorUpdateTime(0), */_defaultVolumeState(true), _activeSceneTransitionEffect(nullptr), _sceneTransitionStartTime(0), _sceneTransitionEndTime(0),
	  _sharedSceneWasSetExplicitly(false), _modifierOverrideCursorID(0), _subtitleRenderer(subRenderer), _multiClickStartTime(0), _multiClickInterval(500), _multiClickCount(0), _numMouseBlockers(0),
	  _pendingSceneReturnCount(0) {
	_random.reset(new Common::RandomSource("mtropolis"));

	_coroManager.reset(ICoroutineManager::create());
	_vthread.reset(new VThread(_coroManager.get()));

	for (int i = 0; i < kColorDepthModeCount; i++) {
		_displayModeSupported[i] = false;
		_realDisplayMode = kColorDepthModeInvalid;
		_fakeDisplayMode = kColorDepthModeInvalid;
	}

	_realTimeBase = system->getMillis();
	_playTimeBase = system->getMillis();

	for (int i = 0; i < Actions::kMouseButtonCount; i++)
		_mouseFocusFlags[i] = false;

	_worldManagerInterface.reset(new WorldManagerInterface());
	_worldManagerInterface->setSelfReference(_worldManagerInterface);

	_assetManagerInterface.reset(new AssetManagerInterface());
	_assetManagerInterface->setSelfReference(_assetManagerInterface);

	_systemInterface.reset(new SystemInterface());
	_systemInterface->setSelfReference(_systemInterface);

	_getSetAttribIDsToAttribName[AttributeIDs::kAttribCache] = "cache";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribDirect] = "direct";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribVisible] = "visible";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribLayer] = "layer";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribPaused] = "paused";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribVisible] = "loop";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribPosition] = "position";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribWidth] = "width";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribHeight] = "height";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribRate] = "rate";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribRange] = "range";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribCel] = "cel";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribLoopBackForth] = "loopbackforth";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribPlayEveryFrame] = "playeveryframe";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribTimeValue] = "timevalue";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribTrackDisable] = "trackdisable";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribTrackEnable] = "trackenable";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribVolume] = "volume";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribBalance] = "balance";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribText] = "text";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribMasterVolume] = "mastervolume";
	_getSetAttribIDsToAttribName[AttributeIDs::kAttribUserTimeout] = "usertimeout";
}

Runtime::~Runtime() {
	// Clear the project first, which should detach any references to other things
	unloadProject();

	_subtitleRenderer.reset();
}

bool Runtime::runFrame() {
	uint32 timeMillis = _system->getMillis();

	uint32 realMSec = timeMillis - _realTimeBase - _realTime;

	_realTime = timeMillis - _realTimeBase;
	_playTime = timeMillis - _playTimeBase;

	for (;;) {
#ifdef MTROPOLIS_DEBUG_ENABLE
		if (_debugger && _debugger->isPaused())
			break;
#endif

		VThreadState state = _vthread->step();
		if (state != kVThreadReturn) {
			// Still doing blocking tasks
			break;
		}

		if (_sceneTransitionState != kSceneTransitionStateTransitioning && _osEventQueue.size() > 0) {
			Common::SharedPtr<OSEvent> evt = _osEventQueue[0];
			_osEventQueue.remove_at(0);

			OSEventType evtType = evt->getEventType();
			switch (evtType) {
			case kOSEventTypeKeyboard:
				if (_project) {
					Common::SharedPtr<KeyEventDispatch> dispatch(new KeyEventDispatch(evt.staticCast<KeyboardInputEvent>()));

					// We don't want to filter by enabled because of the edge case where a keyboard
					// messenger fires and disables or enables other keyboard messengers.
					// Not sure this is actually possible though... can multiple messengers respond to
					// the same keystroke?  Not sure.
					_project->recursiveCollectObjectsMatchingCriteria(dispatch->getKeyboardMessengerArray(), KeyEventDispatch::keyboardMessengerFilterFunc, dispatch.get(), false);

					if (dispatch->getKeyboardMessengerArray().size() > 0) {
						DispatchKeyTaskData *taskData = _vthread->pushTask("Runtime::dispatchKeyTask", this, &Runtime::dispatchKeyTask);
						taskData->dispatch = dispatch;
					}

					_project->onKeyboardEvent(this, *static_cast<KeyboardInputEvent *>(evt.get()));
				}
				break;
			case kOSEventTypeMouseDown:
			case kOSEventTypeMouseUp:
			case kOSEventTypeMouseMove: {
					MouseInputEvent *mouseEvt = static_cast<MouseInputEvent *>(evt.get());

					if (evtType == kOSEventTypeMouseDown) {
						if (_multiClickStartTime + _multiClickInterval <= _playTime) {
							_multiClickStartTime = _playTime;
							_multiClickCount = 1;
						} else
							_multiClickCount++;
					}

					// Maybe shouldn't post the update mouse button task if non-left buttons are pressed?
					if ((evtType == kOSEventTypeMouseDown || evtType == kOSEventTypeMouseUp) && mouseEvt->getButton() == Actions::kMouseButtonLeft) {
						UpdateMouseStateTaskData *taskData = _vthread->pushTask("Runtime::updateMouseStateTask", this, &Runtime::updateMouseStateTask);
						taskData->mouseDown = (evtType == kOSEventTypeMouseDown);
					}

					// Pushed second, so this happens first
					if (mouseEvt->getX() != _cachedMousePosition.x || mouseEvt->getY() != _cachedMousePosition.y) {
						UpdateMousePositionTaskData *taskData = _vthread->pushTask("Runtime::updateMousePositionTask", this, &Runtime::updateMousePositionTask);
						taskData->x = mouseEvt->getX();
						taskData->y = mouseEvt->getY();
					}
				} break;
			case kOSEventTypeAction: {
					Actions::Action action = static_cast<ActionEvent *>(evt.get())->getAction();

					DispatchActionTaskData *taskData = _vthread->pushTask("Runtime::dispatchAction", this, &Runtime::dispatchActionTask);
					taskData->action = action;
				} break;
			default:
				break;
			}
			continue;
		}

		if (_forceCursorRefreshOnce) {
			_forceCursorRefreshOnce = false;

			UpdateMousePositionTaskData *taskData = _vthread->pushTask("Runtime::updateMousePositionTask", this, &Runtime::updateMousePositionTask);
			taskData->x = _cachedMousePosition.x;
			taskData->y = _cachedMousePosition.y;
			continue;
		}

		if (_queuedProjectDesc) {
			Common::SharedPtr<ProjectDescription> desc = _queuedProjectDesc;
			_queuedProjectDesc.reset();

			unloadProject();

			_macFontMan.reset(new Graphics::MacFontManager(0, desc->getLanguage()));

			_project.reset(new Project(this));
			_project->setSelfReference(_project);

			_project->loadFromDescription(*desc, getHacks());

			ensureMainWindowExists();

			_rootLinkingScope.addObject(_project->getStaticGUID(), _project->getName(), _project);

			// We have to materialize global variables because they are not cloned from aliases.
			debug(1, "Materializing global variables...");
			_project->materializeGlobalVariables(this, &_rootLinkingScope);

			debug(1, "Materializing project...");
			_project->materializeSelfAndDescendents(this, &_rootLinkingScope);

			for (const Common::SharedPtr<Structural> &section : _project->getChildren()) {
				for (const Common::SharedPtr<Structural> &subsection : section->getChildren()) {
					for (const Common::SharedPtr<Structural> &scene : subsection->getChildren())
						scene->setSceneLoadState(Structural::SceneLoadState::kSceneNotLoaded);
				}
			}

			debug(1, "Project is fully loaded!  Starting up...");

			if (_project->getChildren().size() == 0) {
				error("Project has no sections");
			}

			Structural *firstSection = _project->getChildren()[0].get();
			if (firstSection->getChildren().size() == 0) {
				error("Project has no subsections");
			}

			Structural *firstSubsection = firstSection->getChildren()[0].get();
			if (firstSubsection->getChildren().size() < 2) {
				error("Project has no subsections");
			}

			Common::SharedPtr<MessageProperties> psProps(new MessageProperties(Event(EventIDs::kProjectStarted, 0), DynamicValue(), _project->getSelfReference()));
			Common::SharedPtr<MessageDispatch> psDispatch(new MessageDispatch(psProps, _project.get(), false, true, false));
			queueMessage(psDispatch);

			for (const Common::SharedPtr<SceneTransitionHooks> &hook : _hacks.sceneTransitionHooks)
				hook->onProjectStarted(this);

			_pendingSceneTransitions.push_back(HighLevelSceneTransition(firstSubsection->getChildren()[1], HighLevelSceneTransition::kTypeChangeToScene, false, false));
			continue;
		}

		// The order of operations for dynamic object behaviors is:
		// - Parent changes
		// - Parent Enabled -> Clone for each cloned object
		// - Show for each cloned object that is visible
		// - Hide -> Kill -> Parent Disabled for each killed object
		if (_pendingParentChanges.size() > 0) {
			ObjectParentChange parentChange = _pendingParentChanges.remove_at(0);

			RuntimeObject *obj = parentChange._object.lock().get();
			RuntimeObject *newParent = parentChange._newParent.lock().get();

			if (obj) {
				if (newParent)
					executeChangeObjectParent(obj, newParent);
				else
					warning("Object re-parenting failed, the new parent was invalid!");
			}

			continue;
		}

		if (_pendingClones.size() > 0) {
			RuntimeObject *objectToClone = _pendingClones.remove_at(0).lock().get();

			if (objectToClone)
				executeCloneObject(objectToClone);

			continue;
		}

		if (_pendingShowClonedObject.size() > 0) {
			Structural *objectToShow = _pendingShowClonedObject.remove_at(0).lock().get();

			if (objectToShow && objectToShow->isElement() && static_cast<Element *>(objectToShow)->isVisual() && static_cast<VisualElement *>(objectToShow)->isVisible()) {
				Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kElementShow, 0), DynamicValue(), objectToShow->getSelfReference()));
				Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, objectToShow, false, true, false));
				sendMessageOnVThread(dispatch);
			}

			continue;
		}

		if (_pendingKills.size() > 0) {
			RuntimeObject *objectToKill = _pendingKills.remove_at(0).lock().get();

			if (objectToKill)
				executeKillObject(objectToKill);

			continue;
		}

		// Teardowns must only occur during idle conditions where there are no VThread tasks
		if (_pendingTeardowns.size() > 0) {
			for (Common::Array<Teardown>::const_iterator it = _pendingTeardowns.begin(), itEnd = _pendingTeardowns.end(); it != itEnd; ++it) {
				executeTeardown(*it);
			}
			_pendingTeardowns.clear();
			_sceneGraphChanged = true;
			continue;
		}

		if (_pendingLowLevelTransitions.size() > 0) {
			LowLevelSceneStateTransitionAction transition = _pendingLowLevelTransitions[0];
			_pendingLowLevelTransitions.remove_at(0);

			executeLowLevelSceneStateTransition(transition);
			continue;
		}

		// This has to be in this specific spot: Queued messages that occur from scene transitions are normally discharged
		// after Scene Started, Scene Changed, and element Show events, but before scene transition.
		//
		// Obsidian depends on this behavior in several scripts, most notably setting up conditional ambience correctly.
		// For example, in Inspiration chapter, on exiting the plane into the statue:
		// Shared scene fires Parent Enabled which triggers "GEN_SND_Start_Ambience on PE" which sends GEN_SND_Start_Ambience
		// but not immediately, so it goes into the queue.
		// After the main scene loads, it fires Scene Started and which in turn triggers "NAV_setup_navigation on SS" which
		// sends NAV_setup_navigation immediately, which sets the current nav node variable.
		// Then, the queued GEN_SND_Start_Ambience can read the nav node variable to set up ambience correctly.
		//
		// If messages are discharged before low-level scene transitions, then the music plays in the lower level of the
		// statue on disembarking because the nav node variable is set to the wrong value.
		if (_messageQueue.size() > 0) {
			Common::SharedPtr<MessageDispatch> msg = _messageQueue[0];
			_messageQueue.remove_at(0);

			sendMessageOnVThread(msg);
			continue;
		}

		// Scene returns are processed before transitions even if the transition was
		// executed first.  SPQR requires this behavior to properly return from the
		// menu, since it sets a scene change via WorldManager to the restored scene
		// and then triggers a Return modifier which would return to the last scene
		// that added to the return list, which in this case is the scene that was
		// active before opening the menu.
		if (_pendingSceneReturnCount > 0) {
			_pendingSceneReturnCount--;

			executeHighLevelSceneReturn();
			_sceneGraphChanged = true;
			continue;
		}

		if (_pendingSceneTransitions.size() > 0) {
			HighLevelSceneTransition transition = _pendingSceneTransitions[0];
			_pendingSceneTransitions.remove_at(0);

			executeHighLevelSceneTransition(transition);
			_sceneGraphChanged = true;
			continue;
		}

		if (_sceneTransitionState == kSceneTransitionStateWaitingForDraw) {
			if (_sourceSceneTransitionEffect._transitionType != SceneTransitionTypes::kNone)
				_activeSceneTransitionEffect = &_sourceSceneTransitionEffect;
			else if (_destinationSceneTransitionEffect._transitionType != SceneTransitionTypes::kNone)
				_activeSceneTransitionEffect = &_destinationSceneTransitionEffect;
			else
				_activeSceneTransitionEffect = nullptr;

			_sceneTransitionState = kSceneTransitionStateTransitioning;
			_sceneTransitionStartTime = _playTime;

			uint32 transitionDuration = 0;

			if (_activeSceneTransitionEffect) {
				transitionDuration = _activeSceneTransitionEffect->_duration;

				if (transitionDuration < _hacks.minTransitionDuration)
					transitionDuration = _hacks.minTransitionDuration;
			}

			if (transitionDuration == 0) {
				// No transition at all.  This needs to skip past the transition phase and hit the next condition
				_sceneTransitionEndTime = _playTime;
			} else {
				_sceneTransitionEndTime = _playTime + transitionDuration;

				if (!_mainWindow.expired()) {
					Common::SharedPtr<Window> mainWindow = _mainWindow.lock();
					_sceneTransitionOldFrame.reset(new Graphics::ManagedSurface());
					_sceneTransitionNewFrame.reset(new Graphics::ManagedSurface());

					_sceneTransitionOldFrame->copyFrom(*mainWindow->getSurface());

					Render::renderProject(this, mainWindow.get(), nullptr);

					_sceneTransitionNewFrame->copyFrom(*mainWindow->getSurface());
				}
			}
		}

		if (_sceneTransitionState == kSceneTransitionStateTransitioning && _playTime >= _sceneTransitionEndTime) {
			_sceneTransitionState = kSceneTransitionStateNotTransitioning;

			if (_sceneTransitionNewFrame && !_mainWindow.expired())
				_mainWindow.lock()->getSurface()->copyFrom(*_sceneTransitionNewFrame);

			_sceneTransitionOldFrame.reset();
			_sceneTransitionNewFrame.reset();

			_sourceSceneTransitionEffect = SceneTransitionEffect();
			_destinationSceneTransitionEffect = SceneTransitionEffect();

			for (const SceneStackEntry &sceneStackEntry : _sceneStack)
				recursiveAutoPlayMedia(sceneStackEntry.scene.get());

			for (const Common::SharedPtr<SceneTransitionHooks> &hooks : _hacks.sceneTransitionHooks)
				hooks->onSceneTransitionEnded(this, _activeMainScene);

			queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kSceneTransitionEnded, 0), _activeMainScene.get(), true, true);
			continue;
		}

		if (_sceneTransitionState == kSceneTransitionStateTransitioning) {
			// Keep looping transition and don't do anything else until it's done
			break;
		}

		{
			Common::SharedPtr<ScheduledEvent> firstScheduledEvent = _scheduler.getFirstEvent();
			if (firstScheduledEvent && firstScheduledEvent->getScheduledTime() <= _playTime) {
				_scheduler.descheduleFirstEvent();

				firstScheduledEvent->activate(this);
				continue;
			}
		}

		if (_collisionCheckTime < _playTime) {
			_collisionCheckTime = _playTime;

			checkBoundaries();
			checkCollisions(nullptr);
			continue;
		}

		// Reset cursor if the cursor element is gone
		if (_haveCursorElement && _elementTrackedToCursor.expired())
			updateMainWindowCursor();

		if (_isQuitting)
			return false;

		// Ran out of actions
		break;
	}

#ifdef MTROPOLIS_DEBUG_ENABLE
	if (_debugger)
		_debugger->runFrame(realMSec);
#else
	(void)realMSec;
#endif

	// Frame completed
	return true;
}

struct WindowSortingBucket {
	size_t originalIndex;
	Window *window;

	static bool sortPredicate(const WindowSortingBucket &a, const WindowSortingBucket &b) {
		int aStrata = a.window->getStrata();
		int bStrata = b.window->getStrata();

		if (aStrata != bStrata)
			return aStrata < bStrata;

		return a.originalIndex < b.originalIndex;
	}
};

void Runtime::drawFrame() {
	int width = _system->getWidth();
	int height = _system->getHeight();

	_system->fillScreen(Render::resolveRGB(0, 0, 0, getRenderPixelFormat()));

	bool needToRenderSubtitles = false;
	if (_subtitleRenderer && _subtitleRenderer->update(_playTime))
		setSceneGraphDirty();

	{
		Common::SharedPtr<Window> mainWindow = _mainWindow.lock();
		if (mainWindow) {
			if (_sceneTransitionState == kSceneTransitionStateTransitioning) {
				assert(_activeSceneTransitionEffect != nullptr);
				Render::renderSceneTransition(this, mainWindow.get(), *_activeSceneTransitionEffect, _sceneTransitionStartTime, _sceneTransitionEndTime, _playTime, *_sceneTransitionOldFrame, *_sceneTransitionNewFrame);
				needToRenderSubtitles = true;
			} else {
				bool skipped = false;
				Render::renderProject(this, mainWindow.get(), &skipped);

				needToRenderSubtitles = !skipped;
			}

			if (needToRenderSubtitles && _subtitleRenderer)
				_subtitleRenderer->composite(*mainWindow);
		}
	}

	const size_t numWindows = _windows.size();
	WindowSortingBucket singleBucket[1];
	Common::Array<WindowSortingBucket> multipleBuckets;
	WindowSortingBucket *sortedBuckets = singleBucket;

	if (numWindows < 2) {
		sortedBuckets = singleBucket;

		singleBucket[0].originalIndex = 0;
		singleBucket[0].window = _windows[0].get();
	} else {
		multipleBuckets.resize(numWindows);
		sortedBuckets = &multipleBuckets[0];

		for (size_t i = 0; i < numWindows; i++) {
			sortedBuckets[i].originalIndex = i;
			sortedBuckets[i].window = _windows[i].get();
		}

		Common::sort(sortedBuckets, sortedBuckets + numWindows, WindowSortingBucket::sortPredicate);
	}

	for (size_t i = 0; i < numWindows; i++) {
		const Window &window = *sortedBuckets[i].window;
		const Graphics::ManagedSurface &surface = *window.getSurface();

		int32 destLeft = window.getX();
		int32 destRight = destLeft + surface.w;
		int32 destTop = window.getY();
		int32 destBottom = destTop + surface.h;

		int32 srcLeft = 0;
		//int32 srcRight = surface.w;
		int32 srcTop = 0;
		//int32 srcBottom = surface.h;

		// Clip to drawable area
		if (destLeft < 0) {
			int leftAdjust = -destLeft;
			destLeft += leftAdjust;
			srcLeft += leftAdjust;
		}

		if (destTop < 0) {
			int topAdjust = -destTop;
			destTop += topAdjust;
			srcTop += topAdjust;
		}

		if (destRight > width) {
			int rightAdjust = width - destRight;
			destRight += rightAdjust;
			//srcRight += rightAdjust;
		}

		if (destBottom > height) {
			int bottomAdjust = height - destBottom;
			destBottom += bottomAdjust;
			//srcBottom += bottomAdjust;
		}

		if (destLeft >= destRight || destTop >= destBottom || destLeft >= width || destTop >= height || destRight <= 0 || destBottom <= 0)
			continue;

		_system->copyRectToScreen(surface.getBasePtr(srcLeft, srcTop), surface.pitch, destLeft, destTop, destRight - destLeft, destBottom - destTop);
	}

	_system->updateScreen();

	Common::SharedPtr<CursorGraphic> cursor;
	bool mouseVisible = true;

	Common::SharedPtr<Window> focusWindow = _mouseFocusWindow.lock();

	if (!focusWindow)
		focusWindow = findTopWindow(_realMousePosition.x, _realMousePosition.y);

	if (focusWindow) {
		cursor = focusWindow->getCursorGraphic();
		mouseVisible = focusWindow->getMouseVisible();
	}

	if (!cursor)
		cursor = _defaultCursor;

	if ((cursor != _lastFrameCursor || !_lastFrameMouseVisible) && mouseVisible) {
		CursorMan.showMouse(true);
		CursorMan.replaceCursor(cursor->getCursor());

		_lastFrameCursor = cursor;
		_lastFrameMouseVisible = true;
	} else if (_lastFrameMouseVisible && !mouseVisible) {
		CursorMan.showMouse(false);
		_lastFrameMouseVisible = false;
	}

	if (_project)
		_project->onPostRender();
}

Common::SharedPtr<Structural> Runtime::findDefaultSharedSceneForScene(Structural *scene) {
	Structural *subsection = scene->getParent();

	const Common::Array<Common::SharedPtr<Structural> > &children = subsection->getChildren();
	if (children.size() == 0 || children[0].get() == scene)
		return Common::SharedPtr<Structural>();

	return children[0];
}

void Runtime::executeTeardown(const Teardown &teardown) {
	if (Common::SharedPtr<Structural> structural = teardown.structural.lock()) {
		recursiveDeactivateStructural(structural.get());

		if (teardown.onlyRemoveChildren) {
			structural->removeAllChildren();
			structural->removeAllModifiers();
			structural->removeAllAssets();

			assert(structural->getSceneLoadState() == Structural::SceneLoadState::kSceneLoaded);
			structural->setSceneLoadState(Structural::SceneLoadState::kSceneNotLoaded);
		} else {
			Structural *parent = structural->getParent();

			// Nothing should be holding strong references to structural objects after they're removed from the project
			assert(parent != nullptr);

			if (!parent) {
				return; // Already unlinked but still alive somehow
			}

			parent->removeChild(structural.get());

			structural->setParent(nullptr);
		}
	}

	if (Common::SharedPtr<Modifier> modifier = teardown.modifier.lock()) {
		IModifierContainer *container = nullptr;
		RuntimeObject *parent = modifier->getParent().lock().get();

		if (parent) {
			if (parent->isStructural())
				container = static_cast<Structural *>(parent);
			else if (parent->isModifier())
				container = static_cast<Modifier *>(parent)->getChildContainer();
		}

		if (container)
			container->removeModifier(modifier.get());
	}
}

void Runtime::executeLowLevelSceneStateTransition(const LowLevelSceneStateTransitionAction &action) {
	switch (action.getActionType()) {
	case LowLevelSceneStateTransitionAction::kSendMessage:
		sendMessageOnVThread(action.getMessage());
		break;
	case LowLevelSceneStateTransitionAction::kLoad:
		loadScene(action.getScene());

		// Refresh play time so anything time-based doesn't skip ahead
		refreshPlayTime();
		break;
	case LowLevelSceneStateTransitionAction::kUnload: {
			Teardown teardown;
			teardown.onlyRemoveChildren = true;
			teardown.structural = action.getScene();

			_pendingTeardowns.push_back(teardown);
		} break;
	case LowLevelSceneStateTransitionAction::kAutoResetCursor:
		if (_autoResetCursor) {
			clearModifierCursorOverride();
			forceCursorRefreshOnce();
		}
		break;
	case LowLevelSceneStateTransitionAction::kShowDefaultVisibleElements:
		executeSceneChangeRecursiveVisibilityChange(action.getScene().get(), true);
		break;
	case LowLevelSceneStateTransitionAction::kHideAllElements:
		executeSceneChangeRecursiveVisibilityChange(action.getScene().get(), false);
		break;
	default:
		assert(false);
		break;
	}
}

void Runtime::executeSceneChangeRecursiveVisibilityChange(Structural *structural, bool showing) {
	const Common::Array<Common::SharedPtr<Structural> > &children = structural->getChildren();

	// Queue in reverse order since VThread sends are LIFO
	for (size_t i = 0; i < children.size(); i++)
		executeSceneChangeRecursiveVisibilityChange(children[children.size() - 1 - i].get(), showing);

	if (structural->isElement() && static_cast<Element *>(structural)->isVisual()) {
		VisualElement *visual = static_cast<VisualElement *>(structural);

		ApplyDefaultVisibilityTaskData *taskData = getVThread().pushTask("Runtime::applyDefaultVisibility", this, &Runtime::applyDefaultVisibility);
		taskData->element = visual;
		taskData->targetVisibility = showing;
	}
}

void Runtime::executeChangeObjectParent(RuntimeObject *object, RuntimeObject *newParent) {
	// TODO: Should do circularity checks

	if (object->isModifier()) {
		Common::SharedPtr<Modifier> modifier = object->getSelfReference().lock().staticCast<Modifier>();

		IModifierContainer *oldParentContainer = nullptr;

		RuntimeObject *oldParent = modifier->getParent().lock().get();

		if (oldParent == newParent)
			return;

		if (oldParent->isStructural())
			oldParentContainer = static_cast<Structural *>(oldParent);
		else if (oldParent->isModifier())
			oldParentContainer = static_cast<Modifier *>(oldParent)->getChildContainer();

		IModifierContainer *newParentContainer = nullptr;
		if (newParent->isStructural())
			newParentContainer = static_cast<Structural *>(newParent);
		else if (newParent->isModifier())
			newParentContainer = static_cast<Modifier *>(newParent)->getChildContainer();

		if (!newParentContainer) {
			warning("Object re-parent failed, the new parent isn't a modifier container");
			return;
		}

		oldParentContainer->removeModifier(modifier.get());
		newParentContainer->appendModifier(modifier);

		modifier->setParent(newParent->getSelfReference());

		{
			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kParentChanged, 0), DynamicValue(), modifier->getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, modifier.get(), false, false, false));
			sendMessageOnVThread(dispatch);
		}
	}

	if (object->isStructural()) {
		Common::SharedPtr<Structural> structural = object->getSelfReference().lock().staticCast<Structural>();

		Structural *oldParent = structural->getParent();

		if (oldParent == newParent)
			return;

		if (!newParent->isStructural()) {
			warning("Object re-parent failed, the new parent isn't structural");
			return;
		}

		Structural *newParentStructural = static_cast<Structural *>(newParent);

		oldParent->removeChild(structural.get());
		newParentStructural->addChild(structural);

		{
			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kParentChanged, 0), DynamicValue(), structural->getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, structural.get(), false, true, false));
			sendMessageOnVThread(dispatch);
		}
	}
}

void Runtime::executeCloneObject(RuntimeObject *object) {
	Common::HashMap<RuntimeObject *, RuntimeObject *> objectRemaps;

	if (object->isModifier()) {
		Common::SharedPtr<Modifier> modifierRef = object->getSelfReference().lock().staticCast<Modifier>();
		Common::WeakPtr<RuntimeObject> relinkParent = modifierRef->getParent();

		ObjectCloner cloner(this, relinkParent, &objectRemaps);
		cloner.visitChildModifierRef(modifierRef);

		ObjectRefRemapper remapper(objectRemaps);
		remapper.visitChildModifierRef(modifierRef);

		IModifierContainer *container = nullptr;
		Common::SharedPtr<RuntimeObject> parent = relinkParent.lock();
		if (parent) {
			if (parent->isStructural())
				container = static_cast<Structural *>(parent.get());
			else if (parent->isModifier())
				container = static_cast<Modifier *>(parent.get())->getChildContainer();
		}

		if (container)
			container->appendModifier(modifierRef);
		else
			error("Internal error: Cloned a modifier, but the parent isn't a modifier container");

		{
			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kClone, 0), DynamicValue(), modifierRef));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, modifierRef.get(), true, true, false));
			sendMessageOnVThread(dispatch);
		}

		{
			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kParentEnabled, 0), DynamicValue(), modifierRef));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, modifierRef.get(), true, true, false));
			sendMessageOnVThread(dispatch);
		}
	} else if (object->isStructural()) {
		Common::SharedPtr<Structural> structuralRef = object->getSelfReference().lock().staticCast<Structural>();
		Common::WeakPtr<RuntimeObject> relinkParent = structuralRef->getParent()->getSelfReference();

		ObjectCloner cloner(this, relinkParent, &objectRemaps);
		cloner.visitChildStructuralRef(structuralRef);

		ObjectRefRemapper remapper(objectRemaps);
		remapper.visitChildStructuralRef(structuralRef);

		structuralRef->getParent()->addChild(structuralRef);

		_pendingPostCloneShowChecks.push_back(structuralRef);

		{
			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kClone, 0), DynamicValue(), structuralRef));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, structuralRef.get(), true, true, false));
			sendMessageOnVThread(dispatch);
		}

		{
			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kParentEnabled, 0), DynamicValue(), structuralRef));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, structuralRef.get(), true, true, false));
			sendMessageOnVThread(dispatch);
		}
	} else
		error("Internal error: Cloned something unusual");
}

void Runtime::executeKillObject(RuntimeObject *object) {
	// TODO: Should do circularity checks

	if (object->isModifier()) {
		Modifier *modifier = static_cast<Modifier *>(object);

		{
			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kParentDisabled, 0), DynamicValue(), modifier->getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, modifier, true, true, false));
			sendMessageOnVThread(dispatch);
		}

		{
			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kKill, 0), DynamicValue(), modifier->getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, modifier, true, true, false));
			sendMessageOnVThread(dispatch);
		}

		Teardown teardown;
		teardown.modifier = modifier->getSelfReference().lock().staticCast<Modifier>();

		_pendingTeardowns.push_back(teardown);
	}

	if (object->isStructural()) {
		Structural *structural = static_cast<Structural *>(object);

		// Task order is LIFO, so order is Hide -> Kill -> Parent Disabled
		{
			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kParentDisabled, 0), DynamicValue(), structural->getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, structural, true, true, false));
			sendMessageOnVThread(dispatch);
		}

		{
			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kKill, 0), DynamicValue(), structural->getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, structural, true, true, false));
			sendMessageOnVThread(dispatch);
		}

		if (structural->isElement() && static_cast<Element *>(structural)->isVisual())
			static_cast<VisualElement *>(structural)->pushVisibilityChangeTask(this, false);

		Teardown teardown;
		teardown.structural = structural->getSelfReference().lock().staticCast<Structural>();

		_pendingTeardowns.push_back(teardown);
	}
}

void Runtime::executeCompleteTransitionToScene(const Common::SharedPtr<Structural> &targetScene) {
	// NOTE: Transitioning to the same scene is allowed, Obsidian relies on this to avoid getting stuck
	// after going up the wrong side in the Bureau chapter final area (i.e. after reaching the sky face).

	if (_sceneStack.size() == 0)
		_sceneStack.resize(1);	// Reserve shared scene slot

	Common::SharedPtr<Structural> targetSharedScene;

	if (_sharedSceneWasSetExplicitly)
		targetSharedScene = _activeSharedScene;
	else
		targetSharedScene = findDefaultSharedSceneForScene(targetScene.get());

	for (const Common::SharedPtr<SceneTransitionHooks> &hooks : _hacks.sceneTransitionHooks)
		hooks->onSceneTransitionSetup(this, _activeMainScene, targetScene);

	if (targetScene == targetSharedScene)
		error("Transitioned into a default shared scene, this is not supported");

	if (_activeMainScene == targetSharedScene)
		error("Transitioned into scene currently being used as a target scene, this is not supported");

	for (size_t i = _sceneStack.size() - 1; i > 0; i--) {
		Common::SharedPtr<Structural> stackedScene = _sceneStack[i].scene;

		queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kSceneEnded, 0), _activeMainScene.get(), true, true);
		_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(_activeMainScene, LowLevelSceneStateTransitionAction::kHideAllElements));
		queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kParentDisabled, 0), _activeMainScene.get(), true, true);
		_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(_activeMainScene, LowLevelSceneStateTransitionAction::kUnload));

		if (stackedScene == targetSharedScene)
			error("Transitioned to a shared scene which was already on the stack as a normal scene.  This is not supported.");

		_sceneStack.remove_at(i);
	}

	if (targetSharedScene != _activeSharedScene) {
		if (_activeSharedScene) {
			queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kSceneEnded, 0), _activeSharedScene.get(), true, true);
			_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(_activeMainScene, LowLevelSceneStateTransitionAction::kHideAllElements));
			queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kParentDisabled, 0), _activeSharedScene.get(), true, true);
			_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(_activeSharedScene, LowLevelSceneStateTransitionAction::kUnload));
		}

		_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(targetSharedScene, LowLevelSceneStateTransitionAction::kLoad));
		queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kParentEnabled, 0), targetSharedScene.get(), true, true);
		queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kSceneStarted, 0), targetSharedScene.get(), true, true);

		_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(targetSharedScene, LowLevelSceneStateTransitionAction::kShowDefaultVisibleElements));

		SceneStackEntry sharedSceneEntry;
		sharedSceneEntry.scene = targetSharedScene;

		_sceneStack[0] = sharedSceneEntry;
	}

	{
		_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(targetScene, LowLevelSceneStateTransitionAction::kLoad));
		queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kParentEnabled, 0), targetScene.get(), true, true);
		queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kSceneStarted, 0), targetScene.get(), true, true);

		SceneStackEntry sceneEntry;
		sceneEntry.scene = targetScene;

		_sceneStack.push_back(sceneEntry);
	}

	// This might not be exactly where this belongs but it must go after Parent Enabled for sure, otherwise
	// the wave puzzle in the Mac version of Obsidian completes instantly because Parent Enabled hasn't had
	// a chance to clear the flags.  It looks like what's supposed to happen is the cursor override gets
	// cleared by the scene transition starting and the cursor reset happens after.  Fix this later...
	_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(targetScene, LowLevelSceneStateTransitionAction::kAutoResetCursor));

	_activeMainScene = targetScene;
	_activeSharedScene = targetSharedScene;

	// Scene transitions have to be set up by the destination scene
	_sceneTransitionState = kSceneTransitionStateWaitingForDraw;
	_activeSceneTransitionEffect = nullptr;

	executeSharedScenePostSceneChangeActions();
}

void Runtime::executeHighLevelSceneReturn() {
	if (_sceneStack.size() == 0)
		_sceneStack.resize(1); // Reserve shared scene slot

	if (_sceneReturnList.size() == 0) {
		warning("A scene return was requested, but no scenes are in the scene return list");
		return;
	}

	const SceneReturnListEntry &sceneReturn = _sceneReturnList.back();

	if (sceneReturn.scene == _activeSharedScene)
		error("Transitioned into the active shared scene as the main scene, this is not supported");

	if (sceneReturn.scene != _activeMainScene) {
		assert(_activeMainScene.get() != nullptr); // There should always be an active main scene after the first transition

		if (sceneReturn.isAddToDestinationSceneTransition) {
			// In this case we unload the active main scene and reactivate the old main
			queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kSceneEnded, 0), _activeMainScene.get(), true, true);
			queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kParentDisabled, 0), _activeMainScene.get(), true, true);
			_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(_activeMainScene, LowLevelSceneStateTransitionAction::kUnload));

			queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kSceneReactivated, 0), sceneReturn.scene.get(), true, true);

			for (uint i = 1; i < _sceneStack.size(); i++) {
				if (_sceneStack[i].scene == _activeMainScene) {
					_sceneStack.remove_at(i);
					break;
				}
			}

			_activeMainScene = sceneReturn.scene;

			executeSharedScenePostSceneChangeActions();
		} else {
			executeCompleteTransitionToScene(sceneReturn.scene);
		}
	}
}

void Runtime::executeHighLevelSceneTransition(const HighLevelSceneTransition &transition) {
	if (_sceneStack.size() == 0)
		_sceneStack.resize(1); // Reserve shared scene slot

	// This replicates a bunch of quirks/bugs of mTropolis's scene transition logic,
	// see the accompanying notes file.  There are probably some missing cases related to
	// shared scene, calling return/scene transitions during scene deactivation, or other
	// edge cases that hopefully no games actually do!
	switch (transition.type) {
	case HighLevelSceneTransition::kTypeChangeToScene: {
			const Common::SharedPtr<Structural> targetScene = transition.scene;

			// This check may not be accurate, but we need to avoid adding the existing scene to the return list.
			// SPQR depends on this behavior: Hitting Esc while in the menu will fire off another transition to
			// the menu scene with addToReturnList set.  We want to avoid adding the menu to the return list
			// multiple times since it will prevent the exit button from working properly.
			if ((transition.addToDestinationScene || transition.addToReturnList) && targetScene != _activeMainScene) {
				SceneReturnListEntry returnListEntry;
				returnListEntry.isAddToDestinationSceneTransition = transition.addToDestinationScene;
				returnListEntry.scene = _activeMainScene;
				_sceneReturnList.push_back(returnListEntry);
			}

			if (transition.addToDestinationScene) {
				if (targetScene != _activeMainScene) {
					Common::SharedPtr<Structural> targetSharedScene;

					if (_sharedSceneWasSetExplicitly)
						targetSharedScene = _activeSharedScene;
					else
						targetSharedScene = findDefaultSharedSceneForScene(targetScene.get());

					if (targetScene == targetSharedScene)
						error("Transitioned into a default shared scene, this is not supported");

					if (_activeMainScene == targetSharedScene)
						error("Transitioned into scene currently being used as a target scene, this is not supported");

					queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kSceneDeactivated, 0), _activeMainScene.get(), true, true);

					if (targetSharedScene != _activeSharedScene) {
						if (_activeSharedScene) {
							queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kSceneEnded, 0), _activeSharedScene.get(), true, true);
							queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kParentDisabled, 0), _activeSharedScene.get(), true, true);
							_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(_activeSharedScene, LowLevelSceneStateTransitionAction::kUnload));
						}

						_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(targetSharedScene, LowLevelSceneStateTransitionAction::kLoad));
						queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kParentEnabled, 0), targetSharedScene.get(), true, true);
						queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kSceneStarted, 0), targetSharedScene.get(), true, true);
						_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(targetSharedScene, LowLevelSceneStateTransitionAction::kShowDefaultVisibleElements));

						SceneStackEntry sharedSceneEntry;
						sharedSceneEntry.scene = targetScene;

						_sceneStack[0] = sharedSceneEntry;
					}


					bool sceneAlreadyInStack = false;
					for (size_t i = _sceneStack.size() - 1; i > 0; i--) {
						Common::SharedPtr<Structural> stackedScene = _sceneStack[i].scene;
						if (stackedScene == targetScene) {
							sceneAlreadyInStack = true;
							break;
						}
					}

					// This is probably wrong if it's already in the stack, but transitioning to already-in-stack scenes is extremely buggy in mTropolis Player anyway
					if (!sceneAlreadyInStack) {
						_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(targetScene, LowLevelSceneStateTransitionAction::kLoad));
						queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kParentEnabled, 0), targetScene.get(), true, true);
						queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kSceneStarted, 0), targetScene.get(), true, true);

						SceneStackEntry sceneEntry;
						sceneEntry.scene = targetScene;

						_sceneStack.push_back(sceneEntry);
					}

					_activeMainScene = targetScene;
					_activeSharedScene = targetSharedScene;

					executeSharedScenePostSceneChangeActions();
				}
			} else {
				executeCompleteTransitionToScene(targetScene);
			}
		} break;
	case HighLevelSceneTransition::kTypeChangeSharedScene: {
			Common::SharedPtr<Structural> targetSharedScene = transition.scene;

			if (targetSharedScene != _activeSharedScene) {
				if (_activeSharedScene) {
					queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kSceneEnded, 0), _activeSharedScene.get(), true, true);
					queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kParentDisabled, 0), _activeSharedScene.get(), true, true);
					_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(_activeSharedScene, LowLevelSceneStateTransitionAction::kUnload));
				}

				_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(targetSharedScene, LowLevelSceneStateTransitionAction::kLoad));
				queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kParentEnabled, 0), targetSharedScene.get(), true, true);
				queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kSceneStarted, 0), targetSharedScene.get(), true, true);
				_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(targetSharedScene, LowLevelSceneStateTransitionAction::kShowDefaultVisibleElements));

				SceneStackEntry sharedSceneEntry;
				sharedSceneEntry.scene = targetSharedScene;

				_sceneStack[0] = sharedSceneEntry;

				_activeSharedScene = targetSharedScene;
			}

			_sharedSceneWasSetExplicitly = true;
		} break;
	case HighLevelSceneTransition::kTypeForceLoadScene: {
			_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(transition.scene, LowLevelSceneStateTransitionAction::kLoad));
			queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kParentEnabled, 0), transition.scene.get(), true, true);
			queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kSceneStarted, 0), transition.scene.get(), true, true);
		} break;
	case HighLevelSceneTransition::kTypeRequestUnloadScene: {
			bool isActiveScene = false;
			for (const SceneStackEntry &stackEntry : _sceneStack) {
				if (stackEntry.scene == transition.scene) {
					isActiveScene = true;
					break;
				}
			}

			if (!isActiveScene)
				_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(transition.scene, LowLevelSceneStateTransitionAction::kUnload));
		} break;
	default:
		error("Unknown high-level scene transition type");
		break;
	}
}

void Runtime::executeSharedScenePostSceneChangeActions() {
	Structural *subsection = _activeMainScene->getParent();

	const Common::Array<Common::SharedPtr<Structural> > &subsectionScenes = subsection->getChildren();

	queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kSharedSceneSceneChanged, 0), _activeSharedScene.get(), true, true);
	if (subsectionScenes.size() > 1) {
		if (_activeMainScene == subsectionScenes[subsectionScenes.size() - 1])
			queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kSharedSceneNoNextScene, 0), _activeSharedScene.get(), true, true);
		if (_activeMainScene == subsectionScenes[1])
			queueEventAsLowLevelSceneStateTransitionAction(Event(EventIDs::kSharedSceneNoPrevScene, 0), _activeSharedScene.get(), true, true);
	}

	_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(_activeMainScene, LowLevelSceneStateTransitionAction::kShowDefaultVisibleElements));
}

void Runtime::recursiveDeactivateStructural(Structural *structural) {
	for (const Common::SharedPtr<Structural> &child : structural->getChildren()) {
		recursiveDeactivateStructural(child.get());
	}

	structural->deactivate();
}

void Runtime::recursiveAutoPlayMedia(Structural *structural) {
	if (structural->isElement())
		static_cast<Element *>(structural)->triggerAutoPlay(this);

	for (const Common::SharedPtr<Structural> &child : structural->getChildren()) {
		recursiveAutoPlayMedia(child.get());
	}
}

void Runtime::recursiveActivateStructural(Structural *structural) {
	structural->activate();

	for (const Common::SharedPtr<Structural> &child : structural->getChildren()) {
		recursiveActivateStructural(child.get());
	}
}

bool Runtime::isStructuralMouseInteractive(Structural *structural, MouseInteractivityTestType testType) {
	if (structural->isElement()) {
		Element *element = static_cast<Element *>(structural);
		if (element->isVisual()) {
			VisualElement *visual = static_cast<VisualElement *>(element);
			if (visual->getDragMotionProperties())
				return true;	// Drag motion is always mouse interactive
		}
	}

	for (const Common::SharedPtr<Modifier> &modifier : structural->getModifiers()) {
		if (isModifierMouseInteractive(modifier.get(), testType))
			return true;
	}

	return false;
}

bool Runtime::isModifierMouseInteractive(Modifier *modifier, MouseInteractivityTestType testType) {
	static const EventIDs::EventID allEventIDs[] = {
		EventIDs::kMouseUp,
		EventIDs::kMouseDown,
		EventIDs::kMouseOver,
		EventIDs::kMouseOutside,
		EventIDs::kMouseTrackedInside,
		EventIDs::kMouseTracking,
		EventIDs::kMouseTrackedOutside,
		EventIDs::kMouseUpInside,
		EventIDs::kMouseUpOutside
	};


	static const EventIDs::EventID mouseClickEventIDs[] = {
		EventIDs::kMouseUp,
		EventIDs::kMouseDown,
		EventIDs::kMouseTrackedInside,
		EventIDs::kMouseTracking,
		EventIDs::kMouseTrackedOutside,
		EventIDs::kMouseUpInside,
		EventIDs::kMouseUpOutside
	};

	const EventIDs::EventID *evtIDs = nullptr;
	size_t numEventIDs = 0;

	if (testType == kMouseInteractivityTestAnything) {
		evtIDs = allEventIDs;
		numEventIDs = ARRAYSIZE(allEventIDs);
	} else if (testType == kMouseInteractivityTestMouseClick) {
		evtIDs = mouseClickEventIDs;
		numEventIDs = ARRAYSIZE(mouseClickEventIDs);
	}

	for (size_t i = 0; i < numEventIDs; i++) {
		EventIDs::EventID evtID = evtIDs[i];
		if (modifier->respondsToEvent(Event(evtID, 0)))
			return true;
	}

	IModifierContainer *propagationContainer = modifier->getMessagePropagationContainer();
	if (propagationContainer) {
		for (const Common::SharedPtr<Modifier> &child : propagationContainer->getModifiers()) {
			if (isModifierMouseInteractive(child.get(), testType))
				return true;
		}
	}

	return false;
}

void Runtime::recursiveFindMouseCollision(Structural *&bestResult, int32 &bestLayer, int32 &bestStackHeight, bool &bestDirect, Structural *candidate, int32 stackHeight, int32 relativeX, int32 relativeY, MouseInteractivityTestType testType) {
	int32 childRelativeX = relativeX;
	int32 childRelativeY = relativeY;
	if (candidate->isElement()) {
		Element *element = static_cast<Element *>(candidate);
		if (element->isVisual()) {
			VisualElement *visual = static_cast<VisualElement *>(candidate);

			if (visual->isVisible()) {
				int layer = visual->getLayer();
				bool isDirect = visual->isDirectToScreen();

				// Layering priority:
				bool isInFront = false;
				if (isDirect && !bestDirect)
					isInFront = true;
				else if (isDirect == bestDirect) {
					if (layer > bestLayer)
						isInFront = true;
					else if (layer == bestLayer) {
						if (stackHeight > bestStackHeight)
							isInFront = true;
					}
				}

				if (isInFront && visual->isMouseInsideDrawableArea(relativeX, relativeY) && isStructuralMouseInteractive(visual, testType) && visual->isMouseCollisionAtPoint(relativeX, relativeY)) {
					bestResult = candidate;
					bestLayer = layer;
					bestStackHeight = stackHeight;
					bestDirect = isDirect;
				}
			}

			// Need to check: Does hiding an element also hide its children?
			childRelativeX -= visual->getRelativeRect().left;
			childRelativeY -= visual->getRelativeRect().top;
		}
	}

	for (const Common::SharedPtr<Structural> &child : candidate->getChildren())
		recursiveFindMouseCollision(bestResult, bestLayer, bestStackHeight, bestDirect, child.get(), stackHeight, childRelativeX, childRelativeY, testType);
}

void Runtime::queueEventAsLowLevelSceneStateTransitionAction(const Event &evt, Structural *root, bool cascade, bool relay) {
	Common::SharedPtr<MessageProperties> props(new MessageProperties(evt, DynamicValue(), Common::WeakPtr<RuntimeObject>()));
	Common::SharedPtr<MessageDispatch> msg(new MessageDispatch(props, root, cascade, relay, false));
	_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(msg));
}

void Runtime::loadScene(const Common::SharedPtr<Structural> &scene) {
	assert(scene->getSceneLoadState() != Structural::SceneLoadState::kNotAScene);

	if (scene->getSceneLoadState() == Structural::SceneLoadState::kSceneNotLoaded) {
		scene->setSceneLoadState(Structural::SceneLoadState::kSceneLoaded);

		debug(1, "Loading scene '%s'", scene->getName().c_str());
		Element *element = static_cast<Element *>(scene.get());
		uint32 streamID = element->getStreamLocator() & 0xffff; // Not actually sure how many bits are legal here

		Subsection *subsection = static_cast<Subsection *>(scene->getParent());

		if (streamID == 0) {
			debug(1, "Scene is empty");
		} else {
			_project->loadSceneFromStream(scene, streamID, getHacks());
			debug(1, "Scene loaded OK, materializing objects...");
			scene->materializeDescendents(this, subsection->getSceneLoadMaterializeScope());
			debug(1, "Scene materialized OK");
		}

		recursiveActivateStructural(scene.get());
		debug(1, "Structural elements activated OK");

#ifdef MTROPOLIS_DEBUG_ENABLE
		if (_debugger) {
			_debugger->complainAboutUnfinished(scene.get());
			_debugger->refreshSceneStatus();
		}
#endif
	}
}

CORO_BEGIN_DEFINITION(Runtime::SendMessageOnVThreadCoroutine)
	struct Locals {
	};

	CORO_BEGIN_FUNCTION
		CORO_AWAIT(params->runtime->sendMessageOnVThread(params->dispatch));
	CORO_END_FUNCTION
CORO_END_DEFINITION

void Runtime::sendMessageOnVThread(const Common::SharedPtr<MessageDispatch> &dispatch) {
	EventIDs::EventID eventID = dispatch->getMsg()->getEvent().eventType;

	// 0 is normally not produced and is invalid, 1 is "None" and is an invalid message
	if (eventID == 1 || eventID == 0)
		return;

#ifndef DISABLE_TEXT_CONSOLE
	const int msgDebugLevel = 3;

	if (gDebugLevel >= msgDebugLevel) {
		const char *nameStr = "";
		int srcID = 0;
		const char *destStr = "";
		int destID = 0;
		Common::SharedPtr<RuntimeObject> src = dispatch->getMsg()->getSource().lock();

		if (src) {
			srcID = src->getStaticGUID();
			if (src->isStructural())
				nameStr = static_cast<Structural *>(src.get())->getName().c_str();
			else if (src->isModifier())
				nameStr = static_cast<Modifier *>(src.get())->getName().c_str();
		}

		RuntimeObject *dest = dispatch->getRootPropagator();
		if (dest) {
			destID = dest->getStaticGUID();
			if (dest->isStructural())
				destStr = static_cast<Structural *>(dest)->getName().c_str();
			else if (dest->isModifier())
				destStr = static_cast<Modifier *>(dest)->getName().c_str();
		}

		const Event evt = dispatch->getMsg()->getEvent();
		bool cascade = dispatch->isCascade();
		bool relay = dispatch->isRelay();

		Common::String msgDebugString;
		msgDebugString = Common::String::format("(%i,%i)", evt.eventType, evt.eventInfo);
		if (evt.eventType == EventIDs::kAuthorMessage && _project) {
			msgDebugString += " '";
			msgDebugString += _project->findAuthorMessageName(evt.eventInfo);
			msgDebugString += "'";
		} else {
			const char *extType = nullptr;
			switch (evt.eventType) {
			case EventIDs::kElementEnableEdit:
				extType = "Element Enable Edit";
				break;
			case EventIDs::kElementDisableEdit:
				extType = "Element Disable Edit";
				break;
			case EventIDs::kElementSelect:
				extType = "Element Select";
				break;
			case EventIDs::kElementDeselect:
				extType = "Element Deselect";
				break;
			case EventIDs::kElementToggleSelect:
				extType = "Element Toggle Select";
				break;
			case EventIDs::kElementUpdatedCalculated:
				extType = "Element Updated Calculated";
				break;
			case EventIDs::kElementShow:
				extType = "Element Show";
				break;
			case EventIDs::kElementHide:
				extType = "Element Hide";
				break;
			case EventIDs::kElementScrollUp:
				extType = "Element Scroll Up";
				break;
			case EventIDs::kElementScrollDown:
				extType = "Element Scroll Down";
				break;
			case EventIDs::kElementScrollRight:
				extType = "Element Scroll Right";
				break;
			case EventIDs::kElementScrollLeft:
				extType = "Element Scroll Left";
				break;

			case EventIDs::kMotionStarted:
				extType = "Motion Started";
				break;
			case EventIDs::kMotionEnded:
				extType = "Motion Started";
				break;

			case EventIDs::kTransitionStarted:
				extType = "Transition Started";
				break;
			case EventIDs::kTransitionEnded:
				extType = "Transition Ended";
				break;

			case EventIDs::kMouseDown:
				extType = "Mouse Down";
				break;
			case EventIDs::kMouseUp:
				extType = "Mouse Up";
				break;
			case EventIDs::kMouseOver:
				extType = "Mouse Over";
				break;
			case EventIDs::kMouseOutside:
				extType = "Mouse Outside";
				break;
			case EventIDs::kMouseTrackedInside:
				extType = "Mouse Tracked Inside";
				break;
			case EventIDs::kMouseTrackedOutside:
				extType = "Mouse Tracked Outside";
				break;
			case EventIDs::kMouseTracking:
				extType = "Mouse Tracking";
				break;
			case EventIDs::kMouseUpInside:
				extType = "Mouse Up Inside";
				break;
			case EventIDs::kMouseUpOutside:
				extType = "Mouse Up Outside";
				break;

			case EventIDs::kSceneStarted:
				extType = "Scene Started";
				break;
			case EventIDs::kSceneEnded:
				extType = "Scene Ended";
				break;
			case EventIDs::kSceneDeactivated:
				extType = "Scene Deactivate";
				break;
			case EventIDs::kSceneReactivated:
				extType = "Scene Reactivated";
				break;
			case EventIDs::kSceneTransitionEnded:
				extType = "Scene Transition Ended";
				break;

			case EventIDs::kSharedSceneReturnedToScene:
				extType = "Scene Returned To Scene";
				break;
			case EventIDs::kSharedSceneSceneChanged:
				extType = "Scene Scene Changed";
				break;
			case EventIDs::kSharedSceneNoNextScene:
				extType = "Shared Scene No Next Scene";
				break;
			case EventIDs::kSharedSceneNoPrevScene:
				extType = "Shared Scene No Prev Scene";
				break;

			case EventIDs::kParentEnabled:
				extType = "Parent Enabled";
				break;
			case EventIDs::kParentDisabled:
				extType = "Parent Disabled";
				break;
			case EventIDs::kParentChanged:
				extType = "Parent Changed";
				break;

			case EventIDs::kPreloadMedia:
				extType = "Preload Media";
				break;
			case EventIDs::kFlushMedia:
				extType = "Flush Media";
				break;
			case EventIDs::kPrerollMedia:
				extType = "Preroll Media";
				break;

			case EventIDs::kCloseProject:
				extType = "Close Project";
				break;

			case EventIDs::kUserTimeout:
				extType = "User Timeout";
				break;
			case EventIDs::kProjectStarted:
				extType = "Project Started";
				break;
			case EventIDs::kProjectEnded:
				extType = "Project Ended";
				break;
			case EventIDs::kFlushAllMedia:
				extType = "Flush All Media";
				break;

			case EventIDs::kAttribGet:
				extType = "Attrib Get";
				break;
			case EventIDs::kAttribSet:
				extType = "Attrib Set";
				break;

			case EventIDs::kClone:
				extType = "Clone";
				break;
			case EventIDs::kKill:
				extType = "Kill";
				break;

			case EventIDs::kPlay:
				extType = "Play";
				break;
			case EventIDs::kStop:
				extType = "Stop";
				break;
			case EventIDs::kPause:
				extType = "Pause";
				break;
			case EventIDs::kUnpause:
				extType = "Unpause";
				break;
			case EventIDs::kTogglePause:
				extType = "Toggle Pause";
				break;
			case EventIDs::kAtFirstCel:
				extType = "At First Cel";
				break;
			case EventIDs::kAtLastCel:
				extType = "At Last Cell";
				break;
			default:
				break;
			}

			if (extType) {
				msgDebugString += " '";
				msgDebugString += extType;
				msgDebugString += "'";
			}
		}

		Common::String valueStr;
		const DynamicValue &payload = dispatch->getMsg()->getValue();

		if (payload.getType() != DynamicValueTypes::kNull) {
			switch (payload.getType()) {
			case DynamicValueTypes::kBoolean:
				valueStr = (payload.getBool() ? "true" : "false");
				break;
			case DynamicValueTypes::kInteger:
				valueStr = Common::String::format("%i", payload.getInt());
				break;
			case DynamicValueTypes::kFloat:
				valueStr = Common::String::format("%g", payload.getFloat());
				break;
			case DynamicValueTypes::kPoint:
				valueStr = Common::String::format("(%i,%i)", payload.getPoint().x, payload.getPoint().y);
				break;
			case DynamicValueTypes::kIntegerRange:
				valueStr = Common::String::format("(%i thru %i)", payload.getIntRange().min, payload.getIntRange().max);
				break;
			case DynamicValueTypes::kVector:
				valueStr = Common::String::format("(%g deg %g mag)", payload.getVector().angleDegrees, payload.getVector().magnitude);
				break;
			case DynamicValueTypes::kString:
				valueStr = "'" + payload.getString() + "'";
				break;
			case DynamicValueTypes::kList:
				valueStr = "List";
				break;
			case DynamicValueTypes::kObject:
				valueStr = "Object";
				if (RuntimeObject *obj = payload.getObject().object.lock().get())
					valueStr += Common::String::format(" %x", obj->getStaticGUID());
				break;
			case DynamicValueTypes::kLabel:
				valueStr = Common::String::format("Label(%u,%u)", static_cast<uint>(payload.getLabel().superGroupID), static_cast<uint>(payload.getLabel().id));
				if (const Common::String *labelName = _project->findNameOfLabel(payload.getLabel()))
					valueStr = valueStr + "[\"" + (*labelName) + "\"]";
				break;
			default:
				valueStr = "<BAD TYPE> (this is a bug!)";
				break;
			}

			valueStr = " with value " + valueStr;
		}

		debug(msgDebugLevel, "Object %x '%s' posted message %s to %x '%s'%s  mod: %s   ele: %s", srcID, nameStr, msgDebugString.c_str(), destID, destStr, valueStr.c_str(), relay ? "all" : "first", cascade ? "all" : "targetOnly");
	}
#endif

	_vthread->pushCoroutine<DispatchMessageCoroutine>(this, dispatch);
}

CORO_BEGIN_DEFINITION(Runtime::DispatchMessageCoroutine)
	struct Locals {
		bool isTerminated = false;
		RuntimeObject *rootObject = nullptr;
		MessageDispatch::RootType rootType = MessageDispatch::RootType::Invalid;
	};

	CORO_BEGIN_FUNCTION
		locals->rootObject = params->dispatch->getRootWeakPtr().lock().get();
		CORO_IF(locals->rootObject != nullptr)
			locals->rootType = params->dispatch->getRootType();
			CORO_IF (locals->rootType == MessageDispatch::RootType::Command)
				CORO_AWAIT(static_cast<Structural *>(locals->rootObject)->asyncConsumeCommand(params->runtime, params->dispatch->getMsg()));
			CORO_ELSE_IF (locals->rootType == MessageDispatch::RootType::Structural)
				CORO_CALL(SendMessageToStructuralCoroutine, params->runtime, &locals->isTerminated, static_cast<Structural *>(locals->rootObject), params->dispatch.get());
			CORO_ELSE
				if (params->dispatch->getRootType() != MessageDispatch::RootType::Modifier)
					error("Internal error: Message propagation target was something other than structural or modifier");

				CORO_CALL(SendMessageToModifierCoroutine, params->runtime, &locals->isTerminated, static_cast<Modifier *>(locals->rootObject), params->dispatch.get());
			CORO_END_IF
		CORO_END_IF
	CORO_END_FUNCTION
CORO_END_DEFINITION

CORO_BEGIN_DEFINITION(Runtime::SendMessageToStructuralCoroutine)
	struct Locals {
		const Common::Array<Common::SharedPtr<Structural> > *childrenArray = nullptr;
		uint childIndex = 0;
	};

	CORO_BEGIN_FUNCTION
		// Send to the structural object.  Structural objects only consume commands.
		CORO_IF (params->structural->respondsToEvent(params->dispatch->getMsg()->getEvent()))
			CORO_AWAIT(params->runtime->postConsumeCommandTask(params->structural, params->dispatch->getMsg()));

			CORO_IF (!params->dispatch->isRelay())
				*params->isTerminatedPtr = true;
				CORO_RETURN;
			CORO_END_IF
		CORO_END_IF

		// Send to modifiers
		if (params->structural->getSceneLoadState() == Structural::SceneLoadState::kSceneNotLoaded)
			params->runtime->hotLoadScene(params->structural);

		CORO_IF (params->structural->getModifiers().size() > 0)
			CORO_CALL(Runtime::SendMessageToModifierContainerCoroutine, params->runtime, params->isTerminatedPtr, params->structural, params->dispatch);

			CORO_IF(*params->isTerminatedPtr)
				CORO_RETURN;
			CORO_END_IF
		CORO_END_IF

		// Send to children if cascade
		CORO_IF(params->dispatch->isCascade())
			CORO_FOR((locals->childrenArray = &params->structural->getChildren()), (locals->childIndex < locals->childrenArray->size()), (locals->childIndex++))
				CORO_CALL(Runtime::SendMessageToStructuralCoroutine, params->runtime, params->isTerminatedPtr, (*locals->childrenArray)[locals->childIndex].get(), params->dispatch);

				CORO_IF (*params->isTerminatedPtr)
					CORO_RETURN;
				CORO_END_IF
			CORO_END_FOR
		CORO_END_IF
	CORO_END_FUNCTION
CORO_END_DEFINITION

CORO_BEGIN_DEFINITION(Runtime::SendMessageToModifierContainerCoroutine)
	struct Locals {
		const Common::Array<Common::SharedPtr<Modifier> > *childrenArray = nullptr;
		uint childIndex = 0;
	};

	CORO_BEGIN_FUNCTION
		locals->childrenArray = &params->modifierContainer->getModifiers();

		CORO_FOR((locals->childIndex = 0), (locals->childIndex < locals->childrenArray->size() && !*(params->isTerminatedPtr)), (locals->childIndex++))
			CORO_CALL(SendMessageToModifierCoroutine, params->runtime, params->isTerminatedPtr, (*locals->childrenArray)[locals->childIndex].get(), params->dispatch);
		CORO_END_FOR
	CORO_END_FUNCTION
CORO_END_DEFINITION

CORO_BEGIN_DEFINITION(Runtime::SendMessageToModifierCoroutine)
	struct Locals {
		bool responds = false;
		IModifierContainer *childContainer = nullptr;
	};

	CORO_BEGIN_FUNCTION
		// Handle the action in the VThread
		locals->responds = params->modifier->respondsToEvent(params->dispatch->getMsg()->getEvent());

		// Queue propagation to children, if any, when the VThread task is done
		if (locals->responds && !params->dispatch->isRelay()) {
			*params->isTerminatedPtr = true;
		} else {
			locals->childContainer = params->modifier->getMessagePropagationContainer();
		}

		// Post to the message action itself to VThread
		CORO_IF (locals->responds)
			debug(3, "Modifier %x '%s' consumed message (%i,%i)", params->modifier->getStaticGUID(), params->modifier->getName().c_str(), params->dispatch->getMsg()->getEvent().eventType, params->dispatch->getMsg()->getEvent().eventInfo);
			CORO_AWAIT(params->runtime->postConsumeMessageTask(params->modifier, params->dispatch->getMsg()));
		CORO_END_IF

		CORO_IF(locals->childContainer && !(*params->isTerminatedPtr))
			CORO_CALL(SendMessageToModifierContainerCoroutine, params->runtime, params->isTerminatedPtr, locals->childContainer, params->dispatch);
		CORO_END_IF
	CORO_END_FUNCTION
CORO_END_DEFINITION

VThreadState Runtime::dispatchKeyTask(const DispatchKeyTaskData &data) {
	Common::SharedPtr<KeyEventDispatch> dispatchPtr = data.dispatch;
	KeyEventDispatch &dispatch = *dispatchPtr.get();

	if (dispatch.isTerminated())
		return kVThreadReturn;
	else {
		// Requeue propagation after whatever happens with this propagation step
		DispatchKeyTaskData *requeueData = _vthread->pushTask("Runtime::dispatchKeyTask", this, &Runtime::dispatchKeyTask);
		requeueData->dispatch = dispatchPtr;

		return dispatch.continuePropagating(this);
	}
}

VThreadState Runtime::dispatchActionTask(const DispatchActionTaskData &data) {
	switch (data.action)
	{
	case Actions::kDebugSkipMovies:
#ifdef MTROPOLIS_DEBUG_ENABLE
		_project->debugSkipMovies();
#endif
		break;
	default:
		warning("Unhandled action %i", static_cast<int>(data.action));
		break;
	}

	return kVThreadReturn;
}

VThreadState Runtime::consumeMessageTask(const ConsumeMessageTaskData &data) {
	IMessageConsumer *consumer = data.consumer;
	assert(consumer->respondsToEvent(data.message->getEvent()));
	return consumer->consumeMessage(this, data.message);
}

VThreadState Runtime::consumeCommandTask(const ConsumeCommandTaskData &data) {
	Structural *structural = data.structural;
	return structural->asyncConsumeCommand(this, data.message);
}

VThreadState Runtime::updateMouseStateTask(const UpdateMouseStateTaskData &data) {
	struct MessageToSend {
		EventIDs::EventID eventID;
		Structural *target;
	};

	Common::Array<MessageToSend> messagesToSend;

	if (data.mouseDown) {
		// Mouse down
		Structural *tracked = nullptr;
		int32 bestSceneStack = INT32_MIN;
		int32 bestLayer = INT32_MIN;
		bool bestDirect = false;

		for (size_t ri = 0; ri < _sceneStack.size(); ri++) {
			const SceneStackEntry &sceneStackEntry = _sceneStack[_sceneStack.size() - 1 - ri];
			recursiveFindMouseCollision(tracked, bestSceneStack, bestLayer, bestDirect, sceneStackEntry.scene.get(), _sceneStack.size() - 1 - ri, _cachedMousePosition.x, _cachedMousePosition.y, kMouseInteractivityTestMouseClick);
		}

		if (tracked) {
			_mouseTrackingObject = tracked->getSelfReference().staticCast<Structural>();
			_mouseTrackingDragStart = _cachedMousePosition;
			if (tracked->isElement() && static_cast<Element *>(tracked)->isVisual()) {
				Common::Rect initialRect = static_cast<VisualElement *>(tracked)->getRelativeRect();
				_mouseTrackingObjectInitialOrigin = Common::Point(initialRect.left, initialRect.top);
			} else
				_mouseTrackingObjectInitialOrigin = Common::Point(0, 0);
			_trackedMouseOutside = false;

			MessageToSend msg;
			msg.eventID = EventIDs::kMouseDown;
			msg.target = tracked;
			messagesToSend.push_back(msg);
		}
	} else {
		// Mouse up
		Common::SharedPtr<Structural> tracked = _mouseTrackingObject.lock();
		if (tracked) {
			{
				MessageToSend msg;
				msg.eventID = EventIDs::kMouseUp;
				msg.target = tracked.get();
				messagesToSend.push_back(msg);
			}

			{
				MessageToSend msg;
				msg.eventID = _trackedMouseOutside ? EventIDs::kMouseUpOutside : EventIDs::kMouseUpInside;
				msg.target = tracked.get();
				messagesToSend.push_back(msg);
			}

			_mouseTrackingObject.reset();
			_trackedMouseOutside = false;
		}
	}

	DynamicValue mousePtValue;
	mousePtValue.setPoint(Common::Point(_cachedMousePosition.x, _cachedMousePosition.y));

	for (size_t ri = 0; ri < messagesToSend.size(); ri++) {
		const MessageToSend &msg = messagesToSend[messagesToSend.size() - 1 - ri];
		Common::SharedPtr<MessageProperties> props(new MessageProperties(Event(msg.eventID, 0), mousePtValue, nullptr));
		Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(props, msg.target, false, true, false));
		sendMessageOnVThread(dispatch);
	}

	return kVThreadReturn;
}


VThreadState Runtime::updateMousePositionTask(const UpdateMousePositionTaskData &data) {
	if (!_project)
		return kVThreadReturn;

	// This intentionally replicates a mTropolis bug/quirk where drag motion modifiers DO NOT
	// prevent "Mouse Outside" events from occurring if the mouse is moved fast enough to get
	// outside of the object.  Also, note that "Mouse Outside" intentionally DOES NOT match up
	// with the logic of "Mouse Up Inside" and "Mouse Up Outside" in updateMouseStateTask.
	//
	// e.g. if you drag an object under another object and then release the mouse, then
	// what should happen is a Mouse Outside event is sent to the bottom object, and then
	// a Mouse Up Inside event is sent when the button is released.

	Structural *collisionItem = nullptr;
	int32 bestSceneStack = INT32_MIN;
	int32 bestLayer = INT32_MIN;
	bool bestDirect = false;

	if (_numMouseBlockers == 0) {
		for (size_t ri = 0; ri < _sceneStack.size(); ri++) {
			const SceneStackEntry &sceneStackEntry = _sceneStack[_sceneStack.size() - 1 - ri];
			recursiveFindMouseCollision(collisionItem, bestSceneStack, bestLayer, bestDirect, sceneStackEntry.scene.get(), _sceneStack.size() - 1 - ri, data.x, data.y, kMouseInteractivityTestAnything);
		}
	}

	Common::SharedPtr<Structural> newMouseOver;
	Common::SharedPtr<Structural> oldMouseOver = _mouseOverObject.lock();
	if (collisionItem)
		newMouseOver = collisionItem->getSelfReference().lock().staticCast<Structural>();

	struct MessageToSend {
		EventIDs::EventID eventID;
		Structural *target;
	};

	Common::Array<MessageToSend> messagesToSend;

	if (newMouseOver != oldMouseOver) {
		if (oldMouseOver) {
			MessageToSend msg;
			msg.eventID = EventIDs::kMouseOutside;
			msg.target = oldMouseOver.get();
			messagesToSend.push_back(msg);
		}
		if (newMouseOver) {
			MessageToSend msg;
			msg.eventID = EventIDs::kMouseOver;
			msg.target = newMouseOver.get();
			messagesToSend.push_back(msg);
		}

		_mouseOverObject = newMouseOver;
	}

	Common::SharedPtr<Structural> tracked = _mouseTrackingObject.lock();


	if (tracked) {
		{
			MessageToSend msg;
			msg.eventID = EventIDs::kMouseTracking;
			msg.target = tracked.get();
			messagesToSend.push_back(msg);
		}

		assert(tracked->isElement());
		Element *element = static_cast<Element *>(tracked.get());
		assert(element->isVisual());
		VisualElement *visual = static_cast<VisualElement *>(element);
		Common::Point parentOrigin = visual->getParentOrigin();
		int32 relativeX = data.x - parentOrigin.x;
		int32 relativeY = data.y - parentOrigin.y;
		bool mouseOutside = !visual->isMouseInsideDrawableArea(relativeX, relativeY) || !visual->isMouseCollisionAtPoint(relativeX, relativeY);

		if (mouseOutside != _trackedMouseOutside) {
			if (mouseOutside) {
				MessageToSend msg;
				msg.eventID = EventIDs::kMouseTrackedOutside;
				msg.target = tracked.get();
				messagesToSend.push_back(msg);
			} else {
				MessageToSend msg;
				msg.eventID = EventIDs::kMouseTrackedInside;
				msg.target = tracked.get();
				messagesToSend.push_back(msg);
			}

			_trackedMouseOutside = mouseOutside;
		}

		// TODO: Figure out the right location for this
		if (element->isVisual()) {
			Common::Point targetPoint = Common::Point(data.x - _mouseTrackingDragStart.x + _mouseTrackingObjectInitialOrigin.x, data.y - _mouseTrackingDragStart.y + _mouseTrackingObjectInitialOrigin.y);
			static_cast<VisualElement *>(element)->handleDragMotion(this, _mouseTrackingObjectInitialOrigin, targetPoint);
		}
	}

	DynamicValue mousePtValue;
	mousePtValue.setPoint(Common::Point(data.x, data.y));

	for (size_t ri = 0; ri < messagesToSend.size(); ri++) {
		const MessageToSend &msg = messagesToSend[messagesToSend.size() - 1 - ri];
		Common::SharedPtr<MessageProperties> props(new MessageProperties(Event(msg.eventID, 0), mousePtValue, nullptr));
		Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(props, msg.target, false, true, false));
		sendMessageOnVThread(dispatch);
	}

	// Cursor element positions are only updated on mouse movement, not when initially set
	bool needUpdateElementPosition = (_cachedMousePosition.x != data.x || _cachedMousePosition.y != data.y);

	_cachedMousePosition.x = data.x;
	_cachedMousePosition.y = data.y;

	if (needUpdateElementPosition)
		updateCursorElementPosition();

	return kVThreadReturn;
}

VThreadState Runtime::applyDefaultVisibility(const ApplyDefaultVisibilityTaskData &data) {
	Event evt;
	if (data.targetVisibility) {
		if (data.element->isVisibleByDefault() == false || data.element->isVisible())
			return kVThreadReturn;

		evt = Event(EventIDs::kElementShow, 0);
	} else {
		if (!data.element->isVisible())
			return kVThreadReturn;

		evt = Event(EventIDs::kElementHide, 0);
	}

	// Visibility change events are sourced from the element
 	Common::SharedPtr<MessageProperties> props(new MessageProperties(evt, DynamicValue(), data.element->getSelfReference()));
	Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(props, data.element, false, false, true));

	sendMessageOnVThread(dispatch);

	return kVThreadReturn;
}

void Runtime::updateMainWindowCursor() {
	const uint32 kHandPointUpID = 10005;
	const uint32 kArrowID = 10011;

	if (!_mainWindow.expired()) {
		if (_haveCursorElement) {
			Common::SharedPtr<Window> mainWindow = _mainWindow.lock();
			if (_elementTrackedToCursor.expired()) {
				mainWindow->setMouseVisible(true);
				_elementTrackedToCursor.reset();
			} else {
				mainWindow->setMouseVisible(false);
				return;
			}
		}

		uint32 selectedCursor = kArrowID;
		if (!_mouseOverObject.expired())
			selectedCursor = kHandPointUpID;

		if (_haveModifierOverrideCursor)
			selectedCursor = _modifierOverrideCursorID;

		if (_project) {
			Common::SharedPtr<CursorGraphicCollection> cursorGraphics = _project->getCursorGraphics();
			if (cursorGraphics) {
				Common::SharedPtr<CursorGraphic> graphic = cursorGraphics->getGraphicByID(selectedCursor);
				if (graphic) {
					Common::SharedPtr<Window> mainWindow = _mainWindow.lock();
					mainWindow->setCursorGraphic(graphic);
					mainWindow->setMouseVisible(true);
				}
			}
		}
	}
}

void Runtime::queueMessage(const Common::SharedPtr<MessageDispatch>& dispatch) {
	_messageQueue.push_back(dispatch);
}

void Runtime::queueOSEvent(const Common::SharedPtr<OSEvent> &osEvent) {
	_osEventQueue.push_back(osEvent);
}

Scheduler &Runtime::getScheduler() {
	return _scheduler;
}

void Runtime::getScenesInRenderOrder(Common::Array<Structural*> &scenes) const {
	for (Common::Array<SceneStackEntry>::const_iterator it = _sceneStack.begin(), itEnd = _sceneStack.end(); it != itEnd; ++it) {
		scenes.push_back(it->scene.get());
	}
}

void Runtime::instantiateIfAlias(Common::SharedPtr<Modifier> &modifier, const Common::WeakPtr<RuntimeObject> &relinkParent) {
	if (modifier->isAlias()) {
		Common::SharedPtr<Modifier> templateModifier = _project->resolveAlias(static_cast<AliasModifier *>(modifier.get())->getAliasID());
		if (!templateModifier) {
			error("Failed to resolve alias");
		}

		Common::SharedPtr<Modifier> clonedModifier = templateModifier->shallowClone();
		clonedModifier->setSelfReference(clonedModifier);
		clonedModifier->setRuntimeGUID(allocateRuntimeGUID());

		clonedModifier->setName(modifier->getName());

		modifier = clonedModifier;
		clonedModifier->setParent(relinkParent);

		ModifierChildCloner cloner(this, clonedModifier);
		clonedModifier->visitInternalReferences(&cloner);

		// Aliased variables use the same variable storage, but are treated as distinct objects.
		if (clonedModifier->isVariable()) {
 			assert(templateModifier->isVariable());
			static_cast<VariableModifier *>(clonedModifier.get())->setStorage(static_cast<const VariableModifier *>(templateModifier.get())->getStorage());
		}
	}
}

Common::SharedPtr<Window> Runtime::findTopWindow(int32 x, int32 y) const {
	Common::SharedPtr<Window> bestWindow;
	int bestStrata = 0;
	for (const Common::SharedPtr<Window> &window : _windows) {
		if ((!bestWindow || bestStrata <= window->getStrata()) && !window->isMouseTransparent() && x >= window->getX() && y >= window->getY()) {
			int32 relX = x - window->getX();
			int32 relY = y - window->getY();
			if (relX < window->getWidth() && relY < window->getHeight()) {
				bestStrata = window->getStrata();
				bestWindow = window;
			}
		}
	}

	return bestWindow;
}

void Runtime::setVolume(double volume) {
	Audio::Mixer *mixer = _system->getMixer();
	mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, static_cast<int>(volume * Audio::Mixer::kMaxMixerVolume));
	mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, static_cast<int>(volume * Audio::Mixer::kMaxMixerVolume));
	mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, static_cast<int>(volume * Audio::Mixer::kMaxMixerVolume));
	mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, static_cast<int>(volume * Audio::Mixer::kMaxMixerVolume));
}

void Runtime::onMouseDown(int32 x, int32 y, Actions::MouseButton mButton) {
	_realMousePosition.x = x;
	_realMousePosition.y = y;

	Common::SharedPtr<Window> focusWindow = _mouseFocusWindow.lock();
	if (!focusWindow) {
		focusWindow = findTopWindow(x, y);
		if (!focusWindow)
			return;

		// New focus window, clear all leftover mouse button flags
		for (int i = 0; i < Actions::kMouseButtonCount; i++)
			_mouseFocusFlags[i] = false;

		_mouseFocusWindow = focusWindow;
	}

	focusWindow->onMouseDown(x - focusWindow->getX(), y - focusWindow->getY(), mButton);

	_mouseFocusFlags[mButton] = true;
}

void Runtime::onMouseMove(int32 x, int32 y) {
	_realMousePosition.x = x;
	_realMousePosition.y = y;

	Common::SharedPtr<Window> focusWindow = _mouseFocusWindow.lock();

	if (!focusWindow)
		focusWindow = findTopWindow(x, y);

	if (focusWindow)
		focusWindow->onMouseMove(x - focusWindow->getX(), y - focusWindow->getY());
}

void Runtime::onMouseUp(int32 x, int32 y, Actions::MouseButton mButton) {
	_realMousePosition.x = x;
	_realMousePosition.y = y;

	Common::SharedPtr<Window> focusWindow = _mouseFocusWindow.lock();
	if (!focusWindow)
		return;

	focusWindow->onMouseUp(x - focusWindow->getX(), y - focusWindow->getY(), mButton);
	_mouseFocusFlags[mButton] = false;
	bool anyTrue = false;
	for (int i = 0; i < Actions::kMouseButtonCount; i++) {
		if (_mouseFocusFlags[i]) {
			anyTrue = true;
			break;
		}
	}

	if (!anyTrue)
		_mouseFocusWindow.reset();
}

void Runtime::onKeyboardEvent(const Common::EventType evtType, bool repeat, const Common::KeyState &keyEvt) {
	Common::SharedPtr<Window> focusWindow = _keyFocusWindow.lock();
	if (focusWindow)
		focusWindow->onKeyboardEvent(evtType, repeat, keyEvt);
}


void Runtime::onAction(MTropolis::Actions::Action action) {
	Common::SharedPtr<Window> focusWindow = _keyFocusWindow.lock();
	if (focusWindow)
		focusWindow->onAction(action);
}

const Common::Point &Runtime::getCachedMousePosition() const {
	return _cachedMousePosition;
}

void Runtime::setModifierCursorOverride(uint32 cursorID) {
	if (!_haveModifierOverrideCursor || _modifierOverrideCursorID != cursorID) {
		_haveModifierOverrideCursor = true;
		_modifierOverrideCursorID = cursorID;
		updateMainWindowCursor();
	}
}

void Runtime::clearModifierCursorOverride() {
	if (_haveModifierOverrideCursor) {
		_haveModifierOverrideCursor = false;
		updateMainWindowCursor();
	}
}

void Runtime::forceCursorRefreshOnce() {
	_forceCursorRefreshOnce = true;
}

void Runtime::setAutoResetCursor(bool enabled) {
	_autoResetCursor = enabled;
}

uint Runtime::getMultiClickCount() const {
	return _multiClickCount;
}

bool Runtime::isAwaitingSceneTransition() const {
	return _sceneTransitionState != kSceneTransitionStateNotTransitioning;
}

Common::RandomSource *Runtime::getRandom() const {
	return _random.get();
}

WorldManagerInterface *Runtime::getWorldManagerInterface() const {
	return _worldManagerInterface.get();
}

AssetManagerInterface *Runtime::getAssetManagerInterface() const {
	return _assetManagerInterface.get();
}

SystemInterface *Runtime::getSystemInterface() const {
	return _systemInterface.get();
}

ISaveUIProvider *Runtime::getSaveProvider() const {
	return _saveProvider;
}

ILoadUIProvider *Runtime::getLoadProvider() const {
	return _loadProvider;
}

Audio::Mixer *Runtime::getAudioMixer() const {
	return _mixer;
}

Hacks &Runtime::getHacks() {
	return _hacks;
}

const Hacks &Runtime::getHacks() const {
	return _hacks;
}

void Runtime::setSceneGraphDirty() {
	_sceneGraphChanged = true;
}

void Runtime::clearSceneGraphDirty() {
	_sceneGraphChanged = false;
}

bool Runtime::isSceneGraphDirty() const {
	return _sceneGraphChanged;
}

void Runtime::addCollider(ICollider *collider) {
	Common::SharedPtr<CollisionCheckState> state(new CollisionCheckState());
	state->collider = collider;

	_colliders.push_back(state);
}

void Runtime::removeCollider(ICollider *collider) {
	size_t numColliders = _colliders.size();
	for (size_t i = 0; i < numColliders; i++) {
		if (_colliders[i]->collider == collider) {
			_colliders.remove_at(i);
			return;
		}
	}
}

void Runtime::checkCollisions(ICollider *optRestrictToCollider) {
	if (!_colliders.size())
		return;

	Common::Array<ColliderInfo> collisionObjects;
	for (size_t i = 0; i < _sceneStack.size(); i++)
		recursiveFindColliders(_sceneStack[i].scene.get(), i, collisionObjects, 0, 0, true);

	Common::sort(collisionObjects.begin(), collisionObjects.end(), sortColliderPredicate);

	for (const Common::SharedPtr<CollisionCheckState> &collisionCheckPtr : _colliders) {
		CollisionCheckState &colCheck = *collisionCheckPtr.get();

		if (optRestrictToCollider && colCheck.collider != optRestrictToCollider)
			continue;

		Modifier *modifier = nullptr;
		bool collideInFront;
		bool collideBehind;
		bool excludeParents;
		colCheck.collider->getCollisionProperties(modifier, collideInFront, collideBehind, excludeParents);

		Structural *owner = modifier->findStructuralOwner();
		if (!owner)
			continue;

		if (!owner->isElement())
			continue;

		Element *element = static_cast<Element *>(owner);
		if (!element->isVisual())
			continue;

		Common::Array<Common::WeakPtr<VisualElement> > collidingElements;

		VisualElement *visual = static_cast<VisualElement *>(element);
		if (visual->isVisible()) {
			bool foundSelf = false;
			size_t selfIndex = 0;
			Common::Rect selfRect = Common::Rect(0, 0, 0, 0);

			for (size_t i = 0; i < collisionObjects.size(); i++) {
				if (collisionObjects[i].element == visual) {
					selfIndex = i;
					selfRect = collisionObjects[i].absRect;
					foundSelf = true;
					break;
				}
			}

			// This should always be true
			if (foundSelf) {
				size_t minIndex = 0;
				size_t maxIndex = collisionObjects.size();
				if (!collideBehind)
					minIndex = selfIndex + 1;
				if (!collideInFront)
					maxIndex = selfIndex;

				for (size_t i = minIndex; i < maxIndex; i++) {
					if (i == selfIndex)
						continue;

					const ColliderInfo &collisionObject = collisionObjects[i];
					VisualElement *collisionObjectElement = collisionObject.element;

					assert(collisionObjectElement != visual);

					// Potential collision
					if (!collisionObject.absRect.intersects(selfRect))
						continue;

					if (excludeParents) {
						bool isParent = false;

						Structural *parentSearch = visual->getParent();
						while (parentSearch != nullptr) {
							if (parentSearch == collisionObjectElement) {
								isParent = true;
								break;
							}
							parentSearch = parentSearch->getParent();
						}

						if (isParent)
							continue;
					}

					collidingElements.push_back(collisionObjectElement->getSelfReference().staticCast<VisualElement>());
				}
			}
		}

		Common::Array<Common::WeakPtr<VisualElement> > &oldCollidingElements = colCheck.activeElements;

		bool shouldStop = false;

		for (size_t oldIndex = 0; oldIndex < oldCollidingElements.size();) {
			Common::SharedPtr<VisualElement> oldColElement = oldCollidingElements[oldIndex].lock();
			if (oldColElement.get() == nullptr) {
				collidingElements.remove_at(oldIndex);
				continue;
			}

			bool isStillColliding = false;
			for (size_t newIndex = 0; newIndex < collidingElements.size(); newIndex++) {
				Common::SharedPtr<VisualElement> newColElement = collidingElements[newIndex].lock();
				if (newColElement == oldColElement) {
					isStillColliding = true;
					collidingElements.remove_at(newIndex);
					break;
				}
			}

			if (!isStillColliding)
				oldCollidingElements.remove_at(oldIndex);
			else
				oldIndex++;

			if (!shouldStop)
				colCheck.collider->triggerCollision(this, oldColElement.get(), true, isStillColliding, shouldStop);
		}

		for (size_t newIndex = 0; newIndex < collidingElements.size(); newIndex++) {
			Common::SharedPtr<VisualElement> colElement = collidingElements[newIndex].lock();

			if (!shouldStop)
				colCheck.collider->triggerCollision(this, colElement.get(), false, true, shouldStop);

			oldCollidingElements.push_back(colElement);
		}
	}
}

void Runtime::setCursorElement(const Common::WeakPtr<VisualElement> &element) {
	_elementTrackedToCursor = element;
	_haveCursorElement = !element.expired();

	updateMainWindowCursor();
}

void Runtime::updateCursorElementPosition() {
	Common::SharedPtr<VisualElement> element = _elementTrackedToCursor.lock();
	if (!element)
		return;

	Common::Point elementPos = element->getGlobalPosition();
	if (elementPos != _cachedMousePosition) {
		VisualElement::OffsetTranslateTaskData *taskData = _vthread->pushTask("VisualElement::offsetTranslateTask", element.get(), &VisualElement::offsetTranslateTask);
		taskData->dx = _cachedMousePosition.x - elementPos.x;
		taskData->dy = _cachedMousePosition.y - elementPos.y;
	}
}

void Runtime::addBoundaryDetector(IBoundaryDetector *boundaryDetector) {
	BoundaryCheckState state;
	state.currentContacts = 0;
	state.detector = boundaryDetector;
	state.position = Common::Point(0, 0);
	state.positionResolved = false;

	Modifier *modifier;
	uint edgeFlags;
	bool mustBeCompletelyOutside;
	bool continuous;
	boundaryDetector->getCollisionProperties(modifier, edgeFlags, mustBeCompletelyOutside, continuous);

	_boundaryChecks.push_back(state);
}

void Runtime::removeBoundaryDetector(IBoundaryDetector *boundaryDetector) {
	size_t numColliders = _boundaryChecks.size();
	for (size_t i = 0; i < numColliders; i++) {
		if (_boundaryChecks[i].detector == boundaryDetector) {
			_boundaryChecks.remove_at(i);
			return;
		}
	}
}

void Runtime::addPostEffect(IPostEffect *postEffect) {
	_postEffects.push_back(postEffect);
}

void Runtime::removePostEffect(IPostEffect *postEffect) {
	size_t numPostEffects = _postEffects.size();
	for (size_t i = 0; i < numPostEffects; i++) {
		if (_postEffects[i] == postEffect) {
			_postEffects.remove_at(i);
			return;
		}
	}
}

const Common::Array<IPostEffect *> &Runtime::getPostEffects() const {
	return _postEffects;
}

const Palette &Runtime::getGlobalPalette() const {
	return _globalPalette;
}

void Runtime::setGlobalPalette(const Palette &palette) {
	if (_realDisplayMode <= kColorDepthMode8Bit)
		g_system->getPaletteManager()->setPalette(palette.getPalette(), 0, 256);
	else
		setSceneGraphDirty();

	_globalPalette = palette;
}

void Runtime::addMouseBlocker() {
	_numMouseBlockers++;
}

void Runtime::removeMouseBlocker() {
	assert(_numMouseBlockers > 0);
	_numMouseBlockers--;
}

void Runtime::checkBoundaries() {
	// Boundary Detection Messenger behavior is very quirky in mTropolis 1.1.  Basically, if an object moves in the direction of
	// the boundary, then it may trigger collision checks with the boundary.  If it moves but does not move in the direction of
	// the boundary, then it is considered no longer in contact with the boundary - period - which means it can trigger again
	// once it moves in the boundary direction.
	for (BoundaryCheckState &checkState : _boundaryChecks) {
		Modifier *modifier;
		uint edgeFlags;
		bool mustBeCompletelyOutside;
		bool continuous;
		checkState.detector->getCollisionProperties(modifier, edgeFlags, mustBeCompletelyOutside, continuous);

		Structural *structural = modifier->findStructuralOwner();
		if (structural == nullptr || !structural->isElement() || !static_cast<Element *>(structural)->isVisual())
			continue;

		VisualElement *visual = static_cast<VisualElement *>(structural);

		Common::Rect thisRect = visual->getRelativeRect();
		Common::Point point(thisRect.left, thisRect.top);

		if (!checkState.positionResolved) {
			checkState.positionResolved = true;
			checkState.position = point;
			continue;
		}

		if (point == checkState.position)
			continue;

		Structural *parentStructural = visual->getParent();
		if (parentStructural == nullptr || !parentStructural->isElement() || !static_cast<Element *>(parentStructural)->isVisual())
			continue;

		VisualElement *parentVisual = static_cast<VisualElement *>(parentStructural);

		Common::Point delta = point - checkState.position;

		int16 parentWidth = parentVisual->getRelativeRect().width();
		int16 parentHeight = parentVisual->getRelativeRect().height();

		uint contacts = 0;
		if (delta.x < 0) {
			int16 edge = mustBeCompletelyOutside ? thisRect.right : thisRect.left;
			if (edge < 0)
				contacts |= IBoundaryDetector::kEdgeLeft;
		}
		if (delta.y < 0) {
			int16 edge = mustBeCompletelyOutside ? thisRect.bottom : thisRect.top;
			if (edge < 0)
				contacts |= IBoundaryDetector::kEdgeTop;
		}
		if (delta.x > 0) {
			int16 edge = mustBeCompletelyOutside ? thisRect.left : thisRect.right;
			if (edge >= parentWidth)
				contacts |= IBoundaryDetector::kEdgeRight;
		}
		if (delta.y > 0) {
			int16 edge = mustBeCompletelyOutside ? thisRect.top : thisRect.bottom;
			if (edge >= parentHeight)
				contacts |= IBoundaryDetector::kEdgeBottom;
		}

		uint activatedContacts = contacts;

		// If non-continuous, then only activate new contacts
		if (!continuous)
			activatedContacts &= ~checkState.currentContacts;

		checkState.position = point;
		checkState.currentContacts = contacts;

		if (activatedContacts & edgeFlags)
			checkState.detector->triggerCollision(this);
	}
}

void Runtime::recursiveFindColliders(Structural *structural, size_t sceneStackDepth, Common::Array<ColliderInfo> &colliders, int32 parentOriginX, int32 parentOriginY, bool isRoot) {
	int32 childOffsetX = parentOriginX;
	int32 childOffsetY = parentOriginY;
	if (structural->isElement()) {
		Element *element = static_cast<Element *>(structural);
		if (element->isVisual()) {
			VisualElement *visual = static_cast<VisualElement *>(element);
			const Common::Rect &rect = visual->getRelativeRect();

			childOffsetX += rect.left;
			childOffsetY += rect.top;

			// isRoot = Is a scene, and colliding with scenes is not allowed
			if (!isRoot && visual->isVisible()) {
				ColliderInfo colliderInfo;
				colliderInfo.absRect = visual->getRelativeCollisionRect();
				colliderInfo.absRect.translate(parentOriginX, parentOriginY);
				colliderInfo.element = visual;
				colliderInfo.layer = visual->getLayer();
				colliderInfo.sceneStackDepth = sceneStackDepth;

				colliders.push_back(colliderInfo);
			}
		}
	}

	for (const Common::SharedPtr<Structural> &child : structural->getChildren())
		recursiveFindColliders(child.get(), sceneStackDepth, colliders, childOffsetX, childOffsetY, false);
}

bool Runtime::sortColliderPredicate(const ColliderInfo &a, const ColliderInfo &b) {
	if (a.layer != b.layer)
		return a.layer < b.layer;
	return a.sceneStackDepth < b.sceneStackDepth;
}

const Common::String *Runtime::resolveAttributeIDName(uint32 attribID) const {
	Common::HashMap<uint32, Common::String>::const_iterator it = _getSetAttribIDsToAttribName.find(attribID);
	if (it == _getSetAttribIDsToAttribName.end())
		return nullptr;
	else
		return &it->_value;
}

const Common::WeakPtr<Window> &Runtime::getMainWindow() const {
	return _mainWindow;
}

const Common::SharedPtr<Graphics::ManagedSurface> &Runtime::getSaveScreenshotOverride() const {
	return _saveScreenshotOverride;
}

void Runtime::setSaveScreenshotOverride(const Common::SharedPtr<Graphics::ManagedSurface> &screenshot) {
	_saveScreenshotOverride = screenshot;
}

bool Runtime::isIdle() const {
	// The runtime is idle if nothing is happening except for scheduled events and the OS queue
	if (_vthread->hasTasks())
		return false;

	if (_sceneTransitionState != kSceneTransitionStateNotTransitioning)
		return false;

	if (_forceCursorRefreshOnce)
		return false;

	if (_queuedProjectDesc)
		return false;

	if (_pendingTeardowns.size() > 0)
		return false;

	if (_pendingLowLevelTransitions.size() > 0)
		return false;

	if (_pendingClones.size() > 0)
		return false;

	if (_pendingPostCloneShowChecks.size() > 0)
		return false;

	if (_pendingShowClonedObject.size() > 0)
		return false;

	if (_pendingParentChanges.size() > 0)
		return false;

	if (_pendingKills.size() > 0)
		return false;

	if (_messageQueue.size() > 0)
		return false;

	if (_pendingSceneReturnCount > 0)
		return false;

	if (_pendingSceneTransitions.size() > 0)
		return false;

	if (_isQuitting)
		return false;

	return true;
}

const Common::SharedPtr<SubtitleRenderer> &Runtime::getSubtitleRenderer() const {
	return _subtitleRenderer;
}

void Runtime::queueCloneObject(const Common::WeakPtr<RuntimeObject> &obj) {
	Common::SharedPtr<RuntimeObject> ptr = obj.lock();

	// Cloning the same object multiple times doesn't work
	for (const Common::WeakPtr<RuntimeObject> &candidate : _pendingClones)
		if (candidate.lock() == ptr)
			return;

	_pendingClones.push_back(obj);
}

void Runtime::queueKillObject(const Common::WeakPtr<RuntimeObject> &obj) {
	Common::SharedPtr<RuntimeObject> ptr = obj.lock();

	for (const Common::WeakPtr<RuntimeObject> &candidate : _pendingKills)
		if (candidate.lock() == ptr)
			return;

	_pendingKills.push_back(obj);
}

void Runtime::queueChangeObjectParent(const Common::WeakPtr<RuntimeObject> &obj, const Common::WeakPtr<RuntimeObject> &newParent) {
	_pendingParentChanges.push_back(ObjectParentChange(obj, newParent));
}

void Runtime::hotLoadScene(Structural *structural) {
	assert(structural->getSceneLoadState() != Structural::SceneLoadState::kNotAScene);
	loadScene(structural->getSelfReference().lock().staticCast<Structural>());
}

void Runtime::ensureMainWindowExists() {
	// Maybe there's a better spot for this
	if (_mainWindow.expired() && _project) {
		const ProjectPresentationSettings &presentationSettings = _project->getPresentationSettings();

		int32 centeredX = (static_cast<int32>(_displayWidth) - static_cast<int32>(presentationSettings.width)) / 2;
		int32 centeredY = (static_cast<int32>(_displayHeight) - static_cast<int32>(presentationSettings.height)) / 2;

		centeredX += _hacks.mainWindowOffset.x;
		centeredY += _hacks.mainWindowOffset.y;

		Common::SharedPtr<Window> mainWindow(new MainWindow(WindowParameters(this, centeredX, centeredY, presentationSettings.width, presentationSettings.height, _displayModePixelFormats[_realDisplayMode])));
		addWindow(mainWindow);
		_mainWindow.reset(mainWindow);

		_keyFocusWindow = mainWindow;

		updateMainWindowCursor();
	}
}

void Runtime::unloadProject() {
	_activeMainScene.reset();
	_activeSharedScene.reset();
	_sceneStack.clear();
	_sceneReturnList.clear();
	_pendingLowLevelTransitions.clear();
	_pendingSceneTransitions.clear();
	_pendingTeardowns.clear();
	_messageQueue.clear();
	_vthread.reset(new VThread(_coroManager.get()));

	if (!_mainWindow.expired()) {
		removeWindow(_mainWindow.lock().get());
	}

	// These should be last
	_project.reset();
	_rootLinkingScope.reset();

	_haveModifierOverrideCursor = false;
}

void Runtime::refreshPlayTime() {
	_playTime = _system->getMillis() - _playTimeBase;
}


void Runtime::queueProject(const Common::SharedPtr<ProjectDescription> &desc) {
	_queuedProjectDesc = desc;
}

void Runtime::closeProject() {
	// TODO: There are actually some elaborate cases here involving opening projects, project return list,
	// Project Ended message, etc. and Obsidian actually attempts to stop MIDI playback on project end.
	// For now we just quit.
	_isQuitting = true;
}

void Runtime::addVolume(int volumeID, const char *name, bool isMounted) {
	VolumeState volume;
	volume.name = name;
	volume.isMounted = isMounted;
	volume.volumeID = volumeID;

	_volumes.push_back(volume);
}

bool Runtime::getVolumeState(const Common::String &name, int &outVolumeID, bool &outIsMounted) const {
	for (const VolumeState &volume : _volumes) {
		if (caseInsensitiveEqual(volume.name, name)) {
			outVolumeID = volume.volumeID;
			outIsMounted = volume.isMounted;
			return true;
		}
	}

	if (_defaultVolumeState) {
		outIsMounted = true;
		return true;
	}

	return false;
}

void Runtime::addSceneStateTransition(const HighLevelSceneTransition &transition) {
	_pendingSceneTransitions.push_back(transition);
}

void Runtime::addSceneReturn() {
	_pendingSceneReturnCount++;
}

void Runtime::setSceneTransitionEffect(bool isInDestinationScene, SceneTransitionEffect *effect) {
	SceneTransitionEffect *target = isInDestinationScene ? &_destinationSceneTransitionEffect : &_sourceSceneTransitionEffect;
	if (!effect)
		*target = SceneTransitionEffect();
	else
		*target = *effect;
}

Project *Runtime::getProject() const {
	return _project.get();
}

void Runtime::postConsumeMessageTask(IMessageConsumer *consumer, const Common::SharedPtr<MessageProperties> &msg) {
	ConsumeMessageTaskData *params = _vthread->pushTask("Runtime::consumeMessageTask", this, &Runtime::consumeMessageTask);
	params->consumer = consumer;
	params->message = msg;
}

void Runtime::postConsumeCommandTask(Structural *structural, const Common::SharedPtr<MessageProperties> &msg) {
	ConsumeCommandTaskData *params = _vthread->pushTask("Runtime::consumeMessageTask", this, &Runtime::consumeCommandTask);
	params->structural = structural;
	params->message = msg;
}

uint32 Runtime::allocateRuntimeGUID() {
	return _nextRuntimeGUID++;
}

void Runtime::addWindow(const Common::SharedPtr<Window> &window) {
	_windows.push_back(window);
}

void Runtime::removeWindow(Window *window) {
	for (size_t i = 0; i < _windows.size(); i++) {
		if (_windows[i].get() == window) {
			window->detachFromRuntime();
			_windows.remove_at(i);
			break;
		}
	}
}

void Runtime::setupDisplayMode(ColorDepthMode displayMode, const Graphics::PixelFormat &pixelFormat) {
	_displayModeSupported[displayMode] = true;
	_displayModePixelFormats[displayMode] = pixelFormat;
}

bool Runtime::isDisplayModeSupported(ColorDepthMode displayMode) const {
	return _displayModeSupported[displayMode];
}

bool Runtime::switchDisplayMode(ColorDepthMode realDisplayMode, ColorDepthMode fakeDisplayMode) {
	_fakeDisplayMode = fakeDisplayMode;

	if (_realDisplayMode != realDisplayMode) {
		_realDisplayMode = realDisplayMode;
		_windows.clear();
		return true;
	}

	return false;
}

void Runtime::setDisplayResolution(uint16 width, uint16 height) {
	_displayWidth = width;
	_displayHeight = height;
}

void Runtime::getDisplayResolution(uint16 &outWidth, uint16 &outHeight) const {
	outWidth = _displayWidth;
	outHeight = _displayHeight;
}

ColorDepthMode Runtime::getRealColorDepth() const {
	return _realDisplayMode;
}

ColorDepthMode Runtime::getFakeColorDepth() const {
	return _fakeDisplayMode;
}

const Graphics::PixelFormat& Runtime::getRenderPixelFormat() const {
	assert(_realDisplayMode != kColorDepthModeInvalid);

	return _displayModePixelFormats[_realDisplayMode];
}

const Common::SharedPtr<Graphics::MacFontManager>& Runtime::getMacFontManager() const {
	return _macFontMan;
}

const Common::SharedPtr<Structural> &Runtime::getActiveMainScene() const {
	return _activeMainScene;
}

const Common::SharedPtr<Structural> &Runtime::getActiveSharedScene() const {
	return _activeSharedScene;
}

void Runtime::getSceneStack(Common::Array<Common::SharedPtr<Structural> >& sceneStack) const {
	sceneStack.clear();
	for (const SceneStackEntry &stackEntry : _sceneStack)
		sceneStack.push_back(stackEntry.scene);
}

bool Runtime::mustDraw() const {
	if (_sceneTransitionState == kSceneTransitionStateWaitingForDraw)
		return true;
	return false;
}

uint64 Runtime::getRealTime() const {
	return _realTime;
}

uint64 Runtime::getPlayTime() const {
	return _playTime;
}

VThread& Runtime::getVThread() const {
	return *_vthread.get();
}

#ifdef MTROPOLIS_DEBUG_ENABLE

void Runtime::debugSetEnabled(bool enabled) {
	if (enabled) {
		if (!_debugger)
			_debugger.reset(new Debugger(this));
	} else {
		_debugger.reset();
	}
}
void Runtime::debugBreak() {
	debugSetEnabled(true);
	_debugger->setPaused(true);
}

Debugger *Runtime::debugGetDebugger() const {
	return _debugger.get();
}

void Runtime::debugGetPrimaryTaskList(Common::Array<Common::SharedPtr<DebugPrimaryTaskList> > &primaryTaskLists) {
	{
		Common::SharedPtr<DebugPrimaryTaskList> vthreadTaskList(new DebugPrimaryTaskList("Execute"));
		primaryTaskLists.push_back(vthreadTaskList);
	}

	{
		Common::SharedPtr<DebugPrimaryTaskList> projectQueueTaskList(new DebugPrimaryTaskList("Project queue"));
		primaryTaskLists.push_back(projectQueueTaskList);
	}

	{
		Common::SharedPtr<DebugPrimaryTaskList> messageQueueTaskList(new DebugPrimaryTaskList("Message queue"));
		primaryTaskLists.push_back(messageQueueTaskList);
	}

	{
		Common::SharedPtr<DebugPrimaryTaskList> teardownTaskList(new DebugPrimaryTaskList("Teardowns"));
		primaryTaskLists.push_back(teardownTaskList);
	}

	{
		Common::SharedPtr<DebugPrimaryTaskList> llstTasks(new DebugPrimaryTaskList("Low-level scene transitions"));
		primaryTaskLists.push_back(llstTasks);
	}

	{
		Common::SharedPtr<DebugPrimaryTaskList> hlstTasks(new DebugPrimaryTaskList("High-level scene transitions"));
		primaryTaskLists.push_back(hlstTasks);
	}

	{
		Common::SharedPtr<DebugPrimaryTaskList> scheduledEventsTasks(new DebugPrimaryTaskList("Scheduled events"));
		primaryTaskLists.push_back(scheduledEventsTasks);
	}
}

#endif /* MTROPOLIS_DEBUG_ENABLE */

const Common::Array<Common::SharedPtr<Modifier> > &IModifierContainer::getModifiers() const {
	return const_cast<IModifierContainer &>(*this).getModifiers();
}

ChildLoaderContext::ChildLoaderContext() : remainingCount(0), type(kTypeUnknown) {
	memset(&this->containerUnion, 0, sizeof(this->containerUnion));
}

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

PlayMediaSignaller::PlayMediaSignaller() {
}

PlayMediaSignaller::~PlayMediaSignaller() {
}

void PlayMediaSignaller::playMedia(Runtime *runtime, Project *project) {
	const size_t numReceivers = _receivers.size();
	for (size_t i = 0; i < numReceivers; i++) {
		_receivers[i]->playMedia(runtime, project);
	}
}

void PlayMediaSignaller::addReceiver(IPlayMediaSignalReceiver *receiver) {
	_receivers.push_back(receiver);
}

void PlayMediaSignaller::removeReceiver(IPlayMediaSignalReceiver *receiver) {
	for (size_t i = 0; i < _receivers.size(); i++) {
		if (_receivers[i] == receiver) {
			_receivers.remove_at(i);
			break;
		}
	}
}


SegmentUnloadSignaller::SegmentUnloadSignaller(Project *project, int segmentIndex) : _project(project), _segmentIndex(segmentIndex) {
}

SegmentUnloadSignaller::~SegmentUnloadSignaller() {
}

void SegmentUnloadSignaller::onSegmentUnloaded() {
	_project = nullptr;

	// Need to be careful here because a receiver may unload this object, causing _receivers.size() to be invalid
	const size_t numReceivers = _receivers.size();
	for (size_t i = 0; i < numReceivers; i++) {
		_receivers[i]->onSegmentUnloaded(_segmentIndex);
	}
}

void SegmentUnloadSignaller::addReceiver(ISegmentUnloadSignalReceiver *receiver) {
	_receivers.push_back(receiver);
}

void SegmentUnloadSignaller::removeReceiver(ISegmentUnloadSignalReceiver *receiver) {
	for (size_t i = 0; i < _receivers.size(); i++) {
		if (_receivers[i] == receiver) {
			_receivers.remove_at(i);
			break;
		}
	}
}

KeyboardEventSignaller::KeyboardEventSignaller() {
}

KeyboardEventSignaller::~KeyboardEventSignaller() {
}

void KeyboardEventSignaller::onKeyboardEvent(Runtime *runtime, const KeyboardInputEvent &keyEvt) {
	const size_t numReceivers = _receivers.size();
	for (size_t i = 0; i < numReceivers; i++) {
		_receivers[i]->onKeyboardEvent(runtime, keyEvt);
	}
}

void KeyboardEventSignaller::addReceiver(IKeyboardEventReceiver *receiver) {
	_receivers.push_back(receiver);
}

void KeyboardEventSignaller::removeReceiver(IKeyboardEventReceiver *receiver) {
	for (size_t i = 0; i < _receivers.size(); i++) {
		if (_receivers[i] == receiver) {
			_receivers.remove_at(i);
			break;
		}
	}
}

MediaCueState::MediaCueState() : minTime(0), maxTime(0), sourceModifier(nullptr), triggerTiming(kTriggerTimingStart) {
}

void MediaCueState::checkTimestampChange(Runtime *runtime, uint32 oldTS, uint32 newTS, bool continuousTimestamps, bool canTriggerDuring) {
	bool entersRange = (static_cast<int32>(oldTS) < minTime && static_cast<int32>(newTS) >= minTime);
	bool exitsRange = (static_cast<int32>(oldTS) <= maxTime && static_cast<int32>(newTS) > maxTime);
	bool endsInRange = (static_cast<int32>(newTS) >= minTime && static_cast<int32>(newTS) <= maxTime);

	bool shouldTrigger = false;
	switch (triggerTiming) {
	case kTriggerTimingStart:
		shouldTrigger = continuousTimestamps ? entersRange : endsInRange;
		break;
	case kTriggerTimingEnd:
		shouldTrigger = continuousTimestamps ? exitsRange : false;
		break;
	case kTriggerTimingDuring:
		shouldTrigger = canTriggerDuring ? endsInRange : false;
		break;
	default:
		break;
	}

	// Given the positioning of this, there's not really a way for the immediate flag to have any effect?
	if (shouldTrigger)
		send.sendFromMessenger(runtime, sourceModifier->getMediaCueModifier(), sourceModifier->getMediaCueTriggerSource().lock().get(), incomingData, nullptr);
}


Project::LabelSuperGroup::LabelSuperGroup() : firstRootNodeIndex(0), numRootNodes(0), numTotalNodes(0), superGroupID(0) {
}

Project::LabelTree::LabelTree() : firstChildIndex(0), numChildren(0), id(0) {
}

Project::Segment::Segment() : weakStream(nullptr) {
}

Project::StreamDesc::StreamDesc() : streamType(kStreamTypeUnknown), segmentIndex(0), size(0), pos(0) {
}

Project::AssetDesc::AssetDesc() : typeCode(0), id(0), streamID(0), filePosition(0) {
}

Project::Project(Runtime *runtime)
	: Structural(runtime), _projectFormat(Data::kProjectFormatUnknown),
	  _haveGlobalObjectInfo(false), _haveProjectStructuralDef(false), _playMediaSignaller(new PlayMediaSignaller()),
	  _keyboardEventSignaller(new KeyboardEventSignaller()),
	  _platform(kProjectPlatformUnknown), _rootArchive(nullptr), _runtimeVersion(kRuntimeVersion100) {
}

Project::~Project() {
	// Project teardown can be chaotic, we need to get rid of things in an orderly fashion.

	// Remove all modifiers and structural children, which should unhook anything referencing an asset
	_modifiers.clear();
	_children.clear();

	// Remove all global modifiers
	_globalModifiers.clear();

	// Unhook assets assets
	_assets.clear();

	// Unhook plug-ins
	_plugIns.clear();

	// Unhook cursor graphics
	_cursorGraphics.reset();

	// Close all segment streams
	for (size_t i = 0; i < _segments.size(); i++)
		closeSegmentStream(i);

	// Last of all, release project resources
	_resources.reset();
}

VThreadState Project::asyncConsumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (Event(EventIDs::kCloseProject, 0).respondsTo(msg->getEvent())) {
		runtime->closeProject();
		return kVThreadReturn;
	}

	return Structural::asyncConsumeCommand(runtime, msg);
}

MiniscriptInstructionOutcome Project::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "allowquit" || attrib == "allowquitkey") {
		DynamicValueWriteDiscardHelper::create(result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return Structural::writeRefAttribute(thread, result, attrib);
}

void Project::loadFromDescription(const ProjectDescription &desc, const Hacks &hacks) {
	_resources = desc.getResources();
	_cursorGraphics = desc.getCursorGraphics();
	_subtitles = desc.getSubtitles();
	_platform = desc.getPlatform();
	_rootArchive = desc.getRootArchive();
	_projectRootDir = desc.getProjectRootDir();
	_runtimeVersion = desc.getRuntimeVersion();

	debug(1, "Loading new project...");

	const Common::Array<Common::SharedPtr<PlugIn> > &plugIns = desc.getPlugIns();

	for (Common::Array<Common::SharedPtr<PlugIn> >::const_iterator it = plugIns.begin(), itEnd = plugIns.end(); it != itEnd; ++it) {
		Common::SharedPtr<PlugIn> plugIn = (*it);

		_plugIns.push_back(plugIn);

		plugIn->registerModifiers(&_plugInRegistry);
	}

	const Data::PlugInModifierRegistry &plugInDataLoaderRegistry = _plugInRegistry.getDataLoaderRegistry();

	size_t numSegments = desc.getSegments().size();
	debug(1, "Loading %d segments", (int)numSegments);
	_segments.resize(numSegments);

	for (size_t i = 0; i < numSegments; i++) {
		_segments[i].desc = desc.getSegments()[i];
	}

	// Try to open the first segment
	openSegmentStream(0);

	Common::SeekableReadStream *baseStream = _segments[0].weakStream;
	uint16 startValue = baseStream->readUint16LE();

	if (startValue == 1) {
		// Windows format
		_projectFormat = Data::kProjectFormatWindows;
	} else if (startValue == 0) {
		// Mac format
		_projectFormat = Data::kProjectFormatMacintosh;
	} else if (startValue == 8) {
		// Cross-platform format
		_projectFormat = Data::kProjectFormatNeutral;
	} else {
		warning("Unrecognized project segment header (startValue: %d)", startValue);
		_projectFormat = Data::kProjectFormatWindows;
	}

	Common::SeekableSubReadStream stream(baseStream, 2, baseStream->size());

	Data::DataReader catReader(2, stream, (_projectFormat == Data::kProjectFormatMacintosh) ? Data::kDataFormatMacintosh : Data::kDataFormatWindows, desc.getRuntimeVersion(), desc.isRuntimeVersionAuto());

	uint32 magic = 0;
	uint32 hdr1 = 0;
	uint32 hdr2 = 0;
	if (!catReader.readMultiple(magic, hdr1, hdr2) || magic != 0xaa55a5a5 || (hdr1 != 0 && hdr1 != 0x2000000) || hdr2 != 14) {
		error("Unrecognized project segment header (%x, %x, %d)", magic, hdr1, hdr2);
	}

	if (hdr1 == 0x2000000 && _isRuntimeVersionAutoDetect && _runtimeVersion < kRuntimeVersion200) {
		debug(1, "Version auto-detect: Detected as 2.0.0 from V2 project header");
		catReader.setRuntimeVersion(kRuntimeVersion200);
		_runtimeVersion = kRuntimeVersion200;
	}

	Common::SharedPtr<Data::DataObject> dataObject;
	Data::loadDataObject(_plugInRegistry.getDataLoaderRegistry(), catReader, dataObject);

	if (!dataObject || dataObject->getType() != Data::DataObjectTypes::kProjectHeader) {
		error("Expected project header but found something else");
	}

	Data::loadDataObject(plugInDataLoaderRegistry, catReader, dataObject);
	if (!dataObject || dataObject->getType() != Data::DataObjectTypes::kProjectCatalog) {
		error("Expected project catalog but found something else");
	}

	// Catalog version can update version auto-detect
	_runtimeVersion = catReader.getRuntimeVersion();

	Data::ProjectCatalog *catalog = static_cast<Data::ProjectCatalog *>(dataObject.get());

	_segments.resize(catalog->segments.size());

	debug(1, "Catalog loaded OK, identified %i streams", static_cast<int>(catalog->streams.size()));

	_streams.resize(catalog->streams.size());
	for (size_t i = 0; i < _streams.size(); i++) {
		StreamDesc &streamDesc = _streams[i];
		const Data::ProjectCatalog::StreamDesc &srcStream = catalog->streams[i];

		if (!strcmp(srcStream.streamType, "assetStream") || !strcmp(srcStream.streamType, "assetstream"))
			streamDesc.streamType = kStreamTypeAsset;
		else if (!strcmp(srcStream.streamType, "bootStream") || !strcmp(srcStream.streamType, "bootstream"))
			streamDesc.streamType = kStreamTypeBoot;
		else if (!strcmp(srcStream.streamType, "sceneStream") || !strcmp(srcStream.streamType, "scenestream"))
			streamDesc.streamType = kStreamTypeScene;
		else
			streamDesc.streamType = kStreamTypeUnknown;

		streamDesc.segmentIndex = srcStream.segmentIndexPlusOne - 1;
		streamDesc.size = (_platform == kProjectPlatformMacintosh) ? srcStream.macSize : srcStream.winSize;
		streamDesc.pos = (_platform == kProjectPlatformMacintosh) ? srcStream.macPos : srcStream.winPos;
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

	loadBootStream(bootStreamIndex, hacks);

	debug(1, "Boot stream loaded successfully");
}

void Project::loadSceneFromStream(const Common::SharedPtr<Structural> &scene, uint32 streamID, const Hacks &hacks) {
	if (streamID == 0 || streamID > _streams.size()) {
		error("Invalid stream ID");
	}

	size_t streamIndex = streamID - 1;

	const StreamDesc &streamDesc = _streams[streamIndex];
	uint segmentIndex = streamDesc.segmentIndex;

	openSegmentStream(segmentIndex);

	Common::SeekableSubReadStream stream(_segments[segmentIndex].weakStream, streamDesc.pos, streamDesc.pos + streamDesc.size);
	Data::DataReader reader(streamDesc.pos, stream, (_platform == kProjectPlatformMacintosh) ? Data::kDataFormatMacintosh : Data::kDataFormatWindows, _runtimeVersion, _isRuntimeVersionAutoDetect);

	if (getRuntime()->getHacks().mtiHispaniolaDamagedStringHack && scene->getName() == "C01b : Main Deck Helm Kidnap")
		reader.setPermitDamagedStrings(true);

	const Data::PlugInModifierRegistry &plugInDataLoaderRegistry = _plugInRegistry.getDataLoaderRegistry();

	{
		Common::SharedPtr<Data::DataObject> dataObject;
		Data::loadDataObject(plugInDataLoaderRegistry, reader, dataObject);

		if (dataObject == nullptr || dataObject->getType() != Data::DataObjectTypes::kStreamHeader) {
			error("Scene stream header was missing");
		}
	}

	ChildLoaderStack loaderStack;
	AssetDefLoaderContext assetDefLoader;

	{
		ChildLoaderContext loaderContext;
		loaderContext.containerUnion.filteredElements.filterFunc = Data::DataObjectTypes::isElement;
		loaderContext.containerUnion.filteredElements.structural = scene.get();
		loaderContext.remainingCount = 0;
		loaderContext.type = ChildLoaderContext::kTypeFilteredElements;

		loaderStack.contexts.push_back(loaderContext);
	}

	int numObjectsLoaded = 0;
	while (stream.pos() != streamDesc.size) {
		Common::SharedPtr<Data::DataObject> dataObject;
		Data::loadDataObject(plugInDataLoaderRegistry, reader, dataObject);

		if (!dataObject) {
			error("Failed to load stream");
		}

		Data::DataObjectTypes::DataObjectType dataObjectType = dataObject->getType();

		if (Data::DataObjectTypes::isAsset(dataObjectType)) {
			// Asset defs can appear anywhere
			loadAssetDef(streamIndex, assetDefLoader, *dataObject.get());
		} else if (dataObjectType == Data::DataObjectTypes::kAssetDataChunk) {
			// Ignore
			continue;
		} else if (loaderStack.contexts.size() > 0) {
			loadContextualObject(streamIndex, loaderStack, *dataObject.get());
		} else {
			error("Unexpectedly exited scene context in loader");
		}

		numObjectsLoaded++;
	}
	debug(9, "Loaded %d scene objects", numObjectsLoaded);

	if ((loaderStack.contexts.size() == 1 && loaderStack.contexts[0].type != ChildLoaderContext::kTypeFilteredElements) || loaderStack.contexts.size() > 1) {
		error("Scene stream loader finished in an expected state, something didn't finish loading");
	}

	scene->holdAssets(assetDefLoader.assets);
	assignAssets(assetDefLoader.assets, hacks);
}

Common::SharedPtr<Modifier> Project::resolveAlias(uint32 aliasID) const {
	if (aliasID == 0 || aliasID > _globalModifiers.getModifiers().size())
		return Common::SharedPtr<Modifier>();

	return _globalModifiers.getModifiers()[aliasID - 1];
}

Common::SharedPtr<Modifier> Project::findGlobalVarWithName(const Common::String &name) const {
	for (const Common::SharedPtr<Modifier> &modifier : _globalModifiers.getModifiers()) {
		if (modifier && modifier->isVariable() && MTropolis::caseInsensitiveEqual(name, modifier->getName()))
			return modifier;
	}

	return nullptr;
}

void Project::materializeGlobalVariables(Runtime *runtime, ObjectLinkingScope *outerScope) {
	for (Common::Array<Common::SharedPtr<Modifier> >::const_iterator it = _globalModifiers.getModifiers().begin(), itEnd = _globalModifiers.getModifiers().end(); it != itEnd; ++it) {
		Modifier *modifier = it->get();
		if (!modifier)
			continue;

		if (modifier->isVariable())
			modifier->materialize(runtime, outerScope);
	}
}

const ProjectPresentationSettings& Project::getPresentationSettings() const {
	return _presentationSettings;
}

bool Project::isProject() const {
	return true;
}

Common::String Project::getAssetNameByID(uint32 assetID) const {
	if (assetID >= _assetsByID.size())
		return Common::String();

	return _assetsByID[assetID]->name;
}

Common::WeakPtr<Asset> Project::getAssetByID(uint32 assetID) const {
	if (assetID >= _assetsByID.size())
		return Common::WeakPtr<Asset>();

	const AssetDesc *desc = _assetsByID[assetID];
	if (desc == nullptr)
		return Common::WeakPtr<Asset>();

	return desc->asset;
}

void Project::forceLoadAsset(uint32 assetID, Common::Array<Common::SharedPtr<Asset> > &outHoldAssets) {
	AssetDesc *assetDesc = _assetsByID[assetID];
	uint32 streamID = assetDesc->streamID;

	size_t streamIndex = streamID - 1;

	const StreamDesc &streamDesc = _streams[streamIndex];
	uint segmentIndex = streamDesc.segmentIndex;

	openSegmentStream(segmentIndex);

	Common::SeekableSubReadStream stream(_segments[segmentIndex].weakStream, streamDesc.pos, streamDesc.pos + streamDesc.size);
	Data::DataReader reader(streamDesc.pos, stream, (_projectFormat == Data::kProjectFormatMacintosh) ? Data::kDataFormatMacintosh : Data::kDataFormatWindows, _runtimeVersion, _isRuntimeVersionAutoDetect);

	const Data::PlugInModifierRegistry &plugInDataLoaderRegistry = _plugInRegistry.getDataLoaderRegistry();

	reader.seek(assetDesc->filePosition - streamDesc.pos);

	Common::SharedPtr<Data::DataObject> dataObject;
	Data::loadDataObject(plugInDataLoaderRegistry, reader, dataObject);

	if (!dataObject) {
		error("Failed to force-load asset data object");
	}

	Data::DataObjectTypes::DataObjectType dataObjectType = dataObject->getType();

	if (!Data::DataObjectTypes::isAsset(dataObjectType)) {
		error("Failed to force-load asset, the data object at the expected position wasn't an asset");
	}

	AssetDefLoaderContext assetDefLoader;
	loadAssetDef(streamIndex, assetDefLoader, *dataObject.get());

	assignAssets(assetDefLoader.assets, getRuntime()->getHacks());

	outHoldAssets = Common::move(assetDefLoader.assets);
}

bool Project::getAssetIDByName(const Common::String &assetName, uint32 &outAssetID) const {
	for (uint32 assetID = 0; assetID < _assetsByID.size(); assetID++) {
		const AssetDesc *assetDesc = _assetsByID[assetID];
		if (!assetDesc)
			continue;

		if (caseInsensitiveEqual(assetName, assetDesc->name)) {
			outAssetID = assetID;
			return true;
		}
	}

	return false;
}

size_t Project::getSegmentForStreamIndex(size_t streamIndex) const {
	return _streams[streamIndex].segmentIndex;
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
		Common::Path defaultPath = _projectRootDir.appendComponent(segment.desc.filePath);

		if (_platform == kProjectPlatformMacintosh)
			segment.rcStream.reset(Common::MacResManager::openFileOrDataFork(defaultPath, *_rootArchive));
		else
			segment.rcStream.reset(_rootArchive->createReadStreamForMember(defaultPath));

		if (!segment.rcStream) {
			warning("Segment '%s' isn't in the project directory", segment.desc.filePath.c_str());

			Common::ArchiveMemberList memberList;
			Common::ArchiveMemberPtr locatedMember;

			_rootArchive->listMembers(memberList);

			for (const Common::ArchiveMemberPtr &member : memberList) {
				if (member->getFileName().equalsIgnoreCase(segment.desc.filePath)) {
					if (locatedMember)
						error("Segment '%s' exists multiple times in the workspace, and isn't in the project directory, couldn't disambiguate", segment.desc.filePath.c_str());

					locatedMember = member;
				}
			}

			if (!locatedMember)
				error("Segment '%s' is missing from the workspace", segment.desc.filePath.c_str());

			if (_platform == kProjectPlatformMacintosh)
				segment.rcStream.reset(Common::MacResManager::openFileOrDataFork(locatedMember->getPathInArchive(), *_rootArchive));
			else
				segment.rcStream.reset(locatedMember->createReadStream());

			if (!segment.rcStream)
				error("Failed to open segment file %s", segment.desc.filePath.c_str());
		}

		segment.weakStream = segment.rcStream.get();
	}

	segment.unloadSignaller.reset(new SegmentUnloadSignaller(this, segmentIndex));
}

void Project::closeSegmentStream(int segmentIndex) {
	Segment &segment = _segments[segmentIndex];

	if (!segment.weakStream)
		return;

	segment.unloadSignaller->onSegmentUnloaded();
	segment.unloadSignaller.reset();
	segment.rcStream.reset();
	segment.weakStream = nullptr;
}

Common::SeekableReadStream* Project::getStreamForSegment(int segmentIndex) {
	return _segments[segmentIndex].weakStream;
}

const Common::String *Project::findNameOfLabel(const Label &label) const {
	for (const LabelSuperGroup &superGroup : _labelSuperGroups) {
		if (superGroup.superGroupID == label.superGroupID) {
			size_t firstRootIndex = superGroup.firstRootNodeIndex;
			size_t totalNodes = superGroup.numTotalNodes;

			for (size_t i = 0; i < totalNodes; i++) {
				const LabelTree &tree = _labelTree[i + firstRootIndex];
				if (tree.id == label.id)
					return &tree.name;
			}
		}
	}

	return nullptr;
}

Common::SharedPtr<SegmentUnloadSignaller> Project::notifyOnSegmentUnload(int segmentIndex, ISegmentUnloadSignalReceiver *receiver) {
	Common::SharedPtr<SegmentUnloadSignaller> signaller = _segments[segmentIndex].unloadSignaller;
	if (signaller)
		signaller->addReceiver(receiver);
	return signaller;
}

void Project::onPostRender() {
	_playMediaSignaller->playMedia(getRuntime(), this);
}

Common::SharedPtr<PlayMediaSignaller> Project::notifyOnPlayMedia(IPlayMediaSignalReceiver *receiver) {
	_playMediaSignaller->addReceiver(receiver);
	return _playMediaSignaller;
}

void Project::onKeyboardEvent(Runtime *runtime, const KeyboardInputEvent &keyEvt) {
	_keyboardEventSignaller->onKeyboardEvent(runtime, keyEvt);
}

Common::SharedPtr<KeyboardEventSignaller> Project::notifyOnKeyboardEvent(IKeyboardEventReceiver *receiver) {
	_keyboardEventSignaller->addReceiver(receiver);
	return _keyboardEventSignaller;
}

const char *Project::findAuthorMessageName(uint32 id) const {
	for (size_t i = 0; i < _labelSuperGroups.size(); i++) {
		const LabelSuperGroup &sg = _labelSuperGroups[i];
		if (sg.name == "Author Messages") {
			size_t firstNode = sg.firstRootNodeIndex;
			size_t numNodes = sg.numTotalNodes;
			for (size_t j = 0; j < numNodes; j++) {
				const LabelTree &treeNode = _labelTree[j + firstNode];
				if (treeNode.id == id)
					return treeNode.name.c_str();
			}

			break;
		}
	}

	return "Unknown";
}

const Common::SharedPtr<CursorGraphicCollection> &Project::getCursorGraphics() const {
	return _cursorGraphics;
}

void Project::loadBootStream(size_t streamIndex, const Hacks &hacks) {
	const StreamDesc &streamDesc = _streams[streamIndex];

	size_t segmentIndex = streamDesc.segmentIndex;
	openSegmentStream(segmentIndex);

	Common::SeekableSubReadStream stream(_segments[segmentIndex].weakStream, streamDesc.pos, streamDesc.pos + streamDesc.size);
	Data::DataReader reader(streamDesc.pos, stream, (_platform == kProjectPlatformMacintosh) ? Data::kDataFormatMacintosh : Data::kDataFormatWindows, _runtimeVersion, _isRuntimeVersionAutoDetect);

	ChildLoaderStack loaderStack;
	AssetDefLoaderContext assetDefLoader;

	const Data::PlugInModifierRegistry &plugInDataLoaderRegistry = _plugInRegistry.getDataLoaderRegistry();

	int numObjectsLoaded = 0;
	while (stream.pos() != streamDesc.size) {
		Common::SharedPtr<Data::DataObject> dataObject;
		Data::loadDataObject(plugInDataLoaderRegistry, reader, dataObject);

		if (!dataObject) {
			error("Failed to load project boot data");
		}

		Data::DataObjectTypes::DataObjectType dataObjectType = dataObject->getType();

		if (Data::DataObjectTypes::isAsset(dataObjectType)) {
			// Asset defs can appear anywhere
			loadAssetDef(streamIndex, assetDefLoader, *dataObject.get());
		} else if (dataObjectType == Data::DataObjectTypes::kAssetDataChunk) {
			// Ignore
			continue;
		} else if (loaderStack.contexts.size() > 0) {
			loadContextualObject(streamIndex, loaderStack, *dataObject.get());
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
			case Data::DataObjectTypes::kProjectLabelMap:
				loadLabelMap(*static_cast<const Data::ProjectLabelMap *>(dataObject.get()));
				break;
			case Data::DataObjectTypes::kProjectStructuralDef: {
					if (_haveProjectStructuralDef)
						error("Multiple project structural defs");

					const Data::ProjectStructuralDef *def = static_cast<const Data::ProjectStructuralDef *>(dataObject.get());
					_name = def->name;
					_guid = def->guid;

					_haveProjectStructuralDef = true;

					ChildLoaderContext loaderContext;
					loaderContext.containerUnion.structural = this;
					loaderContext.remainingCount = 0;
					loaderContext.type = ChildLoaderContext::kTypeProject;

					loaderStack.contexts.push_back(loaderContext);

					initAdditionalSegments(def->name);
				} break;
			case Data::DataObjectTypes::kStreamHeader:
			case Data::DataObjectTypes::kUnknown19:
			case Data::DataObjectTypes::kUnknown2B:
				// Ignore
				break;
			default:
				error("Unexpected object type in boot stream");
			}
		}

		numObjectsLoaded++;
	}

	debug(9, "Loaded %d boot objects", numObjectsLoaded);

	if (loaderStack.contexts.size() != 1 || loaderStack.contexts[0].type != ChildLoaderContext::kTypeProject) {
		error("Boot stream loader finished in an expected state, something didn't finish loading");
	}

	holdAssets(assetDefLoader.assets);
	assignAssets(assetDefLoader.assets, hacks);
}

const SubtitleTables &Project::getSubtitles() const {
	return _subtitles;
}

RuntimeVersion Project::getRuntimeVersion() const {
	return _runtimeVersion;
}

ProjectPlatform Project::getPlatform() const {
	return _platform;
}

void Project::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	error("Cloning a project is not supported");
}

Common::SharedPtr<Structural> Project::shallowClone() const {
	error("Cloning a project is not supported");
	return nullptr;
}

void Project::loadPresentationSettings(const Data::PresentationSettings &presentationSettings) {
	_presentationSettings.bitsPerPixel = presentationSettings.bitsPerPixel;
	if (_presentationSettings.bitsPerPixel != 8 && _presentationSettings.bitsPerPixel != 16) {
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

			if (assetCatalog.haveRev4Fields)
				assetDesc.typeCode = assetInfo.rev4Fields.assetType;
			else
				assetDesc.typeCode = 0;

			assetDesc.streamID = assetInfo.streamID;
			assetDesc.filePosition = assetInfo.filePosition;

			_assetsByID[assetDesc.id] = &assetDesc;
			if (!assetDesc.name.empty())
				_assetNameToID[assetDesc.name] = assetDesc.id;
		}
	}
}

void Project::loadGlobalObjectInfo(ChildLoaderStack &loaderStack, const Data::GlobalObjectInfo& globalObjectInfo) {
	if (_haveGlobalObjectInfo)
		error("Multiple global object infos");

	_haveGlobalObjectInfo = true;

	if (globalObjectInfo.numGlobalModifiers > 0) {
		ChildLoaderContext loaderContext;
		loaderContext.containerUnion.modifierContainer = &_globalModifiers;
		loaderContext.remainingCount = globalObjectInfo.numGlobalModifiers;
		loaderContext.type = ChildLoaderContext::kTypeCountedModifierList;

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
		SIModifierFactory *factory = getModifierFactoryForDataObjectType(dataObject.getType());

		if (!factory)
			error("Unknown or unsupported modifier type, or non-modifier encountered where a modifier was expected");

		modifier = factory->createModifier(loaderContext, dataObject);
	}
	if (!modifier)
		error("Modifier object failed to load");
	assert(modifier->getModifierFlags().flagsWereLoaded);

	uint32 guid = modifier->getStaticGUID();
	const Common::HashMap<uint32, Common::SharedPtr<ModifierHooks> > &hooksMap = getRuntime()->getHacks().modifierHooks;
	Common::HashMap<uint32, Common::SharedPtr<ModifierHooks> >::const_iterator hooksIt = hooksMap.find(guid);
	if (hooksIt != hooksMap.end()) {
		modifier->setHooks(hooksIt->_value);
		hooksIt->_value->onCreate(modifier.get());
	}

	return modifier;
}

void Project::loadLabelMap(const Data::ProjectLabelMap &projectLabelMap) {
	debug(1, "Loading label map...");

	_labelSuperGroups.resize(projectLabelMap.numSuperGroups);

	size_t totalLabels = 0;
	for (size_t i = 0; i < projectLabelMap.numSuperGroups; i++) {
		_labelSuperGroups[i].numTotalNodes = 0;
		for (size_t j = 0; j < projectLabelMap.superGroups[i].numChildren; j++)
			totalLabels += recursiveCountLabels(projectLabelMap.superGroups[i].tree[j]);
	}

	Common::Array<const Data::ProjectLabelMap::LabelTree *> treeQueue;
	treeQueue.resize(totalLabels);
	_labelTree.resize(totalLabels);

	// Expand label tree into a breadth-first tree but cluster all super-groups
	size_t insertionOffset = 0;
	size_t dequeueOffset = 0;
	for (size_t i = 0; i < projectLabelMap.numSuperGroups; i++) {
		const Data::ProjectLabelMap::SuperGroup &dataSG = projectLabelMap.superGroups[i];
		LabelSuperGroup &sg = _labelSuperGroups[i];

		sg.name = dataSG.name;
		sg.superGroupID = dataSG.id;

		sg.firstRootNodeIndex = insertionOffset;
		sg.numRootNodes = dataSG.numChildren;

		for (size_t j = 0; j < dataSG.numChildren; j++)
			treeQueue[insertionOffset++] = &dataSG.tree[j];

		while (dequeueOffset < insertionOffset) {
			const Data::ProjectLabelMap::LabelTree &dataTree = *treeQueue[dequeueOffset];
			LabelTree &labelTree = _labelTree[dequeueOffset];

			labelTree.id = dataTree.id;
			labelTree.name = dataTree.name;
			dequeueOffset++;

			labelTree.firstChildIndex = insertionOffset;
			labelTree.numChildren = dataTree.numChildren;
			for (size_t j = 0; j < dataTree.numChildren; j++)
				treeQueue[insertionOffset++] = &dataTree.children[j];
		}

		sg.numTotalNodes = insertionOffset - sg.firstRootNodeIndex;
	}

	debug(1, "Loaded %i labels and %i supergroups", static_cast<int>(_labelTree.size()), static_cast<int>(_labelSuperGroups.size()));
}

size_t Project::recursiveCountLabels(const Data::ProjectLabelMap::LabelTree& tree) {
	size_t numLabels = 1;	// For the node itself
	for (size_t i = 0; i < tree.numChildren; i++)
		numLabels += recursiveCountLabels(tree.children[i]);
	return numLabels;
}

ObjectLinkingScope *Project::getPersistentStructuralScope() {
	return &_structuralScope;
}

ObjectLinkingScope *Project::getPersistentModifierScope() {
	return &_modifierScope;
}

void Project::assignAssets(const Common::Array<Common::SharedPtr<Asset> >& assets, const Hacks &hacks) {
	for (Common::Array<Common::SharedPtr<Asset> >::const_iterator it = assets.begin(), itEnd = assets.end(); it != itEnd; ++it) {
		Common::SharedPtr<Asset> asset = *it;
		uint32 assetID = asset->getAssetID();

		if (assetID >= _assetsByID.size()) {
			warning("Bad asset ID %u", assetID);
			continue;
		}

		AssetDesc *desc = _assetsByID[assetID];
		if (desc == nullptr) {
			warning("Asset attempting to use deleted asset slot %u", assetID);
			continue;
		}

		if (desc->asset.expired()) {
			desc->asset = asset;

			for (const Common::SharedPtr<AssetHooks> &hook : hacks.assetHooks)
				hook->onLoaded(asset.get(), desc->name);
		}
	}
}

void Project::initAdditionalSegments(const Common::String &projectName) {
	for (uint segmentIndex = 1; segmentIndex < _segments.size(); segmentIndex++) {
		Segment &segment = _segments[segmentIndex];

		Common::String segmentName = projectName + Common::String::format("%i", static_cast<int>(segmentIndex + 1));

		if (_projectFormat == Data::kProjectFormatNeutral) {
			segmentName += ".mxx";
		} else if (_projectFormat == Data::kProjectFormatWindows) {
			if (_runtimeVersion >= kRuntimeVersion200)
				segmentName += ".mxw";
			else
				segmentName += ".mpx";
		} else if (_projectFormat == Data::kProjectFormatMacintosh) {
			if (_runtimeVersion >= kRuntimeVersion200)
				segmentName += ".mxm";
		}

		// Attempt to find the segment
		segment.desc.filePath = segmentName;
		segment.desc.volumeID = segmentIndex;
	}
}

void Project::loadContextualObject(size_t streamIndex, ChildLoaderStack &stack, const Data::DataObject &dataObject) {
	ChildLoaderContext &topContext = stack.contexts.back();
	const Data::DataObjectTypes::DataObjectType dataObjectType = dataObject.getType();

	// The stack entry must always be popped before loading the object because the load process may descend into more children,
	// such as when behaviors are nested.
	switch (topContext.type) {
	case ChildLoaderContext::kTypeCountedModifierList: {
			IModifierContainer *container = topContext.containerUnion.modifierContainer;

			if ((--topContext.remainingCount) == 0)
				stack.contexts.pop_back();

			ModifierLoaderContext loaderContext(&stack);

			container->appendModifier(loadModifierObject(loaderContext, dataObject));
		} break;
	case ChildLoaderContext::kTypeFlagTerminatedModifierList: {
		IModifierContainer *container = topContext.containerUnion.modifierContainer;

			size_t modifierListContextOffset = stack.contexts.size() - 1;

			ModifierLoaderContext loaderContext(&stack);

			Common::SharedPtr<Modifier> modifier = loadModifierObject(loaderContext, dataObject);

			if (modifier->getModifierFlags().isLastModifier)
				stack.contexts.remove_at(modifierListContextOffset);

			container->appendModifier(modifier);
		} break;
	case ChildLoaderContext::kTypeProject: {
			Structural *project = topContext.containerUnion.structural;

			if (dataObjectType == Data::DataObjectTypes::kSectionStructuralDef) {

				const Data::SectionStructuralDef &sectionObject = static_cast<const Data::SectionStructuralDef &>(dataObject);

				Common::SharedPtr<Structural> section(new Section());
				section->setSelfReference(section);

				if (!static_cast<Section *>(section.get())->load(sectionObject))
					error("Failed to load section");

				project->addChild(section);
				section->setParent(project);

				// For some reason all section objects have the "no more siblings" structural flag.
				// There doesn't appear to be any indication of how many section objects there will
				// be either.
				//if (sectionObject.structuralFlags & Data::StructuralFlags::kNoMoreSiblings)
				//	stack.contexts.pop_back();

				if (sectionObject.structuralFlags & Data::StructuralFlags::kHasChildren) {
					ChildLoaderContext loaderContext;
					loaderContext.containerUnion.structural = section.get();
					loaderContext.remainingCount = 0;
					loaderContext.type = ChildLoaderContext::kTypeSection;

					stack.contexts.push_back(loaderContext);
				}

				if (sectionObject.structuralFlags & Data::StructuralFlags::kHasModifiers) {
					ChildLoaderContext loaderContext;
					loaderContext.containerUnion.modifierContainer = section.get();
					loaderContext.type = ChildLoaderContext::kTypeFlagTerminatedModifierList;

					stack.contexts.push_back(loaderContext);
				}
			} else if (Data::DataObjectTypes::isModifier(dataObjectType)) {
				ModifierLoaderContext loaderContext(&stack);
				project->appendModifier(loadModifierObject(loaderContext, dataObject));
			} else {
				error("Unexpected object type in this context");
			}
		} break;
	case ChildLoaderContext::kTypeSection: {
			Structural *section = topContext.containerUnion.structural;

			if (dataObject.getType() == Data::DataObjectTypes::kSubsectionStructuralDef) {
				const Data::SubsectionStructuralDef &subsectionObject = static_cast<const Data::SubsectionStructuralDef &>(dataObject);

				Common::SharedPtr<Structural> subsection(new Subsection());
				subsection->setSelfReference(subsection);

				if (!static_cast<Subsection *>(subsection.get())->load(subsectionObject))
					error("Failed to load subsection");

				section->addChild(subsection);

				if (subsectionObject.structuralFlags & Data::StructuralFlags::kNoMoreSiblings)
					stack.contexts.pop_back();

				if (subsectionObject.structuralFlags & Data::StructuralFlags::kHasChildren) {
					ChildLoaderContext loaderContext;
					loaderContext.containerUnion.filteredElements.structural = subsection.get();
					loaderContext.containerUnion.filteredElements.filterFunc = Data::DataObjectTypes::isValidSceneRootElement;
					loaderContext.remainingCount = 0;
					loaderContext.type = ChildLoaderContext::kTypeFilteredElements;

					stack.contexts.push_back(loaderContext);
				}

				if (subsectionObject.structuralFlags & Data::StructuralFlags::kHasModifiers) {
					ChildLoaderContext loaderContext;
					loaderContext.containerUnion.modifierContainer = subsection.get();
					loaderContext.type = ChildLoaderContext::kTypeFlagTerminatedModifierList;

					stack.contexts.push_back(loaderContext);
				}
			} else if (Data::DataObjectTypes::isModifier(dataObjectType)) {
				ModifierLoaderContext loaderContext(&stack);
				section->appendModifier(loadModifierObject(loaderContext, dataObject));
			} else {
				error("Unexpected object type in this context");
			}
		} break;
	case ChildLoaderContext::kTypeFilteredElements: {
			Structural *container = topContext.containerUnion.filteredElements.structural;

			if (topContext.containerUnion.filteredElements.filterFunc(dataObjectType)) {
				const Data::StructuralDef &structuralDef = static_cast<const Data::StructuralDef &>(dataObject);

				SIElementFactory *elementFactory = getElementFactoryForDataObjectType(dataObjectType);
				if (!elementFactory) {
					error("No element factory defined for structural object");
				}

				ElementLoaderContext elementLoaderContext(getRuntime(), this, streamIndex);
				Common::SharedPtr<Element> element = elementFactory->createElement(elementLoaderContext, dataObject);

				uint32 guid = element->getStaticGUID();
				const Common::HashMap<uint32, Common::SharedPtr<StructuralHooks> > &hooksMap = getRuntime()->getHacks().structuralHooks;
				Common::HashMap<uint32, Common::SharedPtr<StructuralHooks> >::const_iterator hooksIt = hooksMap.find(guid);
				if (hooksIt == hooksMap.end()) {
					Common::SharedPtr<StructuralHooks> defaultStructuralHooks = getRuntime()->getHacks().defaultStructuralHooks;
					if (defaultStructuralHooks) {
						element->setHooks(defaultStructuralHooks);
						defaultStructuralHooks->onCreate(element.get());
					}
				} else {
					element->setHooks(hooksIt->_value);
					hooksIt->_value->onCreate(element.get());
				}

				container->addChild(element);

				if (structuralDef.structuralFlags & Data::StructuralFlags::kNoMoreSiblings)
					stack.contexts.pop_back();

				if (structuralDef.structuralFlags & Data::StructuralFlags::kHasChildren) {
					ChildLoaderContext loaderContext;
					// Visual elements can contain non-visual element children, but non-visual elements
					// can only contain non-visual element children
					loaderContext.containerUnion.filteredElements.filterFunc = element->isVisual() ? Data::DataObjectTypes::isElement : Data::DataObjectTypes::isNonVisualElement;
					loaderContext.containerUnion.filteredElements.structural = element.get();
					loaderContext.remainingCount = 0;
					loaderContext.type = ChildLoaderContext::kTypeFilteredElements;

					stack.contexts.push_back(loaderContext);
				}

				if (structuralDef.structuralFlags & Data::StructuralFlags::kHasModifiers) {
					ChildLoaderContext loaderContext;
					loaderContext.containerUnion.modifierContainer = element.get();
					loaderContext.type = ChildLoaderContext::kTypeFlagTerminatedModifierList;

					stack.contexts.push_back(loaderContext);
				}
			} else if (Data::DataObjectTypes::isModifier(dataObjectType)) {
				ModifierLoaderContext loaderContext(&stack);
				container->appendModifier(loadModifierObject(loaderContext, dataObject));
			} else {
				error("Unexpected object type in this context");
			}
		} break;
	default:
		error("Tried to load a contextual object outside of a context");
		break;
	}
}

void Project::loadAssetDef(size_t streamIndex, AssetDefLoaderContext& context, const Data::DataObject& dataObject) {
	assert(Data::DataObjectTypes::isAsset(dataObject.getType()));

	SIAssetFactory *factory = getAssetFactoryForDataObjectType(dataObject.getType());
	if (!factory) {
		error("Unimplemented asset type");
		return;
	}

	AssetLoaderContext loaderContext(streamIndex);
	Common::SharedPtr<Asset> asset = factory->createAsset(loaderContext, dataObject);
	if (!asset) {
		warning("An asset failed to load");
		return;
	}
	context.assets.push_back(asset);
}

bool Section::load(const Data::SectionStructuralDef &data) {
	_name = data.name;
	_guid = data.guid;

	return true;
}

bool Section::isSection() const {
	return true;
}

Common::SharedPtr<Structural> Section::shallowClone() const {
	error("Cloning sections is not supported");
	return nullptr;
}

void Section::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	error("Cloning sections is not supported");
}

ObjectLinkingScope *Section::getPersistentStructuralScope() {
	return &_structuralScope;
}

ObjectLinkingScope *Section::getPersistentModifierScope() {
	return &_modifierScope;
}

bool Subsection::load(const Data::SubsectionStructuralDef &data) {
	_name = data.name;
	_guid = data.guid;

	return true;
}

bool Subsection::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	return Structural::readAttribute(thread, result, attrib);
}

bool Subsection::isSubsection() const {
	return true;
}

Common::SharedPtr<Structural> Subsection::shallowClone() const {
	error("Cloning subsections is not supported");
	return nullptr;
}

void Subsection::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	error("Cloning subsections is not supported");
}

ObjectLinkingScope *Subsection::getSceneLoadMaterializeScope() {
	return getPersistentStructuralScope();
}

ObjectLinkingScope *Subsection::getPersistentStructuralScope() {
	return &_structuralScope;
}

ObjectLinkingScope *Subsection::getPersistentModifierScope() {
	return &_modifierScope;
}

Element::Element() : _streamLocator(0), _sectionID(0), _haveCheckedAutoPlay(false) {
}

Element::Element(const Element &other)
	: Structural(other), _streamLocator(other._streamLocator), _sectionID(other._sectionID)
	// Don't copy checked autoplay or mediacues lists
	, _haveCheckedAutoPlay(false), _mediaCues() {
}

bool Element::canAutoPlay() const {
	return false;
}

void Element::queueAutoPlayEvents(Runtime *runtime, bool isAutoPlaying) {
	if (isAutoPlaying) {
		Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kPlay, 0), DynamicValue(), getSelfReference()));
		Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, false, true));
		runtime->queueMessage(dispatch);
	}
}

bool Element::isElement() const {
	return true;
}

uint32 Element::getStreamLocator() const {
	return _streamLocator;
}

void Element::addMediaCue(MediaCueState *mediaCue) {
	_mediaCues.push_back(mediaCue);
}

void Element::removeMediaCue(const MediaCueState *mediaCue) {
	for (size_t i = 0; i < _mediaCues.size(); i++) {
		if (_mediaCues[i] == mediaCue) {
			_mediaCues.remove_at(i);
			break;
		}
	}
}

void Element::triggerAutoPlay(Runtime *runtime) {
	if (_haveCheckedAutoPlay)
		return;

	_haveCheckedAutoPlay = true;

	queueAutoPlayEvents(runtime, canAutoPlay());
}

void Element::tryAutoSetName(Runtime *runtime, Project *project) {
}

bool Element::resolveMediaMarkerLabel(const Label& label, int32 &outResolution) const {
	return false;
}

void Element::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	Structural::visitInternalReferences(visitor);
}

VisualElementTransitionProperties::VisualElementTransitionProperties() : _isDirty(true), _alpha(255) {
}

uint8 VisualElementTransitionProperties::getAlpha() const {
	return _alpha;
}

void VisualElementTransitionProperties::setAlpha(uint8 alpha) {
	_isDirty = true;
	_alpha = alpha;
}

bool VisualElementTransitionProperties::isDirty() const {
	return _isDirty;
}

void VisualElementTransitionProperties::clearDirty() {
	_isDirty = false;
}

VisualElementRenderProperties::VisualElementRenderProperties()
	: _inkMode(kInkModeDefault), _shape(kShapeRect), _foreColor(0, 0, 0), _backColor(255, 255, 255),
	  _borderColor(0, 0, 0), _shadowColor(0, 0, 0), _borderSize(0), _shadowSize(0), _isDirty(true) {
}

VisualElementRenderProperties::InkMode VisualElementRenderProperties::getInkMode() const {
	return _inkMode;
}

void VisualElementRenderProperties::setInkMode(InkMode inkMode) {
	_isDirty = true;
	_inkMode = inkMode;
}

void VisualElementRenderProperties::setShape(Shape shape) {
	_isDirty = true;
	_shape = shape;
}

VisualElementRenderProperties::Shape VisualElementRenderProperties::getShape() const {
	return _shape;
}

const ColorRGB8 &VisualElementRenderProperties::getForeColor() const {
	return _foreColor;
}

void VisualElementRenderProperties::setForeColor(const ColorRGB8 &color) {
	_isDirty = true;
	_foreColor = color;
}

const ColorRGB8 &VisualElementRenderProperties::getBackColor() const {
	return _backColor;
}

void VisualElementRenderProperties::setBackColor(const ColorRGB8 &color) {
	_isDirty = true;
	_backColor = color;
}

const ColorRGB8 &VisualElementRenderProperties::getBorderColor() const {
	return _borderColor;
}

void VisualElementRenderProperties::setBorderColor(const ColorRGB8 &color) {
	_isDirty = true;
	_borderColor = color;
}

const ColorRGB8 &VisualElementRenderProperties::getShadowColor() const {
	return _shadowColor;
}

void VisualElementRenderProperties::setShadowColor(const ColorRGB8 &color) {
	_isDirty = true;
	_shadowColor = color;
}

uint16 VisualElementRenderProperties::getBorderSize() const {
	return _borderSize;
}

void VisualElementRenderProperties::setBorderSize(uint16 size) {
	_isDirty = true;
	_borderSize = size;
}

uint16 VisualElementRenderProperties::getShadowSize() const {
	return _shadowSize;
}

void VisualElementRenderProperties::setShadowSize(uint16 size) {
	_isDirty = true;
	_shadowSize = size;
}

const Common::Array<Common::Point> &VisualElementRenderProperties::getPolyPoints() const {
	return _polyPoints;
}

Common::Array<Common::Point> &VisualElementRenderProperties::modifyPolyPoints() {
	_isDirty = true;
	return _polyPoints;
}

bool VisualElementRenderProperties::isDirty() const {
	return _isDirty;
}

void VisualElementRenderProperties::clearDirty() {
	_isDirty = false;
}

VisualElementRenderProperties &VisualElementRenderProperties::operator=(const VisualElementRenderProperties &other) {
	_inkMode = other._inkMode;
	_shape = other._shape;
	_foreColor = other._foreColor;
	_backColor = other._backColor;
	_borderSize = other._borderSize;
	_borderColor = other._borderColor;
	_shadowSize = other._shadowSize;
	_shadowColor = other._shadowColor;

	_polyPoints = other._polyPoints;

	_isDirty = true;

	return *this;
}

VisualElement::VisualElement()
	: _rect(0, 0, 0, 0), _cachedAbsoluteOrigin(Common::Point(0, 0)), _contentsDirty(true), _directToScreen(false), _visible(false), _visibleByDefault(true), _layer(0),
	  _topLeftBevelShading(0), _bottomRightBevelShading(0), _interiorShading(0), _bevelSize(0) {
}

VisualElement::VisualElement(const VisualElement &other)
	: Element(other), _directToScreen(other._directToScreen), _visible(other._visible), _visibleByDefault(other._visibleByDefault)
	, _rect(other._rect), _cachedAbsoluteOrigin(other._cachedAbsoluteOrigin), _layer(other._layer), _topLeftBevelShading(other._topLeftBevelShading)
	, _bottomRightBevelShading(other._bottomRightBevelShading), _interiorShading(other._interiorShading), _bevelSize(other._bevelSize)
	, _dragProps(nullptr), _renderProps(other._renderProps), _primaryGraphicModifier(nullptr), _transitionProps(other._transitionProps)
	, _palette(other._palette), _prevRect(other._prevRect), _contentsDirty(true) {
}

bool VisualElement::isVisual() const {
	return true;
}

bool VisualElement::isTextLabel() const {
	return false;
}

bool VisualElement::isVisible() const {
	return _visible;
}

bool VisualElement::isVisibleByDefault() const {
	return _visibleByDefault;
}

void VisualElement::setVisible(Runtime *runtime, bool visible) {
	if (_visible != visible) {
		runtime->setSceneGraphDirty();
		_visible = visible;
	}
}

bool VisualElement::isDirectToScreen() const {
	return _directToScreen;
}

void VisualElement::setDirectToScreen(bool directToScreen) {
	if (_directToScreen != directToScreen) {
		_contentsDirty = true;
		_directToScreen = directToScreen;
	}
}

uint16 VisualElement::getLayer() const {
	return _layer;
}

void VisualElement::setLayer(uint16 layer) {
	if (_layer != layer) {
		_contentsDirty = true;
		_layer = layer;
	}
}

CORO_BEGIN_DEFINITION(VisualElement::VisualElementConsumeCommandCoroutine)
	struct Locals {
	};

	CORO_BEGIN_FUNCTION
		CORO_IF(Event(EventIDs::kElementShow, 0).respondsTo(params->msg->getEvent()))
			if (!params->self->_visible) {
				params->self->_visible = true;
				params->runtime->setSceneGraphDirty();
			}

			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kElementShow, 0), DynamicValue(), params->self->getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, params->self, false, true, false));

			CORO_CALL(Runtime::SendMessageOnVThreadCoroutine, params->runtime, dispatch);

			CORO_RETURN;
		CORO_ELSE_IF(Event(EventIDs::kElementHide, 0).respondsTo(params->msg->getEvent()))
			if (params->self->_visible) {
				params->self->_visible = false;

				if (params->self->_hooks)
					params->self->_hooks->onHidden(params->self, params->self->_visible);

				params->runtime->setSceneGraphDirty();
			}

			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kElementHide, 0), DynamicValue(), params->self->getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, params->self, false, true, false));

			CORO_CALL(Runtime::SendMessageOnVThreadCoroutine, params->runtime, dispatch);

			CORO_RETURN;
		CORO_END_IF

		CORO_CALL(Element::ElementConsumeCommandCoroutine, params->self, params->runtime, params->msg);
	CORO_END_FUNCTION
CORO_END_DEFINITION

VThreadState VisualElement::asyncConsumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	runtime->getVThread().pushCoroutine<VisualElementConsumeCommandCoroutine>(this, runtime, msg);
	return kVThreadReturn;
}

bool VisualElement::respondsToEvent(const Event &evt) const {
	if (Event(EventIDs::kAuthorMessage, 13).respondsTo(evt)) {
		if (getRuntime()->getHacks().mtiSceneReturnHack && getParent() && getParent()->isSubsection())
			return true;
	}

	return Element::respondsToEvent(evt);
}

VThreadState VisualElement::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (Event(EventIDs::kAuthorMessage, 13).respondsTo(msg->getEvent())) {
		if (getRuntime()->getHacks().mtiSceneReturnHack) {
			if (getParent() && getParent()->isSubsection()) {
				runtime->addSceneStateTransition(HighLevelSceneTransition(this->getSelfReference().lock().staticCast<Structural>(), HighLevelSceneTransition::kTypeChangeToScene, false, false));

				return kVThreadReturn;
			}
		}
	}

	return Element::consumeMessage(runtime, msg);
}

bool VisualElement::isMouseInsideDrawableArea(int32 relativeX, int32 relativeY) const {
	if (relativeX < _rect.left || relativeX >= _rect.right || relativeY < _rect.top || relativeY >= _rect.bottom)
		return false;

	// NOTE: This is actually incomplete, graphic modifiers are supposed to mask out the drawable area for non-rect
	// shapes, so what we SHOULD be doing here is generating a mask and hoisting the mask gen code out of
	// GraphicModifier to more common code, and check the mask here.
	//
	// For now, we just use this for click detection.
	relativeX -= _rect.left;
	relativeY -= _rect.top;

	switch (_renderProps.getShape()) {
	case VisualElementRenderProperties::kShapePolygon:
	case VisualElementRenderProperties::kShapeStar: {
		Common::Point starPoints[10];
			const Common::Point *polyPoints = nullptr;
			size_t numPolyPoints = 0;

			if (_renderProps.getShape() == VisualElementRenderProperties::kShapeStar) {
				int16 width = _rect.width();
				int16 height = _rect.height();
				starPoints[0] = Common::Point(width / 2, 0);
				starPoints[1] = Common::Point(width * 2 / 3, height / 3);
				starPoints[2] = Common::Point(width, height / 3);
				starPoints[3] = Common::Point(width * 3 / 4, height / 2);
				starPoints[4] = Common::Point(width, height);
				starPoints[5] = Common::Point(width / 2, height * 2 / 3);
				starPoints[6] = Common::Point(0, height);
				starPoints[7] = Common::Point(width / 4, height / 2);
				starPoints[8] = Common::Point(0, height / 3);
				starPoints[9] = Common::Point(width / 3, height / 3);
				polyPoints = starPoints;
				numPolyPoints = 10;
			} else {
				numPolyPoints = _renderProps.getPolyPoints().size();
				if (numPolyPoints > 0)
					polyPoints = &_renderProps.getPolyPoints()[0];
				else
					return false;
			}

			bool insideMask = false;
			for (size_t edgeIndex = 2; edgeIndex < numPolyPoints; edgeIndex++) {
				const Common::Point *points[3] = {&polyPoints[0], &polyPoints[edgeIndex - 1], &polyPoints[edgeIndex]};

				int32 rays[3][2];
				int32 normals[3][2];

				for (int i = 0; i < 3; i++) {
					const Common::Point *nextPoint = points[(i + 1) % 3];
					rays[i][0] = nextPoint->x - points[i]->x;
					rays[i][1] = nextPoint->y - points[i]->y;

					normals[i][0] = -rays[i][1];
					normals[i][1] = rays[i][0];
				}

				int32 cDist = rays[1][0] * normals[0][0] + rays[1][1] * normals[0][1];
				if (cDist == 0)
					continue;	// Degenerate triangle

				if (cDist < 0) {
					// Counter-clockwise triangle, flip normals
					for (int i = 0; i < 3; i++) {
						normals[i][0] = -normals[i][0];
						normals[i][1] = -normals[i][1];
					}
				}

				bool inFrontOfAll = true;
				for (int i = 0; i < 3; i++) {
					int32 nx = normals[i][0];
					int32 ny = normals[i][1];
					int32 dist = (relativeX - points[i]->x) * nx + (relativeY - points[i]->y) * ny;
					bool isInFront = (dist > 0);
					if (dist == 0) {
						if (nx != 0)
							isInFront = (nx >= 0);
						else
							isInFront = (ny >= 0);
					}

					if (!isInFront) {
						inFrontOfAll = false;
						break;
					}
				}

				if (inFrontOfAll)
					insideMask = !insideMask;
			}

			return insideMask;
		} break;
	case VisualElementRenderProperties::kShapeRect:
		// Rect is always in collision if inside of the rect
		return true;

	case VisualElementRenderProperties::kShapeOval: {
			int32 w = _rect.width();
			int32 h = _rect.height();

			int32 dcx = relativeX * 2 - w;
			int32 dcy = relativeY * 2 - h;
			dcx *= h;
			dcy *= w;

			int32 expandedRadius = w * h;

			return (dcx * dcx + dcy * dcy <= expandedRadius * expandedRadius);
		} break;

	case VisualElementRenderProperties::kShapeRoundedRect:
		// Rounded rect corners are 13x13 at maximum

	default:
		warning("Unsupported shape type for checking mouse collision");
		return false;
	};

	return true;
}

bool VisualElement::isMouseCollisionAtPoint(int32 relativeX, int32 relativeY) const {
	return true;
}

bool VisualElement::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "visible") {
		result.setBool(_visible);
		return true;
	} else if (attrib == "direct") {
		result.setBool(_directToScreen);
		return true;
	} else if (attrib == "position") {
		result.setPoint(Common::Point(_rect.left, _rect.top));
		return true;
	} else if (attrib == "centerposition") {
		result.setPoint(getCenterPosition());
		return true;
	} else if (attrib == "size") {
		result.setPoint(Common::Point(_rect.width(), _rect.height()));
		return true;
	} else if (attrib == "width") {
		result.setInt(_rect.right - _rect.left);
		return true;
	} else if (attrib == "height") {
		result.setInt(_rect.bottom - _rect.top);
		return true;
	} else if (attrib == "globalposition") {
		result.setPoint(getGlobalPosition());
		return true;
	} else if (attrib == "layer") {
		result.setInt(_layer);
		return true;
	}

	return Element::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome VisualElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	if (attrib == "visible") {
		DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetVisibility, true>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "direct") {
		DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetDirect, true>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "position") {
		DynamicValueWriteOrRefAttribFuncHelper<VisualElement, &VisualElement::scriptSetPosition, &VisualElement::scriptWriteRefPositionAttribute>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "size") {
		DynamicValueWriteOrRefAttribFuncHelper<VisualElement, &VisualElement::scriptSetSize, &VisualElement::scriptWriteRefSizeAttribute>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "centerposition") {
		DynamicValueWriteOrRefAttribFuncHelper<VisualElement, &VisualElement::scriptSetCenterPosition, &VisualElement::scriptWriteRefCenterPositionAttribute>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "width") {
		DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetWidth, true>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "height") {
		DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetHeight, true>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "layer") {
		DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetLayer, true>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "invalidaterect") {
		// Not sure what this does, MTI uses it frequently
		DynamicValueWriteDiscardHelper::create(writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return Element::writeRefAttribute(thread, writeProxy, attrib);
}

const Common::Rect &VisualElement::getRelativeRect() const {
	return _rect;
}

Common::Rect VisualElement::getRelativeCollisionRect() const {
	return getRelativeRect();
}

void VisualElement::setRelativeRect(const Common::Rect &rect) {
	_rect = rect;
}

Common::Point VisualElement::getParentOrigin() const {
	Common::Point pos = Common::Point(0, 0);
	if (_parent && _parent->isElement()) {
		Element *element = static_cast<Element *>(_parent);
		if (element->isVisual()) {
			pos = static_cast<VisualElement *>(element)->getGlobalPosition();
		}
	}

	return pos;
}

Common::Point VisualElement::getGlobalPosition() const {
	Common::Point pos = getParentOrigin();

	pos.x += _rect.left;
	pos.y += _rect.top;

	return pos;
}

const Common::Point &VisualElement::getCachedAbsoluteOrigin() const {
	return _cachedAbsoluteOrigin;
}

void VisualElement::setCachedAbsoluteOrigin(const Common::Point &absOrigin) {
	_cachedAbsoluteOrigin = absOrigin;
}

void VisualElement::setDragMotionProperties(const Common::SharedPtr<DragMotionProperties> &dragProps) {
	_dragProps = dragProps;
}

const Common::SharedPtr<DragMotionProperties> &VisualElement::getDragMotionProperties() const {
	return _dragProps;
}

void VisualElement::handleDragMotion(Runtime *runtime, const Common::Point &initialPoint, const Common::Point &targetPointRef) {
	if (!_dragProps)
		return;

	Common::Point targetPoint = targetPointRef;

	// NOTE: Constraints do not override insets if the object is out of bounds
	if (_dragProps->constraintDirection == kConstraintDirectionHorizontal)
		targetPoint.y = initialPoint.y;
	if (_dragProps->constraintDirection == kConstraintDirectionVertical)
		targetPoint.x = initialPoint.x;

	if (_dragProps->constrainToParent && _parent && _parent->isElement() && static_cast<Element *>(_parent)->isVisual()) {
		Common::Rect constrainInset = _dragProps->constraintMargin;

		Common::Rect parentRect = static_cast<VisualElement *>(_parent)->getRelativeRect();

		// rect.width - inset.right
		int32 minX = constrainInset.left;
		int32 minY = constrainInset.top;
		int32 maxX = parentRect.width() - constrainInset.right - _rect.width();
		int32 maxY = parentRect.height() - constrainInset.bottom - _rect.height();

		// TODO: Handle "squished" case where max < min, it does work but it's weird
		if (targetPoint.x < minX)
			targetPoint.x = minX;

		if (targetPoint.y < minY)
			targetPoint.y = minY;

		if (targetPoint.x > maxX)
			targetPoint.x = maxX;

		if (targetPoint.y > maxY)
			targetPoint.y = maxY;

		if (_hooks)
			_hooks->onSetPosition(runtime, this, Common::Point(_rect.left, _rect.top), targetPoint);

		offsetTranslate(targetPoint.x - _rect.left, targetPoint.y - _rect.top, false);
	}
}

VThreadState VisualElement::offsetTranslateTask(const OffsetTranslateTaskData &data) {
	offsetTranslate(data.dx, data.dy, false);
	return kVThreadReturn;
}

void VisualElement::setTransitionProperties(const VisualElementTransitionProperties &props) {
	_transitionProps = props;
}

const VisualElementTransitionProperties &VisualElement::getTransitionProperties() const {
	return _transitionProps;
}

void VisualElement::setRenderProperties(const VisualElementRenderProperties &props, const Common::WeakPtr<GraphicModifier> &primaryGraphicModifier) {
	_renderProps = props;
	_primaryGraphicModifier = primaryGraphicModifier;
}

const VisualElementRenderProperties &VisualElement::getRenderProperties() const {
	return _renderProps;
}

const Common::WeakPtr<GraphicModifier> &VisualElement::getPrimaryGraphicModifier() const {
	return _primaryGraphicModifier;
}

void VisualElement::setShading(int16 topLeftBevelShading, int16 bottomRightBevelShading, int16 interiorShading, uint32 bevelSize) {
	if (_topLeftBevelShading != topLeftBevelShading || _bottomRightBevelShading != bottomRightBevelShading || _interiorShading != interiorShading || _bevelSize != bevelSize) {
		_topLeftBevelShading = topLeftBevelShading;
		_bottomRightBevelShading = bottomRightBevelShading;
		_interiorShading = interiorShading;
		_bevelSize = bevelSize;

		_contentsDirty = true;
	}
}

bool VisualElement::needsRender() const {
	if (_renderProps.isDirty() || _prevRect != _rect || _contentsDirty)
		return true;

	return false;
}

void VisualElement::finalizeRender() {
	_renderProps.clearDirty();
	_prevRect = _rect;
	_contentsDirty = false;
}

void VisualElement::setPalette(const Common::SharedPtr<Palette> &palette) {
	_palette = palette;
	_contentsDirty = true;
}

const Common::SharedPtr<Palette> &VisualElement::getPalette() const {
	return _palette;
}

void VisualElement::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	Element::visitInternalReferences(visitor);
}


void VisualElement::pushVisibilityChangeTask(Runtime * runtime, bool desiredVisibility) {
	ChangeFlagTaskData *changeVisibilityTask = runtime->getVThread().pushTask("VisualElement::changeVisibilityTask", this, &VisualElement::changeVisibilityTask);
	changeVisibilityTask->desiredFlag = true;
	changeVisibilityTask->runtime = runtime;
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void VisualElement::debugInspect(IDebugInspectionReport *report) const {
	report->declareDynamic("layer", Common::String::format("%i", static_cast<int>(_layer)));
	report->declareDynamic("relRect", Common::String::format("(%i,%i)-(%i,%i)", static_cast<int>(_rect.left), static_cast<int>(_rect.top), static_cast<int>(_rect.right), static_cast<int>(_rect.bottom)));
	report->declareDynamic("directToScreen", Common::String(_directToScreen ? "true" : "false"));
	report->declareDynamic("visible", Common::String(_visible ? "true" : "false"));

	Element::debugInspect(report);
}
#endif

MiniscriptInstructionOutcome VisualElement::scriptSetVisibility(MiniscriptThread *thread, const DynamicValue &result) {
	// FIXME: Need to make this fire Show/Hide events??
	if (result.getType() == DynamicValueTypes::kBoolean) {
		const bool targetValue = result.getBool();

		// Weird quirk: The element's visible flag reads as "false" until initial Show events fire, but
		// setting "visible" here prevents it from showing.  This is necessary for the vidbots in Obsidian's
		// bureau area, to make them appear turned off initially.
		_visibleByDefault = targetValue;

		if (_visible != targetValue) {
			_visible = targetValue;
			thread->getRuntime()->setSceneGraphDirty();
		}
		return kMiniscriptInstructionOutcomeContinue;
	}

	return kMiniscriptInstructionOutcomeFailed;
}

bool VisualElement::loadCommon(const Common::String &name, uint32 guid, const Data::Rect &rect, uint32 elementFlags, uint16 layer, uint32 streamLocator, uint16 sectionID) {
	if (!rect.toScummVMRect(_rect))
		return false;

	_name = name;
	_guid = guid;
	_visibleByDefault = ((elementFlags & Data::ElementFlags::kHidden) == 0);	// Element isn't actually flagged as visible until after Scene Changed, when Show commands are fired
	_directToScreen = ((elementFlags & Data::ElementFlags::kNotDirectToScreen) == 0);
	_streamLocator = streamLocator;
	_sectionID = sectionID;
	_layer = layer;

	return true;
}

MiniscriptInstructionOutcome VisualElement::scriptSetDirect(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kBoolean) {
		_directToScreen = value.getBool();
		return kMiniscriptInstructionOutcomeContinue;
	}
	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome VisualElement::scriptSetPosition(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kPoint) {
		Common::Point destPoint = value.getPoint();

		if (_hooks)
			_hooks->onSetPosition(thread->getRuntime(), this, Common::Point(_rect.left, _rect.top), destPoint);

		int32 xDelta = destPoint.x - _rect.left;
		int32 yDelta = destPoint.y - _rect.top;

		if (xDelta != 0 || yDelta != 0)
			offsetTranslate(xDelta, yDelta, false);

		return kMiniscriptInstructionOutcomeContinue;
	}

	// Assigning non-point values to position silently fails
	// Obsidian relies on this behavior due to a bug in the air puzzle completion script
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome VisualElement::scriptSetPositionX(MiniscriptThread *thread, const DynamicValue &dest) {
	int32 asInteger = 0;
	if (!dest.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	Common::Point updatedPoint = Common::Point(asInteger, _rect.top);
	if (_hooks)
		_hooks->onSetPosition(thread->getRuntime(), this, Common::Point(_rect.left, _rect.top), updatedPoint);
	int32 xDelta = updatedPoint.x - _rect.left;
	int32 yDelta = updatedPoint.y - _rect.top;

	if (xDelta != 0 || yDelta != 0)
		offsetTranslate(xDelta, yDelta, false);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome VisualElement::scriptSetPositionY(MiniscriptThread *thread, const DynamicValue &dest) {
	int32 asInteger = 0;
	if (!dest.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	Common::Point updatedPoint = Common::Point(_rect.left, asInteger);
	if (_hooks)
		_hooks->onSetPosition(thread->getRuntime(), this, Common::Point(_rect.left, _rect.top), updatedPoint);

	int32 xDelta = updatedPoint.x - _rect.left;
	int32 yDelta = updatedPoint.y - _rect.top;

	if (xDelta != 0 || yDelta != 0)
		offsetTranslate(xDelta, yDelta, false);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome VisualElement::scriptSetCenterPosition(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kPoint) {
		const Common::Point destPoint = value.getPoint();
		const Common::Point &srcPoint = getCenterPosition();
		int32 xDelta = destPoint.x - srcPoint.x;
		int32 yDelta = destPoint.y - srcPoint.y;

		if (xDelta != 0 || yDelta != 0)
			offsetTranslate(xDelta, yDelta, false);

		return kMiniscriptInstructionOutcomeContinue;
	}
	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome VisualElement::scriptSetCenterPositionX(MiniscriptThread *thread, const DynamicValue &dest) {
	int32 asInteger = 0;
	if (!dest.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	int32 xDelta = asInteger - getCenterPosition().x;

	if (xDelta != 0)
		offsetTranslate(xDelta, 0, false);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome VisualElement::scriptSetCenterPositionY(MiniscriptThread *thread, const DynamicValue &dest) {
	int32 asInteger = 0;
	if (!dest.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	int32 yDelta = asInteger - getCenterPosition().y;

	if (yDelta != 0)
		offsetTranslate(0, yDelta, false);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome VisualElement::scriptSetSize(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (value.getType() == DynamicValueTypes::kPoint) {
		Common::Point pt = value.getPoint();

		if (_rect.bottom - _rect.top != asInteger || _rect.right - _rect.left != asInteger) {
			_rect.right = _rect.left + pt.x;
			_rect.bottom = _rect.top + pt.y;

			thread->getRuntime()->setSceneGraphDirty();
		}
	} else {
#ifdef MTROPOLIS_DEBUG_ENABLE
		if (Debugger *debugger = thread->getRuntime()->debugGetDebugger())
			debugger->notify(kDebugSeverityError, "'size' value wasn't a point");
#endif
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome VisualElement::scriptSetWidth(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	if (_rect.right - _rect.left != asInteger) {
		_rect.right = _rect.left + asInteger;

		thread->getRuntime()->setSceneGraphDirty();
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome VisualElement::scriptSetHeight(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	if (_rect.bottom - _rect.top != asInteger) {
		_rect.bottom = _rect.top + asInteger;

		thread->getRuntime()->setSceneGraphDirty();
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome VisualElement::scriptSetLayer(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	if (asInteger != _layer) {
		VisualElement *scene = findScene();

		// If a layer is assigned and a colliding element exists, then the layers are swapped
		if (scene) {
			VisualElement *collision = recursiveFindItemWithLayer(scene, asInteger);
			if (collision)
				collision->_layer = _layer;
		}
		_layer = asInteger;

		thread->getRuntime()->setSceneGraphDirty();
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome VisualElement::scriptWriteRefPositionAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	if (attrib == "x") {
		DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetPositionX, true>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "y") {
		DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetPositionY, true>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome VisualElement::scriptWriteRefCenterPositionAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	if (attrib == "x") {
		DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetCenterPositionX, true>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "y") {
		DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetCenterPositionY, true>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome VisualElement::scriptWriteRefSizeAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	if (attrib == "x") {
		DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetWidth, true>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "y") {
		DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetHeight, true>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return kMiniscriptInstructionOutcomeFailed;
}

void VisualElement::offsetTranslate(int32 xDelta, int32 yDelta, bool cachedOriginOnly) {
	if (!cachedOriginOnly) {
		_rect.left += xDelta;
		_rect.right += xDelta;
		_rect.top += yDelta;
		_rect.bottom += yDelta;
	}

	_cachedAbsoluteOrigin.x += xDelta;
	_cachedAbsoluteOrigin.y += yDelta;

	for (const Common::SharedPtr<Structural> &child : _children) {
		if (child->isElement()) {
			Element *element = static_cast<Element *>(child.get());
			if (element->isVisual())
				static_cast<VisualElement *>(element)->offsetTranslate(xDelta, yDelta, true);
		}
	}

	if (xDelta != 0 || yDelta != 0)
		_contentsDirty = true;
}

Common::Point VisualElement::getCenterPosition() const {
	return Common::Point((_rect.left + _rect.right) / 2, (_rect.top + _rect.bottom) / 2);
}

CORO_BEGIN_DEFINITION(VisualElement::ChangeVisibilityCoroutine)
	struct Locals {
	};

	CORO_BEGIN_FUNCTION
		CORO_IF(params->self->_visible != params->desiredFlag)
			params->self->setVisible(params->runtime, params->desiredFlag);

			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(params->desiredFlag ? EventIDs::kElementShow : EventIDs::kElementHide, 0), DynamicValue(), params->self->getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, params->self, false, true, false));

			CORO_CALL(Runtime::SendMessageOnVThreadCoroutine, params->runtime, dispatch);
		CORO_END_IF
	CORO_END_FUNCTION
CORO_END_DEFINITION

VThreadState VisualElement::changeVisibilityTask(const ChangeFlagTaskData &taskData) {
	if (_visible != taskData.desiredFlag) {
		setVisible(taskData.runtime, taskData.desiredFlag);

		Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(taskData.desiredFlag ? EventIDs::kElementShow : EventIDs::kElementHide, 0), DynamicValue(), getSelfReference()));
		Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
		taskData.runtime->sendMessageOnVThread(dispatch);
	}

	return kVThreadReturn;
}

VisualElement *VisualElement::recursiveFindItemWithLayer(VisualElement *element, int32 layer) {
	if (element->_layer == layer)
		return element;

	for (const Common::SharedPtr<Structural> &child : element->getChildren()) {
		if (child->isElement()) {
			Element *childElement = static_cast<Element *>(child.get());
			if (childElement->isVisual()) {
				VisualElement *result = recursiveFindItemWithLayer(static_cast<VisualElement *>(childElement), layer);
				if (result)
					return result;
			}
		}
	}

	return nullptr;
}

void VisualElement::renderShading(Graphics::Surface &surf) const {
	uint32 bevelSize = _bevelSize;
	uint32 w = surf.w;
	uint32 h = surf.h;

	uint32 maxHBevel = (w + 1) / 2;
	uint32 maxVBevel = (h + 1) / 2;

	if (bevelSize > maxHBevel)
		bevelSize = maxHBevel;
	if (bevelSize > maxVBevel)
		bevelSize = maxVBevel;

	uint32 rMask = surf.format.ARGBToColor(0, 255, 0, 0);
	uint32 gMask = surf.format.ARGBToColor(0, 0, 255, 0);
	uint32 bMask = surf.format.ARGBToColor(0, 0, 0, 255);

	byte bytesPerPixel = surf.format.bytesPerPixel;

	if (_topLeftBevelShading != 0) {
		bool isBrighten = (_topLeftBevelShading > 0);

		uint32 rAdd = quantizeShading(rMask, _topLeftBevelShading);
		uint32 gAdd = quantizeShading(gMask, _topLeftBevelShading);
		uint32 bAdd = quantizeShading(bMask, _topLeftBevelShading);

		// Top bar
		for (uint y = 0; y < bevelSize; y++)
			renderShadingScanlineDynamic(surf.getBasePtr(0, y), w - y, rMask, rAdd, gMask, gAdd, bMask, bAdd, isBrighten, bytesPerPixel);

		// Left bar
		uint leftBarEndY = h + 1 - bevelSize;
		for (uint y = bevelSize; y < leftBarEndY; y++)
			renderShadingScanlineDynamic(surf.getBasePtr(0, y), bevelSize, rMask, rAdd, gMask, gAdd, bMask, bAdd, isBrighten, bytesPerPixel);

		// Lower diagonal
		for (uint y = leftBarEndY; y < h; y++)
			renderShadingScanlineDynamic(surf.getBasePtr(0, y), bevelSize - 1 - (y - leftBarEndY), rMask, rAdd, gMask, gAdd, bMask, bAdd, isBrighten, bytesPerPixel);
	}

	if (_bottomRightBevelShading != 0) {
		bool isBrighten = (_bottomRightBevelShading > 0);

		uint32 rAdd = quantizeShading(rMask, _bottomRightBevelShading);
		uint32 gAdd = quantizeShading(gMask, _bottomRightBevelShading);
		uint32 bAdd = quantizeShading(bMask, _bottomRightBevelShading);

		// Upper diagonal
		for (uint y = 1; y < bevelSize; y++)
			renderShadingScanlineDynamic(surf.getBasePtr(w - y, y), y, rMask, rAdd, gMask, gAdd, bMask, bAdd, isBrighten, bytesPerPixel);

		uint rightBarEndY = h - bevelSize;
		if (rightBarEndY < bevelSize)
			rightBarEndY = bevelSize;

		uint rightBarX = w - bevelSize;
		if (rightBarX < bevelSize)
			rightBarX = bevelSize;

		// Right bar
		for (uint y = bevelSize; y < rightBarEndY; y++)
			renderShadingScanlineDynamic(surf.getBasePtr(rightBarX, y), w - rightBarX, rMask, rAdd, gMask, gAdd, bMask, bAdd, isBrighten, bytesPerPixel);

		// Bottom bar
		for (uint y = rightBarEndY; y < h; y++) {
			uint bottomBarStartX = bevelSize - (y - rightBarEndY);
			renderShadingScanlineDynamic(surf.getBasePtr(bottomBarStartX, y), w - bottomBarStartX, rMask, rAdd, gMask, gAdd, bMask, bAdd, isBrighten, bytesPerPixel);
		}
	}

	if (_interiorShading != 0) {
		uint32 startX = bevelSize;
		uint32 endX = w - bevelSize;
		uint32 startY = bevelSize;
		uint32 endY = h - bevelSize;

		if (startX < endX && startY < endY) {
			bool isBrighten = (_bottomRightBevelShading > 0);

			uint32 rAdd = quantizeShading(rMask, _bottomRightBevelShading);
			uint32 gAdd = quantizeShading(gMask, _bottomRightBevelShading);
			uint32 bAdd = quantizeShading(bMask, _bottomRightBevelShading);

			for (uint y = startY; y < endY; y++)
				renderShadingScanlineDynamic(surf.getBasePtr(startX, y), endX - startX, rMask, rAdd, gMask, gAdd, bMask, bAdd, isBrighten, bytesPerPixel);
		}
	}
}

uint32 VisualElement::quantizeShading(uint32 mask, int16 shading) {
	uint32 absShading = (shading < 0) ? static_cast<int32>(-shading) : static_cast<int32>(shading);

	if ((mask & 0xff) == 0) {
		// Nothing in the low bytes, so shift down to avoid upper bits overflow
		return ((mask >> 8) * absShading) & mask;
	}

	// Something was in the low bits, so avoid lower bits underflow instead
	return ((mask * absShading) >> 8) & mask;
}

void VisualElement::renderShadingScanlineDynamic(void *data, size_t numElements, uint32 rMask, uint32 rAdd, uint32 gMask, uint32 gAdd, uint32 bMask, uint32 bAdd, bool isBrighten, byte bytesPerPixel) {
	if (isBrighten) {
		switch (bytesPerPixel) {
		case 2:
			renderBrightenScanline<uint16>(static_cast<uint16 *>(data), numElements, rMask, rAdd, gMask, gAdd, bMask, bAdd);
			break;
		case 4:
			renderBrightenScanline<uint32>(static_cast<uint32 *>(data), numElements, rMask, rAdd, gMask, gAdd, bMask, bAdd);
			break;
		default:
			break;
		}
	} else {
		switch (bytesPerPixel) {
		case 2:
			renderDarkenScanline<uint16>(static_cast<uint16 *>(data), numElements, rMask, rAdd, gMask, gAdd, bMask, bAdd);
			break;
		case 4:
			renderDarkenScanline<uint32>(static_cast<uint32 *>(data), numElements, rMask, rAdd, gMask, gAdd, bMask, bAdd);
			break;
		default:
			break;
		}
	}
}

template<class TElement>
void VisualElement::renderBrightenScanline(TElement *element, size_t numElements, TElement rMask, TElement rAdd, TElement gMask, TElement gAdd, TElement bMask, TElement bAdd) {
	TElement rLimit = rMask - rAdd;
	TElement gLimit = gMask - gAdd;
	TElement bLimit = bMask - bAdd;

	while (numElements > 0) {
		TElement v = *element;

		if ((v & rMask) > rLimit)
			v |= rMask;
		else
			v += rAdd;

		if ((v & gMask) > gLimit)
			v |= gMask;
		else
			v += gAdd;

		if ((v & bMask) > bLimit)
			v |= bMask;
		else
			v += bAdd;

		*element = v;

		numElements--;
		element++;
	}
}

template<class TElement>
void VisualElement::renderDarkenScanline(TElement *element, size_t numElements, TElement rMask, TElement rSub, TElement gMask, TElement gSub, TElement bMask, TElement bSub) {
	TElement rZero = ~rMask;
	TElement gZero = ~gMask;
	TElement bZero = ~bMask;

	while (numElements > 0) {
		TElement v = *element;

		if ((v & rMask) < rSub)
			v &= rZero;
		else
			v -= rSub;

		if ((v & gMask) < gSub)
			v &= gZero;
		else
			v -= gSub;

		if ((v & bMask) < bSub)
			v &= bZero;
		else
			v -= bSub;

		*element = v;

		numElements--;
		element++;
	}
}

bool NonVisualElement::isVisual() const {
	return false;
}

bool NonVisualElement::loadCommon(const Common::String &name, uint32 guid, uint32 elementFlags) {
	_name = name;
	_guid = guid;
	_streamLocator = 0;
	_sectionID = 0;

	return true;
}


ModifierFlags::ModifierFlags() : isLastModifier(false), flagsWereLoaded(false) {
}

bool ModifierFlags::load(const uint32 dataModifierFlags) {
	isLastModifier = ((dataModifierFlags & 0x2) != 0);
	flagsWereLoaded = true;
	return true;
}

ModifierSaveLoad::~ModifierSaveLoad() {
}

void ModifierSaveLoad::save(Modifier *modifier, Common::WriteStream *stream) {
	const Common::String &name = modifier->getName();

	stream->writeUint32BE(modifier->getStaticGUID());
	stream->writeUint16BE(name.size());
	stream->writeString(name);

	saveInternal(stream);
}

bool ModifierSaveLoad::load(Modifier *modifier, Common::ReadStream *stream, uint32 saveFileVersion) {
	uint32 checkGUID = stream->readUint32BE();
	uint16 nameLen = stream->readUint16BE();

	if (stream->err())
		return false;

	const Common::String &name = modifier->getName();

	if (name.size() != nameLen)
		return false;

	Common::Array<char> checkName;
	checkName.resize(nameLen);

	if (nameLen > 0) {
		stream->read(&checkName[0], nameLen);
		if (stream->err() || memcmp(&checkName[0], name.c_str(), nameLen))
			return false;
	}

	if (modifier->getStaticGUID() != checkGUID)
		return false;

	return loadInternal(stream, saveFileVersion);
}

ModifierHooks::~ModifierHooks() {
}

void ModifierHooks::onCreate(Modifier *modifier) {
}

Modifier::Modifier() : _parent(nullptr) {
}

Modifier::~Modifier() {
}

bool Modifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "parent") {
		result.setObject(_parent);
		return true;
	} else if (attrib == "subsection") {
		RuntimeObject *scan = _parent.lock().get();
		while (scan) {
			if (scan->isSubsection()) {
				result.setObject(scan->getSelfReference());
				return true;
			}

			if (scan->isStructural())
				scan = static_cast<Structural *>(scan)->getParent();
			else if (scan->isModifier())
				scan = static_cast<Modifier *>(scan)->getParent().lock().get();
			else
				break;
		}

		return false;
	} else if (attrib == "name") {
		result.setString(_name);
		return true;
	} else if (attrib == "element") {
		Structural *owner = findStructuralOwner();
		result.setObject(owner ? owner->getSelfReference() : Common::WeakPtr<RuntimeObject>());
		return true;
	} else if (attrib == "previous") {
		Modifier *sibling = findPrevSibling();
		if (sibling)
			result.setObject(sibling->getSelfReference());
		else
			result.clear();
		return true;
	} else if (attrib == "next") {
		Modifier *sibling = findNextSibling();
		if (sibling)
			result.setObject(sibling->getSelfReference());
		else
			result.clear();
		return true;
	}

	return false;
}

MiniscriptInstructionOutcome Modifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	if (attrib == "parent") {
		DynamicValueWriteObjectHelper::create(_parent.lock().get(), writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "name") {
		DynamicValueWriteStringHelper::create(&_name, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return RuntimeObject::writeRefAttribute(thread, writeProxy, attrib);
}

void Modifier::materialize(Runtime *runtime, ObjectLinkingScope *outerScope) {
	ObjectLinkingScope innerScope;
	innerScope.setParent(outerScope);

	ModifierInnerScopeBuilder innerScopeBuilder(runtime, this, &innerScope);
	this->visitInternalReferences(&innerScopeBuilder);

	ModifierChildMaterializer childMaterializer(runtime, &innerScope);
	this->visitInternalReferences(&childMaterializer);

	linkInternalReferences(outerScope);
	setRuntimeGUID(runtime->allocateRuntimeGUID());
}

bool Modifier::isAlias() const {
	return false;
}

bool Modifier::isVariable() const {
	return false;
}

bool Modifier::isBehavior() const {
	return false;
}

bool Modifier::isCompoundVariable() const {
	return false;
}

bool Modifier::isKeyboardMessenger() const {
	return false;
}

Common::SharedPtr<ModifierSaveLoad> Modifier::getSaveLoad(Runtime *runtime) {
	return nullptr;
}

bool Modifier::isModifier() const {
	return true;
}

IModifierContainer *Modifier::getMessagePropagationContainer() {
	return nullptr;
}

IModifierContainer *Modifier::getChildContainer() {
	return nullptr;
}

const Common::WeakPtr<RuntimeObject>& Modifier::getParent() const {
	return _parent;
}

void Modifier::setParent(const Common::WeakPtr<RuntimeObject> &parent) {
	_parent = parent;
}

Modifier *Modifier::findNextSibling() const {
	RuntimeObject *parent = getParent().lock().get();
	if (parent) {
		IModifierContainer *container = nullptr;
		if (parent->isModifier())
			container = static_cast<Modifier *>(parent)->getChildContainer();
		else if (parent->isStructural())
			container = static_cast<Structural *>(parent);

		if (container)
		{
			const Common::Array<Common::SharedPtr<Modifier> > &neighborhood = container->getModifiers();
			bool found = false;
			size_t foundIndex = 0;
			for (size_t i = 0; i < neighborhood.size(); i++) {
				if (neighborhood[i].get() == this) {
					foundIndex = i;
					found = true;
					break;
				}
			}

			if (found && foundIndex < neighborhood.size() - 1)
				return neighborhood[foundIndex + 1].get();
		}
	}

	return nullptr;
}

Modifier *Modifier::findPrevSibling() const {
	RuntimeObject *parent = getParent().lock().get();
	if (parent) {
		IModifierContainer *container = nullptr;
		if (parent->isModifier())
			container = static_cast<Modifier *>(parent)->getChildContainer();
		else if (parent->isStructural())
			container = static_cast<Structural *>(parent);

		if (container) {
			const Common::Array<Common::SharedPtr<Modifier> > &neighborhood = container->getModifiers();
			bool found = false;
			size_t foundIndex = 0;
			for (size_t i = 0; i < neighborhood.size(); i++) {
				if (neighborhood[i].get() == this) {
					foundIndex = i;
					found = true;
					break;
				}
			}

			if (found && foundIndex > 0)
				return neighborhood[foundIndex - 1].get();
		}
	}

	return nullptr;
}

bool Modifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState Modifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	// If you're here, a message type was reported as responsive by respondsToEvent but consumeMessage wasn't overridden
	assert(false);
	return kVThreadError;
}

void Modifier::setName(const Common::String& name) {
	_name = name;
}

const Common::String& Modifier::getName() const {
	return _name;
}

const ModifierFlags& Modifier::getModifierFlags() const {
	return _modifierFlags;
}

void Modifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
}

bool Modifier::loadPlugInHeader(const PlugInModifierLoaderContext &plugInContext) {
	_guid = plugInContext.plugInModifierData.guid;
	_name = plugInContext.plugInModifierData.name;
	_modifierFlags.load(plugInContext.plugInModifierData.modifierFlags);

	return true;
}

void Modifier::recursiveCollectObjectsMatchingCriteria(Common::Array<Common::WeakPtr<RuntimeObject> > &results, bool (*evalFunc)(void *userData, RuntimeObject *object), void *userData, bool onlyEnabled) {
	if (evalFunc(userData, this))
		results.push_back(getSelfReference());

	IModifierContainer *childContainer = nullptr;
	if (onlyEnabled)
		childContainer = getMessagePropagationContainer();
	else
		childContainer = getChildContainer();

	if (childContainer) {
		for (const Common::SharedPtr<Modifier> &child : childContainer->getModifiers())
			child->recursiveCollectObjectsMatchingCriteria(results, evalFunc, userData, onlyEnabled);
	}
}

void Modifier::setHooks(const Common::SharedPtr<ModifierHooks> &hooks) {
	_hooks = hooks;
}

const Common::SharedPtr<ModifierHooks> &Modifier::getHooks() const {
	return _hooks;
}

#ifdef MTROPOLIS_DEBUG_ENABLE

SupportStatus Modifier::debugGetSupportStatus() const {
	return kSupportStatusNone;
}

const Common::String &Modifier::debugGetName() const {
	return _name;
}

void Modifier::debugInspect(IDebugInspectionReport *report) const {
	if (report->declareStatic("type"))
		report->declareStaticContents(debugGetTypeName());
	if (report->declareStatic("guid"))
		report->declareStaticContents(Common::String::format("%x", getStaticGUID()));
	if (report->declareStatic("runtimeID"))
		report->declareStaticContents(Common::String::format("%x", getRuntimeGUID()));
}

#endif /* MTROPOLIS_DEBUG_ENABLE */

VariableStorage::~VariableStorage() {
}

VariableModifier::VariableModifier(const Common::SharedPtr<VariableStorage> &storage) : _storage(storage) {
}

VariableModifier::VariableModifier(const VariableModifier &other) : Modifier(other), _storage(other._storage->clone()) {
}

bool VariableModifier::isVariable() const {
	return true;
}

bool VariableModifier::isListVariable() const {
	return false;
}


Common::SharedPtr<ModifierSaveLoad> VariableModifier::getSaveLoad(Runtime *runtime) {
	return _storage->getSaveLoad(runtime);
}

const Common::SharedPtr<VariableStorage> &VariableModifier::getStorage() const {
	return _storage;
}

void VariableModifier::setStorage(const Common::SharedPtr<VariableStorage> &storage) {
	_storage = storage;
}

bool VariableModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "value") {
		varGetValue(result);
		return true;
	}

	return Modifier::readAttribute(thread, result, attrib);
}

void VariableModifier::disable(Runtime *runtime) {
}

DynamicValueWriteProxy VariableModifier::createWriteProxy() {
	DynamicValueWriteProxy proxy;
	proxy.pod.objectRef = this;
	proxy.pod.ptrOrOffset = 0;
	proxy.pod.ifc = DynamicValueWriteInterfaceGlue<VariableModifier::WriteProxyInterface>::getInstance();
	return proxy;
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void VariableModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);

	if (report->declareStatic("storage"))
		report->declareStaticContents(Common::String::format("%p", static_cast<void *>(_storage.get())));
}
#endif

MiniscriptInstructionOutcome VariableModifier::WriteProxyInterface::write(MiniscriptThread *thread, const DynamicValue &dest, void *objectRef, uintptr ptrOrOffset) {
	if (!static_cast<VariableModifier *>(objectRef)->varSetValue(thread, dest))
		return kMiniscriptInstructionOutcomeFailed;
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome VariableModifier::WriteProxyInterface::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &dest, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) {
	return static_cast<VariableModifier *>(objectRef)->writeRefAttribute(thread, dest, attrib);
}

MiniscriptInstructionOutcome VariableModifier::WriteProxyInterface::refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &dest, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) {
	return static_cast<VariableModifier *>(objectRef)->writeRefAttributeIndexed(thread, dest, attrib, index);
}

bool Modifier::loadTypicalHeader(const Data::TypicalModifierHeader &typicalHeader) {
	if (!_modifierFlags.load(typicalHeader.modifierFlags))
		return false;
	_guid = typicalHeader.guid;
	_name = typicalHeader.name;

	return true;
}

Structural *Modifier::findStructuralOwner() const {
	RuntimeObject *scan = _parent.lock().get();
	while (scan) {
		if (scan->isModifier())
			scan = static_cast<Modifier *>(scan)->_parent.lock().get();
		else if (scan->isStructural())
			return static_cast<Structural *>(scan);
		else
			return nullptr;
	}

	return nullptr;
}

void Modifier::linkInternalReferences(ObjectLinkingScope *scope) {
}

} // End of namespace MTropolis
