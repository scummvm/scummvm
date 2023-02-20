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

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winspool.h>

#include "backends/printing/printman.h"
#include "win32-printman.h"
#include "common/ustr.h"


class Win32PrintingManager : public PrintingManager {
public:
	virtual ~Win32PrintingManager();

	void printImage(Common::String jobName, const Graphics::ManagedSurface &surf);
	void printImage(Common::String jobName, const byte *pixels, const byte *palette, uint32 width, uint32 height);

private:
	HDC createDefaultPrinterContext();
	HDC createPrinterContext(LPWSTR devName);
	HPALETTE buildPalette(const byte *paletteData);
	HBITMAP buildBitmap(HDC hdc, const byte *pixels, const byte *palette, uint width, uint height);
	HBITMAP buildBitmap(HDC hdc, const Graphics::ManagedSurface &surf);
};


Win32PrintingManager::~Win32PrintingManager() {}

void Win32PrintingManager::printImage(Common::String jobName, const Graphics::ManagedSurface &surf) {

	HDC hdcPrint = createDefaultPrinterContext();

	HDC hdcImg = CreateCompatibleDC(hdcPrint);

	HBITMAP bitmap = buildBitmap(hdcPrint, surf);
	if (!bitmap)
		goto delDC;

	Escape(hdcPrint, STARTDOC, jobName.size(), jobName.c_str(), NULL);

	SelectObject(hdcImg, bitmap);

	BitBlt(hdcPrint, 0, 0, surf.w, surf.h, hdcImg, 0, 0, SRCCOPY);

	Escape(hdcPrint, NEWFRAME, 0, NULL, NULL);
	Escape(hdcPrint, ENDDOC, 0, NULL, NULL);

	DeleteObject(bitmap);
delDC:
	DeleteDC(hdcImg);
	DeleteDC(hdcPrint);

	
}

void Win32PrintingManager::printImage(Common::String jobName, const byte *pixels, const byte *paletteData, uint32 width, uint32 height) {
	HDC hdcPrint = createDefaultPrinterContext();

	HDC hdcImg = CreateCompatibleDC(hdcPrint);
	
	HBITMAP bitmap = buildBitmap(hdcPrint, pixels, paletteData, width, height);
	if (!bitmap)
		goto delDC;

	Escape(hdcPrint, STARTDOC, jobName.size(), jobName.c_str(), NULL);

	SelectObject(hdcImg, bitmap);

	BitBlt(hdcPrint, 0, 0, width, height, hdcImg, 0, 0, SRCCOPY);
	//TransparentBlt(hdcPrint, 0, 0, width, height, hdcImg, 0, 0, width, height, transpColor);

	Escape(hdcPrint, NEWFRAME, 0, NULL, NULL);
	Escape(hdcPrint, ENDDOC, 0, NULL, NULL);

	DeleteObject(bitmap);
delDC:
	DeleteDC(hdcImg);
	DeleteDC(hdcPrint);
}


HDC Win32PrintingManager::createDefaultPrinterContext() {
	wchar_t szPrinter[MAX_PATH];
	BOOL success;
	DWORD cchPrinter(ARRAYSIZE(szPrinter));

	success=GetDefaultPrinterW(szPrinter, &cchPrinter);
	if (!success)
		return NULL;

	return createPrinterContext(szPrinter);
}
HDC Win32PrintingManager::createPrinterContext(LPWSTR devName) {
	HANDLE handle;
	BOOL success;

	success=OpenPrinterW(devName, &handle, NULL);
	if (!success)
		return NULL;

	int size = DocumentPropertiesW(NULL, handle, devName, NULL, NULL, 0);
	DEVMODE *devmode = (DEVMODE *)malloc(size);
	DocumentPropertiesW(NULL, handle, devName, devmode, NULL, DM_OUT_BUFFER);

	ClosePrinter(handle);

	HDC printerDC = CreateDCW(L"WINSPOOL", devName, NULL, devmode);
	return printerDC;
}

