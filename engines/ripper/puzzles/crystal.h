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

#ifndef RIPPER_PUZZLES_CRYSTAL_H
#define RIPPER_PUZZLES_CRYSTAL_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/random.h"
#include "common/rect.h"

#include "ripper/display.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class CrystalPuzzle {
public:
	enum Result {
		kExited,
		kSolved,
		kLoadFailed
	};

	explicit CrystalPuzzle(RipperEngine *engine);

	Result run(uint completionFlag);

private:
	bool captureBackground();
	void restoreBackground() const;
	bool loadAssets();
	bool loadPieceSet(const char *prefix, Common::Array<BitmapAssetFrame> &frames);
	void render();
	void drawFrame(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y) const;
	int findTrayPiece(const Common::Point &point) const;
	int findGridCell(const Common::Point &point) const;
	int findPlacedPiece(const Common::Point &point) const;
	void beginDrag(uint piece, const Common::Point &point);
	void placeDraggedPiece(uint cell);
	void returnDraggedPieceToTray();
	bool isSolved() const;
	bool complete(uint completionFlag);
	uint readPuzzleLevel() const;
	void seedInitialPieces(uint puzzleLevel);
	void playMovementCue();
	void updateCursor(const Common::Point &point);
	Common::Point trayPosition(uint piece) const;
	Common::Point gridPosition(uint cell, const BitmapAssetFrame &frame) const;

	RipperEngine *_engine;
	Common::Array<BitmapAssetFrame> _placedFrames;
	Common::Array<BitmapAssetFrame> _trayFrames;
	Common::Array<BitmapAssetFrame> _blankingFrames;
	IndexedDisplaySnapshot _backgroundDisplay;
	Common::Array<byte> _puzzlePalette;
	Audio::SoundHandle _audioHandles[4];
	Common::RandomSource _random;
	Common::Point _dragPoint;
	int _pieceCells[16];
	int _cellPieces[72];
	int _draggedPiece;
	int _hoveredPiece;
	uint _keywordIndex;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_CRYSTAL_H
