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

#ifndef BLADERUNNER_VQA_PLAYER_H
#define BLADERUNNER_VQA_PLAYER_H

#include "bladerunner/vqa_decoder.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "graphics/surface.h"

namespace BladeRunner {

enum LoopSetModes {
	kLoopSetModeJustStart = 0, // sets _frameBeginNext, _repeatsCount, _frameEnd
	kLoopSetModeEnqueue   = 1, // sets _frameBeginNext, _repeatsCountQueued, _frameEndQueued
	kLoopSetModeImmediate = 2  // like ModeJustStart, but also sets _frameNext to _frameBeginNext and updates _frameNextTime to current
};

class BladeRunnerEngine;
class View;
class Lights;
class ZBuffer;

class VQAPlayer {
	friend class Debugger;
	friend class OuttakePlayer;

	BladeRunnerEngine           *_vm;
	Common::String               _name;
	Common::SeekableReadStream  *_s;
	VQADecoder                   _decoder;
	Audio::QueuingAudioStream   *_audioStream;
	Graphics::Surface           *_surface;

	static const uint32  kVqaFrameTimeDiff             = 4000; // 60 * 1000 / 15
	static const int     kMaxAudioPreloadedFrames      = 15;
	// Use speech sound type as in original engine
	static const Audio::Mixer::SoundType kVQASoundType = Audio::Mixer::kSpeechSoundType;

	int _frame;
	int _frameNext;
	int _frameBeginNext; // The frame to begin from, after current playing loop ends.
	                     // Does not necessarily reflect current playing loop's start frame
	int _frameEnd;       // The frame to end at for current playing loop
	int _loopIdTarget;   // Does not necessarily reflect current playing loop's id (for a queue of loops this will have the id of the last one in the queue)
	                     // Used: - as param for _callbackLoopEnded() (which typically is loopEnded()), but never actually used in there)
	                     //       - for the MA05 inshot glitch workaround
	                     // It is set at every setLoop call except for the _loopInitial case (when no video stream is loaded)
	int _repeatsCount;   // -1 loop forever
	                     //  0 final repetition (or don't repeat after playing)
	                     //    When that repetition is completeed VQAPlayer::update() returns -3 value
	                     //    See Scene::advanceFrame() and OuttakePlayer::play() for checks for -3 result
	                     // Value is decreased per completed loop of current playing videoloop until it reaches 0

	int _repeatsCountQueued;
	int _frameEndQueued;

	int _lastAudioFrameSuccessfullyQueued;

	int _loopIdInitial;
	int _repeatsCountInitial;

	uint32 _frameNextTime;
	bool   _hasAudio;
	bool   _audioStarted;
	Audio::SoundHandle _soundHandle;

	bool   _specialPS15GlitchFix;
	bool   _specialUG18DoNotRepeatLastLoop;

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
		  _frameBeginNext(-1),
		  _frameEnd(-1),
		  _loopIdTarget(-1),
		  _repeatsCount(-1),
		  _repeatsCountQueued(-1),
		  _frameEndQueued(-1),
		  _lastAudioFrameSuccessfullyQueued(-1),
		  _loopIdInitial(-1),
		  _repeatsCountInitial(-1),
		  _frameNextTime(0),
		  _hasAudio(false),
		  _audioStarted(false),
		  _specialPS15GlitchFix(false),
		  _specialUG18DoNotRepeatLastLoop(false),
		  _callbackLoopEnded(nullptr),
		  _callbackData(nullptr) { }

	~VQAPlayer() {
		close();
	}

	bool open();
	void close();

	bool loadVQPTable(const Common::String& vqpResName);

	int  update(bool forceDraw = false, bool advanceFrame = true, bool useTime = true, Graphics::Surface *customSurface = nullptr);
	void updateZBuffer(ZBuffer *zbuffer);
	void updateView(View *view);
	void updateScreenEffects(ScreenEffects *screenEffects);
	void updateLights(Lights *lights);

	bool setBeginAndEndFrame(int begin, int end, int repeatsCount, int loopSetMode, void(*callback)(void *, int, int), void *callbackData);
	bool setLoop(int loopId, int repeatsCount, int loopSetMode, void(*callback)(void*, int, int), void *callbackData);

	bool seekToFrame(int frame);

	bool getCurrentBeginAndEndFrame(int frame, int *begin, int *end);
	int getLoopBeginFrame(int loopId);
	int getLoopEndFrame(int loopId);

	int getLoopIdTarget() const { return _loopIdTarget; };

	int getFrameCount() const;

	int getQueuedAudioFrames() const;

private:
	void queueAudioFrame(Audio::AudioStream *audioStream);
};

} // End of namespace BladeRunner

#endif
