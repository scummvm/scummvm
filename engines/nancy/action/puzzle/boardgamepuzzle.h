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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_ACTION_BOARDGAMEPUZZLE_H
#define NANCY_ACTION_BOARDGAMEPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/commontypes.h"
#include "engines/nancy/movieplayer.h"

namespace Nancy {
namespace Action {

// Movie-driven board game introduced in Nancy12 (AR 164) - the "Mother Clock"
// puzzle. The board states are pre-rendered frames of a Bink movie the game
// navigates between as moves are made.
class BoardGamePuzzle : public RenderActionRecord {
public:
	BoardGamePuzzle() : RenderActionRecord(7) {}
	virtual ~BoardGamePuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "BoardGamePuzzle"; }

	static const uint kNumSounds = 6;
	static const uint kNumButtons = 10;
	static const uint kNumMoves = 12;

	// One turn's scripted move: advance by _amount; landing on _jumpFrom warps
	// to _jumpTo playing movie frames [_jumpFrameStart, _jumpFrameEnd].
	struct MoveRecord {
		int16 amount = 0;
		int16 jumpFrom = 0;
		int16 jumpTo = 0;
		int16 jumpFrameStart = 0;
		int16 jumpFrameEnd = 0;
	};

	// File data
	Common::Path _imageName;
	Common::Path _movieName;		// the board-game Bink movie ("..._ANIM")
	Common::Rect _movieRect;		// movie source/params rect (buf+0x244)
	Common::Rect _boardRect;		// board area within the viewport (buf+0x254)

	Common::Array<Common::Rect> _cardBlueSrcs;	// available-card sprites in _image (buf+0x23)
	Common::Array<Common::Rect> _cardWhiteSrcs;	// active-card sprites in _image (buf+0xc3)
	Common::Array<Common::Rect> _buttonRects;	// 10 on-screen card positions (buf+0x163)
	Common::Rect _resetPressedSrc;				// pressed reset sprite in _image (buf+0x203)
	Common::Rect _resetButtonRect;				// on-screen reset position (buf+0x213)

	int16 _framesPerPosition = 0;	// track position N shows at movie frame N*this+1
	int16 _winTarget = 0;			// land here exactly to win; overshoot loses
	Common::Array<MoveRecord> _moves;	// 12 scripted turn records (buf+0x26a)

	SceneChangeWithFlag _winScene;	// buf+0x2e2
	SceneChangeWithFlag _loseScene;	// buf+0x2fb (also the exit/quit target)

	RandomSoundBlock _sounds[kNumSounds];	// button / click / clank / slide / key / beep

	// Named indices into _sounds (best-guess mapping from the sound names).
	enum SoundIndex {
		kButtonSound = 0,	// ButtonDownUp02 - a move button is pressed
		kResetSound = 1,	// Click_MetalButton02 - the reset button
		kLandSound = 2,		// Clank_Metal01 - token settles on a square
		kSlideSound = 3,	// MetalSlide09 - token slides during a move
		kWinSound = 4,		// Key_Turn_Double - reached the target
		kLoseSound = 5		// Beep01 - overshot the target
	};

	// Runtime state
	enum BoardState { kBoardWaiting, kBoardAnimating, kBoardResult };

	MoviePlayer _moviePlayer;
	int16 _position = 0;				// current track position (0.._winTarget)
	Common::Array<bool> _buttonUsed;	// a move button may be pressed once
	int16 _activeCard = -1;				// card currently being played (white sprite)
	bool _resetPressed = false;			// reset button showing its pressed sprite
	uint32 _resetPressedTime = 0;
	BoardState _boardState = kBoardWaiting;
	bool _solved = false;				// reached the target exactly
	bool _lost = false;					// overshot the target
	uint32 _resultTime = 0;				// ms timestamp when the game ended, for the result hold

	Graphics::ManagedSurface _image;

	void redraw();
	void drawCard(uint index);
	void resolveMove(int button);
	void playSoundBlock(uint index);
	int framePosition(int position) const { return _framesPerPosition * position + 1; }
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_BOARDGAMEPUZZLE_H
