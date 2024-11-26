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
#include "common/random.h"

#include "graphics/managed_surface.h"

#include "mtropolis/assets.h"
#include "mtropolis/audio_player.h"
#include "mtropolis/coroutines.h"
#include "mtropolis/miniscript.h"
#include "mtropolis/modifiers.h"
#include "mtropolis/modifier_factory.h"
#include "mtropolis/saveload.h"

#include "mtropolis/elements.h"

namespace MTropolis {

class CompoundVarLoader : public ISaveReader {
public:
	CompoundVarLoader(Runtime *runtime, RuntimeObject *object);

	bool readSave(Common::ReadStream *stream, uint32 saveFileVersion) override;

private:
	Runtime *_runtime;
	RuntimeObject *_object;
};

CompoundVarLoader::CompoundVarLoader(Runtime *runtime, RuntimeObject *object) : _runtime(runtime), _object(object) {
}

bool CompoundVarLoader::readSave(Common::ReadStream *stream, uint32 saveFileVersion) {
	if (_object == nullptr || !_object->isModifier())
		return false;

	Modifier *modifier = static_cast<Modifier *>(_object);
	Common::SharedPtr<ModifierSaveLoad> saveLoad = modifier->getSaveLoad(_runtime);
	if (!saveLoad)
		return false;

	if (!saveLoad->load(modifier, stream, saveFileVersion))
		return false;

	if (stream->err())
		return false;

	saveLoad->commitLoad();

	return true;
}

BehaviorModifier::BehaviorModifier() : _switchable(false), _isEnabled(false) {
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

void BehaviorModifier::removeModifier(const Modifier *modifier) {
	for (Common::Array<Common::SharedPtr<Modifier> >::iterator it = _children.begin(), itEnd = _children.end(); it != itEnd; ++it) {
		if (it->get() == modifier) {
			_children.erase(it);
			return;
		}
	}
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

			SwitchTaskData *switchTask = runtime->getVThread().pushTask("BehaviorModifier::switchTask", this, &BehaviorModifier::switchTask);
			switchTask->targetState = false;
			switchTask->eventID = EventIDs::kParentDisabled;
			switchTask->runtime = runtime;
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

#ifdef MTROPOLIS_DEBUG_ENABLE
void BehaviorModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);

	report->declareDynamic("switchable", _switchable ? "true" : "false");
	report->declareDynamic("enabled", _isEnabled ? "true" : "false");
}
#endif

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
		runtime->getVThread().pushCoroutine<MiniscriptThread::ResumeThreadCoroutine>(thread);
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

ColorTableModifier::ColorTableModifier() : _assetID(0xffffffff) {
}

bool ColorTableModifier::load(ModifierLoaderContext &context, const Data::ColorTableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_applyWhen.load(data.applyWhen))
		return false;

	_assetID = data.assetID;

	return true;
}

bool ColorTableModifier::respondsToEvent(const Event &evt) const {
	return _applyWhen.respondsTo(evt);
}

VThreadState ColorTableModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_applyWhen.respondsTo(msg->getEvent())) {
		Common::SharedPtr<Asset> ctabAsset = runtime->getProject()->getAssetByID(_assetID).lock();
		if (ctabAsset) {
			if (ctabAsset->getAssetType() == kAssetTypeColorTable) {
				const ColorRGB8 *colors = static_cast<ColorTableAsset *>(ctabAsset.get())->getColors();

				Palette palette(colors);

				if (runtime->getFakeColorDepth() <= kColorDepthMode8Bit) {
					runtime->setGlobalPalette(palette);
				} else {
					Structural *structural = this->findStructuralOwner();
					if (structural != nullptr && structural->isElement() && static_cast<Element *>(structural)->isVisual()) {
						static_cast<VisualElement *>(structural)->setPalette(Common::SharedPtr<Palette>(new Palette(palette)));
					} else {
						warning("Attempted to apply a color table to a non-element");
					}
				}
			} else {
				error("Color table modifier applied an asset that wasn't a color table");
			}
		} else {
			warning("Failed to apply color table, asset %u wasn't found", _assetID);
		}

		return kVThreadReturn;
	}

	return kVThreadReturn;
}

Common::SharedPtr<Modifier> ColorTableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ColorTableModifier(*this));
}

const char *ColorTableModifier::getDefaultName() const {
	return "Color Table Modifier";
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


SoundFadeModifier::SoundFadeModifier() : _fadeToVolume(0), _durationMSec(0) {
}

bool SoundFadeModifier::load(ModifierLoaderContext &context, const Data::SoundFadeModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_enableWhen.load(data.enableWhen) || !_disableWhen.load(data.disableWhen))
		return false;

	_fadeToVolume = data.fadeToVolume;
	_durationMSec = ((((data.codedDuration[0] * 60) + data.codedDuration[1]) * 60 + data.codedDuration[2]) * 100 + data.codedDuration[3]) * 10;

	return true;
}

bool SoundFadeModifier::respondsToEvent(const Event &evt) const {
	return evt.respondsTo(_enableWhen) || evt.respondsTo(_disableWhen);
}

VThreadState SoundFadeModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	warning("Sound fade modifier is not implemented");
	return kVThreadReturn;
}

Common::SharedPtr<Modifier> SoundFadeModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new SoundFadeModifier(*this));
}

const char *SoundFadeModifier::getDefaultName() const {
	return "Sound Fade Modifier";
}

bool SaveAndRestoreModifier::respondsToEvent(const Event &evt) const {
	if (_saveWhen.respondsTo(evt) || _restoreWhen.respondsTo(evt))
		return true;

	return false;
}

VThreadState SaveAndRestoreModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_saveOrRestoreValue.getSourceType() != DynamicValueSourceTypes::kVariableReference) {
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

	// There doesn't appear to be any flag for this, it just uses the file path field
	bool isPrompt = (_filePath == "Ask User");

	if (_saveWhen.respondsTo(msg->getEvent())) {
		CompoundVarSaver saver(runtime, obj);

		const Graphics::ManagedSurface *screenshotOverrideManaged = runtime->getSaveScreenshotOverride().get();
		const Graphics::Surface *screenshotOverride = nullptr;

		if (screenshotOverrideManaged)
			screenshotOverride = &screenshotOverrideManaged->rawSurface();

		bool succeeded = false;
		if (isPrompt)
			succeeded = runtime->getSaveProvider()->promptSave(&saver, screenshotOverride);
		else
			succeeded = runtime->getSaveProvider()->namedSave(&saver, screenshotOverride, _fileName);

		if (succeeded) {
			for (const Common::SharedPtr<SaveLoadHooks> &hooks : runtime->getHacks().saveLoadHooks)
				hooks->onSave(runtime, this, static_cast<Modifier *>(obj));
		}
		return kVThreadReturn;
	} else if (_restoreWhen.respondsTo(msg->getEvent())) {
		CompoundVarLoader loader(runtime, obj);

		bool succeeded = false;
		if (isPrompt)
			succeeded = runtime->getLoadProvider()->promptLoad(&loader);
		else
			succeeded = runtime->getLoadProvider()->namedLoad(&loader, _fileName);

		if (succeeded) {
			for (const Common::SharedPtr<SaveLoadHooks> &hooks : runtime->getHacks().saveLoadHooks)
				hooks->onLoad(runtime, this, static_cast<Modifier *>(obj));
		}
		return kVThreadReturn;
	}

	return kVThreadError;
}

void SaveAndRestoreModifier::linkInternalReferences(ObjectLinkingScope *scope) {
	Modifier::linkInternalReferences(scope);

	_saveOrRestoreValue.linkInternalReferences(scope);
}

void SaveAndRestoreModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	Modifier::visitInternalReferences(visitor);

	_saveOrRestoreValue.visitInternalReferences(visitor);
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
		_sendSpec.sendFromMessenger(runtime, this, msg->getSource().lock().get(), msg->getValue(), nullptr);
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

bool SetModifier::respondsToEvent(const Event &evt) const {
	if (_executeWhen.respondsTo(evt))
		return true;

	return false;
}

