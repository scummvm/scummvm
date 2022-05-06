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
		loaderContext.type = ChildLoaderContext::kTypeCountedModifierList;
		loaderContext.remainingCount = data.numChildren;

		context.childLoaderStack->contexts.push_back(loaderContext);
	}

	if (!_enableWhen.load(data.enableWhen) || !_disableWhen.load(data.disableWhen))
		return false;

	_guid = data.guid;
	_name = data.name;
	_modifierFlags.load(data.modifierFlags);
	_switchable = ((data.behaviorFlags & Data::BehaviorModifier::kBehaviorFlagSwitchable) != 0);
	_isEnabled = !_switchable;

	return true;
}

const Common::Array<Common::SharedPtr<Modifier> > &BehaviorModifier::getModifiers() const {
	return _children;
}

void BehaviorModifier::appendModifier(const Common::SharedPtr<Modifier> &modifier) {
	_children.push_back(modifier);
	modifier->setParent(getSelfReference());
}

IModifierContainer *BehaviorModifier::getMessagePropagationContainer() {
	if (_isEnabled)
		return this;
	else
		return nullptr;
}

IModifierContainer* BehaviorModifier::getChildContainer() {
	return this;
}

bool BehaviorModifier::respondsToEvent(const Event &evt) const {
	if (_switchable) {
		if (_enableWhen.respondsTo(evt))
			return true;
		if (_disableWhen.respondsTo(evt))
			return true;
	}
	return false;
}

VThreadState BehaviorModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_switchable) {
		if (_disableWhen.respondsTo(msg->getEvent())) {
			SwitchTaskData *taskData = runtime->getVThread().pushTask("BehaviorModifier::switchTask", this, &BehaviorModifier::switchTask);
			taskData->targetState = false;
			taskData->eventID = EventIDs::kParentDisabled;
			taskData->runtime = runtime;
		}
		if (_enableWhen.respondsTo(msg->getEvent())) {
			SwitchTaskData *taskData = runtime->getVThread().pushTask("BehaviorModifier::switchTask", this, &BehaviorModifier::switchTask);
			taskData->targetState = true;
			taskData->eventID = EventIDs::kParentEnabled;
			taskData->runtime = runtime;
		}
	}

	return kVThreadReturn;
}

VThreadState BehaviorModifier::switchTask(const SwitchTaskData &taskData) {
	if (_isEnabled != taskData.targetState) {
		_isEnabled = taskData.targetState;

		if (_children.size() > 0) {
			PropagateTaskData *propagateData = taskData.runtime->getVThread().pushTask("BehaviorModifier::propagateTask", this, &BehaviorModifier::propagateTask);
			propagateData->eventID = taskData.eventID;
			propagateData->index = 0;
			propagateData->runtime = taskData.runtime;
		}
	}

	return kVThreadReturn;
}

VThreadState BehaviorModifier::propagateTask(const PropagateTaskData &taskData) {
	if (taskData.index + 1 < _children.size()) {
		PropagateTaskData *propagateData = taskData.runtime->getVThread().pushTask("BehaviorModifier::propagateTask", this, &BehaviorModifier::propagateTask);
		propagateData->eventID = taskData.eventID;
		propagateData->index = taskData.index + 1;
		propagateData->runtime = taskData.runtime;
	}

	Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(taskData.eventID, 0), DynamicValue(), this->getSelfReference()));
	Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, _children[taskData.index].get(), true, true, false));
	taskData.runtime->sendMessageOnVThread(dispatch);

	return kVThreadReturn;
}

Common::SharedPtr<Modifier> BehaviorModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new BehaviorModifier(*this));
}

void BehaviorModifier::linkInternalReferences(ObjectLinkingScope *scope) {
	Modifier::linkInternalReferences(scope);
}

void BehaviorModifier::visitInternalReferences(IStructuralReferenceVisitor* visitor) {
	for (Common::Array<Common::SharedPtr<Modifier> >::iterator it = _children.begin(), itEnd = _children.end(); it != itEnd; ++it) {
		visitor->visitChildModifierRef(*it);
	}
}

