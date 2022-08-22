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

#include "common/memstream.h"

#include "mtropolis/assets.h"
#include "mtropolis/audio_player.h"
#include "mtropolis/miniscript.h"
#include "mtropolis/modifiers.h"
#include "mtropolis/modifier_factory.h"
#include "mtropolis/saveload.h"

#include "mtropolis/elements.h"

namespace MTropolis {

class CompoundVarLoader : public ISaveReader {
public:
	explicit CompoundVarLoader(RuntimeObject *object);

	bool readSave(Common::ReadStream *stream, uint32 saveFileVersion) override;

private:
	RuntimeObject *_object;
};

CompoundVarLoader::CompoundVarLoader(RuntimeObject *object) : _object(object) {
}

bool CompoundVarLoader::readSave(Common::ReadStream *stream, uint32 saveFileVersion) {
	if (_object == nullptr || !_object->isModifier())
		return false;

	Modifier *modifier = static_cast<Modifier *>(_object);
	Common::SharedPtr<ModifierSaveLoad> saveLoad = modifier->getSaveLoad();
	if (!saveLoad)
		return false;

	if (!saveLoad->load(modifier, stream, saveFileVersion))
		return false;

	if (stream->err())
		return false;

	saveLoad->commitLoad();

	return true;
}

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
			// These are executed in reverse order.  The disable event is propagated to children, then the disable task
			// runs to forcibly disable any children.
			//
			// This works a bit weirdly in practice with child behaviors since ultimately we want them to be disabled and
			// fire their Parent Disabled task but we don't actually do it on disable.  We instead rely on it being kind of
			// the logical outcome of how this works:
			//
			// If the behavior is enabled, then Parent Disabled will propagate through the behavior, followed by the children
			// actually being disabled.
			DisableTaskData *disableTask = runtime->getVThread().pushTask("BehaviorModifier::disableTask", this, &BehaviorModifier::disableTask);
			disableTask->runtime = runtime;

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

void BehaviorModifier::disable(Runtime *runtime) {
	if (_switchable && _isEnabled)
		_isEnabled = false;

	for (const Common::SharedPtr<Modifier> &child : _children)
		child->disable(runtime);
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

	Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(taskData.eventID, 0), DynamicValue(), this->getSelfReference()));
	Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, _children[taskData.index].get(), true, true, false));
	taskData.runtime->sendMessageOnVThread(dispatch);

	return kVThreadReturn;
}

VThreadState BehaviorModifier::disableTask(const DisableTaskData &taskData) {
	disable(taskData.runtime);
	return kVThreadReturn;
}

Common::SharedPtr<Modifier> BehaviorModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new BehaviorModifier(*this));
}

const char *BehaviorModifier::getDefaultName() const {
	return "Behavior";
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

const char *MiniscriptModifier::getDefaultName() const {
	return "Miniscript Modifier";
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
	if (_saveOrRestoreValue.getType() != DynamicValueTypes::kVariableReference) {
		warning("Save/restore failed, don't know how to use something that isn't a var reference");
		return kVThreadError;
	}

	const VarReference &var = _saveOrRestoreValue.getVarReference();

	Common::WeakPtr<RuntimeObject> objWeak;
	var.resolve(this, objWeak);

	if (objWeak.expired()) {
		warning("Save/load failed, couldn't resolve compound var");
		return kVThreadError;
	}

	RuntimeObject *obj = objWeak.lock().get();
	if (!obj->isModifier()) {
		warning("Save/load failed, source wasn't a modifier");
		return kVThreadError;
	}

	if (_saveWhen.respondsTo(msg->getEvent())) {
		CompoundVarSaver saver(obj);
		if (runtime->getSaveProvider()->promptSave(&saver, runtime->getSaveScreenshotOverride().get())) {
			for (const Common::SharedPtr<SaveLoadHooks> &hooks : runtime->getHacks().saveLoadHooks)
				hooks->onSave(runtime, this, static_cast<Modifier *>(obj));
		}
		return kVThreadReturn;
	} else if (_restoreWhen.respondsTo(msg->getEvent())) {
		CompoundVarLoader loader(obj);
		if (runtime->getLoadProvider()->promptLoad(&loader)) {
			for (const Common::SharedPtr<SaveLoadHooks> &hooks : runtime->getHacks().saveLoadHooks)
				hooks->onLoad(runtime, this, static_cast<Modifier *>(obj));
		}
		return kVThreadReturn;
	}

	return kVThreadError;
}

Common::SharedPtr<Modifier> SaveAndRestoreModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new SaveAndRestoreModifier(*this));
}

const char *SaveAndRestoreModifier::getDefaultName() const {
	return "Save And Restore Modifier";
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
		_sendSpec.sendFromMessenger(runtime, this, msg->getValue(), nullptr);
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

const char *MessengerModifier::getDefaultName() const {
	return "Messenger";
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

const char *SetModifier::getDefaultName() const {
	return "Set Modifier";
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

const char *AliasModifier::getDefaultName() const {
	return "";
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

			if (_targetSectionGUID == 0xfffffffeu) {
				// For some reason, some scene change modifiers have a garbled section ID.  In that case, look in the current section.
				Structural *sectionSearch = findStructuralOwner();
				while (sectionSearch && !sectionSearch->isSection())
					sectionSearch = sectionSearch->getParent();

				section = sectionSearch;
			} else {
				for (Common::Array<Common::SharedPtr<Structural> >::const_iterator it = project->getChildren().begin(), itEnd = project->getChildren().end(); it != itEnd; ++it) {
					Structural *candidate = it->get();
					assert(candidate->isSection());
					if (candidate->getStaticGUID() == _targetSectionGUID) {
						section = candidate;
						break;
					}
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
			warning("Change Scene Modifier failed, scene could not be resolved");
		}
	}

	return kVThreadReturn;
}

Common::SharedPtr<Modifier> ChangeSceneModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ChangeSceneModifier(*this));
}

const char *ChangeSceneModifier::getDefaultName() const {
	return "Change Scene Modifier";
}

bool SoundEffectModifier::load(ModifierLoaderContext &context, const Data::SoundEffectModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_executeWhen.load(data.executeWhen) || !_terminateWhen.load(data.terminateWhen))
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

bool SoundEffectModifier::respondsToEvent(const Event &evt) const {
	return _executeWhen.respondsTo(evt) || _terminateWhen.respondsTo(evt);
}

VThreadState SoundEffectModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_terminateWhen.respondsTo(msg->getEvent())) {
		if (_player) {
			_player->stop();
			_player.reset();
		}
	} else if (_executeWhen.respondsTo(msg->getEvent())) {
		disable(runtime);
	}

	return kVThreadReturn;
}

void SoundEffectModifier::disable(Runtime *runtime) {
	if (_soundType == kSoundTypeAudioAsset) {
		if (!_cachedAudio)
			loadAndCacheAudio(runtime);

		if (_cachedAudio) {
			if (_player) {
				_player->stop();
				_player.reset();
			}

			size_t numSamples = _cachedAudio->getNumSamples(*_metadata);
			_player.reset(new AudioPlayer(runtime->getAudioMixer(), 255, 0, _metadata, _cachedAudio, false, 0, 0, numSamples));
		}
	}
}

void SoundEffectModifier::loadAndCacheAudio(Runtime *runtime) {
	if (_cachedAudio)
		return;

	Project *project = runtime->getProject();
	Common::SharedPtr<Asset> asset = project->getAssetByID(_assetID).lock();

	if (!asset) {
		warning("Sound effect modifier references asset %i but the asset isn't loaded!", _assetID);
		return;
	}

	if (asset->getAssetType() != kAssetTypeAudio) {
		warning("Sound element assigned an asset that isn't audio");
		return;
	}

	_cachedAudio = static_cast<AudioAsset *>(asset.get())->loadAndCacheAudio(runtime);
	_metadata = static_cast<AudioAsset *>(asset.get())->getMetadata();
}

