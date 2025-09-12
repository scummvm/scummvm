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

#ifdef USE_PRINTING
#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winspool.h>

#include "backends/printing/printman.h"
#include "backends/platform/sdl/win32/win32_wrapper.h"

#include "win32-printman.h"
#include "common/ustr.h"


class Win32PrintingManager : public Common::PrintingManager {
public:
	virtual ~Win32PrintingManager();

	void doPrint(const Graphics::ManagedSurface &surf) override;

	Common::StringArray listPrinterNames() const override;

	Common::String getDefaultPrinterName() const override;

private:
	HDC createDefaultPrinterContext();
	HDC createPrinterContext(LPTSTR devName);
	HBITMAP buildBitmap(HDC hdc, const Graphics::ManagedSurface &surf);
};


Win32PrintingManager::~Win32PrintingManager() {}

void Win32PrintingManager::doPrint(const Graphics::ManagedSurface &surf) {

	HDC hdcPrint;
	if (!_printerName.size())
		hdcPrint = createDefaultPrinterContext();
	else
		hdcPrint = createPrinterContext(Win32::stringToTchar(_printerName));

	DOCINFOA info;
	info.cbSize = sizeof(info);
	info.fwType = 0;
	info.lpszDatatype = nullptr;
	info.lpszOutput = nullptr;
	info.lpszDocName = _jobName.c_str();

	HDC hdcImg = CreateCompatibleDC(hdcPrint);

	HBITMAP bitmap = buildBitmap(hdcPrint, surf);
	if (!bitmap) {
		DeleteDC(hdcImg);
		return;
	}

	StartDocA(hdcPrint, &info);
	StartPage(hdcPrint);

	SelectObject(hdcImg, bitmap);

	BitBlt(hdcPrint, 0, 0, surf.w, surf.h, hdcImg, 0, 0, SRCCOPY);

	EndPage(hdcPrint);
	EndDoc(hdcPrint);

	DeleteObject(bitmap);
	DeleteDC(hdcImg);
	DeleteDC(hdcPrint);
}

HDC Win32PrintingManager::createDefaultPrinterContext() {
	TCHAR szPrinter[MAX_PATH];
	BOOL success;
	DWORD cchPrinter(ARRAYSIZE(szPrinter));

	success = GetDefaultPrinter(szPrinter, &cchPrinter);
	if (!success)
		return NULL;

	return createPrinterContext(szPrinter);
}

HDC Win32PrintingManager::createPrinterContext(LPTSTR devName) {
	HANDLE handle;
	BOOL success;

	success = OpenPrinter(devName, &handle, NULL);
	if (!success)
		return NULL;

	int size = DocumentProperties(NULL, handle, devName, NULL, NULL, 0);
	DEVMODE *devmode = (DEVMODE *)malloc(size);
	DocumentProperties(NULL, handle, devName, devmode, NULL, DM_OUT_BUFFER);

	if (devmode->dmFields & DM_ORIENTATION) {
		devmode->dmOrientation = (_orientation == kPageOrientationPortrait) ? DMORIENT_PORTRAIT : DMORIENT_LANDSCAPE;
	}

	DocumentProperties(NULL, handle, devName, devmode, devmode, DM_OUT_BUFFER | DM_IN_BUFFER);

	ClosePrinter(handle);

	HDC printerDC = CreateDC(TEXT("WINSPOOL"), devName, NULL, devmode);
	return printerDC;
}

HBITMAP Win32PrintingManager::buildBitmap(HDC hdc, const Graphics::ManagedSurface &surf) {
	const uint colorCount = 256;
	BITMAPINFO *bitmapInfo = (BITMAPINFO *)malloc(sizeof(BITMAPINFO) + sizeof(RGBQUAD) * (colorCount - 1));

	if (!bitmapInfo)
		return NULL;

	bitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo->bmiHeader.biWidth = surf.w;
	bitmapInfo->bmiHeader.biHeight = -((LONG)surf.h);
	bitmapInfo->bmiHeader.biPlanes = 1;
	bitmapInfo->bmiHeader.biBitCount = (surf.format.isCLUT8() ? 8 : surf.format.bpp());
	bitmapInfo->bmiHeader.biCompression = BI_RGB;
	bitmapInfo->bmiHeader.biSizeImage = 0;
	bitmapInfo->bmiHeader.biClrUsed = (surf.format.isCLUT8() ? colorCount : 0);
	bitmapInfo->bmiHeader.biClrImportant = (surf.format.isCLUT8() ? colorCount : 0);

	if (surf.hasPalette()) {
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

Common::StringArray Win32PrintingManager::listPrinterNames() const {
	DWORD size;
	DWORD numPrinterInfos;

	// Get the required size
	BOOL success = EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 4, NULL, 0, &size, &numPrinterInfos);

	Common::StringArray printerNames;
	BYTE *printerInfos = new BYTE[size];
	success = EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 4, (LPBYTE)printerInfos, size, &size, &numPrinterInfos);
	if (!success)
		return Common::StringArray();

	for (uint i = 0; i < numPrinterInfos; i++) {
		PRINTER_INFO_4 info = *((PRINTER_INFO_4 *)(printerInfos + i * sizeof(PRINTER_INFO_4)));
		TCHAR *name = (TCHAR *)info.pPrinterName;
		printerNames.push_back(Win32::tcharToString(name));
	}

	delete[] printerInfos;

	return printerNames;
}

Common::String Win32PrintingManager::getDefaultPrinterName() const {
	DWORD size;
	GetDefaultPrinter(NULL, &size);

	TCHAR *str = new TCHAR[size];
	BOOL success = GetDefaultPrinter(str, &size);
	if (!success)
		return Common::String();

	Common::String name = Win32::tcharToString(str);

	delete[] str;

	return name;
}

Common::PrintingManager *createWin32PrintingManager() {
	return new Win32PrintingManager();
}

#endif
#endif
