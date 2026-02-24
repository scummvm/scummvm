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

#include "common/printman.h"
#include "backends/platform/sdl/win32/win32_wrapper.h"

#include "win32-printman.h"
#include "common/ustr.h"


class Win32PrintingManager : public Common::PrintingManager {
public:
	virtual ~Win32PrintingManager();

	void doPrint(const Graphics::ManagedSurface &surf, const Common::Rect &destRect) override;

	Common::StringArray listPrinterNames() const override;

	Common::String getDefaultPrinterName() const override;

	Common::Rect getPrintableArea() const override;
	Common::Point getPrintableAreaOffset() const override;
	Common::Rect getPaperDimensions() const override;

private:
	HDC createDefaultPrinterContext() const;
	HDC createPrinterContext(LPTSTR devName) const;
	HDC createPrinterContext() const;
	BITMAPINFO *buildBitmapInfo(const Graphics::ManagedSurface &surf);
};


Win32PrintingManager::~Win32PrintingManager() {}

void Win32PrintingManager::doPrint(const Graphics::ManagedSurface &surf, const Common::Rect &destRect) {
	HDC hdcPrint = createPrinterContext();
	if (!hdcPrint)
		return;

	DOCINFOA info;
	info.cbSize = sizeof(info);
	info.fwType = 0;
	info.lpszDatatype = nullptr;
	info.lpszOutput = nullptr;
	info.lpszDocName = _jobName.c_str();

	BITMAPINFO *bitmapInfo = buildBitmapInfo(surf);
	if (!bitmapInfo) {
		DeleteDC(hdcPrint);
		return;
	}

	// Ensure the pitch is a multiple of 4, as required by StretchDIBits
	int32 pitchCeiledUp = (surf.pitch + 3) / 4 * 4;
	byte *dibPixels = nullptr;
	if (pitchCeiledUp != surf.pitch) {
		dibPixels = new byte[pitchCeiledUp * surf.h]();
		const byte *src = (const byte *)surf.getPixels();
		byte *dst = dibPixels;
		for (int y = 0; y < surf.h; y++) {
			memcpy(dst, src, surf.pitch);
			src += surf.pitch;
			dst += pitchCeiledUp;
		}
	}

	StartDocA(hdcPrint, &info);
	StartPage(hdcPrint);

	StretchDIBits(hdcPrint,
				  destRect.left, destRect.top, destRect.width(), destRect.height(),
				  0, 0, surf.w, surf.h,
				  dibPixels ? dibPixels : surf.getPixels(),
				  bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

	delete[] dibPixels;
	free(bitmapInfo);

	EndPage(hdcPrint);
	EndDoc(hdcPrint);

	DeleteDC(hdcPrint);
}

HDC Win32PrintingManager::createDefaultPrinterContext() const {
	TCHAR szPrinter[MAX_PATH];
	BOOL success;
	DWORD cchPrinter(ARRAYSIZE(szPrinter));

	success = GetDefaultPrinter(szPrinter, &cchPrinter);
	if (!success)
		return NULL;

	return createPrinterContext(szPrinter);
}

HDC Win32PrintingManager::createPrinterContext(LPTSTR devName) const {
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

HDC Win32PrintingManager::createPrinterContext() const {
	if (_printerName.empty())
		return createDefaultPrinterContext();
	else
		return createPrinterContext(Win32::stringToTchar(_printerName));
}

BITMAPINFO *Win32PrintingManager::buildBitmapInfo(const Graphics::ManagedSurface &surf) {
	const uint colorCount = 256;
	BITMAPINFO *bitmapInfo = (BITMAPINFO *)malloc(sizeof(BITMAPINFO) + sizeof(RGBQUAD) * (colorCount - 1));

	if (!bitmapInfo)
		return NULL;

	bitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo->bmiHeader.biWidth = surf.w;
	bitmapInfo->bmiHeader.biHeight = -((LONG)surf.h);
	bitmapInfo->bmiHeader.biPlanes = 1;
	bitmapInfo->bmiHeader.biBitCount = (surf.format.isCLUT8() ? 8 : surf.format.bpp());
	bitmapInfo->bmiHeader.biCompression = (surf.format.isCLUT8() ? BI_RGB : BI_BITFIELDS);
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
	} else {
		DWORD *masks = (DWORD *)bitmapInfo->bmiColors;
		masks[0] = (DWORD)surf.format.rMax() << surf.format.rShift;
		masks[1] = (DWORD)surf.format.gMax() << surf.format.gShift;
		masks[2] = (DWORD)surf.format.bMax() << surf.format.bShift;
	}

	return bitmapInfo;
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

Common::Rect Win32PrintingManager::getPrintableArea() const {
	HDC hdcPrint = createPrinterContext();
	if (hdcPrint) {
		int16 width = GetDeviceCaps(hdcPrint, HORZRES);
		int16 height = GetDeviceCaps(hdcPrint, VERTRES);
		DeleteDC(hdcPrint);
		return Common::Rect(width, height);
	}

	return Common::Rect();
}

Common::Point Win32PrintingManager::getPrintableAreaOffset() const {
	HDC hdcPrint = createPrinterContext();
	if (hdcPrint) {
		int16 x = GetDeviceCaps(hdcPrint, PHYSICALOFFSETX);
		int16 y = GetDeviceCaps(hdcPrint, PHYSICALOFFSETY);
		DeleteDC(hdcPrint);
		return Common::Point(x, y);
	}

	return Common::Point();
}

Common::Rect Win32PrintingManager::getPaperDimensions() const {
	HDC hdcPrint = createPrinterContext();
	if (hdcPrint) {
		int16 width = GetDeviceCaps(hdcPrint, PHYSICALWIDTH);
		int16 height = GetDeviceCaps(hdcPrint, PHYSICALHEIGHT);
		DeleteDC(hdcPrint);
		return Common::Rect(width, height);
	}

	return Common::Rect();
}

Common::PrintingManager *createWin32PrintingManager() {
	return new Win32PrintingManager();
}

#endif
