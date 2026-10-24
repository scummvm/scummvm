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
#include "common/ptr.h"
#include "common/rect.h"
#include "common/str.h"

#include "ripper/dialogue.h"
#include "ripper/milestones.h"
#include "ripper/script/compiled_script.h"
#include "ripper/script/scene_runtime.h"

namespace Common {
class Serializer;
}

namespace Ripper {

class BriefingManager;
class ResourceManager;
class RipperEngine;
class SceneActionDispatcher;

class ScriptManager {
public:
	explicit ScriptManager(RipperEngine *engine);
	~ScriptManager();

	bool initialize(ResourceManager &resources, bool loadRetailOverlays);
	bool runStartupPath();
	bool serviceScene();
	void drawDialogueOverlay(bool captureBacking = false);
	void drawBriefingOverlay();
	bool updateInteractiveCursor(const Common::Point &point, bool *failed = nullptr);
	void updateModalSceneCursor(const Common::Point &point);
	bool openWorldMap();
	bool openInventory(int initialUnlockFlag = -1, bool grantItem = false);
	bool hasActivePrompt() const;
	bool showHelp(const char *source);
	bool triggerHelpHotkey(const char *source);
	bool hasPendingSceneTransition() const {
		return _runtimeRestorePending || !_runtime.pendingSceneMember.empty() ||
			_runtime.cyberExitRequested ||
			_runtime.cyberKeyboardCommand != 0;
	}
	bool hasPendingRuntimeRestore() const { return _runtimeRestorePending; }
	bool isCyberActive() const { return _runtime.cyberActive; }
	bool isCyberExitRequested() const { return _runtime.cyberExitRequested; }
	bool isDemoRuntimeComplete() const { return _demoRuntimeComplete; }
	bool isAwaitingInteraction() const { return _runtime.awaitingInteraction; }
	uint getActiveFrame() const { return _runtime.activeFrame; }
	const Common::String &getPendingSceneMember() const { return _runtime.pendingSceneMember; }
	const Common::String &getPendingSceneEntryLabel() const { return _runtime.pendingSceneEntryLabel; }
	bool hasPlayedScene(const Common::String &scene) const { return isScenePlayed(scene); }
	int resolveToolbarActionKey(uint16 command) const;
	bool dispatchToolbarActionKey(uint actionIndex, uint16 command, const char *source);
	bool replayPreviousInteractionScene(const char *source);
	void requestCyberExit(const char *source);
	void suspendForCyber(SceneRuntimeState &snapshot);
	void restoreFromCyber(SceneRuntimeState &snapshot);
	bool startCyberScript(const Common::String &memberName);
	bool canSaveGame() const;

	struct SaveState {
		explicit SaveState(const ScriptManager &scripts);

		SceneRuntimeState runtime;
		Milestones milestones;
		DialogueChooser dialogue;
		Common::Array<Common::String> playedScenes;
		byte briefingArmed;
		uint32 briefingSelector;
		uint16 announcedSelectors;
	};

	bool syncGame(Common::Serializer &serializer);
	bool syncGame(Common::Serializer &serializer, SaveState &state);
	bool applySaveState(SaveState &state);
	void logRuntimeFailure(const char *reason) const;

	const CompiledScript &getActiveScript() const { return _runtime.activeScript; }
	DialogueChooser *getDialogue() const { return _dialogue.get(); }

private:
	friend class SceneActionDispatcher;
	class IdleMediaCallback;

	bool executeCallback(const CompiledScript &script, uint32 callbackOffset, int &result,
		uint *nextFrame = nullptr, uint commandStart = 0);
	bool handleActiveRuntimeExit(int result, const CompiledScript &script,
		const char *callbackPhase);
	bool acceptConcurrentRuntimeExit(int result, const char *callbackPhase);
	bool captureCyberKeyboardCommand();
	bool serviceCyberKeyboardCommand();
	bool serviceToolbarActionKey(const char *source, bool &handled);
	void bindBa0Frame(uint frameIndex);
	void initializeBa0InteractionState(const ScriptFrame &frame);
	void beginBa0InteractionWait(const Common::String &frameLabel, uint interactionCount);
	bool advanceBa0ToFrame(uint nextFrame);
	bool executeConcurrentFrame();
	bool performPendingSceneTransition();
	bool runWorldMapCheckpoint(uint chapter);
	uint resolveFrameIndex(const CompiledScript &script, const Common::String &label) const;
	Common::Rect interactionBounds(const ScriptInteraction &interaction) const;
	const ScriptInteraction *findBa0Interaction(const Common::Point &point,
		uint *interactionIndex = nullptr) const;
	bool isScenePlayed(const Common::String &scene) const;
	void setScenePlayed(const Common::String &scene, bool played);
	void markScenePlayed(const Common::String &scene);
	void retainPreviousInteractionScene(const Common::String &mediaPath,
		const char *source);
	static Common::String argumentString(const ScriptArgument &argument);

	RipperEngine *_engine;
	CompiledScript _startup;
	SceneRuntimeState _runtime;
	bool _demoScriptAbi;
	bool _demoRuntimeComplete;
	bool _runtimeRestorePending;
	Common::Array<Common::String> _playedScenes;
	Common::String _previousInteractionSceneMediaPath;
	uint _sceneCallbackFrame;
	IdleMediaCallback *_activeIdleMediaCallback;
	byte _chooserTemplateMode;
	Common::ScopedPtr<BriefingManager> _briefing;
	Common::ScopedPtr<DialogueChooser> _dialogue;
};

} // End of namespace Ripper

#endif // RIPPER_SCRIPT_H
