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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "drascula/drascula.h"

namespace Drascula {

void DrasculaEngine::placeIgor() {
	int pos_igor[6] = { 1, 0, igorX, igorY, 54, 61 };

	if (currentChapter == 4) {
		pos_igor[1] = 138;
	} else {
		if (trackIgor == 3)
			pos_igor[1] = 138;
		else if (trackIgor == 1)
			pos_igor[1] = 76;
	}

	copyRectClip(pos_igor, frontSurface, screenSurface);
}

void DrasculaEngine::placeDrascula() {
	int pos_dr[6] = { 0, 122, drasculaX, drasculaY, 45, 77 };

	if (trackDrascula == 1)
		pos_dr[0] = 47;
	else if (trackDrascula == 0)
		pos_dr[0] = 1;
	else if (trackDrascula == 3 && currentChapter == 1)
		pos_dr[0] = 93;

	if (currentChapter == 6)
		copyRectClip(pos_dr, drawSurface2, screenSurface);
	else
		copyRectClip(pos_dr, backSurface, screenSurface);
}

void DrasculaEngine::placeBJ() {
	int pos_bj[6] = { 0, 99, bjX, bjY, 26, 76 };

	if (trackBJ == 3)
		pos_bj[0] = 10;
	else if (trackBJ == 0)
		pos_bj[0] = 37;

	copyRectClip(pos_bj, drawSurface3, screenSurface);
}

void DrasculaEngine::hiccup(int counter) {
	int y = 0, trackCharacter = 0;
	if (currentChapter == 3)
		y = -1;

	do {
		counter--;

		updateRoom();
		if (currentChapter == 3)
			updateScreen(0, 0, 0, y, 320, 200, screenSurface);
		else
			updateScreen(0, 1, 0, y, 320, 198, screenSurface);

		if (trackCharacter == 0)
			y++;
		else
			y--;

		if (currentChapter == 3) {
			if (y == 1)
				trackCharacter = 1;
			if (y == -1)
				trackCharacter = 0;
		} else {
			if (y == 2)
				trackCharacter = 1;
			if (y == 0)
				trackCharacter = 0;
		}
	} while (counter > 0);

	updateRoom();
	updateScreen();
}

void DrasculaEngine::startWalking() {
	characterMoved = 1;

	stepX = STEP_X;
	stepY = STEP_Y;

	if (currentChapter == 2) {
		if ((roomX < curX) && (roomY <= (curY + curHeight)))
			quadrant_1();
		else if ((roomX < curX) && (roomY > (curY + curHeight)))
			quadrant_3();
		else if ((roomX > curX + curWidth) && (roomY <= (curY + curHeight)))
			quadrant_2();
		else if ((roomX > curX + curWidth) && (roomY > (curY + curHeight)))
			quadrant_4();
		else if (roomY < curY + curHeight)
			walkUp();
		else if (roomY > curY + curHeight)
			walkDown();
	} else {
		if ((roomX < curX + curWidth / 2 ) && (roomY <= (curY + curHeight)))
			quadrant_1();
		else if ((roomX < curX + curWidth / 2) && (roomY > (curY + curHeight)))
			quadrant_3();
		else if ((roomX > curX + curWidth / 2) && (roomY <= (curY + curHeight)))
			quadrant_2();
		else if ((roomX > curX + curWidth / 2) && (roomY > (curY + curHeight)))
			quadrant_4();
		else
			characterMoved = 0;
	}
	startTime = getTime();
}

void DrasculaEngine::moveCharacters() {
	int curPos[6];
	int r;

	if (characterMoved == 1 && stepX == STEP_X) {
		for (r = 0; r < stepX; r++) {
			if (currentChapter != 2) {
				if (trackProtagonist == 0 && roomX - r == curX + curWidth / 2) {
					characterMoved = 0;
					stepX = STEP_X;
					stepY = STEP_Y;
				}
				if (trackProtagonist == 1 && roomX + r == curX + curWidth / 2) {
					characterMoved = 0;
					stepX = STEP_X;
					stepY = STEP_Y;
					curX = roomX - curWidth / 2;
					curY = roomY - curHeight;
				}
			} else if (currentChapter == 2) {
				if (trackProtagonist == 0 && roomX - r == curX) {
					characterMoved = 0;
					stepX = STEP_X;
					stepY = STEP_Y;
				}
				if (trackProtagonist == 1 && roomX + r == curX + curWidth) {
					characterMoved = 0;
					stepX = STEP_X;
					stepY = STEP_Y;
					curX = roomX - curWidth + 4;
					curY = roomY - curHeight;
				}
			}
		}
	}
	if (characterMoved == 1 && stepY == STEP_Y) {
		for (r = 0; r < stepY; r++) {
			if (trackProtagonist == 2 && roomY - r == curY + curHeight) {
				characterMoved = 0;
				stepX = STEP_X;
				stepY = STEP_Y;
			}
			if (trackProtagonist == 3 && roomY + r == curY + curHeight) {
				characterMoved = 0;
				stepX = STEP_X;
				stepY = STEP_Y;
			}
		}
	}

	if (currentChapter == 1 || currentChapter == 4 || currentChapter == 5 || currentChapter == 6) {
		if (hare_se_ve == 0) {
			increaseFrameNum();
			return;
		}
	}

	if (characterMoved == 0) {
		curPos[0] = 0;
		curPos[1] = DIF_MASK_HARE;
		curPos[2] = curX;
		curPos[3] = curY;
		if (currentChapter == 2) {
			curPos[4] = curWidth;
			curPos[5] = curHeight;
		} else {
			curPos[4] = CHARACTER_WIDTH;
			curPos[5] = CHARACTER_HEIGHT;
		}

		if (trackProtagonist == 0) {
			curPos[1] = 0;
			if (currentChapter == 2)
				copyRectClip(curPos, extraSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], extraSurface, screenSurface);
		} else if (trackProtagonist == 1) {
			if (currentChapter == 2)
				copyRectClip(curPos, extraSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], extraSurface, screenSurface);
		} else if (trackProtagonist == 2) {
			if (currentChapter == 2)
				copyRectClip(curPos, backSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], backSurface, screenSurface);
		} else {
			if (currentChapter == 2)
				copyRectClip(curPos, frontSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], frontSurface, screenSurface);
		}
	} else if (characterMoved == 1) {
		curPos[0] = _frameX[num_frame];
		curPos[1] = frame_y + DIF_MASK_HARE;
		curPos[2] = curX;
		curPos[3] = curY;
		if (currentChapter == 2) {
			curPos[4] = curWidth;
			curPos[5] = curHeight;
		} else {
			curPos[4] = CHARACTER_WIDTH;
			curPos[5] = CHARACTER_HEIGHT;
		}
		if (trackProtagonist == 0) {
			curPos[1] = 0;
			if (currentChapter == 2)
				copyRectClip(curPos, extraSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], extraSurface, screenSurface);
		} else if (trackProtagonist == 1) {
			if (currentChapter == 2)
				copyRectClip(curPos, extraSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], extraSurface, screenSurface);
		} else if (trackProtagonist == 2) {
			if (currentChapter == 2)
				copyRectClip(curPos, backSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], backSurface, screenSurface);
		} else {
			if (currentChapter == 2)
				copyRectClip(curPos, frontSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], frontSurface, screenSurface);
		}
		increaseFrameNum();
	}
}

