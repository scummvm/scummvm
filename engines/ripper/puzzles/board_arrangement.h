/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of their respective copyright holders.
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

#ifndef RIPPER_PUZZLES_BOARD_ARRANGEMENT_H
#define RIPPER_PUZZLES_BOARD_ARRANGEMENT_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/rect.h"

#include "ripper/display.h"
#include "ripper/puzzles/puzzle.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class BoardArrangementPuzzle : public Puzzle {
public:
	explicit BoardArrangementPuzzle(RipperEngine *engine);

	Result run(uint completionFlag) override;

private:
	bool loadAssets();
	bool loadPcx(const Common::String &path, BitmapAssetFrame &frame);
	bool loadPieceSet(const char *prefix, Common::Array<BitmapAssetFrame> &frames);
	void loadPersistentState();
	void storePersistentState() const;
	void drawFrame(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y) const;
	void render() const;
	int findPiece(const Common::Point &point, int *zIndex = nullptr) const;
	bool updateCursor(const Common::Point &point);
	void beginDrag(uint piece, int zIndex);
	void updateDrag(const Common::Point &point);
	void finishDrag();
	void animateInvalidDrop(const Common::Point &target);
	bool isInvalidDrop(const Common::Point &position, const BitmapAssetFrame &frame) const;
	bool isSolved(bool logPairs) const;
	bool complete(uint completionFlag);
	bool isExitPoint(const Common::Point &point) const;
	void bringPieceToFront(uint piece);

	Common::Array<BitmapAssetFrame> _largePieces;
	Common::Array<BitmapAssetFrame> _smallPieces;
	BitmapAssetFrame _template;
	IndexedDisplaySnapshot _incomingDisplay;
	IndexedDisplaySnapshot _boardDisplay;
	Audio::SoundHandle _moveAudioHandle;
	Common::Point _positions[8];
	uint _zOrder[8];
	Common::Point _dragPosition;
	Common::Point _dragOrigin;
	int _draggedPiece;
	int _hoveredPiece;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_BOARD_ARRANGEMENT_H
