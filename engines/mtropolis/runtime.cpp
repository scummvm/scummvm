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
#include "mtropolis/asset_factory.h"
#include "mtropolis/element_factory.h"
#include "mtropolis/miniscript.h"
#include "mtropolis/modifier_factory.h"
#include "mtropolis/modifiers.h"
#include "mtropolis/render.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/random.h"
#include "common/substream.h"
#include "common/system.h"

#include "graphics/cursorman.h"
#include "graphics/managed_surface.h"
#include "graphics/surface.h"
#include "graphics/wincursor.h"
#include "graphics/maccursor.h"
#include "graphics/macgui/macfontmanager.h"

#include "audio/mixer.h"

namespace MTropolis {

class MainWindow : public Window {
public:
	MainWindow(const WindowParameters &windowParams);

	void onMouseDown(int32 x, int32 y, int mouseButton) override;
	void onMouseMove(int32 x, int32 y) override;
	void onMouseUp(int32 x, int32 y, int mouseButton) override;
	void onKeyboardEvent(const Common::EventType evtType, bool repeat, const Common::KeyState &keyEvt) override;

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


class ModifierInnerScopeBuilder : public IStructuralReferenceVisitor {
public:
	ModifierInnerScopeBuilder(ObjectLinkingScope *scope);

	void visitChildStructuralRef(Common::SharedPtr<Structural> &structural) override;
	void visitChildModifierRef(Common::SharedPtr<Modifier> &modifier) override;
	void visitWeakStructuralRef(Common::WeakPtr<Structural> &structural) override;
	void visitWeakModifierRef(Common::WeakPtr<Modifier> &modifier) override;

private:
	ObjectLinkingScope *_scope;
};

ModifierInnerScopeBuilder::ModifierInnerScopeBuilder(ObjectLinkingScope *scope) : _scope(scope) {
}

void ModifierInnerScopeBuilder::visitChildStructuralRef(Common::SharedPtr<Structural> &structural) {
	assert(false);
}

void ModifierInnerScopeBuilder::visitChildModifierRef(Common::SharedPtr<Modifier> &modifier) {
	_scope->addObject(modifier->getStaticGUID(), modifier->getName(), modifier);
}

void ModifierInnerScopeBuilder::visitWeakStructuralRef(Common::WeakPtr<Structural> &structural) {
}

void ModifierInnerScopeBuilder::visitWeakModifierRef(Common::WeakPtr<Modifier> &modifier) {
	// Do nothing
}

class ModifierChildMaterializer : public IStructuralReferenceVisitor {
public:
	ModifierChildMaterializer(Runtime *runtime, Modifier *modifier, ObjectLinkingScope *outerScope);

