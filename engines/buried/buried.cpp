/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/error.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "common/winexe_ne.h"
#include "common/winexe_pe.h"
#include "engines/util.h"
#include "graphics/wincursor.h"
#include "gui/message.h"

#include "buried/agent_evaluation.h"
#include "buried/biochip_right.h"
#include "buried/buried.h"
#include "buried/console.h"
#include "buried/frame_window.h"
#include "buried/gameui.h"
#include "buried/graphics.h"
#include "buried/message.h"
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/sound.h"
#include "buried/video_window.h"
#include "buried/window.h"

namespace Buried {

BuriedEngine::BuriedEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_gfx = nullptr;
	_mainEXE = nullptr;
	_library = nullptr;
	_sound = nullptr;
	_timerSeed = 0;
	_mainWindow = nullptr;
	_focusedWindow = nullptr;
	_captureWindow = nullptr;
	_pauseStartTime = 0;
	_yielding = false;
	_allowVideoSkip = true;

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "WIN31/MANUAL", 0, 2); // v1.05 era
	SearchMan.addSubDirectoryMatching(gameDataDir, "WIN95/MANUAL", 0, 2); // v1.10 era (Trilogy release)

	// GOG.com release, plainly extracted files
	SearchMan.addSubDirectoryMatching(gameDataDir, "data1", 0, 3);
	SearchMan.addSubDirectoryMatching(gameDataDir, "data2", 0, 3);
	SearchMan.addSubDirectoryMatching(gameDataDir, "data3", 0, 3);
}

BuriedEngine::~BuriedEngine() {
	delete _mainWindow;
	delete _gfx;
	delete _mainEXE;
	delete _library;
	delete _sound;

	// The queue should be empty since all windows destroy their messages
}

Common::Error BuriedEngine::run() {
	setDebugger(new BuriedConsole(this));

	ConfMan.registerDefault("skip_support", true);
	_allowVideoSkip = ConfMan.getBool("skip_support");

	if (isTrueColor()) {
		initGraphics(640, 480, nullptr);

		if (_system->getScreenFormat().bytesPerPixel == 1)
			return Common::kUnsupportedColorMode;
	} else {
		initGraphics(640, 480);
	}

	if (isWin95()) {
		_mainEXE = new Common::PEResources();
		_library = new Common::PEResources();
	} else {
		_mainEXE = new Common::NEResources();

		// Demo only uses the main EXE
		if (!isDemo())
			_library = new Common::NEResources();
	}

	if (isCompressed()) {
		if (!_mainEXE->loadFromCompressedEXE(getEXEName()))
			error("Failed to load main EXE '%s'", getEXEName().c_str());

		if (_library && !_library->loadFromCompressedEXE(getLibraryName()))
			error("Failed to load library DLL '%s'", getLibraryName().c_str());
	} else {
		if (!_mainEXE->loadFromEXE(getEXEName()))
			error("Failed to load main EXE '%s'", getEXEName().c_str());

		if (_library && !_library->loadFromEXE(getLibraryName()))
			error("Failed to load library DLL '%s'", getLibraryName().c_str());
	}

	syncSoundSettings();

	_gfx = new GraphicsManager(this);
	_sound = new SoundManager(this);
	_mainWindow = new FrameWindow(this);
	_mainWindow->showWindow(Window::kWindowShow);

	checkForOriginalSavedGames();

	if (isDemo()) {
		((FrameWindow *)_mainWindow)->showTitleSequence();
		((FrameWindow *)_mainWindow)->showMainMenu();
	} else {
		bool doIntro = true;

		if (ConfMan.hasKey("save_slot")) {
			uint32 gameToLoad = ConfMan.getInt("save_slot");
			doIntro = (loadGameState(gameToLoad).getCode() != Common::kNoError);

			// If the trial version tries to load a game without a time
			// zone that's part of the trial version, force the intro.
			if (isTrial() && !((FrameWindow *)_mainWindow)->getMainChildWindow())
				doIntro = true;
		}

		// Play the intro only if we're starting from scratch
		if (doIntro)
			((FrameWindow *)_mainWindow)->showClosingScreen();
	}

	while (!shouldQuit()) {
		updateVideos();

		pollForEvents();

		sendAllMessages();

		_gfx->updateScreen();
		_system->delayMillis(10);
	}

	return Common::kNoError;
}

