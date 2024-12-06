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

CBofApp *CBofApp::_pBofApp;

CBofApp::CBofApp() {
	StartupCode();
}

CBofApp::CBofApp(const char *pszAppName) {
	StartupCode();

	setAppName(pszAppName);
}

CBofApp::~CBofApp() {
	ShutDownCode();

	_szAppName[0] = '\0';
	_pMainWnd = nullptr;
	_pPalette = nullptr;
	_pBofApp = nullptr;
}


void CBofApp::StartupCode() {
	_pBofApp = this;

	// Open the Boffo debug options file (BOFFO.INI)
	g_pDebugOptions = new CBofDebugOptions(DEBUG_INI);
	g_pDebugOptions->readSetting("DebugOptions", "MainLoops", &_nIterations, DEFAULT_MAINLOOPS);

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
	CBofPalette::setSharedPalette(nullptr);

	if (g_pDebugOptions != nullptr) {
		delete g_pDebugOptions;
		g_pDebugOptions = nullptr;
	}
}


ErrorCode CBofApp::preInit() {
	if ((_pPalette == nullptr) && (_pDefPalette == nullptr)) {
		_pDefPalette = new CBofPalette();
		_pDefPalette->createDefault();
		setPalette(_pDefPalette);
	}

	return _errCode;
}


ErrorCode CBofApp::initialize() {
	return _errCode;
}


ErrorCode CBofApp::runApp() {
	int nCount = _nIterations;

	// Acquire and dispatch messages until we need to quit, or too many errors

	Graphics::FrameLimiter limiter(g_system, 60, false);
	while (!g_engine->shouldQuit() && CBofError::getErrorCount() < MAX_ERRORS) {
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

		}

		delete _consoleVideo;
		_consoleVideo = nullptr;

		// Handle sounds and timers
		CBofSound::audioTask();
		CBofTimer::handleTimers();

		if (nCount < 0)  {
			nCount++;
			if (nCount == 0)
				nCount = 1;

		} else {
			for (int i = 0; i < nCount; i++) {
				// Give each window it's own main loop (sort-of)
				CBofWindow *pWindow = CBofWindow::getWindowList();
				while (pWindow != nullptr) {
					if (shouldQuit())
						return ERR_NONE;

					if (pWindow->isCreated()) {
						pWindow->onMainLoop();
					}

					pWindow = (CBofWindow *)pWindow->getNext();
				}
			}

			nCount = _nIterations;
		}

		// Handle events
		_pMainWnd->handleEvents();

		limiter.delayBeforeSwap();
		g_engine->_screen->update();
		limiter.startFrame();
	}

	return _errCode;
}


ErrorCode CBofApp::shutdown() {
	return _errCode;
}


void CBofApp::postShutDown() {
	delete _pWindow;
	_pWindow = nullptr;

	// No more palettes
	_pPalette = nullptr;

	delete _pDefPalette;
	_pDefPalette = nullptr;
}

void CBofApp::setPalette(CBofPalette *pPalette) {
	_pPalette = pPalette;

	if (pPalette != nullptr) {
		if (g_system->getScreenFormat().bytesPerPixel == 1) {
			const HPALETTE &pal = pPalette->getPalette();
			g_system->getPaletteManager()->setPalette(pal._data, 0, pal._numColors);
		}

	} else {
		// Use default palette
		_pPalette = _pDefPalette;
	}
}

void CBofApp::addCursor(CBofCursor &cCursor) {
	_cCursorList.addToTail(cCursor);
}

void CBofApp::delCursor(int nIndex) {
	_cCursorList.remove(nIndex);
}

bool CBofApp::consolePlayVideo(const Common::Path &path) {
	delete _consoleVideo;

	_consoleVideo = new Video::SmackerDecoder();
	_consoleVideo->setSoundType(Audio::Mixer::kSFXSoundType);
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

CBofPoint getMousePos() {
	return CBofWindow::getMousePos();
}


void bofMessageBox(const char *pszTitle, const char *pszMessage) {
	Common::String msg = Common::String::format("%s - %s", pszTitle, pszMessage);
	g_engine->errorDialog(msg.c_str());
}

} // namespace Bagel