	void visitChildStructuralRef(Common::SharedPtr<Structural> &structural) override;
	void visitChildModifierRef(Common::SharedPtr<Modifier> &modifier) override;
	void visitWeakStructuralRef(Common::WeakPtr<Structural> &structural) override;
	void visitWeakModifierRef(Common::WeakPtr<Modifier> &modifier) override;

private:
	Runtime *_runtime;
	Modifier *_modifier;
	ObjectLinkingScope *_outerScope;
};

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

ModifierChildMaterializer::ModifierChildMaterializer(Runtime *runtime, Modifier *modifier, ObjectLinkingScope *outerScope)
	: _runtime(runtime), _modifier(modifier), _outerScope(outerScope) {
}

void ModifierChildMaterializer::visitChildStructuralRef(Common::SharedPtr<Structural> &structural) {
	assert(false);
}

void ModifierChildMaterializer::visitChildModifierRef(Common::SharedPtr<Modifier> &modifier) {
	_runtime->instantiateIfAlias(modifier, _modifier->getSelfReference());
	modifier->materialize(_runtime, _outerScope);
}

void ModifierChildMaterializer::visitWeakStructuralRef(Common::WeakPtr<Structural> &structural) {
	// Do nothing
}

void ModifierChildMaterializer::visitWeakModifierRef(Common::WeakPtr<Modifier> &modifier) {
	// Do nothing
}

ModifierChildCloner::ModifierChildCloner(Runtime *runtime, const Common::WeakPtr<RuntimeObject> &relinkParent)
	: _runtime(runtime), _relinkParent(relinkParent) {
}

void ModifierChildCloner::visitChildStructuralRef(Common::SharedPtr<Structural> &structural) {
	assert(false);
}

void ModifierChildCloner::visitChildModifierRef(Common::SharedPtr<Modifier> &modifier) {
	modifier = modifier->shallowClone();
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


bool Point16::load(const Data::Point &point) {
	x = point.x;
	y = point.y;

	return true;
}

MiniscriptInstructionOutcome Point16::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, const Common::String &attrib) {
	if (attrib == "x") {
		DynamicValueWriteIntegerHelper<int16>::create(&x, proxy);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "y") {
		DynamicValueWriteIntegerHelper<int16>::create(&y, proxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return kMiniscriptInstructionOutcomeFailed;
}

Common::String Point16::toString() const {
	return Common::String::format("(%i,%i)", x, y);
}

bool Rect16::load(const Data::Rect &rect) {
	top = rect.top;
	left = rect.left;
	bottom = rect.bottom;
	right = rect.right;

	if (bottom < top || right < left)
		return false;

	return true;
}

bool Rect16::loadUnchecked(const Data::Rect &rect) {
	top = rect.top;
	left = rect.left;
	bottom = rect.bottom;
	right = rect.right;

	return true;
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


bool Label::load(const Data::Label &label) {
	id = label.labelID;
	superGroupID = label.superGroupID;

	return true;
}

bool ColorRGB8::load(const Data::ColorRGB16 &color) {
	this->r = (color.red * 510 + 1) / 131070;
	this->g = (color.green * 510 + 1) / 131070;
	this->b = (color.blue * 510 + 1) / 131070;

	return true;
}

MessageFlags::MessageFlags() : relay(true), cascade(true), immediate(true) {
}

DynamicListContainerBase::~DynamicListContainerBase() {
}

void DynamicListDefaultSetter::defaultSet(int32 &value) {
	value = 0;
}

void DynamicListDefaultSetter::defaultSet(double &value) {
	value = 0.0;
}

void DynamicListDefaultSetter::defaultSet(Point16 &value) {
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

bool DynamicListValueImporter::importValue(const DynamicValue &dynValue, const Point16 *&outPtr) {
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

void DynamicListValueExporter::exportValue(DynamicValue &dynValue, const Point16 &value) {
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

bool DynamicListContainer<VarReference>::setAtIndex(size_t index, const DynamicValue &dynValue) {
	if (dynValue.getType() != DynamicValueTypes::kVariableReference)
		return false;

	size_t requiredSize = index + 1;

	if (_array.size() < requiredSize) {
		size_t prevSize = _array.size();
		_array.resize(requiredSize);
		_strings.resize(requiredSize);

		for (size_t i = prevSize; i < index; i++) {
			_array[i].guid = 0;
		}

		const VarReference &varRef = dynValue.getVarReference();
		_array[index].guid = varRef.guid;
		_strings[index] = *varRef.source;

		rebuildStringPointers();
	} else {
		const VarReference &varRef = dynValue.getVarReference();
		_array[index].guid = varRef.guid;
		_strings[index] = *varRef.source;
	}

	return true;
}

void DynamicListContainer<VarReference>::truncateToSize(size_t sz) {
	if (_array.size() > sz)
		_array.resize(sz);
}

bool DynamicListContainer<VarReference>::expandToMinimumSize(size_t sz) {
	if (_array.size() < sz) {
		size_t prevSize = _array.size();
		_array.resize(sz);
		_strings.resize(sz);

		for (size_t i = prevSize; i < sz; prevSize++) {
			_array[i].guid = 0;
			_array[i].source = nullptr;
		}
	}

	return true;
}

bool DynamicListContainer<VarReference>::getAtIndex(size_t index, DynamicValue &dynValue) const {
	// TODO: Refactor this whole thing to use linkInternalReferences
	if (index >= _array.size())
		return false;

	assert(false);
	return false;
}

void DynamicListContainer<VarReference>::setFrom(const DynamicListContainerBase &other) {
	const DynamicListContainer<VarReference> &otherTyped = static_cast<const DynamicListContainer<VarReference> &>(other);

	_array = otherTyped._array;
	_strings = otherTyped._strings;
	rebuildStringPointers();
}

const void *DynamicListContainer<VarReference>::getConstArrayPtr() const {
	return &_array;
}

void *DynamicListContainer<VarReference>::getArrayPtr() {
	return &_array;
}

size_t DynamicListContainer<VarReference>::getSize() const {
	return _array.size();
}

bool DynamicListContainer<VarReference>::compareEqual(const DynamicListContainerBase &other) const {
	const DynamicListContainer<VarReference> &otherTyped = static_cast<const DynamicListContainer<VarReference> &>(other);
	return _array == otherTyped._array;
}

DynamicListContainerBase *DynamicListContainer<VarReference>::clone() const {
	return new DynamicListContainer<VarReference>(*this);
}

void DynamicListContainer<VarReference>::rebuildStringPointers() {
	assert(_strings.size() == _array.size());

	size_t numStrings = _array.size();
	for (size_t i = 0; i < numStrings; i++) {
		_array[i].source = &_strings[i];
	}
}

DynamicList::DynamicList() : _type(DynamicValueTypes::kEmpty), _container(nullptr) {
}

DynamicList::DynamicList(const DynamicList &other) : _type(DynamicValueTypes::kEmpty), _container(nullptr) {
	initFromOther(other);
}

DynamicList::~DynamicList() {
	clear();
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

const Common::Array<Point16> &DynamicList::getPoint() const {
	assert(_type == DynamicValueTypes::kPoint);
	return *static_cast<const Common::Array<Point16> *>(_container->getConstArrayPtr());
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

const Common::Array<VarReference> &DynamicList::getVarReference() const {
	assert(_type == DynamicValueTypes::kVariableReference);
	return *static_cast<const Common::Array<VarReference> *>(_container->getConstArrayPtr());
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

Common::Array<Point16> &DynamicList::getPoint() {
	assert(_type == DynamicValueTypes::kPoint);
	return *static_cast<Common::Array<Point16> *>(_container->getArrayPtr());
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

Common::Array<VarReference> &DynamicList::getVarReference() {
	assert(_type == DynamicValueTypes::kVariableReference);
	return *static_cast<Common::Array<VarReference> *>(_container->getArrayPtr());
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
		if (_container != nullptr && _container->getSize() != 0) {
			DynamicValue converted;
			if (!value.convertToType(_type, converted))
				return false;
			return setAtIndex(index, converted);
		} else {
			clear();
			changeToType(value.getType());
			return _container->setAtIndex(index, value);
		}
	} else {
		return _container->setAtIndex(index, value);
	}
}

void DynamicList::truncateToSize(size_t sz) {
	if (sz == 0)
		clear();
	else if (_container)
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
		clear();
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
	proxy.pod.ifc = &WriteProxyInterface::_instance;
	proxy.pod.objectRef = this;
	proxy.pod.ptrOrOffset = index;
}

bool DynamicList::changeToType(DynamicValueTypes::DynamicValueType type) {
	switch (type) {
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
		_container = new DynamicListContainer<Point16>();
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
	case DynamicValueTypes::kVariableReference:
		_container = new DynamicListContainer<VarReference>();
		break;
	case DynamicValueTypes::kIncomingData:
		_container = new DynamicListContainer<void>();
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
	}

	_type = type;

	return true;
}

void DynamicList::clear() {
	_type = DynamicValueTypes::kEmpty;
	if (_container)
		delete _container;
	_container = nullptr;
}

void DynamicList::initFromOther(const DynamicList &other) {
	assert(_container == nullptr);
	assert(_type == DynamicValueTypes::kEmpty);

	if (other._type != DynamicValueTypes::kEmpty) {
		changeToType(other._type);
		_container->setFrom(*other._container);
	}
}

MiniscriptInstructionOutcome DynamicList::WriteProxyInterface::write(MiniscriptThread *thread, const DynamicValue &value, void *objectRef, uintptr ptrOrOffset) const {
	if (!static_cast<DynamicList *>(objectRef)->setAtIndex(ptrOrOffset, value))
		return kMiniscriptInstructionOutcomeFailed;
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome DynamicList::WriteProxyInterface::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) const {
	DynamicList *list = static_cast<DynamicList *>(objectRef);

	switch (list->getType()) {
	case DynamicValueTypes::kPoint:
		list->expandToMinimumSize(ptrOrOffset + 1);
		return list->getPoint()[ptrOrOffset].refAttrib(thread, proxy, attrib);
	case DynamicValueTypes::kIntegerRange:
		list->expandToMinimumSize(ptrOrOffset + 1);
		return list->getIntRange()[ptrOrOffset].refAttrib(thread, proxy, attrib);
	case DynamicValueTypes::kVector:
		list->expandToMinimumSize(ptrOrOffset + 1);
		return list->getVector()[ptrOrOffset].refAttrib(thread, proxy, attrib);
	case DynamicValueTypes::kObject: {
			if (list->getSize() <= ptrOrOffset) {
				return kMiniscriptInstructionOutcomeFailed;
				}

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

MiniscriptInstructionOutcome DynamicList::WriteProxyInterface::refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) const {
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
		}
	case DynamicValueTypes::kObject: {
			if (list->getSize() <= ptrOrOffset)
				return kMiniscriptInstructionOutcomeFailed;

			Common::SharedPtr<RuntimeObject> obj = list->getObjectReference()[ptrOrOffset].object.lock();
			proxy.containerList.reset();
			if (!obj && !obj->writeRefAttributeIndexed(thread, proxy, attrib, index))
				return kMiniscriptInstructionOutcomeFailed;

			return kMiniscriptInstructionOutcomeContinue;
		}
	default:
		return kMiniscriptInstructionOutcomeFailed;
	}

	return kMiniscriptInstructionOutcomeFailed;
}

DynamicList::WriteProxyInterface DynamicList::WriteProxyInterface::_instance;

DynamicValue::DynamicValue() : _type(DynamicValueTypes::kNull) {
}

DynamicValue::DynamicValue(const DynamicValue &other) : _type(DynamicValueTypes::kNull) {
	initFromOther(other);
}

DynamicValue::~DynamicValue() {
	clear();
}

bool DynamicValue::load(const Data::InternalTypeTaggedValue &data, const Common::String &varSource, const Common::String &varString) {
	switch (data.type) {
	case Data::InternalTypeTaggedValue::kNull:
		_type = DynamicValueTypes::kNull;
		break;
	case Data::InternalTypeTaggedValue::kIncomingData:
		_type = DynamicValueTypes::kIncomingData;
		break;
	case Data::InternalTypeTaggedValue::kInteger:
		_type = DynamicValueTypes::kInteger;
		_value.asInt = data.value.asInteger;
		break;
	case Data::InternalTypeTaggedValue::kString:
		_type = DynamicValueTypes::kString;
		_str = varString;
		break;
	case Data::InternalTypeTaggedValue::kPoint:
		_type = DynamicValueTypes::kPoint;
		if (!_value.asPoint.load(data.value.asPoint))
			return false;
		break;
	case Data::InternalTypeTaggedValue::kIntegerRange:
		_type = DynamicValueTypes::kIntegerRange;
		if (!_value.asIntRange.load(data.value.asIntegerRange))
			return false;
		break;
	case Data::InternalTypeTaggedValue::kFloat:
		_type = DynamicValueTypes::kFloat;
		_value.asFloat = data.value.asFloat.toDouble();
		break;
	case Data::InternalTypeTaggedValue::kBool:
		_type = DynamicValueTypes::kBoolean;
		_value.asBool = (data.value.asBool != 0);
		break;
	case Data::InternalTypeTaggedValue::kVariableReference:
		_type = DynamicValueTypes::kVariableReference;
		_value.asVarReference.guid = data.value.asVariableReference.guid;
		_value.asVarReference.source = &_str;
		_str = varSource;
		break;
	case Data::InternalTypeTaggedValue::kLabel:
		_type = DynamicValueTypes::kLabel;
		if (!_value.asLabel.load(data.value.asLabel))
			return false;
		break;
	default:
		assert(false);
		return false;
	}

	return true;
}

bool DynamicValue::load(const Data::PlugInTypeTaggedValue &data) {
	switch (data.type) {
	case Data::PlugInTypeTaggedValue::kNull:
		_type = DynamicValueTypes::kNull;
		break;
	case Data::PlugInTypeTaggedValue::kIncomingData:
		_type = DynamicValueTypes::kIncomingData;
		break;
	case Data::PlugInTypeTaggedValue::kInteger:
		_type = DynamicValueTypes::kInteger;
		_value.asInt = data.value.asInt;
		break;
	case Data::PlugInTypeTaggedValue::kIntegerRange:
		_type = DynamicValueTypes::kIntegerRange;
		if (!_value.asIntRange.load(data.value.asIntRange))
			return false;
		break;
	case Data::PlugInTypeTaggedValue::kFloat:
		_type = DynamicValueTypes::kFloat;
		_value.asFloat = data.value.asFloat.toDouble();
		break;
	case Data::PlugInTypeTaggedValue::kBoolean:
		_type = DynamicValueTypes::kBoolean;
		_value.asBool = (data.value.asBoolean != 0);
		break;
	case Data::PlugInTypeTaggedValue::kEvent:
		_type = DynamicValueTypes::kEvent;
		if (!_value.asEvent.load(data.value.asEvent))
			return false;
		break;
	case Data::PlugInTypeTaggedValue::kLabel:
		_type = DynamicValueTypes::kLabel;
		if (!_value.asLabel.load(data.value.asLabel))
			return false;
		break;
	case Data::PlugInTypeTaggedValue::kString:
		_type = DynamicValueTypes::kString;
		_str = data.str;
		break;
	case Data::PlugInTypeTaggedValue::kVariableReference:
		_type = DynamicValueTypes::kVariableReference;
		_value.asVarReference.guid = data.value.asVarRefGUID;
		_value.asVarReference.source = &_str;
		_str.clear(); // Extra data doesn't seem to correlate to this
		break;
	case Data::PlugInTypeTaggedValue::kPoint:
		_type = DynamicValueTypes::kPoint;
		if (!_value.asPoint.load(data.value.asPoint))
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

const Point16 &DynamicValue::getPoint() const {
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

const VarReference &DynamicValue::getVarReference() const {
	assert(_type == DynamicValueTypes::kVariableReference);
	return _value.asVarReference;
}

const Common::String &DynamicValue::getString() const {
	assert(_type == DynamicValueTypes::kString);
	return _str;
}

const bool &DynamicValue::getBool() const {
	assert(_type == DynamicValueTypes::kBoolean);
	return _value.asBool;
}

const Common::SharedPtr<DynamicList> &DynamicValue::getList() const {
	assert(_type == DynamicValueTypes::kList);
	return _list;
}

const ObjectReference &DynamicValue::getObject() const {
	assert(_type == DynamicValueTypes::kObject);
	return _obj;
}

const DynamicValueReadProxyPOD &DynamicValue::getReadProxyPOD() const {
	assert(_type == DynamicValueTypes::kReadProxy);
	return _value.asReadProxy;
}

const DynamicValueWriteProxyPOD &DynamicValue::getWriteProxyPOD() const {
	assert(_type == DynamicValueTypes::kWriteProxy);
	return _value.asWriteProxy;
}


DynamicValueReadProxy DynamicValue::getReadProxyTEMP() const {
	assert(_type == DynamicValueTypes::kReadProxy);

	DynamicValueReadProxy proxy;
	proxy.pod = _value.asReadProxy;
	proxy.containerList = _list;
	return proxy;
}

DynamicValueWriteProxy DynamicValue::getWriteProxyTEMP() const {
	assert(_type == DynamicValueTypes::kWriteProxy);

	DynamicValueWriteProxy proxy;
	proxy.pod = _value.asWriteProxy;
	proxy.containerList = _list;
	return proxy;
}

const Common::SharedPtr<DynamicList> &DynamicValue::getReadProxyContainer() const {
	assert(_type == DynamicValueTypes::kReadProxy);
	return _list;
}

const Common::SharedPtr<DynamicList> &DynamicValue::getWriteProxyContainer() const {
	assert(_type == DynamicValueTypes::kWriteProxy);
	return _list;
}

void DynamicValue::setInt(int32 value) {
	if (_type != DynamicValueTypes::kInteger)
		clear();
	_type = DynamicValueTypes::kInteger;
	_value.asInt = value;
}

void DynamicValue::setFloat(double value) {
	if (_type != DynamicValueTypes::kFloat)
		clear();
	_type = DynamicValueTypes::kFloat;
	_value.asFloat = value;
}

void DynamicValue::setPoint(const Point16 &value) {
	if (_type != DynamicValueTypes::kPoint)
		clear();
	_type = DynamicValueTypes::kPoint;
	_value.asPoint = value;
}

void DynamicValue::setIntRange(const IntRange &value) {
	if (_type != DynamicValueTypes::kIntegerRange)
		clear();
	_type = DynamicValueTypes::kIntegerRange;
	_value.asIntRange = value;
}

void DynamicValue::setVector(const AngleMagVector &value) {
	if (_type != DynamicValueTypes::kVector)
		clear();
	_type = DynamicValueTypes::kVector;
	_value.asVector = value;
}

void DynamicValue::setLabel(const Label &value) {
	if (_type != DynamicValueTypes::kLabel)
		clear();
	_type = DynamicValueTypes::kLabel;
	_value.asLabel = value;
}

void DynamicValue::setEvent(const Event &value) {
	if (_type != DynamicValueTypes::kEvent)
		clear();
	_type = DynamicValueTypes::kEvent;
	_value.asEvent = value;
}

void DynamicValue::setVarReference(const VarReference &value) {
	if (_type != DynamicValueTypes::kVariableReference)
		clear();
	_type = DynamicValueTypes::kVariableReference;
	_value.asVarReference.guid = value.guid;
	_value.asVarReference.source = &_str;
	_str = *value.source;
}

void DynamicValue::setString(const Common::String &value) {
	if (_type != DynamicValueTypes::kString)
		clear();
	_type = DynamicValueTypes::kString;
	_str = value;
}

void DynamicValue::setBool(bool value) {
	if (_type != DynamicValueTypes::kBoolean)
		clear();
	_type = DynamicValueTypes::kBoolean;
	_value.asBool = value;
}

void DynamicValue::setList(const Common::SharedPtr<DynamicList> &value) {
	if (_type != DynamicValueTypes::kList)
		clear();
	_type = DynamicValueTypes::kList;
	_list = value;
}

void DynamicValue::setReadProxy(const DynamicValueReadProxy &readProxy) {
	Common::SharedPtr<DynamicList> listRef = readProxy.containerList;	// Back up list ref in case this is a self-assign
	if (_type != DynamicValueTypes::kReadProxy)
		clear();
	_type = DynamicValueTypes::kReadProxy;
	_value.asReadProxy = readProxy.pod;
	_list = listRef;
}

void DynamicValue::setWriteProxy(const DynamicValueWriteProxy &writeProxy) {
	Common::SharedPtr<DynamicList> listRef = writeProxy.containerList; // Back up list ref in case this is a self-assign
	if (_type != DynamicValueTypes::kWriteProxy)
		clear();
	_type = DynamicValueTypes::kWriteProxy;
	_value.asWriteProxy = writeProxy.pod;
	_list = listRef;
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
	if (_type == targetType) {
		result = *this;
		return true;
	}

	switch (_type) {
	case DynamicValueTypes::kInteger:
		return convertIntToType(targetType, result);
	case DynamicValueTypes::kFloat:
		return convertFloatToType(targetType, result);
	case DynamicValueTypes::kBoolean:
		return convertBoolToType(targetType, result);
	default:
		warning("Couldn't convert dynamic value from source type");
		return false;
	}
}

void DynamicValue::setObject(const ObjectReference &value) {
	if (_type != DynamicValueTypes::kObject)
		clear();
	_type = DynamicValueTypes::kObject;
	_obj = value;
}

void DynamicValue::setObject(const Common::WeakPtr<RuntimeObject> &value) {
	setObject(ObjectReference(value));
}

void DynamicValue::swap(DynamicValue &other) {
	internalSwap(_type, other._type);
	internalSwap(_str, other._str);
	internalSwap(_list, other._list);
	internalSwap(_obj, other._obj);

	ValueUnion tempValue;
	memcpy(&tempValue, &_value, sizeof(ValueUnion));
	memcpy(&_value, &other._value, sizeof(ValueUnion));
	memcpy(&other._value, &tempValue, sizeof(ValueUnion));
}

DynamicValue &DynamicValue::operator=(const DynamicValue &other) {
	if (this != &other) {
		DynamicValue temp(other);
		swap(temp);
	}

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
		return _value.asPoint == other._value.asPoint;
	case DynamicValueTypes::kIntegerRange:
		return _value.asIntRange == other._value.asIntRange;
	case DynamicValueTypes::kVector:
		return _value.asVector == other._value.asVector;
	case DynamicValueTypes::kLabel:
		return _value.asLabel == other._value.asLabel;
	case DynamicValueTypes::kEvent:
		return _value.asEvent == other._value.asEvent;
	case DynamicValueTypes::kVariableReference:
		return _value.asVarReference == other._value.asVarReference;
	case DynamicValueTypes::kIncomingData:
		return true;
	case DynamicValueTypes::kString:
		return _str == other._str;
	case DynamicValueTypes::kBoolean:
		return _value.asBool == other._value.asBool;
	case DynamicValueTypes::kList:
		return (*_list.get()) == (*other._list.get());
	case DynamicValueTypes::kObject:
		return _obj == other._obj;
	default:
		break;
	}

	assert(false);
	return false;
}

void DynamicValue::clear() {
	_list.reset();
	_obj.reset();
	_str.clear();
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

void DynamicValue::initFromOther(const DynamicValue &other) {
	assert(_type == DynamicValueTypes::kNull);

	switch (other._type) {
	case DynamicValueTypes::kNull:
	case DynamicValueTypes::kIncomingData:	// FIXME: Get rid of this
		break;
	case DynamicValueTypes::kInteger:
		_value.asInt = other._value.asInt;
		break;
	case DynamicValueTypes::kFloat:
		_value.asFloat = other._value.asFloat;
		break;
	case DynamicValueTypes::kPoint:
		_value.asPoint = other._value.asPoint;
		break;
	case DynamicValueTypes::kIntegerRange:
		_value.asIntRange = other._value.asIntRange;
		break;
	case DynamicValueTypes::kVector:
		_value.asVector = other._value.asVector;
		break;
	case DynamicValueTypes::kLabel:
		_value.asLabel = other._value.asLabel;
		break;
	case DynamicValueTypes::kEvent:
		_value.asEvent = other._value.asEvent;
		break;
	case DynamicValueTypes::kVariableReference:
		_value.asVarReference = other._value.asVarReference;
		_str = other._str;
		_value.asVarReference.source = &_str;
		break;
	case DynamicValueTypes::kString:
		_str = other._str;
		break;
	case DynamicValueTypes::kBoolean:
		_value.asBool = other._value.asBool;
		break;
	case DynamicValueTypes::kList:
		_list = other._list;
		break;
	case DynamicValueTypes::kObject:
		_obj = other._obj;
		break;
	default:
		assert(false);
		break;
	}

	_type = other._type;
}

MiniscriptInstructionOutcome DynamicValueWriteStringHelper::write(MiniscriptThread *thread, const DynamicValue &value, void *objectRef, uintptr ptrOrOffset) const {
	Common::String &dest = *static_cast<Common::String *>(objectRef);
	switch (value.getType()) {
	case DynamicValueTypes::kString:
		dest = value.getString();
		return kMiniscriptInstructionOutcomeContinue;
	default:
		return kMiniscriptInstructionOutcomeFailed;
	}
}

MiniscriptInstructionOutcome DynamicValueWriteStringHelper::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) const {
	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome DynamicValueWriteStringHelper::refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) const {
	return kMiniscriptInstructionOutcomeFailed;
}

void DynamicValueWriteStringHelper::create(Common::String *strValue, DynamicValueWriteProxy &proxy) {
	proxy.pod.ptrOrOffset = 0;
	proxy.pod.objectRef = strValue;
	proxy.pod.ifc = &_instance;
}

DynamicValueWriteStringHelper DynamicValueWriteStringHelper::_instance;

MiniscriptInstructionOutcome DynamicValueWriteBoolHelper::write(MiniscriptThread *thread, const DynamicValue &value, void *objectRef, uintptr ptrOrOffset) const {
	bool &dest = *static_cast<bool *>(objectRef);
	switch (value.getType()) {
	case DynamicValueTypes::kBoolean:
		dest = value.getBool();
		return kMiniscriptInstructionOutcomeContinue;
	default:
		return kMiniscriptInstructionOutcomeFailed;
	}
}

MiniscriptInstructionOutcome DynamicValueWriteBoolHelper::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) const {
	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome DynamicValueWriteBoolHelper::refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) const {
	return kMiniscriptInstructionOutcomeFailed;
}

void DynamicValueWriteBoolHelper::create(bool *boolValue, DynamicValueWriteProxy &proxy) {
	proxy.pod.ptrOrOffset = 0;
	proxy.pod.objectRef = boolValue;
	proxy.pod.ifc = &_instance;
}

DynamicValueWriteBoolHelper DynamicValueWriteBoolHelper::_instance;

MiniscriptInstructionOutcome DynamicValueWriteObjectHelper::write(MiniscriptThread *thread, const DynamicValue &value, void *objectRef, uintptr ptrOrOffset) const {
	thread->error("Can't write to read-only object value");
	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome DynamicValueWriteObjectHelper::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) const {
	return static_cast<RuntimeObject *>(objectRef)->writeRefAttribute(thread, proxy, attrib);
}

MiniscriptInstructionOutcome DynamicValueWriteObjectHelper::refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) const {
	return static_cast<RuntimeObject *>(objectRef)->writeRefAttributeIndexed(thread, proxy, attrib, index);
}

void DynamicValueWriteObjectHelper::create(RuntimeObject *obj, DynamicValueWriteProxy &proxy) {
	proxy.containerList.reset();	// Object references are always anchored while threads are running, so don't need to preserve the container
	proxy.pod.ifc = &DynamicValueWriteObjectHelper::_instance;
	proxy.pod.objectRef = obj;
	proxy.pod.ptrOrOffset = 0;
}

DynamicValueWriteObjectHelper DynamicValueWriteObjectHelper::_instance;

MessengerSendSpec::MessengerSendSpec() : destination(0), _linkType(kLinkTypeNotYetLinked) {
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
					resolvedModifierDest = resolution.staticCast<Modifier>();
					_linkType = kLinkTypeModifier;
				} else if (resolution->isStructural()) {
					resolvedStructuralDest = resolution.staticCast<Structural>();
					_linkType = kLinkTypeStructural;
				} else {
					_linkType = kLinkTypeUnresolved;
				}
			} else {
				_linkType = kLinkTypeUnresolved;
			}
		} break;
	}
}

void MessengerSendSpec::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	visitor->visitWeakModifierRef(resolvedModifierDest);
	visitor->visitWeakStructuralRef(resolvedStructuralDest);
}

void MessengerSendSpec::resolveDestination(Runtime *runtime, Modifier *sender, Common::WeakPtr<Structural> &outStructuralDest, Common::WeakPtr<Modifier> &outModifierDest) const {
	outStructuralDest.reset();
	outModifierDest.reset();

	if (_linkType == kLinkTypeModifier) {
		outModifierDest = resolvedModifierDest;
	} else if (_linkType == kLinkTypeStructural) {
		outStructuralDest = resolvedStructuralDest;
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
		case kMessageDestChildren:
		case kMessageDestSubsection:
		case kMessageDestSourcesParent:
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
		warning("Couldn't resolve mesenger destination");
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

void MessengerSendSpec::sendFromMessenger(Runtime *runtime, Modifier *sender, const DynamicValue &incomingData) const {
	if (this->with.getType() == DynamicValueTypes::kIncomingData)
		sendFromMessengerWithCustomData(runtime, sender, incomingData);
	else
		sendFromMessengerWithCustomData(runtime, sender, this->with);
}

void MessengerSendSpec::sendFromMessengerWithCustomData(Runtime *runtime, Modifier *sender, const DynamicValue &data) const {
	Common::SharedPtr<MessageProperties> props(new MessageProperties(this->send, data, sender->getSelfReference()));

	Common::WeakPtr<Modifier> modifierDestRef;
	Common::WeakPtr<Structural> structuralDestRef;

	resolveDestination(runtime, sender, structuralDestRef, modifierDestRef);

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

Event Event::create() {
	Event evt;
	evt.eventInfo = 0;
	evt.eventType = EventIDs::kNothing;

	return evt;
}

Event Event::create(EventIDs::EventID eventType, uint32 eventInfo) {
	Event evt;
	evt.eventType = eventType;
	evt.eventInfo = eventInfo;

	return evt;
}

bool Event::respondsTo(const Event &otherEvent) const {
	return (*this) == otherEvent;
}

bool Event::load(const Data::Event &data) {
	eventType = static_cast<EventIDs::EventID>(data.eventID);
	eventInfo = data.eventInfo;

	return true;
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

bool VarReference::resolveContainer(IModifierContainer *modifierContainer, Common::WeakPtr<RuntimeObject> &outObject) const {
	for (const Common::SharedPtr<Modifier> &modifier : modifierContainer->getModifiers())
		if (resolveSingleModifier(modifier.get(), outObject))
			return true;

	return false;
}

bool VarReference::resolveSingleModifier(Modifier *modifier, Common::WeakPtr<RuntimeObject> &outObject) const {
	if (modifier->getStaticGUID() == guid || (source && caseInsensitiveEqual(modifier->getName(), *source))) {
		outObject = modifier->getSelfReference();
		return true;
	}

	return false;
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

void IPlugInModifierRegistrar::registerPlugInModifier(const char *name, const IPlugInModifierFactoryAndDataFactory *loaderFactory) {
	return this->registerPlugInModifier(name, loaderFactory, loaderFactory);
}

PlugIn::~PlugIn() {
}

ProjectResources::~ProjectResources() {
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

ProjectDescription::ProjectDescription(ProjectPlatform platform) : _language(Common::EN_ANY), _platform(platform) {
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

const Common::Array<Common::SharedPtr<Modifier> >& SimpleModifierContainer::getModifiers() const {
	return _modifiers;
}

void SimpleModifierContainer::appendModifier(const Common::SharedPtr<Modifier> &modifier) {
	_modifiers.push_back(modifier);
	if (modifier)
		modifier->setParent(nullptr);
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
	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome RuntimeObject::writeRefAttributeIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib, const DynamicValue &index) {
	return kMiniscriptInstructionOutcomeFailed;
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

WorldManagerInterface::WorldManagerInterface() {
}

bool WorldManagerInterface::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "currentscene") {
		Common::SharedPtr<RuntimeObject> mainScene = thread->getRuntime()->getActiveMainScene();
		if (mainScene)
			result.setObject(mainScene->getSelfReference());
		else
			result.clear();
		return true;
	}

	return RuntimeObject::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome WorldManagerInterface::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "currentscene") {
		DynamicValueWriteFuncHelper<WorldManagerInterface, &WorldManagerInterface::setCurrentScene>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "refreshcursor") {
		DynamicValueWriteFuncHelper<WorldManagerInterface, &WorldManagerInterface::setRefreshCursor>::create(this, result);
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

	thread->getRuntime()->addSceneStateTransition(HighLevelSceneTransition(scene->getSelfReference().lock().staticCast<Structural>(), HighLevelSceneTransition::kTypeChangeToScene, false, false));

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome WorldManagerInterface::setRefreshCursor(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kBoolean)
		return kMiniscriptInstructionOutcomeFailed;

	if (value.getBool())
		thread->getRuntime()->forceCursorRefreshOnce();

	return kMiniscriptInstructionOutcomeContinue;
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
		DynamicValueWriteFuncHelper<SystemInterface, &SystemInterface::setEjectCD>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "gamemode") {
		DynamicValueWriteFuncHelper<SystemInterface, &SystemInterface::setGameMode>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "mastervolume") {
		DynamicValueWriteFuncHelper<SystemInterface, &SystemInterface::setMasterVolume>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "monitorbitdepth") {
		DynamicValueWriteFuncHelper<SystemInterface, &SystemInterface::setMonitorBitDepth>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "volumename") {
		DynamicValueWriteFuncHelper<SystemInterface, &SystemInterface::setVolumeName>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return RuntimeObject::writeRefAttribute(thread, result, attrib);
}

int32 SystemInterface::displayModeToBitDepth(ColorDepthMode displayMode) {
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

ColorDepthMode SystemInterface::bitDepthToDisplayMode(int32 bits) {
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

	const ColorDepthMode depthMode = SystemInterface::bitDepthToDisplayMode(asInteger);
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

Structural::Structural() : _parent(nullptr), _paused(false), _loop(false) {
}

Structural::~Structural() {
}

ProjectPresentationSettings::ProjectPresentationSettings() : width(640), height(480), bitsPerPixel(8) {
}

bool Structural::isStructural() const {
	return true;
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

		// Scene returns the scene of the Miniscript modifier, even though it's looked up
		// as if it's an element property, because it's treated like a keyword.
		RuntimeObject *possibleScene = thread->getModifier();
		while (possibleScene) {
			if (possibleScene->isModifier()) {
				possibleScene = static_cast<Modifier *>(possibleScene)->getParent().lock().get();
				continue;
			}

			if (possibleScene->isStructural()) {
				Structural *parent = static_cast<Structural *>(possibleScene)->getParent();
				if (parent->isSubsection())
					break;
				else
					possibleScene = parent;
			}
		}
		if (possibleScene)
			result.setObject(possibleScene->getSelfReference());
		else
			result.clear();
		return true;
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

	return RuntimeObject::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome Structural::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "name") {
		DynamicValueWriteStringHelper::create(&_name, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "paused") {
		DynamicValueWriteFuncHelper<Structural, &Structural::scriptSetPaused>::create(this, result);
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
	} else if (attrib == "parent") {
		// NOTE: Re-parenting objects is allowed by mTropolis but we don't currently support that.
		Structural *parent = getParent();
		if (parent) {
			DynamicValueWriteObjectHelper::create(getParent(), result);
			return kMiniscriptInstructionOutcomeContinue;
		} else {
			return kMiniscriptInstructionOutcomeFailed;
		}
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
		DynamicValueWriteFuncHelper<Structural, &Structural::scriptSetLoop>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return RuntimeObject::writeRefAttribute(thread, result, attrib);
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

VThreadState Structural::consumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	warning("Command type %i was ignored", msg->getEvent().eventType);
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
	if (!thread->getRuntime()->isAwaitingSceneTransition()) {
		Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(targetValue ? EventIDs::kPause : EventIDs::kUnpause, 0), DynamicValue(), getSelfReference()));
		Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
		thread->getRuntime()->sendMessageOnVThread(dispatch);
	}

	return kMiniscriptInstructionOutcomeYieldToVThreadNoRetry;
}

MiniscriptInstructionOutcome Structural::scriptSetLoop(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kBoolean)
		return kMiniscriptInstructionOutcomeFailed;

	_loop = value.getBool();

	return kMiniscriptInstructionOutcomeContinue;
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

	if (name.size() > 0)
		_nameToObject[toCaseInsensitive(name)] = object;
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


HighLevelSceneTransition::HighLevelSceneTransition(const Common::SharedPtr<Structural> &scene, Type type, bool addToDestinationScene, bool addToReturnList)
	: scene(scene), type(type), addToDestinationScene(addToDestinationScene), addToReturnList(addToReturnList) {
}

MessageDispatch::MessageDispatch(const Common::SharedPtr<MessageProperties> &msgProps, Structural *root, bool cascade, bool relay, bool couldBeCommand)
	: _cascade(cascade), _relay(relay), _terminated(false), _msg(msgProps), _isCommand(false) {
	if (couldBeCommand && EventIDs::isCommand(msgProps->getEvent().eventType)) {
		_isCommand = true;

		PropagationStack topEntry;
		topEntry.index = 0;
		topEntry.propagationStage = PropagationStack::kStageSendCommand;
		topEntry.ptr.structural = root;

		_propagationStack.push_back(topEntry);
	} else {
		PropagationStack topEntry;
		topEntry.index = 0;
		topEntry.propagationStage = PropagationStack::kStageSendToStructuralSelf;
		topEntry.ptr.structural = root;

		_propagationStack.push_back(topEntry);
	}
}

MessageDispatch::MessageDispatch(const Common::SharedPtr<MessageProperties> &msgProps, Modifier *root, bool cascade, bool relay, bool couldBeCommand)
	: _cascade(cascade), _relay(relay), _terminated(false), _msg(msgProps), _isCommand(false) {
	if (couldBeCommand && EventIDs::isCommand(msgProps->getEvent().eventType)) {
		_isCommand = true;
		// Can't actually send this so don't even bother.  Modifiers are not allowed to respond to commands.
	} else {
		PropagationStack topEntry;
		topEntry.index = 0;
		topEntry.propagationStage = PropagationStack::kStageSendToModifier;
		topEntry.ptr.modifier = root;

		_isCommand = (couldBeCommand && EventIDs::isCommand(msgProps->getEvent().eventType));

		_propagationStack.push_back(topEntry);
	}
}

bool MessageDispatch::isTerminated() const {
	return _terminated;
}

VThreadState MessageDispatch::continuePropagating(Runtime *runtime) {
	// By the point this function is called, continuePropagating has been re-posted to the VThread,
	// so any propagation state changed in this function will be handled after any VThread tasks
	// posted here.
	while (_propagationStack.size() > 0) {
		PropagationStack &stackTop = _propagationStack.back();

		switch (stackTop.propagationStage)
		{
		case PropagationStack::kStageSendToModifier: {
				Modifier *modifier = stackTop.ptr.modifier;
				_propagationStack.pop_back();

				// Handle the action in the VThread
				bool responds = modifier->respondsToEvent(_msg->getEvent());

				// Queue propagation to children, if any, when the VThread task is done
				if (responds && !_relay) {
					_terminated = true;
				} else {
					IModifierContainer *childContainer = modifier->getMessagePropagationContainer();
					if (childContainer && childContainer->getModifiers().size() > 0) {
						PropagationStack childPropagation;
						childPropagation.propagationStage = PropagationStack::kStageSendToModifierContainer;
						childPropagation.index = 0;
						childPropagation.ptr.modifierContainer = childContainer;
						_propagationStack.push_back(childPropagation);
					}
				}

				// Post to the message action itself to VThread
				if (responds) {
					debug(3, "Modifier %x '%s' consumed message (%i,%i)", modifier->getStaticGUID(), modifier->getName().c_str(), _msg->getEvent().eventType, _msg->getEvent().eventInfo);
					runtime->postConsumeMessageTask(modifier, _msg);
					return kVThreadReturn;
				}
			} break;
		case PropagationStack::kStageSendToModifierContainer: {
				IModifierContainer *container = stackTop.ptr.modifierContainer;
				const Common::Array<Common::SharedPtr<Modifier> > &children = container->getModifiers();
				if (stackTop.index >= children.size()) {
					_propagationStack.pop_back();
				} else {
					Common::SharedPtr<Modifier> target = children[stackTop.index++];

					PropagationStack modifierPropagation;
					modifierPropagation.propagationStage = PropagationStack::kStageSendToModifier;
					modifierPropagation.index = 0;
					modifierPropagation.ptr.modifier = target.get();
					_propagationStack.push_back(modifierPropagation);
				}
			} break;
		case PropagationStack::kStageSendToStructuralChildren: {
				Structural *structural = stackTop.ptr.structural;
				const Common::Array<Common::SharedPtr<Structural> > &children = structural->getChildren();
				if (stackTop.index >= children.size()) {
					_propagationStack.pop_back();
				} else {
					PropagationStack childPropagation;
					childPropagation.propagationStage = PropagationStack::kStageSendToStructuralSelf;
					childPropagation.index = 0;
					childPropagation.ptr.structural = children[stackTop.index++].get();
					_propagationStack.push_back(childPropagation);
				}
			} break;
		case PropagationStack::kStageSendToStructuralSelf: {
				Structural *structural = stackTop.ptr.structural;
				stackTop.propagationStage = PropagationStack::kStageSendToStructuralModifiers;
				stackTop.index = 0;
				stackTop.ptr.structural = structural;

				bool responds = structural->respondsToEvent(_msg->getEvent());

				if (responds && !_relay) {
					_terminated = true;
				}

				if (responds)
					runtime->postConsumeMessageTask(structural, _msg);

				return kVThreadReturn;
			} break;
		case PropagationStack::kStageSendCommand: {
				Structural *structural = stackTop.ptr.structural;
				_propagationStack.pop_back();
				_terminated = true;

				runtime->postConsumeCommandTask(structural, _msg);

				return kVThreadReturn;
			} break;
		case PropagationStack::kStageSendToStructuralModifiers: {
				Structural *structural = stackTop.ptr.structural;

				// Once done with modifiers, propagate to children if set to cascade
				if (_cascade) {
					stackTop.propagationStage = PropagationStack::kStageSendToStructuralChildren;
					stackTop.index = 0;
					stackTop.ptr.structural = structural;
				} else {
					_propagationStack.pop_back();
				}

				if (structural->getModifiers().size() > 0) {
					PropagationStack modifierContainerPropagation;
					modifierContainerPropagation.propagationStage = PropagationStack::kStageSendToModifierContainer;
					modifierContainerPropagation.index = 0;
					modifierContainerPropagation.ptr.modifierContainer = structural;
					_propagationStack.push_back(modifierContainerPropagation);
				}
			} break;
		default:
			return kVThreadError;
		}
	}

	_terminated = true;

	return kVThreadReturn;
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


RuntimeObject *MessageDispatch::getRootPropagator() const {
	if (_propagationStack.size() > 0) {
		const PropagationStack &lowest = _propagationStack[0];
		switch (lowest.propagationStage) {
		case PropagationStack::kStageSendToModifier:
			return lowest.ptr.modifier;
		case PropagationStack::kStageSendCommand:
		case PropagationStack::kStageSendToStructuralChildren:
		case PropagationStack::kStageSendToStructuralModifiers:
		case PropagationStack::kStageSendToStructuralSelf:
			return lowest.ptr.structural;
		default:
			break;
		}
	}

	return nullptr;
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
	for (Common::Array<Common::SharedPtr<ScheduledEvent>>::iterator it = _events.begin(), itEnd = _events.end(); it != itEnd; ++it) {
		it->get()->_scheduler = nullptr;
	}

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

Runtime::SceneStackEntry::SceneStackEntry() {
}

Runtime::Runtime(OSystem *system, Audio::Mixer *mixer, ISaveUIProvider *saveProvider, ILoadUIProvider *loadProvider)
	: _system(system), _mixer(mixer), _saveProvider(saveProvider), _loadProvider(loadProvider),
	_nextRuntimeGUID(1), _realDisplayMode(kColorDepthModeInvalid), _fakeDisplayMode(kColorDepthModeInvalid),
	_displayWidth(1024), _displayHeight(768), _realTimeBase(0), _playTimeBase(0), _sceneTransitionState(kSceneTransitionStateNotTransitioning),
	_lastFrameCursor(nullptr), _defaultCursor(new DefaultCursorGraphic()), _platform(kProjectPlatformUnknown),
	_cachedMousePosition(Point16::create(0, 0)), _realMousePosition(Point16::create(0, 0)), _trackedMouseOutside(false),
	_forceCursorRefreshOnce(true), _haveModifierOverrideCursor(false) {
	_random.reset(new Common::RandomSource("mtropolis"));

	_vthread.reset(new VThread());

	for (int i = 0; i < kColorDepthModeCount; i++) {
		_displayModeSupported[i] = false;
		_realDisplayMode = kColorDepthModeInvalid;
		_fakeDisplayMode = kColorDepthModeInvalid;
	}

	_realTimeBase = system->getMillis();
	_playTimeBase = system->getMillis();

	for (int i = 0; i < Actions::kMouseButtonCount; i++)
		_mouseFocusFlags[Actions::kMouseButtonCount] = false;
	
	_worldManagerInterface.reset(new WorldManagerInterface());
	_worldManagerInterface->setSelfReference(_worldManagerInterface);

	_assetManagerInterface.reset(new AssetManagerInterface());
	_assetManagerInterface->setSelfReference(_assetManagerInterface);

	_systemInterface.reset(new SystemInterface());
	_systemInterface->setSelfReference(_systemInterface);
}

bool Runtime::runFrame() {
	uint32 timeMillis = _system->getMillis();

	uint32 realMSec = timeMillis - _realTimeBase - _realTime;
	uint32 playMSec = timeMillis - _playTimeBase - _playTime;

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

		if (_osEventQueue.size() > 0) {
			Common::SharedPtr<OSEvent> evt = _osEventQueue[0];
			_osEventQueue.remove_at(0);

			OSEventType evtType = evt->getEventType();
			switch (evtType)
			{
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
				}
				break;
			case kOSEventTypeMouseDown:
			case kOSEventTypeMouseUp:
			case kOSEventTypeMouseMove: {
					MouseInputEvent *mouseEvt = static_cast<MouseInputEvent *>(evt.get());

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
		}

		if (_queuedProjectDesc) {
			Common::SharedPtr<ProjectDescription> desc = _queuedProjectDesc;
			_queuedProjectDesc.reset();

			unloadProject();

			_macFontMan.reset(new Graphics::MacFontManager(0, desc->getLanguage()));

			_project.reset(new Project(this));
			_project->setSelfReference(_project);

			_project->loadFromDescription(*desc);

			ensureMainWindowExists();

			_rootLinkingScope.addObject(_project->getStaticGUID(), _project->getName(), _project);

			// We have to materialize global variables because they are not cloned from aliases.
			debug(1, "Materializing global variables...");
			_project->materializeGlobalVariables(this, &_rootLinkingScope);

			debug(1, "Materializing project...");
			_project->materializeSelfAndDescendents(this, &_rootLinkingScope);

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

			Common::SharedPtr<MessageProperties> psProps(new MessageProperties(Event::create(EventIDs::kProjectStarted, 0), DynamicValue(), _project->getSelfReference()));
			Common::SharedPtr<MessageDispatch> psDispatch(new MessageDispatch(psProps, _project.get(), false, true, false));
			queueMessage(psDispatch);

			_pendingSceneTransitions.push_back(HighLevelSceneTransition(firstSubsection->getChildren()[1], HighLevelSceneTransition::kTypeChangeToScene, false, false));
			continue;
		}

		if (_messageQueue.size() > 0) {
			Common::SharedPtr<MessageDispatch> msg = _messageQueue[0];
			_messageQueue.remove_at(0);

			sendMessageOnVThread(msg);
			continue;
		}

		// Teardowns must only occur during idle conditions where there are no queued message and no VThread tasks
		if (_pendingTeardowns.size() > 0) {
			for (Common::Array<Teardown>::const_iterator it = _pendingTeardowns.begin(), itEnd = _pendingTeardowns.end(); it != itEnd; ++it) {
				executeTeardown(*it);
			}
			_pendingTeardowns.clear();
			continue;
		}

		if (_pendingLowLevelTransitions.size() > 0) {
			LowLevelSceneStateTransitionAction transition = _pendingLowLevelTransitions[0];
			_pendingLowLevelTransitions.remove_at(0);

			executeLowLevelSceneStateTransition(transition);
			continue;
		}

		if (_pendingSceneTransitions.size() > 0) {
			HighLevelSceneTransition transition = _pendingSceneTransitions[0];
			_pendingSceneTransitions.remove_at(0);

			executeHighLevelSceneTransition(transition);
			continue;
		}

		if (_sceneTransitionState == kSceneTransitionStateWaitingForDraw) {
			if (_sceneTransitionEffect.duration == 0) {
				// This needs to skip past the transition phase and hit the next condition
				_sceneTransitionEndTime = _playTime;
				_sceneTransitionState = kSceneTransitionStateTransitioning;
			} else {
				_sceneTransitionState = kSceneTransitionStateDrawingTargetFrame;
				_sceneTransitionEndTime = _playTime + _sceneTransitionEffect.duration / 10;
			}
		}

		if (_sceneTransitionState == kSceneTransitionStateTransitioning && _playTime >= _sceneTransitionEndTime) {
			_sceneTransitionState = kSceneTransitionStateNotTransitioning;
			queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kSceneTransitionEnded, 0), _activeMainScene.get(), true, true);
			continue;
		}

		{
			Common::SharedPtr<ScheduledEvent> firstScheduledEvent = _scheduler.getFirstEvent();
			if (firstScheduledEvent && firstScheduledEvent->getScheduledTime() <= _playTime) {
				_scheduler.descheduleFirstEvent();

				firstScheduledEvent->activate(this);
				continue;
			}
		}

		// Ran out of actions
		break;
	}

#ifdef MTROPOLIS_DEBUG_ENABLE
	if (_debugger)
		_debugger->runFrame(realMSec);

#endif

	// Frame completed
	return true;
}

struct WindowSortingBucket
{
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

	{
		Common::SharedPtr<Window> mainWindow = _mainWindow.lock();
		if (mainWindow)
			Render::renderProject(this, mainWindow.get());
	}

	const size_t numWindows = _windows.size();
	WindowSortingBucket singleBucket;
	Common::Array<WindowSortingBucket> multipleBuckets;
	WindowSortingBucket *sortedBuckets = &singleBucket;

	if (numWindows < 2)
		sortedBuckets = &singleBucket;
	else {
		multipleBuckets.resize(numWindows);
		sortedBuckets = &multipleBuckets[0];
	}

	for (size_t i = 0; i < numWindows; i++) {
		sortedBuckets[i].originalIndex = i;
		sortedBuckets[i].window = _windows[i].get();
	}

	Common::sort(sortedBuckets, sortedBuckets + numWindows, WindowSortingBucket::sortPredicate);
	
	for (size_t i = 0; i < numWindows; i++) {
		const Window &window = *sortedBuckets[i].window;
		const Graphics::ManagedSurface &surface = *window.getSurface();

		int32 destLeft = window.getX();
		int32 destRight = destLeft + surface.w;
		int32 destTop = window.getY();
		int32 destBottom = destTop + surface.h;

		int32 srcLeft = 0;
		int32 srcRight = surface.w;
		int32 srcTop = 0;
		int32 srcBottom = surface.h;

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
			srcRight += rightAdjust;
		}

		if (destBottom > height) {
			int bottomAdjust = height - destBottom;
			destBottom += bottomAdjust;
			srcBottom += bottomAdjust;
		}

		if (destLeft >= destRight || destTop >= destBottom || destLeft >= width || destTop >= height || destRight <= 0 || destBottom <= 0)
			continue;

		_system->copyRectToScreen(surface.getBasePtr(srcLeft, srcTop), surface.pitch, destLeft, destTop, destRight - destLeft, destBottom - destTop);
	}

	_system->updateScreen();

	Common::SharedPtr<CursorGraphic> cursor;
	
	Common::SharedPtr<Window> focusWindow = _mouseFocusWindow.lock();

	if (!focusWindow)
		focusWindow = findTopWindow(_realMousePosition.x, _realMousePosition.y);

	if (focusWindow)
		cursor = focusWindow->getCursorGraphic();

	if (!cursor)
		cursor = _defaultCursor;

	if (cursor != _lastFrameCursor) {
		CursorMan.showMouse(true);
		CursorMan.replaceCursor(cursor->getCursor());

		_lastFrameCursor = cursor;
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
	Common::SharedPtr<Structural> structural = teardown.structural.lock();
	if (!structural)
		return;	// Already gone

	recursiveDeactivateStructural(structural.get());

	if (teardown.onlyRemoveChildren) {
		structural->removeAllChildren();
		structural->removeAllModifiers();
		structural->removeAllAssets();
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

void Runtime::executeLowLevelSceneStateTransition(const LowLevelSceneStateTransitionAction &action) {
	switch (action.getActionType())
	{
	case LowLevelSceneStateTransitionAction::kSendMessage:
		sendMessageOnVThread(action.getMessage());
		break;
	case LowLevelSceneStateTransitionAction::kLoad:
		loadScene(action.getScene());
		break;
	case LowLevelSceneStateTransitionAction::kUnload: {
			Teardown teardown;
			teardown.onlyRemoveChildren = true;
			teardown.structural = action.getScene();

			_pendingTeardowns.push_back(teardown);
		} break;
	default:
		assert(false);
		break;
	}
}

void Runtime::executeCompleteTransitionToScene(const Common::SharedPtr<Structural> &targetScene) {
	if (targetScene == _activeMainScene)
		return;

	if (_sceneStack.size() == 0)
		_sceneStack.resize(1);	// Reserve shared scene slot

	Common::SharedPtr<Structural> targetSharedScene = findDefaultSharedSceneForScene(targetScene.get());

	if (targetScene == targetSharedScene)
		error("Transitioned into a default shared scene, this is not supported");

	if (_activeMainScene == targetSharedScene)
		error("Transitioned into scene currently being used as a target scene, this is not supported");

	bool sceneAlreadyInStack = false;
	for (size_t i = _sceneStack.size() - 1; i > 0; i--) {
		Common::SharedPtr<Structural> stackedScene = _sceneStack[i].scene;
		if (stackedScene == targetScene) {
			sceneAlreadyInStack = true;
		} else {
			queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kSceneEnded, 0), _activeMainScene.get(), false, true);
			queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kParentDisabled, 0), _activeMainScene.get(), true, true);
			_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(_activeMainScene, LowLevelSceneStateTransitionAction::kUnload));

			if (stackedScene == targetSharedScene)
				error("Transitioned to a shared scene which was already on the stack as a normal scene.  This is not supported.");

			_sceneStack.remove_at(i);
		}
	}

	if (targetSharedScene != _activeSharedScene) {
		if (_activeSharedScene) {
			queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kSceneEnded, 0), _activeSharedScene.get(), false, true);
			queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kParentDisabled, 0), _activeSharedScene.get(), true, true);
			_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(_activeSharedScene, LowLevelSceneStateTransitionAction::kUnload));
		}

		_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(targetSharedScene, LowLevelSceneStateTransitionAction::kLoad));
		queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kParentEnabled, 0), targetSharedScene.get(), true, true);
		queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kSceneStarted, 0), targetSharedScene.get(), false, true);

