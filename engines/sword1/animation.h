/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 *
 */

#ifndef SWORD1_ANIMATION_H
#define SWORD1_ANIMATION_H

#include "graphics/dxa_player.h"
#include "graphics/mpeg_player.h"

#include "sword1/screen.h"
#include "sword1/sound.h"
#include "sound/audiostream.h"

namespace Sword1 {

enum {
	SEQ_FERRARI = 0,
	SEQ_LADDER,
	SEQ_STEPS,
	SEQ_SEWER,
	SEQ_INTRO,
	SEQ_RIVER,
	SEQ_TRUCK,
	SEQ_GRAVE,
	SEQ_MONTFCON,
	SEQ_TAPESTRY,
	SEQ_IRELAND,
	SEQ_FINALE,
	SEQ_HISTORY,
	SEQ_SPANISH,
	SEQ_WELL,
	SEQ_CANDLE,
	SEQ_GEODROP,
	SEQ_VULTURE,
	SEQ_ENDDEMO,
	SEQ_CREDITS
};

#define INTRO_LOGO_OVLS 12
#define INTRO_TEXT_OVLS 8

class MovieText {
public:
	uint16 _startFrame;
	uint16 _endFrame;
	char *_text;
	MovieText(int startFrame, int endFrame, char *text) {
		_startFrame = startFrame;
		_endFrame = endFrame;
		_text = strdup(text);
	}
	~MovieText() {
		free(_text);
	}
};

class MoviePlayer {
public:
	MoviePlayer(SwordEngine *vm, Screen *screen, Text *textMan, Audio::Mixer *snd, OSystem *system);
	virtual ~MoviePlayer(void);
	virtual bool load(uint32 id);
	void play(void);
	void updatePalette(byte *pal, bool packed = true);
private:
	bool checkSkipFrame(void);
protected:
	SwordEngine *_vm;
	Screen *_screen;
	Text *_textMan;
	Audio::Mixer *_snd;
	OSystem *_system;
	Common::Array<MovieText *> _movieTexts;
	byte *_textSpriteBuf;
	int _textX, _textY, _textWidth, _textHeight;
	byte _black, _white;

	uint32 _id;

	uint _currentFrame;
	int _framesSkipped;
	bool _forceFrame;

	int _frameWidth, _frameHeight;
	int _frameX, _frameY;

	Audio::SoundHandle _bgSoundHandle;
	Audio::AudioStream *_bgSoundStream;
	uint32 _ticks;

	virtual void handleScreenChanged(void);
	virtual bool initOverlays(uint32 id);
	virtual bool decodeFrame(void) = 0;
	virtual void processFrame(void) = 0;
	virtual bool syncFrame(void);
	virtual void updateScreen(void) = 0;
};

#ifdef USE_ZLIB

class MoviePlayerDXA : public MoviePlayer, ::Graphics::DXAPlayer {
protected:
	virtual void setPalette(byte *pal);
public:
	MoviePlayerDXA(SwordEngine *vm, Screen *screen, Text *textMan, Audio::Mixer *snd, OSystem *system);
	virtual ~MoviePlayerDXA(void);
	bool load(uint32 id);
protected:
	bool initOverlays(uint32 id);
	bool decodeFrame(void);
	void processFrame(void);
	void updateScreen(void);
};

#endif

#ifdef USE_MPEG2

class AnimationState : public Graphics::BaseAnimationState {
private:
	MoviePlayer *_player;
	Screen *_screen;

public:
	AnimationState(MoviePlayer *player, Screen *screen, OSystem *system);
	~AnimationState(void);
	OverlayColor *giveRgbBuffer(void);

private:
	void drawYUV(int width, int height, byte *const *dat);

#ifdef BACKEND_8BIT
	void setPalette(byte *pal);
#endif

protected:
	virtual Audio::AudioStream *createAudioStream(const char *name, void *arg);
};

class MoviePlayerMPEG : public MoviePlayer {
public:
	MoviePlayerMPEG(SwordEngine *vm, Screen *screen, Text *textMan, Audio::Mixer *snd, OSystem *system);
	virtual ~MoviePlayerMPEG(void);
	bool load(uint32 id);
protected:
	void insertOverlay(OverlayColor *buf, uint8 *ovl, OverlayColor *pal);
	AnimationState *_anim;
	OverlayColor *_introPal;
	uint8 *_logoOvls[INTRO_LOGO_OVLS];

	bool initOverlays(uint32 id);
	bool decodeFrame(void);
	void processFrame(void);
	void updateScreen(void);
	void handleScreenChanged(void);
};

#endif

struct FileQueue {
	Audio::AudioStream *stream;
	FileQueue *next;
};

class SplittedAudioStream : public Audio::AudioStream {
public:
	SplittedAudioStream(void);
	~SplittedAudioStream(void);
	void appendStream(Audio::AudioStream *stream);
	virtual int readBuffer(int16 *buffer, const int numSamples);
	virtual bool isStereo(void) const;
	virtual bool endOfData(void) const;
	virtual int getRate(void) const;
private:
	FileQueue *_queue;
};

MoviePlayer *makeMoviePlayer(uint32 id, SwordEngine *vm, Screen *screen, Text *textMan, Audio::Mixer *snd, OSystem *system);

} // End of namespace Sword1

#endif
