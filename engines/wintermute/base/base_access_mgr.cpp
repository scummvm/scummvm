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

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "engines/wintermute/base/base.h"
#include "engines/wintermute/base/base_access_mgr.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/utils/string_util.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/dcgf.h"

#include "common/text-to-speech.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
BaseAccessMgr::BaseAccessMgr(BaseGame *inGame) : BaseClass(inGame) {
	_voice = nullptr;
	_ttsAvailable = false;
	_activeObject = nullptr;
	_prevActiveObject = nullptr;
	BasePlatform::setRectEmpty(&_hintRect);
	_hintAfterGUI = false;
	_playingType = TTS_CAPTION;
	_ctrlPressed = false;
}

//////////////////////////////////////////////////////////////////////////
BaseAccessMgr::~BaseAccessMgr() {
	_activeObject = nullptr; // ref only
	_ttsAvailable = false;
}

//////////////////////////////////////////////////////////////////////////
bool BaseAccessMgr::initialize() {
	_ttsAvailable = false;

	if (!_game->_accessTTSEnabled)
		return true;

	_voice = g_system->getTextToSpeechManager();
	if (_voice) {
		_ttsAvailable = true;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BaseAccessMgr::initLoop() {
	BasePlatform::setRectEmpty(&_hintRect);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BaseAccessMgr::displayBeforeGUI() {
	if (!_game->_accessKeyboardEnabled)
		return true;

	if (!_hintAfterGUI) {
		return displayInternal();
	} else {
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
bool BaseAccessMgr::displayAfterGUI() {
	if (!_game->_accessKeyboardEnabled)
		return true;

	if (_hintAfterGUI) {
		return displayInternal();
	} else {
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
bool BaseAccessMgr::displayInternal() {
	if (!_ctrlPressed) {
		return true;
	}

	if (!BasePlatform::isRectEmpty(&_hintRect)) {
		_game->_renderer->drawRect(_hintRect.left, _hintRect.top, _hintRect.right, _hintRect.bottom, 0xFFFF0000, 4);

		// reposition mouse pointer
		if (_game->_accessKeyboardCursorSkip && _activeObject != _prevActiveObject) {
			_prevActiveObject = _activeObject;
			Common::Point32 p;
			p.x = _hintRect.left + (_hintRect.right - _hintRect.left) / 2;
			p.y = _hintRect.top + (_hintRect.bottom - _hintRect.top) / 2;

			p.x += _game->_renderer->_drawOffsetX;
			p.y += _game->_renderer->_drawOffsetY;

			p.x = MAX<int32>(0, p.x);
			p.y = MAX<int32>(0, p.y);
			p.x = MIN<int32>(_game->_renderer->_width - 1, p.x);
			p.y = MIN<int32>(_game->_renderer->_height - 1, p.y);

			//ClientToScreen(Game->m_Renderer->m_Window, &p);
			BasePlatform::setCursorPos(p.x, p.y);
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BaseAccessMgr::setHintRect(Common::Rect32 *hintRect, bool afterGUI) {
	if (!hintRect) {
		BasePlatform::setRectEmpty(&_hintRect);
	} else {
		_hintRect = *hintRect;
	}

	_hintAfterGUI = afterGUI;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BaseAccessMgr::setActiveObject(BaseObject *activeObj) {
	if (!_game->_accessKeyboardEnabled) {
		return true;
	}

	_activeObject = activeObj;
	if (!activeObj) {
		_prevActiveObject = nullptr;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
BaseObject *BaseAccessMgr::getActiveObject() {
	return _activeObject;
}

//////////////////////////////////////////////////////////////////////////
bool BaseAccessMgr::speak(const char *str, TTTSType type) {
	if (!_ttsAvailable) {
		return true;
	}
	if (type == TTS_CAPTION && !_game->_accessTTSCaptions) {
		return true;
	}
	if (type == TTS_TALK && !_game->_accessTTSTalk) {
		return true;
	}
	if (type == TTS_KEYPRESS && !_game->_accessTTSKeypress) {
		return true;
	}

	if (!str || !str[0]) {
		return true;
	}

	WideString textStr;
	if (_game->_textEncoding == TEXT_UTF8) {
		textStr = StringUtil::utf8ToWide(str);
	} else {
		textStr = StringUtil::ansiToWide(str, CHARSET_DEFAULT);
	}

	if (!textStr.empty()) {
		_playingType = type;
		return _voice->say(str);
	} else {
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
bool BaseAccessMgr::stop() {
	if (!_ttsAvailable) {
		return true;
	}
	_playingType = TTS_CAPTION;
	return _voice->stop();
}

//////////////////////////////////////////////////////////////////////////
BaseObject *BaseAccessMgr::getNextObject() {
	_activeObject = _game->getNextAccessObject(_activeObject);
	return _activeObject;
}

//////////////////////////////////////////////////////////////////////////
BaseObject *BaseAccessMgr::getPrevObject() {
	_activeObject = _game->getPrevAccessObject(_activeObject);
	return _activeObject;
}

} // End of namespace Wintermute
