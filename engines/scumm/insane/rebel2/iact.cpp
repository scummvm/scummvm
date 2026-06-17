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

#include "common/config-manager.h"
#include "common/system.h"
#include "common/memstream.h"
#include "common/util.h"

#include "scumm/scumm_v7.h"

#include "scumm/smush/smush_player.h"
#include "scumm/smush/rebel/codec_ra2.h"
#include "scumm/smush/rebel/font_rebel2.h"

#include "scumm/insane/rebel2/rebel.h"

namespace Scumm {

const int kRA2Handler7DirectInputNumerator = 4;
const int kRA2Handler7DirectInputDenominator = 5;
const int kRA2Handler7MouseTargetRangeX = 0xc0;

static bool readLevel2BackgroundChunkHeader(Common::SeekableReadStream &stream, int64 containerEnd, const char *context,
		uint32 &tag, uint32 &chunkSize, int64 &dataEnd, int64 &nextChunkPos) {
	const int64 headerPos = stream.pos();
	if (headerPos < 0 || headerPos + 8 > containerEnd)
		return false;

	tag = stream.readUint32BE();
	chunkSize = stream.readUint32BE();

	const int64 dataStart = stream.pos();
	if ((int64)chunkSize > containerEnd - dataStart) {
		debugC(DEBUG_INSANE, "loadLevel2Background: Truncated %s chunk 0x%08X at %lld: size=%u, remaining=%lld",
			context, tag, headerPos, chunkSize, containerEnd - dataStart);
		return false;
	}

	dataEnd = dataStart + chunkSize;
	nextChunkPos = dataEnd + (chunkSize & 1);
	if (nextChunkPos > containerEnd)
		nextChunkPos = dataEnd;

	return true;
}

//
// procPreRendering -- Pre-frame setup: background restore and corridor overlays.
//
// Restores Level 2 background before FOBJ decoding (Handler 8) and handles
// Handler 25 corridor overlay positioning.
//
void InsaneRebel2::procPreRendering(byte *renderBitmap) {
	Insane::procPreRendering(renderBitmap);

	// Pan the reticle from held directional controls (on-screen/physical gamepad dpad,
	// keyboard arrows) once per frame. No-op outside gameplay; mouse aiming is unaffected.
	updateGameplayAimFromGamepad();

	// Reset opcode 6 init flag at the start of each new video.
	// This ensures the per-wave init (clearBit, link table reset, wave state)
	// fires exactly once per wave video, not every frame.
	//
	// Exception: seamless continuation segments (flag 0x40, e.g. the looping 06PLAY1B attack
	// run) keep the init flag set, otherwise the wave init's clearBit(0) would resurrect
	// shield targets the player already destroyed, resetting the shield on every loop.
	if (_player && _player->_frame == 0) {
		const bool shieldContinuation = _rebelShieldGateActive && (_player->_curVideoFlags & 0x40) != 0;
		if (!shieldContinuation)
			_rebelOp6Initialized = false;
	}

	// For Level 2 handler 8 gameplay, restore the background BEFORE FOBJ decoding.
	// The tiny FOBJ sprites (7x10, 9x38 pixels) only draw new sprite positions but don't
	// clear old ones. By restoring the full background each frame, we ensure old sprite
	// positions are erased before new ones are drawn.
	//
	// This is called at the start of handleFrame(), before any FOBJ chunks are processed.
	//
	// IMPORTANT: Only restore when the render buffer pitch matches the background pitch (320).
	// Levels like Level 12 (Sewers) use oversized buffers (424x260) where FOBJ/FETCH handles
	// background restoration. Copying the 320-wide background into a 640-wide buffer with
	// hardcoded pitch=320 would corrupt the corridor rendering.
	if (_rebelHandler == 8 && _level2BackgroundLoaded && _level2Background && renderBitmap) {
		int bufferPitch = (_player && _player->_width > 0) ? _player->_width : 320;
		if (bufferPitch == 320) {
			for (int y = 0; y < 200; y++) {
				memcpy(renderBitmap + y * 320, _level2Background + y * 320, 320);
			}
		}
	}

	// For Handler 25 (Level 2 speeder bike), draw the corridor overlay BEFORE FOBJ decoding.
	// The corridor overlay (par3=4 -> _rebelEmbeddedHud[4]) is DAT_00482268, a 350x230 buffer.
	// From FUN_0041cadb line 216: FUN_00428a10(param_1,0,DAT_0045790c,DAT_0045790e,DAT_00482268)
	// It's drawn at (DAT_0045790c, DAT_0045790e) which are _rebelViewOffsetX/Y.
	//
	// For Mode 1: DAT_0045790c = damageLevel * -5 - 14, range -39 (covered) to -14 (uncovered)
	//
	// From FUN_00428a10: When position is negative, we skip source pixels and draw at 0.
	// Handler 25: Corridor overlay and FOBJ position offsets are set during
	// IACT opcode 6 processing (iactRebel2Opcode6), matching the original
	// FUN_41CADB architecture. No corridor drawing needed here.

	// Chapter selection: Set FOBJ offset for O_LEVEL.SAN preview strip.
	// The 80x800 FOBJ strip is at left=320. With offset X=-90, it renders at
	// X=230 (inside the preview box). The Y offset scrolls the strip vertically
	// so only the selected chapter's 50px slice appears at Y=75.
	// STOR captures raw FOBJ data regardless of screen rendering, so the
	// offset on frame 0 doesn't affect the STOR/FTCH mechanism.
	if (_gameState == kStateChapterSelect && _player) {
		if (renderBitmap) {
			const int clearWidth = (_player->_width > 0) ? _player->_width : _vm->_screenWidth;
			const int clearHeight = (_player->_height > 0) ? _player->_height : _vm->_screenHeight;
			if (clearWidth > 0 && clearHeight > 0)
				memset(renderBitmap, 0, (size_t)clearWidth * clearHeight);
		}
		_player->_fobjOffsetX = _previewOffsetX;
		_player->_fobjOffsetY = _previewOffsetY;
	}
}

// procIACT -- Main IACT chunk dispatcher (overrides Insane::procIACT).
void InsaneRebel2::procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					  int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
					  int16 par1, int16 par2, int16 par3, int16 par4) {
	debugC(DEBUG_INSANE, "IACT: opcode=%d par2=%d par3=%d par4=%d gameState=%d sceneId=%d",
		par1, par2, par3, par4, _gameState, _currSceneId);

	if (_keyboardDisable)
		return;

	// Handle menu IACT - menu videos have embedded ANIM data in IACT chunks
	// Menu IACTs have par1=8 (code), par2=46 (flags), par4>=1000 (userId)
	// The embedded ANIM contains the full menu frame
	if (_gameState == kStateMainMenu && par1 == 8 && par4 >= 1000) {
		debugC(DEBUG_INSANE, "IACT: Menu mode - processing embedded ANIM (userId=%d)", par4);

		// Scan for embedded ANIM tag in the IACT data
		int64 startPos = b.pos();
		int64 totalSize = b.size();
		debugC(DEBUG_INSANE, "IACT: stream pos=%d, size=%d, remaining=%d",
			(int)startPos, (int)totalSize, (int)(totalSize - startPos));

		if (totalSize > startPos) {
			int64 remaining = totalSize - startPos;
			int scanSize = (int)MIN<int64>(remaining, 65536);
			byte *scanBuf = (byte *)malloc(scanSize);
			if (scanBuf) {
				int bytesRead = b.read(scanBuf, scanSize);
				debugC(DEBUG_INSANE, "IACT: Read %d bytes, first 16: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
					bytesRead, scanBuf[0], scanBuf[1], scanBuf[2], scanBuf[3],
					scanBuf[4], scanBuf[5], scanBuf[6], scanBuf[7],
					scanBuf[8], scanBuf[9], scanBuf[10], scanBuf[11],
					scanBuf[12], scanBuf[13], scanBuf[14], scanBuf[15]);

				// Look for ANIM tag (embedded SAN containing menu frame)
				for (int i = 0; i + 8 <= bytesRead; ++i) {
					if (READ_BE_UINT32(scanBuf + i) == MKTAG('A','N','I','M')) {
						int64 animStreamPos = startPos + i;
						uint32 animReportedSize = READ_BE_UINT32(scanBuf + i + 4);
						int32 toCopy = (int)MIN<int64>((int64)animReportedSize + 8, totalSize - animStreamPos);
						debugC(DEBUG_INSANE, "IACT: Found embedded ANIM at offset %d, size %d", (int)i, (int)animReportedSize);
						if (toCopy > 0) {
							byte *animData = (byte *)malloc(toCopy);
							if (animData) {
								b.seek(animStreamPos);
								b.read(animData, toCopy);
								// Use userId as the HUD slot (1000 -> slot 0 for menu background)
								loadEmbeddedSan(0, animData, toCopy, renderBitmap);
								free(animData);
							}
						}
						b.seek(startPos);
						free(scanBuf);
						return;
					}
				}

				debugC(DEBUG_INSANE, "IACT: No ANIM tag found in menu IACT data");
				b.seek(startPos);
				free(scanBuf);
			}
		}
		return;
	}

	if (_currSceneId == 1)
		iactRebel2Scene1(renderBitmap, codecparam, setupsan12, setupsan13, b, size, flags, par1, par2, par3, par4);
}

// iactRebel2Scene1 -- Scene 1 IACT dispatcher (FUN_4028C5 / FUN_4033CF).
void InsaneRebel2::iactRebel2Scene1(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				  int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
				  int16 par1, int16 par2, int16 par3, int16 par4) {
	// par1 is the Opcode (word at offset +0)
	// par2 is word at offset +2
	// par3 is word at offset +4
	// par4 is word at offset +6
	//
	// Based on disassembly of FUN_4028C5 and FUN_4033CF:
	//
	// For IACT opcode 4 (enemy position update), the structure is:
	//   Offset +0x06: Type/SubType (par3)
	//   Offset +0x08: Enemy ID
	//   Offset +0x0A: X position
	//   Offset +0x0C: Y position
	//   Offset +0x0E: Width
	//   Offset +0x10: Height
	//
	// The original game calculates bounding box center:
	//   centerX = X + (Width / 2)
	//   centerY = Y + (Height / 2)
	// Then subtracts scroll offsets:
	//   screenX = centerX - DAT_0043e006 (scrollX)
	//   screenY = centerY - DAT_0043e008 (scrollY)

	//   screenX = centerX - DAT_0043e006 (scrollX)
	//   screenY = centerY - DAT_0043e008 (scrollY)

	if (par1 == 4) {
		enemyUpdate(renderBitmap, b, par2, par3, par4);
	} else if (par1 == 2) {
		// Delegate handling to dedicated opcode 2 handler
		iactRebel2Opcode2(b, par2, par3, par4);
	} else if (par1 == 3) {
		iactRebel2Opcode3(b, par2, par3, par4);
	}
	else if (par1 == 5) {
		// Opcode 5: Collision Zone Registration (FUN_004033cf case 5)
		// Sub-opcode 0x0D (13) = Primary collision zones (obstacles)
		// Sub-opcode 0x0E (14) = Secondary collision zones (boundaries)
		// par2 is the sub-opcode that determines which zone table to use
		debugC(DEBUG_INSANE, "IACT Opcode 5: par2=%d par3=%d par4=%d", par2, par3, par4);

		if (par2 == 0x0D || par2 == 0x0E) {
			// Register the collision zone from the remaining IACT data
			// par4 (userId from IACT header) is the filter value used by FUN_4092D9
			// for the < 1000 test (offset +6 in the original stored pointer)
			registerCollisionZone(b, par2, par4);
		}

	} else if (par1 == 7) {
		// Opcode 7: Handler 7 corridor/velocity control (FUN_40C3CC case 5)
		// IACT header: par1=7, par2=flags, par3=0, par4=sub-opcode
		// Body contains 2 int16 values (body[0], body[1])
		//
		// par4 sub-opcodes (from FUN_40C3CC case 5 switch on param_5[3]):
		//   0: Set velocity params (DAT_00443b12, DAT_00443b14)
		//   1: Set left X + top Y corridor boundaries (DAT_00443b0a, DAT_00443b0c)
		//   2: Set right X + bottom Y corridor boundaries (DAT_00443b0e, DAT_00443b10)
		//   5: Set flag (DAT_00443b52)

		int16 body0 = 0, body1 = 0;
		if (b.size() - b.pos() >= 4) {
			body0 = b.readSint16LE();
			body1 = b.readSint16LE();
		}

		switch (par4) {
		case 0:
			// Velocity/wind data — affects ship drift in FUN_40C3CC physics
			// DAT_00443b12 = horizontal wind, DAT_00443b14 = vertical wind
			_windParamX = body0;
			_windParamY = body1;
			debugC(DEBUG_INSANE, "Opcode 7 par4=0: wind=(%d,%d)", body0, body1);
			break;
		case 1:
			// Set LEFT X boundary and TOP Y boundary
			_corridorLeftX = body0;
			_corridorTopY = body1;
			// Mode-dependent margin adjustment (FUN_40C3CC lines 341-351)
			if (_flyControlMode == 2) {
				_corridorLeftX += 15;
			} else if (_flyControlMode == 0) {
				_corridorLeftX += 20;
			}
			debugC(DEBUG_INSANE, "Opcode 7 par4=1: corridor left=%d top=%d (adjusted left=%d)",
				body0, body1, _corridorLeftX);
			break;
		case 2:
			// Set RIGHT X boundary and BOTTOM Y boundary
			_corridorRightX = body0;
			_corridorBottomY = body1;
			// Mode-dependent margin adjustment (FUN_40C3CC lines 356-365)
			if (_flyControlMode == 2) {
				_corridorRightX -= 15;
			} else if (_flyControlMode == 0) {
				_corridorRightX -= 20;
			}
			debugC(DEBUG_INSANE, "Opcode 7 par4=2: corridor right=%d bottom=%d (adjusted right=%d)",
				body0, body1, _corridorRightX);
			break;
		case 5:
			// DAT_00443b52: repeats FLY002 ship overlay in FUN_40D836.
			_flyOverlayRepeatCount = body0;
			debugC(DEBUG_INSANE, "Opcode 7 par4=5: flyOverlayRepeat=%d", _flyOverlayRepeatCount);
			break;
		default:
			debugC(DEBUG_INSANE, "Opcode 7 par4=%d: body=(%d,%d) — unknown sub-opcode", par4, body0, body1);
			break;
		}

	} else if (par1 == 6) {
		// Opcode 6: Level setup / mode switch (FUN_41CADB case 4)
		iactRebel2Opcode6(renderBitmap, b, size, par2, par3, par4);
	} else if (par1 == 8) {
		// Opcode 8: HUD resource loading (FUN_41CADB case 6)
		iactRebel2Opcode8(renderBitmap, b, size, par2, par3, par4);
	} else if (par1 == 9) {
		// Opcode 9: Text/subtitle display
		iactRebel2Opcode9(renderBitmap, b, par2, par3, par4);
	} else if (par1 == 0 || par1 == 1) {
		// Low Opcodes seen in logs
		debugC(DEBUG_INSANE, "IACT: Low Opcode %d (par2=%d par3=%d par4=%d)", par1, par2, par3, par4);
	} else {
		debugC(DEBUG_INSANE, "IACT: Unknown Opcode %d (par2=%d par3=%d par4=%d)", par1, par2, par3, par4);
	}
}

