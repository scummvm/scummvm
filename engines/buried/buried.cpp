/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include "engines/util.h"
#include "gui/message.h"

#include "buried/buried.h"
#include "buried/console.h"
#include "buried/database.h"
#include "buried/frame_window.h"
#include "buried/graphics.h"
#include "buried/message.h"
#include "buried/resources.h"
#include "buried/sound.h"
#include "buried/video_window.h"
#include "buried/window.h"

namespace Buried {

BuriedEngine::BuriedEngine(OSystem *syst, const BuriedGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_gfx = 0;
	_mainEXE = 0;
	_library = 0;
	_sound = 0;
	_timerSeed = 0;
	_mainWindow = 0;
	_focusedWindow = 0;
	_captureWindow = 0;
	_console = 0;
	_pauseStartTime = 0;
	_yielding = false;

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "WIN31/MANUAL", 0, 2);
}

BuriedEngine::~BuriedEngine() {
	delete _mainWindow;
	delete _gfx;
	delete _mainEXE;
	delete _library;
	delete _sound;
	delete _console;

	// The queue should be empty since all windows destroy their messages
}

Common::Error BuriedEngine::run() {
	_console = new BuriedConsole(this);

	if (isTrueColor()) {
#ifndef USE_RGB_COLOR
		// Can't play 24bpp version without support
		return Common::kUnsupportedColorMode;
#else
		initGraphics(640, 480, true, 0);

		if (_system->getScreenFormat().bytesPerPixel == 1)
			return Common::kUnsupportedColorMode;
#endif
	} else {
		initGraphics(640, 480, true);
	}

	if (isWin95()) {
		_mainEXE = new DatabasePE();
		_library = new DatabasePE();
	} else if (isCompressed()) {
		_mainEXE = new DatabaseNECompressed();
		_library = new DatabaseNECompressed();
	} else {
		_mainEXE = new DatabaseNE();

		// Demo only uses the main EXE
		if (!isDemo())
			_library = new DatabaseNE();
	}

	if (!_mainEXE->load(getEXEName()))
		error("Failed to load main EXE '%s'", getEXEName().c_str());

	if (_library && !_library->load(getLibraryName()))
		error("Failed to load library DLL '%s'", getLibraryName().c_str());

	syncSoundSettings();

	_gfx = new GraphicsManager(this);
	_sound = new SoundManager(this);
	_mainWindow = new FrameWindow(this);
	_mainWindow->showWindow(Window::kWindowShow);

	if (isDemo()) {
		((FrameWindow *)_mainWindow)->showTitleSequence();
		((FrameWindow *)_mainWindow)->showMainMenu();
	} else {
		bool doIntro = true;

		if (ConfMan.hasKey("save_slot")) {
			uint32 gameToLoad = ConfMan.getInt("save_slot");
			doIntro = (loadGameState(gameToLoad).getCode() != Common::kNoError);
		}

		// Play the intro only if we're starting from scratch
		if (doIntro)
			((FrameWindow *)_mainWindow)->showClosingScreen();
	}

	while (!shouldQuit()) {
		updateTimers();
		updateVideos();

		pollForEvents();

		sendAllMessages();

		_gfx->updateScreen();
		_system->delayMillis(10);
	}

	return Common::kNoError;
}

GUI::Debugger *BuriedEngine::getDebugger() {
	return _console;
}