		SceneStackEntry sharedSceneEntry;
		sharedSceneEntry.scene = targetSharedScene;

		_sceneStack[0] = sharedSceneEntry;
	}

	if (!sceneAlreadyInStack) {
		_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(targetScene, LowLevelSceneStateTransitionAction::kLoad));
		queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kParentEnabled, 0), targetScene.get(), true, true);
		queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kSceneStarted, 0), targetScene.get(), false, true);

		SceneStackEntry sceneEntry;
		sceneEntry.scene = targetScene;

		_sceneStack.push_back(sceneEntry);
	}

	_activeMainScene = targetScene;
	_activeSharedScene = targetSharedScene;

	// Scene transitions have to be set up by the destination scene
	_sceneTransitionState = kSceneTransitionStateWaitingForDraw;
	_sceneTransitionEffect.transitionType = kTransitionTypeNone;
	_sceneTransitionEffect.duration = 0;

	executeSharedScenePostSceneChangeActions();
}

void Runtime::executeHighLevelSceneTransition(const HighLevelSceneTransition &transition) {
	if (_sceneStack.size() == 0)
		_sceneStack.resize(1); // Reserve shared scene slot

	// This replicates a bunch of quirks/bugs of mTropolis's scene transition logic,
	// see the accompanying notes file.  There are probably some missing cases related to
	// shared scene, calling return/scene transitions during scene deactivation, or other
	// edge cases that hopefully no games actually do!
	switch (transition.type) {
	case HighLevelSceneTransition::kTypeReturn: {
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
					queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kSceneEnded, 0), _activeMainScene.get(), false, true);
					queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kParentDisabled, 0), _activeMainScene.get(), true, true);
					_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(_activeMainScene, LowLevelSceneStateTransitionAction::kUnload));

					queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kSceneReactivated, 0), sceneReturn.scene.get(), false, true);

					_activeMainScene = sceneReturn.scene;

					executeSharedScenePostSceneChangeActions();
				} else {
					executeCompleteTransitionToScene(sceneReturn.scene);
				}
			}
		} break;
	case HighLevelSceneTransition::kTypeChangeToScene: {
			const Common::SharedPtr<Structural> targetScene = transition.scene;

			if (transition.addToDestinationScene || !transition.addToReturnList) {
				SceneReturnListEntry returnListEntry;
				returnListEntry.isAddToDestinationSceneTransition = transition.addToDestinationScene;
				returnListEntry.scene = _activeMainScene;
				_sceneReturnList.push_back(returnListEntry);
			}

			if (transition.addToDestinationScene) {
				if (targetScene != _activeMainScene) {
					Common::SharedPtr<Structural> targetSharedScene = findDefaultSharedSceneForScene(targetScene.get());

					if (targetScene == targetSharedScene)
						error("Transitioned into a default shared scene, this is not supported");

					if (_activeMainScene == targetSharedScene)
						error("Transitioned into scene currently being used as a target scene, this is not supported");

					queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kSceneDeactivated, 0), _activeMainScene.get(), false, true);

					if (targetSharedScene != _activeSharedScene) {
						if (_activeSharedScene) {
							queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kSceneEnded, 0), _activeSharedScene.get(), false, true);
							queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kParentDisabled, 0), _activeSharedScene.get(), true, true);
							_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(_activeSharedScene, LowLevelSceneStateTransitionAction::kUnload));
						}

						_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(targetSharedScene, LowLevelSceneStateTransitionAction::kLoad));
						queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kParentEnabled, 0), targetSharedScene.get(), true, true);
						queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kSceneStarted, 0), targetSharedScene.get(), false, true);

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
						queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kParentEnabled, 0), targetScene.get(), true, true);
						queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kSceneStarted, 0), targetScene.get(), false, true);

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
	default:
		break;
	}
}

