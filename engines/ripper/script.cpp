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

#include "ripper/script.h"

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
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"
#include "ripper/scene_audio.h"
#include "ripper/scene_dispatcher.h"
#include "ripper/toolbar.h"
#include "ripper/world_map.h"

namespace Ripper {

// RunStartupFrontEndLoop at 0x10778 uses selection cursor 0xe for the white
// menu pointer. PollInteractionAndResolveSelection at 0x13c8d enters the same
// front-end action service for the top 50-pixel band of every scene frame.
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
static const int kCallbackSuspendedForText = -5;

class ScriptManager::IdleMediaCallback : public MediaSequenceCallback {
public:
	IdleMediaCallback(ScriptManager *manager, CompiledScript &script,
			uint32 callbackOffset, uint targetFrame, uint initialFrame) :
			_manager(manager), _script(script), _callbackOffset(callbackOffset),
			_targetFrame(targetFrame), _nextFrame(initialFrame), _result(0),
			_resumeCommand(0), _requestSelector(0), _failureFrame(0),
			_started(false), _textPending(false), _serviced(false),
			_succeeded(false) {
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
				_succeeded = false;
				_serviced = true;
				return 1;
			}
			_textPending = false;
			const bool matched = entryResult == ModalDialogManager::kTextEntryAccepted &&
				ScriptManager::textAnswersMatch(_entered, _expected);
			debugC(1, kDebugCyber,
				"Ripper: scene text request completed script='%s' selector=%u accepted=%d matched=%d length=%u failureFrame=%u",
				_script.getMemberName().c_str(), _requestSelector,
				entryResult == ModalDialogManager::kTextEntryAccepted, matched,
				_entered.size(), _failureFrame);
			if (!matched) {
				_nextFrame = _failureFrame;
				_result = -2;
				_succeeded = true;
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
	bool succeeded() const { return _succeeded; }
	int result() const { return _result; }
	uint nextFrame() const { return _nextFrame; }
	uint targetFrame() const { return _targetFrame; }

private:
	uint16 execute(uint frame, uint commandStart) {
		_manager->_sceneCallbackFrame = frame;
		_manager->_activeIdleMediaCallback = this;
		_succeeded = _manager->executeCallback(_script, _callbackOffset,
			_result, &_nextFrame, commandStart);
		_manager->_activeIdleMediaCallback = nullptr;
		_manager->_sceneCallbackFrame = 0;
		if (_succeeded && _result == kCallbackSuspendedForText)
			return 0;
		_serviced = true;
		return 1;
	}
	ScriptManager *_manager;
	CompiledScript &_script;
	uint32 _callbackOffset;
	uint _targetFrame;
	uint _nextFrame;
	int _result;
	uint _resumeCommand;
	uint _requestSelector;
	uint _failureFrame;
	Common::String _expected;
	Common::String _entered;
	bool _started;
	bool _textPending;
	bool _serviced;
	bool _succeeded;
};

static Common::String compiledScriptMemberName(const Common::String &target) {
	// RunSceneScriptLoop at 0x124e9 replaces everything from the first dot with
	// ".run" before opening the compiled member. Script sources therefore pass
	// names such as DK1.SCR even though SCRIPT.PL stores the DK1 runtime.
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

ScriptManager::ScriptManager(RipperEngine *engine) : _engine(engine), _sceneCallbackFrame(0),
		_activeIdleMediaCallback(nullptr), _chooserTemplateMode(0),
		_briefing(new BriefingManager(engine)), _dialogue(new DialogueChooser()) {
}

bool ScriptManager::canSaveGame() const {
	return !_runtime.cyberActive && _runtime.awaitingInteraction && _runtime.pendingSceneMember.empty() &&
		_runtime.activeFrame < _runtime.activeScript.getFrames().size();
}

void ScriptManager::requestCyberExit(const char *source) {
	if (!_runtime.cyberActive)
		return;
	// CleanupCurrentSceneFrameInteractions at 0x13832 dispatches phase 3 to
	// HandleSceneEntryChoiceListLifecycle at 0x1523d before any nested-runtime
	// exit is returned. Retire the shared chooser here for Escape, toolbar, and
	// script exits alike so its saved display region cannot survive the unwind.
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

bool ScriptManager::startActiveFrame(uint frameIndex) {
	return advanceBa0ToFrame(frameIndex);
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

bool ScriptManager::syncGame(Common::Serializer &serializer) {
	Common::String ba0Member = serializer.isSaving() ? _runtime.activeScript.getMemberName() : Common::String();
	Common::String concurrentMember = serializer.isSaving() ? _runtime.concurrentScript.getMemberName() : Common::String();
	Common::String concurrentEntry = serializer.isSaving() ? _runtime.concurrentEntryLabel : Common::String();
	Common::String previousFrame = serializer.isSaving() ? _runtime.previousFrameLabel : Common::String();
	uint32 activeFrame = _runtime.activeFrame;
	uint16 frontEndActionMask = _runtime.frontEndActionMask;
	byte awaitingInteraction = _runtime.awaitingInteraction ? 1 : 0;
	byte briefingArmed = _briefing->isArmed() ? 1 : 0;
	uint32 briefingSelector = _briefing->getSelector();

	serializer.syncString(ba0Member);
	serializer.syncAsUint32LE(activeFrame);
	if (serializer.getVersion() >= 4)
		serializer.syncAsUint16LE(frontEndActionMask);
	serializer.syncString(previousFrame);
	serializer.syncString(concurrentMember);
	serializer.syncString(concurrentEntry);
	serializer.syncAsByte(awaitingInteraction);
	serializer.syncAsByte(briefingArmed);
	serializer.syncAsUint32LE(briefingSelector);
	if (serializer.isLoading() && (ba0Member.empty() || ba0Member.size() > 128 ||
			concurrentMember.size() > 128 || concurrentEntry.size() > 128 ||
			previousFrame.size() > 128))
		return false;

	if (!_engine->getMilestones()->syncGame(serializer))
		return false;

	uint32 playedSceneCount = _playedScenes.size();
	serializer.syncAsUint32LE(playedSceneCount);
	if (serializer.isLoading()) {
		if (playedSceneCount > 4096)
			return false;
		_playedScenes.clear();
		_playedScenes.resize(playedSceneCount);
	}
	for (uint i = 0; i < playedSceneCount; ++i) {
		serializer.syncString(_playedScenes[i]);
		if (serializer.isLoading() && _playedScenes[i].size() > 128)
			return false;
	}

	uint32 interactionCount = _runtime.activeInteractionEnabled.size();
	serializer.syncAsUint32LE(interactionCount);
	if (serializer.isLoading()) {
		if (interactionCount > 255)
			return false;
		_runtime.activeInteractionEnabled.clear();
		_runtime.activeInteractionEnabled.resize(interactionCount);
	}
	for (uint i = 0; i < interactionCount; ++i) {
		byte enabled = _runtime.activeInteractionEnabled[i] ? 1 : 0;
		serializer.syncAsByte(enabled);
		if (serializer.isLoading())
			_runtime.activeInteractionEnabled[i] = enabled != 0;
	}

	if (!_dialogue->syncGame(serializer) || serializer.err())
		return false;
	if (serializer.isSaving())
		return true;

	CompiledScript restoredBa0;
	if (!restoredBa0.load(_engine->getResources()->scripts(), ba0Member))
		return false;
	if (activeFrame >= restoredBa0.getFrames().size() ||
			interactionCount != restoredBa0.getFrames()[activeFrame].interactionCount)
		return false;

	CompiledScript restoredConcurrent;
	if (!concurrentMember.empty() &&
			!restoredConcurrent.load(_engine->getResources()->scripts(), concurrentMember))
		return false;

	_runtime.activeScript = Common::move(restoredBa0);
	_runtime.concurrentScript = Common::move(restoredConcurrent);
	_runtime.concurrentEntryLabel = concurrentEntry;
	_runtime.previousFrameLabel = previousFrame;
	_runtime.activeFrame = activeFrame;
	_runtime.frontEndActionMask = serializer.getVersion() >= 4 ? frontEndActionMask : 0xffff;
	_runtime.awaitingInteraction = awaitingInteraction != 0;
	if (!_briefing->restore(briefingArmed != 0, briefingSelector))
		return false;
	_runtime.pendingSceneMember.clear();
	_runtime.pendingSceneEntryLabel.clear();
	_runtime.clearPreservedAudioOnTransition = false;
	_runtime.hoveredInteraction = -1;
	_runtime.resumeLoadedPresentation = !_dialogue->isPending() &&
		_runtime.activeScript.getFrames()[_runtime.activeFrame].presentationType == 1;
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
	// DispatchFrontEndAction at 0x190b7 selects contextual help while
	// SceneRuntime+0x189 bit 0x20 is set. HandleSceneEntryChoiceListLifecycle
	// sets that bit for the choice list currently modeled by DialogueChooser.
	return _dialogue->isPending();
}

bool ScriptManager::showHelp(const char *source) {
	// PollInteractionAndResolveSelection at 0x13c8d and DispatchFrontEndAction
	// at 0x190b7 use the same prompt-state branch for F1 and toolbar Help.
	const bool promptActive = hasActivePrompt();
	const uint resourceId = _runtime.cyberActive ? kCyberHelpResource :
		(promptActive ? kPromptHelpResource : kGeneralHelpResource);
	debugC(1, kDebugScene,
		"Ripper: opening scene help source=%s resource=%u promptActive=%d cyberActive=%d",
		source, resourceId, promptActive, _runtime.cyberActive);
	_engine->getCursor()->setVisible(true);
	// PollInteractionAndResolveSelection at 0x13c8d sends Cyber help resource
	// 0x1a4 through RunModalTextDialog without replacing the active palette.
	// The MENUB pixels therefore retain the colors of the Cyber presentation.
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

bool ScriptManager::initialize(ResourceManager &resources) {
	return _briefing->initialize(resources) &&
		_dialogue->initialize(resources) &&
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
		_runtime.pendingSceneMember = targetScript;
		_runtime.pendingSceneEntryLabel.clear();
		_runtime.clearPreservedAudioOnTransition = true;
		debugC(2, kDebugScene,
			"Ripper: queued world map scene transition target='%s'",
			_runtime.pendingSceneMember.c_str());
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

	// HandleSceneSelectionAction at 0x191e2 records the selected destination,
	// then RunFrontEndActionMenu at 0x18b3a runs WMAP*.RUN before entering it.
	// The shipped checkpoint scripts perform their work in the sole frame's
	// entry callback; the exit callback only terminates that nested runtime.
	const ScriptFrame &frame = checkpoint.getFrames()[0];
	int result = 0;
	if (!executeCallback(checkpoint, frame.enterCallbackOffset, result) ||
			result != 0) {
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
	Common::String value;
	for (uint i = 0; i < argument.data.size() && argument.data[i] != 0; ++i)
		value += (char)argument.data[i];
	return value;
}

bool ScriptManager::textAnswersMatch(const Common::String &entered,
		const Common::String &expected) {
	uint enteredIndex = 0;
	uint expectedIndex = 0;
	while (true) {
		while (enteredIndex < entered.size() &&
				!Common::isAlnum((byte)entered[enteredIndex]))
			++enteredIndex;
		while (expectedIndex < expected.size() &&
				!Common::isAlnum((byte)expected[expectedIndex]))
			++expectedIndex;
		if (enteredIndex == entered.size() || expectedIndex == expected.size())
			break;
		byte enteredCharacter = (byte)entered[enteredIndex++];
		byte expectedCharacter = (byte)expected[expectedIndex++];
		if (enteredCharacter >= 'A' && enteredCharacter <= 'Z')
			enteredCharacter += 'a' - 'A';
		if (expectedCharacter >= 'A' && expectedCharacter <= 'Z')
			expectedCharacter += 'a' - 'A';
		if (enteredCharacter != expectedCharacter)
			return false;
	}
	while (enteredIndex < entered.size() &&
			!Common::isAlnum((byte)entered[enteredIndex]))
		++enteredIndex;
	while (expectedIndex < expected.size() &&
			!Common::isAlnum((byte)expected[expectedIndex]))
		++expectedIndex;
	return enteredIndex == entered.size() && expectedIndex == expected.size();
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

uint ScriptManager::resolveFrameIndex(const CompiledScript &script,
		const Common::String &label) const {
	if (label.empty())
		return 0;

	for (uint i = 0; i < script.getFrames().size(); ++i) {
		if (script.getString(script.getFrames()[i].labelOffset).equalsIgnoreCase(label)) {
			return i;
		}
	}

	// FindSceneFrameIndexByLabel at 0x1464d returns frame 0 when no label
	// matches. HA2.RUN relies on this for its "hcw1in" handoff to HC2.RUN,
	// whose first frame is labeled "HCW1CIN".
	const Common::String fallbackLabel = script.getFrames().empty() ?
		Common::String() : script.getString(script.getFrames()[0].labelOffset);
	debugC(2, kDebugScene,
		"Ripper: scene script='%s' entry='%s' not found, using retail fallback "
		"frame=0 label='%s'",
		script.getMemberName().c_str(), label.c_str(), fallbackLabel.c_str());
	return 0;
}

void ScriptManager::beginBa0InteractionWait(const Common::String &frameLabel,
		uint interactionCount) {
	// ExecuteSceneFrameAndInteractions at 0x13277 services the new controls once after
	// frame media, discarding that result before it starts the blocking chooser loop.
	if (_engine->getInput()->pollEvents())
		_engine->quitGame();
	_engine->getInput()->discardMouseTransitions();
	_runtime.awaitingInteraction = true;
	debugC(1, kDebugScene,
		"Ripper: active scene script='%s' frame='%s' awaiting %u interactions",
		_runtime.activeScript.getMemberName().c_str(), frameLabel.c_str(), interactionCount);
}

void ScriptManager::initializeBa0InteractionState(const ScriptFrame &frame) {
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

void ScriptManager::bindBa0Frame(uint frameIndex) {
	// BindSceneRuntimeCurrentFrame at 0x145d6 preserves the outgoing frame label
	// at SceneRuntime+0x177 before changing the current frame record.
	if (_runtime.activeFrame < _runtime.activeScript.getFrames().size())
		_runtime.previousFrameLabel = _runtime.activeScript.getString(_runtime.activeScript.getFrames()[_runtime.activeFrame].labelOffset);
	else
		_runtime.previousFrameLabel.clear();
	_runtime.activeFrame = frameIndex;
	debugC(2, kDebugScripts,
		"Ripper: bound active scene script='%s' frame=%u previous='%s'",
		_runtime.activeScript.getMemberName().c_str(), _runtime.activeFrame, _runtime.previousFrameLabel.c_str());
}

bool ScriptManager::executeCallback(CompiledScript &script, uint32 callbackOffset, int &result,
		uint *nextFrame, uint commandStart) {
	Common::Array<ScriptCommand> commands;
	result = 0;
	if (nextFrame)
		*nextFrame = 0;
	if (!script.decodeCallback(callbackOffset, true, commands))
		return false;
	debugC(2, kDebugScripts,
		"Ripper: decoded callback script='%s' offset=0x%x commands=%u",
		script.getMemberName().c_str(), callbackOffset, commands.size());

	uint commandIndex = commandStart;
	bool branchTaken = false;
	while (commandIndex < commands.size()) {
		const ScriptCommand &command = commands[commandIndex];
		debugC(2, kDebugScripts,
			"Ripper: execute script='%s' offset=0x%x opcode=0x%02x name='%s' selector=%u arguments=%u",
			script.getMemberName().c_str(), command.offset, command.opcode,
			scriptOpcodeName(command.opcode), command.selector, command.arguments.size());

		switch (command.opcode) {
		case kMilestoneCondition:
		case kScenePlayedCondition: {
			if (command.arguments.size() < 3)
				return false;
			const bool expected = command.arguments[0].value != 0;
			bool actual = false;
			if (command.opcode == kMilestoneCondition) {
				const uint flag = command.arguments[1].value;
				actual = _engine->getMilestones()->isSet(flag);
				// The numeric fields uniquely identify this gate and avoid expanding
				// mutable script or label storage in this high-frequency variadic trace.
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
				uint targetIndex = 0;
				while (targetIndex < commands.size() &&
					commands[targetIndex].offset != command.arguments[2].value)
					++targetIndex;
				if (targetIndex == commands.size()) {
					warning("Ripper: callback branch target 0x%x is not a command boundary in '%s'",
						command.arguments[2].value, script.getMemberName().c_str());
					return false;
				}
				commandIndex = targetIndex;
				continue;
			}
			break;
		}

		case kDialogueChoiceCondition: {
			if (command.arguments.size() < 2)
				return false;
			const bool choicesAvailable = _dialogue->hasChoices();
			const bool takeBranch = command.arguments[0].value == 0 ?
				choicesAvailable : !choicesAvailable;
			debugC(2, kDebugDialogue,
				"Ripper: dialogue availability condition choices=%u invert=%u target=0x%x selected=%d",
				choicesAvailable, command.arguments[0].value != 0,
				command.arguments[1].value, takeBranch);
			if (takeBranch) {
				branchTaken = true;
				uint targetIndex = 0;
				while (targetIndex < commands.size() &&
					commands[targetIndex].offset != command.arguments[1].value)
					++targetIndex;
				if (targetIndex == commands.size()) {
					warning("Ripper: dialogue branch target 0x%x is not a command boundary in '%s'",
						command.arguments[1].value, script.getMemberName().c_str());
					return false;
				}
				commandIndex = targetIndex;
				continue;
			}
			break;
		}

		case kPreviousSceneCondition: {
			if (command.arguments.size() < 3 ||
				command.arguments[1].value >= script.getFrames().size())
				return false;
			if (&script != &_runtime.activeScript) {
				warning("Ripper: previous-scene condition has no tracked runtime in '%s' at 0x%x",
					script.getMemberName().c_str(), command.offset);
				return false;
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
				uint targetIndex = 0;
				while (targetIndex < commands.size() &&
					commands[targetIndex].offset != command.arguments[2].value)
					++targetIndex;
				if (targetIndex == commands.size()) {
					warning("Ripper: callback branch target 0x%x is not a command boundary in '%s'",
						command.arguments[2].value, script.getMemberName().c_str());
					return false;
				}
				commandIndex = targetIndex;
				continue;
			}
			break;
		}

		case kDefaultBranch: {
			if (command.arguments.size() < 1)
				return false;
			const bool useDefaultBranch = !branchTaken;
			branchTaken = false;
			debugC(3, kDebugScripts, "Ripper: default branch target=0x%x selected=%d",
				command.arguments[0].value, useDefaultBranch);
			if (useDefaultBranch) {
				uint targetIndex = 0;
				while (targetIndex < commands.size() &&
					commands[targetIndex].offset != command.arguments[0].value)
					++targetIndex;
				if (targetIndex == commands.size()) {
					warning("Ripper: callback branch target 0x%x is not a command boundary in '%s'",
						command.arguments[0].value, script.getMemberName().c_str());
					return false;
				}
				commandIndex = targetIndex;
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
				return false;
			const bool value = command.opcode == kSetMilestoneFlag;
			if (!_engine->getMilestones()->set(command.arguments[0].value, value,
				value ? "set milestone flag" : "clear milestone flag"))
				return false;
			break;
		}

		case kDisableInteraction: {
			if (command.arguments.size() < 1)
				return false;
			if (&script != &_runtime.activeScript || _runtime.activeFrame >= _runtime.activeScript.getFrames().size()) {
				warning("Ripper: interaction disable opcode has no active scene frame in '%s' at 0x%x",
					script.getMemberName().c_str(), command.offset);
				return false;
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
			if (command.arguments.size() < 5)
				return false;
			const Common::String mediaPath = script.getString(command.arguments[0].value);
			const bool allowEscSpace = command.arguments[2].value == 0;
			// ExecutePresentationEntry at 0x1652a deactivates the UI selection
			// presentation before media playback. The following frame activation
			// restores the cursor after the callback finishes.
			_engine->getCursor()->setVisible(false);
			if (!_engine->getMedia()->play(mediaPath, allowEscSpace,
				(int32)command.arguments[3].value, (int32)command.arguments[4].value, true))
				return false;
			// HandleSceneEntryMediaAndSetBasenameFlag at 0x159e1 marks the
			// presentation basename only after ExecutePresentationEntry returns.
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
					return false;
				const ScriptFrame &responseFrame = _runtime.activeScript.getFrames()[command.arguments[1].value];
				const Common::String responseLabel = _runtime.activeScript.getString(responseFrame.labelOffset);
				includeChoice = !isScenePlayed(responseLabel);
				debugC(2, kDebugDialogue,
					"Ripper: dialogue response frame=%u label='%s' played=%d",
					command.arguments[1].value, responseLabel.c_str(), !includeChoice);
			}
			if (!_dialogue->execute(script, command, includeChoice))
				return false;
			if (command.opcode == kStartDialogue)
				debugC(2, kDebugAudio,
					"Ripper: dialogue chooser retained scene audio active=%d",
					_engine->getSceneAudio()->isActive());
			break;
		}

		case kStartSceneRuntime: {
			if (command.arguments.size() < 3)
				return false;
			const Common::String target = argumentString(command.arguments[0]);
			const Common::String entryLabel = argumentString(command.arguments[1]);
			// HandleSceneEntryAndStartConcurrentSceneRuntime at 0x15cd3 always
			// turns opcode 0x1d into a scene handoff when the command belongs to
			// the concurrent runtime. The third argument only selects this path
			// when the active scene issues the command.
			if (&script == &_runtime.concurrentScript || command.arguments[2].value == 0) {
				_runtime.pendingSceneMember = compiledScriptMemberName(target);
				_runtime.pendingSceneEntryLabel = entryLabel;
				if (&script == &_runtime.concurrentScript)
					_runtime.clearPreservedAudioOnTransition = true;
				debugC(1, kDebugScene,
					"Ripper: queued script transition target='%s' entry='%s' source='%s'",
					_runtime.pendingSceneMember.c_str(), _runtime.pendingSceneEntryLabel.c_str(),
					script.getMemberName().c_str());
				result = -3;
				return true;
			}
			const Common::String memberName = compiledScriptMemberName(target);
			if (!_runtime.concurrentScript.load(_engine->getResources()->scripts(), memberName))
				return false;
			_runtime.concurrentEntryLabel = entryLabel;
			debugC(1, kDebugScene, "Ripper: created concurrent script='%s' entry='%s'",
				target.c_str(), entryLabel.c_str());
			break;
		}

		case kSetNamedFlag: {
			if (command.arguments.size() < 2)
				return false;
			const bool played = command.arguments[0].value != 0;
			const Common::String scene = argumentString(command.arguments[1]);
			if (scene.empty())
				return false;
			const bool previous = isScenePlayed(scene);
			// HandleSceneEntrySetOrClearNamedFlag at 0x15dfe forwards the
			// first argument and named play-list entry to the shared bitset
			// helper; zero clears the bit and nonzero sets it.
			setScenePlayed(scene, played);
			debugC(2, kDebugScene,
				"Ripper: updated named scene flag scene='%s' value=%d previous=%d script='%s' offset=0x%x",
				scene.c_str(), played, previous, script.getMemberName().c_str(), command.offset);
			break;
		}

		case kLoadAudio: {
			if (command.arguments.size() < 2)
				return false;
			const Common::String audioPath = script.getString(command.arguments[0].value);
			const bool preserve = (command.arguments[1].value & 1) != 0;
			if (!_engine->getSceneAudio()->load(audioPath, preserve))
				return false;
			break;
		}

		case kConfigureAudio: {
			if (command.arguments.size() < 4)
				return false;
			const Common::String key = argumentString(command.arguments[0]);
			const uint volume = command.arguments[1].value == 0 ? 100 : command.arguments[1].value & 0xff;
			const uint trigger = command.arguments[2].value & 0xffff;
			const uint control = command.arguments[3].value & 0xff;
			if (!_engine->getSceneAudio()->configure(key, volume, trigger, control))
				return false;
			break;
		}

		case kClearAudio:
		case kStopAudio: {
			if (command.arguments.size() < 1)
				return false;
			const Common::String key = argumentString(command.arguments[0]);
			if (command.opcode == kClearAudio)
				_engine->getSceneAudio()->clear(key);
			else
				_engine->getSceneAudio()->stop(key);
			break;
		}

		case kSetAudioVolume: {
			if (command.arguments.size() < 4)
				return false;
			const Common::String key = argumentString(command.arguments[0]);
			_engine->getSceneAudio()->setVolume(key,
				command.arguments[1].value & 0xff,
				command.arguments[2].value & 0xffff,
				command.arguments[3].value & 0xffff);
			break;
		}

		case kWaitForFrameCounter: {
			if (command.arguments.size() < 1)
				return false;
			const uint targetFrame = command.arguments[0].value;
			// HandleSceneEntryWaitForSceneFrameCounter at 0x1633e suspends the
			// idle command stream until RunMediaSequence publishes this one-based
			// frame. IdleMediaCallback enters the stream only after that boundary.
			if (_sceneCallbackFrame < targetFrame) {
				warning("Ripper: scene callback frame=%u did not reach wait target=%u script='%s' offset=0x%x",
					_sceneCallbackFrame, targetFrame, script.getMemberName().c_str(),
					command.offset);
				return false;
			}
			debugC(2, kDebugScene,
				"Ripper: scene idle callback reached media frame=%u target=%u script='%s' offset=0x%x",
				_sceneCallbackFrame, targetFrame, script.getMemberName().c_str(),
				command.offset);
			break;
		}

		case kRequestTextInput: {
			if (command.arguments.size() < 4 || !nextFrame ||
					!_activeIdleMediaCallback)
				return false;
			const Common::String prompt = argumentString(command.arguments[0]);
			const Common::String expected = argumentString(command.arguments[1]);
			const uint failureFrame = command.arguments[2].value;
			const uint layoutVariant = command.arguments[3].value;
			if (layoutVariant != 2) {
				warning("Ripper: unsupported scene text request layout=%u script='%s' offset=0x%x",
					layoutVariant, script.getMemberName().c_str(), command.offset);
				return false;
			}

			// DispatchSceneEntryAction 30 captures the active presentation as the
			// following chooser's one-shot template. ConfigureSceneEntryChooserLayout
			// at 0x18740 maps layout 2 to the unskinned primary chooser template at
			// 0x8a2de, retaining the edit-field surround from the active movie.
			const byte templateMode = _chooserTemplateMode;
			_chooserTemplateMode = 0;
			if (!_activeIdleMediaCallback->beginTextRequest(prompt, expected,
					failureFrame, command.selector, commandIndex + 1))
				return false;
			debugC(2, kDebugCyber,
				"Ripper: suspended scene callback for text request script='%s' selector=%u layout=%u templateMode=%u resumeCommand=%u",
				script.getMemberName().c_str(), command.selector, layoutVariant,
				templateMode, commandIndex + 1);
			result = kCallbackSuspendedForText;
			return true;
		}

		case kPreviewMedia: {
			if (command.arguments.size() < 4)
				return false;
			const Common::String mediaPath = script.getString(command.arguments[0].value);
			if (!_engine->getMedia()->playScene(mediaPath, (int32)command.arguments[1].value,
				(int32)command.arguments[2].value, true))
				return false;
			debugC(2, kDebugScripts,
				"Ripper: retained first frame for scene media command '%s'",
				mediaPath.c_str());
			break;
		}

		case kSelectFrame:
			if (command.arguments.size() < 1)
				return false;
			if (nextFrame)
				*nextFrame = command.arguments[0].value;
			result = -2;
			debugC(2, kDebugScripts, "Ripper: callback selected frame=%u control=%d",
				command.arguments[0].value, result);
			return true;

		case kDispatchSceneAction: {
			bool stopCallback = false;
			if (!SceneActionDispatcher::dispatch(*this, script, command, result, stopCallback))
				return false;
			if (stopCallback)
				return true;
			break;
		}

		case kFadePalette: {
			if (command.arguments.size() < 2)
				return false;
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
			return false;
		}
		++commandIndex;
	}
	return true;
}

bool ScriptManager::acceptCyberRuntimeExit(int result, const CompiledScript &script,
		const char *callbackPhase) const {
	if (result != -4 || !_runtime.cyberActive || !_runtime.cyberExitRequested)
		return false;

	// DispatchSceneEntryAction at 0x36892 returns -4 for action 9999, and
	// RunSceneScriptLoop at 0x124e9 treats it as a normal nested-runtime exit
	// regardless of which active-frame callback produced it.
	debugC(1, kDebugCyber,
		"Ripper: accepted Cyber runtime exit script='%s' frame=%u callback='%s' result=%d",
		script.getMemberName().c_str(), _runtime.activeFrame, callbackPhase, result);
	return true;
}

bool ScriptManager::runStartupPath() {
	if (_startup.getFrames().empty() || _runtime.activeScript.getFrames().empty())
		return false;

	int result = 0;
	const ScriptFrame &startupFrame = _startup.getFrames()[0];
	if (!executeCallback(_startup, startupFrame.exitCallbackOffset, result) || result != -3) {
		warning("Ripper: startup script did not request the BA0 transition (result=%d)", result);
		return false;
	}

	uint ba0StartFrame = 0xffffffff;
	for (uint i = 0; i < _runtime.activeScript.getFrames().size(); ++i) {
		if (_runtime.activeScript.getString(_runtime.activeScript.getFrames()[i].labelOffset).equalsIgnoreCase("start")) {
			ba0StartFrame = i;
			break;
		}
	}
	if (ba0StartFrame == 0xffffffff) {
		warning("Ripper: BA0 does not contain the start frame");
		return false;
	}

	// InitializeSceneDisplayModeAndContext at 0x1e28a switches to a newly cleared scene page.
	// ScummVM retains one framebuffer across presentations, so clear the previous cinematic here.
	g_system->fillScreen(0);
	presentScreen();
	debugC(1, kDebugScene, "Ripper: initialized cleared scene display for BA0");

	debugC(1, kDebugScene, "Ripper: entering BA0 frame=%u label='start'", ba0StartFrame);
	bindBa0Frame(ba0StartFrame);
	initializeBa0InteractionState(_runtime.activeScript.getFrames()[ba0StartFrame]);
	result = 0;
	if (!executeCallback(_runtime.activeScript, _runtime.activeScript.getFrames()[ba0StartFrame].enterCallbackOffset, result) || result != 0)
		return false;
	beginBa0InteractionWait("start", _runtime.activeScript.getFrames()[ba0StartFrame].interactionCount);
	return true;
}

bool ScriptManager::executeConcurrentFrame() {
	if (_runtime.concurrentScript.getFrames().empty())
		return true;

	const uint frameIndex =
		resolveFrameIndex(_runtime.concurrentScript, _runtime.concurrentEntryLabel);

	const ScriptFrame &frame = _runtime.concurrentScript.getFrames()[frameIndex];
	debugC(2, kDebugScene, "Ripper: servicing concurrent script='%s' frame=%u label='%s'",
		_runtime.concurrentScript.getMemberName().c_str(), frameIndex, _runtime.concurrentEntryLabel.c_str());
	int result = 0;
	uint nextFrame = frameIndex;
	if (!executeCallback(_runtime.concurrentScript, frame.enterCallbackOffset, result, &nextFrame))
		return false;
	if (result == -3 && !_runtime.pendingSceneMember.empty()) {
		debugC(1, kDebugScene,
			"Ripper: concurrent script requested transition target='%s' entry='%s'",
			_runtime.pendingSceneMember.c_str(), _runtime.pendingSceneEntryLabel.c_str());
		_runtime.concurrentScript = CompiledScript();
		_runtime.concurrentEntryLabel.clear();
		return true;
	}
	if (result != 0)
		return false;
	if (!executeCallback(_runtime.concurrentScript, frame.exitCallbackOffset, result, &nextFrame))
		return false;
	if (result == -3 && !_runtime.pendingSceneMember.empty()) {
		debugC(1, kDebugScene,
			"Ripper: concurrent script requested transition target='%s' entry='%s'",
			_runtime.pendingSceneMember.c_str(), _runtime.pendingSceneEntryLabel.c_str());
		_runtime.concurrentScript = CompiledScript();
		_runtime.concurrentEntryLabel.clear();
		return true;
	}
	if (result != -2) {
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
	// RunSceneScriptLoop at 0x124e9 retires non-preserved slots at every scene
	// handoff. Concurrent and world-map handoffs clear preserve bits first.
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
	return advanceBa0ToFrame(startFrame);
}

bool ScriptManager::advanceBa0ToFrame(uint nextFrame) {
	_engine->getCursor()->setVisible(false);
	_runtime.hoveredInteraction = -1;
	// RunSceneScriptLoop at 0x124e9 has no frame-count bound here. A -2 result
	// re-enters the loop so the concurrent runtime is serviced before the next
	// active frame, even when opcode 0x14 selected that same frame again.
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

		bindBa0Frame(nextFrame);
		const ScriptFrame &frame = _runtime.activeScript.getFrames()[_runtime.activeFrame];
		const Common::String label = _runtime.activeScript.getString(frame.labelOffset);
		debugC(1, kDebugScene,
			"Ripper: entering active scene script='%s' frame=%u label='%s' type=%u interactions=%u",
			_runtime.activeScript.getMemberName().c_str(), _runtime.activeFrame, label.c_str(),
			frame.presentationType, frame.interactionCount);
		initializeBa0InteractionState(frame);

		int result = 0;
		uint callbackFrame = _runtime.activeFrame;
		if (!executeCallback(_runtime.activeScript, frame.enterCallbackOffset, result, &callbackFrame))
			return false;
		if (acceptCyberRuntimeExit(result, _runtime.activeScript, "enter"))
			return true;
		if (result == -3 && !_runtime.pendingSceneMember.empty())
			return performPendingSceneTransition();
		if (result == -2) {
			_engine->getSceneAudio()->resetTriggers();
			nextFrame = callbackFrame;
			continue;
		}
		if (result != 0)
			return false;

		if (frame.presentationType == 0 || frame.presentationType == 1) {
			const Common::String mediaPath = _runtime.activeScript.getString(frame.mediaNameOffset);
			debugC(2, kDebugScene,
				"Ripper: frame presentation label='%s' media='%s' origin=%d,%d",
				label.c_str(), mediaPath.c_str(), frame.x, frame.y);
			markScenePlayed(label);
			if (frame.interactionCount != 0)
				beginBa0InteractionWait(label, frame.interactionCount);
			const bool dialogueIdleCallback =
				frame.idleCallbackOffset != 0 && _dialogue->hasChoices();
			if (dialogueIdleCallback) {
				int idleResult = 0;
				if (!executeCallback(_runtime.activeScript, frame.idleCallbackOffset, idleResult))
					return false;
				if (acceptCyberRuntimeExit(idleResult, _runtime.activeScript, "idle"))
					return true;
				if (idleResult != 0) {
					warning("Ripper: dialogue idle callback for frame '%s' returned %d",
						label.c_str(), idleResult);
					return false;
				}
				debugC(1, kDebugDialogue,
					"Ripper: activated frame dialogue chooser before media label='%s' idle=0x%x",
					label.c_str(), frame.idleCallbackOffset);
			}

			uint idleWaitFrame = 0;
			bool idleTextRequest = false;
			if (frame.idleCallbackOffset != 0 && !_dialogue->hasChoices()) {
				Common::Array<ScriptCommand> idleCommands;
				if (!_runtime.activeScript.decodeCallback(frame.idleCallbackOffset, true, idleCommands))
					return false;
				for (uint commandIndex = 0; commandIndex < idleCommands.size(); ++commandIndex) {
					const ScriptCommand &idleCommand = idleCommands[commandIndex];
					if (idleCommand.opcode == kWaitForFrameCounter &&
							!idleCommand.arguments.empty())
						idleWaitFrame = idleCommand.arguments[0].value;
					else if (idleCommand.opcode == kRequestTextInput)
						idleTextRequest = true;
				}
				if (idleTextRequest && idleWaitFrame == 0) {
					warning("Ripper: scene text request frame='%s' has no media-frame wait",
						label.c_str());
					return false;
				}
			}
			const bool dialoguePending = _dialogue->isPending();
			const bool waitsForSceneInput =
				frame.interactionCount != 0 || dialoguePending || idleTextRequest;
			if (waitsForSceneInput && !_runtime.awaitingInteraction)
				beginBa0InteractionWait(label, frame.interactionCount);
			// ExecuteSceneFrameAndInteractions at 0x13277 enters
			// PollInteractionAndResolveSelection at 0x13c8d when either scene
			// controls or the frame's idle callback are active. A dialogue
			// chooser created by that callback therefore holds the type-1
			// presentation instead of falling through to the exit callback.
			const bool loopUntilInput = frame.presentationType == 1 &&
				(frame.interactionCount != 0 || dialoguePending);
			const bool allowEscSpace = frame.presentationType == 0;
			debugC(2, kDebugVideo,
				"Ripper: frame presentation controls label='%s' keyboard=%d mouse=%d dialogue=%d "
				"idleTextRequest=%d waitFrame=%u",
				label.c_str(), allowEscSpace, loopUntilInput, dialoguePending,
				idleTextRequest, idleWaitFrame);
			IdleMediaCallback idleCallback(this, _runtime.activeScript, frame.idleCallbackOffset,
				idleWaitFrame, _runtime.activeFrame);
			uint16 idleCommand = 0;
			if (!_engine->getMedia()->playScene(mediaPath, frame.x, frame.y, false,
				loopUntilInput, allowEscSpace,
				idleTextRequest ? &idleCallback : nullptr, &idleCommand))
				return false;
			if (_runtime.cyberKeyboardCommand == kCyberEscapeCommand) {
				if (!serviceCyberKeyboardCommand())
					return false;
				return true;
			}
			if (idleTextRequest) {
				if (!idleCallback.serviced()) {
					warning("Ripper: scene text request frame='%s' media='%s' ended before target=%u",
						label.c_str(), mediaPath.c_str(), idleCallback.targetFrame());
					return false;
				}
				if (!idleCallback.succeeded())
					return false;
				result = idleCallback.result();
				callbackFrame = idleCallback.nextFrame();
				debugC(2, kDebugScene,
					"Ripper: scene text idle callback completed frame='%s' result=%d nextFrame=%u mediaCommand=0x%04x",
					label.c_str(), result, callbackFrame, idleCommand);
				if (acceptCyberRuntimeExit(result, _runtime.activeScript, "media idle"))
					return true;
				if (result == -2) {
					_runtime.awaitingInteraction = false;
					_engine->getSceneAudio()->resetTriggers();
					nextFrame = callbackFrame;
					continue;
				}
				if (result != 0)
					return false;
			}
			if (frame.idleCallbackOffset != 0 && !dialogueIdleCallback &&
					!idleTextRequest) {
				// PollInteractionAndResolveSelection at 0x13c8d calls
				// StepFrameIdleCallbackCommandStream at 0x143af before reading
				// chooser input. Type-0 frames reach that poll after their media
				// completes. EE2.RUN frame EEZ1 relies on this ordering to play
				// Q2_V5.WAV once before its unzoom hotspot becomes interactive.
				result = 0;
				callbackFrame = _runtime.activeFrame;
				if (!executeCallback(_runtime.activeScript,
						frame.idleCallbackOffset, result, &callbackFrame))
					return false;
				debugC(2, kDebugScene,
					"Ripper: serviced frame idle callback before interaction "
					"frame='%s' idle=0x%x result=%d nextFrame=%u",
					label.c_str(), frame.idleCallbackOffset, result, callbackFrame);
				if (acceptCyberRuntimeExit(result, _runtime.activeScript,
						"interaction idle"))
					return true;
				if (result == -3 && !_runtime.pendingSceneMember.empty())
					return performPendingSceneTransition();
				if (result == -2) {
					_runtime.awaitingInteraction = false;
					_engine->getSceneAudio()->resetTriggers();
					nextFrame = callbackFrame;
					continue;
				}
				if (result != 0)
					return false;
			}
			if (!_runtime.pendingSceneMember.empty()) {
				debugC(1, kDebugScene,
					"Ripper: scene presentation returned transition target='%s' entry='%s'",
					_runtime.pendingSceneMember.c_str(), _runtime.pendingSceneEntryLabel.c_str());
				return performPendingSceneTransition();
			}
			// ExecutePresentationEntry at 0x1652a routes .AVI through
			// RunMediaPresentation at 0x168af, whose controlled path restores
			// the surrounding display. Other extensions enter RunMediaSequence
			// at 0x1e516 and retain their final frame, including full-screen
			// document presentations such as KK_Z12.SMK.
			if (allowEscSpace && mediaPath.hasSuffixIgnoreCase(".avi")) {
				_dialogue->rebuildPresentationBands("controlled-avi-complete");
			} else if (allowEscSpace) {
				debugC(2, kDebugScene,
					"Ripper: retained controlled sequence display media='%s' "
					"route=RunMediaSequence@0x1e516",
					mediaPath.c_str());
			}
			if (waitsForSceneInput)
				return true;
		}

		if (frame.interactionCount != 0 && !_runtime.awaitingInteraction) {
			beginBa0InteractionWait(label, frame.interactionCount);
			return true;
		}

		callbackFrame = _runtime.activeFrame;
		if (!executeCallback(_runtime.activeScript, frame.exitCallbackOffset, result, &callbackFrame))
			return false;
		if (acceptCyberRuntimeExit(result, _runtime.activeScript, "exit"))
			return true;
		if (result == -3 && !_runtime.pendingSceneMember.empty())
			return performPendingSceneTransition();
		if (result != -2) {
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

bool ScriptManager::captureCyberKeyboardCommand() {
	if (!_runtime.cyberActive || !_runtime.awaitingInteraction ||
			!_engine->getInput()->hasPendingKey())
		return false;
	const uint16 command = _engine->getInput()->peekKey();
	if (command != kCyberLeftCommand && command != kCyberRightCommand &&
			command != kCyberChooseCommand && command != kCyberEscapeCommand)
		return false;
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
		// HandleSceneChooserSpecialCommand at 0x17c5a turns Escape into the
		// normal -4 nested-runtime exit before resolving frame hotspots.
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

	int result = 0;
	uint nextFrame = _runtime.activeFrame;
	debugC(2, kDebugCyber,
		"Ripper: Cyber keyboard command=0x%04x interaction=%u label='%s' callback=0x%x",
		command, interactionIndex, interaction->label.c_str(), interaction->callbackOffset);
	if (!executeCallback(_runtime.activeScript, interaction->callbackOffset, result, &nextFrame))
		return false;
	_runtime.awaitingInteraction = false;
	_engine->getCursor()->setVisible(false);
	_runtime.hoveredInteraction = -1;
	if (acceptCyberRuntimeExit(result, _runtime.activeScript, "keyboard interaction"))
		return true;
	if (result != -2) {
		warning("Ripper: Cyber keyboard interaction=%u returned unexpected result %d",
			interactionIndex, result);
		return false;
	}
	_engine->getSceneAudio()->resetTriggers();
	return advanceBa0ToFrame(nextFrame);
}

bool ScriptManager::serviceScene() {
	if (_runtime.resumeLoadedPresentation) {
		_runtime.resumeLoadedPresentation = false;
		const ScriptFrame &frame = _runtime.activeScript.getFrames()[_runtime.activeFrame];
		const Common::String label = _runtime.activeScript.getString(frame.labelOffset);
		const Common::String mediaPath = _runtime.activeScript.getString(frame.mediaNameOffset);
		debugC(1, kDebugSaveLoad,
			"Ripper: resuming loaded interactive presentation frame=%u label='%s' media='%s'",
			_runtime.activeFrame, label.c_str(), mediaPath.c_str());
		if (!_engine->getMedia()->playScene(mediaPath, frame.x, frame.y, false, true, false))
			return false;
		if (!_runtime.pendingSceneMember.empty())
			return performPendingSceneTransition();
	}
	const MouseState mouse = _engine->getInput()->publishMouseState();
	if (_runtime.cyberExitRequested)
		return true;
	captureCyberKeyboardCommand();
	if (_runtime.cyberKeyboardCommand != 0)
		return serviceCyberKeyboardCommand();
	if (_engine->getInput()->peekKey() == kHelpCommand) {
		_engine->getInput()->consumeKey();
		if (!showHelp("scene"))
			return false;
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
				return advanceBa0ToFrame(dialogueFrame);
			}
		}
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
			return advanceBa0ToFrame(dialogueFrame);
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

	const ScriptFrame &frame = _runtime.activeScript.getFrames()[_runtime.activeFrame];
	uint hoveredInteractionIndex = 0;
	const ScriptInteraction *hoveredInteraction =
		findBa0Interaction(mouse.position, &hoveredInteractionIndex);

	const uint cursorIndex = hoveredInteraction ?
		(hoveredInteraction->conditionOffset != 0 ? 8 : hoveredInteraction->initialSelection) :
		(dialoguePending && _dialogue->contains(mouse.position) ? kDialogueCursor :
			_engine->getCursor()->getSelectionIndex());
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
		debugC(1, kDebugInput,
			"Ripper: hotspot click frame=%u label='%s' interaction=%u action='%s' "
				"point=%d,%d rect=%d,%d,%d,%d cursor=%u condition=0x%x callback=0x%x flags=0x%02x",
			_runtime.activeFrame, _runtime.activeScript.getString(frame.labelOffset).c_str(), hoveredInteractionIndex,
			hoveredInteraction->label.c_str(), mouse.position.x, mouse.position.y,
			bounds.left, bounds.top, bounds.width(), bounds.height(), cursorIndex,
			hoveredInteraction->conditionOffset,
			hoveredInteraction->callbackOffset, hoveredInteraction->flags);
		debugC(2, kDebugScene,
			"Ripper: active scene chooser selected script='%s' interaction=%u label='%s' point=%d,%d rect=%d,%d,%d,%d",
			_runtime.activeScript.getMemberName().c_str(), hoveredInteractionIndex,
			hoveredInteraction->label.c_str(), mouse.position.x, mouse.position.y,
			bounds.left, bounds.top, bounds.width(), bounds.height());
		int result = 0;
		uint nextFrame = _runtime.activeFrame;
		if (!executeCallback(_runtime.activeScript, hoveredInteraction->callbackOffset, result, &nextFrame))
			return false;
		debugC(1, kDebugScripts,
			"Ripper: hotspot action script='%s' callback=0x%x result=%d nextFrame=%u",
			_runtime.activeScript.getMemberName().c_str(), hoveredInteraction->callbackOffset, result, nextFrame);
		if (result == -3 && !_runtime.pendingSceneMember.empty()) {
			_runtime.awaitingInteraction = false;
			_engine->getCursor()->setVisible(false);
			_runtime.hoveredInteraction = -1;
			return performPendingSceneTransition();
		}
		if (acceptCyberRuntimeExit(result, _runtime.activeScript, "pointer interaction")) {
			_runtime.awaitingInteraction = false;
			_engine->getCursor()->setVisible(false);
			_runtime.hoveredInteraction = -1;
			return true;
		}
		if (result != -2) {
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
		return advanceBa0ToFrame(nextFrame);
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
		const ScriptInteraction *interaction = findBa0Interaction(point);
		const uint cursorIndex = _dialogue->contains(point) ? kDialogueCursor :
			(interaction ? (interaction->conditionOffset != 0 ? 8 : interaction->initialSelection) :
				_engine->getCursor()->getSelectionIndex());
		_engine->getCursor()->setVisible(true);
		_engine->getCursor()->update(cursorIndex);
		return false;
	}
	const ScriptInteraction *interaction = findBa0Interaction(point);
	const uint cursorIndex = interaction ?
		(interaction->conditionOffset != 0 ? 8 : interaction->initialSelection) :
		_engine->getCursor()->getSelectionIndex();
	_engine->getCursor()->setVisible(true);
	_engine->getCursor()->update(cursorIndex);
	return false;
}

void ScriptManager::updateModalSceneCursor(const Common::Point &point) {
	// RunModalSelectionTableDialogWithRestore at 0x1f7f8 retains the scene
	// selection-state list while the modal chooser is active. Outside the modal
	// rectangle, ProcessChooserControlInput at 0x57372 services that retained
	// list so hotspot cursors continue to animate without dispatching actions.
	const ScriptInteraction *interaction = _runtime.awaitingInteraction ?
		findBa0Interaction(point) : nullptr;
	uint cursorIndex = interaction ?
		(interaction->conditionOffset != 0 ? 8 : interaction->initialSelection) :
		_engine->getCursor()->getSelectionIndex();
	if (_dialogue->isPending() && _dialogue->contains(point))
		cursorIndex = kDialogueCursor;
	_engine->getCursor()->setVisible(true);
	_engine->getCursor()->update(cursorIndex);
}

Common::Rect ScriptManager::interactionBounds(const ScriptInteraction &interaction) const {
	// ExecuteSceneFrameAndInteractions at 0x13277 transposes the compiled
	// interaction axes and adds the active 0x32 presentation origin to the
	// first logical coordinate before registering the physical UI rectangle.
	return Common::Rect(interaction.y, interaction.x + kSceneInteractionOriginY,
		interaction.y + interaction.height,
		interaction.x + kSceneInteractionOriginY + interaction.width);
}

const ScriptInteraction *ScriptManager::findBa0Interaction(const Common::Point &point,
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
