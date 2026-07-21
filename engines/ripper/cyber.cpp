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

#include "ripper/cyber.h"

#include "common/debug.h"
#include "common/memstream.h"
#include "common/serializer.h"
#include "common/system.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/scenes/librarian_scene.h"
#include "ripper/scenes/wofford_scene.h"
#include "ripper/media.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const uint kAudioSnapshotVersion = 3;
static const uint kSceneSelectionIndex = 0;
static const uint kCyberSelectionIndex = 22;
static const char *const kCyberEntryMedia = "deckin.avi";
static const char *const kCyberScript = "cybrmenu.run";

} // End of anonymous namespace

CyberManager::CyberManager(RipperEngine *engine) : _engine(engine) {
}

bool CyberManager::captureDisplay(IndexedDisplaySnapshot &snapshot) const {
	if (!snapshot.capture()) {
		warning("Ripper: could not capture Cyber transition display");
		return false;
	}
	debugC(2, kDebugCyber,
		"Ripper: captured suspended scene for Cyber transition size=%ux%u paletteEntries=256",
		kRipperScreenWidth, kRipperScreenHeight);
	return true;
}

void CyberManager::restoreDisplay(const IndexedDisplaySnapshot &snapshot) const {
	snapshot.restore();
}

bool CyberManager::captureAudio(Common::Array<byte> &state) const {
	Common::MemoryWriteStreamDynamic stream(DisposeAfterUse::NO);
	Common::Serializer serializer(nullptr, &stream);
	serializer.setVersion(kAudioSnapshotVersion);
	if (!_engine->getMedia()->syncGame(serializer) || serializer.err())
		return false;
	state.resize(stream.size());
	if (!state.empty())
		memcpy(state.data(), stream.getData(), state.size());
	debugC(2, kDebugCyber,
		"Ripper: captured suspended Cyber audio state bytes=%u", state.size());
	return true;
}

bool CyberManager::restoreAudio(const Common::Array<byte> &state) const {
	if (state.empty())
		return false;
	Common::MemoryReadStream stream(state.data(), state.size(), DisposeAfterUse::NO);
	Common::Serializer serializer(&stream, nullptr);
	serializer.setVersion(kAudioSnapshotVersion);
	const bool restored = _engine->getMedia()->syncGame(serializer) && !serializer.err();
	debugC(restored ? 2 : 1, kDebugCyber,
		"Ripper: restored suspended Cyber audio state bytes=%u success=%d",
		state.size(), restored);
	return restored;
}

void CyberManager::suspendRuntime(SceneRuntimeState &snapshot) const {
	ScriptManager *scripts = _engine->getScripts();
	scripts->suspendForCyber(snapshot);
}

void CyberManager::restoreRuntime(SceneRuntimeState &snapshot) const {
	ScriptManager *scripts = _engine->getScripts();
	scripts->restoreFromCyber(snapshot);
}

