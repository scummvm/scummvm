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

BaseAnimationState::BaseAnimationState(SoundMixer *snd, OSystem *sys, int width, int height) 
	: MOVIE_WIDTH(width), MOVIE_HEIGHT(height), _snd(snd), _sys(sys) {
#ifndef BACKEND_8BIT
	colortab = NULL;
	rgb_2_pix = NULL;
	bitFormat = 0;
#endif
}

BaseAnimationState::~BaseAnimationState() {
#ifdef USE_MPEG2
	_snd->stopHandle(bgSound);
	if (decoder)
		mpeg2_close(decoder);
	delete mpgfile;
#ifndef BACKEND_8BIT
	_sys->hideOverlay();
	free(overlay);
	free(colortab);
	free(rgb_2_pix);
#endif
	delete bgSoundStream;
#endif
}


bool BaseAnimationState::init(const char *name, void *audioArg) {
#ifdef USE_MPEG2
	char tempFile[512];

	decoder = NULL;
	mpgfile = NULL;
	bgSoundStream = NULL;

#ifdef BACKEND_8BIT

	uint i, p;

	// Load lookup palettes
	sprintf(tempFile, "%s.pal", name);

	File f;

	if (!f.open(tempFile)) {
		warning("Cutscene: %s palette missing", tempFile);
		return false;
	}

	p = 0;
	while (1) {
		palettes[p].end = f.readUint16LE();
		palettes[p].cnt = f.readUint16LE();

		if (f.ioFailed())
			break;

		for (i = 0; i < palettes[p].cnt; i++) {
			palettes[p].pal[4 * i] = f.readByte();
			palettes[p].pal[4 * i + 1] = f.readByte();
			palettes[p].pal[4 * i + 2] = f.readByte();
			palettes[p].pal[4 * i + 3] = 0;
		}
		for (; i < 256; i++) {
			palettes[p].pal[4 * i] = 0;
			palettes[p].pal[4 * i + 1] = 0;
			palettes[p].pal[4 * i + 2] = 0;
			palettes[p].pal[4 * i + 3] = 0;
		}

		p++;
	}

	f.close();

	palnum = 0;
	maxPalnum = p;
	setPalette(palettes[palnum].pal);
	lut = lut2 = lookup[0];
	curpal = -1;
	cr = 0;
	buildLookup(palnum, 256);
	lut2 = lookup[1];
	lutcalcnum = (BITDEPTH + palettes[palnum].end + 2) / (palettes[palnum].end + 2);
#else
	buildLookup();
	overlay = (OverlayColor*)calloc(MOVIE_WIDTH * MOVIE_HEIGHT, sizeof(OverlayColor));
	_sys->showOverlay();
#endif

	// Open MPEG2 stream
	mpgfile = new File();
	sprintf(tempFile, "%s.mp2", name);
	if (!mpgfile->open(tempFile)) {
		warning("Cutscene: Could not open %s", tempFile);
		return false;
	}

	// Load and configure decoder
	decoder = mpeg2_init();
	if (decoder == NULL) {
		warning("Cutscene: Could not allocate an MPEG2 decoder");
		return false;
	}

	info = mpeg2_info(decoder);
	framenum = 0;
	frameskipped = 0;
	ticks = _sys->getMillis();

	// Play audio
	bgSoundStream = createAudioStream(name, audioArg);

	if (bgSoundStream != NULL) {
		_snd->playInputStream(SoundMixer::kSFXAudioDataType, &bgSound, bgSoundStream, -1, 255, 0, false);
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

	do {
		state = mpeg2_parse(decoder);
		sequence_i = info->sequence;

		switch (state) {
		case STATE_BUFFER:
			size = mpgfile->read(buffer, BUFFER_SIZE);
			mpeg2_buffer(decoder, buffer, buffer + size);
			break;

		case STATE_SLICE:
		case STATE_END:
			if (info->display_fbuf) {
				/* simple audio video sync code:
				 * we calculate the actual frame by taking the elapsed audio time and try
				 * to stay inside +- 1 frame of this calculated frame number by dropping
				 * frames if we run behind and delaying if we are too fast
				 */

				/* Avoid deadlock is sound was too far ahead */
				if (bgSoundStream && !bgSound.isActive())
					return false;

				if (checkPaletteSwitch() || (bgSoundStream == NULL) ||
					((_snd->getSoundElapsedTime(bgSound) * 12) / 1000 < framenum + 1) ||
					frameskipped > 10) {
					if (frameskipped > 10) {
						warning("force frame %i redraw", framenum);
						frameskipped = 0;
					}
					drawYUV(sequence_i->width, sequence_i->height, info->display_fbuf->buf);

					if (bgSoundStream) {
						while ((_snd->getSoundElapsedTime(bgSound) * 12) / 1000 < framenum)
							_sys->delayMillis(10);
					} else {
						ticks += 83;
						while (_sys->getMillis() < ticks)
							_sys->delayMillis(10);
						// FIXME: This used to be used for the Sword2 version of this
						// method. I do not see any compelling reason why it should be
						// used, but maybe I am wrong; so if you know more, either
						// remove this comment, or change the implementation of the 
						// method to use "sleepUntil" for BS2.
						//_vm->sleepUntil(ticks);
					}

				} else {
					warning("dropped frame %i", framenum);
					frameskipped++;
				}

#ifdef BACKEND_8BIT
				buildLookup(palnum + 1, lutcalcnum);
#endif

				framenum++;
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
	if (framenum == palettes[palnum].end) {
		unsigned char *l = lut2;
		palnum++;
		setPalette(palettes[palnum].pal);
		lutcalcnum = (BITDEPTH + palettes[palnum].end - (framenum + 1) + 2) / (palettes[palnum].end - (framenum + 1) + 2);
		lut2 = lut;
		lut = l;
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

	if (p >= maxPalnum)
		return;
  
	if (p != curpal) {
		curpal = p;
		cr = 0;
		pos = 0;
	}

	if (cr > BITDEPTH)
		return;

	for (ii = 0; ii < lines; ii++) {
		r = (-16 * 256 + (int) (256 * 1.596) * ((cr << SHIFT) - 128)) / 256;
		for (cb = 0; cb <= BITDEPTH; cb++) {
			g = (-16 * 256 - (int) (0.813 * 256) * ((cr << SHIFT) - 128) - (int) (0.391 * 256) * ((cb << SHIFT) - 128)) / 256;
			b = (-16 * 256 + (int) (2.018 * 256) * ((cb << SHIFT) - 128)) / 256;

			for (y = 0; y <= BITDEPTH; y++) {
				int idx, bst = 0;
				int dis = 2 * SQR(r - palettes[p].pal[0]) + 4 * SQR(g - palettes[p].pal[1]) + SQR(b - palettes[p].pal[2]);

				for (idx = 1; idx < 256; idx++) {
					long d2 = 2 * SQR(r - palettes[p].pal[4 * idx]) + 4 * SQR(g - palettes[p].pal[4 * idx + 1]) + SQR(b - palettes[p].pal[4 * idx + 2]);
					if (d2 < dis) {
						bst = idx;
						dis = d2;
					}
				}
				lut2[pos++] = bst;
	
				r += (1 << SHIFT);
				g += (1 << SHIFT);
				b += (1 << SHIFT);
			}
			r -= (BITDEPTH+1)*(1 << SHIFT);
		}
		cr++;
		if (cr > BITDEPTH)
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
	if (gBitFormat == bitFormat && colortab && rgb_2_pix)
		return;

	free(colortab);
	free(rgb_2_pix);

	colortab = (int16 *)malloc(4 * 256 * sizeof(int16));

	int16 *Cr_r_tab = &colortab[0 * 256];
	int16 *Cr_g_tab = &colortab[1 * 256];
	int16 *Cb_g_tab = &colortab[2 * 256];
	int16 *Cb_b_tab = &colortab[3 * 256];

	rgb_2_pix = (uint16 *)malloc(3 * 768 * sizeof(uint16));

	uint16 *r_2_pix_alloc = &rgb_2_pix[0 * 768];
	uint16 *g_2_pix_alloc = &rgb_2_pix[1 * 768];
	uint16 *b_2_pix_alloc = &rgb_2_pix[2 * 768];

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

	if (gBitFormat == 565) {
		for (i = 0; i < 256; i++) {
			r_2_pix_alloc[i + 256] = i >> (8 - 5);
			r_2_pix_alloc[i + 256] <<= 11;
			g_2_pix_alloc[i + 256] = i >> (8 - 6);
			g_2_pix_alloc[i + 256] <<= 5;
			b_2_pix_alloc[i + 256] = i >> (8 - 5);
			// b_2_pix_alloc[i + 256] <<= 0;
		}
	} else if (gBitFormat == 555) {
		for (i = 0; i < 256; i++) {
			r_2_pix_alloc[i + 256] = i >> (8 - 5);
			r_2_pix_alloc[i + 256] <<= 10;
			g_2_pix_alloc[i + 256] = i >> (8 - 5);
			g_2_pix_alloc[i + 256] <<= 5;
			b_2_pix_alloc[i + 256] = i >> (8 - 5);
			// b_2_pix_alloc[i + 256] <<= 0;
		}
	} else {
		error("Unknown bit format %d", gBitFormat);
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

	bitFormat = gBitFormat;
}

void BaseAnimationState::plotYUV(int width, int height, byte *const *dat) {
	OverlayColor *ptr = overlay + (MOVIE_HEIGHT - height) / 2 * MOVIE_WIDTH + (MOVIE_WIDTH - width) / 2;

	byte *lum = dat[0];
	byte *cr = dat[2];
	byte *cb = dat[1];

	byte *lum2 = lum + width;

	int16 cr_r;
	int16 crb_g;
	int16 cb_b;

	OverlayColor *row1 = ptr;
	OverlayColor *row2 = ptr + MOVIE_WIDTH;

	int x, y;

	for (y = 0; y < height; y += 2) {
		for (x = 0; x < width; x += 2) {
			register byte L;

			cr_r  = 0 * 768 + 256 + colortab[*cr + 0 * 256];
			crb_g = 1 * 768 + 256 + colortab[*cr + 1 * 256] + colortab[*cb + 2 * 256];
			cb_b  = 2 * 768 + 256 + colortab[*cb + 3 * 256];
			++cr;
			++cb;

			L = *lum++;
			*row1++ = (rgb_2_pix[L + cr_r] | rgb_2_pix[L + crb_g] | rgb_2_pix[L + cb_b]);
			L = *lum++;
			*row1++ = (rgb_2_pix[L + cr_r] | rgb_2_pix[L + crb_g] | rgb_2_pix[L + cb_b]);

			// Now, do second row.

			L = *lum2++;
			*row2++ = (rgb_2_pix[L + cr_r] | rgb_2_pix[L + crb_g] | rgb_2_pix[L + cb_b]);
			L = *lum2++;
			*row2++ = (rgb_2_pix[L + cr_r] | rgb_2_pix[L + crb_g] | rgb_2_pix[L + cb_b]);
		}

		// These values are at the start of the next line, (due
		// to the ++'s above), but they need to be at the start
		// of the line after that.

		lum  += width;
		lum2 += width;
		row1 += (2 * MOVIE_WIDTH - width);
		row2 += (2 * MOVIE_WIDTH - width);
	}
}

#endif

} // End of namespace Graphics