VThreadState SetModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_executeWhen.respondsTo(msg->getEvent())) {
		if (_target.getSourceType() != DynamicValueSourceTypes::kVariableReference) {
#ifdef MTROPOLIS_DEBUG_ENABLE
			if (Debugger *debugger = runtime->debugGetDebugger())
				debugger->notifyFmt(kDebugSeverityError, "Set modifier target isn't a variable reference");
#endif
			return kVThreadError;
		} else {
			Common::SharedPtr<Modifier> targetModifier = _target.getVarReference().resolution.lock();
			if (!targetModifier) {
#ifdef MTROPOLIS_DEBUG_ENABLE
				if (Debugger *debugger = runtime->debugGetDebugger())
					debugger->notifyFmt(kDebugSeverityError, "Set modifier target was invalid");
#endif
				return kVThreadError;
			} else if (!targetModifier->isVariable()) {
#ifdef MTROPOLIS_DEBUG_ENABLE
				if (Debugger *debugger = runtime->debugGetDebugger())
					debugger->notifyFmt(kDebugSeverityError, "Set modifier target was invalid");
#endif
				return kVThreadError;
			} else {
				DynamicValue srcValue = _source.produceValue(msg->getValue());
				VariableModifier *targetVar = static_cast<VariableModifier *>(targetModifier.get());
				if (!targetVar->varSetValue(nullptr, srcValue)) {
#ifdef MTROPOLIS_DEBUG_ENABLE
					if (Debugger *debugger = runtime->debugGetDebugger())
						debugger->notifyFmt(kDebugSeverityError, "Set modifier failed to set target value");
#endif
					return kVThreadError;
				}
			}
		}
	}
	return kVThreadReturn;
}

void SetModifier::linkInternalReferences(ObjectLinkingScope *outerScope) {
	_source.linkInternalReferences(outerScope);
	_target.linkInternalReferences(outerScope);
}

void SetModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	_source.visitInternalReferences(visitor);
	_target.visitInternalReferences(visitor);
}

Common::SharedPtr<Modifier> SetModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new SetModifier(*this));
}

const char *SetModifier::getDefaultName() const {
	return "Set Modifier";
}

AliasModifier::AliasModifier() : _aliasID(0) {
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

SoundEffectModifier::SoundEffectModifier() : _soundType(kSoundTypeBeep), _assetID(0) {
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

PathMotionModifier::PointDef::PointDef() : frame(0), useFrame(false) {
}

PathMotionModifier::PathMotionModifier()
	: _reverse(false), _loop(false), _alternate(false),
	  _startAtBeginning(false), _frameDurationDUSec(1), _isAlternatingDirection(false), _lastPointTimeDUSec(0), _currentPointIndex(0) {
}

PathMotionModifier::~PathMotionModifier() {
	if (_scheduledEvent) {
		_scheduledEvent->cancel();
		_scheduledEvent.reset();
	}
}

bool PathMotionModifier::load(ModifierLoaderContext &context, const Data::PathMotionModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_executeWhen.load(data.executeWhen) || !_terminateWhen.load(data.terminateWhen))
		return false;

	_reverse = ((data.flags & Data::PathMotionModifier::kFlagReverse) != 0);
	_loop = ((data.flags & Data::PathMotionModifier::kFlagLoop) != 0);
	_alternate = ((data.flags & Data::PathMotionModifier::kFlagAlternate) != 0);
	_startAtBeginning = ((data.flags & Data::PathMotionModifier::kFlagStartAtBeginning) != 0);

	_frameDurationDUSec = data.frameDurationTimes10Million;

	if (_frameDurationDUSec == 0)
		_frameDurationDUSec = 1; // Maybe set this to 1/60?  It seems like subframe movement is possible though.

	_points.resize(data.numPoints);

	for (size_t i = 0; i < _points.size(); i++) {
		const Data::PathMotionModifier::PointDef &inPoint = data.points[i];
		PointDef &outPoint = _points[i];

		outPoint.frame = inPoint.frame;
		outPoint.useFrame = ((inPoint.frameFlags & Data::PathMotionModifier::PointDef::kFrameFlagPlaySequentially) == 0);
		if (!inPoint.point.toScummVMPoint(outPoint.point))
			return false;

		if (data.havePointDefMessageSpecs) {
			const Data::PathMotionModifier::PointDefMessageSpec &messageSpec = inPoint.messageSpec;
			if (!outPoint.sendSpec.load(messageSpec.send, messageSpec.messageFlags, messageSpec.with, messageSpec.withSource, messageSpec.withString, messageSpec.destination))
				return false;
		} else {
			outPoint.sendSpec.destination = kMessageDestNone;
		}
	}

	return true;
}

bool PathMotionModifier::respondsToEvent(const Event &evt) const {
	return _executeWhen.respondsTo(evt) || _terminateWhen.respondsTo(evt);
}

VThreadState PathMotionModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_terminateWhen.respondsTo(msg->getEvent())) {
		TerminateTaskData *terminateTask = runtime->getVThread().pushTask("PathMotionModifier::terminateTask", this, &PathMotionModifier::terminateTask);
		terminateTask->runtime = runtime;

		return kVThreadReturn;
	}
	if (_executeWhen.respondsTo(msg->getEvent())) {
		ExecuteTaskData *executeTask = runtime->getVThread().pushTask("PathMotionModifier::executeTask", this, &PathMotionModifier::executeTask);
		executeTask->runtime = runtime;

		_incomingData = msg->getValue();
		_triggerSource = msg->getSource();

		return kVThreadReturn;
	}

	return kVThreadReturn;
}

void PathMotionModifier::disable(Runtime *runtime) {
	if (_scheduledEvent) {
		_scheduledEvent->cancel();
		_scheduledEvent.reset();
	}
}

void PathMotionModifier::linkInternalReferences(ObjectLinkingScope *scope) {
	for (PointDef &point : _points)
		point.sendSpec.linkInternalReferences(scope);
}

void PathMotionModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	for (PointDef &point : _points)
		point.sendSpec.visitInternalReferences(visitor);
}

VThreadState PathMotionModifier::executeTask(const ExecuteTaskData &taskData) {
	if (_points.size() == 0)
		return kVThreadError;

	Runtime *runtime = taskData.runtime;

	uint64 timeMSec = runtime->getPlayTime();

	uint prevPointIndex = _currentPointIndex;
	uint newPointIndex = _reverse ? _points.size() - 1 : 0;

	_lastPointTimeDUSec = timeMSec * 10000u;
	_isAlternatingDirection = false;

	if (_scheduledEvent) {
		_scheduledEvent->cancel();
		_scheduledEvent.reset();
	}

	scheduleNextAdvance(runtime, _lastPointTimeDUSec);

	ChangePointsTaskData *changePointsTask = runtime->getVThread().pushTask("PathMotionModifier::changePoints", this, &PathMotionModifier::changePointsTask);
	changePointsTask->runtime = runtime;
	changePointsTask->prevPoint = (_startAtBeginning ? prevPointIndex : newPointIndex);
	changePointsTask->newPoint = newPointIndex;
	changePointsTask->isTerminal = (_loop == false && _points.size() == 1);

	SendMessageToParentTaskData *sendMessageToParentTask = runtime->getVThread().pushTask("PathMotionModifier::sendMessageToParent", this, &PathMotionModifier::sendMessageToParentTask);
	sendMessageToParentTask->runtime = runtime;
	sendMessageToParentTask->eventID = EventIDs::kMotionStarted;

	return kVThreadReturn;
}

VThreadState PathMotionModifier::changePointsTask(const ChangePointsTaskData &taskData) {
	Runtime *runtime = taskData.runtime;

	_currentPointIndex = taskData.newPoint;

	// TODO: Figure out if this is the correct order.  These are pushed tasks so order is reversed: We set position first,
	// then set cel, then fire the message if any.  I don't think the cel or position change have side effects.
	if (_points[_currentPointIndex].sendSpec.destination != kMessageDestNone) {
		TriggerMessageTaskData *triggerMessageTask = runtime->getVThread().pushTask("PathMotionModifier::triggerMessage", this, &PathMotionModifier::triggerMessageTask);
		triggerMessageTask->runtime = runtime;
		triggerMessageTask->pointIndex = _currentPointIndex;
	}

	// However, we DO need to fire Motion Ended events BEFORE we fire the last point's message.
	if (taskData.isTerminal) {
		SendMessageToParentTaskData *sendToParentTask = runtime->getVThread().pushTask("PathMotionModifier::sendMessageToParent", this, &PathMotionModifier::sendMessageToParentTask);
		sendToParentTask->runtime = runtime;
		sendToParentTask->eventID = EventIDs::kMotionEnded;
	}

	if (_points[_currentPointIndex].useFrame) {
		ChangeCelTaskData *changeCelTask = runtime->getVThread().pushTask("PathMotionModifier::changeCel", this, &PathMotionModifier::changeCelTask);
		changeCelTask->runtime = runtime;
		changeCelTask->pointIndex = _currentPointIndex;
	}

	Common::Point positionDelta = _points[taskData.newPoint].point - _points[taskData.prevPoint].point;
	if (positionDelta != Common::Point(0, 0)) {
		ChangePositionTaskData *changePositionTask = runtime->getVThread().pushTask("PathMotionModifier::changePosition", this, &PathMotionModifier::changePositionTask);
		changePositionTask->runtime = runtime;
		changePositionTask->positionDelta = positionDelta;
	}

	return kVThreadReturn;
}

