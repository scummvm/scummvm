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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * This file is dual-licensed.
 * In addition to the GPLv2 license mentioned above, MojoTouch has exclusively licensed
 * this code on November 10th, 2021, to be use in closed-source products.
 * Therefore, any contributions (commits) to it will also be dual-licensed.
 *
 */

#include "groovie/groovie.h"
#include "groovie/logic/gallery.h"

namespace Groovie {

// Links between the pieces in the Gallery challenge
// For example, the first row signifies that piece 1
// is connected to pieces 2, 4 and 5
const byte GalleryGame::kGalleryLinks[21][10] = {
	{ 2,  4,  5,  0,  0,  0,  0,  0,  0,  0 },	//  1
	{ 1,  5,  3,  0,  0,  0,  0,  0,  0,  0 },	//  2
	{ 2,  5,  9, 12,  0,  0,  0,  0,  0,  0 },	//  3
	{ 1,  5,  6,  7,  8,  0,  0,  0,  0,  0 },	//  4
	{ 1,  2,  3,  4,  7,  8,  9,  0,  0,  0 },	//  5
	{ 4,  7, 10, 11, 13, 14, 15, 16, 18,  0 },	//  6
	{ 4,  5,  6,  8,  9, 10,  0,  0,  0,  0 },	//  7
	{ 4,  5,  7,  0,  0,  0,  0,  0,  0,  0 },	//  8
	{ 3,  5,  7, 10, 11, 12, 18,  0,  0,  0 },	//  9
	{ 6,  7,  9, 11,  0,  0,  0,  0,  0,  0 },	// 10
	{ 6,  9, 10, 18,  0,  0,  0,  0,  0,  0 },	// 11
	{ 3,  9, 18, 21,  0,  0,  0,  0,  0,  0 },	// 12
	{ 6, 14, 17, 19,  0,  0,  0,  0,  0,  0 },	// 13
	{ 6, 13, 15, 17, 19, 20, 21,  0,  0,  0 },	// 14
	{ 6, 14, 16, 18, 21,  0,  0,  0,  0,  0 },	// 15
	{ 6, 15,  0,  0,  0,  0,  0,  0,  0,  0 },	// 16
	{13, 14, 19,  0,  0,  0,  0,  0,  0,  0 },	// 17
	{ 6,  9, 11, 12, 15, 21,  0,  0,  0,  0 },	// 18
	{13, 14, 17, 20,  0,  0,  0,  0,  0,  0 },	// 19
	{14, 19, 21,  0,  0,  0,  0,  0,  0,  0 },	// 20
	{12, 14, 15, 18, 20,  0,  0,  0,  0,  0 }	// 21
};

const int kPieceCount = 21;
enum kGalleryPieceStatus {
	kPieceUnselected = 0,
	kPieceSelected = 1
};

void GalleryGame::run(byte *scriptVariables) {
	byte pieceStatus[kPieceCount];
	byte status1[kPieceCount];

	memcpy(pieceStatus, scriptVariables + 26, kPieceCount);

	int selectedPieces = 0;
	for (int i = 0; i < kPieceCount; i++) {
		status1[i] = 0;
		// in this context it seems like kPieceSelected means it's available for selection
		if (pieceStatus[i] == kPieceSelected) {
			byte status2[kPieceCount];
			for (int j = 0; j < kPieceCount; j++)
				status2[j] = pieceStatus[j];

			status2[i] = 0;

			byte curLink = kGalleryLinks[i][0];
			int linkedPiece = 1;
			while (curLink != 0) {
				linkedPiece++;
				status2[curLink - 1] = kPieceUnselected;
				curLink = kGalleryLinks[i][linkedPiece - 1];
			}
			status1[i] = galleryAI(status2, 1);
			// in this context, kPieceSelected means we think it's an optimal move
			if (status1[i] == kPieceSelected) {
				selectedPieces++;
			}
		}
	}

	if (selectedPieces == 0) {
		// optimal move not found, choose a move with a high score?
		int highestScore = 0;
		for (int i = 0; i < kPieceCount; i++) {
			if (highestScore < status1[i]) {
				highestScore = status1[i];
			}
		}

		if (highestScore == 2) {
			highestScore = 1;
		} else {
			if (highestScore < kPieceCount) {
				highestScore = 2;
			} else {
				highestScore -= 12;
			}
		}

		for (int i = 0; i < kPieceCount; i++) {
			if (highestScore < status1[i]) {
				status1[i] = kPieceSelected;
				selectedPieces++;
			}
		}
	}

	int selectedPiece = 0;

	// var 49 is set by the script calling o_random
	// choose one of our good moves
	byte v12 = scriptVariables[49] % selectedPieces;
	for (int i = 0; i < kPieceCount; i++) {
		if (status1[selectedPiece] == 1 && !v12--)
			break;

		selectedPiece++;
	}

	scriptVariables[47] = (selectedPiece + 1) / 10;
	scriptVariables[48] = (selectedPiece + 1) % 10;
}

byte GalleryGame::galleryAI(byte *pieceStatus, int depth) {
	byte status1[kPieceCount];
	byte status2[kPieceCount];

	int selectedPieces = 0;

	for (int i = 0; i < kPieceCount; i++) {
		status1[i] = 0;
		if (pieceStatus[i] == kPieceSelected) {
			for (int j = 0; j < kPieceCount; j++)
				status2[j] = pieceStatus[j];

			status2[i] = 0;
			selectedPieces = 1;

			byte curLink = kGalleryLinks[i][0];
			int linkedPiece = 1;
			while (curLink != 0) {
				linkedPiece++;
				status2[curLink - 1] = kPieceUnselected;
				curLink = kGalleryLinks[i][linkedPiece - 1];
			}
			status1[i] = galleryAI(status2, depth == 0 ? 1 : 0);
			if (!depth && status1[i] == kPieceSelected) {
				return 1;
			}
		}
	}

	if (selectedPieces) {
		byte v8 = 0;
		byte v9 = 0;
		byte v10 = 0;
		for (int j = 0; j < 21; ++j) {
			byte v12 = status1[j];
			if (v12) {
				++v10;
				if (v12 == 1)
					++v9;
				else
					v8 += v12;
			}
		}
		if (v9 == v10)
			return 1; // I believe 1 means this is an optimal move
		else if (_easierAi && v9 * 3 >= v10 * 2)
			return 1; // close enough to an optimal move?
		else
			return (v8 + 102 * v9) / v10;// otherwise, higher numbers are better
	}

	return depth == 0 ? 2 : 1;
}


void GalleryGame::testsWriteMove(int move, byte pieceStatus[kPieceCount]) {
	if (pieceStatus[move] != kPieceSelected)
		error("illegal move to %d", move + 1);

	pieceStatus[move] = kPieceUnselected;
	for (int i = 0; i < 10; i++) {
		byte curLink = kGalleryLinks[move][i];
		if (!curLink)
			break;
		pieceStatus[curLink - 1] = kPieceUnselected;
	}
}

void GalleryGame::ensureSamanthaWins(int seed) {
	byte scriptVariables[1024];
	byte goalPieceStatus[kPieceCount];
	memset(goalPieceStatus, 0, sizeof(goalPieceStatus));
	Common::RandomSource rng("ensureSamanthaWins");

	rng.setSeed(seed);
	warning("starting ensureSamanthaWins with seed %u", seed);

	memset(scriptVariables, 1, sizeof(scriptVariables));

	for (int i = 0; i < 100; i++) {
		bool isStauf = i % 2;
		scriptVariables[49] = rng.getRandomNumber(14);

		run(scriptVariables);

		int selectedMove = scriptVariables[47] * 10 + scriptVariables[48] - 1;
		warning("Move %d: %s moved to %d", i, (isStauf ? "Stauf" : "Samantha"), selectedMove + 1);

		testsWriteMove(selectedMove, scriptVariables + 26);

		if (memcmp(scriptVariables + 26, goalPieceStatus, sizeof(goalPieceStatus)) == 0) {
			if (isStauf)
				error("Stauf won");
			else
				warning("Samantha won");

			return;
		}
	}
	error("game took too long");
}

void GalleryGame::test() {
	warning("running gallery tests");
	for (int i = 0; i < 20; i++) {
		ensureSamanthaWins(i);
	}
	warning("finished running gallery tests");
}

} // End of Groovie namespace