// Miniscript modifier
bool MiniscriptModifier::load(ModifierLoaderContext &context, const Data::MiniscriptModifier &data) {
	if (!this->loadTypicalHeader(data.modHeader) || !_enableWhen.load(data.enableWhen))
		return false;

	if (!MiniscriptParser::parse(data.program, _program, _references))
		return false;

	return true;
}

bool MiniscriptModifier::respondsToEvent(const Event &evt) const {
	return _enableWhen.respondsTo(evt);
}

VThreadState MiniscriptModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_enableWhen.respondsTo(msg->getEvent())) {
		Common::SharedPtr<MiniscriptThread> thread(new MiniscriptThread(runtime, msg, _program, _references, this));
		MiniscriptThread::runOnVThread(runtime->getVThread(), thread);
	}

	return kVThreadReturn;
}

Common::SharedPtr<Modifier> MiniscriptModifier::shallowClone() const {
	MiniscriptModifier *clonePtr = new MiniscriptModifier(*this);
	Common::SharedPtr<Modifier> clone(clonePtr);

	// Keep the Miniscript program (which is static), but clone the references
	clonePtr->_references.reset(new MiniscriptReferences(*_references));

	return clone;
}

void MiniscriptModifier::linkInternalReferences(ObjectLinkingScope* scope) {
	_references->linkInternalReferences(scope);
}

void MiniscriptModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	_references->visitInternalReferences(visitor);
}

bool SaveAndRestoreModifier::load(ModifierLoaderContext &context, const Data::SaveAndRestoreModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_saveWhen.load(data.saveWhen) || !_restoreWhen.load(data.restoreWhen))
		return false;

	if (!_saveOrRestoreValue.load(data.saveOrRestoreValue, data.varName, data.varString))
		return false;

	_filePath = data.filePath;
	_fileName = data.fileName;

	return true;
}

bool SaveAndRestoreModifier::respondsToEvent(const Event &evt) const {
	if (_saveWhen.respondsTo(evt) || _restoreWhen.respondsTo(evt))
		return true;

	return false;
}

VThreadState SaveAndRestoreModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_saveWhen.respondsTo(msg->getEvent())) {
		error("Saves not implemented yet");
		return kVThreadReturn;
	} else if (_restoreWhen.respondsTo(msg->getEvent())) {
		error("Restores not implemented yet");
		return kVThreadReturn;
	}

	return kVThreadError;
}

Common::SharedPtr<Modifier> SaveAndRestoreModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new SaveAndRestoreModifier(*this));
}

bool MessengerModifier::load(ModifierLoaderContext &context, const Data::MessengerModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_when.load(data.when) || !_sendSpec.load(data.send, data.messageFlags, data.with, data.withSource, data.withString, data.destination))
		return false;

	return true;
}

bool MessengerModifier::respondsToEvent(const Event &evt) const {
	return _when.respondsTo(evt);
}

VThreadState MessengerModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_when.respondsTo(msg->getEvent())) {
		_sendSpec.sendFromMessenger(runtime, this);
	}

	return kVThreadReturn;
}

void MessengerModifier::linkInternalReferences(ObjectLinkingScope *outerScope) {
	_sendSpec.linkInternalReferences(outerScope);
}

void MessengerModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	_sendSpec.visitInternalReferences(visitor);
}

Common::SharedPtr<Modifier> MessengerModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new MessengerModifier(*this));
}

bool SetModifier::load(ModifierLoaderContext &context, const Data::SetModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_executeWhen.load(data.executeWhen) || !_source.load(data.source, data.sourceName, data.sourceString) || !_target.load(data.target, data.targetName, data.targetString))
		return false;

	return true;
}

Common::SharedPtr<Modifier> SetModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new SetModifier(*this));
}

bool AliasModifier::load(ModifierLoaderContext &context, const Data::AliasModifier &data) {
	_guid = data.guid;
	if (!_modifierFlags.load(data.modifierFlags))
		return false;
	_name = data.name;

	_aliasID = data.aliasIndexPlusOne;

	return true;
}

