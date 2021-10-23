/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
	byte status2[kPieceCount];

	memcpy(pieceStatus, scriptVariables + 26, kPieceCount);

	int selectedPieces = 0;
	for (int i = 0; i < kPieceCount; i++) {
		status1[i] = 0;
		if (pieceStatus[i] == kPieceSelected) {
			for (int j = 0; j < kPieceCount; j++)
				status2[j] = pieceStatus[j];

			byte curLink = kGalleryLinks[i][0];
			pieceStatus[i] = kPieceUnselected;
			status2[i] = 0;

			int linkedPiece = 1;
			while (curLink != 0) {
				linkedPiece++;
				status2[curLink - 1] = kPieceUnselected;
				curLink = kGalleryLinks[i][linkedPiece - 1];
			}
			status1[i] = galleryAI(status2, 1);
			if (status1[i] == kPieceSelected) {
				selectedPieces++;
			}
		}
	}

	if (selectedPieces == 0) {
		int esi = 0;
		for (int i = 0; i < kPieceCount; i++) {
			if (esi < status1[i]) {
				esi = status1[i];
			}
		}

		if (esi == 2) {
			esi = 1;
		} else {
			if (esi < kPieceCount) {
				esi = 2;
			} else {
				esi -= 12;
			}
		}

		for (int i = 0; i < kPieceCount; i++) {
			if (esi < status1[i]) {
				status1[i] = kPieceSelected;
				selectedPieces++;
			}
		}
	}

	int selectedPiece = 0;

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

			byte curLink = kGalleryLinks[i][0];
			status2[i] = 0;
			selectedPieces = 1;

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
			byte v12 = status2[j];
			if (v12) {
				++v10;
				if (v12 == 1)
					++v9;
				else
					v8 += v12;
			}
		}
		if (v9 == v10)
			return 1;
		else
			return (v8 + 102 * v9) / v10;
	}

	return depth == 0 ? 2 : 1;
}

} // End of Groovie namespace
