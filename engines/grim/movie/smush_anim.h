/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GRIM_MOVIE_SMUSH_ANIM_H
#define GRIM_MOVIE_SMUSH_ANIM_H

#include "common/scummsys.h"
#include "common/str.h"

#include "engines/grim/movie/smush_animator.h"
#include "engines/grim/movie/smush_mixer.h"

namespace Grim {

class SmushTimer;
class SmushSound;

struct SmushDemoTarget {
	byte *pixels;
	int width;
	int height;
	int bytesPerPixel;
};

class SmushDemoVideo {
public:
	virtual ~SmushDemoVideo() {}

	virtual int videoStart(int width, int height, int depth) = 0;
	virtual void videoStop() = 0;
	virtual void videoLock(SmushDemoTarget *target, SmushDemoRect *rect) = 0;
	virtual void videoUnlock() = 0;
	virtual void videoSetPalette(const uint32 *palette, int start, int count) = 0;
};

class SmushDemoPlayer : public SmushDemoClient {

private:
	SmushDemoVideo *_video;
	SmushDemoDriver *_driver;
	SmushDemoStream *_stream;
	SmushDemoAnim *_anim;
	SmushDemoAudio _audio;
	bool _useDriver;

	int _playing;

	byte _staging[4096 + 64];
	uint32 _palette[256];
	byte *_frameSource;
	int _frameWidth;
	int _frameHeight;
	int _positionX;
	int _positionY;
	int _sourceX;
	int _sourceY;
	int _paused;
	int _done;
	int _lastResult;
	int _frameReady;
	int _paletteFlag;
	int _staged;
	int _audioPaused;
	int _audioStartFrame;
	int _audioClockBase;
	int _audioClockLast;
	int _startTime;
	uint32 _lastUpdate;
	int _fps;
	int _frame;
	int _framesShown;
	int _frameCount;
	int _dropCount;
	int _behind;
	int _firstUpdate;
	int _streamStop;

	int audioOpen(int rate, int channels, int bits);
	void audioClose();
	int audioPause();
	int audioResume();
	void audioFill();
	int audioGetTime();
	int getFramesDue();
	int stepAnimation();
	void decodeIACT(const byte *block);
	void smushPlayBlitCopy(const byte *source, int sourcePitch, byte *destination, int destinationPitch, int width, int rows);

public:
	SmushDemoPlayer(SmushDemoVideo *video, SmushDemoDriver *driver);
	~SmushDemoPlayer();

	int preFrame(int frame, int frameCount) override;
	int postFrame(int frame, int frameCount) override;
	int frameCallback(int flags, bool paletteChanged, bool display, int paletteState) override;
	int chunkCallback(const byte *chunk, int size, int flags) override;
	void setFrameRate(int fps) override;
	void setFrameBuffer(byte *buffer, int x, int y, int sourceX, int sourceY, int width, int height) override;

	int initialize(int bufferBytes, int width, int height);
	void terminate();
	int open(const Common::String &name, int offset);
	int close();
	int update(int *frameDecoded, int *finished, int *paletteChanged, const uint32 **palette);
	void pause();
	void resume();
	void getFrameBuffer(const byte **pixels, int *width, int *height);
	void getSize(int *x, int *y);
	bool streamStep();

	int smushPlayStartMovie(const Common::String &name, int flags, int bufferBytes, int width, int height, int offset);
	int smushPlayUpdateMovie(int *finished);
	int smushPlayStopMovie();
	int smushPlayPauseMovie();
	int smushPlayResumeMovie();
	int smushPlayBlit(SmushDemoTarget *target, const SmushDemoRect *rect);
};

class SmushAnimClient {
public:
	virtual ~SmushAnimClient() {}

	virtual int animatorRender(int flags) = 0;
	virtual void resetFrameCounter() = 0;
	virtual int getFrameCounter() = 0;
};

class SmushAnimPlay {

private:
	SmushTimer *_timer;
	SmushSound *_sound;
	int _frameTimer;
	int _syncTimer;
	int _finished;
	int _framesDue;
	int _lateTicks;
	int _nominalDelay;
	int _currentDelay;
	int _skipFlag;
	int _state;
	int _started;
	int _syncPaused;

	static void frameTimer(int timer, void *context);
	static void syncTimer(int timer, void *context);

	int anmRenderBody(SmushAnimClient *client);

public:
	SmushAnimPlay(SmushTimer *timer, SmushSound *sound);
	~SmushAnimPlay();

	int anmStart(int fps);
	void anmStop();
	int anmRender(SmushAnimClient *client);

};

} // End of namespace Grim
#endif
