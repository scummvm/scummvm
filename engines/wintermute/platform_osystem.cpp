/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/base/BGame.h"
#include "engines/wintermute/base/gfx/osystem/base_render_osystem.h"
#include "engines/wintermute/utils/PathUtil.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/base/BRegistry.h"
#include "engines/wintermute/base/BSoundMgr.h"
#include "engines/wintermute/base/scriptables/ScEngine.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "common/system.h"

namespace WinterMute {

CBGame *CBPlatform::_gameRef = NULL;

#define CLASS_NAME "GF_FRAME"
int CBPlatform::initialize(CBGame *inGame, int argc, char *argv[]) {
	_gameRef = inGame;
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CBPlatform::handleEvent(Common::Event *event) {
	switch (event->type) {

	case Common::EVENT_LBUTTONDOWN:
		if (_gameRef) {
			if (_gameRef->isLeftDoubleClick()) _gameRef->onMouseLeftDblClick();
			else _gameRef->onMouseLeftDown();
		}
		break;
	case Common::EVENT_RBUTTONDOWN:
		if (_gameRef) {
			if (_gameRef->isRightDoubleClick()) _gameRef->onMouseRightDblClick();
			else _gameRef->onMouseRightDown();
		}
		break;
	case Common::EVENT_MBUTTONDOWN:
		if (_gameRef) _gameRef->onMouseMiddleDown();
		break;
	case Common::EVENT_LBUTTONUP:
		if (_gameRef) _gameRef->onMouseLeftUp();
		break;
	case Common::EVENT_RBUTTONUP:
		if (_gameRef) _gameRef->onMouseRightUp();
		break;
	case Common::EVENT_MBUTTONUP:
		if (_gameRef) _gameRef->onMouseMiddleUp();
		break;
	case Common::EVENT_KEYDOWN:
		if (_gameRef) _gameRef->handleKeypress(event);
		break;
	case Common::EVENT_KEYUP:
		if (_gameRef) _gameRef->handleKeyRelease(event);
		break;
	case Common::EVENT_WHEELUP:
	case Common::EVENT_WHEELDOWN:
		if (_gameRef) _gameRef->handleMouseWheel(event->mouse.y);
		break;
		/*#ifdef __IPHONEOS__
		        {
		            CBRenderOSystem *renderer = static_cast<CBRenderOSystem *>(_gameRef->_renderer);
		            POINT p;
		            GetCursorPos(&p);
		            _gameRef->SetActiveObject(renderer->GetObjectAt(p.x, p.y));

		            if (_gameRef->_activeObject != NULL && strcmp(_gameRef->_activeObject->getClassName(), "CUIButton") == 0) {
		                CUIButton *btn = static_cast<CUIButton *>(_gameRef->_activeObject);
		                if (btn->_visible && !btn->_disable) btn->_press = true;
		            }
		        }
		#endif*/

//TODO
		/*  case SDL_MOUSEWHEEL:
		        if (_gameRef) _gameRef->handleMouseWheel(event->wheel.y);
		        break;

		    case SDL_KEYDOWN:
		    case SDL_TEXTINPUT:
		        if (_gameRef) _gameRef->handleKeypress(event);
		        break;

		    case SDL_WINDOWEVENT:
		        switch (event->window.event) {
		        case SDL_WINDOWEVENT_FOCUS_GAINED:
		        case SDL_WINDOWEVENT_RESTORED:
		            if (_gameRef) _gameRef->OnActivate(true, true);
		            SDL_ShowCursor(SDL_DISABLE);
		            break;
		        case SDL_WINDOWEVENT_FOCUS_LOST:
		        case SDL_WINDOWEVENT_MINIMIZED:
		#ifndef __IPHONEOS__
		            if (_gameRef) _gameRef->OnActivate(false, false);
		            SDL_ShowCursor(SDL_ENABLE);
		#endif
		            break;

		        case SDL_WINDOWEVENT_CLOSE:
		            break;

		        }
		        break;
		*/
	case Common::EVENT_QUIT:
	case Common::EVENT_RTL:
/*#ifdef __IPHONEOS__
		if (_gameRef) {
			_gameRef->AutoSaveOnExit();
			_gameRef->_quitting = true;
		}
#else*/
		if (_gameRef) _gameRef->onWindowClose();
//#endif

		break;
	default:
		// TODO: Do we care about any other events?
		break;

	}
}

//////////////////////////////////////////////////////////////////////////
int CBPlatform::SDLEventWatcher(void *userdata, Common::Event *event) {
	//TODO
	/*  if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_MINIMIZED) {
	        if (_gameRef) _gameRef->AutoSaveOnExit();
	        if (_gameRef) _gameRef->OnActivate(false, false);
	        SDL_ShowCursor(SDL_ENABLE);
	    }
	*/
	return 1;
}


//////////////////////////////////////////////////////////////////////////
// Win32 API bindings
//////////////////////////////////////////////////////////////////////////
void CBPlatform::outputDebugString(const char *lpOutputString) {
/*
#ifdef __WIN32__
	::OutputDebugString(lpOutputString);
#endif
*/
}


//////////////////////////////////////////////////////////////////////////
uint32 CBPlatform::getTime() {
	return g_system->getMillis();
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::getCursorPos(Point32 *lpPoint) {
	CBRenderOSystem *renderer = static_cast<CBRenderOSystem *>(_gameRef->_renderer);

	Common::Point p = g_system->getEventManager()->getMousePos();
	lpPoint->x = p.x;
	lpPoint->y = p.y;

	renderer->pointFromScreen(lpPoint);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::setCursorPos(int X, int Y) {
	CBRenderOSystem *renderer = static_cast<CBRenderOSystem *>(_gameRef->_renderer);

	Point32 p;
	p.x = X;
	p.y = Y;
	renderer->pointToScreen(&p);
// TODO
	//SDL_WarpMouseInWindow(renderer->GetSdlWindow(), p.x, p.y);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::showWindow(int nCmdShow) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::deleteFile(const char *lpFileName) {
	return remove(lpFileName) ? true : false;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::copyFile(const char *from, const char *to, bool failIfExists) {
//	try {
	warning("CBPlatform::copyFile(%s, %s, %d) - not implemented", from, to, failIfExists);
	return false;
//		if (failIfExists && boost::filesystem::exists(to)) return false;
//		boost::filesystem::copy_file(from, to);
//		return true;
//	} catch (...) {
//		return false;
//	}
}

//////////////////////////////////////////////////////////////////////////
void CBPlatform::setCapture() {
	return;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::releaseCapture() {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::setForegroundWindow() {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::setRectEmpty(Rect32 *lprc) {
	lprc->left = lprc->right = lprc->top = lprc->bottom = 0;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::isRectEmpty(const Rect32 *lprc) {
	return (lprc->left >= lprc->right) || (lprc->top >= lprc->bottom);
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::ptInRect(Rect32 *lprc, Point32 p) {
	return (p.x >= lprc->left) && (p.x < lprc->right) && (p.y >= lprc->top) && (p.y < lprc->bottom);
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::setRect(Rect32 *lprc, int left, int top, int right, int bottom) {
	lprc->left   = left;
	lprc->top    = top;
	lprc->right  = right;
	lprc->bottom = bottom;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::intersectRect(Rect32 *lprcDst, const Rect32 *lprcSrc1, const Rect32 *lprcSrc2) {
	if (isRectEmpty(lprcSrc1) || isRectEmpty(lprcSrc2) ||
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
bool CBPlatform::unionRect(Rect32 *lprcDst, Rect32 *lprcSrc1, Rect32 *lprcSrc2) {
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
			lprcDst->left   = MIN(lprcSrc1->left, lprcSrc2->left);
			lprcDst->top    = MIN(lprcSrc1->top, lprcSrc2->top);
			lprcDst->right  = MAX(lprcSrc1->right, lprcSrc2->right);
			lprcDst->bottom = MAX(lprcSrc1->bottom, lprcSrc2->bottom);
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::copyRect(Rect32 *lprcDst, Rect32 *lprcSrc) {
	if (lprcDst == NULL || lprcSrc == NULL) return false;

	*lprcDst = *lprcSrc;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::offsetRect(Rect32 *lprc, int dx, int dy) {
	if (lprc == NULL) return false;

	lprc->left   += dx;
	lprc->top    += dy;
	lprc->right  += dx;
	lprc->bottom += dy;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::equalRect(Rect32 *rect1, Rect32 *rect2) {
	return rect1->left == rect2->left && rect1->right == rect2->right && rect1->top == rect2->top && rect1->bottom == rect2->bottom;
}


//////////////////////////////////////////////////////////////////////////
AnsiString CBPlatform::getSystemFontPath() {
/*#ifdef __WIN32__
	// we're looking for something like "c:\windows\fonts\";
	char winDir[MAX_PATH_LENGTH + 1];
	winDir[MAX_PATH_LENGTH] = '\0';
	::GetWindowsDirectory(winDir, MAX_PATH_LENGTH);
	return PathUtil::Combine(AnsiString(winDir), "fonts");
#else*/
	// !PORTME
	//return "/Library/Fonts/";
	return "";
//#endif
}

//////////////////////////////////////////////////////////////////////////
AnsiString CBPlatform::getPlatformName() {
	// TODO: Should conform to the WME-spec.
	//return AnsiString(SDL_GetPlatform());
	return AnsiString("ScummVM");
}

//////////////////////////////////////////////////////////////////////////
char *CBPlatform::strupr(char *string) {
	if (string) {
		for (size_t i = 0; i < strlen(string); ++i) {
			string[i] = toupper(string[i]);
		}
	}
	return string;
}

//////////////////////////////////////////////////////////////////////////
char *CBPlatform::strlwr(char *string) {
	if (string) {
		for (size_t i = 0; i < strlen(string); ++i) {
			string[i] = tolower(string[i]);
		}
	}
	return string;
}

} // end of namespace WinterMute
