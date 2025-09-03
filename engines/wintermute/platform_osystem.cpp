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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/osystem/base_render_osystem.h"
#include "engines/wintermute/platform_osystem.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "common/system.h"

namespace Wintermute {

BaseGame *BasePlatform::_game = nullptr;
WintermuteEngine *BasePlatform::_engineRef = nullptr;

#define CLASS_NAME "GF_FRAME"
int BasePlatform::initialize(WintermuteEngine *engineRef, BaseGame *inGame, int argc, char *argv[]) {
	_game = inGame;
	_engineRef = engineRef;
	return true;
}

void BasePlatform::deinit() {
	_game = nullptr;
	_engineRef = nullptr;
}

//////////////////////////////////////////////////////////////////////////
void BasePlatform::handleEvent(Common::Event *event) {
	switch (event->type) {

	case Common::EVENT_LBUTTONDOWN:
		if (_game) {
			if (_game->isLeftDoubleClick()) {
				_game->onMouseLeftDblClick();
			} else {
				_game->onMouseLeftDown();
			}
		}
		break;
	case Common::EVENT_RBUTTONDOWN:
		if (_game) {
			if (_game->isRightDoubleClick()) {
				_game->onMouseRightDblClick();
			} else {
				_game->onMouseRightDown();
			}
		}
		break;
	case Common::EVENT_MBUTTONDOWN:
		if (_game) {
			_game->onMouseMiddleDown();
		}
		break;
	case Common::EVENT_LBUTTONUP:
		if (_game) {
			_game->onMouseLeftUp();
		}
		break;
	case Common::EVENT_RBUTTONUP:
		if (_game) {
			_game->onMouseRightUp();
		}
		break;
	case Common::EVENT_MBUTTONUP:
		if (_game) {
			_game->onMouseMiddleUp();
		}
		break;
	case Common::EVENT_KEYDOWN:
		if (_game) {
			_game->handleKeypress(event);
		}
		break;
	case Common::EVENT_KEYUP:
		if (_game) {
			_game->handleKeyRelease(event);
		}
		break;
	case Common::EVENT_WHEELUP:
	case Common::EVENT_WHEELDOWN:
		if (_game) {
			_game->handleMouseWheel(event->type == Common::EVENT_WHEELUP ? 1 : -1);
		}
		break;
	case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
		if (_game) {
			_game->handleCustomActionStart((BaseGameCustomAction)event->customType);
		}
		break;
	case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
		if (_game) {
			_game->handleCustomActionEnd((BaseGameCustomAction)event->customType);
		}
		break;
	case Common::EVENT_SCREEN_CHANGED:
		if (_game) {
			_game->_renderer->onWindowChange();
		}
		break;
// Focus-events have been removed (_game->onActivate originally)
	case Common::EVENT_RETURN_TO_LAUNCHER:
		_game->_quitting = true;
		break;
	case Common::EVENT_QUIT:
// Block kept in case we want to support autoSaveOnExit.
// Originally this was the behaviour for WME Lite on iOS:
//		if (_game) {
//			_game->AutoSaveOnExit();
//			_game->_quitting = true;
//		}

// The engine CAN query for closing, but we disable it for now, as the EVENT_QUIT-event
// can't be stopped.
//		if (_game) {
//			_game->onWindowClose();
//		}
		break;
	default:
		// TODO: Do we care about any other events?
		break;

	}
}


//////////////////////////////////////////////////////////////////////////
// Win32 API bindings
//////////////////////////////////////////////////////////////////////////
bool BasePlatform::getCursorPos(Common::Point32 *lpPoint) {
	Common::Point p = g_system->getEventManager()->getMousePos();
	lpPoint->x = p.x;
	lpPoint->y = p.y;

	// in 3d mode we take the mouse position as is for now
	// this seems to give the right results
	// actually, BaseRenderer has no functions pointFromScreen/pointToScreen anyways
#ifndef ENABLE_WME3D
	if (!_game->_useD3D) {
		BaseRenderOSystem *renderer = static_cast<BaseRenderOSystem *>(_game->_renderer);
		renderer->pointFromScreen(lpPoint);
	}
#endif

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BasePlatform::setCursorPos(int x, int y) {
	Common::Point32 p;
	p.x = x;
	p.y = y;

	// in 3d mode we take the mouse position as is for now
	// this seems to give the right results
	// actually, BaseRenderer has no functions pointFromScreen/pointToScreen anyways
#ifndef ENABLE_WME3D
	if (!_game->_useD3D) {
		BaseRenderOSystem *renderer = static_cast<BaseRenderOSystem *>(_game->_renderer);
		renderer->pointToScreen(&p);
	}
#endif

	g_system->warpMouse(x, y);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BasePlatform::setRectEmpty(Common::Rect32 *lprc) {
	if (lprc == nullptr) {
		return false;
	}
	lprc->left = lprc->right = lprc->top = lprc->bottom = 0;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BasePlatform::isRectEmpty(const Common::Rect32 *lprc) {
	if (lprc == nullptr) {
		return false;
	}
	return ((lprc->left >= lprc->right) || (lprc->top >= lprc->bottom));
}

//////////////////////////////////////////////////////////////////////////
bool BasePlatform::ptInRect(Common::Rect32 *lprc, Common::Point32 p) {
	if (lprc == nullptr) {
		return false;
	}
	return (p.x >= lprc->left) && (p.x < lprc->right) && (p.y >= lprc->top) && (p.y < lprc->bottom);
}

//////////////////////////////////////////////////////////////////////////
bool BasePlatform::setRect(Common::Rect32 *lprc, int32 left, int32 top, int32 right, int32 bottom) {
	if (lprc == nullptr) {
		return false;
	}

	lprc->left   = left;
	lprc->right  = right;
	lprc->top    = top;
	lprc->bottom = bottom;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BasePlatform::intersectRect(Common::Rect32 *lprcDst, const Common::Rect32 *lprcSrc1, const Common::Rect32 *lprcSrc2) {
	if (lprcDst == nullptr || lprcSrc1 == nullptr || lprcSrc2 == nullptr) {
		return false;
	}

	if (BasePlatform::isRectEmpty(lprcSrc1) || BasePlatform::isRectEmpty(lprcSrc2) ||
		lprcSrc1->left >= lprcSrc2->right || lprcSrc2->left >= lprcSrc1->right ||
		lprcSrc1->top >= lprcSrc2->bottom || lprcSrc2->top >= lprcSrc1->bottom) {
		setRectEmpty(lprcDst);
		return false;
	}
	lprcDst->left   = MAX(lprcSrc1->left, lprcSrc2->left);
	lprcDst->right  = MIN(lprcSrc1->right, lprcSrc2->right);
	lprcDst->top    = MAX(lprcSrc1->top, lprcSrc2->top);
	lprcDst->bottom = MIN(lprcSrc1->bottom, lprcSrc2->bottom);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BasePlatform::unionRect(Common::Rect32 *lprcDst, const Common::Rect32 *lprcSrc1, const Common::Rect32 *lprcSrc2) {
	if (lprcDst == nullptr || lprcSrc1 == nullptr || lprcSrc2 == nullptr) {
		return false;
	}

	if (isRectEmpty(lprcSrc1)) {
		if (isRectEmpty(lprcSrc2)) {
			setRectEmpty(lprcDst);
			return false;
		} else {
			*lprcDst = *lprcSrc2;
		}
	} else {
		if (isRectEmpty(lprcSrc2)) {
			*lprcDst = *lprcSrc1;
		} else {
			lprcDst->top    = MIN(lprcSrc1->top, lprcSrc2->top);
			lprcDst->left   = MIN(lprcSrc1->left, lprcSrc2->left);
			lprcDst->bottom = MAX(lprcSrc1->bottom, lprcSrc2->bottom);
			lprcDst->right  = MAX(lprcSrc1->right, lprcSrc2->right);
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BasePlatform::copyRect(Common::Rect32 *lprcDst, const Common::Rect32 *lprcSrc) {
	if (lprcDst == nullptr || lprcSrc == nullptr) {
		return false;
	}

	*lprcDst = *lprcSrc;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BasePlatform::equalRect(const Common::Rect32 *lprc1, const Common::Rect32 *lprc2) {
	if (lprc1 == nullptr || lprc2 == nullptr) {
		return false;
	}

	return ((lprc1->left == lprc2->left) && (lprc1->right == lprc2->right) &&
			(lprc1->top == lprc2->top) && (lprc1->bottom == lprc2->bottom));
}

//////////////////////////////////////////////////////////////////////////
bool BasePlatform::offsetRect(Common::Rect32 *lprc, int32 x, int32 y) {
	if (!lprc)
		return false;
	lprc->left   += x;
	lprc->right  += x;
	lprc->top    += y;
	lprc->bottom += y;
	return true;
}

//////////////////////////////////////////////////////////////////////////
AnsiString BasePlatform::getPlatformName() {
	// TODO: Should conform to the WME-spec.
	//return AnsiString(SDL_GetPlatform());
	return AnsiString("ScummVM");
}

//////////////////////////////////////////////////////////////////////////
char *BasePlatform::wintermute_strlwr(char *string) {
	if (string) {
		for (size_t i = 0; i < strlen(string); ++i) {
			string[i] = tolower(string[i]);
		}
	}
	return string;
}

} // End of namespace Wintermute
