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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SCUMM_INSANE_MORTIMER_INTERVIEW_H
#define SCUMM_INSANE_MORTIMER_INTERVIEW_H

#include "common/random.h"
#include "scumm/insane/mortimer/text.h"

namespace Graphics {
struct Surface;
}

namespace Scumm {

// The native interview controller in SHELL 40a884, including its riddle
// callbacks. Movie requests are consumed by the engine after stopping audio.
class MortimerInterview {
public:
	enum Effect { kNoEffect, kButtonEffect, kPayoutEffect, kSwitchEffect };

	explicit MortimerInterview(bool isDemo = true);
	bool loadResources(uint region = 0, bool programmer = false);
	void setDifficulty(uint difficulty) { _difficulty = difficulty; }
	void resetRiddle();
	void resetAttempts();
	void start(uint species, uint stones, int scene = 10);
	void rescueAll() { _species = 0x7f; _selected = 0; }
	void click(const Common::Point &point);
	void scroll(int direction);
	void update();
	void movieFinished();
	void movieLoaded() { _moviePending = false; }
	void skip();
	void draw(Graphics::Surface &screen, const MortimerMovie &movie, byte mouth) const;
	void drawCursor(Graphics::Surface &screen, const Common::Point &point) const;

	bool needsMovie() const { return _moviePending; }
	bool playingMovie() const { return !_movieName.empty(); }
	bool fullScreen() const;
	bool showDashboard() const { return !fullScreen() || _stage == kPopup; }
	bool forceSubtitles() const { return _stage == kSong; }
	bool movieDoubled() const;
	bool wantsFlight() const { return _stage == kFlyAway; }
	bool finished() const { return _stage == kFinished; }
	bool solved() const { return _stage == kCorrect || _stage == kBonus || _stage == kOpenGate || _stage == kFinished; }
	bool payingBonus() const { return _stage == kBonus; }
	bool bonusFinished() const { return _stage == kOpenGate || _stage == kFinished; }
	uint takeBonusStones();
	bool restartsRegion() const { return _restartRegion; }
	int sceneId() const;
	int riddleId() const { return _answer * 10 + _textDifficulty; }
	bool outOfGuesses() const { return _guesses == 0; }
	const Common::String &movieName() const { return _movieName; }
	const Common::String &errorMessage() const { return _error; }
	const byte *palette(const MortimerMovie &movie) const;
	uint stones() const { return _stones; }
	uint species() const { return _species; }
	int highlightedAnimal() const;
	Effect takeEffect();

private:
	enum Stage {
		kGate, kSong, kPopup, kIdle, kAnswer, kRiddle, kSolve,
		kNoStones, kPay, kGuess, kCorrect, kBonus, kWrong, kOpenGate,
		kEnding, kHelp, kFlyAway, kFinished, kCost, kNoAnimals, kTried
	};

	void enter(Stage stage);
	void selectAnimal(int animal);
	int hitControl(const Common::Point &point) const;
	bool loadText(const char *path, bool riddles);
	void drawText(Graphics::Surface &screen, const Common::String &text) const;
	Common::String animalMovie(const char *pattern, int animal) const;

	Common::RandomSource _random;
	const bool _isDemo;
	uint _region, _difficulty, _textDifficulty, _tried;
	Common::Array<MortimerSprite> _panel, _portraits, _gate, _cursor, _mouth;
	MortimerFont _font, _captionFont;
	Common::String _dialogue[7][3], _riddles[7][3], _captions[16];
	Common::String _movieName, _error;
	byte _palette[768], _gatePalette[768];
	Stage _stage;
	Effect _effect;
	int _answer, _previousAnswer, _selected, _question, _guess, _lastAction, _textScroll, _guesses;
	uint _species, _stones, _frame, _switchFrame, _payment, _bonusStones;
	bool _moviePending, _riddleView, _solving, _help, _helpExplained, _restartRegion, _programmer;
};

} // End of namespace Scumm

#endif