CyberManager::Result CyberManager::run() {
	IndexedDisplaySnapshot display;
	Common::Array<byte> audioState;
	if (!captureDisplay(display) || !captureAudio(audioState)) {
		restoreDisplay(display);
		return kLoadFailed;
	}

	SceneRuntimeState runtime;
	suspendRuntime(runtime);
	const bool restoreVisibleCursor = runtime.awaitingInteraction;
	ScriptManager *scripts = _engine->getScripts();
	InputManager *input = _engine->getInput();
	CursorManager *cursor = _engine->getCursor();
	Result result = kLoadFailed;

	_engine->getToolbar()->leave();
	cursor->setSelectionIndex(kCyberSelectionIndex);
	cursor->dispatchSelectionIndexChange(kCyberSelectionIndex);
	cursor->setVisible(false);
	input->drainKeys();
	input->discardMouseTransitions();
	_engine->getMedia()->clearSceneAudio(true);
	g_system->fillScreen(0);
	g_system->updateScreen();
	debugC(1, kDebugCyber,
		"Ripper: entering Cyber transition media='%s' script='%s' helpTable=0x1a4 toolbarMask=0x0000 cursor=%u controls=escape-space",
		kCyberEntryMedia, kCyberScript, kCyberSelectionIndex);

	// RunCyberMenuSceneTransition at 0x2a86f passes
	// PollPresentationEscOrSpaceCommand at 0x49039 to RunMediaPresentation.
	// Escape skips DECKIN.AVI and Space retains the original pause behavior.
	bool active = _engine->getMedia()->play(kCyberEntryMedia, true);
	if (active && !_engine->shouldQuit()) {
		g_system->fillScreen(0);
		g_system->updateScreen();
		active = scripts->ba0().load(_engine->getResources()->scripts(), kCyberScript);
	}
	if (active && !_engine->shouldQuit()) {
		debugC(1, kDebugCyber,
			"Ripper: activated Cyber nested runtime frames=%u interactions=%u",
			scripts->ba0().getFrames().size(), scripts->ba0().getInteractions().size());
		active = scripts->startActiveFrame(0);
	}

	while (active && !scripts->isCyberExitRequested() && !_engine->shouldQuit()) {
		if (input->pollEvents()) {
			_engine->quitGame();
			break;
		}
		if (!scripts->serviceScene()) {
			active = false;
			break;
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
	if (active || _engine->shouldQuit())
		result = kExited;

	debugC(result == kExited ? 1 : 2, kDebugCyber,
		"Ripper: leaving Cyber nested runtime result=%d exitRequested=%d quit=%d",
		result, scripts->isCyberExitRequested(), _engine->shouldQuit());
	_engine->getMedia()->clearSceneAudio(true);
	const bool audioRestored = restoreAudio(audioState);
	restoreRuntime(runtime);
	restoreDisplay(display);
	cursor->setSelectionIndex(kSceneSelectionIndex);
	cursor->dispatchSelectionIndexChange(kSceneSelectionIndex);
	input->drainKeys();
	input->discardMouseTransitions();
	cursor->setVisible(restoreVisibleCursor);
	if (restoreVisibleCursor)
		scripts->updateInteractiveCursor(input->peekMouseState().position);
	if (!audioRestored)
		result = kLoadFailed;
	return result;
}

CyberManager::Result CyberManager::runProgram(uint action,
		const char *scriptBaseName, uint argument) {
	IndexedDisplaySnapshot display;
	Common::Array<byte> audioState;
	if (!captureDisplay(display) || !captureAudio(audioState)) {
		restoreDisplay(display);
		return kLoadFailed;
	}

	SceneRuntimeState runtime;
	suspendRuntime(runtime);
	const bool restoreVisibleCursor = runtime.awaitingInteraction;
	ScriptManager *scripts = _engine->getScripts();
	InputManager *input = _engine->getInput();
	CursorManager *cursor = _engine->getCursor();
	const uint restoreSelectionIndex = cursor->getSelectionIndex();
	const bool isKaDialogue = action == kSceneActionKaDialogue;
	const bool isWoffordMedia = action == kSceneActionKcOrWoffordProgram &&
		Common::String(scriptBaseName) == "wofford-media";
	const bool isSelfContainedScene = isKaDialogue || isWoffordMedia;
	const Common::String programName = isKaDialogue ? Common::String("ka-dialogue") :
		(isWoffordMedia ? Common::String(scriptBaseName) :
			Common::String::format("%s.run", scriptBaseName));
	Result result = kLoadFailed;

	_engine->getToolbar()->leave();
	cursor->setVisible(false);
	input->drainKeys();
	input->discardMouseTransitions();
	_engine->getMedia()->clearSceneAudio(true);
	g_system->fillScreen(0);
	g_system->updateScreen();
	debugC(1, kDebugCyber,
		"Ripper: entering Cyber program action=%u program='%s' argument=%u suspendedScript='%s' suspendedFrame=%u toolbarMask=0x0000 savedCursor=%u",
		action, programName.c_str(), argument,
		runtime.activeScript.getMemberName().c_str(), runtime.activeFrame,
		restoreSelectionIndex);

	bool active = true;
	if (isKaDialogue) {
		LibrarianScene librarian(_engine);
		const LibrarianScene::Result librarianResult = librarian.run(argument);
		active = librarianResult != LibrarianScene::kLoadFailed;
		if (active)
			result = kExited;
	} else if (isWoffordMedia) {
		WoffordScene wofford(_engine);
		const WoffordScene::Result woffordResult = wofford.run(argument);
		active = woffordResult != WoffordScene::kLoadFailed;
		if (active)
			result = kExited;
	} else {
		active = scripts->ba0().load(_engine->getResources()->scripts(), programName);
		if (active && !_engine->shouldQuit()) {
			debugC(1, kDebugCyber,
				"Ripper: activated Cyber program action=%u script='%s' frames=%u interactions=%u",
				action, programName.c_str(), scripts->ba0().getFrames().size(),
				scripts->ba0().getInteractions().size());
			active = scripts->startActiveFrame(0);
		}
	}

	while (!isSelfContainedScene && active &&
			!scripts->isCyberExitRequested() && !_engine->shouldQuit()) {
		if (input->pollEvents()) {
			_engine->quitGame();
			break;
		}
		if (!scripts->serviceScene()) {
			const uint frame = scripts->getActiveFrame();
			const Common::String label = frame < scripts->ba0().getFrames().size() ?
				scripts->ba0().getString(scripts->ba0().getFrames()[frame].labelOffset) :
				Common::String();
			warning("Ripper: Cyber program service failed action=%u script='%s' frame=%u/%u label='%s' awaitingInteraction=%d pendingScript='%s' pendingEntry='%s'",
				action, programName.c_str(), frame, scripts->ba0().getFrames().size(),
				label.c_str(), scripts->isAwaitingInteraction(),
				scripts->getPendingSceneMember().c_str(), scripts->getPendingSceneEntryLabel().c_str());
			active = false;
			break;
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
	if (!isSelfContainedScene && (active || _engine->shouldQuit()))
		result = kExited;

	debugC(result == kExited ? 1 : 2, kDebugCyber,
		"Ripper: leaving Cyber program action=%u program='%s' result=%d exitRequested=%d quit=%d",
		action, programName.c_str(), result, scripts->isCyberExitRequested(),
		_engine->shouldQuit());
	_engine->getMedia()->clearSceneAudio(true);
	const bool audioRestored = restoreAudio(audioState);
	restoreRuntime(runtime);
	restoreDisplay(display);
	cursor->setSelectionIndex(restoreSelectionIndex);
	cursor->dispatchSelectionIndexChange(restoreSelectionIndex);
	input->drainKeys();
	input->discardMouseTransitions();
	cursor->setVisible(restoreVisibleCursor);
	if (restoreVisibleCursor)
		scripts->updateInteractiveCursor(input->peekMouseState().position);
	if (!audioRestored)
		result = kLoadFailed;
	return result;
}

} // End of namespace Ripper
