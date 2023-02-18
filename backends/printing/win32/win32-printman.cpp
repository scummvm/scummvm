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

	virtual void printImage(Common::String jobName, byte *pixels, byte *palette, uint32 width, uint32 height);

private:
	HANDLE openDefaultPrinter();
	//HDC createPrinterContext(HANDLE printer);
};


Win32PrintingManager::~Win32PrintingManager() {}

void Win32PrintingManager::printImage(Common::String jobName, byte *pixels, byte *palette, uint32 width, uint32 height) {
	BOOL success;

	// open printer
	HANDLE printer = openDefaultPrinter();

	// create job
	DOC_INFO_1A docInfo;
	docInfo.pOutputFile = NULL;
	docInfo.pDocName = const_cast<LPSTR>(jobName.c_str());
	docInfo.pDatatype = "BITMAP";
	DWORD job = StartDocPrinterA(printer, 1, (LPBYTE) & docInfo);
	if (!job) {
		ClosePrinter(printer);
		return;
	}

	// do the page
	success = StartPagePrinter(printer);

	success = EndPagePrinter(printer);
	
	// close job
	success = EndDocPrinter(printer);
	
	// close printer
	success = ClosePrinter(printer);
}

HANDLE Win32PrintingManager::openDefaultPrinter() {
	wchar_t szPrinter[MAX_PATH];
	HANDLE handle;
	DWORD cchPrinter(ARRAYSIZE(szPrinter));
	GetDefaultPrinterW(szPrinter, &cchPrinter);
	OpenPrinterW(szPrinter, &handle, NULL);

	return handle;
}

/*
HDC Win32PrintingManager::createPrinterContext(HANDLE printer) {
	int size = DocumentPropertiesW(NULL, printer, szPrinter, NULL, NULL, 0);
	DEVMODE *devmode = (DEVMODE *)malloc(size);
	DocumentPropertiesW(NULL, printer, szPrinter, devmode, NULL, DM_OUT_BUFFER);
	HDC printerDC = CreateDCW(L"WINSPOOL", szPrinter, NULL, devmode);
	return printerDC;
}*/

PrintingManager *createWin32PrintingManager() {
	return new Win32PrintingManager();
}

#endif // WIN32
