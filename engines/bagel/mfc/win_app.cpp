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
#include "engines/engine.h"

namespace Bagel {
namespace MFC {

/**
 * Used for temporary handle wrapper objects
 */
class CTempGdiObject : public CGdiObject {
	DECLARE_DYNCREATE(CTempGdiObject)
};
IMPLEMENT_DYNCREATE(CTempGdiObject, CGdiObject)

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
	_currentPalette.DeleteObject();
	_systemPalette.DeleteObject();

	// Release the handle maps
	delete m_pmapHDC;
	delete m_pmapHGDIOBJ;
	m_pmapHDC = nullptr;
	m_pmapHGDIOBJ = nullptr;

	_activeApp = _priorWinApp;
	if (_activeApp && Engine::shouldQuit())
		_activeApp->_quitFlag = QUIT_QUITTING;
}

bool CWinApp::InitApplication() {
	_settings.load();

	_defaultFont.CreateFont(8, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FF_ROMAN, "MS Sans Serif");
	_defaultPen.CreatePen(PS_SOLID, 1, 0);
	_defaultBrush.CreateSolidBrush(RGB(255, 255, 255));

	// Set up current palette. It's empty by default, but
	// we need to at least get it registered.
	// Then it will be filled out via setPalette later
	LOGPALETTE lp;
	lp.palVersion = 0x300;
	lp.palNumEntries = 0;
	_currentPalette.CreatePalette(&lp);

	// Set up system palette
	LOGPALETTE *lps = (LOGPALETTE *)malloc(sizeof(LOGPALETTE) + 255 * sizeof(PALETTEENTRY));
	lps->palVersion = 0x300;
	lps->palNumEntries = 256;
	Common::fill((byte *)&lps->palPalEntry[0], (byte *)&lps->palPalEntry[Graphics::PALETTE_COUNT], 0);

	// Set first 10 system palette colors
	PALETTEENTRY sysColorsStart[10] = {
		{ 0x00, 0x00, 0x00, 0 },       // 0: Black
		{ 0x80, 0x00, 0x00, 0 },       // 1: Dark Red
		{ 0x00, 0x80, 0x00, 0 },       // 2: Dark Green
		{ 0x80, 0x80, 0x00, 0 },       // 3: Olive
		{ 0x00, 0x00, 0x80, 0 },       // 4: Dark Blue
		{ 0x80, 0x00, 0x80, 0 },       // 5: Purple
		{ 0x00, 0x80, 0x80, 0 },       // 6: Teal
		{ 0xC0, 0xC0, 0xC0, 0 },       // 7: Gray
		{ 0xC0, 0xDC, 0xC0, 0 },       // 8: Silver (Windows UI color)
		{ 0xA6, 0xCA, 0xF0, 0 }        // 9: Light Gray (Windows UI color)
	};

	for (int i = 0; i < 10; ++i) {
		lps->palPalEntry[i] = sysColorsStart[i];
	}

	// Set last 10 system palette colors
	PALETTEENTRY sysColorsEnd[10] = {
		{ 0xFF, 0xFF, 0xFF, 0 },       // 246: White
		{ 0xFF, 0x00, 0x00, 0 },       // 247: Red
		{ 0x00, 0xFF, 0x00, 0 },       // 248: Green
		{ 0xFF, 0xFF, 0x00, 0 },       // 249: Yellow
		{ 0x00, 0x00, 0xFF, 0 },       // 250: Blue
		{ 0xFF, 0x00, 0xFF, 0 },       // 251: Magenta
		{ 0x00, 0xFF, 0xFF, 0 },       // 252: Cyan
		{ 0xFF, 0xFF, 0xFF, 0 },       // 253: White again (duplicate of 246)
		{ 0xFF, 0xFB, 0xF0, 0 },       // 254: Light Gray (UI highlight)
		{ 0xA0, 0xA0, 0xA4, 0 }        // 255: "Button face" gray
	};

	for (int i = 0; i < 10; ++i) {
		lps->palPalEntry[246 + i] = sysColorsEnd[i];
	}

	// Set up the system palette with the palette data
	_systemPalette.CreatePalette(lps);
	free(lps);

	return true;
}

bool CWinApp::InitInstance() {
	return true;
}

int CWinApp::ExitInstance() {
	return 0;
}

bool CWinApp::SaveAllModified() {
	_settings.save();
	return true;
}

int CWinApp::Run() {
	if (isQuitting())
		return 0;

	// Ensure app has been initialized
	assert(_defaultFont.font());

	SetCursor(LoadCursor(IDC_ARROW));

	if (!m_pMainWnd) {
		m_pMainWnd = GetActiveWindow();
		assert(m_pMainWnd);
	} else {
		assert(m_pMainWnd);
		SetActiveWindow(m_pMainWnd);
	}

	runEventLoop();

	SaveAllModified();
	ExitInstance();

	return 0;
}

void CWinApp::SetDialogBkColor() {
}

HCURSOR CWinApp::LoadStandardCursor(const char *lpszCursorName) {
	return _cursors.loadCursor((intptr)lpszCursorName);
}

HCURSOR CWinApp::LoadCursor(const char *lpszResourceName) {
	return _cursors.loadCursor((intptr)lpszResourceName);
}

HCURSOR CWinApp::LoadCursor(unsigned int nIDResource) {
	return _cursors.loadCursor(nIDResource);
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

HFONT CWinApp::getFont(const char *lpszFacename, int nHeight) {
	if ((!lpszFacename || !*lpszFacename) || !nHeight)
		// Use default font
		return _fonts.getFont("MS Sans Serif", 8);
	else
		return _fonts.getFont(lpszFacename, nHeight);
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

void CWinApp::CloseAllDocuments(bool bEndSession) {
	if (m_pDocManager != nullptr)
		m_pDocManager->CloseAllDocuments(bEndSession);
}

unsigned int CWinApp::GetProfileInt(const char *lpszSection,
        const char *lpszEntry, int nDefault) {
	return _settings[lpszSection].getInt(lpszEntry, nDefault);
}

void CWinApp::WriteProfileInt(const char *lpszSection,
        const char *lpszEntry, int nValue) {
	_settings[lpszSection].setInt(lpszEntry, nValue);
}

CString CWinApp::GetProfileString(const char *lpszSection,
		const char *lpszEntry, const char *lpszDefault) {
	Common::String str = _settings[lpszSection].getString(lpszEntry, lpszDefault);
	return CString(str.c_str());
}

bool CWinApp::WriteProfileString(const char *lpszSection,
		const char *lpszEntry, const char *lpszValue) {
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
	_currentPalette.SetPaletteEntries(pal);
	g_system->getPaletteManager()->setPalette(pal);
}

byte CWinApp::getColor(COLORREF color) const {
	if (_currentPalette.isEmpty())
		return 0;

	if (color <= 0xff || (color >> 24) == 1)
		return (byte)(color & 0xff);

	return _currentPalette.palette()->findBestColor(
		GetRValue(color),
		GetGValue(color),
		GetBValue(color)
	);
}

HRSRC CWinApp::findResource(const char *lpName, const char *lpType) {
	return _resources.findResource(lpName, lpType);
}

size_t CWinApp::sizeofResource(HRSRC hResInfo) {
	return _resources.resourceSize(hResInfo);
}

HGLOBAL CWinApp::loadResource(HRSRC hResInfo) {
	return _resources.loadResource(hResInfo);
}

void *CWinApp::lockResource(HGLOBAL hResData) {
	return GlobalLock(hResData);
}

void CWinApp::unlockResource(HGLOBAL hResData) {
	GlobalUnlock(hResData);
}

bool CWinApp::freeResource(HGLOBAL hResData) {
	GlobalFree(hResData);
	return true;
}

CHandleMap<CGdiObject> *CWinApp::afxMapHGDIOBJ(bool bCreate) {
	if (m_pmapHGDIOBJ == nullptr && bCreate)
		m_pmapHGDIOBJ = new CHandleMap<CGdiObject>();

	return m_pmapHGDIOBJ;
}

CHandleMap<CDC> *CWinApp::afxMapHDC(bool bCreate) {
	if (m_pmapHDC == nullptr && bCreate)
		m_pmapHDC = new CHandleMap<CDC>();

	return m_pmapHDC;
}

CHandleMap<CWnd> *CWinApp::afxMapWnd(bool bCreate) {
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

const char *CWinApp::AfxRegisterWndClass(unsigned int nClassStyle,
		HCURSOR hCursor, HBRUSH hbrBackground, HICON hIcon) {
	return "ScummVMWindow";
}

bool CWinApp::GetClassInfo(HINSTANCE hInstance,
		const char *lpClassName, LPWNDCLASS lpWndClass) {
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

int LoadString(HINSTANCE hInstance, unsigned int uID,
		char *lpBuffer, int cchBufferMax) {
	if (lpBuffer == nullptr || cchBufferMax <= 0)
		return 0;

	// Calculate which string block (resource ID) contains this string
	unsigned int blockID = (uID / 16) + 1;
	unsigned int indexInBlock = uID % 16;

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

HMODULE LoadLibrary(const char *lpLibFileName) {
	error("LoadLibrary is unsupported");
}

void FreeLibrary(HMODULE hModule) {
	error("FreeLibrary is unsupported");
}


FARPROC GetProcAddress(HMODULE hModule,
                       const char *lpProcName) {
	error("TODO: GetProcAddress");
}

HMODULE GetModuleHandle(const char *lpModuleName) {
	error("TODO: GetModuleHandle");
}

const char *AfxRegisterWndClass(unsigned int nClassStyle,
        HCURSOR hCursor, HBRUSH hbrBackground, HICON hIcon) {
	return AfxGetApp()->AfxRegisterWndClass(nClassStyle,
		hCursor, hbrBackground, hIcon);
}

bool GetClassInfo(HINSTANCE hInstance,
		const char *lpClassName, LPWNDCLASS lpWndClass) {
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