VThreadState PathMotionModifier::triggerMessageTask(const TriggerMessageTaskData &taskData) {
	_points[taskData.pointIndex].sendSpec.sendFromMessenger(taskData.runtime, this, _triggerSource.lock().get(), _incomingData, nullptr);

	return kVThreadReturn;
}

VThreadState PathMotionModifier::sendMessageToParentTask(const SendMessageToParentTaskData &taskData) {
	Structural *owner = this->findStructuralOwner();

	if (owner) {
		Common::SharedPtr<MessageProperties> props(new MessageProperties(Event(taskData.eventID, 0), DynamicValue(), this->getSelfReference()));
		Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(props, owner, true, true, false));

		// Send immediately
		taskData.runtime->sendMessageOnVThread(dispatch);
	}

	return kVThreadReturn;
}

VThreadState PathMotionModifier::changeCelTask(const ChangeCelTaskData &taskData) {
	if (_points[taskData.pointIndex].useFrame) {
		Structural *structural = findStructuralOwner();

		if (structural) {
			MiniscriptThread thread(taskData.runtime, nullptr, nullptr, nullptr, this);
			DynamicValueWriteProxy proxy;

			MiniscriptInstructionOutcome writeRefOutcome = structural->writeRefAttribute(&thread, proxy, "cel");
			if (writeRefOutcome == kMiniscriptInstructionOutcomeContinue) {
				DynamicValue cel;
				cel.setInt(_points[taskData.pointIndex].frame + 1);
				(void) proxy.pod.ifc->write(&thread, cel, proxy.pod.objectRef, proxy.pod.ptrOrOffset);
			}
		}
	}

	return kVThreadReturn;
}

VThreadState PathMotionModifier::changePositionTask(const ChangePositionTaskData &taskData) {
	Structural *structural = findStructuralOwner();

	if (structural && structural->isElement() && static_cast<Element *>(structural)->isVisual()) {
		VisualElement *visual = static_cast<VisualElement *>(structural);
		VisualElement::OffsetTranslateTaskData *offsetTranslateTask = taskData.runtime->getVThread().pushTask("VisualElement::offsetTranslate", visual, &VisualElement::offsetTranslateTask);
		offsetTranslateTask->dx = taskData.positionDelta.x;
		offsetTranslateTask->dy = taskData.positionDelta.y;
	}

	return kVThreadReturn;
}

VThreadState PathMotionModifier::advanceFrameTask(const AdvanceFrameTaskData &taskData) {
	// Check what the new time will be and if it's in the future.  This also handles the case where the current time was changed
	// due to a triggered message re-executing the modifier: In that case, this will prevent any subframe advances that would have happened.
	uint64 newTime = _lastPointTimeDUSec + _frameDurationDUSec;
	if (newTime >= taskData.terminationTimeDUSec)
		return kVThreadReturn;

	_lastPointTimeDUSec = newTime;

	bool isPlayingForward = (_reverse == _isAlternatingDirection);
	bool isAtLastPoint = isPlayingForward ? (_currentPointIndex == _points.size() - 1) : (_currentPointIndex == 0);

	if (isAtLastPoint) {
		// If this isn't looping, we're done
		if (_loop == false) {
			if (_scheduledEvent) {
				_scheduledEvent->cancel();
				_scheduledEvent.reset();
			}
			return kVThreadReturn;
		}

		// Otherwise, check for alternation and trigger it
		if (_alternate) {
			isPlayingForward = !isPlayingForward;
			_isAlternatingDirection = !_isAlternatingDirection;
		}
	}

	uint prevPointIndex = _currentPointIndex;

	// If the path only has one point, we still act like it's advancing, messages
	uint nextPointIndex = 0;
	if (isPlayingForward) {
		nextPointIndex = _currentPointIndex + 1;
		if (nextPointIndex > _points.size())
			nextPointIndex = 0;
	} else {
		if (_currentPointIndex == 0)
			nextPointIndex = _points.size() - 1;
		else
			nextPointIndex = _currentPointIndex - 1;
	}

	bool isTerminal = false;
	if (!_loop) {
		isTerminal = isPlayingForward ? (nextPointIndex == _points.size() - 1) : (nextPointIndex == 0);
		if (isTerminal && _scheduledEvent) {
			_scheduledEvent->cancel();
			_scheduledEvent.reset();
		}
	}

	// Push the next frame advance for this advancement
	AdvanceFrameTaskData *advanceFrameTask = taskData.runtime->getVThread().pushTask("PathMotionModifier::advanceFrame", this, &PathMotionModifier::advanceFrameTask);
	advanceFrameTask->runtime = taskData.runtime;
	advanceFrameTask->terminationTimeDUSec = taskData.terminationTimeDUSec;

	// Push this frame advance
	ChangePointsTaskData *changePointsTask = taskData.runtime->getVThread().pushTask("PathMotionModifier::changePoints", this, &PathMotionModifier::changePointsTask);
	changePointsTask->runtime = taskData.runtime;
	changePointsTask->prevPoint = prevPointIndex;
	changePointsTask->newPoint = nextPointIndex;
	changePointsTask->isTerminal = isTerminal;

	return kVThreadReturn;
}

void PathMotionModifier::scheduleNextAdvance(Runtime *runtime, uint64 startingFromTimeDUSec) {
	assert(_scheduledEvent.get() == nullptr);

	uint64 nextFrameTimeMSec = (startingFromTimeDUSec + _frameDurationDUSec + 9999u) / 10000u;
	_scheduledEvent = runtime->getScheduler().scheduleMethod<PathMotionModifier, &PathMotionModifier::advance>(nextFrameTimeMSec, this);
}

void PathMotionModifier::advance(Runtime *runtime) {
	_scheduledEvent.reset();

	uint64 currentTimeDUSec = runtime->getPlayTime() * 10000u;

	uint64 framesToAdvance = (currentTimeDUSec - _lastPointTimeDUSec) / _frameDurationDUSec;
	uint64 nextFrameDUSec = _lastPointTimeDUSec + framesToAdvance * _frameDurationDUSec;

	// Schedule the next advance now, since the advance may be cancelled by a message triggered by one of the advances
	scheduleNextAdvance(runtime, nextFrameDUSec);

	AdvanceFrameTaskData *advanceFrameTask = runtime->getVThread().pushTask("PathMotionModifier::advanceFrame", this, &PathMotionModifier::advanceFrameTask);
	advanceFrameTask->runtime = runtime;
	advanceFrameTask->terminationTimeDUSec = currentTimeDUSec;
}

VThreadState PathMotionModifier::terminateTask(const TerminateTaskData &taskData) {
	if (_scheduledEvent) {
		SendMessageToParentTaskData *sendToParentTask = taskData.runtime->getVThread().pushTask("PathMotionModifier::endMotion", this, &PathMotionModifier::sendMessageToParentTask);
		sendToParentTask->runtime = taskData.runtime;
		sendToParentTask->eventID = EventIDs::kMotionEnded;
	}

	disable(taskData.runtime);

	return kVThreadReturn;
}

Common::SharedPtr<Modifier> PathMotionModifier::shallowClone() const {
	Common::SharedPtr<PathMotionModifier> clone(new PathMotionModifier(*this));
	clone->_incomingData = DynamicValue();
	clone->_scheduledEvent.reset();
	return clone;
}

const char *PathMotionModifier::getDefaultName() const {
	return "Path Motion Modifier";
}

SimpleMotionModifier::SimpleMotionModifier() : _motionType(kMotionTypeIntoScene), _directionFlags(0), _steps(0), _delayMSecTimes4800(0), _lastTickTime(0) {
}

SimpleMotionModifier::~SimpleMotionModifier() {
	if (_scheduledEvent) {
		_scheduledEvent->cancel();
		_scheduledEvent.reset();
	}
}

bool SimpleMotionModifier::load(ModifierLoaderContext &context, const Data::SimpleMotionModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_executeWhen.load(data.executeWhen) || !_terminateWhen.load(data.terminateWhen))
		return false;

	_directionFlags = data.directionFlags;
	_steps = data.steps;
	_motionType = static_cast<MotionType>(data.motionType);
	_delayMSecTimes4800 = data.delayMSecTimes4800;

	return true;
}

bool SimpleMotionModifier::respondsToEvent(const Event &evt) const {
	return _executeWhen.respondsTo(evt) || _terminateWhen.respondsTo(evt);
}

