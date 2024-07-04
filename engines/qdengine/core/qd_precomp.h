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

#ifndef QDENGINE_CORE_QD_PRECOMP_H
#define QDENGINE_CORE_QD_PRECOMP_H
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

#pragma warning(disable : 4786)
#pragma warning(disable : 4018)
#pragma warning(disable : 4244)

#include "qdengine/xlibs/xutil/xutil.h"
#include "qdengine/xlibs/util/xmath/xmath.h"

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include <algorithm>
#include <functional>
#include <list>
#include <stack>
#include <string>
#include <vector>
#include "common/system.h"


#include "engines/qdengine/core/qd_fwd.h"
#include "qdengine/core/system/app_core.h"
#include "qdengine/core/system/app_error_handler.h"
#include "qdengine/core/system/input/mouse_input.h"
#include "qdengine/core/system/input/keyboard_input.h"
#include "qdengine/core/qdcore/util/qd_save_stream.h"
#include "qdengine/xlibs/util/xmath/SafeMath.h"

namespace QDEngine {


#define FOR_EACH(list, iterator) \
	for(iterator = (list).begin(); iterator != (list).end(); ++iterator)

// STUB: XZipStream
#define _NO_ZIP_
class XZipStream : public XStream {};

// STUB FIXME
#define APIENTRY
#define PASCAL
#define WINAPI
#define CALLBACK

#define BOOL          bool
#define HMMIO         void *
#define HWND          void *
#define HANDLE        void *
#define HMODULE       void *
#define HRESULT       int
#define HGDIOBJ       void *
#define FARPROC       int *
#define LPARAM        long int
#define LPSTR         char *
#define LPCSTR        const char *
#define LPWSTR        wchar_t *
#define LPCWSTR       const wchar_t *
#define LPBOOL        bool *
#define LRESULT       long
#define LPVOID        void *
#define MMCKINFO      int
#define UINT          unsigned int
#define WPARAM        int
#define DWORD         int
#define WORD          short
#define COLORREF      unsigned
#define BYTE          unsigned char
#define REFCLSID	  const void *
#define REFIID        const void *
#define XML_Char      char

#define LPDIRECTSOUND       int
#define LPDIRECTSOUNDBUFFER void *

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
#define WM_MBUTTONUP   0x0208
#define WM_MOUSEMOVE   0x0200
#define WM_LBUTTONDOWN 0x0201
#define WM_RBUTTONDOWN 0x0204
#define WM_MBUTTONDOWN 0x0207
#define WM_LBUTTONDBLCLK 0x0203
#define WM_RBUTTONDBLCLK 0x0206
#define WM_MBUTTONDBLCLK 0x0209

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
#define CB_ERR          -1

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

#define STM_SETIMAGE        0x0172
#define IMAGE_BITMAP        0
#define LR_DEFAULTCOLOR     0x00000000
#define LR_LOADFROMFILE     0x00000010
#define LR_CREATEDIBSECTION 0x00002000

#define WS_EX_TOOLWINDOW    0x00000080L
#define WS_POPUP            0x80000000L
#define SS_BITMAP           0x0000000EL
#define HWND_NOTOPMOST      ((HWND)-2)

#define SWP_NOSIZE      0x0001
#define SWP_SHOWWINDOW  0x0040
#define SWP_NOZORDER    0x0004
#define SW_SHOWNORMAL   1
#define SW_HIDE         0

#define SM_CXSCREEN 0
#define SM_CYSCREEN 1
#define RGN_OR      0

#define CLSID_FilterGraph 0
#define IID_IGraphBuilder 0
#define CLSCTX_INPROC     0
#define IMAGE_BITMAP      0

#define EXCEPTION_CONTINUE_EXECUTION        -1
#define EXCEPTION_CONTINUE_SEARCH            0
#define EXCEPTION_EXECUTE_HANDLER            1

#define CONTEXT_INTEGER                     0x00000002
#define CONTEXT_CONTROL                     0x00000001

#define EXCEPTION_INT_DIVIDE_BY_ZERO        0xC0000094
#define EXCEPTION_INT_OVERFLOW              0xC0000095
#define EXCEPTION_ACCESS_VIOLATION          0xC0000005
#define EXCEPTION_BREAKPOINT                0x80000003
#define EXCEPTION_DATATYPE_MISALIGNMENT     0x80000002
#define EXCEPTION_SINGLE_STEP               0x80000004
#define EXCEPTION_ARRAY_BOUNDS_EXCEEDED     0xC000008C
#define EXCEPTION_FLT_DENORMAL_OPERAND      0xC000008D
#define EXCEPTION_FLT_DIVIDE_BY_ZERO        0xC000008E
#define EXCEPTION_FLT_INEXACT_RESULT        0xC000008F
#define EXCEPTION_FLT_INVALID_OPERATION     0xC0000090
#define EXCEPTION_FLT_OVERFLOW              0xC0000091
#define EXCEPTION_FLT_STACK_CHECK           0xC0000092
#define EXCEPTION_FLT_UNDERFLOW             0xC0000093
#define EXCEPTION_PRIV_INSTRUCTION          0xC0000096

#define GSTSO_PARAMS    0x01
#define GSTSO_MODULE    0x02
#define GSTSO_SYMBOL    0x04
#define GSTSO_SRCLINE   0x08

#define KEY_EVENT       1
#define WM_ACTIVATEAPP  0x001C
#define WM_CHAR         0x0102
#define WM_SYSCHAR      0x0106
#define WAIT_OBJECT_0   0

#define DDSD_PIXELFORMAT          0x00001000
#define DDSD_WIDTH                0x00000004
#define DDSD_PITCH                0x00000008
#define DDSCAPS_BACKBUFFER        0x00000002
#define DDSCAPS_SYSTEMMEMORY      0x00000008
#define DDLOCK_SURFACEMEMORYPTR   0x00000040
#define DDSD_CAPS                 0x00000001
#define DDSD_HEIGHT               0x00000002
#define DDSCAPS_PRIMARYSURFACE    0x00000001
#define DD_OK                     0
#define DDENUMRET_OK              0
#define LPDDSURFACEDESC           DDSURFACEDESC*

#define THREAD_PRIORITY_TIME_CRITICAL 0
#define INFINITE                      0xFFFFFFFF
#define WAVE_FORMAT_PCM               0x0001

namespace mpp {

enum MpegState {
    MPEG_STOP = 0,
    MPEG_PLAY = 1,
    MPEG_PAUSE = 2,
};

void MpegInitLibrary(void *dsound_device);
void MpegDeinitLibrary();
void MpegOpenToPlay(const char *fname, bool cycled = true);
void MpegStop();
void MpegPause();
void MpegResume();
void MpegSetVolume(int volume);
int MpegIsPlay();

}

struct KEY_EVENT_RECORD {
	unsigned wVirtualScanCode;
	unsigned wVirtualKeyCode;
	unsigned wRepeatCount;
	unsigned bKeyDown;
};

struct INPUT_RECORD {
	unsigned EventType;
	union {
		KEY_EVENT_RECORD          KeyEvent;
  } Event;
};

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

//Balmer
typedef struct {
    WORD  wFormatTag;         // Format type
    WORD  nChannels;          // Number of channels (mono, stereo, etc.)
    DWORD nSamplesPerSec;     // Sample rate
    DWORD nAvgBytesPerSec;    // For buffer estimation
    WORD  nBlockAlign;        // Block size of data
    WORD  wBitsPerSample;     // Bits per sample
    WORD  cbSize;             // Size of extra format information
} WAVEFORMATEX;

typedef struct _DDSCAPS {
    DWORD dwCaps;         // capabilities of surface wanted
} DDSCAPS;

typedef struct _DDPIXELFORMAT {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwFourCC;
    union {
        DWORD dwRGBBitCount;
        DWORD dwYUVBitCount;
        DWORD dwZBufferBitDepth;
        DWORD dwAlphaBitDepth;
        DWORD dwLuminanceBitCount;
        DWORD dwBumpBitCount;
        DWORD dwPrivateFormatBitCount;
    };
    union {
        DWORD dwRBitMask;
        DWORD dwYBitMask;
        DWORD dwStencilBitDepth;
        DWORD dwLuminanceBitMask;
        DWORD dwBumpDuBitMask;
        DWORD dwOperations;
    };
    union {
        DWORD dwGBitMask;
        DWORD dwUBitMask;
        DWORD dwZBitMask;
        DWORD dwBumpDvBitMask;
        DWORD dwBumpLuminanceBitMask;
        DWORD dwRGBAlphaBitMask;
    };
    DWORD dwBBitMask;
    DWORD dwRGBZBitMask;
} DDPIXELFORMAT;

typedef struct _DDSURFACEDESC {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwHeight;
    DWORD dwWidth;
    DDPIXELFORMAT ddpfPixelFormat;
	DDSCAPS ddsCaps;
} DDSURFACEDESC;

typedef struct {
  int map[256];
  void *data;
  int (*convert)(void *data, const char *s);
  void (*release)(void *data);
} XML_Encoding;

typedef struct PCONTEXT {
	unsigned int ContextFlags;
	unsigned int Dr0;
	unsigned int Dr1;
	unsigned int Dr2;
	unsigned int Dr3;
	unsigned int Dr6;
	unsigned int Dr7;
	unsigned int FloatSave;
	unsigned int SegGs;
	unsigned int SegFs;
	unsigned int SegEs;
	unsigned int SegDs;
	unsigned int Edi;
	unsigned int Esi;
	unsigned int Ebx;
	unsigned int Edx;
	unsigned int Ecx;
	unsigned int Eax;
	unsigned int Ebp;
	unsigned int Eip;
	unsigned int SegCs;
	unsigned int EFlags;
	unsigned int Esp;
	unsigned int SegSs;
}PCONTEXT;

typedef struct PEXCEPTION_RECORD {
	unsigned int ExceptionCode;
	void *ExceptionAddress;
}PEXCEPTION_RECORD;

typedef struct _EXCEPTION_POINTERS {
	PEXCEPTION_RECORD *ExceptionRecord;
	PCONTEXT *ContextRecord;
}EXCEPTION_POINTERS;

extern void *INVALID_HANDLE_VALUE;
const int CP_ACP = 0;
const int CP_UTF8 = 65001;

void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext);
int MessageBox(HWND hWnd, const char *lpText, const char *lpCaption, UINT uType);
void ShellExecute(HWND hwnd, const char *lpOperation, const char *lpFile, const char *lpParameters, const char *lpDirectory, int nShowCmd);
int GetLogicalDrives();
uint GetDriveType(LPCSTR lpRootPathName);
FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
bool FreeLibrary(void *hModule);
HMODULE LoadLibrary(const char *lpLibFileName);
bool ShowWindow(HWND hWnd, int nCmdShow);
bool CloseWindow(HWND hWnd);
bool DestroyWindow(HWND hWnd);
bool UpdateWindow(HWND hWnd);
bool SendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
bool SetErrorMode(UINT uMode);
DWORD WINAPI GetLastError();
int MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);
int WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar);
LRESULT SendDlgItemMessage(HWND hDlg, int nIDDlgItem, UINT Msg, WPARAM wParam, LPARAM lParam);
bool EndDialog(HWND hDlg, int nResult);
LPARAM MAKELPARAM(short a, short b);
void ZeroMemory(void *dest, size_t size);
bool SetCurrentDirectory(const char *lpPathName);
DWORD GetCurrentDirectory(DWORD nBufferLength, char *lpBuffer);
bool CreateDirectory(const char *lpPathName, void *lpSecurityAttributes);
char *_fullpath(char *absPath, const char *relPath, size_t maxLength);
long SetWindowLong(HWND hWnd, int nIndex, long dwNewLong);
DWORD SetFilePointer(HANDLE hfile, long lDistanceToMove, long *lpDistanceToMoveHigh, DWORD dwMoveMethod);
bool CloseHandle(HANDLE hObject);
HANDLE CreateFile(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, void *lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
bool ReadFile(HANDLE hFile, void *lpBuffer, DWORD nNumberOfBytesToRead, unsigned long *lpNumberOfBytesRead, void *lpOverlapped);
bool WriteFile(HANDLE hFile, const void *lpBuffer, DWORD nNumberOfBytesToWrite, unsigned long *lpNumberOfBytesWritten, void *lpOverlapped);
DWORD GetFileSize(HANDLE hFile, void *lpFileSizeHigh);
bool FlushFileBuffers(HANDLE hFile);
HRESULT CoInitialize(void *pvReserved);
void CoUninitialize();
HRESULT CoCreateInstance(REFCLSID rclsid, void *pUnkOuter, DWORD dwClsContext, REFIID riid, void **ppv);
int ov_open(void *datasource, void *vf, const char *initial, long ibytes);
int ov_read(void *vf, char *buffer, int length, int bigendianp, int word, int sgned, int *bitstream);
int ov_time_total(void *vf, int i);
int ov_clear(void *vf);
int ov_time_tell(void *vf);
DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);
bool SetEvent(HANDLE hEvent);
bool SetThreadPriority(HANDLE hThread, int nPriority);
HANDLE CreateEvent(void *lpEventAttributes, bool bManualReset, bool bInitialState, const char *lpName);
bool ResetEvent(HANDLE hEvent);
void initclock();
bool WINAPI GetNumberOfConsoleInputEvents(HANDLE hConsoleInput, unsigned long *lpcNumberOfEvents);
bool WINAPI ReadConsoleInput(HANDLE hConsoleInput, void *lpBuffer, unsigned long nLength, unsigned long *lpNumberOfEventsRead);
int GetAsyncKeyState(int vKey);
int GetKeyState(int nVirtKey);
DWORD timeGetTime();


int engineMain();

} // namespace QDEngine

#endif // QDENGINE_CORE_QD_PRECOMP_H
