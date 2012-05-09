/*
This file is part of WME Lite.
http://dead-code.org/redir.php?target=wmelite

Copyright (c) 2011 Jan Nedoma

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/BRenderSDL.h"
#include "engines/wintermute/PathUtil.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/BRegistry.h"
#include "engines/wintermute/BSoundMgr.h"
#include "engines/wintermute/scriptables/ScEngine.h"
#include "common/str.h"
#include "common/textconsole.h"

#include "SDL.h" // TODO remove

#ifdef __WIN32__
#   include <dbghelp.h>
#   include <direct.h>
#else
#   include <unistd.h>
#endif


namespace WinterMute {

CBGame *CBPlatform::Game = NULL;


#define CLASS_NAME "GF_FRAME"
//////////////////////////////////////////////////////////////////////////
int CBPlatform::Initialize(CBGame *inGame, int argc, char *argv[]) {
	//setlocale(LC_CTYPE, "");

	Game = inGame;
	if (!Game) return 1;


	bool windowedMode = true;


	// parse command line
	char *SaveGame = NULL;
	char param[MAX_PATH];
	for (int i = 0; i < argc; i++) {
		strcpy(param, argv[i]);

		if (scumm_stricmp(param, "-project") == 0) {
			if (argc > i) strcpy(param, argv[i + 1]);
			else param[0] = '\0';

			if (strcmp(param, "") != 0) {
				char *IniDir = CBUtils::GetPath(param);
				char *IniName = CBUtils::GetFilename(param);

				// switch to ini's dir
				chdir(IniDir);

				// set ini name
				sprintf(param, "./%s", IniName);
				Game->_registry->SetIniName(param);

				delete [] IniDir;
				delete [] IniName;
			}
		} else if (scumm_stricmp(param, "-windowed") == 0) windowedMode = true;
	}


	if (Game->_registry->ReadBool("Debug", "DebugMode")) Game->DEBUG_DebugEnable("./wme.log");

	Game->_dEBUG_ShowFPS = Game->_registry->ReadBool("Debug", "ShowFPS");

	if (Game->_registry->ReadBool("Debug", "DisableSmartCache")) {
		Game->LOG(0, "Smart cache is DISABLED");
		Game->_smartCache = false;
	}

	bool AllowDirectDraw = Game->_registry->ReadBool("Debug", "AllowDirectDraw", false);

	// load general game settings
	Game->Initialize1();


	if (FAILED(Game->LoadSettings("startup.settings"))) {
		Game->LOG(0, "Error loading game settings.");
		delete Game;
		Game = NULL;

#ifdef __WIN32__
		::MessageBox(NULL, "Some of the essential files are missing. Please reinstall.", NULL, MB_OK | MB_ICONERROR);
#endif
		return 2;
	}

	Game->Initialize2();

	Game->GetDebugMgr()->OnGameInit();
	Game->_scEngine->LoadBreakpoints();



	HRESULT ret;

	// initialize the renderer
	ret = Game->_renderer->InitRenderer(Game->_settingsResWidth, Game->_settingsResHeight, windowedMode);
	if (FAILED(ret)) {
		Game->LOG(ret, "Error initializing renderer. Exiting.");

		delete Game;
		Game = NULL;
		return 3;
	}

	Game->Initialize3();

#ifdef __IPHONEOS__
	SDL_AddEventWatch(CBPlatform::SDLEventWatcher, NULL);
#endif

	// initialize sound manager (non-fatal if we fail)
	ret = Game->_soundMgr->Initialize();
	if (FAILED(ret)) {
		Game->LOG(ret, "Sound is NOT available.");
	}


	// load game
	uint32 DataInitStart = GetTime();

	if (FAILED(Game->LoadFile(Game->_settingsGameFile ? Game->_settingsGameFile : "default.game"))) {
		Game->LOG(ret, "Error loading game file. Exiting.");
		delete Game;
		Game = NULL;
		return false;
	}
	Game->SetWindowTitle();
	Game->_renderer->_ready = true;
	Game->_miniUpdateEnabled = true;

	Game->LOG(0, "Engine initialized in %d ms", GetTime() - DataInitStart);
	Game->LOG(0, "");


	if (SaveGame) {
		Game->LoadGame(SaveGame);
		delete [] SaveGame;
	}

	// all set, ready to go
	return 0;
}


//////////////////////////////////////////////////////////////////////////
int CBPlatform::MessageLoop() {
	bool done = false;

	while (!done) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			HandleEvent(&event);
		}

		if (Game && Game->_renderer->_active && Game->_renderer->_ready) {

			Game->DisplayContent();
			Game->DisplayQuickMsg();

			Game->DisplayDebugInfo();

			// ***** flip
			if (!Game->_suspendedRendering) Game->_renderer->Flip();
			if (Game->_loading) Game->LoadGame(Game->_scheduledLoadSlot);
		}
		if (Game->_quitting) break;

	}

	if (Game) {
		// remember previous window position
		/*
		if(Game->_renderer && Game->_renderer->_windowed)
		{
		    if(!::IsIconic(Game->_renderer->_window))
		    {
		        int PosX = Game->_renderer->_windowRect.left;
		        int PosY = Game->_renderer->_windowRect.top;
		        PosX -= Game->_renderer->_monitorRect.left;
		        PosY -= Game->_renderer->_monitorRect.top;

		        Game->_registry->WriteInt("Video", "WindowPosX", PosX);
		        Game->_registry->WriteInt("Video", "WindowPosY", PosY);
		    }
		}
		*/

		delete Game;
		Game = NULL;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CBPlatform::HandleEvent(SDL_Event *event) {
	switch (event->type) {

	case SDL_MOUSEBUTTONDOWN:

#ifdef __IPHONEOS__
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
#endif
		switch (event->button.button) {
		case SDL_BUTTON_LEFT:
			if (Game) {
				if (Game->IsLeftDoubleClick()) Game->OnMouseLeftDblClick();
				else Game->OnMouseLeftDown();
			}
			break;
		case SDL_BUTTON_RIGHT:
			if (Game) {
				if (Game->IsRightDoubleClick()) Game->OnMouseRightDblClick();
				else Game->OnMouseRightDown();
			}
			break;
		case SDL_BUTTON_MIDDLE:
			if (Game) Game->OnMouseMiddleDown();
			break;
		}
		break;

	case SDL_MOUSEBUTTONUP:
		switch (event->button.button) {
		case SDL_BUTTON_LEFT:
			if (Game) Game->OnMouseLeftUp();
			break;
		case SDL_BUTTON_RIGHT:
			if (Game) Game->OnMouseRightUp();
			break;
		case SDL_BUTTON_MIDDLE:
			if (Game) Game->OnMouseMiddleUp();
			break;
		}
		break;
//TODO
/*	case SDL_MOUSEWHEEL:
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
	case SDL_QUIT:
#ifdef __IPHONEOS__
		if (Game) {
			Game->AutoSaveOnExit();
			Game->_quitting = true;
		}
#else
		if (Game) Game->OnWindowClose();
#endif

		break;

	}
}

//////////////////////////////////////////////////////////////////////////
int CBPlatform::SDLEventWatcher(void *userdata, SDL_Event *event) {
	//TODO
/*	if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_MINIMIZED) {
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
HINSTANCE CBPlatform::ShellExecute(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd) {
#ifdef __WIN32__
	return ::ShellExecute(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
#else
	return NULL;
#endif
}

//////////////////////////////////////////////////////////////////////////
void CBPlatform::OutputDebugString(LPCSTR lpOutputString) {
#ifdef __WIN32__
	::OutputDebugString(lpOutputString);
#endif
}


//////////////////////////////////////////////////////////////////////////
uint32 CBPlatform::GetTime() {
	return SDL_GetTicks();
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::GetCursorPos(LPPOINT lpPoint) {
	CBRenderSDL *renderer = static_cast<CBRenderSDL *>(Game->_renderer);

	int x, y;
	SDL_GetMouseState(&x, &y);
	lpPoint->x = x;
	lpPoint->y = y;

	renderer->PointFromScreen(lpPoint);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::SetCursorPos(int X, int Y) {
	CBRenderSDL *renderer = static_cast<CBRenderSDL *>(Game->_renderer);

	POINT p;
	p.x = X;
	p.y = Y;
	renderer->PointToScreen(&p);
// TODO
	//SDL_WarpMouseInWindow(renderer->GetSdlWindow(), p.x, p.y);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::ShowWindow(HWND hWnd, int nCmdShow) {
#ifdef __WIN32__
	return ::ShowWindow(hWnd, nCmdShow);
#else
	return FALSE;
#endif
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
#ifdef __WIN32__
	return ::SetCapture(hWnd);
#else
	return NULL;
#endif
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::ReleaseCapture() {
#ifdef __WIN32__
	return ::ReleaseCapture();
#else
	return FALSE;
#endif
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::SetForegroundWindow(HWND hWnd) {
#ifdef __WIN32__
	return ::SetForegroundWindow(hWnd);
#else
	return FALSE;
#endif
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::SetRectEmpty(LPRECT lprc) {
	lprc->left = lprc->right = lprc->top = lprc->bottom = 0;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::IsRectEmpty(const LPRECT lprc) {
	return (lprc->left >= lprc->right) || (lprc->top >= lprc->bottom);
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::PtInRect(LPRECT lprc, POINT p) {
	return (p.x >= lprc->left) && (p.x < lprc->right) && (p.y >= lprc->top) && (p.y < lprc->bottom);
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::SetRect(LPRECT lprc, int left, int top, int right, int bottom) {
	lprc->left   = left;
	lprc->top    = top;
	lprc->right  = right;
	lprc->bottom = bottom;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::IntersectRect(LPRECT lprcDst, CONST LPRECT lprcSrc1, CONST LPRECT lprcSrc2) {
	if (IsRectEmpty(lprcSrc1) || IsRectEmpty(lprcSrc2) ||
	        lprcSrc1->left >= lprcSrc2->right || lprcSrc2->left >= lprcSrc1->right ||
	        lprcSrc1->top >= lprcSrc2->bottom || lprcSrc2->top >= lprcSrc1->bottom) {
		SetRectEmpty(lprcDst);
		return FALSE;
	}
	lprcDst->left   = MAX(lprcSrc1->left, lprcSrc2->left);
	lprcDst->right  = MIN(lprcSrc1->right, lprcSrc2->right);
	lprcDst->top    = MAX(lprcSrc1->top, lprcSrc2->top);
	lprcDst->bottom = MIN(lprcSrc1->bottom, lprcSrc2->bottom);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::UnionRect(LPRECT lprcDst, RECT *lprcSrc1, RECT *lprcSrc2) {
	if (IsRectEmpty(lprcSrc1)) {
		if (IsRectEmpty(lprcSrc2)) {
			SetRectEmpty(lprcDst);
			return FALSE;
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

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::CopyRect(LPRECT lprcDst, RECT *lprcSrc) {
	if (lprcDst == NULL || lprcSrc == NULL) return FALSE;

	*lprcDst = *lprcSrc;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::OffsetRect(LPRECT lprc, int dx, int dy) {
	if (lprc == NULL) return FALSE;

	lprc->left   += dx;
	lprc->top    += dy;
	lprc->right  += dx;
	lprc->bottom += dy;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::EqualRect(LPRECT rect1, LPRECT rect2) {
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
	return "/Library/Fonts/";
#endif
}

//////////////////////////////////////////////////////////////////////////
AnsiString CBPlatform::GetPlatformName() {
	// TODO: Should conform to the WME-spec.
	//return AnsiString(SDL_GetPlatform());
	return AnsiString("ScummVM");
}

//////////////////////////////////////////////////////////////////////////
int scumm_stricmp(const char *str1, const char *str2) {
#ifdef __WIN32__
	return ::stricmp(str1, str2);
#else
	return ::strcasecmp(str1, str2);
#endif
}

//////////////////////////////////////////////////////////////////////////
int scumm_strnicmp(const char *str1, const char *str2, size_t maxCount) {
#ifdef __WIN32__
	return ::strnicmp(str1, str2, maxCount);
#else
	return ::strncasecmp(str1, str2, maxCount);
#endif
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