VThreadState SimpleMotionModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_executeWhen.respondsTo(msg->getEvent())) {
		if (!_scheduledEvent) {
			if (_motionType == kMotionTypeRandomBounce)
				startRandomBounce(runtime);
			else {
#ifdef MTROPOLIS_DEBUG_ENABLE
				if (Debugger *debugger = runtime->debugGetDebugger())
					debugger->notifyFmt(kDebugSeverityError, "Simple motion modifier was activated with an unsupported motion type");
#endif
			}
		}
		return kVThreadReturn;
	}
	if (_terminateWhen.respondsTo(msg->getEvent())) {
		disable(runtime);
		return kVThreadReturn;
	}
	return kVThreadReturn;
}

void SimpleMotionModifier::disable(Runtime *runtime) {
	if (_scheduledEvent) {
		_scheduledEvent->cancel();
		_scheduledEvent.reset();
	}
}

void SimpleMotionModifier::startRandomBounce(Runtime *runtime) {
	_velocity = Common::Point(24, 24);	// Seems to be static
	_lastTickTime = runtime->getPlayTime();

	_scheduledEvent = runtime->getScheduler().scheduleMethod<SimpleMotionModifier, &SimpleMotionModifier::runRandomBounce>(_lastTickTime + 1, this);
}

void SimpleMotionModifier::runRandomBounce(Runtime *runtime) {
	uint numTicks = 100;

	uint64 currentTime = runtime->getPlayTime();

 	if (_delayMSecTimes4800 > 0) {
		uint64 ticksToExecute = (currentTime - _lastTickTime) * 4800u / _delayMSecTimes4800;

		if (ticksToExecute < 100) {
			numTicks = static_cast<uint>(ticksToExecute);
			_lastTickTime += (static_cast<uint64>(numTicks) * _delayMSecTimes4800 / 4800u);
		} else {
			_lastTickTime = currentTime;
		}
	}

	if (numTicks > 0) {
		Structural *structural = this->findStructuralOwner();
		if (structural && structural->isElement() && static_cast<Element *>(structural)->isVisual()) {
			VisualElement *visual = static_cast<VisualElement *>(structural);

			const Common::Point initialPosition = visual->getGlobalPosition();
			Common::Point newPosition = initialPosition;

			Common::Rect relRect = visual->getRelativeRect();

			int32 w = relRect.width();
			int32 h = relRect.height();

			Window *mainWindow = runtime->getMainWindow().lock().get();
			if (mainWindow) {
				int32 windowWidth = mainWindow->getWidth();
				int32 windowHeight = mainWindow->getHeight();

				for (uint tick = 0; tick < numTicks; tick++) {
					Common::Rect newRect(newPosition.x + _velocity.x, newPosition.y + _velocity.y, newPosition.x + _velocity.x + w, newPosition.y + _velocity.y + h);

					if (newRect.left < 0) {
						newRect.translate(-newRect.left, 0);
						_velocity.x = runtime->getRandom()->getRandomNumber(31) + 1;
					} else if (newRect.right > windowWidth) {
						newRect.translate(windowWidth - newRect.right, 0);
						_velocity.x = -1 - static_cast<int32>(runtime->getRandom()->getRandomNumber(31));
					}

					if (newRect.top < 0) {
						newRect.translate(0, -newRect.top);
						_velocity.y = runtime->getRandom()->getRandomNumber(31) + 1;
					} else if (newRect.bottom > windowHeight) {
						newRect.translate(windowHeight - newRect.bottom, 0);
						_velocity.y = -1 - static_cast<int32>(runtime->getRandom()->getRandomNumber(31));
					}

					newPosition = Common::Point(newRect.left, newRect.top);
					_velocity.y++;
				}
			}

			if (visual->getHooks())
				visual->getHooks()->onSetPosition(runtime, visual, initialPosition, newPosition);

			if (newPosition != initialPosition) {
				VisualElement::OffsetTranslateTaskData *taskData = runtime->getVThread().pushTask("VisualElement::offsetTranslateTask", visual, &VisualElement::offsetTranslateTask);
				taskData->dx = newPosition.x - initialPosition.x;
				taskData->dy = newPosition.y - initialPosition.y;
			}
		}
	}

	_scheduledEvent = runtime->getScheduler().scheduleMethod<SimpleMotionModifier, &SimpleMotionModifier::runRandomBounce>(currentTime + 1, this);
}

Common::SharedPtr<Modifier> SimpleMotionModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new SimpleMotionModifier(*this));
}

const char *SimpleMotionModifier::getDefaultName() const {
	return "Simple Motion Modifier";
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
		DynamicValue vec = _vec.produceValue(msg->getValue());

		if (!vec.convertToType(DynamicValueTypes::kVector, vec)) {
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

	// Variable-sourced motion is continuously updated and doesn't need to be re-triggered.
	// The Pong minigame in Obsidian's Bureau chapter depends on this.
	if (_vec.getSourceType() == DynamicValueSourceTypes::kVariableReference) {
		DynamicValue vec = _vec.produceValue(DynamicValue());

		if (vec.convertToType(DynamicValueTypes::kVector, vec))
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
	_vec.linkInternalReferences(scope);
}

void VectorMotionModifier::visitInternalReferences(IStructuralReferenceVisitor* visitor) {
	_vec.visitInternalReferences(visitor);
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


SharedSceneModifier::SharedSceneModifier() : _targetSectionGUID(0), _targetSubsectionGUID(0), _targetSceneGUID(0) {
}

SharedSceneModifier::~SharedSceneModifier() {
}

bool SharedSceneModifier::load(ModifierLoaderContext &context, const Data::SharedSceneModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_executeWhen.load(data.executeWhen))
		return false;

	_targetSectionGUID = data.sectionGUID;
	_targetSubsectionGUID = data.subsectionGUID;
	_targetSceneGUID = data.sceneGUID;

	return true;
}

bool SharedSceneModifier::respondsToEvent(const Event &evt) const {
	return _executeWhen.respondsTo(evt);
}

VThreadState SharedSceneModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_executeWhen.respondsTo(msg->getEvent())) {
		Project *project = runtime->getProject();
		bool found = false;
		for (const Common::SharedPtr<Structural> &section : project->getChildren()) {
			if (section->getStaticGUID() == _targetSectionGUID) {
				for (const Common::SharedPtr<Structural> &subsection : section->getChildren()) {
					if (subsection->getStaticGUID() == _targetSubsectionGUID) {
						for (const Common::SharedPtr<Structural> &scene : subsection->getChildren()) {
							if (scene->getStaticGUID() == _targetSceneGUID) {
								runtime->addSceneStateTransition(HighLevelSceneTransition(scene, HighLevelSceneTransition::kTypeChangeSharedScene, false, false));
								found = true;
								break;
							}
						}
						break;
					}
				}
				break;
			}
		}

		if (!found) {
#ifdef MTROPOLIS_DEBUG_ENABLE
			if (Debugger *debugger = runtime->debugGetDebugger())
				debugger->notifyFmt(kDebugSeverityError, "Failed to resolve shared scene modifier target scene");
#endif
			return kVThreadError;
		}
	}
	return kVThreadReturn;
}

void SharedSceneModifier::disable(Runtime *runtime) {
}

Common::SharedPtr<Modifier> SharedSceneModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new SharedSceneModifier(*this));
}

const char *SharedSceneModifier::getDefaultName() const {
	return "Shared Scene Modifier";
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

CORO_BEGIN_DEFINITION(IfMessengerModifier::RunEvaluateAndSendCoroutine)
	struct Locals {
		Common::WeakPtr<RuntimeObject> triggerSource;
		DynamicValue incomingData;
		bool isTrue = false;
		Common::SharedPtr<MiniscriptThread> thread;
	};

	CORO_BEGIN_FUNCTION
		// Is this the right place for this?  Not sure if Miniscript can change incomingData
		locals->triggerSource = params->msg->getSource();
		locals->incomingData = params->msg->getValue();

		locals->thread.reset(new MiniscriptThread(params->runtime, params->msg, params->self->_program, params->self->_references, params->self));

		CORO_CALL(MiniscriptThread::ResumeThreadCoroutine, locals->thread);

		CORO_IF (!locals->thread->evaluateTruthOfResult(locals->isTrue))
			CORO_ERROR;
		CORO_END_IF

		CORO_IF(locals->isTrue)
			CORO_AWAIT(params->self->_sendSpec.sendFromMessenger(params->runtime, params->self, locals->triggerSource.lock().get(), locals->incomingData, nullptr));
		CORO_END_IF
	CORO_END_FUNCTION
CORO_END_DEFINITION

VThreadState IfMessengerModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_when.respondsTo(msg->getEvent()))
		runtime->getVThread().pushCoroutine<IfMessengerModifier::RunEvaluateAndSendCoroutine>(this, runtime, msg);

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

