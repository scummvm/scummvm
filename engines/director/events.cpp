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

#include "common/system.h"
#include "common/translation.h"

#include "audio/mixer.h"

#include "gui/message.h"

#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/cursor.h"
#include "director/channel.h"
#include "director/sprite.h"
#include "director/window.h"
#include "director/castmember.h"
#include "director/lingo/lingo.h"

namespace Director {

uint32 DirectorEngine::getMacTicks() { return g_system->getMillis() * 60 / 1000.; }

bool DirectorEngine::processEvents(bool captureClick) {
	debugC(3, kDebugEvents, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
	debugC(3, kDebugEvents, "@@@@   Processing events");
	debugC(3, kDebugEvents, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		if (!_wm->processEvent(event)) {
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
	bool flag = MacWindow::processEvent(event);

	if (_currentMovie && _currentMovie->processEvent(event))
		flag = true;

	return flag;
}

bool Movie::processEvent(Common::Event &event) {
	Score *sc = getScore();
	if (sc->getCurrentFrame() >= sc->_frames.size()) {
		warning("processEvents: request to access frame %d of %d", sc->getCurrentFrame(), sc->_frames.size() - 1);
		return false;
	}
	uint16 spriteId = 0;

	Common::Point pos;

	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		pos = _window->getMousePos();

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
		if (_currentHiliteChannelId && !sc->_channels[_currentHiliteChannelId]->isMouseIn(pos)) {
			g_director->getCurrentWindow()->setDirty(true);
			g_director->getCurrentWindow()->addDirtyRect(sc->_channels[_currentHiliteChannelId]->getBbox());
			_currentHiliteChannelId = 0;
		}

		// for the list style button, we still have chance to trigger events though button.
		if (!(g_director->_wm->_mode & Graphics::kWMModeButtonDialogStyle) && g_director->_wm->_mouseDown && g_director->_wm->_hilitingWidget) {
			if (g_director->getVersion() < 400)
				spriteId = sc->getActiveSpriteIDFromPos(pos);
			else
				spriteId = sc->getMouseSpriteIDFromPos(pos);

			if (spriteId > 0 && sc->_channels[spriteId]->_sprite->shouldHilite()) {
				_currentHiliteChannelId = spriteId;
				g_director->getCurrentWindow()->setDirty(true);
				g_director->getCurrentWindow()->addDirtyRect(sc->_channels[_currentHiliteChannelId]->getBbox());
			}
		}

		if (_currentDraggedChannel) {
			if (_currentDraggedChannel->_sprite->_moveable) {
				pos = _window->getMousePos();

				_currentDraggedChannel->addDelta(pos - _draggingSpritePos);
				_draggingSpritePos = pos;
			} else {
				_currentDraggedChannel = nullptr;
			}
		}
		return true;

	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN:
		if (sc->_waitForClick) {
			sc->_waitForClick = false;
			sc->renderCursor(_window->getMousePos(), true);
		} else {
			pos = _window->getMousePos();

			// D3 doesn't have both mouse up and down.
			// But we still want to know if the mouse is down for press effects.
			// Since we don't have mouse up and down before D3, then we use ActiveSprite
			if (g_director->getVersion() < 400)
				spriteId = sc->getActiveSpriteIDFromPos(pos);
			else
				spriteId = sc->getMouseSpriteIDFromPos(pos);

			// Set `the clickOn` Lingo property.
			// Even in D4, `the clickOn` uses the old "active" sprite instead of mouse sprite.
			_currentClickOnSpriteId = sc->getActiveSpriteIDFromPos(pos);

			if (!spriteId && _isBeepOn) {
				g_lingo->func_beep(1);
			}

			if (spriteId > 0 && sc->_channels[spriteId]->_sprite->shouldHilite()) {
				_currentHiliteChannelId = spriteId;
				g_director->_wm->_hilitingWidget = true;
				g_director->getCurrentWindow()->setDirty(true);
				g_director->getCurrentWindow()->addDirtyRect(sc->_channels[_currentHiliteChannelId]->getBbox());
			}

			CastMember *cast = getCastMember(sc->_channels[spriteId]->_sprite->_castId);
			if (cast && cast->_type == kCastButton)
				_mouseDownWasInButton = true;

			_lastEventTime = g_director->getMacTicks();
			_lastClickTime2 = _lastClickTime;
			_lastClickTime = _lastEventTime;
			_lastClickPos = pos;
			if (_timeOutMouse)
				_lastTimeOut = _lastEventTime;

			debugC(3, kDebugEvents, "event: Button Down @(%d, %d), movie '%s', sprite id: %d", pos.x, pos.y, _macName.c_str(), spriteId);
			queueUserEvent(kEventMouseDown, spriteId);

			if (sc->_channels[spriteId]->_sprite->_moveable) {
				_draggingSpritePos = _window->getMousePos();
				_currentDraggedChannel = sc->_channels[spriteId];
			}
		}

		return true;

	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONUP:
		pos = _window->getMousePos();

		if (g_director->getVersion() < 400)
			spriteId = sc->getActiveSpriteIDFromPos(pos);
		else
			spriteId = sc->getMouseSpriteIDFromPos(pos);

		if (_currentHiliteChannelId && sc->_channels[_currentHiliteChannelId]) {
			g_director->getCurrentWindow()->setDirty(true);
			g_director->getCurrentWindow()->addDirtyRect(sc->_channels[_currentHiliteChannelId]->getBbox());
		}

		g_director->_wm->_hilitingWidget = false;

		debugC(3, kDebugEvents, "event: Button Up @(%d, %d), movie '%s', sprite id: %d", pos.x, pos.y, _macName.c_str(), spriteId);

		_currentDraggedChannel = nullptr;

		// If this is a button cast member, and the last mouse down event was in a button
		// (any button), flip this button's hilite flag.
		// Now you might think, "Wait, we don't flip this flag in the mouseDown event.
		// And why any button??? This doesn't make any sense."
		// No, it doesn't make sense, but it's what Director does.
		if (_mouseDownWasInButton) {
			CastMember *cast = getCastMember(sc->_channels[spriteId]->_sprite->_castId);
			if (cast && cast->_type == kCastButton)
				cast->_hilite = !cast->_hilite;
		}

		queueUserEvent(kEventMouseUp, spriteId);
		sc->renderCursor(pos);

		_currentHiliteChannelId = 0;
		_mouseDownWasInButton = false;
		return true;

	case Common::EVENT_KEYDOWN:
		_keyCode = _vm->_KeyCodes.contains(event.kbd.keycode) ? _vm->_KeyCodes[event.kbd.keycode] : 0;
		_key = (unsigned char)(event.kbd.ascii & 0xff);
		_keyFlags = event.kbd.flags;

		debugC(1, kDebugEvents, "processEvents(): movie '%s', keycode: %d", _macName.c_str(), _keyCode);

		_lastEventTime = g_director->getMacTicks();
		_lastKeyTime = _lastEventTime;
		if (_timeOutKeyDown)
			_lastTimeOut = _lastEventTime;

		queueUserEvent(kEventKeyDown);
		return true;

	case Common::EVENT_KEYUP:
		_keyFlags = event.kbd.flags;
		return true;

	default:
		break;
	}

	return false;
}

} // End of namespace Director
