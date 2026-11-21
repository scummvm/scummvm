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

#ifndef HOLLYWOOD_RUNAWAY_SCENE2010_H
#define HOLLYWOOD_RUNAWAY_SCENE2010_H

#include "common/random.h"
#include "hollywood/runaway/audio.h"
#include "hollywood/runaway/graphics.h"
#include "hollywood/runaway/speech.h"
#include "hollywood/runaway/video.h"
#include "hollywood/scene_text_store.h"

namespace Hollywood {

// The retail opening (2010) and the interchapter narration.
// All timing comes from the engine's pause-aware clock.
class RunawayOpening {
public:
	explicit RunawayOpening(RunawayAudio &audio) : _audio(audio), _random("runawayOpening"), _speech(audio, _random) {}
	bool load(byte chapter = 0);
	bool loadEndingNarration(byte cue);
	void start(uint32 now);
	void update(uint32 now);
	void skip(uint32 now);
	void skipSpeech(uint32 now);
	void draw(Graphics::ManagedSurface &screen);
	bool finished() const { return _step == kFinished || _failed; }
	bool failed() const { return _failed; }
	const Common::U32String &subtitle() const { return _speech.subtitle(); }

private:
	enum Step {
		kSitDown, kIntroduction, kLeanForward, kQuestion, kLeanBack, kBackground,
		kTurnToScreen, kJourney, kMontage, kTurnBack, kLastNarration,
		kCity, kTitle, kAccident, kAccidentTitle, kAccidentEnd, kCityExit,
		kFilm, kStayAway, kApproach, kUnconscious, kTurn, kHospital,
		kHitman, kReply, kFilmEnd, kChapter, kChapterGesture, kFinished
	};
	void enter(Step step, uint32 now);
	void advance();
	void speak(uint16 cue, byte variation);
	void nextLine();
	void showLine();
	void startMovie(const char *name, uint first, uint last);
	bool updateMovie();
	void drawNarrator(Graphics::ManagedSurface &screen);
	void drawCity(Graphics::ManagedSurface &screen);
	void drawFilm(Graphics::ManagedSurface &screen);
	void drawTitles(Graphics::ManagedSurface &screen);
	uint narrationVariations() const;
	bool speaking() const { return _row != 0; }

	RunawayAudio &_audio;
	Common::RandomSource _random;
	SceneTextStore _text;
	RunawaySpeech _speech;
	RunawayVideo _video;
	Graphics::ManagedSurface _narratorBackground, _city, _cityTitle;
	RunawayFilmPortrait _portraits;
	Graphics::ManagedSurface _pictures[17]; // Sixteen pictures, then a dissolve to black.
	RunawayAnimation _narrator[6], _cityActor, _titles[4], _credits[12];
	Common::Array<byte> _parallax[3], _chapterTitle;
	Step _step = kFinished;
	uint32 _now = 0, _started = 0, _deadline = 0;
	uint _frame = 0, _firstFrame = 0, _lastFrame = 0, _events = 0;
	uint _bodyFrame = 20, _headBank = 1;
	byte _chapter = 1, _chapterVariation = 0;
	byte _narrationCue = 0;
	bool _chapterOnly = false;
	uint16 _row = 0, _lastRow = 0, _sample = 0;
	bool _narrating = true, _themeStarted = false, _failed = false;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE2010_H