Common::SharedPtr<Modifier> SoundEffectModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new SoundEffectModifier(*this));
}

const char *SoundEffectModifier::getDefaultName() const {
	return "Sound Effect Modifier";
}

PathMotionModifierV2::PointDef::PointDef() : frame(0), useFrame(false) {
}

PathMotionModifierV2::PathMotionModifierV2()
	: _reverse(false), _loop(false), _alternate(false),
	  _startAtBeginning(false), _frameDurationTimes10Million(0) {
}

bool PathMotionModifierV2::load(ModifierLoaderContext &context, const Data::PathMotionModifierV2 &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_executeWhen.load(data.executeWhen) || !_terminateWhen.load(data.terminateWhen))
		return false;

	_reverse = ((data.flags & Data::PathMotionModifierV2::kFlagReverse) != 0);
	_loop = ((data.flags & Data::PathMotionModifierV2::kFlagLoop) != 0);
	_alternate = ((data.flags & Data::PathMotionModifierV2::kFlagAlternate) != 0);
	_startAtBeginning = ((data.flags & Data::PathMotionModifierV2::kFlagStartAtBeginning) != 0);

	_frameDurationTimes10Million = data.frameDurationTimes10Million;

	_points.resize(data.numPoints);

	for (size_t i = 0; i < _points.size(); i++) {
		const Data::PathMotionModifierV2::PointDef &inPoint = data.points[i];
		PointDef &outPoint = _points[i];

		outPoint.frame = inPoint.frame;
		outPoint.useFrame = ((inPoint.frameFlags & Data::PathMotionModifierV2::PointDef::kFrameFlagPlaySequentially) != 0);
		if (!inPoint.point.toScummVMPoint(outPoint.point) || !outPoint.sendSpec.load(inPoint.send, inPoint.messageFlags, inPoint.with, inPoint.withSource, inPoint.withString, inPoint.destination))
			return false;
	}

	return true;
}

bool PathMotionModifierV2::respondsToEvent(const Event &evt) const {
	return _executeWhen.respondsTo(evt) || _terminateWhen.respondsTo(evt);
}

VThreadState PathMotionModifierV2::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_executeWhen.respondsTo(msg->getEvent())) {
#ifdef MTROPOLIS_DEBUG_ENABLE
		if (Debugger *debugger = runtime->debugGetDebugger())
			debugger->notify(kDebugSeverityWarning, "Path motion modifier was supposed to execute, but this isn't implemented yet");
#endif
		_incomingData = msg->getValue();

		return kVThreadReturn;
	}
	if (_terminateWhen.respondsTo(msg->getEvent())) {
		disable(runtime);
		return kVThreadReturn;
	}

	return kVThreadReturn;
}

void PathMotionModifierV2::disable(Runtime *runtime) {
#ifdef MTROPOLIS_DEBUG_ENABLE
	if (Debugger *debugger = runtime->debugGetDebugger())
		debugger->notify(kDebugSeverityWarning, "Path motion modifier was supposed to terminate, but this isn't implemented yet");
#endif
}

Common::SharedPtr<Modifier> PathMotionModifierV2::shallowClone() const {
	Common::SharedPtr<PathMotionModifierV2> clone(new PathMotionModifierV2(*this));
	clone->_incomingData = DynamicValue();
	return clone;
}

const char *PathMotionModifierV2::getDefaultName() const {
	return "Path Motion Modifier";
}

bool DragMotionModifier::load(ModifierLoaderContext &context, const Data::DragMotionModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_dragProps.reset(new DragMotionProperties());

	// constraint margin is unchecked here because it's a margin, not a real rectangle, but it's stored as if it's a rect
	if (!_enableWhen.load(data.enableWhen) || !_disableWhen.load(data.disableWhen) || !data.constraintMargin.toScummVMRectUnchecked(_dragProps->constraintMargin))
		return false;

	bool constrainVertical = false;
	bool constrainHorizontal = false;
	if (data.haveMacPart) {
		_dragProps->constrainToParent = ((data.platform.mac.flags & Data::DragMotionModifier::MacPart::kConstrainToParent) != 0);
		constrainHorizontal = ((data.platform.mac.flags & Data::DragMotionModifier::MacPart::kConstrainHorizontal) != 0);
		constrainVertical = ((data.platform.mac.flags & Data::DragMotionModifier::MacPart::kConstrainVertical) != 0);
	} else if (data.haveWinPart) {
		_dragProps->constrainToParent = (data.platform.win.constrainToParent != 0);
		constrainVertical = (data.platform.win.constrainVertical != 0);
		constrainHorizontal = (data.platform.win.constrainHorizontal != 0);
	} else {
		return false;
	}

	if (constrainVertical) {
		if (constrainHorizontal)
			return false;	// ???
		else
			_dragProps->constraintDirection = kConstraintDirectionVertical;
	} else {
		if (constrainHorizontal)
			_dragProps->constraintDirection = kConstraintDirectionHorizontal;
		else
			_dragProps->constraintDirection = kConstraintDirectionNone;
	}

	return true;
}

Common::SharedPtr<Modifier> DragMotionModifier::shallowClone() const {
	Common::SharedPtr<DragMotionModifier> clone = Common::SharedPtr<DragMotionModifier>(new DragMotionModifier(*this));
	clone->_dragProps.reset(new DragMotionProperties(*_dragProps));
	return clone;
}

const char *DragMotionModifier::getDefaultName() const {
	return "Drag Motion Modifier";
}

bool DragMotionModifier::respondsToEvent(const Event &evt) const {
	return _enableWhen.respondsTo(evt) || _disableWhen.respondsTo(evt);
}

VThreadState DragMotionModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_enableWhen.respondsTo(msg->getEvent())) {
		Structural *owner = this->findStructuralOwner();
		if (owner->isElement() && static_cast<Element *>(owner)->isVisual())
			static_cast<VisualElement *>(owner)->setDragMotionProperties(_dragProps);
		return kVThreadReturn;
	}
	if (_disableWhen.respondsTo(msg->getEvent())) {
		disable(runtime);
		return kVThreadReturn;
	}

	return kVThreadReturn;
}

void DragMotionModifier::disable(Runtime *runtime) {
	Structural *owner = this->findStructuralOwner();
	if (owner->isElement() && static_cast<Element *>(owner)->isVisual())
		static_cast<VisualElement *>(owner)->setDragMotionProperties(nullptr);
}

VectorMotionModifier::~VectorMotionModifier() {
	if (_scheduledEvent) {
		_scheduledEvent->cancel();
		_scheduledEvent.reset();
	}
}

bool VectorMotionModifier::load(ModifierLoaderContext &context, const Data::VectorMotionModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_enableWhen.load(data.enableWhen) || !_disableWhen.load(data.disableWhen) || !_vec.load(data.vec, data.vecSource, data.vecString))
		return false;

	return true;
}

bool VectorMotionModifier::respondsToEvent(const Event &evt) const {
	return _enableWhen.respondsTo(evt) || _disableWhen.respondsTo(evt);
}

