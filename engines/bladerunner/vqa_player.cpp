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

#include "bladerunner/vqa_player.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/time.h"
#include "bladerunner/audio_player.h"
#if BLADERUNNER_ORIGINAL_SETTINGS
#include "bladerunner/audio_speech.h"
#endif
#include "bladerunner/zbuffer.h"

#include "audio/decoders/raw.h"

#include "common/system.h"

namespace BladeRunner {

bool VQAPlayer::open() {
	close();

	_s = _vm->getResourceStream(_vm->_enhancedEdition ? ("video/" + _name) : _name);
	if (!_s) {
		return false;
	}

	if (!_decoder.loadStream(_s)) {
		delete _s;
		_s = nullptr;
		return false;
	}

#if !BLADERUNNER_ORIGINAL_BUGS
	_specialPS15GlitchFix = false;
	_specialUG18DoNotRepeatLastLoop = false;
	// TB05 has wrong end of a loop and this will load empty zbuffer from next loop, which will lead to broken pathfinding
	if (_name.equals("TB05_2.VQA")) {
		_decoder._loopInfo.loops[1].end = 60;
	} else if (_name.equals("DR04OVER.VQA")) {
		// smoke (overlay) after explosion of Dermo Labs in DR04
		// This has still frames in the end that so it looked as if the smoke was "frozen"
		_decoder._loopInfo.loops[0].end  = 58; // 59 up to 74 are still frames
	} else if (_name.equals("CT01.VQA") || _name.equals("CT01_2.VQA") || _name.equals("CT01_3.VQA") ) {
		// In the last frame of the Mainloop (255) a Howie Lee's customer's hand
		// backwards abruptly the loop looks jarring. We skip the last frame.
		// The issue is also present in the non-spinner versions of the loop
		// and for all chapters where this scene is available (Acts 1 through 5)
		_decoder._loopInfo.loops[2].end  = 254;
		_decoder._loopInfo.loops[3].end  = 254;

		_decoder._loopInfo.loops[7].end  = 510;
		_decoder._loopInfo.loops[8].end  = 510;
	} else if (_name.equals("PS15.VQA") || _name.equals("PS15_2.VQA")) {
		// Fix should be applied in Act 1-3 versions of this background
		_specialPS15GlitchFix = true;
	} else if (_name.equals("UG19OVR1.VQA")) {
		// Original has x: 244, y: 88
		// This still does not look quite right
		// TODO What is this overlay supposed to be for?
		_decoder.overrideOffsetXY(248, 110); 
	} else if (_name.equals("UG18OVR2.VQA")) {
		_specialUG18DoNotRepeatLastLoop = true;
	}
#endif

	_hasAudio = _decoder.hasAudio();
	if (_hasAudio) {
		_audioStream = Audio::makeQueuingAudioStream(_decoder.frequency(), false);
		_lastAudioFrameSuccessfullyQueued = 1;
	}

	_repeatsCount = 0;
	_loopIdTarget = -1;
	_frame = -1;
	_frameBeginNext = -1;
	_frameEnd = getFrameCount() - 1;
	_frameEndQueued = -1;
	_repeatsCountQueued = -1;

	if (_loopIdInitial >= 0) {
		// loopIdInitial is set to the loop Id value that should play,
		// when the SeekableReadStream (_s) is nullptr
		// see setLoop()
		setLoop(_loopIdInitial, _repeatsCountInitial, kLoopSetModeImmediate, nullptr, nullptr);
	} else {
		_frameNext = 0;
		// TODO? Removed as redundant
//		setBeginAndEndFrame(0, _frameEnd, 0, kLoopSetModeJustStart, nullptr, nullptr);
	}

	return true;
}

void VQAPlayer::close() {
	_decoder.close();
	_vm->_mixer->stopHandle(_soundHandle);
	delete _s;
	_s = nullptr;
}

bool VQAPlayer::loadVQPTable(const Common::String &vqpResName) {
	Common::SeekableReadStream *vqpFileSRS = _vm->getResourceStream(vqpResName);
	if (!vqpFileSRS) {
		return false;
	}

	bool vqpFileReadError = false;

	if (vqpFileSRS != nullptr) {
		uint32 numOfPalettes = vqpFileSRS->readUint32LE();
		uint32 palettesReadIn = 0;

		if (vqpFileSRS->eos() || vqpFileSRS->err()) {
			vqpFileReadError = true;
		} else {
			_decoder.allocatePaletteVQPTable(numOfPalettes);
			uint8 colorb = 0;
			uint32 colorsReadIn = 0;
			bool endVqpFileParsing = false;
			for (uint32 i = 0; i < numOfPalettes && !endVqpFileParsing; ++i) {
				colorsReadIn = 0;
				// For each palette read a 2d array for color combinations
				for (uint16 j = 0; j < 256 && !endVqpFileParsing; ++j) {
					for (uint16 k = 0; k <= j && !endVqpFileParsing; ++k) {
						colorb = vqpFileSRS->readByte();
						if (vqpFileSRS->eos() || vqpFileSRS->err()) {
							endVqpFileParsing = true;
							break;
						}
						++colorsReadIn;
						_decoder.updatePaletteVQPTable(i, j, k, colorb);
					}
				}
				// Since VQP is omitting the duplicates the palette entries are
				// the number of combinations of 256 items in tuples of 2 items
				// with the addition of the number of tuples of the same item
				// (ie. (0,0), (1,1)) which are 256 (the whole diagonal of the 2d array).
				// Thus:
				// (256! / (2! * (256-2)!)) + 256 = ((256 * 255) / 2) + 256 = 32896 entries per palette
				if (colorsReadIn == 32896) {
					++palettesReadIn;
				}
			}
			// Quick validation check
			if (palettesReadIn != numOfPalettes) {
				debug("Error: [VQP] Palettes Read-In: %d mismatch with number in header: %d\n", palettesReadIn, numOfPalettes);
				_decoder.deleteVQPTable();
				vqpFileReadError = true;
			}
		}
		delete vqpFileSRS;
		vqpFileSRS = nullptr;

		if (!vqpFileReadError) {
			//debug("Info: [VQP] Palettes Read-In: %d matches number in header", palettesReadIn);
			return true;
		}
	}

	return false;
}

int VQAPlayer::update(bool forceDraw, bool advanceFrame, bool useTime, Graphics::Surface *customSurface) {
	uint32 now = 60 * _vm->_time->currentSystem();
	int result = -1;

	if (_frameNext < 0) {
		_frameNext = _frameBeginNext;
	}

	if ((_repeatsCount > 0 || _repeatsCount == -1) && (_frameNext > _frameEnd)) {
		int loopEndQueued = _frameEndQueued;
		bool specialEnqueue = false;

		if (_frameEndQueued != -1) {
			_frameEnd = _frameEndQueued;
			_frameEndQueued = -1;
#if !BLADERUNNER_ORIGINAL_BUGS
			// Fix glitch in transition from inShot to mainloop
			// in Act 5 at McCoy's apartment (moving from bedroom to balcony).
			// This emulates a fast-forward, which is required
			// in order to have proper z-buffer info,
			// and display the new first frame of the loop (60) without artifacts.
			// The code is similar to Scene::advanceFrame()
			// This will be done once, since this first loop (loopId 1)
			// is only executed once before moving on to loopId 2
			if (_name.equals("MA05_3.VQA") && _loopIdTarget == 1) {
				while (update(false, true, false) != 59) {
					updateZBuffer(_vm->_zbuffer);
				}
				// This works because the loopId 1 executes once before moving to _loop 2
				// See Scene::advanceFrame()
				//     Scene::loopEnded()
				//
				_frameBeginNext = 60;
			} else if (_name.equals("UG18OVR2.VQA")) {
				// This overlay has three loops (0,1,2) that
				// need to be played back to back.
				// However, then engine can only queue up to two loops.
				// So in this case, we force enqueuing the final loop explicitly here
				// loop 0:   0 -  59
				// loop 1:  60 - 119
				// loop 2: 120 - 135
				// 
				if (_loopIdTarget == 1) {
					// we just loaded up the previously enqueued loop 1
					_frameNext = _frameBeginNext;
					// this also has to be enqueued to be (fake) repeated forever,
					// (presumably _repeatsCountQueued is also -1),
					// in order for the code to proceed to the newly queued loop 2 after this one (loop 1) ends
					_repeatsCount =  -1;
					if (_callbackLoopEnded != nullptr) {
						_callbackLoopEnded(_callbackData, 0, _loopIdTarget);
					}
					specialEnqueue = true;
					_loopIdTarget = 2;
					_frameBeginNext = 120;
					_frameEndQueued = 135;
					_repeatsCountQueued = 0;
				}
			}
		} else if (_specialUG18DoNotRepeatLastLoop && _loopIdTarget == 2) {
			// This extra check is needed to stop the last loop (2) of UG18OVR2.VQA from repeating,
			// in case we loaded a saved game while the queued loops (1 or 2) were playing.
			result = -3;
			// _repeatsCount == 0, so return here at the end of the video, to release the resource
			return result;
#endif
		}

		if (!specialEnqueue) {
			_frameNext = _frameBeginNext;

			if (loopEndQueued == -1) {
				if (_repeatsCount > 0) {
					--_repeatsCount;
				}
				//callback for repeat, it is not used in the blade runner
			} else {
				_repeatsCount = _repeatsCountQueued; // ASDF IS THIS STORED IN SAVED GAME?
				_repeatsCountQueued = -1;

				if (_callbackLoopEnded != nullptr) {
					_callbackLoopEnded(_callbackData, 0, _loopIdTarget);
				}
			}
		}
		result = -1;

	} else if (_frameNext > _frameEnd) {
		result = -3;
		// _repeatsCount == 0, so return here at the end of the video, to release the resource
		return result;

	} else if (useTime && (now - (_frameNextTime - kVqaFrameTimeDiff) < kVqaFrameTimeDiff)) {
		// Not yet time to move to next frame.
		// Note, we use unsigned difference to avoid potential time overflow issues
		result = -1;

	} else if (advanceFrame) {
		_frame = _frameNext;
		_decoder.readFrame(_frameNext, kVQAReadVideo);
		_decoder.decodeVideoFrame(customSurface != nullptr ? customSurface : _surface, _frameNext);

		int maxAllowedAudioPreloadedFrames = kMaxAudioPreloadedFrames;
		if (_frameEnd - _frameNext < kMaxAudioPreloadedFrames - 1) {
			maxAllowedAudioPreloadedFrames = _frameEnd - _frameNext + 1;
		}

		if (_hasAudio) {
			if (!_audioStarted) {
				// start with preloading up to (kMaxAudioPreloadedFrames - 1) frames at most, before reaching the _frameEnd frame
				for (int i = 0; i < kMaxAudioPreloadedFrames - 1; ++i) {
					if (_frameNext + i < _frameEnd) {
						_decoder.readFrame(_frameNext + i, kVQAReadAudio);
						queueAudioFrame(_decoder.decodeAudioFrame());
						_lastAudioFrameSuccessfullyQueued = _frameNext + i;
					}
				}
				if (_vm->_mixer->isReady()) {
					// Audio stream starts playing, consuming queued "audio frames"
					// Note: On its own, the audio will not re-synch with video;
					// It plays independently so it can get ahead!
#if BLADERUNNER_ORIGINAL_SETTINGS
					_vm->_mixer->playStream(kVQASoundType, &_soundHandle, _audioStream, -1, (_vm->_audioSpeech->getVolume() * Audio::Mixer::kMaxChannelVolume) / 100);
#else
					// using the default volume argument (Audio::Mixer::kMaxChannelVolume)
					// will result in the the configured volume for speech being used,
					// since playStream() does get the soundtype volume into consideration.
					// See: Channel::updateChannelVolumes() in audio/mixer.cpp
					_vm->_mixer->playStream(kVQASoundType, &_soundHandle, _audioStream);
#endif // BLADERUNNER_ORIGINAL_SETTINGS
				}
				_audioStarted = true;
			}

			// Due to our audio stream being queuable, the queued audio frames will play,
			// even if the game is "paused" eg. by moving the ScummVM window.
			// However, the video will stop playing immediately in that case.
			// That would result in a audio video desynch, with audio being ahead of video.
			// When the video resumes, we need to catch up to the audio "frame" of the queue that was last played,
			// without queuing more audio, and then start queuing audio again.

			// The following still covers the case of adding the final 15th audio frame to the queue
			// when first starting the audio stream.
			int tmpCurrentQueuedAudioFrames = getQueuedAudioFrames();
			if (_lastAudioFrameSuccessfullyQueued != _frameEnd) {
				// if video is behind audio, then resynch,
				// which here means: don't queue and don't play audio until video catches up.
			    if (_lastAudioFrameSuccessfullyQueued - tmpCurrentQueuedAudioFrames < _frameNext) {
					int addToQueueRep = 0;
					while (addToQueueRep < (maxAllowedAudioPreloadedFrames - tmpCurrentQueuedAudioFrames)
					       && _lastAudioFrameSuccessfullyQueued + 1 <= _frameEnd) {
						_decoder.readFrame(_lastAudioFrameSuccessfullyQueued + 1, kVQAReadAudio);
						queueAudioFrame(_decoder.decodeAudioFrame());
						++_lastAudioFrameSuccessfullyQueued;
						++addToQueueRep;
					}
				}
			}
		}

		if (useTime) {
			_frameNextTime += kVqaFrameTimeDiff;

			// In some cases (as overlay paused by kia or game window is moved) new time might be still in the past.
			// This can cause rapid playback of video where every refresh renders different frame of the video.
			// Can be avoided by setting next time to the future.
			// Note, we use unsigned difference to avoid time overflow issues
			if (now - (_frameNextTime - kVqaFrameTimeDiff) > kVqaFrameTimeDiff) {
				_frameNextTime = now + kVqaFrameTimeDiff;
			}
		}
		++_frameNext;
		result = _frame;
	}

	if (result < 0 && forceDraw && _frame != -1) {
		_decoder.decodeVideoFrame(customSurface != nullptr ? customSurface : _surface, _frame, true);
		result = _frame;
	}
	return result; // Note: result here could be negative.
	               // Negative valid value should only be -1, since there are various assertions
	               // assert(frame >= -1) in overlay modes (elevator, scores, spinner)
}

void VQAPlayer::updateZBuffer(ZBuffer *zbuffer) {
	_decoder.decodeZBuffer(zbuffer);
#if !BLADERUNNER_ORIGINAL_BUGS
	if (_specialPS15GlitchFix) {
		// The glitch (bad z-buffer, value zero (0))
		// is present in the following pixels:
		//  x: 387, y in [179, 192]
		//  x: 388, y in [179, 202]
		for (int y = 179; y < 193; ++y) {
			_vm->_zbuffer->setDataZbufExplicit(387, y, 10720);
			_vm->_zbuffer->setDataZbufExplicit(388, y, 10720);
		}
		for (int y = 193; y < 203; ++y) {
			_vm->_zbuffer->setDataZbufExplicit(388, y, 10720);
		}
	}
#endif
}

void VQAPlayer::updateView(View *view) {
	_decoder.decodeView(view);
}

void VQAPlayer::updateScreenEffects(ScreenEffects *screenEffects) {
	_decoder.decodeScreenEffects(screenEffects);
}

void VQAPlayer::updateLights(Lights *lights) {
	_decoder.decodeLights(lights);
}

bool VQAPlayer::setLoop(int loopId, int repeatsCount, int loopSetMode, void (*callback)(void *, int, int), void *callbackData) {
	if (_s == nullptr) {
		_loopIdInitial = loopId;
		_repeatsCountInitial = repeatsCount;
		return true;
	}

	// TODO IF LOOP IS A "TARGET LOOP ID" then begin and end will get the values for that target (final) loop id
	int begin, end;
	if (!_decoder.getLoopBeginAndEndFrame(loopId, &begin, &end)) {
		return false;
	}
	if (setBeginAndEndFrame(begin, end, repeatsCount, loopSetMode, callback, callbackData)) {
		_loopIdTarget = loopId; // TODO ASDF MAYBE SET THIS AS TARGET LOOP!
		return true;
	}
	return false;
}

bool VQAPlayer::setBeginAndEndFrame(int begin, int end, int repeatsCount, int loopSetMode, void (*callback)(void *, int, int), void *callbackData) {
	if ( begin >= getFrameCount()
	    || end >= getFrameCount()
	    || begin >= end
	    || loopSetMode < 0
	    || loopSetMode >= 3
	) {
		warning("VQAPlayer::setBeginAndEndFrame - Invalid arguments for video");
		return false; // VQA_DECODER_ERROR_BAD_INPUT case
	}

	if (repeatsCount < 0) {
		repeatsCount = -1; // loop "forever"
	}

	if (_repeatsCount == 0 && loopSetMode == kLoopSetModeEnqueue) {
		// if the member var _repeatsCount is 0 (which means "current playing loop will not be repeated")
		// then do not enqueue and, instead, treat the request as kLoopSetModeImmediate
		loopSetMode = kLoopSetModeImmediate;
	}

	_frameBeginNext = begin; // TODO ASDF THIS IN THE MULTI QUEUE CASE WILL BE THE BEGIN FRAME OF THE FINAL ITEM IN QUEUE

	if (loopSetMode == kLoopSetModeJustStart) {
		_repeatsCount = repeatsCount;
		_frameEnd = end;
	} else if (loopSetMode == kLoopSetModeEnqueue) {
		_repeatsCountQueued = repeatsCount; // TODO applies only to the last of the queued loops
		_frameEndQueued = end; // TODO ASDF THIS IN THE MULTI QUEUE CASE WILL BE THE END FRAME OF THE FINAL ITEM IN QUEUE
	} else if (loopSetMode == kLoopSetModeImmediate) {
		_repeatsCount = repeatsCount;
		_frameEnd = end;
		seekToFrame(begin);
	}

	_callbackLoopEnded = callback;
	_callbackData = callbackData;

	return true;
}

bool VQAPlayer::seekToFrame(int frame) {
	_frameNext = frame;
	_frameNextTime = 60 * _vm->_time->currentSystem();
	return true;
}

bool VQAPlayer::getCurrentBeginAndEndFrame(int frame, int *begin, int *end) {
	// updates the values of begin (frame) and end (frame)
	// based on the value of the "frame" argument.
	// First, the current loopId is detemined from the value of the "frame" argument.
	// TODO ASDF THIS MIGHT BE USEFUL!!! _decoder.getLoopIdFromFrame(frame);
	// ALSO SEE USE OF THE CURRENT METHOD IN Scene::resume()
	int playingLoopId = _decoder.getLoopIdFromFrame(frame);
	if (playingLoopId != -1) {
		return _decoder.getLoopBeginAndEndFrame(playingLoopId, begin, end);
	}
	return false;
}

int VQAPlayer::getLoopBeginFrame(int loopId) {
	int begin, end;
	if (!_decoder.getLoopBeginAndEndFrame(loopId, &begin, &end)) {
		return -1;
	}
	return begin;
}

int VQAPlayer::getLoopEndFrame(int loopId) {
	int begin, end;
	if (!_decoder.getLoopBeginAndEndFrame(loopId, &begin, &end)) {
		return -1;
	}
	return end;
}

int VQAPlayer::getFrameCount() const {
	return _decoder.numFrames();
}

int VQAPlayer::getQueuedAudioFrames() const {
	return _audioStream->numQueuedStreams();
}

// Adds another audio "frame" to the queue of the audio stream
void VQAPlayer::queueAudioFrame(Audio::AudioStream *audioStream) {
	if (audioStream == nullptr) {
		return;
	}

	int n = _audioStream->numQueuedStreams();
	// TODO Maybe remove this warning or make it a debug-only message?
	if (n == 0) {
		warning("numQueuedStreams: %d", n);
	}

	_audioStream->queueAudioStream(audioStream, DisposeAfterUse::YES);
}

} // End of namespace BladeRunner
