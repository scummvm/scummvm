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

#ifndef RIPPER_SCRIPT_H
#define RIPPER_SCRIPT_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

namespace Common {
class Serializer;
}

namespace Ripper {

class AssetLibrary;
class BriefingManager;
class ResourceManager;
class RipperEngine;
class DialogueManager;

enum ScriptOpcode : byte {
	kNoOp0                       = 0x00, // HandleSceneEntryNoOp at 0x146e5
	kNoOp1                       = 0x01, // HandleSceneEntryNoOp at 0x146e5
	kNoOp2                       = 0x02, // HandleSceneEntryNoOp at 0x146e5
	kNoOp3                       = 0x03, // HandleSceneEntryNoOp at 0x146e5
	kNoOp4                       = 0x04, // HandleSceneEntryNoOp at 0x146e5
	kNoOp5                       = 0x05, // HandleSceneEntryNoOp at 0x146e5
	kNoOp6                       = 0x06, // HandleSceneEntryNoOp at 0x146e5
	kNoOp7                       = 0x07, // HandleSceneEntryNoOp at 0x146e5
	kMilestoneCondition          = 0x08, // HandleSceneEntryPromptMilestoneCondition at 0x1470a
	kScenePlayedCondition        = 0x09, // HandleSceneEntryPromptScenePlayedCondition at 0x1488f
	kDialogueChoiceCondition     = 0x0a, // HandleSceneEntryStepPromptCondition at 0x149b4
	kPreviousSceneCondition      = 0x0b, // HandleSceneEntryPromptPreviousSceneCondition at 0x14a37
	kDefaultBranch               = 0x0c, // HandleSceneEntryStepPromptDefaultBranch at 0x14bbb
	kClearBranchFlag             = 0x0d, // HandleSceneEntryStepPromptClearBranchFlag at 0x14c11
	kSetMilestoneFlag            = 0x0e, // HandleSceneEntrySetMilestoneFlag at 0x14c4d
	kClearMilestoneFlag          = 0x0f, // HandleSceneEntryClearMilestoneFlag at 0x14cc2
	kDisableInteraction          = 0x10, // HandleSceneEntryClearInteractionSelectionAndStepPrompt at 0x14d41
	kSetInteractionSelection     = 0x11, // HandleSceneEntrySetInteractionSelectionAndStepPrompt at 0x14dbc
	kResetInteractionSelection   = 0x12, // HandleSceneEntryResetInteractionProxySelectionAndStepPrompt at 0x14e31
	kShowTextPanel               = 0x13, // HandleSceneEntryWrappedTextPanelLifecycle at 0x14ec4
	kSelectFrame                 = 0x14, // HandleSceneEntrySetResultAndStepPrompt at 0x15042
	kAddDialogueChoice           = 0x15, // HandleSceneEntryPushChoiceRecordAndStepPrompt at 0x15085
	kAddConditionalDialogueChoice = 0x16, // HandleSceneEntryPushFrameChoiceOnPlayedStateCondition at 0x150ea
	kStartDialogue               = 0x17, // HandleSceneEntryChoiceListLifecycle at 0x1523d
	kDispatchSceneAction         = 0x18, // HandleSceneEntryPromptAndDispatchAction at 0x1574d
	kRequestTextInput            = 0x19, // HandleSceneEntryAsyncTextRequest at 0x157a1
	kPlayMedia                   = 0x1a, // HandleSceneEntryMediaAndSetBasenameFlag at 0x159e1
	kPreviewMedia                = 0x1b, // HandleSceneEntryMediaPreviewOrPrompt at 0x15b03
	kFadePalette                 = 0x1c, // HandleSceneEntryStepPromptTransition at 0x15c30
	kStartSceneRuntime           = 0x1d, // HandleSceneEntryAndStartConcurrentSceneRuntime at 0x15cd3
	kSetNamedFlag                = 0x1e, // HandleSceneEntrySetOrClearNamedFlag at 0x15dfe
	kLoadAudio                   = 0x1f, // HandleSceneEntryLoadResourceIntoFirstFreeSlot at 0x15e48
	kConfigureAudio              = 0x20, // HandleSceneEntryConfigureOrStartNamedAudioTrigger at 0x15eea
	kClearAudio                  = 0x21, // HandleSceneEntryClearNamedAudioTrigger at 0x15fba
	kStopAudio                   = 0x22, // HandleSceneEntryStopNamedAudioTrigger at 0x16043
	kSetAudioVolume              = 0x23, // HandleSceneEntrySetNamedSlotVolume at 0x160cc
	kWaitForFrameCounter         = 0x24  // HandleSceneEntryWaitForSceneFrameCounter at 0x1633e
};

struct ScriptArgument {
	byte type;
	uint32 value;
	Common::Array<byte> data;
};

struct ScriptCommand {
	uint32 offset;
	ScriptOpcode opcode;
	uint16 selector;
	Common::Array<ScriptArgument> arguments;
};

struct ScriptFrame {
	byte presentationType;
	uint32 mediaNameOffset;
	uint32 labelOffset;
	uint32 textOffset;
	byte interactionCount;
	uint32 enterCallbackOffset;
	uint32 idleCallbackOffset;
	uint32 exitCallbackOffset;
	uint16 firstInteractionIndex;
	int16 x;
	int16 y;
	uint16 initialSelection;
};

struct ScriptInteraction {
	Common::String label;
	int16 x;
	int16 y;
	int16 width;
	int16 height;
	byte initialSelection;
	uint32 conditionOffset;
	uint32 callbackOffset;
	byte flags;
};

class CompiledScript {
public:
	CompiledScript();

