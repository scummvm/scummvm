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
#include "bagel/mfc/gfx/cursor.h"
#include "bagel/mfc/libs/events.h"

namespace Bagel {
namespace MFC {

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
		CWinThread(), EventLoop(),
		_cursors(_resources),
		_fonts(_resources) {
	_priorWinApp = _activeApp;
	_activeApp = this;
	Libs::Event::init();
}

CWinApp::~CWinApp() {
	// Free the defaults
	_defaultFont.DeleteObject();
	_defaultPen.DeleteObject();
	_defaultBrush.DeleteObject();
	_palette.DeleteObject();

	// Release the handle maps
	delete m_pmapHDC;
	delete m_pmapHGDIOBJ;
	m_pmapHDC = nullptr;
	m_pmapHGDIOBJ = nullptr;

	_activeApp = _priorWinApp;
	if (_activeApp)
		_activeApp->_quitFlag = _quitFlag;
}

BOOL CWinApp::InitApplication() {
	_settings.load();

	_defaultFont.CreateFont(8, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FF_ROMAN, "MS Sans Serif");
	_defaultPen.CreatePen(PS_SOLID, 1, 0);
	_defaultBrush.CreateSolidBrush(RGB(255, 255, 255));

	// Set up palette. It's empty by default, but
	// we need to at least get it registered.
	// Then it will be filled out via setPalette later
	LOGPALETTE lp;
	lp.palVersion = 0x300;
	lp.palNumEntries = 0;
	_palette.CreatePalette(&lp);

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
	// Ensure app has been initialized
	assert(_defaultFont._font);

	SetCursor(LoadCursor(IDC_ARROW));

	if (!m_pMainWnd) {
		m_pMainWnd = GetActiveWindow();
		assert(m_pMainWnd);
	} else {
		assert(m_pMainWnd);
		SetActiveWindow(m_pMainWnd);
	}

	runEventLoop();

	ExitInstance();
	SaveAllModified();

	return 0;
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
	if (m_pDocManager == NULL)
		m_pDocManager = new CDocManager();
	m_pDocManager->AddDocTemplate(pTemplate);
}

void CWinApp::OnFileNew() {
	if (m_pDocManager != nullptr)
		m_pDocManager->OnFileNew();
}

void CWinApp::OnFileOpen() {
	assert(m_pDocManager != nullptr);
	m_pDocManager->OnFileOpen();
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

CString CWinApp::GetProfileString(LPCSTR lpszSection,
		LPCSTR lpszEntry, LPCSTR lpszDefault) {
	Common::String str = _settings[lpszSection].getString(lpszEntry, lpszDefault);
	return CString(str.c_str());
}

BOOL CWinApp::WriteProfileString(LPCSTR lpszSection,
		LPCSTR lpszEntry, LPCSTR lpszValue) {
	_settings[lpszSection].setString(lpszEntry, lpszValue);
	return true;
}

void CWinApp::setDirectory(const char *folder) {
	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	SearchMan.remove("CurrentDir");

	_currentDirectory = gameDataDir;
	if (folder && *folder) {
		_currentDirectory = gameDataDir.getChild(folder);
		SearchMan.addDirectory("CurrentDir", _currentDirectory, 10, 2);
	}
}

void CWinApp::setPalette(const Graphics::Palette &pal) {
	_palette.SetPaletteEntries(pal);
	g_system->getPaletteManager()->setPalette(*_palette._palette);
}

byte CWinApp::getColor(COLORREF color) const {
	if (_palette.isEmpty())
		return 0;

	if (color <= 0xff || (color >> 24) == 1)
		return (byte)(color & 0xff);

	return _palette._palette->findBestColor(
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

CHandleMap<CWnd> *CWinApp::afxMapWnd(BOOL bCreate) {
	if (m_pmapWnd == nullptr && bCreate)
		m_pmapWnd = new CHandleMap<CWnd>();

	return m_pmapWnd;
}

void CWinApp::AfxUnlockTempMaps() {
	if (m_pmapHDC)
		m_pmapHDC->DeleteTemp();
	if (m_pmapHGDIOBJ)
		m_pmapHGDIOBJ->DeleteTemp();
}

LPCSTR CWinApp::AfxRegisterWndClass(UINT nClassStyle,
		HCURSOR hCursor, HBRUSH hbrBackground, HICON hIcon) {
	return "ScummVMWindow";
}

BOOL CWinApp::GetClassInfo(HINSTANCE hInstance,
		LPCSTR lpClassName, LPWNDCLASS lpWndClass) {
	assert(lpWndClass);

	WNDCLASS &wc = *lpWndClass;
	memset(lpWndClass, 0, sizeof(WNDCLASS));
	wc.style = CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_OWNDC;
	return true;
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

int LoadString(HINSTANCE hInstance, UINT uID,
		LPSTR lpBuffer, int cchBufferMax) {
	if (lpBuffer == nullptr || cchBufferMax <= 0)
		return 0;

	// Calculate which string block (resource ID) contains this string
	UINT blockID = (uID / 16) + 1;
	UINT indexInBlock = uID % 16;

	// Find the RT_STRING resource with that block ID
	HRSRC hRes = FindResource(hInstance, MAKEINTRESOURCE(blockID), RT_STRING);
	if (!hRes)
		return 0;

	HGLOBAL hResData = LoadResource(hInstance, hRes);
	if (!hResData)
		return 0;

	const byte *pData = (const byte *)LockResource(hResData);
	assert(pData);

	// Walk through up to 16 strings in the block
	for (uint i = 0; i < 16; ++i) {
		uint length = *pData++;

		if (i == indexInBlock) {
			// Found the desired string
			int copyLen = MIN((int)length, cchBufferMax - 1);
			memcpy(lpBuffer, (const char *)pData, copyLen);
			lpBuffer[copyLen] = '\0';
			return copyLen;
		}

		// Skip this string
		pData += length;
	}

	// String ID not found (shouldn't happen if resource is valid)
	return 0;
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

LPCSTR AfxRegisterWndClass(UINT nClassStyle,
        HCURSOR hCursor, HBRUSH hbrBackground, HICON hIcon) {
	return AfxGetApp()->AfxRegisterWndClass(nClassStyle,
		hCursor, hbrBackground, hIcon);
}

BOOL GetClassInfo(HINSTANCE hInstance,
		LPCSTR lpClassName, LPWNDCLASS lpWndClass) {
	return AfxGetApp()->GetClassInfo(hInstance, lpClassName, lpWndClass);
}

int GetSystemMetrics(int nIndex) {
	switch (nIndex) {
	case SM_CXCURSOR:
		return Gfx::CURSOR_W;
	case SM_CYCURSOR:
		return Gfx::CURSOR_H;
	case SM_CXSCREEN:
		return g_system->getWidth();
	case SM_CYSCREEN:
		return g_system->getHeight();
	default:
		error("TODO: GetSystemMetrics");
		break;
	}
}

} // namespace MFC
} // namespace Bagel