Common::SharedPtr<Modifier> AliasModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new AliasModifier(*this));
}

uint32 AliasModifier::getAliasID() const {
	return _aliasID;
}

bool AliasModifier::isAlias() const {
	return true;
}

bool ChangeSceneModifier::load(ModifierLoaderContext& context, const Data::ChangeSceneModifier& data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_executeWhen.load(data.executeWhen))
		return false;

	if ((data.changeSceneFlags & Data::ChangeSceneModifier::kChangeSceneFlagNextScene) != 0)
		_sceneSelectionType = kSceneSelectionTypeNext;
	else if ((data.changeSceneFlags & Data::ChangeSceneModifier::kChangeSceneFlagPrevScene) != 0)
		_sceneSelectionType = kSceneSelectionTypePrevious;
	else if ((data.changeSceneFlags & Data::ChangeSceneModifier::kChangeSceneFlagSpecificScene) != 0)
		_sceneSelectionType = kSceneSelectionTypeSpecific;
	else
		return false;

	_targetSectionGUID = data.targetSectionGUID;
	_targetSubsectionGUID = data.targetSubsectionGUID;
	_targetSceneGUID = data.targetSceneGUID;

	_addToReturnList = ((data.changeSceneFlags & Data::ChangeSceneModifier::kChangeSceneFlagAddToReturnList) != 0);
	_addToDestList = ((data.changeSceneFlags & Data::ChangeSceneModifier::kChangeSceneFlagAddToDestList) != 0);
	_wrapAround = ((data.changeSceneFlags & Data::ChangeSceneModifier::kChangeSceneFlagWrapAround) != 0);

	return true;
}

bool ChangeSceneModifier::respondsToEvent(const Event &evt) const {
	if (_executeWhen.respondsTo(evt))
		return true;

	return false;
}

VThreadState ChangeSceneModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties>& msg) {
	if (_executeWhen.respondsTo(msg->getEvent())) {
		Common::SharedPtr<Structural> targetScene;
		if (_sceneSelectionType == kSceneSelectionTypeSpecific) {
			Structural *project = runtime->getProject();
			Structural *section = nullptr;
			for (Common::Array<Common::SharedPtr<Structural> >::const_iterator it = project->getChildren().begin(), itEnd = project->getChildren().end(); it != itEnd; ++it) {
				Structural *candidate = it->get();
				assert(candidate->isSection());
				if (candidate->getStaticGUID() == _targetSectionGUID) {
					section = candidate;
					break;
				}
			}

			if (section) {
				Structural *subsection = nullptr;
				for (Common::Array<Common::SharedPtr<Structural> >::const_iterator it = section->getChildren().begin(), itEnd = section->getChildren().end(); it != itEnd; ++it) {
					Structural *candidate = it->get();
					assert(candidate->isSubsection());
					if (candidate->getStaticGUID() == _targetSubsectionGUID) {
						subsection = candidate;
						break;
					}
				}

				if (subsection) {
					for (Common::Array<Common::SharedPtr<Structural> >::const_iterator it = subsection->getChildren().begin(), itEnd = subsection->getChildren().end(); it != itEnd; ++it) {
						const Common::SharedPtr<Structural> &candidate = *it;
						assert(candidate->isElement() && static_cast<const Element *>(candidate.get())->isVisual());
						if (candidate->getStaticGUID() == _targetSceneGUID) {
							targetScene = candidate;
							break;
						}
					}
				} else {
					warning("Change Scene Modifier failed, subsection could not be resolved");
				}
			} else {
				warning("Change Scene Modifier failed, section could not be resolved");
			}
		} else {
			Structural *mainScene = runtime->getActiveMainScene().get();
			if (mainScene) {
				Structural *subsection = mainScene->getParent();

				const Common::Array<Common::SharedPtr<Structural> > &scenes = subsection->getChildren();
				if (scenes.size() == 1)
					error("Scene list is invalid");

				size_t sceneIndex = 0;
				for (size_t i = 1; i < scenes.size(); i++) {
					if (scenes[i].get() == mainScene) {
						sceneIndex = i;
						break;
					}
				}

				if (sceneIndex == 0) {
					warning("Change Scene Modifier failed, couldn't identify current scene's cyclical position");
				} else {
					if (_sceneSelectionType == kSceneSelectionTypePrevious) {
						if (sceneIndex == 1) {
							if (!_wrapAround)
								return kVThreadReturn;
							targetScene = scenes.back();
						} else {
							targetScene = scenes[sceneIndex - 1];
						}
					} else if (_sceneSelectionType == kSceneSelectionTypeNext) {

						if (sceneIndex == scenes.size() - 1) {
							if (!_wrapAround)
								return kVThreadReturn;
							targetScene = scenes[1];
						} else {
							targetScene = scenes[sceneIndex + 1];
						}
					}
				}
			}
		}

		if (targetScene) {
			runtime->addSceneStateTransition(HighLevelSceneTransition(targetScene, HighLevelSceneTransition::kTypeChangeToScene, _addToDestList, _addToReturnList));
		} else {
			warning("Change Scene Modifier failed, subsection could not be resolved");
		}
	}

	return kVThreadReturn;
}

