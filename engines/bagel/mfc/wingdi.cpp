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
#include "bagel/mfc/global_functions.h"
#include "bagel/mfc/afxwin.h"

namespace Bagel {
namespace MFC {

int GetDeviceCaps(HDC hdc, int index) {
	error("TODO: GetDeviceCaps");
}

HDC CreateCompatibleDC(HDC hdc) {
	return new CDC::Impl(hdc);
}

HDC GetDC(HWND hWnd) {
	assert(!hWnd);

	CDC::Impl *dc = new CDC::Impl();
	dc->setScreenRect();
	return dc;
}

int ReleaseDC(HWND hWnd, HDC hDC) {	
	// In ScummVM window creation is hard-coded with CS_OWNDC.
	// Which means, the only DCs passed here should be global
	// temporary DCs that can be immediately deleted
	assert(!hWnd && hDC);
	delete (CDC::Impl *)hDC;
	return 1;
}

BOOL DeleteDC(HDC hDC) {
	delete (CDC::Impl *)hDC;
	return true;
}

BOOL DeleteObject(HGDIOBJ ho) {
	error("TODO: DeleteObject");
}

int GetObject(HANDLE h, int c, LPVOID pv) {
	error("TODO: GetObject");
}

HDC BeginPaint(HWND hWnd, LPPAINTSTRUCT lpPaint) {
	CWnd *wnd = static_cast<CWnd *>(hWnd);
	return wnd->BeginPaint(lpPaint);
}

BOOL EndPaint(HWND hWnd, const PAINTSTRUCT *lpPaint) {
	CWnd *wnd = static_cast<CWnd *>(hWnd);
	return wnd->EndPaint(lpPaint);
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
	CDC::Impl *dc = (CDC::Impl *)hdc;
	return dc->Attach(h);
}

HGDIOBJ GetStockObject(int i) {
	switch (i) {
	case SYSTEM_FONT:
		return AfxGetApp()->getDefaultFont();
	default:
		break;
	}

	error("TODO: GetStockObject value");
}

HPALETTE SelectPalette(HDC hdc, HPALETTE hPal, BOOL bForceBkgd) {
	auto *surf = static_cast<CDC::Impl *>(hdc);
	return surf->selectPalette(hPal);
}

UINT RealizePalette(HDC hdc) {
	auto *surf = static_cast<CDC::Impl *>(hdc);
	surf->realizePalette();
	return 256;
}

HBITMAP CreateDIBitmap(HDC hdc, CONST BITMAPINFOHEADER *pbmih, DWORD flInit,
		CONST VOID *pjBits, CONST BITMAPINFO *pbmi, UINT iUsage) {
	CBitmap::Impl *bitmap = new CBitmap::Impl();

	// Figure out the pixel format
	assert(pbmih->biSize == 40 && pbmih->biPlanes == 1);
	Graphics::PixelFormat format;

	switch (pbmih->biBitCount) {
	case 1:
		// Monochrome. Since ScummVM doesn't support it directly,
		// I use a standard 8bpp surface, but in the format's
		// aLoss field, I use a value of 255 as a flag.
		// This should be okay, as loss & shift aren't used for 8bpp.
		format = Graphics::PixelFormat::createFormatCLUT8();
		format.aLoss = 255;
		break;

	case 8:
		format = Graphics::PixelFormat::createFormatCLUT8();
		break;
	case 16:
		format = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
		break;
	case 32:
		format = Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24);
		break;
	default:
		error("Unknown biBitCount");
		break;
	}

	// Create the bitmap
	bitmap->create(pbmih->biWidth, pbmih->biHeight, format);

	// Set the palette
	if (pbmi && pbmih->biClrUsed) {
		Graphics::Palette pal(pbmih->biClrUsed);
		for (uint i = 0; i < pbmih->biClrUsed; ++i) {
			const RGBQUAD &col = pbmi->bmiColors[i];
			pal.set(i, col.rgbRed, col.rgbGreen, col.rgbBlue);
		}

		bitmap->setPalette(pal.data(), 0, pal.size());
	}

	// If pixels are provided, copy them over
	if (pjBits) {
		size_t size = bitmap->w * bitmap->h *
			bitmap->format.bytesPerPixel;
		Common::copy((const byte *)pjBits, (const byte *)pjBits + size,
			(byte *)bitmap->getPixels());
	}

	return bitmap;
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

int GetTextExtent(HDC hdc, LPCSTR text, size_t len) {
	error("TODO: GetTextExtent");
}

BOOL GetTextMetrics(HDC hdc, LPTEXTMETRIC lptm) {
	CDC *dc = CDC::FromHandle(hdc);
	return dc->GetTextMetrics(lptm);
}

intptr GetWindowWord(HWND hWnd, int nIndex) {
	error("TODO: GetWindowWord");
}

int AddFontResource(LPCSTR fontName) {
	return AfxGetApp()->addFontResource(fontName);
}

bool RemoveFontResource(LPCSTR fontName) {
	return AfxGetApp()->removeFontResource(fontName);
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

BOOL SetCapture(HWND hWnd) {
	AfxGetApp()->SetCapture(hWnd);
	return true;
}

BOOL ReleaseCapture() {
	AfxGetApp()->ReleaseCapture();
	return true;
}

HWND GetCapture() {
	return AfxGetApp()->GetCapture();
}

HCURSOR LoadCursor(HINSTANCE hInstance,
                   LPCSTR lpCursorName) {
	return AfxGetApp()->LoadCursor(lpCursorName);
}

HCURSOR SetCursor(HCURSOR hCursor) {
	return AfxGetApp()->SetCursor(hCursor);
}

int ShowCursor(BOOL bShow) {
	g_system->showMouse(bShow);
	return 0;
}

BOOL LineDDA(int nXStart, int nYStart,
             int nXEnd, int nYEnd, LINEDDAPROC lpLineFunc,
             LPARAM lpData) {
	error("TODO: LineDDA");
}

BYTE GetRValue(COLORREF color) {
	return  ((BYTE)(color & 0xFF));
}

BYTE GetGValue(COLORREF color) {
	return ((BYTE)((color >> 8) & 0xFF));
}

BYTE GetBValue(COLORREF color) {
	return ((BYTE)((color >> 16) & 0xFF));
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
	switch (nIndex) {
	case COLOR_3DHIGHLIGHT:
		return RGB(255, 255, 255);
	case COLOR_3DSHADOW:
		return RGB(128, 128, 128);
	case COLOR_BTNFACE:
		return RGB(192, 192, 192);
	default:
		error("Unknown GetSysColor value");
		break;
	}
}

BOOL DestroyMenu(HMENU hMenu) {
	error("TODO: DestroyMenu");
}

void SetActiveWindow(HWND hWnd) {
	error("TODO: SetActiveWindow");
}

} // namespace MFC
} // namespace Bagel
