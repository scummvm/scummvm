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

class Win32PrintJob;

class Win32PrintingManager : public PrintingManager {
public:
	virtual ~Win32PrintingManager();
	
	PrintJob *createJob(Common::String jobName);
};

class Win32PrintJob : public PrintJob {
public:
	friend class Win32PrintingManager;

	Win32PrintJob(Common::String jobName);
	~Win32PrintJob();

	void drawBitmap(const Graphics::ManagedSurface &surf, int x, int y);
	void drawText(const Common::String &text, int x, int y);

	void setTextColor(int r, int g, int b);

	void pageFinished();
	void endDoc();
	void abortJob();

	private:
	HDC createDefaultPrinterContext();
	HDC createPrinterContext(LPTSTR devName);
	HBITMAP buildBitmap(HDC hdc, const Graphics::ManagedSurface &surf);

	HDC hdcPrint;
	bool jobActive;
};


Win32PrintingManager::~Win32PrintingManager() {}

PrintJob *Win32PrintingManager::createJob(Common::String jobName) {
	return new Win32PrintJob(jobName);
}


Win32PrintJob::Win32PrintJob(Common::String jobName) : jobActive(true) {
	hdcPrint = createDefaultPrinterContext();

	Escape(hdcPrint, STARTDOC, jobName.size(), jobName.c_str(), NULL);
}

Win32PrintJob::~Win32PrintJob() {
	if (jobActive) {
		abortJob();
		warning("Printjob still active during destruction!");
	}
	DeleteDC(hdcPrint);
}

void Win32PrintJob::drawBitmap(const Graphics::ManagedSurface &surf, int x, int y) {
	HDC hdcImg = CreateCompatibleDC(hdcPrint);

	HBITMAP bitmap = buildBitmap(hdcPrint, surf);
	if (!bitmap)
		goto delDC;

	SelectObject(hdcImg, bitmap);

	BitBlt(hdcPrint, x, y, surf.w, surf.h, hdcImg, 0, 0, SRCCOPY);
	
	DeleteObject(bitmap);
delDC:
	DeleteDC(hdcImg);
}

void Win32PrintJob::drawText(const Common::String &text, int x, int y) {
	TextOutA(hdcPrint, x, y, const_cast<char*>(text.c_str()), text.size());
}

void Win32PrintJob::setTextColor(int r, int g, int b) {
	SetTextColor(hdcPrint, RGB(r, g, b));
}

void Win32PrintJob::pageFinished() {
	Escape(hdcPrint, NEWFRAME, 0, NULL, NULL);
}

void Win32PrintJob::endDoc() {
	Escape(hdcPrint, ENDDOC, 0, NULL, NULL);
	jobActive = false;
}

void Win32PrintJob::abortJob() {
	Escape(hdcPrint, ABORTDOC, 0, NULL, NULL);
	jobActive = false;
}

HDC Win32PrintJob::createDefaultPrinterContext() {
	TCHAR szPrinter[MAX_PATH];
	BOOL success;
	DWORD cchPrinter(ARRAYSIZE(szPrinter));

	success=GetDefaultPrinter(szPrinter, &cchPrinter);
	if (!success)
		return NULL;

	return createPrinterContext(szPrinter);
}
HDC Win32PrintJob::createPrinterContext(LPTSTR devName) {
	HANDLE handle;
	BOOL success;

	success=OpenPrinter(devName, &handle, NULL);
	if (!success)
		return NULL;

	int size = DocumentProperties(NULL, handle, devName, NULL, NULL, 0);
	DEVMODE *devmode = (DEVMODE *)malloc(size);
	DocumentProperties(NULL, handle, devName, devmode, NULL, DM_OUT_BUFFER);

	ClosePrinter(handle);

	HDC printerDC = CreateDC(TEXT("WINSPOOL"), devName, NULL, devmode);
	return printerDC;
}

HBITMAP Win32PrintJob::buildBitmap(HDC hdc, const Graphics::ManagedSurface &surf) {
	const uint colorCount = 256;
	BITMAPINFO *bitmapInfo = (BITMAPINFO *)malloc(sizeof(BITMAPINFO) + sizeof(RGBQUAD) * (colorCount - 1));

	if (!bitmapInfo)
		return NULL;

	bitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo->bmiHeader.biWidth = surf.w;
	bitmapInfo->bmiHeader.biHeight = -((LONG)surf.h); // Blame the OS2 team for bitmaps being upside down
	bitmapInfo->bmiHeader.biPlanes = 1;
	bitmapInfo->bmiHeader.biBitCount = (surf.format.isCLUT8()?8:surf.format.bpp());
	bitmapInfo->bmiHeader.biCompression = BI_RGB;
	bitmapInfo->bmiHeader.biSizeImage = 0;
	bitmapInfo->bmiHeader.biClrUsed = (surf.format.isCLUT8()?colorCount:0);
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

PrintingManager *createWin32PrintingManager() {
	return new Win32PrintingManager();
}

#endif // WIN32