Common::SharedPtr<Modifier> ChangeSceneModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ChangeSceneModifier(*this));
}

bool SoundEffectModifier::load(ModifierLoaderContext &context, const Data::SoundEffectModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_executeWhen.load(data.executeWhen) || !_terminateWhen.load(data.executeWhen))
		return false;

	if (data.assetID == Data::SoundEffectModifier::kSpecialAssetIDSystemBeep) {
		_soundType = kSoundTypeBeep;
		_assetID = 0;
	} else {
		_soundType = kSoundTypeAudioAsset;
		_assetID = data.assetID;
	}

	return true;
}

Common::SharedPtr<Modifier> SoundEffectModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new SoundEffectModifier(*this));
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

Common::SharedPtr<Modifier> DragMotionModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new DragMotionModifier(*this));
}

bool VectorMotionModifier::load(ModifierLoaderContext &context, const Data::VectorMotionModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_enableWhen.load(data.enableWhen) || !_disableWhen.load(data.disableWhen) || !_vec.load(data.vec, data.vecSource, data.vecString))
		return false;

	return true;
}

Common::SharedPtr<Modifier> VectorMotionModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new VectorMotionModifier(*this));
}

bool SceneTransitionModifier::load(ModifierLoaderContext &context, const Data::SceneTransitionModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_enableWhen.load(data.enableWhen) || !_disableWhen.load(data.disableWhen))
		return false;

	_duration = data.duration;
	_steps = data.steps;
	_transitionType = static_cast<TransitionType>(data.transitionType);
	_transitionDirection = static_cast<TransitionDirection>(data.direction);

	return true;
}

Common::SharedPtr<Modifier> SceneTransitionModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new SceneTransitionModifier(*this));
}

bool ElementTransitionModifier::load(ModifierLoaderContext &context, const Data::ElementTransitionModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_enableWhen.load(data.enableWhen) || !_disableWhen.load(data.disableWhen))
		return false;

	_rate = data.rate;
	_steps = data.steps;
	_transitionType = static_cast<TransitionType>(data.transitionType);
	_revealType = static_cast<RevealType>(data.revealType);

	return true;
}

Common::SharedPtr<Modifier> ElementTransitionModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ElementTransitionModifier(*this));
}

bool IfMessengerModifier::load(ModifierLoaderContext &context, const Data::IfMessengerModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_when.load(data.when) || !_sendSpec.load(data.send, data.messageFlags, data.with, data.withSource, data.withString, data.destination))
		return false;

	if (!MiniscriptParser::parse(data.program, _program, _references))
		return false;

	return true;
}

Common::SharedPtr<Modifier> IfMessengerModifier::shallowClone() const {
	IfMessengerModifier *clonePtr = new IfMessengerModifier(*this);
	Common::SharedPtr<Modifier> clone(clonePtr);

	// Keep the Miniscript program (which is static), but clone the references
	clonePtr->_references.reset(new MiniscriptReferences(*_references));

	return clone;
}

