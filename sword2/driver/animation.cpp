/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
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
#include "sword2/sword2.h"
#include "sword2/driver/menu.h"
#include "sword2/driver/render.h"

#include "common/file.h"

namespace Sword2 {

// Build 'Best-Match' RGB lookup table
void AnimationState::buildLookup(int p, int lines) {
	int y, cb;
	int r, g, b, ii;

  
	if (p != curpal) {
		curpal = p;
		cr = 0;
		pos = 0;
	}

	if (cr >= BITDEPTH)
		return;

	for (ii = 0; ii < lines; ii++) {
		r = (-16*256 + (int)(256*1.596) * ((cr<<SHIFT)-128)) / 256;
		for (cb = 0; cb < BITDEPTH; cb++) {
			g = (-16*256 - (int)(0.813*256) * ((cr<<SHIFT)-128) - (int)(0.391*256) * ((cb<<SHIFT)-128)) / 256;
			b = (-16*256 + (int)(2.018*256) * ((cb<<SHIFT)-128)) / 256;

			for (y = 0; y < BITDEPTH; y++) {
				int idx, bst = 0;
				int dis = 2*SQR(r-palettes[p].pal[0])+4*SQR(g-palettes[p].pal[1])+SQR(b-palettes[p].pal[2]);

				for (idx = 1; idx < 256; idx++) {
					long d2 = 2*SQR(r-palettes[p].pal[4*idx])+4*SQR(g-palettes[p].pal[4*idx+1])+SQR(b-palettes[p].pal[4*idx+2]);
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
			r -= 256;
		}
		cr++;
		if (cr >= BITDEPTH)
			return;
	}
}

void MoviePlayer::checkPaletteSwitch(AnimationState * st) {
	// if we have reached the last image with this palette, switch to new one
	if (st->framenum == st->palettes[st->palnum].end) {
		unsigned char *l = st->lut2;
		st->palnum++;
		_vm->_graphics->setPalette(0, 256, st->palettes[st->palnum].pal, RDPAL_INSTANT);
		st->lutcalcnum =  (BITDEPTH + st->palettes[st->palnum].end - (st->framenum + 1) + 2) / (st->palettes[st->palnum].end - (st->framenum + 1) + 2);
		st->lut2 = st->lut;
		st->lut = l;
	}
}

#ifndef USE_MPEG2
bool MoviePlayer::decodeFrame(AnimationState * st) {
	// Dummy for MPEG2-less builds
	return false;
}
#else
bool MoviePlayer::decodeFrame(AnimationState * st) {
	mpeg2_state_t state;
	const mpeg2_sequence_t *sequence_i;
	size_t size = (size_t)-1;

	do {
		state = mpeg2_parse (st->decoder);
		sequence_i = st->info->sequence;

		switch (state) {
			case STATE_BUFFER:
				size = st->mpgfile->read(st->buffer, BUFFER_SIZE);
				mpeg2_buffer (st->decoder, st->buffer, st->buffer + size);
			break;

			case STATE_SLICE:
			case STATE_END:
				if (st->info->display_fbuf) {
					checkPaletteSwitch(st);
					_vm->_graphics->plotYUV(st->lut, sequence_i->width, sequence_i->height, st->info->display_fbuf->buf);
					st->framenum++;
					st->buildLookup(st->palnum+1, st->lutcalcnum);
					return true;
				}
			break;

			default:
				break;
		}
	} while (size);

	return false;
}
#endif

#ifndef USE_MPEG2
AnimationState *MoviePlayer::initAnimation(const char *name) {
	return 0;
}
#else
AnimationState *MoviePlayer::initAnimation(const char *name) {
	char basename[512], tempFile[512];
	AnimationState *st = new AnimationState;
  	int i, p;

	strcpy(basename, name);
	basename[strlen(basename)-4] = 0;

	// Load lookup palettes
	// TODO: Binary format so we can use File class
	sprintf(tempFile, "%s/%s.pal", _vm->getGameDataPath(), basename);
	FILE *f = fopen(tempFile, "r");

	if (!f) {
		warning("Cutscene: %s.pal palette missing", basename);
		return 0;
	}

	p = 0;
	while (!feof(f)) {
		fscanf(f, "%i %i", &st->palettes[p].end, &st->palettes[p].cnt);
  		for (i = 0; i < st->palettes[p].cnt; i++) {
  			int r, g, b;
  			fscanf(f, "%i", &r);
  			fscanf(f, "%i", &g);
  			fscanf(f, "%i", &b);
			st->palettes[p].pal[4*i] = r;
			st->palettes[p].pal[4*i+1] = g;
			st->palettes[p].pal[4*i+2] = b;
		}
		p++;
	}
	fclose(f);

	st->palnum = 0;
	_vm->_graphics->setPalette(0, 256, st->palettes[st->palnum].pal, RDPAL_INSTANT);
	st->lut = st->lut2 = st->lookup[0];
	st->curpal = -1;
	st->cr = 0;
	st->buildLookup(st->palnum, 256);
	st->lut2 = st->lookup[1];

	// Open MPEG2 stream
	st->mpgfile = new File();
	sprintf(tempFile, "%s.mp2", basename);
	if (!st->mpgfile->open(tempFile)) {
		warning("Cutscene: Could not open %s", tempFile);
		delete st;
		return 0;
	}

	// Load and configure decoder
	st->decoder = mpeg2_init ();
	if (st->decoder == NULL) {
		warning("Cutscene: Could not allocate an MPEG2 decoder");
		delete st;
		return 0;
	}

	st->info = mpeg2_info(st->decoder);
	st->framenum = 0;

	// Load in palette data
	st->lutcalcnum =  (BITDEPTH + st->palettes[st->palnum].end + 2) / (st->palettes[st->palnum].end + 2);


	/* Play audio - TODO: Sync with video?*/
	// Another TODO: There is no reason that this only allows OGG, and not MP3, or any other format
	// the mixer might support one day... is there?
	File *sndFile = new File;
	sprintf(tempFile, "%s.ogg", basename);
	if (sndFile->open(tempFile))
		_vm->_mixer->playVorbis(&st->bgSound, sndFile, 100000000);

	// FIXME: This leaks (sndFile will never be deleted)

	return st;
}
#endif

#ifndef USE_MPEG2
void MoviePlayer::doneAnimation(AnimationState *st) {
}
#else
void MoviePlayer::doneAnimation(AnimationState *st) {
	_vm->_mixer->stopHandle(st->bgSound);

	mpeg2_close (st->decoder);
	st->mpgfile->close();
	delete st->mpgfile;
	delete st;
}
#endif
} // End of namespace Sword2
