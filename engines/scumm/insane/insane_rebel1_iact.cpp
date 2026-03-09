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

#include "common/system.h"
#include "common/endian.h"

#include "scumm/scumm_v7.h"
#include "scumm/insane/insane_rebel1.h"

namespace Scumm {

// LVL1 stage-2 0x5D damage/event codes. The gameplay stream exposes low record ids
// (6..18), while the recovered outer loop compares the post-latch state against the
// later translated values seen in the executable. Accept both representations.
static inline bool isL1Stage2DamageLatch(uint16 code) {
	switch (code) {
	case 0x0006:
	case 0x0007:
	case 0x0008:
	case 0x0009:
	case 0x000A:
	case 0x000B:
	case 0x000C:
	case 0x000D:
	case 0x000E:
	case 0x000F:
	case 0x0010:
	case 0x0011:
	case 0x0012:
	case 0x0049:
	case 0x004B:
	case 0x004E:
	case 0x0051:
	case 0x0054:
	case 0x005C:
	case 0x005E:
	case 0x0060:
	case 0x0062:
	case 0x0064:
		return true;
	default:
		return false;
	}
}

static inline bool isL1Stage2SweepDamage(uint16 frameCounter, int16 perspectiveX) {
	switch (frameCounter) {
	case 0x0034:
	case 0x00ED:
	case 0x0173:
		return perspectiveX <= 0x28;
	case 0x0088:
	case 0x00FA:
	case 0x0151:
		return perspectiveX >= 0x18;
	default:
		return false;
	}
}

static const int16 kLevel7BranchFrames[6][6] = {
	{ -1,  78, 267, 398, 556, 630 },
	{ -1, 187, 376, 507, 665, 739 },
	{ -1, 187, 376, 507, 665, 739 },
	{ -1,  -1,  -1, 284, 442, 516 },
	{ -1,  -1,  -1, 143, 301, 375 },
	{ -1, 112, 301, 432, 590, 664 }
};

static const int16 kLevel7BranchDir[6] = {
	0, 1, 1, -1, 1, 1
};

static const int16 kLevel7BranchThreshold[6] = {
	0, 170, 170, 160, 160, 160
};

static const int16 kLevel8BranchFrames[3][3] = {
	{ 2588, 1709,  262 },
	{ 2323, 1444,   -2 },
	{  877,   -2,   -2 }
};

static inline bool isLevel4DamageLatch(uint16 code) {
	switch (code) {
	case 0x0008:
	case 0x000A:
	case 0x000C:
	case 0x0010:
	case 0x0018:
	case 0x001C:
	case 0x001E:
	case 0x0024:
	case 0x0026:
	case 0x0028:
	case 0x002D:
		return true;
	default:
		return false;
	}
}

static inline bool isLevel6DamageLatch(uint16 code) {
	switch (code) {
	case 0x0003:
	case 0x0008:
	case 0x0009:
	case 0x000D:
	case 0x000F:
	case 0x0028:
		return true;
	default:
		return false;
	}
}

static inline bool isLevel10DamageLatch(uint16 code) {
	if (code < 0x7F) {
		if (code > 0x3D) {
			if (code > 0x3E) {
				if (code < 0x5C) {
					if (code < 0x54) {
						if (code < 0x44)
							return false;
						if (code > 0x44)
							return code == 0x4F;
					} else if (code > 0x54) {
						if (code < 0x58)
							return code == 0x56;
						if (code < 0x59)
							return true;
						return code == 0x5A;
					}
				} else if (code > 0x5C) {
					if (code < 0x73) {
						if (code < 0x6A)
							return false;
						if (code > 0x6A)
							return code == 0x6F;
					} else if (code > 0x73) {
						if (code < 0x77)
							return code == 0x75;
						if (code < 0x78)
							return true;
						return code == 0x7B;
					}
				}
			}
			return true;
		}

		if (code > 0x1F) {
			if (code > 0x20) {
				if (code < 0x2E) {
					if (code < 0x22)
						return false;
					if (code > 0x22)
						return code == 0x24;
				} else if (code > 0x2E) {
					if (code < 0x35)
						return code == 0x32;
					if (code < 0x36)
						return false;
					return code == 0x3C;
				}
			}
			return true;
		}

		if (code > 0x0E) {
			if (code > 0x0F) {
				if (code < 0x14)
					return false;
				if (code > 0x14)
					return code == 0x1A;
			}
			return true;
		}

		return code > 3 && (code < 5 || code == 6);
	}

	if (code > 0x7F) {
		if (code < 0xC7) {
			if (code < 0xA2) {
				if (code < 0x95) {
					if (code < 0x83)
						return false;
					if (code > 0x83)
						return code == 0x90;
				} else if (code > 0x95) {
					if (code < 0x9E)
						return false;
					if (code > 0x9E)
						return code == 0xA0;
				}
			} else if (code > 0xA2) {
				if (code < 0xB7) {
					if (code < 0xA5)
						return false;
					if (code > 0xA5)
						return code == 0xAD;
				} else if (code > 0xB7) {
					if (code < 0xBB)
						return code == 0xB9;
					if (code < 0xBC)
						return true;
					return code == 0xBE;
				}
			}
			return true;
		}

		if (code > 0xC7) {
			if (code < 0xE6) {
				if (code < 0xD7) {
					if (code < 0xC9)
						return false;
					if (code > 0xC9)
						return code == 0xCF;
				} else if (code > 0xD9) {
					if (code < 0xDD)
						return code == 0xDB;
					if (code < 0xDE)
						return true;
					return code == 0xDF;
				}
			} else if (code > 0xE6) {
				if (code < 0xF5) {
					if (code < 0xF0)
						return false;
					if (code > 0xF0)
						return code == 0xF3;
				} else if (code > 0xF5) {
					if (code < 0xFF)
						return code == 0xFD;
					if (code < 0x100)
						return true;
					return code == 0x103;
				}
			}
		}
	}

	return true;
}

static inline bool hasLevel6PerspectiveHazard(uint16 frame, int16 perspectiveX, int16 perspectiveY) {
	switch (frame) {
	case 0x006A:
	case 0x00FD:
	case 0x011C:
	case 0x0563:
		return perspectiveX < 0x18;
	case 0x0144:
		return perspectiveX < 0x29 && perspectiveY < 0x20;
	case 0x016F:
	case 0x0222:
	case 0x02EB:
		return perspectiveX < 0x29;
	case 0x01DE:
	case 0x0492:
		return perspectiveX < 8;
	case 0x024A:
		return perspectiveY < 0x27;
	case 0x0318:
		return perspectiveY < 0x0F;
	case 0x0397:
		return perspectiveX < 0x18 && perspectiveY < 0x20;
	case 0x0405:
		return perspectiveX > 0x28;
	case 0x0462:
		return perspectiveY < 0x20;
	case 0x04F1:
		return perspectiveX < 0x29 && perspectiveY >= 0x0F;
	case 0x04FF:
		return perspectiveX < 0x29 && perspectiveY < 0x20;
	case 0x0617:
		return !(perspectiveX > 7 && perspectiveX < 0x29 &&
			perspectiveY > 0x0E && perspectiveY < 0x20);
	default:
		return false;
	}
}

static inline bool hasLevel8PerspectiveHazardRoute0(uint16 frame, int16 perspectiveX, int16 perspectiveY) {
	switch (frame) {
	case 0x00CD:
		return perspectiveX < 0x29;
	case 0x00EF:
		return perspectiveY < 0x0F;
	case 0x0294:
	case 0x04BE:
	case 0x076C:
		return perspectiveX < 0x29 && perspectiveY < 0x20;
	case 0x03A2:
		return perspectiveX < 0x18;
	case 0x05C9:
	case 0x085A:
	case 0x096F:
		return perspectiveY < 0x20;
	default:
		return false;
	}
}

static inline bool hasLevel8PerspectiveHazardRoute1(uint16 frame, int16 perspectiveX, int16 perspectiveY) {
	switch (frame) {
	case 0x0189:
		return perspectiveY < 0x0F;
	case 0x0297:
		return perspectiveX < 0x18;
	case 0x03B3:
	case 0x0661:
		return perspectiveX < 0x29 && perspectiveY < 0x20;
	case 0x04BE:
	case 0x074F:
	case 0x0864:
		return perspectiveY < 0x20;
	default:
		return false;
	}
}

static inline bool hasLevel8PerspectiveHazardRoute2(uint16 frame, int16 perspectiveX, int16 perspectiveY) {
	switch (frame) {
	case 0x00BB:
		return perspectiveX < 0x29 && perspectiveY < 0x20;
	case 0x01A9:
	case 0x02BE:
		return perspectiveY < 0x20;
	default:
		return false;
	}
}

void InsaneRebel1::resetFrameObjectState() {
	memset(_frameObjectState, 0, sizeof(_frameObjectState));
	for (int i = 0x50; i < 0x96; i++)
		_frameObjectState[i] = 0xFF;
}

void InsaneRebel1::updateGostSlotPosition(int16 targetIdx, int16 left, int16 top, int16 right, int16 bottom) {
	const int16 targetKey = targetIdx + 1;
	for (int i = 0; i < kMaxGostSlots; i++) {
		if (_gostSlots[i].targetId == targetKey && _gostSlots[i].frame < 10) {
			_gostSlots[i].posX = (left + right) / 2;
			_gostSlots[i].posY = (top + bottom) / 2;
		}
	}
}

void InsaneRebel1::applyFrameObjectHitState(int16 targetIdx) {
	if (targetIdx < 0)
		return;

	const int byteIndex = targetIdx >> 3;
	if (byteIndex < 0 || byteIndex >= 0x96 || byteIndex >= kFrameObjectStateBytes)
		return;

	const byte bit = (byte)(0x80 >> (targetIdx & 7));
	const int altIndex = byteIndex + 0x96;
	if (altIndex >= kFrameObjectStateBytes)
		return;

	if ((_frameObjectState[altIndex] & bit) == 0)
		_frameObjectState[byteIndex] |= bit;
	else
		_frameObjectState[altIndex] &= ~bit;
}

bool InsaneRebel1::handleFrameObjectTarget(int16 objectId, int16 left, int16 top, int16 width, int16 height,
		int codec, uint8 &ra1Param) {
	if (!_interactiveVideoActive)
		return true;

	int absObjectId = (objectId < 0) ? -objectId : objectId;
	if (absObjectId == 0)
		return true;

	const int bitIndex = absObjectId - 1;
	const int byteIndex = bitIndex >> 3;
	if (byteIndex < 0 || byteIndex >= 0x96 || byteIndex >= kFrameObjectStateBytes)
		return true;

	const byte bit = (byte)(0x80 >> (bitIndex & 7));
	const int altIndex = byteIndex + 0x96;
	const bool primarySet = (_frameObjectState[byteIndex] & bit) != 0;
	const bool secondarySet = (altIndex < kFrameObjectStateBytes) && ((_frameObjectState[altIndex] & bit) != 0);
	const int16 right = left + width;
	const int16 bottom = top + height;

	if (objectId > 0 && objectId < 0x280) {
		if (!primarySet || secondarySet)
			checkTargetHit(objectId - 1, left, top, right, bottom);
		else
			updateGostSlotPosition(objectId - 1, left, top, right, bottom);
	}

	const bool updatedPrimarySet = (_frameObjectState[byteIndex] & bit) != 0;
	const bool updatedSecondarySet = (altIndex < kFrameObjectStateBytes) && ((_frameObjectState[altIndex] & bit) != 0);
	if (updatedPrimarySet)
		return false;

	if (updatedSecondarySet && codec == 0x17)
		ra1Param = (uint8)(ra1Param - 0x10);

	return true;
}

void InsaneRebel1::rebuildProjectionTable(int16 curveStep, int16 curveExtent) {
	_projectionCurveExtent = curveExtent;

	int step = curveStep >> 1;
	int accum = 0;
	int threshold = 0;
	int value = step;

	for (int i = 0; i < kProjectionTableSize / 2; i++) {
		if (step < 0) {
			accum -= step;
			if (threshold < accum) {
				threshold += kProjectionTableSize / 2;
				value += 1;
			}
		} else {
			accum += step;
			if (threshold < accum) {
				threshold += kProjectionTableSize / 2;
				value -= 1;
			}
		}

		_projectionTable[i] = (int16)value;
		_projectionTable[kProjectionTableSize - 1 - i] = (int16)-value;
	}
}

void InsaneRebel1::resetProjectionTable() {
	rebuildProjectionTable(0, 1);
}

void InsaneRebel1::checkDynamicLevelBranch() {
	if (!_interactiveVideoActive || _levelRouteIndex < 0 || _pendingRouteIndex >= 0 || _vm->_smushVideoShouldFinish)
		return;

	if (_currentLevel == 6) {
		const int route = CLIP<int>(_levelRouteIndex, 0, 5);
		for (int nextRoute = 1; nextRoute < 6; ++nextRoute) {
			const int triggerFrame = kLevel7BranchFrames[route][nextRoute];
			if (triggerFrame <= 0 || nextRoute == route || _frameCounter != (uint32)(triggerFrame - 1))
				continue;

			const bool takeBranch = (kLevel7BranchDir[nextRoute] > 0)
				? (_shipPosX > kLevel7BranchThreshold[nextRoute])
				: (_shipPosX < kLevel7BranchThreshold[nextRoute]);
			if (!takeBranch)
				continue;

			_pendingRouteIndex = nextRoute;
			_vm->_smushVideoShouldFinish = true;
			debug(1, "RA1 L7 branch: route=%d -> %d at frame=%u shipX=%d",
				route, nextRoute, (unsigned)_frameCounter, _shipPosX);
			return;
		}
	}

	if (_currentLevel == 7) {
		const int route = CLIP<int>(_levelRouteIndex, 0, 2);
		const int frame = (int)_frameCounter;
		const int leftBlockedFrame = kLevel8BranchFrames[route][2];
		const int rightBlockedFrame = kLevel8BranchFrames[route][1];
		const bool shotEdge = _playerFired && _fireCooldown == 0;
		int nextRoute = -1;

		for (int i = 0; i < 3; ++i) {
			const int triggerFrame = kLevel8BranchFrames[route][i];
			if (triggerFrame < 0)
				continue;

			if (shotEdge && frame > triggerFrame - 0x32 && frame <= triggerFrame)
				_levelRouteChoice = (_shipPosX < kRA1CenterX) ? 1 : 2;

			if (frame != triggerFrame)
				continue;

			const bool chooseLeft = (_levelRouteChoice == 1) ||
				((_shipPosX < kRA1CenterX) && (_levelRouteChoice != 2));
			if (chooseLeft) {
				if (frame != leftBlockedFrame)
					nextRoute = 1;
			} else {
				if (frame != rightBlockedFrame)
					nextRoute = 2;
			}
			_levelRouteChoice = 0;
			break;
		}

		if (nextRoute >= 0 && nextRoute != route) {
			_pendingRouteIndex = nextRoute;
			_vm->_smushVideoShouldFinish = true;
			debug(1, "RA1 L8 branch: route=%d -> %d at frame=%u shipX=%d",
				route, nextRoute, (unsigned)_frameCounter, _shipPosX);
		}
	}
}

void InsaneRebel1::projectGameplayPoint(int16 &x, int16 &y) const {
	x = (int16)(x - _perspectiveX);

	int curveIndex = 0x4F - (x >> 2);
	curveIndex = CLIP<int>(curveIndex, 0, kProjectionTableSize - 1);

	const int yCompensation = (_perspectiveY + (_projectionCurveExtent >> 1)) - _projectionTable[curveIndex];
	y = (int16)(y - yCompensation);
}

void InsaneRebel1::unprojectGameplayPoint(int16 &x, int16 &y) const {
	int curveIndex = 0x4F - (x >> 2);
	curveIndex = CLIP<int>(curveIndex, 0, kProjectionTableSize - 1);

	const int yCompensation = (_perspectiveY + (_projectionCurveExtent >> 1)) - _projectionTable[curveIndex];
	y = (int16)(y + yCompensation);
	x = (int16)(x + _perspectiveX);
}

// preprocessMouseAxes — FUN_231BE (0x231BE) centered-axis output law, adapted to
// ScummVM's absolute 320x200 mouse space.
// Preserve the DOS bias/offset persistence and one-frame jump latch from
// FUN_231BE, but avoid hard recentring the host mouse into the DOS safe window.
// The actual frame-averaging behavior stays untouched.
void InsaneRebel1::preprocessMouseAxes(int16 &inputX, int16 &inputY) {
	if (_mouseRecentering)
		return;

	int16 logicalX = (int16)CLIP<int>(_vm->_mouse.x, 0, 319);
	int16 logicalY = (int16)CLIP<int>(_vm->_mouse.y, 0, 199);
	const int16 rawX = (int16)(logicalX << 1);
	const int16 rawY = logicalY;
	const int16 deltaX = (int16)(logicalX - kRA1CenterX);
	const int16 deltaY = (int16)(logicalY - kRA1CenterY);
	const int16 normX = (int16)(((int32)deltaX * 127) / 160);
	const int16 normY = (int16)(((int32)deltaY * 127) / 100);
	int16 biasX = (int16)((rawX + _mouseOffsetX - 0x140) >> 2);
	int16 biasY = (int16)((rawY + _mouseOffsetY - 100) >> 1);

	if (biasY < 0x65) {
		const bool largeJump =
			(_mousePrevBiasX + 0x14 < biasX) ||
			(_mousePrevBiasY + 0x14 < biasY) ||
			(biasX < _mousePrevBiasX - 0x14) ||
			(biasY < _mousePrevBiasY - 0x14);
		if (largeJump) {
			if (!_mouseBiasLatch) {
				biasX = _mousePrevBiasX;
				biasY = _mousePrevBiasY;
				_mouseBiasLatch = true;
			}
		} else {
			_mouseBiasLatch = false;
		}
	} else {
		biasX = _mousePrevBiasX;
		biasY = _mousePrevBiasY;
		_mouseBiasLatch = true;
	}

	const int16 scaledX = (int16)(normX + biasX);
	const int16 scaledY = (int16)(normY + biasY);

	_mouseBiasX = biasX;
	_mouseBiasY = biasY;
	_mousePrevBiasX = biasX;
	_mousePrevBiasY = biasY;

	int accumX = rawX + _mouseOffsetX;
	if (accumX < 0xC0)
		_mouseOffsetX = (int16)(0xC0 - rawX);
	else if (accumX > 0x1C0)
		_mouseOffsetX = (int16)(0x1C0 - rawX);

	int accumY = rawY + _mouseOffsetY;
	if (accumY < -0x1C)
		_mouseOffsetY = (int16)(-0x1C - rawY);
	else if (accumY > 0xE4)
		_mouseOffsetY = (int16)(0xE4 - rawY);

	accumX = rawX + _mouseOffsetX;
	if (accumX < 0x145) {
		if (accumX < 0x142) {
			if (accumX < 0x13C)
				_mouseOffsetX += 4;
			else if (accumX < 0x13F)
				_mouseOffsetX += 1;
		} else {
			_mouseOffsetX -= 1;
		}
	} else {
		_mouseOffsetX -= 4;
	}

	accumY = rawY + _mouseOffsetY;
	if (accumY < 0x69) {
		if (accumY < 0x66) {
			if (accumY < 0x60)
				_mouseOffsetY += 4;
			else if (accumY < 99)
				_mouseOffsetY += 1;
		} else {
			_mouseOffsetY -= 1;
		}
	} else {
		_mouseOffsetY -= 4;
	}

	inputX = CLIP<int16>(scaledX, -0xA0, 0xA0);
	inputY = CLIP<int16>(scaledY, -127, 127);
}

// updateShipPhysics — FUN_1DEB5 (0x1DEB5). Accumulator-based position system.
// Roll accumulator (_74CA) driven by input, position accumulators (_74C2/_74C6)
// driven by roll + drift + cross-coupling. Ship position = base + accum >> 8.
void InsaneRebel1::updateShipPhysics() {
	_frameCounter++;

	// HandleGameOp07_ShipFlight resets the ship accumulators and camera when
	// the GAME 0x07 frame counter enters at 0. Level 1 happened to work because
	// its runlevel code pre-initialized the same state, but later 0x07-driven
	// stages like L3 rely on the handler to do this reset itself.
	if (_gameCounter == 0) {
		_posAccumX = 0;
		_posAccumY = 0;
		_rollAccum = 0;
		_liftSmooth = 0;
		_shipPosX = kRA1CenterX;
		_shipPosY = kRA1CenterY;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_perspectiveX = 0x20;
		_perspectiveY = 0x17;
		resetProjectionTable();
	}

	// Decrement cooldown
	if (_damageCooldown > 0)
		_damageCooldown--;

	// --- Step 1: Gameplay axes from FUN_231BE ---
	// HandleGameOp07_ShipFlight consumes the preprocessed axes in DAT_756C/756E,
	// not raw mouse coordinates. Reuse the same centered-axis law here.
	int16 inputX = 0;
	int16 inputY = 0;
	preprocessMouseAxes(inputX, inputY);
	inputX = CLIP<int16>(inputX, -127, 127);
	inputY = CLIP<int16>(inputY, -127, 127);

	// --- Step 2: Roll accumulator (_74CA) ---
	// Normal mode: accumulate; mode 0x10: snap to input
	_rollAccum += (_tuning.roll * (int32)inputX) >> 5;
	_rollAccum = CLIP<int32>(_rollAccum, -0x47F, 0x47F);

	// --- Step 3: Vertical smoothing (_74CE) ---
	// Exponential decay toward -inputY
	_liftSmooth += (-_liftSmooth - (int32)inputY) >> 1;
	_liftSmooth = CLIP<int32>(_liftSmooth, -0x20, 0x20);

	// --- Step 4: Position accumulator deltas ---
	// X delta: drift + slide coupling - cross-coupling
	int32 rng = _turbulenceEnabled ? (int32)_vm->_rnd.getRandomNumber(199) : 100;  // 0-199, centered at 100
	int32 crossTermX;
	if (_liftSmooth < 0)
		crossTermX = ((int32)_tuning.lift * _liftSmooth * _rollAccum) >> 11;
	else
		crossTermX = ((int32)_tuning.lift * _liftSmooth * _rollAccum) >> 12;

	int32 deltaX = (((rng - 100) - (int32)_tuning.drift * _driftParam) >> 1)
	             + (((int32)_tuning.slide * _rollAccum) >> 7)
	             - crossTermX;

	// Y delta: roll magnitude + lift cross-coupling
	int32 absRoll = ABS(_rollAccum);
	int32 crossTermY;
	if (_liftSmooth < 0)
		crossTermY = ((int32)_tuning.lift * (0x7DE - absRoll) * _liftSmooth) >> 12;
	else
		crossTermY = ((int32)_tuning.lift * (0x7DE - absRoll) * _liftSmooth) >> 13;

	int32 deltaY = (absRoll >> 1) + crossTermY;

	// --- Step 5: Update position accumulators ---
	_posAccumX += deltaX;
	_posAccumX = CLIP<int32>(_posAccumX, -0x8200, 0x8200);
	_posAccumY += deltaY;
	_posAccumY = CLIP<int32>(_posAccumY, -0x3200, 0x4600);

	// --- Step 6: Derive pixel position from accumulators ---
	// Original: _74BA = _74C2 >> 8, _74BC = _74C6 >> 8
	// Ship position = base + offset
	_shipPosX = kRA1CenterX + (int16)(_posAccumX >> 8);
	_shipPosY = kRA1CenterY + (int16)(_posAccumY >> 8);

	// Clamp to screen bounds
	_shipPosX = CLIP<int16>(_shipPosX, kRA1MinX, kRA1MaxX);
	_shipPosY = CLIP<int16>(_shipPosY, kRA1MinY, kRA1MaxY);

	// --- Step 7: Corridor collision (FUN_1C54D) ---
	// Wall contact forces position accumulators to corridor edge and sets
	// damage flags. Flag bit 0x10 (zone hit) suppresses damage bits only.
	{
		bool hasZoneHit = (_damageFlags & 0x10) != 0;

			if (_shipPosX > _corridorRightX) {
				_posAccumX = (int32)(_corridorRightX - kRA1CenterX) * 0x100;
				_shipPosX = _corridorRightX;
				if (!hasZoneHit) {
					if (_rollAccum > -0x100)
					_rollAccum = -0x100;  // Push left
				_damageFlags |= 0x02;  // Right wall
			}
			}
			if (_shipPosX < _corridorLeftX) {
				_posAccumX = (int32)(_corridorLeftX - kRA1CenterX) * 0x100;
				_shipPosX = _corridorLeftX;
				if (!hasZoneHit) {
					if (_rollAccum < 0x100)
					_rollAccum = 0x100;   // Push right
				_damageFlags |= 0x04;  // Left wall
			}
			}
			if (_shipPosY < _corridorTopY) {
				_posAccumY = (int32)(_corridorTopY - kRA1CenterY) * 0x100 + 0x100;
				_shipPosY = _corridorTopY;
				if (!hasZoneHit)
					_damageFlags |= 0x01;
			}
			if (_shipPosY > _corridorBottomY) {
				_posAccumY = (int32)(_corridorBottomY - kRA1CenterY) * 0x100 - 0x100;
				_shipPosY = _corridorBottomY;
				if (!hasZoneHit)
					_damageFlags |= 0x08;
		}
	}

	// --- Step 8: Perspective offsets ---
	// FUN_1DEB5 computes these linearly from ship offsets:
	//   viewX = clamp((_74BA + 0x20), 0, 0x40)
	//   viewY = clamp((_74BC + 0x17), 0, 0x2E)
	_perspectiveX = CLIP<int16>((int16)(_shipPosX - kRA1CenterX + 0x20), 0, 0x40);
	_perspectiveY = CLIP<int16>((int16)(_shipPosY - kRA1CenterY + 0x17), 0, 0x2E);

	// FUN_1DEB5 updates the curve table via FUN_22549 after SetCameraOffset.
	// The full DOS path blends a few roll-history terms; use the current roll
	// accumulator so side-looking still bends the gameplay projection.
	rebuildProjectionTable(CLIP<int16>((int16)(-(_rollAccum >> 7)), -0x1A, 0x1A), 0x1A);

	// --- Step 9: Direction sprite index (FUN_1DEB5 LAB_1e23e) ---
	// Horizontal component from _74CA (rollAccum):
	//   |rollAccum| <= 0x80: center (0)
	//   rollAccum > 0x80:  ((rollAccum - 0x80) >> 8) * 5 + 5   (right: 5,10,15,20)
	//   rollAccum < -0x80: ((abs(rollAccum) - 0x80) >> 8) * 5 + 25 (left: 25,30,35,40)
	int hComponent;
	if (_rollAccum > 0x80) {
		hComponent = ((_rollAccum - 0x80) >> 8) * 5 + 5;
	} else if (_rollAccum < -0x80) {
		hComponent = ((-_rollAccum - 0x80) >> 8) * 5 + 25;
	} else {
		hComponent = 0;
	}

	// Vertical component from _74CE (liftSmooth):
	//   (_74CE + 0x20) * 5 / 0x41  → 0..4  (5 rows)
	int vComponent = (_liftSmooth + 0x20) * 5 / 0x41;

	if (_shipBank.numSprites > 0)
		_shipDirIndex = CLIP<int16>((int16)(vComponent + hComponent), 0, _shipBank.numSprites - 1);

	// --- Step 10: Damage/event bit synthesis + damage processing ---
	// RA1 FUN_1B297-style latches from GAME opcodes:
	//   0x5D latch 0xFFFF -> bit 0x40 (obstacle/contact)
	//   0x5F non-zero + RNG -> bit 0x80 (projectile-like hit)
	if (_gameLatch5D == 0xFFFF || (_currentLevel == 0 && _flyControlMode == 2 &&
		isL1Stage2DamageLatch(_gameLatch5D)))
		_damageFlags |= 0x40;
	if (_gameLatch5F != 0 && _vm->_rnd.getRandomNumber((uint16)(_gameLatch5F - 1)) == 0)
		_damageFlags |= 0x80;

	// Damage guard/mask from FUN_1DEB5: (_damageFlags & 0x96) != 0
	// damageFlags & 0x96 = bits 1,2,4,7 = wall collisions (0x16) + projectile hit (0x80)
	if ((_damageFlags & 0x96) != 0 && _damageCooldown == 0 &&
		_health >= 0 && _deathTimer <= 0) {
		// Projectile hit (bit 7 = 0x80)
		if (_damageFlags & 0x80)
			_health -= _tuning.shot;
		// Wall collision (bits 1,2,4 = 0x16)
		if (_damageFlags & 0x16)
			_health -= _tuning.wham;

		if (_health < 0)
			_deathTimer = kDeathTimerInit;

		_prevDamageFlags = _damageFlags;
		_damageCooldown = kDamageCooldownInit;
		_screenFlash = 3;
	}

	// Latches are per-frame event inputs in the original pipeline.
	_gameLatch5D = 0;
	_gameLatch5F = 0;

	// Death animation countdown
	if (_health < 0 && _deathTimer > 0)
		_deathTimer--;

	// Health regeneration + survival bonus every 32 frames (UpdatePeriodicScoreAndHealth)
	if ((_frameCounter & 0x1F) == 0) {
		if (_health >= 0 && _health < kMaxHealth)
			_health++;
		if (_health >= 0)
			_score += _tuning.time;
	}

	// Screen flash decay
	if (_screenFlash > 0)
		_screenFlash--;

	// Clear per-frame damage flags
	_damageFlags = 0;

	// --- Path branching detection ---
	// Original (FUN_1B297): at GAME counter 394 (0x18A), sets nextSceneA=0x67/nextSceneB=0x69.
	// After this point, drift goes strongly negative (pushing ship left for the hard path).
	// If ship is right of center, player chose the right/easy path → switch to L1PLAY1R.
	// Keep this as a one-shot decision: once threshold is reached, lock path.
	if (_pathBranchEnabled && _gameCounter >= kPathBranchCounter) {
		if (_shipPosX > kRA1CenterX) {
			_rightPathSelected = true;
			_vm->_smushVideoShouldFinish = true;
			debug(1, "RA1: Right path selected (counter=%d, shipX=%d)", _gameCounter, _shipPosX);
		} else {
			debug(1, "RA1: Left path retained (counter=%d, shipX=%d)", _gameCounter, _shipPosX);
		}
		_pathBranchEnabled = false;
	}

	checkDynamicLevelBranch();

	debug(7, "RA1 ship: pos=(%d,%d) roll=%d lift=%d accX=%d accY=%d dir=%d health=%d corridor=[%d,%d]-[%d,%d]",
		_shipPosX, _shipPosY, _rollAccum, _liftSmooth,
		_posAccumX, _posAccumY, _shipDirIndex, _health,
		_corridorLeftX, _corridorTopY, _corridorRightX, _corridorBottomY);
}

// updateTurretPhysics — FUN_1E6A7 (0x1E6A7), opcode 0x08 path.
// Stage-2 cockpit mode uses different smoothing/clamps than FUN_1DEB5.
void InsaneRebel1::updateTurretPhysics() {
	_frameCounter++;

	// FUN_1E6A7 consumes GAME field1 as frame counter (arg6 in dispatcher call).
	// The 0x10/0x40 gates come from dispatcher arg4 (callback control bits),
	// not from GAME payload fields.
	const int32 counter = _gameCounter;
	const uint16 modeFlags = _frameDispatchFlags;

	// RA1 latches consumed by handler family in FUN_1B297.
	if (_currentLevel == 0 && _flyControlMode == 2 && isL1Stage2SweepDamage((uint16)counter, _perspectiveX))
		_damageFlags |= 0x20;
	if (_gameLatch5D == 0xFFFF || (_currentLevel == 0 && _flyControlMode == 2 &&
		isL1Stage2DamageLatch(_gameLatch5D)))
		_damageFlags |= 0x40;
	if (_gameLatch5F != 0 &&
		((_currentLevel == 0 && _flyControlMode == 2)
			? (_vm->_rnd.getRandomNumber(2) == 0)
			: (_vm->_rnd.getRandomNumber((uint16)(_gameLatch5F - 1)) == 0)))
		_damageFlags |= 0x80;

	if (counter == 0) {
		_posAccumX = 0;
		_posAccumY = 0;
		_rollAccum = 0;
		_liftSmooth = 0;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
	}

	// Damage gate from FUN_1E6A7.
	if (_damageFlags != 0 && _damageCooldown == 0 && _health >= 0 && _deathTimer <= 0) {
		if (_damageFlags == 0x80)
			_health -= _tuning.shot;
		else
			_health -= _tuning.wham;

		if (_health < 0)
			_deathTimer = kDeathTimerInit;

		_prevDamageFlags = _damageFlags;
		_damageCooldown = kDamageCooldownInit;
		_screenFlash = 3;
	}

	if (_damageCooldown > 0)
		_damageCooldown--;

	if (_health < 0 && _deathTimer > 0)
		_deathTimer--;

	// FUN_1E6A7 movement gate: counter > 8 or flags bit 0x40.
	if (counter > 8 || (modeFlags & 0x40)) {
		// FUN_1E6A7 consumes DAT_756C/DAT_756E from the shared input bridge,
		// not raw mouse coordinates.
		int16 inputX = 0;
		int16 inputY = 0;
		preprocessMouseAxes(inputX, inputY);
		inputX = CLIP<int16>(inputX, -127, 127);
		inputY = CLIP<int16>(inputY, -127, 127);

		_rollAccum += (_tuning.roll * (int32)inputX) >> 4;
		_rollAccum = (_rollAccum * 3) >> 2;
		_rollAccum = CLIP<int32>(_rollAccum, -0x480, 0x480);

		_liftSmooth += (((int32)_liftSmooth - (int32)inputY) * (int32)_tuning.lift) >> 8;
		_liftSmooth = (_liftSmooth * 3) >> 2;
		_liftSmooth = CLIP<int32>(_liftSmooth, -0x32, 0x32);

		if ((modeFlags & 0x10) == 0) {
			_posAccumX += ((int32)_tuning.slide * _rollAccum) >> 6;
			_posAccumY -= ((int32)_liftSmooth * 64);
			_posAccumX = CLIP<int32>(_posAccumX, -0x8C00, 0x8C00);
			_posAccumY = CLIP<int32>(_posAccumY, -0x4600, 0x3C00);
		}
	}

	const int16 offsetX = (int16)(_posAccumX >> 8);
	const int16 offsetY = (int16)(_posAccumY >> 8);

	_perspectiveX = CLIP<int16>((int16)(offsetX + 0x20), 0, 0x40);
	_perspectiveY = CLIP<int16>((int16)(offsetY + 0x17), 0, 0x2E);

	// FUN_1E6A7 rebuilds the side-look curve with a shallower table than the
	// main flight handler, derived directly from roll.
	rebuildProjectionTable((int16)(-(_rollAccum >> 9)), 0x0D);

	// Direction bucket synthesis from FUN_1E6A7.
	int dir = 0;
	if (_flyControlMode == 2) {
		if (_rollAccum > 0x380) dir = 4;
		else if (_rollAccum > 0x280) dir = 3;
		else if (_rollAccum > 0x180) dir = 2;
		else if (_rollAccum > 0x80) dir = 1;
		else if (_rollAccum > -0x80) dir = 0;
		else if (_rollAccum > -0x180) dir = 5;
		else if (_rollAccum > -0x280) dir = 6;
		else if (_rollAccum > -0x380) dir = 7;
		else dir = 8;
	} else {
		if (_rollAccum > 0x380) dir = 8;
		else if (_rollAccum > 0x280) dir = 7;
		else if (_rollAccum > 0x180) dir = 6;
		else if (_rollAccum > 0x80) dir = 5;
		else if (_rollAccum > -0x80) dir = 4;
		else if (_rollAccum > -0x180) dir = 3;
		else if (_rollAccum > -0x280) dir = 2;
		else if (_rollAccum > -0x380) dir = 1;
		else dir = 0;

		if (offsetY < -0x1E)
			dir += 0x12;
		else if (offsetY < 0x1E)
			dir += 9;
	}
	const RA1SpriteBank *shipBank = &_shipBank;
	if (_currentLevel == 0 && _flyControlMode == 2 && _shipBankAlt.numSprites > 0)
		shipBank = &_shipBankAlt;
	if (shipBank->numSprites > 0)
		_shipDirIndex = CLIP<int16>((int16)dir, 0, shipBank->numSprites - 1);

	// Regeneration + survival bonus via FUN_1BB0E call in this path.
	if ((_frameCounter & 0x1F) == 0) {
		if (_health >= 0 && _health < kMaxHealth)
			_health++;
		if (_health >= 0)
			_score += _tuning.time;
	}

	if (_screenFlash > 0)
		_screenFlash--;

	_gameLatch5D = 0;
	_gameLatch5F = 0;
	_damageFlags = 0;
}

// updateAsteroidPhysics — FUN_1CDA7 (0x1CDA7). Opcode 0x0B handler.
// Uses 10-frame input history averaging instead of accumulators.
// Ship position = averaged input + center offset.
// Viewport = second history buffer for smooth camera scrolling.
void InsaneRebel1::updateAsteroidPhysics() {
	// Control feel tweak: original uses full 10-sample average in FUN_1CDA7.
	// We keep the same pipeline but average over fewer samples for responsiveness.
	const int kAsteroidSmoothWindow = 2;

	// RA1 FUN_1B297-style per-frame latches for 0x0B sections:
	//   0x5D latch 0xFFFF -> bit 0x40 (scripted obstacle/contact)
	//   0x5F non-zero + RNG -> bit 0x80 (scripted random hit)
	if (_gameLatch5D == 0xFFFF ||
		(_currentLevel == 3 && isLevel4DamageLatch(_gameLatch5D)) ||
		(_currentLevel == 5 && isLevel6DamageLatch(_gameLatch5D)) ||
		(_currentLevel == 9 && isLevel10DamageLatch(_gameLatch5D)))
		_damageFlags |= 0x40;
	if (_gameLatch5F != 0 &&
		((_currentLevel == 3 || _currentLevel == 9)
			? (_vm->_rnd.getRandomNumber(2) == 0)
			: (_vm->_rnd.getRandomNumber((uint16)(_gameLatch5F - 1)) == 0)))
		_damageFlags |= 0x80;

	if (_currentLevel == 5 && hasLevel6PerspectiveHazard((uint16)_frameCounter, _perspectiveX, _perspectiveY))
		_damageFlags |= 0x20;

	if (_currentLevel == 7) {
		bool walkerHazard = false;
		switch (CLIP<int>(_levelRouteIndex, 0, 2)) {
		case 0:
			walkerHazard = hasLevel8PerspectiveHazardRoute0((uint16)_frameCounter, _perspectiveX, _perspectiveY);
			break;
		case 1:
			walkerHazard = hasLevel8PerspectiveHazardRoute1((uint16)_frameCounter, _perspectiveX, _perspectiveY);
			break;
		case 2:
			walkerHazard = hasLevel8PerspectiveHazardRoute2((uint16)_frameCounter, _perspectiveX, _perspectiveY);
			break;
		}

		if (walkerHazard)
			_damageFlags |= 0x20;
	}

	// Health regeneration (FUN_1BB0E): +1 every 32 frames when alive
	if (_health >= 0 && _health < kMaxHealth && (_frameCounter & 0x1F) == 0) {
		_health++;
	}

	// Damage application (FUN_1CDA7 lines 20-41)
	// No cooldown — all three damage types can stack each frame
	if (_damageFlags != 0 && _health >= 0 && _deathTimer < 1) {
		_screenFlash = 5;
		if (_damageFlags & 0x80)
			_health -= _tuning.shot;
		if (_damageFlags & 0x40)
			_health -= _tuning.miss;
		if (_damageFlags & 0x20)
			_health -= _tuning.wham;
		if (_health < 0) {
			_deathTimer = 15;  // 0x0F — shorter than Level 1's 30
		}
		_prevDamageFlags = _damageFlags;
		_damageFlags = 0;
	}

	// Latches are frame-local event inputs in the original pipeline.
	_gameLatch5D = 0;
	_gameLatch5F = 0;

	// Death fade countdown
	if (_deathTimer > 1 && _health < 0) {
		_deathTimer--;
	}

	// Screen flash countdown
	if (_screenFlash > 0) {
		_screenFlash--;
	}

	// --- Cursor and perspective smoothing (FUN_1CDA7) ---
	// _inputHistory* maps to 0x7580/0x7594, _viewHistory* to 0x75A8/0x75BC.
	int16 inputX = 0;
	int16 inputY = 0;
	preprocessMouseAxes(inputX, inputY);
	inputX = CLIP<int16>(inputX, -0xA0, 0xA0);
	inputY = CLIP<int16>(inputY, -100, 100);

	for (int i = kInputHistorySize - 1; i > 0; i--) {
		_inputHistoryX[i] = _inputHistoryX[i - 1];
		_inputHistoryY[i] = _inputHistoryY[i - 1];
	}
	_inputHistoryX[0] = inputX;
	_inputHistoryY[0] = inputY;

	int sumInputX = 0;
	int sumInputY = 0;
	for (int i = 0; i < kAsteroidSmoothWindow; i++) {
		sumInputX += _inputHistoryX[i];
		sumInputY += _inputHistoryY[i];
	}

	_avgInputX = (int16)(sumInputX / kAsteroidSmoothWindow);
	_avgInputY = (int16)(-sumInputY / kAsteroidSmoothWindow);
	_avgInputX = CLIP<int16>(_avgInputX, -0xA0, 0xA0);
	_avgInputY = CLIP<int16>(_avgInputY, -0x46, 0x41);

	_shipPosX = _avgInputX + 0xA0;
	_shipPosY = _avgInputY + 0x46;

	for (int i = kInputHistorySize - 1; i > 0; i--) {
		_viewHistoryX[i] = _viewHistoryX[i - 1];
		_viewHistoryY[i] = _viewHistoryY[i - 1];
	}
	_viewHistoryX[0] = _avgInputX;
	_viewHistoryY[0] = _avgInputY;

	int sumViewX = 0;
	int sumViewY = 0;
	for (int i = 0; i < kAsteroidSmoothWindow; i++) {
		sumViewX += _viewHistoryX[i];
		sumViewY += _viewHistoryY[i];
	}

	int16 avgViewX = (int16)(sumViewX / kAsteroidSmoothWindow);
	int16 avgViewY = (int16)(sumViewY / kAsteroidSmoothWindow);
	_perspectiveX = CLIP<int16>((int16)((avgViewX >> 1) + 0x20), 0, 0x40);
	_perspectiveY = CLIP<int16>((int16)((avgViewY >> 1) + 0x17), 0, 0x2E);
	resetProjectionTable();

	_frameCounter++;

	if (_currentLevel == 3 && _levelGameplayPhase == 2 && _frameCounter == 0x3E)
		_gameplayFlags75ff |= 2;

	checkDynamicLevelBranch();

	debug(7, "RA1 asteroid: pos=(%d,%d) avg=(%d,%d) view=(%d,%d) health=%d flash=%d",
		_shipPosX, _shipPosY, _avgInputX, _avgInputY,
		_perspectiveX, _perspectiveY, _health, _screenFlash);
}


// updateOnFootPhysics — HandleGameOp19_OnFootSequence (0x19) + HandleGameOp1A_OnFootVariant (0x1A).
// On-foot handler for Level 9 (Stormtroopers). Character walks left/right, crosshair tracks mouse.
//
// Original has TWO separate variable pairs:
//   DAT_000041a0/41a2 = camera offset (SetCameraOffset, ProjectPointToScreen)
//   g_perspectiveX/Y  = crosshair center (on-foot targeting)
// Our _perspectiveX/_perspectiveY maps to the camera offset (DAT_000041a0/41a2).
// The crosshair center (0xA3, 0x82) is a separate constant for on-foot mode.
static const int16 kOnFootCenterX = 0xA3;  // g_perspectiveX in HandleGameOp19
static const int16 kOnFootCenterY = 0x82;  // g_perspectiveY in HandleGameOp19

void InsaneRebel1::updateOnFootPhysics() {
	// --- First-frame initialization (0x19 counter==0) ---
	if (!_onFootInitialized) {
		_onFootInitialized = true;
		_shipDirIndex = 15;       // Center facing
		_onFootAnimCounter = 0;
		_onFootCharX = 0;
		_onFootCharY = 0;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;

		// SetCameraOffset(0,0) — no viewport crop for on-foot levels
		_perspectiveX = 0;
		_perspectiveY = 0;
		resetProjectionTable();
	}

	// --- 0x19: Character walk animation + damage ---
	// Track fire button for animation
	if (!_playerFired)
		_onFootAnimCounter = 0;
	else
		_onFootAnimCounter++;

	// Walk direction state machine (from HandleGameOp19)
	if (_shipDirIndex == 0) {
		// Left edge: snap to center, step character left
		_shipDirIndex = 15;
		_onFootCharX -= 0x3A;
	} else if (_shipDirIndex < 5) {
		_shipDirIndex--;
	} else if (_shipDirIndex < 10) {
		_shipDirIndex++;
	} else if (_shipDirIndex == 10) {
		// Right edge: snap to center, step character right
		_shipDirIndex = 15;
		_onFootCharX += 0x3A;
	} else if (_onFootAnimCounter < 5 && !_playerFired) {
		// Aim direction from crosshair toward character (QuantizeDirection8Way)
		int16 dx = _shipPosX - (_onFootCharX + kOnFootCenterX);
		int16 aimDir = 0;
		if (dx > 30)
			aimDir = 4;
		else if (dx > 10)
			aimDir = 2;
		else if (dx < -30)
			aimDir = -4;
		else if (dx < -10)
			aimDir = -2;
		_shipDirIndex = CLIP<int16>(aimDir + 15, 11, 19);
	} else {
		// Walking based on mouse input direction
		int16 inputX = 0, inputY = 0;
		preprocessMouseAxes(inputX, inputY);
		if (inputX > 0x1E && _onFootCharX < 0x72)
			_shipDirIndex = 6;  // Walk right
		else if (inputX < -0x1E && _onFootCharX > -0x72)
			_shipDirIndex = 4;  // Walk left
	}

	// --- 0x1A: Crosshair positioning (HandleGameOp1A_OnFootVariant) ---
	// shipPosX/Y = mouse_input + crosshair_center + character_offset
	int16 inputX = 0, inputY = 0;
	preprocessMouseAxes(inputX, inputY);
	inputX = CLIP<int16>(inputX, -100, 100);
	int16 inputYNeg = CLIP<int16>((int16)(-inputY), -0x4B, 0x0F);
	_shipPosX = inputX + kOnFootCenterX + _onFootCharX;
	_shipPosY = inputYNeg + kOnFootCenterY + _onFootCharY - 0x32;

	// --- Scripted damage latches → damageFlags (matching FUN_1B297 pattern) ---
	// GAME 0x5D/0x5F set latches; convert to damage flags before the check.
	if (_gameLatch5D == 0xFFFF)
		_damageFlags |= 0x40;
	if (_gameLatch5F != 0 &&
		_vm->_rnd.getRandomNumber((uint16)(_gameLatch5F - 1)) == 0)
		_damageFlags |= 0x80;

	// --- Damage handling (from HandleGameOp19_OnFootSequence) ---
	// On-foot uses single tuning value (DAT_00001b29 offset = miss) for all damage types.
	if (_damageFlags != 0 && _damageCooldown == 0 && _health >= 0 && _deathTimer < 1) {
		_health -= _tuning.miss;
		if (_health < 0)
			_deathTimer = 15;
		_prevDamageFlags = _damageFlags;
		_damageCooldown = 3;
		_screenFlash = 5;
	}

	if (_damageCooldown > 0)
		_damageCooldown--;

	// Health regen + survival bonus (UpdatePeriodicScoreAndHealth)
	if ((_frameCounter & 0x1F) == 0) {
		if (_health >= 0 && _health < kMaxHealth)
			_health++;
		if (_health >= 0)
			_score += _tuning.time;
	}

	_gameLatch5D = 0;
	_gameLatch5F = 0;

	if (_deathTimer > 1 && _health < 0)
		_deathTimer--;
	if (_screenFlash > 0)
		_screenFlash--;

	_damageFlags = 0;
	_frameCounter++;
}

void InsaneRebel1::procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
	int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
	int16 par1, int16 par2, int16 par3, int16 par4) {
}