TimerMessengerModifier::TimerMessengerModifier() : _milliseconds(0), _looping(false) {
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

		_triggerSource = msg->getSource();

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

	_sendSpec.sendFromMessenger(runtime, this, _triggerSource.lock().get(), _incomingData, nullptr);
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
		_triggerSource = msg->getSource();
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
	_send.sendFromMessenger(runtime, this, _triggerSource.lock().get(), _incomingData, nullptr);
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
	// If we get a message that enables AND disables this at the same time, then we need to detect collisions and fire them,
	// then disable this element.
	// MTI depends on this behavior for the save game menu.

	if (_disableWhen.respondsTo(msg->getEvent())) {
		runtime->getVThread().pushTask("CollisionDetectionModifier::disableTask", this, &CollisionDetectionMessengerModifier::disableTask);
	}
	if (_enableWhen.respondsTo(msg->getEvent())) {
		runtime->getVThread().pushTask("CollisionDetectionModifier::enableTask", this, &CollisionDetectionMessengerModifier::enableTask);

		_incomingData = msg->getValue();
		if (_incomingData.getType() == DynamicValueTypes::kList)
			_incomingData.setList(_incomingData.getList()->clone());
		_triggerSource = msg->getSource();
		_runtime = runtime;
	}

	return kVThreadReturn;
}

VThreadState CollisionDetectionMessengerModifier::enableTask(const EnableTaskData &taskData) {
	if (!_isActive) {
		_isActive = true;
		_runtime->addCollider(this);
		_runtime->checkCollisions(this);
	}
	return kVThreadReturn;
}

VThreadState CollisionDetectionMessengerModifier::disableTask(const DisableTaskData &taskData) {
	disable(_runtime);
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

	_sendSpec.sendFromMessenger(runtime, this, _triggerSource.lock().get(), _incomingData, customDestination);
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
		if (runtime->getProject()->getPlatform() == kProjectPlatformWindows) {
			// Windows projects check "alt"
			if ((keyEvt.flags & Common::KBD_ALT) == 0)
				return false;
		} else if (runtime->getProject()->getPlatform() == kProjectPlatformMacintosh) {
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
		if (runtime->getProject()->getPlatform() == kProjectPlatformWindows)
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
			Common::U32String u(uchar);
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
	_sendSpec.sendFromMessenger(runtime, this, nullptr, charStrValue, nullptr);
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

ImageEffectModifier::ImageEffectModifier() : _type(kTypeUnknown), _bevelWidth(0), _toneAmount(0), _includeBorders(false) {
}

bool ImageEffectModifier::load(ModifierLoaderContext &context, const Data::ImageEffectModifier &data) {
	if (!loadTypicalHeader(data.modHeader) || !_applyWhen.load(data.applyWhen) || !_removeWhen.load(data.removeWhen))
		return false;

	_includeBorders = ((data.flags & 0x40000000) != 0);
	_type = static_cast<Type>(data.type);
	_bevelWidth = data.bevelWidth;
	_toneAmount = data.toneAmount;

	return true;
}

bool ImageEffectModifier::respondsToEvent(const Event &evt) const {
	return _applyWhen.respondsTo(evt) || _removeWhen.respondsTo(evt);
}

VThreadState ImageEffectModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_removeWhen.respondsTo(msg->getEvent())) {
		RemoveTaskData *removeTask = runtime->getVThread().pushTask("ImageEffectModifier::removeTask", this, &ImageEffectModifier::removeTask);
		removeTask->runtime = runtime;
	}
	if (_applyWhen.respondsTo(msg->getEvent())) {
		ApplyTaskData *applyTask = runtime->getVThread().pushTask("ImageEffectModifier::applyTask", this, &ImageEffectModifier::applyTask);
		applyTask->runtime = runtime;
	}

	return kVThreadReturn;
}

void ImageEffectModifier::disable(Runtime *runtime) {
	Structural *structural = findStructuralOwner();
	if (!structural || !structural->isElement() || !static_cast<Element *>(structural)->isVisual())
		return;

	VisualElement *visual = static_cast<VisualElement *>(structural);
	visual->setShading(0, 0, 0, 0);
}

Common::SharedPtr<Modifier> ImageEffectModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ImageEffectModifier(*this));
}

const char *ImageEffectModifier::getDefaultName() const {
	return "Image Effect Modifier";
}

VThreadState ImageEffectModifier::applyTask(const ApplyTaskData &taskData) {
	Structural *structural = findStructuralOwner();
	if (!structural || !structural->isElement() || !static_cast<Element *>(structural)->isVisual())
		return kVThreadReturn;

	VisualElement *visual = static_cast<VisualElement *>(structural);

	int16 shadingLevel = static_cast<int16>(_toneAmount) * 256 / 100;

	switch (_type) {
	case kTypeDeselectedBevels:
		visual->setShading(-shadingLevel, shadingLevel, 0, _bevelWidth);
		break;
	case kTypeSelectedBevels:
		visual->setShading(shadingLevel, -shadingLevel, 0, _bevelWidth);
		break;
	case kTypeToneUp:
		visual->setShading(0, 0, shadingLevel, 0);
		break;
	case kTypeToneDown:
		visual->setShading(0, 0, -shadingLevel, 0);
		break;
	default:
		break;
	}

	return kVThreadReturn;
}

VThreadState ImageEffectModifier::removeTask(const RemoveTaskData &taskData) {
	this->disable(taskData.runtime);

	return kVThreadReturn;
}

ReturnModifier::ReturnModifier() {
}

bool ReturnModifier::load(ModifierLoaderContext &context, const Data::ReturnModifier &data) {
	if (!loadTypicalHeader(data.modHeader) || !_executeWhen.load(data.executeWhen))
		return false;

	return true;
}

bool ReturnModifier::respondsToEvent(const Event &evt) const {
	return _executeWhen.respondsTo(evt);
}

VThreadState ReturnModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	runtime->addSceneReturn();
	return kVThreadReturn;
}

void ReturnModifier::disable(Runtime *runtime) {
}

Common::SharedPtr<Modifier> ReturnModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ReturnModifier(*this));
}

const char *ReturnModifier::getDefaultName() const {
	return "Return Modifier";
}


CursorModifierV1::CursorModifierV1() : _cursorIndex(kCursor_Interact) {
}

bool CursorModifierV1::load(ModifierLoaderContext &context, const Data::CursorModifierV1 &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (data.hasMacOnlyPart)
		_cursorIndex = data.macOnlyPart.cursorIndex;

	return true;
}

bool CursorModifierV1::respondsToEvent(const Event &evt) const {
	return _applyWhen.respondsTo(evt);
}

VThreadState CursorModifierV1::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_applyWhen.respondsTo(msg->getEvent())) {
		warning("Cursor modifier V1 should be applied, but is not implemented");
		return kVThreadReturn;
	}
	return kVThreadReturn;
}

void CursorModifierV1::disable(Runtime *runtime) {
	warning("Cursor modifier V1 should probably dismiss when disabled?");
}

Common::SharedPtr<Modifier> CursorModifierV1::shallowClone() const {
	return Common::SharedPtr<Modifier>(new CursorModifierV1(*this));
}

const char *CursorModifierV1::getDefaultName() const {
	return "Cursor Modifier";
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
	// Do nothing I guess, no variables can be disabled
}

Common::SharedPtr<ModifierSaveLoad> CompoundVariableModifier::getSaveLoad(Runtime *runtime) {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(runtime, this));
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

void CompoundVariableModifier::removeModifier(const Modifier *modifier) {
	for (Common::Array<Common::SharedPtr<Modifier> >::iterator it = _children.begin(), itEnd = _children.end(); it != itEnd; ++it) {
		if (it->get() == modifier) {
			_children.erase(it);
			return;
		}
	}
}

void CompoundVariableModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	for (Common::Array<Common::SharedPtr<Modifier> >::iterator it = _children.begin(), itEnd = _children.end(); it != itEnd; ++it) {
		visitor->visitChildModifierRef(*it);
	}
}

bool CompoundVariableModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	Modifier *var = findChildByName(thread->getRuntime(), attrib);
	if (var) {
		// Shouldn't dereference the value here, some scripts (e.g. "<go dest> on MUI" in Obsidian) depend on it not being dereferenced
		result.setObject(var->getSelfReference());
		return true;
	}
	return Modifier::readAttribute(thread, result, attrib);
}

bool CompoundVariableModifier::readAttributeIndexed(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib, const DynamicValue &index) {
	Modifier *var = findChildByName(thread->getRuntime(), attrib);
	if (!var || !var->isVariable())
		return false;

	return var->readAttributeIndexed(thread, result, "value", index);
}

