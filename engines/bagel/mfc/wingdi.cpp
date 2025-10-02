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
#include "bagel/mfc/gfx/blitter.h"
#include "bagel/mfc/gfx/palette_map.h"

namespace Bagel {
namespace MFC {

int GetDeviceCaps(HDC hdc, int index) {
	CDC::Impl *dc = (CDC::Impl *)hdc;
	const CBitmap::Impl *bitmap = (CBitmap::Impl *)dc->_bitmap;

	switch (index) {
	case HORZRES:
		return bitmap->w;
	case VERTRES:
		return bitmap->h;
	case BITSPIXEL:
		return bitmap->format.bytesPerPixel * 8;
	case RASTERCAPS:
		return (bitmap->format.bytesPerPixel == 1 ? RC_PALETTE : 0) |
			RC_BITBLT |
			RC_BITMAP64 |
			RC_DI_BITMAP |
			RC_DIBTODEV |
			RC_STRETCHBLT;

	default:
		break;
	}

	error("TODO: CDC::GetDeviceCaps");
	return 0;
}

HDC CreateCompatibleDC(HDC hdc) {
	return new CDC::Impl(hdc);
}

HDC GetDC(HWND hWnd) {
	if (hWnd) {
		CWnd *wnd = CWnd::FromHandle(hWnd);
		CDC *dc = wnd->GetDC();
		return dc->m_hDC;
	} else {
		// Screen DC, so set the screen rect
		CDC::Impl *dc = new CDC::Impl();
		dc->setScreenRect();
		return dc;
	}
}

int ReleaseDC(HWND hWnd, HDC hDC) {
	if (hWnd) {
		// Window based DC
		CWnd *wnd = CWnd::FromHandle(hWnd);
		CDC *dc = CDC::FromHandle(hDC);
		return wnd->ReleaseDC(dc);
	} else {
		// System screen DC
		delete (CDC::Impl *)hDC;
		return 1;
	}
}

bool DeleteDC(HDC hDC) {
	delete (CDC::Impl *)hDC;
	return true;
}

bool DeleteObject(HGDIOBJ ho) {
	delete (CGdiObjectImpl *)ho;
	return true;
}

int GetObject(HANDLE h, int c, void *pv) {
	error("TODO: GetObject");
}

HDC BeginPaint(HWND hWnd, LPPAINTSTRUCT lpPaint) {
	CWnd *wnd = static_cast<CWnd *>(hWnd);
	return wnd->BeginPaint(lpPaint);
}

bool EndPaint(HWND hWnd, const PAINTSTRUCT *lpPaint) {
	CWnd *wnd = static_cast<CWnd *>(hWnd);
	return wnd->EndPaint(lpPaint);
}

intptr DialogBoxParam(HINSTANCE hInstance,
                       const char *lpTemplateName, HWND hWndParent,
                       DLGPROC lpDialogFunc, LPARAM dwInitParam) {
	error("TODO: DialogBoxParam");
}

bool IsWindow(HWND hWnd) {
	error("TODO: IsWindow");
}

bool SetWindowText(HWND hWnd, const char *lpszString) {
	CWnd *wnd = CWnd::FromHandle(hWnd);
	wnd->SetWindowText(lpszString);
	return true;
}

int GetWindowText(HWND hWnd, char *lpszStringBuf, int nMaxCount) {
	CWnd *wnd = CWnd::FromHandle(hWnd);
	return wnd->GetWindowText(lpszStringBuf, nMaxCount);
}

bool ScreenToClient(HWND hWnd, LPPOINT lpPoint) {
	CWnd *wnd = CWnd::FromHandle(hWnd);
	wnd->ScreenToClient(lpPoint);
	return true;
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

HPALETTE SelectPalette(HDC hdc, HPALETTE hPal, bool bForceBkgd) {
	auto *surf = static_cast<CDC::Impl *>(hdc);
	return surf->selectPalette(hPal, bForceBkgd);
}

unsigned int RealizePalette(HDC hdc) {
	auto *surf = static_cast<CDC::Impl *>(hdc);
	surf->realizePalette();
	return 256;
}

HBITMAP CreateDIBitmap(HDC hdc, CONST BITMAPINFOHEADER *pbmih, uint32 flInit,
		CONST void *pjBits, CONST BITMAPINFO *pbmi, unsigned int iUsage) {
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
	Graphics::Palette pal((pbmi && pbmih->biClrUsed) ? pbmih->biClrUsed : 0);
	if (pbmi && pbmih->biClrUsed) {
		for (uint i = 0; i < pbmih->biClrUsed; ++i) {
			const RGBQUAD &col = pbmi->bmiColors[i];
			pal.set(i, col.rgbRed, col.rgbGreen, col.rgbBlue);
		}

//		bitmap->setPalette(pal.data(), 0, pal.size());
	}

	// If pixels are provided, copy them over
	if (flInit & CBM_INIT) {
		assert(pjBits);

		if (bitmap->format.bytesPerPixel == 1 && !pal.empty()) {
			const CDC::Impl *destDC = (const CDC::Impl *)hdc;
			const CPalette::Impl *destPal = (const CPalette::Impl * )destDC->_palette;

			Gfx::PaletteMap palMap(pal, *destPal);
			palMap.map((const byte *)pjBits, (byte *)bitmap->getPixels(),
				bitmap->w * bitmap->h);

		} else {
			size_t size = bitmap->w * bitmap->h *
				bitmap->format.bytesPerPixel;
			Common::copy((const byte *)pjBits, (const byte *)pjBits + size,
				(byte *)bitmap->getPixels());
		}
	}

	return bitmap;
}

int GetDIBits(HDC hdc, HBITMAP hbm, unsigned int start, unsigned int cLines,
              void *lpvBits, LPBITMAPINFO lpbmi, unsigned int usage) {
	error("TODO: GetDIBits");
}

bool BitBlt(HDC hdc, int xDest, int yDest, int width, int height,
        HDC hdcSrc, int xSrc, int ySrc, uint32 rop) {
	CDC::Impl *srcDc = (CDC::Impl *)hdcSrc;
	CDC::Impl *destDc = (CDC::Impl *)hdc;
	CBitmap::Impl *src = (CBitmap::Impl *)srcDc->_bitmap;
	CBitmap::Impl *dest = (CBitmap::Impl *)destDc->_bitmap;
	uint bgColor = destDc->getBkColor();

	Gfx::blit(src, dest,
		Common::Rect(xSrc, ySrc, xSrc + width, ySrc + height),
		Common::Point(xDest, yDest),
		bgColor, rop);
	return true;
}

bool StretchBlt(HDC hdcDest, int xDest, int yDest, int wDest, int hDest,
                HDC hdcSrc, int xSrc, int ySrc, int wSrc, int hSrc, uint32 rop) {
	CDC *srcDC = CDC::FromHandle(hdcSrc);
	CDC *destDC = CDC::FromHandle(hdcDest);
	return destDC->StretchBlt(xDest, yDest, wDest, hDest,
		srcDC, xSrc, ySrc, wSrc, hSrc, rop);
}

int SetStretchBltMode(HDC hdc, int mode) {
	CDC *dc = CDC::FromHandle(hdc);
	return dc->SetStretchBltMode(mode);
}

int StretchDIBits(HDC hdc, int xDest, int yDest, int DestWidth, int DestHeight,
        int xSrc, int ySrc, int SrcWidth, int SrcHeight,
        CONST void *lpBits, CONST BITMAPINFO *lpbmi, unsigned int iUsage, uint32 rop) {
	error("TODO: StretchDIBits");
}

int GetTextExtent(HDC hdc, const char *text, size_t len) {
	CDC *dc = CDC::FromHandle(hdc);
	CSize size = dc->GetTextExtent(CString(text, len));
	return size.cx;
}

bool GetTextMetrics(HDC hdc, LPTEXTMETRIC lptm) {
	CDC *dc = CDC::FromHandle(hdc);
	return dc->GetTextMetrics(lptm);
}

intptr GetWindowWord(HWND hWnd, int nIndex) {
	assert(nIndex == GWW_HINSTANCE);
	return 0;
}

int AddFontResource(const char *fontName) {
	return AfxGetApp()->addFontResource(fontName);
}

bool RemoveFontResource(const char *fontName) {
	return AfxGetApp()->removeFontResource(fontName);
}

int SetScrollPos(HWND hWnd, int /*nBar*/,
		int nPos, bool bRedraw) {
	CWnd *wnd = CWnd::FromHandle(hWnd);
	return wnd->SetScrollPos(nPos, bRedraw);
}

void SetScrollRange(HWND hWnd, int /*nBar*/,
		int nMinPos, int nMaxPos, bool bRedraw) {
	CWnd *wnd = CWnd::FromHandle(hWnd);
	wnd->SetScrollRange(nMinPos, nMaxPos, bRedraw);
}

bool ClipCursor(const RECT *lpRect) {
	// Ignored in ScummVM
	return false;
}

bool GetCursorPos(LPPOINT lpPoint) {
	Common::Point mousePos = AfxGetApp()->getMousePos();
	lpPoint->x = mousePos.x;
	lpPoint->y = mousePos.y;
	return true;
}

bool SetCursorPos(int x, int y) {
	AfxGetApp()->setMousePos(Common::Point(x, y));
	return true;
}

bool SetCapture(HWND hWnd) {
	AfxGetApp()->SetCapture(hWnd);
	return true;
}

bool ReleaseCapture() {
	AfxGetApp()->ReleaseCapture();
	return true;
}

HWND GetCapture() {
	return AfxGetApp()->GetCapture();
}

HCURSOR LoadCursor(HINSTANCE hInstance,
                   const char *lpCursorName) {
	return AfxGetApp()->LoadCursor(lpCursorName);
}

HCURSOR SetCursor(HCURSOR hCursor) {
	return AfxGetApp()->SetCursor(hCursor);
}

int ShowCursor(bool bShow) {
	g_system->showMouse(bShow);
	return 0;
}

bool LineDDA(int x0, int y0, int x1, int y1, LINEDDAPROC lpProc, CDC *cdc) {
	if (!lpProc)
		return false;

	int dx = ABS(x1 - x0);
	int dy = ABS(y1 - y0);

	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;

	int err = dx - dy;

	for (;;) {
		lpProc(x0, y0, cdc);  // Call the callback for this pixel

		if (x0 == x1 && y0 == y1)
			break;

		int e2 = 2 * err;

		if (e2 > -dy)
		{
			err -= dy;
			x0 += sx;
		}

		if (e2 < dx)
		{
			err += dx;
			y0 += sy;
		}
	}

	return true;
}

byte GetRValue(COLORREF color) {
	return  ((byte)(color & 0xFF));
}

byte GetGValue(COLORREF color) {
	return ((byte)((color >> 8) & 0xFF));
}

byte GetBValue(COLORREF color) {
	return ((byte)((color >> 16) & 0xFF));
}

HWND GetDlgItem(HWND hDlg, int nIDDlgItem) {
	error("TODO: GetDlgItem");
}

bool EndDialog(HWND hDlg, intptr nResult) {
	error("TODO: EndDialog");
}

bool SetDlgItemInt(HWND hDlg, int nIDDlgItem,
                   unsigned int uValue, bool bSigned) {
	error("TODO: SetDlgItemInt");
}

bool CheckRadioButton(HWND hDlg, int nIDFirstButton,
                      int nIDLastButton, int nIDCheckButton) {
	error("TODO: CheckRadioButton");
}

uint32 GetSysColor(int nIndex) {
	switch (nIndex) {
	case COLOR_3DHIGHLIGHT:
		return RGB(255, 255, 255);
	case COLOR_BTNSHADOW:
		return RGB(128, 128, 128);
	case COLOR_BTNFACE:
		return RGB(192, 192, 192);
	case COLOR_BTNTEXT:
	case COLOR_WINDOWTEXT:
		return RGB(0, 0, 0);
	default:
		error("Unknown GetSysColor value");
		break;
	}
}

HBRUSH GetSysColorBrush(int nIndex) {
	switch (nIndex) {
	case COLOR_WINDOW:
		return new CBrush::Impl(RGB(255, 255, 255));

	default:
		error("Unknown GetSysColorBrush value");
		break;
	}
}

bool DestroyMenu(HMENU hMenu) {
	error("TODO: DestroyMenu");
}

void SetActiveWindow(HWND hWnd) {
	error("TODO: SetActiveWindow");
}

} // namespace MFC
} // namespace Bagel
