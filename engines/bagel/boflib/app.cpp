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

#include "graphics/framelimiter.h"
#include "graphics/paletteman.h"

#include "bagel/boflib/boffo.h"
#include "bagel/boflib/debug.h"
#include "bagel/boflib/misc.h"
#include "bagel/boflib/app.h"
#include "bagel/boflib/timer.h"
#include "bagel/boflib/gfx/text.h"
#include "bagel/boflib/sound.h"
#include "bagel/bagel.h"

namespace Bagel {

#define DEBUG_LOG "DEBUG.LOG"
#define DEBUG_INI "BOFFO.INI"

#define BOFDISP 0

#if BOF_MAC && USEDRAWSPROCKET
CGrafPtr gBackBuffer;
#endif

CBofApp *CBofApp::m_pBofApp;

#if BOF_WINDOWS
HINSTANCE CBofApp::m_hInstance = nullptr;

extern "C" HINSTANCE g_hInst = nullptr;
#endif

CHAR g_szCopyright[] = "Copyright(C) 1996 Boffo Games, Inc.  All rights reserved.";

// Local functions
//
BOOL FirstInstance();
#if BOF_MAC && USEDRAWSPROCKET
void InitDSpContextAttributes(DSpContextAttributes *inAttributes);
#endif

CBofApp::CBofApp() {
	StartupCode();
}


CBofApp::CBofApp(const CHAR *pszAppName) {
	StartupCode();

	SetAppName(pszAppName);
}

CBofApp::~CBofApp() {
	ShutDownCode();

	m_szAppName[0] = '\0';
	m_pMainWnd = nullptr;
	m_pPalette = nullptr;
	m_pBofApp = nullptr;
}


VOID CBofApp::StartupCode() {
	m_pBofApp = this;

	// Open the Boffo debug options file (BOFFO.INI)
	BOOL bRand = TRUE;
	if ((g_pDebugOptions = new CBofDebugOptions(DEBUG_INI)) != nullptr) {
		g_pDebugOptions->ReadSetting("DebugOptions", "MainLoops", &m_nIterations, DEFAULT_MAINLOOPS);
		bRand = g_pDebugOptions->m_bRandomOn;
	}

	// Initialize the logging file (DEBUG.LOG)
	g_pDebugLog = new CBofLog(DEBUG_LOG);

#if BOF_DEBUG
	// Mark all currently allocated memory blocks so that a call
	// to VerifyAllBlocksDeleted() in the destructor will tell us about any
	// new blocks that have not been freed.
	MarkMemBlocks();
#endif

	//
	// Initialize the boffo libraries
	//

	// Init the Window library
	CBofWindow::Initialize();

	// Init the text library
	CBofText::Initialize();
}


VOID CBofApp::ShutDownCode() {
	// Un-initialize the text library
	CBofText::ShutDown();

	// Shut down the Window library
	CBofWindow::ShutDown();

	// Kill any shared palette
	CBofPalette::SetSharedPalette(nullptr);

#if BOF_DEBUG
	// Make sure that all memory allocated by our game has been deleted
	VerifyAllBlocksDeleted();
#endif

	if (g_pDebugLog != nullptr) {
		delete g_pDebugLog;
		g_pDebugLog = nullptr;
	}
	if (g_pDebugOptions != nullptr) {
		delete g_pDebugOptions;
		g_pDebugOptions = nullptr;
	}
}


ERROR_CODE CBofApp::PreInit() {
	if ((m_pPalette == nullptr) && (m_pDefPalette == nullptr)) {
		if ((m_pDefPalette = new CBofPalette()) != nullptr) {
			m_pDefPalette->CreateDefault();
			SetPalette(m_pDefPalette);
		}
	}

	return m_errCode;
}


ERROR_CODE CBofApp::Initialize() {
	return m_errCode;
}


ERROR_CODE CBofApp::RunApp() {
	CBofWindow *pWindow;
	INT i, nCount;
	Common::Event evt;

	nCount = m_nIterations;

	// Acquire and dispatch messages until we need to quit, or too many errors

	Graphics::FrameLimiter limiter(g_system, 60);
	while (!g_engine->shouldQuit() && CBofError::GetErrorCount() < MAX_ERRORS) {
		// Handle sounds and timers
		CBofSound::AudioTask();
		CBofTimer::HandleTimers();

		if (nCount < 0)  {
			nCount++;
			if (nCount == 0)
				nCount = 1;

		} else {
			for (i = 0; i < nCount; i++) {
				// Give each window it's own main loop (sort-of)
				pWindow = CBofWindow::GetWindowList();
				while (pWindow != nullptr) {
					if (shouldQuit())
						return ERR_NONE;

					if (pWindow->IsCreated()) {
						pWindow->OnMainLoop();
					}

					pWindow = (CBofWindow *)pWindow->GetNext();
				}
			}

			nCount = m_nIterations;
		}

		// Handle events
		CBofWindow::GetActiveWindow()->handleEvents();

		limiter.delayBeforeSwap();
		g_engine->_screen->update();
		limiter.startFrame();
	}

	return m_errCode;
}


ERROR_CODE CBofApp::ShutDown() {
	return m_errCode;
}


ERROR_CODE CBofApp::PreShutDown() {
	return m_errCode;
}

ERROR_CODE CBofApp::PostShutDown() {
#if BOFDISP
	CBofDisplayObject::CleanUp();
#endif

	if (m_pWindow != nullptr) {
		delete m_pWindow;
		m_pWindow = nullptr;
	}

	// No more palettes
	m_pPalette = nullptr;

	if (m_pDefPalette != nullptr) {
		delete m_pDefPalette;
		m_pDefPalette = nullptr;
	}

	return m_errCode;
}

VOID CBofApp::SetPalette(CBofPalette *pPalette) {
	m_pPalette = pPalette;

	if (pPalette != nullptr) {
		if (g_system->getScreenFormat().bytesPerPixel == 1) {
			const HPALETTE &pal = pPalette->GetPalette();
			g_system->getPaletteManager()->setPalette(pal._data, 0, pal._numColors);
		}

	} else {
		// Use default palette
		m_pPalette = m_pDefPalette;
	}
}

VOID CBofApp::AddCursor(CBofCursor &cCursor) {
	m_cCursorList.AddToTail(cCursor);
}

VOID CBofApp::DelCursor(INT nIndex) {
	m_cCursorList.Remove(nIndex);
}


VOID BofPostMessage(CBofWindow *pWindow, ULONG lMessage, ULONG lParam1, ULONG lParam2) {
#if BOF_WINDOWS
	HWND hWnd;

	hWnd = nullptr;
	if (pWindow != nullptr)
		hWnd = pWindow->GetHandle();

	::PostMessage(hWnd, lMessage, (WPARAM)lParam1, (LPARAM)lParam2);

#elif BOF_MAC
	CBofMessage *pMessage;

	// Create a user defined message.
	// NOTE: This message will be deleted by HandleMacEvent()
	//
	if ((pMessage = new CBofMessage) != nullptr) {

		pMessage->m_pWindow = pWindow;
		pMessage->m_nMessage = lMessage;
		pMessage->m_lParam1 = lParam1;
		pMessage->m_lParam2 = lParam2;

		PostEvent(app3Evt, (LONG)pMessage);
	}
#endif
}


///////////////////////////////////////////////////////////////////////////
// Global routines
///////////////////////////////////////////////////////////////////////////

VOID SetMousePos(CBofPoint &cPoint) {
	g_system->warpMouse(cPoint.x, cPoint.y);
}


CBofPoint GetMousePos() {
	CBofPoint cPoint;

#if BOF_WINDOWS
	POINT stPoint;

	GetCursorPos(&stPoint);
	cPoint.x = stPoint.x;
	cPoint.y = stPoint.y;

#elif BOF_MAC
	Point stPoint;

	::GetMouse(&stPoint);
	::LocalToGlobal(&stPoint);

	cPoint.x = stPoint.h;
	cPoint.y = stPoint.v;
#endif
	return cPoint;
}


VOID BofMessageBox(const CHAR *pszTitle, const CHAR *pszMessage) {
	CBofCursor::Show();

#if BOF_WINDOWS
	CBofApp *pApp;
	HWND hWnd;

	hWnd = nullptr;
	if ((pApp = CBofApp::GetApp()) != nullptr) {
		CBofWindow *pWnd;
		if ((pWnd = pApp->GetMainWindow()) != nullptr) {
			hWnd = pWnd->GetHandle();
		}
	}

	::MessageBox(hWnd, pszTitle, pszMessage, MB_OK);

#elif BOF_MAC
	MacMessageBox(pszTitle, pszMessage);
#else
#endif

	CBofCursor::Hide();
}

} // namespace Bagel