MiniscriptInstructionOutcome CompoundVariableModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	Modifier *var = findChildByName(thread->getRuntime(), attrib);
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
	Modifier *var = findChildByName(thread->getRuntime(), attrib);
	if (!var || !var->isModifier())
		return kMiniscriptInstructionOutcomeFailed;

	return var->writeRefAttributeIndexed(thread, writeProxy, "value", index);
}

Modifier *CompoundVariableModifier::findChildByName(Runtime *runtime, const Common::String &name) const {
	if (runtime->getHacks().mtiVariableReferencesHack) {
		const Common::String &myName = getName();

		if (myName.size() == 1 && (myName == "a" || myName == "b" || myName == "c" || myName == "d")) {
			Project *project = runtime->getProject();
			Modifier *modifier = project->findGlobalVarWithName(MTropolis::toCaseInsensitive(name)).get();

			if (modifier)
				return modifier;
		}

		if (myName.size() == 1 && myName == "g") {
			if (caseInsensitiveEqual(name, "choresdone") || caseInsensitiveEqual(name, "donechore")) {
				Project *project = runtime->getProject();
				Modifier *modifier = project->findGlobalVarWithName(MTropolis::toCaseInsensitive(name)).get();

				if (modifier)
					return modifier;
			}
		}
	}

	for (Common::Array<Common::SharedPtr<Modifier> >::const_iterator it = _children.begin(), itEnd = _children.end(); it != itEnd; ++it) {
		Modifier *modifier = it->get();
		if (caseInsensitiveEqual(name, modifier->getName()))
			return modifier;
	}

	return nullptr;
}

CompoundVariableModifier::SaveLoad::ChildSaveLoad::ChildSaveLoad() : modifier(nullptr) {
}

CompoundVariableModifier::SaveLoad::SaveLoad(Runtime *runtime, CompoundVariableModifier *modifier) /* : _modifier(modifier) */ {
	// Gross hacks for MTI save games.
	//
	// This looks like it's due to some kind of divergence between mTropolis 1.1 and whatever
	// MTI shipped with.  MTI's saves are done using a compound variable named "MTI" in the Load/Save scene
	// which contains aliases to compound vars a, b, c, d, and g.  While these are aliases to the same globals
	// as are used elsewhere (unlike the "billyState" hack), mTropolis 1.1 will DUPLICATE compound variables children
	// unless the children themselves are aliases, which is not the case in MTI.
	//
	// Consequently, the default behavior here is that the compounds in the Load/Save menu will not reference the
	// children of the aliases compound.  So, we need to patch those references here.
	bool isMTIHackG = false;
	bool isMTIHackGlobalContainer = false;
	if (runtime->getHacks().mtiVariableReferencesHack) {
		const Common::String &name = modifier->getName();
		if (name == "g") {
			isMTIHackG = true;
		} else if (name == "a" || name == "b" || name == "c" || name == "d") {
			isMTIHackGlobalContainer = true;
		}
	}

	if (isMTIHackG) {
		// For "g" use the "g" in the project instead
		for (const Common::SharedPtr<Modifier> &projChild : runtime->getProject()->getModifiers()) {
			if (projChild->getName() == "g" && projChild->isCompoundVariable()) {
				modifier = static_cast<CompoundVariableModifier *>(projChild.get());
				break;
			}
		}
	}

	for (const Common::SharedPtr<Modifier> &child : modifier->_children) {
		bool loadFromGlobal = false;

		if (isMTIHackGlobalContainer)
			loadFromGlobal = true;
		else if (isMTIHackG) {
			// Hack to fix Hispaniola not transitioning to night
			loadFromGlobal = caseInsensitiveEqual(child->getName(), "choresdone") || caseInsensitiveEqual(child->getName(), "donechore");
		}

		if (loadFromGlobal) {
			Common::SharedPtr<Modifier> globalVarModifier = runtime->getProject()->findGlobalVarWithName(child->getName());

			if (globalVarModifier) {
				Common::SharedPtr<ModifierSaveLoad> childSL = globalVarModifier->getSaveLoad(runtime);

				ChildSaveLoad childSaveLoad;
				childSaveLoad.saveLoad = childSL;
				childSaveLoad.modifier = globalVarModifier.get();
				_childrenSaveLoad.push_back(childSaveLoad);

				continue;
			}
		}

		Common::SharedPtr<ModifierSaveLoad> childSL = child->getSaveLoad(runtime);
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

BooleanVariableModifier::BooleanVariableModifier() : VariableModifier(Common::SharedPtr<VariableStorage>(new BooleanVariableStorage())) {
}

bool BooleanVariableModifier::load(ModifierLoaderContext &context, const Data::BooleanVariableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	static_cast<BooleanVariableStorage *>(_storage.get())->_value = (data.value != 0);

	return true;
}

bool BooleanVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	DynamicValue boolValue;
	if (!value.convertToType(DynamicValueTypes::kBoolean, boolValue))
		return false;

	static_cast<BooleanVariableStorage *>(_storage.get())->_value = boolValue.getBool();

	return true;
}

void BooleanVariableModifier::varGetValue(DynamicValue &dest) const {
	dest.setBool(static_cast<BooleanVariableStorage *>(_storage.get())->_value);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void BooleanVariableModifier::debugInspect(IDebugInspectionReport *report) const {
	VariableModifier::debugInspect(report);

	report->declareDynamic("value", static_cast<BooleanVariableStorage *>(_storage.get())->_value ? "true" : "false");
}
#endif

Common::SharedPtr<Modifier> BooleanVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new BooleanVariableModifier(*this));
}

const char *BooleanVariableModifier::getDefaultName() const {
	return "Boolean Variable";
}

BooleanVariableStorage::BooleanVariableStorage() : _value(false) {
}

Common::SharedPtr<ModifierSaveLoad> BooleanVariableStorage::getSaveLoad(Runtime *runtime) {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
}

Common::SharedPtr<VariableStorage> BooleanVariableStorage::clone() const {
	return Common::SharedPtr<VariableStorage>(new BooleanVariableStorage(*this));
}

BooleanVariableStorage::SaveLoad::SaveLoad(BooleanVariableStorage *storage) : _storage(storage) {
	_value = _storage->_value;
}

void BooleanVariableStorage::SaveLoad::commitLoad() const {
	_storage->_value = _value;
}

void BooleanVariableStorage::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	stream->writeByte(_value ? 1 : 0);
}

bool BooleanVariableStorage::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
	byte b = stream->readByte();
	if (stream->err())
		return false;

	_value = (b != 0);
	return true;
}

IntegerVariableModifier::IntegerVariableModifier() : VariableModifier(Common::SharedPtr<VariableStorage>(new IntegerVariableStorage())) {
}

bool IntegerVariableModifier::load(ModifierLoaderContext& context, const Data::IntegerVariableModifier& data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	static_cast<IntegerVariableStorage *>(_storage.get())->_value = data.value;

	return true;
}

IntegerVariableStorage::IntegerVariableStorage() : _value(0) {
}

Common::SharedPtr<ModifierSaveLoad> IntegerVariableStorage::getSaveLoad(Runtime *runtime) {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
}

Common::SharedPtr<VariableStorage> IntegerVariableStorage::clone() const {
	return Common::SharedPtr<VariableStorage>(new IntegerVariableStorage(*this));
}

bool IntegerVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	DynamicValue intValue;
	if (!value.convertToType(DynamicValueTypes::kInteger, intValue))
		return false;

	static_cast<IntegerVariableStorage *>(_storage.get())->_value = intValue.getInt();

	return true;
}

void IntegerVariableModifier::varGetValue(DynamicValue &dest) const {
	dest.setInt(static_cast<IntegerVariableStorage *>(_storage.get())->_value);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void IntegerVariableModifier::debugInspect(IDebugInspectionReport *report) const {
	VariableModifier::debugInspect(report);

	report->declareDynamic("value", Common::String::format("%i", static_cast<IntegerVariableStorage *>(_storage.get())->_value));
}
#endif

Common::SharedPtr<Modifier> IntegerVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new IntegerVariableModifier(*this));
}

const char *IntegerVariableModifier::getDefaultName() const {
	return "Integer Variable";
}

IntegerVariableStorage::SaveLoad::SaveLoad(IntegerVariableStorage *storage) : _storage(storage) {
	_value = _storage->_value;
}

void IntegerVariableStorage::SaveLoad::commitLoad() const {
	_storage->_value = _value;
}

void IntegerVariableStorage::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	stream->writeSint32BE(_value);
}

bool IntegerVariableStorage::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
	_value = stream->readSint32BE();

	if (stream->err())
		return false;

	return true;
}