TimerMessengerModifier::~TimerMessengerModifier() {
	if (_scheduledEvent)
		_scheduledEvent->cancel();
}

bool TimerMessengerModifier::load(ModifierLoaderContext &context, const Data::TimerMessengerModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_executeWhen.load(data.executeWhen) || !this->_terminateWhen.load(data.terminateWhen))
		return false;

	if (!_sendSpec.load(data.send, data.messageAndTimerFlags, data.with, data.withSource, data.withString, data.destination))
		return false;

	_milliseconds = data.minutes * (60 * 1000) + data.seconds * (1000) + data.hundredthsOfSeconds * 10;
	_looping = ((data.messageAndTimerFlags & Data::TimerMessengerModifier::kTimerFlagLooping) != 0);

	return true;
}

bool TimerMessengerModifier::respondsToEvent(const Event &evt) const {
	return _executeWhen.respondsTo(evt) || _terminateWhen.respondsTo(evt);
}

VThreadState TimerMessengerModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	// If this terminates AND starts then just cancel out and terminate
	if (_terminateWhen.respondsTo(msg->getEvent())) {
		if (_scheduledEvent)
			_scheduledEvent->cancel();
	} else if (_executeWhen.respondsTo(msg->getEvent())) {
		if (!_scheduledEvent) {
			_scheduledEvent = runtime->getScheduler().scheduleMethod<TimerMessengerModifier, &TimerMessengerModifier::activate>(runtime->getPlayTime() + _milliseconds, this);
		}
	}

	return kVThreadReturn;
}

void TimerMessengerModifier::linkInternalReferences(ObjectLinkingScope *outerScope) {
	_sendSpec.linkInternalReferences(outerScope);
}

void TimerMessengerModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	_sendSpec.visitInternalReferences(visitor);
}

Common::SharedPtr<Modifier> TimerMessengerModifier::shallowClone() const {
	TimerMessengerModifier *clone = new TimerMessengerModifier(*this);
	clone->_scheduledEvent.reset();

	return Common::SharedPtr<Modifier>(clone);
}

void TimerMessengerModifier::activate(Runtime *runtime) {
	if (_looping)
		_scheduledEvent = runtime->getScheduler().scheduleMethod<TimerMessengerModifier, &TimerMessengerModifier::activate>(runtime->getPlayTime() + _milliseconds, this);
	else
		_scheduledEvent.reset();

	_sendSpec.sendFromMessenger(runtime, this);
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

	if (!_send.load(data.send, data.messageFlagsHigh << 16, data.with, data.withSource, data.withString, data.destination))
		return false;

	return true;
}

Common::SharedPtr<Modifier> BoundaryDetectionMessengerModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new BoundaryDetectionMessengerModifier(*this));
}

bool CollisionDetectionMessengerModifier::load(ModifierLoaderContext &context, const Data::CollisionDetectionMessengerModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_enableWhen.load(data.enableWhen) || !this->_disableWhen.load(data.disableWhen))
		return false;

	if (!_sendSpec.load(data.send, data.messageAndModifierFlags, data.with, data.withSource, data.withString, data.destination))
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

Common::SharedPtr<Modifier> CollisionDetectionMessengerModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new CollisionDetectionMessengerModifier(*this));
}

KeyboardMessengerModifier::~KeyboardMessengerModifier() {
	if (_signaller)
		_signaller->removeReceiver(this);
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
		memcpy(&_macRomanChar, &data.keycode, 1);
		break;
	}

	if (!_sendSpec.load(data.message, data.messageFlagsAndKeyStates, data.with, data.withSource, data.withString, data.destination))
		return false;

	return true;
}

bool KeyboardMessengerModifier::respondsToEvent(const Event &evt) const {
	if (Event::create(EventIDs::kParentEnabled, 0).respondsTo(evt) || Event::create(EventIDs::kParentDisabled, 0).respondsTo(evt))
		return true;

	return false;
}

