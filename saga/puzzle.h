/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SAGA_PUZZLE_H_
#define SAGA_PUZZLE_H_

namespace Saga {

#define PUZZLE_PIECES 15

class Puzzle {
private:
	enum kRQStates {
		kRQNoHint = 0,
		kRQHintRequested = 1,
		kRQHintRequestedStage2 = 2,
		kRQSakkaDenies = 3,
		kRQSkipEverything = 4
	};

	SagaEngine *_vm;

	bool _solved;
	bool _active;
	bool _newPuzzle;
	bool _sliding;

	kRQStates _hintRqState;
	int _hintGiver;
	int _hintOffer;
	int _hintCount;
	int _helpCount;

	int _puzzlePiece;
	int _piecePriority[PUZZLE_PIECES];

	int _lang;

public:
	Puzzle(SagaEngine *vm);

	void execute(void);
	void exitPuzzle(void);

	bool isSolved(void) { return _solved; }
	bool isActive(void) { return _active; }

	void handleReply(int reply);
	void handleClick(Point mousePt);

	void movePiece(Point mousePt);

private:
	void initPieceInfo(int i, int16 curX, int16 curY, byte offX, byte offY, int16 trgX, 
					   int16 trgY, uint8 flag, uint8 count, Point point0, Point point1,
					   Point point2, Point point3, Point point4, Point point5);

	static void hintTimerCallback(void *refCon);

	void solicitHint(void);

	void initPieces(void);
	void showPieces(void);
	void slidePiece(int x1, int y1, int x2, int y2);
	void dropPiece(Point mousePt);
	void alterPiecePriority(void);
	void drawCurrentPiece(void);

	void giveHint(void);
	void clearHint(void);

private:
	struct PieceInfo {
		int16 curX;
		int16 curY;
		byte offX;
		byte offY;
		int16 trgX;
		int16 trgY;
		uint8 flag;
		uint8 count;
		Point point[6];
	};

	PieceInfo _pieceInfo[PUZZLE_PIECES];
	int _slidePointX, _slidePointY;
	Rect _hintBox;
};

} // End of namespace Saga

#endif
