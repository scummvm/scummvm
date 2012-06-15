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
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BRenderSDL.h"
#include "engines/wintermute/utils/PathUtil.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/Base/BRegistry.h"
#include "engines/wintermute/Base/BSoundMgr.h"
#include "engines/wintermute/Base/scriptables/ScEngine.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "common/system.h"

namespace WinterMute {

CBGame *CBPlatform::Game = NULL;

#define CLASS_NAME "GF_FRAME"
int CBPlatform::Initialize(CBGame *inGame, int argc, char *argv[]) {
	Game = inGame;
}

//////////////////////////////////////////////////////////////////////////
void CBPlatform::HandleEvent(Common::Event *event) {
	switch (event->type) {

	case Common::EVENT_LBUTTONDOWN:
		if (Game) {
			if (Game->IsLeftDoubleClick()) Game->OnMouseLeftDblClick();
			else Game->OnMouseLeftDown();
		}
		break;
	case Common::EVENT_RBUTTONDOWN:
		if (Game) {
			if (Game->IsRightDoubleClick()) Game->OnMouseRightDblClick();
			else Game->OnMouseRightDown();
		}
		break;
	case Common::EVENT_MBUTTONDOWN:
		if (Game) Game->OnMouseMiddleDown();
		break;
	case Common::EVENT_LBUTTONUP:
		if (Game) Game->OnMouseLeftUp();
		break;
	case Common::EVENT_RBUTTONUP:
		if (Game) Game->OnMouseRightUp();
		break;
	case Common::EVENT_MBUTTONUP:
		if (Game) Game->OnMouseMiddleUp();
		break;
	case Common::EVENT_KEYDOWN:
		if (Game) Game->HandleKeypress(event);
		break;
	case Common::EVENT_KEYUP:
		if (Game) Game->handleKeyRelease(event);
		break;
		/*#ifdef __IPHONEOS__
		        {
		            CBRenderSDL *renderer = static_cast<CBRenderSDL *>(Game->_renderer);
		            POINT p;
		            GetCursorPos(&p);
		            Game->SetActiveObject(renderer->GetObjectAt(p.x, p.y));

		            if (Game->_activeObject != NULL && strcmp(Game->_activeObject->GetClassName(), "CUIButton") == 0) {
		                CUIButton *btn = static_cast<CUIButton *>(Game->_activeObject);
		                if (btn->_visible && !btn->_disable) btn->_press = true;
		            }
		        }
		#endif*/

//TODO
		/*  case SDL_MOUSEWHEEL:
		        if (Game) Game->HandleMouseWheel(event->wheel.y);
		        break;

		    case SDL_KEYDOWN:
		    case SDL_TEXTINPUT:
		        if (Game) Game->HandleKeypress(event);
		        break;

		    case SDL_WINDOWEVENT:
		        switch (event->window.event) {
		        case SDL_WINDOWEVENT_FOCUS_GAINED:
		        case SDL_WINDOWEVENT_RESTORED:
		            if (Game) Game->OnActivate(true, true);
		            SDL_ShowCursor(SDL_DISABLE);
		            break;
		        case SDL_WINDOWEVENT_FOCUS_LOST:
		        case SDL_WINDOWEVENT_MINIMIZED:
		#ifndef __IPHONEOS__
		            if (Game) Game->OnActivate(false, false);
		            SDL_ShowCursor(SDL_ENABLE);
		#endif
		            break;

		        case SDL_WINDOWEVENT_CLOSE:
		            break;

		        }
		        break;
		*/
	case Common::EVENT_QUIT:
#ifdef __IPHONEOS__
		if (Game) {
			Game->AutoSaveOnExit();
			Game->_quitting = true;
		}
#else
		if (Game) Game->OnWindowClose();
#endif

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
	        if (Game) Game->AutoSaveOnExit();
	        if (Game) Game->OnActivate(false, false);
	        SDL_ShowCursor(SDL_ENABLE);
	    }
	*/
	return 1;
}


//////////////////////////////////////////////////////////////////////////
// Win32 API bindings
//////////////////////////////////////////////////////////////////////////
HINSTANCE CBPlatform::ShellExecute(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, int nShowCmd) {
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
void CBPlatform::OutputDebugString(LPCSTR lpOutputString) {
/*
#ifdef __WIN32__
	::OutputDebugString(lpOutputString);
#endif
*/
}


//////////////////////////////////////////////////////////////////////////
uint32 CBPlatform::GetTime() {
	return g_system->getMillis();
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::GetCursorPos(LPPOINT lpPoint) {
	CBRenderSDL *renderer = static_cast<CBRenderSDL *>(Game->_renderer);

	Common::Point p = g_system->getEventManager()->getMousePos();
	lpPoint->x = p.x;
	lpPoint->y = p.y;

	renderer->PointFromScreen(lpPoint);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::SetCursorPos(int X, int Y) {
	CBRenderSDL *renderer = static_cast<CBRenderSDL *>(Game->_renderer);

	POINT p;
	p.x = X;
	p.y = Y;
	renderer->PointToScreen(&p);
// TODO
	//SDL_WarpMouseInWindow(renderer->GetSdlWindow(), p.x, p.y);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::ShowWindow(HWND hWnd, int nCmdShow) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::DeleteFile(const char *lpFileName) {
	return remove(lpFileName) ? true : false;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::CopyFile(const char *from, const char *to, bool failIfExists) {
//	try {
	warning("CBPlatform::CopyFile(%s, %s, %d) - not implemented", from, to, failIfExists);
	return false;
//		if (failIfExists && boost::filesystem::exists(to)) return false;
//		boost::filesystem::copy_file(from, to);
//		return true;
//	} catch (...) {
//		return false;
//	}
}

//////////////////////////////////////////////////////////////////////////
HWND CBPlatform::SetCapture(HWND hWnd) {
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::ReleaseCapture() {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::SetForegroundWindow(HWND hWnd) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::SetRectEmpty(LPRECT lprc) {
	lprc->left = lprc->right = lprc->top = lprc->bottom = 0;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::IsRectEmpty(const LPRECT lprc) {
	return (lprc->left >= lprc->right) || (lprc->top >= lprc->bottom);
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::PtInRect(LPRECT lprc, POINT p) {
	return (p.x >= lprc->left) && (p.x < lprc->right) && (p.y >= lprc->top) && (p.y < lprc->bottom);
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::SetRect(LPRECT lprc, int left, int top, int right, int bottom) {
	lprc->left   = left;
	lprc->top    = top;
	lprc->right  = right;
	lprc->bottom = bottom;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::IntersectRect(LPRECT lprcDst, const LPRECT lprcSrc1, const LPRECT lprcSrc2) {
	if (IsRectEmpty(lprcSrc1) || IsRectEmpty(lprcSrc2) ||
	        lprcSrc1->left >= lprcSrc2->right || lprcSrc2->left >= lprcSrc1->right ||
	        lprcSrc1->top >= lprcSrc2->bottom || lprcSrc2->top >= lprcSrc1->bottom) {
		SetRectEmpty(lprcDst);
		return false;
	}
	lprcDst->left   = MAX(lprcSrc1->left, lprcSrc2->left);
	lprcDst->right  = MIN(lprcSrc1->right, lprcSrc2->right);
	lprcDst->top    = MAX(lprcSrc1->top, lprcSrc2->top);
	lprcDst->bottom = MIN(lprcSrc1->bottom, lprcSrc2->bottom);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::UnionRect(LPRECT lprcDst, RECT *lprcSrc1, RECT *lprcSrc2) {
	if (IsRectEmpty(lprcSrc1)) {
		if (IsRectEmpty(lprcSrc2)) {
			SetRectEmpty(lprcDst);
			return false;
		} else {
			*lprcDst = *lprcSrc2;
		}
	} else {
		if (IsRectEmpty(lprcSrc2)) {
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
bool CBPlatform::CopyRect(LPRECT lprcDst, RECT *lprcSrc) {
	if (lprcDst == NULL || lprcSrc == NULL) return false;

	*lprcDst = *lprcSrc;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::OffsetRect(LPRECT lprc, int dx, int dy) {
	if (lprc == NULL) return false;

	lprc->left   += dx;
	lprc->top    += dy;
	lprc->right  += dx;
	lprc->bottom += dy;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::EqualRect(LPRECT rect1, LPRECT rect2) {
	return rect1->left == rect2->left && rect1->right == rect2->right && rect1->top == rect2->top && rect1->bottom == rect2->bottom;
}


//////////////////////////////////////////////////////////////////////////
AnsiString CBPlatform::GetSystemFontPath() {
#ifdef __WIN32__
	// we're looking for something like "c:\windows\fonts\";
	char winDir[MAX_PATH + 1];
	winDir[MAX_PATH] = '\0';
	::GetWindowsDirectory(winDir, MAX_PATH);
	return PathUtil::Combine(AnsiString(winDir), "fonts");
#else
	// !PORTME
	//return "/Library/Fonts/";
	return "";
#endif
}

//////////////////////////////////////////////////////////////////////////
AnsiString CBPlatform::GetPlatformName() {
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
