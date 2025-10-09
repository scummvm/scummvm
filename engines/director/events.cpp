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

#include "common/events.h"
#include "common/keyboard.h"
#include "common/system.h"
#include "common/translation.h"

#include "audio/mixer.h"

#include "gui/message.h"

#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/channel.h"
#include "director/sprite.h"
#include "director/window.h"
#include "director/castmember/castmember.h"

namespace Director {

int DirectorEngine::getMacTicks() { return (int)(g_system->getMillis() * 60 / 1000.) - _tickBaseline; }

bool DirectorEngine::pollEvent(Common::Event &event) {
	// used by UnitTest XObject
	if (!_injectedEvents.empty()) {
		event = _injectedEvents.remove_at(0);
		return true;
	}
	return g_system->getEventManager()->pollEvent(event);
}

bool DirectorEngine::processEvents(bool captureClick, bool skipWindowManager) {
	debugC(9, kDebugEvents, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
	debugC(9, kDebugEvents, "@@@@   Processing events");
	debugC(9, kDebugEvents, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	Common::Event event;
	while (pollEvent(event)) {
		if (skipWindowManager || !_wm->processEvent(event)) {
			// We only want to handle these events if the event
			// wasn't handled by the window manager.
			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				if (_cursorWindow) {
					// The cursor is no longer in a window.
					// Set it to the default arrow cursor.
					_wm->replaceCursor(Graphics::kMacCursorArrow);
					_cursorWindow = nullptr;
				}
				break;
			default:
				break;
			}
		}

		// We want to handle these events regardless.
		switch (event.type) {
		case Common::EVENT_QUIT:
			processEventQUIT();
			if (captureClick)
				return true;
			break;
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_KEYDOWN:
			if (captureClick)
				return true;
			break;
		default:
			break;
		}
	}

	return false;
}

void DirectorEngine::processEventQUIT() {
	if (g_lingo->_exitLock) {
		Common::U32String message = _("The game prevents quitting at this moment. Are you sure you want to quit anyway?");
		GUI::MessageDialog dialog(message, _("Yes"), _("No"));

		g_system->getEventManager()->resetQuit(); // Clear the quit event
		_mixer->pauseAll(true);

		int result = dialog.runModal();
		if (result == GUI::kMessageOK)
			_stage->getCurrentMovie()->getScore()->_playState = kPlayStopped;
		_mixer->pauseAll(false);
	} else {
		_stage->getCurrentMovie()->getScore()->_playState = kPlayStopped;
	}
}

bool Window::processEvent(Common::Event &event) {
	bool flag = false;

	if (_currentMovie && _currentMovie->processEvent(event))
		flag = true;

	return flag;
}

bool Movie::processEvent(Common::Event &event) {
	// When in GUI message box is being shown, movie may record clicking on the message box as a movie event
	// Make sure that these events (mouseUp, mouseDown) are not recorded in the movie
	if (_inGuiMessageBox) {
		return false;
	}

	Score *sc = getScore();
	if (sc->getCurrentFrameNum() > sc->getFramesNum()) {
		warning("processEvents: request to access frame %d of %d", sc->getCurrentFrameNum(), sc->getFramesNum());
		return false;
	}
	uint16 spriteId = 0;

	if (event.mouse != Common::Point(-1, -1)) {
		if (g_director->getVersion() < 400)
			spriteId = _score->getActiveSpriteIDFromPos(event.mouse);
		else
			spriteId = _score->getMouseSpriteIDFromPos(event.mouse);

		_currentHoveredSpriteId = spriteId;
	}

	Common::Point pos;

	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		pos = event.mouse;

		_lastEventTime = g_director->getMacTicks();
		_lastRollTime =	 _lastEventTime;

		if (_vm->getCursorWindow() != _window) {
			// Cursor just entered this window. Force a cursor update.
			_vm->setCursorWindow(_window);
			sc->renderCursor(pos, true);
		} else {
			sc->renderCursor(pos);
		}

		// hiliteChannelId is specified for BitMap castmember, so we deal with them separately with other castmember
		// if we are moving out of bounds, then we don't hilite it anymore
		if (_currentHiliteChannelId && (sc->_channels[_currentHiliteChannelId]->isMouseIn(pos) != kCollisionYes)) {
			g_director->getCurrentWindow()->setDirty(true);
			g_director->getCurrentWindow()->addDirtyRect(sc->_channels[_currentHiliteChannelId]->getBbox());
			_currentHiliteChannelId = 0;
		}

		// for the list style button, we still have chance to trigger events though button.
		if (!(g_director->_wm->_mode & Graphics::kWMModeButtonDialogStyle) && g_director->_wm->_mouseDown && g_director->_wm->_hilitingWidget) {
			if (spriteId > 0 && sc->_channels[spriteId]->_sprite->shouldHilite()) {
				_currentHiliteChannelId = spriteId;
				g_director->getCurrentWindow()->setDirty(true);
				g_director->getCurrentWindow()->addDirtyRect(sc->_channels[_currentHiliteChannelId]->getBbox());
			}
		}

		if (_currentDraggedChannel) {
			if (_currentDraggedChannel->_sprite->_moveable) {
				pos = _draggingSpriteOffset + event.mouse;
				if (!_currentDraggedChannel->_sprite->_trails) {
					g_director->getCurrentMovie()->getWindow()->addDirtyRect(_currentDraggedChannel->getBbox());
				}
				_currentDraggedChannel->setPosition(pos.x, pos.y, true);
				_currentDraggedChannel->_dirty = true;
				g_director->getCurrentMovie()->getWindow()->addDirtyRect(_currentDraggedChannel->getBbox());
			} else {
				_currentDraggedChannel = nullptr;
			}
		}

		// TODO: In the original, these events are generated only
		// along with the kEventIdle event which depends on the idleHandlerPeriod property
		if (g_director->getVersion() >= 500) {

			// In D5, these events are only generated if a mouse button is pressed
			if (g_director->getVersion() < 600)
				if (g_system->getEventManager()->getButtonState() == 0)
					return true;

			if (spriteId > 0) {
				if (spriteId != _lastEnteredChannelId) {
					if (_lastEnteredChannelId) {
						processEvent(kEventMouseLeave, _lastEnteredChannelId);
					}

					_lastEnteredChannelId = spriteId;
					processEvent(kEventMouseEnter, spriteId);
				}
			} else {
				if (_lastEnteredChannelId) {
					processEvent(kEventMouseLeave, _lastEnteredChannelId);
					_lastEnteredChannelId = 0;
				}
			}
		}
		return true;

	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN:
		pos = event.mouse;
		if (sc->_waitForClick) {
			sc->_waitForClick = false;
			sc->renderCursor(pos, true);
		} else {
			pos = event.mouse;

			if (g_director->getVersion() >= 600) {
				if (_lastClickedSpriteId && _lastClickedSpriteId != spriteId) {
					queueInputEvent(kEventMouseUpOutSide, _lastClickedSpriteId, pos);
				}
			}

			// FIXME: Check if these are tracked with the right mouse button
			_lastEventTime = g_director->getMacTicks();
			_lastClickTime2 = _lastClickTime;
			_lastClickTime = _lastEventTime;
			_lastClickPos = pos;
			if (_timeOutMouse)
				_lastTimeOut = _lastEventTime;

			LEvent ev = kEventMouseDown;
			// In D5 and up, right mouse clicks don't trigger the mouseDown handler.
			// They are caught by the rightMouseDown handler only.
			if ((g_director->getVersion() >= 500) && event.type == Common::EVENT_RBUTTONDOWN)
				ev = kEventRightMouseDown;

			if (g_director->getVersion() >= 500 && event.type == Common::EVENT_LBUTTONDOWN && _vm->_emulateMultiButtonMouse) {
				if (g_director->getPlatform() == Common::kPlatformMacintosh) {
					// On Mac, when the mouse button and Control key are pressed
					// at the same time, this simulates right button click
					if (_keyFlags & Common::KBD_CTRL) {
						ev = kEventRightMouseDown;
					}
				}
			}

			debugC(3, kDebugEvents, "Movie::processEvent(): Button Down @(%d, %d), movie '%s'", pos.x, pos.y, _macName.c_str());
			queueInputEvent(ev, 0, pos);

			// D5 has special behavior here
			if (g_director->getVersion() >= 500 && g_director->getVersion() < 600) {
				if (_lastClickedSpriteId)
					queueInputEvent(kEventMouseEnter, _lastClickedSpriteId, pos);
			}
		}

		return true;

	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONUP:
		{
			pos = event.mouse;

			debugC(3, kDebugEvents, "Movie::processEvent(): Button Up @(%d, %d), movie '%s'", pos.x, pos.y, _macName.c_str());

			LEvent ev = kEventMouseUp;
			// In D5 and up, right mouse clicks don't trigger the mouseUp handler.
			// They are caught by the rightMouseUp handler only.
			if ((g_director->getVersion() >= 500) && event.type == Common::EVENT_RBUTTONUP)
				ev = kEventRightMouseUp;

			if (g_director->getVersion() >= 500 && event.type == Common::EVENT_LBUTTONUP && _vm->_emulateMultiButtonMouse) {
				if (g_director->getPlatform() == Common::kPlatformMacintosh) {
					// On Mac, when the mouse button and Control key are pressed
					// at the same time, this simulates right button click
					if (_keyFlags & Common::KBD_CTRL) {
						ev = kEventRightMouseUp;
					}
				}
			}

			queueInputEvent(ev, 0, pos);

			// D5 has special behavior here
			if (g_director->getVersion() >= 500 && g_director->getVersion() < 600) {
				if (spriteId)
					queueInputEvent(kEventMouseLeave, spriteId, pos);
			}

			sc->renderCursor(pos);
		}
		return true;

	case Common::EVENT_KEYDOWN:
		_keyCode = _vm->_KeyCodes.contains(event.kbd.keycode) ? _vm->_KeyCodes[event.kbd.keycode] : 0;
		_key = event.kbd.ascii;
		// While most non-letter keys don't affect "the keyPress", there
		// are some that do and (sadly) we have to account for that.
		switch (event.kbd.keycode) {
		case Common::KEYCODE_LEFT:
			_key = 28;
			break;
		case Common::KEYCODE_RIGHT:
			_key = 29;
			break;
		case Common::KEYCODE_UP:
			_key = 30;
			break;
		case Common::KEYCODE_DOWN:
			_key = 31;
			break;
		default:
			break;
		}
		_keyFlags = event.kbd.flags;

		if (event.kbd.keycode == Common::KEYCODE_LSHIFT || event.kbd.keycode == Common::KEYCODE_RSHIFT ||
			event.kbd.keycode == Common::KEYCODE_LCTRL || event.kbd.keycode == Common::KEYCODE_RCTRL ||
			event.kbd.keycode == Common::KEYCODE_LALT || event.kbd.keycode == Common::KEYCODE_RALT ||
			event.kbd.keycode == Common::KEYCODE_LSUPER || event.kbd.keycode == Common::KEYCODE_RSUPER) {
			// modifier keys don't trigger a KEYDOWN event
			return true;
		}

		debugC(1, kDebugEvents, "Movie::processEvent(): movie '%s', keycode: %d", _macName.c_str(), _keyCode);

		_lastEventTime = g_director->getMacTicks();
		_lastKeyTime = _lastEventTime;
		if (_timeOutKeyDown)
			_lastTimeOut = _lastEventTime;

		queueInputEvent(kEventKeyDown, sc->getSpriteIDOfActiveWidget());
		g_director->loadSlowdownCooloff();
		return true;

	case Common::EVENT_KEYUP:
		queueInputEvent(kEventKeyUp, sc->getSpriteIDOfActiveWidget());
		_keyFlags = event.kbd.flags;
		return true;

	default:
		break;
	}

	return false;
}

bool Window::processWMEvent(Graphics::WindowClick click, Common::Event &event) {
	bool flag = false;
	switch (click) {
	case Graphics::kBorderCloseButton:
		if (_currentMovie && event.type == Common::EVENT_LBUTTONUP) {
			_currentMovie->processEvent(kEventCloseWindow, 0);
			setVisible(false);

			flag = true;
		}
		break;

	case Graphics::kBorderActivate:
		sendWindowEvent(kEventActivateWindow);
		flag = true;
		break;

	case Graphics::kBorderDeactivate:
		sendWindowEvent(kEventDeactivateWindow);
		flag = true;
		break;

	case Graphics::kBorderDragged:
		sendWindowEvent(kEventMoveWindow);
		flag = true;
		break;

	case Graphics::kBorderResized:
		sendWindowEvent(kEventResizeWindow);
		flag = true;
		break;

	case Graphics::kBorderMaximizeButton:
		if (event.type == Common::EVENT_LBUTTONUP) {
			sendWindowEvent(kEventZoomWindow);

			flag = true;
			break;
		}
		break;
	default:
		break;
	}

	flag |= processEvent(event);
	return flag;
}

void Window::sendWindowEvent(LEvent event) {
	if (_currentMovie && _window->isVisible() && !_isStage) {
		// We cannot call processEvent here directly because it might
		// be called from within another event processing (like 'on startMovie'	)
		// which would mess up the Lingo state.
		_currentMovie->queueInputEvent(event, 0, Common::Point(-1, -1));
	}
}


} // End of namespace Director