void Runtime::executeSharedScenePostSceneChangeActions() {
	Structural *subsection = _activeMainScene->getParent();

	const Common::Array<Common::SharedPtr<Structural> > &subsectionScenes = subsection->getChildren();

	queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kSharedSceneSceneChanged, 0), _activeSharedScene.get(), false, true);
	if (subsectionScenes.size() > 1) {
		if (_activeMainScene == subsectionScenes[subsectionScenes.size() - 1])
			queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kSharedSceneNoNextScene, 0), _activeSharedScene.get(), false, true);
		if (_activeMainScene == subsectionScenes[1])
			queueEventAsLowLevelSceneStateTransitionAction(Event::create(EventIDs::kSharedSceneNoPrevScene, 0), _activeSharedScene.get(), false, true);
	}
}

void Runtime::recursiveDeactivateStructural(Structural *structural) {
	for (const Common::SharedPtr<Structural> &child : structural->getChildren()) {
		recursiveDeactivateStructural(child.get());
	}

	structural->deactivate();
}

void Runtime::recursiveActivateStructural(Structural *structural) {
	structural->activate();

	for (const Common::SharedPtr<Structural> &child : structural->getChildren()) {
		recursiveActivateStructural(child.get());
	}
}

bool Runtime::isStructuralMouseInteractive(Structural *structural) {
	for (const Common::SharedPtr<Modifier> &modifier : structural->getModifiers()) {
		if (isModifierMouseInteractive(modifier.get()))
			return true;
	}

	return false;
}