Common::String BuriedEngine::getString(uint32 stringID) {
	return _mainEXE->loadString(stringID);
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

Common::SeekableReadStream *BuriedEngine::getBitmapStream(uint32 bitmapID) {
	// The demo's bitmaps are in the main EXE
	if (isDemo())
		return _mainEXE->getBitmapStream(bitmapID);

	// The rest in the database library
	return _library->getBitmapStream(bitmapID);
}

Common::SeekableReadStream *BuriedEngine::getNavData(uint32 resourceID) {
	return _mainEXE->getResourceStream("NAVDATA", resourceID);
}

Common::SeekableReadStream *BuriedEngine::getSndData(uint32 resourceID) {
	return _mainEXE->getResourceStream("SNDDATA", resourceID);
}

Common::SeekableReadStream *BuriedEngine::getAnimData(uint32 resourceID) {
	return _mainEXE->getResourceStream("ANIMDATA", resourceID);
}

Common::SeekableReadStream *BuriedEngine::getAIData(uint32 resourceID) {
	return _mainEXE->getResourceStream("AIDATA", resourceID);
}

Common::SeekableReadStream *BuriedEngine::getItemData(uint32 resourceID) {
	return _mainEXE->getResourceStream("ITEMDATA", resourceID);
}

Common::SeekableReadStream *BuriedEngine::getBookData(uint32 resourceID) {
	return _mainEXE->getResourceStream("BOOKDATA", resourceID);
}

Common::SeekableReadStream *BuriedEngine::getFileBCData(uint32 resourceID) {
	return _mainEXE->getResourceStream("FILEBCDATA", resourceID);
}

Common::SeekableReadStream *BuriedEngine::getINNData(uint32 resourceID) {
	return _mainEXE->getResourceStream("INNDATA", resourceID);
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

void BuriedEngine::updateTimers() {
	for (TimerMap::iterator it = _timers.begin(); it != _timers.end(); it++) {
		if (g_system->getMillis() >= it->_value.nextTrigger) {
			it->_value.nextTrigger += it->_value.period;
			it->_value.owner->postMessage(new TimerMessage(it->_key));
		}
	}
}

void BuriedEngine::removeAllTimers(Window *window) {
	for (TimerMap::iterator it = _timers.begin(); it != _timers.end(); it++)
		if (it->_value.owner == window)
			_timers.erase(it);
}

void BuriedEngine::addVideo(VideoWindow *window) {
	_videos.push_back(window);
}

void BuriedEngine::removeVideo(VideoWindow *window) {
	_videos.remove(window);
}

void BuriedEngine::updateVideos() {
	for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++)
		(*it)->updateVideo();
}

void BuriedEngine::postMessageToWindow(Window *dest, Message *message) {
	MessageInfo msg;
	msg.dest = dest;
	msg.message = message;
	_messageQueue.push_back(msg);
}

void BuriedEngine::sendAllMessages() {
	while (!shouldQuit() && !_messageQueue.empty()) {
		MessageInfo msg = _messageQueue.front();
		_messageQueue.pop_front();

		msg.dest->sendMessage(msg.message);
		// Control of the pointer is passed to the destination
	}
}

void BuriedEngine::removeMessages(Window *window, int messageBegin, int messageEnd) {
	for (MessageQueue::iterator it = _messageQueue.begin(); it != _messageQueue.end();) {
		if (it->dest == window && it->message->getMessageType() >= messageBegin && it->message->getMessageType() <= messageEnd) {
			delete it->message;
			it = _messageQueue.erase(it);
		} else {
			it++;
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
			it++;
		}
	}
}

bool BuriedEngine::hasMessage(Window *window, int messageBegin, int messageEnd) const {
	for (MessageQueue::const_iterator it = _messageQueue.begin(); it != _messageQueue.end(); it++)
		if ((!window || it->dest == window) && it->message->getMessageType() >= messageBegin && it->message->getMessageType() <= messageEnd)
			return true;

	return false;
}

void BuriedEngine::yield() {
	// A cut down version of the Win16 yield function. Win32 handles this
	// asynchronously, which we don't want. Only needed for internal event loops.

	// Mark us that we're yielding so we can't save or load in a synchronous sequence
	_yielding = true;

	updateTimers();
	updateVideos();

	pollForEvents();

	// We don't send messages any messages from here. Otherwise, this is the same
	// as our main loop.

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
			if (event.kbd.keycode == Common::KEYCODE_d && (event.kbd.flags & Common::KBD_CTRL)) {
				// Gobble up ctrl+d for the console
				_console->attach();
				_console->onFrame();
			} else {
				if (_focusedWindow)
					_focusedWindow->postMessage(new KeyDownMessage(event.kbd, 0));
			}
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
	return _mainEXE->getVersion();
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
		_sound->stop();

		for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++)
			(*it)->pauseVideo();

		_pauseStartTime = g_system->getMillis();
	} else {
		_sound->restart();

		for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++)
			(*it)->resumeVideo();

		uint32 timeDiff = g_system->getMillis() - _pauseStartTime;

		for (TimerMap::iterator it = _timers.begin(); it != _timers.end(); it++)
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

} // End of namespace Buried
