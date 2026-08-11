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
#include "graphics/mfc/afxwin.h"
#include "graphics/mfc/global_functions.h"

namespace Graphics {
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

HGLOBAL GlobalAlloc(unsigned int uFlags, size_t dwBytes) {
	return new MemBlock(dwBytes,
		uFlags == GMEM_ZEROINIT);
}

void *GlobalLock(HGLOBAL hMem) {
	return ((MemBlock *)hMem)->_ptr;
}

bool GlobalUnlock(HGLOBAL hMem) {
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

int MessageBox(HWND hWnd, const char *lpText,
               const char *lpCaption, unsigned int uType) {
	error("%s %s", lpText, lpCaption);
	return 0;
}

int MessageBox(const char *lpText, const char *lpCaption, unsigned int uType) {
	error("%s %s", lpText, lpCaption);
	return 0;
}

int MessageBox(const char *lpText) {
	error("%s", lpText);
	return 0;
}

unsigned int GetPrivateProfileInt(const char *lpAppName,
                          const char *lpKeyName, int nDefault, const char *lpFileName) {
	return AfxGetApp()->GetProfileInt(lpAppName, lpKeyName, nDefault);
}

extern uint32 GetPrivateProfileString(const char *lpAppName,
        const char *lpKeyName, const char *lpDefault, char *lpReturnedString,
        uint32  nSize, const char *lpFileName) {
	CString str = AfxGetApp()->GetProfileString(lpAppName,
		lpKeyName, lpDefault);
	Common::strcpy_s(lpReturnedString, nSize, str.c_str());
	return true;
}


bool WritePrivateProfileString(
		const char *lpAppName, const char *lpKeyName,
		const char *lpString, const char *lpFileName) {
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

bool UnhookWindowsHookEx(HHOOK hhk) {
	AfxGetApp()->UnhookKeyboard(hhk);
	return true;
}

LRESULT CallNextHookEx(HHOOK hhk, int nCode,
        WPARAM wParam, LPARAM lParam) {
	error("TODO: CallNextHookEx");
}

uintptr SetTimer(HWND hWnd, uintptr nIDEvent, unsigned int nElapse,
		void (CALLBACK * lpfnTimer)(HWND, unsigned int, uintptr, uint32)) {
	return AfxGetApp()->SetTimer(hWnd, nIDEvent, nElapse, lpfnTimer);
}

bool KillTimer(HWND hWnd, uintptr nIDEvent) {
	return AfxGetApp()->KillTimer(hWnd, nIDEvent);
}

void Sleep(unsigned int milli) {
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

bool PeekMessage(LPMSG lpMsg, HWND hWnd,
                 unsigned int wMsgFilterMin, unsigned int wMsgFilterMax,
                 unsigned int wRemoveMsg) {
	return AfxGetApp()->PeekMessage(lpMsg, hWnd,
		wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
}

void TranslateMessage(LPMSG lpMsg) {
	AfxGetApp()->TranslateMessage(lpMsg);
}

void DispatchMessage(LPMSG lpMsg) {
	AfxGetApp()->DispatchMessage(lpMsg);
}

bool PostMessage(HWND hWnd, unsigned int Msg,
        WPARAM wParam, LPARAM lParam) {
	return AfxGetApp()->PostMessage(hWnd, Msg, wParam, lParam);
}

LRESULT SendMessage(HWND hWnd, unsigned int Msg,
        WPARAM wParam, LPARAM lParam) {
	CWnd *wnd = CWnd::FromHandle(hWnd);
	return wnd->SendMessage(Msg, wParam, lParam);
}

HINSTANCE AfxGetResourceHandle() {
	// Can be left as a default value for ScummVM
	return nullptr;
}

HINSTANCE AfxFindResourceHandle(const char *lpszName, const char *lpszType) {
	return AfxGetResourceHandle();
}

HRSRC FindResource(HMODULE hModule,
        const char *lpName, const char *lpType) {
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

bool FreeResource(HGLOBAL hResData) {
	return AfxGetApp()->freeResource(hResData);
}

HFONT CreateFontIndirect(const LOGFONT *lf) {
	return AfxGetApp()->getFont(
		lf->lfFaceName, lf->lfHeight);
}

bool AfxExtractSubString(CString &rString, const char *lpszFullString,
		int iSubString, char chSep) {
	if (lpszFullString == nullptr)
		return false;

	while (iSubString--) {
		lpszFullString = strchr(lpszFullString, chSep);
		if (!lpszFullString) {
			rString.Empty();
			return false;
		}

		lpszFullString++;	// Point past the separator
	}

	const char *lpchEnd = strchr(lpszFullString, chSep);
	int nLen = !lpchEnd ? strlen(lpszFullString) :
		(int)(lpchEnd - lpszFullString);
	ASSERT(nLen >= 0);

	rString = CString(lpszFullString, nLen);
	return true;
}

} // namespace MFC
} // namespace Graphics
