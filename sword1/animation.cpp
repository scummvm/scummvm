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


#include "common/config-manager.h"
#include "common/str.h"
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

void AnimationState::drawYUV(int width, int height, byte *const *dat) {
#ifdef BACKEND_8BIT
	_scr->plotYUV(lut, width, height, dat);
#else
	plotYUV(lookup, width, height, dat);
	_sys->copyRectToOverlay(overlay, MOVIE_WIDTH, 0, 40, MOVIE_WIDTH, MOVIE_HEIGHT);
#endif
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
	bool initOK = anim->init(filename);

	if (initOK) {
		while (anim->decodeFrame()) {
#ifndef BACKEND_8BIT
			_sys->updateScreen();
#endif
			OSystem::Event event;
			while (_sys->pollEvent(event)) {
				switch (event.event_code) {
#ifndef BACKEND_8BIT
				case OSystem::EVENT_SCREEN_CHANGED:
					anim->invalidateLookup(true);
					break;
#endif
				case OSystem::EVENT_KEYDOWN:
					if (event.kbd.keycode == 27) {
						delete anim;
						return;
					}
					break;
				case OSystem::EVENT_QUIT:
					_sys->quit();
					break;
				default:
					break;
				}
			}
		}
	}

	delete anim;

#endif
}

} // End of namespace Sword1
