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
	BehaviorModifier();

	bool load(ModifierLoaderContext &context, const Data::BehaviorModifier &data);

	bool isBehavior() const override { return true; }

	const Common::Array<Common::SharedPtr<Modifier> > &getModifiers() const override;
	void appendModifier(const Common::SharedPtr<Modifier> &modifier) override;
	void removeModifier(const Modifier *modifier) override;

	IModifierContainer *getMessagePropagationContainer() override;
	IModifierContainer *getChildContainer() override;

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Behavior Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	void debugInspect(IDebugInspectionReport *report) const override;
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

class ColorTableModifier : public Modifier {
public:
	ColorTableModifier();

	bool load(ModifierLoaderContext &context, const Data::ColorTableModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override {}

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Color Table Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _applyWhen;

	uint32 _assetID;
};

class SoundFadeModifier : public Modifier {
public:
	SoundFadeModifier();

	bool load(ModifierLoaderContext &context, const Data::SoundFadeModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override {}

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Sound Fade Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusNone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _enableWhen;
	Event _disableWhen;

	uint16 _fadeToVolume;
	uint32 _durationMSec;
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

protected:
	void linkInternalReferences(ObjectLinkingScope *scope) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _saveWhen;
	Event _restoreWhen;

	DynamicValueSource _saveOrRestoreValue;

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

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override {}

	void linkInternalReferences(ObjectLinkingScope *outerScope) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Set Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _executeWhen;
	DynamicValueSource _source;
	DynamicValueSource _target;
};

class AliasModifier : public Modifier {
public:
	AliasModifier();

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
	SoundEffectModifier();