// iactRebel2Opcode2 -- Link table and state setup (FUN_00407fcb).
void InsaneRebel2::iactRebel2Opcode2(Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4) {
	// Handle IACT opcode 2 subcases based on par3 (type). Mirrors FUN_00407fcb behavior where relevant.
	// Keep existing linking behavior (par3 == 4) for compatibility.

	// Link case: par3 == 4
	if (par3 == 4) {
		int16 childId = b.readSint16LE(); // Offset +8
		int16 parentId = b.readSint16LE(); // Offset +10

		// Validate BOTH parentId AND childId to avoid triggering "set/clear ALL bits" behavior
		// when childId <= 0. The original game's setBit(0)/clearBit(0) affects ALL bits,
		// which would disable/enable all enemies at once - not the intended linking behavior.
		if (parentId >= 1 && parentId < 512 && childId >= 1 && childId < 512) {
			// Shift links (original: 4 link slots at DAT_0045797c/817c/897c/917c)
			_rebelLinks[parentId][2] = _rebelLinks[parentId][1];
			_rebelLinks[parentId][1] = _rebelLinks[parentId][0];
			_rebelLinks[parentId][0] = childId;

			// Mirror parent's bit state to child (INVERTED):
			// - Parent alive (bit clear) → setBit(child) → child hidden
			// - Parent dead (bit set) → clearBit(child) → child shown
			// From FUN_0041CADB case 0, par3==4:
			//   bVar3 = FUN_00423970(parentId);
			//   if (bVar3 == 0) setBit(childId); else clearBit(childId);
			// This ensures linked children (explosion/death sprites) are hidden
			// while the parent is alive, and revealed when the parent is destroyed.
			if (!isBitSet(parentId)) {
				setBit(childId);
				debugC(DEBUG_INSANE, "Linked ID=%d to Parent=%d (Slot 0) - child DISABLED (parent alive)", childId, parentId);
			} else {
				clearBit(childId);
				debugC(DEBUG_INSANE, "Linked ID=%d to Parent=%d (Slot 0) - child ENABLED (parent dead)", childId, parentId);
			}
		} else {
			debugC(DEBUG_INSANE, "Skipping link with invalid IDs childId=%d parentId=%d", childId, parentId);
		}
		return;
	} else if (par3 == 1) { // Probabilistic / counter cases: par3 == 1
		int16 value = par4; // sVar6
		int16 targetId = b.readSint16LE(); // Offset +8 (sVar7)

		// Validate targetId >= 1 to avoid triggering "set/clear ALL bits" behavior
		// The original game's setBit(0)/clearBit(0) affects ALL bits, not intended here
		if (targetId < 1 || targetId >= 0x200)
			return;

		// Handler 8/25: FUN_401234 case 0 / FUN_0041CADB case 0 par3==1
		// From original FUN_0041CADB:
		//   if (par4 == 100) clearBit(body0);  // Force enable
		//   else { bitMask = 1 << (par4 & 0x1f); if (waveState & bitMask) setBit(body0); }
		if ((_rebelHandler == 8 || _rebelHandler == 25) && value != 0) {
			if (value == 100) {
				// par4==100: Force enable the target (original: FUN_00423a00)
				clearBit(targetId);
				debugC(DEBUG_INSANE, "Opcode2 (H%d): Force ENABLE target=%d (par4=100)", _rebelHandler, targetId);
			} else {
				// Check wave state: if enemy type has been killed, disable target
				int bitMask = 1 << (value & 0x1f);
				if ((_rebelWaveState & bitMask) != 0) {
					setBit(targetId);
					debugC(DEBUG_INSANE, "Opcode2 (H%d): Disable target=%d (type %d killed, wave=0x%x)", _rebelHandler, targetId, value, _rebelWaveState);
				}
			}
			return;
		}

		if (value > 1 && value < 10) { // 1 < value < 10: random disable
			if (_vm->_rnd.getRandomNumber(value) == 0) {
				setBit(targetId);
				debugC(DEBUG_INSANE, "IACT Opcode2: Random DISABLE target=%d (value=%d)", targetId, value);
			}
		} else if (value > 10 && value < 20) { // 10 < value < 20: enable/disable with special value==11 = force enable
			if (value == 11) {
				clearBit(targetId);
				debugC(DEBUG_INSANE, "IACT Opcode2: FORCE ENABLE target=%d (value=11)", targetId);
			} else {
				if (_vm->_rnd.getRandomNumber(value - 10) == 0) {
					clearBit(targetId);
					debugC(DEBUG_INSANE, "IACT Opcode2: Random ENABLE target=%d (value=%d)", targetId, value);
				} else {
					setBit(targetId);
					debugC(DEBUG_INSANE, "IACT Opcode2: Random DISABLE target=%d (value=%d)", targetId, value);
				}
			}
		} else if (value > 99 && value < 110) { // 99 < value < 110: increment value counter if target active
			if (!isBitSet(targetId)) {
				int idx = value - 100;
				if (idx >= 0 && idx < 10) {
					_rebelValueCounters[idx]++;
					_rebelLastCounter = _rebelValueCounters[idx];
					// Track that this target feeds value-counter[idx] so destroying it
					// decrements the shield gauge. Only while a shield gate is active.
					if (_rebelShieldGateActive && targetId >= 0 && targetId < 512) {
						_rebelGaugeSlot[targetId] = (int8)idx;
						_rebelGaugeArmed = true;
						_rebelLastArmedSlot = idx;
					}
					debugC(DEBUG_INSANE, "IACT Opcode2: Increment VAL counter[%d] -> %d (target=%d)", value, _rebelValueCounters[idx], targetId);
				}
			}

		} else if (value > 0x3ff) { // Bitmask case: value > 0x3FF
			for (int slot = 1; slot <= 9; ++slot) {
				if ((value & (1 << (slot - 1))) != 0) {
					if (!isBitSet(targetId)) {
						_rebelMaskCounters[slot]++;
						_rebelLastCounter = _rebelMaskCounters[slot];
						if (_rebelShieldGateActive && targetId >= 0 && targetId < 512) {
							_rebelGaugeSlot[targetId] = (int8)(10 + slot);
							_rebelGaugeArmed = true;
							_rebelLastArmedSlot = 10 + slot;
						}
						debugC(DEBUG_INSANE, "IACT Opcode2: Increment MASK counter[%d] -> %d (target=%d)", slot, _rebelMaskCounters[slot], targetId);
					}
				}
			}
		}

		// Unknown sub-type: log and return
		debugC(DEBUG_INSANE, "IACT Opcode2: Unhandled par3=%d par4=%d", par3, par4);
	}
}

// iactRebel2Opcode3 -- Damage and hit counter processing (FUN_4092D9 / FUN_40E35E / FUN_401234).
void InsaneRebel2::iactRebel2Opcode3(Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4) {
	// IACT opcode 3 — damage and hit counter processing.
	// Based on FUN_4092D9 (Handler 0x26), FUN_40E35E (Handler 7), FUN_401234 (Handler 8).
	//
	// The common dispatcher (FUN_4033CF) stores opcode 3 entries in the projectile impact
	// list (DAT_0043f9e0). For handlers 0x26/7 these are processed per-frame by the
	// per-handler collision function (FUN_4092D9/FUN_40E35E). For handlers 8/25 they're
	// processed immediately during IACT dispatch.
	//
	// FUN_403ba9() loop in FUN_4092D9 (lines 209-239):
	//   par3 == 1/2: Direct hit — increment hit counter, apply damage if conditions met
	//     - body[0] (offset +8): srcId for isBitSet check
	//     - par4 != 0: damage from DAT_0047e0f4 (direct hit damage table)
	//     - par3==1: par4 must be 1..9 for damage
	//     - par3==2: par4 must be > 99, with wave state bit check for par4 >= 101
	//
	//   par3 == 5: Probabilistic damage — probability check from DAT_0047e0fc
	//     - body[1] (offset +10): srcId for isBitSet check (different from par3=1/2!)
	//     - Damage from DAT_0047e0f8 (probabilistic damage table)
	//
	// Stream position on entry: at offset +8 (body[0], first word after 8-byte header)

	// Handler 25 has a different opcode 3 structure (FUN_41CADB case 1):
	//   par3==5: probabilistic damage WITH cover check (DAT_0045790a < 2)
	//   par3==1: increment hit counter ONLY (NO damage), requires par4 != 4
	//   par4==100: direct damage (separate check after par3 branches, NO cover check)
	// Other handlers (0x26/7/8) use FUN_4092D9/FUN_40E35E/FUN_401234 with different logic.
	if (_rebelHandler == 25) {
		// Handler 25 opcode 3 — FUN_41CADB case 1
		int16 srcIdBody0 = b.readSint16LE(); // body[0] (offset +8)
		int16 srcIdBody1 = b.readSint16LE(); // body[1] (offset +10)

		if (par3 == 5) {
			// Probabilistic damage with cover check (lines 81-92)
			debugC(DEBUG_INSANE, "Opcode3: H25 par3=5 srcId=%d isBitSet=%d damageLevel=%d",
				srcIdBody1, isBitSet(srcIdBody1), _rebelDamageLevel);

			if (_rebelDamageLevel < 2 && !isBitSet(srcIdBody1)) {
				LevelDifficultyParams params = getDifficultyParams();
				int probability = (params.shotAccuracy >= 0) ? params.shotAccuracy : 0;
				int roll = _vm->_rnd.getRandomNumber(99);
				debugC(DEBUG_INSANE, "Opcode3: probability=%d roll=%d (need roll < prob)", probability, roll);

				if (roll < probability) {
					int damageAmount = (params.shotDamage >= 0) ? params.shotDamage : 0;
					if (applyPlayerDamage(damageAmount)) {
						debugC(DEBUG_INSANE, "H25 PROBABILISTIC damage from %d. Damage=%d total=%d",
							srcIdBody1, damageAmount, _playerDamage);
					}
					if (!_noDamage)
						initDamageFlash();
				}
			} else {
				debugC(DEBUG_INSANE, "Opcode3: H25 par3=5 BLOCKED (damageLevel=%d isBitSet=%d)",
					_rebelDamageLevel, isBitSet(srcIdBody1));
			}
		} else if (par3 == 1 && !isBitSet(srcIdBody0) && par4 != 4) {
			// Hit counter only — NO damage (lines 94-98)
			_rebelHitCounter++;
			debugC(DEBUG_INSANE, "H25 hit counter++ -> %d (par3=1 par4=%d, no damage)",
				_rebelHitCounter, par4);
		} else {
			debugC(DEBUG_INSANE, "Opcode3: H25 par3=%d par4=%d (no action)", par3, par4);
		}

		// Direct damage: par4==100, separate from par3 branches (lines 99-111)
		if (par4 == 100 && !isBitSet(srcIdBody0)) {
			LevelDifficultyParams dparams = getDifficultyParams();
			int directHitDamage = (dparams.missDamage >= 0) ? dparams.missDamage : 0;
			if (applyPlayerDamage(directHitDamage)) {
				debugC(DEBUG_INSANE, "H25 DIRECT HIT par4=100 damage=%d total=%d",
					directHitDamage, _playerDamage);
			}
			if (!_noDamage)
				initDamageFlash();
		}
	} else if (par3 == 1 || par3 == 2) {
		// Non-Handler-25 direct hit path — FUN_4092D9 lines 209-227
		int16 srcId = b.readSint16LE(); // body[0] (offset +8): source enemy ID

		debugC(DEBUG_INSANE, "Opcode3: par3=%d par4=%d srcId=%d isBitSet=%d",
			par3, par4, srcId, isBitSet(srcId));

		if (!isBitSet(srcId)) {
			_rebelHitCounter++;
			debugC(DEBUG_INSANE, "Incremented hit counter -> %d", _rebelHitCounter);

			LevelDifficultyParams dparams = getDifficultyParams();
			int directHitDamage = (dparams.missDamage >= 0) ? dparams.missDamage : 0;

			if (par4 != 0 && directHitDamage > 0) {
				bool shouldDamage = false;

				if (par3 == 1 && par4 < 10) {
					shouldDamage = true;
				} else if (par3 == 2 && par4 > 99) {
					if (par4 < 0x65 || (_rebelPhaseState & (1 << ((par4 + 0x9b) & 0x1f))) == 0) {
						shouldDamage = true;
					}
				}

				if (shouldDamage) {
					if (applyPlayerDamage(directHitDamage)) {
						debugC(DEBUG_INSANE, "DIRECT HIT damage from enemy %d. par3=%d par4=%d damage=%d total=%d",
							srcId, par3, par4, directHitDamage, _playerDamage);
					}
					if (!_noDamage)
						initDamageFlash();
				}
			}
		}
	} else if (par3 == 5) {
		// Non-Handler-25 probabilistic damage — FUN_4092D9 lines 228-239
		b.skip(2); // Skip body[0]
		int16 srcId = b.readSint16LE(); // body[1] (offset +10)

		debugC(DEBUG_INSANE, "Opcode3: par3=5 srcId=%d isBitSet=%d", srcId, isBitSet(srcId));

		if (!isBitSet(srcId)) {
			LevelDifficultyParams params = getDifficultyParams();
			int probability = (params.shotAccuracy >= 0) ? params.shotAccuracy : 0;

			int roll = _vm->_rnd.getRandomNumber(99);
			debugC(DEBUG_INSANE, "Opcode3: probability=%d roll=%d (need roll < prob)", probability, roll);

			if (roll < probability) {
				int damageAmount = (params.shotDamage >= 0) ? params.shotDamage : 0;
				if (applyPlayerDamage(damageAmount)) {
					debugC(DEBUG_INSANE, "PROBABILISTIC damage from enemy %d. Damage=%d total=%d",
						srcId, damageAmount, _playerDamage);
				}
				if (!_noDamage) {
					if (_rebelHandler == 8) {
						triggerDamageEffect();
					} else {
						initDamageFlash();
					}
				}
			}
		}
	} else {
		debugC(DEBUG_INSANE, "Opcode3: UNHANDLED par3=%d par4=%d", par3, par4);
	}
}

// Helper split out of FUN_004033CF case 6; not a separate original function.
void InsaneRebel2::updateOpcode6Handler(int16 par2) {
	// Update handler type if par2 is a known handler value (from FUN_4033CF case 6).
	if (par2 == 7 || par2 == 8 || par2 == 0x19 || par2 == 0x26) {
		// Reset Level 2 background flag when transitioning away from Handler 8
		if (_rebelHandler == 8 && par2 != 8) {
			_level2BackgroundLoaded = false;
		}
		_rebelHandler = par2;
		debugC(DEBUG_INSANE, "Opcode 6: Setting handler=%d", par2);
	}
}