VThreadState VectorMotionModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_enableWhen.respondsTo(msg->getEvent())) {
		DynamicValue vec;
		if (_vec.getType() == DynamicValueTypes::kIncomingData) {
			vec = msg->getValue();
		} else if (!_vecVar.expired()) {
			Modifier *modifier = _vecVar.lock().get();

			if (!modifier->isVariable()) {
#ifdef MTROPOLIS_DEBUG_ENABLE
				if (Debugger *debugger = runtime->debugGetDebugger())
					debugger->notify(kDebugSeverityError, "Vector variable reference was to a non-variable");
#endif
				return kVThreadError;
			}

			VariableModifier *varModifier = static_cast<VariableModifier *>(modifier);
			varModifier->varGetValue(nullptr, vec);
		} else {
#ifdef MTROPOLIS_DEBUG_ENABLE
			if (Debugger *debugger = runtime->debugGetDebugger())
				debugger->notify(kDebugSeverityError, "Vector variable reference wasn't resolved");
#endif
			return kVThreadError;
		}

		if (vec.getType() != DynamicValueTypes::kVector) {
#ifdef MTROPOLIS_DEBUG_ENABLE
			if (Debugger *debugger = runtime->debugGetDebugger())
				debugger->notify(kDebugSeverityError, "Vector value was not actually a vector");
#endif
			return kVThreadError;
		}

		_resolvedVector = vec.getVector();

		if (!_scheduledEvent) {
			_lastTickTime = runtime->getPlayTime();
			_subpixelX = 0;
			_subpixelY = 0;
			_scheduledEvent = runtime->getScheduler().scheduleMethod<VectorMotionModifier, &VectorMotionModifier::trigger>(_lastTickTime + 1, this);
			return kVThreadReturn;
		}
	}
	if (_disableWhen.respondsTo(msg->getEvent())) {
		disable(runtime);
		return kVThreadReturn;
	}

	return kVThreadReturn;
}

void VectorMotionModifier::disable(Runtime *runtime) {
	if (_scheduledEvent) {
		_scheduledEvent->cancel();
		_scheduledEvent.reset();
	}
}

void VectorMotionModifier::trigger(Runtime *runtime) {
	uint64 currentTime = runtime->getPlayTime();
	_scheduledEvent = runtime->getScheduler().scheduleMethod<VectorMotionModifier, &VectorMotionModifier::trigger>(currentTime + 1, this);

	Modifier *vecSrcModifier = _vecVar.lock().get();

	// Variable-sourced motion is continuously updated and doesn't need to be re-triggered.
	// The Pong minigame in Obsidian's Bureau chapter depends on this.
	if (vecSrcModifier && vecSrcModifier->isVariable()) {
		DynamicValue vec;
		VariableModifier *varModifier = static_cast<VariableModifier *>(vecSrcModifier);
		varModifier->varGetValue(nullptr, vec);

		if (vec.getType() == DynamicValueTypes::kVector)
			_resolvedVector = vec.getVector();
	}

	double radians = _resolvedVector.angleDegrees * (M_PI / 180.0);

	// Distance is per-tick, which is 1/60 of a sec, so the multiplier is 60.0/1000.0 or 0.06
	// We then scale the entire thing up by 2^64, so the result is 60*65536/1000
	double distance = static_cast<double>(currentTime - _lastTickTime) * _resolvedVector.magnitude * 3932.16;

	int32 dx = static_cast<int32>(cos(radians) * distance) + static_cast<int32>(_subpixelX);
	int32 dy = static_cast<int32>(-sin(radians) * distance) + static_cast<int32>(_subpixelY);
	_subpixelX = (dx & 0xffff);
	_subpixelY = (dy & 0xffff);

	dx -= _subpixelX;
	dy -= _subpixelY;

	dx /= 65536;
	dy /= 65536;

	Structural *structural = findStructuralOwner();
	if (structural->isElement()) {
		Element *element = static_cast<Element *>(structural);
		if (element->isVisual()) {
			VisualElement *visual = static_cast<VisualElement *>(element);

			VisualElement::OffsetTranslateTaskData *taskData = runtime->getVThread().pushTask("VisualElement::offsetTranslateTask", visual, &VisualElement::offsetTranslateTask);
			taskData->dx = dx;
			taskData->dy = dy;
		}
	}

	_lastTickTime = currentTime;
}

Common::SharedPtr<Modifier> VectorMotionModifier::shallowClone() const {
	Common::SharedPtr<VectorMotionModifier> clone(new VectorMotionModifier(*this));
	clone->_scheduledEvent.reset();
	return clone;
}

const char *VectorMotionModifier::getDefaultName() const {
	return "Vector Motion Modifier";
}

void VectorMotionModifier::linkInternalReferences(ObjectLinkingScope *scope) {
	if (_vec.getType() == DynamicValueTypes::kVariableReference) {
		const VarReference &varRef = _vec.getVarReference();
		Common::WeakPtr<RuntimeObject> objRef = scope->resolve(varRef.guid, varRef.source, false);

		RuntimeObject *obj = objRef.lock().get();
		if (obj == nullptr || !obj->isModifier()) {
			warning("Vector motion modifier source was set to a variable, but the variable reference was invalid");
		} else {
			_vecVar = objRef.staticCast<Modifier>();
		}
	}
}

void VectorMotionModifier::visitInternalReferences(IStructuralReferenceVisitor* visitor) {
	visitor->visitWeakModifierRef(_vecVar);
}

bool SceneTransitionModifier::load(ModifierLoaderContext &context, const Data::SceneTransitionModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_enableWhen.load(data.enableWhen) || !_disableWhen.load(data.disableWhen))
		return false;

	_duration = data.duration;
	_steps = data.steps;
	if (!SceneTransitionTypes::loadFromData(_transitionType, data.transitionType))
		return false;
	if (!SceneTransitionDirections::loadFromData(_transitionDirection, data.direction))
		return false;

	return true;
}

bool SceneTransitionModifier::respondsToEvent(const Event &evt) const {
	return _enableWhen.respondsTo(evt) || _disableWhen.respondsTo(evt);
}

VThreadState SceneTransitionModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_enableWhen.respondsTo(msg->getEvent())) {
		SceneTransitionEffect effect;

		// For some reason, these vary
		uint32 timeDivisor = 100;
		switch (effect._transitionType) {
		case SceneTransitionTypes::kRandomDissolve:
			timeDivisor = 50;
			break;
		case SceneTransitionTypes::kFade:
			timeDivisor = 25;
			break;
		default:
			break;
		}

		effect._duration = _duration / timeDivisor;
		effect._steps = _steps;
		effect._transitionDirection = _transitionDirection;
		effect._transitionType = _transitionType;
		runtime->setSceneTransitionEffect(true, &effect);
	}
	if (_disableWhen.respondsTo(msg->getEvent()))
		disable(runtime);

	return kVThreadReturn;
}

void SceneTransitionModifier::disable(Runtime *runtime) {
	runtime->setSceneTransitionEffect(true, nullptr);
}

Common::SharedPtr<Modifier> SceneTransitionModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new SceneTransitionModifier(*this));
}

const char *SceneTransitionModifier::getDefaultName() const {
	return "Scene Transition Modifier";
}

ElementTransitionModifier::ElementTransitionModifier() : _rate(0), _steps(0),
	_transitionType(kTransitionTypeFade), _revealType(kRevealTypeReveal), _transitionStartTime(0), _currentStep(0) {
}

ElementTransitionModifier::~ElementTransitionModifier() {
	if (_scheduledEvent) {
		_scheduledEvent->cancel();
		_scheduledEvent.reset();
	}
}

bool ElementTransitionModifier::load(ModifierLoaderContext &context, const Data::ElementTransitionModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_enableWhen.load(data.enableWhen) || !_disableWhen.load(data.disableWhen))
		return false;

	_rate = data.rate;
	_steps = data.steps;

	switch (data.transitionType) {
	case Data::ElementTransitionModifier::kTransitionTypeFade:
		_transitionType = kTransitionTypeFade;
		break;
	case Data::ElementTransitionModifier::kTransitionTypeOvalIris:
		_transitionType = kTransitionTypeOvalIris;
		break;
	case Data::ElementTransitionModifier::kTransitionTypeRectangularIris:
		_transitionType = kTransitionTypeRectangularIris;
		break;
	case Data::ElementTransitionModifier::kTransitionTypeZoom:
		_transitionType = kTransitionTypeZoom;
		break;
	default:
		return false;
	}

	switch (data.revealType) {
	case Data::ElementTransitionModifier::kRevealTypeConceal:
		_revealType = kRevealTypeConceal;
		break;
	case Data::ElementTransitionModifier::kRevealTypeReveal:
		_revealType = kRevealTypeReveal;
		break;
	default:
		return false;
	}

	return true;
}

