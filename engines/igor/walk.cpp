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
 */

#include "igor/igor.h"

namespace Igor {

static int16 roundReal(float f) {
	return (int16)(f + .5);
}

static int16 truncReal(float f) {
	return (int16)f;
}

void IgorEngine::setRoomWalkBounds(int x1, int y1, int x2, int y2) {
	assert(x1 <= x2 && y1 <= y2);
	_roomWalkBounds.x1 = x1;
	_roomWalkBounds.x2 = x2;
	_roomWalkBounds.y1 = y1;
	_roomWalkBounds.y2 = y2;
}

void IgorEngine::buildWalkPath(int srcX, int srcY, int dstX, int dstY) {
	if (srcX != dstX || srcY != dstY) {
		_walkData[0] = _walkData[_walkDataLastIndex];
		_walkDataLastIndex = 1;
		int srcArea = _roomObjectAreasTable[_screenLayer2[srcY * 320 + srcX]].area;
		int dstArea = _roomObjectAreasTable[_screenLayer2[dstY * 320 + dstX]].area;
		debugC(9, kDebugWalk, "srcArea = %d dstArea = %d", srcArea, dstArea);
		// for (int i = 1; dstArea != currentArea; ++i) {
		// 	const int boxOffset = srcArea * _roomDataOffsets.area.boxSrcSize + dstArea * _roomDataOffsets.area.boxDstSize;
		// 	int nextArea = _roomActionsTable[boxOffset + i + _roomDataOffsets.area.box];
			// debugC(9, kDebugWalk, "nextArea %d (%d,%d,%d)", nextArea, _roomDataOffsets.area.box, _roomDataOffsets.area.boxSrcSize, _roomDataOffsets.area.boxDstSize);
		// 	int nextPosX, nextPosY;
		// 	if (dstArea != nextArea) {
		// 		getClosestAreaTrianglePoint(nextArea, currentArea, &nextPosY, &nextPosX, srcY, srcX);
		// 	} else {
		// 		getClosestAreaTrianglePoint2(nextArea, currentArea, &nextPosY, &nextPosX, dstY, dstX, srcY, srcX);
		// 	}
			// debugC(9, kDebugWalk, "buildWalkPath() transitionArea = %d next %d,%d pos %d,%d offset 0x%X", nextArea, nextPosX, nextPosY, dstX, dstY, _roomDataOffsets.area.box);
		// 	buildWalkPathArea(srcX, srcY, nextPosX, nextPosY);
		// 	srcX = nextPosX;
		// 	srcY = nextPosY;
		// 	currentArea = nextArea;
		// }
		buildWalkPathArea(srcX, srcY, dstX, dstY);
		--_walkDataLastIndex;
	}
	debugC(9, kDebugWalk, "buildWalkPath() end _walkDataLastIndex %d", _walkDataLastIndex);
}

int IgorEngine::lookupScale(int xOffset, int yOffset, int h) const {
	int index = READ_LE_UINT16(_walkScaleTable + 0x734 + _walkWidthScaleTable[h - 1] * 2);
	int offset = _walkScaleTable[0x4FC + index + xOffset];
	index = READ_LE_UINT16(_walkScaleTable + 0x6CE + h * 2);
	offset += _walkScaleTable[index + yOffset] * 30;
	return offset;
}

void IgorEngine::lookupScale(int curX, int curY, uint8 &scale, uint8 &xScale, uint8 &yScale) const {
	scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
	yScale = _walkScaleTable[0x901 + scale];
	xScale = _walkWidthScaleTable[scale - 1];
}

void IgorEngine::moveIgor(int pos, int frame) {
	assert(_gameState.enableLight == 1 || _gameState.enableLight == 2);
	debugC(9, kDebugWalk, "moveIgorHelper _walkDataCurrentIndex %d pos %d frame %d", _walkDataCurrentIndex, pos, frame);
	WalkData *wd = &_walkData[_walkDataCurrentIndex];
	uint8 _walkClipSkipX = wd->clipSkipX;
	uint8 _walkHeightScale = wd->scaleHeight;
	int16 _walkClipWidth = wd->clipWidth;
	uint16 _walkScaleWidth = wd->scaleWidth;
	uint8 _walkXPosChanged = wd->xPosChanged;
	int16 _walkDxPos = wd->dxPos + 1;
	uint8 _walkYPosChanged = wd->yPosChanged;
	int16 _walkDyPos = wd->dyPos;
	int16 _walkDataCurrentPosX2 = wd->x;
	int16 _walkDataCurrentPosY2 = wd->y;

	uint16 _walkDataDrawOffset = (wd->y - wd->scaleWidth + 1) * 320;

	int xPos = _walkWidthScaleTable[wd->scaleHeight - 1] / 2;
	if (wd->x > xPos) {
		_walkDataDrawOffset += wd->x - xPos;
	}
	if (_walkXPosChanged != 0) {
		_walkDataDrawOffset -= _walkDxPos;
	}
	if (_walkYPosChanged != 0) {
		_walkDataDrawOffset -= _walkDyPos * 320;
	}
	if (_gameState.enableLight == 2) {
		int8 colorLum = _roomObjectAreasTable[_screenLayer2[_walkDataCurrentPosY2 * 320 + _walkDataCurrentPosX2]].y2Lum;
		if (_gameState.colorLum != colorLum) {
			for (int color = 192 * 3; color <= 207 * 3; ++color) {
				int c = _currentPalette[color] + colorLum;
				if (c < 1) {
					c = 0;
				} else if (c > 62) {
					c = 63;
				}
				_currentPalette[color] = c;
			}
			setPaletteRange(192, 207);
			_gameState.colorLum = colorLum;
		}
	}
	uint16 screenIgorDrawOffset = _walkDataDrawOffset;
	uint16 igorScaledWidth = _walkDxPos + _walkClipWidth;
	uint16 igorScaledHeight = _walkHeightScale + _walkDyPos;
	uint16 igorBodyScanLine = 0;
	if (_walkYPosChanged != 0) {
		for (int i = 1; i <= _walkDyPos; ++i) {
			memcpy(_igorTempFrames + igorBodyScanLine * 50, _screenLayer1 + _walkDataDrawOffset, igorScaledWidth);
			_walkDataDrawOffset += 320;
			++igorBodyScanLine;
		}
	}
	if (_walkXPosChanged != 0) {
		for (int yOffset = 0; yOffset < _walkScaleWidth; ++yOffset) {
			assert(_walkDxPos > 0);
			assert(igorBodyScanLine * 50 + _walkDxPos <= 3000);
			memcpy(_igorTempFrames + igorBodyScanLine * 50, _screenLayer1 + _walkDataDrawOffset, _walkDxPos);
			int xOffset = _walkClipSkipX - 1;
			for (int i = 0; i < _walkClipWidth; ++i) {
				int offset = lookupScale(xOffset, yOffset, _walkHeightScale);
				offset += frame * 1500;
				uint8 color = _facingIgorFrames[pos - 1][offset];
				if (color != 0) {
					assert(_walkDataDrawOffset + _walkDxPos + i >= 0);
					int index = _screenLayer2[_walkDataDrawOffset + _walkDxPos + i];
					int yPos = _roomObjectAreasTable[index].y1Lum;
					if (wd->y <= yPos) {
						_igorTempFrames[igorBodyScanLine * 50 + i + _walkDxPos] = _screenLayer1[_walkDataDrawOffset + _walkDxPos + i];
					} else {
						if (_gameState.enableLight == 1 && wd->y <= _roomObjectAreasTable[index].y2Lum) {
							color -= _roomObjectAreasTable[index].deltaLum;
						}
						_igorTempFrames[igorBodyScanLine * 50 + i + _walkDxPos] = color;
					}
				} else {
					_igorTempFrames[igorBodyScanLine * 50 + i + _walkDxPos] = _screenLayer1[_walkDataDrawOffset + _walkDxPos + i];
				}
				++xOffset;
			}
			_walkDataDrawOffset += 320;
			++igorBodyScanLine;
		}
	} else {
		for (int yOffset = 0; yOffset < _walkScaleWidth; ++yOffset) {
			int xOffset = _walkClipSkipX - 1;
			for (int i = 0; i < _walkClipWidth; ++i) {
				int offset = lookupScale(xOffset, yOffset, _walkHeightScale);
				offset += frame * 1500;
				uint8 color = _facingIgorFrames[pos - 1][offset];
				if (color != 0) {
					assert(_walkDataDrawOffset + i >= 0);
					int index = _screenLayer2[_walkDataDrawOffset + i];
					int yPos = _roomObjectAreasTable[index].y1Lum;
					if (wd->y <= yPos) {
						_igorTempFrames[igorBodyScanLine * 50 + i] = _screenLayer1[_walkDataDrawOffset + i];
					} else {
						if (_gameState.enableLight == 1 && wd->y <= _roomObjectAreasTable[index].y2Lum) {
							color -= _roomObjectAreasTable[index].deltaLum;
						}
						_igorTempFrames[igorBodyScanLine * 50 + i] = color;
					}
				} else {
					_igorTempFrames[igorBodyScanLine * 50 + i] = _screenLayer1[_walkDataDrawOffset + i];
				}
				++xOffset;
			}
			const uint8 *src = _screenLayer1 + _walkDataDrawOffset + _walkClipWidth;
			memcpy(_igorTempFrames + igorBodyScanLine * 50 + _walkClipWidth, src, _walkDxPos);
			_walkDataDrawOffset += 320;
			++igorBodyScanLine;
		}
	}
	if (_walkYPosChanged == 0) {
		for (int i = 1; i <= _walkDyPos; ++i) {
			memcpy(_igorTempFrames + igorBodyScanLine * 50, _screenLayer1 + _walkDataDrawOffset, igorScaledWidth);
			_walkDataDrawOffset += 320;
			++igorBodyScanLine;
		}
	}
	for (igorBodyScanLine = 0; igorBodyScanLine < igorScaledHeight; ++igorBodyScanLine) {
		assert(screenIgorDrawOffset + igorScaledWidth <= 320 * 200);
		memcpy(_screenVGA + screenIgorDrawOffset, _igorTempFrames + igorBodyScanLine * 50, igorScaledWidth);
		screenIgorDrawOffset += 320;
	}
}

void IgorEngine::buildWalkPathArea(int srcX, int srcY, int dstX, int dstY) {
	if (srcX != dstX || srcY != dstY) {
		const int dx = dstX - srcX;
		const int dy = dstY - srcY;
		debugC(9, kDebugWalk, "buildWalkPathArea() dx = %d dy = %d src %d,%d dst %d,%d", dx, dy, srcX, srcY, dstX, dstY);
		assert(_walkDataLastIndex > 0);
		if (ABS(dy) * 2 > ABS(dx)) {
			if (srcY > dstY) {
				buildWalkPathAreaUpDirection(srcX, srcY, dstX, dstY);
			} else {
				buildWalkPathAreaDownDirection(srcX, srcY, dstX, dstY);
			}
		} else {
			if (srcX < dstX) {
				buildWalkPathAreaRightDirection(srcX, srcY, dstX, dstY);
			} else {
				buildWalkPathAreaLeftDirection(srcX, srcY, dstX, dstY);
			}
		}
	}
}

int IgorEngine::getVerticalStepsCount(int minX, int minY, int maxX, int maxY) {
	debugC(9, kDebugWalk, "getVerticalStepsCount() %d %d %d %d", minX, minY, maxX, maxY);
	int curX = 2;
	if ((_walkXScaleRoom[minX] != 1 || _walkXScaleRoom[maxX] != 3) && (_walkXScaleRoom[maxX] != 1 || _walkXScaleRoom[minX] != 3)) {
		curX = _walkXScaleRoom[minX];
	}
	int curY = minY;
	int count = 0;
	while (1) {
		uint8 scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
		scale = _walkScaleTable[0x901 + scale];
		if (maxY - curY <= scale) {
			break;
		}
		curY += scale;
		++count;
	}
	return count;
}

int IgorEngine::getHorizontalStepsCount(int minX, int minY, int maxX, int maxY) {
	uint8 scale, frame = _walkCurrentFrame;

	scale = _walkYScaleRoom[(_walkXScaleRoom[maxX] - 1) * 144 + maxY];
	float r1 = _walkScaleSpeedTable[scale - 1];
	scale = _walkYScaleRoom[(_walkXScaleRoom[minX] - 1) * 144 + minY];
	float r2 = _walkScaleSpeedTable[scale - 1];
	debugC(9, kDebugWalk, "getHorizontalStepsCount() maxX - minX = %d r1 = %f r2 = %f", maxX - minX, r1, r2);

	int16 steps = roundReal((maxX - minX) / ((r1 + r2) / 2.0f));
	int count = 0;
	if (steps != 0) {
		float r3 = (maxY - minY) / (float)steps;
		int curX = minX;
		int curY = minY;
		float r4 = r3;
		while (1) {
			scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
			uint8 c = _walkScaleTable[0x769 + scale * 8 + frame];
			if (maxX - curX <= c) {
				break;
			}
			curX += c;
			curY = minY + truncReal(r4);
			if (frame == 8) {
				frame = 1;
			} else {
				++frame;
			}
			r4 += r3;
			++count;
		}
	}
	return count;
}

void IgorEngine::buildWalkPathAreaUpDirection(int srcX, int srcY, int dstX, int dstY) {
	int _walkCurrentPosLocalVar = _walkCurrentPos;
	if (_walkCurrentPos == 3) {
		_walkCurrentPos = 1;
		_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
		_walkData[_walkDataLastIndex].posNum = (dstX > srcX) ? kFacingPositionRight : kFacingPositionLeft;
		_walkData[_walkDataLastIndex].frameNum = 5;
		++_walkDataLastIndex;
	} else {
		_walkCurrentPos = 1;
	}
	if (_walkCurrentFrame > 6) {
		_walkCurrentFrame -= 6;
	}
	WalkData *wd;
	uint8 scale, xScale, yScale;
	float vStepDist, vStepCur;
	int curX = srcX;
	int curY = srcY;
	if (srcX > dstX) {
		int vStepsCount = getVerticalStepsCount(dstX, dstY, srcX, srcY);
		if (vStepsCount > 0) {
			vStepCur = vStepDist = (srcX - dstX) / (float)vStepsCount;
			for (int i = 1; i <= vStepsCount; ++i) {
				wd = &_walkData[_walkDataLastIndex];
				curX = srcX - truncReal(vStepCur);
				lookupScale(curX, curY, scale, xScale, yScale);
				curY -= yScale;
				lookupScale(curX, curY, scale, xScale, yScale);
				wd->setPos(curX, curY, 1, _walkCurrentFrame);
				WalkData::setNextFrame(kFacingPositionBack, _walkCurrentFrame);
				wd->setScale(scale, scale);
				wd->yPosChanged = 0;
				wd->dyPos = _walkData[_walkDataLastIndex - 1].y - wd->y;
				int x = xScale - xScale / 2 + curX - 1;
				if (x > 319) {
					wd->clipSkipX = 1;
					wd->clipWidth = 319 - (x - xScale);
					wd->xPosChanged = 0;
					wd->dxPos = 0;
				} else {
					x = curX - xScale / 2;
					if (x < 0) {
						wd->clipWidth = x + xScale;
						wd->clipSkipX = xScale - wd->clipWidth + 1;
						wd->xPosChanged = 0;
						wd->dxPos = _walkData[_walkDataLastIndex - 1].x - wd->x;
					} else {
						wd->clipWidth = xScale;
						wd->clipSkipX = 1;
						wd->xPosChanged = 0;
						wd->dxPos = _walkData[_walkDataLastIndex - 1].x - wd->x;
					}
				}
				++_walkDataLastIndex;
				vStepCur += vStepDist;
			}
		} else {
			if (_walkCurrentPosLocalVar == 1 && _walkToObjectPosX == dstX && _walkToObjectPosY == dstY) {
				_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
				_walkData[_walkDataLastIndex].posNum = 1;
				_walkData[_walkDataLastIndex].frameNum = 1;
				++_walkDataLastIndex;
			}
		}
		lookupScale(dstX, dstY, scale, xScale, yScale);
		wd = &_walkData[_walkDataLastIndex];
		wd->setPos(dstX, dstY, 1, _walkCurrentFrame);
		WalkData::setNextFrame(kFacingPositionBack, _walkCurrentFrame);
		wd->setScale(scale, scale);
		wd->yPosChanged = 0;
		wd->dyPos = _walkData[_walkDataLastIndex - 1].y - wd->y;
		int x = xScale - xScale / 2 + dstX - 1;
		if (x > 319) {
			wd->clipSkipX = 1;
			wd->clipWidth = 319 - (x - xScale / 2);
			wd->xPosChanged = 0;
			wd->dxPos = 0;
		} else {
			x = dstX - xScale / 2;
			if (x < 0) {
				wd->clipWidth = xScale + x;
				wd->clipSkipX = xScale - wd->clipWidth + 1;
				wd->xPosChanged = 0;
				wd->dxPos = curX - dstX;
			} else {
				wd->clipWidth = xScale;
				wd->clipSkipX = 1;
				wd->xPosChanged = 0;
				wd->dxPos = curX - dstX;
			}
		}
		++_walkDataLastIndex;
	} else {
		int vStepsCount = getVerticalStepsCount(dstX, dstY, srcX, srcY);
		if (vStepsCount > 0) {
			vStepCur = vStepDist = (dstX - srcX) / (float)vStepsCount;
			for (int i = 1; i <= vStepsCount; ++i) {
				wd = &_walkData[_walkDataLastIndex];
				curX = srcX + truncReal(vStepCur);
				lookupScale(curX, curY, scale, xScale, yScale);
				curY -= yScale;
				lookupScale(curX, curY, scale, xScale, yScale);
				wd->setPos(curX, curY, 1, _walkCurrentFrame);
				WalkData::setNextFrame(kFacingPositionBack, _walkCurrentFrame);
				lookupScale(curX, curY, scale, xScale, yScale);
				wd->setScale(scale, scale);
				wd->yPosChanged = 0;
				wd->dyPos = _walkData[_walkDataLastIndex - 1].y - wd->y;
				int x = xScale - xScale / 2 + curX - 1;
				if (x > 319) {
					wd->clipSkipX = 1;
					wd->clipWidth = 319 - (x - xScale);
					wd->xPosChanged = 0;
					wd->dxPos = wd->x - _walkData[_walkDataLastIndex - 1].x;
				} else {
					x = curX - xScale / 2;
					if (x < 0) {
						wd->clipWidth = x + xScale;
						wd->clipSkipX = xScale - wd->clipWidth + 1;
						wd->xPosChanged = 1;
						wd->dxPos = 0;
					} else {
						wd->clipWidth = xScale;
						wd->clipSkipX = 1;
						wd->xPosChanged = 1;
						wd->dxPos = wd->x - _walkData[_walkDataLastIndex - 1].x;
					}
				}
				++_walkDataLastIndex;
				vStepCur += vStepDist;
			}
		} else {
			if (_walkCurrentPosLocalVar == 1 && _walkToObjectPosX == dstX && _walkToObjectPosY == dstY) {
				_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
				_walkData[_walkDataLastIndex].posNum = 1;
				_walkData[_walkDataLastIndex].frameNum = 1;
				++_walkDataLastIndex;
			}
		}
		lookupScale(dstX, dstY, scale, xScale, yScale);
		wd = &_walkData[_walkDataLastIndex];
		wd->setPos(dstX, dstY, 1, _walkCurrentFrame);
		WalkData::setNextFrame(kFacingPositionBack, _walkCurrentFrame);
		wd->setScale(scale, scale);
		wd->yPosChanged = 0;
		wd->dyPos = _walkData[_walkDataLastIndex - 1].y - wd->y;
		int x = xScale - xScale / 2 + dstX - 1;
		if (x > 319) {
			wd->clipSkipX = 1;
			int16 _dx = x - xScale;
			int16 _cx = 319 - _dx;
			wd->clipWidth = _cx;
			wd->xPosChanged = 0;
			wd->dxPos = dstX - curX;
		} else {
			x = dstX - xScale / 2;
			if (x < 0) {
				wd->clipWidth = xScale + x;
				wd->clipSkipX = xScale - wd->clipWidth + 1;
				wd->xPosChanged = 1;
				wd->dxPos = 0;
			} else {
				wd->clipWidth = xScale;
				wd->clipSkipX = 1;
				wd->xPosChanged = 1;
				wd->dxPos = dstX - curX;
			}
		}
		++_walkDataLastIndex;
	}
}

void IgorEngine::buildWalkPathAreaDownDirection(int srcX, int srcY, int dstX, int dstY) {
	int _walkCurrentPosLocalVar = _walkCurrentPos;
	if (_walkCurrentPos == 1) {
		_walkCurrentPos = 3;
		_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
		_walkData[_walkDataLastIndex].posNum = (srcX > dstX) ? kFacingPositionLeft : kFacingPositionRight;
		_walkData[_walkDataLastIndex].frameNum = 1;
		++_walkDataLastIndex;
	} else {
		_walkCurrentPos = 3;
	}
	if (_walkCurrentFrame > 6) {
		_walkCurrentFrame -= 6;
	}
	WalkData *wd;
	uint8 scale, xScale, yScale;
	float vStepCur, vStepDist;
	int curX = srcX;
	int curY = srcY;
	if (srcX > dstX) {
		int vStepsCount = getVerticalStepsCount(srcX, srcY, dstX, dstY);
		if (vStepsCount > 0) {
			vStepCur = vStepDist = (srcX - dstX) / (float)vStepsCount;
			for (int i = 1; i <= vStepsCount; ++i) {
				wd = &_walkData[_walkDataLastIndex];
				curX = srcX - truncReal(vStepCur);
				lookupScale(curX, curY, scale, xScale, yScale);
				curY += yScale;
				lookupScale(curX, curY, scale, xScale, yScale);
				wd->setPos(curX, curY, 3, _walkCurrentFrame);
				WalkData::setNextFrame(kFacingPositionFront, _walkCurrentFrame);
				wd->setScale(scale, scale);
				wd->yPosChanged = 1;
				wd->dyPos = wd->y - _walkData[_walkDataLastIndex - 1].y;
				int x = xScale - xScale / 2 + curX - 1;
				if (x > 319) {
					wd->clipSkipX = 1;
					wd->clipWidth = 319 - (x - xScale);
					wd->xPosChanged = 0;
					wd->dxPos = 0;
				} else {
					x = curX - xScale / 2;
					if (x < 0) {
						wd->clipWidth = x + xScale;
						wd->clipSkipX = xScale - wd->clipWidth + 1;
						wd->xPosChanged = 0;
						wd->dxPos = _walkData[_walkDataLastIndex - 1].x - wd->x;
					} else {
						wd->clipWidth = xScale;
						wd->clipSkipX = 1;
						wd->xPosChanged = 0;
						wd->dxPos = _walkData[_walkDataLastIndex - 1].x - wd->x;
					}
				}
				++_walkDataLastIndex;
				vStepCur += vStepDist;
			}
		} else {
			if (_walkCurrentPosLocalVar == 3 && _walkToObjectPosX == dstX && _walkToObjectPosY == dstY) {
				_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
				_walkData[_walkDataLastIndex].posNum = 3;
				_walkData[_walkDataLastIndex].frameNum = 1;
				++_walkDataLastIndex;
			}
		}
		lookupScale(dstX, dstY, scale, xScale, yScale);
		wd = &_walkData[_walkDataLastIndex];
		wd->setPos(dstX, dstY, 3, _walkCurrentFrame);
		WalkData::setNextFrame(kFacingPositionFront, _walkCurrentFrame);
		wd->setScale(scale, scale);
		wd->yPosChanged = 1;
		wd->dyPos = wd->y - _walkData[_walkDataLastIndex - 1].y;
		int x = xScale - xScale / 2 + dstX - 1;
		if (x > 319) {
			wd->clipSkipX = 1;
			wd->clipWidth = 319 - (x - xScale);
			wd->xPosChanged = 0;
			wd->dxPos = 0;
		} else {
			x = dstX - xScale / 2;
			if (x < 0) {
				wd->clipWidth = xScale + x;
				wd->clipSkipX = xScale - wd->clipWidth + 1;
				wd->xPosChanged = 0;
				wd->dxPos = curX - dstX;
			} else {
				wd->clipWidth = xScale;
				wd->clipSkipX = 1;
				wd->xPosChanged = 0;
				wd->dxPos = curX - dstX;
			}
		}
		++_walkDataLastIndex;
	} else {
		int vStepsCount = getVerticalStepsCount(srcX, srcY, dstX, dstY);
		if (vStepsCount > 0) {
			vStepCur = vStepDist = (dstX - srcX) / (float)vStepsCount;
			for (int i = 1; i <= vStepsCount; ++i) {
				wd = &_walkData[_walkDataLastIndex];
				curX = srcX + truncReal(vStepCur);
				lookupScale(curX, curY, scale, xScale, yScale);
				curY += yScale;
				lookupScale(curX, curY, scale, xScale, yScale);
				wd->setPos(curX, curY, 3, _walkCurrentFrame);
				WalkData::setNextFrame(kFacingPositionFront, _walkCurrentFrame);
				wd->setScale(scale, scale);
				wd->yPosChanged = 1;
				wd->dyPos = wd->y - _walkData[_walkDataLastIndex - 1].y;
				int x = xScale - xScale / 2 + curX - 1;
				if (x > 319) {
					wd->clipSkipX = 1;
					wd->clipWidth = 319 - (x - xScale);
					wd->xPosChanged = 1;
					wd->dxPos = wd->x - _walkData[_walkDataLastIndex - 1].x;
				} else {
					x = curX - xScale / 2;
					if (x < 0) {
						wd->clipWidth = x + xScale;
						wd->clipSkipX = xScale - wd->clipWidth + 1;
						wd->xPosChanged = 1;
						wd->dxPos = 0;
					} else {
						wd->clipWidth = xScale;
						wd->clipSkipX = 1;
						wd->xPosChanged = 1;
						wd->dxPos = wd->x - _walkData[_walkDataLastIndex - 1].x;
					}
				}
				++_walkDataLastIndex;
				vStepCur += vStepDist;
			}
		} else {
			if (_walkCurrentPosLocalVar == 3 && _walkToObjectPosX == dstX && _walkToObjectPosY == dstY) {
				_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
				_walkData[_walkDataLastIndex].posNum = 3;
				_walkData[_walkDataLastIndex].frameNum = 1;
				++_walkDataLastIndex;
			}
		}
		lookupScale(dstX, dstY, scale, xScale, yScale);
		wd = &_walkData[_walkDataLastIndex];
		wd->setPos(dstX, dstY, 3, _walkCurrentFrame);
		WalkData::setNextFrame(kFacingPositionFront, _walkCurrentFrame);
		wd->setScale(scale, scale);
		wd->yPosChanged = 1;
		wd->dyPos = wd->y - _walkData[_walkDataLastIndex - 1].y;
		int x = xScale - xScale / 2 + dstX - 1;
		if (x > 319) {
			wd->clipSkipX = 1;
			wd->clipWidth = 319 - (x - xScale);
			wd->xPosChanged = 1;
			wd->dxPos = dstX - curX;
		} else {
			x = dstX - xScale / 2;
			if (x < 0) {
				wd->clipWidth = xScale + x;
				wd->clipSkipX = xScale - wd->clipWidth + 1;
				wd->xPosChanged = 1;
				wd->dxPos = 0;
			} else {
				wd->clipWidth = xScale;
				wd->clipSkipX = 1;
				wd->xPosChanged = 1;
				wd->dxPos = dstX - curX;
			}
		}
		++_walkDataLastIndex;
	}
}

void IgorEngine::buildWalkPathAreaRightDirection(int srcX, int srcY, int dstX, int dstY) {
	int _walkCurrentPosLocalVar = _walkCurrentPos;
	if (_walkCurrentPos == kFacingPositionLeft) {
		_walkCurrentPos = kFacingPositionRight;
		_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
		_walkData[_walkDataLastIndex].posNum = (srcY > dstY) ? kFacingPositionBack : kFacingPositionFront;
		_walkData[_walkDataLastIndex].frameNum = 5;
		++_walkDataLastIndex;
	} else {
		_walkCurrentPos = kFacingPositionRight;
	}
	WalkData *wd;
	uint8 scale, xScale, xSkip;
	float hStepCur, hStepDist;
	int curX = srcX;
	int curY = srcY;
	if (srcY > dstY) {
		int hStepsCount = getHorizontalStepsCount(srcX, dstY, dstX, srcY);
		if (hStepsCount > 0) {
			hStepCur = hStepDist = (srcY - dstY) / (float)hStepsCount;
			for (int i = 1; i <= hStepsCount; ++i) {
				wd = &_walkData[_walkDataLastIndex];
				scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
				xSkip = _walkScaleTable[0x769 + scale * 8 + _walkCurrentFrame];
				if ((curX + xSkip) >= dstX) {
					break;
				}
				curX += xSkip;
				curY = srcY - truncReal(hStepCur);
				wd->setPos(curX, curY, 2, _walkCurrentFrame);
				scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
				xScale = _walkWidthScaleTable[scale - 1];
				wd->setScale(scale, scale);
				wd->yPosChanged = 0;
				wd->dyPos = _walkData[_walkDataLastIndex - 1].y - wd->y;
				int x = xScale - xScale / 2 + curX - 1;
				if (x > 319) {
					wd->clipSkipX = 1;
					wd->clipWidth = 319 - (x - xScale);
					wd->xPosChanged = 1;
					wd->dxPos = xSkip;
				} else {
					x = x - xScale / 2;
					if (x < 0) {
						wd->clipWidth = x + xScale;
						wd->clipSkipX = xScale - wd->clipWidth + 1;
						wd->xPosChanged = 1;
						wd->dxPos = 0;
					} else {
						wd->clipWidth = xScale;
						wd->clipSkipX = 1;
						wd->xPosChanged = 1;
						wd->dxPos = xSkip;
					}
				}
				++_walkDataLastIndex;
				hStepCur += hStepDist;
				WalkData::setNextFrame(kFacingPositionRight, _walkCurrentFrame);
			}
		} else {
			if (_walkCurrentPosLocalVar == 2 && _walkToObjectPosX == dstX && _walkToObjectPosY == dstY) {
				_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
				_walkData[_walkDataLastIndex].posNum = 2;
				_walkData[_walkDataLastIndex].frameNum = 1;
				++_walkDataLastIndex;
			}
		}
		wd = &_walkData[_walkDataLastIndex];
		wd->setPos(dstX, dstY, 2, _walkCurrentFrame);
		scale = _walkYScaleRoom[(_walkXScaleRoom[dstX] - 1) * 144 + dstY];
		xScale = _walkWidthScaleTable[scale - 1];
		wd->setScale(scale, scale);
		wd->yPosChanged = 0;
		wd->dyPos = _walkData[_walkDataLastIndex - 1].y - wd->y;
		int x = xScale - xScale / 2 - 1;
		if (x > 319) {
			wd->clipSkipX = 1;
			wd->clipWidth = 319 - (x - xScale);
			wd->xPosChanged = 1;
			wd->dxPos = dstX - curX;
		} else {
			x = dstX - xScale / 2;
			if (x < 0) {
				wd->clipWidth = xScale + x;
				wd->clipSkipX = xScale - wd->clipWidth + 1;
				wd->xPosChanged = 1;
				wd->dxPos = 0;
			} else {
				wd->clipWidth = xScale;
				wd->clipSkipX = 1;
				wd->xPosChanged = 1;
				wd->dxPos = dstX - curX;
			}
		}
		++_walkDataLastIndex;
		WalkData::setNextFrame(kFacingPositionLeft, _walkCurrentFrame);
	} else {
		int hStepsCount = getHorizontalStepsCount(srcX, srcY, dstX, dstY);
		if (hStepsCount > 0) {
			hStepCur = hStepDist = (dstY - srcY) / (float)hStepsCount;
			for (int i = 1; i <= hStepsCount; ++i) {
				wd = &_walkData[_walkDataLastIndex];
				scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
				xSkip = _walkScaleTable[0x769 + scale * 8 + _walkCurrentFrame];
				if (curX + xSkip >= dstX) {
					break;
				}
				curX += xSkip;
				curY = srcY + truncReal(hStepCur);
				_walkData[_walkDataLastIndex].setPos(curX, curY, 2, _walkCurrentFrame);
				scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
				xScale = _walkWidthScaleTable[scale - 1];
				wd->setScale(scale, scale);
				wd->yPosChanged = 1;
				wd->dyPos = wd->y - _walkData[_walkDataLastIndex - 1].y;
				int x = xScale - xScale / 2 + curX - 1;
				if (x > 319) {
					wd->clipSkipX = 1;
					wd->clipWidth = 319 - (x - xScale);
					wd->xPosChanged = 1;
					wd->dxPos = xSkip;
				} else {
					x = curX - xScale / 2;
					if (x < 0) {
						wd->clipWidth = x + xScale;
						wd->clipSkipX = xScale - wd->clipWidth + 1;
						wd->xPosChanged = 1;
						wd->dxPos = 0;
					} else {
						wd->clipWidth = xScale;
						wd->clipSkipX = 1;
						wd->xPosChanged = 1;
						wd->dxPos = xSkip;
					}
				}
				++_walkDataLastIndex;
				hStepCur += hStepDist;
				WalkData::setNextFrame(kFacingPositionRight, _walkCurrentFrame);
			}
		} else {
			if (_walkCurrentPosLocalVar == 2 && _walkToObjectPosX == dstX && _walkToObjectPosY == dstY) {
				_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
				_walkData[_walkDataLastIndex].posNum = 2;
				_walkData[_walkDataLastIndex].frameNum = 1;
				++_walkDataLastIndex;
			}
		}
		wd = &_walkData[_walkDataLastIndex];
		wd->setPos(dstX, dstY, 2, _walkCurrentFrame);
		scale = _walkYScaleRoom[(_walkXScaleRoom[dstX] - 1) * 144 + dstY];
		xScale = _walkWidthScaleTable[scale - 1];
		wd->setScale(scale, scale);
		wd->yPosChanged = 1;
		wd->dyPos = wd->y - _walkData[_walkDataLastIndex - 1].y;
		int x = xScale - xScale / 2 + dstX - 1;
		if (x > 319) {
			wd->clipSkipX = 1;
			wd->clipWidth = 319 - (x - xScale);
			wd->xPosChanged = 1;
			wd->dxPos = dstX - curX;
		} else {
			x = dstX - xScale / 2;
			if (x < 0) {
				wd->clipWidth = xScale + x;
				wd->clipSkipX = xScale - wd->clipWidth + 1;
				wd->xPosChanged = 1;
				wd->dxPos = 0;
			} else {
				wd->clipWidth = xScale;
				wd->clipSkipX = 1;
				wd->xPosChanged = 1;
				wd->dxPos = dstX - curX;
			}
		}
		++_walkDataLastIndex;
		WalkData::setNextFrame(kFacingPositionRight, _walkCurrentFrame);
	}
}

void IgorEngine::buildWalkPathAreaLeftDirection(int srcX, int srcY, int dstX, int dstY) {
	int _walkCurrentPosLocalVar = _walkCurrentPos;
	if (_walkCurrentPos == kFacingPositionRight) {
		_walkCurrentPos = kFacingPositionLeft;
		_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
		_walkData[_walkDataLastIndex].posNum = (srcY > dstY) ? kFacingPositionBack : kFacingPositionFront;
		_walkData[_walkDataLastIndex].frameNum = 5;
		++_walkDataLastIndex;
	} else {
		_walkCurrentPos = kFacingPositionLeft;
	}
	WalkData *wd;
	uint8 scale, xScale, xSkip;
	float hStepCur, hStepDist;
	int curX = srcX;
	int curY = srcY;
	if (srcY > dstY) {
		int hStepsCount = getHorizontalStepsCount(dstX, dstY, srcX, srcY);
		if (hStepsCount > 0) {
			hStepCur = hStepDist = (srcY - dstY) / (float)hStepsCount;
			for (int i = 1; i <= hStepsCount; ++i) {
				wd = &_walkData[_walkDataLastIndex];
				scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
				xSkip = _walkScaleTable[0x769 + scale * 8 + _walkCurrentFrame];
				if ((curX - xSkip) <= dstX) {
					break;
				}
				curX -= xSkip;
				curY = srcY - truncReal(hStepCur);
				wd->setPos(curX, curY, 4, _walkCurrentFrame);
				scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
				xScale = _walkWidthScaleTable[scale - 1];
				wd->setScale(scale, scale);
				wd->yPosChanged = 0;
				wd->dyPos = _walkData[_walkDataLastIndex - 1].y - wd->y;
				int x = xScale - xScale / 2 + curX - 1;
				if (x > 319) {
					wd->clipSkipX = 1;
					wd->clipWidth = 319 - (x - xScale);
					wd->xPosChanged = 0;
					wd->dxPos = 0;
				} else {
					x = curX - xScale / 2;
					if (x < 0) {
						wd->clipWidth = x + xScale;
						wd->clipSkipX = xScale - wd->clipWidth + 1;
						wd->xPosChanged = 0;
						wd->dxPos = xSkip;
					} else {
						wd->clipWidth = xScale;
						wd->clipSkipX = 1;
						wd->xPosChanged = 0;
						wd->dxPos = xSkip;
					}
				}
				++_walkDataLastIndex;
				hStepCur += hStepDist;
				WalkData::setNextFrame(kFacingPositionLeft, _walkCurrentFrame);
			}
		} else {
			if (_walkCurrentPosLocalVar == 4 && _walkToObjectPosX == dstX && _walkToObjectPosY == dstY) {
				_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
				_walkData[_walkDataLastIndex].posNum = 4;
				_walkData[_walkDataLastIndex].frameNum = 1;
				++_walkDataLastIndex;
			}
		}
		wd = &_walkData[_walkDataLastIndex];
		wd->setPos(dstX, dstY, 4, _walkCurrentFrame);
		scale = _walkYScaleRoom[(_walkXScaleRoom[dstX] - 1) * 144 + dstY];
		xScale = _walkWidthScaleTable[scale - 1];
		wd->setScale(scale, scale);
		wd->yPosChanged = 0;
		wd->dyPos = curY - dstY;
		int x = xScale - xScale / 2 + dstX - 1;
		if (x > 319) {
			wd->clipSkipX = 1;
			wd->clipWidth = 319 - (x - xScale);
			wd->xPosChanged = 0;
			wd->dxPos = 0;
		} else {
			x = dstX - xScale / 2;
			if (x < 0) {
				wd->clipWidth = xScale + x;
				wd->clipSkipX = xScale - wd->clipWidth + 1;
				wd->xPosChanged = 0;
				wd->dxPos = curX - dstX;
			} else {
				wd->clipWidth = xScale;
				wd->clipSkipX = 1;
				wd->xPosChanged = 0;
				wd->dxPos = curX - dstX;
			}
		}
		++_walkDataLastIndex;
		WalkData::setNextFrame(kFacingPositionLeft, _walkCurrentFrame);
	} else {
		int hStepsCount = getHorizontalStepsCount(dstX, srcY, srcX, dstY);
		if (hStepsCount > 0) {
			hStepCur = hStepDist = (dstY - srcY) / (float)hStepsCount;
			for (int i = 1; i <= hStepsCount; ++i) {
				wd = &_walkData[_walkDataLastIndex];
				scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
				xSkip = _walkScaleTable[0x769 + scale * 8 + _walkCurrentFrame];
				if (curX - xSkip <= dstX) {
					break;
				}
				curX -= xSkip;
				curY = srcY + truncReal(hStepCur);
				wd->setPos(curX, curY, 4, _walkCurrentFrame);
				scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
				xScale = _walkWidthScaleTable[scale - 1];
				wd->setScale(scale, scale);
				wd->yPosChanged = 1;
				wd->dyPos = wd->y - _walkData[_walkDataLastIndex - 1].y;
				int x = xScale - xScale / 2 + curX - 1;
				if (x > 319) {
					wd->clipSkipX = 1;
					wd->clipWidth = 319 - (x - xScale);
					wd->xPosChanged = 0;
					wd->dxPos = 0;
				} else {
					x = curX - xScale / 2;
					if (x < 0) {
						wd->clipWidth = x + xScale;
						wd->clipSkipX = xScale - wd->clipWidth + 1;
						wd->xPosChanged = 0;
						wd->dxPos = xSkip;
					} else {
						wd->clipWidth = xScale;
						wd->clipSkipX = 1;
						wd->xPosChanged = 0;
						wd->dxPos = xSkip;
					}
				}
				++_walkDataLastIndex;
				hStepCur += hStepDist;
				WalkData::setNextFrame(kFacingPositionLeft, _walkCurrentFrame);
			}
		} else {
			if (_walkCurrentPosLocalVar == 4 && _walkToObjectPosX == dstX && _walkToObjectPosY == dstY) {
				_walkData[_walkDataLastIndex] = _walkData[_walkDataLastIndex - 1];
				_walkData[_walkDataLastIndex].posNum = 4;
				_walkData[_walkDataLastIndex].frameNum = 1;
				++_walkDataLastIndex;
			}
		}
		wd = &_walkData[_walkDataLastIndex];
		wd->setPos(dstX, dstY, 4, _walkCurrentFrame);
		scale = _walkYScaleRoom[(_walkXScaleRoom[curX] - 1) * 144 + curY];
		xScale = _walkWidthScaleTable[scale - 1];
		wd->setScale(scale, scale);
		wd->yPosChanged = 1;
		wd->dyPos = curY - dstY;
		int x = xScale - xScale / 2 + dstX - 1;
		if (x > 319) {
			wd->clipSkipX = 1;
			wd->clipWidth = 319 - (x - xScale);
			wd->xPosChanged = 0;
			wd->dxPos = 0;
		} else {
			x = dstX - xScale / 2;
			if (x < 0) {
				wd->clipWidth = xScale + x;
				wd->clipSkipX = xScale - wd->clipWidth + 1;
				wd->xPosChanged = 0;
				wd->dxPos = curX - dstX;
			} else {
				wd->clipWidth = xScale;
				wd->clipSkipX = 1;
				wd->xPosChanged = 0;
				wd->dxPos = curX - dstX;
			}
		}
		++_walkDataLastIndex;
		WalkData::setNextFrame(kFacingPositionLeft, _walkCurrentFrame);
	}
}

void IgorEngine::waitForIgorMove() {
	_gameTicks = 0;
	do {
		if (compareGameTick(1, 16)) {
			if (_walkDataCurrentIndex > _walkDataLastIndex) {
				_gameState.igorMoving = false;
				_walkDataLastIndex = _walkDataCurrentIndex;
			}
			if (_gameState.igorMoving) {
				moveIgor(_walkData[_walkDataCurrentIndex].posNum, _walkData[_walkDataCurrentIndex].frameNum);
				++_walkDataCurrentIndex;
			}
		}
		// if (_updateRoomBackground) {
		// 	(this->*_updateRoomBackground)();
		// }
		waitForTimer();
	} while (_gameState.igorMoving);
}

void IgorEngine::handleRoomIgorWalk() {
	if (_walkDataCurrentIndex > _walkDataLastIndex) {
		_gameState.igorMoving = false;
		_walkDataLastIndex = _walkDataCurrentIndex;
		// if (_actionCode > 0) {
		// 	hideCursor();
		// 	executeAction(_actionCode);
		// 	if (!_gameState.dialogueTextRunning) {
		// 		showCursor();
		// 	}
		// 	clearAction();
		// }
	}
	if (_gameState.igorMoving) {
		moveIgor(_walkData[_walkDataCurrentIndex].posNum, _walkData[_walkDataCurrentIndex].frameNum);
		++_walkDataCurrentIndex;
	}
}

} // End of namespace Igor
