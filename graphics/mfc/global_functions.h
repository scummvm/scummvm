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

#ifndef GRAPHICS_MFC_GLOBAL_FUNCTIONS_H
#define GRAPHICS_MFC_GLOBAL_FUNCTIONS_H

#include "common/stream.h"
#include "graphics/mfc/afxstr.h"
#include "graphics/mfc/minwindef.h"
#include "graphics/mfc/winnt.h"

namespace Graphics {
namespace MFC {

class CWinApp;
class CWnd;

/* Global Memory Flags */
#define GMEM_FIXED          0x0000
#define GMEM_MOVEABLE       0x0002
#define GMEM_NOCOMPACT      0x0010
#define GMEM_NODISCARD      0x0020
#define GMEM_ZEROINIT       0x0040
#define GMEM_MODIFY         0x0080
#define GMEM_DISCARDABLE    0x0100
#define GMEM_NOT_BANKED     0x1000
#define GMEM_SHARE          0x2000
#define GMEM_DDESHARE       0x2000
#define GMEM_NOTIFY         0x4000
#define GMEM_LOWER          GMEM_NOT_BANKED
#define GMEM_VALID_FLAGS    0x7F72
#define GMEM_INVALID_HANDLE 0x8000

#define GHND                (GMEM_MOVEABLE | GMEM_ZEROINIT)
#define GPTR                (GMEM_FIXED | GMEM_ZEROINIT)

#define MB_ICONHAND                 0x00000010L
#define MB_ICONQUESTION             0x00000020L
#define MB_ICONEXCLAMATION          0x00000030L
#define MB_ICONASTERISK             0x00000040L
#define MB_ICONINFORMATION          MB_ICONASTERISK
#define MB_ICONSTOP                 MB_ICONHAND

extern CWinApp *AfxGetApp();
extern CWnd *AfxGetMainWnd();
extern HINSTANCE AfxGetInstanceHandle();
extern int LoadString(HINSTANCE hInstance,
                      unsigned int uID, char *lpBuffer, int cchBufferMax);
extern HMODULE LoadLibrary(const char *lpLibFileName);
extern void FreeLibrary(HMODULE hModule);
extern FARPROC GetProcAddress(HMODULE hModule,
                              const char * lpProcName);
extern HMODULE GetModuleHandle(const char *lpModuleName);

extern const char *AFXAPI AfxRegisterWndClass(unsigned int nClassStyle,
        HCURSOR hCursor = 0, HBRUSH hbrBackground = 0, HICON hIcon = 0);
extern bool GetClassInfo(HINSTANCE hInstance,
	const char *lpClassName, LPWNDCLASS lpWndClass);
extern int GetSystemMetrics(int nIndex);

extern HGLOBAL GlobalAlloc(unsigned int uFlags, size_t dwBytes);
extern void *GlobalLock(HGLOBAL hMem);
extern bool GlobalUnlock(HGLOBAL hMem);
extern HGLOBAL GlobalFree(HGLOBAL hMem);
extern size_t GlobalSize(HGLOBAL hMem);
extern size_t GlobalCompact(uint32 dwMinFree);
#define LocalAlloc          GlobalAlloc
#define LocalLock           GlobalLock
#define LocalUnlock         GlobalUnlock
#define LocalFree           GlobalFree
#define LocalCompact        GlobalCompact
#define GetFreeSpace(w)     (0x100000L)

extern int MessageBox(HWND hWnd, const char *lpText,
                      const char *lpCaption, unsigned int uType);
extern int MessageBox(const char *lpText, const char *lpCaption, unsigned int uType = 0);
extern int MessageBox(const char *lpText);
extern unsigned int GetPrivateProfileInt(const char *lpAppName,
                                 const char *lpKeyName, int nDefault, const char *lpFileName);
extern uint32 GetPrivateProfileString(const char *lpAppName,
                                     const char *lpKeyName, const char *lpDefault, char * lpReturnedString,
                                     uint32  nSize, const char *lpFileName);

extern bool WritePrivateProfileString(
    const char *lpAppName, const char *lpKeyName,
    const char *lpString, const char *lpFileName);

extern HTASK GetCurrentTask();
extern FARPROC MakeProcInstance(FARPROC lpProc, HINSTANCE hInstance);
extern void FreeProcInstance(FARPROC lpProc);
extern HHOOK SetWindowsHookEx(int idHook,
                              HOOKPROC lpfn, HINSTANCE hmod, HTASK dwThreadId);
extern bool UnhookWindowsHookEx(HHOOK hhk);
extern LRESULT CallNextHookEx(HHOOK hhk, int nCode,
                              WPARAM wParam, LPARAM lParam);
extern uintptr SetTimer(HWND hWnd, uintptr nIDEvent, unsigned int nElapse,
    void (CALLBACK *lpfnTimer)(HWND, unsigned int, uintptr, uint32) = nullptr);
extern bool KillTimer(HWND hWnd, uintptr nIDEvent);
extern void Sleep(unsigned int milli);
extern uint32 GetTickCount();

extern Common::SeekableReadStream *OpenFile(const char *filename);
extern bool FileExists(const char *filename);
extern long FileLength(const char *filename);

extern bool PeekMessage(LPMSG lpMsg, HWND hWnd,
                        unsigned int wMsgFilterMin, unsigned int wMsgFilterMax,
                        unsigned int wRemoveMsg);
extern void TranslateMessage(LPMSG lpMsg);
extern void DispatchMessage(LPMSG lpMsg);
extern bool PostMessage(HWND hWnd, unsigned int Msg,
                        WPARAM wParam, LPARAM lParam);
extern LRESULT SendMessage(HWND hWnd, unsigned int Msg,
                           WPARAM wParam, LPARAM lParam);

extern HINSTANCE AfxGetResourceHandle();
extern HINSTANCE AfxFindResourceHandle(const char *lpszName, const char *lpszType);
extern HRSRC FindResource(HMODULE hModule,
    const char *lpName, const char *lpType);
extern size_t SizeofResource(HMODULE hModule, HRSRC hResInfo);
extern HGLOBAL LoadResource(HMODULE hModule, HRSRC hResInfo);
extern void *LockResource(HGLOBAL hResData);
extern void UnlockResource(HGLOBAL hResData);
extern bool FreeResource(HGLOBAL hResData);
extern HFONT CreateFontIndirect(const LOGFONT *lf);

extern bool AfxExtractSubString(CString &rString, const char *lpszFullString,
	int iSubString, char chSep = '\n');

inline char *strUpper(char *s) {
	for (char *curr = s; *curr; ++curr)
		*curr = toupper(*curr);
	return s;
}

inline char *strLower(char *s) {
	for (char *curr = s; *curr; ++curr)
		*curr = tolower(*curr);
	return s;
}

} // namespace MFC
} // namespace Graphics

#endif