void DrasculaEngine::quadrant_1() {
	float distanceX, distanceY;

	if (currentChapter == 2)
		distanceX = curX - roomX;
	else
		distanceX = curX + curWidth / 2 - roomX;

	distanceY = (curY + curHeight) - roomY;

	if (distanceX < distanceY) {
		curDirection = 0;
		trackProtagonist = 2;
		stepX = (int)(distanceX / (distanceY / STEP_Y));
	} else {
		curDirection = 7;
		trackProtagonist = 0;
		stepY = (int)(distanceY / (distanceX / STEP_X));
	}
}

void DrasculaEngine::quadrant_2() {
	float distanceX, distanceY;

	if (currentChapter == 2)
		distanceX = abs(curX + curWidth - roomX);
	else
		distanceX = abs(curX + curWidth / 2 - roomX);

	distanceY = (curY + curHeight) - roomY;

	if (distanceX < distanceY) {
		curDirection = 1;
		trackProtagonist = 2;
		stepX = (int)(distanceX / (distanceY / STEP_Y));
	} else {
		curDirection = 2;
		trackProtagonist = 1;
		stepY = (int)(distanceY / (distanceX / STEP_X));
	}
}

void DrasculaEngine::quadrant_3() {
	float distanceX, distanceY;

	if (currentChapter == 2)
		distanceX = curX - roomX;
	else
		distanceX = curX + curWidth / 2 - roomX;

	distanceY = roomY - (curY + curHeight);

	if (distanceX < distanceY) {
		curDirection = 5;
		trackProtagonist = 3;
		stepX = (int)(distanceX / (distanceY / STEP_Y));
	} else {
		curDirection = 6;
		trackProtagonist = 0;
		stepY = (int)(distanceY / (distanceX / STEP_X));
	}
}

