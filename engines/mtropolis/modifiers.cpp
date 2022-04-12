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

#include "mtropolis/miniscript.h"
#include "mtropolis/modifiers.h"
#include "mtropolis/modifier_factory.h"

#include "common/memstream.h"

namespace MTropolis {

bool BehaviorModifier::load(ModifierLoaderContext &context, const Data::BehaviorModifier &data) {
	if (data.numChildren > 0) {
		ChildLoaderContext loaderContext;
		loaderContext.containerUnion.modifierContainer = this;
		loaderContext.type = ChildLoaderContext::kTypeModifierList;
		loaderContext.remainingCount = data.numChildren;

		context.childLoaderStack->contexts.push_back(loaderContext);
	}

	if (!_enableWhen.load(data.enableWhen) || !_disableWhen.load(data.disableWhen))
		return false;

	_guid = data.guid;
	_name = data.name;
	_modifierFlags.load(data.modifierFlags);

	return true;
}

const Common::Array<Common::SharedPtr<Modifier> > &BehaviorModifier::getModifiers() const {
	return _children;
}

void BehaviorModifier::appendModifier(const Common::SharedPtr<Modifier> &modifier) {
	_children.push_back(modifier);
}


// Miniscript modifier
bool MiniscriptModifier::load(ModifierLoaderContext &context, const Data::MiniscriptModifier &data) {
	if (!_enableWhen.load(data.enableWhen))
		return false;

	_program = MiniscriptParser::parse(data.program);
	if (!_program)
		return false;

	return true;
}

MessengerSendSpec::MessengerSendSpec() : destination(0) {
}

bool MessengerSendSpec::load(const Data::Event &dataEvent, uint32 dataMessageFlags, const Data::MessageDataLocator &dataLocator, const Common::String &dataWithSourceName, uint32 dataDestination) {
	messageFlags.relay = ((dataMessageFlags & 0x20000000) == 0);
	messageFlags.cascade = ((dataMessageFlags & 0x40000000) == 0);
	messageFlags.immediate = ((dataMessageFlags & 0x80000000) == 0);

	if (!this->send.load(dataEvent))
		return false;

	if (!this->with.load(dataLocator, dataWithSourceName))
		return false;

	this->destination = dataDestination;

	return true;
}

bool MessengerModifier::load(ModifierLoaderContext &context, const Data::MessengerModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_when.load(data.when) || !_sendSpec.load(data.send, data.messageFlags, data.with, data.withSourceName, data.destination))
		return false;

	return true;
}

bool SetModifier::load(ModifierLoaderContext &context, const Data::SetModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_executeWhen.load(data.executeWhen) || !_sourceLoc.load(data.sourceLocator, data.sourceName) || !_targetLoc.load(data.targetLocator, data.targetName))
		return false;

	return true;
}

bool DragMotionModifier::load(ModifierLoaderContext &context, const Data::DragMotionModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_enableWhen.load(data.enableWhen) || !_disableWhen.load(data.disableWhen) || !_constraintMargin.load(data.constraintMargin))
		return false;

	bool constrainVertical = false;
	bool constrainHorizontal = false;
	if (data.haveMacPart) {
		_constrainToParent = ((data.platform.mac.flags & Data::DragMotionModifier::MacPart::kConstrainToParent) != 0);
		constrainVertical = ((data.platform.mac.flags & Data::DragMotionModifier::MacPart::kConstrainHorizontal) != 0);
		constrainHorizontal = ((data.platform.mac.flags & Data::DragMotionModifier::MacPart::kConstrainVertical) != 0);
	} else if (data.haveWinPart) {
		_constrainToParent = (data.platform.win.constrainToParent != 0);
		constrainVertical = (data.platform.win.constrainVertical != 0);
		constrainHorizontal = (data.platform.win.constrainHorizontal != 0);
	} else {
		return false;
	}

	if (constrainVertical) {
		if (constrainHorizontal)
			return false;	// ???
		else
			_constraintDirection = kConstraintDirectionVertical;
	} else {
		if (constrainHorizontal)
			_constraintDirection = kConstraintDirectionHorizontal;
		else
			_constraintDirection = kConstraintDirectionNone;
	}

	return true;
}

bool VectorMotionModifier::load(ModifierLoaderContext &context, const Data::VectorMotionModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_enableWhen.load(data.enableWhen) || !_disableWhen.load(data.disableWhen) || !_sourceVarLoc.load(data.varSource, data.varSourceName))
		return false;

	return true;
}

bool IfMessengerModifier::load(ModifierLoaderContext &context, const Data::IfMessengerModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_when.load(data.when) || !_sendSpec.load(data.send, data.messageFlags, data.with, data.withSource, data.destination))
		return false;

	_program = MiniscriptParser::parse(data.program);
	if (!_program)
		return false;

	return true;
}

