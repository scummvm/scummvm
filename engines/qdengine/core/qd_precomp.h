#ifndef __QD_PRECOMP_H__
#define __QD_PRECOMP_H__

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/system.h"

#define FOR_EACH(list, iterator) \
	for(iterator = (list).begin(); iterator != (list).end(); ++iterator)

// STUB: XZipStream
#define _NO_ZIP_

// STUB FIXME
#define APIENTRY
#define PASCAL
#define WINAPI
#define CALLBACK
#define BOOL          bool
#define HMMIO         void *
#define HWND          void *
#define HINSTANCE     void *
#define HANDLE        void *
#define HMODULE       void *
#define HRESULT       int
#define FARPROC       int *
#define LPARAM        long int
#define LPSTR         char *
#define LPCSTR        const char *
#define LPWSTR        wchar_t *
#define LPCWSTR       const wchar_t *
#define LPBOOL        bool *
#define LRESULT       long
#define MMCKINFO      int
#define UINT          unsigned int
#define WPARAM        int
#define WAVEFORMATEX  int
#define DWORD         int
#define COLORREF      unsigned
#define BYTE          unsigned char
#define DRIVE_CDROM   5

#define MAX_PATH      260
#define _MAX_PATH     256
#define _MAX_DRIVE    3
#define _MAX_EXT      256
#define _MAX_FNAME    256
#define _MAX_DIR      256

#define IDYES         6
#define IDOK          1
#define IDNO          7
#define IDRETRY       4
#define IDIGNORE      5
#define IDCANCEL      2

#define VK_ESCAPE     0x1B
#define VK_RETURN     0x0D
#define VK_SPACE      0x20
#define VK_LEFT       0x25
#define VK_UP         0x26
#define VK_RIGHT      0x27
#define VK_DOWN       0x28
#define VK_BACK       0x08
#define VK_DELETE     0x2E
#define VK_HOME       0x24
#define VK_END        0x23
#define VK_F10        0x79
#define VK_F9         0x78
#define VK_F5         0x74
#define VK_F6         0x75
#define VK_PAUSE      0x13
#define VK_NEXT       0x22
#define VK_PRIOR      0x21
#define PM_REMOVE     0x0001
#define SC_MAXIMIZE   0xF030

// STUB FIXME
#define WM_SYSCOMMAND  0x0112
#define WM_SYSKEYDOWN  0x0104
#define WM_SYSKEYUP    0x0105
#define WM_KEYDOWN     0x0100
#define WM_KEYUP       0x0101
#define WM_QUIT        0x0012
#define WM_SETTEXT     0x000C
#define WM_INITDIALOG  0x0110
#define WM_COMMAND     0x0111
#define WM_LBUTTONUP   0x0202
#define WM_RBUTTONUP   0x0205
#define WM_MOUSEMOVE   0x0200
#define WM_LBUTTONDOWN 0x0201
#define WM_RBUTTONDOWN 0x0204

#define MB_OK               0x00000000L
#define MB_YESNO            0x00000004L
#define MB_TASKMODAL        0x00002000L
#define MB_OKCANCEL         0x00000001L
#define MB_ICONERROR        0x00000010L
#define MK_LBUTTON          0x0001
#define MK_RBUTTON          0x0002
#define MK_MBUTTON          0x0010
#define MB_ICONEXCLAMATION  0x00000030L
#define MB_ABORTRETRYIGNORE 0x00000002L

#define SW_HIDE                0
#define SW_SHOWNORMAL          1
#define SW_SHOWMAXIMIZED       3
#define SEM_FAILCRITICALERRORS 1

#define BM_GETCHECK     0x00F0
#define BST_CHECKED     1
#define BM_SETCHECK     0x00F1
#define TBM_GETPOS      0x0400
#define TBM_GETRANGEMIN 0x0401
#define TBM_GETRANGEMAX 0x0402
#define TBM_SETPOS      0x0407
#define TBM_SETRANGE    0x0406
#define TBM_SETTICFREQ  0x0405
#define MAKELONG(a, b)  ((a) | ((b) << 16))
#define LOWORD(l)       (short)(l)
#define HIWORD(l)       ((short)(((DWORD)(l) >> 16) & 0xFFFF))
#define CB_RESETCONTENT 0x0145
#define CB_ADDSTRING    0x0143
#define CB_GETCURSEL    0x0147
#define CB_SETCURSEL    0x014E
#define CB_GETCOUNT     0x0146
#define CB_ERR 			-1
#define GWL_USERDATA    -21

