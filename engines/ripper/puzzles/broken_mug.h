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

#ifndef RIPPER_PUZZLES_BROKEN_MUG_H
#define RIPPER_PUZZLES_BROKEN_MUG_H

#include "common/array.h"
#include "common/rect.h"

namespace Ripper {

class RipperEngine;

class BrokenMugPuzzle {
public:
	enum Result {
		kExited,
		kSolved,
		kLoadFailed
	};

	explicit BrokenMugPuzzle(RipperEngine *engine);

	Result run();
	static bool playCompletionMedia(RipperEngine *engine);

private:
	struct Frame {
		uint16 width;
		uint16 height;
		Common::Array<byte> pixels;
		Common::Array<byte> palette;

		Frame() : width(0), height(0) {}
	};

	struct Piece {
		Common::Array<Frame> frames;
		Common::Point position;
		uint orientation;
	};

	bool loadPieces();
	bool loadPiece(uint pieceIndex);
	bool captureBackground();
	void restoreBackground() const;
	void render();
	void drawPiece(byte *screen, uint pitch, const Piece &piece, const Frame &frame) const;
	int findPiece(const Common::Point &point) const;
	void beginDrag(uint pieceIndex, const Common::Point &point);
	void updateDrag(const Common::Point &point);
	void beginRotation();
	bool updateRotation(uint32 now);
	void finishDrag();
	bool isSolved() const;
	bool completePuzzle();
	const Frame &currentFrame(const Piece &piece) const;

	RipperEngine *_engine;
	Piece _pieces[9];
	Common::Array<uint> _frontToBack;
	Common::Array<byte> _backgroundPixels;
	Common::Array<byte> _backgroundPalette;
	Common::Array<byte> _activePuzzlePalette;
	Common::Point _dragOffset;
	int _draggedPiece;
	int _rotationStep;
	uint32 _nextRotationMillis;
	bool _dropPending;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_BROKEN_MUG_H