bool TimerMessengerModifier::load(ModifierLoaderContext &context, const Data::TimerMessengerModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_executeWhen.load(data.executeWhen) || !this->_terminateWhen.load(data.terminateWhen))
		return false;

	if (!_sendSpec.load(data.send, data.messageAndTimerFlags, data.with, data.withSource, data.destination))
		return false;

	_milliseconds = data.minutes * (60 * 1000) + data.seconds * (1000) + data.hundredthsOfSeconds * 10;
	_looping = ((data.messageAndTimerFlags & Data::TimerMessengerModifier::kTimerFlagLooping) != 0);

	return true;
}

bool BoundaryDetectionMessengerModifier::load(ModifierLoaderContext &context, const Data::BoundaryDetectionMessengerModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_enableWhen.load(data.enableWhen) || !this->_disableWhen.load(data.disableWhen))
		return false;

	_exitTriggerMode = ((data.messageFlagsHigh & Data::BoundaryDetectionMessengerModifier::kDetectExiting) != 0) ? kExitTriggerExiting : kExitTriggerOnceExited;
	_detectionMode = ((data.messageFlagsHigh & Data::BoundaryDetectionMessengerModifier::kWhileDetected) != 0) ? kContinuous : kOnFirstDetection;
		
	_detectTopEdge = ((data.messageFlagsHigh & Data::BoundaryDetectionMessengerModifier::kDetectTopEdge) != 0);
	_detectBottomEdge = ((data.messageFlagsHigh & Data::BoundaryDetectionMessengerModifier::kDetectBottomEdge) != 0);
	_detectLeftEdge = ((data.messageFlagsHigh & Data::BoundaryDetectionMessengerModifier::kDetectLeftEdge) != 0);
	_detectRightEdge = ((data.messageFlagsHigh & Data::BoundaryDetectionMessengerModifier::kDetectRightEdge) != 0);

	if (!_send.load(data.send, data.messageFlagsHigh << 16, data.with, data.withSource, data.destination))
		return false;

	return true;
}

bool CollisionDetectionMessengerModifier::load(ModifierLoaderContext &context, const Data::CollisionDetectionMessengerModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_enableWhen.load(data.enableWhen) || !this->_disableWhen.load(data.disableWhen))
		return false;

	if (!_sendSpec.load(data.send, data.messageAndModifierFlags, data.with, data.withSource, data.destination))
		return false;

	_detectInFront = ((data.messageAndModifierFlags & Data::CollisionDetectionMessengerModifier::kDetectLayerInFront) != 0);
	_detectBehind = ((data.messageAndModifierFlags & Data::CollisionDetectionMessengerModifier::kDetectLayerBehind) != 0);
	_ignoreParent = ((data.messageAndModifierFlags & Data::CollisionDetectionMessengerModifier::kNoCollideWithParent) != 0);
	_sendToCollidingElement = ((data.messageAndModifierFlags & Data::CollisionDetectionMessengerModifier::kSendToCollidingElement) != 0);
	_sendToOnlyFirstCollidingElement = ((data.messageAndModifierFlags & Data::CollisionDetectionMessengerModifier::kSendToOnlyFirstCollidingElement) != 0);

	switch (data.messageAndModifierFlags & Data::CollisionDetectionMessengerModifier::kDetectionModeMask) {
	case Data::CollisionDetectionMessengerModifier::kDetectionModeFirstContact:
		_detectionMode = kDetectionModeFirstContact;
		break;
	case Data::CollisionDetectionMessengerModifier::kDetectionModeWhileInContact:
		_detectionMode = kDetectionModeWhileInContact;
		break;
	case Data::CollisionDetectionMessengerModifier::kDetectionModeExiting:
		_detectionMode = kDetectionModeExiting;
		break;
	default:
		return false;	// Unknown flag combination
	}

	return true;
}

