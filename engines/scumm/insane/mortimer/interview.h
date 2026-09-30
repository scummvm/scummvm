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
#include "graphics/font.h"
#include "scumm/insane/mortimer/movie.h"

namespace Graphics {
struct Surface;
}

namespace Scumm {

class MortimerFont : public Graphics::Font {
public:
	bool load(const char *path);
	int getFontHeight() const override { return _height; }
	int getMaxCharWidth() const override { return _width; }
	int getCharWidth(uint32 chr) const override;
	void drawChar(Graphics::Surface *surface, uint32 chr, int x, int y, uint32 color) const override;

private:
	Common::Array<MortimerSprite> _glyphs;
	int _height = 0, _width = 0;
};

// The native interview controller in SHELL 40a884, including its riddle
// callbacks. Movie requests are consumed by the engine after stopping audio.
class MortimerInterview {
public:
	enum Effect { kNoEffect, kButtonEffect, kPayoutEffect, kSwitchEffect };

	MortimerInterview();
	bool loadResources();
	void resetRiddle();
	void start(uint species, uint stones, int scene = 10);
	void click(const Common::Point &point);
	void scroll(int direction);
	void update();
	void movieFinished();
	void movieLoaded() { _moviePending = false; }
	void skip();
	void draw(Graphics::Surface &screen, const MortimerMovie &movie) const;
	void drawCursor(Graphics::Surface &screen, const Common::Point &point) const;

	bool needsMovie() const { return _moviePending; }
	bool playingMovie() const { return !_movieName.empty(); }
	bool fullScreen() const;
	bool showDashboard() const { return !fullScreen() || _stage == kPopup; }
	bool movieDoubled() const;
	bool wantsFlight() const { return _stage == kFlyAway; }
	bool finished() const { return _stage == kFinished; }
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
		kNoStones, kPay, kGuess, kCorrect, kWrong, kOpenGate,
		kEnding, kHelp, kFlyAway, kFinished
	};

	void enter(Stage stage);
	void selectAnimal(int animal);
	int hitControl(const Common::Point &point) const;
	bool loadText(const char *path, bool riddles);
	void drawText(Graphics::Surface &screen, const Common::String &text) const;
	Common::String animalMovie(const char *pattern, int animal) const;

	Common::RandomSource _random;
	Common::Array<MortimerSprite> _panel, _portraits, _gate, _cursor;
	MortimerFont _font, _captionFont;
	Common::String _dialogue[7][3], _riddles[7], _captions[16];
	Common::String _movieName, _error;
	byte _palette[768], _gatePalette[768];
	Stage _stage;
	Effect _effect;
	int _answer, _selected, _question, _guess, _lastAction, _textScroll;
	uint _species, _stones, _guesses, _frame, _switchFrame, _payment;
	bool _moviePending, _riddleView, _solving, _help, _helpExplained;
};

} // End of namespace Scumm

#endif