void InsaneRebel1::procSKIP(int32 subSize, Common::SeekableReadStream &b) {
}

// handleGameChunk — FUN_1BE1B (0x1BE1B). Central GAME opcode dispatcher.
// Reads 7x32-bit BE integers from GAME chunk, routes to per-opcode handlers.
void InsaneRebel1::handleGameChunk(int32 subSize, Common::SeekableReadStream &b) {
	if (subSize < 8)
		return;

	uint32 opcode = b.readUint32BE();
	uint32 param1 = b.readUint32BE();

	switch (opcode) {
	case 0x5E:
		// RA1 dispatcher inline reset/init path (FUN_1BE1B case 0x5E).
		// This is not a pure control-mode assignment.
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_gameLatch5D = 0;
		_gameLatch5F = 0;
		_driftParam = 0;
		_rollAccum = 0;
		_liftSmooth = 0;
		_posAccumX = 0;
		_posAccumY = 0;
		memset(_inputHistoryX, 0, sizeof(_inputHistoryX));
		memset(_inputHistoryY, 0, sizeof(_inputHistoryY));
		memset(_viewHistoryX, 0, sizeof(_viewHistoryX));
		memset(_viewHistoryY, 0, sizeof(_viewHistoryY));
		_avgInputX = 0;
		_avgInputY = 0;
		_mouseOffsetX = 0;
		_mouseOffsetY = 0;
		_mouseBiasX = 0;
		_mouseBiasY = 0;
		_mousePrevBiasX = 0;
		_mousePrevBiasY = 0;
		_mouseBiasLatch = false;
		_mouseRecentering = false;

		// Shooting/targeting reset
		_playerFired = false;
		_fireCooldown = 0;
		memset(_shotSlots, 0, sizeof(_shotSlots));
		_shotAlternator = 0;
		_shotSideToggle = false;
		_targetProximity = 0;
		_prevTargetProx = 0;
		_targetAnimCounter = 0;
		_targetCount = 0;
		_prevTargetCount = 0;
		memset(_targetBoxX, 0, sizeof(_targetBoxX));
		memset(_targetBoxY, 0, sizeof(_targetBoxY));
		memset(_targetBoxVariant, 0, sizeof(_targetBoxVariant));
		memset(_gostSlots, 0, sizeof(_gostSlots));
		_gostSlotIdx = 0;
		_killCount = 0;
		_lastHitTarget = 0;

		// Field1 == 0 corresponds to baseline recenter behavior in the original.
		if ((int32)param1 == 0) {
			_shipPosX = kRA1CenterX;
			_shipPosY = kRA1CenterY;
		}

		_activeGameOpcode = 0;
		_frameGameOpcodeMask = 0;
		_frameDispatchFlags = 0;
		resetProjectionTable();
		debug(5, "RA1 GAME 0x5E: reset state field1=%d mode=%d", (int32)param1, (int)_flyControlMode);
		break;

	case 0x5D:
		if ((uint16)param1 == 0xFFFF) {
			_gameLatch5D = 0xFFFF;
		} else if (param1 > 0) {
			const int bitIndex = (int)param1 - 1;
			const int byteIndex = bitIndex >> 3;
			if (byteIndex >= 0 && byteIndex < 0x96 &&
				(_frameObjectState[byteIndex] & (byte)(0x80 >> (bitIndex & 7))) == 0) {
				_gameLatch5D = (uint16)param1;
			}
		}
		debug(5, "RA1 GAME 0x5D (link/event latch) param=%u", _gameLatch5D);
		break;

	case 0x5F:
		if (param1 > 0) {
			const int bitIndex = (int)param1 - 1;
			const int byteIndex = bitIndex >> 3;
			if (byteIndex >= 0 && byteIndex < 0x96 &&
				(_frameObjectState[byteIndex] & (byte)(0x80 >> (bitIndex & 7))) == 0) {
				_gameLatch5F = (uint16)param1;
			}
		}
		debug(5, "RA1 GAME 0x5F (random-hit latch) param=%u", _gameLatch5F);
		break;

	case 0x07:
		_activeGameOpcode = 0x07;
		_frameGameOpcodeMask |= (1u << 0x07);
		// Per-frame corridor data: f1=frame counter, f2=max frames, f3=drift bias, f4=unused
		// f1 is the original's _DAT_7740 (game frame counter)
		// f3 is the drift/wind parameter combined with tuning table
		_gameCounter = param1;
		if (subSize >= 20) {
			b.readUint32BE(); // f2 (max frames, unused in physics)
			_driftParam = (int16)(int32)b.readUint32BE();
			b.readUint32BE(); // f4 (unused in original assembly)
			debug(7, "RA1 GAME 0x07: counter=%d driftParam=%d", _gameCounter, _driftParam);
		}
		break;

	case 0x0D:
		// Corridor boundaries: per-frame flight corridor
		// Original params: left, top, WIDTH, HEIGHT (not right/bottom!)
		// FUN_1C54D computes center = (left+width/2, top+height/2), transforms, then checks edges.
		if (subSize >= 20) {
			int16 corridorLeft = (int16)param1;
			int16 corridorTop = (int16)b.readUint32BE();
			int16 corridorWidth = (int16)b.readUint32BE();
			int16 corridorHeight = (int16)b.readUint32BE();

			int16 centerX = corridorLeft + corridorWidth / 2;
			int16 centerY = corridorTop + corridorHeight / 2;
			projectGameplayPoint(centerX, centerY);

			_corridorLeftX = centerX - corridorWidth / 2;
			_corridorTopY = centerY - corridorHeight / 2;
			_corridorRightX = _corridorLeftX + corridorWidth;
			_corridorBottomY = _corridorTopY + corridorHeight;
			debug(5, "RA1 GAME 0x0D: raw=[%d,%d]+(%d,%d) cam=(%d,%d) transformed=[%d,%d]-[%d,%d]",
				corridorLeft, corridorTop, corridorWidth, corridorHeight,
				_perspectiveX, _perspectiveY,
				_corridorLeftX, _corridorTopY, _corridorRightX, _corridorBottomY);
		}
		break;

	case 0x0E:
		// Secondary collision zone (FUN_1C6E9): AABB test, sets damageFlags bit 4 (0x10)
		// Original params: left, top, WIDTH, HEIGHT (same as 0x0D)
		if (subSize >= 20) {
			int16 zoneLeft = (int16)param1;
			int16 zoneTop = (int16)b.readUint32BE();
			int16 zoneWidth = (int16)b.readUint32BE();
			int16 zoneHeight = (int16)b.readUint32BE();

			int16 centerX = zoneLeft + zoneWidth / 2;
			int16 centerY = zoneTop + zoneHeight / 2;
			projectGameplayPoint(centerX, centerY);

			zoneLeft = centerX - zoneWidth / 2;
			zoneTop = centerY - zoneHeight / 2;
			int16 zoneRight = zoneLeft + zoneWidth;
			int16 zoneBottom = zoneTop + zoneHeight;
			if (_shipPosX > zoneLeft && _shipPosX < zoneRight &&
				_shipPosY > zoneTop && _shipPosY < zoneBottom) {
				_damageFlags |= 0x10;
			}
			debug(7, "RA1 GAME 0x0E: zone=[%d,%d]-[%d,%d] cam=(%d,%d) flags=0x%02x",
				zoneLeft, zoneTop, zoneRight, zoneBottom, _perspectiveX, _perspectiveY, _damageFlags);
		}
		break;

	case 0x0B:
		_activeGameOpcode = 0x0B;
		_frameGameOpcodeMask |= (1u << 0x0B);
		// Asteroid/surface per-frame handler (FUN_1CDA7).
		// field1 = frame counter, field2 = max frames
		_gameCounter = param1;
		if (subSize >= 20) {
			uint32 maxFrames = b.readUint32BE(); // field2 (max frames)
			b.readUint32BE(); // field3
			b.readUint32BE(); // field4

			// RA1 scripts drive progression with GAME counters. Finish 0x0B-driven
			// interactive videos once the script counter reaches the terminal frame.
			if (_interactiveVideoActive && maxFrames > 0 &&
				_gameCounter >= (int32)maxFrames - 1) {
				_vm->_smushVideoShouldFinish = true;
				debug(1, "RA1: finishing 0x0B interactive video at counter=%d/%u", _gameCounter, maxFrames);
			}
		}
		debug(7, "RA1 GAME 0x0B: counter=%d", _gameCounter);
		break;

	case 0x5A:
		// Target detection — HandleGameOp5A (0x1C0EF). AABB from video stream.
		// Original checks event mask: if target already killed, skip to GOST update.
		if (subSize >= 24) {
			int16 targetIdx = (int16)param1;
			int16 left = (int16)b.readUint32BE();
			int16 top = (int16)b.readUint32BE();
			int16 w = (int16)b.readUint32BE();
			int16 h = (int16)b.readUint32BE();
			int16 right = left + w;
			int16 bottom = top + h;

			if (targetIdx >= 0) {
				const int byteIdx = targetIdx >> 3;
				if (byteIdx >= 0 && byteIdx < 0x96 && byteIdx < kFrameObjectStateBytes) {
					const byte bit = (byte)(0x80 >> (targetIdx & 7));
					const int altIdx = byteIdx + 0x96;
					const bool primarySet = (_frameObjectState[byteIdx] & bit) != 0;
					const bool secondarySet = (altIdx < kFrameObjectStateBytes) &&
						((_frameObjectState[altIdx] & bit) != 0);

					if (!primarySet || secondarySet) {
						checkTargetHit(targetIdx, left, top, right, bottom);
					} else {
						updateGostSlotPosition(targetIdx, left, top, right, bottom);
					}
				} else {
					checkTargetHit(targetIdx, left, top, right, bottom);
				}
			}
			debug(5, "RA1 GAME 0x5A: target=%d rect=[%d,%d]-[%d,%d] prox=%d",
				targetIdx, left, top, right, bottom, _targetProximity);
		}
		break;

	case 0x08:
	case 0x09:
	case 0x0A:
	case 0x19:
	case 0x1A:
		_activeGameOpcode = (uint16)opcode;
		_frameGameOpcodeMask |= (1u << opcode);
		_gameCounter = param1;
		if (subSize >= 20) {
			uint32 param2 = b.readUint32BE();
			uint32 param3 = b.readUint32BE();
			uint32 param4 = b.readUint32BE();
			debug(5, "RA1 GAME 0x%02x: counter=%d params=(%d,%d,%d)",
				opcode, _gameCounter, param2, param3, param4);
		}
		break;

	default:
		debug(7, "RA1 GAME unknown 0x%02x size=%d", opcode, subSize);
		break;
	}
}