	bool load(ModifierLoaderContext &context, const Data::SoundEffectModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Sound Effect Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
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

class PathMotionModifier : public Modifier {
public:
	PathMotionModifier();
	~PathMotionModifier();

	bool load(ModifierLoaderContext &context, const Data::PathMotionModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

	void linkInternalReferences(ObjectLinkingScope *scope) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

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

	struct ExecuteTaskData {
		ExecuteTaskData() : runtime(nullptr) {}

		Runtime *runtime;
	};

	struct TerminateTaskData {
		TerminateTaskData() : runtime(nullptr) {}

		Runtime *runtime;
	};

	struct ChangePointsTaskData {
		ChangePointsTaskData() : runtime(nullptr), prevPoint(0), newPoint(0), isTerminal(false) {}

		Runtime *runtime;
		uint prevPoint;
		uint newPoint;
		bool isTerminal;
	};

	struct TriggerMessageTaskData {
		TriggerMessageTaskData() : runtime(nullptr), pointIndex(0) {}

		Runtime *runtime;
		uint pointIndex;
	};

	struct SendMessageToParentTaskData {
		SendMessageToParentTaskData() : runtime(nullptr), eventID(EventIDs::kNothing) {}

		Runtime *runtime;
		EventIDs::EventID eventID;
	};

	struct ChangeCelTaskData {
		ChangeCelTaskData() : runtime(nullptr), pointIndex(0) {}

		Runtime *runtime;
		uint pointIndex;
	};

	struct ChangePositionTaskData {
		ChangePositionTaskData() : runtime(nullptr) {}

		Runtime *runtime;
		Common::Point positionDelta;
	};

	struct AdvanceFrameTaskData {
		AdvanceFrameTaskData() : runtime(nullptr), terminationTimeDUSec(0) {}

		Runtime *runtime;
		uint64 terminationTimeDUSec;
	};

	VThreadState executeTask(const ExecuteTaskData &taskData);
	VThreadState terminateTask(const TerminateTaskData &taskData);
	VThreadState changePointsTask(const ChangePointsTaskData &taskData);
	VThreadState triggerMessageTask(const TriggerMessageTaskData &taskData);
	VThreadState sendMessageToParentTask(const SendMessageToParentTaskData &taskData);
	VThreadState changeCelTask(const ChangeCelTaskData &taskData);
	VThreadState changePositionTask(const ChangePositionTaskData &taskData);
	VThreadState advanceFrameTask(const AdvanceFrameTaskData &taskData);

	void scheduleNextAdvance(Runtime *runtime, uint64 startingFromTimeDUSec);
	void advance(Runtime *runtime);

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _executeWhen;
	Event _terminateWhen;

	bool _reverse;
	bool _loop;
	bool _alternate;
	bool _startAtBeginning;

	uint64 _frameDurationDUSec;

	Common::Array<PointDef> _points;

	DynamicValue _incomingData;

	Common::WeakPtr<RuntimeObject> _triggerSource;
	Common::SharedPtr<ScheduledEvent> _scheduledEvent;
	bool _isAlternatingDirection;
	uint _currentPointIndex;
	uint64 _lastPointTimeDUSec;
};

class SimpleMotionModifier : public Modifier {
public:
	SimpleMotionModifier();
	~SimpleMotionModifier();

	bool load(ModifierLoaderContext &context, const Data::SimpleMotionModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Simple Motion Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusNone; }
#endif

private:
	enum MotionType {
		kMotionTypeOutOfScene = 1,
		kMotionTypeIntoScene = 2,
		kMotionTypeRandomBounce = 3,
	};

	enum DirectionFlags {
		kDirectionFlagDown = 1,
		kDirectionFlagUp = 2,
		kDirectionFlagRight = 4,
		kDirectionFlagLeft = 8,
	};

	void startRandomBounce(Runtime *runtime);
	void runRandomBounce(Runtime *runtime);

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _executeWhen;
	Event _terminateWhen;

	MotionType _motionType;
	uint16 _directionFlags;
	uint16 _steps;
	uint32 _delayMSecTimes4800;

	uint64 _lastTickTime;

	Common::Point _velocity;

	Common::SharedPtr<ScheduledEvent> _scheduledEvent;
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

	DynamicValueSource _vec;

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
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
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

class SharedSceneModifier : public Modifier {
public:
	SharedSceneModifier();
	~SharedSceneModifier();

	bool load(ModifierLoaderContext &context, const Data::SharedSceneModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Shared Scene Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _executeWhen;

	uint32 _targetSectionGUID;
	uint32 _targetSubsectionGUID;
	uint32 _targetSceneGUID;
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
	struct RunEvaluateAndSendCoroutine {
		CORO_DEFINE_RETURN_TYPE(void);
		CORO_DEFINE_PARAMS_3(IfMessengerModifier *, self, Runtime *, runtime, Common::SharedPtr<MessageProperties>, msg);
	};

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
	void linkInternalReferences(ObjectLinkingScope *scope) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

	Event _when;
	MessengerSendSpec _sendSpec;

	Common::SharedPtr<MiniscriptProgram> _program;
	Common::SharedPtr<MiniscriptReferences> _references;
};

class TimerMessengerModifier : public Modifier {
public:
	TimerMessengerModifier();
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
	Common::WeakPtr<RuntimeObject> _triggerSource;
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
	Common::WeakPtr<RuntimeObject> _triggerSource;
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

	struct EnableTaskData {
	};

	struct DisableTaskData {
	};

	VThreadState enableTask(const EnableTaskData &taskData);
	VThreadState disableTask(const DisableTaskData &taskData);

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
	Common::WeakPtr<RuntimeObject> _triggerSource;
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

class ImageEffectModifier : public Modifier {
public:
	ImageEffectModifier();

	bool load(ModifierLoaderContext &context, const Data::ImageEffectModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Image Effect Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusNone; }
#endif

private:
	enum Type {
		kTypeUnknown = 0,

		kTypeInvert = 1,
		kTypeSelectedBevels,
		kTypeDeselectedBevels,
		kTypeToneDown,
		kTypeToneUp,
	};

	struct ApplyTaskData {
		ApplyTaskData() : runtime(nullptr) {}

		Runtime *runtime;
	};

	struct RemoveTaskData {
		RemoveTaskData() : runtime(nullptr) {}

		Runtime *runtime;
	};

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	VThreadState applyTask(const ApplyTaskData &taskData);
	VThreadState removeTask(const RemoveTaskData &taskData);

	Event _applyWhen;
	Event _removeWhen;
	Type _type;
	uint16 _bevelWidth;
	uint16 _toneAmount;
	bool _includeBorders;
};

class ReturnModifier : public Modifier {
public:
	ReturnModifier();

	bool load(ModifierLoaderContext &context, const Data::ReturnModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Return Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusNone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _executeWhen;
};

class CursorModifierV1 : public Modifier {
public:
	CursorModifierV1();

	bool load(ModifierLoaderContext &context, const Data::CursorModifierV1 &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Cursor Modifier V1"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusNone; }
#endif

private:
	enum {
		kCursor_Inactive,
		kCursor_Interact,
		kCursor_HandGrabBW,
		kCursor_HandOpenBW,
		kCursor_HandPointUp,
		kCursor_HandPointRight,
		kCursor_HandPointLeft,
		kCursor_HandPointDown,
		kCursor_HandGrabColor,
		kCursor_HandOpenColor,
		kCursor_Arrow,
		kCursor_Pencil,
		kCursor_Smiley,
		kCursor_Wait,
		kCursor_Hidden,
	};

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _applyWhen;
	uint32 _cursorIndex;
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

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad(Runtime *runtime) override;

	IModifierContainer *getChildContainer() override;

	bool isCompoundVariable() const override { return true; }

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Compound Variable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	class SaveLoad : public ModifierSaveLoad {
	public:
		SaveLoad(Runtime *runtime, CompoundVariableModifier *modifier);

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
	void removeModifier(const Modifier *modifier) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	bool readAttributeIndexed(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib, const DynamicValue &index) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttributeIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib, const DynamicValue &index) override;

	Modifier *findChildByName(Runtime *runtime, const Common::String &name) const;

	Common::Array<Common::SharedPtr<Modifier> > _children;
};

class BooleanVariableModifier : public VariableModifier {
public:
	BooleanVariableModifier();

	bool load(ModifierLoaderContext &context, const Data::BooleanVariableModifier &data);

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(DynamicValue &dest) const override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Boolean Variable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class BooleanVariableStorage : public VariableStorage {
public:
	friend class BooleanVariableModifier;

	BooleanVariableStorage();

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad(Runtime *runtime) override;

	Common::SharedPtr<VariableStorage> clone() const override;

private:
	class SaveLoad : public ModifierSaveLoad {
	public:
		explicit SaveLoad(BooleanVariableStorage *modifier);

	private:
		void commitLoad() const override;
		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;

		BooleanVariableStorage *_storage;
		bool _value;
	};

	bool _value;
};

class IntegerVariableModifier : public VariableModifier {
public:
	IntegerVariableModifier();

	bool load(ModifierLoaderContext &context, const Data::IntegerVariableModifier &data);

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(DynamicValue &dest) const override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Integer Variable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class IntegerVariableStorage : public VariableStorage {
public:
	friend class IntegerVariableModifier;

	IntegerVariableStorage();

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad(Runtime *runtime) override;

	Common::SharedPtr<VariableStorage> clone() const override;

private:
	class SaveLoad : public ModifierSaveLoad {
	public:
		explicit SaveLoad(IntegerVariableStorage *storage);

	private:
		void commitLoad() const override;
		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;

		IntegerVariableStorage *_storage;
		int32 _value;
	};

	int32 _value;
};

class IntegerRangeVariableModifier : public VariableModifier {
public:
	IntegerRangeVariableModifier();

	bool load(ModifierLoaderContext &context, const Data::IntegerRangeVariableModifier &data);

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(DynamicValue &dest) const override;

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Integer Range Variable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class IntegerRangeVariableStorage : public VariableStorage {
public:
	friend class IntegerRangeVariableModifier;

	IntegerRangeVariableStorage();

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad(Runtime *runtime) override;

	Common::SharedPtr<VariableStorage> clone() const override;

private:
	class SaveLoad : public ModifierSaveLoad {
	public:
		explicit SaveLoad(IntegerRangeVariableStorage *storage);

	private:
		void commitLoad() const override;
		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;

		IntegerRangeVariableStorage *_storage;
		IntRange _range;
	};

	IntRange _range;
};

class VectorVariableModifier : public VariableModifier {
public:
	VectorVariableModifier();

	bool load(ModifierLoaderContext &context, const Data::VectorVariableModifier &data);

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(DynamicValue &dest) const override;

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Vector Variable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class VectorVariableStorage : public VariableStorage {
	friend class VectorVariableModifier;

	VectorVariableStorage();

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad(Runtime *runtime) override;

	Common::SharedPtr<VariableStorage> clone() const override;

private:
	class SaveLoad : public ModifierSaveLoad {
	public:
		explicit SaveLoad(VectorVariableStorage *storage);

	private:
		void commitLoad() const override;
		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;

		VectorVariableStorage *_storage;
		AngleMagVector _vector;
	};

	AngleMagVector _vector;
};

class PointVariableModifier : public VariableModifier {
public:
	PointVariableModifier();

	bool load(ModifierLoaderContext &context, const Data::PointVariableModifier &data);

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(DynamicValue &dest) const override;

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Point Variable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class PointVariableStorage : public VariableStorage {
public:
	friend class PointVariableModifier;

	PointVariableStorage();

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad(Runtime *runtime) override;

	Common::SharedPtr<VariableStorage> clone() const override;

private:
	class SaveLoad : public ModifierSaveLoad {
	public:
		explicit SaveLoad(PointVariableStorage *storage);

	private:
		void commitLoad() const override;
		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;

		PointVariableStorage *_storage;
		Common::Point _value;
	};

	Common::Point _value;
};

class FloatingPointVariableModifier : public VariableModifier {
public:
	FloatingPointVariableModifier();

	bool load(ModifierLoaderContext &context, const Data::FloatingPointVariableModifier &data);

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(DynamicValue &dest) const override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Floating Point Variable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class FloatingPointVariableStorage : public VariableStorage {
public:
	friend class FloatingPointVariableModifier;

	FloatingPointVariableStorage();

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad(Runtime *runtime) override;

	Common::SharedPtr<VariableStorage> clone() const override;

private:
	class SaveLoad : public ModifierSaveLoad {
	public:
		explicit SaveLoad(FloatingPointVariableStorage *storage);

	private:
		void commitLoad() const override;
		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;

		FloatingPointVariableStorage *_storage;
		double _value;
	};

	double _value;
};

class StringVariableModifier : public VariableModifier {
public:
	StringVariableModifier();

	bool load(ModifierLoaderContext &context, const Data::StringVariableModifier &data);

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(DynamicValue &dest) const override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "String Variable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class StringVariableStorage : public VariableStorage {
public:
	friend class StringVariableModifier;

	StringVariableStorage();

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad(Runtime *runtime) override;

	Common::SharedPtr<VariableStorage> clone() const override;

private:
	class SaveLoad : public ModifierSaveLoad {
	public:
		explicit SaveLoad(StringVariableStorage *storage);

	private:
		void commitLoad() const override;
		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;

		StringVariableStorage *_storage;
		Common::String _value;
	};

	Common::String _value;
};

class ObjectReferenceVariableModifierV1 : public VariableModifier {
public:
	ObjectReferenceVariableModifierV1();

	bool load(ModifierLoaderContext &context, const Data::ObjectReferenceVariableModifierV1 &data);

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(DynamicValue &dest) const override;

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Object Reference Variable Modifier V1"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusNone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _setToSourcesParentWhen;
};

class ObjectReferenceVariableV1Storage : public VariableStorage {
public:
	friend class ObjectReferenceVariableModifierV1;

	ObjectReferenceVariableV1Storage();

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad(Runtime *runtime) override;

	Common::SharedPtr<VariableStorage> clone() const override;

private:
	class SaveLoad : public ModifierSaveLoad {
	public:
		explicit SaveLoad(ObjectReferenceVariableV1Storage *storage);

	private:
		void commitLoad() const override;
		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;

		ObjectReferenceVariableV1Storage *_storage;
		Common::WeakPtr<RuntimeObject> _value;
	};

	Common::WeakPtr<RuntimeObject> _value;
};

}	// End of namespace MTropolis

#endif
