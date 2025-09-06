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
	MemBlock(size_t size, bool clearFlag) : _size(size) {
		_ptr = (byte *)malloc(size);
		if (clearFlag)
			Common::fill(_ptr, _ptr + size, 0);
	}
	~MemBlock() {
		free(_ptr);
	}
};

HGLOBAL GlobalAlloc(UINT uFlags, size_t dwBytes) {
	return new MemBlock(dwBytes,
		uFlags == GMEM_ZEROINIT);
}

void *GlobalLock(HGLOBAL hMem) {
	return ((MemBlock *)hMem)->_ptr;
}

BOOL GlobalUnlock(HGLOBAL hMem) {
	return true;
}

HGLOBAL GlobalFree(HGLOBAL hMem) {
	delete (MemBlock *)hMem;
	return nullptr;
}

size_t GlobalSize(HGLOBAL hMem) {
	return ((MemBlock *)hMem)->_size;
}

size_t GlobalCompact(uint32 dwMinFree) {
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
	error("%s", lpText);
	return 0;
}

UINT GetPrivateProfileInt(LPCSTR lpAppName,
                          LPCSTR lpKeyName, int nDefault, LPCSTR lpFileName) {
	return AfxGetApp()->GetProfileInt(lpAppName, lpKeyName, nDefault);
}

extern uint32 GetPrivateProfileString(LPCSTR lpAppName,
        LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString,
        uint32  nSize, LPCSTR lpFileName) {
	CString str = AfxGetApp()->GetProfileString(lpAppName,
		lpKeyName, lpDefault);
	Common::strcpy_s(lpReturnedString, nSize, str.c_str());
	return true;
}


BOOL WritePrivateProfileString(
		LPCSTR lpAppName, LPCSTR lpKeyName,
		LPCSTR lpString, LPCSTR lpFileName) {
	AfxGetApp()->WriteProfileString(lpAppName,
		lpKeyName, lpString);
	return true;
}

HTASK GetCurrentTask() {
	return 0;
}

FARPROC MakeProcInstance(FARPROC lpProc, HINSTANCE hInstance) {
	// Just return the passed proc as is
	return lpProc;
}

void FreeProcInstance(FARPROC lpProc) {
	// No implementation needed
}

HHOOK SetWindowsHookEx(int idHook, HOOKPROC lpfn,
		HINSTANCE hmod, HTASK dwThreadId) {
	// We only currently support keyboard in ScummVM
	assert(idHook == WH_KEYBOARD);

	return AfxGetApp()->HookKeyboard(lpfn);
}

BOOL UnhookWindowsHookEx(HHOOK hhk) {
	AfxGetApp()->UnhookKeyboard(hhk);
	return true;
}

LRESULT CallNextHookEx(HHOOK hhk, int nCode,
        WPARAM wParam, LPARAM lParam) {
	error("TODO: CallNextHookEx");
}

UINT_PTR SetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT nElapse,
		void (CALLBACK * lpfnTimer)(HWND, UINT, UINT_PTR, uint32)) {
	return AfxGetApp()->SetTimer(hWnd, nIDEvent, nElapse, lpfnTimer);
}

BOOL KillTimer(HWND hWnd, UINT_PTR nIDEvent) {
	return AfxGetApp()->KillTimer(hWnd, nIDEvent);
}

void Sleep(UINT milli) {
	g_system->delayMillis(milli);
}

uint32 GetTickCount() {
	return g_system->getMillis();
}

Common::SeekableReadStream *OpenFile(const char *filename) {
	Common::String fname(filename);
	if (fname.hasPrefix(".\\"))
		fname = Common::String(fname.c_str() + 2);

	// In cases where ..\ is used as a prefix, presume
	// that we're only a single level deep, and the remainder
	// is relative to the game's root folder. So we can
	// just strip it off, and Common::File will find it
	if (fname.hasPrefix("..\\"))
		fname = Common::String(fname.c_str() + 3);

	fname.replace('\\', '/');

	Common::File *f = new Common::File();
	if (f->open(Common::Path(fname)))
		return f;

	delete f;
	return nullptr;
}

bool FileExists(const char *filename) {
	Common::SeekableReadStream *rs = OpenFile(filename);
	bool result = rs != nullptr;
	delete rs;
	return result;
}

long FileLength(const char *filename) {
	Common::SeekableReadStream *rs = OpenFile(filename);
	long result = rs ? rs->size() : -1;
	delete rs;
	return result;
}

BOOL PeekMessage(LPMSG lpMsg, HWND hWnd,
                 UINT wMsgFilterMin, UINT wMsgFilterMax,
                 UINT wRemoveMsg) {
	return AfxGetApp()->PeekMessage(lpMsg, hWnd,
		wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
}

void TranslateMessage(LPMSG lpMsg) {
	AfxGetApp()->TranslateMessage(lpMsg);
}

void DispatchMessage(LPMSG lpMsg) {
	AfxGetApp()->DispatchMessage(lpMsg);
}

BOOL PostMessage(HWND hWnd, UINT Msg,
        WPARAM wParam, LPARAM lParam) {
	return AfxGetApp()->PostMessage(hWnd, Msg, wParam, lParam);
}

LRESULT SendMessage(HWND hWnd, UINT Msg,
        WPARAM wParam, LPARAM lParam) {
	CWnd *wnd = CWnd::FromHandle(hWnd);
	return wnd->SendMessage(Msg, wParam, lParam);
}

HINSTANCE AfxGetResourceHandle() {
	// Can be left as a default value for ScummVM
	return nullptr;
}

HINSTANCE AfxFindResourceHandle(LPCSTR lpszName, LPCSTR lpszType) {
	return AfxGetResourceHandle();
}

HRSRC FindResource(HMODULE hModule,
        LPCSTR lpName, LPCSTR lpType) {
	return AfxGetApp()->findResource(lpName, lpType);
}

size_t SizeofResource(HMODULE hModule, HRSRC hResInfo) {
	return AfxGetApp()->sizeofResource(hResInfo);
}

HGLOBAL LoadResource(HMODULE hModule, HRSRC hResInfo) {
	return AfxGetApp()->loadResource(hResInfo);
}

void *LockResource(HGLOBAL hResData) {
	return AfxGetApp()->lockResource(hResData);
}

void UnlockResource(HGLOBAL hResData) {
	return AfxGetApp()->unlockResource(hResData);
}

BOOL FreeResource(HGLOBAL hResData) {
	return AfxGetApp()->freeResource(hResData);
}

HFONT CreateFontIndirect(const LOGFONT *lf) {
	return AfxGetApp()->getFont(
		lf->lfFaceName, lf->lfHeight);
}

BOOL AfxExtractSubString(CString &rString, LPCSTR lpszFullString,
		int iSubString, char chSep) {
	if (lpszFullString == nullptr)
		return FALSE;

	while (iSubString--) {
		lpszFullString = strchr(lpszFullString, chSep);
		if (!lpszFullString) {
			rString.Empty();
			return FALSE;
		}

		lpszFullString++;	// Point past the separator
	}

	LPCSTR lpchEnd = strchr(lpszFullString, chSep);
	int nLen = !lpchEnd ? strlen(lpszFullString) :
		(int)(lpchEnd - lpszFullString);
	ASSERT(nLen >= 0);

	rString = CString(lpszFullString, nLen);
	return TRUE;
}

} // namespace MFC
} // namespace Bagel