VThreadState KeyboardMessengerModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
 	if (Event::create(EventIDs::kParentEnabled, 0).respondsTo(msg->getEvent())) {
		if (!_signaller)
			_signaller = runtime->getProject()->notifyOnKeyboardEvent(this);
	} else if (Event::create(EventIDs::kParentDisabled, 0).respondsTo(msg->getEvent())) {
		if (_signaller) {
			_signaller->removeReceiver(this);
			_signaller.reset();
		}
	}

	return kVThreadReturn;
}


Common::SharedPtr<Modifier> KeyboardMessengerModifier::shallowClone() const {
	Common::SharedPtr<KeyboardMessengerModifier> cloned(new KeyboardMessengerModifier(*this));

	cloned->_signaller.reset();
	return cloned;
}

void KeyboardMessengerModifier::onKeyboardEvent(Runtime *runtime, Common::EventType evtType, bool repeat, const Common::KeyState &keyEvt) {
	bool responds = false;
	if (evtType == Common::EVENT_KEYDOWN) {
		if (repeat)
			responds = _onRepeat;
		else
			responds = _onDown;
	} else if (evtType == Common::EVENT_KEYUP)
		responds = _onUp;

	if (!responds)
		return;

	if (_keyModCommand) {
		if (runtime->getPlatform() == kProjectPlatformWindows) {
			// Windows projects check "alt"
			if ((keyEvt.flags & Common::KBD_ALT) == 0)
				return;
		} else if (runtime->getPlatform() == kProjectPlatformMacintosh) {
			if ((keyEvt.flags & Common::KBD_META) == 0)
				return;
		}
	}

	if (_keyModControl) {
		if ((keyEvt.flags & Common::KBD_CTRL) == 0)
			return;
	}

	if (_keyModOption) {
		if ((keyEvt.flags & Common::KBD_ALT) == 0)
			return;
	}

	Common::String encodedChar;

	Common::String resolvedCharStr;

	KeyCodeType resolvedType = kAny;
	switch (keyEvt.keycode) {
	case Common::KEYCODE_HOME:
		resolvedType = kHome;
		break;
	case Common::KEYCODE_KP_ENTER:
		resolvedType = kEnter;
		break;
	case Common::KEYCODE_END:
		resolvedType = kEnd;
		break;
	case Common::KEYCODE_HELP:
		resolvedType = kHelp;
		break;
	case Common::KEYCODE_F1:
		// Windows projects map F1 to "help"
		if (runtime->getPlatform() == kProjectPlatformWindows)
			resolvedType = kHelp;
		break;
	case Common::KEYCODE_BACKSPACE:
		resolvedType = kBackspace;
		break;
	case Common::KEYCODE_TAB:
		resolvedType = kTab;
		break;
	case Common::KEYCODE_PAGEUP:
		resolvedType = kPageUp;
		break;
	case Common::KEYCODE_PAGEDOWN:
		resolvedType = kPageDown;
		break;
	case Common::KEYCODE_RETURN:
		resolvedType = kReturn;
		break;
	case Common::KEYCODE_ESCAPE:
		resolvedType = kEscape;
		break;
	case Common::KEYCODE_LEFT:
		resolvedType = kArrowLeft;
		break;
	case Common::KEYCODE_RIGHT:
		resolvedType = kArrowRight;
		break;
	case Common::KEYCODE_UP:
		resolvedType = kArrowUp;
		break;
	case Common::KEYCODE_DOWN:
		resolvedType = kDelete;
		break;
	default: {
			bool isQuestion = (keyEvt.ascii == '?');
			uint32 uchar = keyEvt.ascii;
			Common::U32String u(&uchar, 1);
			resolvedCharStr = u.encode(Common::kMacRoman);

			// STUPID HACK PLEASE FIX ME: ScummVM has no way of just telling us that the character mapping failed,
			// we we have to check if it encoded "?"
			if (resolvedCharStr.size() < 1 || (resolvedCharStr[0] == '?' && !isQuestion))
				return;

			resolvedType = kMacRomanChar;
		} break;
	}

	if (_keyCodeType != kAny && resolvedType != _keyCodeType)
		return;

	if (_keyCodeType == kMacRomanChar && (resolvedCharStr.size() == 0 || resolvedCharStr[0] != _macRomanChar))
		return;

	Common::SharedPtr<MessageProperties> msgProps;
	if (_sendSpec.with.getType() == DynamicValueTypes::kIncomingData) {
		if (resolvedCharStr.size() != 1)
			warning("Keyboard messenger is supposed to send the character code, but they key was a special key and we haven't implemented conversion of those keycodes");

		DynamicValue charStr;
		charStr.setString(resolvedCharStr);
		_sendSpec.sendFromMessengerWithCustomData(runtime, this, charStr);
	} else {
		_sendSpec.sendFromMessenger(runtime, this);
	}
}

void KeyboardMessengerModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	_sendSpec.visitInternalReferences(visitor);
}

void KeyboardMessengerModifier::linkInternalReferences(ObjectLinkingScope *scope) {
	_sendSpec.linkInternalReferences(scope);
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

Common::SharedPtr<Modifier> TextStyleModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new TextStyleModifier(*this));
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

Common::SharedPtr<Modifier> GraphicModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new GraphicModifier(*this));
}

bool CompoundVariableModifier::load(ModifierLoaderContext &context, const Data::CompoundVariableModifier &data) {
	if (data.numChildren > 0) {
		ChildLoaderContext loaderContext;
		loaderContext.containerUnion.modifierContainer = this;
		loaderContext.type = ChildLoaderContext::kTypeCountedModifierList;
		loaderContext.remainingCount = data.numChildren;

		context.childLoaderStack->contexts.push_back(loaderContext);
	}

	if (!_modifierFlags.load(data.modifierFlags))
		return false;
	_guid = data.guid;
	_name = data.name;

	return true;
}

IModifierContainer *CompoundVariableModifier::getChildContainer() {
	return this;
}

const Common::Array<Common::SharedPtr<Modifier> > &CompoundVariableModifier::getModifiers() const {
	return _children;
}

void CompoundVariableModifier::appendModifier(const Common::SharedPtr<Modifier>& modifier) {
	_children.push_back(modifier);
	modifier->setParent(getSelfReference());
}

void CompoundVariableModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	for (Common::Array<Common::SharedPtr<Modifier> >::iterator it = _children.begin(), itEnd = _children.end(); it != itEnd; ++it) {
		visitor->visitChildModifierRef(*it);
	}
}

bool CompoundVariableModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	Modifier *var = findChildByName(attrib);
	if (var) {
		if (var->isVariable()) {
			static_cast<VariableModifier *>(var)->varGetValue(thread, result);
		} else {
			result.setObject(var->getSelfReference());
		}
		return true;
	}
	return Modifier::readAttribute(thread, result, attrib);
}

bool CompoundVariableModifier::readAttributeIndexed(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib, const DynamicValue &index) {
	Modifier *var = findChildByName(attrib);
	if (!var || !var->isVariable())
		return false;

	return var->readAttributeIndexed(thread, result, "value", index);
}

MiniscriptInstructionOutcome CompoundVariableModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	Modifier *var = findChildByName(attrib);
	if (!var)
		return kMiniscriptInstructionOutcomeFailed;

	if (var->isVariable())
		writeProxy = static_cast<VariableModifier *>(var)->createWriteProxy();
	else if (var->isModifier())
		DynamicValueWriteObjectHelper::create(var, writeProxy);
	else
		return kMiniscriptInstructionOutcomeFailed;

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome CompoundVariableModifier::writeRefAttributeIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib, const DynamicValue &index) {
	Modifier *var = findChildByName(attrib);
	if (!var || !var->isModifier())
		return kMiniscriptInstructionOutcomeFailed;

	return var->writeRefAttributeIndexed(thread, writeProxy, "value", index);
}

Modifier *CompoundVariableModifier::findChildByName(const Common::String &name) const {
	for (Common::Array<Common::SharedPtr<Modifier> >::const_iterator it = _children.begin(), itEnd = _children.end(); it != itEnd; ++it) {
		Modifier *modifier = it->get();
		if (caseInsensitiveEqual(name, modifier->getName()))
			return modifier;
	}

	return nullptr;
}

