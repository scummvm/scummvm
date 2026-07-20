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
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/scenes/librarian_scene.h"
#include "ripper/media.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const uint kScreenWidth = 640;
static const uint kScreenHeight = 400;
static const uint kPaletteSize = 256 * 3;
static const uint kAudioSnapshotVersion = 3;
static const uint kSceneSelectionIndex = 0;
static const uint kCyberSelectionIndex = 22;
static const char *const kCyberEntryMedia = "deckin.avi";
static const char *const kCyberScript = "cybrmenu.run";

} // End of anonymous namespace

CyberManager::CyberManager(RipperEngine *engine) : _engine(engine) {
}

bool CyberManager::captureDisplay(DisplaySnapshot &snapshot) const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 ||
			screen->w != kScreenWidth || screen->h != kScreenHeight) {
		if (screen)
			g_system->unlockScreen();
		warning("Ripper: could not capture Cyber transition display");
		return false;
	}

	snapshot.pixels.resize(kScreenWidth * kScreenHeight);
	for (uint y = 0; y < kScreenHeight; ++y)
		memcpy(snapshot.pixels.data() + y * kScreenWidth,
			screen->getBasePtr(0, y), kScreenWidth);
	g_system->unlockScreen();
	snapshot.palette.resize(kPaletteSize);
	g_system->getPaletteManager()->grabPalette(snapshot.palette.data(), 0, 256);
	debugC(2, kDebugCyber,
		"Ripper: captured suspended scene for Cyber transition size=%ux%u paletteEntries=256",
		kScreenWidth, kScreenHeight);
	return true;
}