#define GENERIC_READ            0x80000000
#define GENERIC_WRITE           0x40000000
#define FILE_SHARE_READ         0x00000001
#define FILE_SHARE_WRITE        0x00000002
#define OPEN_EXISTING           3
#define CREATE_ALWAYS           2
#define OPEN_ALWAYS             4
#define FILE_ATTRIBUTE_NORMAL   0x00000080
#define FILE_FLAG_RANDOM_ACCESS 0x10000000
#define FILE_FLAG_NO_BUFFERING  0x20000000
#define FILE_END                2
#define FILE_CURRENT            1
#define GMEM_FIXED              0x0000

#define STM_SETIMAGE 0x0172
#define IMAGE_BITMAP 0
#define LR_DEFAULTCOLOR 0x00000000
#define LR_LOADFROMFILE 0x00000010
#define LR_CREATEDIBSECTION 0x00002000
#define WS_EX_TOOLWINDOW 0x00000080L
#define WS_POPUP 0x80000000L
#define SS_BITMAP 0x0000000EL
#define HWND_NOTOPMOST ((HWND)-2)
#define SWP_NOSIZE 0x0001
#define SWP_SHOWWINDOW 0x0040
#define SWP_NOZORDER 0x0004
#define SW_SHOWNORMAL 1
#define SW_HIDE 0
#define SM_CXSCREEN 0
#define SM_CYSCREEN 1
#define RGN_OR 0

typedef struct MSG {
	HWND   hwnd;
	UINT   message;
	int    wParam;
	LPARAM lParam;
	DWORD  time;
	DWORD  lPrivate;
};

#define IMAGE_BITMAP 0


void *INVALID_HANDLE_VALUE = reinterpret_cast<void*>(-1);
const int CP_ACP = 0;
const int CP_UTF8 = 65001;

void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext) {
	warning("STUB: _splitpath");
}

int MessageBox(HWND hWnd, const char *lpText, const char *lpCaption, UINT uType) {
	warning("STUB: MessageBox");
	return 0;
}

void ShellExecute(HWND hwnd, const char *lpOperation, const char *lpFile, const char *lpParameters, const char *lpDirectory, int nShowCmd) {
	warning("STUB: ShellExecute");
}

void Sleep(int dwMilliseconds) {
	warning("STUB: Sleep");
}

int GetLogicalDrives() {
	warning("STUB: GetLogicalDrives");
	return 0;
}

uint GetDriveType(LPCSTR lpRootPathName) {
	warning("STUB: GetDriveTypeA");
	return 0;
}

FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName) {
	warning("STUB: GetProcAddress");
	return 0;
}

bool FreeLibrary(void *hModule) {
	warning("STUB: FreeLibrary");
	return false;
}

HMODULE LoadLibrary(const char *lpLibFileName) {
	warning("STUB: LoadLibrary");
	return 0;
}

bool ShowWindow(HWND hWnd, int nCmdShow) {
	warning("STUB: ShowWindow");
	return false;
}

bool CloseWindow(HWND hWnd) {
	warning("STUB: CloseWindow");
	return false;
}

bool DestroyWindow(HWND hWnd) {
	warning("STUB: DestroyWindow");
	return false;
}

bool UpdateWindow(HWND hWnd) {
	warning("STUB: UpdateWindow");
	return false;
}

bool PeekMessage(MSG *msg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
	warning("STUB: PeakMessage");
	return false;
}

bool SendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	warning("STUB: SendMessage");
	return false;
}

bool SetErrorMode(UINT uMode) {
	warning("STUB: SetErrorMode");
	return false;
}

DWORD WINAPI GetLastError() {
	warning("STUB: GetLastError");
	return 0;
}

int MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar) {
	warning("STUB: MultiByteToWideChar");
	return 0;
}

int WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar) {
	warning("STUB: WideCharToMultiByte");
	return 0;
}