bool ElementTransitionModifier::respondsToEvent(const Event &evt) const {
	return _enableWhen.respondsTo(evt) || _disableWhen.respondsTo(evt);
}

VThreadState ElementTransitionModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	// How element transition modifiers work:
	// - When activated, if Reveal, then Show is sent (regardless of whether element is visible or not).
	// - Then, for both reveal and conceal, Transition Started is sent by the modifier.
	// - When a conceal transition completes, Hide is sent to the element.
	// - Then, for both reveal and conceal, Transition Ended is sent by the modifier.
	// - If a transition is active and the Disable signal is called, then the transition completes immediately.
	//
	// Q. Does that mean if an element has a Revealer followed by a Concealer and they take opposing messages,
	//    that the element will be hidden if a reveal interrupts the concealer due to its hide message happening
	//    second, but that won't happen if they're in Concealer-Revealer order?
	// A. Yes.
	if (_enableWhen.respondsTo(msg->getEvent())) {
		if (_scheduledEvent) {
			_scheduledEvent->cancel();
			_scheduledEvent.reset();
		}

		_scheduledEvent = runtime->getScheduler().scheduleMethod<ElementTransitionModifier, &ElementTransitionModifier::continueTransition>(runtime->getPlayTime() + 1, this);
		_transitionStartTime = runtime->getPlayTime();
		_currentStep = 0;
		setTransitionProgress(0, _steps);

		// Pushed tasks, so these are executed in reverse order (Show -> Transition Started)
		{
			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kTransitionStarted, 0), DynamicValue(), getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, findStructuralOwner(), false, true, false));
			runtime->sendMessageOnVThread(dispatch);
		}

		if (_revealType == kRevealTypeReveal)
		{
			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kElementShow, 0), DynamicValue(), getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, findStructuralOwner(), false, false, true));
			runtime->sendMessageOnVThread(dispatch);
		}

		return kVThreadReturn;
	}

	if (_disableWhen.respondsTo(msg->getEvent())) {
		disable(runtime);
		return kVThreadReturn;
	}

	return Modifier::consumeMessage(runtime, msg);
}

void ElementTransitionModifier::disable(Runtime *runtime) {
	if (_scheduledEvent) {
		_scheduledEvent->cancel();

		completeTransition(runtime);
	}
}

Common::SharedPtr<Modifier> ElementTransitionModifier::shallowClone() const {
	Common::SharedPtr<ElementTransitionModifier> clone(new ElementTransitionModifier(*this));
	clone->_scheduledEvent.reset();

	return clone;
}

const char *ElementTransitionModifier::getDefaultName() const {
	return "Element Transition Modifier";
}

void ElementTransitionModifier::continueTransition(Runtime *runtime) {
	_scheduledEvent.reset();

	const uint64 playTime = runtime->getPlayTime();
	const uint64 timeSinceStart = playTime - _transitionStartTime;

	uint32 step = static_cast<uint32>(timeSinceStart * _rate / 1000);

	if (step >= _steps || _rate == 0) {
		completeTransition(runtime);
		return;
	}

	if (step != _currentStep) {
		setTransitionProgress(step, _steps);
		_currentStep = step;
	}

	runtime->setSceneGraphDirty();
	_scheduledEvent = runtime->getScheduler().scheduleMethod<ElementTransitionModifier, &ElementTransitionModifier::continueTransition>(playTime + 1, this);
}

void ElementTransitionModifier::completeTransition(Runtime *runtime) {
	// Pushed tasks, so these are executed in reverse order (Hide -> Transition Ended)
	{
		Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kTransitionEnded, 0), DynamicValue(), getSelfReference()));
		Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, findStructuralOwner(), false, true, false));
		runtime->sendMessageOnVThread(dispatch);
	}

	if (_revealType == kRevealTypeConceal) {
		Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event(EventIDs::kElementHide, 0), DynamicValue(), getSelfReference()));
		Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, findStructuralOwner(), false, false, true));
		runtime->sendMessageOnVThread(dispatch);
	}

	setTransitionProgress(( _revealType == kRevealTypeReveal) ? 1 : 0, 1);
	runtime->setSceneGraphDirty();
}

void ElementTransitionModifier::setTransitionProgress(uint32 step, uint32 maxSteps) {
	Structural *structural = findStructuralOwner();
	if (structural && structural->isElement() && static_cast<Element *>(structural)->isVisual()) {
		VisualElement *visual = static_cast<VisualElement *>(structural);
		VisualElementTransitionProperties props = visual->getTransitionProperties();

		if (_transitionType == kTransitionTypeFade) {
			if (step > maxSteps)
				step = maxSteps;

			uint32 alpha = step * 255 / maxSteps;
			if (_revealType == kRevealTypeConceal)
				alpha = 255 - alpha;

			props.setAlpha(alpha);
			visual->setTransitionProperties(props);
		} else {
			warning("Unsupported transition type");
		}
	}
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

bool IfMessengerModifier::respondsToEvent(const Event &evt) const {
	return _when.respondsTo(evt);
}

VThreadState IfMessengerModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_when.respondsTo(msg->getEvent())) {
		Common::SharedPtr<MiniscriptThread> thread(new MiniscriptThread(runtime, msg, _program, _references, this));

		EvaluateAndSendTaskData *evalAndSendData = runtime->getVThread().pushTask("IfMessengerModifier::evaluateAndSendTask", this, &IfMessengerModifier::evaluateAndSendTask);
		evalAndSendData->thread = thread;
		evalAndSendData->runtime = runtime;
		evalAndSendData->incomingData = msg->getValue();

		MiniscriptThread::runOnVThread(runtime->getVThread(), thread);
	}

	return kVThreadReturn;
}

Common::SharedPtr<Modifier> IfMessengerModifier::shallowClone() const {
	IfMessengerModifier *clonePtr = new IfMessengerModifier(*this);
	Common::SharedPtr<Modifier> clone(clonePtr);

	// Keep the Miniscript program (which is static), but clone the references
	clonePtr->_references.reset(new MiniscriptReferences(*_references));

	return clone;
}

const char *IfMessengerModifier::getDefaultName() const {
	return "If Messenger";
}

void IfMessengerModifier::linkInternalReferences(ObjectLinkingScope *scope) {
	_sendSpec.linkInternalReferences(scope);
	_references->linkInternalReferences(scope);
}

void IfMessengerModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	_sendSpec.visitInternalReferences(visitor);
	_references->visitInternalReferences(visitor);
}


VThreadState IfMessengerModifier::evaluateAndSendTask(const EvaluateAndSendTaskData &taskData) {
	MiniscriptThread *thread = taskData.thread.get();

	bool isTrue = false;
	if (!thread->evaluateTruthOfResult(isTrue))
		return kVThreadError;

	if (isTrue)
		_sendSpec.sendFromMessenger(taskData.runtime, this, taskData.incomingData, nullptr);

	return kVThreadReturn;
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
		disable(runtime);
		return kVThreadReturn;
	}
	if (_executeWhen.respondsTo(msg->getEvent())) {
		// 0-time events are not allowed
		uint32 realMilliseconds = _milliseconds;
		if (realMilliseconds == 0)
			realMilliseconds = 1;

		debug(3, "Timer %x '%s' scheduled to execute in %i milliseconds", getStaticGUID(), getName().c_str(), realMilliseconds);

		if (_scheduledEvent) {
			_scheduledEvent->cancel();
			_scheduledEvent.reset();
		}

		_scheduledEvent = runtime->getScheduler().scheduleMethod<TimerMessengerModifier, &TimerMessengerModifier::trigger>(runtime->getPlayTime() + realMilliseconds, this);
		_incomingData = msg->getValue();
		if (_incomingData.getType() == DynamicValueTypes::kList)
			_incomingData.setList(_incomingData.getList()->clone());

		return kVThreadReturn;
	}

	return kVThreadReturn;
}