// Helper split out of FUN_00401234 case 4; not a separate original function.
void InsaneRebel2::handleOpcode6Handler8(Common::SeekableReadStream &b, int16 par4) {
	// Handler 8 specific logic (third-person on foot) - FUN_00401234 case 4.
	// DAT_0043e000 = local_14[3], which maps to the IACT header's par4/userId.
	_shipLevelMode = par4;

	// local_14[4] is the first body word after the 8-byte IACT header.
	int16 bodyStatusFlag = 0;
	if (b.pos() + 2 <= b.size()) {
		int64 savedPos = b.pos();
		bodyStatusFlag = b.readSint16LE();
		b.seek(savedPos);
	}

	// If local_14[4] == 1, enable status bar and re-render laser texture (FUN_0040bb87)
	if (bodyStatusFlag == 1) {
		_rebelStatusBarSprite = 5;
		if (_smush_iconsNut && _smush_iconsNut->getNumChars() > 5) {
			initLaserTexture(_smush_iconsNut, 5);
		}
	}

	// Reset state when shipLevelMode != 0 && local_14[4] == 1 (FUN_00401234 lines 97-103)
	// Guard with _rebelOp6Initialized: runs once per wave video, not per frame.
	if (_shipLevelMode != 0 && bodyStatusFlag == 1 && !_rebelOp6Initialized) {
		clearBit(0);
		for (int i = 0; i < 512; i++) {
			_rebelLinks[i][0] = 0;
			_rebelLinks[i][1] = 0;
			_rebelLinks[i][2] = 0;
		}
		_rebelWaveState = _rebelPhaseState;
		_rebelOp6Initialized = true;
		debugC(DEBUG_INSANE, "Opcode 6 (Handler 8): Wave init, wave=0x%x", _rebelWaveState);
	}

	// Skip position calculation for special modes 4 and 5
	if (_shipLevelMode != 4 && _shipLevelMode != 5) {
		// ----- Movement Range Transition (Covered vs Shooting) -----
		// Based on FUN_00401234 lines 85-120:
		// Mode 2 = "Covered" state - contract movement range to 41 (0x29)
		// Other modes = "Shooting" state - expand movement range to 127 (0x7f)
		// Transition happens gradually at +/-10 per frame for smooth animation
		if (_shipLevelMode == 2) {
			// Covered state - contract movement range
			if (_movementRangeLimit > 41) {
				_movementRangeLimit -= 10;
			}
			if (_movementRangeLimit < 41) {
				_movementRangeLimit = 41;
			}
		} else {
			// Shooting state - expand movement range
			if (_movementRangeLimit < 127) {
				_movementRangeLimit += 10;
			}
			if (_movementRangeLimit > 127) {
				_movementRangeLimit = 127;
			}
		}

		// Calculate target position from mouse input
		// Mouse X maps to ship horizontal tilt, Mouse Y to vertical tilt
		// Based on FUN_00401234 lines 151-166:
		// local_18 = ((DAT_0047a7e0 * 5 + 0x27b) * 0x40) / 0xfe
		// local_1c = ((DAT_0047a7e2 * 5 + 0x27b) * 0x10) / 0xfe

		// Map the effective aim position (-127 to 127 range) to the ship target.
		Common::Point aimPos = getGameplayAimPoint();
		int16 mouseOffsetX = (int16)((aimPos.x - 160) * 127 / 160);
		int16 mouseOffsetY = (int16)((aimPos.y - 100) * 127 / 100);

		// Clamp X offset to movement range limit (covered/shooting state)
		// Based on FUN_00401234 lines 119-136
		if (mouseOffsetX > _movementRangeLimit)
			mouseOffsetX = _movementRangeLimit;
		if (mouseOffsetX < -_movementRangeLimit)
			mouseOffsetX = -_movementRangeLimit;
		// Y offset always uses full range (+/-127)
		if (mouseOffsetY > 127)
			mouseOffsetY = 127;
		if (mouseOffsetY < -127)
			mouseOffsetY = -127;

		// Calculate target positions using the original formula
		// Original FUN_00401234 lines 151-166:
		//   local_18 = ((mouseX * 5 + 0x27b) * 0x40) / 0xfe    -> X target
		//   local_1c = ((mouseY * 5 + 0x27b) * 0x10) / 0xfe    -> Y target
		//   _DAT_0043e004 = -local_1c   (stored negated for cursor display)
		// The interpolation (lines 181-193) uses local_1c (positive), NOT _DAT_0043e004.
		// So the interpolation target must be the positive formula result.
		_shipTargetX = (int16)(((mouseOffsetX * 5 + 0x27b) * 0x40) / 0xfe);
		_shipTargetY = (int16)(((mouseOffsetY * 5 + 0x27b) * 0x10) / 0xfe);

		// Smooth interpolation toward target (max 50 pixels per frame)
		const int16 maxStep = 50;  // 0x32 in hex
		if (_shipPosX < _shipTargetX) {
			int16 newX = _shipPosX + maxStep;
			_shipPosX = (newX > _shipTargetX) ? _shipTargetX : newX;
		} else if (_shipPosX > _shipTargetX) {
			int16 newX = _shipPosX - maxStep;
			_shipPosX = (newX < _shipTargetX) ? _shipTargetX : newX;
		}

		if (_shipPosY < _shipTargetY) {
			int16 newY = _shipPosY + maxStep;
			_shipPosY = (newY > _shipTargetY) ? _shipTargetY : newY;
		} else if (_shipPosY > _shipTargetY) {
			int16 newY = _shipPosY - maxStep;
			_shipPosY = (newY < _shipTargetY) ? _shipTargetY : newY;
		}

		// FUN_00401234 calls FUN_00424510(-DAT_0043e006, -DAT_0043e008)
		// after updating the handler-8 camera. This shifts subsequent FOBJ
		// decoding into screen coordinates; FUN_00401CCF then draws HUD and
		// weapon sprites without a separate final-buffer scroll.
		if (_player) {
			_player->_fobjOffsetX = -_shipPosX;
			_player->_fobjOffsetY = -_shipPosY;
		}

		// Calculate ship direction indices for sprite selection
		// Map mouse position to 5x7 direction grid (like Handler 7)
		int16 mouseX = aimPos.x;
		int16 mouseY = aimPos.y;

		// Scale mouse if video is larger than 320x200
		if (_player && _player->_width > 320) {
			mouseX = (mouseX * 320) / _player->_width;
		}
		if (_player && _player->_height > 200) {
			mouseY = (mouseY * 200) / _player->_height;
		}

		// Horizontal: 5 zones (0=far left, 2=center, 4=far right)
		if (mouseX < 64)
			_shipDirectionH = 0;
		else if (mouseX < 128)
			_shipDirectionH = 1;
		else if (mouseX < 192)
			_shipDirectionH = 2;
		else if (mouseX < 256)
			_shipDirectionH = 3;
		else
			_shipDirectionH = 4;

		// Vertical: 7 zones (0=far up, 3=center, 6=far down)
		if (mouseY < 28)
			_shipDirectionV = 0;
		else if (mouseY < 57)
			_shipDirectionV = 1;
		else if (mouseY < 86)
			_shipDirectionV = 2;
		else if (mouseY < 114)
			_shipDirectionV = 3;
		else if (mouseY < 143)
			_shipDirectionV = 4;
		else if (mouseY < 171)
			_shipDirectionV = 5;
		else
			_shipDirectionV = 6;

		_shipDirectionIndex = _shipDirectionH * 7 + _shipDirectionV;
	}

	// Update firing state from mouse button or joystick fire action
	// Mode 4 (autopilot) disables shooting - FUN_00401CCF line 82-84
	if (_shipLevelMode == 4) {
		_shipFiring = false;
	} else {
		_shipFiring = (_vm->VAR(_vm->VAR_LEFTBTN_HOLD) != 0) ||
			_vm->getActionState(kScummActionInsaneAttack);
	}

	debugC(DEBUG_INSANE, "Opcode 6 (Handler 8): mode=%d bodyFlag=%d range=%d shipPos=(%d,%d) target=(%d,%d) firing=%d dir=(%d,%d,%d)",
		_shipLevelMode, bodyStatusFlag, _movementRangeLimit, _shipPosX, _shipPosY, _shipTargetX, _shipTargetY, _shipFiring,
		_shipDirectionH, _shipDirectionV, _shipDirectionIndex);
}

// Helper split out of FUN_0040C3CC case 4; not a separate original function.
void InsaneRebel2::handleOpcode6Handler7(Common::SeekableReadStream &b, int16 par4) {
	// Handler 7 specific logic (third-person ship) - FUN_0040d836 / FUN_0040c3cc
	// Used for Level 3 and similar space combat levels.

	// Set control mode: DAT_004437c0 = param_5[3] = par4 in FUN_40C3CC case 4.
	// This determines collision mode and shooting capability:
	//   Mode 0: Obstacle avoidance - SECONDARY zones, corridor boundaries
	//   Mode 1: Tunnel flight - PRIMARY zones, per-edge push-back (hMargin=0x28)
	//   Mode 2: Combat mode - shooting ENABLED, SECONDARY zones
	//   Mode 3: Tunnel flight - PRIMARY zones, per-edge push-back (hMargin=0x0f)
	_flyControlMode = par4;
	debugC(DEBUG_INSANE, "Opcode 6 (Handler 7): Control mode set to %d (shooting %s)",
		par4, (par4 == 2) ? "ENABLED" : "DISABLED");

	// Status bar: param_5[4] == 1 in original (first body word, 5th IACT word)
	// In our parsing, par3 maps to param_5[2] and the body follows par4.
	// FUN_40C3CC: if (param_5[4] == 1) FUN_0040bb87(DAT_0047a828,5);
	// par3 is param_5[2], which the original doesn't use here.
	// The body word for status bar is read separately below.
	int16 bodyStatusFlag = 0;
	if (b.size() - b.pos() >= 2) {
		bodyStatusFlag = b.readSint16LE();
	}
	if (bodyStatusFlag == 1) {
		_rebelStatusBarSprite = 5;
		if (_smush_iconsNut && _smush_iconsNut->getNumChars() > 5) {
			initLaserTexture(_smush_iconsNut, 5);
		}
		debugC(DEBUG_INSANE, "Opcode 6 (Handler 7): Status bar enabled (body flag=%d)", bodyStatusFlag);
	}

	// Ship position update - FUN_40C3CC case 4, lines 49-327.
	// Velocity-based physics with momentum/inertia:
	//   Mouse offset from center -> scaled input [-127,127]
	//   -> velocity history averaging -> physics delta (clamped +/-12/frame)
	//   -> position clamping -> corridor collision -> perspective offsets
	//
	// Level data table (DAT_0047e0e8 + difficulty*0x242 + levelType*0x22):
	//   offset 2: Y speed          offset 4: X speed (levelSpeed)
	//   offset 6: wind multiplier
	// Our extracted difficulty table starts at DAT_0047e0f0, so for Handler 7
	// level types these fields map to lift/slide/drift of the preceding row.
	const int flightParamIndex = CLIP(_rebelLevelType - 1, 0, 16);
	const LevelDifficultyParams &flightParams =
		kDifficultyTable[CLIP(_difficulty, 0, 5)][flightParamIndex];

	// Step 1: Raw mouse input as offset from screen center.
	// DAT_0047a7e0 = mouseX - 160, DAT_0047a7e2 = mouseY - 100.
	// Handler 7 applies DAT_0047a7fe to its local vertical input after clamping.
	const Common::Point aimPos = getGameplayAimPoint();
	const int16 mouseX = aimPos.x;
	const int16 mouseY = aimPos.y;
	int16 inputX = (int16)(mouseX - 160);  // DAT_0047a7e0
	int16 inputY = (int16)(mouseY - 100);  // DAT_0047a7e2

	// Clamp: mouse mode uses [-160, 160] for X, [-127, 127] for Y (lines 55-70).
	if (inputX > 160)
		inputX = 160;
	if (inputX < -160)
		inputX = -160;
	if (inputY > 127)
		inputY = 127;
	if (inputY < -127)
		inputY = -127;

	// Step 2: Scale to [-127, 127] (lines 82-84).
	// Mouse mode: scaledInputX = (DAT_0047a7e0 * 0x7f) / 0xa0.
	int16 scaledInputX = (int16)((inputX * 127) / 160);
	int16 scaledInputY = _optControlsFlipped ? (int16)-inputY : inputY;  // local_14

	// Direct mouse/touch/gamepad aiming can hold the cursor at an edge
	// indefinitely. Keep this sensitivity concession local to Handler 7
	// third-person ship steering. A held analog stick is already bounded by its
	// physical range, so keep its full logical range for more responsive L10 flight.
	if (!_gamepadAimActive) {
		scaledInputX = (int16)((scaledInputX * kRA2Handler7DirectInputNumerator) /
			kRA2Handler7DirectInputDenominator);
		scaledInputY = (int16)((scaledInputY * kRA2Handler7DirectInputNumerator) /
			kRA2Handler7DirectInputDenominator);
	}
	// Mouse/touch can hold an absolute cursor at an edge indefinitely, so they use
	// bounded target steering. Gamepad input for Handler 7 now feeds the original
	// center-relative flight axes and lets the assembly-derived velocity history,
	// lift/slide tables, wind, and clamps produce the ship movement.
	const bool useTargetSteering = !_gamepadAimActive;
	int16 mouseFlightTargetX = _flyShipScreenX;
	if (useTargetSteering) {
		mouseFlightTargetX = (int16)(0xd4 + (scaledInputX * kRA2Handler7MouseTargetRangeX) / 127);
		mouseFlightTargetX = CLIP<int16>(mouseFlightTargetX, 0x14, 0x194);
	}

	// Step 3: Velocity history + smoothed average (lines 141-157).
	for (int i = 24; i > 0; i--) {
		_velocityHistory[i] = _velocityHistory[i - 1];
	}
	_velocityHistory[0] = scaledInputX;

	const int smoothWindow = 5;
	int velSum = 0;
	for (int i = 0; i < smoothWindow; i++) {
		velSum += _velocityHistory[i];
	}
	_smoothedVelocity = (int16)(velSum / smoothWindow);  // DAT_0044370c

	// Step 4: Wind history (lines 158-173).
	const int16 windMult = flightParams.driftRate;
	int windSumX = 0, windSumY = 0;
	for (int i = 14; i > 0; i--) {
		_windHistoryX[i] = _windHistoryX[i - 1];
		windSumX += _windHistoryX[i];
	}
	_windHistoryX[0] = _windParamX;
	int16 windEffectX = (int16)((windMult * (windSumX + _windParamX)) / 15);

	for (int i = 14; i > 0; i--) {
		_windHistoryY[i] = _windHistoryY[i - 1];
		windSumY += _windHistoryY[i];
	}
	_windHistoryY[0] = _windParamY;
	int16 windEffectY = (int16)((windMult * (windSumY + _windParamY)) / 15);

	// Step 5: Position delta (lines 174-242).
	const int16 levelSpeed = flightParams.slideRate;
	const int16 levelYSpeed = flightParams.liftRate;
	int16 absSmoothVel = ABS(_smoothedVelocity);
	int16 positionDeltaX;

	if (_flyControlMode == 1) {
		// Mode 1: Full cross-axis coupling (lines 174-186).
		// Banking: vertical input deflects horizontal movement.
		if (scaledInputX < 1) {
			positionDeltaX = (int16)((levelSpeed * _smoothedVelocity - absSmoothVel * scaledInputY - windEffectX) >> 9);
		} else {
			positionDeltaX = (int16)((levelSpeed * _smoothedVelocity + absSmoothVel * scaledInputY - windEffectX) >> 9);
		}
	} else {
		// Mode 0/2/3: Reduced cross-axis coupling (lines 218-230).
		if (scaledInputX < 1) {
			positionDeltaX = (int16)((levelSpeed * _smoothedVelocity - (absSmoothVel * scaledInputY >> 2) - windEffectX) >> 9);
		} else {
			positionDeltaX = (int16)((levelSpeed * _smoothedVelocity + (absSmoothVel * scaledInputY >> 2) - windEffectX) >> 9);
		}
	}

	// Clamp X delta to +/-12 per frame (lines 187-192 / 231-236).
	if (positionDeltaX < -11)
		positionDeltaX = -12;
	if (positionDeltaX > 11)
		positionDeltaX = 12;

	// FUN_0040C3CC integrates relative flight axes. The real mouse is an
	// absolute position, so steer toward a bounded position target instead of
	// letting a held off-center cursor keep pushing the ship until it bounces.
	if (useTargetSteering) {
		int targetDeltaX = mouseFlightTargetX - _flyShipScreenX;
		const int targetSteeringDivisor = _gamepadAimActive ? 2 : 4;
		positionDeltaX = (int16)CLIP<int>(targetDeltaX / targetSteeringDivisor, -12, 12);
		if (positionDeltaX == 0 && targetDeltaX != 0)
			positionDeltaX = (targetDeltaX < 0) ? -1 : 1;
	}

	// Apply X delta (line 193 / 237).
	_flyShipScreenX += positionDeltaX;

	// Y delta.
	if (_flyControlMode == 1) {
		// Mode 1: clamped to +/-12 with wind (lines 194-216).
		int yCalc = levelYSpeed * scaledInputY - (windEffectY >> 1);
		int yDelta = yCalc >> 10;
		if (yDelta < -12)
			yDelta = -12;
		if (yDelta > 12)
			yDelta = 12;
		_flyShipScreenY -= (int16)yDelta;
	} else {
		// Mode 0/2/3: unclamped (lines 238-241).
		_flyShipScreenY -= (int16)((levelYSpeed * scaledInputY) >> 10);
	}

	// Store vertical input for direction sprite (line 243).
	_verticalInput = scaledInputY;  // DAT_0044370e

	// Ship facing direction (line 244).
	_facingRight = (0xd4 < _smoothedVelocity + _flyShipScreenX);

	// Step 6: Position clamping (lines 245-256).
	if (_flyShipScreenX > 0x194)
		_flyShipScreenX = 0x194;  // 404
	if (_flyShipScreenY > 0xF0)
		_flyShipScreenY = 0xF0;    // 240
	if (_flyShipScreenX < 0x14)
		_flyShipScreenX = 0x14;    // 20
	if (_flyShipScreenY < 0x14)
		_flyShipScreenY = 0x14;    // 20

	// Step 7: Corridor collision - mode 0/2 only (lines 257-292).
	if (_flyControlMode == 0 || _flyControlMode == 2) {
		LevelDifficultyParams wallParams = getDifficultyParams();
		int corridorWallDmg = (wallParams.dodgeDamage >= 0) ? wallParams.dodgeDamage : 0;

		// Right boundary (lines 258-270).
		// Original: position is ALWAYS clamped; damage/bounce only when cooldown < 5.
		if (_corridorRightX < _flyShipScreenX) {
			_flyShipScreenX = _corridorRightX;
			if (_hitCooldown < 5) {
				for (int i = 0; i < 25; i++)
					_velocityHistory[i] = -127;
				_hitCooldown = 10;
				_spaceShotDirection = 1;
				applyPlayerDamage(corridorWallDmg);
				if (!_noDamage)
					initDamageFlash();
				_rebelHitCounter++;
				playSfx(1, 127, 100);  // CRASH.SAD, right wall, pan right
			}
		}

		// Left boundary (lines 271-283).
		if (_flyShipScreenX < _corridorLeftX) {
			_flyShipScreenX = _corridorLeftX;
			if (_hitCooldown < 5) {
				for (int i = 0; i < 25; i++)
					_velocityHistory[i] = 127;
				_hitCooldown = 10;
				_spaceShotDirection = 0;
				applyPlayerDamage(corridorWallDmg);
				if (!_noDamage)
					initDamageFlash();
				_rebelHitCounter++;
				playSfx(1, 127, -100);  // CRASH.SAD, left wall, pan left
			}
		}

		// Y boundary clamping - no damage (lines 285-292).
		if (_corridorBottomY < _flyShipScreenY) {
			_flyShipScreenY = _corridorBottomY;
		}
		if (_flyShipScreenY < _corridorTopY) {
			_flyShipScreenY = _corridorTopY;
		}
	}

	// Step 8: Perspective offsets (lines 293-316).
	// f(x) = (focal * center * |offset|) / ((center - focal) * |offset| + focal * center)
	// Close view (DAT_0047a7fc < 1): focalX=0x34, focalY=0x2d.
	// Far view (DAT_0047a7fc >= 1): focalX=0x2b, focalY=0x19.
	{
		int absOffX = ABS(_flyShipScreenX - 0xd4);
		int16 focalX = 0x2b;  // Far view default for Level 3
		if (absOffX > 0) {
			_perspectiveX = (int16)((focalX * 0xd4 * absOffX) /
				((0xd4 - focalX) * absOffX + focalX * 0xd4));
		} else {
			_perspectiveX = 0;
		}
		if (_flyShipScreenX < 0xd5)
			_perspectiveX = -_perspectiveX;

		int absOffY = ABS(_flyShipScreenY - 0x82);
		int16 focalY = 0x19;  // Far view default for Level 3
		if (absOffY > 0) {
			_perspectiveY = (int16)((focalY * 0x82 * absOffY) /
				((0x82 - focalY) * absOffY + focalY * 0x82));
		} else {
			_perspectiveY = 0;
		}
		if (_flyShipScreenY < 0x83)
			_perspectiveY = -_perspectiveY;
	}

	// View shift = clamped smoothed velocity (FUN_0040d836 lines 68-74).
	_viewShift = _smoothedVelocity;
	if (_viewShift > 127)
		_viewShift = 127;
	if (_viewShift < -127)
		_viewShift = -127;

	// Step 9: Direction sprite (FUN_0040d836 lines 88-106).
	// 5x7 grid: vDir(0-4) * 7 + hDir(0-6) = sprite index (0-34).
	// vDir from vertical input: (0xa0 - verticalInput) >> 6.
	int16 vDir = (int16)(((int)(0xa0 - _verticalInput) + ((0xa0 - _verticalInput) < 0 ? 63 : 0)) >> 6);
	if (vDir < 0)
		vDir = 0;
	if (vDir > 4)
		vDir = 4;

	// hDir from smoothed velocity: (0x95 - smoothedVelocity) / 0x2b.
	int16 hDir = (int16)((0x95 - _smoothedVelocity) / 0x2b);
	if (hDir < 0)
		hDir = 0;
	if (hDir > 6)
		hDir = 6;

	// Hysteresis at center (lines 90-97, 98-105).
	if (hDir == 3 && ABS(_smoothedVelocity) > 10) {
		hDir = (_smoothedVelocity < 1) ? 4 : 2;
	}
	if (vDir == 2 && ABS(_verticalInput) > 15) {
		vDir = (_verticalInput < 1) ? 3 : 1;
	}

	_shipDirectionIndex = vDir * 7 + hDir;
	if (_shipDirectionIndex < 0)
		_shipDirectionIndex = 0;
	if (_shipDirectionIndex > 34)
		_shipDirectionIndex = 34;

	_shipFiring = (_flyControlMode == 2) &&
		((_vm->VAR(_vm->VAR_LEFTBTN_HOLD) != 0) ||
		 _vm->getActionState(kScummActionInsaneAttack));

	debugC(DEBUG_INSANE, "H7: mouse=(%d,%d) raw=(%d,%d) scaled=(%d,%d) targetX=%d pos=(%d,%d) "
		"vel=%d vIn=%d dx=%d dir=%d mode=%d targetSteer=%d padAim=%d",
		mouseX, mouseY, inputX, inputY, scaledInputX, scaledInputY,
		mouseFlightTargetX, _flyShipScreenX, _flyShipScreenY, _smoothedVelocity,
		_verticalInput, positionDeltaX, _shipDirectionIndex, _flyControlMode,
		useTargetSteering ? 1 : 0, _gamepadAimActive ? 1 : 0);
}

