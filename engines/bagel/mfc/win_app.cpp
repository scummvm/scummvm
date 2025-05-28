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
#include "bagel/mfc/global_functions.h"
#include "bagel/mfc/afxwin.h"
#include "bagel/mfc/libs/events.h"

namespace Bagel {
namespace MFC {

IMPLEMENT_DYNAMIC(CWinApp, CWinThread)

CWinApp *CWinApp::_activeApp = nullptr;

CWinApp::CWinApp(const char *appName) :
	CWinThread(), _cursors(_resources) {
	assert(!_activeApp);    // Only one app per engine
	_activeApp = this;
	Libs::Event::init();
}

CWinApp::~CWinApp() {
	_activeApp = nullptr;
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
	Libs::Event ev;

	// Poll for event
	if (!g_system->getEventManager()->pollEvent(ev)) {
		g_system->delayMillis(10);
		return true;
	}

	// Check for quit event
	if (ev.type == Common::EVENT_QUIT ||
	        ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
		return false;

	// Convert other event types
	msg = ev;
	msg.hwnd = m_pMainWnd;

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
	Gfx::Cursor *c = (Gfx::Cursor *)hCursor;
	c->showCursor();
	return hCursor;
}

void CWinApp::BeginWaitCursor() {
	error("TODO: CWinApp::BeginWaitCursor");
}

void CWinApp::EndWaitCursor() {
	error("TODO: CWinApp::EndWaitCursor");
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
	error("TODO: CWinApp::GetProfileInt");
}

void CWinApp::WriteProfileInt(LPCSTR lpszSection,
                              LPCSTR lpszEntry, int nValue) {
	error("TODO: CWinApp::WriteProfileInt");
}

void CWinApp::setDirectory(const char *folder) {
	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	_currentDirectory = gameDataDir.getChild(folder);
}

Common::FSNode CWinApp::getDirectory() const {
	return _currentDirectory;
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
	error("TODO: AfxGetInstanceHandle");
}

HINSTANCE AfxGetResourceHandle() {
	error("TODO: AfxGetResourceHandle");
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
