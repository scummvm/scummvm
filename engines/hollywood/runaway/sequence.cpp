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

#include "common/scummsys.h"
#include "common/str-enc.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "hollywood/runaway/sequence.h"

namespace Hollywood {

void RunawaySequence::start(const Command *commands, const SceneTextStore &text, uint32 now, char chapter, bool keepEffects) {
	if (_preparePause.isActive()) {
		_preparePause.clear();
	}
	if (!active()) {
		_listenerX = _audio.listenerPosition();
	}
	_background.free();
	_destination = Common::Rect(0, kRunawayViewportTop, kRunawayScreenWidth, kRunawayViewportTop + kRunawayViewportHeight);
	_chapter = chapter;
	_text = &text;
	_speech.stop();
	_keepEffects = keepEffects;
	if (!_keepEffects) {
		_audio.stopEffect();
	}
	_audio.setListenerPosition(kRunawayScreenWidth / 2);
	_lines.clear();
	_line = 0;
	_commands = commands;
	_index = 0;
	_entered = _showPortrait = false;
	_skipFilm = false;
	_skipSpeechPending = false;
	_now = _started = now;
	update(now);
}

void RunawaySequence::setBackground(const Graphics::ManagedSurface &background, const Common::Rect &destination) {
	_background.copyFrom(background);
	_destination = destination;
}

void RunawaySequence::advance() {
	++_index;
	_entered = false;
	_started = _now;
}

void RunawaySequence::speak(uint cue, uint variation, uint lastVariation, bool global) {
	for (uint v = variation; v <= MAX(variation, lastVariation); ++v) {
		SceneSpeechCue speech;
		if (global) {
			_text->getStaticSpeechCue(cue, v, speech.textRecordId, speech.continuationCount, speech.voiceSampleId);
		} else {
			speech = _text->stageCue(cue, v);
		}
		if (!speech.valid() || speech.textRecordId == 0xffff)
			continue;
		for (uint i = 0; i < MAX<uint>(1, speech.continuationCount); ++i) {
			Line line;
			line.text = Common::U32String(_text->largeTextRecord(speech.textRecordId + i), Common::kDos850);
			line.sample = speech.voiceSampleId ? speech.voiceSampleId + i : 0;
			line.global = global;
			if (!line.text.empty())
				_lines.push_back(line);
		}
	}
	if (subtitle().empty())
		nextLine();
}

void RunawaySequence::nextLine() {
	_speech.stop();
	if (_line >= _lines.size()) {
		_line = 0;
		_lines.clear();
		return;
	}
	const Line &line = _lines[_line++];
	_speech.start(line.text, line.sample, line.global, _now, 0);
}

void RunawaySequence::finishPreparation() {
	if (_preparePause.isActive()) {
		_preparePause.clear();
	}
	if (_skipSpeechPending) {
		_skipSpeechPending = false;
		nextLine();
	}
}

void RunawaySequence::cacheRevisitedFrames() {
	uint furthestFrame = 0;
	for (uint i = _index + 1; _commands[i].operation != kEnd; ++i) {
		const Command &command = _commands[i];
		if (command.operation == kOpen || command.operation == kImage) {
			break;
		}
		if (command.operation == kPlay) {
			if (command.a < furthestFrame) {
				_video.cacheFrames(command.a, MIN<uint>(command.b, furthestFrame));
			}
			if (command.c == kLoopDuringSpeech || command.c == kIdleDuringSpeech) {
				_video.cacheFrames(command.a, command.b);
			}
			furthestFrame = MAX<uint>(furthestFrame, command.b);
		} else if (command.operation == kStill && !command.b) {
			if (command.a < furthestFrame) {
				_video.cacheFrames(command.a, command.a);
			}
			furthestFrame = MAX<uint>(furthestFrame, command.a);
		}
	}
}

void RunawaySequence::prefetch() {
	if (!_video.isBink() || _preparePause.isActive() || _skipFilm) {
		return;
	}
	// Decode at most one additional frame per update without changing the
	// current shot or speech. The main decoder can advance while holding a still.
	bool nextVideo = _commands[_index].operation != kPlay;
	for (uint i = _index + 1; _commands[i].operation != kEnd; ++i) {
		const Command &command = _commands[i];
		if (command.operation == kOpen || command.operation == kImage) {
			break;
		}
		RunawayVideo *video = nullptr;
		if ((command.operation == kTalk || command.operation == kWaitSpeech) && command.c) {
			if (!_portraits.load(_chapter, _movie)) {
				error("Cannot load Runaway film portraits");
			}
			const uint portrait = (command.c - 1) / 2;
			const RunawayVideo::FrameStatus status = _portraits.prepare(portrait, 0);
			if (status == RunawayVideo::kFrameError) {
				error("Cannot prepare Runaway film portrait %u", portrait);
			}
			if (status == RunawayVideo::kFramePending) {
				if (_portraits.prepare(portrait, 1) == RunawayVideo::kFrameError) {
					error("Cannot prepare Runaway film portrait %u", portrait);
				}
				return;
			}
		} else if (command.operation == kStill && command.b) {
			if (!_stills.frameCount()) {
				if (!_stills.load(Common::String::format("DATAV%c%02u.002", _chapter, _movie).c_str())) {
					error("Cannot load Runaway film stills");
				}
				_stills.cacheFrames(0, _stills.frameCount() - 1);
				return;
			}
			video = &_stills;
		} else if (nextVideo && (command.operation == kPlay || command.operation == kStill)) {
			video = &_video;
			nextVideo = false;
		}
		if (video) {
			const RunawayVideo::FrameStatus status = video->prefetchFrame(command.a, 0);
			if (status == RunawayVideo::kFrameError) {
				error("Cannot prepare Runaway film frame %u", command.a);
			}
			if (status == RunawayVideo::kFramePending) {
				if (video->prefetchFrame(command.a, 1) == RunawayVideo::kFrameError) {
					error("Cannot prepare Runaway film frame %u", command.a);
				}
				return;
			}
		}
	}
}

void RunawaySequence::update(uint32 now) {
	_now = now;
	if (!_skipFilm && !_preparePause.isActive() && (_skipSpeechPending || (!subtitle().empty() && _speech.finished(now)))) {
		_skipSpeechPending = false;
		nextLine();
	}
	while (active()) {
		const Command &command = _commands[_index];
		// Preserve the final music cue and normal cleanup when skipping a film.
		if (_skipFilm && command.operation != kMusic && command.operation != kMusicVolume && command.operation != kEnd) {
			advance();
			continue;
		}
		if (!_entered) {
			// Single-keyframe movies must be decoded incrementally before a cut.
			// Pause audio and the engine clock while returning to the event loop.
			const bool prepare = command.operation == kOpen || command.operation == kPlay || command.operation == kStill ||
				((command.operation == kTalk || command.operation == kWaitSpeech) && command.c);
			if (prepare && !_preparePause.isActive()) {
				_preparePause = g_engine->pauseEngine();
				now = _now = g_engine->getTotalPlayTime();
			}
			switch (command.operation) {
			case kOpen: {
				const Common::String name = Common::String::format("DATAV%c%02u.%03u", _chapter, command.a, command.b ? command.b : 1);
				if (!_video.load(name.c_str()))
					error("Cannot load Runaway film %s", name.c_str());
				cacheRevisitedFrames();
				_movie = command.a;
				_stills.close();
				_useStill = false;
				_showPortrait = false;
				break;
			}
			case kPlay: {
				_useStill = false;
				_showPortrait = false;
				_repeatAt = 0;
				const RunawayVideo::FrameStatus status = _video.prepareFrame(command.a, 4);
				if (status == RunawayVideo::kFrameError) {
					error("Cannot seek to Runaway film frame %u", command.a);
				}
				if (status == RunawayVideo::kFramePending) {
					return;
				}
				break;
			}
			case kStill: {
				_showPortrait = false;
				_useStill = command.b != 0;
				if (_useStill && !_stills.frameCount()) {
					if (!_stills.load(Common::String::format("DATAV%c%02u.002", _chapter, _movie).c_str())) {
						error("Cannot load Runaway film stills");
					}
					_stills.cacheFrames(0, _stills.frameCount() - 1);
				}
				const RunawayVideo::FrameStatus status = (_useStill ? _stills : _video).prepareFrame(command.a, 4);
				if (status == RunawayVideo::kFrameError) {
					error("Cannot load Runaway film still %u", command.a);
				}
				if (status == RunawayVideo::kFramePending) {
					return;
				}
				break;
			}
			case kTalk:
			case kWaitSpeech:
				if (command.operation == kTalk || command.c) {
					_showPortrait = command.c != 0;
					_silentPortrait = command.d != 0;
					_portrait = (command.c - 1) / 2;
					if (_showPortrait && !_portraits.load(_chapter, _movie)) {
						error("Cannot load Runaway film portrait %u", command.c - 1);
					}
					if (_showPortrait) {
						const RunawayVideo::FrameStatus status = _portraits.prepare(_portrait, 4);
						if (status == RunawayVideo::kFrameError) {
							error("Cannot load Runaway film portrait %u", command.c - 1);
						}
						if (status == RunawayVideo::kFramePending) {
							return;
						}
					}
				}
				break;
			case kImage: {
				Graphics::ManagedSurface image;
				if (!loadRunawaySurface(command.a, kRunawayScreenWidth, kRunawayViewportHeight, image)) {
					error("Cannot load Runaway image %u", command.a);
				}
				_video.close();
				_stills.close();
				_useStill = _showPortrait = false;
				_background.create(kRunawayScreenWidth, kRunawayScreenHeight, image.format);
				_background.clear();
				_background.blitFrom(image, Common::Point(0, kRunawayViewportTop));
				break;
			}
			case kSpeech:
				speak(command.a, command.b, command.c, command.d != 0);
				break;
			case kMusic:
				_audio.playMusic(command.a, command.c != 0);
				_audio.setMusicVolume(command.b);
				break;
			case kEffect:
				_audio.playEffect(command.a, command.b, command.c, command.d != 0);
				break;
			case kMusicVolume:
				_audio.setMusicVolume(command.a);
				break;
			case kStopEffects:
				if (command.a) {
					_audio.stopEffect(command.a - 1);
				} else {
					_audio.stopEffect();
				}
				break;
			default:
				break;
			}
			if (prepare) {
				_started = now;
				finishPreparation();
			}
			if (command.operation == kTalk) {
				speak(command.a, command.b);
			}
			_entered = true;
		}
		switch (command.operation) {
		case kPlay: {
			const uint elapsedFrames = _video.frameAtTime(now - _started);
			const uint count = command.b - command.a + 1;
			const uint frame = command.a + (command.c == kLoopDuringSpeech ? elapsedFrames % count : MIN(elapsedFrames, count - 1));
			const bool frameReady = _video.prepareFrame(frame, 0) == RunawayVideo::kFrameReady;
			const RunawayVideo::FrameStatus status = _video.prepareFrame(frame, 4);
			if (status == RunawayVideo::kFrameError) {
				error("Cannot decode Runaway film frame %u", frame);
			}
			if (status == RunawayVideo::kFramePending) {
				if (!_preparePause.isActive()) {
					_preparePause = g_engine->pauseEngine();
				}
				return;
			}
			finishPreparation();
			if (frameReady) {
				// Use updates between movie frames to prepare the next speakers.
				prefetch();
			}
			if (command.c == kUntilSpeechEnds) {
				if (elapsedFrames < count && !subtitle().empty())
					return;
				break;
			}
			if (command.c == kIdleDuringSpeech && elapsedFrames >= count && !subtitle().empty()) {
				if (!_repeatAt)
					_repeatAt = now + 80;
				if ((int32)(now - _repeatAt) >= 0) {
					// The native player occasionally repeats Gina's idle shot.
					_repeatAt = now + 80;
					if (!_random.getRandomNumber(24))
						_started = now;
				}
			}
			if ((command.c != kLoopDuringSpeech && elapsedFrames < count) || (command.c != kOnce && !subtitle().empty()))
				return;
			break;
		}
		case kTalk:
		case kWaitSpeech:
			if (!subtitle().empty()) {
				prefetch();
				return;
			}
			break;
		case kWaitInput:
			prefetch();
			return;
		case kDelay:
			if (now - _started < command.a) {
				prefetch();
				return;
			}
			break;
		case kWaitEffect:
			if (_audio.effectPlaying() && now - _started < command.a) {
				prefetch();
				return;
			}
			break;
		case kEnd:
			_video.close();
			_stills.close();
			_portraits.close();
			_speech.stop();
			if (!_keepEffects) {
				_audio.stopEffect();
			}
			_lines.clear();
			_commands = nullptr;
			_audio.setListenerPosition(_listenerX);
			if (_preparePause.isActive()) {
				_preparePause.clear();
			}
			return;
		default:
			break;
		}
		advance();
	}
}

void RunawaySequence::skip() {
	if (!active()) {
		return;
	}
	_speech.stop();
	_lines.clear();
	_skipSpeechPending = false;
	_skipFilm = true;
	// Do not briefly start the music belonging to skipped portions of the film.
	for (uint i = _index; _commands[i].operation != kEnd; ++i) {
		if (_commands[i].operation == kMusic) {
			_index = i;
			_entered = false;
		}
	}
}

void RunawaySequence::skipSpeech() {
	if (!active() || _skipFilm) {
		return;
	}
	if (!subtitle().empty()) {
		_speech.stop();
		_skipSpeechPending = true;
	} else if (!_skipSpeechPending && _commands[_index].operation == kWaitInput) {
		advance();
	}
}

void RunawaySequence::draw(Graphics::ManagedSurface &screen) {
	if (!active() || _preparePause.isActive()) {
		return;
	}
	screen.clear();
	if (_commands[_index].operation == kWaitEffect && _commands[_index].b) {
		return;
	}
	const Common::Rect &viewport = _destination;
	if (_background.w)
		screen.blitFrom(_background);
	(_useStill ? _stills : _video).draw(screen, viewport);
	if (_showPortrait) {
		if (!_portraits.draw(screen, viewport, _portrait, _silentPortrait ? 0 : speechFrame()))
			error("Cannot draw Runaway film portrait %u", _portrait);
	}
}

} // End of namespace Hollywood
