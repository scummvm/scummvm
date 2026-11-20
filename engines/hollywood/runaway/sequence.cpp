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

void RunawaySequence::update(uint32 now) {
	_now = now;
	if (!_skipFilm && !subtitle().empty() && _speech.finished(now))
		nextLine();
	while (active()) {
		const Command &command = _commands[_index];
		// Preserve the final music cue and normal cleanup when skipping a film.
		if (_skipFilm && command.operation != kMusic && command.operation != kMusicVolume && command.operation != kEnd) {
			advance();
			continue;
		}
		if (!_entered) {
			_entered = true;
			switch (command.operation) {
			case kOpen: {
				const Common::String name = Common::String::format("DATAV%c%02u.%03u", _chapter, command.a, command.b ? command.b : 1);
				if (!_video.load(name.c_str()))
					error("Cannot load Runaway film %s", name.c_str());
				_movie = command.a;
				_stills.close();
				_useStill = false;
				_showPortrait = false;
				break;
			}
			case kPlay:
				_useStill = false;
				_showPortrait = false;
				_repeatAt = 0;
				if (!_video.seekToFrame(command.a))
					error("Cannot seek to Runaway film frame %u", command.a);
				break;
			case kStill:
				_showPortrait = false;
				_useStill = command.b != 0;
				if (_useStill && !_stills.frameCount() && !_stills.load(Common::String::format("DATAV%c%02u.002", _chapter, _movie).c_str()))
					error("Cannot load Runaway film stills");
				if (!(_useStill ? _stills : _video).seekToFrame(command.a))
					error("Cannot load Runaway film still %u", command.a);
				break;
			case kTalk:
			case kWaitSpeech:
				if (command.operation == kTalk || command.c) {
					_showPortrait = command.c != 0;
					_silentPortrait = command.d != 0;
					_portrait = (command.c - 1) / 2;
					if (_showPortrait && !_portraits.load(_chapter, _movie)) {
						error("Cannot load Runaway film portrait %u", command.c - 1);
					}
				}
				if (command.operation == kTalk) {
					speak(command.a, command.b);
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
		}
		switch (command.operation) {
		case kPlay: {
			const uint elapsedFrames = _video.frameAtTime(now - _started);
			const uint count = command.b - command.a + 1;
			const uint frame = command.a + (command.c == kLoopDuringSpeech ? elapsedFrames % count : MIN(elapsedFrames, count - 1));
			if (!_video.decodeToFrame(frame))
				error("Cannot decode Runaway film frame %u", frame);
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
			if (!subtitle().empty())
				return;
			break;
		case kWaitInput:
			return;
		case kDelay:
			if (now - _started < command.a)
				return;
			break;
		case kWaitEffect:
			if (_audio.effectPlaying() && now - _started < command.a)
				return;
			break;
		case kEnd:
			_video.close();
			_stills.close();
			_speech.stop();
			if (!_keepEffects) {
				_audio.stopEffect();
			}
			_lines.clear();
			_commands = nullptr;
			_audio.setListenerPosition(_listenerX);
			return;
		default:
			break;
		}
		advance();
	}
}

void RunawaySequence::skip() {
	if (!active())
		return;
	if (!subtitle().empty()) {
		nextLine();
		return;
	}
	const Command &command = _commands[_index];
	if (command.operation == kPlay) {
		// Bink movies may have only one keyframe; seeking to the end decodes
		// the entire movie synchronously, blocking input while it catches up.
		_skipFilm = true;
	} else if (command.operation == kDelay || command.operation == kWaitEffect || command.operation == kWaitInput) {
		advance();
	}
}

void RunawaySequence::draw(Graphics::ManagedSurface &screen) {
	screen.clear();
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