void TimerMessengerModifier::disable(Runtime *runtime) {
	if (_scheduledEvent) {
		_scheduledEvent->cancel();
		_scheduledEvent.reset();
	}
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

const char *TimerMessengerModifier::getDefaultName() const {
	return "Timer Messenger";
}

void TimerMessengerModifier::trigger(Runtime *runtime) {
	debug(3, "Timer %x '%s' triggered", getStaticGUID(), getName().c_str());
	if (_looping) {
		uint32 realMilliseconds = _milliseconds;
		if (realMilliseconds == 0)
			realMilliseconds = 1;
		_scheduledEvent = runtime->getScheduler().scheduleMethod<TimerMessengerModifier, &TimerMessengerModifier::trigger>(runtime->getPlayTime() + realMilliseconds, this);
	} else
		_scheduledEvent.reset();

	_sendSpec.sendFromMessenger(runtime, this, _incomingData, nullptr);
}

BoundaryDetectionMessengerModifier::BoundaryDetectionMessengerModifier()
	: _exitTriggerMode(kExitTriggerExiting), _detectTopEdge(false), _detectBottomEdge(false),
	  _detectLeftEdge(false), _detectRightEdge(false), _detectionMode(kContinuous),
	  _runtime(nullptr), _isActive(false) {
}

BoundaryDetectionMessengerModifier::~BoundaryDetectionMessengerModifier() {
	if (_isActive)
		_runtime->removeBoundaryDetector(this);
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

bool BoundaryDetectionMessengerModifier::respondsToEvent(const Event &evt) const {
	return _enableWhen.respondsTo(evt) || _disableWhen.respondsTo(evt);
}

VThreadState BoundaryDetectionMessengerModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_enableWhen.respondsTo(msg->getEvent()) && !_isActive) {
		_runtime = runtime;
		_runtime->addBoundaryDetector(this);
		_isActive = true;

		_incomingData = msg->getValue();
		if (_incomingData.getType() == DynamicValueTypes::kList)
			_incomingData.setList(_incomingData.getList()->clone());
	}
	if (_disableWhen.respondsTo(msg->getEvent())) {
		disable(runtime);
	}

	return kVThreadReturn;
}

void BoundaryDetectionMessengerModifier::disable(Runtime *runtime) {
	if (_isActive) {
		_runtime->removeBoundaryDetector(this);
		_isActive = false;
		_runtime = nullptr;
	}
}

void BoundaryDetectionMessengerModifier::linkInternalReferences(ObjectLinkingScope *outerScope) {
	_send.linkInternalReferences(outerScope);
}

void BoundaryDetectionMessengerModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	_send.visitInternalReferences(visitor);
}

void BoundaryDetectionMessengerModifier::getCollisionProperties(Modifier *&modifier, uint &edgeFlags, bool &mustBeCompletelyOutside, bool &continuous) const {
	modifier = const_cast<BoundaryDetectionMessengerModifier *>(this);

	uint flags = 0;
	if (_detectBottomEdge)
		flags |= kEdgeBottom;
	if (_detectTopEdge)
		flags |= kEdgeTop;
	if (_detectRightEdge)
		flags |= kEdgeRight;
	if (_detectLeftEdge)
		flags |= kEdgeLeft;

	edgeFlags = flags;
	mustBeCompletelyOutside = (_exitTriggerMode == kExitTriggerOnceExited);
	continuous = (_detectionMode == kContinuous);
}

void BoundaryDetectionMessengerModifier::triggerCollision(Runtime *runtime) {
	_send.sendFromMessenger(runtime, this, _incomingData, nullptr);
}

Common::SharedPtr<Modifier> BoundaryDetectionMessengerModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new BoundaryDetectionMessengerModifier(*this));
}

const char *BoundaryDetectionMessengerModifier::getDefaultName() const {
	return "Boundary Detection Messenger";
}

CollisionDetectionMessengerModifier::CollisionDetectionMessengerModifier()
	: _detectionMode(kDetectionModeFirstContact), _detectInFront(true), _detectBehind(true),
	  _ignoreParent(true), _sendToCollidingElement(false), _sendToOnlyFirstCollidingElement(false),
	  _runtime(nullptr), _isActive(false) {
}

CollisionDetectionMessengerModifier::~CollisionDetectionMessengerModifier() {
	if (_isActive)
		_runtime->removeCollider(this);
}

bool CollisionDetectionMessengerModifier::load(ModifierLoaderContext &context, const Data::CollisionDetectionMessengerModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_enableWhen.load(data.enableWhen) || !_disableWhen.load(data.disableWhen))
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

bool CollisionDetectionMessengerModifier::respondsToEvent(const Event &evt) const {
	return _enableWhen.respondsTo(evt) || _disableWhen.respondsTo(evt);
}

VThreadState CollisionDetectionMessengerModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_enableWhen.respondsTo(msg->getEvent())) {
		if (!_isActive) {
			_isActive = true;
			_runtime = runtime;
			_incomingData = msg->getValue();
			if (_incomingData.getType() == DynamicValueTypes::kList)
				_incomingData.setList(_incomingData.getList()->clone());

			runtime->addCollider(this);
		}
	}
	if (_disableWhen.respondsTo(msg->getEvent())) {
		disable(runtime);
	}

	return kVThreadReturn;
}

void CollisionDetectionMessengerModifier::disable(Runtime *runtime) {
	if (_isActive) {
		_isActive = false;
		_runtime->removeCollider(this);
		_incomingData = DynamicValue();
	}
}

void CollisionDetectionMessengerModifier::linkInternalReferences(ObjectLinkingScope *scope) {
	_sendSpec.linkInternalReferences(scope);
}

void CollisionDetectionMessengerModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	_sendSpec.visitInternalReferences(visitor);
}

Common::SharedPtr<Modifier> CollisionDetectionMessengerModifier::shallowClone() const {
	Common::SharedPtr<CollisionDetectionMessengerModifier> clone(new CollisionDetectionMessengerModifier(*this));
	clone->_isActive = false;
	clone->_incomingData = DynamicValue();
	return clone;
}

const char *CollisionDetectionMessengerModifier::getDefaultName() const {
	return "Collision Messenger";
}

void CollisionDetectionMessengerModifier::getCollisionProperties(Modifier *&modifier, bool &collideInFront, bool &collideBehind, bool &excludeParents) const {
	collideBehind = _detectBehind;
	collideInFront = _detectInFront;
	excludeParents = _ignoreParent;
	modifier = const_cast<CollisionDetectionMessengerModifier *>(this);
}

void CollisionDetectionMessengerModifier::triggerCollision(Runtime *runtime, Structural *collidingElement, bool wasInContact, bool isInContact, bool &shouldStop) {
	switch (_detectionMode) {
	case kDetectionModeExiting:
		if (isInContact || !wasInContact)
			return;
		break;
	case kDetectionModeFirstContact:
		if (!isInContact || wasInContact)
			return;
		break;
	case kDetectionModeWhileInContact:
		if (!isInContact)
			return;
		break;
	default:
		error("Unknown collision detection mode");
		return;
	}

	RuntimeObject *customDestination = nullptr;
	if (_sendToCollidingElement) {
		if (_sendToOnlyFirstCollidingElement)
			shouldStop = true;

		customDestination = collidingElement;
	}

	_sendSpec.sendFromMessenger(runtime, this, _incomingData, customDestination);
}

KeyboardMessengerModifier::~KeyboardMessengerModifier() {
}

KeyboardMessengerModifier::KeyboardMessengerModifier()
	: _onDown(false), _onUp(false), _onRepeat(false), _keyModControl(false), _keyModCommand(false), _keyModOption(false),
	  _isEnabled(false), _keyCodeType(kAny), _macRomanChar(0) {
}

