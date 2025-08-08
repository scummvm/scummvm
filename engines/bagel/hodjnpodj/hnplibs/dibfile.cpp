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
