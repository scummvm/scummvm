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

#include "common/events.h"

#include "mtropolis/render.h"
#include "mtropolis/runtime.h"
#include "mtropolis/data.h"

namespace MTropolis {

struct AudioMetadata;
class AudioPlayer;
class CachedAudio;
struct ModifierLoaderContext;
class MiniscriptProgram;
class MiniscriptReferences;
class MiniscriptThread;

class BehaviorModifier : public Modifier, public IModifierContainer {
public:
	bool load(ModifierLoaderContext &context, const Data::BehaviorModifier &data);

	bool isBehavior() const override { return true; }

	const Common::Array<Common::SharedPtr<Modifier> > &getModifiers() const override;
	void appendModifier(const Common::SharedPtr<Modifier> &modifier) override;

	IModifierContainer *getMessagePropagationContainer() override;
	IModifierContainer *getChildContainer() override;

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Behavior Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	struct SwitchTaskData {
		SwitchTaskData() : targetState(false), eventID(EventIDs::kNothing), runtime(nullptr) {}

		bool targetState;
		EventIDs::EventID eventID;
		Runtime *runtime;
	};

	struct PropagateTaskData {
		PropagateTaskData() : index(0), eventID(EventIDs::kNothing), runtime(nullptr) {}

		size_t index;
		EventIDs::EventID eventID;
		Runtime *runtime;
	};

	struct DisableTaskData {
		DisableTaskData() : runtime(nullptr) {}

		Runtime *runtime;
	};

	VThreadState switchTask(const SwitchTaskData &taskData);
	VThreadState propagateTask(const PropagateTaskData &taskData);
	VThreadState disableTask(const DisableTaskData &taskData);

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
	void linkInternalReferences(ObjectLinkingScope *scope) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

	Common::Array<Common::SharedPtr<Modifier> > _children;

	Event _enableWhen;
	Event _disableWhen;
	bool _switchable;
	bool _isEnabled;
};

class MiniscriptModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::MiniscriptModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override {}

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Miniscript Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
	void linkInternalReferences(ObjectLinkingScope *scope) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

	Event _enableWhen;

	Common::SharedPtr<MiniscriptProgram> _program;
	Common::SharedPtr<MiniscriptReferences> _references;
};

class SaveAndRestoreModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::SaveAndRestoreModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override {}

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Save And Restore Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _saveWhen;
	Event _restoreWhen;

	DynamicValue _saveOrRestoreValue;

	Common::String _filePath;
	Common::String _fileName;
};

class MessengerModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::MessengerModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override {}

	void linkInternalReferences(ObjectLinkingScope *outerScope) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Messenger Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _when;
	MessengerSendSpec _sendSpec;
};

class SetModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::SetModifier &data);

	void disable(Runtime *runtime) override {}

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Set Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _executeWhen;
	DynamicValue _source;
	DynamicValue _target;
};

class AliasModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::AliasModifier &data);
	uint32 getAliasID() const;

	bool isAlias() const override;

	void disable(Runtime *runtime) override {}

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Alias Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	uint32 _aliasID;
};

class ChangeSceneModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::ChangeSceneModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override {}

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Change Scene Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

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
	bool _wrapAround;
};

class SoundEffectModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::SoundEffectModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Sound Effect Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	enum SoundType {
		kSoundTypeBeep,
		kSoundTypeAudioAsset,
	};

	void loadAndCacheAudio(Runtime *runtime);

	Event _executeWhen;
	Event _terminateWhen;

	SoundType _soundType;
	uint32 _assetID;

	Common::SharedPtr<CachedAudio> _cachedAudio;
	Common::SharedPtr<AudioMetadata> _metadata;
	Common::SharedPtr<AudioPlayer> _player;
};

class PathMotionModifierV2 : public Modifier {
public:
	PathMotionModifierV2();

	bool load(ModifierLoaderContext &context, const Data::PathMotionModifierV2 &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Path Motion Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusPartial; }
#endif

private:
	struct PointDef {
		PointDef();

		Common::Point point;
		uint32 frame;
		bool useFrame;