// Helper split out of FUN_0041CADB case 4; not a separate original function.
void InsaneRebel2::handleOpcode6Handler25(byte *renderBitmap, Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4) {
	// Handler 25 (0x19) specific logic (mixed mode - speeder bike).
	// Based on FUN_0041cadb case 4 (opcode 6) lines 113-229.

	// Read the reset flag from IACT data at offset 8-9 (local_14[4] in decompiled code).
	// The stream position should be at offset 8 after par4 was read.
	// From FUN_0041cadb line 114: if (local_14[4] == 1) { ... reset ... }
	int16 par5 = 0;
	if (b.pos() + 2 <= b.size()) {
		int64 savedPos = b.pos();
		par5 = b.readSint16LE();
		b.seek(savedPos);  // Don't consume the stream
	}

	// If par5 == 1, enable status bar and reset state (lines 114-121).
	// Note: This is local_14[4] in the decompiled code, NOT local_14[3] (par4).
	if (par5 == 1) {
		_rebelStatusBarSprite = 5;
		if (_smush_iconsNut && _smush_iconsNut->getNumChars() > 5) {
			initLaserTexture(_smush_iconsNut, 5);
		}

		// Guard with _rebelOp6Initialized: runs once per wave video, not per frame.
		if (!_rebelOp6Initialized) {
			clearBit(0);
			for (int i = 0; i < 512; i++) {
				_rebelLinks[i][0] = 0;
				_rebelLinks[i][1] = 0;
				_rebelLinks[i][2] = 0;
			}
			_rebelWaveState = _rebelPhaseState;
			_rebelOp6Initialized = true;
			debugC(DEBUG_INSANE, "Opcode 6 (Handler 25): Wave init, wave=0x%x autopilot=%d damageLevel=%d",
				_rebelWaveState, _rebelAutopilot, _rebelDamageLevel);
		}
	}

	// Set sprite mode (DAT_00457900 = local_14[3]) - controls which GRD sprite to render.
	// From FUN_0041cadb line 122: DAT_00457900 = local_14[3].
	// In this IACT parser: local_14[3] = offset 6-7 = par4.
	// Mode 1: Uncovered, shooting position - sprite on left
	// Mode 2: Covered, vertical shift
	// Mode 3: Transition between covered/uncovered - sprite position depends on direction
	// Mode 4: Alternative uncovered position - sprite on right
	_grdSpriteMode = par4;  // local_14[3] maps to par4 (offset 6-7)

	debugC(DEBUG_INSANE, "Handler25 Opcode6: par2=%d par3=%d par4=%d(mode) par5=%d(reset) autopilot=%d damageLevel=%d controlMode=%d",
		par2, par3, par4, par5, _rebelAutopilot, _rebelDamageLevel, _rebelControlMode);

	// Autopilot logic (lines 123-146).
	// From original FUN_0041cadb - NO damageLevel check, toggle happens immediately.
	// The damage level counter provides the smooth visual transition.
	if (!_rebelAutoPlay) {
		if (_rebelAutopilot == 0) {
			// Uncovered: RIGHT button enters cover.
			if ((_rebelControlMode & 2) != 0) {
				_rebelAutopilot = 1;
				debugC(DEBUG_INSANE, "Handler25: Entering cover (right click), controlMode=%d", _rebelControlMode);
			}
		} else {
			// Covered: ANY button exits cover.
			if (_rebelControlMode != 0) {
				_rebelAutopilot = 0;
				debugC(DEBUG_INSANE, "Handler25: Exiting cover (button click), controlMode=%d", _rebelControlMode);
			}
		}

		// Clear control mode after processing (sticky flags consumed).
		_rebelControlMode = 0;
	} else {
		// Auto play: random autopilot changes.
		if (_rebelAutopilot == 0) {
			if (_vm->_rnd.getRandomNumber(100) == 0) {
				_rebelAutopilot = 1;
			}
		} else {
			if (_vm->_rnd.getRandomNumber(15) == 0) {
				_rebelAutopilot = 0;
				_rebelFlightDir = _vm->_rnd.getRandomNumber(2);
			}
		}
	}

	// Update damage level counter (lines 147-154).
	// This provides the smooth transition animation between covered/uncovered states.
	int prevDamageLevel = _rebelDamageLevel;
	if (_rebelAutopilot == 0) {
		// Uncovered: decrement damage level towards 0.
		if (_rebelDamageLevel > 0) {
			_rebelDamageLevel--;
		}
	} else {
		// Covered: increment damage level towards 5.
		if (_rebelDamageLevel < 5) {
			_rebelDamageLevel++;
		}
	}
	if (_rebelDamageLevel != prevDamageLevel) {
		debugC(DEBUG_INSANE, "Handler25: damageLevel transition %d -> %d (autopilot=%d)",
			prevDamageLevel, _rebelDamageLevel, _rebelAutopilot);
	}

	// Flight direction logic for mode 3 (lines 155-177).
	if (_grdSpriteMode == 3) {
		if (_rebelDamageLevel == 5) {
			// At max damage, check for direction change input.
			int16 mouseX = getGameplayAimPoint().x;
			if (_player && _player->_width > 320) {
				mouseX = (mouseX * 320) / _player->_width;
			}
			if (mouseX > 235) {  // 0x4b + 160 = 235
				_rebelFlightDir = 1;
			}
			if (mouseX < 85) {   // 160 - 0x4b = 85
				_rebelFlightDir = 0;
			}
		}
	} else {
		_rebelFlightDir = 0;
	}

	// Calculate sprite and view offset positions based on mode (lines 182-213).
	// DAT_0045790c = view offset X (for corridor overlay)
	// DAT_0045790e = view offset Y (for corridor overlay)
	// DAT_00457910 = sprite position X (relative to center)
	// DAT_00457912 = sprite position Y (relative to center)
	if (_grdSpriteMode == 1) {
		// Mode 1: Uncovered, shooting - sprite shifts left as damage increases.
		_rebelViewMode1 = 0x0e;
		_rebelViewMode2 = 0;
		_rebelViewOffsetX = _rebelDamageLevel * -5 + -14;   // DAT_0045790c
		_rebelViewOffset2X = _rebelDamageLevel * -22;       // DAT_00457910
		_rebelViewOffsetY = 0;                              // DAT_0045790e
		_rebelViewOffset2Y = 0;                             // DAT_00457912
	} else if (_grdSpriteMode == 4) {
		// Mode 4: Alternative uncovered - sprite shifts right.
		_rebelViewMode1 = 0x22;
		_rebelViewMode2 = 0;
		_rebelViewOffsetX = _rebelDamageLevel * 10 + -16;   // DAT_0045790c
		_rebelViewOffset2X = _rebelDamageLevel * 17 + -85;  // DAT_00457910 (0x11 = 17, -0x55 = -85)
		_rebelViewOffsetY = 0;
		_rebelViewOffset2Y = 0;
	} else if (_grdSpriteMode == 2) {
		// Mode 2: Covered - vertical shift.
		_rebelViewMode1 = 0;
		_rebelViewMode2 = 0x0e;
		_rebelViewOffsetY = _rebelDamageLevel * -5 + -14;   // DAT_0045790e
		_rebelViewOffset2Y = (5 - _rebelDamageLevel) * 15 + -60;  // DAT_00457912 (0xf = 15, -0x3c = -60)
		_rebelViewOffsetX = 0;
		_rebelViewOffset2X = 0;
	} else if (_grdSpriteMode == 3) {
		// Mode 3: Transition - direction-dependent horizontal shift.
		_rebelViewMode1 = 0x0f;
		_rebelViewMode2 = 0;
		// (-(DAT_00457902 == 0) & 6) - 3 = if dir==0: 6-3=3, else 0-3=-3
		int16 dirMultX = (_rebelFlightDir == 0) ? 3 : -3;
		// (-(DAT_00457902 == 0) & 0x28) - 0x14 = if dir==0: 40-20=20, else 0-20=-20
		int16 dirMultX2 = (_rebelFlightDir == 0) ? 20 : -20;
		_rebelViewOffsetX = dirMultX * (5 - _rebelDamageLevel) + -15;  // DAT_0045790c
		_rebelViewOffset2X = dirMultX2 * (5 - _rebelDamageLevel);      // DAT_00457910
		_rebelViewOffsetY = 0;
		_rebelViewOffset2Y = 0;
	} else {
		// Mode 0 or unknown: use Mode 1 defaults as fallback.
		_rebelViewMode1 = 0x0e;
		_rebelViewMode2 = 0;
		_rebelViewOffsetX = _rebelDamageLevel * -5 + -14;
		_rebelViewOffset2X = _rebelDamageLevel * -22;
		_rebelViewOffsetY = 0;
		_rebelViewOffset2Y = 0;
		debugC(DEBUG_INSANE, "Opcode 6 (Handler 25): Unknown mode %d, using Mode 1 fallback", _grdSpriteMode);
	}

	debugC(DEBUG_INSANE, "Opcode 6 (Handler 25): mode=%d damage=%d dir=%d autopilot=%d viewOff=(%d,%d) spritePos=(%d,%d)",
		_grdSpriteMode, _rebelDamageLevel, _rebelFlightDir, _rebelAutopilot,
		_rebelViewOffsetX, _rebelViewOffsetY, _rebelViewOffset2X, _rebelViewOffset2Y);

	// Set FOBJ position offsets (FUN_00424510 in original, line 214).
	// All subsequent FOBJs in this frame will be shifted by these offsets.
	if (_player) {
		_player->_fobjOffsetX = _rebelViewOffsetX;
		_player->_fobjOffsetY = _rebelViewOffsetY;
	}

	// Draw corridor overlay opaquely (FUN_00428a10 in original, line 216).
	// This wipes previous frame content so codec 23 delta skip regions show clean corridor.
	drawHandler25CorridorOverlay(renderBitmap);
}

