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

#ifndef ANIMATION_H
#define ANIMATION_H

#include "graphics/animation.h"

#include "sword1/screen.h"
#include "sword1/sound.h"



namespace Sword1 {

class AnimationState : public Graphics::BaseAnimationState {
private:
	Screen *_scr;

public:
	AnimationState(Screen *scr, SoundMixer *snd, OSystem *sys);
	~AnimationState();

private:
	void drawYUV(int width, int height, byte *const *dat);

#ifdef BACKEND_8BIT
	void setPalette(byte *pal);
#endif
};

class MoviePlayer {
private:
	Screen *_scr;
	SoundMixer *_snd;
	OSystem *_sys;

public:
	MoviePlayer(Screen *scr, SoundMixer *snd, OSystem *sys);
	void play(const char *filename);
};

} // End of namespace Sword2

#endif