IntegerRangeVariableModifier::IntegerRangeVariableModifier() : VariableModifier(Common::SharedPtr<VariableStorage>(new IntegerRangeVariableStorage())) {
}

bool IntegerRangeVariableModifier::load(ModifierLoaderContext& context, const Data::IntegerRangeVariableModifier& data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!static_cast<IntegerRangeVariableStorage *>(_storage.get())->_range.load(data.range))
		return false;

	return true;
}

bool IntegerRangeVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	DynamicValue intRangeValue;
	if (!value.convertToType(DynamicValueTypes::kIntegerRange, intRangeValue))
		return false;

	static_cast<IntegerRangeVariableStorage *>(_storage.get())->_range = intRangeValue.getIntRange();

	return true;
}

void IntegerRangeVariableModifier::varGetValue(DynamicValue &dest) const {
	dest.setIntRange(static_cast<IntegerRangeVariableStorage *>(_storage.get())->_range);
}

bool IntegerRangeVariableModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	IntegerRangeVariableStorage *storage = static_cast<IntegerRangeVariableStorage *>(_storage.get());

	if (attrib == "start") {
		result.setInt(storage->_range.min);
		return true;
	}
	if (attrib == "end") {
		result.setInt(storage->_range.max);
		return true;
	}
	return Modifier::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome IntegerRangeVariableModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	IntegerRangeVariableStorage *storage = static_cast<IntegerRangeVariableStorage *>(_storage.get());

	if (attrib == "start") {
		DynamicValueWriteIntegerHelper<int32>::create(&storage->_range.min, result);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "end") {
		DynamicValueWriteIntegerHelper<int32>::create(&storage->_range.max, result);
		return kMiniscriptInstructionOutcomeContinue;
	}
	return Modifier::writeRefAttribute(thread, result, attrib);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void IntegerRangeVariableModifier::debugInspect(IDebugInspectionReport *report) const {
	IntegerRangeVariableStorage *storage = static_cast<IntegerRangeVariableStorage *>(_storage.get());

	VariableModifier::debugInspect(report);

	report->declareDynamic("value", storage->_range.toString());
}
#endif

Common::SharedPtr<Modifier> IntegerRangeVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new IntegerRangeVariableModifier(*this));
}

const char *IntegerRangeVariableModifier::getDefaultName() const {
	return "Integer Range Variable";
}

IntegerRangeVariableStorage::IntegerRangeVariableStorage() {
}

Common::SharedPtr<ModifierSaveLoad> IntegerRangeVariableStorage::getSaveLoad(Runtime *runtime) {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
}

Common::SharedPtr<VariableStorage> IntegerRangeVariableStorage::clone() const {
	return Common::SharedPtr<VariableStorage>(new IntegerRangeVariableStorage(*this));
}

IntegerRangeVariableStorage::SaveLoad::SaveLoad(IntegerRangeVariableStorage *storage) : _storage(storage) {
	_range = _storage->_range;
}

void IntegerRangeVariableStorage::SaveLoad::commitLoad() const {
	_storage->_range = _range;
}

void IntegerRangeVariableStorage::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	stream->writeSint32BE(_storage->_range.min);
	stream->writeSint32BE(_storage->_range.max);
}

bool IntegerRangeVariableStorage::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
	_storage->_range.min = stream->readSint32BE();
	_storage->_range.max = stream->readSint32BE();

	if (stream->err())
		return false;

	return true;
}

VectorVariableModifier::VectorVariableModifier() : VariableModifier(Common::SharedPtr<VariableStorage>(new VectorVariableStorage())) {
}

bool VectorVariableModifier::load(ModifierLoaderContext &context, const Data::VectorVariableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	VectorVariableStorage *storage = static_cast<VectorVariableStorage *>(_storage.get());

	storage->_vector.angleDegrees = data.vector.angleRadians.toDouble() * (180 / M_PI);
	storage->_vector.magnitude = data.vector.magnitude.toDouble();

	return true;
}

bool VectorVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	DynamicValue vectorValue;
	if (!value.convertToType(DynamicValueTypes::kVector, vectorValue))
		return false;

	VectorVariableStorage *storage = static_cast<VectorVariableStorage *>(_storage.get());

	storage->_vector = vectorValue.getVector();

	return true;
}

void VectorVariableModifier::varGetValue(DynamicValue &dest) const {
	VectorVariableStorage *storage = static_cast<VectorVariableStorage *>(_storage.get());

	dest.setVector(storage->_vector);
}

bool VectorVariableModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	VectorVariableStorage *storage = static_cast<VectorVariableStorage *>(_storage.get());

	if (attrib == "magnitude") {
		result.setFloat(storage->_vector.magnitude);
		return true;
	} else if (attrib == "angle") {
		result.setFloat(storage->_vector.angleDegrees);
		return true;
	}

	return VariableModifier::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome VectorVariableModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	VectorVariableStorage *storage = static_cast<VectorVariableStorage *>(_storage.get());

	if (attrib == "magnitude") {
		DynamicValueWriteFloatHelper<double>::create(&storage->_vector.magnitude, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "angle") {
		DynamicValueWriteFloatHelper<double>::create(&storage->_vector.angleDegrees, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return writeRefAttribute(thread, result, attrib);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void VectorVariableModifier::debugInspect(IDebugInspectionReport *report) const {
	VariableModifier::debugInspect(report);

	VectorVariableStorage *storage = static_cast<VectorVariableStorage *>(_storage.get());

	report->declareDynamic("value", storage->_vector.toString());
}
#endif

Common::SharedPtr<Modifier> VectorVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new VectorVariableModifier(*this));
}

const char *VectorVariableModifier::getDefaultName() const {
	return "Vector Variable";
}

VectorVariableStorage::VectorVariableStorage() {
}

Common::SharedPtr<ModifierSaveLoad> VectorVariableStorage::getSaveLoad(Runtime *runtime) {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
}

Common::SharedPtr<VariableStorage> VectorVariableStorage::clone() const {
	return Common::SharedPtr<VariableStorage>(new VectorVariableStorage(*this));
}

VectorVariableStorage::SaveLoad::SaveLoad(VectorVariableStorage *storage) : _storage(storage) {
	_vector = _storage->_vector;
}

void VectorVariableStorage::SaveLoad::commitLoad() const {
	_storage->_vector = _vector;
}

void VectorVariableStorage::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	stream->writeDoubleBE(_vector.angleDegrees);
	stream->writeDoubleBE(_vector.magnitude);
}

bool VectorVariableStorage::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
	_vector.angleDegrees = stream->readDoubleBE();
	_vector.magnitude = stream->readDoubleBE();

	if (stream->err())
		return false;

	return true;
}

PointVariableModifier::PointVariableModifier() : VariableModifier(Common::SharedPtr<VariableStorage>(new PointVariableStorage())) {
}

bool PointVariableModifier::load(ModifierLoaderContext &context, const Data::PointVariableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	PointVariableStorage *storage = static_cast<PointVariableStorage *>(_storage.get());

	if (!data.value.toScummVMPoint(storage->_value))
		return false;

	return true;
}

bool PointVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	DynamicValue pointValue;
	if (!value.convertToType(DynamicValueTypes::kPoint, pointValue))
		return false;

	PointVariableStorage *storage = static_cast<PointVariableStorage *>(_storage.get());

	storage->_value = pointValue.getPoint();

	return true;
}

void PointVariableModifier::varGetValue(DynamicValue &dest) const {
	PointVariableStorage *storage = static_cast<PointVariableStorage *>(_storage.get());

	dest.setPoint(storage->_value);
}

bool PointVariableModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	PointVariableStorage *storage = static_cast<PointVariableStorage *>(_storage.get());

	if (attrib == "x") {
		result.setInt(storage->_value.x);
		return true;
	}
	if (attrib == "y") {
		result.setInt(storage->_value.y);
		return true;
	}

	return VariableModifier::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome PointVariableModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	PointVariableStorage *storage = static_cast<PointVariableStorage *>(_storage.get());

	if (attrib == "x") {
		DynamicValueWriteIntegerHelper<int16>::create(&storage->_value.x, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "y") {
		DynamicValueWriteIntegerHelper<int16>::create(&storage->_value.y, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return writeRefAttribute(thread, writeProxy, attrib);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void PointVariableModifier::debugInspect(IDebugInspectionReport *report) const {
	VariableModifier::debugInspect(report);

	PointVariableStorage *storage = static_cast<PointVariableStorage *>(_storage.get());

	report->declareDynamic("value", pointToString(storage->_value));
}
#endif

Common::SharedPtr<Modifier> PointVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new PointVariableModifier(*this));
}

const char *PointVariableModifier::getDefaultName() const {
	return "Point Variable";
}

PointVariableStorage::PointVariableStorage() {
}

Common::SharedPtr<ModifierSaveLoad> PointVariableStorage::getSaveLoad(Runtime *runtime) {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
}

Common::SharedPtr<VariableStorage> PointVariableStorage::clone() const {
	return Common::SharedPtr<VariableStorage>(new PointVariableStorage(*this));
}

PointVariableStorage::SaveLoad::SaveLoad(PointVariableStorage *storage) : _storage(storage) {
	_value = storage->_value;
}

void PointVariableStorage::SaveLoad::commitLoad() const {
	_storage->_value = _value;
}

void PointVariableStorage::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	stream->writeSint16BE(_value.x);
	stream->writeSint16BE(_value.y);
}