LRESULT SendDlgItemMessage(HWND hDlg, int nIDDlgItem, UINT Msg, WPARAM wParam, LPARAM lParam) {
	warning("STUB: SendDlgItemMessage");
	return 0;
}

bool EndDialog(HWND hDlg, int nResult) {
	warning("STUB: EndDialog");
	return false;
}

LPARAM MAKELPARAM(short a, short b) {
	warning("STUB: MAKELPARAM");
	return 0;
}

void ZeroMemory(void *dest, size_t size) {
	warning("STUB: ZeroMemory");
}

bool SetCurrentDirectory(const char *lpPathName) {
	warning("STUB: SetCurrentDirectory");
	return false;
}

DWORD GetCurrentDirectory(DWORD nBufferLength, char *lpBuffer) {
	warning("STUB: GetCurrentDirectory");
	return 0;
}

bool CreateDirectory(const char *lpPathName, void *lpSecurityAttributes) {
	warning("STUB: CreateDirectory");
	return false;
}

char *_fullpath(char *absPath, const char *relPath, size_t maxLength) {
	warning("STUB: _fullpath");
	return 0;
}

long SetWindowLong(HWND hWnd, int nIndex, long dwNewLong) {
	warning("STUB: SetWindowLong");
	return 0;
}

HANDLE LoadImage(HINSTANCE hinst, LPCSTR name, UINT type, int cxDesired, int cyDesired, UINT fuLoad) {
	warning("STUB: LoadImage");
	return 0;
}

HMODULE GetModuleHandle(LPCSTR lpModuleName) {
	warning("STUB: GetModuleHandle");
	return 0;
}

DWORD SetFilePointer(HANDLE hfile, long lDistanceToMove, long * lpDistanceToMoveHigh, DWORD  dwMoveMethod) {
	warning("STUB: SetFilePointer");
	return 0;
}

bool CloseHandle(HANDLE hObject) {
	warning("STUB: CloseHandle");
	return false;
}

HANDLE CreateFile( LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,void *lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile
) {
	warning("STUB: CreateFile()");
	return 0;
}

bool ReadFile(HANDLE hFile, void *lpBuffer, DWORD nNumberOfBytesToRead, unsigned long *lpNumberOfBytesRead, void *lpOverlapped) {
	warning("STUB: ReadFile()");
	return false;
}

bool WriteFile(HANDLE hFile, const void *lpBuffer, DWORD nNumberOfBytesToWrite, unsigned long *lpNumberOfBytesWritten, void *lpOverlapped) {
	warning("STUB: WriteFile()");
	return false;
}

DWORD GetFileSize(HANDLE hFile, void *lpFileSizeHigh) {
	warning("STUB: GetFileSize()");
	return 0;
}

bool FlushFileBuffers(HANDLE hFile) {
	warning("STUB: FlushFileBuffers()");
	return false;
}

HRESULT CoInitialize(void *pvReserved) {
	warning("STUB: CoInitialize()");
	return 0;
}

void CoUninitialize() {
	warning("STUB: CoUninitialize()");
}

HANDLE GlobalAlloc(UINT uFlags, size_t dwBytes) {
	warning("STUB: GlobalAlloc()");
	return 0;
}

HANDLE GlobalReAlloc(HANDLE hMem, size_t dwBytes, UINT uFlags) {
	warning("STUB: GlobalReAlloc()");
	return 0;
}

void GlobalFree(HANDLE hMem) {
	warning("STUB: GlobalFree()");
}

const char *MAKEINTRESOURCE(int i) {
	warning("STUB: MAKEINTRESOURCE()");
}

bool DeleteObject(HANDLE hObject) {
	warning("STUB: DeleteObject()");
	return false;
}

HWND CreateWindowEx(DWORD dwExStyle, const char *lpClassName, const char *lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, int hMenu, HINSTANCE hInstance, void *lpParam) {
	warning("STUB: CreteWindowEx()");
	return 0;
}

struct BITMAP {
	int bmType;
	int bmWidth;
	int bmHeight;
	int bmWidthBytes;
	short bmPlanes;
	short bmBitsPixel;
	void *bmBits;
};

struct RECT {
	int left;
	int top;
	int right;
	int bottom;
};

struct HRGN {
	int i;
};