void CyberManager::restoreDisplay(const DisplaySnapshot &snapshot) const {
	if (snapshot.pixels.size() != kScreenWidth * kScreenHeight ||
			snapshot.palette.size() != kPaletteSize)
		return;
	g_system->copyRectToScreen(snapshot.pixels.data(), kScreenWidth,
		0, 0, kScreenWidth, kScreenHeight);
	g_system->getPaletteManager()->setPalette(snapshot.palette.data(), 0, 256);
	g_system->updateScreen();
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

void CyberManager::suspendRuntime(RuntimeSnapshot &snapshot) const {
	ScriptManager *scripts = _engine->getScripts();
	snapshot.activeScript = Common::move(scripts->_ba0);
	snapshot.concurrentScript = Common::move(scripts->_concurrent);
	snapshot.concurrentEntryLabel = scripts->_concurrentEntryLabel;
	snapshot.pendingSceneMember = scripts->_pendingSceneMember;
	snapshot.pendingSceneEntryLabel = scripts->_pendingSceneEntryLabel;
	snapshot.activeInteractionEnabled = Common::move(scripts->_activeBa0InteractionEnabled);
	snapshot.previousFrameLabel = scripts->_previousBa0FrameLabel;
	snapshot.activeFrame = scripts->_activeBa0Frame;
	snapshot.frontEndActionMask = scripts->_frontEndActionMask;
	snapshot.hoveredInteraction = scripts->_hoveredBa0Interaction;
	snapshot.awaitingInteraction = scripts->_awaitingBa0Interaction;
	snapshot.resumeLoadedPresentation = scripts->_resumeLoadedPresentation;
	snapshot.clearPreservedAudioOnTransition = scripts->_clearPreservedAudioOnTransition;
	snapshot.cyberActive = scripts->_cyberActive;
	snapshot.cyberExitRequested = scripts->_cyberExitRequested;
	snapshot.cyberKeyboardCommand = scripts->_cyberKeyboardCommand;

	scripts->_ba0 = CompiledScript();
	scripts->_concurrent = CompiledScript();
	scripts->_concurrentEntryLabel.clear();
	scripts->_pendingSceneMember.clear();
	scripts->_pendingSceneEntryLabel.clear();
	scripts->_activeBa0InteractionEnabled.clear();
	scripts->_previousBa0FrameLabel.clear();
	scripts->_activeBa0Frame = 0;
	scripts->_frontEndActionMask = 0;
	scripts->_hoveredBa0Interaction = -1;
	scripts->_awaitingBa0Interaction = false;
	scripts->_resumeLoadedPresentation = false;
	scripts->_clearPreservedAudioOnTransition = false;
	scripts->_cyberActive = true;
	scripts->_cyberExitRequested = false;
	scripts->_cyberKeyboardCommand = 0;
	debugC(2, kDebugCyber,
		"Ripper: suspended scene runtime script='%s' frame=%u concurrent='%s'",
		snapshot.activeScript.getMemberName().c_str(), snapshot.activeFrame,
		snapshot.concurrentScript.getMemberName().c_str());
}

void CyberManager::restoreRuntime(RuntimeSnapshot &snapshot) const {
	ScriptManager *scripts = _engine->getScripts();
	scripts->_ba0 = Common::move(snapshot.activeScript);
	scripts->_concurrent = Common::move(snapshot.concurrentScript);
	scripts->_concurrentEntryLabel = snapshot.concurrentEntryLabel;
	scripts->_pendingSceneMember = snapshot.pendingSceneMember;
	scripts->_pendingSceneEntryLabel = snapshot.pendingSceneEntryLabel;
	scripts->_activeBa0InteractionEnabled = Common::move(snapshot.activeInteractionEnabled);
	scripts->_previousBa0FrameLabel = snapshot.previousFrameLabel;
	scripts->_activeBa0Frame = snapshot.activeFrame;
	scripts->_frontEndActionMask = snapshot.frontEndActionMask;
	scripts->_hoveredBa0Interaction = snapshot.hoveredInteraction;
	scripts->_awaitingBa0Interaction = snapshot.awaitingInteraction;
	scripts->_resumeLoadedPresentation = snapshot.resumeLoadedPresentation;
	scripts->_clearPreservedAudioOnTransition = snapshot.clearPreservedAudioOnTransition;
	scripts->_cyberActive = snapshot.cyberActive;
	scripts->_cyberExitRequested = snapshot.cyberExitRequested;
	scripts->_cyberKeyboardCommand = snapshot.cyberKeyboardCommand;
	debugC(2, kDebugCyber,
		"Ripper: restored suspended scene runtime script='%s' frame=%u concurrent='%s'",
		scripts->_ba0.getMemberName().c_str(), scripts->_activeBa0Frame,
		scripts->_concurrent.getMemberName().c_str());
}

CyberManager::Result CyberManager::run() {
	DisplaySnapshot display;
	Common::Array<byte> audioState;
	if (!captureDisplay(display) || !captureAudio(audioState)) {
		restoreDisplay(display);
		return kLoadFailed;
	}

	RuntimeSnapshot runtime;
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
		active = scripts->_ba0.load(_engine->getResources()->scripts(), kCyberScript);
	}
	if (active && !_engine->shouldQuit()) {
		debugC(1, kDebugCyber,
			"Ripper: activated Cyber nested runtime frames=%u interactions=%u",
			scripts->_ba0.getFrames().size(), scripts->_ba0.getInteractions().size());
		active = scripts->advanceBa0ToFrame(0);
	}

	while (active && !scripts->_cyberExitRequested && !_engine->shouldQuit()) {
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
		result, scripts->_cyberExitRequested, _engine->shouldQuit());
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
	DisplaySnapshot display;
	Common::Array<byte> audioState;
	if (!captureDisplay(display) || !captureAudio(audioState)) {
		restoreDisplay(display);
		return kLoadFailed;
	}

	RuntimeSnapshot runtime;
	suspendRuntime(runtime);
	const bool restoreVisibleCursor = runtime.awaitingInteraction;
	ScriptManager *scripts = _engine->getScripts();
	InputManager *input = _engine->getInput();
	CursorManager *cursor = _engine->getCursor();
	const uint restoreSelectionIndex = cursor->getSelectionIndex();
	const bool isKaDialogue = action == kSceneActionKaDialogue;
	const Common::String programName = isKaDialogue ? Common::String("ka-dialogue") :
		Common::String::format("%s.run", scriptBaseName);
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
	} else {
		active = scripts->_ba0.load(_engine->getResources()->scripts(), programName);
		if (active && !_engine->shouldQuit()) {
			debugC(1, kDebugCyber,
				"Ripper: activated Cyber program action=%u script='%s' frames=%u interactions=%u",
				action, programName.c_str(), scripts->_ba0.getFrames().size(),
				scripts->_ba0.getInteractions().size());
			active = scripts->advanceBa0ToFrame(0);
		}
	}

	while (!isKaDialogue && active && !scripts->_cyberExitRequested && !_engine->shouldQuit()) {
		if (input->pollEvents()) {
			_engine->quitGame();
			break;
		}
		if (!scripts->serviceScene()) {
			const uint frame = scripts->_activeBa0Frame;
			const Common::String label = frame < scripts->_ba0.getFrames().size() ?
				scripts->_ba0.getString(scripts->_ba0.getFrames()[frame].labelOffset) :
				Common::String();
			warning("Ripper: Cyber program service failed action=%u script='%s' frame=%u/%u label='%s' awaitingInteraction=%d pendingScript='%s' pendingEntry='%s'",
				action, programName.c_str(), frame, scripts->_ba0.getFrames().size(),
				label.c_str(), scripts->_awaitingBa0Interaction,
				scripts->_pendingSceneMember.c_str(), scripts->_pendingSceneEntryLabel.c_str());
			active = false;
			break;
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
	if (!isKaDialogue && (active || _engine->shouldQuit()))
		result = kExited;

	debugC(result == kExited ? 1 : 2, kDebugCyber,
		"Ripper: leaving Cyber program action=%u program='%s' result=%d exitRequested=%d quit=%d",
		action, programName.c_str(), result, scripts->_cyberExitRequested,
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
