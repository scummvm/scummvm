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

#include "common/system.h"
#include "common/textconsole.h"
#include "bagel/mfc/wingdi.h"

namespace Bagel {
namespace MFC {

int GetDeviceCaps(HDC hdc, int index) {
	error("TODO: GetDeviceCaps");
}

HDC CreateCompatibleDC(HDC hdc) {
	error("TODO: CreateCompatibleDC");
}

int ReleaseDC(HWND hWnd, HDC hDC) {
	error("TODO: ReleaseDC");
}

BOOL DeleteDC(HDC hdc) {
	error("TODO: DeleteDC");
}

BOOL DeleteObject(HGDIOBJ ho) {
	error("TODO: DeleteObject");
}

HDC GetDC(HWND hWnd) {
	error("TODO: GetDC");
}

int GetObject(HANDLE h, int c, LPVOID pv) {
	error("TODO: GetObject");
}

HDC BeginPaint(HWND hWnd, LPPAINTSTRUCT lpPaint) {
	error("TODO: BeginPaint");
}

BOOL EndPaint(HWND hWnd, const PAINTSTRUCT *lpPaint) {
	error("TODO: EndPaint");
}

INT_PTR DialogBoxParam(HINSTANCE hInstance,
		LPCTSTR lpTemplateName, HWND hWndParent,
		DLGPROC lpDialogFunc, LPARAM dwInitParam) {
	error("TODO: DialogBoxParam");
}

BOOL IsWindow(HWND hWnd) {
	error("TODO: IsWindow");
}

BOOL SetWindowText(HWND hWnd, LPCSTR lpszString) {
	error("TODO: SetWindowText");
}

int GetWindowText(HWND hWnd, LPSTR lpszStringBuf, int nMaxCount) {
	error("TODO: GetWindowText");
}

BOOL ScreenToClient(HWND hWnd, LPPOINT lpPoint) {
	error("TODO: ScreenToClient");
}


HGDIOBJ SelectObject(HDC hdc, HGDIOBJ h) {
	error("TODO: SelectObject");
}

HPALETTE SelectPalette(HDC hdc, HPALETTE hPal, BOOL bForceBkgd) {
	error("TODO: SelectPalette");
}

UINT RealizePalette(HDC hdc) {
	error("TODO: RealizePalette");
}

HBITMAP CreateDIBitmap(HDC hdc, CONST BITMAPINFOHEADER *pbmih,
		DWORD flInit, CONST VOID *pjBits, CONST BITMAPINFO *pbmi, UINT iUsage) {
	error("TODO: CreateDIBitmap");
}

int GetDIBits(HDC hdc, HBITMAP hbm, UINT start, UINT cLines,
	LPVOID lpvBits, LPBITMAPINFO lpbmi, UINT usage) {
	error("TODO: GetDIBits");
}

BOOL BitBlt(HDC hdc, int x, int y, int cx, int cy,
		HDC hdcSrc, int x1, int y1, DWORD rop) {
	error("TODO: BitBlt");
}

BOOL StretchBlt(HDC hdcDest, int xDest, int yDest, int wDest, int hDest,
		HDC hdcSrc, int xSrc, int ySrc, int wSrc, int hSrc, DWORD rop) {
	error("TODO: StretchBlt");
}

int SetStretchBltMode(HDC hdc, int mode) {
	error("TODO: SetStretchBltMode");
}

int StretchDIBits(HDC hdc, int xDest, int yDest, int DestWidth, int DestHeight,
		int xSrc, int ySrc, int SrcWidth, int SrcHeight,
		CONST void *lpBits, CONST BITMAPINFO *lpbmi, UINT iUsage, DWORD rop) {
	error("TODO: StretchDIBits");
}

WORD GetWindowWord(HWND hWnd, int nIndex) {
	error("TODO: GetWindowWord");
}

int WINAPI AddFontResource(LPCSTR fontName) {
	error("TODO: AddFontResource");
}

int SetScrollPos(HWND hWnd, int nBar,
		int nPos, BOOL bRedraw) {
	error("TODO: SetScrollPos");
}

BOOL SetScrollRange(HWND hWnd, int nBar,
		int nMinPos, int nMaxPos, BOOL bRedraw) {
	error("TODO: SetScrollRange");
}

BOOL ClipCursor(const RECT *lpRect) {
	error("TODO: ClipCursor");
}

BOOL GetCursorPos(LPPOINT lpPoint) {
	error("TODO: GetCursorPos");
}

BOOL SetCursorPos(int x, int y) {
	g_system->warpMouse(x, y);
	return TRUE;
}

BOOL SetCapture() {
	error("TODO: SetCapture");
}

BOOL ReleaseCapture() {
	error("TODO: ReleaseCapture");
}

HCURSOR LoadCursor(HINSTANCE hInstance,
	LPCSTR lpCursorName) {
	error("TODO: LoadCursor");
}

HCURSOR SetCursor(HCURSOR hCursor) {
	error("TODO: SetCursor");
}

int ShowCursor(BOOL bShow) {
	error("TODO: ShowCursor");
}

BOOL LineDDA(int nXStart, int nYStart,
		int nXEnd, int nYEnd, LINEDDAPROC lpLineFunc,
		LPARAM lpData) {
	error("TODO: LineDDA");
}

BYTE GetRValue(COLORREF color) {
	error("TODO: GetRValue");
}

BYTE GetGValue(COLORREF color) {
	error("TODO: GetGValue");
}

BYTE GetBValue(COLORREF color) {
	error("TODO: GetBValue");
}

HWND GetDlgItem(HWND hDlg, int nIDDlgItem) {
	error("TODO: GetDlgItem");
}

BOOL EndDialog(HWND hDlg, INT_PTR nResult) {
	error("TODO: EndDialog");
}

BOOL SetDlgItemInt(HWND hDlg, int nIDDlgItem,
		UINT uValue, BOOL bSigned) {
	error("TODO: SetDlgItemInt");
}

BOOL CheckRadioButton(HWND hDlg, int nIDFirstButton,
	int nIDLastButton, int nIDCheckButton) {
	error("TODO: CheckRadioButton");
}

DWORD GetSysColor(int nIndex) {
	error("TODO: GetSysColor");
}

} // namespace MFC
} // namespace Bagel
