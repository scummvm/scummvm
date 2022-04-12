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

#ifndef MTROPOLIS_MODIFIERS_H
#define MTROPOLIS_MODIFIERS_H

#include "mtropolis/runtime.h"
#include "mtropolis/data.h"

namespace MTropolis {

struct ModifierLoaderContext;
class MiniscriptProgram;

class BehaviorModifier : public Modifier, public IModifierContainer {
public:
	bool load(ModifierLoaderContext &context, const Data::BehaviorModifier &data);

	const Common::Array<Common::SharedPtr<Modifier> > &getModifiers() const override;
	void appendModifier(const Common::SharedPtr<Modifier> &modifier) override;

private:
	Common::Array<Common::SharedPtr<Modifier> > _children;
	Event _enableWhen;
	Event _disableWhen;
};

class MiniscriptModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::MiniscriptModifier &data);

private:
	Event _enableWhen;

	Common::SharedPtr<MiniscriptProgram> _program;
};

class MessengerModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::MessengerModifier &data);

private:
	Event _when;
	MessengerSendSpec _sendSpec;
};

class SetModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::SetModifier &data);

private:
	Event _executeWhen;
	DynamicValue _source;
	DynamicValue _target;
};

class DragMotionModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::DragMotionModifier &data);

private:
	Event _enableWhen;
	Event _disableWhen;

	enum ConstraintDirection {
		kConstraintDirectionNone,
		kConstraintDirectionHorizontal,
		kConstraintDirectionVertical,
	};

	ConstraintDirection _constraintDirection;
	Rect16 _constraintMargin;
	bool _constrainToParent;
};

class VectorMotionModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::VectorMotionModifier &data);

private:
	Event _enableWhen;
	Event _disableWhen;

	DynamicValue _vec;
};

class IfMessengerModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::IfMessengerModifier &data);

private:
	Event _when;
	MessengerSendSpec _sendSpec;

	Common::SharedPtr<MiniscriptProgram> _program;
};

class TimerMessengerModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::TimerMessengerModifier &data);

private:
	Event _executeWhen;
	Event _terminateWhen;
	MessengerSendSpec _sendSpec;
	uint32 _milliseconds;
	bool _looping;
};

class BoundaryDetectionMessengerModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::BoundaryDetectionMessengerModifier &data);

private:
	enum ExitTriggerMode {
		kExitTriggerExiting,
		kExitTriggerOnceExited,
	};

	enum DetectionMode {
		kContinuous,
		kOnFirstDetection,
	};

	Event _enableWhen;
	Event _disableWhen;
	ExitTriggerMode _exitTriggerMode;
	DetectionMode _detectionMode;
	bool _detectTopEdge;
	bool _detectBottomEdge;
	bool _detectLeftEdge;
	bool _detectRightEdge;
	MessengerSendSpec _send;
};

class CollisionDetectionMessengerModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::CollisionDetectionMessengerModifier &data);

private:
	enum DetectionMode {
		kDetectionModeFirstContact,
		kDetectionModeWhileInContact,
		kDetectionModeExiting,
	};

	Event _enableWhen;
	Event _disableWhen;
	MessengerSendSpec _sendSpec;

	DetectionMode _detectionMode;
	bool _detectInFront;
	bool _detectBehind;
	bool _ignoreParent;
	bool _sendToCollidingElement; // ... instead of to send spec destination, but send spec with/flags still apply!
	bool _sendToOnlyFirstCollidingElement;
};

class KeyboardMessengerModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::KeyboardMessengerModifier &data);

private:
	Event _send;

	enum KeyCodeType {
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
		kMacRomanChar = 0xff,
	};

	bool _onDown : 1;
	bool _onUp : 1;
	bool _onRepeat : 1;
	bool _keyModControl : 1;
	bool _keyModCommand : 1;
	bool _keyModOption : 1;
	KeyCodeType _keyCodeType;
	uint8_t _macRomanChar;

	MessengerSendSpec _sendSpec;
};

class TextStyleModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::TextStyleModifier &data);

	enum Alignment {
		kAlignmentLeft = 0,
		kAlignmentCenter = 1,
		kAlignmentRight = 0xffff,
	};

	struct StyleFlags {
		bool bold : 1;
		bool italic : 1;
		bool underline : 1;
		bool outline : 1;
		bool shadow : 1;
		bool condensed : 1;
		bool expanded : 1;

		StyleFlags();
		bool load(uint8 dataStyleFlags);
	};

	uint16 _macFontID;
	uint16 _size;
	ColorRGB8 _textColor;
	ColorRGB8 _backgroundColor;
	Alignment _alignment;
	Event _applyWhen;
	Event _removeWhen;
	Common::String _fontFamilyName;
};

class GraphicModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::GraphicModifier &data);

	enum InkMode {
		kInkModeCopy = 0x0,
		kInkModeTransparent = 0x1,				// src*dest
		kInkModeGhost = 0x3,					// (1-src)+dest
		kInkModeReverseCopy = 0x4,				// 1-src
		kInkModeReverseGhost = 0x7,				// src+dest
		kInkModeReverseTransparent = 0x9,		// (1-src)*dest
		kInkModeBlend = 0x20,					// (src*bgcolor)+(dest*(1-bgcolor)
		kInkModeBackgroundTransparent = 0x24,	// BG color is transparent
		kInkModeChameleonDark = 0x25,			// src+dest
		kInkModeChameleonLight = 0x27,			// src*dest
		kInkModeBackgroundMatte = 0x224,		// BG color is transparent and non-interactive
		kInkModeInvisible = 0xffff,				// Not drawn, but interactive
	};

	enum Shape {
		kShapeRect = 0x1,
		kShapeRoundedRect = 0x2,
		kShapeOval = 0x3,
		kShapePolygon = 0x9,
		kShapeStar = 0xb,	// 5-point star, horizontal arms are at (top+bottom*2)/3
	};

private:
	Event _applyWhen;
	Event _removeWhen;
	InkMode _inkMode;
	Shape _shape;

	ColorRGB8 _foreColor;
	ColorRGB8 _backColor;
	uint16 _borderSize;
	ColorRGB8 _borderColor;
	uint16 _shadowSize;
	ColorRGB8 _shadowColor;

	Common::Array<Point16> _polyPoints;
};

class BooleanVariableModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::BooleanVariableModifier &data);

private:
	bool _value;
};

class IntegerVariableModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::IntegerVariableModifier &data);

private:
	int32 _value;
};

class IntegerRangeVariableModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::IntegerRangeVariableModifier &data);

private:
	IntRange _range;
};

class VectorVariableModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::VectorVariableModifier &data);

private:
	AngleMagVector _vector;
};

class PointVariableModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::PointVariableModifier &data);

private:
	Point16 _value;
};

class FloatingPointVariableModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::FloatingPointVariableModifier &data);

private:
	double _value;
};

class StringVariableModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::StringVariableModifier &data);

private:
	Common::String _value;
};

}	// End of namespace MTropolis

#endif