HRGN CreateRectRgn(int x1, int y1, int x2, int y2) {
	warning("STUB: CreateRectRgn()");
	return HRGN{0};
}

int CombineRgn(HRGN hrgnDest, HRGN hrgnSrc1, HRGN hrgnSrc2, int iMode) {
	warning("STUB: CombineRgn()");
	return 0;
}

int SetWindowRgn(HWND hWnd, HRGN hRgn, bool bRedraw) {
	warning("STUB: SetWindowRgn()");
	return 0;
}

#define HGDIOBJ void *
int GetObject(HGDIOBJ hgdiobj, int cbBuffer, void *lpvObject) {
	warning("STUB: GetObject()");
	return 0;
}

bool GetClientRect(HWND hWnd, RECT *lpRect) {
	warning("STUB: GetClientRect()");
	return false;
}

bool SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags) {
	warning("STUB: SetWindowPos()");
	return false;
}

int GetSystemMetrics(int nIndex) {
	warning("STUB: GetSystemMetrics()");
	return 0;
}

namespace mpp {
enum MpegState {
	MPEG_STOP=0,
	MPEG_PLAY=1,
	MPEG_PAUSE=2,
};
void MpegInitLibrary(void *dsound_device) {
	warning("STUB: MpegInitLibrary()");
}
void MpegDeinitLibrary() {
	warning("STUB: MpegDeinitLibrary()");
};
void MpegOpenToPlay(const char *fname, bool cycled = true) {
	warning("STUB: MpegOpenToPlay(%s, %d)", fname, cycled);

}
void MpegStop() {
	warning("STUB: MpegStop()");
}
void MpegPause() {
	warning("STUB: MpegPause()");
}
void MpegResume() {
	warning("STUB: MpegResume()");
}
void MpegSetVolume(int volume) {
	warning("STUB: MpegSetVolume(%d)", volume);
}
int MpegIsPlay() {
	warning("STUB: MpegIsPlay()");
	return 0;
}
}

int WINAPI engineMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow);

#ifndef _QUEST_EDITOR
#else
/*  #define   _STLP_NO_NEW_IOSTREAMS 1

    #ifdef _STLP_NO_NEW_IOSTREAMS
        #define   _STLP_NO_OWN_NAMESPACE 1
    #else
        #define _STLP_USE_OWN_NAMESPACE 1
        #define _STLP_REDEFINE_STD 1
    #endif*/
#endif // _QUEST_EDITOR

#ifndef _FINAL_VERSION
#ifndef __QD_SYSLIB__
#define __QD_DEBUG_ENABLE__
#endif
#endif

#ifdef __QD_DEBUG_ENABLE_
#define __QDBG(a) a
#else
#define __QDBG(a)
#endif

#ifdef _DEBUG
#define DBGCHECK _ASSERTE(_CrtCheckMemory())
#else
#define DBGCHECK
#endif

//#define _GR_ENABLE_ZBUFFER
#if 0
#include "qdengine/xlibs/Heap/Additional/addition.h"
#endif


#if _MSC_VER < 1300
#define for if(false); else for
#endif

#pragma warning(disable : 4786)
#pragma warning(disable : 4018)
#pragma warning(disable : 4244)

#include "qdengine/xlibs/xutil/xutil.h"
#include "qdengine/xlibs/util/xmath/xmath.h"
#include "qdengine/xlibs/util/xmath/XMathLib.h"

#ifndef _NO_ZIP_
#if 0
#include "qdengine/xlibs/XZip/XZip.h"
#endif
#else
class XZipStream : public XStream {

};
#endif

#include <list>
#include <stack>
#include <vector>
#include <string>
#if 0
#include "qdengine/xlibs/STLPort/stlport/hash_map"
#endif
#include <algorithm>
#include <functional>

#include "qdengine/xlibs/util/xmath/SafeMath.h"

#include "qdengine/core/system/app_core.h"
#include "qdengine/core/system/app_log.h"
#include "qdengine/core/system/app_error_handler.h"

#include "qdengine/core/system/input/mouse_input.h"
#include "qdengine/core/system/input/keyboard_input.h"

#include "qdengine/core/qdcore/util/qd_save_stream.h"
#include "engines/qdengine/core/qd_fwd.h"

#endif /* __QD_PRECOMP_H__ */