		MessengerSendSpec sendSpec;
	};

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _executeWhen;
	Event _terminateWhen;

	bool _reverse;
	bool _loop;
	bool _alternate;
	bool _startAtBeginning;

	uint32 _frameDurationTimes10Million;

	Common::Array<PointDef> _points;

	DynamicValue _incomingData;
};

class DragMotionModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::DragMotionModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Drag Motion Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _enableWhen;
	Event _disableWhen;

	Common::SharedPtr<DragMotionProperties> _dragProps;
};

class VectorMotionModifier : public Modifier {
public:
	~VectorMotionModifier();

	bool load(ModifierLoaderContext &context, const Data::VectorMotionModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Vector Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
	void linkInternalReferences(ObjectLinkingScope *scope) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

	void trigger(Runtime *runtime);

	Event _enableWhen;
	Event _disableWhen;

	DynamicValue _vec;
	Common::WeakPtr<Modifier> _vecVar;

	AngleMagVector _resolvedVector;
	uint16 _subpixelX;
	uint16 _subpixelY;

	Common::SharedPtr<ScheduledEvent> _scheduledEvent;
	uint64 _lastTickTime;
};

class SceneTransitionModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::SceneTransitionModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Scene Transition Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _enableWhen;
	Event _disableWhen;

	uint32 _duration;	// 6000000 is maximum
	uint16 _steps;
	SceneTransitionTypes::SceneTransitionType _transitionType;
	SceneTransitionDirections::SceneTransitionDirection _transitionDirection;
};

class ElementTransitionModifier : public Modifier {
public:
	ElementTransitionModifier();
	~ElementTransitionModifier();

	bool load(ModifierLoaderContext &context, const Data::ElementTransitionModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

	enum TransitionType {
		kTransitionTypeRectangularIris,
		kTransitionTypeOvalIris,
		kTransitionTypeZoom,
		kTransitionTypeFade,
	};

	enum RevealType {
		kRevealTypeReveal,
		kRevealTypeConceal,
	};

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Element Transition Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusPartial; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	void continueTransition(Runtime *runtime);
	void completeTransition(Runtime *runtime);

	void setTransitionProgress(uint32 steps, uint32 maxSteps);

	Event _enableWhen;
	Event _disableWhen;

	uint32 _rate;	// Steps per second
	uint16 _steps;
	TransitionType _transitionType;
	RevealType _revealType;

	uint64 _transitionStartTime;
	uint32 _currentStep;

	Common::SharedPtr<ScheduledEvent> _scheduledEvent;
};

class IfMessengerModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::IfMessengerModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override {}

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "If Messenger Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	struct EvaluateAndSendTaskData {
		EvaluateAndSendTaskData() : runtime(nullptr) {}

		Common::SharedPtr<MiniscriptThread> thread;
		Runtime *runtime;
		DynamicValue incomingData;
	};

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
	void linkInternalReferences(ObjectLinkingScope *scope) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

	VThreadState evaluateAndSendTask(const EvaluateAndSendTaskData &taskData);

	Event _when;
	MessengerSendSpec _sendSpec;

	Common::SharedPtr<MiniscriptProgram> _program;
	Common::SharedPtr<MiniscriptReferences> _references;
};

class TimerMessengerModifier : public Modifier {
public:
	~TimerMessengerModifier();

	bool load(ModifierLoaderContext &context, const Data::TimerMessengerModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

	void linkInternalReferences(ObjectLinkingScope *outerScope) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Timer Messenger Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	void trigger(Runtime *runtime);

	Event _executeWhen;
	Event _terminateWhen;
	MessengerSendSpec _sendSpec;
	uint32 _milliseconds;
	bool _looping;
	DynamicValue _incomingData;

	Common::SharedPtr<ScheduledEvent> _scheduledEvent;
};

class BoundaryDetectionMessengerModifier : public Modifier, public IBoundaryDetector {
public:
	BoundaryDetectionMessengerModifier();
	~BoundaryDetectionMessengerModifier();

