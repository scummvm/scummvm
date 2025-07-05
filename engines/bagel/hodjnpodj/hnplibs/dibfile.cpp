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

#include "common/memstream.h"
#include "image/bmp.h"
#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/dibapi.h"

namespace Bagel {
namespace HodjNPodj {

/*
 * Dib Header Marker - used in writing DIBs to files
 */
#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')

BOOL SaveDIB(HDIB hDib, CFile &file) {
	#ifdef TODO
	BITMAPFILEHEADER bmfHdr; // Header for Bitmap file
	LPBITMAPINFOHEADER lpBI;   // Pointer to DIB info structure
	DWORD dwDIBSize;

	if (hDib == nullptr)
		return FALSE;

	/*
	 * Get a pointer to the DIB memory, the first of which contains
	 * a BITMAPINFO structure
	 */
	lpBI = (LPBITMAPINFOHEADER) GlobalLock((HGLOBAL)hDib);
	if (lpBI == nullptr)
		return FALSE;

	if (!IS_WIN30_DIB(lpBI)) {
		GlobalUnlock((HGLOBAL)hDib);
		return FALSE;       // It's an other-style DIB (save not supported)
	}

	/*
	 * Fill in the fields of the file header
	 */

	/* Fill in file type (first 2 bytes must be "BM" for a bitmap) */
	bmfHdr.bfType = DIB_HEADER_MARKER;  // "BM"

	// Calculating the size of the DIB is a bit tricky (if we want to
	// do it right).  The easiest way to do this is to call GlobalSize()
	// on our global handle, but since the size of our global memory may have
	// been padded a few bytes, we may end up writing out a few too
	// many bytes to the file (which may cause problems with some apps).
	//
	// So, instead let's calculate the size manually (if we can)
	//
	// First, find size of header plus size of color table.  Since the
	// first DWORD in both BITMAPINFOHEADER and BITMAPCOREHEADER conains
	// the size of the structure, let's use this.

	dwDIBSize = *(LPDWORD)lpBI + ::PaletteSize((LPSTR)lpBI);  // Partial Calculation

	// Now calculate the size of the image

	if ((lpBI->biCompression == BI_RLE8) || (lpBI->biCompression == BI_RLE4)) {
		// It's an RLE bitmap, we can't calculate size, so trust the
		// biSizeImage field

		dwDIBSize += lpBI->biSizeImage;
	} else {
		DWORD dwBmBitsSize;  // Size of Bitmap Bits only

		// It's not RLE, so size is Width (DWORD aligned) * Height

		dwBmBitsSize = WIDTHBYTES((lpBI->biWidth) * ((DWORD)lpBI->biBitCount)) * lpBI->biHeight;

		dwDIBSize += dwBmBitsSize;

		// Now, since we have calculated the correct size, why don't we
		// fill in the biSizeImage field (this will fix any .BMP files which
		// have this field incorrect).

		lpBI->biSizeImage = dwBmBitsSize;
	}


	// Calculate the file size by adding the DIB size to sizeof(BITMAPFILEHEADER)

	bmfHdr.bfSize = dwDIBSize + sizeof(BITMAPFILEHEADER);
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;

	/*
	 * Now, calculate the offset the actual bitmap bits will be in
	 * the file -- It's the Bitmap file header plus the DIB header,
	 * plus the size of the color table.
	 */
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + lpBI->biSize
	                   + PaletteSize((LPSTR)lpBI);

	TRY {
		// Write the file header
		file.Write((LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER));
		//
		// Write the DIB header and the bits
		//
		file.WriteHuge(lpBI, dwDIBSize);
	}
	CATCH(CFileException, e) {
		GlobalUnlock((HGLOBAL)hDib);
		THROW_LAST();
	}
	END_CATCH

	GlobalUnlock((HGLOBAL)hDib);
	return TRUE;
	#else
	error("TODO: SaveDIB");
	#endif
}

HDIB ReadDIBFile(CFile &file) {
	Image::BitmapDecoder decoder;
	if (!decoder.loadStream(file))
		return nullptr;

	Graphics::ManagedSurface *surf =
		new Graphics::ManagedSurface();
	surf->copyFrom(*decoder.getSurface());

	if (decoder.hasPalette()) {
		// WORKAROUND: dfa/art/mallet.bmp has 257 palette entries.
		// Other bitmaps may likewise.
		const Graphics::Palette &pal = decoder.getPalette();
		surf->setPalette(pal.data(), 0, MIN<int>(pal.size(), 256));
	}

	return surf;
}

HDIB ReadDIBResource(const char *pszName) {
	Image::BitmapDecoder decoder;
	HINSTANCE hInst = nullptr;
	uint dwBytes;
	HRSRC hRsc;
	HGLOBAL hGbl;
	byte *pData;

	hRsc = FindResource(hInst, pszName, RT_BITMAP);
	if (hRsc != nullptr) {
		dwBytes = (size_t)SizeofResource(hInst, hRsc);
		hGbl = LoadResource(hInst, hRsc);
		if ((dwBytes != 0) &&
			(hGbl != nullptr)) {
			pData = (byte *)LockResource(hGbl);
			Common::MemoryReadStream rs(pData, dwBytes);

			bool success = decoder.loadStream(rs);

			UnlockResource(hGbl);
			FreeResource(hGbl);

			if (success) {
				Graphics::ManagedSurface *surf =
					new Graphics::ManagedSurface();
				surf->copyFrom(*decoder.getSurface());

				if (decoder.hasPalette()) {
					const Graphics::Palette &pal = decoder.getPalette();
					surf->setPalette(pal.data(), 0, pal.size());
				}

				return surf;
			}
		}
	}

	return nullptr;
}

} // namespace HodjNPodj
} // namespace Bagel