void DrasculaEngine::quadrant_4() {
	float distanceX, distanceY;

	if (currentChapter == 2)
		distanceX = abs(curX + curWidth - roomX);
	else
		distanceX = abs(curX + curWidth / 2 - roomX);

	distanceY = roomY - (curY + curHeight);

	if (distanceX < distanceY) {
		curDirection = 4;
		trackProtagonist = 3;
		stepX = (int)(distanceX / (distanceY / STEP_Y));
	} else {
		curDirection = 3;
		trackProtagonist = 1;
		stepY = (int)(distanceY / (distanceX / STEP_X));
	}
}

void DrasculaEngine::increaseFrameNum() {
	timeDiff = getTime() - startTime;

	if (timeDiff >= 6) {
		startTime = getTime();
		num_frame++;
		if (num_frame == 6)
			num_frame = 0;

		if (curDirection == 0 || curDirection == 7) {
			curX -= stepX;
			curY -= stepY;
		} else if (curDirection == 1 || curDirection == 2) {
			curX += stepX;
			curY -= stepY;
		} else if (curDirection == 3 || curDirection == 4) {
			curX += stepX;
			curY += stepY;
		} else if (curDirection == 5 || curDirection == 6) {
			curX -= stepX;
			curY += stepY;
		}
	}

	if (currentChapter != 2) {
		curY += (int)(curHeight - newHeight);
		curX += (int)(curWidth - newWidth);
		curHeight = (int)newHeight;
		curWidth = (int)newWidth;
	}
}

void DrasculaEngine::walkDown() {
	curDirection = 4;
	trackProtagonist = 3;
	stepX = 0;
}

void DrasculaEngine::walkUp() {
	curDirection = 0;
	trackProtagonist = 2;
	stepX = 0;
}

void DrasculaEngine::moveVonBraun() {
	int pos_vb[6];

	if (vonBraunHasMoved == 0) {
		pos_vb[0] = 256;
		pos_vb[1] = 129;
		pos_vb[2] = vonBraunX;
		pos_vb[3] = 66;
		pos_vb[4] = 33;
		pos_vb[5] = 69;
		if (trackVonBraun == 0)
			pos_vb[0] = 222;
		else if (trackVonBraun == 1)
			pos_vb[0] = 188;
	} else {
		pos_vb[0] = actorFrames[kFrameVonBraun];
		pos_vb[1] = (trackVonBraun == 0) ? 62 : 131;
		pos_vb[2] = vonBraunX;
		pos_vb[3] = 66;
		pos_vb[4] = 28;
		pos_vb[5] = 68;

		actorFrames[kFrameVonBraun] += 29;
		if (actorFrames[kFrameVonBraun] > 146)
			actorFrames[kFrameVonBraun] = 1;
	}

	copyRectClip(pos_vb, frontSurface, screenSurface);
}

void DrasculaEngine::placeVonBraun(int pointX) {
	trackVonBraun = (pointX < vonBraunX) ? 0 : 1;
	vonBraunHasMoved = 1;

	for (;;) {
		updateRoom();
		updateScreen();
		if (trackVonBraun == 0) {
			vonBraunX = vonBraunX - 5;
			if (vonBraunX <= pointX)
				break;
		} else {
			vonBraunX = vonBraunX + 5;
			if (vonBraunX >= pointX)
				break;
		}
		pause(5);
	}

	vonBraunHasMoved = 0;
}



} // End of namespace Drascula