bool KeyboardMessengerModifier::isKeyboardMessenger() const {
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
		memcpy(&_macRomanChar, &data.keycode, 1);
		break;
	}

	if (!_sendSpec.load(data.message, data.messageFlagsAndKeyStates, data.with, data.withSource, data.withString, data.destination))
		return false;

	return true;
}

bool KeyboardMessengerModifier::respondsToEvent(const Event &evt) const {
	if (Event(EventIDs::kParentEnabled, 0).respondsTo(evt) || Event(EventIDs::kParentDisabled, 0).respondsTo(evt))
		return true;

	return false;
}

VThreadState KeyboardMessengerModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
 	if (Event(EventIDs::kParentEnabled, 0).respondsTo(msg->getEvent())) {
		_isEnabled = true;
	} else if (Event(EventIDs::kParentDisabled, 0).respondsTo(msg->getEvent())) {
		disable(runtime);
	}

	return kVThreadReturn;
}

void KeyboardMessengerModifier::disable(Runtime *runtime) {
	_isEnabled = false;
}

Common::SharedPtr<Modifier> KeyboardMessengerModifier::shallowClone() const {
	Common::SharedPtr<KeyboardMessengerModifier> cloned(new KeyboardMessengerModifier(*this));
	cloned->_isEnabled = false;
	return cloned;
}

const char *KeyboardMessengerModifier::getDefaultName() const {
	return "Keyboard Messenger";
}

bool KeyboardMessengerModifier::checkKeyEventTrigger(Runtime *runtime, Common::EventType evtType, bool repeat, const Common::KeyState &keyEvt, Common::String &outCharStr) const {
	if (!_isEnabled)
		return false;

	bool responds = false;
	if (evtType == Common::EVENT_KEYDOWN) {
		if (repeat)
			responds = _onRepeat;
		else
			responds = _onDown;
	} else if (evtType == Common::EVENT_KEYUP)
		responds = _onUp;

	if (!responds)
		return false;

	if (_keyModCommand) {
		if (runtime->getPlatform() == kProjectPlatformWindows) {
			// Windows projects check "alt"
			if ((keyEvt.flags & Common::KBD_ALT) == 0)
				return false;
		} else if (runtime->getPlatform() == kProjectPlatformMacintosh) {
			if ((keyEvt.flags & Common::KBD_META) == 0)
				return false;
		}
	}

	if (_keyModControl) {
		if ((keyEvt.flags & Common::KBD_CTRL) == 0)
			return false;
	}

	if (_keyModOption) {
		if ((keyEvt.flags & Common::KBD_ALT) == 0)
			return false;
	}

	outCharStr.clear();

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
	default:
		if (keyEvt.ascii != 0) {
			bool isQuestion = (keyEvt.ascii == '?');
			uint32 uchar = keyEvt.ascii;
			Common::U32String u(&uchar, 1);
			outCharStr = u.encode(Common::kMacRoman);

			// STUPID HACK PLEASE FIX ME: ScummVM has no way of just telling us that the character mapping failed,
			// so we have to check if it encoded "?"
			if (outCharStr.size() < 1 || (outCharStr[0] == '?' && !isQuestion))
				return false;

			resolvedType = kMacRomanChar;
		}
		break;
	}

	if (_keyCodeType != kAny && resolvedType != _keyCodeType)
		return false;

	if (_keyCodeType == kMacRomanChar && (outCharStr.size() == 0 || outCharStr[0] != _macRomanChar))
		return false;

	return true;
}

void KeyboardMessengerModifier::dispatchMessage(Runtime *runtime, const Common::String &charStr) {
	Common::SharedPtr<MessageProperties> msgProps;

	if (charStr.size() != 1)
		warning("Keyboard messenger is supposed to send the character code, but they key was a special key and we haven't implemented conversion of those keycodes");

	DynamicValue charStrValue;
	charStrValue.setString(charStr);
	_sendSpec.sendFromMessenger(runtime, this, charStrValue, nullptr);
}

void KeyboardMessengerModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	_sendSpec.visitInternalReferences(visitor);
}

void KeyboardMessengerModifier::linkInternalReferences(ObjectLinkingScope *scope) {
	_sendSpec.linkInternalReferences(scope);
}

bool TextStyleModifier::load(ModifierLoaderContext &context, const Data::TextStyleModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_textColor.load(data.textColor) || !_backgroundColor.load(data.backgroundColor) || !_applyWhen.load(data.applyWhen) || !_removeWhen.load(data.removeWhen))
		return false;

	_macFontID = data.macFontID;
	_size = data.size;
	_fontFamilyName = data.fontFamilyName;

	if (!_styleFlags.load(data.flags))
		return false;

	switch (data.alignment) {
	case 0:
		_alignment = kTextAlignmentLeft;
		break;
	case 1:
		_alignment = kTextAlignmentCenter;
		break;
	case 0xffff:
		_alignment = kTextAlignmentRight;
		break;
	default:
		warning("Unrecognized text alignment");
		return false;
	}

	return true;
}

bool TextStyleModifier::respondsToEvent(const Event &evt) const {
	if (_applyWhen.respondsTo(evt) || _removeWhen.respondsTo(evt))
		return true;

	return Modifier::respondsToEvent(evt);
}

VThreadState TextStyleModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_applyWhen.respondsTo(msg->getEvent())) {
		Structural *owner = findStructuralOwner();
		if (owner && owner->isElement()) {
			Element *element = static_cast<Element *>(owner);
			if (element->isVisual()) {
				VisualElement *visualElement = static_cast<VisualElement *>(element);
				if (visualElement->isTextLabel()) {
					static_cast<TextLabelElement *>(visualElement)->setTextStyle(_macFontID, _fontFamilyName, _size, _alignment, _styleFlags);
				}
			}
		}

		return kVThreadReturn;
	} else if (_removeWhen.respondsTo(msg->getEvent())) {
		disable(runtime);
		return kVThreadReturn;
	}

	return Modifier::consumeMessage(runtime, msg);
}

void TextStyleModifier::disable(Runtime *runtime) {
	// Doesn't actually do anything
}

Common::SharedPtr<Modifier> TextStyleModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new TextStyleModifier(*this));
}

const char *TextStyleModifier::getDefaultName() const {
	return "Text Style Messenger";
}

bool GraphicModifier::load(ModifierLoaderContext &context, const Data::GraphicModifier &data) {
	ColorRGB8 foreColor;
	ColorRGB8 backColor;
	ColorRGB8 borderColor;
	ColorRGB8 shadowColor;

	if (!loadTypicalHeader(data.modHeader) || !_applyWhen.load(data.applyWhen) || !_removeWhen.load(data.removeWhen)
		|| !foreColor.load(data.foreColor) || !backColor.load(data.backColor)
		|| !borderColor.load(data.borderColor) || !shadowColor.load(data.shadowColor))
		return false;

	// We need the poly points even if this isn't a poly shape since I think it's possible to change the shape type at runtime
	Common::Array<Common::Point> &polyPoints = _renderProps.modifyPolyPoints();
	polyPoints.resize(data.polyPoints.size());
	for (size_t i = 0; i < data.polyPoints.size(); i++) {
		polyPoints[i].x = data.polyPoints[i].x;
		polyPoints[i].y = data.polyPoints[i].y;
	}

	_renderProps.setInkMode(static_cast<VisualElementRenderProperties::InkMode>(data.inkMode));
	_renderProps.setShape(static_cast<VisualElementRenderProperties::Shape>(data.shape));
	_renderProps.setBorderSize(data.borderSize);
	_renderProps.setShadowSize(data.shadowSize);
	_renderProps.setForeColor(foreColor);
	_renderProps.setBackColor(backColor);
	_renderProps.setBorderColor(borderColor);
	_renderProps.setShadowColor(shadowColor);

	return true;
}

