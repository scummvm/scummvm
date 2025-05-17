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

#include "common/textconsole.h"
#include "bagel/mfc/global_functions.h"
#include "bagel/mfc/afxwin.h"

namespace Bagel {
namespace MFC {

IMPLEMENT_DYNAMIC(CWinApp, CWinThread)

CWinApp::CWinApp(const char *appName) : CWinThread() {
}

BOOL CWinApp::PreTranslateMessage(MSG *pMsg) {
	return FALSE;
}

void CWinApp::SetDialogBkColor() {
}

HCURSOR CWinApp::LoadStandardCursor(LPCSTR lpszCursorName) const {
	return MFC::LoadCursor(NULL, lpszCursorName);
}

HCURSOR CWinApp::LoadCursor(LPCSTR lpszResourceName) const {
	return MFC::LoadCursor(NULL, lpszResourceName);
}

HCURSOR CWinApp::LoadCursor(UINT nIDResource) const {
	error("TODO: CWinApp::LoadCursor");
}

void CWinApp::BeginWaitCursor() {
	error("TODO: CWinApp::BeginWaitCursor");
}

void CWinApp::EndWaitCursor() {
	error("TODO: CWinApp::EndWaitCursor");
}

void CWinApp::DoWaitCursor(int nCode) {
	error("TODO: CWinApp::DoWaitCursor");
}

void CWinApp::AddDocTemplate(CDocTemplate *pTemplate) {
	error("TODO: CWinApp::AddDocTemplate");
}

BOOL CWinApp::SaveAllModified() {
	error("TODO: CWinApp::SaveAllModified");
}

void CWinApp::CloseAllDocuments(BOOL bEndSession) {
	error("TODO: CWinApp::CloseAllDocuments");
}

CWinApp *AfxGetApp() {
	error("TODO: AfxGetApp");
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

FARPROC GetProcAddress(HMODULE hModule,
		LPCSTR lpProcName) {
	error("TODO: GetProcAddress");
}

HMODULE GetModuleHandle(LPCSTR lpModuleName) {
	error("TODO: GetModuleHandle");
}


LPCSTR AFXAPI AfxRegisterWndClass(UINT nClassStyle,
                                  HCURSOR hCursor, HBRUSH hbrBackground, HICON hIcon) {
	error("TODO: AfxRegisterWndClass");
}

int GetSystemMetrics(int nIndex) {
	error("TODO: GetSystemMetrics");
}

} // namespace MFC
} // namespace Bagel
