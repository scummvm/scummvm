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
#include "graphics/palette.h"
#include "graphics/paletteman.h"
#include "video/smk_decoder.h"

#include "bagel/baglib/bagel.h"
#include "bagel/boflib/boffo.h"
#include "bagel/boflib/debug.h"
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

char g_szCopyright[] = "Copyright(C) 1996 Boffo Games, Inc.  All rights reserved.";

// Local functions
//
bool FirstInstance();
#if BOF_MAC && USEDRAWSPROCKET
void InitDSpContextAttributes(DSpContextAttributes *inAttributes);
#endif

CBofApp::CBofApp() {
	StartupCode();
}


CBofApp::CBofApp(const char *pszAppName) {
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


void CBofApp::StartupCode() {
	m_pBofApp = this;

	// Open the Boffo debug options file (BOFFO.INI)
	g_pDebugOptions = new CBofDebugOptions(DEBUG_INI);
	if (g_pDebugOptions != nullptr) {
		g_pDebugOptions->ReadSetting("DebugOptions", "MainLoops", &m_nIterations, DEFAULT_MAINLOOPS);
	}

	//
	// Initialize the boffo libraries
	//

	// Init the Window library
	CBofWindow::initialize();

	// Init the text library
	CBofText::initialize();
}


void CBofApp::ShutDownCode() {
	// Un-initialize the text library
	CBofText::shutdown();

	// Shut down the Window library
	CBofWindow::shutdown();

	// Kill any shared palette
	CBofPalette::SetSharedPalette(nullptr);

	if (g_pDebugOptions != nullptr) {
		delete g_pDebugOptions;
		g_pDebugOptions = nullptr;
	}
}


ErrorCode CBofApp::PreInit() {
	if ((m_pPalette == nullptr) && (m_pDefPalette == nullptr)) {
		m_pDefPalette = new CBofPalette();
		if (m_pDefPalette != nullptr) {
			m_pDefPalette->CreateDefault();
			SetPalette(m_pDefPalette);
		}
	}

	return m_errCode;
}


ErrorCode CBofApp::initialize() {
	return m_errCode;
}


ErrorCode CBofApp::RunApp() {
	int nCount = m_nIterations;

	// Acquire and dispatch messages until we need to quit, or too many errors

	Graphics::FrameLimiter limiter(g_system, 60);
	while (!g_engine->shouldQuit() && CBofError::GetErrorCount() < MAX_ERRORS) {
		// Support for playing videos via the console
		if (_consoleVideo && _consoleVideo->isPlaying()) {
			if (_consoleVideo->needsUpdate()) {
				const Graphics::Surface *s = _consoleVideo->decodeNextFrame();
				Graphics::Palette pal(_consoleVideo->getPalette(), 256);
				g_engine->_screen->blitFrom(*s, Common::Point(0, 0), &pal);
			}

			limiter.delayBeforeSwap();
			g_engine->_screen->update();
			limiter.startFrame();
			continue;

		} else if (_consoleVideo) {
			delete _consoleVideo;
			_consoleVideo = nullptr;
		}

		// Handle sounds and timers
		CBofSound::AudioTask();
		CBofTimer::HandleTimers();

		if (nCount < 0)  {
			nCount++;
			if (nCount == 0)
				nCount = 1;

		} else {
			for (int i = 0; i < nCount; i++) {
				// Give each window it's own main loop (sort-of)
				CBofWindow *pWindow = CBofWindow::GetWindowList();
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
		m_pMainWnd->handleEvents();

		limiter.delayBeforeSwap();
		g_engine->_screen->update();
		limiter.startFrame();
	}

	return m_errCode;
}


ErrorCode CBofApp::shutdown() {
	return m_errCode;
}


ErrorCode CBofApp::PreShutDown() {
	return m_errCode;
}

ErrorCode CBofApp::PostShutDown() {
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

void CBofApp::SetPalette(CBofPalette *pPalette) {
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

void CBofApp::AddCursor(CBofCursor &cCursor) {
	m_cCursorList.AddToTail(cCursor);
}

void CBofApp::DelCursor(int nIndex) {
	m_cCursorList.Remove(nIndex);
}

bool CBofApp::consolePlayVideo(const Common::Path &path) {
	delete _consoleVideo;

	_consoleVideo = new Video::SmackerDecoder();
	if (_consoleVideo->loadFile(path)) {
		_consoleVideo->start();
		return true;

	} else {
		delete _consoleVideo;
		_consoleVideo = nullptr;
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////
// Global routines
///////////////////////////////////////////////////////////////////////////

CBofPoint GetMousePos() {
	return CBofWindow::getMousePos();
}


void BofMessageBox(const char *pszTitle, const char *pszMessage) {
	CBofCursor::Show();

#if BOF_WINDOWS
	HWND hWnd = nullptr;
	CBofApp *pApp = CBofApp::GetApp();
	if (pApp != nullptr) {
		CBofWindow *pWnd = pApp->GetMainWindow();
		if (pWnd != nullptr) {
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
