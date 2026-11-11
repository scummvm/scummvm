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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/script/script.h"

#include "ripper/briefing.h"
#include "ripper/dialogue.h"

#include "common/debug.h"
#include "common/ptr.h"
#include "common/serializer.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/util.h"

#include "ripper/detection.h"
#include "ripper/cursor.h"
#include "ripper/input.h"
#include "ripper/inventory.h"
#include "ripper/media/media.h"
#include "ripper/milestones.h"
#include "ripper/modal/modal_dialog.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"
#include "ripper/media/scene_audio.h"
#include "ripper/script/scene_dispatcher.h"
#include "ripper/settings.h"
#include "ripper/ui/toolbar.h"
#include "ripper/world_map.h"

namespace Ripper {

static const uint kToolbarCursor = 14;
static const uint kDialogueCursor = 16;
static const uint16 kHelpCommand = 0x3b00;
static const uint16 kDialogueEnterCommand = 0x0d;
static const uint16 kDialogueUpCommand = 0x4800;
static const uint16 kDialogueDownCommand = 0x5000;
static const uint16 kCyberLeftCommand = 0x4b00;
static const uint16 kCyberRightCommand = 0x4d00;
static const uint16 kCyberChooseCommand = 0x0d;
static const uint16 kCyberEscapeCommand = 0x1b;
static const uint kGeneralHelpResource = 400;
static const uint kPromptHelpResource = 0x19b;
static const uint kCyberHelpResource = 0x1a4;
static const int kSceneInteractionOriginY = 50;

static const uint kDefaultPaletteFadeSteps = 9;
static const uint kSceneTextEntryMaximumLength = 60;

struct ScriptManager::FramePresentationPlan {
	Common::String mediaPath;
	Common::String text;
	uint idleWaitFrame = 0;
	bool dialogueIdleCallback = false;
	bool idleTextRequest = false;
	bool waitsForSceneInput = false;
	bool activatesInputDuringMedia = false;
	bool loopUntilInput = false;
	bool allowEscSpace = false;
};

struct ScriptManager::FramePresentationResult {
	enum Flow {
		kContinueFrame,
		kWaitForInput,
		kSelectFrame,
		kChangeScene,
		kTerminateRuntime,
		kRuntimeReplaced,
		kCyberEscape,
		kFailed
	};

	Flow flow;
	uint nextFrame;
	const char *callbackPhase;

	FramePresentationResult(Flow flow_, uint nextFrame_ = 0,
			const char *callbackPhase_ = nullptr) :
			flow(flow_), nextFrame(nextFrame_), callbackPhase(callbackPhase_) {}
};

class ScriptManager::IdleMediaCallback : public MediaSequenceCallback {
public:
	IdleMediaCallback(ScriptManager *manager, const CompiledScript &script,
			uint32 callbackOffset, uint targetFrame, uint initialFrame) :
			_manager(manager), _script(script), _callbackOffset(callbackOffset),
			_targetFrame(targetFrame), _nextFrame(initialFrame), _result(kCallbackFailed),
			_resumeCommand(0), _requestSelector(0), _failureFrame(0),
			_started(false), _textPending(false), _serviced(false) {
	}

	uint16 service(uint frame) override {
		if (_serviced || (!_started && frame < _targetFrame))
			return 0;
		if (_textPending) {
			const ModalDialogManager::TextEntryResult entryResult =
				_manager->_engine->getModalDialog()->serviceTextEntry(_entered);
			if (entryResult == ModalDialogManager::kTextEntryPending)
				return 0;
			if (entryResult == ModalDialogManager::kTextEntryFailed) {
				_result = kCallbackFailed;
				_serviced = true;
				return 1;
			}
			_textPending = false;
			const bool matched = entryResult == ModalDialogManager::kTextEntryAccepted &&
				textAnswersMatch(_entered, _expected);
			debugC(1, kDebugCyber,
				"Ripper: scene text request completed script='%s' selector=%u accepted=%d matched=%d length=%u failureFrame=%u",
				_script.getMemberName().c_str(), _requestSelector,
				entryResult == ModalDialogManager::kTextEntryAccepted, matched,
				_entered.size(), _failureFrame);
			if (!matched) {
				_nextFrame = _failureFrame;
				_result = kCallbackSelectFrame;
				_serviced = true;
				return 1;
			}
			return execute(frame, _resumeCommand);
		}
		_started = true;
		return execute(frame, 0);
	}

	bool continueAfterEnd() const override { return _textPending; }
	bool ownsInput() const override { return _textPending; }

	bool beginTextRequest(const Common::String &prompt,
			const Common::String &expected, uint failureFrame,
			uint selector, uint resumeCommand) {
		_expected = expected;
		_failureFrame = failureFrame;
		_requestSelector = selector;
		_resumeCommand = resumeCommand;
		_entered.clear();
		_textPending = _manager->_engine->getModalDialog()->beginTextEntry(prompt,
			kSceneTextEntryMaximumLength, kCyberHelpResource,
			"scene-script-password",
			ModalDialogManager::kSceneEntryPresentation);
		return _textPending;
	}