// Helper split out of FUN_00407FCB case 4; not a separate original function.
void InsaneRebel2::handleOpcode6Turret(Common::SeekableReadStream &b, int16 par4) {
	// Handler 0x26: FUN_407FCB line 77-79 - set level type from par4, read par5 for init trigger.
	// param_5[3] = par4 = levelType, param_5[4] = par5 = init flag.
	_rebelLevelType = par4;

	// Read par5 from IACT body (param_5[4]).
	int16 par5 = 0;
	if (b.pos() + 2 <= b.size()) {
		int64 savedPos = b.pos();
		par5 = b.readSint16LE();
		b.seek(savedPos);
	}

	if (par5 == 1) {
		// Re-render laser texture for this level (FUN_0040bb87).
		// levelType 5 uses sprite 53, all others use sprite 5.
		_rebelStatusBarSprite = (_rebelLevelType == 5) ? 53 : 5;
		if (_smush_iconsNut && _smush_iconsNut->getNumChars() > _rebelStatusBarSprite) {
			initLaserTexture(_smush_iconsNut, _rebelStatusBarSprite);
		}

		if (!_rebelOp6Initialized) {
			clearBit(0);
			for (int i = 0; i < 512; i++) {
				_rebelLinks[i][0] = 0;
				_rebelLinks[i][1] = 0;
				_rebelLinks[i][2] = 0;
			}
			_rebelWaveState = _rebelPhaseState;
			_rebelHitCounter = 0;
			_rebelOp6Initialized = true;
			debugC(DEBUG_INSANE, "Opcode 6 (Handler 0x26): Wave init, levelType=%d waveState=0x%x",
				_rebelLevelType, _rebelWaveState);
		}
	}
}

// Helper split out of generic opcode 6 initialization; not a separate original function.
void InsaneRebel2::handleOpcode6GenericInit(int16 par4) {
	// Other handlers: par4 == 1 triggers init (NOT level type).
	if (_rebelHandler != 0x26 && par4 == 1) {
		_rebelStatusBarSprite = 5;
		if (_smush_iconsNut && _smush_iconsNut->getNumChars() > 5) {
			initLaserTexture(_smush_iconsNut, 5);
		}

		if (!_rebelOp6Initialized) {
			clearBit(0);
			for (int i = 0; i < 512; i++) {
				_rebelLinks[i][0] = 0;
				_rebelLinks[i][1] = 0;
				_rebelLinks[i][2] = 0;
			}
			_rebelWaveState = _rebelPhaseState;
			_rebelHitCounter = 0;
			_rebelOp6Initialized = true;
			debugC(DEBUG_INSANE, "Opcode 6: Wave init - cleared bits/links, waveState=0x%x", _rebelWaveState);
		}
	}
}

// Helper split out of generic opcode 6 flight state; not a separate original function.
void InsaneRebel2::updateOpcode6GenericFlightState() {
	// Step 3: Autopilot/control mode logic (lines 123-146)
	// This determines whether the ship flies on autopilot or manual control.
	if (!_rebelAutoPlay) {
		// Normal mode: check control mode flags.
		if (_rebelAutopilot == 0) {
			if ((_rebelControlMode & 2) != 0) {
				_rebelAutopilot = 1;
			}
		} else {
			if (_rebelControlMode != 0) {
				_rebelAutopilot = 0;
			}
		}
	} else {
		// Auto play: random autopilot changes.
		if (_rebelAutopilot == 0) {
			if (_vm->_rnd.getRandomNumber(100) == 0) {
				_rebelAutopilot = 1;
			}
		} else {
			if (_vm->_rnd.getRandomNumber(15) == 0) {
				_rebelAutopilot = 0;
				_rebelFlightDir = _vm->_rnd.getRandomNumber(2);
			}
		}
	}

	// Step 4: Update damage level counter (lines 147-154).
	if (_rebelAutopilot == 0) {
		if (_rebelDamageLevel > 0) {
			_rebelDamageLevel--;
		}
	} else {
		if (_rebelDamageLevel < 5) {
			_rebelDamageLevel++;
		}
	}

	// Handle level type 3 special direction logic (lines 155-181).
	if (_rebelLevelType == 3) {
		if (_rebelDamageLevel == 5) {
			// Check for joystick/key input to change direction.
			// Simplified: use mouse position.
			int16 mouseX = getGameplayAimPoint().x;
			if (mouseX > 235) {
				_rebelFlightDir = 1;
			}
			if (mouseX < 85) {
				_rebelFlightDir = 0;
			}
		}
	} else {
		_rebelFlightDir = 0;
	}

	// Step 5: Calculate view offsets based on level type (lines 182-213).
	switch (_rebelLevelType) {
	case 1:
		// Type 1: Vertical movement.
		_rebelViewMode1 = 0x0e;
		_rebelViewMode2 = 0;
		_rebelViewOffsetX = _rebelDamageLevel * -5 - 0x0e;
		_rebelViewOffset2X = _rebelDamageLevel * -0x16;
		_rebelViewOffsetY = 0;
		_rebelViewOffset2Y = 0;
		break;

	case 4:
		// Type 4: Different vertical movement.
		_rebelViewMode1 = 0x22;
		_rebelViewMode2 = 0;
		_rebelViewOffsetX = _rebelDamageLevel * 10 - 0x10;
		_rebelViewOffset2X = _rebelDamageLevel * 0x11 - 0x55;
		_rebelViewOffsetY = 0;
		_rebelViewOffset2Y = 0;
		break;

	case 2:
		// Type 2: Horizontal movement.
		_rebelViewMode1 = 0;
		_rebelViewMode2 = 0x0e;
		_rebelViewOffsetY = _rebelDamageLevel * -5 - 0x0e;
		_rebelViewOffset2Y = (5 - _rebelDamageLevel) * 0x0f - 0x3c;
		_rebelViewOffsetX = 0;
		_rebelViewOffset2X = 0;
		break;

	case 3:
		// Type 3: Direction-based movement.
		_rebelViewMode1 = 0x0f;
		_rebelViewMode2 = 0;
		{
			int dirFactor = (_rebelFlightDir == 0) ? 3 : -3;  // (-(ushort)(DAT_00457902 == 0) & 6) - 3
			int dirFactor2 = (_rebelFlightDir == 0) ? 0x14 : -0x14;  // (-(ushort)(DAT_00457902 == 0) & 0x28) - 0x14
			_rebelViewOffsetX = dirFactor * (5 - _rebelDamageLevel) - 0x0f;
			_rebelViewOffset2X = dirFactor2 * (5 - _rebelDamageLevel);
		}
		_rebelViewOffsetY = 0;
		_rebelViewOffset2Y = 0;
		break;

	default:
		// Default: No special offsets.
		_rebelViewMode1 = 0;
		_rebelViewMode2 = 0;
		_rebelViewOffsetX = 0;
		_rebelViewOffsetY = 0;
		_rebelViewOffset2X = 0;
		_rebelViewOffset2Y = 0;
		break;
	}

	debugC(DEBUG_INSANE, "Opcode 6: levelType=%d autopilot=%d damageLevel=%d viewOffset=(%d,%d)",
		_rebelLevelType, _rebelAutopilot, _rebelDamageLevel, _rebelViewOffsetX, _rebelViewOffsetY);
}

// Helper split out of opcode 6 embedded ANIM scanning; not a separate original function.
void InsaneRebel2::scanOpcode6EmbeddedAnim(byte *renderBitmap, Common::SeekableReadStream &b, int32 chunkSize, int16 par4) {
	// Detect and load embedded ANIM (SAN) within the remaining IACT payload.
	// Note: chunkSize is the remaining IACT payload size after par1-par4 header.
	int64 startPos = b.pos();

	// Use chunkSize (remaining IACT payload) rather than b.size() (entire FRME stream).
	int64 remaining = chunkSize;
	if (remaining > 0) {
		int scanSize = (int)MIN<int64>(remaining, 65536);
		byte *scanBuf = (byte *)malloc(scanSize);
		if (scanBuf) {
			int bytesRead = b.read(scanBuf, scanSize);
			for (int i = 0; i + 8 <= bytesRead; ++i) {
				if (READ_BE_UINT32(scanBuf + i) == MKTAG('A','N','I','M')) {
					int64 animStreamPos = startPos + i;
					uint32 animReportedSize = READ_BE_UINT32(scanBuf + i + 4);

					// Limit to remaining IACT payload (chunkSize - offset into payload).
					int32 toCopy = (int)MIN<int64>((int64)animReportedSize + 8, chunkSize - i);
					if (toCopy > 0) {
						byte *animData = (byte *)malloc(toCopy);
						if (animData) {
							b.seek(animStreamPos);
							b.read(animData, toCopy);
							loadEmbeddedSan(par4, animData, toCopy, renderBitmap);
							free(animData);
						}
					}
					b.seek(startPos);
					free(scanBuf);
					return;
				}
			}
			b.seek(startPos);
			free(scanBuf);
		}
	}
}

//
// iactRebel2Opcode6 -- Level setup / mode switch (FUN_41CADB case 4)
//
// Per-wave initialization: clears bit table, resets link tables, configures
// handler mode (ship/turret/corridor), and loads collision zones. The original
// gates the reset with handler-specific IACT fields, not always frame 0.
//
void InsaneRebel2::iactRebel2Opcode6(byte *renderBitmap, Common::SeekableReadStream &b, int32 chunkSize, int16 par2, int16 par3, int16 par4) {
	// Opcode 6: Level setup / mode switch
	// Based on FUN_41CADB case 4 (switch on *local_14 - 2 == 4, meaning opcode 6)
	//
	// For Handler 8 (third-person on foot) - FUN_00401234 case 4:
	// - par4 sets ship level mode (DAT_0043e000)
	// - first body word == 1 triggers status bar display and state reset
	// - Updates ship position based on mouse input
	//
	// For Handler 0x26/0x19 (turret/FPS):
	// - Handler-specific status/reset word
	// - Different view offset calculations

	debugC(DEBUG_INSANE, "IACT Opcode 6: par2=%d par3=%d par4=%d", par2, par3, par4);

	updateOpcode6Handler(par2);

	if (_rebelHandler == 8) {
		handleOpcode6Handler8(b, par4);
		return;
	}

	if (_rebelHandler == 7) {
		handleOpcode6Handler7(b, par4);
		return;
	}

	if (_rebelHandler == 25) {
		handleOpcode6Handler25(renderBitmap, b, par2, par3, par4);
		return;
	}

	if (_rebelHandler == 0x26) {
		handleOpcode6Turret(b, par4);
	}

	handleOpcode6GenericInit(par4);
	updateOpcode6GenericFlightState();
	scanOpcode6EmbeddedAnim(renderBitmap, b, chunkSize, par4);
}

//
// iactRebel2Opcode8 -- HUD/Ship resource loading (FUN_0040c3cc / FUN_00401234 / FUN_00407fcb)
//
// Decodes embedded ANIM data from IACT chunks and dispatches to
// handler-specific loaders for NUT sprites, HUD overlays, and backgrounds.
//
// Handler-specific routing:
//   Handler 7  (FLY):  FLY NUT sprites via par4 (1, 2, 3, 11)
//   Handler 8  (POV):  POV NUT sprites via par3 (1, 3, 6, 7) or background via par4=5
//   Handler 0x26 (turret): Turret HUD NUT via par3/par4 (1-4)
//   Handler 0x19: Speeder bike GRD/HUD resources via par4
//
// Helper split out of FUN_0040C3CC case 6; not a separate original function.
bool InsaneRebel2::loadOpcode8Handler7FlySprites(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4) {
	// Handler 7: FLY NUT Loading (Third-Person Ship)
	// FUN_0040c3cc case 6: par4 determines FLY sprite slot.
	bool isHandler7FLY = (_rebelHandler == 7 && (par4 == 1 || par4 == 2 || par4 == 3 || par4 == 11));
	if (isHandler7FLY && remaining >= 14) {
		if (loadHandler7FlySprites(b, remaining, par4)) {
			b.seek(startPos);
			return true;
		}
		b.seek(startPos);
	}

	return false;
}

// Helper split out of FUN_0040C3CC case 6 shot-table loading; not a separate original function.
bool InsaneRebel2::loadOpcode8Handler7ShotTable(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4) {
	// FUN_0040c3cc case 6:
	//   par4=12 -> FUN_0040fcfa(text, DAT_004437c2, DAT_00443808)
	//   par4=13 -> FUN_0040fcfa(text, DAT_0044384e, DAT_00443894)
	if (_rebelHandler != 7 || (par4 != 12 && par4 != 13))
		return false;

	if (loadHandler7ShotTable(b, startPos, remaining, par4)) {
		b.seek(startPos);
		return true;
	}

	b.seek(startPos);
	return false;
}

// Helper split out of FUN_00405663 edge-table loading; not a separate original function.
bool InsaneRebel2::loadOpcode8EdgeTable(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4) {
	// Edge Blend Table Loading (par4 == 1000)
	// FUN_405663: After all handler-specific opcode 8 processing, checks if par4==1000.
	// If so, loads a per-level 256x256 color blend table from the IACT chunk data.
	// This table controls the edge glow color of laser beams (e.g. red vs green).
	// Data starts at byte offset 18 in the IACT chunk (in_stack_00000014 + 9 shorts).
	// The stream is positioned after par1..par4 (8 bytes), so FUN_00405663's +18 is startPos + 10.
	if (par4 == 1000 && remaining >= 10 + 8 + 32896) {
		byte *edgeData = (byte *)malloc(8 + 32896);
		if (edgeData) {
			b.seek(startPos + 10);
			b.read(edgeData, 8 + 32896);
			initEdgeTable(edgeData);
			free(edgeData);
			debugC(DEBUG_INSANE, "Opcode 8: Loaded per-level edge blend table (par4=1000)");
		}
		b.seek(startPos);
		return true;
	}

	return false;
}

