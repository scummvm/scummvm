/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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

#include "sound/mixer.h"


namespace Sword2 {

class AnimationState : public ::Graphics::BaseAnimationState {
private:
	Sword2Engine *_vm;

public:
	AnimationState(Sword2Engine *vm);
	~AnimationState();

#ifndef BACKEND_8BIT
	void drawTextObject(SpriteInfo *s, byte *src);
#endif

	void clearScreen();
	void updateScreen(void);

private:
	void drawYUV(int width, int height, byte *const *dat);

#ifdef BACKEND_8BIT
	void setPalette(byte *pal);
#endif
};

struct MovieInfo {
	char name[9];
	uint frames;
};
 
class MoviePlayer {
private:
	Sword2Engine *_vm;
	SoundMixer *_snd;
	OSystem *_sys;

	byte *_textSurface;

	static struct MovieInfo _movies[];

	void openTextObject(MovieTextObject *obj);
	void closeTextObject(MovieTextObject *obj);
	void drawTextObject(AnimationState *anim, MovieTextObject *obj);

	int32 playDummy(const char *filename, MovieTextObject *text[], byte *musicOut);

public:
	MoviePlayer(Sword2Engine *vm);
	int32 play(const char *filename, MovieTextObject *text[], byte *musicOut);
};

} // End of namespace Sword2

#endif