// processShot — FUN_1CCA0 (0x1CCA0). Spawns shot into explosion slot when fired.
// Called once per frame during interactive rendering.
void InsaneRebel1::processShot() {
	if (!_playerFired || _fireCooldown != 0)
		return;

	// On-foot mode: only spawn when in aiming stance (dirIndex 11-19) or flags force it.
	// Original: if (((10 < g_shipDirIndex) && (g_shipDirIndex < 0x14)) || ((DAT_000075fe & 8) != 0))
	const bool onFootMode = (_activeGameOpcode == 0x19 || _activeGameOpcode == 0x1A);
	if (onFootMode) {
		if (!(_shipDirIndex > 10 && _shipDirIndex < 20) && !(_gameplayFlags75fe & 8))
			return;
	}

	// Find first available slot (timer < 1 or > 5), matching FUN_1CCA0.
	int slot = -1;
	for (int i = 0; i < kMaxShotSlots; i++) {
		if (_shotSlots[i].timer <= 0 || _shotSlots[i].timer > 5) {
			slot = i;
			break;
		}
	}
	if (slot < 0) {
		return;
	}

	// Shot origin depends on game mode:
	// On-foot: character position (g_shipOffsetX + g_perspectiveX)
	// Turret: perspective-adjusted center
	// Flight: cursor position
	const bool turretMode = (_activeGameOpcode == 0x08 || _activeGameOpcode == 0x0A);
	int16 originX, originY;
	if (onFootMode) {
		originX = _onFootCharX + kOnFootCenterX;
		originY = _onFootCharY + kOnFootCenterY;
	} else if (turretMode) {
		originX = (int16)(kRA1CenterX + (_perspectiveX - 0x20));
		originY = (int16)(kRA1CenterY + (_perspectiveY - 0x17));
	} else {
		originX = _shipPosX;
		originY = _shipPosY;
	}

	_shotSlots[slot].timer = (_gameplayFlags75ff & 0x2) ? 2 : 5;
	_shotSlots[slot].posX = _shipPosX;
	_shotSlots[slot].posY = _shipPosY;
	_shotSlots[slot].centerX = originX;
	_shotSlots[slot].centerY = originY;
	_shotSlots[slot].variant = _shotAlternator;
	_shotAlternator = 1 - _shotAlternator;
	playSfx(kSfxLaserShot, 127, 0);

	debug(5, "RA1 shot: slot=%d pos=(%d,%d) origin=(%d,%d)", slot,
		_shotSlots[slot].posX, _shotSlots[slot].posY, originX, originY);
}