bool PointVariableStorage::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
	_value.x = stream->readSint16BE();
	_value.y = stream->readSint16BE();

	if (stream->err())
		return false;

	return true;
}

FloatingPointVariableModifier::FloatingPointVariableModifier() : VariableModifier(Common::SharedPtr<FloatingPointVariableStorage>(new FloatingPointVariableStorage())) {
}

bool FloatingPointVariableModifier::load(ModifierLoaderContext &context, const Data::FloatingPointVariableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	FloatingPointVariableStorage *storage = static_cast<FloatingPointVariableStorage *>(_storage.get());

	storage->_value = data.value.toDouble();

	return true;
}

bool FloatingPointVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	DynamicValue floatValue;
	if (!value.convertToType(DynamicValueTypes::kFloat, floatValue))
		return false;

	FloatingPointVariableStorage *storage = static_cast<FloatingPointVariableStorage *>(_storage.get());

	storage->_value = floatValue.getFloat();

	return true;
}

void FloatingPointVariableModifier::varGetValue(DynamicValue &dest) const {
	FloatingPointVariableStorage *storage = static_cast<FloatingPointVariableStorage *>(_storage.get());

	dest.setFloat(storage->_value);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void FloatingPointVariableModifier::debugInspect(IDebugInspectionReport *report) const {
	VariableModifier::debugInspect(report);

	FloatingPointVariableStorage *storage = static_cast<FloatingPointVariableStorage *>(_storage.get());

	report->declareDynamic("value", Common::String::format("%g", storage->_value));
}
#endif

Common::SharedPtr<Modifier> FloatingPointVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new FloatingPointVariableModifier(*this));
}

const char *FloatingPointVariableModifier::getDefaultName() const {
	return "Floating Point Variable";
}

FloatingPointVariableStorage::FloatingPointVariableStorage() : _value(0.0) {
}

Common::SharedPtr<ModifierSaveLoad> FloatingPointVariableStorage::getSaveLoad(Runtime *runtime) {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
}

Common::SharedPtr<VariableStorage> FloatingPointVariableStorage::clone() const {
	return Common::SharedPtr<VariableStorage>(new FloatingPointVariableStorage(*this));
}

FloatingPointVariableStorage::SaveLoad::SaveLoad(FloatingPointVariableStorage *storage) : _storage(storage) {
	_value = _storage->_value;
}

void FloatingPointVariableStorage::SaveLoad::commitLoad() const {
	_storage->_value = _value;
}

void FloatingPointVariableStorage::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	stream->writeDoubleBE(_storage->_value);
}

bool FloatingPointVariableStorage::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
	_storage->_value = stream->readDoubleBE();

	if (stream->err())
		return false;

	return true;
}

StringVariableModifier::StringVariableModifier() : VariableModifier(Common::SharedPtr<VariableStorage>(new StringVariableStorage())) {
}

bool StringVariableModifier::load(ModifierLoaderContext &context, const Data::StringVariableModifier &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	StringVariableStorage *storage = static_cast<StringVariableStorage *>(_storage.get());

	storage->_value = data.value;

	return true;
}

bool StringVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	DynamicValue stringValue;
	if (!value.convertToType(DynamicValueTypes::kString, stringValue))
		return false;

	StringVariableStorage *storage = static_cast<StringVariableStorage *>(_storage.get());

	storage->_value = stringValue.getString();

	return true;
}

void StringVariableModifier::varGetValue(DynamicValue &dest) const {
	StringVariableStorage *storage = static_cast<StringVariableStorage *>(_storage.get());

	dest.setString(storage->_value);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void StringVariableModifier::debugInspect(IDebugInspectionReport *report) const {
	VariableModifier::debugInspect(report);

	StringVariableStorage *storage = static_cast<StringVariableStorage *>(_storage.get());

	report->declareDynamic("value", storage->_value);
}
#endif

Common::SharedPtr<Modifier> StringVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new StringVariableModifier(*this));
}

const char *StringVariableModifier::getDefaultName() const {
	return "String Variable";
}

StringVariableStorage::StringVariableStorage() {
}

Common::SharedPtr<ModifierSaveLoad> StringVariableStorage::getSaveLoad(Runtime *runtime) {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
}

Common::SharedPtr<VariableStorage> StringVariableStorage::clone() const {
	return Common::SharedPtr<VariableStorage>(new StringVariableStorage(*this));
}

StringVariableStorage::SaveLoad::SaveLoad(StringVariableStorage *storage) : _storage(storage) {
	_value = _storage->_value;
}

void StringVariableStorage::SaveLoad::commitLoad() const {
	_storage->_value = _value;
}

void StringVariableStorage::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	stream->writeUint32BE(_value.size());
	stream->writeString(_value);
}

bool StringVariableStorage::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
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

ObjectReferenceVariableModifierV1::ObjectReferenceVariableModifierV1() : VariableModifier(Common::SharedPtr<VariableStorage>(new ObjectReferenceVariableV1Storage())) {
}

bool ObjectReferenceVariableModifierV1::load(ModifierLoaderContext &context, const Data::ObjectReferenceVariableModifierV1 &data) {
	if (!loadTypicalHeader(data.modHeader))
		return false;

	if (!_setToSourcesParentWhen.load(data.setToSourcesParentWhen))
		return false;

	return true;
}

bool ObjectReferenceVariableModifierV1::respondsToEvent(const Event &evt) const {
	return _setToSourcesParentWhen.respondsTo(evt);
}

VThreadState ObjectReferenceVariableModifierV1::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (msg->getEvent().respondsTo(_setToSourcesParentWhen)) {
		warning("Set to source's parent is not implemented");
	}
	return kVThreadError;
}


bool ObjectReferenceVariableModifierV1::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "object") {
		ObjectReferenceVariableV1Storage *storage = static_cast<ObjectReferenceVariableV1Storage *>(_storage.get());

		if (storage->_value.expired())
			result.clear();
		else
			result.setObject(storage->_value);
		return true;
	}

	return VariableModifier::readAttribute(thread, result, attrib);
}

bool ObjectReferenceVariableModifierV1::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	ObjectReferenceVariableV1Storage *storage = static_cast<ObjectReferenceVariableV1Storage *>(_storage.get());

	// Somewhat tricky aspect: If this is set to another object reference variable modifier, then this will reference
	// the other object variable modifier, it will NOT copy it.
	if (value.getType() == DynamicValueTypes::kNull)
		storage->_value.reset();
	else if (value.getType() == DynamicValueTypes::kObject)
		storage->_value = value.getObject().object;
	else
		return false;

	return true;
}

void ObjectReferenceVariableModifierV1::varGetValue(DynamicValue &dest) const {
	dest.setObject(getSelfReference());
}

Common::SharedPtr<Modifier> ObjectReferenceVariableModifierV1::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ObjectReferenceVariableModifierV1(*this));
}

const char *ObjectReferenceVariableModifierV1::getDefaultName() const {
	return "Object Reference Variable";
}

ObjectReferenceVariableV1Storage::ObjectReferenceVariableV1Storage() {
}

Common::SharedPtr<ModifierSaveLoad> ObjectReferenceVariableV1Storage::getSaveLoad(Runtime *runtime) {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
}

Common::SharedPtr<VariableStorage> ObjectReferenceVariableV1Storage::clone() const {
	return Common::SharedPtr<VariableStorage>(new ObjectReferenceVariableV1Storage(*this));
}

ObjectReferenceVariableV1Storage::SaveLoad::SaveLoad(ObjectReferenceVariableV1Storage *storage) : _storage(storage) {
}

void ObjectReferenceVariableV1Storage::SaveLoad::commitLoad() const {
	_storage->_value = _value;
}

void ObjectReferenceVariableV1Storage::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	error("Saving version 1 object reference variables is not currently supported");
}

bool ObjectReferenceVariableV1Storage::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
	return true;
}

} // End of namespace MTropolis
