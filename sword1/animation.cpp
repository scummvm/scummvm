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
#include "common/file.h"
#include "sword1/animation.h"
#include "sound/audiostream.h"


namespace Sword1 {

AnimationState::AnimationState(Screen *scr, SoundMixer *snd, OSystem *sys)
	: BaseAnimationState(snd, sys, 640, 400), _scr(scr) {
}

AnimationState::~AnimationState() {
}

bool AnimationState::init(const char *name) {
#ifdef USE_MPEG2
	char tempFile[512];

	decoder = NULL;
	mpgfile = NULL;
	sndfile = NULL;
	bgSoundStream = NULL;

#ifdef BACKEND_8BIT

	uint i, p;

	// Load lookup palettes
	// TODO: Binary format so we can use File class
	sprintf(tempFile, "%s.pal", name);
	FILE *f = fopen(tempFile, "r");

	if (!f) {
		warning("Cutscene: %s.pal palette missing", name);
		return false;
	}

	p = 0;
	while (!feof(f)) {
		int end, cnt;

		if (fscanf(f, "%i %i", &end, &cnt) != 2)
			break;

		palettes[p].end = (uint) end;
		palettes[p].cnt = (uint) cnt;

		for (i = 0; i < palettes[p].cnt; i++) {
			int r, g, b;
			fscanf(f, "%i", &r);
			fscanf(f, "%i", &g);
			fscanf(f, "%i", &b);
			palettes[p].pal[4 * i] = r;
			palettes[p].pal[4 * i + 1] = g;
			palettes[p].pal[4 * i + 2] = b;
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
	fclose(f);

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
	_sys->show_overlay();
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
	ticks = _sys->get_msecs();

	// Play audio
	sndfile = new File();
	bgSoundStream = AudioStream::openStreamFile(name, sndfile);

	if (bgSoundStream != NULL) {
		_snd->playInputStream(&bgSound, bgSoundStream, false, 255, 0, -1, false);
	} else {
		warning("Cutscene: Could not open Audio Track for %s", name);
	}

	return true;
#else /* USE_MPEG2 */
	return false;
#endif
}


#ifdef BACKEND_8BIT

void AnimationState::setPalette(byte *pal) {
	_sys->setPalette(pal, 0, 256);
}

#endif

bool AnimationState::decodeFrame() {
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
					((_snd->getChannelElapsedTime(bgSound) * 12) / 1000 < framenum + 1) ||
					frameskipped > 10) {
					if (frameskipped > 10) {
						warning("force frame %i redraw", framenum);
						frameskipped = 0;
					}
#ifdef BACKEND_8BIT
					_scr->plotYUV(lut, sequence_i->width, sequence_i->height, info->display_fbuf->buf);
#else
					plotYUV(lookup, sequence_i->width, sequence_i->height, info->display_fbuf->buf);
					_sys->copy_rect_overlay(overlay, MOVIE_WIDTH, 0, 40, MOVIE_WIDTH, MOVIE_HEIGHT);
#endif

					if (bgSoundStream) {
						while ((_snd->getChannelElapsedTime(bgSound) * 12) / 1000 < framenum)
							_sys->delay_msecs(10);
					} else {
						ticks += 83;
						while (_sys->get_msecs() < ticks)
							_sys->delay_msecs(10);
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

MoviePlayer::MoviePlayer(Screen *scr, SoundMixer *snd, OSystem *sys)
	: _scr(scr), _snd(snd), _sys(sys) {
}

/**
 * Plays an animated cutscene.
 * @param filename the file name of the cutscene file
 */
void MoviePlayer::play(const char *filename) {
#ifdef USE_MPEG2
	AnimationState *anim = new AnimationState(_scr, _snd, _sys);

	if (anim->init(filename)) {
		while (anim->decodeFrame()) {
#ifndef BACKEND_8BIT
			_sys->updateScreen();
#endif
			// FIXME: check for ESC and abbort animation be just returning from the function
			OSystem::Event event;
			while (_sys->poll_event(&event)) {
				if ((event.event_code == OSystem::EVENT_KEYDOWN) &&
				    (event.kbd.keycode == 27)) {
					delete anim;
					return;
				}
				if (event.event_code == OSystem::EVENT_QUIT)
					_sys->quit();
			}
		}
	}

	delete anim;

#endif
}

} // End of namespace Sword2