bool GraphicModifier::respondsToEvent(const Event &evt) const {
	return _applyWhen.respondsTo(evt) || _removeWhen.respondsTo(evt);
}

VThreadState GraphicModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	Structural *owner = findStructuralOwner();
	if (!owner)
		return kVThreadError;

	if (!owner->isElement())
		return kVThreadReturn;

	Element *element = static_cast<Element *>(owner);
	if (!element->isVisual())
		return kVThreadReturn;

	VisualElement *visual = static_cast<VisualElement *>(element);

	// If a graphic modifier is the active graphic modifier, then it may be removed, but removing it resets to default, not to
	// any other graphic modifier.  If it is not the active graphic modifier, then removing it has no effect.
	// This is required for correct rendering of the beaker when freeing Max in Obsidian.
	if (_applyWhen.respondsTo(msg->getEvent())) {
		visual->setRenderProperties(_renderProps, this->getSelfReference().staticCast<GraphicModifier>());
	}
	if (_removeWhen.respondsTo(msg->getEvent())) {
		disable(runtime);
	}

	return kVThreadReturn;
}

void GraphicModifier::disable(Runtime *runtime) {
	Structural *owner = findStructuralOwner();
	if (!owner)
		return;

	if (!owner->isElement())
		return;

	Element *element = static_cast<Element *>(owner);
	if (!element->isVisual())
		return;

	VisualElement *visual = static_cast<VisualElement *>(element);

	if (visual->getPrimaryGraphicModifier().lock().get() == this)
		static_cast<VisualElement *>(element)->setRenderProperties(VisualElementRenderProperties(), Common::WeakPtr<GraphicModifier>());
}

Common::SharedPtr<Modifier> GraphicModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new GraphicModifier(*this));
}

const char *GraphicModifier::getDefaultName() const {
	return "Graphic Modifier";
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

void CompoundVariableModifier::disable(Runtime *runtime) {
	// Do nothing I guess, no variables can be disdabled
}

Common::SharedPtr<ModifierSaveLoad> CompoundVariableModifier::getSaveLoad() {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
}

IModifierContainer *CompoundVariableModifier::getChildContainer() {
	return this;
}

const Common::Array<Common::SharedPtr<Modifier> > &CompoundVariableModifier::getModifiers() const {
	return _children;
}

void CompoundVariableModifier::appendModifier(const Common::SharedPtr<Modifier> &modifier) {
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
		// Shouldn't dereference the value here, some scripts (e.g. "<go dest> on MUI" in Obsidian) depend on it not being dereferenced
		result.setObject(var->getSelfReference());
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

CompoundVariableModifier::SaveLoad::ChildSaveLoad::ChildSaveLoad() : modifier(nullptr) {
}

CompoundVariableModifier::SaveLoad::SaveLoad(CompoundVariableModifier *modifier) /* : _modifier(modifier) */ {
	for (const Common::SharedPtr<Modifier> &child : modifier->_children) {
		Common::SharedPtr<ModifierSaveLoad> childSL = child->getSaveLoad();
		if (childSL) {
			ChildSaveLoad childSaveLoad;
			childSaveLoad.saveLoad = childSL;
			childSaveLoad.modifier = child.get();
			_childrenSaveLoad.push_back(childSaveLoad);
		}
	}
}

void CompoundVariableModifier::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	stream->writeUint32BE(_childrenSaveLoad.size());
	for (const ChildSaveLoad &childSL : _childrenSaveLoad)
		childSL.saveLoad->save(childSL.modifier, stream);
}

bool CompoundVariableModifier::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
	const uint32 numChildren = stream->readUint32BE();
	if (stream->err())
		return false;

	if (numChildren != _childrenSaveLoad.size())
		return false;

	for (const ChildSaveLoad &childSL : _childrenSaveLoad) {
		if (!childSL.saveLoad->load(childSL.modifier, stream, saveFileVersion))
			return false;
	}

	return true;
}

void CompoundVariableModifier::SaveLoad::commitLoad() const {
	for (const ChildSaveLoad &childSL : _childrenSaveLoad)
		childSL.saveLoad->commitLoad();
}

Common::SharedPtr<Modifier> CompoundVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new CompoundVariableModifier(*this));
}

const char *CompoundVariableModifier::getDefaultName() const {
	return "Compound Variable";
}

bool BooleanVariableModifier::load(ModifierLoaderContext &context, const Data::BooleanVariableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_value = (data.value != 0);

	return true;
}

Common::SharedPtr<ModifierSaveLoad> BooleanVariableModifier::getSaveLoad() {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
}

bool BooleanVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	switch (value.getType()) {
	case DynamicValueTypes::kBoolean:
		_value = value.getBool();
		break;
	case DynamicValueTypes::kFloat:
		_value = (value.getFloat() != 0.0);
		break;
	case DynamicValueTypes::kInteger:
		_value = (value.getInt() != 0);
		break;
	default:
		return false;
	}

	return true;
}

void BooleanVariableModifier::varGetValue(MiniscriptThread *thread, DynamicValue &dest) const {
	dest.setBool(_value);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void BooleanVariableModifier::debugInspect(IDebugInspectionReport *report) const {
	VariableModifier::debugInspect(report);

	report->declareDynamic("value", _value ? "true" : "false");
}
#endif

Common::SharedPtr<Modifier> BooleanVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new BooleanVariableModifier(*this));
}

const char *BooleanVariableModifier::getDefaultName() const {
	return "Boolean Variable";
}

BooleanVariableModifier::SaveLoad::SaveLoad(BooleanVariableModifier *modifier) : _modifier(modifier) {
	_value = _modifier->_value;
}

void BooleanVariableModifier::SaveLoad::commitLoad() const {
	_modifier->_value = _value;
}

void BooleanVariableModifier::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	stream->writeByte(_value ? 1 : 0);
}

bool BooleanVariableModifier::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
	byte b = stream->readByte();
	if (stream->err())
		return false;

	_value = (b != 0);
	return true;
}

bool IntegerVariableModifier::load(ModifierLoaderContext& context, const Data::IntegerVariableModifier& data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_value = data.value;

	return true;
}

Common::SharedPtr<ModifierSaveLoad> IntegerVariableModifier::getSaveLoad() {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
}

bool IntegerVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kFloat)
		_value = static_cast<int32>(floor(value.getFloat() + 0.5));
	else if (value.getType() == DynamicValueTypes::kInteger)
		_value = value.getInt();
	else if (value.getType() == DynamicValueTypes::kString) {
		// Should this scan %lf to a double and round it instead?
		int i;
		if (!sscanf(value.getString().c_str(), "%i", &i))
			return false;
		_value = i;
	} else
		return false;

	return true;
}

void IntegerVariableModifier::varGetValue(MiniscriptThread *thread, DynamicValue &dest) const {
	dest.setInt(_value);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void IntegerVariableModifier::debugInspect(IDebugInspectionReport *report) const {
	VariableModifier::debugInspect(report);

	report->declareDynamic("value", Common::String::format("%i", _value));
}
#endif

Common::SharedPtr<Modifier> IntegerVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new IntegerVariableModifier(*this));
}

const char *IntegerVariableModifier::getDefaultName() const {
	return "Integer Variable";
}

IntegerVariableModifier::SaveLoad::SaveLoad(IntegerVariableModifier *modifier) : _modifier(modifier) {
	_value = _modifier->_value;
}

void IntegerVariableModifier::SaveLoad::commitLoad() const {
	_modifier->_value = _value;
}

void IntegerVariableModifier::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	stream->writeSint32BE(_value);
}

bool IntegerVariableModifier::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
	_value = stream->readSint32BE();

	if (stream->err())
		return false;

	return true;
}

bool IntegerRangeVariableModifier::load(ModifierLoaderContext& context, const Data::IntegerRangeVariableModifier& data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_range.load(data.range))
		return false;

	return true;
}

