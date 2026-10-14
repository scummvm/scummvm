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

#include "common/file.h"
#include "common/util.h"
#include "common/str-enc.h"
#include "image/bmp.h"
#include "hollywood/resource.h"
#include "hollywood/runaway/presentation.h"

namespace Hollywood {

static const uint kInterrogationSpeakerActor[] = {1, 2, 0};
static const uint kInterrogationMouthBase[] = {67, 40, 20};

static void dimViewport(Graphics::ManagedSurface &screen, uint brightness) {
	for (int y = kRunawayViewportTop; y < kRunawayViewportTop + kRunawayViewportHeight; ++y) {
		uint16 *pixels = (uint16 *)screen.getBasePtr(0, y);
		for (int x = 0; x < screen.w; ++x) {
			const uint16 p = pixels[x];
			pixels[x] = (((p >> 11) * brightness / 1000) << 11) |
				((((p >> 5) & 63) * brightness / 1000) << 5) | ((p & 31) * brightness / 1000);
		}
	}
}

bool RunawayInterrogation::load() {
	if (!_background.empty())
		return true;
	ChunkArchive archive;
	if (!archive.open(Common::Path("RESOURCE.E04")) ||
			!_text.loadStage("RESOURCE.003", "Runaway interrogation", 504) ||
			!loadRunawaySurface(archive, 0, 1024, 600, _background))
		return false;
	const uint frames[] = {80, 53, 33};
	for (uint i = 0; i < ARRAYSIZE(_actors); ++i) {
		if (!_actors[i].load(archive, i + 1, frames[i]))
			return false;
	}
	return true;
}

void RunawayInterrogation::start(uint32 now) {
	_now = now;
	_phrase = 0;
	// The four native exchanges alternate a threat and Gina's reply.
	const byte speakers[4][4] = {{0, 2, 0, 0}, {1, 2, 0, 0}, {1, 2, 0, 2}, {0, 2, 1, 2}};
	const uint exchange = _random.getRandomNumber(3);
	_phraseCount = exchange < 2 ? 2 : 4;
	for (uint i = 0; i < _phraseCount; ++i)
		_speakers[i] = speakers[exchange][i];
	for (uint i = 0; i < ARRAYSIZE(_actors); ++i) {
		_frames[i] = 0;
		_idle[i] = false;
		_lastTick[i] = now;
	}
	nextPhrase();
}

void RunawayInterrogation::nextPhrase() {
	_row = 0;
	_lipSync.stop();
	_subtitle.clear();
	if (finished()) {
		_audio.stopSpeech();
		_audio.stopEffect();
		return;
	}
	const uint speaker = _speakers[_phrase];
	const SceneSpeechCue cue = _text.stageCue(speaker, _random.getRandomNumber(speaker == 2 ? 3 : 2));
	const uint count = MAX<uint>(1, cue.continuationCount);
	if (cue.textRecordId < 500 || cue.textRecordId - 500 + count > _text._stageLargeRows.size() / 321 ||
			!cue.voiceSampleId || cue.voiceSampleId + count > 6000)
		error("Invalid Runaway interrogation speech cue");
	_row = cue.textRecordId;
	_lastRow = _row + count - 1;
	_sample = cue.voiceSampleId;
	showSpeechRow();
}

void RunawayInterrogation::showSpeechRow() {
	_subtitle = Common::U32String(_text.largeTextRecord(_row), Common::kDos850);
	const uint32 duration = _audio.playSpeech(_sample);
	_lipSync.start(_subtitle, _now, duration, _random);
	const uint actor = kInterrogationSpeakerActor[_speakers[_phrase]];
	_frames[actor] = kInterrogationMouthBase[actor] + _lipSync.frame(_now);
	_idle[actor] = true;
	_lastTick[actor] = _now;
	_deadline = _now + (duration ? MAX<uint>(1000, duration + 150) : MAX<uint>(3000, _subtitle.size() * 55));
}

void RunawayInterrogation::skipSpeech() {
	if (!speaking())
		return;
	const uint actor = kInterrogationSpeakerActor[_speakers[_phrase]];
	_frames[actor] = kInterrogationMouthBase[actor];
	_audio.stopSpeech();
	if (_row < _lastRow) {
		++_row;
		++_sample;
		showSpeechRow();
	} else {
		++_phrase;
		nextPhrase();
	}
}

void RunawayInterrogation::update(uint32 now) {
	_now = now;
	if (speaking() && !_audio.speechPlaying() && (int32)(now - _deadline) >= 0)
		skipSpeech();
	if (finished())
		return;
	const uint idleEnd[] = {67, 39, 19};
	const uint speaker = kInterrogationSpeakerActor[_speakers[_phrase]];
	for (uint i = 0; i < ARRAYSIZE(_actors); ++i) {
		if (i == speaker) {
			// Native 0x43c5a0/0x43c700/0x43c860 share the text-driven
			// mouth sequence. Gina's table maps virtual 70..81 to 67..78.
			_frames[i] = kInterrogationMouthBase[i] + _lipSync.frame(now);
			_idle[i] = true;
			_lastTick[i] = now;
			continue;
		}
		const uint tick = i == 0 ? 60 : 80;
		while (now - _lastTick[i] >= tick) {
			_lastTick[i] += tick;
			if (_idle[i]) {
				if (_frames[i] != kInterrogationMouthBase[i]) {
					_frames[i] = kInterrogationMouthBase[i];
				} else if (_random.getRandomNumber(14) == 0) {
					_frames[i] = kInterrogationMouthBase[i] + 6;
				} else if (_random.getRandomNumber(i == 0 ? 49 : 9) == 0) {
					_idle[i] = false;
					_frames[i] = 0;
				}
			} else if (_frames[i] < idleEnd[i]) {
				++_frames[i];
				if (i == 2 && _frames[i] == 9)
					_audio.playEffect(23, 255, 1);
				if (i == 1 && (_frames[i] == 10 || _frames[i] == 33))
					_audio.playEffect(_frames[i] == 10 ? 24 : 25, _frames[i] == 10 ? 102 : 255);
			} else if (_random.getRandomNumber(i == 0 ? 1 : 4) == 0) {
				_idle[i] = true;
			}
		}
	}
}

void RunawayInterrogation::draw(Graphics::ManagedSurface &screen) const {
	screen.clear();
	screen.blitFrom(_background, Common::Point(0, kRunawayViewportTop));
	const uint order[] = {1, 2, 0};
	for (uint i : order)
		_actors[i].draw(screen, _frames[i], 0, kRunawayViewportTop);
}

bool RunawayEnding::load() {
	ChunkArchive archive, cards;
	if (!archive.open(Common::Path("RESOURCE.E01")) || !cards.open(Common::Path("RESOURCE.X01")) ||
			!loadRunawaySurface(archive, 0, 1024, 600, _outside) ||
			!loadRunawaySurface(cards, 1, 1024, 600, _card) ||
			!loadRunawaySurface(118, 48, 48, _demoLogo))
		return false;
	Common::Array<byte> patch;
	for (uint chunk = 3; chunk <= 7; ++chunk) {
		if (!archive.readVariableChunk(chunk, patch) || !applyRunawayPatch(patch, _outside))
			return false;
	}
	const uint frames[] = {48, 700, 186, 31, 329, 307, 1052};
	for (uint i = 0; i < ARRAYSIZE(_layers); ++i) {
		// The native overview leaves chunk 13 inactive.
		if (i != 4 && !_layers[i].load(archive, 9 + i, frames[i]))
			return false;
	}
	Graphics::ManagedSurface scratch;
	scratch.copyFrom(_outside);
	for (uint i = 0; i < ARRAYSIZE(_occlusion); ++i) {
		if (!archive.readVariableChunk(16 + i, _occlusion[i]) || !applyRunawayPatch(_occlusion[i], scratch))
			return false;
	}
	return true;
}

void RunawayEnding::start(uint32 now) {
	_stage = kOutside;
	_started = _now = _lastTick = now;
	_screenshot = 0;
	_fadeDuration = 0;
	_frames[0] = 0;
	const uint ambient[] = {1, 2, 5};
	for (uint i : ambient) {
		_active[i] = _random.getRandomBit();
		_frames[i] = _active[i] ? _random.getRandomNumber(_layers[i].frameCount() - 1) : _layers[i].frameCount() - 1;
	}
	// The two foreground bird tracks are alternatives, not simultaneous loops.
	_active[6] = _random.getRandomBit();
	_active[3] = !_active[6];
	_frames[3] = _active[3] ? 0 : 30;
	_frames[6] = _active[6] ? _random.getRandomNumber(79) : 1051;
	// The overview's music continues through the screenshots and final card.
	// Unlike room music it plays once; the native card waits for its tail.
	_musicUntil = now + _audio.playMusic(21, false);
	_audio.playEffect(12, 77, 0, true);
}

void RunawayEnding::nextSlide(uint32 now) {
	_started = now;
	_fadeDuration = 0;
	_audio.stopEffect();
	// 0x4446c0 visits Screenshots/01.bmp through 16.bmp. The native loader
	// ignores absent or unsuitable images, allowing incomplete installations
	// to reach the final card too. Keep just the current screenshot in memory.
	while (++_screenshot <= 16) {
		Common::File file;
		if (!file.open(Common::Path(Common::String::format("%02u.bmp", _screenshot))))
			continue;
		Image::BitmapDecoder decoder;
		if (!decoder.loadStream(file) || !decoder.getSurface() ||
				decoder.getSurface()->w != kRunawayScreenWidth || decoder.getSurface()->h != kRunawayViewportHeight ||
				decoder.getSurface()->format.bytesPerPixel < 3) {
			warning("Runaway: invalid promotional screenshot %02u.bmp", _screenshot);
			continue;
		}
		_slide.convertFrom(*decoder.getSurface(), _outside.format);
		_stage = kSlideshow;
		return;
	}
	_slide.free();
	_stage = kCard;
}

void RunawayEnding::update(uint32 now) {
	_now = now;
	while (_stage == kOutside && now - _lastTick >= 80) {
		_lastTick += 80;
		_frames[0] = (_frames[0] + 1) % _layers[0].frameCount();
		const uint ambient[] = {1, 2, 3, 5, 6};
		for (uint i : ambient) {
			if (_active[i]) {
				if (_frames[i] + 1 < _layers[i].frameCount())
					++_frames[i];
				else
					_active[i] = false;
			} else if (i != 3 && i != 6 && _random.getRandomNumber(i == 5 ? 49 : 9) == 0) {
				_active[i] = true;
				_frames[i] = 0;
			}
		}
	}
	if (_stage == kOutside && now - _started >= 12000) {
		nextSlide(_started + 12000);
	}
	while (_stage == kSlideshow || _stage == kCard) {
		if (!_fadeDuration) {
			// The French release holds its larger final advertisement for
			// twenty seconds; the German and Spanish cards hold for five.
			uint32 holdEnd = _started + (_stage == kCard && _language == Common::FR_FRA ? 20500 : 5500);
			if (_stage == kCard && (int32)(_musicUntil - holdEnd) > 0)
				holdEnd = _musicUntil;
			if ((int32)(now - holdEnd) < 0)
				break;
			_fadeStarted = holdEnd;
			_fadeDuration = 500;
		}
		if (now - _fadeStarted < _fadeDuration)
			break;
		if (_stage == kSlideshow) {
			nextSlide(_fadeStarted + _fadeDuration);
		} else {
			_stage = kFinished;
			_audio.stopMusic();
		}
	}
	if (_stage == kCard && _fadeDuration)
		_audio.setMusicVolume(96 * (_fadeDuration - (now - _fadeStarted)) / _fadeDuration);
}

void RunawayEnding::skip(uint32 now) {
	_now = now;
	if (_stage == kOutside) {
		nextSlide(now);
	} else if ((_stage == kSlideshow || _stage == kCard) && !_fadeDuration && now - _started >= 500) {
		_fadeStarted = now;
		_fadeDuration = _stage == kCard ? 250 : 500;
	}
}

void RunawayEnding::draw(Graphics::ManagedSurface &screen) const {
	screen.clear();
	const uint elapsed = _now - _started;
	if (_stage == kOutside) {
		screen.blitFrom(_outside, Common::Point(0, kRunawayViewportTop));
		const uint order[] = {0, 2, 5, 1};
		for (uint i : order)
			_layers[i].draw(screen, _frames[i], 0, kRunawayViewportTop);
		if (_active[1] && _frames[1] >= 257 && _frames[1] <= 336) {
			applyRunawayPatch(_occlusion[0], screen, kRunawayViewportTop);
			applyRunawayPatch(_occlusion[1], screen, kRunawayViewportTop);
		} else if (_active[1] && _frames[1] >= 507 && _frames[1] <= 532) {
			applyRunawayPatch(_occlusion[1], screen, kRunawayViewportTop);
		}
		_layers[3].draw(screen, _frames[3], 0, kRunawayViewportTop);
		_layers[6].draw(screen, _frames[6], 0, kRunawayViewportTop);
		dimViewport(screen, elapsed < 10000 ? 1000 : (12000 - MIN<uint>(12000, elapsed)) / 2);
	} else if (_stage == kSlideshow || _stage == kCard) {
		screen.blitFrom(_stage == kSlideshow ? _slide : _card, Common::Point(0, kRunawayViewportTop));
		// The later demos blink different parts of their localized artwork
		// (DE 0x4447d0, FR 0x44f190); the Spanish card has no blinking text.
		const uint blinkTime = _fadeDuration ? _fadeStarted - _started : elapsed;
		if (_stage == kCard && blinkTime >= 500 && ((blinkTime - 500) / 500) % 2) {
			if (_language == Common::DE_DEU)
				screen.fillRect(Common::Rect(372, 330 + kRunawayViewportTop, 652, 355 + kRunawayViewportTop), 0);
			else if (_language == Common::FR_FRA)
				screen.fillRect(Common::Rect(373, 183 + kRunawayViewportTop, 653, 208 + kRunawayViewportTop), 0);
		}
		const uint brightness = _fadeDuration ?
			1000 * (_fadeDuration - MIN<uint>(_fadeDuration, _now - _fadeStarted)) / _fadeDuration : MIN<uint>(1000, elapsed * 2);
		dimViewport(screen, brightness);
	}
	if (_stage == kOutside || _stage == kSlideshow)
		screen.blitFrom(_demoLogo, Common::Point(976, 720));
}

} // End of namespace Hollywood