Common::String BuriedEngine::getString(uint32 stringID) {
	bool continueReading = true;
	Common::String result;

	while (continueReading) {
		Common::String string = _mainEXE->loadString(stringID);

		if (string.empty())
			return "";

		if (string[0] == '!') {
			string.deleteChar(0);
			stringID++;
		} else {
			continueReading = false;
		}

		result += string;
	}

	// Change any \r to \n
	for (uint32 i = 0; i < result.size(); i++)
		if (result[i] == '\r')
			result.setChar('\n', i);

	return result;
}

Common::String BuriedEngine::getFilePath(uint32 stringID) {
	Common::String path = getString(stringID);
	Common::String output;

	if (path.empty())
		return output;

	uint i = 0;

	// The non-demo paths have CD info followed by a backslash.
	// We ignore this.
	// In the demo, we remove the "BITDATA" prefix because the
	// binaries are in the same directory.
	if (isDemo())
		i += 8;
	else
		i += 2;

	for (; i < path.size(); i++) {
		if (path[i] == '\\')
			output += '/';
		else
			output += path[i];
	}

	return output;
}

Graphics::WinCursorGroup *BuriedEngine::getCursorGroup(uint32 cursorGroupID) {
	return Graphics::WinCursorGroup::createCursorGroup(_mainEXE, cursorGroupID);
}

Common::SeekableReadStream *BuriedEngine::getBitmapStream(uint32 bitmapID) {
	// The demo's bitmaps are in the main EXE
	if (isDemo())
		return _mainEXE->getResource(Common::kWinBitmap, bitmapID);

	// The rest in the database library
	return _library->getResource(Common::kWinBitmap, bitmapID);
}

Common::SeekableReadStream *BuriedEngine::getNavData(uint32 resourceID) {
	return _mainEXE->getResource(Common::String("NAVDATA"), resourceID);
}

Common::SeekableReadStream *BuriedEngine::getSndData(uint32 resourceID) {
	return _mainEXE->getResource(Common::String("SNDDATA"), resourceID);
}

Common::SeekableReadStream *BuriedEngine::getAnimData(uint32 resourceID) {
	return _mainEXE->getResource(Common::String("ANIMDATA"), resourceID);
}

Common::SeekableReadStream *BuriedEngine::getAIData(uint32 resourceID) {
	return _mainEXE->getResource(Common::String("AIDATA"), resourceID);
}

Common::SeekableReadStream *BuriedEngine::getItemData(uint32 resourceID) {
	return _mainEXE->getResource(Common::String("ITEMDATA"), resourceID);
}

Common::SeekableReadStream *BuriedEngine::getBookData(uint32 resourceID) {
	return _mainEXE->getResource(Common::String("BOOKDATA"), resourceID);
}

Common::SeekableReadStream *BuriedEngine::getFileBCData(uint32 resourceID) {
	return _mainEXE->getResource(Common::String("FILEBCDATA"), resourceID);
}

Common::SeekableReadStream *BuriedEngine::getINNData(uint32 resourceID) {
	return _mainEXE->getResource(Common::String("INNDATA"), resourceID);
}

uint BuriedEngine::createTimer(Window *window, uint period) {
	uint timer = ++_timerSeed;

	Timer timerInfo;
	timerInfo.owner = window;
	timerInfo.period = period;
	timerInfo.nextTrigger = _system->getMillis() + period;

	_timers[timer] = timerInfo;
	return timer;
}

bool BuriedEngine::killTimer(uint timer) {
	TimerMap::iterator it = _timers.find(timer);

	if (it == _timers.end())
		return false;

	_timers.erase(it);
	return true;
}

void BuriedEngine::removeAllTimers(Window *window) {
	for (TimerMap::iterator it = _timers.begin(); it != _timers.end(); ++it) {
		if (it->_value.owner == window)
			_timers.erase(it);
	}
}

void BuriedEngine::addVideo(VideoWindow *window) {
	_videos.push_back(window);
}

