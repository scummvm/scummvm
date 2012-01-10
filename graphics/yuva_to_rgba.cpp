/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// The YUV to RGB conversion code is derived from SDL's YUV overlay code, which
// in turn appears to be derived from mpeg_play. The following copyright
// notices have been included in accordance with the original license. Please
// note that the term "software" in this context only applies to the
// buildLookup() and plotYUV*() functions below.

// Copyright (c) 1995 The Regents of the University of California.
// All rights reserved.
//
// Permission to use, copy, modify, and distribute this software and its
// documentation for any purpose, without fee, and without written agreement is
// hereby granted, provided that the above copyright notice and the following
// two paragraphs appear in all copies of this software.
//
// IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
// OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
// CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
// ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
// PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

// Copyright (c) 1995 Erik Corry
// All rights reserved.
//
// Permission to use, copy, modify, and distribute this software and its
// documentation for any purpose, without fee, and without written agreement is
// hereby granted, provided that the above copyright notice and the following
// two paragraphs appear in all copies of this software.
//
// IN NO EVENT SHALL ERIK CORRY BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
// SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF
// THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF ERIK CORRY HAS BEEN ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//
// ERIK CORRY SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
// BASIS, AND ERIK CORRY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
// UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

// Portions of this software Copyright (c) 1995 Brown University.
// All rights reserved.
//
// Permission to use, copy, modify, and distribute this software and its
// documentation for any purpose, without fee, and without written agreement
// is hereby granted, provided that the above copyright notice and the
// following two paragraphs appear in all copies of this software.
//
// IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
// OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF BROWN
// UNIVERSITY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// BROWN UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
// BASIS, AND BROWN UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE,
// SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

#include "common/scummsys.h"
#include "common/singleton.h"

#include "graphics/surface.h"

namespace Graphics {

class YUVAToRGBALookup {
public:
	YUVAToRGBALookup(Graphics::PixelFormat format);
	~YUVAToRGBALookup();

	int16 *_colorTab;
	uint32 *_rgbToPix;
	uint32 *_alphaToPix;
};

YUVAToRGBALookup::YUVAToRGBALookup(Graphics::PixelFormat format) {
	_colorTab = new int16[4 * 256]; // 2048 bytes

	int16 *Cr_r_tab = &_colorTab[0 * 256];
	int16 *Cr_g_tab = &_colorTab[1 * 256];
	int16 *Cb_g_tab = &_colorTab[2 * 256];
	int16 *Cb_b_tab = &_colorTab[3 * 256];

	_rgbToPix = new uint32[3 * 768]; // 9216 bytes

	uint32 *r_2_pix_alloc = &_rgbToPix[0 * 768];
	uint32 *g_2_pix_alloc = &_rgbToPix[1 * 768];
	uint32 *b_2_pix_alloc = &_rgbToPix[2 * 768];

	_alphaToPix = new uint32[256]; // 958 bytes

	int16 CR, CB;
	int i;

	// Generate the tables for the display surface

	for (i = 0; i < 256; i++) {
		// Gamma correction (luminescence table) and chroma correction
		// would be done here. See the Berkeley mpeg_play sources.

		CR = CB = (i - 128);
		Cr_r_tab[i] = (int16) ( (0.419 / 0.299) * CR) + 0 * 768 + 256;
		Cr_g_tab[i] = (int16) (-(0.299 / 0.419) * CR) + 1 * 768 + 256;
		Cb_g_tab[i] = (int16) (-(0.114 / 0.331) * CB);
		Cb_b_tab[i] = (int16) ( (0.587 / 0.331) * CB) + 2 * 768 + 256;
	}

	// Set up entries 0-255 in rgb-to-pixel value tables.
	for (i = 0; i < 256; i++) {
		r_2_pix_alloc[i + 256] = format.ARGBToColor(0, i, 0, 0);
		g_2_pix_alloc[i + 256] = format.ARGBToColor(0, 0, i, 0);
		b_2_pix_alloc[i + 256] = format.ARGBToColor(0, 0, 0, i);
	}

	// Set up entries 0-255 in alpha-to-pixel value table.
	for (i = 0; i < 256; i++) {
		_alphaToPix[i] = format.ARGBToColor(i, 0, 0, 0);
	}

	// Spread out the values we have to the rest of the array so that we do
	// not need to check for overflow.
	for (i = 0; i < 256; i++) {
		r_2_pix_alloc[i] = r_2_pix_alloc[256];
		r_2_pix_alloc[i + 512] = r_2_pix_alloc[511];
		g_2_pix_alloc[i] = g_2_pix_alloc[256];
		g_2_pix_alloc[i + 512] = g_2_pix_alloc[511];
		b_2_pix_alloc[i] = b_2_pix_alloc[256];
		b_2_pix_alloc[i + 512] = b_2_pix_alloc[511];
	}
}

YUVAToRGBALookup::~YUVAToRGBALookup() {
	delete[] _rgbToPix;
	delete[] _colorTab;
	delete[] _alphaToPix;
}

class YUVAToRGBAManager : public Common::Singleton<YUVAToRGBAManager> {
public:
	const YUVAToRGBALookup *getLookup(Graphics::PixelFormat format);

private:
	friend class Common::Singleton<SingletonBaseType>;
	YUVAToRGBAManager();
	~YUVAToRGBAManager();