// Helper split out of opcode 8 aux SFX loading; not a separate original function.
bool InsaneRebel2::loadOpcode8AuxSfx(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4) {
	// Auxiliary Sound Buffer Loading (par4 20-47)
	// FUN_401234 case 6 (handler 8): par4 0x14-0x1b (20-27) -> aux buffer 0
	// FUN_41CADB case 6 (handler 25): par4 0x15-0x1b (21-27) -> aux buffer 0,
	//   0x1f-0x25 (31-37) -> aux buffer 1, 0x28 (40) -> aux buffer 3,
	//   0x29-0x2f (41-47) -> aux buffer 2
	// Data layout: offset 14 = uint32 data size, offset 18 = PCM data start.
	// Stream is at offset 8 (after par1-par4), so data size at +6, PCM at +10.
	if (par4 < 20 || par4 > 47)
		return false;

	int auxBuffer = -1;
	if (par4 >= 20 && par4 <= 27) {
		auxBuffer = 0;
	} else if (par4 >= 31 && par4 <= 37) {
		auxBuffer = 1;
	} else if (par4 == 40) {
		auxBuffer = 3;
	} else if (par4 >= 41 && par4 <= 47) {
		auxBuffer = 2;
	}

	if (auxBuffer >= 0 && remaining >= 10) {
		b.seek(startPos + 6); // Skip to data size field (byte offset 14 from IACT start)
		uint32 dataSize = b.readUint32LE();
		if (dataSize > 0 && remaining >= (int64)(10 + dataSize)) {
			byte *soundData = (byte *)malloc(dataSize);
			if (soundData) {
				b.read(soundData, dataSize);
				loadAuxSfx(auxBuffer, soundData, dataSize);
				free(soundData);
				debugC(DEBUG_INSANE, "Opcode 8: Loaded %u bytes into aux sound buffer %d (par4=%d)",
					dataSize, auxBuffer, par4);
			}
		} else {
			debugC(DEBUG_INSANE, "Opcode 8: Aux sound par4=%d dataSize=%u exceeds remaining=%lld",
				par4, dataSize, (long long)remaining);
		}
	}
	b.seek(startPos);
	return true;
}

// Helper split out of FUN_0041CADB case 6 shot-origin loading; not a separate original function.
bool InsaneRebel2::loadOpcode8ShotOriginTable(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4) {
	// Handler 25 (0x19): Shot-Origin Lookup Table (par4 == 8)
	// FUN_0041CADB case 6 pushes 30 short pointers into sscanf with format at 0x482360:
	//   "%hd %hd  %hd %hd ... %hd %hd" (15 X/Y pairs).
	// Parsed values are written into DAT_004578a6 / DAT_004578c6 at indices 5..19.
	if (_rebelHandler == 25 && par4 == 8) {
		if (loadHandler25ShotOriginTable(b, startPos, remaining)) {
			b.seek(startPos);
			return true;
		}
	}

	return false;
}

// Helper split out of opcode 8 embedded ANIM scanning; not a separate original function.
void InsaneRebel2::loadOpcode8EmbeddedAnim(byte *renderBitmap, Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par3, int16 par4) {
	// Remaining handlers require finding ANIM tag in the stream.
	debugC(DEBUG_INSANE, "Opcode 8: Scanning for ANIM tag (startPos=%lld remaining=%lld)",
		(long long)startPos, (long long)remaining);

	if (remaining <= 0) {
		return;
	}

	int scanSize = (int)MIN<int64>(remaining, 65536);
	byte *scanBuf = (byte *)malloc(scanSize);
	if (!scanBuf) {
		return;
	}

	int bytesRead = b.read(scanBuf, scanSize);
	debugC(DEBUG_INSANE, "Opcode 8: Read %d bytes for ANIM scan", bytesRead);

	int animOffset = -1;
	for (int i = 0; i + 8 <= bytesRead; ++i) {
		if (READ_BE_UINT32(scanBuf + i) == MKTAG('A','N','I','M')) {
			animOffset = i;
			debugC(DEBUG_INSANE, "Opcode 8: Found ANIM at offset %d", i);
			break;
		}
	}

	if (animOffset < 0) {
		debugC(DEBUG_INSANE, "Opcode 8: No ANIM tag found");
		free(scanBuf);
		b.seek(startPos);
		return;
	}

	uint32 animReportedSize = READ_BE_UINT32(scanBuf + animOffset + 4);
	int32 animDataSize = (int)MIN<int64>((int64)animReportedSize + 8, remaining - animOffset);
	if (animDataSize <= 0) {
		free(scanBuf);
		b.seek(startPos);
		return;
	}

	byte *animData = (byte *)malloc(animDataSize);
	if (!animData) {
		free(scanBuf);
		b.seek(startPos);
		return;
	}

	b.seek(startPos + animOffset);
	b.read(animData, animDataSize);
	handleOpcode8EmbeddedAnim(renderBitmap, animData, animDataSize, par3, par4);

	free(animData);
	free(scanBuf);
	b.seek(startPos);
}

// Helper split out of opcode 8 embedded ANIM routing; not a separate original function.
bool InsaneRebel2::handleOpcode8EmbeddedAnim(byte *renderBitmap, byte *animData, int32 animDataSize, int16 par3, int16 par4) {
	bool handled = false;

	// Handler 0x26: Turret HUD Overlays.
	// FUN_00407fcb case 8: handler 0x26 uses par4 1-4 for HUD NUT loading.
	// Some chunks use par3 for the same low/high selector.
	if (!handled && _rebelHandler == 0x26) {
		int hudSelector = (par4 >= 1 && par4 <= 4) ? par4 : par3;

		if (hudSelector >= 1 && hudSelector <= 4) {
			handled = loadTurretHudOverlay(animData, animDataSize, hudSelector);
		}
	}

	// Handler 8: POV Ship Sprites or Background.
	// FUN_00401234 case 6: par4 selects POV NUT type (1,3,6,7) or background (5).
	// NOTE: par3 is always 0 for Handler 8; par4 contains the actual sprite type.
	if (!handled && _rebelHandler == 8) {
		if (par4 == 5) {
			handled = loadLevel2Background(animData, animDataSize, renderBitmap);
		} else if (par4 == 1 || par4 == 3 || par4 == 6 || par4 == 7) {
			handled = loadHandler8ShipSprites(animData, animDataSize, par4);
		}
	}

	// Handler 25 (0x19): Level 2 GRD Ship Sprites and Background.
	// FUN_0041cadb case 6 (opcode 8): Uses PAR4 for switch selection.
	//   par4=1: GRD001 - Primary ship sprite -> DAT_00482240 / _grd001Sprite
	//   par4=2: GRD002 - Secondary ship sprite -> DAT_00482238 / _grd002Sprite
	//   par4=4: 350x230 corridor overlay -> DAT_00482268, draws immediately
	//   par4=5: 320x200 background -> DAT_0048226c
	//   par4=6: Overlay -> DAT_00482250, draws immediately
	//   par4=7: Overlay -> DAT_00482248, draws immediately
	//   par4=10: GRD005 - Mode 3 overlay -> DAT_00482258 / _grd005Sprite
	if (!handled && _rebelHandler == 25) {
		if (par4 == 1 || par4 == 2 || par4 == 10) {
			handled = loadHandler25GrdSprites(animData, animDataSize, par4);
		} else if (par4 == 5) {
			handled = loadLevel2Background(animData, animDataSize, renderBitmap);
		} else if (par4 == 4 || par4 == 6 || par4 == 7) {
			debugC(DEBUG_INSANE, "Opcode 8: Handler 25 overlay par4=%d - drawing to screen", par4);
			loadEmbeddedSan(par4, animData, animDataSize, renderBitmap);
			handled = true;
		}
	}

	// Fallback: Embedded SAN HUD overlays.
	// For other cases, load as embedded SAN frame to HUD overlay slots.
	if (!handled) {
		const bool highRes = isHiRes();
		const bool highResHud = (par3 == 2 || par3 == 4);
		const bool lowResHud = (par3 == 1 || par3 == 3);

		if ((!highRes && highResHud) || (highRes && lowResHud)) {
			debugC(DEBUG_INSANE, "Opcode 8: Skipping %s HUD par3=%d while running in %s mode",
				highResHud ? "high-res" : "low-res", par3, highRes ? "high-res" : "low-res");
			handled = true;
		} else {
			// Determine userId: Handler 0x19 uses par3, others use par4.
			// Heuristic: if par3 is valid GRD range (1-13) and par4 is invalid, prefer par3.
			bool usePar3 = (_rebelHandler == 0x19);
			if (!usePar3 && par3 >= 1 && par3 <= 13 && (par4 <= 0 || par4 >= 1000)) {
				usePar3 = true;
			}
			int userId = usePar3 ? par3 : par4;

			// Skip audio tracks (userId >= 1000).
			if (userId > 0 && userId < 1000) {
				debugC(DEBUG_INSANE, "Opcode 8: Loading embedded SAN HUD userId=%d (handler=%d par3=%d par4=%d)",
					userId, _rebelHandler, par3, par4);
				loadEmbeddedSan(userId, animData, animDataSize, renderBitmap);
				handled = true;
			}
		}
	}

	if (!handled) {
		debugC(DEBUG_INSANE, "Opcode 8: Unhandled case - handler=%d par3=%d par4=%d", _rebelHandler, par3, par4);
	}

	return handled;
}

void InsaneRebel2::iactRebel2Opcode8(byte *renderBitmap, Common::SeekableReadStream &b, int32 chunkSize, int16 par2, int16 par3, int16 par4) {
	debugC(DEBUG_INSANE, "IACT Opcode 8: handler=%d par2=%d par3=%d par4=%d (gameState=%d)",
		_rebelHandler, par2, par3, par4, _gameState);

	int64 startPos = b.pos();
	int64 remaining = (chunkSize > 0) ? chunkSize : (b.size() - startPos);

	if (loadOpcode8Handler7FlySprites(b, startPos, remaining, par4))
		return;

	if (loadOpcode8Handler7ShotTable(b, startPos, remaining, par4))
		return;

	if (loadOpcode8EdgeTable(b, startPos, remaining, par4))
		return;

	if (loadOpcode8AuxSfx(b, startPos, remaining, par4))
		return;

	if (loadOpcode8ShotOriginTable(b, startPos, remaining, par4))
		return;

	loadOpcode8EmbeddedAnim(renderBitmap, b, startPos, remaining, par3, par4);
}

// loadHandler25ShotOriginTable -- Parse shot origin coordinate pairs from IACT payload.
bool InsaneRebel2::loadHandler25ShotOriginTable(Common::SeekableReadStream &b, int64 startPos, int64 remaining) {
	// IACT layout at this point:
	// - stream is positioned after par1..par4 (8 bytes consumed by caller)
	// - FUN_0041CADB reads from offset +18 relative to IACT start -> startPos + 10
	// - payload size for this opcode family is at offset +14 -> startPos + 6
	if (remaining < 12)
		return false;

	int64 savedPos = b.pos();
	b.seek(startPos + 6);
	uint32 textSize = b.readUint32LE();

	int64 textPos = startPos + 10;
	int64 maxAvail = remaining - 10;
	if (maxAvail <= 0) {
		b.seek(savedPos);
		return false;
	}

	int64 bytesToRead = maxAvail;
	if (textSize > 0 && (int64)textSize <= maxAvail) {
		bytesToRead = textSize;
	}

	char *buf = new char[(size_t)bytesToRead + 1];

	b.seek(textPos);
	b.read((byte *)buf, bytesToRead);
	buf[bytesToRead] = '\0';

	// Parse signed 16-bit integers from the ASCII payload.
	int16 vals[30];
	int count = 0;
	const char *p = buf;
	const char *end = buf + bytesToRead;
	while (p < end && count < 30) {
		while (p < end && *p != '-' && *p != '+' && !Common::isDigit(*p))
			++p;
		if (p >= end)
			break;

		int sign = 1;
		if (*p == '-' || *p == '+') {
			if (*p == '-')
				sign = -1;
			++p;
		}

		if (p >= end || !Common::isDigit(*p))
			continue;

		int value = 0;
		while (p < end && Common::isDigit(*p)) {
			value = value * 10 + (*p - '0');
			if (value > 32768)
				value = 32768; // Keep accumulation bounded before sign/clamp.
			++p;
		}

		vals[count++] = (int16)CLIP<int>(sign * value, -32768, 32767);
	}

	delete[] buf;
	b.seek(savedPos);

	if (count < 20) {
		debugC(DEBUG_INSANE, "Opcode 8: Handler25 par4=8 parse failed (count=%d, expected up to 30)", count);
		return false;
	}

	// FUN_0041CADB mapping:
	// token1->0x4578b0 (X index 5), token2->0x4578d0 (Y index 5), ...
	// token29->0x4578cc (X index 19), token30->0x4578ec (Y index 19).
	for (int i = 0; i < 15; ++i) {
		int pair = i * 2;
		if (pair + 1 >= count)
			break;
		_grdShotOriginX[5 + i] = vals[pair];
		_grdShotOriginY[5 + i] = vals[pair + 1];
	}
	_grdShotOriginTableLoaded = true;

	debugC(DEBUG_INSANE, "Opcode 8: Loaded Handler25 shot-origin table (pairs=%d) idx5=(%d,%d) idx14=(%d,%d)",
		count / 2, _grdShotOriginX[5], _grdShotOriginY[5], _grdShotOriginX[14], _grdShotOriginY[14]);
	return true;
}

// loadHandler7ShotTable -- Parse handler 7 laser muzzle coordinate pairs from IACT payload.
bool InsaneRebel2::loadHandler7ShotTable(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4) {
	// FUN_0040FCFA parses 35 "%hd %hd" pairs from offset +18 in the IACT
	// chunk into two parallel 35-entry tables. These tables are BSS globals in
	// the EXE, so their values only exist in the SAN/IACT stream.
	if (remaining < 12)
		return false;

	int64 savedPos = b.pos();
	b.seek(startPos + 6);
	uint32 textSize = b.readUint32LE();

	int64 textPos = startPos + 10;
	int64 maxAvail = remaining - 10;
	if (maxAvail <= 0) {
		b.seek(savedPos);
		return false;
	}

	int64 bytesToRead = maxAvail;
	if (textSize > 0 && (int64)textSize <= maxAvail)
		bytesToRead = textSize;

	char *buf = new char[(size_t)bytesToRead + 1];
	b.seek(textPos);
	b.read((byte *)buf, bytesToRead);
	buf[bytesToRead] = '\0';

	int16 vals[70];
	int count = 0;
	const char *p = buf;
	const char *end = buf + bytesToRead;
	while (p < end && count < 70) {
		while (p < end && *p != '-' && *p != '+' && !Common::isDigit(*p))
			++p;
		if (p >= end)
			break;

		int sign = 1;
		if (*p == '-' || *p == '+') {
			if (*p == '-')
				sign = -1;
			++p;
		}

		if (p >= end || !Common::isDigit(*p))
			continue;

		int value = 0;
		while (p < end && Common::isDigit(*p)) {
			value = value * 10 + (*p - '0');
			if (value > 32768)
				value = 32768;
			++p;
		}

		vals[count++] = (int16)CLIP<int>(sign * value, -32768, 32767);
	}

	delete[] buf;
	b.seek(savedPos);

	if (count < 70) {
		debugC(DEBUG_INSANE, "Opcode 8: Handler7 par4=%d shot table parse failed (count=%d)", par4, count);
		return false;
	}

	int16 *tableX = (par4 == 12) ? _flyLeftGunX : _flyRightGunX;
	int16 *tableY = (par4 == 12) ? _flyLeftGunY : _flyRightGunY;
	for (int i = 0; i < 35; ++i) {
		tableX[i] = vals[i * 2];
		tableY[i] = vals[i * 2 + 1];
	}

	if (par4 == 12)
		_flyLeftGunTableLoaded = true;
	else
		_flyRightGunTableLoaded = true;

	debugC(DEBUG_INSANE, "Opcode 8: Loaded Handler7 %s gun table idx0=(%d,%d) idx17=(%d,%d) idx34=(%d,%d)",
		(par4 == 12) ? "left" : "right",
		tableX[0], tableY[0], tableX[17], tableY[17], tableX[34], tableY[34]);
	return true;
}

