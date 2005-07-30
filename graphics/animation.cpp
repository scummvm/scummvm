/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "graphics/animation.h"
#include "sound/audiostream.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/system.h"
#include "common/util.h"
#include "common/scaler/intern.h"

namespace Graphics {

BaseAnimationState::BaseAnimationState(Audio::Mixer *snd, OSystem *sys, int width, int height)
	: _movieWidth(width), _movieHeight(height), _snd(snd), _sys(sys) {
#ifndef BACKEND_8BIT
	_colorTab = NULL;
	_rgbToPix = NULL;
	_bitFormat = 0;
#endif
}

BaseAnimationState::~BaseAnimationState() {
#ifdef USE_MPEG2
	_snd->stopHandle(_bgSound);
	if (_mpegDecoder)
		mpeg2_close(_mpegDecoder);
	delete _mpegFile;
#ifndef BACKEND_8BIT
	_sys->hideOverlay();
	free(_overlay);
	free(_colorTab);
	free(_rgbToPix);
#endif
	delete _bgSoundStream;
#endif
}


bool BaseAnimationState::init(const char *name, void *audioArg) {
#ifdef USE_MPEG2
	char tempFile[512];

	_mpegDecoder = NULL;
	_mpegFile = NULL;
	_bgSoundStream = NULL;

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
	while (1) {
		_palettes[p].end = f.readUint16LE();
		_palettes[p].cnt = f.readUint16LE();

		if (f.ioFailed())
			break;

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
	_overlay = (OverlayColor*)calloc(_movieWidth * _movieHeight, sizeof(OverlayColor));
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
	_frameSkipped = 0;
	_ticks = _sys->getMillis();

	// Play audio
	_bgSoundStream = createAudioStream(name, audioArg);

	if (_bgSoundStream != NULL) {
		_snd->playInputStream(Audio::Mixer::kSFXSoundType, &_bgSound, _bgSoundStream, -1, 255, 0, false);
	} else {
		warning("Cutscene: Could not open Audio Track for %s", name);
	}

	return true;
#else /* USE_MPEG2 */
	return false;
#endif
}

AudioStream *BaseAnimationState::createAudioStream(const char *name, void *arg) {
	return AudioStream::openStreamFile(name);
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
				/* simple audio video sync code:
				 * we calculate the actual frame by taking the elapsed audio time and try
				 * to stay inside +- 1 frame of this calculated frame number by dropping
				 * frames if we run behind and delaying if we are too fast
				 */

				if (checkPaletteSwitch() || (_bgSoundStream == NULL) ||
					((_snd->getSoundElapsedTime(_bgSound) * 12) / 1000 < _frameNum + 1) ||
					_frameSkipped > 10) {
					if (_frameSkipped > 10) {
						warning("force frame %i redraw", _frameNum);
						_frameSkipped = 0;
					}
					drawYUV(sequence_i->width, sequence_i->height, _mpegInfo->display_fbuf->buf);

					if (_bgSoundStream && _snd->isSoundHandleActive(_bgSound)) {
						while (_snd->isSoundHandleActive(_bgSound) && (_snd->getSoundElapsedTime(_bgSound) * 12) / 1000 < _frameNum) {
							_sys->delayMillis(10);
						}
						// In case the background sound ends prematurely, update
						// _ticks so that we can still fall back on the no-sound
						// sync case for the subsequent frames.
						_ticks = _sys->getMillis();
					} else {
						_ticks += 83;
						while (_sys->getMillis() < _ticks)
							_sys->delayMillis(10);
					}
				} else {
					warning("dropped frame %i", _frameNum);
					_frameSkipped++;
				}

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
	if (gBitFormat == _bitFormat && _colorTab && _rgbToPix)
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
		Cr_r_tab[i] = (int16) ( (0.419 / 0.299) * CR);
		Cr_g_tab[i] = (int16) (-(0.299 / 0.419) * CR);
		Cb_g_tab[i] = (int16) (-(0.114 / 0.331) * CB);
		Cb_b_tab[i] = (int16) ( (0.587 / 0.331) * CB);
	}

	// Set up entries 0-255 in rgb-to-pixel value tables.
	for (i = 0; i < 256; i++) {
		r_2_pix_alloc[i + 256] = _sys->RGBToColor(i, 0, 0);
		g_2_pix_alloc[i + 256] = _sys->RGBToColor(0, i, 0);
		b_2_pix_alloc[i + 256] = _sys->RGBToColor(0, 0, i);
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

	_bitFormat = gBitFormat;
}

void BaseAnimationState::plotYUV(int width, int height, byte *const *dat) {
	OverlayColor *ptr = _overlay + (_movieHeight - height) / 2 * _movieWidth + (_movieWidth - width) / 2;

	byte *lum = dat[0];
	byte *cr = dat[2];
	byte *cb = dat[1];

	byte *lum2 = lum + width;

	int16 cr_r;
	int16 crb_g;
	int16 cb_b;

	OverlayColor *row1 = ptr;
	OverlayColor *row2 = ptr + _movieWidth;

	int x, y;

	for (y = 0; y < height; y += 2) {
		for (x = 0; x < width; x += 2) {
			register byte L;

			cr_r  = 0 * 768 + 256 + _colorTab[*cr + 0 * 256];
			crb_g = 1 * 768 + 256 + _colorTab[*cr + 1 * 256] + _colorTab[*cb + 2 * 256];
			cb_b  = 2 * 768 + 256 + _colorTab[*cb + 3 * 256];
			++cr;
			++cb;

			L = *lum++;
			*row1++ = (_rgbToPix[L + cr_r] | _rgbToPix[L + crb_g] | _rgbToPix[L + cb_b]);
			L = *lum++;
			*row1++ = (_rgbToPix[L + cr_r] | _rgbToPix[L + crb_g] | _rgbToPix[L + cb_b]);

			// Now, do second row.

			L = *lum2++;
			*row2++ = (_rgbToPix[L + cr_r] | _rgbToPix[L + crb_g] | _rgbToPix[L + cb_b]);
			L = *lum2++;
			*row2++ = (_rgbToPix[L + cr_r] | _rgbToPix[L + crb_g] | _rgbToPix[L + cb_b]);
		}

		// These values are at the start of the next line, (due
		// to the ++'s above), but they need to be at the start
		// of the line after that.

		lum  += width;
		lum2 += width;
		row1 += (2 * _movieWidth - width);
		row2 += (2 * _movieWidth - width);
	}
}

#endif

} // End of namespace Graphics
