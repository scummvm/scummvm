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
#include "common/config-manager.h"
#include "common/endian.h"

#include "scumm/scumm_v7.h"
#include "scumm/insane/rebel1/rebel.h"

namespace Scumm {

inline int16 applyRebel1AnalogDeadzone(int16 axisValue) {
	const int deadZone = MAX(0, ConfMan.getInt("joystick_deadzone")) * 1000;
	const int axis = axisValue;
	return (ABS(axis) <= deadZone) ? 0 : axisValue;
}

inline int16 smoothRebel1Op0BAnalogInput(int16 inputValue, int16 &filteredValue,
		int16 axisMax, int responseDivisor) {
	const int delta = (int)inputValue - (int)filteredValue;
	int step = delta / responseDivisor;

	if (step == 0 && delta != 0)
		step = (delta > 0) ? 1 : -1;

	filteredValue = CLIP<int>(filteredValue + step, -axisMax, axisMax);
	return filteredValue;
}

inline int16 scaleRebel1CenteredMouseAxis(int16 value, int16 center, int axisMax) {
	return (int16)CLIP<int32>(((int32)(value - center) * axisMax) / center, -axisMax, axisMax);
}

inline int16 stepRebel1Op0BReticleAxis(int axisValue) {
	if (axisValue >= 0)
		return (int16)(axisValue >> 4);

	// The 3DO ARM code integrates the standard control-pad path with ASR #4.
	return (int16)-((-axisValue + 15) >> 4);
}

int16 shapeRebel1Op0BGamepadAxis(int16 inputValue, int16 axisMax) {
	const int axis = CLIP<int>(inputValue, -axisMax, axisMax);
	const int absAxis = ABS(axis);
	const int precisionRadius = MAX<int>(1, (axisMax * 2) / 5);
	const int precisionOutput = MAX<int>(1, precisionRadius / 3);
	int shaped = 0;

	if (absAxis <= precisionRadius) {
		shaped = (absAxis * precisionOutput + precisionRadius / 2) / precisionRadius;
	} else {
		const int outerInput = absAxis - precisionRadius;
		const int outerInputRange = axisMax - precisionRadius;
		const int outerOutputRange = axisMax - precisionOutput;
		const int linear = outerInput * outerOutputRange / outerInputRange;
		const int accel = (outerInput * outerInput * outerOutputRange +
			(outerInputRange * outerInputRange) / 2) /
			(outerInputRange * outerInputRange);
		shaped = precisionOutput + (linear + 2 * accel) / 3;
	}

	return axis < 0 ? -shaped : shaped;
}

const int16 kRA1Op09AimXScale[5] = { 0, 44, 88, 128, 165 };
const int16 kRA1Op09AimYScale[5] = { 256, 252, 240, 221, 196 };
const int kRA1CenteredAxisMax = 127;
const int kRA1Op0BVerticalAxisMax = 100;
const int kRA1EnhancedFlightDirectMaxX = 64;
const int kRA1EnhancedFlightDirectMaxY = 40;
const int kRA1MouseFlightRollTargetScale = 16;
const int kRA1ControlPadAxisStep = 0x1E;
const int16 kOnFootCenterX = 0xA3;  // g_perspectiveX in HandleGameOp19
const int16 kOnFootCenterY = 0x82;  // g_perspectiveY in HandleGameOp19
const int16 kOnFootCursorBaseY = kOnFootCenterY - 0x32;
const int16 kOnFootGamepadStep = 7;
const int16 kOnFootCursorMinX = 0;
const int16 kOnFootCursorMaxX = 319;
const int16 kOnFootCursorMinY = 0;
const int16 kOnFootCursorMaxY = 199;
const int16 kRA1Level12TargetA = 195;
const int16 kRA1Level12TargetB = 197;
const int16 kRA1Level12TargetC = 199;

// Level 15 final approach 0x5D damage/event codes consumed by
// RunLevel1GameLoop. The latch stores the raw GAME parameter; no translation is
// performed by HandleGameOp5D_SegmentLinkLatch.
inline bool isLevel15FinalDamageLatch(uint16 code) {
	switch (code) {
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

inline bool hasLevel15FinalSweepDamage(uint16 frameCounter, int16 perspectiveX) {
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

inline bool ra1DispatcherHudOnlyWhenDisabled(uint32 opcode) {
	switch (opcode) {
	case 0x07:
	case 0x08:
	case 0x0B:
	case 0x1A:
		return true;
	default:
		return false;
	}
}

inline uint16 ra1PrioritizedFrameOpcode(uint32 opcodeMask) {
	if (opcodeMask & (1u << 0x1A))
		return 0x1A;
	if (opcodeMask & (1u << 0x19))
		return 0x19;
	if (opcodeMask & (1u << 0x0B))
		return 0x0B;
	if (opcodeMask & (1u << 0x0A))
		return 0x0A;
	if (opcodeMask & (1u << 0x09))
		return 0x09;
	if (opcodeMask & (1u << 0x08))
		return 0x08;
	if (opcodeMask & (1u << 0x07))
		return 0x07;

	return 0;
}

bool ra1TargetCursorUsesProjection(uint16 opcode) {
	switch (opcode) {
	// FOBJ target chunks can precede the paired 0x0A GAME chunk, leaving 0x08
	// as the active opcode in turret sections such as L1PLAY2.
	case 0x08:
	case 0x09:
	case 0x0A:
	case 0x0B:
	case 0x1A:
		return true;
	default:
		return false;
	}
}

inline bool isLevel2DamageLatch(uint16 code) {
	switch (code) {
	case 0x0003:
	case 0x0009:
	case 0x000A:
	case 0x000D:
	case 0x0012:
	case 0x0015:
		return true;
	default:
		return false;
	}
}

// Level 2 asteroid-contact helper from FUN_00012d70. RunLevel2Flow calls it
// once per frontend frame and raises damage flag 0x20 when the current asteroid
// pass intersects the camera position.
inline bool hasLevel2AsteroidImpact(uint16 frameCounter, int16 perspectiveX, int16 perspectiveY) {
	switch (frameCounter) {
	case 0x0071:
	case 0x0271:
		return perspectiveX >= 0x28;
	case 0x011E:
		return perspectiveY >= 0x1F;
	case 0x01E1:
	case 0x02ED:
		return perspectiveX > 0x17;
	default:
		return false;
	}
}

const int16 kLevel7BranchFrames[6][6] = {
	{ -1,  78, 267, 398, 556, 630 },
	{ -1, 187, 376, 507, 665, 739 },
	{ -1, 187, 376, 507, 665, 739 },
	{ -1,  -1,  -1, 284, 442, 516 },
	{ -1,  -1,  -1, 143, 301, 375 },
	{ -1, 112, 301, 432, 590, 664 }
};

const int16 kLevel7BranchDir[6] = {
	0, 1, 1, -1, 1, 1
};

const int16 kLevel7BranchThreshold[6] = {
	0, 170, 170, 160, 160, 160
};

inline bool isLevel4DamageLatch(uint16 code) {
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

inline bool isLevel6DamageLatch(uint16 code) {
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

inline bool isLevel10DamageLatch(uint16 code) {
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

inline bool isLevel12DamageLatch(uint16 code) {
	// RunLevel12Flow treats these 0x5D latch values as safe; every other value
	// raises damage flag 0x40. This helper is intentionally the inverse of the
	// original branch shape so the update path reads as "is damage".
	switch (code) {
	case 0x0000:
	case 0x0037:
	case 0x003E:
	case 0x0059:
	case 0x00AC:
	case 0x00C3:
	case 0x00C5:
	case 0x00C7:
		return false;
	default:
		return true;
	}
}

inline bool isLevel14Phase2DamageLatch(uint16 code) {
	switch (code) {
	case 0x0008:
	case 0x000E:
	case 0x0010:
	case 0x0012:
		return true;
	default:
		return false;
	}
}

inline bool hasLevel6PerspectiveHazard(uint16 frame, int16 perspectiveX, int16 perspectiveY) {
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

// Named translations of the original Level 8 route collision helpers
// FUN_12FE1/FUN_130C9/FUN_13195. The names are new to the port.
inline bool hasLevel8WalkerHazardRoute0(uint16 frame, int16 viewX, int16 viewY) {
	switch (frame) {
	case 0x00CD:
	case 0x00EF:
		return viewX <= 0x28;
	case 0x0294:
		return viewY >= 0x0F;
	case 0x03A2:
		return viewX >= 0x18;
	case 0x04BE:
	case 0x076C:
		return viewX <= 0x28 && viewY <= 0x1F;
	case 0x05C9:
	case 0x085A:
	case 0x096F:
		return viewY <= 0x1F;
	default:
		return false;
	}
}

inline bool hasLevel8WalkerHazardRoute1(uint16 frame, int16 viewX, int16 viewY) {
	switch (frame) {
	case 0x0189:
		return viewY >= 0x0F;
	case 0x0297:
		return viewX >= 0x18;
	case 0x03B3:
	case 0x0661:
		return viewX <= 0x28 && viewY <= 0x1F;
	case 0x04BE:
	case 0x074F:
	case 0x0864:
		return viewY <= 0x1F;
	default:
		return false;
	}
}

inline bool hasLevel8WalkerHazardRoute2(uint16 frame, int16 viewX, int16 viewY) {
	switch (frame) {
	case 0x00BB:
		return viewX <= 0x28 && viewY <= 0x1F;
	case 0x01A9:
	case 0x02BE:
		return viewY <= 0x1F;
	default:
		return false;
	}
}

inline bool hasLevel8WalkerPlayerHit(int route, uint16 frame, int16 viewX, int16 viewY) {
	switch (CLIP<int>(route, 0, 2)) {
	case 0:
		return hasLevel8WalkerHazardRoute0(frame, viewX, viewY);
	case 1:
		return hasLevel8WalkerHazardRoute1(frame, viewX, viewY);
	case 2:
		return hasLevel8WalkerHazardRoute2(frame, viewX, viewY);
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

	// Protected targets (shield generators in Level 4, etc.) can be hit
	// repeatedly — skip event mask toggle. Original: DAT_00007732/7734 check
	// in HandleGameOp5A.
	if (targetIdx + 1 == _protectedTargetA || targetIdx + 1 == _protectedTargetB)
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

bool InsaneRebel1::isFrameObjectPrimarySet(int16 objectId) const {
	if (objectId <= 0)
		return false;

	const int bitIndex = objectId - 1;
	const int byteIndex = bitIndex >> 3;
	if (byteIndex < 0 || byteIndex >= 0x96 || byteIndex >= kFrameObjectStateBytes)
		return false;

	const byte bit = (byte)(0x80 >> (bitIndex & 7));
	return (_frameObjectState[byteIndex] & bit) != 0;
}

bool InsaneRebel1::areFrameObjectPrimaryBitsSet(int byteIndex, byte mask) const {
	if (byteIndex < 0 || byteIndex >= 0x96 || byteIndex >= kFrameObjectStateBytes)
		return false;

	return (_frameObjectState[byteIndex] & mask) == mask;
}

void InsaneRebel1::clearFrameObjectPrimaryBits(int byteIndex, byte mask) {
	if (byteIndex < 0 || byteIndex >= 0x96 || byteIndex >= kFrameObjectStateBytes)
		return;

	_frameObjectState[byteIndex] &= ~mask;
}

// Port helpers for RunLevel14Flow. The original checks DAT_7614..7616 and
// DAT_7605..7606 inline, then increments a local 60-frame completion counter.
bool InsaneRebel1::areLevel14Phase1TargetsDestroyed() const {
	// g_gameplayPhaseFlags starts at primary byte 0; DAT_7614 is byte 0x12.
	return areFrameObjectPrimaryBitsSet(0x12, 0x01) &&
		areFrameObjectPrimaryBitsSet(0x13, 0xFF) &&
		areFrameObjectPrimaryBitsSet(0x14, 0xFE);
}

bool InsaneRebel1::areLevel14Phase2TargetsDestroyed() const {
	// Phase 2 checks DAT_7605 low nibble and all of DAT_7606.
	return areFrameObjectPrimaryBitsSet(0x03, 0x0F) &&
		areFrameObjectPrimaryBitsSet(0x04, 0xFF);
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
	const int16 right = left + width;
	const int16 bottom = top + height;

	if (objectId >= 0x280 && _frameObjectHitRevealPending) {
		// DispatchSmushFrameChunks keeps a one-object latch after a target hit.
		// The following high-id FOBJ clears its hidden bit and becomes the
		// replacement/death frame. This is used heavily by Level 9 troopers.
		_frameObjectState[byteIndex] &= ~bit;
		_frameObjectHitRevealPending = false;
		debugC(DEBUG_INSANE, "FOBJ reveal: object=%d frameObjectByte=%d bit=0x%02x",
			objectId, byteIndex, bit);
	}

	const bool primarySet = (_frameObjectState[byteIndex] & bit) != 0;
	const bool secondarySet = (altIndex < kFrameObjectStateBytes) && ((_frameObjectState[altIndex] & bit) != 0);

	if (objectId > 0 && objectId < 0x280) {
		if (!primarySet || secondarySet) {
			const int16 previousKillCount = _killCount;
			checkTargetHit(objectId - 1, left, top, right, bottom);
			const bool updatedPrimarySet = (_frameObjectState[byteIndex] & bit) != 0;
			_frameObjectHitRevealPending =
				(_killCount != previousKillCount) && !primarySet && updatedPrimarySet;
		} else {
			_frameObjectHitRevealPending = false;
			updateGostSlotPosition(objectId - 1, left, top, right, bottom);
		}
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

void InsaneRebel1::checkDynamicLevelBranch(int32 curFrame) {
	if (!_interactiveVideoActive || _levelRouteIndex < 0)
		return;

	if ((_currentLevel == 6 || _currentLevel == 7) && _pendingRouteIndex >= 0) {
		const uint32 routeFrame = (_currentLevel == 6 && curFrame >= 0) ?
			(uint32)curFrame : (uint32)_gameCounter;
		if (!_vm->_smushVideoShouldFinish &&
			_pendingRouteCutoverFrame >= 0 &&
			routeFrame >= (uint32)_pendingRouteCutoverFrame) {
			// Level 7 route switches open the destination ANM as a fresh file.
			// Keep Rebel runtime state, but do not carry SMUSH decoder state
			// from the previous route into the new file.
			if (_player && _currentLevel != 6)
				_player->setPreserveGameVideoStateOnRelease(true);
			_vm->_smushVideoShouldFinish = true;
			const int32 resumeFrame = (_currentLevel == 6 && _pendingRouteStartFrame < 0) ?
				0 : _pendingRouteStartFrame;
			debugC(DEBUG_INSANE, "L%d cutover: route=%d -> %d at %s=%u (resumeFrame=%d)",
				_currentLevel + 1, _levelRouteIndex, _pendingRouteIndex,
				_currentLevel == 6 ? "localFrame" : "frame",
				(unsigned)routeFrame, (int)resumeFrame);
		}
		return;
	}

	if (_pendingRouteIndex >= 0 || _vm->_smushVideoShouldFinish)
		return;

	if (_currentLevel == 6) {
		// RunLevel7Flow compares the branch table against g_frameCounter. The
		// playback callback writes that value from the ANM-local frame index,
		// not from the decoded GAME counter embedded in these non-linear files.
		if (curFrame < 0)
			return;
		const uint32 routeFrame = (uint32)curFrame;
		// GAME 0x09 publishes its branch-tested position in g_shipPosX.
		// Keep the drawn ship center and the 0x09 aim cursor split,
		// so compare the effective gameplay cursor here.
		const int16 branchX = getGameplayCursorX();
		const int route = CLIP<int>(_levelRouteIndex, 0, 5);
		for (int nextRoute = 1; nextRoute < 6; ++nextRoute) {
			const int triggerFrame = kLevel7BranchFrames[route][nextRoute];
			if (triggerFrame <= 0)
				continue;

			const uint32 decisionFrame = (uint32)(triggerFrame - 1);
			if (routeFrame + 0x1E == decisionFrame) {
				_level7WarningFrames = 0x1E;
				_level7WarningThreshold = kLevel7BranchThreshold[nextRoute];
			}

			if (routeFrame != decisionFrame)
				continue;

			const bool takeBranch = (kLevel7BranchDir[nextRoute] > 0)
				? (branchX > kLevel7BranchThreshold[nextRoute])
				: (branchX < kLevel7BranchThreshold[nextRoute]);
			if (!takeBranch) {
				if (routeFrame == decisionFrame)
					debugC(DEBUG_INSANE, "L7 branch miss: route=%d candidate=%d localFrame=%u gameFrame=%d shipX=%d dir=%d threshold=%d",
						route, nextRoute, (unsigned)routeFrame, (int)_gameCounter, branchX,
						kLevel7BranchDir[nextRoute], kLevel7BranchThreshold[nextRoute]);
				continue;
			}

			_pendingRouteIndex = nextRoute;
			_pendingRouteCutoverFrame = (int32)routeFrame + 7;
			_pendingRouteStartFrame = (int32)routeFrame;
			_pendingRouteVideoStartFrame = 1 + (_pendingRouteCutoverFrame - _pendingRouteStartFrame);
			_level7WarningFrames = 0;
			debugC(DEBUG_INSANE, "L7 branch: route=%d -> %d at localFrame=%u gameFrame=%d decisionFrame=%u shipX=%d resumeSourceFrame=%d cutoverFrame=%d destFrame=%d",
				route, nextRoute, (unsigned)routeFrame, (int)_gameCounter, (unsigned)decisionFrame, branchX,
				(int)_pendingRouteStartFrame, (int)_pendingRouteCutoverFrame,
				(int)_pendingRouteVideoStartFrame);
			return;
		}
	}

	// Level 8 owns its branch choice in updateLevel8WalkerState(), where the
	// original RunLevel8Flow also draws the timer/arrows and updates the local
	// choice variable. This function only performs the delayed route cutover.
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

uint16 InsaneRebel1::getEffectiveGameOpcode() const {
	const uint16 frameOpcode = ra1PrioritizedFrameOpcode(_frameGameOpcodeMask);
	if (frameOpcode != 0)
		return frameOpcode;

	return _activeGameOpcode;
}

uint16 InsaneRebel1::getTargetHitGameOpcode() const {
	const uint16 frameOpcode = ra1PrioritizedFrameOpcode(_frameGameOpcodeMask | _frameGameOpcodeHintMask);
	if (frameOpcode != 0)
		return frameOpcode;

	return _activeGameOpcode;
}

int16 InsaneRebel1::getGameplayCursorX(uint16 opcode) const {
	return (opcode == 0x09) ? _flightAimX : _shipPosX;
}

int16 InsaneRebel1::getGameplayCursorX() const {
	return getGameplayCursorX(getEffectiveGameOpcode());
}

int16 InsaneRebel1::getGameplayCursorY(uint16 opcode) const {
	return (opcode == 0x09) ? _flightAimY : _shipPosY;
}

int16 InsaneRebel1::getGameplayCursorY() const {
	return getGameplayCursorY(getEffectiveGameOpcode());
}

void InsaneRebel1::setGameplayCursor(uint16 opcode, int16 x, int16 y) {
	if (opcode == 0x09) {
		_flightAimX = x;
		_flightAimY = y;
	} else {
		_shipPosX = x;
		_shipPosY = y;
	}
}

void InsaneRebel1::setGameplayCursor(int16 x, int16 y) {
	setGameplayCursor(getEffectiveGameOpcode(), x, y);
}

void InsaneRebel1::updateFlightVariantCursor() {
	if (getEffectiveGameOpcode() != 0x09)
		return;

	const int bucket = CLIP<int>(ABS(_rollAccum) >> 8, 0, ARRAYSIZE(kRA1Op09AimXScale) - 1);
	int32 xScale = kRA1Op09AimXScale[bucket];
	if (_rollAccum > 0)
		xScale = -xScale;

	// Assembly-verified 0x09 layout:
	//   ship sprite center = (_74B6 + _74BA, _74B8 + _74BC)
	//   cursor center      = (_74BE, _74C0)
	// The flight sprite center already lives in _shipPos.
	const int16 shipBaseX = _shipPosX;
	const int16 shipBaseY = _shipPosY;
	const int32 liftTerm = (int32)_liftSmooth - 0x0F;
	_flightAimX = CLIP<int32>(shipBaseX + ((liftTerm * xScale) >> 8), kRA1MinX, kRA1MaxX);
	_flightAimY = CLIP<int32>(shipBaseY + ((liftTerm * kRA1Op09AimYScale[bucket]) >> 8),
		kRA1MinY, kRA1MaxY);

	debugC(DEBUG_INSANE, "op09 cursor: frame=%d shipBase=(%d,%d) shipPos=(%d,%d) aim=(%d,%d) roll=%d lift=%d bucket=%d dir=%d persp=(%d,%d)",
		_gameCounter, shipBaseX, shipBaseY, _shipPosX, _shipPosY, _flightAimX, _flightAimY,
		_rollAccum, _liftSmooth, bucket, _shipDirIndex, _perspectiveX, _perspectiveY);
}

// preprocessMouseAxes — FUN_231BE (0x231BE) centered-axis output law, adapted to
// the absolute 320x200 mouse space. The old DOS virtual-mouse/recenter
// control path is intentionally not used. For opcode 0x0B, gamepad input uses
// the 3DO standard-pad reticle model: axis samples move the reticle, and
// releasing the pad holds the last reticle position.
bool InsaneRebel1::isOp0BReticleControlLevel() const {
	switch (_currentLevel) {
	case 1:  // Level 2
	case 3:  // Level 4
	case 7:  // Level 8
	case 8:  // Level 9
	case 9:  // Level 10
	case 11: // Level 12
	case 13: // Level 14
	case 14: // Level 15
		return true;
	default:
		return false;
	}
}

bool InsaneRebel1::shouldInvertTouchYSettingForCurrentLevel() const {
	return isTouchscreenActive() && isOp0BReticleControlLevel();
}

bool InsaneRebel1::usesGamepadReticleAimForCurrentFrame() const {
	return getEffectiveGameOpcode() == 0x0B;
}

void InsaneRebel1::resetGamepadReticleAim() {
	_gamepadAimAxisX = 0;
	_gamepadAimAxisY = 0;
	_gamepadAimActive = false;
}

bool InsaneRebel1::updateGamepadReticleAim(int16 &inputX, int16 &inputY, bool *usedJoystick) {
	if (!usesGamepadReticleAimForCurrentFrame())
		return false;

	const int dpadX =
		(_vm->getActionState(kScummActionInsaneRight) ? 1 : 0) -
		(_vm->getActionState(kScummActionInsaneLeft) ? 1 : 0);
	int dpadY =
		(_vm->getActionState(kScummActionInsaneUp) ? 1 : 0) -
		(_vm->getActionState(kScummActionInsaneDown) ? 1 : 0);

	const int16 analogAxisX = applyRebel1AnalogDeadzone(_joystickAxisX);
	const int16 analogAxisY = applyRebel1AnalogDeadzone(_joystickAxisY);
	const int analogX = CLIP<int32>(((int32)analogAxisX * kRA1CenteredAxisMax) / Common::JOYAXIS_MAX,
		-kRA1CenteredAxisMax, kRA1CenteredAxisMax);
	int analogY = CLIP<int32>((-(int32)analogAxisY * kRA1Op0BVerticalAxisMax) / Common::JOYAXIS_MAX,
		-kRA1Op0BVerticalAxisMax, kRA1Op0BVerticalAxisMax);

	if (_optControlsYFlip) {
		dpadY = -dpadY;
		analogY = -analogY;
	}

	int deltaX = 0;
	int deltaY = 0;
	bool activeGamepadAim = false;

	if (dpadX || dpadY) {
		deltaX = stepRebel1Op0BReticleAxis(dpadX * kRA1ControlPadAxisStep);
		deltaY = stepRebel1Op0BReticleAxis(dpadY * kRA1ControlPadAxisStep);
		activeGamepadAim = true;
	} else if (analogX || analogY) {
		deltaX = stepRebel1Op0BReticleAxis(analogX);
		deltaY = stepRebel1Op0BReticleAxis(analogY);
		activeGamepadAim = true;
	}

	if (activeGamepadAim) {
		if (!_gamepadAimActive) {
			_gamepadAimAxisX = CLIP<int16>(_avgInputX, -kRA1CenteredAxisMax, kRA1CenteredAxisMax);
			_gamepadAimAxisY = CLIP<int16>((int16)-_avgInputY, -kRA1Op0BVerticalAxisMax, kRA1Op0BVerticalAxisMax);
		}
		_gamepadAimAxisX = CLIP<int16>((int16)(_gamepadAimAxisX + deltaX), -kRA1CenteredAxisMax, kRA1CenteredAxisMax);
		_gamepadAimAxisY = CLIP<int16>((int16)(_gamepadAimAxisY + deltaY), -kRA1Op0BVerticalAxisMax, kRA1Op0BVerticalAxisMax);
		_gamepadAimActive = true;
	}

	if (!_gamepadAimActive)
		return false;

	if (usedJoystick)
		*usedJoystick = true;
	_activeInputSource = kInputSourceJoystickReticle;
	inputX = _gamepadAimAxisX;
	inputY = _gamepadAimAxisY;
	return true;
}

void InsaneRebel1::preprocessMouseAxes(int16 &inputX, int16 &inputY, bool *usedJoystick) {
	if (usedJoystick)
		*usedJoystick = false;

	if (updateGamepadReticleAim(inputX, inputY, usedJoystick))
		return;

	const uint16 effectiveOpcode = getEffectiveGameOpcode();
	const bool directFlightInput = effectiveOpcode == 0x07;
	const bool flightVariantInput = effectiveOpcode == 0x09;

	const int16 analogAxisX = applyRebel1AnalogDeadzone(_joystickAxisX);
	const int16 analogAxisY = applyRebel1AnalogDeadzone(_joystickAxisY);
	const int joyX =
		(_vm->getActionState(kScummActionInsaneRight) ? 1 : 0) -
		(_vm->getActionState(kScummActionInsaneLeft) ? 1 : 0);
	const int joyY =
		(_vm->getActionState(kScummActionInsaneUp) ? 1 : 0) -
		(_vm->getActionState(kScummActionInsaneDown) ? 1 : 0);

	if (joyX != 0 || joyY != 0) {
		_activeInputSource = kInputSourceJoystickDigital;

		if (usedJoystick)
			*usedJoystick = true;

		inputX = joyX * kRA1ControlPadAxisStep;
		inputY = joyY * kRA1ControlPadAxisStep;

		if (_optControlsYFlip)
			inputY = -inputY;

		return;
	}

	if (_activeInputSource == kInputSourceJoystickAnalog) {
		if (usedJoystick)
			*usedJoystick = true;

		if (analogAxisX != 0 || analogAxisY != 0) {
			const int axisMaxX = directFlightInput ? kRA1EnhancedFlightDirectMaxX : kRA1CenteredAxisMax;
			const int axisMaxY = directFlightInput ? kRA1EnhancedFlightDirectMaxY : kRA1CenteredAxisMax;
			inputX = CLIP<int32>(((int32)analogAxisX * axisMaxX) / Common::JOYAXIS_MAX, -axisMaxX, axisMaxX);
			inputY = CLIP<int32>(((int32)analogAxisY * axisMaxY) / Common::JOYAXIS_MAX, -axisMaxY, axisMaxY);
		} else {
			inputX = 0;
			inputY = 0;
		}

		if (_optControlsYFlip)
			inputY = -inputY;

		return;
	}

	if (_activeInputSource == kInputSourceJoystickDigital) {
		if (usedJoystick)
			*usedJoystick = true;

		inputX = 0;
		inputY = 0;

		if (_optControlsYFlip)
			inputY = -inputY;

		return;
	}

	int16 logicalX = (int16)CLIP<int>(_vm->_mouse.x, 0, 319);
	int16 logicalY = (int16)CLIP<int>(_vm->_mouse.y, 0, 199);

	if (directFlightInput || flightVariantInput) {
		inputX = scaleRebel1CenteredMouseAxis(logicalX, kRA1CenterX, kRA1EnhancedFlightDirectMaxX);
		inputY = scaleRebel1CenteredMouseAxis(logicalY, kRA1CenterY, kRA1EnhancedFlightDirectMaxY);
	} else {
		inputX = (int16)CLIP<int32>(((int32)(logicalX - kRA1CenterX) * kRA1CenteredAxisMax) / kRA1CenterX,
			-kRA1CenteredAxisMax, kRA1CenteredAxisMax);
		inputY = (int16)CLIP<int32>(((int32)(logicalY - kRA1CenterY) * kRA1CenteredAxisMax) / kRA1CenterY,
			-kRA1CenteredAxisMax, kRA1CenteredAxisMax);
	}

	bool flipY = _optControlsYFlip;
	if (shouldInvertTouchYSettingForCurrentLevel())
		flipY = !flipY;
	if (flipY)
		inputY = -inputY;
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
		_flightAimX = kRA1CenterX;
		_flightAimY = kRA1CenterY;
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
	const uint16 effectiveOpcode = getEffectiveGameOpcode();
	int16 inputX = 0;
	int16 inputY = 0;
	bool usedJoystick = false;
	preprocessMouseAxes(inputX, inputY, &usedJoystick);
	const int16 rawInputX = inputX;
	const int16 rawInputY = inputY;
	inputX = CLIP<int16>(inputX, -kRA1CenteredAxisMax, kRA1CenteredAxisMax);
	inputY = CLIP<int16>(inputY, -kRA1CenteredAxisMax, kRA1CenteredAxisMax);
	const char *inputSourceName = "mouse";
	if (_activeInputSource == kInputSourceJoystickAnalog)
		inputSourceName = "joystick-analog";
	else if (_activeInputSource == kInputSourceJoystickDigital)
		inputSourceName = "joystick-dpad";

	// --- Step 2: Roll accumulator (_74CA) ---
	// Normal mode: accumulate. For absolute mouse input in flight handlers,
	// steer toward a bounded roll target so holding the cursor off center does
	// not continue accelerating the ship until it clamps.
	if ((effectiveOpcode == 0x07 || effectiveOpcode == 0x09) &&
			_activeInputSource == kInputSourceMouse && !usedJoystick) {
		const int32 targetRoll = (int32)inputX * kRA1MouseFlightRollTargetScale;
		_rollAccum += (targetRoll - _rollAccum) >> 2;
	} else {
		_rollAccum += (_tuning.roll * (int32)inputX) >> 5;
	}
	_rollAccum = CLIP<int32>(_rollAccum, -0x47F, 0x47F);

	// --- Step 3: Vertical smoothing (_74CE) ---
	// Exponential decay toward -inputY
	_liftSmooth += (-_liftSmooth - (int32)inputY) >> 1;
	_liftSmooth = CLIP<int32>(_liftSmooth, -0x20, 0x20);

	// --- Step 4: Position accumulator deltas ---
	// X delta: drift + slide coupling - cross-coupling
	int32 rng = 100;  // RandScaleByte(200), centered at 100
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

	// --- Step 7: Perspective offsets (SetCameraOffset) ---
	// FUN_1DEB5 computes these linearly from ship offsets:
	//   viewX = clamp((_74BA + 0x20), 0, 0x40)
	//   viewY = clamp((_74BC + 0x17), 0, 0x2E)
	_perspectiveX = CLIP<int16>((int16)(_shipPosX - kRA1CenterX + 0x20), 0, 0x40);
	_perspectiveY = CLIP<int16>((int16)(_shipPosY - kRA1CenterY + 0x17), 0, 0x2E);
	// Screen shake: when enabled, add random ±2 jitter (original SetCameraOffset at 0x22514)
	if (_screenShakeEnabled) {
		_perspectiveX = CLIP<int16>((int16)(_perspectiveX + (_vm->_rnd.getRandomNumber(4) - 2)), 0, 0x40);
		_perspectiveY = CLIP<int16>((int16)(_perspectiveY + (_vm->_rnd.getRandomNumber(4) - 2)), 0, 0x2E);
	}

	// FUN_1DEB5 updates the curve table via FUN_22549 after SetCameraOffset.
	// The full DOS path blends a few roll-history terms; use the current roll
	// accumulator so side-looking still bends the gameplay projection. DOS
	// negates before the arithmetic shift.
	rebuildProjectionTable((int16)CLIP<int32>((-_rollAccum) >> 7, -0x1A, 0x1A), 0x1A);

	// --- Step 8: Direction sprite index (FUN_1DEB5 LAB_1e23e) ---
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

	debugC(DEBUG_INSANE, "ship input: frame=%d source=%s controls=%s turbulence=%d usedJoystick=%d raw=(%d,%d) clipped=(%d,%d) storedAxis=(%d,%d) actionState(L,R,U,D)=(%d,%d,%d,%d) roll=%d lift=%d pos=(%d,%d) view=(%d,%d) dir=%d level=%d mode=%d opcode=0x%X",
		_gameCounter, inputSourceName,
		"enhanced", 0, usedJoystick,
		rawInputX, rawInputY, inputX, inputY,
		_joystickAxisX, _joystickAxisY,
		_vm->getActionState(kScummActionInsaneLeft),
		_vm->getActionState(kScummActionInsaneRight),
		_vm->getActionState(kScummActionInsaneUp),
		_vm->getActionState(kScummActionInsaneDown),
		_rollAccum, _liftSmooth,
		_shipPosX, _shipPosY, _perspectiveX, _perspectiveY, _shipDirIndex,
		_currentLevel, _flyControlMode, _activeGameOpcode);

	// --- Step 9: Damage/event bit synthesis + damage processing ---
	// RA1 FUN_1B297-style latches from GAME opcodes:
	//   0x5D latch 0xFFFF -> bit 0x40 (obstacle/contact)
	//   0x5F non-zero + RNG -> bit 0x80 (projectile-like hit)
	if (_gameLatch5D == 0xFFFF)
		_damageFlags |= 0x40;
	if (_gameLatch5F != 0 && _vm->_rnd.getRandomNumber((uint16)(_gameLatch5F - 1)) == 0)
		_damageFlags |= 0x80;

	// Damage guard/mask from FUN_1DEB5: (_damageFlags & 0x96) != 0
	// damageFlags & 0x96 = bits 1,2,4,7 = wall collisions (0x16) + projectile hit (0x80)
	if (!_noDamage && (_damageFlags & 0x96) != 0 && _damageCooldown == 0 &&
		_health >= 0 && _deathTimer <= 0) {
		// Projectile hit (bit 7 = 0x80)
		if (_damageFlags & 0x80)
			_health -= _tuning.shot;
		// Wall collision (bits 1,2,4 = 0x16)
		if (_damageFlags & 0x16)
			_health -= _tuning.wham;

		if (_health < 0) {
			_deathTimer = kDeathTimerInit;
			// g_deathCauseIndicator (0x772E) — set based on damage source
			if (_damageFlags & 0x80)
				_deathCauseIndicator = 2;  // Projectile hit death
			else
				_deathCauseIndicator = 1;  // Collision death
		}

		_prevDamageFlags = _damageFlags;
		_damageCooldown = kDamageCooldownInit;
		// HandleGameOp07_ShipFlight dispatches g_sfxDamageHit here.
		playSfx(kSfxBoom, 127, 0);
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

	// Screen flash decay — screen shake follows flash (EnableScreenShake/DisableScreenShake at 0x224ED)
	if (_screenFlash > 0) {
		_screenFlash--;
		_screenShakeEnabled = (_screenFlash > 0);
	}

	// Clear per-frame damage flags
	_damageFlags = 0;

	// --- Path branching detection ---
	// Original (FUN_1B297): at GAME counter 394 (0x18A), sets nextSceneA=0x67/nextSceneB=0x69.
	// After this point, drift goes strongly negative (pushing ship left for the hard path).
	// If ship is right of center, player chose the hard branch → switch to L1PLAY1R.
	// Keep this as a one-shot decision: once threshold is reached, lock path.
	if (_pathBranchEnabled && _gameCounter >= kPathBranchCounter) {
		if (_shipPosX > kRA1CenterX) {
			_rightPathSelected = true;
			preserveInteractiveVideoAudioState();
			_vm->_smushVideoShouldFinish = true;
			debugC(DEBUG_INSANE, "Right path selected (counter=%d, shipX=%d)", _gameCounter, _shipPosX);
		} else {
			debugC(DEBUG_INSANE, "Left path retained (counter=%d, shipX=%d)", _gameCounter, _shipPosX);
		}
		_pathBranchEnabled = false;
	}

	if (_currentLevel != 6)
		checkDynamicLevelBranch();

	debugC(DEBUG_INSANE, "ship: pos=(%d,%d) roll=%d lift=%d accX=%d accY=%d dir=%d health=%d corridor=[%d,%d]-[%d,%d]",
		_shipPosX, _shipPosY, _rollAccum, _liftSmooth,
		_posAccumX, _posAccumY, _shipDirIndex, _health,
		_corridorLeftX, _corridorTopY, _corridorRightX, _corridorBottomY);
}

// Port helper: FUN_1E6A7 computes this direction bucket inline before applying
// the current frame's movement update.
void InsaneRebel1::updateTurretShipDirection(int16 offsetY) {
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
}

void InsaneRebel1::getCollisionShipCenter(int16 &x, int16 &y) const {
	// Original 0x0D/0x0E collision compares script zones transformed by
	// FUN_223FE against the gameplay-window ship center (base center +
	// g_shipOffset). This is DOS screen space; render overlays add the viewport
	// offset separately when drawing into the larger source buffer.
	//
	// In Level 1 part 2, HandleGameOp0A_TurretVariant reuses _shipPos for the
	// targeting cursor, so collision must read the movement accumulator instead.
	if (_currentLevel == 0 && _flyControlMode == 2) {
		x = (int16)(kRA1CenterX + (int16)(_posAccumX >> 8));
		y = (int16)(kRA1CenterY + (int16)(_posAccumY >> 8));
	} else {
		x = _shipPosX;
		y = _shipPosY;
	}
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

	if (_gameLatch5D == 0xFFFF)
		_damageFlags |= 0x40;
	if (_gameLatch5F != 0 && _vm->_rnd.getRandomNumber((uint16)(_gameLatch5F - 1)) == 0)
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

	// GAME 0x0A appears before 0x08 in turret/combat ANMs. The original
	// draws shots/targeting and the ship from this pre-physics center, then
	// updates the camera offset at the end of 0x08 for the final viewport copy.
	const int16 preMoveOffsetX = (int16)(_posAccumX >> 8);
	const int16 preMoveOffsetY = (int16)(_posAccumY >> 8);
	_turretFrameShipOffsetX = preMoveOffsetX;
	_turretFrameShipOffsetY = preMoveOffsetY;
	_turretFrameShipCenterX = (int16)(kRA1CenterX + preMoveOffsetX);
	_turretFrameShipCenterY = (int16)(kRA1CenterY + preMoveOffsetY);
	_turretFrameShipCenterValid = true;

	// Damage gate from FUN_1E6A7.
	if (!_noDamage && _damageFlags != 0 && _damageCooldown == 0 && _health >= 0 && _deathTimer <= 0) {
		if (_damageFlags == 0x80)
			_health -= _tuning.shot;
		else
			_health -= _tuning.wham;

		if (_health < 0) {
			_deathTimer = kDeathTimerInit;
			if (_damageFlags & 0x80)
				_deathCauseIndicator = 2;
			else
				_deathCauseIndicator = 1;
		}

		_prevDamageFlags = _damageFlags;
		_damageCooldown = kDamageCooldownInit;
		// HandleGameOp08_TurretFlight dispatches g_sfxDamageHit here.
		playSfx(kSfxBoom, 127, 0);
		_screenFlash = 3;
	}

	if (_damageCooldown > 0)
		_damageCooldown--;

	if (_health < 0 && _deathTimer > 0)
		_deathTimer--;

	updateTurretShipDirection(preMoveOffsetY);

	// FUN_1E6A7 movement gate: counter > 8 or flags bit 0x40.
	if (counter > 8 || (modeFlags & 0x40)) {
		// FUN_1E6A7 consumes DAT_756C/DAT_756E from the shared input bridge,
		// not raw mouse coordinates.
		int16 inputX = 0;
		int16 inputY = 0;
		bool usedJoystick = false;
		preprocessMouseAxes(inputX, inputY, &usedJoystick);
		inputX = CLIP<int16>(inputX, -kRA1CenteredAxisMax, kRA1CenteredAxisMax);
		inputY = CLIP<int16>(inputY, -kRA1CenteredAxisMax, kRA1CenteredAxisMax);
		const int16 rawInputX = inputX;
		const int16 rawInputY = inputY;

		if (usedJoystick && _flyControlMode == 2) {
			// Extra concession for Level 1 part 2. The original 0x08 handler
			// uses raw axes directly; do not damp Level 13's surface controls.
			inputX /= 2;
			inputY /= 2;
		}

		debugC(DEBUG_INSANE, "turret input: source=%s controls=%s mouse=(%d,%d) actions(L,R,U,D)=(%d,%d,%d,%d) raw=(%d,%d) final=(%d,%d) level=%d mode=%d opcode=0x%X",
			usedJoystick ? "joystick-actions" : "mouse-path",
			"enhanced",
			_vm->_mouse.x, _vm->_mouse.y,
			_vm->getActionState(kScummActionInsaneLeft),
			_vm->getActionState(kScummActionInsaneRight),
			_vm->getActionState(kScummActionInsaneUp),
			_vm->getActionState(kScummActionInsaneDown),
			rawInputX, rawInputY, inputX, inputY,
			_currentLevel, _flyControlMode, _activeGameOpcode);

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
	// DOS negates before the arithmetic shift.
	rebuildProjectionTable((int16)((-_rollAccum) >> 9), 0x0D);

	// Regeneration + survival bonus via FUN_1BB0E call in this path.
	if ((_frameCounter & 0x1F) == 0) {
		if (_health >= 0 && _health < kMaxHealth)
			_health++;
		if (_health >= 0)
			_score += _tuning.time;
	}

	if (_screenFlash > 0) {
		_screenFlash--;
		_screenShakeEnabled = (_screenFlash > 0);
	}

	_gameLatch5D = 0;
	_gameLatch5F = 0;
	_damageFlags = 0;
}

// New port helpers for the palette-flash block that is inline in FUN_1CDA7.
void InsaneRebel1::restoreScreenFlashPalette() {
	if (!_screenFlashBasePaletteValid)
		return;

	if (_player)
		_player->setPalette(_screenFlashBasePalette);
	_screenFlashBasePaletteValid = false;
}

void InsaneRebel1::updateScreenFlashPalette() {
	if (!_player) {
		_screenFlashBasePaletteValid = false;
		return;
	}

	if (_screenFlash <= 0) {
		restoreScreenFlashPalette();
		return;
	}

	if (!_screenFlashBasePaletteValid) {
		memcpy(_screenFlashBasePalette, _player->getVideoPalette(), sizeof(_screenFlashBasePalette));
		_screenFlashBasePaletteValid = true;
	}

	byte flashPalette[0x300];
	const int blend = CLIP<int>(8 - _screenFlash, 0, 8);
	for (int i = 0; i < 0x300; i++)
		flashPalette[i] = (byte)(0xFF - (((0xFF - _screenFlashBasePalette[i]) * blend) >> 3));
	_player->setPalette(flashPalette);
}

// updateGameOp0BPhysics — FUN_1CDA7 (0x1CDA7). GAME opcode 0x0B handler.
// Uses 10-frame input history averaging instead of accumulators.
// Ship position = averaged input + center offset.
// Viewport = second history buffer for smooth camera scrolling.
void InsaneRebel1::updateGameOp0BPhysics() {
	// Enhanced controls keep the original 0x0B pipeline but average fewer
	// samples for responsiveness.
	const int gameOp0BSmoothWindow = 2;

	// RA1 FUN_1B297-style per-frame latches for 0x0B sections:
	//   0x5D latch 0xFFFF -> bit 0x40 (scripted obstacle/contact)
	//   0x5F non-zero + RNG -> bit 0x80 (scripted random hit)
	const bool level15Phase1 = (_currentLevel == 14 && _levelGameplayPhase == 1);
	const bool level15FinalPhase = (_currentLevel == 14 && _levelGameplayPhase == 2);
	const bool level14Phase1 = (_currentLevel == 13 && _levelGameplayPhase == 1);
	const bool level14Phase2 = (_currentLevel == 13 && _levelGameplayPhase == 2);
	bool level2AsteroidHit = false;

	if (_gameLatch5D == 0xFFFF ||
		(_currentLevel == 1 && isLevel2DamageLatch(_gameLatch5D)) ||
		(_currentLevel == 3 && isLevel4DamageLatch(_gameLatch5D)) ||
		(_currentLevel == 5 && isLevel6DamageLatch(_gameLatch5D)) ||
		(_currentLevel == 9 && isLevel10DamageLatch(_gameLatch5D)) ||
		(_currentLevel == 11 && isLevel12DamageLatch(_gameLatch5D)) ||
		(level14Phase2 && isLevel14Phase2DamageLatch(_gameLatch5D)) ||
		(level15FinalPhase && isLevel15FinalDamageLatch(_gameLatch5D)))
		_damageFlags |= 0x40;
	if (_gameLatch5F != 0 && !level14Phase2) {
		bool randomProjectileHit = false;
		// Original level loops spell these fixed-probability cases separately.
		// The shared 0x0B path collapses them into one branch.
		if (level14Phase1)
			randomProjectileHit = (_vm->_rnd.getRandomNumber(3) == 0);
		else if (_currentLevel == 3 || _currentLevel == 9 || _currentLevel == 11 ||
				level15Phase1 || level15FinalPhase)
			randomProjectileHit = (_vm->_rnd.getRandomNumber(2) == 0);
		else
			randomProjectileHit = (_vm->_rnd.getRandomNumber((uint16)(_gameLatch5F - 1)) == 0);

		if (randomProjectileHit)
			_damageFlags |= 0x80;
	}

	if (_currentLevel == 5 && hasLevel6PerspectiveHazard((uint16)_frameCounter, _perspectiveX, _perspectiveY))
		_damageFlags |= 0x20;
	if (level15FinalPhase && hasLevel15FinalSweepDamage((uint16)_gameCounter, _perspectiveX))
		_damageFlags |= 0x20;

	if (_currentLevel == 1) {
		level2AsteroidHit = hasLevel2AsteroidImpact((uint16)_gameCounter, _perspectiveX, _perspectiveY);
		if (level2AsteroidHit)
			_damageFlags |= 0x20;
	}

	bool level8WalkerPlayerHit = false;
	if (_currentLevel == 7) {
		const uint16 walkerFrame = (uint16)_gameCounter;
		level8WalkerPlayerHit = hasLevel8WalkerPlayerHit(_levelRouteIndex, walkerFrame,
			_perspectiveX, _perspectiveY);
		// RunLevel8Flow sets damage flag 0x20 when the AT-AT route contact
		// helper hits the player; boss damage uses _walkerHealth separately.
		if (level8WalkerPlayerHit)
			_damageFlags |= 0x20;
	}

	// Health regeneration (FUN_1BB0E): +1 every 32 frames when alive
	if (_health >= 0 && _health < kMaxHealth && (_frameCounter & 0x1F) == 0) {
		_health++;
	}

	// Damage application (FUN_1CDA7 lines 20-41)
	// Original 0x0B mapping: 0x80 -> +0x13, 0x40 -> +0x0F, 0x20 -> +0x11.
	// No cooldown — all three damage types can stack each frame
	if (!_noDamage && _damageFlags != 0 && _health >= 0 && _deathTimer < 1) {
		const int16 oldHealth = _health;
		const byte appliedDamageFlags = _damageFlags;
		_screenFlash = 5;
		if (_damageFlags & 0x80)
			_health -= _tuning.shot;
		if (_damageFlags & 0x40)
			_health -= _tuning.miss;
		if (_damageFlags & 0x20)
			_health -= _tuning.wham;
		if (_health < 0) {
			_deathTimer = 15;  // 0x0F — shorter than Level 1's 30
			if (_damageFlags & 0x80)
				_deathCauseIndicator = 2;
			else
				_deathCauseIndicator = 1;
		}
		// FUN_1CDA7 dispatches g_sfxDamageHit, initialized from SYS/BOOM.SAD.
		playSfx(kSfxBoom, 127, 0);
		if (_currentLevel == 1) {
			debugC(DEBUG_INSANE, "L2 player hit: frame=%u view=(%d,%d) latch=%u asteroid=%d flags=0x%02x health=%d->%d",
				(unsigned)(uint16)_gameCounter, _perspectiveX, _perspectiveY,
				(unsigned)_gameLatch5D, level2AsteroidHit ? 1 : 0,
				appliedDamageFlags, oldHealth, _health);
		}
		if (level8WalkerPlayerHit) {
			debugC(DEBUG_INSANE, "L8 player hit by walker: route=%d frame=%u view=(%d,%d) flags=0x%02x health=%d->%d",
				CLIP<int>(_levelRouteIndex, 0, 2), (unsigned)(uint16)_gameCounter,
				_perspectiveX, _perspectiveY, appliedDamageFlags, oldHealth, _health);
		}
		_prevDamageFlags = appliedDamageFlags;
		_damageFlags = 0;
	}

	// Latches are frame-local event inputs in the original pipeline.
	_gameLatch5D = 0;
	_gameLatch5F = 0;

	// Death fade countdown
	if (_deathTimer > 1 && _health < 0) {
		_deathTimer--;
	}

	// Screen flash countdown — screen shake follows flash
	if (_screenFlash > 0) {
		_screenFlash--;
		_screenShakeEnabled = (_screenFlash > 0);
	}
	updateScreenFlashPalette();

	// --- Cursor and perspective smoothing (FUN_1CDA7) ---
	// _inputHistory* maps to 0x7580/0x7594, _viewHistory* to 0x75A8/0x75BC.
	int16 inputX = 0;
	int16 inputY = 0;
	bool usedJoystick = false;
	preprocessMouseAxes(inputX, inputY, &usedJoystick);
	inputX = CLIP<int16>(inputX, -0xA0, 0xA0);
	inputY = CLIP<int16>(inputY, -kRA1Op0BVerticalAxisMax, kRA1Op0BVerticalAxisMax);
	const int16 rawInputX = inputX;
	const int16 rawInputY = inputY;
	const bool op0BAnalogSmoothing = (_activeInputSource == kInputSourceJoystickAnalog);
	const char *inputSourceName = "mouse-path";

	if (_activeInputSource == kInputSourceJoystickAnalog)
		inputSourceName = "joystick-analog";
	else if (_activeInputSource == kInputSourceJoystickDigital)
		inputSourceName = "joystick-dpad";
	else if (_activeInputSource == kInputSourceJoystickReticle)
		inputSourceName = "joystick-reticle";

	const bool relativeGamepadReticle = usedJoystick && _activeInputSource == kInputSourceJoystickReticle;
	const bool preciseGamepadReticle = usedJoystick && isOp0BReticleControlLevel() &&
		!relativeGamepadReticle;

	if (usedJoystick && !relativeGamepadReticle) {
		// The 0x0B first-person handler is shared by multiple RA1 stages. Smooth
		// analog stick input over time and shape the affected gamepad levels with
		// a low-gain linear center plus an accelerating outer range.
		if (op0BAnalogSmoothing) {
			if (preciseGamepadReticle) {
				inputX = shapeRebel1Op0BGamepadAxis(inputX, kRA1CenteredAxisMax);
				inputY = shapeRebel1Op0BGamepadAxis(inputY, kRA1Op0BVerticalAxisMax);
			}
			const int analogRampDivisor = preciseGamepadReticle ? 20 : 10;
			inputX = smoothRebel1Op0BAnalogInput(inputX, _level2JoystickFilteredX, kRA1CenteredAxisMax, analogRampDivisor);
			inputY = smoothRebel1Op0BAnalogInput(inputY, _level2JoystickFilteredY, kRA1Op0BVerticalAxisMax, analogRampDivisor);
		} else {
			_level2JoystickFilteredX = 0;
			_level2JoystickFilteredY = 0;
			inputX /= 2;
			inputY /= 2;
		}
	} else {
		_level2JoystickFilteredX = 0;
		_level2JoystickFilteredY = 0;
	}
	_inputAxisDeltaX = inputX;

	debugC(DEBUG_INSANE, "GAME 0x0B input: frame=%d source=%s controls=%s window=%d precisionPad=%d view=(%d,%d) health=%d prevFlags=0x%02x axis=(%d,%d) mouse=(%d,%d) actions(L,R,U,D)=(%d,%d,%d,%d) raw=(%d,%d) final=(%d,%d) level=%d opcode=0x%X",
		_gameCounter,
		inputSourceName,
		"enhanced",
		gameOp0BSmoothWindow,
		preciseGamepadReticle ? 1 : 0,
		_perspectiveX, _perspectiveY,
		_health, _prevDamageFlags,
		_joystickAxisX, _joystickAxisY,
		_vm->_mouse.x, _vm->_mouse.y,
		_vm->getActionState(kScummActionInsaneLeft),
		_vm->getActionState(kScummActionInsaneRight),
		_vm->getActionState(kScummActionInsaneUp),
		_vm->getActionState(kScummActionInsaneDown),
		rawInputX, rawInputY, inputX, inputY,
		_currentLevel, _activeGameOpcode);

	for (int i = kInputHistorySize - 1; i > 0; i--) {
		_inputHistoryX[i] = _inputHistoryX[i - 1];
		_inputHistoryY[i] = _inputHistoryY[i - 1];
	}
	_inputHistoryX[0] = inputX;
	_inputHistoryY[0] = inputY;

	int sumInputX = 0;
	int sumInputY = 0;
	for (int i = 0; i < gameOp0BSmoothWindow; i++) {
		sumInputX += _inputHistoryX[i];
		sumInputY += _inputHistoryY[i];
	}

	_avgInputX = (int16)(sumInputX / gameOp0BSmoothWindow);
	_avgInputY = (int16)(-sumInputY / gameOp0BSmoothWindow);
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
	for (int i = 0; i < gameOp0BSmoothWindow; i++) {
		sumViewX += _viewHistoryX[i];
		sumViewY += _viewHistoryY[i];
	}

	int16 avgViewX = (int16)(sumViewX / gameOp0BSmoothWindow);
	int16 avgViewY = (int16)(sumViewY / gameOp0BSmoothWindow);
	_perspectiveX = CLIP<int16>((int16)((avgViewX >> 1) + 0x20), 0, 0x40);
	_perspectiveY = CLIP<int16>((int16)((avgViewY >> 1) + 0x17), 0, 0x2E);
	resetProjectionTable();

	_frameCounter++;

	if (_currentLevel == 11) {
		// RunLevel12Flow checks the three attackers around frame 0x550. The
		// event-mask bits map to L12PLAY's one-based FOBJ IDs 195, 197, and 199
		// in the port's frame-object helper. On failure the original keeps
		// pumping L12PLAY until frame 0x564, plays L12RETRY, then restarts
		// L12PLAY without resetting health.
		if (_levelGameplayPhase == 0 && _frameCounter == 0x550) {
			const bool targetADestroyed = isFrameObjectPrimarySet(kRA1Level12TargetA);
			const bool targetBDestroyed = isFrameObjectPrimarySet(kRA1Level12TargetB);
			const bool targetCDestroyed = isFrameObjectPrimarySet(kRA1Level12TargetC);
			if (!targetADestroyed || !targetBDestroyed || !targetCDestroyed) {
				_levelGameplayPhase = 1;
				debugC(DEBUG_INSANE, "L12 retry armed: frame=0x%04x targets=(%d,%d,%d)",
					_frameCounter,
					targetADestroyed ? 1 : 0,
					targetBDestroyed ? 1 : 0,
					targetCDestroyed ? 1 : 0);
			}
		}
		if (_levelGameplayPhase == 1 && _frameCounter >= 0x564)
			_vm->_smushVideoShouldFinish = true;
	}

	if (_currentLevel == 13 && (_levelGameplayPhase == 1 || _levelGameplayPhase == 2)) {
		const bool targetsDestroyed = (_levelGameplayPhase == 1) ?
			areLevel14Phase1TargetsDestroyed() : areLevel14Phase2TargetsDestroyed();
		if (targetsDestroyed && _level14SuccessFrames < 0x3C)
			_level14SuccessFrames++;
		if (_level14SuccessFrames >= 0x3C)
			_vm->_smushVideoShouldFinish = true;
	}

	// Level 4 Phase 2: enable torpedo mode at frontend/movie frame 0x3E and
	// finish as soon as the torpedo registers a hit. The DOS loop exits on killCount.
	if (_currentLevel == 3 && _levelGameplayPhase == 2) {
		if (_currentSmushFrame == 0x3E)
			_gameplayFlags75ff |= 2;
		if (_killCount > 0)
			_vm->_smushVideoShouldFinish = true;
	}

	// Level 4 Phase 1: track shield generator hits per frame.
	// Original (RunLevel4Flow): g_recentKillObjectIdPlus1 checked every frame.
	// When enough hits accumulated (>0x30), generator is "destroyed" (clear protectedTarget).
	// When both destroyed for 60 frames, phase ends.
	if (_currentLevel == 3 && _levelGameplayPhase == 1) {
		if (_lastHitTarget == _protectedTargetA && _protectedTargetA != 0)
			_shieldGenHitsA++;
		if (_lastHitTarget == _protectedTargetB && _protectedTargetB != 0)
			_shieldGenHitsB++;

		if (_shieldGenHitsA > 0x30)
			_protectedTargetA = 0;
		if (_shieldGenHitsB > 0x30)
			_protectedTargetB = 0;

		// Both destroyed: count down 60 frames then end phase
		if (_protectedTargetA == 0 && _protectedTargetB == 0 &&
			_shieldGenHitsA > 0x30 && _shieldGenHitsB > 0x30) {
			_shieldGenHitsA++;  // reuse as countdown
			if (_shieldGenHitsA > 0x30 + 0x3C)
				_vm->_smushVideoShouldFinish = true;
		}
	}

	// Level 5 Phase 1: DOS RunLevel5Flow exits L5PLAY only after killCount stays
	// above 2 for 20 frontend frames. That countdown is carried by the runlevel,
	// not by opcode 0x07 itself.
	if (_currentLevel == 4 && _levelGameplayPhase == 1 &&
		_level5SuccessFramesRemaining > 0 && _killCount > 2 && !_vm->_smushVideoShouldFinish) {
		_level5SuccessFramesRemaining--;
		if (_level5SuccessFramesRemaining == 0)
			_vm->_smushVideoShouldFinish = true;
	}

	// Level 15 Phase 2: enable torpedo at frontend/movie frame 0x18A, expose
	// the protected target IDs used by the original flow, and finish when
	// g_gameplayPhaseFlags & 2 becomes set.
	if (_currentLevel == 14 && _levelGameplayPhase == 2) {
		if (_currentSmushFrame == 0x18A) {
			// Original writes the 16-bit flags word: g_hudDisableFlags |= 0x210.
			// Low byte bit 0x10 suppresses normal hit feedback in the torpedo phase;
			// high byte bit 0x02 switches targeting/shot rendering to torpedoes.
			_gameplayFlags75fe |= 0x10;
			_gameplayFlags75ff |= 2;
			_protectedTargetA = 0x67;
			_protectedTargetB = 0x69;
		}
		if (areFrameObjectPrimaryBitsSet(0, 0x02)) {
			_torpedoFired = true;
			_vm->_smushVideoShouldFinish = true;
		}
	}

	checkDynamicLevelBranch();

	debugC(DEBUG_INSANE, "GAME 0x0B: pos=(%d,%d) avg=(%d,%d) view=(%d,%d) health=%d flash=%d",
		_shipPosX, _shipPosY, _avgInputX, _avgInputY,
		_perspectiveX, _perspectiveY, _health, _screenFlash);
}

bool InsaneRebel1::isTorpedoModeActive() const {
	if ((_gameplayFlags75ff & 0x2) == 0)
		return false;

	// The original high-byte flag is only intentionally armed by the level 4
	// torpedo run and the level 15 exhaust-port run. Gate the port's rendering
	// and shot behavior to those phases so stale route/retry state cannot turn
	// ordinary laser sections into torpedo mode.
	return (_currentLevel == 3 && _levelGameplayPhase == 2) ||
		(_currentLevel == 14 && _levelGameplayPhase == 2);
}


// Helper splits for the original on-foot GAME handlers:
// HandleGameOp19_OnFootSequence (0x19) and HandleGameOp1A_OnFootVariant (0x1A).
// On-foot handler for Level 9 (Stormtroopers). Character walks left/right, crosshair tracks mouse.
//
// Original has TWO separate variable pairs:
//   DAT_000041a0/41a2 = camera offset (SetCameraOffset, ProjectPointToScreen)
//   g_perspectiveX/Y  = crosshair center (on-foot targeting)
// Our _perspectiveX/_perspectiveY maps to the camera offset (DAT_000041a0/41a2).
// The crosshair center (0xA3, 0x82) is a separate constant for on-foot mode.
// Port split matching HandleGameOp19_OnFootSequence. The helper name is new to
// this implementation; the original code dispatches the opcode handler directly.
void InsaneRebel1::initOnFootSequence() {
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
}

void InsaneRebel1::preprocessOnFootAim(int16 &inputX, int16 &inputY, bool *usedJoystick) {
	if (usedJoystick)
		*usedJoystick = false;

	const int dpadX =
		(_vm->getActionState(kScummActionInsaneRight) ? 1 : 0) -
		(_vm->getActionState(kScummActionInsaneLeft) ? 1 : 0);
	int dpadY =
		(_vm->getActionState(kScummActionInsaneDown) ? 1 : 0) -
		(_vm->getActionState(kScummActionInsaneUp) ? 1 : 0);

	const int16 analogAxisX = applyRebel1AnalogDeadzone(_joystickAxisX);
	int16 analogAxisY = applyRebel1AnalogDeadzone(_joystickAxisY);
	if (_optControlsYFlip) {
		dpadY = -dpadY;
		analogAxisY = (int16)-analogAxisY;
	}

	int deltaX = 0;
	int deltaY = 0;
	bool activeGamepadAim = false;

	if (dpadX || dpadY) {
		deltaX = dpadX * kOnFootGamepadStep;
		deltaY = dpadY * kOnFootGamepadStep;
		activeGamepadAim = true;
	} else if (_activeInputSource == kInputSourceJoystickAnalog && (analogAxisX || analogAxisY)) {
		const int analogX = CLIP<int32>(((int32)analogAxisX * kRA1CenteredAxisMax) / Common::JOYAXIS_MAX,
			-kRA1CenteredAxisMax, kRA1CenteredAxisMax);
		const int analogY = CLIP<int32>(((int32)analogAxisY * kRA1CenteredAxisMax) / Common::JOYAXIS_MAX,
			-kRA1CenteredAxisMax, kRA1CenteredAxisMax);
		deltaX = stepRebel1Op0BReticleAxis(analogX);
		deltaY = stepRebel1Op0BReticleAxis(analogY);
		activeGamepadAim = (deltaX != 0 || deltaY != 0);
	}

	if (activeGamepadAim) {
		if (!_gamepadAimActive) {
			_gamepadAimAxisX = CLIP<int16>(_shipPosX, kOnFootCursorMinX, kOnFootCursorMaxX);
			_gamepadAimAxisY = CLIP<int16>(_shipPosY, kOnFootCursorMinY, kOnFootCursorMaxY);
		}

		_gamepadAimAxisX = CLIP<int16>((int16)(_gamepadAimAxisX + deltaX), kOnFootCursorMinX, kOnFootCursorMaxX);
		_gamepadAimAxisY = CLIP<int16>((int16)(_gamepadAimAxisY + deltaY), kOnFootCursorMinY, kOnFootCursorMaxY);
		_gamepadAimActive = true;

		if (usedJoystick)
			*usedJoystick = true;
		inputX = (int16)(_gamepadAimAxisX - kOnFootCenterX);
		inputY = (int16)(_gamepadAimAxisY - kOnFootCursorBaseY);
		return;
	}

	if (_gamepadAimActive && _activeInputSource != kInputSourceMouse) {
		if (usedJoystick)
			*usedJoystick = true;
		inputX = (int16)(_gamepadAimAxisX - kOnFootCenterX);
		inputY = (int16)(_gamepadAimAxisY - kOnFootCursorBaseY);
		return;
	}

	_gamepadAimActive = false;
	const int16 logicalX = (int16)CLIP<int>(_vm->_mouse.x, kOnFootCursorMinX, kOnFootCursorMaxX);
	const int16 logicalY = (int16)CLIP<int>(_vm->_mouse.y, kOnFootCursorMinY, kOnFootCursorMaxY);
	inputX = (int16)(logicalX - kOnFootCenterX);
	inputY = (int16)(logicalY - kOnFootCursorBaseY);
}

// Port split matching HandleGameOp19_OnFootSequence. The helper name is new to
// this implementation; the original code dispatches the opcode handler directly.
void InsaneRebel1::updateOnFootSequence() {
	initOnFootSequence();

	// --- 0x19: Post-draw character walk animation + damage ---
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
	} else if (_onFootAnimCounter < 5 && !_playerSecondaryHeld) {
		// Original calls QuantizeDirection8Way with the cursor and character
		// center, but the DOS on-foot axis is mirrored relative to the screen
		// coordinates used by this port. Use the visual screen-space vector so
		// L9PILOT.NUT poses 11..14 aim left and 16..19 aim right.
		const int16 centerX = _onFootCharX + kOnFootCenterX;
		const int16 centerY = _onFootCharY + kOnFootCenterY;
		const int16 aimDir = CLIP<int16>(
			(int16)ra1ShotDirection(centerX, centerY, _shipPosX, _shipPosY), -4, 4);
		_shipDirIndex = aimDir + 15;
	} else {
		// Walking based on input direction. The 3DO second held button skips the
		// early aim-pose branch above and reaches these walk tests immediately.
		int16 inputX = (int16)(_shipPosX - kOnFootCenterX);
		int16 inputY = (int16)(_shipPosY - kOnFootCursorBaseY);
		if (!hasFrameGameOpcode(0x1A))
			preprocessOnFootAim(inputX, inputY);
		if (inputX > 0x1E && _onFootCharX < 0x73)
			_shipDirIndex = 6;  // Walk right
		else if (inputX < -0x1E && _onFootCharX > -0x73)
			_shipDirIndex = 4;  // Walk left
	}

	// --- Scripted damage latches → damageFlags ---
	// L9 on-foot trooper shots use ordinary 0x5D event ids, gated by the
	// 0x5D object bitmask handler, then consumed by FUN_1ED95 through 0x74D4.
	// The ship loops keep narrower level-specific 0x5D damage rules.
	if (_gameLatch5D != 0)
		_damageFlags |= 0x40;
	if (_gameLatch5F != 0 &&
		_vm->_rnd.getRandomNumber((uint16)(_gameLatch5F - 1)) == 0)
		_damageFlags |= 0x80;

	// --- Damage handling (from HandleGameOp19_OnFootSequence) ---
	// On-foot damage uses the same heavy-damage tuning byte as ship shot/collision
	// damage in the original, not the miss penalty.
	if (!_noDamage && _damageFlags != 0 && _damageCooldown == 0 && _health >= 0 && _deathTimer < 1) {
		const int16 oldHealth = _health;
		_health -= _tuning.shot;
		if (_health < 0) {
			_deathTimer = 15;
			_deathCauseIndicator = (_damageFlags & 0x80) ? 2 : 1;
		}
		_prevDamageFlags = _damageFlags;
		_damageCooldown = 3;
		playSfx(kSfxBoom, 127, 0);
		_screenFlash = 5;
		debugC(DEBUG_INSANE, "on-foot player hit: frame=%u latch=%u flags=0x%02x health=%d->%d",
			(unsigned)(uint16)_gameCounter, (unsigned)_gameLatch5D, _damageFlags, oldHealth, _health);
	}
}

// Port split matching HandleGameOp1A_OnFootVariant. The helper name is new to
// this implementation; the original code dispatches the opcode handler directly.
void InsaneRebel1::updateOnFootAimVariant() {
	// --- 0x1A: Crosshair positioning (HandleGameOp1A_OnFootVariant) ---
	// DOS used virtual-mouse axes relative to the character offset. The
	// mouse and gamepad reticle are screen-space controls so the cursor remains
	// able to cross the whole playfield while Luke is standing at either side.
	int16 inputX = 0, inputY = 0;
	preprocessOnFootAim(inputX, inputY);
	_shipPosX = CLIP<int16>((int16)(inputX + kOnFootCenterX), kOnFootCursorMinX, kOnFootCursorMaxX);
	_shipPosY = CLIP<int16>((int16)(inputY + kOnFootCursorBaseY), kOnFootCursorMinY, kOnFootCursorMaxY);
}

void InsaneRebel1::finishOnFootFrame() {
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
	if (_screenFlash > 0) {
		_screenFlash--;
		_screenShakeEnabled = (_screenFlash > 0);
	}
	updateScreenFlashPalette();

	_damageFlags = 0;
	_frameCounter++;
}

void InsaneRebel1::procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
	int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
	int16 par1, int16 par2, int16 par3, int16 par4) {
}

void InsaneRebel1::procSKIP(int32 subSize, Common::SeekableReadStream &b) {
}

void InsaneRebel1::handleGameOpcode5EReset(uint32 param1) {
	// RA1 dispatcher inline reset/init path (FUN_1BE1B case 0x5E).
	// This is not a pure control-mode assignment.
	if (_frameDispatchFlags & 0x40) {
		debugC(DEBUG_INSANE, "GAME 0x5E: reset suppressed by dispatch flags=0x%02x",
			_frameDispatchFlags);
		return;
	}

	const int16 walkerReplayKillCount = (_walkerRoundReplay && _currentLevel == 7) ? _killCount : 0;

	_damageFlags = 0;
	_prevDamageFlags = 0;
	_damageCooldown = 0;
	_deathTimer = 0;
	_screenFlash = 0;
	_screenShakeEnabled = false;
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
	_inputAxisDeltaX = 0;
	_avgInputX = 0;
	_avgInputY = 0;
	_level2JoystickFilteredX = 0;
	_level2JoystickFilteredY = 0;
	resetGamepadReticleAim();

	_playerFired = false;
	_playerSecondaryHeld = false;
	_fireCooldown = 0;
	_rapidFirePhase = 0;
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
	if (_walkerRoundReplay && _currentLevel == 7)
		_killCount = walkerReplayKillCount;

	// Field1 == 0 corresponds to baseline recenter behavior in the original.
	if ((int32)param1 == 0) {
		_perspectiveX = 0x20;
		_perspectiveY = 0x17;
		_shipPosX = kRA1CenterX;
		_shipPosY = kRA1CenterY;
	}

	_activeGameOpcode = 0;
	_frameGameOpcodeMask = 0;
	_frameDispatchFlags = 0;
	resetProjectionTable();
	if ((int32)param1 == 0)
		syncViewportOffset(true);

	// Original RunLevel8Flow initializes its separate g_level8HitboxBuffer
	// after the first L8PLAY runtime reset. We fold that mask into the
	// secondary half of _frameObjectState; route resumes preserve it.
	if (_currentLevel == 7)
		memset(_frameObjectState + 150, 0xFF, 150);

	debugC(DEBUG_INSANE, "GAME 0x5E: reset state field1=%d mode=%d", (int32)param1, (int)_flyControlMode);
}

void InsaneRebel1::handleGameOpcode5DLinkLatch(uint32 param1) {
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

	debugC(DEBUG_INSANE, "GAME 0x5D (link/event latch) param=%u", _gameLatch5D);
}

void InsaneRebel1::handleGameOpcode5FRandomHitLatch(uint32 param1) {
	if (param1 > 0) {
		const int bitIndex = (int)param1 - 1;
		const int byteIndex = bitIndex >> 3;
		if (byteIndex >= 0 && byteIndex < 0x96 &&
			(_frameObjectState[byteIndex] & (byte)(0x80 >> (bitIndex & 7))) == 0) {
			_gameLatch5F = (uint16)param1;
		}
	}

	debugC(DEBUG_INSANE, "GAME 0x5F (random-hit latch) param=%u", _gameLatch5F);
}

void InsaneRebel1::handleGameOpcode07ShipFlight(int32 subSize, Common::SeekableReadStream &b, uint32 param1) {
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
		debugC(DEBUG_INSANE, "GAME 0x07: counter=%d driftParam=%d", _gameCounter, _driftParam);
	}
}

void InsaneRebel1::handleGameOpcode0DCorridor(int32 subSize, Common::SeekableReadStream &b, uint32 param1) {
	// Corridor boundaries: per-frame flight corridor
	// Original params: left, top, WIDTH, HEIGHT (not right/bottom!)
	// FUN_1C54D computes center = (left+width/2, top+height/2), transforms, then checks edges.
	if (subSize < 20)
		return;

	int16 corridorLeft = (int16)param1;
	int16 corridorTop = (int16)b.readUint32BE();
	int16 corridorWidth = (int16)b.readUint32BE();
	int16 corridorHeight = (int16)b.readUint32BE();

	int16 centerX = corridorLeft + corridorWidth / 2;
	int16 centerY = corridorTop + corridorHeight / 2;
	// DOS FUN_1C54D calls FUN_223FE here, which projects the scripted
	// rectangle center into gameplay-window space before testing it against the
	// ship center.
	projectGameplayPoint(centerX, centerY);

	_corridorLeftX = centerX - corridorWidth / 2;
	_corridorTopY = centerY - corridorHeight / 2;
	_corridorRightX = _corridorLeftX + corridorWidth;
	_corridorBottomY = _corridorTopY + corridorHeight;

	// Apply 0x0D immediately so it sees the same pre-physics ship state as
	// the other per-frame GAME latches. Deferring this until after 0x07/0x09
	// movement made right-wall hits fire too early when steering into a wall.
	int16 collisionShipX = _shipPosX;
	int16 collisionShipY = _shipPosY;
	getCollisionShipCenter(collisionShipX, collisionShipY);

	const bool suppressDirectionalDamage = (_frameDispatchFlags & 0x10) != 0;
	const byte oldDirectionalFlags = _damageFlags & 0x0F;
	if (_health >= 0) {
		if (collisionShipX < _corridorLeftX) {
			_posAccumX = (int32)(_corridorLeftX - kRA1CenterX) * 0x100;
			if (!suppressDirectionalDamage) {
				if (_rollAccum < 0x100)
					_rollAccum = 0x100;
				_damageFlags |= 0x04;
			}
		}
		if (collisionShipX > _corridorRightX) {
			_posAccumX = (int32)(_corridorRightX - kRA1CenterX) * 0x100;
			if (!suppressDirectionalDamage) {
				if (_rollAccum > -0x100)
					_rollAccum = -0x100;
				_damageFlags |= 0x02;
			}
		}
		if (collisionShipY < _corridorTopY) {
			_posAccumY = (int32)(_corridorTopY - kRA1CenterY) * 0x100 + 0x100;
			if (!suppressDirectionalDamage)
				_damageFlags |= 0x01;
		}
		if (collisionShipY > _corridorBottomY) {
			_posAccumY = (int32)(_corridorBottomY - kRA1CenterY) * 0x100 - 0x100;
			if (!suppressDirectionalDamage)
				_damageFlags |= 0x08;
		}
	}
	if ((_damageFlags & 0x0F) != oldDirectionalFlags) {
		debugC(DEBUG_INSANE, "0x0D hit: ship=(%d,%d) corridor=[%d,%d]-[%d,%d] flags=0x%02x zoneSuppressed=%d",
			collisionShipX, collisionShipY,
			_corridorLeftX, _corridorTopY, _corridorRightX, _corridorBottomY,
			_damageFlags, suppressDirectionalDamage ? 1 : 0);
	}

	debugC(DEBUG_INSANE, "GAME 0x0D: raw=[%d,%d]+(%d,%d) cam=(%d,%d) transformed=[%d,%d]-[%d,%d]",
		corridorLeft, corridorTop, corridorWidth, corridorHeight,
		_perspectiveX, _perspectiveY,
		_corridorLeftX, _corridorTopY, _corridorRightX, _corridorBottomY);
}

void InsaneRebel1::handleGameOpcode0EZone(int32 subSize, Common::SeekableReadStream &b, uint32 param1) {
	// Secondary collision zone (FUN_1C6E9): AABB test, sets damageFlags bit 4 (0x10)
	// Original params: left, top, WIDTH, HEIGHT (same as 0x0D)
	if (subSize < 20)
		return;

	int16 zoneLeft = (int16)param1;
	int16 zoneTop = (int16)b.readUint32BE();
	int16 zoneWidth = (int16)b.readUint32BE();
	int16 zoneHeight = (int16)b.readUint32BE();
	const int16 rawZoneLeft = zoneLeft;
	const int16 rawZoneTop = zoneTop;

	int16 centerX = zoneLeft + zoneWidth / 2;
	int16 centerY = zoneTop + zoneHeight / 2;
	// Same gameplay-window FUN_223FE transform as opcode 0x0D/FUN_1C54D.
	projectGameplayPoint(centerX, centerY);

	zoneLeft = centerX - zoneWidth / 2;
	zoneTop = centerY - zoneHeight / 2;
	int16 zoneRight = zoneLeft + zoneWidth;
	int16 zoneBottom = zoneTop + zoneHeight;
	int16 collisionShipX = _shipPosX;
	int16 collisionShipY = _shipPosY;
	getCollisionShipCenter(collisionShipX, collisionShipY);

	if (_health >= 0 &&
		collisionShipX > zoneLeft && collisionShipX < zoneRight &&
		collisionShipY > zoneTop && collisionShipY < zoneBottom) {
		_damageFlags |= 0x10;
		debugC(DEBUG_INSANE, "0x0E hit: ship=(%d,%d) zone=[%d,%d]-[%d,%d] raw=[%d,%d]+(%d,%d) cam=(%d,%d) flags=0x%02x",
			collisionShipX, collisionShipY, zoneLeft, zoneTop, zoneRight, zoneBottom,
			rawZoneLeft, rawZoneTop, zoneWidth, zoneHeight,
			_perspectiveX, _perspectiveY, _damageFlags);
	}
	debugC(DEBUG_INSANE, "GAME 0x0E: ship=(%d,%d) zone=[%d,%d]-[%d,%d] cam=(%d,%d) flags=0x%02x",
		collisionShipX, collisionShipY, zoneLeft, zoneTop, zoneRight, zoneBottom,
		_perspectiveX, _perspectiveY, _damageFlags);
}

void InsaneRebel1::handleGameOpcode0BFirstPerson(int32 subSize, Common::SeekableReadStream &b, uint32 param1) {
	_activeGameOpcode = 0x0B;
	_frameGameOpcodeMask |= (1u << 0x0B);
	// GAME 0x0B per-frame handler (FUN_1CDA7).
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
			debugC(DEBUG_INSANE, "finishing 0x0B interactive video at counter=%d/%u", _gameCounter, maxFrames);
		}
	}
	debugC(DEBUG_INSANE, "GAME 0x0B: counter=%d", _gameCounter);
	if (!_gameOp0BPhysicsUpdatedThisFrame) {
		updateGameOp0BPhysics();
		_gameOp0BPhysicsUpdatedThisFrame = true;
		syncViewportOffset(true);
	}
}

void InsaneRebel1::handleGameOpcode5ATarget(int32 subSize, Common::SeekableReadStream &b, uint32 param1) {
	// Target detection — HandleGameOp5A (0x1C0EF). AABB from video stream.
	// Original checks event mask: if target already killed, skip to GOST update.
	if (subSize < 24)
		return;

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
	debugC(DEBUG_INSANE, "GAME 0x5A: target=%d rect=[%d,%d]-[%d,%d] prox=%d",
		targetIdx, left, top, right, bottom, _targetProximity);
}

void InsaneRebel1::handleGameCounterOpcode(uint32 opcode, int32 subSize, Common::SeekableReadStream &b, uint32 param1) {
	_activeGameOpcode = (uint16)opcode;
	_frameGameOpcodeMask |= (1u << opcode);
	_gameCounter = param1;
	if (subSize >= 20) {
		uint32 param2 = b.readUint32BE();
		uint32 param3 = b.readUint32BE();
		uint32 param4 = b.readUint32BE();
		if (opcode == 0x09) {
			debugC(DEBUG_INSANE, "GAME 0x09: counter=%d params=(%d,%d,%d) opcodeMask=0x%08x",
				_gameCounter, param2, param3, param4, _frameGameOpcodeMask);
		} else {
			debugC(DEBUG_INSANE, "GAME 0x%02x: counter=%d params=(%d,%d,%d)",
				opcode, _gameCounter, param2, param3, param4);
		}
	}
}

// handleGameChunk — FUN_1BE1B (0x1BE1B). Central GAME opcode dispatcher.
// Reads 7x32-bit BE integers from GAME chunk, routes to per-opcode handlers.
void InsaneRebel1::handleGameChunk(int32 subSize, Common::SeekableReadStream &b,
		byte *renderBitmap, int width, int height) {
	if (subSize < 8)
		return;

	uint32 opcode = b.readUint32BE();
	uint32 param1 = b.readUint32BE();
	_frameHasGameChunk = true;

	// FUN_1BE1B applies two global gates before the opcode switch. Bit 0 of
	// g_combatModeFlags skips gameplay dispatch entirely; bit 5 of g_hudDisableFlags
	// suppresses the handlers while still requesting HUD refresh for a few opcodes.
	if (_gameplayFlags75ff & 1) {
		debugC(DEBUG_INSANE, "GAME 0x%02x: skipped by combat mode flags=0x%02x",
			opcode, _gameplayFlags75ff);
		return;
	}
	if (_gameplayFlags75fe & 0x20) {
		if (ra1DispatcherHudOnlyWhenDisabled(opcode))
			_hudRenderFlag = 0xFF;
		debugC(DEBUG_INSANE, "GAME 0x%02x: skipped by HUD disable flags=0x%02x",
			opcode, _gameplayFlags75fe);
		return;
	}

	switch (opcode) {
	case 0x5E:
		handleGameOpcode5EReset(param1);
		break;

	case 0x5D:
		handleGameOpcode5DLinkLatch(param1);
		break;

	case 0x5F:
		handleGameOpcode5FRandomHitLatch(param1);
		break;

	case 0x07:
		handleGameOpcode07ShipFlight(subSize, b, param1);
		break;

	case 0x0D:
		handleGameOpcode0DCorridor(subSize, b, param1);
		break;

	case 0x0E:
		handleGameOpcode0EZone(subSize, b, param1);
		break;

	case 0x0B:
		handleGameOpcode0BFirstPerson(subSize, b, param1);
		renderGameOp0BOverlayDuringChunk(renderBitmap, width, width, height);
		break;

	case 0x5A:
		handleGameOpcode5ATarget(subSize, b, param1);
		break;

	case 0x08:
	case 0x09:
	case 0x0A:
	case 0x19:
	case 0x1A:
		handleGameCounterOpcode(opcode, subSize, b, param1);
		break;

	default:
		debugC(DEBUG_INSANE, "GAME unknown 0x%02x size=%d", opcode, subSize);
		break;
	}
}

// processShot — FUN_1CCA0 (0x1CCA0). Spawns shot into explosion slot when fired.
// Called once per frame during interactive rendering.
void InsaneRebel1::processShot() {
	if (_optRapidFire) {
		// 3DO FUN_0000c3a4 advances this before testing fire state; a fresh press
		// fires immediately, while held repeats are gated to phase 0.
		_rapidFirePhase++;
		if (_rapidFirePhase > 2)
			_rapidFirePhase = 0;
	}

	if (!_playerFired)
		return;

	if (_fireCooldown != 0) {
		if (!_optRapidFire || _rapidFirePhase != 0)
			return;
	}

	// On-foot mode: only spawn when in aiming stance (dirIndex 11-19) or flags force it.
	// Original: if (((10 < g_shipDirIndex) && (g_shipDirIndex < 0x14)) || ((DAT_000075fe & 8) != 0))
	const uint16 effectiveOpcode = getEffectiveGameOpcode();
	const bool onFootMode = (effectiveOpcode == 0x19 || effectiveOpcode == 0x1A);
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
	// Turret: ship center
	// Flight: cursor position
	const bool turretMode = (effectiveOpcode == 0x08 || effectiveOpcode == 0x0A);
	int16 originX, originY;
	if (onFootMode) {
		originX = _onFootCharX + kOnFootCenterX;
		originY = _onFootCharY + kOnFootCenterY;
	} else if (turretMode) {
		getTurretShipCenter(originX, originY);
	} else {
		originX = _shipPosX;
		originY = _shipPosY;
	}

	const int16 cursorX = getGameplayCursorX();
	const int16 cursorY = getGameplayCursorY();
	const bool torpedoMode = isTorpedoModeActive();
	_shotSlots[slot].timer = torpedoMode ? 2 : 5;
	_shotSlots[slot].posX = cursorX;
	_shotSlots[slot].posY = cursorY;
	_shotSlots[slot].centerX = originX;
	_shotSlots[slot].centerY = originY;
	_shotSlots[slot].variant = _shotAlternator;
	_shotAlternator = 1 - _shotAlternator;
	playSfx(torpedoMode ? kSfxAlert : kSfxLaserShot, 127, 0);

	if (effectiveOpcode == 0x09 || _currentLevel == 4) {
		debugC(DEBUG_INSANE, "shot: opcode=0x%02x frame=%d slot=%d cursor=(%d,%d) origin=(%d,%d) dir=%d mode=%d",
			effectiveOpcode, _gameCounter, slot, cursorX, cursorY, originX, originY,
			_shipDirIndex, _flyControlMode);
	} else {
		debugC(DEBUG_INSANE, "shot: slot=%d pos=(%d,%d) origin=(%d,%d)", slot,
			cursorX, cursorY, originX, originY);
	}
}

// checkTargetHit — FUN_1C0EF (0x1C0EF). AABB target detection with snap tolerance.
// The original compares raw FOBJ bounds against the cursor after
// UnprojectScreenPoint(). Keep that separate from 0x0D/0x0E collision, which projects
// zones into gameplay-window screen space before comparing against the ship center.
void InsaneRebel1::checkTargetHit(int16 targetIdx, int16 left, int16 top, int16 right, int16 bottom) {
	int16 snap = _tuning.snap;
	const uint16 effectiveOpcode = getTargetHitGameOpcode();
	const bool screenSpaceCursor = ra1TargetCursorUsesProjection(effectiveOpcode);
	const int16 screenCursorX = getGameplayCursorX(effectiveOpcode);
	const int16 screenCursorY = getGameplayCursorY(effectiveOpcode);
	int16 curX = screenCursorX;
	int16 curY = screenCursorY;
	if (screenSpaceCursor)
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

		debugC(DEBUG_INSANE, "target near: opcode=0x%02x target=%d raw=[%d,%d]-[%d,%d] cursorScreen=(%d,%d) cursorTest=(%d,%d) snap=%d prox=%d view=(%d,%d)",
			effectiveOpcode, targetIdx, left, top, right, bottom,
			screenCursorX, screenCursorY, curX, curY, snap, _targetProximity,
			_perspectiveX, _perspectiveY);

		// Check tight lock: cursor within target + snap (no extra margin)
		if (curX > left - snap && curX < right + snap &&
			curY > top - snap && curY < bottom + snap) {
			_targetProximity = 2;  // On-target
			if (snap > 0) {
				int16 snappedX = (left + right) / 2;
				int16 snappedY = (top + bottom) / 2;
				if (screenSpaceCursor)
					projectGameplayPoint(snappedX, snappedY);
				setGameplayCursor(effectiveOpcode, snappedX, snappedY);
			}

			// DOS uses g_recentKillObjectIdPlus1 as a frame-wide latch. Once one
			// target is hit this frame, overlapping FOBJ layers must not consume the
			// same shot again.
			if (_lastHitTarget == 0) {
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

						if (debugChannelSet(-1, DEBUG_INSANE)) {
							Common::String damagedState;
							Common::String hiddenState;
							for (int objectId = 20; objectId <= 43; objectId++) {
								const int bitIndex = objectId - 1;
								const int byteIdx = bitIndex >> 3;
								if (byteIdx < 0 || byteIdx >= 0x96 || byteIdx >= kFrameObjectStateBytes)
									continue;

								const byte stateBit = (byte)(0x80 >> (bitIndex & 7));
								const int altIdx = byteIdx + 0x96;
								const bool primarySet = (_frameObjectState[byteIdx] & stateBit) != 0;
								const bool secondarySet = (altIdx < kFrameObjectStateBytes) &&
									((_frameObjectState[altIdx] & stateBit) != 0);

								if (primarySet)
									hiddenState += Common::String::format("%d,", objectId);
								else if (!secondarySet)
									damagedState += Common::String::format("%d,", objectId);
							}

							debugC(DEBUG_INSANE, "L8 armor: hitObject=%d damaged=[%s] hidden=[%s]",
								targetIdx + 1, damagedState.c_str(), hiddenState.c_str());
						}

						int16 hitCenterX = (left + right) / 2;
						int16 hitCenterY = (top + bottom) / 2;
						projectGameplayPoint(hitCenterX, hitCenterY);
						const int sfxPan = CLIP((hitCenterX - kRA1CenterX) * 127 / kRA1CenterX, -127, 127);
						if ((_gameplayFlags75fe & 0x10) == 0)
							playSfx(kSfxExplode, 127, sfxPan);

						debugC(DEBUG_INSANE, "HIT: target=%d gost=%d pos=(%d,%d) score=%d kills=%d bangSprites=%d",
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
