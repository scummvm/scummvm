/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
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
#include "common/file.h"
#include "sound/audiostream.h"
#include "common/config-manager.h"

namespace Graphics {

BaseAnimationState::BaseAnimationState(SoundMixer *snd, OSystem *sys, int width, int height) 
	: MOVIE_WIDTH(width), MOVIE_HEIGHT(height), _snd(snd), _sys(sys) {
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
		_snd->playInputStream(&bgSound, bgSoundStream, false, -1, 255, 0, false);
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

OverlayColor *BaseAnimationState::lookup = 0;

/**
 * This function should be called when the screen changes to invalidate and,
 * optionally, rebuild the lookup table.
 * @param rebuild If true, rebuild the table.
 */

// FIXME: We will need to call the function from the game's main event loop
// as well, not just the cutscene players' ones.

// FIXME: It would be nice with a heuristic to check if the table really does
// need to be rebuilt.

void BaseAnimationState::invalidateLookup(bool rebuild) {
	free(lookup);
	lookup = 0;
	if (rebuild)
		buildLookup();
}

void BaseAnimationState::buildLookup() {
	if (lookup)
		return;

	lookup = (OverlayColor *)calloc((BITDEPTH+1) * (BITDEPTH+1) * 256, sizeof(OverlayColor));
	if (!lookup) {
		warning("Not enough memory to allocate LUT - cannot play sequence");
		return;
	}

	int y, cb, cr;
	int r, g, b;
	int pos = 0;

	for (cr = 0; cr <= BITDEPTH; cr++) {
		for (cb = 0; cb <= BITDEPTH; cb++) {
			for (y = 0; y < 256; y++) {
				r = ((y - 16) * 256 + (int) (256 * 1.596) * ((cr << SHIFT) - 128)) / 256;
				g = ((y - 16) * 256 - (int) (0.813 * 256) * ((cr << SHIFT) - 128) - (int) (0.391 * 256) * ((cb << SHIFT) - 128)) / 256;
				b = ((y - 16) * 256 + (int) (2.018 * 256) * ((cb << SHIFT) - 128)) / 256;

				if (r < 0) r = 0;
				else if (r > 255) r = 255;
				if (g < 0) g = 0;
				else if (g > 255) g = 255;
				if (b < 0) b = 0;
				else if (b > 255) b = 255;

				lookup[pos++] = _sys->RGBToColor(r, g, b);
			}
		}
	}
}

void BaseAnimationState::plotYUV(OverlayColor *lut, int width, int height, byte *const *dat) {

	if (!lut)
		return;

	OverlayColor *ptr = overlay + (MOVIE_HEIGHT - height) / 2 * MOVIE_WIDTH + (MOVIE_WIDTH - width) / 2;

	int x, y;

	int ypos = 0;
	int cpos = 0;
	int linepos = 0;

	for (y = 0; y < height; y += 2) {
		for (x = 0; x < width; x += 2) {
			int i = ((((dat[2][cpos] + ROUNDADD) >> SHIFT) * (BITDEPTH+1)) + ((dat[1][cpos] + ROUNDADD)>>SHIFT)) * 256;
			cpos++;

			ptr[linepos                ] = lut[i + dat[0][        ypos  ]];
			ptr[MOVIE_WIDTH + linepos++] = lut[i + dat[0][width + ypos++]];
			ptr[linepos                ] = lut[i + dat[0][        ypos  ]];
			ptr[MOVIE_WIDTH + linepos++] = lut[i + dat[0][width + ypos++]];

		}
		linepos += (2 * MOVIE_WIDTH - width);
		ypos += width;
	}
}

#endif

} // End of namespace Graphics