void BuriedEngine::removeVideo(VideoWindow *window) {
	_videos.remove(window);
}

void BuriedEngine::updateVideos() {
	for (VideoList::iterator it = _videos.begin(); it != _videos.end(); ++it)
		(*it)->updateVideo();
}

void BuriedEngine::postMessageToWindow(Window *dest, Message *message) {
	MessageInfo msg;
	msg.dest = dest;
	msg.message = message;
	_messageQueue.push_back(msg);
}

void BuriedEngine::processAudioVideoSkipMessages(VideoWindow *video, int soundId) {
	assert(video || soundId >= 0);

	for (MessageQueue::iterator it = _messageQueue.begin(); it != _messageQueue.end();) {
		MessageType messageType = it->message->getMessageType();

		if (messageType == kMessageTypeKeyUp) {
			Common::KeyState keyState = ((KeyUpMessage *)it->message)->getKeyState();

			// Send any skip keyup events to the audio/video players
			if (keyState.keycode == Common::KEYCODE_ESCAPE) {
				if (video)
					video->onKeyUp(keyState, ((KeyUpMessage *)it->message)->getFlags());

				if (soundId >= 0)
					_sound->stopSound(soundId);

				delete it->message;
				it = _messageQueue.erase(it);
			} else {
				++it;
			}
		} else if (messageType == kMessageTypeKeyDown) {
			Common::KeyState keyState = ((KeyDownMessage *)it->message)->getKeyState();

			// Erase any skip video keydown events from the queue, to avoid
			// interpreting them as game quit events after the video ends
			if (keyState.keycode == Common::KEYCODE_ESCAPE) {
				delete it->message;
				it = _messageQueue.erase(it);
			} else {
				++it;
			}
		} else {
			++it;
		}
	}
}

void BuriedEngine::sendAllMessages() {
	while (!shouldQuit() && !_messageQueue.empty()) {
		MessageInfo msg = _messageQueue.front();
		_messageQueue.pop_front();

		msg.dest->sendMessage(msg.message);
		// Control of the pointer is passed to the destination
	}

	// Generate a timer messages while they exist and there are no messages
	// in the queue.
	while (!shouldQuit() && _messageQueue.empty()) {
		// Generate a timer message
		bool ranTimer = false;

		for (TimerMap::iterator it = _timers.begin(); it != _timers.end(); ++it) {
			uint32 time = g_system->getMillis();

			if (time >= it->_value.nextTrigger) {
				// Adjust the trigger to be what the next one would be, after
				// all the current triggers would be called.
				uint32 triggerCount = (time - it->_value.nextTrigger + it->_value.period) / it->_value.period;
				it->_value.nextTrigger += triggerCount * it->_value.period;
				it->_value.owner->sendMessage(new TimerMessage(it->_key));
				ranTimer = true;
				break;
			}
		}

		// If no timers were run, there's nothing to keep looking for
		if (!ranTimer)
			break;
	}
}

void BuriedEngine::removeMessages(Window *window, int messageBegin, int messageEnd) {
	for (MessageQueue::iterator it = _messageQueue.begin(); it != _messageQueue.end();) {
		if (it->dest == window && it->message->getMessageType() >= messageBegin && it->message->getMessageType() <= messageEnd) {
			delete it->message;
			it = _messageQueue.erase(it);
		} else {
			++it;
		}
	}
}

void BuriedEngine::removeKeyboardMessages(Window *window) {
	removeMessages(window, kMessageTypeKeyBegin, kMessageTypeKeyEnd);
}

void BuriedEngine::removeMouseMessages(Window *window) {
	removeMessages(window, kMessageTypeMouseBegin, kMessageTypeMouseEnd);
}

void BuriedEngine::removeAllMessages(Window *window) {
	for (MessageQueue::iterator it = _messageQueue.begin(); it != _messageQueue.end();) {
		if (it->dest == window) {
			delete it->message;
			it = _messageQueue.erase(it);
		} else {
			++it;
		}
	}
}