	Graphics::PixelFormat _lastFormat;
	YUVAToRGBALookup *_lookup;
};

YUVAToRGBAManager::YUVAToRGBAManager() {
	_lookup = 0;
}

YUVAToRGBAManager::~YUVAToRGBAManager() {
	delete _lookup;
}

const YUVAToRGBALookup *YUVAToRGBAManager::getLookup(Graphics::PixelFormat format) {
	if (_lastFormat == format)
		return _lookup;

	delete _lookup;
	_lookup = new YUVAToRGBALookup(format);
	_lastFormat = format;
	return _lookup;
}

} // End of namespace Graphics

namespace Common {
DECLARE_SINGLETON(Graphics::YUVAToRGBAManager);
}

#define YUVAToRGBAMan (Graphics::YUVAToRGBAManager::instance())

namespace Graphics {

#define PUT_PIXELA(s, a, d) \
	L = &rgbToPix[(s)]; \
	*((PixelInt *)(d)) = (L[cr_r] | L[crb_g] | L[cb_b] | aToPix[a])

template<typename PixelInt>
void convertYUVA420ToRGBA(byte *dstPtr, int dstPitch, const YUVAToRGBALookup *lookup, const byte *ySrc, const byte *uSrc, const byte *vSrc, const byte *aSrc, int yWidth, int yHeight, int yPitch, int uvPitch) {
	int halfHeight = yHeight >> 1;
	int halfWidth = yWidth >> 1;

	// Keep the tables in pointers here to avoid a dereference on each pixel
	const int16 *Cr_r_tab = lookup->_colorTab;
	const int16 *Cr_g_tab = Cr_r_tab + 256;
	const int16 *Cb_g_tab = Cr_g_tab + 256;
	const int16 *Cb_b_tab = Cb_g_tab + 256;
	const uint32 *rgbToPix = lookup->_rgbToPix;
	const uint32 *aToPix = lookup->_alphaToPix;

	for (int h = 0; h < halfHeight; h++) {
		for (int w = 0; w < halfWidth; w++) {
			register const uint32 *L;

			int16 cr_r  = Cr_r_tab[*vSrc];
			int16 crb_g = Cr_g_tab[*vSrc] + Cb_g_tab[*uSrc];
			int16 cb_b  = Cb_b_tab[*uSrc];
			++uSrc;
			++vSrc;

			PUT_PIXELA(*ySrc, *aSrc, dstPtr);
			PUT_PIXELA(*(ySrc + yPitch), *(aSrc + yPitch), dstPtr + dstPitch);
			ySrc++;
			aSrc++;
			dstPtr += sizeof(PixelInt);
			PUT_PIXELA(*ySrc, *aSrc, dstPtr);
			PUT_PIXELA(*(ySrc + yPitch), *(aSrc + yPitch), dstPtr + dstPitch);
			ySrc++;
			aSrc++;
			dstPtr += sizeof(PixelInt);

		}

		dstPtr += dstPitch;
		ySrc += (yPitch << 1) - yWidth;
		aSrc += (yPitch << 1) - yWidth;
		uSrc += uvPitch - halfWidth;
		vSrc += uvPitch - halfWidth;
	}
}

void convertYUVA420ToRGBA(Graphics::Surface *dst, const byte *ySrc, const byte *uSrc, const byte *vSrc, const byte *aSrc, int yWidth, int yHeight, int yPitch, int uvPitch) {
	// Sanity checks
	assert(dst && dst->pixels);
	assert(dst->format.bytesPerPixel == 2 || dst->format.bytesPerPixel == 4);
	assert(ySrc && uSrc && vSrc && aSrc);
	assert((yWidth & 1) == 0);
	assert((yHeight & 1) == 0);

	const YUVAToRGBALookup *lookup = YUVAToRGBAMan.getLookup(dst->format);

	// Use a templated function to avoid an if check on every pixel
	if (dst->format.bytesPerPixel == 2)
		convertYUVA420ToRGBA<uint16>((byte *)dst->pixels, dst->pitch, lookup, ySrc, uSrc, vSrc, aSrc, yWidth, yHeight, yPitch, uvPitch);
	else
		convertYUVA420ToRGBA<uint32>((byte *)dst->pixels, dst->pitch, lookup, ySrc, uSrc, vSrc, aSrc, yWidth, yHeight, yPitch, uvPitch);
}

} // End of namespace Graphics