bool Runtime::isModifierMouseInteractive(Modifier *modifier) {
	const EventIDs::EventID evtIDs[] = {
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

	for (EventIDs::EventID evtID : evtIDs) {
		if (modifier->respondsToEvent(Event::create(evtID, 0)))
			return true;
	}

	IModifierContainer *propagationContainer = modifier->getMessagePropagationContainer();
	if (propagationContainer) {
		for (const Common::SharedPtr<Modifier> &child : propagationContainer->getModifiers()) {
			if (isModifierMouseInteractive(child.get()))
				return true;
		}
	}

	return false;
}

void Runtime::recursiveFindMouseCollision(Structural *&bestResult, int &bestLayer, int &bestStackHeight, Structural *candidate, int stackHeight, int32 relativeX, int32 relativeY) {
	int32 childRelativeX = relativeX;
	int32 childRelativeY = relativeY;
	if (candidate->isElement()) {
		Element *element = static_cast<Element *>(candidate);
		if (element->isVisual()) {
			VisualElement *visual = static_cast<VisualElement *>(candidate);
			int layer = visual->getLayer();

			// Weird layering behavior:
			// Objects in a higher layer in lower scenes still have higher render order, so they're on top
			const bool isInFront = (layer > bestLayer) || (layer == bestLayer && stackHeight > bestStackHeight);

			if (isInFront && visual->isMouseInsideBox(relativeX, relativeY) && isStructuralMouseInteractive(visual) && visual->isMouseCollisionAtPoint(relativeX, relativeY)) {
				bestResult = candidate;
				bestLayer = layer;
				bestStackHeight = stackHeight;
			}

			childRelativeX -= visual->getRelativeRect().left;
			childRelativeY -= visual->getRelativeRect().top;
		}
	}


	for (const Common::SharedPtr<Structural> &child : candidate->getChildren()) {
		recursiveFindMouseCollision(bestResult, bestLayer, bestStackHeight, child.get(), stackHeight, childRelativeX, childRelativeY);
	}
}

void Runtime::queueEventAsLowLevelSceneStateTransitionAction(const Event &evt, Structural *root, bool cascade, bool relay) {
	Common::SharedPtr<MessageProperties> props(new MessageProperties(evt, DynamicValue(), Common::WeakPtr<RuntimeObject>()));
	Common::SharedPtr<MessageDispatch> msg(new MessageDispatch(props, root, cascade, relay, false));
	_pendingLowLevelTransitions.push_back(LowLevelSceneStateTransitionAction(msg));
}

void Runtime::loadScene(const Common::SharedPtr<Structural>& scene) {
	debug(1, "Loading scene '%s'", scene->getName().c_str());
	Element *element = static_cast<Element *>(scene.get());
	uint32 streamID = element->getStreamLocator() & 0xffff; // Not actually sure how many bits are legal here

	Subsection *subsection = static_cast<Subsection *>(scene->getParent());

	_project->loadSceneFromStream(scene, streamID);
	debug(1, "Scene loaded OK, materializing objects...");
	scene->materializeDescendents(this, subsection->getSceneLoadMaterializeScope());
	debug(1, "Scene materialized OK");
	recursiveActivateStructural(scene.get());
	debug(1, "Structural elements activated OK");

#ifdef MTROPOLIS_DEBUG_ENABLE
	if (_debugger) {
		_debugger->complainAboutUnfinished(scene.get());
		_debugger->refreshSceneStatus();
	}
#endif

	refreshPlayTime();
}

void Runtime::sendMessageOnVThread(const Common::SharedPtr<MessageDispatch> &dispatch) {
#ifndef DISABLE_TEXT_CONSOLE
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
			extType = "Motion Ended";
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
			extType = "Mouse Up Outside";
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
		switch (payload.getType())
		{
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
		default:
			valueStr = "<BAD TYPE> (this is a bug!)";
			break;
		}

		valueStr = " with value " + valueStr;
	}

	debug(3, "Object %x '%s' posted message %s to %x '%s'%s  mod: %s   ele: %s", srcID, nameStr, msgDebugString.c_str(), destID, destStr, valueStr.c_str(), relay ? "all" : "first", cascade ? "all" : "targetOnly");
#endif

	DispatchMethodTaskData *taskData = _vthread->pushTask("Runtime::dispatchMessageTask", this, &Runtime::dispatchMessageTask);
	taskData->dispatch = dispatch;
}

