/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2006 The ScummVM project
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
 */

#ifndef SWORD2_ANIMATION_H
#define SWORD2_ANIMATION_H

#include "graphics/animation.h"
#include "graphics/dxa_player.h"
#include "sound/mixer.h"

namespace Sword2 {

struct SpriteInfo;

// This is the structure which is passed to the sequence player. It includes
// the smack to play, and any text lines which are to be displayed over the top
// of the sequence.

struct MovieTextObject {
	uint16 startFrame;
	uint16 endFrame;
	SpriteInfo *textSprite;
	uint32 speechBufferSize;
	uint16 *speech;
};

struct MovieInfo {
	const char *name;
	uint frames;
	bool seamless;
};

class MoviePlayer {
protected:
	Sword2Engine *_vm;
	Audio::Mixer *_mixer;
	OSystem *_system;

	byte _originalPalette[4 * 256];

	byte *_textSurface;

	Audio::SoundHandle _speechHandle;
	Audio::SoundHandle _bgSoundHandle;
	Audio::AudioStream *_bgSoundStream;

	uint32 _ticks;

	uint _currentFrame;
	byte *_frameBuffer;
	int _frameWidth, _frameHeight;
	int _frameX, _frameY;

	byte _black, _white;

	uint _numFrames;
	uint _leadOutFrame;
	bool _seamless;

	int _framesSkipped;
	bool _forceFrame;

	static const MovieInfo _movies[];

	MovieTextObject **_textList;
	int _currentText;

	void savePalette();
	void restorePalette();

	void openTextObject(MovieTextObject *t);
	void closeTextObject(MovieTextObject *t);

	virtual void handleScreenChanged() {}

	virtual void clearScreen();
	virtual void updateScreen();
	virtual bool decodeFrame() = 0;
	virtual bool checkSkipFrame();
	virtual void waitForFrame();
	virtual void drawFrame();
	virtual void drawTextObject(MovieTextObject *t);
	virtual void undrawTextObject(MovieTextObject *t);

public:
	MoviePlayer(Sword2Engine *vm);
	virtual ~MoviePlayer();

	void updatePalette(byte *pal, bool packed = true);
	virtual bool load(const char *name, MovieTextObject *text[]);
	void play(int32 leadIn, int32 leadOut);
};

class MoviePlayerDummy : public MoviePlayer {
protected:
	virtual bool decodeFrame();
	virtual bool checkSkipFrame();
	virtual void waitForFrame();
	virtual void drawFrame();
	virtual void drawTextObject(MovieTextObject *t);
	virtual void undrawTextObject(MovieTextObject *t);

public:
	MoviePlayerDummy(Sword2Engine *vm);
	virtual ~MoviePlayerDummy();

	virtual bool load(const char *name, MovieTextObject *text[]);
};

#ifdef USE_MPEG2
class AnimationState : public ::Graphics::BaseAnimationState {
private:
	Sword2Engine *_vm;
	MoviePlayer *_player;

public:
	AnimationState(Sword2Engine *vm, MoviePlayer *player);
	~AnimationState();

#ifndef BACKEND_8BIT
	void drawTextObject(SpriteInfo *s, byte *src);
#endif

	void clearScreen();

private:
	void drawYUV(int width, int height, byte *const *dat);

#ifdef BACKEND_8BIT
	void setPalette(byte *pal);
#endif
};

class MoviePlayerMPEG : public MoviePlayer {
protected:
	AnimationState *_anim;

	virtual bool checkSkipFrame();
	virtual void waitForFrame();
	virtual bool decodeFrame();

#ifndef BACKEND_8BIT
	virtual void handleScreenChanged();
	virtual void clearScreen();
	virtual void drawFrame();
	virtual void updateScreen();
	virtual void drawTextObject(MovieTextObject *t);
#endif

public:
	MoviePlayerMPEG(Sword2Engine *vm);
	virtual ~MoviePlayerMPEG();

	virtual bool load(const char *name, MovieTextObject *text[]);
};
#endif

#ifdef USE_ZLIB
class MoviePlayerDXA : public MoviePlayer, ::Graphics::DXAPlayer {
protected:
	virtual void setPalette(byte *pal);
	virtual bool decodeFrame();

public:
	MoviePlayerDXA(Sword2Engine *vm);
	virtual ~MoviePlayerDXA();

	virtual bool load(const char *name, MovieTextObject *text[]);
};
#endif

MoviePlayer *makeMoviePlayer(Sword2Engine *vm, const char *name);

} // End of namespace Sword2

#endif