Common::SharedPtr<ModifierSaveLoad> IntegerRangeVariableModifier::getSaveLoad() {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
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

bool IntegerRangeVariableModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "start") {
		result.setInt(_range.min);
		return true;
	}
	if (attrib == "end") {
		result.setInt(_range.max);
		return true;
	}
	return Modifier::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome IntegerRangeVariableModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "start") {
		DynamicValueWriteIntegerHelper<int32>::create(&_range.min, result);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "end") {
		DynamicValueWriteIntegerHelper<int32>::create(&_range.max, result);
		return kMiniscriptInstructionOutcomeContinue;
	}
	return Modifier::writeRefAttribute(thread, result, attrib);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void IntegerRangeVariableModifier::debugInspect(IDebugInspectionReport *report) const {
	VariableModifier::debugInspect(report);

	report->declareDynamic("value", _range.toString());
}
#endif

Common::SharedPtr<Modifier> IntegerRangeVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new IntegerRangeVariableModifier(*this));
}

const char *IntegerRangeVariableModifier::getDefaultName() const {
	return "Integer Range Variable";
}

IntegerRangeVariableModifier::SaveLoad::SaveLoad(IntegerRangeVariableModifier *modifier) : _modifier(modifier) {
	_range = _modifier->_range;
}

void IntegerRangeVariableModifier::SaveLoad::commitLoad() const {
	_modifier->_range = _range;
}

void IntegerRangeVariableModifier::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	stream->writeSint32BE(_range.min);
	stream->writeSint32BE(_range.max);
}

bool IntegerRangeVariableModifier::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
	_range.min = stream->readSint32BE();
	_range.max = stream->readSint32BE();

	if (stream->err())
		return false;

	return true;
}

bool VectorVariableModifier::load(ModifierLoaderContext &context, const Data::VectorVariableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_vector.angleDegrees = data.vector.angleRadians.toDouble() * (180 / M_PI);
	_vector.magnitude = data.vector.magnitude.toDouble();

	return true;
}

Common::SharedPtr<ModifierSaveLoad> VectorVariableModifier::getSaveLoad() {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
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

bool VectorVariableModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "magnitude") {
		result.setFloat(_vector.magnitude);
		return true;
	} else if (attrib == "angle") {
		result.setFloat(_vector.angleDegrees);
		return true;
	}

	return VariableModifier::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome VectorVariableModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "magnitude") {
		DynamicValueWriteFloatHelper<double>::create(&_vector.magnitude, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "angle") {
		DynamicValueWriteFloatHelper<double>::create(&_vector.angleDegrees, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return writeRefAttribute(thread, result, attrib);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void VectorVariableModifier::debugInspect(IDebugInspectionReport *report) const {
	VariableModifier::debugInspect(report);

	report->declareDynamic("value", _vector.toString());
}
#endif

Common::SharedPtr<Modifier> VectorVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new VectorVariableModifier(*this));
}

const char *VectorVariableModifier::getDefaultName() const {
	return "Vector Variable";
}

VectorVariableModifier::SaveLoad::SaveLoad(VectorVariableModifier *modifier) : _modifier(modifier) {
	_vector = _modifier->_vector;
}

void VectorVariableModifier::SaveLoad::commitLoad() const {
	_modifier->_vector = _vector;
}

void VectorVariableModifier::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	stream->writeDoubleBE(_vector.angleDegrees);
	stream->writeDoubleBE(_vector.magnitude);
}

bool VectorVariableModifier::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
	_vector.angleDegrees = stream->readDoubleBE();
	_vector.magnitude = stream->readDoubleBE();

	if (stream->err())
		return false;

	return true;
}

bool PointVariableModifier::load(ModifierLoaderContext &context, const Data::PointVariableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_value.x = data.value.x;
	_value.y = data.value.y;

	return true;
}

Common::SharedPtr<ModifierSaveLoad> PointVariableModifier::getSaveLoad() {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
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

bool PointVariableModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "x") {
		result.setInt(_value.x);
		return true;
	}
	if (attrib == "y") {
		result.setInt(_value.y);
		return true;
	}

	return VariableModifier::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome PointVariableModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	if (attrib == "x") {
		DynamicValueWriteIntegerHelper<int16>::create(&_value.x, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "y") {
		DynamicValueWriteIntegerHelper<int16>::create(&_value.y, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return writeRefAttribute(thread, writeProxy, attrib);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void PointVariableModifier::debugInspect(IDebugInspectionReport *report) const {
	VariableModifier::debugInspect(report);

	report->declareDynamic("value", pointToString(_value));
}
#endif

Common::SharedPtr<Modifier> PointVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new PointVariableModifier(*this));
}

const char *PointVariableModifier::getDefaultName() const {
	return "Point Variable";
}

PointVariableModifier::SaveLoad::SaveLoad(PointVariableModifier *modifier) : _modifier(modifier) {
	_value = _modifier->_value;
}

void PointVariableModifier::SaveLoad::commitLoad() const {
	_modifier->_value = _value;
}

void PointVariableModifier::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	stream->writeSint16BE(_value.x);
	stream->writeSint16BE(_value.y);
}

bool PointVariableModifier::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
	_value.x = stream->readSint16BE();
	_value.y = stream->readSint16BE();

	if (stream->err())
		return false;

	return true;
}

bool FloatingPointVariableModifier::load(ModifierLoaderContext &context, const Data::FloatingPointVariableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_value = data.value.toDouble();

	return true;
}

Common::SharedPtr<ModifierSaveLoad> FloatingPointVariableModifier::getSaveLoad() {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
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

#ifdef MTROPOLIS_DEBUG_ENABLE
void FloatingPointVariableModifier::debugInspect(IDebugInspectionReport *report) const {
	VariableModifier::debugInspect(report);

	report->declareDynamic("value", Common::String::format("%g", _value));
}
#endif

Common::SharedPtr<Modifier> FloatingPointVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new FloatingPointVariableModifier(*this));
}

const char *FloatingPointVariableModifier::getDefaultName() const {
	return "Floating Point Variable";
}

FloatingPointVariableModifier::SaveLoad::SaveLoad(FloatingPointVariableModifier *modifier) : _modifier(modifier) {
	_value = _modifier->_value;
}

void FloatingPointVariableModifier::SaveLoad::commitLoad() const {
	_modifier->_value = _value;
}

void FloatingPointVariableModifier::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	stream->writeDoubleBE(_value);
}

bool FloatingPointVariableModifier::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
	_value = stream->readDoubleBE();

	if (stream->err())
		return false;

	return true;
}

bool StringVariableModifier::load(ModifierLoaderContext &context, const Data::StringVariableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	_value = data.value;

	return true;
}

Common::SharedPtr<ModifierSaveLoad> StringVariableModifier::getSaveLoad() {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
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

#ifdef MTROPOLIS_DEBUG_ENABLE
void StringVariableModifier::debugInspect(IDebugInspectionReport *report) const {
	VariableModifier::debugInspect(report);

	report->declareDynamic("value", _value);
}
#endif

Common::SharedPtr<Modifier> StringVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new StringVariableModifier(*this));
}

const char *StringVariableModifier::getDefaultName() const {
	return "String Variable";
}

StringVariableModifier::SaveLoad::SaveLoad(StringVariableModifier *modifier) : _modifier(modifier) {
	_value = _modifier->_value;
}

void StringVariableModifier::SaveLoad::commitLoad() const {
	_modifier->_value = _value;
}

void StringVariableModifier::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	stream->writeUint32BE(_value.size());
	stream->writeString(_value);
}

bool StringVariableModifier::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
	uint32 size = stream->readUint32BE();

	if (stream->err())
		return false;

	_value.clear();

	if (size > 0) {
		Common::Array<char> chars;
		chars.resize(size);
		stream->read(&chars[0], size);
		if (stream->err())
			return false;

		_value = Common::String(&chars[0], size);
	}

	return true;
}

} // End of namespace MTropolis