bool KeyboardMessengerModifier::load(ModifierLoaderContext &context, const Data::KeyboardMessengerModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_onDown = ((data.messageFlagsAndKeyStates & Data::KeyboardMessengerModifier::kOnDown) != 0);
	_onUp = ((data.messageFlagsAndKeyStates & Data::KeyboardMessengerModifier::kOnUp) != 0);
	_onRepeat = ((data.messageFlagsAndKeyStates & Data::KeyboardMessengerModifier::kOnRepeat) != 0);
	_keyModControl = ((data.keyModifiers & Data::KeyboardMessengerModifier::kControl) != 0);
	_keyModCommand = ((data.keyModifiers & Data::KeyboardMessengerModifier::kCommand) != 0);
	_keyModOption = ((data.keyModifiers & Data::KeyboardMessengerModifier::kOption) != 0);

	switch (data.keycode) {
	case KeyCodeType::kAny:
	case KeyCodeType::kHome:
	case KeyCodeType::kEnter:
	case KeyCodeType::kEnd:
	case KeyCodeType::kHelp:
	case KeyCodeType::kBackspace:
	case KeyCodeType::kTab:
	case KeyCodeType::kPageUp:
	case KeyCodeType::kPageDown:
	case KeyCodeType::kReturn:
	case KeyCodeType::kEscape:
	case KeyCodeType::kArrowLeft:
	case KeyCodeType::kArrowRight:
	case KeyCodeType::kArrowUp:
	case KeyCodeType::kArrowDown:
	case KeyCodeType::kDelete:
		_keyCodeType = static_cast<KeyCodeType>(data.keycode);
		_macRomanChar = 0;
		break;
	default:
		_keyCodeType = kMacRomanChar;
		_macRomanChar = data.keycode;
		break;
	}

	if (!_sendSpec.load(data.message, data.messageFlagsAndKeyStates, data.with, data.withSource, data.destination))
		return false;

	return true;
}

TextStyleModifier::StyleFlags::StyleFlags() : bold(false), italic(false), underline(false), outline(false), shadow(false), condensed(false), expanded(false) {
}

bool TextStyleModifier::StyleFlags::load(uint8 dataStyleFlags) {
	bold = ((dataStyleFlags & 0x01) != 0);
	italic = ((dataStyleFlags & 0x02) != 0);
	underline = ((dataStyleFlags & 0x03) != 0);
	outline = ((dataStyleFlags & 0x04) != 0);
	shadow = ((dataStyleFlags & 0x10) != 0);
	condensed = ((dataStyleFlags & 0x20) != 0);
	expanded = ((dataStyleFlags & 0x40) != 0);
	return true;
}

bool TextStyleModifier::load(ModifierLoaderContext &context, const Data::TextStyleModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_textColor.load(data.textColor) || !_backgroundColor.load(data.backgroundColor) || !_applyWhen.load(data.applyWhen) || !_removeWhen.load(data.removeWhen))
		return false;

	_macFontID = data.macFontID;
	_size = data.size;
	_alignment = static_cast<Alignment>(data.alignment);
	_fontFamilyName = data.fontFamilyName;

	return true;
}

bool GraphicModifier::load(ModifierLoaderContext& context, const Data::GraphicModifier& data) {
	if (!loadTypicalHeader(data.modHeader) || !_applyWhen.load(data.applyWhen) || !_removeWhen.load(data.removeWhen)
		|| !_foreColor.load(data.foreColor) || !_backColor.load(data.backColor)
		|| !_borderColor.load(data.borderColor) || !_shadowColor.load(data.shadowColor))
		return false;

	// We need the poly points even if this isn't a poly shape since I think it's possible to change the shape type at runtime
	_polyPoints.resize(data.polyPoints.size());
	for (size_t i = 0; i < data.polyPoints.size(); i++) {
		_polyPoints[i].x = data.polyPoints[i].x;
		_polyPoints[i].y = data.polyPoints[i].y;
	}

	_inkMode = static_cast<InkMode>(data.inkMode);
	_shape = static_cast<Shape>(data.shape);

	_borderSize = data.borderSize;
	_shadowSize = data.shadowSize;

	return true;
}

bool BooleanVariableModifier::load(ModifierLoaderContext &context, const Data::BooleanVariableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_value = (data.value != 0);

	return true;
}

bool IntegerVariableModifier::load(ModifierLoaderContext& context, const Data::IntegerVariableModifier& data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_value = data.value;

	return true;
}

bool IntegerRangeVariableModifier::load(ModifierLoaderContext& context, const Data::IntegerRangeVariableModifier& data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_min = data.min;
	_max = data.max;

	return true;
}

bool VectorVariableModifier::load(ModifierLoaderContext &context, const Data::VectorVariableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_angleRadians = data.angleRadians.toDouble();
	_magnitude = data.magnitude.toDouble();

	return true;
}

bool PointVariableModifier::load(ModifierLoaderContext &context, const Data::PointVariableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_value.x = data.value.x;
	_value.y = data.value.y;

	return true;
}

bool FloatingPointVariableModifier::load(ModifierLoaderContext &context, const Data::FloatingPointVariableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_value = data.value.toDouble();

	return true;
}

bool StringVariableModifier::load(ModifierLoaderContext &context, const Data::StringVariableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_value = data.value;

	return true;
}

} // End of namespace MTropolis
