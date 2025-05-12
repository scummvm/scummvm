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

#include "common/file.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "bagel/mfc/global_functions.h"

namespace Bagel {
namespace MFC {

HGLOBAL GlobalAlloc(UINT uFlags, SIZE_T dwBytes) {
	error("TODO: GlobalAlloc");
}

LPVOID GlobalLock(HGLOBAL hMem) {
	error("TODO: GlobalLock");
}

BOOL GlobalUnlock(HGLOBAL hMem) {
	error("TODO: GlobalUnlock");
}

HGLOBAL GlobalFree(HGLOBAL hMem) {
	error("TODO: GlobalFree");
}

SIZE_T GlobalSize(HGLOBAL hMem) {
	error("TODO: GlobalSize");
}

SIZE_T GlobalCompact(DWORD dwMinFree) {
	error("TODO: GlobalCompact");
}

int MessageBox(HWND hWnd, LPCSTR lpText,
		LPCSTR lpCaption, UINT uType) {
	error("%s %s", lpText, lpCaption);
	return 0;
}

int MessageBox(LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
	error("%s %s", lpText, lpCaption);
	return 0;
}

UINT GetPrivateProfileInt(LPCSTR lpAppName,
		LPCSTR lpKeyName, INT nDefault, LPCSTR lpFileName) {
	error("TODO: GetPrivateProfileInt");
}

BOOL WritePrivateProfileString(
		LPCSTR lpAppName, LPCSTR lpKeyName,
		LPCSTR lpString, LPCSTR lpFileName) {
	error("TODO: WritePrivateProfileString");
}

HCURSOR LoadCursor(HINSTANCE hInstance,
		LPCSTR lpCursorName) {
	error("TODO: LoadCursor");
}

void SetCursor(HCURSOR hCursor) {
	error("TODO: SetCursor");
}

HTASK GetCurrentTask() {
	error("TODO: GetCurrentTask");
}

FARPROC MakeProcInstance(FARPROC lpProc, HINSTANCE hInstance) {
	error("TODO: MakeProcInstance");
}

void FreeProcInstance(FARPROC lpProc) {
	error("TODO: FreeProcInstance");
}

HHOOK SetWindowsHookEx(int idHook,
		HOOKPROC lpfn, HINSTANCE hmod, HTASK dwThreadId) {
	error("TODO: SetWindowsHookEx");
}

BOOL UnhookWindowsHookEx(HHOOK hhk) {
	error("TODO: UnhookWindowsHookEx");
}

LRESULT CallNextHookEx(HHOOK hhk, int nCode,
	WPARAM wParam, LPARAM lParam) {
	error("TODO: CallNextHookEx");
}

UINT_PTR SetTimer(UINT_PTR nIDEvent, UINT nElapse,
		void (CALLBACK *lpfnTimer)(HWND, UINT, UINT_PTR, DWORD)) {
	error("TODO: SetTimer");
}

BOOL KillTimer(UINT_PTR nIDEvent) {
	error("TODO: KillTimer");
}

void Sleep(UINT milli) {
	g_system->delayMillis(milli);
}

DWORD GetTickCount() {
	return g_system->getMillis();
}

bool FileExists(const char *filename) {
	return Common::File::exists(filename);
}

long FileLength(const char *filename) {
	Common::File f;
	return f.open(filename) ? f.size() : -1;
}

BOOL PeekMessage(LPMSG lpMsg, HWND hWnd,
	UINT wMsgFilterMin, UINT wMsgFilterMax,
	UINT wRemoveMsg) {
	error("TODO: PeekMessage");
}

void TranslateMessage(LPMSG lpMsg) {
	error("TODO: TranslateMessage");
}

void DispatchMessage(LPMSG lpMsg) {
	error("TODO: DispatchMessage");
}

BOOL PostMessage(HWND hWnd, UINT Msg,
		WPARAM wParam, LPARAM lParam) {
	error("TODO: PostMessage");
}

HRSRC FindResource(HMODULE hModule,
		LPCSTR lpName, LPCSTR lpType) {
	error("TODO: FindResource");
}

HGLOBAL LoadResource(HMODULE hModule, HRSRC hResInfo) {
	error("TODO: LoadResource");
}

LPVOID LockResource(HGLOBAL hResData) {
	error("TODO: LockResource");
}

BOOL FreeResource(HGLOBAL hResData) {
	error("TODO: FreeResource");
}

} // namespace MFC
} // namespace Bagel