// ---------------------------------------------------------------------------
// Opcode 8 Helper Functions
// ---------------------------------------------------------------------------
// Extracted from the original monolithic iactRebel2Opcode8 to match
// the original FUN_* function structure.

// loadHandler7FlySprites -- Handler 7 FLY NUT loading (FUN_0040c3cc case 6).
bool InsaneRebel2::loadHandler7FlySprites(Common::SeekableReadStream &b, int64 remaining, int16 par4) {
	// Handler 7 FLY NUT loading - FUN_0040c3cc case 6 (opcode 8)
	// IACT structure after par1-par4 (we're at offset +8):
	//   +0-5 (6 bytes): additional header
	//   +6-9 (4 bytes): NUT data size (little-endian)
	//   +10+: NUT data
	//
	// par4 values (param_5[3] - 1 in assembly):
	//   1 -> case 0: FLY001 - Ship direction sprites (DAT_0047fee8)
	//   2 -> case 1: FLY003 - Targeting overlay (DAT_0047fef8)
	//   3 -> case 2: FLY002 - Laser fire sprites (DAT_0047fef0)
	//  11 -> case 10: FLY004 - High-res alternative (DAT_0047ff00)

	if (remaining < 14) {
		return false;
	}

	// Read additional header and size from fixed offset
	byte header[10];
	if (b.read(header, 10) != 10) {
		return false;
	}

	debugC(DEBUG_INSANE, "loadHandler7FlySprites: header bytes: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
		header[0], header[1], header[2], header[3], header[4],
		header[5], header[6], header[7], header[8], header[9]);

	// Size is at offset 14 from IACT start = bytes 6-9 of our header buffer
	uint32 nutSize = READ_LE_UINT32(header + 6);
	debugC(DEBUG_INSANE, "loadHandler7FlySprites: par4=%d nutSize=%u remaining=%lld",
		par4, nutSize, (long long)remaining);

	if (nutSize == 0 || nutSize > (uint32)(remaining - 10)) {
		return false;
	}

	byte *nutData = (byte *)malloc(nutSize);
	if (!nutData) {
		return false;
	}

	int bytesRead = b.read(nutData, nutSize);
	debugC(DEBUG_INSANE, "loadHandler7FlySprites: Read %d/%u bytes, first 16: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
		bytesRead, nutSize,
		bytesRead > 0 ? nutData[0] : 0, bytesRead > 1 ? nutData[1] : 0,
		bytesRead > 2 ? nutData[2] : 0, bytesRead > 3 ? nutData[3] : 0,
		bytesRead > 4 ? nutData[4] : 0, bytesRead > 5 ? nutData[5] : 0,
		bytesRead > 6 ? nutData[6] : 0, bytesRead > 7 ? nutData[7] : 0,
		bytesRead > 8 ? nutData[8] : 0, bytesRead > 9 ? nutData[9] : 0,
		bytesRead > 10 ? nutData[10] : 0, bytesRead > 11 ? nutData[11] : 0,
		bytesRead > 12 ? nutData[12] : 0, bytesRead > 13 ? nutData[13] : 0,
		bytesRead > 14 ? nutData[14] : 0, bytesRead > 15 ? nutData[15] : 0);

	if (bytesRead != (int)nutSize) {
		warning("Rebel2 loadHandler7FlySprites: Short read! Got %d expected %u", bytesRead, nutSize);
		free(nutData);
		return false;
	}

	// Verify ANIM header
	if (bytesRead >= 8) {
		uint32 animTag = READ_BE_UINT32(nutData);
		if (animTag != MKTAG('A','N','I','M')) {
			warning("Rebel2 loadHandler7FlySprites: No ANIM tag! Data may be corrupted");
			free(nutData);
			return false;
		}
	}

	// Load as a Rebel2 embedded sprite ANIM.
	NutRenderer *newNut = makeRebel2SpriteFromData(_vm, nutData, bytesRead);
	if (!newNut || newNut->getNumChars() <= 0) {
		debugC(DEBUG_INSANE, "loadHandler7FlySprites: NUT load failed for par4=%d", par4);
		delete newNut;
		free(nutData);
		return false;
	}

	debugC(DEBUG_INSANE, "loadHandler7FlySprites: Loaded FLY NUT par4=%d with %d sprites",
		par4, newNut->getNumChars());

	// Assign to appropriate slot based on par4 (matches FUN_0040c3cc case 6 switch)
	bool assigned = true;
	switch (par4) {
	case 1:  // FLY001 - Ship direction sprites (35 frames)
		delete _flyShipSprite;
		_flyShipSprite = newNut;
		debugC(DEBUG_INSANE, "_flyShipSprite set with %d sprites", newNut->getNumChars());
		break;
	case 2:  // FLY003 - Targeting overlay
		delete _flyTargetSprite;
		_flyTargetSprite = newNut;
		break;
	case 3:  // FLY002 - Laser fire sprites
		delete _flyLaserSprite;
		_flyLaserSprite = newNut;
		break;
	case 11: // FLY004 - High-res alternative
		delete _flyHiResSprite;
		_flyHiResSprite = newNut;
		break;
	default:
		delete newNut;
		assigned = false;
		break;
	}

	free(nutData);
	return assigned;
}

// loadTurretHudOverlay -- Handler 0x26 turret HUD loading (FUN_00407fcb case 8).
bool InsaneRebel2::loadTurretHudOverlay(byte *animData, int32 size, int16 selector) {
	// Handler 0x26 turret HUD overlay loading - FUN_00407fcb case 8
	// Resolution-dependent loading:
	//   selector == 1: Low-res primary HUD (DAT_0047fe78 / _hudOverlayNut)
	//   selector == 2: High-res primary HUD (DAT_0047fe78 / _hudOverlayNut)
	//   selector == 3: Low-res secondary HUD (DAT_0047fe80 / _hudOverlay2Nut)
	//   selector == 4: High-res secondary HUD (DAT_0047fe80 / _hudOverlay2Nut)

	if (!animData || size <= 0) {
		return false;
	}

	const bool highRes = isHiRes();
	const int primarySlot = highRes ? 2 : 1;
	const int secondarySlot = highRes ? 4 : 3;

	if (selector >= 1 && selector <= 4 && selector != primarySlot && selector != secondarySlot) {
		debugC(DEBUG_INSANE, "loadTurretHudOverlay: Skipping %s HUD selector=%d (running in %s mode)",
			(selector == 2 || selector == 4) ? "high-res" : "low-res", selector,
			highRes ? "high-res" : "low-res");
		return true;
	}

	if (selector != primarySlot && selector != secondarySlot) {
		return false;  // Not a turret HUD slot
	}

	NutRenderer *newNut = makeRebel2SpriteFromData(_vm, animData, size);
	if (!newNut || newNut->getNumChars() <= 0) {
		debugC(DEBUG_INSANE, "loadTurretHudOverlay: NUT load failed for selector=%d", selector);
		delete newNut;
		return false;
	}

	debugC(DEBUG_INSANE, "loadTurretHudOverlay: Loaded turret HUD NUT selector=%d with %d sprites",
		selector, newNut->getNumChars());

	if (selector == primarySlot) {
		delete _hudOverlayNut;
		_hudOverlayNut = newNut;
	} else {
		delete _hudOverlay2Nut;
		_hudOverlay2Nut = newNut;
	}

	return true;
}

// loadHandler8ShipSprites -- Handler 8 POV NUT loading (FUN_00401234 case 6).
bool InsaneRebel2::loadHandler8ShipSprites(byte *animData, int32 size, int16 par4) {
	// Handler 8 ship POV NUT loading - FUN_00401234 case 6 (opcode 8)
	// par4 values (from IACT data offset +6, NOT par3 which is always 0):
	//   1: POV001 - Primary ship sprite (DAT_0047e010 / _shipSprite)
	//   3: POV004 - Secondary ship sprite (DAT_0047e028 / _shipSprite2)
	//   6: POV002 - Shot impact overlay (DAT_0047e020 / _shipOverlay1)
	//   7: POV003 - Shot impact overlay (DAT_0047e018 / _shipOverlay2)

	if (!animData || size <= 0) {
		return false;
	}

	// Only handle valid POV sprite slots
	if (par4 != 1 && par4 != 3 && par4 != 6 && par4 != 7) {
		return false;
	}

	NutRenderer *newNut = makeRebel2SpriteFromData(_vm, animData, size);
	if (!newNut || newNut->getNumChars() <= 0) {
		debugC(DEBUG_INSANE, "loadHandler8ShipSprites: NUT load failed for par4=%d", par4);
		delete newNut;
		return false;
	}

	debugC(DEBUG_INSANE, "loadHandler8ShipSprites: Loaded ship NUT par4=%d with %d sprites",
		par4, newNut->getNumChars());

	switch (par4) {
	case 1:  // POV001 - Primary ship sprite
		delete _shipSprite;
		_shipSprite = newNut;
		break;
	case 3:  // POV004 - Secondary ship sprite
		delete _shipSprite2;
		_shipSprite2 = newNut;
		break;
	case 6:  // POV002 - Shot impact overlay
		delete _shipOverlay1;
		_shipOverlay1 = newNut;
		break;
	case 7:  // POV003 - Shot impact overlay
		delete _shipOverlay2;
		_shipOverlay2 = newNut;
		break;
	default:
		delete newNut;
		return false;
	}

	return true;
}

// loadHandler25GrdSprites -- Handler 25 GRD NUT loading (FUN_0041cadb case 6).
bool InsaneRebel2::loadHandler25GrdSprites(byte *animData, int32 size, int16 par4) {
	// Handler 25 GRD ship NUT loading - FUN_0041cadb case 6 (opcode 8)
	// par4 values (from IACT data offset +6):
	//   1: GRD001 - Primary ship sprite (DAT_00482240 / _grd001Sprite)
	//   2: GRD002 - Secondary ship sprite (DAT_00482238 / _grd002Sprite)
	//   10: GRD005 - Mode 3 overlay sprite (DAT_00482258 / _grd005Sprite)

	if (!animData || size <= 0) {
		return false;
	}

	// Only handle valid GRD sprite slots
	if (par4 != 1 && par4 != 2 && par4 != 10) {
		return false;
	}

	NutRenderer *newNut = makeRebel2SpriteFromData(_vm, animData, size);
	if (!newNut || newNut->getNumChars() <= 0) {
		debugC(DEBUG_INSANE, "loadHandler25GrdSprites: NUT load failed for par4=%d", par4);
		delete newNut;
		return false;
	}

	debugC(DEBUG_INSANE, "loadHandler25GrdSprites: Loaded GRD NUT par4=%d with %d sprites",
		par4, newNut->getNumChars());

	switch (par4) {
	case 1:  // GRD001 - Primary ship sprite
		delete _grd001Sprite;
		_grd001Sprite = newNut;
		debugC(DEBUG_INSANE, "_grd001Sprite set with %d sprites", newNut->getNumChars());
		break;
	case 2:  // GRD002 - Secondary ship sprite
		delete _grd002Sprite;
		_grd002Sprite = newNut;
		debugC(DEBUG_INSANE, "_grd002Sprite set with %d sprites", newNut->getNumChars());
		break;
	case 10: // GRD005 - Mode 3 overlay sprite
		delete _grd005Sprite;
		_grd005Sprite = newNut;
		debugC(DEBUG_INSANE, "_grd005Sprite set with %d sprites", newNut->getNumChars());
		break;
	default:
		delete newNut;
		return false;
	}

	return true;
}

// loadLevel2Background -- Decode Level 2 background from embedded ANIM (FUN_00401234 case 5).
bool InsaneRebel2::loadLevel2Background(byte *animData, int32 size, byte *renderBitmap) {
	// Level 2 background loading from embedded ANIM - FUN_00401234 case 5
	// par4=5 contains the background image embedded as ANIM with FOBJ codec 3
	// Creates 320x200 buffer (DAT_0047e030 / _level2Background)

	if (!animData || size < 8) {
		return false;
	}

	debugC(DEBUG_INSANE, "loadLevel2Background: Loading Level 2 background (animSize=%d)", size);

	// Allocate background buffer if needed (320x200 = 64000 bytes)
	if (_level2Background == nullptr) {
		_level2Background = (byte *)malloc(320 * 200);
		if (!_level2Background) {
			return false;
		}
		memset(_level2Background, 0, 320 * 200);
	}

	Common::MemoryReadStream stream(animData, size);
	const int64 streamEnd = stream.size();

	uint32 animTag = stream.readUint32BE();
	if (animTag != MKTAG('A','N','I','M')) {
		debugC(DEBUG_INSANE, "loadLevel2Background: Missing ANIM tag, got 0x%08X", animTag);
		return false;
	}

	uint32 animSize = stream.readUint32BE();
	int64 animEnd = streamEnd;
	if ((int64)animSize <= streamEnd - 8) {
		animEnd = 8 + (int64)animSize;
	} else {
		debugC(DEBUG_INSANE, "loadLevel2Background: ANIM truncated: reported size=%u, actual=%lld",
			animSize, streamEnd - 8);
	}
	debugC(DEBUG_INSANE, "loadLevel2Background: Found ANIM tag, size=%u", animSize);

	bool foundBackground = false;
	while (!stream.eos() && stream.pos() + 8 <= animEnd && !foundBackground) {
		uint32 tag;
		uint32 chunkSize;
		int64 chunkDataEnd;
		int64 nextChunkPos;
		if (!readLevel2BackgroundChunkHeader(stream, animEnd, "ANIM", tag, chunkSize, chunkDataEnd, nextChunkPos))
			break;

		if (tag != MKTAG('F','R','M','E')) {
			stream.seek(nextChunkPos);
			continue;
		}

		debugC(DEBUG_INSANE, "loadLevel2Background: Found FRME at %lld, size=%u", stream.pos() - 8, chunkSize);

		while (stream.pos() + 8 <= chunkDataEnd && !stream.eos() && !foundBackground) {
			uint32 subTag;
			uint32 subSize;
			int64 subDataEnd;
			int64 nextSubPos;
			if (!readLevel2BackgroundChunkHeader(stream, chunkDataEnd, "FRME", subTag, subSize, subDataEnd, nextSubPos))
				break;

			if (subTag != MKTAG('F','O','B','J')) {
				stream.seek(nextSubPos);
				continue;
			}

			if (subSize < 14) {
				debugC(DEBUG_INSANE, "loadLevel2Background: FOBJ too small: size=%u", subSize);
				stream.seek(nextSubPos);
				continue;
			}

			// FOBJ header: codec(2), x(2), y(2), w(2), h(2)
			int codec = stream.readUint16LE();
			int fobjX = stream.readSint16LE();
			int fobjY = stream.readSint16LE();
			int fobjW = stream.readSint16LE();
			int fobjH = stream.readSint16LE();
			stream.readUint16LE();  // unknown
			stream.readUint16LE();  // unknown

			debugC(DEBUG_INSANE, "loadLevel2Background: Found FOBJ: codec=%d pos=(%d,%d) size=%dx%d",
				codec, fobjX, fobjY, fobjW, fobjH);

			// Decode codec 3 (RLE) into the original 320x200 background buffer.
			// FUN_0041CADB/FUN_00401234 draw these resources into a fixed buffer
			// and clip them there; Level 11 backgrounds extend past the right edge.
			if (codec == 3 && fobjX >= 0 && fobjY >= 0 && fobjW > 0 && fobjH > 0 &&
					fobjX < 320 && fobjY < 200 && stream.pos() < subDataEnd) {
				int drawW = MIN<int>(fobjW, 320 - fobjX);
				int drawH = MIN<int>(fobjH, 200 - fobjY);
				const byte *rleData = animData + stream.pos();
				smushDecodeRLEOpaque(_level2Background, rleData, fobjX, fobjY, drawW, drawH, 320,
					(int)(subDataEnd - stream.pos()));

				debugC(DEBUG_INSANE, "loadLevel2Background: Decoded Level 2 background (%dx%d at %d,%d, clipped to %dx%d)",
					fobjW, fobjH, fobjX, fobjY, drawW, drawH);
				_level2BackgroundLoaded = true;
				foundBackground = true;

				// Handler 25 uses this buffer as a lookup mask; FUN_0041CADB does not
				// copy it to the live screen. Handler 8 still uses it as a restore source.
				if (renderBitmap && _rebelHandler != 25) {
					int bufferPitch = (_player && _player->_width > 0) ? _player->_width : 320;
					if (bufferPitch == 320) {
						for (int by = 0; by < 200; by++) {
							memcpy(renderBitmap + by * 320, _level2Background + by * 320, 320);
						}
						debugC(DEBUG_INSANE, "loadLevel2Background: Copied to renderBitmap (pitch=%d)", bufferPitch);
					} else {
						debugC(DEBUG_INSANE, "loadLevel2Background: Skipping renderBitmap copy (pitch=%d != 320)", bufferPitch);
					}
				}
			}

			stream.seek(nextSubPos);
		}

		stream.seek(nextChunkPos);
	}

	if (!foundBackground) {
		debugC(DEBUG_INSANE, "loadLevel2Background: Failed to find/decode background FOBJ");
	}

	return foundBackground;
}