bool BuriedEngine::hasMessage(Window *window, int messageBegin, int messageEnd) const {
	// Implementation note: This doesn't currently handle timers, but would on real Windows.
	// Buried doesn't check for timer messages being present, so it's skipped.

	for (MessageQueue::const_iterator it = _messageQueue.begin(); it != _messageQueue.end(); ++it)
		if ((!window || it->dest == window) && it->message->getMessageType() >= messageBegin && it->message->getMessageType() <= messageEnd)
			return true;

	return false;
}

void BuriedEngine::yield(VideoWindow *video, int soundId) {
	// A cut down version of the Win16 yield function. Win32 handles this
	// asynchronously, which we don't want. Only needed for internal event loops.

	// Mark us that we're yielding so we can't save or load in a synchronous sequence
	_yielding = true;

	updateVideos();

	pollForEvents();

	// We only send audio/video skipping messages from here. Otherwise, this is the same
	// as our main loop.
	if ((video || soundId >= 0) && _allowVideoSkip)
		processAudioVideoSkipMessages(video, soundId);

	_gfx->updateScreen();
	_system->delayMillis(10);

	_yielding = false;
}

void BuriedEngine::pollForEvents() {
	// TODO: Key flags
	// TODO: Mouse flags

	Common::Event event;
	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE: {
			_gfx->markMouseMoved();
			Window *window = _captureWindow ? _captureWindow : _mainWindow->childWindowAtPoint(event.mouse);
			window->postMessage(new MouseMoveMessage(window->convertPointToLocal(event.mouse), 0));
			window->postMessage(new SetCursorMessage(kMessageTypeMouseMove));
			break;
		}
		case Common::EVENT_KEYUP:
			if (_focusedWindow)
				_focusedWindow->postMessage(new KeyUpMessage(event.kbd, 0));
			break;
		case Common::EVENT_KEYDOWN:
			if (_focusedWindow)
				_focusedWindow->postMessage(new KeyDownMessage(event.kbd, 0));
			break;
		case Common::EVENT_LBUTTONDOWN: {
			Window *window = _captureWindow ? _captureWindow : _mainWindow->childWindowAtPoint(event.mouse);
			window->postMessage(new LButtonDownMessage(window->convertPointToLocal(event.mouse), 0));
			break;
		}
		case Common::EVENT_LBUTTONUP: {
			Window *window = _captureWindow ? _captureWindow : _mainWindow->childWindowAtPoint(event.mouse);
			window->postMessage(new LButtonUpMessage(window->convertPointToLocal(event.mouse), 0));
			break;
		}
		case Common::EVENT_MBUTTONUP: {
			Window *window = _captureWindow ? _captureWindow : _mainWindow->childWindowAtPoint(event.mouse);
			window->postMessage(new MButtonUpMessage(window->convertPointToLocal(event.mouse), 0));
			break;
		}
		case Common::EVENT_RBUTTONDOWN: {
			Window *window = _captureWindow ? _captureWindow : _mainWindow->childWindowAtPoint(event.mouse);
			window->postMessage(new RButtonDownMessage(window->convertPointToLocal(event.mouse), 0));
			break;
		}
		case Common::EVENT_RBUTTONUP: {
			Window *window = _captureWindow ? _captureWindow : _mainWindow->childWindowAtPoint(event.mouse);
			window->postMessage(new RButtonUpMessage(window->convertPointToLocal(event.mouse), 0));
			break;
		}
		default:
			break;
		}
	}
}

int BuriedEngine::getTransitionSpeed() {
	assert(_mainWindow);
	return ((FrameWindow *)_mainWindow)->getTransitionSpeed();
}

void BuriedEngine::setTransitionSpeed(int newSpeed) {
	assert(_mainWindow);
	((FrameWindow *)_mainWindow)->setTransitionSpeed(newSpeed);
}

uint32 BuriedEngine::getVersion() {
	if (isWin95()) {
		// Not really needed, it should only be 1.1
		return MAKEVERSION(1, 1, 0, 0);
	}

	Common::WinResources::VersionInfo *versionInfo = _mainEXE->getVersionResource(1);
	uint32 result = MAKEVERSION(versionInfo->fileVersion[0], versionInfo->fileVersion[1], versionInfo->fileVersion[2], versionInfo->fileVersion[3]);
	delete versionInfo;

	return result;
}