	bool load(ModifierLoaderContext &context, const Data::BoundaryDetectionMessengerModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

	void linkInternalReferences(ObjectLinkingScope *outerScope) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

	void getCollisionProperties(Modifier *&modifier, uint &edgeFlags, bool &mustBeCompletelyOutside, bool &continuous) const override;
	void triggerCollision(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Boundary Detection Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

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

	Runtime *_runtime;
	bool _isActive;
	DynamicValue _incomingData;
};

class CollisionDetectionMessengerModifier : public Modifier, public ICollider {
public:
	CollisionDetectionMessengerModifier();
	~CollisionDetectionMessengerModifier();

	bool load(ModifierLoaderContext &context, const Data::CollisionDetectionMessengerModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

	void linkInternalReferences(ObjectLinkingScope *scope) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Collision Detection Messenger Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusPartial; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	void getCollisionProperties(Modifier *&modifier, bool &collideInFront, bool &collideBehind, bool &excludeParents) const override;
	void triggerCollision(Runtime *runtime, Structural *collidingElement, bool wasInContact, bool isInContact, bool &outShouldStop) override;

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

	Runtime *_runtime;
	bool _isActive;

	DynamicValue _incomingData;
};

class KeyboardMessengerModifier : public Modifier {
public:
	KeyboardMessengerModifier();
	~KeyboardMessengerModifier();

	bool isKeyboardMessenger() const override;

	bool load(ModifierLoaderContext &context, const Data::KeyboardMessengerModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

	bool checkKeyEventTrigger(Runtime *runtime, Common::EventType evtType, bool repeat, const Common::KeyState &keyEvt, Common::String &outChar) const;
	void dispatchMessage(Runtime *runtime, const Common::String &charStr);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Keyboard Messenger Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;
	void linkInternalReferences(ObjectLinkingScope *scope) override;

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

	Event _send;

	bool _onDown : 1;
	bool _onUp : 1;
	bool _onRepeat : 1;
	bool _keyModControl : 1;
	bool _keyModCommand : 1;
	bool _keyModOption : 1;
	bool _isEnabled : 1;
	KeyCodeType _keyCodeType;
	char _macRomanChar;

	MessengerSendSpec _sendSpec;
};

class TextStyleModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::TextStyleModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Text Style Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	uint16 _macFontID;
	uint16 _size;
	ColorRGB8 _textColor;
	ColorRGB8 _backgroundColor;
	TextAlignment _alignment;
	TextStyleFlags _styleFlags;
	Event _applyWhen;
	Event _removeWhen;
	Common::String _fontFamilyName;
};

class GraphicModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::GraphicModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Graphic Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _applyWhen;
	Event _removeWhen;

	VisualElementRenderProperties _renderProps;
};

// Compound variable modifiers are not true variable modifiers.
// They aren't treated as values by Miniscript and they aren't
// treated as unique objects by aliases.  The only way that
// they behave like variable modifiers is that it's legal to
// put them inside of CompoundVariableModifiers.
class CompoundVariableModifier : public Modifier, public IModifierContainer {
public:
	bool load(ModifierLoaderContext &context, const Data::CompoundVariableModifier &data);

	void disable(Runtime *runtime) override;

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad() override;

	IModifierContainer *getChildContainer() override;

	bool isCompoundVariable() const override { return true; }

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Compound Variable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	class SaveLoad : public ModifierSaveLoad {
	public:
		explicit SaveLoad(CompoundVariableModifier *modifier);

		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;
		void commitLoad() const override;

	private:
		struct ChildSaveLoad {
			ChildSaveLoad();

			Modifier *modifier;
			Common::SharedPtr<ModifierSaveLoad> saveLoad;
		};

		Common::Array<ChildSaveLoad> _childrenSaveLoad;

		//CompoundVariableModifier *_modifier;
	};

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	const Common::Array<Common::SharedPtr<Modifier> > &getModifiers() const override;
	void appendModifier(const Common::SharedPtr<Modifier> &modifier) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	bool readAttributeIndexed(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib, const DynamicValue &index) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttributeIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib, const DynamicValue &index) override;

	Modifier *findChildByName(const Common::String &name) const;

	Common::Array<Common::SharedPtr<Modifier> > _children;
};

class BooleanVariableModifier : public VariableModifier {
public:
	bool load(ModifierLoaderContext &context, const Data::BooleanVariableModifier &data);

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad() override;

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(MiniscriptThread *thread, DynamicValue &dest) const override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Boolean Variable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	class SaveLoad : public ModifierSaveLoad {
	public:
		explicit SaveLoad(BooleanVariableModifier *modifier);

	private:
		void commitLoad() const override;
		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;

		BooleanVariableModifier *_modifier;
		bool _value;
	};

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	bool _value;
};

class IntegerVariableModifier : public VariableModifier {
public:
	bool load(ModifierLoaderContext &context, const Data::IntegerVariableModifier &data);

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad() override;

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(MiniscriptThread *thread, DynamicValue &dest) const override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Integer Variable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	class SaveLoad : public ModifierSaveLoad {
	public:
		explicit SaveLoad(IntegerVariableModifier *modifier);

	private:
		void commitLoad() const override;
		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;

		IntegerVariableModifier *_modifier;
		int32 _value;
	};

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	int32 _value;
};

class IntegerRangeVariableModifier : public VariableModifier {
public:
	bool load(ModifierLoaderContext &context, const Data::IntegerRangeVariableModifier &data);

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad() override;

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(MiniscriptThread *thread, DynamicValue &dest) const override;

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Integer Range Variable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	class SaveLoad : public ModifierSaveLoad {
	public:
		explicit SaveLoad(IntegerRangeVariableModifier *modifier);

	private:
		void commitLoad() const override;
		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;

		IntegerRangeVariableModifier *_modifier;
		IntRange _range;
	};

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	IntRange _range;
};

class VectorVariableModifier : public VariableModifier {
public:
	bool load(ModifierLoaderContext &context, const Data::VectorVariableModifier &data);

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad() override;

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(MiniscriptThread *thread, DynamicValue &dest) const override;

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Vector Variable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	class SaveLoad : public ModifierSaveLoad {
	public:
		explicit SaveLoad(VectorVariableModifier *modifier);

	private:
		void commitLoad() const override;
		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;

		VectorVariableModifier *_modifier;
		AngleMagVector _vector;
	};

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	AngleMagVector _vector;
};

class PointVariableModifier : public VariableModifier {
public:
	bool load(ModifierLoaderContext &context, const Data::PointVariableModifier &data);

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad() override;

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(MiniscriptThread *thread, DynamicValue &dest) const override;

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Point Variable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	class SaveLoad : public ModifierSaveLoad {
	public:
		explicit SaveLoad(PointVariableModifier *modifier);

	private:
		void commitLoad() const override;
		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;

		PointVariableModifier *_modifier;
		Common::Point _value;
	};

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Common::Point _value;
};

class FloatingPointVariableModifier : public VariableModifier {
public:
	bool load(ModifierLoaderContext &context, const Data::FloatingPointVariableModifier &data);

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad() override;

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(MiniscriptThread *thread, DynamicValue &dest) const override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Floating Point Variable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	class SaveLoad : public ModifierSaveLoad {
	public:
		explicit SaveLoad(FloatingPointVariableModifier *modifier);

	private:
		void commitLoad() const override;
		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;

		FloatingPointVariableModifier *_modifier;
		double _value;
	};

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	double _value;
};

class StringVariableModifier : public VariableModifier {
public:
	bool load(ModifierLoaderContext &context, const Data::StringVariableModifier &data);

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad() override;

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(MiniscriptThread *thread, DynamicValue &dest) const override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "String Variable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	class SaveLoad : public ModifierSaveLoad {
	public:
		explicit SaveLoad(StringVariableModifier *modifier);

	private:
		void commitLoad() const override;
		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;

		StringVariableModifier *_modifier;
		Common::String _value;
	};

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Common::String _value;
};

}	// End of namespace MTropolis

#endif