//
// iactRebel2Opcode9 -- Text/subtitle display via IACT chunk
//
// Handles inline text in IACT chunks. Most RA2 subtitles use TRES chunks
// (handled by SmushPlayer::handleTextResource); this opcode is less common.
// Supports multi-line wrapping, centered/shadowed text, and clip regions.
//
void InsaneRebel2::iactRebel2Opcode9(byte *renderBitmap, Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4) {
	// Opcode 9: Text/Subtitle Display via IACT chunk
	// Note: Most RA2 subtitles use TRES chunks handled by SmushPlayer::handleTextResource()
	// This opcode handles inline text in IACT chunks (less common)
	//
	// IACT Chunk Layout (par1-par4 already read by handleIACT):
	// +0x00 (2): opcode = 9 (par1, already read)
	// +0x02 (2): par2 (already read)
	// +0x04 (2): par3 (already read)
	// +0x06 (2): par4 (already read)
	// +0x08 onwards: Text data structure
	//
	// Text Data Structure:
	// +0x00 (2): X position
	// +0x02 (2): Y position
	// +0x04 (2): flags (bit 0=center, bit 1=right, bit 2=wrap, bit 3=difficulty gated)
	// +0x06 (2): clipX (when flag & 4)
	// +0x08 (2): clipY
	// +0x0A (2): clipW
	// +0x0C (2): clipH
	// +0x10 onwards: NUL-terminated text string

	int64 startPos = b.pos();

	// Check for "TRES" tag (0x54524553) indicating string resource lookup
	uint32 tag = b.readUint32BE();

	const char *textStr = nullptr;
	char textBuffer[512];
	int16 posX = 160;  // Default center position
	int16 posY = 150;  // Default bottom-ish position
	int16 textFlags = 1;  // Default: center aligned
	int16 clipX = 16, clipY = 16, clipW = 288, clipH = 168;

	if (tag == MKTAG('T','R','E','S')) {
		// String resource lookup via TRES tag
		// The string index follows after the tag
		int32 stringIndex = b.readSint32LE();

		// Try to get string from SMUSH player's string resource
		if (_player && _player->getString(stringIndex)) {
			textStr = _player->getString(stringIndex);
			debugC(DEBUG_INSANE, "Opcode 9: TRES string index=%d -> \"%s\"", stringIndex, textStr);
		} else {
			debugC(DEBUG_INSANE, "Opcode 9: TRES string index=%d not found", stringIndex);
			return;
		}

		// After TRES + index, read positioning data
		// The remaining data contains X, Y, flags etc.
		if (b.size() - b.pos() >= 14) {
			posX = b.readSint16LE();
			posY = b.readSint16LE();
			textFlags = b.readSint16LE();
			clipX = b.readSint16LE();
			clipY = b.readSint16LE();
			clipW = b.readSint16LE();
			clipH = b.readSint16LE();
		}
	} else {
		// Inline text data - go back and read positioning structure
		b.seek(startPos);

		// Read text data structure
		posX = b.readSint16LE();      // +0x00
		posY = b.readSint16LE();      // +0x02
		textFlags = b.readSint16LE(); // +0x04
		clipX = b.readSint16LE();     // +0x06
		clipY = b.readSint16LE();     // +0x08
		clipW = b.readSint16LE();     // +0x0A
		clipH = b.readSint16LE();     // +0x0C
		b.skip(2);                    // +0x0E padding

		// Read inline text string (NUL-terminated)
		int textLen = 0;
		while (textLen < (int)sizeof(textBuffer) - 1) {
			byte ch = b.readByte();
			if (ch == 0 || b.eos())
				break;
			textBuffer[textLen++] = ch;
		}
		textBuffer[textLen] = '\0';
		textStr = textBuffer;

		debugC(DEBUG_INSANE, "Opcode 9: Inline text at (%d,%d) flags=0x%x -> \"%s\"", posX, posY, textFlags, textStr);
	}

	if (!textStr || textStr[0] == '\0') {
		debugC(DEBUG_INSANE, "Opcode 9: Empty text string, skipping");
		return;
	}

	// Check difficulty gate (flag bit 3 = 0x08)
	// If set, only show text if difficulty check passes (we skip this check for simplicity)
	// FUN_00425D30(0) is called.

	// Get render buffer dimensions
	int width = (_player && _player->_width > 0) ? _player->_width : 320;
	int height = (_player && _player->_height > 0) ? _player->_height : 200;

	// Apply coordinate clamping (from FUN_004033cf disassembly)
	// Low-res: X clamped to [16, 304], Y clamped to [16, 196]
	if (posX < 16)
		posX = 16;
	if (posX > 304)
		posX = 304;
	if (posY < 16)
		posY = 16;
	if (posY > 196)
		posY = 196;

	// Use the message font loaded during initialization (DIHIFONT.NUT)
	if (!_rebelMsgFont) {
		debugC(DEBUG_INSANE, "Opcode 9: No message font loaded (_rebelMsgFont is null)");
		return;
	}

	// Calculate clipping rectangle
	if (!(textFlags & 0x04)) {
		// No clip rect specified, use default full-screen clip
		clipX = 0;
		clipY = 0;
		clipW = width;
		clipH = height;
	}

	Common::Rect clipRect(
		MAX<int>(0, clipX),
		MAX<int>(0, clipY),
		MIN<int>(clipX + clipW, width),
		MIN<int>(clipY + clipH, height)
	);

	// Determine text alignment flags
	TextStyleFlags styleFlags = kStyleAlignLeft;
	if (textFlags & 0x01) {
		styleFlags = kStyleAlignCenter;
	} else if (textFlags & 0x02) {
		styleFlags = kStyleAlignRight;
	}
	if (textFlags & 0x04) {
		styleFlags = (TextStyleFlags)(styleFlags | kStyleWordWrap);
	}

	// Use white color (index 255) for subtitle text
	// The original uses colors from the palette, commonly white or yellow for subtitles
	int16 textColor = 255;

	// RA2 fonts (like DIHIFONT.NUT) have only 58 characters starting at ASCII 32 (space).
	// We need to convert ASCII codes to font indices by subtracting 32.
	// Character mapping: font index = ASCII code - 32
	// So 'D' (68) becomes index 36, 'A' (65) becomes index 33, etc.
	// IMPORTANT: Skip format codes (^f00, ^c255, ^l) which TextRenderer parses as raw ASCII.
	char convertedText[512];
	int srcLen = strlen(textStr);
	int dstIdx = 0;
	int numChars = _rebelMsgFont->getNumChars();

	for (int i = 0; i < srcLen && dstIdx < (int)sizeof(convertedText) - 1; i++) {
		byte ch = (byte)textStr[i];

		// Check for format codes (^f, ^c, ^l) - keep them as raw ASCII
		if (ch == '^' && i + 1 < srcLen) {
			byte next = (byte)textStr[i + 1];
			if (next == 'f' && i + 3 < srcLen) {
				// ^fXX - font switch (4 chars total)
				convertedText[dstIdx++] = textStr[i++];  // ^
				convertedText[dstIdx++] = textStr[i++];  // f
				convertedText[dstIdx++] = textStr[i++];  // X
				convertedText[dstIdx++] = textStr[i];    // X
				continue;
			} else if (next == 'c' && i + 4 < srcLen) {
				// ^cXXX - color switch (5 chars total)
				convertedText[dstIdx++] = textStr[i++];  // ^
				convertedText[dstIdx++] = textStr[i++];  // c
				convertedText[dstIdx++] = textStr[i++];  // X
				convertedText[dstIdx++] = textStr[i++];  // X
				convertedText[dstIdx++] = textStr[i];    // X
				continue;
			} else if (next == 'l') {
				// ^l - line break marker (2 chars)
				convertedText[dstIdx++] = textStr[i++];  // ^
				convertedText[dstIdx++] = textStr[i];    // l
				continue;
			} else if (next == '^') {
				// ^^ - escaped caret (becomes single ^)
				i++;  // Skip first ^
				// Fall through to convert second ^ as normal char
				ch = '^';
			}
		}

		// Convert regular characters from ASCII to font index
		// First convert lowercase to uppercase (the font likely only has uppercase)
		if (ch >= 'a' && ch <= 'z') {
			ch = ch - 'a' + 'A';  // Convert to uppercase
		}

		if (ch >= 32 && ch < (byte)(32 + numChars)) {
			convertedText[dstIdx++] = ch - 32;  // Convert ASCII to font index
		} else if (ch == '\n' || ch == '\r') {
			convertedText[dstIdx++] = ch;  // Keep control characters as-is
		} else {
			convertedText[dstIdx++] = 0;  // Replace invalid characters with space (index 0)
		}
	}
	convertedText[dstIdx] = '\0';

	// Draw the text string (with converted character indices), but only when subtitles are
	// enabled: opcode 9 is a subtitle/message path, so it honors the global
	// "subtitles" setting and the in-game TEXT toggle (same ConfMan key). The chunk is
	// still fully parsed above so stream consumption is unaffected.
	if (ConfMan.getBool("subtitles")) {
		Rebel2FontSet fontSet;
		fontSet.numFonts = 1;
		fontSet.fonts[0] = _rebelMsgFont;

		if (textFlags & 0x04) {
			// Word-wrapped text
			drawRebel2StringWrap(fontSet, convertedText, dstIdx, renderBitmap, clipRect, posX, posY, width, textColor, styleFlags);
		} else {
			// Single-line text
			drawRebel2String(fontSet, convertedText, dstIdx, renderBitmap, clipRect, posX, posY, width, textColor, styleFlags);
		}
	}

	debugC(DEBUG_INSANE, "Opcode 9: Rendered subtitle at (%d,%d) flags=0x%x clip=(%d,%d,%d,%d)",
		posX, posY, textFlags, clipX, clipY, clipW, clipH);
}

// enemyUpdate -- Opcode 4: update enemy position and state (FUN_004028C5 / FUN_0041E7C2).
void InsaneRebel2::enemyUpdate(byte *renderBitmap, Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4) {
	// Opcode 4: Enemy position update
	// Read 5 shorts from the stream (offset +8 through +16)
	int16 enemyId = b.readSint16LE();  // Offset +8
	int16 x = b.readSint16LE();        // Offset +10 (0x0A)

	// If enemy is disabled in bit table, skip update
	bool disabled = isBitSet(enemyId);

	int16 y = b.readSint16LE();        // Offset +12 (0x0C)
	int16 w = b.readSint16LE();        // Offset +14 (0x0E) - Width
	int16 h = b.readSint16LE();        // Offset +16 (0x10) - Height

	// par3==2 (turret handler) marks a translucent surface / force field (FUN_40A2E0 local_8==2):
	// never a direct hit target — it is destroyed by clearing the gauge group (par4) it depends
	// on. Show it while that group still has elements, hide it once the group is cleared.
	if (par3 == 2 && _rebelHandler == 0x26) {
		const int surfaceIdx = (par4 >= 100 && par4 < 110) ? (par4 - 100) : -1;
		if (surfaceIdx >= 0 && _rebelGaugeCleared[surfaceIdx])
			setBit(enemyId);
		else
			clearBit(enemyId);
		return; // surfaces are not added to the hittable enemy list
	}

	// If disabled, stop processing this object
	if (disabled) {
		// debugC(DEBUG_INSANE, "Skipping Opcode 4 for disabled enemy ID=%d", enemyId);
		return;
	}

	// The disassembly shows half-width/half-height are used for centering:
	//   halfW = w >> 1
	//   halfH = h >> 1
	//   centerX = x + halfW
	//   centerY = y + halfH
	// But for drawing the bounding box, we want the top-left corner (x, y) and full dimensions.

	// Update enemy list for hit detection
	// Enemy type comes from par4 (IACT offset +6), NOT par3 (offset +4).
	// In the original (FUN_004028C5/FUN_0041E7C2): sVar5/sVar2 = *(short *)(*local + 6)
	// This maps to par4 (userId field). Used for DAT_0047ab98 wave state bitmask:
	//   DAT_0047ab98 |= 1 << (type & 0x1f)
	debugC(DEBUG_INSANE, "Opcode4: handler=%d enemyId=%d par2=%d par3=%d par4/type=%d pos=(%d,%d) size=(%d,%d)",
		_rebelHandler, enemyId, par2, par3, par4, x, y, w, h);

	bool found = false;
	Common::List<enemy>::iterator it;
	for (it = _enemies.begin(); it != _enemies.end(); ++it) {
		if (it->id == enemyId) {
			it->rect = Common::Rect(x, y, x + w, y + h);
			it->type = par4;  // Enemy type from IACT offset +6 (userId)
			// The _iactBits[] bit table is the authoritative alive/dead state.
			// We only reach here when isBitSet(enemyId) == false, meaning
			// the game considers this enemy alive. Reset destroyed/active
			// to match — this is critical when clearBit(0) re-enables all
			// enemies at wave start but the _enemies list still has stale
			// destroyed=true from a previous wave.
			it->active = true;
			it->destroyed = false;
			found = true;
			break;
		}
	}
	if (!found) {
		initEnemyStruct(enemyId, x, y, w, h, true, false, -1, par4);
	}
}

// initEnemyStruct -- Create and append a new enemy entry.
void InsaneRebel2::initEnemyStruct(int id, int32 x, int32 y, int32 w, int32 h, bool active, bool destroyed, int32 explosionFrame, int type) {
	enemy e;
	e.id = id;
	e.type = type;
	e.rect = Common::Rect(x, y, x + w, y + h);
	e.active = active;
	e.destroyed = destroyed;
	e.explosionFrame = explosionFrame;
	e.savedBackground = nullptr;
	e.savedBgWidth = 0;
	e.savedBgHeight = 0;
	_enemies.push_back(e);
}

} // End of namespace Scumm
