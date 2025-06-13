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
#include "common/config-manager.h"
#include "common/events.h"
#include "common/textconsole.h"
#include "graphics/paletteman.h"
#include "bagel/mfc/global_functions.h"
#include "bagel/mfc/afxwin.h"
#include "bagel/mfc/win_hand.h"
#include "bagel/mfc/libs/events.h"

namespace Bagel {
namespace MFC {

#define FRAME_RATE 50

/**
 * Used for temporary handle wrapper objects
 */
class CTempGdiObject : public CGdiObject {
	DECLARE_DYNCREATE(CTempGdiObject)
};
IMPLEMENT_DYNCREATE(CTempGdiObject, CGdiObject);

IMPLEMENT_DYNAMIC(CWinApp, CWinThread)

CWinApp *CWinApp::_activeApp = nullptr;

CWinApp::CWinApp(const char *appName) :
		CWinThread(), _cursors(_resources),
		_fonts(_resources) {
	assert(!_activeApp);    // Only one app per engine
	_activeApp = this;
	Libs::Event::init();
}

CWinApp::~CWinApp() {
	_activeApp = nullptr;

	delete m_pmapHDC;
	delete m_pmapHGDIOBJ;
}

BOOL CWinApp::InitApplication() {
	_settings.load();
	return true;
}

BOOL CWinApp::InitInstance() {
	return true;
}

int CWinApp::ExitInstance() {
	return 0;
}

BOOL CWinApp::SaveAllModified() {
	_settings.save();
	return true;
}

int CWinApp::Run() {
	InitApplication();
	InitInstance();
	assert(m_pMainWnd);

	MSG msg;
	while (m_pMainWnd && GetMessage(msg)) {
		if (!PreTranslateMessage(&msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	ExitInstance();
	SaveAllModified();

	return 0;
}

bool CWinApp::GetMessage(MSG &msg) {
	m_pMainWnd->GetMessage(msg);
	return !_quitFlag;
}

bool CWinApp::pollEvents(Common::Event &event) {
	if (!g_system->getEventManager()->pollEvent(event)) {
		// Brief pauses and screen updates
		g_system->delayMillis(10);

		uint32 time = g_system->getMillis();
		if (time >= _nextFrameTime) {
			_nextFrameTime = time + (1000 / FRAME_RATE);
			_screen.update();
		}

		// Cleanup any temporary handle wrapper
		AfxUnlockTempMaps();

		return false;
	}

	// Check for quit event
	if ((event.type == Common::EVENT_QUIT) ||
		(event.type == Common::EVENT_RETURN_TO_LAUNCHER)) {
		_quitFlag = true;
		return false;
	}

	return true;
}

BOOL CWinApp::PreTranslateMessage(MSG *pMsg) {
	return FALSE;
}

void CWinApp::SetDialogBkColor() {
}

HCURSOR CWinApp::LoadStandardCursor(LPCSTR lpszCursorName) {
	return _cursors.loadCursor(lpszCursorName);
}

HCURSOR CWinApp::LoadCursor(LPCSTR lpszResourceName) {
	return _cursors.loadCursor(lpszResourceName);
}

HCURSOR CWinApp::LoadCursor(UINT nIDResource) {
	return _cursors.loadCursor((LPCSTR)nIDResource);
}

HCURSOR CWinApp::SetCursor(HCURSOR hCursor) {
	if (hCursor == _currentCursor)
		return hCursor;

	HCURSOR oldCursor = _currentCursor;
	_currentCursor = hCursor;

	Gfx::Cursor *c = (Gfx::Cursor *)hCursor;
	if (c) {
		c->showCursor();
	} else {
		Gfx::Cursor::hide();
	}

	return oldCursor;
}

void CWinApp::BeginWaitCursor() {
	DoWaitCursor(1);
}

void CWinApp::EndWaitCursor() {
	DoWaitCursor(-1);
}

void CWinApp::DoWaitCursor(int nCode) {
	assert(nCode == 0 || nCode == 1 || nCode == -1);

	m_nWaitCursorCount += nCode;
	if (m_nWaitCursorCount > 0) {
		// Set wait cursor
		HCURSOR hcurPrev = MFC::SetCursor(_cursors._waitCursor);
		if (nCode > 0 && m_nWaitCursorCount == 1)
			m_hcurWaitCursorRestore = hcurPrev;
	} else {
		// Turn off wait cursor
		m_nWaitCursorCount = 0;
		MFC::SetCursor(m_hcurWaitCursorRestore);
	}
}

void CWinApp::AddDocTemplate(CDocTemplate *pTemplate) {
	error("TODO: CWinApp::AddDocTemplate");
}

void CWinApp::CloseAllDocuments(BOOL bEndSession) {
	error("TODO: CWinApp::CloseAllDocuments");
}

UINT CWinApp::GetProfileInt(LPCSTR lpszSection,
        LPCSTR lpszEntry, int nDefault) {
	return _settings[lpszSection].getInt(lpszEntry, nDefault);
}

void CWinApp::WriteProfileInt(LPCSTR lpszSection,
        LPCSTR lpszEntry, int nValue) {
	_settings[lpszSection].setInt(lpszEntry, nValue);
}

void CWinApp::setDirectory(const char *folder) {
	const Common::FSNode gameDataDir(ConfMan.getPath("path"));

	_currentDirectory = gameDataDir;
	if (folder && *folder)
		_currentDirectory = gameDataDir.getChild(folder);
}

Common::FSNode CWinApp::getDirectory() const {
	return _currentDirectory;
}

void CWinApp::setPalette(const Graphics::Palette &pal) {
	_palette = pal;
	g_system->getPaletteManager()->setPalette(_palette);
}

byte CWinApp::getColor(COLORREF color) const {
	if (_palette.empty())
		return 0;

	return _palette.findBestColor(
		GetRValue(color),
		GetGValue(color),
		GetBValue(color)
	);
}

HRSRC CWinApp::findResource(LPCSTR lpName, LPCSTR lpType) {
	return _resources.findResource(lpName, lpType);
}

size_t CWinApp::sizeofResource(HRSRC hResInfo) {
	return _resources.resourceSize(hResInfo);
}

HGLOBAL CWinApp::loadResource(HRSRC hResInfo) {
	return _resources.loadResource(hResInfo);
}

LPVOID CWinApp::lockResource(HGLOBAL hResData) {
	return GlobalLock(hResData);
}

void CWinApp::unlockResource(HGLOBAL hResData) {
	GlobalUnlock(hResData);
}

BOOL CWinApp::freeResource(HGLOBAL hResData) {
	GlobalFree(hResData);
	return true;
}

CHandleMap<CGdiObject> *CWinApp::afxMapHGDIOBJ(BOOL bCreate) {
	if (m_pmapHGDIOBJ == nullptr && bCreate)
		m_pmapHGDIOBJ = new CHandleMap<CGdiObject>();

	return m_pmapHGDIOBJ;
}

CHandleMap<CDC> *CWinApp::afxMapHDC(BOOL bCreate) {
	if (m_pmapHDC == nullptr && bCreate)
		m_pmapHDC = new CHandleMap<CDC>();

	return m_pmapHDC;
}

void CWinApp::AfxUnlockTempMaps() {
	if (m_pmapHDC)
		m_pmapHDC->DeleteTemp();
	if (m_pmapHGDIOBJ)
		m_pmapHGDIOBJ->DeleteTemp();
}

/*--------------------------------------------*/

CWinApp *AfxGetApp() {
	return CWinApp::_activeApp;
}

CWnd *AfxGetMainWnd() {
	assert(CWinApp::_activeApp);
	return CWinApp::_activeApp->m_pMainWnd;
}

HINSTANCE AfxGetInstanceHandle() {
	// Unused in ScummVM
	return 0;
}

int LoadString(HINSTANCE hInstance,
               UINT uID, LPSTR lpBuffer, int cchBufferMax) {
	error("TODO: LoadString");
}

HMODULE LoadLibrary(LPCSTR lpLibFileName) {
	error("LoadLibrary is unsupported");
}

void FreeLibrary(HMODULE hModule) {
	error("FreeLibrary is unsupported");
}


FARPROC GetProcAddress(HMODULE hModule,
                       LPCSTR lpProcName) {
	error("TODO: GetProcAddress");
}

HMODULE GetModuleHandle(LPCSTR lpModuleName) {
	error("TODO: GetModuleHandle");
}

LPCSTR AFXAPI AfxRegisterWndClass(UINT nClassStyle,
        HCURSOR hCursor, HBRUSH hbrBackground, HICON hIcon) {
	// Not currently handled
	assert(!hCursor && !hbrBackground && !hIcon);

	// Common class name for all ScummVM windows
	return "ScummVMWindow";
}

int GetSystemMetrics(int nIndex) {
	error("TODO: GetSystemMetrics");
}

} // namespace MFC
} // namespace Bagel