Common::String BuriedEngine::getFilePath(int timeZone, int environment, int fileOffset) {
	return getFilePath(computeFileNameResourceID(timeZone, environment, fileOffset));
}

uint32 BuriedEngine::computeNavDBResourceID(int timeZone, int environment) {
	return RESID_NAVDB_BASE + RESOFFSET_NAVDB_TIMEZONE * timeZone + environment;
}

uint32 BuriedEngine::computeAnimDBResourceID(int timeZone, int environment) {
	return RESID_ANIMDB_BASE + RESOFFSET_ANIMDB_TIMEZONE * timeZone + environment;
}

uint32 BuriedEngine::computeAIDBResourceID(int timeZone, int environment) {
	return RESID_AI_DB_BASE + RESOFFSET_AI_DB_TIMEZONE * timeZone + environment;
}

uint32 BuriedEngine::computeFileNameResourceID(int timeZone, int environment, int fileOffset) {
	return RESID_FILENAMES_BASE + RESOFFSET_FILENAME_TIMEZONE * timeZone + RESOFFSET_FILENAME_ENVIRON * environment + fileOffset;
}

void BuriedEngine::pauseEngineIntern(bool pause) {
	if (pause) {
		_sound->pause(true);

		for (VideoList::iterator it = _videos.begin(); it != _videos.end(); ++it)
			(*it)->pauseVideo();

		_pauseStartTime = g_system->getMillis();
	} else {
		_sound->pause(false);

		for (VideoList::iterator it = _videos.begin(); it != _videos.end(); ++it)
			(*it)->resumeVideo();

		uint32 timeDiff = g_system->getMillis() - _pauseStartTime;

		for (TimerMap::iterator it = _timers.begin(); it != _timers.end(); ++it)
			it->_value.nextTrigger += timeDiff;
	}
}

bool BuriedEngine::runQuitDialog() {
	// TODO: Would be nice to load the text out of the EXE for this
	// v1.04+: IDS_APP_MESSAGE_QUIT_TEXT (9024)
	GUI::MessageDialog dialog(_("Are you sure you want to quit?"), _("Yes"), _("No"));
	return dialog.runModal() == GUI::kMessageOK;
}

bool BuriedEngine::isControlDown() const {
	return _mainWindow && ((FrameWindow *)_mainWindow)->_controlDown;
}

void BuriedEngine::pauseGame() {
	// TODO: Would be nice to load the translated text from IDS_APP_MESSAGE_PAUSED_TEXT (9023)
	GUI::MessageDialog dialog(_("Your game is now Paused.  Click OK to continue."));
	runDialog(dialog);
}

void BuriedEngine::showPoints() {
	if (isDemo())
		return;

	FrameWindow *frameWindow = (FrameWindow *)_mainWindow;
	SceneViewWindow *sceneView = ((GameUIWindow *)frameWindow->getMainChildWindow())->_sceneViewWindow;
	AgentEvaluation *agentEvaluation = new AgentEvaluation(this, sceneView->getGlobalFlags(), -1);

	GUI::MessageDialog dialog(
		agentEvaluation->_scoringTextDescriptionsWithScores,
		"OK",
		Common::U32String(),
		Graphics::kTextAlignLeft
	);
	runDialog(dialog);

	delete agentEvaluation;
}

void BuriedEngine::handleSaveDialog() {
	FrameWindow *frameWindow = (FrameWindow *)_mainWindow;
	BioChipRightWindow *bioChipWindow = ((GameUIWindow *)frameWindow->getMainChildWindow())->_bioChipRightWindow;

	if (isDemo())
		return;

	if (saveGameDialog())
		bioChipWindow->destroyBioChipViewWindow();
}

void BuriedEngine::handleRestoreDialog() {
	FrameWindow *frameWindow = (FrameWindow *)_mainWindow;
	BioChipRightWindow *bioChipWindow = ((GameUIWindow *)frameWindow->getMainChildWindow())->_bioChipRightWindow;

	if (isDemo())
		return;

	if (loadGameDialog())
		bioChipWindow->destroyBioChipViewWindow();
}

} // End of namespace Buried
