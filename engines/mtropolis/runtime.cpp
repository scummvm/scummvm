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
#include "mtropolis/modifier_factory.h"
#include "mtropolis/modifiers.h"
#include "mtropolis/render.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/substream.h"
#include "common/system.h"

#include "graphics/cursorman.h"
#include "graphics/managed_surface.h"
#include "graphics/surface.h"
#include "graphics/wincursor.h"
#include "graphics/maccursor.h"
#include "graphics/macgui/macfontmanager.h"

namespace MTropolis {

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

bool Rect16::load(const Data::Rect &rect) {
	top = rect.top;
	left = rect.left;
	bottom = rect.bottom;
	right = rect.right;

	if (bottom < top || right < left)
		return false;

	return true;
}

bool IntRange::load(const Data::IntRange &range) {
	max = range.max;
	min = range.min;

	return true;
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
	value.angleRadians = 0.0;
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

void DynamicListDefaultSetter::defaultSet(Common::WeakPtr<RuntimeObject> &value) {
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

bool DynamicListValueImporter::importValue(const DynamicValue &dynValue, const Common::WeakPtr<RuntimeObject> *&outPtr) {
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

void DynamicListValueExporter::exportValue(DynamicValue &dynValue, const Common::WeakPtr<RuntimeObject> &value) {
	dynValue.setObject(value);
}

DynamicListContainer<void>::DynamicListContainer() : _size(0) {
}

bool DynamicListContainer<void>::setAtIndex(size_t index, const DynamicValue &dynValue) {
	return true;
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

		for (size_t i = prevSize; i < index; prevSize++) {
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

bool DynamicList::setAtIndex(size_t index, const DynamicValue &value) {
	if (_type != value.getType()) {
		if (_container != nullptr && _container->getSize() != 0)
			return false;
		else {
			clear();
			changeToType(value.getType());
			return _container->setAtIndex(index, value);
		}
	} else {
		return _container->setAtIndex(index, value);
	}
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
		if (!isfinite(rounded) || rounded < 1.0 || rounded > UINT32_MAX)
			return false;

		outIndex = static_cast<size_t>(rounded);
	} else if (value.getType() == DynamicValueTypes::kInteger) {
		int32 i = value.getInt();
		if (i < 1)
			return false;
		static_cast<size_t>(i - 1);
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

	return clonedList;
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
		_container = new DynamicListContainer<Common::WeakPtr<RuntimeObject> >();
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

const Common::WeakPtr<RuntimeObject> &DynamicValue::getObject() const {
	assert(_type == DynamicValueTypes::kObject);
	return _obj;
}

const DynamicValueReadProxy& DynamicValue::getReadProxy() const {
	assert(_type == DynamicValueTypes::kReadProxy);
	return _value.asReadProxy;
}

const DynamicValueWriteProxy &DynamicValue::getWriteProxy() const {
	assert(_type == DynamicValueTypes::kWriteProxy);
	return _value.asWriteProxy;
}

const Common::SharedPtr<DynamicList> &DynamicValue::getReadProxyList() const {
	assert(_type == DynamicValueTypes::kReadProxy);
	return _list;
}

const Common::SharedPtr<DynamicList> &DynamicValue::getWriteProxyList() const {
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

void DynamicValue::setReadProxy(const Common::SharedPtr<DynamicList> &list, const DynamicValueReadProxy &readProxy) {
	Common::SharedPtr<DynamicList> listRef = list;	// Back up list ref in case this is a self-assign
	if (_type != DynamicValueTypes::kReadProxy)
		clear();
	_type = DynamicValueTypes::kReadProxy;
	_value.asReadProxy = readProxy;
	_list = listRef;
}

void DynamicValue::setWriteProxy(const Common::SharedPtr<DynamicList> &list, const DynamicValueWriteProxy &writeProxy) {
	Common::SharedPtr<DynamicList> listRef = list; // Back up list ref in case this is a self-assign
	if (_type != DynamicValueTypes::kWriteProxy)
		clear();
	_type = DynamicValueTypes::kWriteProxy;
	_value.asWriteProxy = writeProxy;
	_list = listRef;
}

void DynamicValue::setObject(const Common::WeakPtr<RuntimeObject> &value) {
	if (_type != DynamicValueTypes::kObject)
		clear();
	_type = DynamicValueTypes::kObject;
	_obj = value;
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

void DynamicValue::initFromOther(const DynamicValue &other) {
	assert(_type == DynamicValueTypes::kNull);

	switch (other._type) {
	case DynamicValueTypes::kNull:
	case DynamicValueTypes::kIncomingData:
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
		case kMessageDestChildren:
		case kMessageDestElementsParent:
		case kMessageDestModifiersParent:
		case kMessageDestSubsection:
		case kMessageDestSourcesParent:
		case kMessageDestBehavior:
		case kMessageDestNextElement:
		case kMessageDestPrevElement:
		case kMessageDestBehaviorsParent:
			warning("Not-yet-implemented message destination type");
			break;
		default:
			break;
		}
	}
}

void MessengerSendSpec::sendFromMessenger(Runtime *runtime, Modifier *sender) const {

	Common::SharedPtr<MessageProperties> props(new MessageProperties(this->send, this->with, sender->getSelfReference()));

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

void IPlugInModifierRegistrar::registerPlugInModifier(const char *name, const IPlugInModifierFactoryAndDataFactory *loaderFactory) {
	return this->registerPlugInModifier(name, loaderFactory, loaderFactory);
}

PlugIn::~PlugIn() {
}

ProjectResources::~ProjectResources() {
}

CursorGraphicCollection::CursorGraphicCollection() {
}

CursorGraphicCollection::~CursorGraphicCollection() {
}

void CursorGraphicCollection::addWinCursorGroup(uint32 cursorGroupID, const Common::SharedPtr<Graphics::WinCursorGroup> &cursorGroup) {
	if (cursorGroup->cursors.size() > 0) {
		// Not sure what the proper logic should be here, but the second one seems to be the one we usually want
		if (cursorGroup->cursors.size() > 1)
			_cursorGraphics[cursorGroupID] = cursorGroup->cursors[1].cursor;
		else
			_cursorGraphics[cursorGroupID] = cursorGroup->cursors.back().cursor;
		_winCursorGroups.push_back(cursorGroup);
	}
}

void CursorGraphicCollection::addMacCursor(uint32 cursorID, const Common::SharedPtr<Graphics::MacCursor> &cursor) {
	_cursorGraphics[cursorID] = cursor.get();
	_macCursors.push_back(cursor);
}


ProjectDescription::ProjectDescription() : _language(Common::EN_ANY) {
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

void ProjectDescription::setLanguage(const Common::Language &language) {
	_language = language;
}

const Common::Language &ProjectDescription::getLanguage() const {
	return _language;
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

bool RuntimeObject::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	return false;
}

bool RuntimeObject::writeRefAttributeIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib, const DynamicValue &index) {
	return false;
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

Structural::Structural() : _parent(nullptr) {
#ifdef MTROPOLIS_DEBUG_ENABLE
	_debugger = nullptr;
#endif
}

Structural::~Structural() {
#ifdef MTROPOLIS_DEBUG_ENABLE
	if (_debugInspector)
		_debugInspector->onDestroyed();
#endif
}

ProjectPresentationSettings::ProjectPresentationSettings() : width(640), height(480), bitsPerPixel(8) {
}

bool Structural::isStructural() const {
	return true;
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

void Structural::setParent(Structural *parent) {
	_parent = parent;
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

#ifdef MTROPOLIS_DEBUG_ENABLE
	_debugInspector.reset(this->debugCreateInspector());
#endif
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

#ifdef MTROPOLIS_DEBUG_ENABLE
SupportStatus Structural::debugGetSupportStatus() const {
	return kSupportStatusNone;
}

const Common::String &Structural::debugGetName() const {
	return _name;
}

Common::SharedPtr<DebugInspector> Structural::debugGetInspector() const {
	return _debugInspector;
}

Debugger* Structural::debugGetDebugger() const {
	return _debugger;
}

DebugInspector *Structural::debugCreateInspector() {
	return new DebugInspector(this);
}
#endif /* MTROPOLIS_DEBUG_ENABLE */

void Structural::linkInternalReferences(ObjectLinkingScope *scope) {
}

ObjectLinkingScope *Structural::getPersistentStructuralScope() {
	return nullptr;
}

ObjectLinkingScope* Structural::getPersistentModifierScope() {
	return nullptr;
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

		warning("Couldn't resolve static GUID %x", staticGUID);
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

		warning("Couldn't resolve object name '%s'", name.c_str());
		return Common::WeakPtr<RuntimeObject>();
	}
}

Common::WeakPtr<RuntimeObject> ObjectLinkingScope::resolve(uint32 staticGUID, const Common::String &name, bool isNameAlreadyInsensitive) const {
	Common::WeakPtr<RuntimeObject> byGUIDResult = resolve(staticGUID);
	if (!byGUIDResult.expired())
		return byGUIDResult;
	else
		return resolve(name, isNameAlreadyInsensitive);
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
				if (responds)
					runtime->postConsumeMessageTask(modifier, _msg);

				return kVThreadReturn;
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

Runtime::SceneStackEntry::SceneStackEntry() {
}

Runtime::Runtime(OSystem *system) : _nextRuntimeGUID(1), _realDisplayMode(kColorDepthModeInvalid), _fakeDisplayMode(kColorDepthModeInvalid),
									_displayWidth(1024), _displayHeight(768), _realTimeBase(0), _playTimeBase(0), _sceneTransitionState(kSceneTransitionStateNotTransitioning),
									_system(system), _lastFrameCursor(nullptr), _defaultCursor(new DefaultCursor()) {
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
}

bool Runtime::runFrame() {
	uint32 timeMillis = _system->getMillis();

	uint32 realMSec = timeMillis - _realTimeBase - _realTime;
	uint32 playMSec = timeMillis - _playTimeBase - _playTime;

	_realTime = timeMillis - _realTimeBase;
	_playTime = timeMillis - _playTimeBase;

#ifdef MTROPOLIS_DEBUG_ENABLE
	if (_debugger)
		_debugger->runFrame(realMSec);
#endif

	for (;;) {
		VThreadState state = _vthread->step();
		if (state != kVThreadReturn) {
			// Still doing blocking tasks
			return false;
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

	Graphics::Cursor *cursor = nullptr;

	// ...
	if (cursor == nullptr)
		cursor = _defaultCursor.get();

	if (cursor != _lastFrameCursor) {
		_lastFrameCursor = cursor;

		CursorMan.showMouse(true);
		CursorMan.replaceCursor(cursor);
	}

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
		sharedSceneEntry.scene = targetScene;

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
	const Common::Array<Common::SharedPtr<Structural> > &children = structural->getChildren();
	for (Common::Array<Common::SharedPtr<Structural> >::const_iterator it = children.begin(), itEnd = children.end(); it != itEnd; ++it) {
		recursiveDeactivateStructural(it->get());
	}

	structural->deactivate();
}

void Runtime::recursiveActivateStructural(Structural *structural) {
	structural->activate();

	const Common::Array<Common::SharedPtr<Structural> > &children = structural->getChildren();
	for (Common::Array<Common::SharedPtr<Structural> >::const_iterator it = children.begin(), itEnd = children.end(); it != itEnd; ++it) {
		recursiveActivateStructural(it->get());
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
	debug(1, "Scene added to renderer OK");

#ifdef MTROPOLIS_DEBUG_ENABLE
	if (_debugger) {
		_debugger->complainAboutUnfinished(scene.get());
		_debugger->refreshSceneStatus();
	}
#endif

	refreshPlayTime();
}

void Runtime::sendMessageOnVThread(const Common::SharedPtr<MessageDispatch> &dispatch) {
	DispatchMethodTaskData *taskData = _vthread->pushTask(this, &Runtime::dispatchMessageTask);
	taskData->dispatch = dispatch;
}

VThreadState Runtime::dispatchMessageTask(const DispatchMethodTaskData &data) {
	Common::SharedPtr<MessageDispatch> dispatchPtr = data.dispatch;
	MessageDispatch &dispatch = *dispatchPtr.get();

	if (dispatch.isTerminated())
		return kVThreadReturn;
	else {
		// Requeue propagation after whatever happens with this propagation step
		DispatchMethodTaskData *requeueData = _vthread->pushTask(this, &Runtime::dispatchMessageTask);
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

void Runtime::queueMessage(const Common::SharedPtr<MessageDispatch>& dispatch) {
	_messageQueue.push_back(dispatch);
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

void Runtime::onMouseDown(int32 x, int32 y, Actions::MouseButton mButton) {
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
	Common::SharedPtr<Window> focusWindow = _mouseFocusWindow.lock();
	if (!focusWindow)
		focusWindow = findTopWindow(x, y);

	if (focusWindow)
		focusWindow->onMouseMove(x - focusWindow->getX(), y - focusWindow->getY());

	// TODO: Change mouse to focus window's cursor
}

void Runtime::onMouseUp(int32 x, int32 y, Actions::MouseButton mButton) {
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



void Runtime::ensureMainWindowExists() {
	// Maybe there's a better spot for this
	if (_mainWindow.expired() && _project) {
		const ProjectPresentationSettings &presentationSettings = _project->getPresentationSettings();

		int32 centeredX = (static_cast<int32>(_displayWidth) - static_cast<int32>(presentationSettings.width)) / 2;
		int32 centeredY = (static_cast<int32>(_displayHeight) - static_cast<int32>(presentationSettings.height)) / 2;
		Common::SharedPtr<Window> mainWindow(new Window(WindowParameters(this, centeredX, centeredY, presentationSettings.width, presentationSettings.height, _displayModePixelFormats[_realDisplayMode])));
		addWindow(mainWindow);
		_mainWindow.reset(mainWindow);
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

void Runtime::addSceneStateTransition(const HighLevelSceneTransition &transition) {
	_pendingSceneTransitions.push_back(transition);
}

Project *Runtime::getProject() const {
	return _project.get();
}

void Runtime::postConsumeMessageTask(IMessageConsumer *consumer, const Common::SharedPtr<MessageProperties> &msg) {
	ConsumeMessageTaskData *params = _vthread->pushTask(this, &Runtime::consumeMessageTask);
	params->consumer = consumer;
	params->message = msg;
}

void Runtime::postConsumeCommandTask(Structural *structural, const Common::SharedPtr<MessageProperties> &msg) {
	ConsumeCommandTaskData *params = _vthread->pushTask(this, &Runtime::consumeCommandTask);
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

Debugger* Runtime::debugGetDebugger() const {
	return _debugger.get();
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

PostRenderSignaller::PostRenderSignaller() {
}

PostRenderSignaller::~PostRenderSignaller() {
}

void PostRenderSignaller::onPostRender(Runtime *runtime, Project *project) {
	const size_t numReceivers = _receivers.size();
	for (size_t i = 0; i < numReceivers; i++) {
		_receivers[i]->onPostRender(runtime, project);
	}
}

void PostRenderSignaller::addReceiver(IPostRenderSignalReceiver *receiver) {
	_receivers.push_back(receiver);
}

void PostRenderSignaller::removeReceiver(IPostRenderSignalReceiver* receiver) {
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

Project::Segment::Segment() : weakStream(nullptr) {
}

Project::Project(Runtime *runtime)
	: _runtime(runtime), _projectFormat(Data::kProjectFormatUnknown), _isBigEndian(false),
	  _haveGlobalObjectInfo(false), _haveProjectStructuralDef(false), _postRenderSignaller(new PostRenderSignaller()) {
}

Project::~Project() {
	for (size_t i = 0; i < _segments.size(); i++)
		closeSegmentStream(i);
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
	_postRenderSignaller->onPostRender(_runtime, this);
}

Common::SharedPtr<PostRenderSignaller> Project::notifyOnPostRender(IPostRenderSignalReceiver *receiver) {
	_postRenderSignaller->addReceiver(receiver);
	return _postRenderSignaller;
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
					loaderContext.containerUnion.filteredElements.structural = container;
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

bool VisualElement::isVisual() const {
	return true;
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

bool VisualElement::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "visible") {
		result.setBool(_visible);
		return true;
	}
	if (attrib == "direct") {
		result.setBool(_directToScreen);
		return true;
	}

	return Element::readAttribute(thread, result, attrib);
}

bool VisualElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	if (attrib == "visible") {
		writeProxy = DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetVisibility>::create(this);
		return true;
	}
	if (attrib == "direct") {
		writeProxy = DynamicValueWriteFuncHelper<VisualElement, &VisualElement::scriptSetDirect>::create(this);
		return true;
	}

	return Element::writeRefAttribute(thread, writeProxy, attrib);
}

bool VisualElement::scriptSetVisibility(const DynamicValue& result) {
	// FIXME: Need to make this fire Show/Hide events!
	if (result.getType() == DynamicValueTypes::kBoolean) {
		_visible = result.getBool();
		return true;
	}

	return false;
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

bool VisualElement::scriptSetDirect(const DynamicValue &dest) {
	if (dest.getType() == DynamicValueTypes::kBoolean) {
		_directToScreen = dest.getBool();
		return true;
	}
	return false;
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

Modifier::Modifier() : _parent(nullptr) {
#ifdef MTROPOLIS_DEBUG_ENABLE
	_debugger = nullptr;
#endif
}

Modifier::~Modifier() {
#ifdef MTROPOLIS_DEBUG_ENABLE
	if (_debugInspector)
		_debugInspector->onDestroyed();
#endif
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

#ifdef MTROPOLIS_DEBUG_ENABLE
	_debugger = runtime->debugGetDebugger();
	_debugInspector.reset(this->debugCreateInspector());
#endif
}

bool Modifier::isAlias() const {
	return false;
}

bool Modifier::isVariable() const {
	return false;
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

#ifdef MTROPOLIS_DEBUG_ENABLE

SupportStatus Modifier::debugGetSupportStatus() const {
	return kSupportStatusNone;
}

const Common::String &Modifier::debugGetName() const {
	return _name;
}

Common::SharedPtr<DebugInspector> Modifier::debugGetInspector() const {
	return _debugInspector;
}

Debugger *Modifier::debugGetDebugger() const {
	return _debugger;
}

DebugInspector *Modifier::debugCreateInspector() {
	return new DebugInspector(this);

}
#endif /* MTROPOLIS_DEBUG_ENABLE */

bool VariableModifier::isVariable() const {
	return true;
}

bool Modifier::loadTypicalHeader(const Data::TypicalModifierHeader &typicalHeader) {
	if (!_modifierFlags.load(typicalHeader.modifierFlags))
		return false;
	_guid = typicalHeader.guid;
	_name = typicalHeader.name;

	return true;
}

void Modifier::linkInternalReferences(ObjectLinkingScope *scope) {
}

} // End of namespace MTropolis
