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