	bool load(AssetLibrary &library, const Common::String &memberName);
	bool decodeCallback(uint32 offset, bool decodeText, Common::Array<ScriptCommand> &commands) const;

	const Common::String &getMemberName() const { return _memberName; }
	const Common::String &getSourceName() const { return _sourceName; }
	uint16 getVersion() const { return _version; }
	const Common::Array<ScriptFrame> &getFrames() const { return _frames; }
	const Common::Array<ScriptInteraction> &getInteractions() const { return _interactions; }
	Common::String getString(uint32 offset) const;

private:
	bool parseHeader();
	bool validateCallbacks() const;
	bool canRead(uint32 offset, uint32 size) const;
	uint16 readUint16(uint32 offset) const;
	int16 readSint16(uint32 offset) const;
	uint32 readUint32(uint32 offset) const;

	Common::String _memberName;
	Common::String _sourceName;
	Common::Array<byte> _data;
	Common::Array<ScriptFrame> _frames;
	Common::Array<ScriptInteraction> _interactions;
	uint16 _version;
	uint32 _argumentLayoutOffset;
	byte _argumentLayoutStride;
	uint16 _argumentLayoutCount;
};

class ScriptManager {
public:
	explicit ScriptManager(RipperEngine *engine);
	~ScriptManager();

	bool initialize(ResourceManager &resources);
	bool runStartupPath();
	bool serviceScene();
	void drawDialogueOverlay(bool captureBacking = false);
	void drawBriefingOverlay();
	bool updateInteractiveCursor(const Common::Point &point, bool *failed = nullptr);
	bool openWorldMap();
	bool hasPendingSceneTransition() const { return !_pendingSceneMember.empty(); }
	bool canSaveGame() const;
	bool syncGame(Common::Serializer &serializer);
	void logRuntimeFailure(const char *reason) const;

	CompiledScript &startup() { return _startup; }
	CompiledScript &ba0() { return _ba0; }

private:
	bool executeCallback(CompiledScript &script, uint32 callbackOffset, int &result,
		uint *nextFrame = nullptr);
	void bindBa0Frame(uint frameIndex);
	void initializeBa0InteractionState(const ScriptFrame &frame);
	void beginBa0InteractionWait(const Common::String &frameLabel, uint interactionCount);
	bool advanceBa0ToFrame(uint nextFrame);
	bool executeConcurrentFrame();
	bool performPendingSceneTransition();
	bool findFrameByLabel(const CompiledScript &script, const Common::String &label, uint &frameIndex) const;
	const ScriptInteraction *findBa0Interaction(const Common::Point &point,
		uint *interactionIndex = nullptr) const;
	bool isScenePlayed(const Common::String &scene) const;
	void markScenePlayed(const Common::String &scene);
	static Common::String argumentString(const ScriptArgument &argument);

	RipperEngine *_engine;
	CompiledScript _startup;
	CompiledScript _ba0;
	CompiledScript _concurrent;
	Common::String _concurrentEntryLabel;
	Common::String _pendingSceneMember;
	Common::String _pendingSceneEntryLabel;
	Common::Array<Common::String> _playedScenes;
	Common::Array<bool> _activeBa0InteractionEnabled;
	Common::String _previousBa0FrameLabel;
	uint _activeBa0Frame;
	int _hoveredBa0Interaction;
	bool _awaitingBa0Interaction;
	bool _resumeLoadedPresentation;
	bool _clearPreservedAudioOnTransition;
	BriefingManager *_briefing;
	DialogueManager *_dialogue;
};

} // End of namespace Ripper

#endif // RIPPER_SCRIPT_H
