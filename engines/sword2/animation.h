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

#include "graphics/dxa_player.h"
#include "graphics/mpeg_player.h"
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
private:
	bool checkSkipFrame();

protected:
	Sword2Engine *_vm;
	Audio::Mixer *_mixer;
	OSystem *_system;

	char *_name;

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
	void calcTextPosition(MovieTextObject *t, int &xPos, int &yPos);

	virtual void handleScreenChanged() {}

	virtual void clearFrame();
	virtual void updateScreen();
	virtual bool decodeFrame() = 0;
	virtual void syncFrame();
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
	bool decodeFrame();
	void syncFrame();
	void drawFrame();
	void drawTextObject(MovieTextObject *t);
	void undrawTextObject(MovieTextObject *t);

public:
	MoviePlayerDummy(Sword2Engine *vm);
	virtual ~MoviePlayerDummy();

	bool load(const char *name, MovieTextObject *text[]);
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

	void clearFrame();

private:
	void drawYUV(int width, int height, byte *const *dat);

#ifdef BACKEND_8BIT
	void setPalette(byte *pal);
#endif
};

class MoviePlayerMPEG : public MoviePlayer {
protected:
	AnimationState *_anim;

	virtual bool decodeFrame();

#ifndef BACKEND_8BIT
	void handleScreenChanged();
	void clearFrame();
	void drawFrame();
	void updateScreen();
	void drawTextObject(MovieTextObject *t);
	void undrawTextObject(MovieTextObject *t);
#endif

public:
	MoviePlayerMPEG(Sword2Engine *vm);
	~MoviePlayerMPEG();

	bool load(const char *name, MovieTextObject *text[]);
};
#endif

#ifdef USE_ZLIB
class MoviePlayerDXA : public MoviePlayer, ::Graphics::DXAPlayer {
protected:
	void setPalette(byte *pal);
	bool decodeFrame();

public:
	MoviePlayerDXA(Sword2Engine *vm);
	~MoviePlayerDXA();

	bool load(const char *name, MovieTextObject *text[]);
};
#endif

MoviePlayer *makeMoviePlayer(Sword2Engine *vm, const char *name);

} // End of namespace Sword2

#endif
