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
#include "common/system.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/display.h"
#include "ripper/input.h"
#include "ripper/scenes/librarian_scene.h"
#include "ripper/scenes/wofford_scene.h"
#include "ripper/media/media.h"
#include "ripper/ripper.h"
#include "ripper/media/scene_audio.h"
#include "ripper/script/scene_dispatcher.h"
#include "ripper/script/script.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const uint kSceneSelectionIndex = 0;
static const uint kCyberSelectionIndex = 22;
static const char *const kCyberEntryMedia = "deckin.avi";
static const char *const kCyberScript = "cybrmenu.run";

} // End of anonymous namespace

struct CyberManager::Session {
	IndexedDisplaySnapshot display;
	SceneAudioManager::Snapshot audioState;
	SceneRuntimeState runtime;
};

CyberManager::CyberManager(RipperEngine *engine) : _engine(engine) {
}

bool CyberManager::suspendSession(Session &session) const {
	if (!session.display.capture()) {
		warning("Ripper: could not capture Cyber transition display");
		return false;
	}
	_engine->getSceneAudio()->captureSnapshot(session.audioState);
	_engine->getScripts()->suspendForCyber(session.runtime);
	return true;
}

void CyberManager::prepareSession(int selectionIndex) const {
	_engine->getToolbar()->leave();
	CursorManager *cursor = _engine->getCursor();
	if (selectionIndex >= 0) {
		cursor->setSelectionIndex(selectionIndex);
		cursor->dispatchSelectionIndexChange(selectionIndex);
	}
	cursor->setVisible(false);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getSceneAudio()->clearAll(true);
	g_system->fillScreen(0);
	presentScreen();
}

bool CyberManager::runScriptLoop(bool active, uint action,
		const Common::String &programName) const {
	ScriptManager *scripts = _engine->getScripts();
	while (active && !scripts->isCyberExitRequested() && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}
		if (!scripts->serviceScene()) {
			if (!programName.empty()) {
				const uint frame = scripts->getActiveFrame();
				const Common::String label = frame < scripts->getActiveScript().getFrames().size() ?
					scripts->getActiveScript().getString(scripts->getActiveScript().getFrames()[frame].labelOffset) :
					Common::String();
				warning("Ripper: Cyber program service failed action=%u script='%s' frame=%u/%u label='%s' awaitingInteraction=%d pendingScript='%s' pendingEntry='%s'",
					action, programName.c_str(), frame, scripts->getActiveScript().getFrames().size(),
					label.c_str(), scripts->isAwaitingInteraction(),
					scripts->getPendingSceneMember().c_str(), scripts->getPendingSceneEntryLabel().c_str());
			}
			return false;
		}
		presentScreen();
		g_system->delayMillis(10);
	}
	return active;
}

CyberManager::Result CyberManager::restoreSession(Session &session,
		uint selectionIndex, Result result) const {
	const bool restoreVisibleCursor = session.runtime.awaitingInteraction;
	_engine->getSceneAudio()->clearAll(true);
	const bool audioRestored = _engine->getSceneAudio()->restoreSnapshot(session.audioState);
	_engine->getScripts()->restoreFromCyber(session.runtime);
	session.display.restore();
	CursorManager *cursor = _engine->getCursor();
	cursor->setSelectionIndex(selectionIndex);
	cursor->dispatchSelectionIndexChange(selectionIndex);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	cursor->setVisible(restoreVisibleCursor);
	if (restoreVisibleCursor)
		_engine->getScripts()->updateInteractiveCursor(
			_engine->getInput()->peekMouseState().position);
	return audioRestored ? result : kLoadFailed;
}

CyberManager::Result CyberManager::run() {
	RipperEngine::SaveLoadScope saveLoad(*_engine);
	Session session;
	if (!suspendSession(session))
		return kLoadFailed;

	ScriptManager *scripts = _engine->getScripts();
	Result result = kLoadFailed;

	prepareSession(kCyberSelectionIndex);
	debugC(1, kDebugCyber,
		"Ripper: entering Cyber transition media='%s' script='%s' helpTable=0x1a4 toolbarMask=0x0000 cursor=%u controls=escape-space",
		kCyberEntryMedia, kCyberScript, kCyberSelectionIndex);

	// Escape skips the entry movie; Space pauses or resumes it.
	bool active = _engine->getMedia()->play(kCyberEntryMedia, true);
	if (active && !_engine->shouldQuit()) {
		g_system->fillScreen(0);
		presentScreen();
		active = scripts->startCyberScript(kCyberScript);
	}

	active = runScriptLoop(active);
	if (active || _engine->shouldQuit())
		result = kExited;

	debugC(result == kExited ? 1 : 2, kDebugCyber,
		"Ripper: leaving Cyber nested runtime result=%d exitRequested=%d quit=%d",
		result, scripts->isCyberExitRequested(), _engine->shouldQuit());
	return restoreSession(session, kSceneSelectionIndex, result);
}

CyberManager::Result CyberManager::runProgram(uint action,
		const char *scriptBaseName, uint argument) {
	RipperEngine::SaveLoadScope saveLoad(*_engine);
	Session session;
	if (!suspendSession(session))
		return kLoadFailed;

	ScriptManager *scripts = _engine->getScripts();
	const uint restoreSelectionIndex = _engine->getCursor()->getSelectionIndex();
	const bool isKaDialogue = action == kSceneActionKaDialogue;
	const bool isWoffordMedia = action == kSceneActionKcOrWoffordProgram &&
		Common::String(scriptBaseName) == "wofford-media";
	const bool isSelfContainedScene = isKaDialogue || isWoffordMedia;
	const Common::String programName = isKaDialogue ? Common::String("ka-dialogue") :
		(isWoffordMedia ? Common::String(scriptBaseName) :
			Common::String::format("%s.run", scriptBaseName));
	Result result = kLoadFailed;

	prepareSession();
	debugC(1, kDebugCyber,
		"Ripper: entering Cyber program action=%u program='%s' argument=%u suspendedScript='%s' suspendedFrame=%u toolbarMask=0x0000 savedCursor=%u",
		action, programName.c_str(), argument,
		session.runtime.activeScript.getMemberName().c_str(), session.runtime.activeFrame,
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
		active = scripts->startCyberScript(programName);
	}

	if (!isSelfContainedScene) {
		active = runScriptLoop(active, action, programName);
		if (active || _engine->shouldQuit())
			result = kExited;
	}

	debugC(result == kExited ? 1 : 2, kDebugCyber,
		"Ripper: leaving Cyber program action=%u program='%s' result=%d exitRequested=%d quit=%d",
		action, programName.c_str(), result, scripts->isCyberExitRequested(),
		_engine->shouldQuit());
	return restoreSession(session, restoreSelectionIndex, result);
}

} // End of namespace Ripper