// checkTargetHit — FUN_1C0EF (0x1C0EF). AABB target detection with snap tolerance.
// The original compares target bounds against the cursor after
// UnprojectScreenPoint(), then reprojects the snapped cursor center after a hit.
void InsaneRebel1::checkTargetHit(int16 targetIdx, int16 left, int16 top, int16 right, int16 bottom) {
	int16 snap = _tuning.snap;
	int16 curX = _shipPosX;
	int16 curY = _shipPosY;
	unprojectGameplayPoint(curX, curY);
	const int slot = _targetCount;

	if (slot < kMaxTargetBoxes) {
		_targetBoxX[slot] = (int16)((left + right) / 2);
		_targetBoxY[slot] = (int16)((top + bottom) / 2);

		const int height = bottom - top;
		int16 glyphVariant = 0;
		if (height >= 0x10)
			glyphVariant = 2;
		else if (height > 3)
			glyphVariant = 1;
		_targetBoxVariant[slot] = glyphVariant;
	}

	_targetCount++;

	// Check proximity: cursor within target + snap + 5 margin
	if (curX > left - snap - 5 && curX < right + snap + 5 &&
		curY > top - snap - 5 && curY < bottom + snap + 5) {
		if (_targetProximity == 0)
			_targetProximity = 1;  // Near
		if (slot < kMaxTargetBoxes)
			_targetBoxVariant[slot] = CLIP<int16>((int16)(_targetBoxVariant[slot] + 3), 0, 5);

		// Check tight lock: cursor within target + snap (no extra margin)
		if (curX > left - snap && curX < right + snap &&
			curY > top - snap && curY < bottom + snap) {
			_targetProximity = 2;  // On-target

			// Check if any active shot slot hits this target
			if (_lastHitTarget != targetIdx + 1) {
				for (int i = 0; i < kMaxShotSlots; i++) {
					if (_shotSlots[i].timer == 1) {  // Shot in final frame = impact
						// Hit! Record in GOST slot for explosion animation
						int gi = _gostSlotIdx;
						_gostSlots[gi].targetId = targetIdx + 1;
						_gostSlots[gi].frame = 0;
						_gostSlots[gi].posX = (left + right) / 2;
						_gostSlots[gi].posY = (top + bottom) / 2;
						_gostSlotIdx = (_gostSlotIdx + 1) % kMaxGostSlots;

						_lastHitTarget = targetIdx + 1;
						_score += _tuning.kill;
						_killCount++;
						applyFrameObjectHitState(targetIdx);
						int16 hitCenterX = (left + right) / 2;
						int16 hitCenterY = (top + bottom) / 2;
						projectGameplayPoint(hitCenterX, hitCenterY);
						const int sfxPan = CLIP((hitCenterX - kRA1CenterX) * 127 / kRA1CenterX, -127, 127);
						playSfx(kSfxExplode, 127, sfxPan);

						// Match FUN_1C0EF: snap in unprojected space, then project back
						// into the current gameplay window before rendering the pointer.
						if (snap > 0) {
							_shipPosX = (left + right) / 2;
							_shipPosY = (top + bottom) / 2;
							projectGameplayPoint(_shipPosX, _shipPosY);
						}

						debug(3, "RA1 HIT: target=%d gost=%d pos=(%d,%d) score=%d kills=%d bangSprites=%d",
							targetIdx, gi, _gostSlots[gi].posX, _gostSlots[gi].posY,
							_score, _killCount, _bangBank.numSprites);
						return;
					}
				}
			}
		}
	}
}

} // End of namespace Scumm
