/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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
 * $URL$
 * $Id$
 *
 */

#include "graphics/video/mpeg_player.h"
#include "common/file.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/scaler/intern.h"

namespace Graphics {

BaseAnimationState::BaseAnimationState(OSystem *sys, int width, int height)
	: _movieWidth(width), _movieHeight(height), _frameWidth(width), _frameHeight(height), _sys(sys) {
#ifndef BACKEND_8BIT
	const int screenW = _sys->getOverlayWidth();
	const int screenH = _sys->getOverlayHeight();

	_movieScale = MIN(screenW / _movieWidth, screenH / _movieHeight);

	assert(_movieScale >= 1);
	if (_movieScale > 3)
		_movieScale = 3;

	_colorTab = NULL;
	_rgbToPix = NULL;
	memset(&_overlayFormat, 0, sizeof(_overlayFormat));
#endif
}

BaseAnimationState::~BaseAnimationState() {
#ifdef USE_MPEG2
	if (_mpegDecoder)
		mpeg2_close(_mpegDecoder);
	delete _mpegFile;
#ifndef BACKEND_8BIT
	_sys->hideOverlay();
	free(_overlay);
	free(_colorTab);
	free(_rgbToPix);
#endif
#endif
}


bool BaseAnimationState::init(const char *name) {
#ifdef USE_MPEG2
	char tempFile[512];

	_mpegDecoder = NULL;
	_mpegFile = NULL;

#ifdef BACKEND_8BIT

	uint i, p;

	// Load lookup palettes
	sprintf(tempFile, "%s.pal", name);

	Common::File f;

	if (!f.open(tempFile)) {
		warning("Cutscene: %s palette missing", tempFile);
		return false;
	}

	p = 0;
	while (!f.eos()) {
		_palettes[p].end = f.readUint16LE();
		_palettes[p].cnt = f.readUint16LE();

		for (i = 0; i < _palettes[p].cnt; i++) {
			_palettes[p].pal[4 * i] = f.readByte();
			_palettes[p].pal[4 * i + 1] = f.readByte();
			_palettes[p].pal[4 * i + 2] = f.readByte();
			_palettes[p].pal[4 * i + 3] = 0;
		}
		for (; i < 256; i++) {
			_palettes[p].pal[4 * i] = 0;
			_palettes[p].pal[4 * i + 1] = 0;
			_palettes[p].pal[4 * i + 2] = 0;
			_palettes[p].pal[4 * i + 3] = 0;
		}

		p++;
	}

	f.close();

	_palNum = 0;
	_maxPalNum = p;
	setPalette(_palettes[_palNum].pal);
	_lut = _lut2 = _yuvLookup[0];
	_curPal = -1;
	_cr = 0;
	buildLookup(_palNum, 256);
	_lut2 = _yuvLookup[1];
	_lutCalcNum = (BITDEPTH + _palettes[_palNum].end + 2) / (_palettes[_palNum].end + 2);
#else
	buildLookup();
	_overlay = (OverlayColor *)calloc(_movieScale * _movieWidth * _movieScale * _movieHeight, sizeof(OverlayColor));
	_sys->showOverlay();
#endif

	// Open MPEG2 stream
	_mpegFile = new Common::File();
	sprintf(tempFile, "%s.mp2", name);
	if (!_mpegFile->open(tempFile)) {
		warning("Cutscene: Could not open %s", tempFile);
		return false;
	}

	// Load and configure decoder
	_mpegDecoder = mpeg2_init();
	if (_mpegDecoder == NULL) {
		warning("Cutscene: Could not allocate an MPEG2 decoder");
		return false;
	}

	_mpegInfo = mpeg2_info(_mpegDecoder);
	_frameNum = 0;

	return true;
#else /* USE_MPEG2 */
	return false;
#endif
}

bool BaseAnimationState::decodeFrame() {
#ifdef USE_MPEG2
	mpeg2_state_t state;
	const mpeg2_sequence_t *sequence_i;
	size_t size = (size_t) -1;
	static byte buf[BUFFER_SIZE];

	do {
		state = mpeg2_parse(_mpegDecoder);
		sequence_i = _mpegInfo->sequence;

		switch (state) {
		case STATE_BUFFER:
			size = _mpegFile->read(buf, BUFFER_SIZE);
			mpeg2_buffer(_mpegDecoder, buf, buf + size);
			break;

		case STATE_SLICE:
		case STATE_END:
			if (_mpegInfo->display_fbuf) {
				checkPaletteSwitch();
				drawYUV(sequence_i->width, sequence_i->height, _mpegInfo->display_fbuf->buf);
#ifdef BACKEND_8BIT
				buildLookup(_palNum + 1, _lutCalcNum);
#endif

				_frameNum++;
				return true;
			}
			break;

		default:
			break;
		}
	} while (size);
#endif
	return false;
}

bool BaseAnimationState::checkPaletteSwitch() {
#ifdef BACKEND_8BIT
	// if we have reached the last image with this palette, switch to new one
	if (_frameNum == _palettes[_palNum].end) {
		unsigned char *l = _lut2;
		_palNum++;
		setPalette(_palettes[_palNum].pal);
		_lutCalcNum = (BITDEPTH + _palettes[_palNum].end - (_frameNum + 1) + 2) / (_palettes[_palNum].end - (_frameNum + 1) + 2);
		_lut2 = _lut;
		_lut = l;
		return true;
	}
#endif

	return false;
}

void BaseAnimationState::handleScreenChanged() {
#ifndef BACKEND_8BIT
	const int screenW = _sys->getOverlayWidth();
	const int screenH = _sys->getOverlayHeight();

	int newScale = MIN(screenW / _movieWidth, screenH / _movieHeight);

	assert(newScale >= 1);
	if (newScale > 3)
		newScale = 3;

	if (newScale != _movieScale) {
		// HACK: Since frames generally do not cover the entire screen,
		//       We need to undraw the old frame. This is a very hacky
		//       way of doing that.
		OverlayColor *buf = (OverlayColor *)calloc(screenW * screenH, sizeof(OverlayColor));
		_sys->copyRectToOverlay(buf, screenW, 0, 0, screenW, screenH);
		free(buf);

		free(_overlay);
		_movieScale = newScale;
		_overlay = (OverlayColor *)calloc(_movieScale * _movieWidth * _movieScale * _movieHeight, sizeof(OverlayColor));
	}

	buildLookup();
#endif
}

#ifdef BACKEND_8BIT

/**
 * Build 'Best-Match' RGB lookup table
 */
void BaseAnimationState::buildLookup(int p, int lines) {
	int y, cb;
	int r, g, b, ii;

	if (p >= _maxPalNum)
		return;

	if (p != _curPal) {
		_curPal = p;
		_cr = 0;
		_pos = 0;
	}

	if (_cr > BITDEPTH)
		return;

	for (ii = 0; ii < lines; ii++) {
		r = (-16 * 256 + (int) (256 * 1.596) * ((_cr << SHIFT) - 128)) / 256;
		for (cb = 0; cb <= BITDEPTH; cb++) {
			g = (-16 * 256 - (int) (0.813 * 256) * ((_cr << SHIFT) - 128) - (int) (0.391 * 256) * ((cb << SHIFT) - 128)) / 256;
			b = (-16 * 256 + (int) (2.018 * 256) * ((cb << SHIFT) - 128)) / 256;

			for (y = 0; y <= BITDEPTH; y++) {
				int idx, bst = 0;
				int dis = 2 * SQR(r - _palettes[p].pal[0]) + 4 * SQR(g - _palettes[p].pal[1]) + SQR(b - _palettes[p].pal[2]);

				for (idx = 1; idx < 256; idx++) {
					long d2 = 2 * SQR(r - _palettes[p].pal[4 * idx]) + 4 * SQR(g - _palettes[p].pal[4 * idx + 1]) + SQR(b - _palettes[p].pal[4 * idx + 2]);
					if (d2 < dis) {
						bst = idx;
						dis = d2;
					}
				}
				_lut2[_pos++] = bst;

				r += (1 << SHIFT);
				g += (1 << SHIFT);
				b += (1 << SHIFT);
			}
			r -= (BITDEPTH + 1) * (1 << SHIFT);
		}
		_cr++;
		if (_cr > BITDEPTH)
			return;
	}
}

#else

// The YUV to RGB conversion code is derived from SDL's YUV overlay code, which
// in turn appears to be derived from mpeg_play. The following copyright
// notices have been included in accordance with the original license. Please
// note that the term "software" in this context only applies to the two
// functions buildLookup() and plotYUV() below.

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

void BaseAnimationState::buildLookup() {
	// Do we already have lookup tables for this bit format?
	Graphics::PixelFormat format = _sys->getOverlayFormat();
	if (format == _overlayFormat && _colorTab && _rgbToPix)
		return;

	free(_colorTab);
	free(_rgbToPix);

	_colorTab = (int16 *)malloc(4 * 256 * sizeof(int16));

	int16 *Cr_r_tab = &_colorTab[0 * 256];
	int16 *Cr_g_tab = &_colorTab[1 * 256];
	int16 *Cb_g_tab = &_colorTab[2 * 256];
	int16 *Cb_b_tab = &_colorTab[3 * 256];

	_rgbToPix = (OverlayColor *)malloc(3 * 768 * sizeof(OverlayColor));

	OverlayColor *r_2_pix_alloc = &_rgbToPix[0 * 768];
	OverlayColor *g_2_pix_alloc = &_rgbToPix[1 * 768];
	OverlayColor *b_2_pix_alloc = &_rgbToPix[2 * 768];

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
		r_2_pix_alloc[i + 256] = format.RGBToColor(i, 0, 0);
		g_2_pix_alloc[i + 256] = format.RGBToColor(0, i, 0);
		b_2_pix_alloc[i + 256] = format.RGBToColor(0, 0, i);
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

	_overlayFormat = format;
}

void BaseAnimationState::plotYUV(int width, int height, byte *const *dat) {
	switch (_movieScale) {
	case 1:
		plotYUV1x(width, height, dat);
		break;
	case 2:
		plotYUV2x(width, height, dat);
		break;
	case 3:
		plotYUV3x(width, height, dat);
		break;
	}
}

void BaseAnimationState::plotYUV1x(int width, int height, byte *const *dat) {
	byte *lum = dat[0];
	byte *cr = dat[2];
	byte *cb = dat[1];

	byte *lum2 = lum + width;

	int16 cr_r;
	int16 crb_g;
	int16 cb_b;

	OverlayColor *row1 = _overlay;
	OverlayColor *row2 = row1 + _movieWidth;

	int x;

	for (; height > 0; height -= 2) {
		OverlayColor *r1 = row1;
		OverlayColor *r2 = row2;

		for (x = width; x > 0; x -= 2) {
			register OverlayColor *L;

			cr_r  = _colorTab[*cr + 0 * 256];
			crb_g = _colorTab[*cr + 1 * 256] + _colorTab[*cb + 2 * 256];
			cb_b  = _colorTab[*cb + 3 * 256];
			++cr;
			++cb;

			L = &_rgbToPix[*lum++];
			*r1++ = L[cr_r] | L[crb_g] | L[cb_b];

			L = &_rgbToPix[*lum++];
			*r1++ = L[cr_r] | L[crb_g] | L[cb_b];

			// Now, do second row.

			L = &_rgbToPix[*lum2++];
			*r2++ = L[cr_r] | L[crb_g] | L[cb_b];

			L = &_rgbToPix[*lum2++];
			*r2++ = L[cr_r] | L[crb_g] | L[cb_b];
		}

		lum  += width;
		lum2 += width;
		row1 += 2 * _movieWidth;
		row2 += 2 * _movieWidth;
	}
}

void BaseAnimationState::plotYUV2x(int width, int height, byte *const *dat) {
	byte *lum = dat[0];
	byte *cr = dat[2];
	byte *cb = dat[1];

	byte *lum2 = lum + width;

	int16 cr_r;
	int16 crb_g;
	int16 cb_b;

	OverlayColor *row1 = _overlay;
	OverlayColor *row2 = row1 + 2 * 2 * _movieWidth;

	int x;

	for (; height > 0; height -= 2) {
		OverlayColor *r1 = row1;
		OverlayColor *r2 = row2;

		for (x = width; x > 0; x -= 2) {
			register OverlayColor *L;
			register OverlayColor  C;

			cr_r  = _colorTab[*cr + 0 * 256];
			crb_g = _colorTab[*cr + 1 * 256] + _colorTab[*cb + 2 * 256];
			cb_b  = _colorTab[*cb + 3 * 256];
			++cr;
			++cb;

			L = &_rgbToPix[*lum++];
			C = L[cr_r] | L[crb_g] | L[cb_b];
			*r1++ = C;
			*r1++ = C;

			L = &_rgbToPix[*lum++];
			C = L[cr_r] | L[crb_g] | L[cb_b];
			*r1++ = C;
			*r1++ = C;

			// Now, do second row.

			L = &_rgbToPix[*lum2++];
			C = L[cr_r] | L[crb_g] | L[cb_b];
			*r2++ = C;
			*r2++ = C;

			L = &_rgbToPix[*lum2++];
			C = L[cr_r] | L[crb_g] | L[cb_b];
			*r2++ = C;
			*r2++ = C;
		}

		memcpy(row1 + 2 * _movieWidth, row1, 2 * _movieWidth * sizeof(OverlayColor));
		memcpy(row2 + 2 * _movieWidth, row2, 2 * _movieWidth * sizeof(OverlayColor));

		lum  += width;
		lum2 += width;
		row1 += 4 * 2 * _movieWidth;
		row2 += 4 * 2 * _movieWidth;
	}
}

void BaseAnimationState::plotYUV3x(int width, int height, byte *const *dat) {
	byte *lum = dat[0];
	byte *cr = dat[2];
	byte *cb = dat[1];

	byte *lum2 = lum + width;

	int16 cr_r;
	int16 crb_g;
	int16 cb_b;

	OverlayColor *row1 = _overlay;
	OverlayColor *row2 = row1 + 3 * 3 * _movieWidth;

	int x;

	for (; height > 0; height -= 2) {
		OverlayColor *r1 = row1;
		OverlayColor *r2 = row2;

		for (x = width; x > 0; x -= 2) {
			register OverlayColor *L;
			register OverlayColor  C;

			cr_r  = _colorTab[*cr + 0 * 256];
			crb_g = _colorTab[*cr + 1 * 256] + _colorTab[*cb + 2 * 256];
			cb_b  = _colorTab[*cb + 3 * 256];
			++cr;
			++cb;

			L = &_rgbToPix[*lum++];
			C = L[cr_r] | L[crb_g] | L[cb_b];
			*r1++ = C;
			*r1++ = C;
			*r1++ = C;

			L = &_rgbToPix[*lum++];
			C = L[cr_r] | L[crb_g] | L[cb_b];
			*r1++ = C;
			*r1++ = C;
			*r1++ = C;

			// Now, do second row.

			L = &_rgbToPix[*lum2++];
			C = L[cr_r] | L[crb_g] | L[cb_b];
			*r2++ = C;
			*r2++ = C;
			*r2++ = C;

			L = &_rgbToPix[*lum2++];
			C = L[cr_r] | L[crb_g] | L[cb_b];
			*r2++ = C;
			*r2++ = C;
			*r2++ = C;
		}

		memcpy(row1 + 3 * _movieWidth, row1, 3 * _movieWidth * sizeof(OverlayColor));
		memcpy(row1 + 2 * 3 * _movieWidth, row1, 3 * _movieWidth * sizeof(OverlayColor));
		memcpy(row2 + 3 * _movieWidth, row2, 3 * _movieWidth * sizeof(OverlayColor));
		memcpy(row2 + 2 * 3 * _movieWidth, row2, 3 * _movieWidth * sizeof(OverlayColor));

		lum  += width;
		lum2 += width;
		row1 += 6 * 3 * _movieWidth;
		row2 += 6 * 3 * _movieWidth;
	}
}

#endif

void BaseAnimationState::updateScreen() {
#ifndef BACKEND_8BIT
	int width = _movieScale * _frameWidth;
	int height = _movieScale * _frameHeight;
	int pitch = _movieScale * _movieWidth;

	const int screenW = _sys->getOverlayWidth();
	const int screenH = _sys->getOverlayHeight();

	int x = (screenW - _movieScale * _frameWidth) / 2;
	int y = (screenH - _movieScale * _frameHeight) / 2;

	_sys->copyRectToOverlay(_overlay, pitch, x, y, width, height);
#endif
	_sys->updateScreen();
}

} // End of namespace Graphics
