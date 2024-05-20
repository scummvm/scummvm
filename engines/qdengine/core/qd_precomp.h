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
#define BOOL          bool
#define CALLBACK
#define HMMIO         void *
#define HWND          void *
#define HINSTANCE     void *
#define HMODULE       void *
#define HRESULT       int
#define FARPROC       int *
#define LPARAM        long int *
#define LPSTR         char *
#define LPCSTR        const char *
#define LRESULT       long
#define MMCKINFO      int
#define PASCAL
#define WINAPI
#define UINT          unsigned int
#define WPARAM        int
#define WAVEFORMATEX  int
#define DWORD         int
#define COLORREF      unsigned
#define BYTE          unsigned char
#define DRIVE_CDROM   5
#define _MAX_EXT      4
#define _MAX_PATH     260
#define IDYES         6
#define IDOK          1
#define IDNO          7
#define IDCANCEL      2
#define MAX_PATH      260
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
#define WM_SYSCOMMAND 0x0112
#define WM_SYSKEYDOWN 0x0104
#define WM_KEYDOWN    0x0100
#define WM_KEYUP      0x0101
#define WM_QUIT       0x0012
#define WM_SETTEXT    0x000C
#define SW_HIDE       0
#define SW_SHOWNORMAL 1
#define MB_YESNO      0x00000004L
#define MB_TASKMODAL  0x00002000L
#define MB_OKCANCEL   0x00000001L
#define SW_SHOWMAXIMIZED   3
#define MB_ICONEXCLAMATION 0x00000030L
#define SEM_FAILCRITICALERRORS 1

typedef struct MSG {
	HWND   hwnd;
	UINT   message;
	int wParam;
	int lParam;
	DWORD  time;
	DWORD  lPrivate;
};

void* INVALID_HANDLE_VALUE = reinterpret_cast<void*>(-1);

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