	bool serviced() const { return _serviced; }
	bool succeeded() const { return _result != kCallbackFailed; }
	CallbackResult result() const { return _result; }
	uint nextFrame() const { return _nextFrame; }
	uint targetFrame() const { return _targetFrame; }

private:
	uint16 execute(uint frame, uint commandStart) {
		CallbackContext context;
		context.mediaFrame = frame;
		context.idleMediaCallback = this;
		_result = _manager->executeCallback(_script, _callbackOffset,
			&_nextFrame, commandStart, context);
		if (_result == kCallbackSuspendedForText)
			return 0;
		_serviced = true;
		return 1;
	}
	ScriptManager *_manager;
	const CompiledScript &_script;
	uint32 _callbackOffset;
	uint _targetFrame;
	uint _nextFrame;
	CallbackResult _result;
	uint _resumeCommand;
	uint _requestSelector;
	uint _failureFrame;
	Common::String _expected;
	Common::String _entered;
	bool _started;
	bool _textPending;
	bool _serviced;
};

static Common::String compiledScriptMemberName(const Common::String &target) {
	return target.substr(0, target.findFirstOf('.')) + ".run";
}

static const char *scriptOpcodeName(ScriptOpcode opcode) {
	switch (opcode) {
	case kMilestoneCondition: return "milestone condition";
	case kScenePlayedCondition: return "scene played condition";
	case kDialogueChoiceCondition: return "dialogue choice condition";
	case kPreviousSceneCondition: return "previous scene condition";
	case kDefaultBranch: return "default branch";
	case kClearBranchFlag: return "clear branch state";
	case kSetMilestoneFlag: return "set milestone flag";
	case kClearMilestoneFlag: return "clear milestone flag";
	case kDisableInteraction: return "disable interaction";
	case kSetInteractionSelection: return "set interaction selection";
	case kResetInteractionSelection: return "reset interaction selection";
	case kShowTextPanel: return "show text panel";
	case kSelectFrame: return "select frame";
	case kAddDialogueChoice: return "add dialogue choice";
	case kAddConditionalDialogueChoice: return "add conditional dialogue choice";
	case kStartDialogue: return "start dialogue";
	case kDispatchSceneAction: return "dispatch scene action";
	case kRequestTextInput: return "request text input";
	case kPlayMedia: return "play media";
	case kPreviewMedia: return "preview media";
	case kFadePalette: return "fade palette";
	case kStartSceneRuntime: return "start scene runtime";
	case kSetNamedFlag: return "set named flag";
	case kLoadAudio: return "load audio slot";
	case kConfigureAudio: return "configure audio slot";
	case kClearAudio: return "clear audio slot";
	case kStopAudio: return "stop audio slot";
	case kSetAudioVolume: return "set audio slot volume";
	case kWaitForFrameCounter: return "wait for frame counter";
	default: return opcode <= kNoOp7 ? "no-op" : "unknown";
	}
}

SceneRuntimeState::SceneRuntimeState() : activeFrame(0), frontEndActionMask(0xffff),
		hoveredInteraction(-1), awaitingInteraction(false), resumeLoadedPresentation(false),
		clearPreservedAudioOnTransition(false), cyberActive(false), cyberExitRequested(false),
		cyberKeyboardCommand(0) {
}

ScriptManager::ScriptManager(RipperEngine *engine) : _engine(engine), _demoScriptAbi(false),
		_demoRuntimeComplete(false), _runtimeRestorePending(false),
		_briefing(new BriefingManager(engine)), _dialogue(new DialogueChooser()) {
}

bool ScriptManager::canSaveGame() const {
	return !_runtime.cyberActive && _runtime.awaitingInteraction && !hasPendingSceneTransition() &&
		_runtime.activeFrame < _runtime.activeScript.getFrames().size();
}

void ScriptManager::requestCyberExit(const char *source) {
	if (!_runtime.cyberActive)
		return;
	_dialogue->dismissForSceneTransition("cyber-runtime-exit");
	_runtime.cyberExitRequested = true;
	debugC(1, kDebugCyber, "Ripper: Cyber nested runtime exit requested source=%s", source);
}

void ScriptManager::suspendForCyber(SceneRuntimeState &snapshot) {
	snapshot = Common::move(_runtime);
	_runtime = SceneRuntimeState();
	_runtime.frontEndActionMask = 0;
	_runtime.cyberActive = true;
	debugC(2, kDebugCyber,
		"Ripper: suspended scene runtime script='%s' frame=%u concurrent='%s'",
		snapshot.activeScript.getMemberName().c_str(), snapshot.activeFrame,
		snapshot.concurrentScript.getMemberName().c_str());
}

void ScriptManager::restoreFromCyber(SceneRuntimeState &snapshot) {
	_runtime = Common::move(snapshot);
	debugC(2, kDebugCyber,
		"Ripper: restored suspended scene runtime script='%s' frame=%u concurrent='%s'",
		_runtime.activeScript.getMemberName().c_str(), _runtime.activeFrame,
		_runtime.concurrentScript.getMemberName().c_str());
}

bool ScriptManager::startCyberScript(const Common::String &memberName) {
	if (!_runtime.activeScript.load(_engine->getResources()->scripts(), memberName))
		return false;
	if (_engine->shouldQuit())
		return true;
	return advanceActiveScriptToFrame(0);
}

void ScriptManager::logRuntimeFailure(const char *reason) const {
	Common::String frameLabel;
	if (_runtime.activeFrame < _runtime.activeScript.getFrames().size())
		frameLabel = _runtime.activeScript.getString(_runtime.activeScript.getFrames()[_runtime.activeFrame].labelOffset);
	warning("Ripper: %s activeScript='%s' frame=%u/%u label='%s' awaitingInteraction=%d "
		"concurrentScript='%s' concurrentEntry='%s' pendingScript='%s' pendingEntry='%s'",
		reason, _runtime.activeScript.getMemberName().c_str(), _runtime.activeFrame, _runtime.activeScript.getFrames().size(),
		frameLabel.c_str(), _runtime.awaitingInteraction,
		_runtime.concurrentScript.getMemberName().c_str(), _runtime.concurrentEntryLabel.c_str(),
		_runtime.pendingSceneMember.c_str(), _runtime.pendingSceneEntryLabel.c_str());
}

ScriptManager::SaveState::SaveState(const ScriptManager &scripts) :
		activeMember(scripts._runtime.activeScript.getMemberName()),
		concurrentMember(scripts._runtime.concurrentScript.getMemberName()),
		concurrentEntryLabel(scripts._runtime.concurrentEntryLabel),
		previousFrameLabel(scripts._runtime.previousFrameLabel),
		activeFrame(scripts._runtime.activeFrame),
		frontEndActionMask(scripts._runtime.frontEndActionMask),
		awaitingInteraction(scripts._runtime.awaitingInteraction ? 1 : 0),
		playedScenes(scripts._playedScenes),
		activeInteractionEnabled(scripts._runtime.activeInteractionEnabled),
		briefingArmed(scripts._briefing->isArmed() ? 1 : 0),
		briefingSelector(scripts._briefing->getSelector()),
		announcedSelectors(scripts._briefing->getAnnouncedSelectors()) {
	milestones.restoreFlags(*scripts._engine->getMilestones());
	scripts._dialogue->captureSaveState(dialogue);
}

bool ScriptManager::syncGame(Common::Serializer &serializer) {
	SaveState state = serializer.isSaving() ? SaveState(*this) : SaveState();
	if (!syncGame(serializer, state))
		return false;
	if (serializer.isSaving())
		return true;
	SceneRuntimeState runtime;
	return prepareSaveState(state, runtime) && applySaveState(state, runtime);
}

bool ScriptManager::syncGame(Common::Serializer &serializer, SaveState &state) const {
	if (serializer.isLoading())
		state.announcedSelectors = 0;

	serializer.syncString(state.activeMember);
	serializer.syncAsUint32LE(state.activeFrame);
	if (serializer.getVersion() >= 4)
		serializer.syncAsUint16LE(state.frontEndActionMask);
	else if (serializer.isLoading())
		state.frontEndActionMask = 0xffff;
	serializer.syncString(state.previousFrameLabel);
	serializer.syncString(state.concurrentMember);
	serializer.syncString(state.concurrentEntryLabel);
	serializer.syncAsByte(state.awaitingInteraction);
	serializer.syncAsByte(state.briefingArmed);
	serializer.syncAsUint32LE(state.briefingSelector);
	serializer.syncAsUint16LE(state.announcedSelectors, 6);
	if (serializer.isLoading() && serializer.getVersion() < 6 &&
			state.briefingArmed != 0 && state.briefingSelector < 9)
		state.announcedSelectors = 1U << state.briefingSelector;
	if (state.briefingArmed > 1 || state.awaitingInteraction > 1 ||
			!BriefingManager::isValidRestore(state.briefingArmed != 0, state.briefingSelector, state.announcedSelectors))
		return false;
	if (serializer.isLoading() && (state.activeMember.empty() || state.activeMember.size() > 128 ||
			state.concurrentMember.size() > 128 || state.concurrentEntryLabel.size() > 128 ||
			state.previousFrameLabel.size() > 128))
		return false;

	if (!state.milestones.syncGame(serializer))
		return false;

	uint32 playedSceneCount = state.playedScenes.size();
	serializer.syncAsUint32LE(playedSceneCount);
	if (serializer.isLoading()) {
		if (playedSceneCount > 4096)
			return false;
		state.playedScenes.clear();
		state.playedScenes.resize(playedSceneCount);
	}
	for (uint i = 0; i < playedSceneCount; ++i) {
		serializer.syncString(state.playedScenes[i]);
		if (serializer.isLoading() && state.playedScenes[i].size() > 128)
			return false;
	}

	uint32 interactionCount = state.activeInteractionEnabled.size();
	serializer.syncAsUint32LE(interactionCount);
	if (serializer.isLoading()) {
		if (interactionCount > 255)
			return false;
		state.activeInteractionEnabled.clear();
		state.activeInteractionEnabled.resize(interactionCount);
	}
	for (uint i = 0; i < interactionCount; ++i) {
		byte enabled = state.activeInteractionEnabled[i] ? 1 : 0;
		serializer.syncAsByte(enabled);
		if (serializer.isLoading())
			state.activeInteractionEnabled[i] = enabled != 0;
	}

	return _dialogue->syncGame(serializer, state.dialogue) && !serializer.err();
}

bool ScriptManager::prepareSaveState(const SaveState &state, SceneRuntimeState &runtime) const {
	runtime = SceneRuntimeState();
	if (!runtime.activeScript.load(_engine->getResources()->scripts(), state.activeMember))
		return false;
	if (state.activeFrame >= runtime.activeScript.getFrames().size() ||
			state.activeInteractionEnabled.size() !=
				runtime.activeScript.getFrames()[state.activeFrame].interactionCount)
		return false;
	if (!state.concurrentMember.empty() &&
			!runtime.concurrentScript.load(_engine->getResources()->scripts(), state.concurrentMember))
		return false;
	return true;
}

bool ScriptManager::applySaveState(SaveState &state, SceneRuntimeState &runtime) {
	runtime.concurrentEntryLabel = Common::move(state.concurrentEntryLabel);
	runtime.previousFrameLabel = Common::move(state.previousFrameLabel);
	runtime.activeInteractionEnabled = Common::move(state.activeInteractionEnabled);
	runtime.activeFrame = state.activeFrame;
	runtime.frontEndActionMask = state.frontEndActionMask;
	runtime.awaitingInteraction = state.awaitingInteraction != 0;
	runtime.resumeLoadedPresentation = !state.dialogue.pending &&
		runtime.activeScript.getFrames()[runtime.activeFrame].presentationType == 1;
	if (!_briefing->restore(state.briefingArmed != 0, state.briefingSelector,
			state.announcedSelectors))
		return false;
	_engine->getMilestones()->restoreFlags(state.milestones);
	_runtime = Common::move(runtime);
	_playedScenes = Common::move(state.playedScenes);
	_dialogue->applySaveState(state.dialogue);
	_runtimeRestorePending = true;
	_engine->getToolbar()->leave();
	_engine->getCursor()->setVisible(false);
	debugC(1, kDebugSaveLoad,
		"Ripper: restored script state member='%s' frame=%u label='%s' concurrent='%s' "
		"entry='%s' flags=%u playedScenes=%u interactions=%u dialogue=%d",
		_runtime.activeScript.getMemberName().c_str(), _runtime.activeFrame,
		_runtime.activeScript.getString(_runtime.activeScript.getFrames()[_runtime.activeFrame].labelOffset).c_str(),
		_runtime.concurrentScript.getMemberName().c_str(), _runtime.concurrentEntryLabel.c_str(),
		Milestones::kFlagCount, _playedScenes.size(),
		_runtime.activeInteractionEnabled.size(), _dialogue->isPending());
	return true;
}

ScriptManager::~ScriptManager() {
}

bool ScriptManager::hasActivePrompt() const {
	return _dialogue->isPending();
}

bool ScriptManager::showHelp(const char *source) {
	const bool promptActive = hasActivePrompt();
	const uint resourceId = _runtime.cyberActive ? kCyberHelpResource :
		(promptActive ? kPromptHelpResource : kGeneralHelpResource);
	debugC(1, kDebugScene,
		"Ripper: opening scene help source=%s resource=%u promptActive=%d cyberActive=%d",
		source, resourceId, promptActive, _runtime.cyberActive);
	_engine->getCursor()->setVisible(true);
	const ModalDialogManager::PaletteBehavior paletteBehavior = _runtime.cyberActive ?
		ModalDialogManager::kPreserveActivePalette :
		ModalDialogManager::kApplyModalPalette;
	return _engine->getModalDialog()->run(resourceId, true,
		ModalDialogManager::kMenubPresentation, paletteBehavior);
}

bool ScriptManager::openInventory(int initialUnlockFlag, bool grantItem) {
	if (_runtime.activeFrame >= _runtime.activeScript.getFrames().size()) {
		warning("Ripper: inventory requested without an active scene frame");
		return false;
	}
	const ScriptFrame &frame = _runtime.activeScript.getFrames()[_runtime.activeFrame];
	const Common::String sceneLabel = _runtime.activeScript.getString(frame.labelOffset);
	const Inventory::Result inventoryResult = grantItem ?
		_engine->getInventory()->grantAndRun(initialUnlockFlag, sceneLabel,
			"scene-action-3") :
		_engine->getInventory()->run(sceneLabel, initialUnlockFlag);
	debugC(inventoryResult == Inventory::kLoadFailed ? 1 : 2, kDebugScene,
		"Ripper: inventory completed result=%d sceneLabel='%s' initialUnlockFlag=%d grant=%d",
		inventoryResult, sceneLabel.c_str(), initialUnlockFlag, grantItem);
	return inventoryResult != Inventory::kLoadFailed;
}

bool ScriptManager::initialize(ResourceManager &resources, bool loadRetailOverlays) {
	_demoScriptAbi = !loadRetailOverlays;
	return _briefing->initialize(resources, _demoScriptAbi) &&
		_dialogue->initialize(resources, loadRetailOverlays) &&
		_startup.load(resources.scripts(), "ripper.run") &&
		_runtime.activeScript.load(resources.scripts(), "ba0.run");
}

bool ScriptManager::openWorldMap() {
	Common::String targetScript;
	uint chapter = 0;
	if (!_engine->getWorldMap()->run(targetScript, chapter))
		return false;
	if (!targetScript.empty()) {
		if (!runWorldMapCheckpoint(chapter))
			return false;
		_runtime.pendingSceneMember = compiledScriptMemberName(targetScript);
		_runtime.pendingSceneEntryLabel.clear();
		_runtime.clearPreservedAudioOnTransition = true;
		debugC(2, kDebugScene,
			"Ripper: queued world map scene transition raw='%s' target='%s'",
			targetScript.c_str(), _runtime.pendingSceneMember.c_str());
	}
	return true;
}

bool ScriptManager::runWorldMapCheckpoint(uint chapter) {
	const Common::String memberName =
		Common::String::format("wmap%u.run", chapter);
	AssetLibrary &scripts = _engine->getResources()->scripts();
	if (!scripts.hasMember(memberName)) {
		debugC(2, kDebugScripts,
			"Ripper: no world-map checkpoint chapter=%u member='%s'",
			chapter, memberName.c_str());
		return true;
	}

	CompiledScript checkpoint;
	if (!checkpoint.load(scripts, memberName) ||
			checkpoint.getFrames().size() != 1) {
		warning("Ripper: invalid world-map checkpoint chapter=%u member='%s'",
			chapter, memberName.c_str());
		return false;
	}

	// WMAP checkpoint scripts perform their work in the sole entry callback;
	// the exit callback only terminates the nested runtime.
	const ScriptFrame &frame = checkpoint.getFrames()[0];
	const CallbackResult result = executeCallback(checkpoint, frame.enterCallbackOffset);
	if (result != kCallbackContinue) {
		warning("Ripper: world-map checkpoint chapter=%u member='%s' "
			"enter=0x%x returned result=%d",
			chapter, memberName.c_str(), frame.enterCallbackOffset, result);
		return false;
	}
	debugC(1, kDebugScripts,
		"Ripper: completed world-map checkpoint chapter=%u member='%s' enter=0x%x",
		chapter, memberName.c_str(), frame.enterCallbackOffset);
	return true;
}

Common::String ScriptManager::argumentString(const ScriptArgument &argument) {
	if (argument.data.empty())
		return Common::String();
	const char *text = (const char *)argument.data.data();
	return Common::String(text, Common::strnlen(text, argument.data.size()));
}

bool ScriptManager::isScenePlayed(const Common::String &scene) const {
	for (uint i = 0; i < _playedScenes.size(); ++i) {
		if (_playedScenes[i].equalsIgnoreCase(scene))
			return true;
	}
	return false;
}

void ScriptManager::setScenePlayed(const Common::String &scene, bool played) {
	if (scene.empty())
		return;

	for (uint i = 0; i < _playedScenes.size(); ++i) {
		if (!_playedScenes[i].equalsIgnoreCase(scene))
			continue;
		if (!played)
			_playedScenes.remove_at(i);
		return;
	}

	if (played)
		_playedScenes.push_back(scene);
}

void ScriptManager::markScenePlayed(const Common::String &scene) {
	if (scene.empty() || isScenePlayed(scene))
		return;
	setScenePlayed(scene, true);
	debugC(2, kDebugScene, "Ripper: marked scene played '%s'", scene.c_str());
}

void ScriptManager::retainPreviousInteractionScene(
		const Common::String &mediaPath, const char *source) {
	if (_demoScriptAbi || !mediaPath.hasSuffixIgnoreCase(".avi"))
		return;
	_previousInteractionSceneMediaPath = mediaPath;
	debugC(2, kDebugScene,
		"Ripper: retained previous interaction scene media='%s' source=%s ",
		mediaPath.c_str(), source);
}

bool ScriptManager::replayPreviousInteractionScene(const char *source) {
	if (_demoScriptAbi || _previousInteractionSceneMediaPath.empty()) {
		debugC(2, kDebugScene,
			"Ripper: ignored previous interaction scene replay source=%s "
			"reason=%s",
			source, _demoScriptAbi ? "demo has no Ctrl+R route" :
				"no retained AVI presentation");
		return false;
	}

	debugC(1, kDebugScene,
		"Ripper: replaying previous interaction scene media='%s' source=%s "
		"command=0x%04x",
		_previousInteractionSceneMediaPath.c_str(), source,
		kReplayPreviousSceneCommand);
	const bool replayed = _engine->getMedia()->play(
		_previousInteractionSceneMediaPath, true, -1, -1, true);
	debugC(1, kDebugScene,
		"Ripper: completed previous interaction scene replay media='%s' "
		"source=%s success=%d",
		_previousInteractionSceneMediaPath.c_str(), source, replayed);
	return replayed;
}

uint ScriptManager::resolveFrameIndex(const CompiledScript &script,
		const Common::String &label) const {
	if (label.empty())
		return 0;

	for (uint i = 0; i < script.getFrames().size(); ++i) {
		if (script.getString(script.getFrames()[i].labelOffset).equalsIgnoreCase(label)) {
			return i;
		}
	}

	const Common::String fallbackLabel = script.getFrames().empty() ?
		Common::String() : script.getString(script.getFrames()[0].labelOffset);
	debugC(2, kDebugScene,
		"Ripper: scene script='%s' entry='%s' not found, using retail fallback "
		"frame=0 label='%s'",
		script.getMemberName().c_str(), label.c_str(), fallbackLabel.c_str());
	return 0;
}

void ScriptManager::beginActiveInteractionWait(const Common::String &frameLabel,
		uint interactionCount) {
	RipperEngine::SaveLoadScope saveLoad(*_engine);
	_engine->getInput()->pollEvents();
	_engine->getInput()->discardMouseTransitions();
	_runtime.awaitingInteraction = true;
	debugC(1, kDebugScene,
		"Ripper: active scene script='%s' frame='%s' awaiting %u interactions",
		_runtime.activeScript.getMemberName().c_str(), frameLabel.c_str(), interactionCount);
}

void ScriptManager::initializeActiveInteractionState(const ScriptFrame &frame) {
	_runtime.activeInteractionEnabled.clear();
	_runtime.activeInteractionEnabled.resize(frame.interactionCount);
	for (uint i = 0; i < frame.interactionCount; ++i) {
		const ScriptInteraction &interaction =
			_runtime.activeScript.getInteractions()[frame.firstInteractionIndex + i];
		_runtime.activeInteractionEnabled[i] = (interaction.flags & 2) == 0;
		const Common::Rect bounds = interactionBounds(interaction);
		debugC(3, kDebugScene,
			"Ripper: active scene interaction proxy script='%s' frame=%u interaction=%u "
			"label='%s' raw=%d,%d,%d,%d screen=%d,%d,%d,%d key=0x%04x cursor=%u enabled=%d",
			_runtime.activeScript.getMemberName().c_str(), _runtime.activeFrame,
			frame.firstInteractionIndex + i, interaction.label.c_str(),
			interaction.x, interaction.y, interaction.width, interaction.height,
			bounds.left, bounds.top, bounds.width(), bounds.height(),
			interaction.keyboardCommand, interaction.initialSelection,
			_runtime.activeInteractionEnabled[i]);
	}
	debugC(3, kDebugScene,
		"Ripper: initialized active scene interaction proxies script='%s' frame=%u count=%u",
		_runtime.activeScript.getMemberName().c_str(), _runtime.activeFrame, frame.interactionCount);
}

void ScriptManager::bindActiveFrame(uint frameIndex) {
	if (_runtime.activeFrame < _runtime.activeScript.getFrames().size())
		_runtime.previousFrameLabel = _runtime.activeScript.getString(_runtime.activeScript.getFrames()[_runtime.activeFrame].labelOffset);
	else
		_runtime.previousFrameLabel.clear();
	_runtime.activeFrame = frameIndex;
	debugC(2, kDebugScripts,
		"Ripper: bound active scene script='%s' frame=%u previous='%s'",
		_runtime.activeScript.getMemberName().c_str(), _runtime.activeFrame, _runtime.previousFrameLabel.c_str());
}

ScriptManager::CallbackResult ScriptManager::dispatchSceneAction(const CompiledScript &script,
		const ScriptCommand &command) {
	if (command.arguments.size() < 2)
		return kCallbackFailed;

	const uint action = command.arguments[0].value;
	const uint argument = command.arguments[1].value;
	debugC(2, kDebugScene,
		"Ripper: dispatch scene action=%u name='%s' argument=%u script='%s' offset=0x%x",
		action, SceneActionDispatcher::actionName(action), argument, script.getMemberName().c_str(), command.offset);

	switch (action) {
	case kSceneActionWorldMap:
		if (!openWorldMap())
			return kCallbackFailed;
		return _runtime.pendingSceneMember.empty() ? kCallbackContinue : kCallbackChangeScene;
	case kSceneActionInventory:
		return openInventory(argument, true) ? kCallbackContinue : kCallbackFailed;
	case kSceneActionSetChooserTemplateMode:
		// The native action saves a media slot for the chooser. Scene text entry
		// captures its backing directly from the active ScummVM display.
		return kCallbackContinue;
	case kSceneActionSetFrontEndActionMask:
		_runtime.frontEndActionMask = (uint16)argument;
		debugC(2, kDebugScene,
			"Ripper: scene action 37 set front-end action mask=0x%04x script='%s' offset=0x%x",
			_runtime.frontEndActionMask, script.getMemberName().c_str(), command.offset);
		return kCallbackContinue;
	case kSceneActionTerminateRuntime:
		if (_runtime.cyberActive)
			requestCyberExit("scene-action-9999");
		return kCallbackTerminateRuntime;
	case kSceneActionBriefing:
		return _briefing->arm(argument) ? kCallbackContinue : kCallbackFailed;
	case kSceneActionStartDemoPrologueCue: {
		if (!_demoScriptAbi)
			break;
		// Keep this one-shot cue playing across scene handoffs until action 201.
		SceneAudioManager *sceneAudio = _engine->getSceneAudio();
		if (!sceneAudio->load("r_p_l1.wav", true) ||
				!sceneAudio->configure("r_p_l1", 100, 0, 0))
			return kCallbackFailed;
		return kCallbackContinue;
	}
	case kSceneActionStopDemoPrologueCue:
		if (!_demoScriptAbi)
			break;
		_engine->getSceneAudio()->stop("r_p_l1");
		return kCallbackContinue;
	default:
		break;
	}

	return SceneActionDispatcher::dispatch(_engine, action, argument,
		script.getMemberName().c_str(), command.offset) ? kCallbackContinue : kCallbackFailed;
}

static bool resolveCallbackBranchTarget(const CompiledScript &script,
		const Common::Array<ScriptCommand> &commands, uint32 targetOffset,
		uint &commandIndex) {
	for (uint index = 0; index < commands.size(); ++index) {
		if (commands[index].offset == targetOffset) {
			commandIndex = index;
			return true;
		}
	}
	warning("Ripper: callback branch target 0x%x is not a command boundary in '%s'",
		targetOffset, script.getMemberName().c_str());
	return false;
}

ScriptManager::CallbackResult ScriptManager::executeCallback(const CompiledScript &script, uint32 callbackOffset,
		uint *nextFrame, uint commandStart, const CallbackContext &context) {
	RipperEngine::SaveLoadScope saveLoad(*_engine);
	Common::Array<ScriptCommand> commands;
	if (nextFrame)
		*nextFrame = 0;
	if (!script.decodeCallback(callbackOffset, commands))
		return kCallbackFailed;
	debugC(3, kDebugScripts,
		"Ripper: decoded callback script='%s' offset=0x%x commands=%u",
		script.getMemberName().c_str(), callbackOffset, commands.size());

	uint commandIndex = commandStart;
	bool branchTaken = false;
	while (commandIndex < commands.size()) {
		if (_engine->shouldQuit())
			return kCallbackFailed;
		const ScriptCommand &command = commands[commandIndex];
		debugC(3, kDebugScripts,
			"Ripper: execute script='%s' offset=0x%x opcode=0x%02x name='%s' selector=%u arguments=%u",
			script.getMemberName().c_str(), command.offset, command.opcode,
			scriptOpcodeName(command.opcode), command.selector, command.arguments.size());

		switch (command.opcode) {
		case kMilestoneCondition:
		case kScenePlayedCondition: {
			if (command.arguments.size() < 3)
				return kCallbackFailed;
			const bool expected = command.arguments[0].value != 0;
			bool actual = false;
			if (command.opcode == kMilestoneCondition) {
				const uint flag = command.arguments[1].value;
				actual = _engine->getMilestones()->isSet(flag);
				debugC(3, kDebugMilestones,
					"Ripper: milestone gate flag=%u expected=%d actual=%d target=0x%x offset=0x%x",
					flag, expected, actual, command.arguments[2].value, command.offset);
			} else {
				const Common::String scene = argumentString(command.arguments[1]);
				actual = isScenePlayed(scene);
				debugC(3, kDebugScripts,
					"Ripper: played condition scene='%s' expected=%d actual=%d target=0x%x",
					scene.c_str(), expected, actual, command.arguments[2].value);
			}
			if (actual != expected) {
				branchTaken = true;
				if (!resolveCallbackBranchTarget(script, commands,
						command.arguments[2].value, commandIndex))
					return kCallbackFailed;
				continue;
			}
			break;
		}

		case kDialogueChoiceCondition: {
			if (command.arguments.size() < 2)
				return kCallbackFailed;
			const bool choicesAvailable = _dialogue->hasChoices();
			const bool takeBranch = command.arguments[0].value == 0 ?
				choicesAvailable : !choicesAvailable;
			debugC(2, kDebugDialogue,
				"Ripper: dialogue availability condition choices=%u invert=%u target=0x%x selected=%d",
				choicesAvailable, command.arguments[0].value != 0,
				command.arguments[1].value, takeBranch);
			if (takeBranch) {
				branchTaken = true;
				if (!resolveCallbackBranchTarget(script, commands,
						command.arguments[1].value, commandIndex))
					return kCallbackFailed;
				continue;
			}
			break;
		}

		case kPreviousSceneCondition: {
			if (command.arguments.size() < 3 ||
				command.arguments[1].value >= script.getFrames().size())
				return kCallbackFailed;
			if (&script != &_runtime.activeScript) {
				warning("Ripper: previous-scene condition has no tracked runtime in '%s' at 0x%x",
					script.getMemberName().c_str(), command.offset);
				return kCallbackFailed;
			}

			const bool expected = command.arguments[0].value != 0;
			const ScriptFrame &comparedFrame = script.getFrames()[command.arguments[1].value];
			const Common::String comparedLabel = script.getString(comparedFrame.labelOffset);
			const bool actual = !_runtime.previousFrameLabel.empty() &&
				_runtime.previousFrameLabel.equalsIgnoreCase(comparedLabel);
			debugC(3, kDebugScripts,
				"Ripper: previous-scene condition frame=%u label='%s' previous='%s' "
				"expected=%d actual=%d target=0x%x",
				command.arguments[1].value, comparedLabel.c_str(),
				_runtime.previousFrameLabel.c_str(), expected, actual,
				command.arguments[2].value);
			if (actual != expected) {
				branchTaken = true;
				if (!resolveCallbackBranchTarget(script, commands,
						command.arguments[2].value, commandIndex))
					return kCallbackFailed;
				continue;
			}
			break;
		}

		case kDefaultBranch: {
			if (command.arguments.size() < 1)
				return kCallbackFailed;
			const bool useDefaultBranch = !branchTaken;
			branchTaken = false;
			debugC(3, kDebugScripts, "Ripper: default branch target=0x%x selected=%d",
				command.arguments[0].value, useDefaultBranch);
			if (useDefaultBranch) {
				if (!resolveCallbackBranchTarget(script, commands,
						command.arguments[0].value, commandIndex))
					return kCallbackFailed;
				continue;
			}
			break;
		}

		case kClearBranchFlag:
			branchTaken = false;
			debugC(3, kDebugScripts, "Ripper: cleared callback branch state");
			break;

		case kSetMilestoneFlag:
		case kClearMilestoneFlag: {
			if (command.arguments.size() < 1)
				return kCallbackFailed;
			const bool value = command.opcode == kSetMilestoneFlag;
			if (!_engine->getMilestones()->set(command.arguments[0].value, value,
				value ? "set milestone flag" : "clear milestone flag"))
				return kCallbackFailed;
			break;
		}

		case kDisableInteraction: {
			if (command.arguments.size() < 1)
				return kCallbackFailed;
			if (&script != &_runtime.activeScript || _runtime.activeFrame >= _runtime.activeScript.getFrames().size()) {
				warning("Ripper: interaction disable opcode has no active scene frame in '%s' at 0x%x",
					script.getMemberName().c_str(), command.offset);
				return kCallbackFailed;
			}
			const uint relativeIndex = command.arguments[0].value & 0xffff;
			const ScriptFrame &frame = _runtime.activeScript.getFrames()[_runtime.activeFrame];
			if (relativeIndex < _runtime.activeInteractionEnabled.size()) {
				const uint interactionIndex = frame.firstInteractionIndex + relativeIndex;
				const ScriptInteraction &interaction = _runtime.activeScript.getInteractions()[interactionIndex];
				_runtime.activeInteractionEnabled[relativeIndex] = false;
				debugC(2, kDebugScene,
					"Ripper: disabled frame interaction relative=%u global=%u label='%s'",
					relativeIndex, interactionIndex, interaction.label.c_str());
			} else {
				debugC(2, kDebugScene,
					"Ripper: ignored out-of-range interaction disable relative=%u count=%u",
					relativeIndex, frame.interactionCount);
			}
			break;
		}

		case kPlayMedia: {
			const uint expectedArgumentCount = _demoScriptAbi ? 4 : 5;
			if (command.arguments.size() < expectedArgumentCount)
				return kCallbackFailed;
			const Common::String mediaPath = script.getString(command.arguments[0].value);
			// Compiler stores this optional authored text as an inline
			// type-7 argument. An absent caption is encoded as a one-byte NUL
			// payload; argument.value is not a string-table offset for this type.
			const Common::String presentationText = !_demoScriptAbi &&
				_engine->getSettings()->getSubtitles() ?
				argumentString(command.arguments[1]) : Common::String();
			const bool allowEscSpace = _demoScriptAbi || command.arguments[2].value == 0;
			const int x = (int32)command.arguments[3].value;
			const int y = (int32)command.arguments[_demoScriptAbi ? 2 : 4].value;
			debugC(3, kDebugScripts,
				"Ripper: media command ABI=%s path='%s' controls=%d "
				"position=%d,%d presentationText=%d textLength=%u "
				"textArgumentType=%u textPayloadLength=%u",
				_demoScriptAbi ? "demo" : "retail", mediaPath.c_str(),
				allowEscSpace, x, y, !presentationText.empty(),
				presentationText.size(), command.arguments[1].type,
				command.arguments[1].data.size());
			retainPreviousInteractionScene(mediaPath, "script-media-command");
			_engine->getCursor()->setVisible(false);
			if (!_engine->getMedia()->play(mediaPath, allowEscSpace, x, y, true,
					presentationText))
				return kCallbackFailed;
			const size_t extension = mediaPath.findFirstOf('.');
			const Common::String playedKey = mediaPath.substr(0, extension);
			markScenePlayed(playedKey);
			debugC(2, kDebugScripts,
				"Ripper: completed media command '%s' controls=%d playedKey='%s'",
				mediaPath.c_str(), allowEscSpace, playedKey.c_str());
			break;
		}

		case kAddDialogueChoice:
		case kAddConditionalDialogueChoice:
		case kStartDialogue: {
			bool includeChoice = true;
			if (command.opcode == kAddConditionalDialogueChoice) {
				if (command.arguments.size() < 2 ||
					command.arguments[1].value >= _runtime.activeScript.getFrames().size())
					return kCallbackFailed;
				const ScriptFrame &responseFrame = _runtime.activeScript.getFrames()[command.arguments[1].value];
				const Common::String responseLabel = _runtime.activeScript.getString(responseFrame.labelOffset);
				includeChoice = !isScenePlayed(responseLabel);
				debugC(3, kDebugDialogue,
					"Ripper: dialogue response frame=%u label='%s' played=%d",
					command.arguments[1].value, responseLabel.c_str(), !includeChoice);
			}
			if (!_dialogue->execute(command, includeChoice))
				return kCallbackFailed;
			break;
		}

		case kStartSceneRuntime: {
			const uint expectedArgumentCount = _demoScriptAbi ? 2 : 3;
			if (command.arguments.size() < expectedArgumentCount)
				return kCallbackFailed;
			const Common::String target = argumentString(command.arguments[0]);
			const Common::String entryLabel = argumentString(command.arguments[1]);
			if (_demoScriptAbi) {
				_runtime.pendingSceneMember = compiledScriptMemberName(target);
				_runtime.pendingSceneEntryLabel = entryLabel;
				debugC(1, kDebugScene,
					"Ripper: queued demo script transition target='%s' entry='%s' source='%s'",
					_runtime.pendingSceneMember.c_str(), _runtime.pendingSceneEntryLabel.c_str(),
					script.getMemberName().c_str());
				return kCallbackChangeScene;
			}
			if (&script == &_runtime.concurrentScript || command.arguments[2].value == 0) {
				_runtime.pendingSceneMember = compiledScriptMemberName(target);
				_runtime.pendingSceneEntryLabel = entryLabel;
				if (&script == &_runtime.concurrentScript)
					_runtime.clearPreservedAudioOnTransition = true;
				debugC(1, kDebugScene,
					"Ripper: queued script transition target='%s' entry='%s' source='%s'",
					_runtime.pendingSceneMember.c_str(), _runtime.pendingSceneEntryLabel.c_str(),
					script.getMemberName().c_str());
				return kCallbackChangeScene;
			}
			const Common::String memberName = compiledScriptMemberName(target);
			if (!_runtime.concurrentScript.load(_engine->getResources()->scripts(), memberName))
				return kCallbackFailed;
			_runtime.concurrentEntryLabel = entryLabel;
			debugC(1, kDebugScene, "Ripper: created concurrent script='%s' entry='%s'",
				target.c_str(), entryLabel.c_str());
			break;
		}

		case kSetNamedFlag: {
			if (command.arguments.size() < 2)
				return kCallbackFailed;
			const bool played = command.arguments[0].value != 0;
			const Common::String scene = argumentString(command.arguments[1]);
			if (scene.empty())
				return kCallbackFailed;
			const bool previous = isScenePlayed(scene);			
			setScenePlayed(scene, played);
			debugC(2, kDebugScene,
				"Ripper: updated named scene flag scene='%s' value=%d previous=%d script='%s' offset=0x%x",
				scene.c_str(), played, previous, script.getMemberName().c_str(), command.offset);
			break;
		}

		case kLoadAudio: {
			if (command.arguments.size() < 2)
				return kCallbackFailed;
			const Common::String audioPath = script.getString(command.arguments[0].value);
			const bool preserve = (command.arguments[1].value & 1) != 0;
			if (!_engine->getSceneAudio()->load(audioPath, preserve))
				return kCallbackFailed;
			break;
		}

		case kConfigureAudio: {
			if (command.arguments.size() < 4)
				return kCallbackFailed;
			const Common::String key = argumentString(command.arguments[0]);
			const uint volume = command.arguments[1].value == 0 ? 100 : command.arguments[1].value & 0xff;
			const uint trigger = command.arguments[2].value & 0xffff;
			const uint control = command.arguments[3].value & 0xff;
			if (!_engine->getSceneAudio()->configure(key, volume, trigger, control))
				return kCallbackFailed;
			break;
		}

		case kClearAudio:
		case kStopAudio: {
			if (command.arguments.size() < 1)
				return kCallbackFailed;
			const Common::String key = argumentString(command.arguments[0]);
			if (command.opcode == kClearAudio)
				_engine->getSceneAudio()->clear(key);
			else
				_engine->getSceneAudio()->stop(key);
			break;
		}

		case kSetAudioVolume: {
			if (command.arguments.size() < 4)
				return kCallbackFailed;
			const Common::String key = argumentString(command.arguments[0]);
			_engine->getSceneAudio()->setVolume(key,
				command.arguments[1].value & 0xff,
				command.arguments[2].value & 0xffff,
				command.arguments[3].value & 0xffff);
			break;
		}

		case kWaitForFrameCounter: {
			if (command.arguments.size() < 1)
				return kCallbackFailed;
			const uint targetFrame = command.arguments[0].value;
			if (context.mediaFrame < targetFrame) {
				warning("Ripper: scene callback frame=%u did not reach wait target=%u script='%s' offset=0x%x",
					context.mediaFrame, targetFrame, script.getMemberName().c_str(),
					command.offset);
				return kCallbackFailed;
			}
			debugC(2, kDebugScene,
				"Ripper: scene idle callback reached media frame=%u target=%u script='%s' offset=0x%x",
				context.mediaFrame, targetFrame, script.getMemberName().c_str(),
				command.offset);
			break;
		}

		case kRequestTextInput: {
			if (command.arguments.size() < 4 || !nextFrame ||
					!context.idleMediaCallback)
				return kCallbackFailed;
			const Common::String prompt = argumentString(command.arguments[0]);
			const Common::String expected = argumentString(command.arguments[1]);
			const uint failureFrame = command.arguments[2].value;
			const uint layoutVariant = command.arguments[3].value;
			if (layoutVariant != 2) {
				warning("Ripper: unsupported scene text request layout=%u script='%s' offset=0x%x",
					layoutVariant, script.getMemberName().c_str(), command.offset);
				return kCallbackFailed;
			}

			if (!context.idleMediaCallback->beginTextRequest(prompt, expected,
					failureFrame, command.selector, commandIndex + 1))
				return kCallbackFailed;
			debugC(2, kDebugCyber,
				"Ripper: suspended scene callback for text request script='%s' selector=%u layout=%u resumeCommand=%u",
				script.getMemberName().c_str(), command.selector, layoutVariant,
				commandIndex + 1);
			return kCallbackSuspendedForText;
		}

		case kPreviewMedia: {
			if (command.arguments.size() < 4)
				return kCallbackFailed;
			const Common::String mediaPath = script.getString(command.arguments[0].value);
			ScenePlaybackOptions options;
			options.x = (int32)command.arguments[1].value;
			options.y = (int32)command.arguments[2].value;
			options.firstFrameOnly = true;
			if (!_engine->getMedia()->playScene(mediaPath, options))
				return kCallbackFailed;
			debugC(2, kDebugScripts,
				"Ripper: retained first frame for scene media command '%s'",
				mediaPath.c_str());
			break;
		}

		case kSelectFrame:
			if (command.arguments.size() < 1)
				return kCallbackFailed;
			if (nextFrame)
				*nextFrame = command.arguments[0].value;
			debugC(2, kDebugScripts, "Ripper: callback selected frame=%u control=%d",
				command.arguments[0].value, kCallbackSelectFrame);
			return kCallbackSelectFrame;

		case kDispatchSceneAction: {
			const CallbackResult result = dispatchSceneAction(script, command);
			if (result != kCallbackContinue)
				return result;
			break;
		}

		case kFadePalette: {
			if (command.arguments.size() < 2)
				return kCallbackFailed;
			const bool fadeIn = command.arguments[0].value != 0;
			const int32 configuredSteps = (int32)command.arguments[1].value;
			const uint stepCount = configuredSteps > 0 ? configuredSteps : kDefaultPaletteFadeSteps;
			debugC(2, kDebugVideo, "Ripper: fading palette %s steps=%u",
				fadeIn ? "in" : "out", stepCount);
			debugC(3, kDebugScripts,
				"Ripper: palette transition script='%s' offset=0x%x mode=%u configuredSteps=%d effectiveSteps=%u",
				script.getMemberName().c_str(), command.offset, command.arguments[0].value,
				configuredSteps, stepCount);
			_engine->getMedia()->fadePalette(fadeIn, stepCount);
			break;
		}

		default:
			warning("Ripper: unsupported opcode 0x%02x in '%s' at 0x%x",
				command.opcode, script.getMemberName().c_str(), command.offset);
			return kCallbackFailed;
		}
		++commandIndex;
	}
	return kCallbackContinue;
}

bool ScriptManager::handleActiveRuntimeExit(CallbackResult result, const CompiledScript &script,
		const char *callbackPhase) {
	if (result != kCallbackTerminateRuntime)
		return false;

	if (_runtime.cyberActive) {
		if (!_runtime.cyberExitRequested)
			return false;
		debugC(1, kDebugCyber,
			"Ripper: accepted Cyber runtime exit script='%s' frame=%u callback='%s' result=%d",
			script.getMemberName().c_str(), _runtime.activeFrame, callbackPhase, result);
	} else if (_demoScriptAbi) {
		debugC(1, kDebugScene,
			"Ripper: demo runtime completed script='%s' frame=%u callback='%s' "
			"result=%d; returning to demo coordinator",
			script.getMemberName().c_str(), _runtime.activeFrame, callbackPhase, result);
		_runtime.awaitingInteraction = false;
		_demoRuntimeComplete = true;
	} else {
		debugC(1, kDebugScene,
			"Ripper: active runtime exited normally script='%s' frame=%u callback='%s' "
			"result=%d; requesting engine exit",
			script.getMemberName().c_str(), _runtime.activeFrame, callbackPhase, result);
		_runtime.awaitingInteraction = false;
		_engine->quitGame();
	}
	return true;
}

bool ScriptManager::acceptConcurrentRuntimeExit(CallbackResult result,
		const char *callbackPhase) {
	if (result != kCallbackTerminateRuntime || _runtime.concurrentScript.getFrames().empty())
		return false;

	debugC(1, kDebugScene,
		"Ripper: concurrent runtime exited normally script='%s' entry='%s' callback='%s' result=%d",
		_runtime.concurrentScript.getMemberName().c_str(),
		_runtime.concurrentEntryLabel.c_str(), callbackPhase, result);
	_runtime.concurrentScript = CompiledScript();
	_runtime.concurrentEntryLabel.clear();
	return true;
}

bool ScriptManager::runStartupPath() {
	RipperEngine::SaveLoadScope saveLoad(*_engine);
	if (_startup.getFrames().empty() || _runtime.activeScript.getFrames().empty())
		return false;

	const ScriptFrame &startupFrame = _startup.getFrames()[0];
	const CallbackResult result = executeCallback(_startup, startupFrame.exitCallbackOffset);
	if (result != kCallbackChangeScene || _runtime.pendingSceneMember.empty()) {
		warning("Ripper: startup script did not request the BA0 transition (result=%d)", result);
		return false;
	}

	return performPendingSceneTransition();
}

bool ScriptManager::executeConcurrentFrame() {
	if (_runtime.concurrentScript.getFrames().empty())
		return true;

	const uint frameIndex =
		resolveFrameIndex(_runtime.concurrentScript, _runtime.concurrentEntryLabel);

	const ScriptFrame &frame = _runtime.concurrentScript.getFrames()[frameIndex];
	debugC(2, kDebugScene, "Ripper: servicing concurrent script='%s' frame=%u label='%s'",
		_runtime.concurrentScript.getMemberName().c_str(), frameIndex, _runtime.concurrentEntryLabel.c_str());
	uint nextFrame = frameIndex;
	CallbackResult result = executeCallback(_runtime.concurrentScript, frame.enterCallbackOffset, &nextFrame);
	if (result == kCallbackFailed)
		return false;
	if (acceptConcurrentRuntimeExit(result, "enter"))
		return true;
	if (result == kCallbackChangeScene && !_runtime.pendingSceneMember.empty()) {
		debugC(1, kDebugScene,
			"Ripper: concurrent script requested transition target='%s' entry='%s'",
			_runtime.pendingSceneMember.c_str(), _runtime.pendingSceneEntryLabel.c_str());
		_runtime.concurrentScript = CompiledScript();
		_runtime.concurrentEntryLabel.clear();
		return true;
	}
	if (result != kCallbackContinue)
		return false;
	result = executeCallback(_runtime.concurrentScript, frame.exitCallbackOffset, &nextFrame);
	if (result == kCallbackFailed)
		return false;
	if (acceptConcurrentRuntimeExit(result, "exit"))
		return true;
	if (result == kCallbackChangeScene && !_runtime.pendingSceneMember.empty()) {
		debugC(1, kDebugScene,
			"Ripper: concurrent script requested transition target='%s' entry='%s'",
			_runtime.pendingSceneMember.c_str(), _runtime.pendingSceneEntryLabel.c_str());
		_runtime.concurrentScript = CompiledScript();
		_runtime.concurrentEntryLabel.clear();
		return true;
	}
	if (result != kCallbackSelectFrame) {
		warning("Ripper: concurrent frame '%s' returned unexpected result %d",
			_runtime.concurrentEntryLabel.c_str(), result);
		return false;
	}
	debugC(2, kDebugScene,
		"Ripper: concurrent frame yielded to active scene script='%s' nextFrame=%u",
		_runtime.activeScript.getMemberName().c_str(), nextFrame);
	return true;
}

bool ScriptManager::performPendingSceneTransition() {
	if (_runtime.pendingSceneMember.empty())
		return true;

	const Common::String memberName = _runtime.pendingSceneMember;
	const Common::String entryLabel = _runtime.pendingSceneEntryLabel;
	_runtime.pendingSceneMember.clear();
	_runtime.pendingSceneEntryLabel.clear();
	debugC(1, kDebugScene,
		"Ripper: applying scene transition target='%s' entry='%s' concurrent='%s' clearPreservedAudio=%d",
		memberName.c_str(), entryLabel.c_str(), _runtime.concurrentScript.getMemberName().c_str(),
		_runtime.clearPreservedAudioOnTransition);
	_engine->getSceneAudio()->clearAll(_runtime.clearPreservedAudioOnTransition);
	_runtime.clearPreservedAudioOnTransition = false;
	_engine->getToolbar()->leave();
	_dialogue->dismissForSceneTransition("scene-runtime-transition");
	_briefing->prepareForSceneTransition();
	if (!_runtime.activeScript.load(_engine->getResources()->scripts(), memberName))
		return false;
	const uint startFrame = resolveFrameIndex(_runtime.activeScript, entryLabel);

	_runtime.previousFrameLabel.clear();
	_runtime.activeFrame = startFrame;
	_runtime.frontEndActionMask = 0xffff;
	_runtime.awaitingInteraction = false;
	_runtime.hoveredInteraction = -1;
	_engine->getCursor()->setVisible(false);
	g_system->fillScreen(0);
	presentScreen();
	return advanceActiveScriptToFrame(startFrame);
}

bool ScriptManager::advanceActiveScriptToFrame(uint nextFrame) {
	_engine->getCursor()->setVisible(false);
	_runtime.hoveredInteraction = -1;
	// Frame selection re-enters the loop so the concurrent runtime runs before the
	// next active frame.
	while (!_engine->shouldQuit()) {
		if (!executeConcurrentFrame())
			return false;
		if (!_runtime.pendingSceneMember.empty())
			return performPendingSceneTransition();
		if (nextFrame >= _runtime.activeScript.getFrames().size()) {
			warning("Ripper: active scene script='%s' requested invalid frame %u count=%u",
				_runtime.activeScript.getMemberName().c_str(), nextFrame, _runtime.activeScript.getFrames().size());
			return false;
		}

		bindActiveFrame(nextFrame);
		const ScriptFrame &frame = _runtime.activeScript.getFrames()[_runtime.activeFrame];
		const Common::String label = _runtime.activeScript.getString(frame.labelOffset);
		debugC(1, kDebugScene,
			"Ripper: entering active scene script='%s' frame=%u label='%s' type=%u interactions=%u",
			_runtime.activeScript.getMemberName().c_str(), _runtime.activeFrame, label.c_str(),
			frame.presentationType, frame.interactionCount);
		initializeActiveInteractionState(frame);

		uint callbackFrame = _runtime.activeFrame;
		CallbackResult result = executeCallback(_runtime.activeScript, frame.enterCallbackOffset, &callbackFrame);
		if (result == kCallbackFailed)
			return false;
		if (handleActiveRuntimeExit(result, _runtime.activeScript, "enter"))
			return true;
		if (result == kCallbackChangeScene && !_runtime.pendingSceneMember.empty())
			return performPendingSceneTransition();
		if (result == kCallbackSelectFrame) {
			_engine->getSceneAudio()->resetTriggers();
			nextFrame = callbackFrame;
			continue;
		}
		if (result != kCallbackContinue)
			return false;

		if (frame.presentationType == 0 || frame.presentationType == 1) {
			FramePresentationPlan plan;
			const CallbackResult idleResult = prepareFramePresentation(frame, label, plan);
			if (idleResult == kCallbackFailed)
				return false;
			if (handleActiveRuntimeExit(idleResult, _runtime.activeScript, "idle"))
				return true;
			if (idleResult != kCallbackContinue)
				return false;

			const FramePresentationResult presentation = executeFramePresentation(frame, label, plan);
			switch (presentation.flow) {
			case FramePresentationResult::kContinueFrame:
				break;
			case FramePresentationResult::kWaitForInput:
			case FramePresentationResult::kRuntimeReplaced:
				return true;
			case FramePresentationResult::kSelectFrame:
				_runtime.awaitingInteraction = false;
				_engine->getSceneAudio()->resetTriggers();
				nextFrame = presentation.nextFrame;
				continue;
			case FramePresentationResult::kChangeScene:
				return performPendingSceneTransition();
			case FramePresentationResult::kTerminateRuntime:
				return handleActiveRuntimeExit(kCallbackTerminateRuntime,
					_runtime.activeScript, presentation.callbackPhase);
			case FramePresentationResult::kCyberEscape:
				return serviceCyberKeyboardCommand();
			case FramePresentationResult::kFailed:
				return false;
			}
		}

		if (frame.interactionCount != 0 && !_runtime.awaitingInteraction) {
			beginActiveInteractionWait(label, frame.interactionCount);
			return true;
		}

		callbackFrame = _runtime.activeFrame;
		result = executeCallback(_runtime.activeScript, frame.exitCallbackOffset, &callbackFrame);
		if (result == kCallbackFailed)
			return false;
		if (handleActiveRuntimeExit(result, _runtime.activeScript, "exit"))
			return true;
		if (result == kCallbackChangeScene && !_runtime.pendingSceneMember.empty())
			return performPendingSceneTransition();
		if (result != kCallbackSelectFrame) {
			warning("Ripper: automatic active scene script='%s' frame='%s' returned unexpected result %d",
				_runtime.activeScript.getMemberName().c_str(), label.c_str(), result);
			return false;
		}
		_engine->getSceneAudio()->resetTriggers();
		nextFrame = callbackFrame;
		if (nextFrame == _runtime.activeFrame) {
			debugC(2, kDebugScene,
				"Ripper: active scene script='%s' frame='%s' retained frame=%u; servicing concurrent='%s'",
				_runtime.activeScript.getMemberName().c_str(), label.c_str(), nextFrame,
				_runtime.concurrentScript.getMemberName().c_str());
		}
	}
	return true;
}

ScriptManager::CallbackResult ScriptManager::prepareFramePresentation(
		const ScriptFrame &frame, const Common::String &label, FramePresentationPlan &plan) {
	plan.mediaPath = _runtime.activeScript.getString(frame.mediaNameOffset);
	plan.text = frame.presentationType == 0 && !_demoScriptAbi &&
		_engine->getSettings()->getSubtitles() && frame.textOffset != 0 ?
		_runtime.activeScript.getPresentationText(frame.textOffset) : Common::String();
	debugC(2, kDebugScene,
		"Ripper: frame presentation label='%s' media='%s' origin=%d,%d "
		"presentationText=%d textLength=%u",
		label.c_str(), plan.mediaPath.c_str(), frame.x, frame.y,
		!plan.text.empty(), plan.text.size());
	if (frame.presentationType == 0)
		retainPreviousInteractionScene(plan.mediaPath, "scene-frame-presentation");
	markScenePlayed(label);
	plan.dialogueIdleCallback = frame.idleCallbackOffset != 0 && _dialogue->hasChoices();
	if (plan.dialogueIdleCallback) {
		const CallbackResult result = executeCallback(_runtime.activeScript, frame.idleCallbackOffset);
		if (result == kCallbackFailed || result == kCallbackTerminateRuntime)
			return result;
		if (result != kCallbackContinue) {
			warning("Ripper: dialogue idle callback for frame '%s' returned %d",
				label.c_str(), result);
			return kCallbackFailed;
		}
		debugC(1, kDebugDialogue,
			"Ripper: activated frame dialogue chooser before media label='%s' idle=0x%x",
			label.c_str(), frame.idleCallbackOffset);
	}

	if (frame.idleCallbackOffset != 0 && !_dialogue->hasChoices()) {
		Common::Array<ScriptCommand> idleCommands;
		if (!_runtime.activeScript.decodeCallback(frame.idleCallbackOffset, idleCommands))
			return kCallbackFailed;
		for (uint commandIndex = 0; commandIndex < idleCommands.size(); ++commandIndex) {
			const ScriptCommand &command = idleCommands[commandIndex];
			if (command.opcode == kWaitForFrameCounter && !command.arguments.empty())
				plan.idleWaitFrame = command.arguments[0].value;
			else if (command.opcode == kRequestTextInput)
				plan.idleTextRequest = true;
		}
		if (plan.idleTextRequest && plan.idleWaitFrame == 0) {
			warning("Ripper: scene text request frame='%s' has no media-frame wait", label.c_str());
			return kCallbackFailed;
		}
	}
	const bool dialoguePending = _dialogue->isPending();
	plan.waitsForSceneInput = frame.interactionCount != 0 || dialoguePending || plan.idleTextRequest;
	plan.activatesInputDuringMedia = (frame.presentationType == 1 && frame.interactionCount != 0) ||
		dialoguePending || plan.idleTextRequest;
	// An idle callback may create a dialogue chooser. Keep type-1 media active
	// while either the frame controls or that chooser await input.
	plan.loopUntilInput = frame.presentationType == 1 && (frame.interactionCount != 0 || dialoguePending);
	plan.allowEscSpace = frame.presentationType == 0;
	return kCallbackContinue;
}

ScriptManager::FramePresentationResult ScriptManager::executeFramePresentation(
		const ScriptFrame &frame, const Common::String &label, const FramePresentationPlan &plan) {
	if (plan.activatesInputDuringMedia && !_runtime.awaitingInteraction)
		beginActiveInteractionWait(label, frame.interactionCount);
	debugC(2, kDebugVideo,
		"Ripper: frame presentation controls label='%s' keyboard=%d mouse=%d dialogue=%d "
		"idleTextRequest=%d waitFrame=%u",
		label.c_str(), plan.allowEscSpace, plan.loopUntilInput, _dialogue->isPending(),
		plan.idleTextRequest, plan.idleWaitFrame);
	IdleMediaCallback idleCallback(this, _runtime.activeScript, frame.idleCallbackOffset,
		plan.idleWaitFrame, _runtime.activeFrame);
	uint16 idleCommand = 0;
	ScenePlaybackOptions options;
	options.x = frame.x;
	options.y = frame.y;
	options.loopUntilInput = plan.loopUntilInput;
	options.allowEscSpace = plan.allowEscSpace;
	options.callback = plan.idleTextRequest ? &idleCallback : nullptr;
	options.command = &idleCommand;
	const bool played = frame.presentationType == 0 ?
		_engine->getMedia()->play(plan.mediaPath, plan.allowEscSpace, frame.x,
			frame.y, true, plan.text) :
		_engine->getMedia()->playScene(plan.mediaPath, options);
	if (!played)
		return FramePresentationResult(FramePresentationResult::kFailed);
	if (_runtimeRestorePending) {
		debugC(1, kDebugSaveLoad,
			"Ripper: unwound replaced active presentation media='%s'", plan.mediaPath.c_str());
		return FramePresentationResult(FramePresentationResult::kRuntimeReplaced);
	}
	if (_runtime.cyberKeyboardCommand == kCyberEscapeCommand)
		return FramePresentationResult(FramePresentationResult::kCyberEscape);
	if (plan.idleTextRequest) {
		if (!idleCallback.serviced()) {
			warning("Ripper: scene text request frame='%s' media='%s' ended before target=%u",
				label.c_str(), plan.mediaPath.c_str(), idleCallback.targetFrame());
			return FramePresentationResult(FramePresentationResult::kFailed);
		}
		if (!idleCallback.succeeded())
			return FramePresentationResult(FramePresentationResult::kFailed);
		const CallbackResult result = idleCallback.result();
		const uint callbackFrame = idleCallback.nextFrame();
		debugC(2, kDebugScene,
			"Ripper: scene text idle callback completed frame='%s' result=%d nextFrame=%u mediaCommand=0x%04x",
			label.c_str(), result, callbackFrame, idleCommand);
		if (result == kCallbackTerminateRuntime)
			return FramePresentationResult(FramePresentationResult::kTerminateRuntime,
				callbackFrame, "media idle");
		if (result == kCallbackSelectFrame)
			return FramePresentationResult(FramePresentationResult::kSelectFrame, callbackFrame);
		if (result != kCallbackContinue)
			return FramePresentationResult(FramePresentationResult::kFailed);
	}
	if (frame.idleCallbackOffset != 0 && !plan.dialogueIdleCallback && !plan.idleTextRequest) {
		// EE2.RUN frame EEZ1 relies on the idle callback running before chooser
		// input so Q2_V5.WAV plays before its hotspot becomes active.
		uint callbackFrame = _runtime.activeFrame;
		const CallbackResult result = executeCallback(_runtime.activeScript, frame.idleCallbackOffset, &callbackFrame);
		if (result == kCallbackFailed)
			return FramePresentationResult(FramePresentationResult::kFailed);
		debugC(2, kDebugScene,
			"Ripper: serviced frame idle callback before interaction frame='%s' idle=0x%x result=%d nextFrame=%u",
			label.c_str(), frame.idleCallbackOffset, result, callbackFrame);
		if (result == kCallbackTerminateRuntime)
			return FramePresentationResult(FramePresentationResult::kTerminateRuntime,
				callbackFrame, "interaction idle");
		if (result == kCallbackChangeScene && !_runtime.pendingSceneMember.empty())
			return FramePresentationResult(FramePresentationResult::kChangeScene);
		if (result == kCallbackSelectFrame)
			return FramePresentationResult(FramePresentationResult::kSelectFrame, callbackFrame);
		if (result != kCallbackContinue)
			return FramePresentationResult(FramePresentationResult::kFailed);
	}
	if (!_runtime.pendingSceneMember.empty()) {
		debugC(1, kDebugScene,
			"Ripper: scene presentation returned transition target='%s' entry='%s'",
			_runtime.pendingSceneMember.c_str(), _runtime.pendingSceneEntryLabel.c_str());
		return FramePresentationResult(FramePresentationResult::kChangeScene);
	}
	// Controlled AVI restores the surrounding display. Other media keeps its
	// final frame, including full-screen documents such as KK_Z12.SMK.
	if (plan.allowEscSpace && plan.mediaPath.hasSuffixIgnoreCase(".avi")) {
		_dialogue->rebuildPresentationBands("controlled-avi-complete");
	} else if (plan.allowEscSpace) {
		debugC(2, kDebugScene, "Ripper: retained controlled sequence display media='%s'", plan.mediaPath.c_str());
	}
	if (frame.interactionCount != 0 && !_runtime.awaitingInteraction) {
		beginActiveInteractionWait(label, frame.interactionCount);
		debugC(2, kDebugScene,
			"Ripper: activated post-presentation interactions frame='%s' count=%u",
			label.c_str(), frame.interactionCount);
	}
	return FramePresentationResult(plan.waitsForSceneInput ?
		FramePresentationResult::kWaitForInput : FramePresentationResult::kContinueFrame);
}

bool ScriptManager::captureCyberKeyboardCommand() {
	if (!_runtime.cyberActive || !_runtime.awaitingInteraction ||
			!_engine->getInput()->hasPendingKey())
		return false;
	const uint16 command = _engine->getInput()->peekKey();
	if (command != kCyberLeftCommand && command != kCyberRightCommand &&
			command != kCyberChooseCommand && command != kCyberEscapeCommand)
		return false;
	// IC_P* idle callbacks may wait to open text with no frame interactions.
	// Left, Right, and Enter must not stop media before that callback runs.
	if (command != kCyberEscapeCommand) {
		bool hasBoundInteraction = false;
		if (_runtime.activeFrame < _runtime.activeScript.getFrames().size()) {
			const ScriptFrame &frame = _runtime.activeScript.getFrames()[_runtime.activeFrame];
			for (uint relativeIndex = 0; relativeIndex < frame.interactionCount; ++relativeIndex) {
				const uint candidateIndex = frame.firstInteractionIndex + relativeIndex;
				if (relativeIndex < _runtime.activeInteractionEnabled.size() &&
						_runtime.activeInteractionEnabled[relativeIndex] &&
						candidateIndex < _runtime.activeScript.getInteractions().size() &&
						_runtime.activeScript.getInteractions()[candidateIndex].keyboardCommand == command) {
					hasBoundInteraction = true;
					break;
				}
			}
		}
		if (!hasBoundInteraction) {
			_engine->getInput()->consumeKey();
			debugC(3, kDebugCyber,
				"Ripper: ignored unbound Cyber keyboard command=0x%04x script='%s' frame=%u",
				command, _runtime.activeScript.getMemberName().c_str(), _runtime.activeFrame);
			return false;
		}
	}
	_runtime.cyberKeyboardCommand = _engine->getInput()->consumeKey();
	debugC(3, kDebugCyber,
		"Ripper: captured Cyber keyboard command=0x%04x script='%s' frame=%u",
		_runtime.cyberKeyboardCommand, _runtime.activeScript.getMemberName().c_str(), _runtime.activeFrame);
	return true;
}

bool ScriptManager::serviceCyberKeyboardCommand() {
	const uint16 command = _runtime.cyberKeyboardCommand;
	_runtime.cyberKeyboardCommand = 0;
	if (!_runtime.cyberActive || command == 0)
		return true;
	if (command == kCyberEscapeCommand) {
		requestCyberExit("keyboard-escape");
		return true;
	}
	if (!_runtime.awaitingInteraction || _runtime.activeFrame >= _runtime.activeScript.getFrames().size())
		return false;

	const ScriptFrame &frame = _runtime.activeScript.getFrames()[_runtime.activeFrame];
	const ScriptInteraction *interaction = nullptr;
	uint interactionIndex = 0;
	for (uint relativeIndex = 0; relativeIndex < frame.interactionCount; ++relativeIndex) {
		const uint candidateIndex = frame.firstInteractionIndex + relativeIndex;
		if (relativeIndex < _runtime.activeInteractionEnabled.size() &&
				_runtime.activeInteractionEnabled[relativeIndex] &&
				candidateIndex < _runtime.activeScript.getInteractions().size() &&
				_runtime.activeScript.getInteractions()[candidateIndex].keyboardCommand == command) {
			interaction = &_runtime.activeScript.getInteractions()[candidateIndex];
			interactionIndex = candidateIndex;
			break;
		}
	}
	if (!interaction || interaction->callbackOffset == 0) {
		warning("Ripper: Cyber keyboard command 0x%04x has no bound interaction in script='%s' frame=%u",
			command, _runtime.activeScript.getMemberName().c_str(), _runtime.activeFrame);
		return false;
	}

	uint nextFrame = _runtime.activeFrame;
	debugC(2, kDebugCyber,
		"Ripper: Cyber keyboard command=0x%04x interaction=%u label='%s' callback=0x%x",
		command, interactionIndex, interaction->label.c_str(), interaction->callbackOffset);
	const CallbackResult result = executeCallback(_runtime.activeScript, interaction->callbackOffset, &nextFrame);
	if (result == kCallbackFailed)
		return false;
	_runtime.awaitingInteraction = false;
	_engine->getCursor()->setVisible(false);
	_runtime.hoveredInteraction = -1;
	if (handleActiveRuntimeExit(result, _runtime.activeScript, "keyboard interaction"))
		return true;
	if (result != kCallbackSelectFrame) {
		warning("Ripper: Cyber keyboard interaction=%u returned unexpected result %d",
			interactionIndex, result);
		return false;
	}
	_engine->getSceneAudio()->resetTriggers();
	return advanceActiveScriptToFrame(nextFrame);
}

int ScriptManager::resolveToolbarActionKey(uint16 command) const {
	if (_runtime.cyberActive || !_runtime.awaitingInteraction)
		return -1;
	return _engine->getToolbar()->resolveActionKey(command,
		_runtime.frontEndActionMask);
}

bool ScriptManager::dispatchToolbarActionKey(uint actionIndex, uint16 command,
		const char *source) {
	return _engine->getToolbar()->dispatchActionKey(actionIndex, command, source);
}

bool ScriptManager::serviceToolbarActionKey(const char *source, bool &handled) {
	handled = false;
	const uint16 command = _engine->getInput()->peekKey();
	const int actionIndex = resolveToolbarActionKey(command);
	if (actionIndex < 0)
		return true;

	_engine->getInput()->consumeKey();
	handled = true;
	if (!dispatchToolbarActionKey(actionIndex, command, source))
		return false;
	if (!_runtime.pendingSceneMember.empty())
		return performPendingSceneTransition();
	return true;
}

bool ScriptManager::triggerHelpHotkey(const char *source) {
	debugC(1, kDebugInput,
		"Ripper: help hotkey triggered command=0x%04x key='F1' source=%s",
		kHelpCommand, source ? source : "unknown");
	showScreenMessage("Hotkey F1: Help");
	presentScreen();
	return showHelp(source);
}

bool ScriptManager::serviceScene() {
	if (_runtimeRestorePending) {
		_runtimeRestorePending = false;
		debugC(1, kDebugSaveLoad,
			"Ripper: continuing loaded script runtime member='%s' frame=%u",
			_runtime.activeScript.getMemberName().c_str(), _runtime.activeFrame);
	}
	if (_runtime.resumeLoadedPresentation) {
		_runtime.resumeLoadedPresentation = false;
		const ScriptFrame &frame = _runtime.activeScript.getFrames()[_runtime.activeFrame];
		const Common::String label = _runtime.activeScript.getString(frame.labelOffset);
		const Common::String mediaPath = _runtime.activeScript.getString(frame.mediaNameOffset);
		debugC(1, kDebugSaveLoad,
			"Ripper: resuming loaded interactive presentation frame=%u label='%s' media='%s'",
			_runtime.activeFrame, label.c_str(), mediaPath.c_str());
		ScenePlaybackOptions options;
		options.x = frame.x;
		options.y = frame.y;
		options.loopUntilInput = true;
		if (!_engine->getMedia()->playScene(mediaPath, options))
			return false;
		if (_runtimeRestorePending)
			return true;
		if (!_runtime.pendingSceneMember.empty())
			return performPendingSceneTransition();
	}
	if (!_demoScriptAbi &&
			_engine->getSettings()->handlePresentationTextCommand(
				_engine->getInput()->peekKey(), "scene-runtime")) {
		_engine->getInput()->consumeKey();
		return true;
	}
	if (!_demoScriptAbi && _runtime.awaitingInteraction &&
			_engine->getInput()->peekKey() == kReplayPreviousSceneCommand) {
		_engine->getInput()->consumeKey();
		replayPreviousInteractionScene("scene-runtime");
		return true;
	}
	const MouseState mouse = _engine->getInput()->publishMouseState();
	if (_runtime.cyberExitRequested)
		return true;
	captureCyberKeyboardCommand();
	if (_runtime.cyberKeyboardCommand != 0)
		return serviceCyberKeyboardCommand();
	if (_engine->getInput()->peekKey() == kHelpCommand) {
		_engine->getInput()->consumeKey();
		if (!triggerHelpHotkey("scene"))
			return false;
		return true;
	}
	if (!_dialogue->isPending()) {
		bool hotkeyHandled = false;
		if (!serviceToolbarActionKey("scene-runtime", hotkeyHandled))
			return false;
		if (hotkeyHandled)
			return true;
	}
	const BriefingServiceResult briefingResult = _briefing->service(mouse);
	if (briefingResult == kBriefingFailed)
		return false;
	if (briefingResult != kBriefingIdle)
		return true;
	const bool dialoguePending = _dialogue->isPending();
	if (dialoguePending) {
		const uint16 dialogueCommand = _engine->getInput()->peekKey();
		if (dialogueCommand == kDialogueEnterCommand ||
				dialogueCommand == kDialogueUpCommand ||
				dialogueCommand == kDialogueDownCommand) {
			_engine->getInput()->consumeKey();
			uint dialogueFrame = 0;
			if (_dialogue->serviceKeyboard(dialogueCommand, dialogueFrame)) {
				_runtime.awaitingInteraction = false;
				_engine->getCursor()->setVisible(false);
				debugC(1, kDebugDialogue,
					"Ripper: dialogue keyboard chooser returned control=-2 nextFrame=%u",
					dialogueFrame);
				_engine->getSceneAudio()->resetTriggers();
				return advanceActiveScriptToFrame(dialogueFrame);
			}
		}
		bool hotkeyHandled = false;
		if (!serviceToolbarActionKey("dialogue-runtime", hotkeyHandled))
			return false;
		if (hotkeyHandled)
			return true;
		if (!_runtime.cyberActive && _engine->getToolbar()->service(mouse, _runtime.frontEndActionMask)) {
			if (!_runtime.pendingSceneMember.empty())
				return performPendingSceneTransition();
			_engine->getCursor()->setVisible(true);
			_engine->getCursor()->update(kToolbarCursor);
			return true;
		}
		uint dialogueFrame = 0;
		if (_dialogue->service(mouse, dialogueFrame)) {
			_runtime.awaitingInteraction = false;
			_engine->getCursor()->setVisible(false);
			debugC(1, kDebugDialogue,
				"Ripper: dialogue chooser returned control=-2 nextFrame=%u", dialogueFrame);
			_engine->getSceneAudio()->resetTriggers();
			return advanceActiveScriptToFrame(dialogueFrame);
		}
		_dialogue->draw();
		if (mouse.pressed != 0 || mouse.released != 0 || mouse.wheel != 0) {
			debugC(3, kDebugDialogue,
				"Ripper: dialogue chooser pending input point=%d,%d buttons=0x%02x "
				"pressed=0x%02x released=0x%02x wheel=%d",
				mouse.position.x, mouse.position.y, mouse.buttons, mouse.pressed,
				mouse.released, mouse.wheel);
		}
		if (_dialogue->contains(mouse.position)) {
			_engine->getCursor()->setVisible(true);
			_engine->getCursor()->update(kDialogueCursor);
			return true;
		}
	}
	if (!_runtime.awaitingInteraction) {
		_engine->getToolbar()->leave();
		_engine->getCursor()->setVisible(false);
		_runtime.hoveredInteraction = -1;
		return true;
	}
	if (!_runtime.cyberActive && !dialoguePending &&
			_engine->getToolbar()->service(mouse, _runtime.frontEndActionMask)) {
		if (!_runtime.pendingSceneMember.empty())
			return performPendingSceneTransition();
		_engine->getCursor()->update(kToolbarCursor);
		return true;
	}

	uint hoveredInteractionIndex = 0;
	const ScriptInteraction *hoveredInteraction =
		findActiveInteraction(mouse.position, &hoveredInteractionIndex);

	const uint cursorIndex = hoveredInteraction ?
		(hoveredInteraction->conditionOffset != 0 ? 8 : hoveredInteraction->initialSelection) :
		_engine->getCursor()->getSelectionIndex();
	_engine->getCursor()->update(cursorIndex);
	const int hoveredIndex = hoveredInteraction ? (int)hoveredInteractionIndex : -1;
	if (hoveredIndex != _runtime.hoveredInteraction) {
		_runtime.hoveredInteraction = hoveredIndex;
		if (hoveredInteraction) {
			const Common::Rect bounds = interactionBounds(*hoveredInteraction);
			debugC(2, kDebugScene,
				"Ripper: active scene hover script='%s' interaction=%u label='%s' cursor=%u "
				"point=%d,%d rect=%d,%d,%d,%d",
				_runtime.activeScript.getMemberName().c_str(), hoveredInteractionIndex,
				hoveredInteraction->label.c_str(), cursorIndex,
				mouse.position.x, mouse.position.y, bounds.left, bounds.top,
				bounds.width(), bounds.height());
		} else {
			debugC(2, kDebugScene,
				"Ripper: active scene hover cleared script='%s' cursor=%u point=%d,%d",
				_runtime.activeScript.getMemberName().c_str(), cursorIndex,
				mouse.position.x, mouse.position.y);
		}
	}

	if ((mouse.pressed & kMouseButtonLeft) == 0)
		return true;
	if (hoveredInteraction) {
		const Common::Rect bounds = interactionBounds(*hoveredInteraction);
		if (dialoguePending) {
			_dialogue->dismissForSceneTransition("scene-interaction");
			debugC(1, kDebugDialogue,
				"Ripper: dialogue chooser closed by scene interaction=%u label='%s'",
				hoveredInteractionIndex, hoveredInteraction->label.c_str());
		}
		debugC(2, kDebugScene,
			"Ripper: active scene chooser selected script='%s' interaction=%u label='%s' point=%d,%d rect=%d,%d,%d,%d",
			_runtime.activeScript.getMemberName().c_str(), hoveredInteractionIndex,
			hoveredInteraction->label.c_str(), mouse.position.x, mouse.position.y,
			bounds.left, bounds.top, bounds.width(), bounds.height());
		uint nextFrame = _runtime.activeFrame;
		const CallbackResult result = executeCallback(_runtime.activeScript, hoveredInteraction->callbackOffset, &nextFrame);
		if (result == kCallbackFailed)
			return false;
		debugC(1, kDebugScripts,
			"Ripper: hotspot action script='%s' callback=0x%x result=%d nextFrame=%u",
			_runtime.activeScript.getMemberName().c_str(), hoveredInteraction->callbackOffset, result, nextFrame);
		if (result == kCallbackChangeScene && !_runtime.pendingSceneMember.empty()) {
			_runtime.awaitingInteraction = false;
			_engine->getCursor()->setVisible(false);
			_runtime.hoveredInteraction = -1;
			return performPendingSceneTransition();
		}
		if (handleActiveRuntimeExit(result, _runtime.activeScript, "pointer interaction")) {
			_runtime.awaitingInteraction = false;
			_engine->getCursor()->setVisible(false);
			_runtime.hoveredInteraction = -1;
			return true;
		}
		if (result != kCallbackSelectFrame) {
			warning("Ripper: active scene script='%s' interaction=%u returned unexpected result %d",
				_runtime.activeScript.getMemberName().c_str(), hoveredInteractionIndex, result);
			return false;
		}
		_runtime.awaitingInteraction = false;
		_engine->getCursor()->setVisible(false);
		_runtime.hoveredInteraction = -1;
		debugC(1, kDebugScene,
			"Ripper: active scene script='%s' yielded to concurrent script='%s' entry='%s' nextFrame=%u",
			_runtime.activeScript.getMemberName().c_str(), _runtime.concurrentScript.getMemberName().c_str(),
			_runtime.concurrentEntryLabel.c_str(), nextFrame);
		_engine->getSceneAudio()->resetTriggers();
		return advanceActiveScriptToFrame(nextFrame);
	}

	debugC(3, kDebugScene,
		"Ripper: active scene primary press missed chooser script='%s' point=%d,%d",
		_runtime.activeScript.getMemberName().c_str(), mouse.position.x, mouse.position.y);
	return true;
}

void ScriptManager::drawDialogueOverlay(bool captureBacking) {
	_dialogue->draw(captureBacking);
}

void ScriptManager::drawBriefingOverlay() {
	_briefing->draw();
}

bool ScriptManager::updateInteractiveCursor(const Common::Point &point, bool *failed) {
	if (captureCyberKeyboardCommand())
		return false;
	const BriefingServiceResult briefingResult =
		_briefing->service(_engine->getInput()->peekMouseState());
	if (failed)
		*failed = briefingResult == kBriefingFailed;
	if (briefingResult != kBriefingIdle)
		return false;
	if (!_runtime.awaitingInteraction || _runtime.activeFrame >= _runtime.activeScript.getFrames().size())
		return false;
	if (!_runtime.cyberActive && _engine->getToolbar()->service(
			_engine->getInput()->peekMouseState(), _runtime.frontEndActionMask)) {
		_engine->getCursor()->setVisible(true);
		_engine->getCursor()->update(kToolbarCursor);
		return true;
	}
	if (_dialogue->isPending()) {
		_dialogue->updateHover(point);
		const ScriptInteraction *interaction = findActiveInteraction(point);
		const uint cursorIndex = _dialogue->contains(point) ? kDialogueCursor :
			(interaction ? (interaction->conditionOffset != 0 ? 8 : interaction->initialSelection) :
				_engine->getCursor()->getSelectionIndex());
		_engine->getCursor()->setVisible(true);
		_engine->getCursor()->update(cursorIndex);
		return false;
	}
	const ScriptInteraction *interaction = findActiveInteraction(point);
	const uint cursorIndex = interaction ?
		(interaction->conditionOffset != 0 ? 8 : interaction->initialSelection) :
		_engine->getCursor()->getSelectionIndex();
	_engine->getCursor()->setVisible(true);
	_engine->getCursor()->update(cursorIndex);
	return false;
}

void ScriptManager::updateModalSceneCursor(const Common::Point &point) {
	// The modal borrows the scene's selection list. Outside its rectangle,
	// service hotspot cursors without dispatching their actions.
	const ScriptInteraction *interaction = _runtime.awaitingInteraction ?
		findActiveInteraction(point) : nullptr;
	uint cursorIndex = interaction ?
		(interaction->conditionOffset != 0 ? 8 : interaction->initialSelection) :
		_engine->getCursor()->getSelectionIndex();
	if (_dialogue->isPending() && _dialogue->contains(point))
		cursorIndex = kDialogueCursor;
	_engine->getCursor()->setVisible(true);
	_engine->getCursor()->update(cursorIndex);
}

Common::Rect ScriptManager::interactionBounds(const ScriptInteraction &interaction) const {
	return Common::Rect(interaction.y, interaction.x + kSceneInteractionOriginY,
		interaction.y + interaction.height,
		interaction.x + kSceneInteractionOriginY + interaction.width);
}

const ScriptInteraction *ScriptManager::findActiveInteraction(const Common::Point &point,
		uint *interactionIndex) const {
	if (_runtime.activeFrame >= _runtime.activeScript.getFrames().size() || point.y >= 400)
		return nullptr;
	const ScriptFrame &frame = _runtime.activeScript.getFrames()[_runtime.activeFrame];
	for (uint i = 0; i < frame.interactionCount; ++i) {
		const uint index = frame.firstInteractionIndex + i;
		const ScriptInteraction &interaction = _runtime.activeScript.getInteractions()[index];
		if (i >= _runtime.activeInteractionEnabled.size() || !_runtime.activeInteractionEnabled[i] ||
			(interaction.flags & 2) != 0 || interaction.width <= 0 || interaction.height <= 0)
			continue;
		if (!interactionBounds(interaction).contains(point))
			continue;
		if (interactionIndex)
			*interactionIndex = index;
		return &interaction;
	}
	return nullptr;
}

} // End of namespace Ripper
