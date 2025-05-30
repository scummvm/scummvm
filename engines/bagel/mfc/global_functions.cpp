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
#include "bagel/mfc/afxwin.h"
#include "bagel/mfc/global_functions.h"

namespace Bagel {
namespace MFC {

struct MemBlock {
	byte *_ptr;
	size_t _size;
	MemBlock(size_t size) :
		_ptr(new byte[size]),
		_size(size) {}
	~MemBlock() {
		delete[] _ptr;
	}
};

HGLOBAL GlobalAlloc(UINT uFlags, SIZE_T dwBytes) {
	MemBlock *block = new MemBlock(dwBytes);
	if (uFlags == GMEM_ZEROINIT)
		Common::fill(block->_ptr, block->_ptr + dwBytes, 0);

	return block;
}

LPVOID GlobalLock(HGLOBAL hMem) {
	return ((MemBlock *)hMem)->_ptr;
}

BOOL GlobalUnlock(HGLOBAL hMem) {
	return true;
}

HGLOBAL GlobalFree(HGLOBAL hMem) {
	delete (MemBlock *)hMem;
	return nullptr;
}

SIZE_T GlobalSize(HGLOBAL hMem) {
	return ((MemBlock *)hMem)->_size;
}

SIZE_T GlobalCompact(DWORD dwMinFree) {
	// No implementation
	return 999999;
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

int MessageBox(LPCSTR lpText) {
	error("%s %s", lpText);
	return 0;
}

UINT GetPrivateProfileInt(LPCSTR lpAppName,
                          LPCSTR lpKeyName, INT nDefault, LPCSTR lpFileName) {
	error("TODO: GetPrivateProfileInt");
}

extern DWORD GetPrivateProfileString(LPCSTR lpAppName,
                                     LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR  lpReturnedString,
                                     DWORD  nSize, LPCSTR lpFileName) {
	error("TODO: GetPrivateProfileString");
}


BOOL WritePrivateProfileString(
    LPCSTR lpAppName, LPCSTR lpKeyName,
    LPCSTR lpString, LPCSTR lpFileName) {
	error("TODO: WritePrivateProfileString");
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

UINT_PTR SetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT nElapse,
                  void (CALLBACK *lpfnTimer)(HWND, UINT, UINT, DWORD)) {
	error("TODO: SetTimer");
}

BOOL KillTimer(HWND hWnd, UINT_PTR nIDEvent) {
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
	// No implementation
}

void DispatchMessage(LPMSG lpMsg) {
	CWnd *wnd = CWnd::FromHandle(lpMsg->hwnd);
	wnd->SendMessage(lpMsg->message,
	                 lpMsg->wParam, lpMsg->lParam);
}

BOOL PostMessage(HWND hWnd, UINT Msg,
                 WPARAM wParam, LPARAM lParam) {
	CWnd *wnd = CWnd::FromHandle(hWnd);
	return wnd->PostMessage(Msg, wParam, lParam);
}

LRESULT SendMessage(HWND hWnd, UINT Msg,
                    WPARAM wParam, LPARAM lParam) {
	CWnd *wnd = CWnd::FromHandle(hWnd);
	return wnd->SendMessage(Msg, wParam, lParam);
}

HRSRC FindResource(HMODULE hModule,
                   LPCSTR lpName, LPCSTR lpType) {
	error("TODO: FindResource");
}

size_t SizeofResource(HMODULE hModule, HRSRC hResInfo) {
	error("TODO: SizeofResource");
}

HGLOBAL LoadResource(HMODULE hModule, HRSRC hResInfo) {
	error("TODO: LoadResource");
}

LPVOID LockResource(HGLOBAL hResData) {
	error("TODO: LockResource");
}

void UnlockResource(HGLOBAL hResData) {
	error("TODO: UnlockResource");
}

BOOL FreeResource(HGLOBAL hResData) {
	error("TODO: FreeResource");
}

} // namespace MFC
} // namespace Bagel
