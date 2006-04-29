/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
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

#ifndef ANIMATION_H
#define ANIMATION_H

#include "graphics/animation.h"

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

class AnimationState : public Graphics::BaseAnimationState {
private:
	Screen *_scr;

public:
	AnimationState(Screen *scr, Audio::Mixer *snd, OSystem *sys);
	~AnimationState();
	void updateScreen();
	OverlayColor *giveRgbBuffer(void);
	bool soundFinished();

private:
	void drawYUV(int width, int height, byte *const *dat);

#ifdef BACKEND_8BIT
	void setPalette(byte *pal);
#endif

protected:
	virtual Audio::AudioStream *createAudioStream(const char *name, void *arg);
};

class MoviePlayer {
public:
	MoviePlayer(Screen *scr, Audio::Mixer *snd, OSystem *sys);
	~MoviePlayer(void);
	void play(uint32 id);
private:
	void insertOverlay(OverlayColor *buf, uint8 *ovl, OverlayColor *pal);
	void processFrame(uint32 animId, AnimationState *anim, uint32 frameNo);
	bool initOverlays(uint32 id);
	Screen *_scr;
	Audio::Mixer *_snd;
	OSystem *_sys;

	static const char *_sequenceList[20];
	uint8 *_logoOvls[INTRO_LOGO_OVLS];
	OverlayColor *_introPal;
};

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

} // End of namespace Sword1

#endif
