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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BLADERUNNER_VQA_PLAYER_H
#define BLADERUNNER_VQA_PLAYER_H

#include "bladerunner/vqa_decoder.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "graphics/surface.h"

namespace BladeRunner {

enum LoopSetModes {
	kLoopSetModeJustStart = 0,
	kLoopSetModeEnqueue = 1,
	kLoopSetModeImmediate = 2
};

class BladeRunnerEngine;
class View;
class Lights;
class ZBuffer;

class VQAPlayer {
	friend class Debugger;

	BladeRunnerEngine           *_vm;
	Common::String               _name;
	Common::SeekableReadStream  *_s;
	VQADecoder                   _decoder;
	Audio::QueuingAudioStream   *_audioStream;
	Graphics::Surface           *_surface;

	int _frame;
	int _frameNext;
	int _frameBegin;
	int _frameEnd;
	int _loop;
	int _repeatsCount;

	int _repeatsCountQueued;
	int _frameEndQueued;

	int _loopInitial;
	int _repeatsCountInitial;

	uint32 _frameNextTime;
	bool   _hasAudio;
	bool   _audioStarted;
	Audio::SoundHandle _soundHandle;

	void (*_callbackLoopEnded)(void *, int frame, int loopId);
	void  *_callbackData;

public:

	VQAPlayer(BladeRunnerEngine *vm, Graphics::Surface *surface, const Common::String &name)
		: _vm(vm),
		  _name(name),
		  _s(nullptr),
		  _surface(surface),
		  _decoder(),
		  _audioStream(nullptr),
		  _frame(-1),
		  _frameNext(-1),
		  _frameBegin(-1),
		  _frameEnd(-1),
		  _loop(-1),
		  _repeatsCount(-1),
		  _repeatsCountQueued(-1),
		  _frameEndQueued(-1),
		  _loopInitial(-1),
		  _repeatsCountInitial(-1),
		  _frameNextTime(0),
		  _hasAudio(false),
		  _audioStarted(false),
		  _callbackLoopEnded(nullptr),
		  _callbackData(nullptr) { }

	~VQAPlayer() {
		close();
	}

	bool open();
	void close();

	int  update(bool forceDraw = false, bool advanceFrame = true, bool useTime = true, Graphics::Surface *customSurface = nullptr);
	void updateZBuffer(ZBuffer *zbuffer);
	void updateView(View *view);
	void updateScreenEffects(ScreenEffects *screenEffects);
	void updateLights(Lights *lights);

	bool setBeginAndEndFrame(int begin, int end, int repeatsCount, int loopSetMode, void(*callback)(void *, int, int), void *callbackData);
	bool setLoop(int loop, int repeatsCount, int loopSetMode, void(*callback)(void*, int, int), void *callbackData);

	bool seekToFrame(int frame);

	int getLoopBeginFrame(int loop);
	int getLoopEndFrame(int loop);

	int getFrameCount();

private:
	void queueAudioFrame(Audio::AudioStream *audioStream);
};

} // End of namespace BladeRunner

#endif
