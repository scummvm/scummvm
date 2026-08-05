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

#ifndef RIPPER_PUZZLES_BOARD_GAME_H
#define RIPPER_PUZZLES_BOARD_GAME_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/random.h"
#include "common/rect.h"

#include "ripper/display.h"
#include "ripper/puzzles/board_game_model.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class BoardGamePuzzle {
public:
	enum Result {
		kExited,
		kSolved,
		kLoadFailed
	};

	explicit BoardGamePuzzle(RipperEngine *engine);

	Result run(uint completionFlag);

private:
	bool loadAssets();
	bool loadBitmap(const Common::String &name, BitmapAssetFrame &frame);
	bool loadSelectionShading();
	void applyPalette();
	void drawFrame(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y, bool translucent = false) const;
	byte blendSelectionPixel(byte source, byte destination) const;
	void drawPiece(byte *screen, uint pitch, int piece,
		const Common::Point &anchor, bool selected = false) const;
	void render();
	int hitCodeAt(const Common::Point &point) const;
	void updateCursor(const Common::Point &point);
	bool serviceKeyword(uint16 command);
	bool applyPlayerMove(int destination);
	bool runAiTurn();
	bool animateMove(const BoardGameModel::Move &move, int movingPiece,
		int capturedPiece, bool thinkingCursor);
	Result finishResult(uint completionFlag);
	BoardGameModel::Move chooseAiMove();
	int evaluatePosition(const BoardGameModel &model, int depth,
		int alpha, int beta) const;
	int staticScore(const BoardGameModel &model) const;
	bool playCue(uint cue);
	void stopCue(uint cue);

	RipperEngine *_engine;
	AssetLibrary _library;
	BitmapAssetFrame _background;
	BitmapAssetFrame _hitMap;
	BitmapAssetFrame _pieces[2][5];
	Common::Array<byte> _selectionShading;
	IndexedDisplaySnapshot _incomingDisplay;
	BoardGameModel _model;
	Common::RandomSource _random;
	Audio::SoundHandle _audioHandles[5];
	Common::Array<int> _legalDestinations;
	Common::Point _movingAnchor;
	int _selectedCell;
	int _hoveredCode;
	int _movingSource;
	int _movingPiece;
	uint _keywordIndex;
	uint _searchDepth;
	uint _savedSelectionIndex;
	bool _savedCursorVisible;
	bool _movingActive;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_BOARD_GAME_H