VThreadState Runtime::dispatchMessageTask(const DispatchMethodTaskData &data) {
	Common::SharedPtr<MessageDispatch> dispatchPtr = data.dispatch;
	MessageDispatch &dispatch = *dispatchPtr.get();

	if (dispatch.isTerminated())
		return kVThreadReturn;
	else {
		// Requeue propagation after whatever happens with this propagation step
		DispatchMethodTaskData *requeueData = _vthread->pushTask("Runtime::dispatchMessageTask", this, &Runtime::dispatchMessageTask);
		requeueData->dispatch = dispatchPtr;

		return dispatch.continuePropagating(this);
	}
}

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

VThreadState Runtime::consumeMessageTask(const ConsumeMessageTaskData &data) {
	IMessageConsumer *consumer = data.consumer;
	assert(consumer->respondsToEvent(data.message->getEvent()));
	return consumer->consumeMessage(this, data.message);
}

VThreadState Runtime::consumeCommandTask(const ConsumeCommandTaskData &data) {
	Structural *structural = data.structural;
	return structural->consumeCommand(this, data.message);
}

VThreadState Runtime::updateMouseStateTask(const UpdateMouseStateTaskData &data) {
	struct MessageToSend {
		EventIDs::EventID eventID;
		Structural *target;
	};

	Common::Array<MessageToSend> messagesToSend;

	if (data.mouseDown) {
		// Mouse down
		Common::SharedPtr<Structural> tracked = _mouseOverObject.lock();
		if (tracked) {
			_mouseTrackingObject = tracked;
			_mouseTrackingDragStart = _cachedMousePosition;
			if (tracked->isElement() && static_cast<Element *>(tracked.get())->isVisual()) {
				Rect16 initialRect = static_cast<VisualElement *>(tracked.get())->getRelativeRect();
				_mouseTrackingObjectInitialOrigin = Point16::create(initialRect.left, initialRect.top);
			} else
				_mouseTrackingObjectInitialOrigin = Point16::create(0, 0);
			_trackedMouseOutside = false;

			MessageToSend msg;
			msg.eventID = EventIDs::kMouseDown;
			msg.target = tracked.get();
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
	mousePtValue.setPoint(Point16::create(_cachedMousePosition.x, _cachedMousePosition.y));

	for (size_t ri = 0; ri < messagesToSend.size(); ri++) {
		const MessageToSend &msg = messagesToSend[messagesToSend.size() - 1 - ri];
		Common::SharedPtr<MessageProperties> props(new MessageProperties(Event::create(msg.eventID, 0), mousePtValue, nullptr));
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
	int bestSceneStack = INT_MIN;
	int bestLayer = INT_MIN;

	for (size_t ri = 0; ri < _sceneStack.size(); ri++) {
		const SceneStackEntry &sceneStackEntry = _sceneStack[_sceneStack.size() - 1 - ri];
		recursiveFindMouseCollision(collisionItem, bestSceneStack, bestLayer, sceneStackEntry.scene.get(), _sceneStack.size() - 1 - ri, data.x, data.y);
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
		Point16 parentOrigin = visual->getParentOrigin();
		int32 relativeX = data.x - parentOrigin.x;
		int32 relativeY = data.y - parentOrigin.y;
		bool mouseOutside = !visual->isMouseInsideBox(relativeX, relativeY) || !visual->isMouseCollisionAtPoint(relativeX, relativeY);

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
			Point16 targetPoint = Point16::create(data.x - _mouseTrackingDragStart.x + _mouseTrackingObjectInitialOrigin.x, data.y - _mouseTrackingDragStart.y + _mouseTrackingObjectInitialOrigin.y);
			static_cast<VisualElement *>(element)->handleDragMotion(this, _mouseTrackingObjectInitialOrigin, targetPoint);
		}
	}

	DynamicValue mousePtValue;
	mousePtValue.setPoint(Point16::create(data.x, data.y));

	for (size_t ri = 0; ri < messagesToSend.size(); ri++) {
		const MessageToSend &msg = messagesToSend[messagesToSend.size() - 1 - ri];
		Common::SharedPtr<MessageProperties> props(new MessageProperties(Event::create(msg.eventID, 0), mousePtValue, nullptr));
		Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(props, msg.target, false, true, false));
		sendMessageOnVThread(dispatch);
	}

	_cachedMousePosition.x = data.x;
	_cachedMousePosition.y = data.y;

	return kVThreadReturn;
}

void Runtime::updateMainWindowCursor() {
	const uint32 kHandPointUpID = 10005;
	const uint32 kArrowID = 10011;

	if (!_mainWindow.expired()) {
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

		if (!modifier->isVariable()) {
			Common::SharedPtr<Modifier> clonedModifier = templateModifier->shallowClone();
			clonedModifier->setSelfReference(clonedModifier);
			clonedModifier->setRuntimeGUID(allocateRuntimeGUID());

			clonedModifier->setName(modifier->getName());

			modifier = clonedModifier;
			clonedModifier->setParent(relinkParent);

			ModifierChildCloner cloner(this, clonedModifier);
			clonedModifier->visitInternalReferences(&cloner);
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

ProjectPlatform Runtime::getPlatform() const {
	return _platform;
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

const Point16 &Runtime::getCachedMousePosition() const {
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


void Runtime::ensureMainWindowExists() {
	// Maybe there's a better spot for this
	if (_mainWindow.expired() && _project) {
		const ProjectPresentationSettings &presentationSettings = _project->getPresentationSettings();

		int32 centeredX = (static_cast<int32>(_displayWidth) - static_cast<int32>(presentationSettings.width)) / 2;
		int32 centeredY = (static_cast<int32>(_displayHeight) - static_cast<int32>(presentationSettings.height)) / 2;
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
	_vthread.reset(new VThread());

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

	return false;
}

void Runtime::addSceneStateTransition(const HighLevelSceneTransition &transition) {
	_pendingSceneTransitions.push_back(transition);
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
};

ChildLoaderContext::ChildLoaderContext() : remainingCount(0), type(kTypeUnknown) {
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

void KeyboardEventSignaller::onKeyboardEvent(Runtime *runtime, Common::EventType evtType, bool repeat, const Common::KeyState &keyEvt) {
	const size_t numReceivers = _receivers.size();
	for (size_t i = 0; i < numReceivers; i++) {
		_receivers[i]->onKeyboardEvent(runtime, evtType, repeat, keyEvt);
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

Project::Segment::Segment() : weakStream(nullptr) {
}

Project::Project(Runtime *runtime)
	: _runtime(runtime), _projectFormat(Data::kProjectFormatUnknown), _isBigEndian(false),
	  _haveGlobalObjectInfo(false), _haveProjectStructuralDef(false), _playMediaSignaller(new PlayMediaSignaller()),
	  _keyboardEventSignaller(new KeyboardEventSignaller()) {
}

Project::~Project() {
	for (size_t i = 0; i < _segments.size(); i++)
		closeSegmentStream(i);
}

void Project::loadFromDescription(const ProjectDescription& desc) {
	_resources = desc.getResources();
	_cursorGraphics = desc.getCursorGraphics();

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
	uint16 startValue = baseStream->readUint16LE();

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

	Data::DataReader reader(2, stream, _projectFormat);

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

	debug(1, "Boot stream loaded successfully");
}

void Project::loadSceneFromStream(const Common::SharedPtr<Structural>& scene, uint32 streamID) {
	if (streamID == 0 || streamID > _streams.size()) {
		error("Invalid stream ID");
	}

	size_t streamIndex = streamID - 1;

	const StreamDesc &streamDesc = _streams[streamIndex];
	uint segmentIndex = streamDesc.segmentIndex;

	openSegmentStream(segmentIndex);

	Common::SeekableSubReadStreamEndian stream(_segments[segmentIndex].weakStream, streamDesc.pos, streamDesc.pos + streamDesc.size, _isBigEndian);
	Data::DataReader reader(streamDesc.pos, stream, _projectFormat);

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

	size_t numObjectsLoaded = 0;
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

	if ((loaderStack.contexts.size() == 1 && loaderStack.contexts[0].type != ChildLoaderContext::kTypeFilteredElements) || loaderStack.contexts.size() > 1) {
		error("Scene stream loader finished in an expected state, something didn't finish loading");
	}

	scene->holdAssets(assetDefLoader.assets);
	assignAssets(assetDefLoader.assets);
}

Common::SharedPtr<Modifier> Project::resolveAlias(uint32 aliasID) const {
	if (aliasID == 0 || aliasID > _globalModifiers.getModifiers().size())
		return Common::SharedPtr<Modifier>();

	return _globalModifiers.getModifiers()[aliasID - 1];
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

Common::WeakPtr<Asset> Project::getAssetByID(uint32 assetID) const {
	if (assetID >= _assetsByID.size())
		return Common::WeakPtr<Asset>();

	const AssetDesc *desc = _assetsByID[assetID];
	if (desc == nullptr)
		return Common::WeakPtr<Asset>();

	return desc->asset;
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
		Common::File *f = new Common::File();
		segment.rcStream.reset(f);
		segment.weakStream = f;

		if (!f->open(segment.desc.filePath)) {
			error("Failed to open segment file %s", segment.desc.filePath.c_str());
		}
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

Common::SharedPtr<SegmentUnloadSignaller> Project::notifyOnSegmentUnload(int segmentIndex, ISegmentUnloadSignalReceiver *receiver) {
	Common::SharedPtr<SegmentUnloadSignaller> signaller = _segments[segmentIndex].unloadSignaller;
	if (signaller)
		signaller->addReceiver(receiver);
	return signaller;
}

void Project::onPostRender() {
	_playMediaSignaller->playMedia(_runtime, this);
}

Common::SharedPtr<PlayMediaSignaller> Project::notifyOnPlayMedia(IPlayMediaSignalReceiver *receiver) {
	_playMediaSignaller->addReceiver(receiver);
	return _playMediaSignaller;
}

void Project::onKeyboardEvent(Runtime *runtime, const Common::EventType evtType, bool repeat, const Common::KeyState &keyEvt) {
	_keyboardEventSignaller->onKeyboardEvent(runtime, evtType, repeat, keyEvt);
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

void Project::loadBootStream(size_t streamIndex) {
	const StreamDesc &streamDesc = _streams[streamIndex];

	size_t segmentIndex = streamDesc.segmentIndex;
	openSegmentStream(segmentIndex);

	Common::SeekableSubReadStreamEndian stream(_segments[segmentIndex].weakStream, streamDesc.pos, streamDesc.pos + streamDesc.size, _isBigEndian);
	Data::DataReader reader(streamDesc.pos, stream, _projectFormat);

	ChildLoaderStack loaderStack;
	AssetDefLoaderContext assetDefLoader;

	const Data::PlugInModifierRegistry &plugInDataLoaderRegistry = _plugInRegistry.getDataLoaderRegistry();

	size_t numObjectsLoaded = 0;
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
				} break;
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

	if (loaderStack.contexts.size() != 1 || loaderStack.contexts[0].type != ChildLoaderContext::kTypeProject) {
		error("Boot stream loader finished in an expected state, something didn't finish loading");
	}

	holdAssets(assetDefLoader.assets);
	assignAssets(assetDefLoader.assets);
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
		IModifierFactory *factory = getModifierFactoryForDataObjectType(dataObject.getType());

		if (!factory)
			error("Unknown or unsupported modifier type, or non-modifier encountered where a modifier was expected");

		modifier = factory->createModifier(loaderContext, dataObject);
	}
	assert(modifier->getModifierFlags().flagsWereLoaded);
	if (!modifier)
		error("Modifier object failed to load");

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

void Project::assignAssets(const Common::Array<Common::SharedPtr<Asset> >& assets) {
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

		if (desc->asset.expired())
			desc->asset = asset;
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

				IElementFactory *elementFactory = getElementFactoryForDataObjectType(dataObjectType);
				if (!elementFactory) {
					error("No element factory defined for structural object");
				}

				ElementLoaderContext elementLoaderContext(_runtime, streamIndex);
				Common::SharedPtr<Element> element = elementFactory->createElement(elementLoaderContext, dataObject);

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

	IAssetFactory *factory = getAssetFactoryForDataObjectType(dataObject.getType());
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

ObjectLinkingScope *Subsection::getSceneLoadMaterializeScope() {
	return getPersistentStructuralScope();
}

ObjectLinkingScope *Subsection::getPersistentStructuralScope() {
	return &_structuralScope;
}

ObjectLinkingScope *Subsection::getPersistentModifierScope() {
	return &_modifierScope;
}

bool Element::isElement() const {
	return true;
}

uint32 Element::getStreamLocator() const {
	return _streamLocator;
}

VisualElement::VisualElement() : _rect(Rect16::create(0, 0, 0, 0)), _cachedAbsoluteOrigin(Point16::create(0, 0)) {
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

bool VisualElement::isDirectToScreen() const {
	return _directToScreen;
}

uint16 VisualElement::getLayer() const {
	return _layer;
}

bool VisualElement::isMouseInsideBox(int32 relativeX, int32 relativeY) const {
	return relativeX >= _rect.left && relativeX < _rect.right && relativeY >= _rect.top && relativeY < _rect.bottom;
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
		result.setPoint(Point16::create(_rect.left, _rect.top));
		return true;
	} else if (attrib == "centerposition") {
		result.setPoint(getCenterPosition());
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
		DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetVisibility>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "direct") {
		DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetDirect>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "position") {
		DynamicValueWriteOrRefAttribFuncHelper<VisualElement, &VisualElement::scriptSetPosition, &VisualElement::scriptWriteRefPositionAttribute>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "centerposition") {
		DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetCenterPosition>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "width") {
		DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetWidth>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "height") {
		DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetHeight>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "layer") {
		DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetLayer>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return Element::writeRefAttribute(thread, writeProxy, attrib);
}

const Rect16 &VisualElement::getRelativeRect() const {
	return _rect;
}

Point16 VisualElement::getParentOrigin() const {
	Point16 pos = Point16::create(0, 0);
	if (_parent && _parent->isElement()) {
		Element *element = static_cast<Element *>(_parent);
		if (element->isVisual()) {
			pos = static_cast<VisualElement *>(element)->getGlobalPosition();
		}
	}

	return pos;
}

Point16 VisualElement::getGlobalPosition() const {
	Point16 pos = getParentOrigin();

	pos.x += _rect.left;
	pos.y += _rect.top;

	return pos;
}

const Point16 &VisualElement::getCachedAbsoluteOrigin() const {
	return _cachedAbsoluteOrigin;
}

void VisualElement::setCachedAbsoluteOrigin(const Point16 &absOrigin) {
	_cachedAbsoluteOrigin = absOrigin;
}

void VisualElement::setDragMotionProperties(const Common::SharedPtr<DragMotionProperties>& dragProps) {
	_dragProps = dragProps;
}

const Common::SharedPtr<DragMotionProperties> &VisualElement::getDragMotionProperties() const {
	return _dragProps;
}

void VisualElement::handleDragMotion(Runtime *runtime, const Point16 &initialPoint, const Point16 &targetPointRef) {
	if (!_dragProps)
		return;

	Point16 targetPoint = targetPointRef;

	// NOTE: Constraints do not override insets if the object is out of bounds
	if (_dragProps->constraintDirection == kConstraintDirectionHorizontal)
		targetPoint.y = initialPoint.y;
	if (_dragProps->constraintDirection == kConstraintDirectionVertical)
		targetPoint.x = initialPoint.x;

	if (_dragProps->constrainToParent && _parent && _parent->isElement() && static_cast<Element *>(_parent)->isVisual()) {
		Rect16 constrainInset = _dragProps->constraintMargin;

		Rect16 parentRect = static_cast<VisualElement *>(_parent)->getRelativeRect();

		// rect.width - inset.right
		int32 minX = constrainInset.left;
		int32 minY = constrainInset.top;
		int32 maxX = parentRect.getWidth() - constrainInset.right - _rect.getWidth();
		int32 maxY = parentRect.getHeight() - constrainInset.bottom - _rect.getHeight();

		// TODO: Handle "squished" case where max < min, it does work but it's weird
		if (targetPoint.x < minX)
			targetPoint.x = minX;

		if (targetPoint.y < minY)
			targetPoint.y = minY;

		if (targetPoint.x > maxX)
			targetPoint.x = maxX;

		if (targetPoint.y > maxY)
			targetPoint.y = maxY;

		offsetTranslate(targetPoint.x - _rect.left, targetPoint.y - _rect.top, false);
	}
}

MiniscriptInstructionOutcome VisualElement::scriptSetVisibility(MiniscriptThread *thread, const DynamicValue &result) {
	// FIXME: Need to make this fire Show/Hide events!
	if (result.getType() == DynamicValueTypes::kBoolean) {
		_visible = result.getBool();
		return kMiniscriptInstructionOutcomeContinue;
	}

	return kMiniscriptInstructionOutcomeFailed;
}

bool VisualElement::loadCommon(const Common::String &name, uint32 guid, const Data::Rect &rect, uint32 elementFlags, uint16 layer, uint32 streamLocator, uint16 sectionID) {
	if (!_rect.load(rect))
		return false;

	_name = name;
	_guid = guid;
	_visible = ((elementFlags & Data::ElementFlags::kHidden) == 0);
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
		const Point16 &destPoint = value.getPoint();
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

	int32 xDelta = asInteger - _rect.left;

	if (xDelta != 0)
		offsetTranslate(xDelta, 0, false);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome VisualElement::scriptSetPositionY(MiniscriptThread *thread, const DynamicValue &dest) {
	int32 asInteger = 0;
	if (!dest.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	int32 yDelta = asInteger - _rect.top;

	if (yDelta != 0)
		offsetTranslate(0, yDelta, false);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome VisualElement::scriptSetCenterPosition(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kPoint) {
		const Point16 &destPoint = value.getPoint();
		const Point16 &srcPoint = getCenterPosition();
		int32 xDelta = destPoint.x - srcPoint.x;
		int32 yDelta = destPoint.y - srcPoint.y;

		if (xDelta != 0 || yDelta != 0)
			offsetTranslate(xDelta, yDelta, false);

		return kMiniscriptInstructionOutcomeContinue;
	}
	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome VisualElement::scriptSetWidth(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	_rect.right = _rect.left + asInteger;

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome VisualElement::scriptSetHeight(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	_rect.bottom = _rect.top + asInteger;

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
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome VisualElement::scriptWriteRefPositionAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	if (attrib == "x") {
		DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetPositionX>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "y") {
		DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetPositionY>::create(this, writeProxy);
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
}

Point16 VisualElement::getCenterPosition() const {
	return Point16::create((_rect.left + _rect.right) / 2, (_rect.top + _rect.bottom) / 2);
}

VThreadState VisualElement::changeVisibilityTask(const ChangeFlagTaskData &taskData) {
	if (_visible != taskData.desiredFlag) {
		_visible = taskData.desiredFlag;

		Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(_visible ? EventIDs::kElementHide : EventIDs::kElementShow, 0), DynamicValue(), getSelfReference()));
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

bool ModifierSaveLoad::load(Modifier *modifier, Common::ReadStream *stream) {
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

	return loadInternal(stream);
}

Modifier::Modifier() : _parent(nullptr) {
}

Modifier::~Modifier() {
}

bool Modifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "parent") {
		result.setObject(_parent);
		return true;
	}
	if (attrib == "subsection") {
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
	}
	if (attrib == "name") {
		result.setString(_name);
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

	ModifierInnerScopeBuilder innerScopeBuilder(&innerScope);
	this->visitInternalReferences(&innerScopeBuilder);

	ModifierChildMaterializer childMaterializer(runtime, this, &innerScope);
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

Common::SharedPtr<ModifierSaveLoad> Modifier::getSaveLoad() {
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

bool Modifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState Modifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	// If you're here, a message type was reported as responsive by respondsToEvent but consumeMessage wasn't overrided
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
}

#endif /* MTROPOLIS_DEBUG_ENABLE */

bool VariableModifier::isVariable() const {
	return true;
}

bool VariableModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "value") {
		varGetValue(thread, result);
		return true;
	}

	return Modifier::readAttribute(thread, result, attrib);
}

DynamicValueWriteProxy VariableModifier::createWriteProxy() {
	DynamicValueWriteProxy proxy;
	proxy.pod.objectRef = this;
	proxy.pod.ptrOrOffset = 0;
	proxy.pod.ifc = &VariableModifier::WriteProxyInterface::_instance;
	return proxy;
}

MiniscriptInstructionOutcome VariableModifier::WriteProxyInterface::write(MiniscriptThread *thread, const DynamicValue &dest, void *objectRef, uintptr ptrOrOffset) const {
	if (!static_cast<VariableModifier *>(objectRef)->varSetValue(thread, dest))
		return kMiniscriptInstructionOutcomeFailed;
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome VariableModifier::WriteProxyInterface::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &dest, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) const {
	return static_cast<VariableModifier *>(objectRef)->writeRefAttribute(thread, dest, attrib);
}

MiniscriptInstructionOutcome VariableModifier::WriteProxyInterface::refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &dest, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) const {
	return static_cast<VariableModifier *>(objectRef)->writeRefAttributeIndexed(thread, dest, attrib, index);
}

VariableModifier::WriteProxyInterface VariableModifier::WriteProxyInterface::_instance;

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
