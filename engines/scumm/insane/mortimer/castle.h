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

#ifndef SCUMM_INSANE_MORTIMER_CASTLE_H
#define SCUMM_INSANE_MORTIMER_CASTLE_H

#include "common/random.h"
#include "scumm/insane/mortimer/flight.h"
#include "scumm/insane/mortimer/text.h"

namespace Scumm {

class MortimerCastle : public MortimerFlight {
public:
	MortimerCastle();
	bool start(int scene, uint flightDifficulty, uint riddleDifficulty, bool girl);
	bool loadMovie();
	void update();
	void click(const Common::Point &point);
	bool skip();
	void movieFinished();
	void hit();
	void drawOverlay(Graphics::Surface &screen, const Common::Point &mouse, const Common::Point &origin, byte mouth) const;
	const MortimerSound *takeSpeech(bool playing);

	bool needsMovie() const { return _moviePending; }
	bool courseView() const { return _stage <= kExit && interactive(); }
	bool acceptsShots() const { return (_stage == kEntry || _stage == kRoom) && !_choosing && !_leaving && !_holdFrames; }
	bool choosing() const { return _choosing; }
	bool paused() const { return _choosing || _stage == kIdle || _holdFrames; }
	bool finished() const { return _stage == kFinished; }
	bool hasSpeech() const { return _nextVoice < _voices.size(); }
	bool forceSubtitles() const { return _stage == kSong; }
	uint damage() const { return _damage; }
	int sceneId() const;
	const char *subtitleName() const;
	const Common::String &movieName() const { return _movieName; }
	const Common::String &errorMessage() const { return _error.empty() ? MortimerMovie::errorMessage() : _error; }

protected:
	void handleAction(const byte *data, uint32 size) override;

private:
	enum Stage { kEntry, kRoom, kLeft, kDrop, kExit, kGate, kSong, kIdle, kRiddle, kWrong, kCorrect, kFinished };
	struct SpeechBlock {
		Common::Array<byte> data;
		uint next = 0, count = 0, size = 0;
	};

	bool loadResources();
	bool loadText();
	bool newPuzzle();
	void enter(Stage stage);
	void retry(bool crash);
	void resolveChoice();
	void showChoices();
	int animal(uint side) const;
	void drawText(Graphics::Surface &screen, const Common::String &text, int x, int y, int width, byte color) const;

	Common::RandomSource _random;
	Common::Array<MortimerSprite> _goggles, _icons, _spin, _flames[4], _plaques[28], _mouth, _cursor;
	MortimerFont _font, _captionFont;
	Common::String _riddles[2][2], _hint;
	Common::String _movieName, _error;
	SpeechBlock _speechBlocks[2];
	MortimerSound _speech[2], _hintSound, _explanation;
	Common::Array<uint> _voices;
	uint _nextVoice, _flightDifficulty, _riddleDifficulty, _puzzle, _step, _damage, _checkpointDamage;
	uint _wrongChoices, _crashes, _frame, _choiceFrame, _hintFrames, _holdFrames;
	uint _rooms[5], _sides[6];
	int _answer, _selection;
	Stage _stage;
	bool _moviePending, _choosing, _leaving, _correct, _resourcesLoaded;
};

} // End of namespace Scumm

#endif
