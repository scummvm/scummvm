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

CWinApp::CWinApp(const char *appName) : CWinThread() {
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
	error("TODO: BeginWaitCursor");
}

void CWinApp::EndWaitCursor() {
	error("TODO: EndWaitCursor");
}


CWinApp *AfxGetApp() {
	error("TODO: AfxGetApp");
}

HINSTANCE AfxGetInstanceHandle() {
	error("TODO: AfxGetInstanceHandle");
}

} // namespace MFC
} // namespace Bagel
