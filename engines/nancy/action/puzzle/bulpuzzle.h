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

#ifndef NANCY_ACTION_BULPUZZLE_H
#define NANCY_ACTION_BULPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// A puzzle based around a simplified version of the Mayan game Bul
class BulPuzzle : public RenderActionRecord {
public:
	BulPuzzle() : RenderActionRecord(7) {}
	virtual ~BulPuzzle() {}

	void init() override;
	void updateGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	enum BulAction { kNone, kRoll, kPass, kReset, kCapture };

	void movePiece(bool player);
	void reset(bool capture);

	Common::String getRecordTypeName() const override { return "BulPuzzle"; }
	bool isViewportRelative() const override { return true; }

	Common::Path _imageName;

	uint16 _numCells = 0;
	uint16 _numPieces = 0;
	uint16 _numRolls = 0;

	uint16 _playerStart = 0;
	uint16 _enemyStart = 0;

	Common::Array<Common::Rect> _diceDestsPlayer;
	Common::Array<Common::Rect> _diceDestsEnemy;

	Common::Array<Common::Rect> _cellDests;

	Common::Array<Common::Rect> _playerBarracksDests;
	Common::Array<Common::Rect> _playerJailDests;

	Common::Array<Common::Rect> _enemyBarracksDests;
	Common::Array<Common::Rect> _enemyJailDests;

	Common::Rect _rollButtonDest;
	Common::Rect _passButtonDest;
	Common::Rect _resetButtonDest;
	Common::Rect _playerLightDest;
	Common::Rect _enemyLightDest;

	Common::Array<Common::Rect> _diceBlackSrcs;
	Common::Array<Common::Rect> _diceCleanSrcs;

	Common::Rect _playerSrc;
	Common::Rect _enemySrc;
	Common::Rect _playerCapturedSrc;
	Common::Rect _enemyCapturedSrc;

	Common::Rect _playerBarracksSrc;
	Common::Rect _enemyBarracksSrc;
	Common::Rect _playerJailSrc;
	Common::Rect _enemyJailSrc;

	Common::Rect _rollButtonSrc;
	Common::Rect _passButtonSrc;
	Common::Rect _passButtonDisabledSrc;
	Common::Rect _resetButtonSrc;
	Common::Rect _playerLightSrc;
	Common::Rect _enemyLightSrc;

	SoundDescription _moveSound;
	SoundDescription _playerCapturedSound;
	SoundDescription _enemyCapturedSound;
	SoundDescription _rollSound;
	SoundDescription _passSound;
	SoundDescription _resetSound;

	SceneChangeWithFlag _solveScene;
	uint16 _solveSoundDelay = 0;
	SoundDescription _solveSound;

	SceneChangeWithFlag _exitScene; // also when losing
	uint16 _loseSoundDelay = 0;
	SoundDescription _loseSound;
	Common::Rect _exitHotspot;

	Graphics::ManagedSurface _image;

	int16 _playerPos = 0;
	int16 _playerPieces = 0;
	int16 _enemyPos = 0;
	int16 _enemyPieces = 0;

	uint16 _turn = 0;
	uint16 _moveDiff = 0;
	uint32 _nextMoveTime = 0;
	bool _pushedButton = false;
	bool _changeLight = false;
	BulAction _currentAction = kNone;

	bool _playerWon = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_BULPUZZLE_H