HPALETTE Win32PrintingManager::buildPalette(const byte *paletteData) {
	LOGPALETTE *lpal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + (256-1)*sizeof(PALETTEENTRY));

	if (!lpal)
		return NULL;

	lpal->palNumEntries = 256;
	lpal->palVersion = 1;

	for (uint i = 0; i < 256; ++i, paletteData+=3) {
		lpal->palPalEntry[i].peRed = paletteData[0];
		lpal->palPalEntry[i].peGreen = paletteData[1];
		lpal->palPalEntry[i].peBlue = paletteData[2];
		lpal->palPalEntry[i].peFlags = 0;
	}

	HPALETTE pal = CreatePalette(lpal);

	free(lpal);

	return pal;
}

HBITMAP Win32PrintingManager::buildBitmap(HDC hdc, const byte *pixels, const byte *palette, uint width, uint height) {
	const uint colorCount = 256;
	BITMAPINFO *bitmapInfo=(BITMAPINFO *)malloc(sizeof(BITMAPINFO) + sizeof(RGBQUAD)*(colorCount-1));

	if (!bitmapInfo)
		return NULL;

	bitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo->bmiHeader.biWidth = width;
	bitmapInfo->bmiHeader.biHeight = -((LONG)height);//Blame the OS2 team for bitmaps being upside down
	bitmapInfo->bmiHeader.biPlanes = 1;
	bitmapInfo->bmiHeader.biBitCount = 8;
	bitmapInfo->bmiHeader.biCompression = BI_RGB;
	bitmapInfo->bmiHeader.biSizeImage = 0;
	bitmapInfo->bmiHeader.biClrUsed = colorCount;
	bitmapInfo->bmiHeader.biClrImportant = colorCount;

	for (uint colorIndex = 0; colorIndex < colorCount; ++colorIndex, palette+=3) {
		bitmapInfo->bmiColors[colorIndex].rgbRed = palette[0];
		bitmapInfo->bmiColors[colorIndex].rgbGreen = palette[1];
		bitmapInfo->bmiColors[colorIndex].rgbBlue = palette[2];
		bitmapInfo->bmiColors[colorIndex].rgbReserved = 0;
	}

	HBITMAP bitmap = CreateDIBitmap(hdc, &(bitmapInfo->bmiHeader), CBM_INIT, pixels, bitmapInfo, DIB_RGB_COLORS);

	free(bitmapInfo);

	return bitmap;
}

HBITMAP Win32PrintingManager::buildBitmap(HDC hdc, const Graphics::ManagedSurface &surf) {
	const uint colorCount = 256;
	BITMAPINFO *bitmapInfo = (BITMAPINFO *)malloc(sizeof(BITMAPINFO) + sizeof(RGBQUAD) * (colorCount - 1));

	if (!bitmapInfo)
		return NULL;

	bitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo->bmiHeader.biWidth = surf.w;
	bitmapInfo->bmiHeader.biHeight = -((LONG)surf.h); // Blame the OS2 team for bitmaps being upside down
	bitmapInfo->bmiHeader.biPlanes = 1;
	bitmapInfo->bmiHeader.biBitCount = 8;
	bitmapInfo->bmiHeader.biCompression = (surf.format.isCLUT8()?BI_RGB:BI_BITFIELDS);
	bitmapInfo->bmiHeader.biSizeImage = 0;
	bitmapInfo->bmiHeader.biClrUsed = colorCount;
	bitmapInfo->bmiHeader.biClrImportant = colorCount;

	if (surf.format.isCLUT8()) {
		byte *colors = new byte[colorCount * 3];
		surf.grabPalette(colors, 0, colorCount);

		byte *palette = colors;
		for (uint colorIndex = 0; colorIndex < colorCount; ++colorIndex, palette += 3) {
			bitmapInfo->bmiColors[colorIndex].rgbRed = palette[0];
			bitmapInfo->bmiColors[colorIndex].rgbGreen = palette[1];
			bitmapInfo->bmiColors[colorIndex].rgbBlue = palette[2];
			bitmapInfo->bmiColors[colorIndex].rgbReserved = 0;
		}

		delete[] colors;
	}

	HBITMAP bitmap = CreateDIBitmap(hdc, &(bitmapInfo->bmiHeader), CBM_INIT, surf.getPixels(), bitmapInfo, DIB_RGB_COLORS);

	free(bitmapInfo);

	return bitmap;
}

PrintingManager *createWin32PrintingManager() {
	return new Win32PrintingManager();
}

#endif // WIN32
