#include "qdengine/core/qd_precomp.h"

int dummy = 1;

void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext) {
	warning("STUB: _splitpath");
	return;
}

int MessageBox(HWND hWnd, const char *lpText, const char *lpCaption, UINT uType) {
	warning("STUB: MessageBox");
	return 0;
}

void ShellExecute(HWND hwnd, const char *lpOperation, const char *lpFile, const char *lpParameters, const char *lpDirectory, int nShowCmd) {
	warning("STUB: ShellExecute");
	return;
}

void Sleep(int dwMilliseconds) {
	warning("STUB: Sleep");
	return;
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
	return 0;
}

bool DeleteObject(HANDLE hObject) {
	warning("STUB: DeleteObject()");
	return false;
}

HWND CreateWindowEx(DWORD dwExStyle, const char *lpClassName, const char *lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, int hMenu, HINSTANCE hInstance, void *lpParam) {
	warning("STUB: CreteWindowEx()");
	return 0;
}


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

HRESULT CoCreateInstance(REFCLSID rclsid, void *pUnkOuter, DWORD dwClsContext, REFIID riid, void **ppv) {
	warning("STUB: CoCreateInstance()");
	return 0;
}

int ov_open(void *datasource, void *vf, const char *initial, long ibytes) {
	warning("STUB: ov_open_callbacks()");
	return 0;
}
int ov_read(void *vf, char *buffer, int length, int bigendianp, int word, int sgned, int *bitstream) {
	warning("STUB: ov_read()");
	return 0;
}

int ov_time_total(void *vf, int i) {
	warning("STUB: ov_time_total()");
	return 0;
}


int ov_clear(void *vf) {
	warning("STUB: ov_clear()");
	return 0;
}

int ov_time_tell(void *vf) {
	warning("STUB: ov_time_total()");
	return 0;
}

DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) {
	warning("STUB: WaitForSingleObject()");
	return 0;
}


bool SetEvent(HANDLE hEvent) {
	warning("STUB: SetEvent()");
	return false;
}

bool SetThreadPriority(HANDLE hThread, int nPriority) {
	warning("STUB: SetThreadPriority()");
	return false;
}

HANDLE CreateEvent(void *lpEventAttributes, bool bManualReset, bool bInitialState, const char *lpName) {
	warning("STUB: CreateEvent()");
	return 0;
}

bool ResetEvent(HANDLE hEvent) {
	warning("STUB: ResetEvent()");
	return false;
}

void initclock() {
	warning("STUB: initClock()");
	return;
}

bool WINAPI GetNumberOfConsoleInputEvents(HANDLE hConsoleInput, unsigned long *lpcNumberOfEvents) {
	warning("STUB: GetNumberOfConsoleInputEvents()");
	return false;
}

bool WINAPI ReadConsoleInput(HANDLE hConsoleInput, void *lpBuffer, unsigned long nLength, unsigned long *lpNumberOfEventsRead) {
	warning("STUB: ReadConsoleInput()");
	return false;
}

bool TranslateMessage(const MSG *lpMsg) {
	warning("STUB: TranslateMessage()");
	return false;
}

bool DispatchMessage(const MSG *lpMsg) {
	warning("STUB: DispatchMessage()");
	return false;
}

int GetAsyncKeyState(int vKey) {
	warning("STUB: GetAsyncKeyState()");
	return 0;
}

int GetKeyState(int nVirtKey) {
	warning("STUB: GetKeyState()");
	return 0;
}
DWORD timeGetTime() {
	warning("STUB: timeGetTime()");
	return 0;
}

namespace mpp {
void MpegInitLibrary(void *dsound_device) {
	warning("STUB: MpegInitLibrary()");
}
void MpegDeinitLibrary() {
	warning("STUB: MpegDeinitLibrary()");
};
void MpegOpenToPlay(const char *fname, bool cycled) {
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

int xtInitApplication() {
	warning("STUB: xtInitApplication()");
	return 0;
};
void xtDoneApplication() {
	warning("STUB: xtDoneApplication()");
	return;
};
void *INVALID_HANDLE_VALUE = reinterpret_cast<void *>(-1);
