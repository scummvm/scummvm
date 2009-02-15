/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
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

#include "graphics/video/dxa_player.h"
#include "graphics/video/smk_player.h"
#include "sound/mixer.h"

#include "sword2/screen.h"

namespace Sword2 {

struct SequenceTextInfo {
	uint32 textNumber;
	uint16 startFrame;
	uint16 endFrame;
};

struct MovieTextObject {
	byte *textMem;
	SpriteInfo textSprite;
	uint16 speechId;

	MovieTextObject() {
		textMem = NULL;
		speechId = 0;
	}
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

	uint32 _numSpeechLines;
	uint32 _firstSpeechFrame;
	MovieTextObject _textObject;
	byte *_textSurface;

	Audio::SoundHandle _bgSoundHandle;
	Audio::AudioStream *_bgSoundStream;

	uint32 _ticks;

	uint32 _pauseStartTick;
	uint32 _pauseTicks;

	uint _currentFrame;
	byte *_frameBuffer;
	int _frameWidth, _frameHeight;
	int _frameX, _frameY;

	byte _black, _white;

	uint _numFrames;
	uint _leadOutFrame;
	bool _seamless;

	int _framesSkipped;

	static const MovieInfo _movies[];

	uint32 _currentText;

	uint32 getTick();

	void savePalette();
	void restorePalette();

	void openTextObject(SequenceTextInfo *t);
	void closeTextObject();
	void calcTextPosition(int &xPos, int &yPos);

	virtual void handleScreenChanged() {}

	virtual void clearFrame();
	virtual void updateScreen();
	virtual bool decodeFrame() = 0;
	virtual bool syncFrame();
	virtual void drawFrame();
	virtual void drawTextObject();
	virtual void undrawTextObject();

public:
	MoviePlayer(Sword2Engine *vm, const char *name);
	virtual ~MoviePlayer();

	virtual bool load();
	bool userInterrupt();
	void play(SequenceTextInfo *textList, uint32 numLines, int32 leadIn, int32 leadOut);
	void pauseMovie(bool pause);
};

class MoviePlayerDummy : public MoviePlayer {
protected:
	bool decodeFrame();
	bool syncFrame();
	void drawFrame();
	void drawTextObject();
	void undrawTextObject();

public:
	MoviePlayerDummy(Sword2Engine *vm, const char *name);
	virtual ~MoviePlayerDummy();

	bool load();
};

class MoviePlayerSMK : public MoviePlayer, ::Graphics::SMKPlayer {
protected:
	bool decodeFrame();

public:
	MoviePlayerSMK(Sword2Engine *vm, const char *name);
	~MoviePlayerSMK();

	bool load();
};

#ifdef USE_ZLIB
class MoviePlayerDXA : public MoviePlayer, ::Graphics::DXAPlayer {
protected:
	bool decodeFrame();

public:
	MoviePlayerDXA(Sword2Engine *vm, const char *name);
	~MoviePlayerDXA();

	bool load();
};
#endif

MoviePlayer *makeMoviePlayer(Sword2Engine *vm, const char *name);

} // End of namespace Sword2

#endif
