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
class MiniscriptReferences;
class MiniscriptThread;

class BehaviorModifier : public Modifier, public IModifierContainer {
public:
	bool load(ModifierLoaderContext &context, const Data::BehaviorModifier &data);

	const Common::Array<Common::SharedPtr<Modifier> > &getModifiers() const override;
	void appendModifier(const Common::SharedPtr<Modifier> &modifier) override;

	IModifierContainer *getChildContainer() override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Behavior Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

	Common::Array<Common::SharedPtr<Modifier> > _children;

	Event _enableWhen;
	Event _disableWhen;
};

class MiniscriptModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::MiniscriptModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Miniscript Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusPartial; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	void linkInternalReferences(ObjectLinkingScope *scope) override;

	Event _enableWhen;

	Common::SharedPtr<MiniscriptProgram> _program;
	Common::SharedPtr<MiniscriptReferences> _references;
};

class MessengerModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::MessengerModifier &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Messenger Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	Event _when;
	MessengerSendSpec _sendSpec;
};

class SetModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::SetModifier &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Set Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	Event _executeWhen;
	DynamicValue _source;
	DynamicValue _target;
};

class AliasModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::AliasModifier &data);
	uint32 getAliasID() const;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Alias Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	uint32 _aliasID;
};

class ChangeSceneModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::ChangeSceneModifier &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Change Scene Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	enum SceneSelectionType {
		kSceneSelectionTypeNext,
		kSceneSelectionTypePrevious,
		kSceneSelectionTypeSpecific,
	};

	Event _executeWhen;
	SceneSelectionType _sceneSelectionType;
	uint32 _targetSectionGUID;
	uint32 _targetSubsectionGUID;
	uint32 _targetSceneGUID;
	bool _addToReturnList;
	bool _addToDestList;
	bool _addToWrapAround;
};

class SoundEffectModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::SoundEffectModifier &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Sound Effect Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	enum SoundType {
		kSoundTypeBeep,
		kSoundTypeAudioAsset,
	};

	Event _executeWhen;
	Event _terminateWhen;

	SoundType _soundType;
	uint32 _assetID;
};

class DragMotionModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::DragMotionModifier &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Drag Motion Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

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

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Vector Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	Event _enableWhen;
	Event _disableWhen;

	DynamicValue _vec;
};

class SceneTransitionModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::SceneTransitionModifier &data);

	enum TransitionType {
		kTransitionTypePatternDissolve = 0x0406,
		kTransitionTypeRandomDissolve  = 0x0410,	// No steps
		kTransitionTypeFade            = 0x041a,
		kTransitionTypeSlide           = 0x03e8,	// Directional
		kTransitionTypePush            = 0x03f2,	// Directional
		kTransitionTypeZoom            = 0x03fc,
		kTransitionTypeWipe            = 0x0424,	// Directional
	};

	enum TransitionDirection {
		kTransitionDirectionUp = 0x385,
		kTransitionDirectionDown = 0x385,
		kTransitionDirectionLeft = 0x386,
		kTransitionDirectionRight = 0x387,
	};

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Scene Transition Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	Event _enableWhen;
	Event _disableWhen;

	uint32 _duration;	// 6000000 is maximum
	uint16 _steps;
	TransitionType _transitionType;
	TransitionDirection _transitionDirection;
};

class ElementTransitionModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::ElementTransitionModifier &data);

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

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Element Transition Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	Event _enableWhen;
	Event _disableWhen;

	uint32 _rate;	// 1-100, higher is faster
	uint16 _steps;
	TransitionType _transitionType;
	RevealType _revealType;
};

class IfMessengerModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::IfMessengerModifier &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "If Messenger Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	Event _when;
	MessengerSendSpec _sendSpec;

	Common::SharedPtr<MiniscriptProgram> _program;
	Common::SharedPtr<MiniscriptReferences> _references;
};

class TimerMessengerModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::TimerMessengerModifier &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Timer Messenger Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	Event _executeWhen;
	Event _terminateWhen;
	MessengerSendSpec _sendSpec;
	uint32 _milliseconds;
	bool _looping;
};

class BoundaryDetectionMessengerModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::BoundaryDetectionMessengerModifier &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Boundary Detection Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

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

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Collision Detection Messenger Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

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

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Keyboard Messenger Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

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

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Text Style Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

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

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Graphic Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

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

class CompoundVariableModifier : public VariableModifier, public IModifierContainer {
public:
	bool load(ModifierLoaderContext &context, const Data::CompoundVariableModifier &data);

	IModifierContainer *getChildContainer() override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Compound Variable Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	const Common::Array<Common::SharedPtr<Modifier> > &getModifiers() const override;
	void appendModifier(const Common::SharedPtr<Modifier> &modifier) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

	Common::Array<Common::SharedPtr<Modifier> > _children;
};

class BooleanVariableModifier : public VariableModifier {
public:
	bool load(ModifierLoaderContext &context, const Data::BooleanVariableModifier &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Boolean Variable Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	bool _value;
};

class IntegerVariableModifier : public VariableModifier {
public:
	bool load(ModifierLoaderContext &context, const Data::IntegerVariableModifier &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Integer Variable Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	int32 _value;
};

class IntegerRangeVariableModifier : public VariableModifier {
public:
	bool load(ModifierLoaderContext &context, const Data::IntegerRangeVariableModifier &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Integer Range Variable Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	IntRange _range;
};

class VectorVariableModifier : public VariableModifier {
public:
	bool load(ModifierLoaderContext &context, const Data::VectorVariableModifier &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Vector Variable Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	AngleMagVector _vector;
};

class PointVariableModifier : public VariableModifier {
public:
	bool load(ModifierLoaderContext &context, const Data::PointVariableModifier &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Point Variable Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	Point16 _value;
};

class FloatingPointVariableModifier : public VariableModifier {
public:
	bool load(ModifierLoaderContext &context, const Data::FloatingPointVariableModifier &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Floating Point Variable Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	double _value;
};

class StringVariableModifier : public VariableModifier {
public:
	bool load(ModifierLoaderContext &context, const Data::StringVariableModifier &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "String Variable Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;

	Common::String _value;
};

}	// End of namespace MTropolis

#endif