Common::SharedPtr<Modifier> CompoundVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new CompoundVariableModifier(*this));
}

bool BooleanVariableModifier::load(ModifierLoaderContext &context, const Data::BooleanVariableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_value = (data.value != 0);

	return true;
}

bool BooleanVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kBoolean)
		_value = value.getBool();
	else
		return false;

	return true;
}

void BooleanVariableModifier::varGetValue(MiniscriptThread *thread, DynamicValue &dest) const {
	dest.setBool(_value);
}

Common::SharedPtr<Modifier> BooleanVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new BooleanVariableModifier(*this));
}

bool IntegerVariableModifier::load(ModifierLoaderContext& context, const Data::IntegerVariableModifier& data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_value = data.value;

	return true;
}

bool IntegerVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kFloat)
		_value = static_cast<int32>(floor(value.getFloat() + 0.5));
	else if (value.getType() == DynamicValueTypes::kInteger)
		_value = value.getInt();
	else
		return false;

	return true;
}

void IntegerVariableModifier::varGetValue(MiniscriptThread *thread, DynamicValue &dest) const {
	dest.setInt(_value);
}

Common::SharedPtr<Modifier> IntegerVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new IntegerVariableModifier(*this));
}

bool IntegerRangeVariableModifier::load(ModifierLoaderContext& context, const Data::IntegerRangeVariableModifier& data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_range.load(data.range))
		return false;

	return true;
}

bool IntegerRangeVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kIntegerRange)
		_range = value.getIntRange();
	else
		return false;

	return true;
}

void IntegerRangeVariableModifier::varGetValue(MiniscriptThread *thread, DynamicValue &dest) const {
	dest.setIntRange(_range);
}

Common::SharedPtr<Modifier> IntegerRangeVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new IntegerRangeVariableModifier(*this));
}

bool VectorVariableModifier::load(ModifierLoaderContext &context, const Data::VectorVariableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_vector.angleRadians = data.vector.angleRadians.toDouble();
	_vector.magnitude = data.vector.magnitude.toDouble();

	return true;
}

bool VectorVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kVector)
		_vector = value.getVector();
	else
		return false;

	return true;
}

void VectorVariableModifier::varGetValue(MiniscriptThread *thread, DynamicValue &dest) const {
	dest.setVector(_vector);
}

Common::SharedPtr<Modifier> VectorVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new VectorVariableModifier(*this));
}

bool PointVariableModifier::load(ModifierLoaderContext &context, const Data::PointVariableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_value.x = data.value.x;
	_value.y = data.value.y;

	return true;
}

bool PointVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kPoint)
		_value = value.getPoint();
	else
		return false;

	return true;
}

void PointVariableModifier::varGetValue(MiniscriptThread *thread, DynamicValue &dest) const {
	dest.setPoint(_value);
}

Common::SharedPtr<Modifier> PointVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new PointVariableModifier(*this));
}

bool FloatingPointVariableModifier::load(ModifierLoaderContext &context, const Data::FloatingPointVariableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_value = data.value.toDouble();

	return true;
}

bool FloatingPointVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kInteger)
		_value = value.getInt();
	else if (value.getType() == DynamicValueTypes::kFloat)
		_value = value.getFloat();
	else
		return false;

	return true;
}

void FloatingPointVariableModifier::varGetValue(MiniscriptThread *thread, DynamicValue &dest) const {
	dest.setFloat(_value);
}

Common::SharedPtr<Modifier> FloatingPointVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new FloatingPointVariableModifier(*this));
}

bool StringVariableModifier::load(ModifierLoaderContext &context, const Data::StringVariableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_value = data.value;

	return true;
}

bool StringVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kString)
		_value = value.getString();
	else
		return false;

	return true;
}

void StringVariableModifier::varGetValue(MiniscriptThread *thread, DynamicValue &dest) const {
	dest.setString(_value);
}

Common::SharedPtr<Modifier> StringVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new StringVariableModifier(*this));
}

} // End of namespace MTropolis
