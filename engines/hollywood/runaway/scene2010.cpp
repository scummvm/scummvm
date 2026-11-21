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
#include "common/textconsole.h"
#include "common/str-enc.h"
#include "common/util.h"
#include "hollywood/resource.h"
#include "hollywood/runaway/presentation.h"
#include "hollywood/runaway/scene2010.h"

namespace Hollywood {
namespace {

// Use the surface blitter's alpha support for the original picture dissolves.
// A 32-bit RGB source leaves 0xffffffff available as a transparent key without
// losing white pixels from the RGB565 artwork.
const Graphics::PixelFormat kPictureFormat(4, 8, 8, 8, 0, 16, 8, 0, 0);

} // End of anonymous namespace

bool RunawayOpening::load(byte chapter) {
	_failed = false;
	_chapterVariation = _narrationCue = 0;
	_chapterOnly = chapter != 0;
	_chapter = chapter ? chapter : 1;
	ChunkArchive archive;
	if (!archive.open(Common::Path("RESOURCE.A00")) ||
			!loadRunawaySurface(archive, 6, 1024, 600, _narratorBackground) ||
			!archive.readVariableChunk(6 + _chapter, _chapterTitle) ||
			!_text.loadStage("RESOURCE.003", "Runaway narrator", 100))
		return false;
	const uint narratorFrames[] = {178, 35, 7, 35, 35, 35};
	for (uint i = 0; i < ARRAYSIZE(_narrator); ++i) {
		if (!_narrator[i].load(archive, i, narratorFrames[i]))
			return false;
	}
	if (_chapterOnly)
		return true;

	if (!archive.open(Common::Path("RESOURCE.B01")) ||
			!loadRunawaySurface(archive, 0, 1592, 600, _city) ||
			!_cityActor.load(archive, 1, 213))
		return false;
	for (uint i = 0; i < ARRAYSIZE(_parallax); ++i) {
		if (!archive.readVariableChunk(i + 2, _parallax[i]))
			return false;
	}
	const uint titleChunks[] = {5, 8, 9, 6};
	for (uint i = 0; i < ARRAYSIZE(_titles); ++i) {
		if (!_titles[i].load(archive, titleChunks[i], 75))
			return false;
	}
	// B01 chunk 7 is the 400x39 title at room coordinates (879, 548).
	Graphics::ManagedSurface scratch(400, 39, _city.format);
	Common::Array<byte> patch;
	if (!archive.readVariableChunk(7, patch) || !applyRunawayPatch(patch, scratch, -548, -879))
		return false;
	_cityTitle.convertFrom(scratch, kPictureFormat);
	for (uint i = 0; i < ARRAYSIZE(_pictures) - 1; ++i) {
		if (!loadRunawaySurface(archive, i + 10, 282, 188, scratch))
			return false;
		_pictures[i].convertFrom(scratch, kPictureFormat);
	}
	_pictures[16].create(282, 188, kPictureFormat);
	for (uint i = 0; i < ARRAYSIZE(_credits); ++i) {
		if (!_credits[i].load(archive, i + 26, 12))
			return false;
	}
	return _portraits.load('B', 8);
}

bool RunawayOpening::loadEndingNarration(byte cue) {
	if (cue < 7 || cue > 9 || !load(6)) {
		return false;
	}
	_narrationCue = cue;
	_chapterTitle.clear();
	return true;
}

uint RunawayOpening::narrationVariations() const {
	if (_narrationCue) {
		return _narrationCue == 8 ? 4 : 2;
	}
	return _chapter == 2 ? 2 : _chapter == 3 ? 5 : _chapter == 4 || _chapter == 5 ? 4 : _chapter == 6 ? 3 : 1;
}

void RunawayOpening::start(uint32 now) {
	_audio.setChapter(_chapter);
	enter(_chapterOnly ? kChapter : kSitDown, now);
}

void RunawayOpening::enter(Step step, uint32 now) {
	_step = step;
	_now = _started = now;
	_events = 0;
	_row = 0;
	_speech.stop();
	switch (step) {
	case kSitDown:
		_firstFrame = 0;
		_lastFrame = 20;
		_headBank = 0;
		break;
	case kLeanForward:
		_firstFrame = 161;
		_lastFrame = 177;
		_headBank = 0;
		break;
	case kLeanBack:
		_firstFrame = 177;
		_lastFrame = 161;
		_headBank = 0;
		break;
	case kTurnToScreen:
		_firstFrame = 128;
		_lastFrame = 160;
		_headBank = 0;
		break;
	case kTurnBack:
		_firstFrame = 160;
		_lastFrame = 128;
		_headBank = 0;
		break;
	case kIntroduction:
		_bodyFrame = 20;
		_headBank = 1;
		speak(0, 0);
		break;
	case kQuestion:
		_bodyFrame = 177;
		_headBank = 5;
		speak(0, 1);
		break;
	case kBackground:
		_bodyFrame = 20;
		_headBank = 1;
		speak(0, 2);
		break;
	case kJourney:
		_bodyFrame = 160;
		_headBank = 4;
		speak(0, 3);
		break;
	case kLastNarration:
		_bodyFrame = 20;
		_headBank = 1;
		speak(0, 9);
		break;
	case kMontage:
		startMovie("DATAVA02.001", 0, 700);
		_audio.playIntroMusic(4);
		break;
	case kCity:
		_video.close();
		_narrating = false;
		_failed = !_text.loadStage("RESOURCE.003", "Runaway opening", 201);
		_audio.playMusic(32);
		_audio.setMusicVolume(204);
		break;
	case kAccident:
		startMovie("DATAVB04.001", 0, 250);
		_audio.playEffect(27);
		break;
	case kAccidentTitle:
		_audio.playEffect(28, 255, 1);
		break;
	case kAccidentEnd:
		startMovie(nullptr, 261, 312);
		break;
	case kCityExit:
		_audio.stopEffect();
		_video.close();
		break;
	case kFilm:
		startMovie("DATAVB08.001", 0, 2565);
		if (!_themeStarted) {
			_audio.playMusic(17, false);
			_audio.setMusicVolume(255);
			_themeStarted = true;
		}
		break;
	case kStayAway:
		if (Common::File::exists("DATAVC00.000"))
			_failed = !_video.seekToFrame(2566);
		speak(1, 2);
		break;
	case kApproach:
		startMovie(nullptr, 2578, 2626);
		break;
	case kUnconscious:
		speak(1, 3);
		break;
	case kTurn:
		startMovie(nullptr, 2638, 2653);
		break;
	case kHospital:
		if (Common::File::exists("DATAVC00.000"))
			_failed = !_video.seekToFrame(2655);
		speak(1, 4);
		break;
	case kHitman:
		speak(1, 5);
		break;
	case kReply:
		speak(1, 6);
		break;
	case kFilmEnd:
		startMovie(nullptr, 2668, 2927);
		_audio.playEffect(42);
		break;
	case kChapter:
		_video.close();
		_audio.stopMusic();
		_audio.stopEffect();
		_narrating = true;
		_failed = !_text.loadStage("RESOURCE.003", "Runaway narrator", 100);
		_bodyFrame = _chapter == 2 && _chapterVariation ? 160 : _chapter == 3 && !_chapterVariation ? 177 :
			_chapter == 3 && _chapterVariation == 2 ? 50 : 20;
		_headBank = _chapter == 2 && _chapterVariation ? 4 : _chapter == 3 && !_chapterVariation ? 5 :
			_chapter == 3 && _chapterVariation == 2 ? 2 : 1;
		if (_narrationCue) {
			_bodyFrame = _chapterVariation == 0 ? (_narrationCue == 7 ? 177 : 160) : _chapterVariation == 2 ? 110 : 20;
			_headBank = _chapterVariation == 0 ? (_narrationCue == 7 ? 5 : 4) : _chapterVariation == 2 ? 3 : 1;
		} else if (_chapter == 4) {
			const byte poses[] = {109, 20, 177, 20};
			const byte heads[] = {3, 1, 5, 1};
			_bodyFrame = poses[_chapterVariation];
			_headBank = heads[_chapterVariation];
		} else if (_chapter == 5) {
			_bodyFrame = _chapterVariation ? 20 : 50;
			_headBank = _chapterVariation ? 1 : 2;
		} else if (_chapter == 6) {
			_bodyFrame = _chapterVariation == 1 ? 177 : 20;
			_headBank = _chapterVariation == 1 ? 5 : 1;
		}
		speak(_narrationCue ? _narrationCue : _chapter, _chapterVariation);
		break;
	case kChapterGesture:
		_headBank = 0;
		if (_narrationCue) {
			if (_chapterVariation == 1) {
				_firstFrame = _narrationCue == 7 ? 177 : 160;
				_lastFrame = _narrationCue == 7 ? 161 : 128;
			} else {
				_firstFrame = _chapterVariation == 2 ? 76 : 111;
				_lastFrame = _chapterVariation == 2 ? 110 : 127;
			}
		} else if (_chapter == 4) {
			_firstFrame = _chapterVariation == 1 ? 110 : _chapterVariation == 2 ? 161 : 177;
			_lastFrame = _chapterVariation == 1 ? 127 : _chapterVariation == 2 ? 177 : 161;
		} else if (_chapter == 2) {
			_firstFrame = 128;
			_lastFrame = 160;
		} else if (_chapter == 6) {
			_firstFrame = _chapterVariation == 1 ? 161 : 177;
			_lastFrame = _chapterVariation == 1 ? 177 : 161;
		} else if (_chapter == 5) {
			if (_chapterVariation == 1) {
				_firstFrame = 51;
				_lastFrame = 75;
			} else {
				_headBank = 1;
				_firstFrame = _chapterVariation == 2 ? 7 : 17;
				_lastFrame = _chapterVariation == 2 ? 10 : 20;
			}
		} else if (_chapterVariation == 1) {
			_firstFrame = 177;
			_lastFrame = 161;
		} else {
			_firstFrame = _chapterVariation == 2 ? 21 : 51;
			_lastFrame = _chapterVariation == 2 ? 50 : 75;
		}
		break;
	case kFinished:
		_video.close();
		_audio.stopMusic();
		_audio.stopEffect();
		break;
	default:
		break;
	}
	_frame = _firstFrame;
	if (!_headBank && _narrating)
		_bodyFrame = _firstFrame;
}

void RunawayOpening::advance() {
	if (_step == kChapter) {
		const uint variations = narrationVariations();
		if (++_chapterVariation < variations)
			enter(_chapterVariation < 4 ? kChapterGesture : kChapter, _now);
		else
			enter(kFinished, _now);
	} else if (_step == kChapterGesture) {
		enter(kChapter, _now);
	} else {
		enter((Step)(_step + 1), _now);
	}
}

void RunawayOpening::speak(uint16 cue, byte variation) {
	const SceneSpeechCue speech = _text.stageCue(cue, variation);
	const uint count = MAX<uint>(1, speech.continuationCount);
	if (!speech.valid() || speech.textRecordId < 500 ||
			speech.textRecordId - 500 + count > _text._stageLargeRows.size() / 321 ||
			!speech.voiceSampleId || speech.voiceSampleId + count > 6000) {
		warning("Runaway: invalid opening speech cue %u/%u", cue, variation);
		_failed = true;
		return;
	}
	_row = speech.textRecordId;
	_lastRow = _row + count - 1;
	_sample = speech.voiceSampleId;
	showLine();
}

void RunawayOpening::showLine() {
	// The framing narration belongs to DATAACA0, independently of chapter.
	_speech.start(Common::U32String(_text.largeTextRecord(_row), Common::kDos850), _sample, _narrating, _now);
}

void RunawayOpening::nextLine() {
	_speech.stop();
	if (_row < _lastRow) {
		++_row;
		++_sample;
		showLine();
	} else {
		_row = 0;
	}
}

void RunawayOpening::startMovie(const char *name, uint first, uint last) {
	if ((name && !_video.load(name)) || last >= _video.frameCount() || !_video.decodeToFrame(first)) {
		warning("Runaway: cannot load opening movie segment %u..%u", first, last);
		_failed = true;
		return;
	}
	_firstFrame = first;
	_lastFrame = last;
}

bool RunawayOpening::updateMovie() {
	_frame = MIN(_lastFrame, _firstFrame + _video.frameAtTime(_now - _started));
	if (!_video.decodeToFrame(_frame)) {
		_failed = true;
		return false;
	}
	return _frame == _lastFrame;
}

void RunawayOpening::update(uint32 now) {
	_now = now;
	if (finished())
		return;
	if (speaking() && !_audio.speechPlaying() && _speech.finished(now))
		nextLine();
	const uint elapsed = now - _started;
	switch (_step) {
	case kSitDown:
	case kLeanForward:
	case kLeanBack:
	case kTurnToScreen:
	case kTurnBack: {
		const uint distance = ABS((int)_lastFrame - (int)_firstFrame);
		const uint offset = MIN(distance, elapsed / 80);
		_bodyFrame = _lastFrame >= _firstFrame ? _firstFrame + offset : _firstFrame - offset;
		if (elapsed >= (distance + 1) * 80)
			advance();
		break;
	}
	case kChapterGesture: {
		const uint distance = ABS((int)_lastFrame - (int)_firstFrame);
		const uint offset = MIN(distance, elapsed / 80);
		_frame = _lastFrame >= _firstFrame ? _firstFrame + offset : _firstFrame - offset;
		if (!_headBank) {
			_bodyFrame = _frame;
		}
		if (elapsed >= (distance + 1) * 80)
			advance();
		break;
	}
	case kMontage: {
		const bool ended = updateMovie();
		const uint cues[] = {73, 215, 357, 499, 642};
		if (_events < ARRAYSIZE(cues) && _frame >= cues[_events] && !speaking()) {
			speak(0, 4 + _events);
			++_events;
		}
		if (ended && !speaking() && _events == ARRAYSIZE(cues)) {
			_video.close();
			advance();
		}
		break;
	}
	case kCity:
		if (elapsed >= 11700)
			advance();
		break;
	case kCityExit:
		if (elapsed >= 3780)
			advance();
		break;
	case kTitle:
	case kAccidentTitle: {
		const uint fadeTime = 74 * (_step == kTitle ? 40 : 20);
		if (_step == kAccidentTitle) {
			if (!(_events & 1) && elapsed >= fadeTime * 2 + 500) {
				_audio.playMusic(17, false);
				_audio.setMusicVolume(255);
				_themeStarted = true;
				_events |= 1;
			}
			if (!(_events & 2) && elapsed >= fadeTime * 2 + 2000) {
				_audio.playEffect(29);
				_events |= 2;
			}
		}
		if (elapsed >= fadeTime * 2 + 2000 + 1480)
			advance();
		break;
	}
	case kAccident:
		if (updateMovie()) {
			if (!_events) {
				_deadline = now + 2000;
				_events = 1;
			}
			if (!_audio.effectPlaying() || (int32)(now - _deadline) >= 0)
				advance();
		}
		break;
	case kAccidentEnd:
	case kApproach:
	case kTurn:
		if (updateMovie())
			advance();
		break;
	case kFilm: {
		const bool ended = updateMovie();
		// Original frame callbacks (0x576d40), independent of rendering rate.
		if (!(_events & 1) && _frame >= 2251) {
			_audio.playMusic(30);
			_audio.setMusicVolume(179);
			_audio.playEffect(41);
			_events |= 1;
		}
		if (!(_events & 2) && _frame >= 2282) {
			speak(1, 0);
			_events |= 2;
		}
		if (!(_events & 4) && _frame >= 2541) {
			speak(1, 1);
			_events |= 4;
		}
		if (ended && !speaking())
			advance();
		break;
	}
	case kFilmEnd:
		if (updateMovie()) {
			advance();
		} else if (!_events && _frame >= 2734) {
			_audio.playEffect(43, 230);
			_events = 1;
		}
		break;
	case kLastNarration:
	case kChapter:
		if (!speaking()) {
			if (!_events) {
				_deadline = now + 500;
				_events = 1;
			}
			if ((int32)(now - _deadline) >= 0)
				advance();
		}
		break;
	default:
		if (!speaking())
			advance();
		break;
	}
}

void RunawayOpening::skip(uint32 now) {
	enter(kFinished, now);
}

void RunawayOpening::skipSpeech(uint32 now) {
	_now = now;
	if (speaking()) {
		nextLine();
	}
}

void RunawayOpening::drawNarrator(Graphics::ManagedSurface &screen) {
	screen.blitFrom(_narratorBackground, Common::Point(0, kRunawayViewportTop));
	_narrator[0].draw(screen, _bodyFrame, 0, kRunawayViewportTop);
	if (_headBank) {
		uint frame = _speech.frame(_now) % 6;
		if (_chapter == 5 && _step == kChapter && _chapterVariation == 2) {
			frame += 10;
		} else if (_chapter == 5 && _step == kChapterGesture && _chapterVariation >= 2) {
			frame = _frame == 20 ? 0 : _frame;
		}
		_narrator[_headBank].draw(screen, frame, 0, kRunawayViewportTop);
	}
	if (_step == kMontage)
		_video.draw(screen, Common::Rect(256, 232, 656, 466));
	if (!_chapterTitle.empty() && (_step == kChapter || _step == kChapterGesture)) {
		applyRunawayPatch(_chapterTitle, screen, kRunawayViewportTop);
	}
}

void RunawayOpening::drawCity(Graphics::ManagedSurface &screen) {
	Graphics::ManagedSurface viewport(screen, Common::Rect(0, kRunawayViewportTop, 1024, kRunawayViewportTop + 600));
	const uint elapsed = _now - _started;
	const int camera = _step == kCity ? 568 - MIN<uint>(240, elapsed > 6900 ? (elapsed - 6900) / 20 : 0) :
		328 - MIN<uint>(328, elapsed / 10);
	viewport.blitFrom(_city, Common::Rect(camera, 0, camera + 1024, 600), Common::Point(0, 0));
	const uint frame = _step == kCity ? 0 : MIN<uint>(212, 48 + elapsed / 80);
	_cityActor.draw(viewport, frame, camera, 0);
	// Native parallax offsets are applied in the room buffer before the
	// viewport is cropped, so its camera displacement also applies here.
	applyRunawayPatch(_parallax[1], viewport, 200, 100 - camera * 2);
	applyRunawayPatch(_parallax[2], viewport, 0, 1100 - camera * 2);
	applyRunawayPatch(_parallax[0], viewport, 0, (620 - camera) * 4 - camera);
	if (_step == kCity && elapsed >= 500 && elapsed < 6900) {
		const uint tick = (elapsed - 500) / 80;
		const uint level = tick < 20 ? tick * 5 : tick < 60 ? 100 : (80 - tick) * 5;
		viewport.transBlitFrom(_cityTitle, Common::Point(879 - camera, 548),
			0xffffffff, false, level * 255 / 100);
	}
}

void RunawayOpening::drawTitles(Graphics::ManagedSurface &screen) {
	const uint elapsed = _now - _started;
	const uint delay = _step == kTitle ? 40 : 20;
	const uint fadeTime = 74 * delay;
	uint first = MIN<uint>(74, elapsed / delay);
	uint second = elapsed > fadeTime + 500 ? MIN<uint>(74, (elapsed - fadeTime - 500) / delay) : 0;
	if (elapsed >= fadeTime * 2 + 2000)
		first = second = 74 - MIN<uint>(74, (elapsed - fadeTime * 2 - 2000) / 20);
	const uint bank = _step == kTitle ? 0 : 2;
	_titles[bank].draw(screen, first, 0, kRunawayViewportTop);
	_titles[bank + 1].draw(screen, second, 0, kRunawayViewportTop);
}

void RunawayOpening::drawFilm(Graphics::ManagedSurface &screen) {
	const Common::Rect viewport(0, kRunawayViewportTop, 1024, kRunawayViewportTop + 600);
	_video.draw(screen, viewport);
	if (_step == kFilm && _frame >= 344 && _frame < 2250) {
		// The original wipes away the right half for the opening credits,
		// then restores the full movie before the accident.
		const uint edge = _frame < 408 ? 1024 - (_frame - 343) * 8 :
			_frame < 2186 ? 512 : MIN<uint>(1024, 512 + (_frame - 2185) * 8);
		screen.fillRect(Common::Rect(edge, viewport.top, 1024, viewport.bottom), 0);
		if (_frame >= 420) {
			const uint elapsed = _frame - 420;
			const uint credit = elapsed / 147, tick = elapsed % 147;
			if (credit < ARRAYSIZE(_credits) && tick < 112)
				_credits[credit].draw(screen, tick < 12 ? tick : tick < 100 ? 11 : 111 - tick, 0, kRunawayViewportTop);
			const uint picture = elapsed / 108;
			const uint alpha = MIN<uint>(255, (elapsed % 108 + 2) * 255 / 20);
			const Common::Point position(627, 140);
			if (picture && picture <= ARRAYSIZE(_pictures))
				screen.blitFrom(_pictures[picture - 1], position);
			if (picture < ARRAYSIZE(_pictures))
				screen.transBlitFrom(_pictures[picture], position, 0xffffffff, false, alpha);
		}
	}
	if (_step == kStayAway || _step == kUnconscious || (_step >= kHospital && _step <= kReply)) {
		const uint portrait = _step == kStayAway ? 0 : _step == kUnconscious ? 1 : 2;
		// The other hitman speaks off screen; the visible man only blinks.
		const uint mouth = _step == kHitman ? ((_now - _started) % 2400 < 160 ? 6 : 0) : _speech.frame(_now);
		if (!_portraits.draw(screen, viewport, portrait, mouth))
			_failed = true;
	}
}

void RunawayOpening::draw(Graphics::ManagedSurface &screen) {
	screen.clear();
	if (_step <= kLastNarration || _step == kChapter || _step == kChapterGesture)
		drawNarrator(screen);
	else if (_step == kCity || _step == kCityExit)
		drawCity(screen);
	else if (_step == kTitle || _step == kAccidentTitle)
		drawTitles(screen);
	else if (_step < kFinished)
		drawFilm(screen);
	const uint elapsed = _now - _started;
	if ((_step == kSitDown || _step == kCity || (_step == kChapter && !_chapterVariation)) && elapsed < 500) {
		dimRunawayViewport(screen, elapsed * 2);
	} else if ((_step == kLastNarration || (_step == kChapter && _chapterVariation + 1 == narrationVariations())) && _events) {
		dimRunawayViewport(screen, MIN<uint>(500, _deadline - _now) * 2);
	} else if (_step == kCityExit && elapsed > 3280) {
		dimRunawayViewport(screen, (3780 - MIN<uint>(3780, elapsed)) * 2);
	}
}

} // End of namespace Hollywood
