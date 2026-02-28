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
#include "common/memstream.h"
#include "common/util.h"

#include "scumm/scumm_v7.h"

#include "scumm/smush/smush_player.h"
#include "scumm/smush/smush_font.h"

#include "scumm/insane/insane_rebel.h"

namespace Scumm {

// External codec functions from codec1.cpp
extern void smushDecodeRLEOpaque(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);

void InsaneRebel2::procPreRendering(byte *renderBitmap) {
	// Call base class implementation first (handles Full Throttle state machine)
	Insane::procPreRendering(renderBitmap);

	// Reset opcode 6 init flag at the start of each new video.
	// This ensures the per-wave init (clearBit, link table reset, wave state)
	// fires exactly once per wave video, not every frame.
	if (_player && _player->_frame == 0) {
		_rebelOp6Initialized = false;
	}

	// For Level 2 gameplay (Handler 8 only), restore the background BEFORE FOBJ decoding.
	// The tiny FOBJ sprites (7x10, 9x38 pixels) only draw new sprite positions but don't
	// clear old ones. By restoring the full background each frame, we ensure old sprite
	// positions are erased before new ones are drawn.
	//
	// This is called at the start of handleFrame(), before any FOBJ chunks are processed.
	if (_rebelHandler == 8 && _level2BackgroundLoaded && _level2Background && renderBitmap) {
		for (int y = 0; y < 200; y++) {
			memcpy(renderBitmap + y * 320, _level2Background + y * 320, 320);
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

	// Chapter selection: Set FOBJ offset to scroll preview thumbnails in O_LEVEL.SAN.
	// Original (FUN_00415CF8): offsets start at (0,0) for the first display update,
	// then FUN_00425170 sets them to (-90, chapter*-50+75) AFTER each frame.
	// Frame 0 must use (0,0) so the 80x800 preview strip at X=320 renders off-screen
	// and STOR captures it cleanly. Frames 1+ use the scroll offset so FTCH re-renders
	// the strip at the correct preview position.
	if (_gameState == kStateChapterSelect && _player) {
		if (_player->_frame > 0) {
			// Clear screen to black before FTCH re-renders the preview strip.
			// Our FTCH only re-draws the preview area (80px wide at X=230);
			// without clearing, old menu text and preview artifacts persist.
			if (renderBitmap) {
				memset(renderBitmap, 0, _vm->_screenWidth * _vm->_screenHeight);
			}
			_player->_fobjOffsetX = _previewOffsetX;
			_player->_fobjOffsetY = _previewOffsetY;
		}
	}
}

void InsaneRebel2::procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					  int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
					  int16 par1, int16 par2, int16 par3, int16 par4) {
	// Debug: Log all IACT opcodes
	debug("Rebel2 IACT: opcode=%d par2=%d par3=%d par4=%d gameState=%d sceneId=%d",
		par1, par2, par3, par4, _gameState, _currSceneId);

	if (_keyboardDisable)
		return;

	// Handle menu IACT - menu videos have embedded ANIM data in IACT chunks
	// Menu IACTs have par1=8 (code), par2=46 (flags), par4>=1000 (userId)
	// The embedded ANIM contains the full menu frame
	if (_gameState == kStateMainMenu && par1 == 8 && par4 >= 1000) {
		debug("Rebel2 IACT: Menu mode - processing embedded ANIM (userId=%d)", par4);

		// Scan for embedded ANIM tag in the IACT data
		int64 startPos = b.pos();
		int64 totalSize = b.size();
		debug("Rebel2 IACT: stream pos=%d, size=%d, remaining=%d",
			(int)startPos, (int)totalSize, (int)(totalSize - startPos));

		if (totalSize > startPos) {
			int64 remaining = totalSize - startPos;
			int scanSize = (int)MIN<int64>(remaining, 65536);
			byte *scanBuf = (byte *)malloc(scanSize);
			if (scanBuf) {
				int bytesRead = b.read(scanBuf, scanSize);
				debug("Rebel2 IACT: Read %d bytes, first 16: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
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
						debug("Rebel2 IACT: Found embedded ANIM at offset %d, size %d", (int)i, (int)animReportedSize);
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

				debug("Rebel2 IACT: No ANIM tag found in menu IACT data");
				b.seek(startPos);
				free(scanBuf);
			}
		}
		return;
	}

	if (_currSceneId == 1)
		iactRebel2Scene1(renderBitmap, codecparam, setupsan12, setupsan13, b, size, flags, par1, par2, par3, par4);
}


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
		debug("Rebel2 IACT Opcode 5: par2=%d par3=%d par4=%d", par2, par3, par4);

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
			debug("Rebel2 Opcode 7 par4=0: wind=(%d,%d)", body0, body1);
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
			debug("Rebel2 Opcode 7 par4=1: corridor left=%d top=%d (adjusted left=%d)",
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
			debug("Rebel2 Opcode 7 par4=2: corridor right=%d bottom=%d (adjusted right=%d)",
				body0, body1, _corridorRightX);
			break;
		case 5:
			// Flag value
			debug("Rebel2 Opcode 7 par4=5: flag=%d", body0);
			break;
		default:
			debug("Rebel2 Opcode 7 par4=%d: body=(%d,%d) — unknown sub-opcode", par4, body0, body1);
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
		debug("Rebel2 IACT: Low Opcode %d (par2=%d par3=%d par4=%d)", par1, par2, par3, par4);
	} else {
		debug("Rebel2 IACT: Unknown Opcode %d (par2=%d par3=%d par4=%d)", par1, par2, par3, par4);
	}
}
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
				debug("Rebel2: Linked ID=%d to Parent=%d (Slot 0) - child DISABLED (parent alive)", childId, parentId);
			} else {
				clearBit(childId);
				debug("Rebel2: Linked ID=%d to Parent=%d (Slot 0) - child ENABLED (parent dead)", childId, parentId);
			}
		} else {
			debug("Rebel2: Skipping link with invalid IDs childId=%d parentId=%d", childId, parentId);
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
				debug("Rebel2 Opcode2 (H%d): Force ENABLE target=%d (par4=100)", _rebelHandler, targetId);
			} else {
				// Check wave state: if enemy type has been killed, disable target
				int bitMask = 1 << (value & 0x1f);
				if ((_rebelWaveState & bitMask) != 0) {
					setBit(targetId);
					debug("Rebel2 Opcode2 (H%d): Disable target=%d (type %d killed, wave=0x%x)", _rebelHandler, targetId, value, _rebelWaveState);
				}
			}
			return;
		}

		if (value > 1 && value < 10) { // 1 < value < 10: random disable
			if (_vm->_rnd.getRandomNumber(value) == 0) {
				setBit(targetId);
				debug("Rebel2 IACT Opcode2: Random DISABLE target=%d (value=%d)", targetId, value);
			}
		} else if (value > 10 && value < 20) { // 10 < value < 20: enable/disable with special value==11 = force enable
			if (value == 11) {
				clearBit(targetId);
				debug("Rebel2 IACT Opcode2: FORCE ENABLE target=%d (value=11)", targetId);
			} else {
				if (_vm->_rnd.getRandomNumber(value - 10) == 0) {
					clearBit(targetId);
					debug("Rebel2 IACT Opcode2: Random ENABLE target=%d (value=%d)", targetId, value);
				} else {
					setBit(targetId);
					debug("Rebel2 IACT Opcode2: Random DISABLE target=%d (value=%d)", targetId, value);
				}
			}
		} else if (value > 99 && value < 110) { // 99 < value < 110: increment value counter if target active
			if (!isBitSet(targetId)) {
				int idx = value - 100;
				if (idx >= 0 && idx < 10) {
					_rebelValueCounters[idx]++;
					_rebelLastCounter = _rebelValueCounters[idx];
					debug("Rebel2 IACT Opcode2: Increment VAL counter[%d] -> %d (target=%d)", value, _rebelValueCounters[idx], targetId);
				}
			}

		} else if (value > 0x3ff) { // Bitmask case: value > 0x3FF
 			for (int slot = 1; slot <= 9; ++slot) {
				if ((value & (1 << (slot - 1))) != 0) {
					if (!isBitSet(targetId)) {
						_rebelMaskCounters[slot]++;
						_rebelLastCounter = _rebelMaskCounters[slot];
						debug("Rebel2 IACT Opcode2: Increment MASK counter[%d] -> %d (target=%d)", slot, _rebelMaskCounters[slot], targetId);
					}
				}
			}
		}

		// Unknown sub-type: log and return
		debug("Rebel2 IACT Opcode2: Unhandled par3=%d par4=%d", par3, par4);
	}
}
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
			debug("Rebel2 Opcode3: H25 par3=5 srcId=%d isBitSet=%d damageLevel=%d",
				srcIdBody1, isBitSet(srcIdBody1), _rebelDamageLevel);

			if (_rebelDamageLevel < 2 && !isBitSet(srcIdBody1)) {
				LevelDifficultyParams params = getDifficultyParams();
				int probability = (params.shotAccuracy >= 0) ? params.shotAccuracy : 0;
				int roll = _vm->_rnd.getRandomNumber(99);
				debug("Rebel2 Opcode3: probability=%d roll=%d (need roll < prob)", probability, roll);

				if (roll < probability) {
					if (!_rebelInvulnerable) {
						int damageAmount = (params.shotDamage >= 0) ? params.shotDamage : 0;
						_playerDamage += damageAmount;
						if (_playerDamage > 255)
							_playerDamage = 255;
						debug("Rebel2: H25 PROBABILISTIC damage from %d. Damage=%d total=%d",
							srcIdBody1, damageAmount, _playerDamage);
					}
					initDamageFlash();
				}
			} else {
				debug("Rebel2 Opcode3: H25 par3=5 BLOCKED (damageLevel=%d isBitSet=%d)",
					_rebelDamageLevel, isBitSet(srcIdBody1));
			}
		} else if (par3 == 1 && !isBitSet(srcIdBody0) && par4 != 4) {
			// Hit counter only — NO damage (lines 94-98)
			_rebelHitCounter++;
			debug("Rebel2: H25 hit counter++ -> %d (par3=1 par4=%d, no damage)",
				_rebelHitCounter, par4);
		} else {
			debug("Rebel2 Opcode3: H25 par3=%d par4=%d (no action)", par3, par4);
		}

		// Direct damage: par4==100, separate from par3 branches (lines 99-111)
		if (par4 == 100 && !isBitSet(srcIdBody0)) {
			if (!_rebelInvulnerable) {
				LevelDifficultyParams dparams = getDifficultyParams();
				int directHitDamage = (dparams.missDamage >= 0) ? dparams.missDamage : 0;
				_playerDamage += directHitDamage;
				if (_playerDamage > 255)
					_playerDamage = 255;
				debug("Rebel2: H25 DIRECT HIT par4=100 damage=%d total=%d",
					directHitDamage, _playerDamage);
			}
			initDamageFlash();
		}
	} else if (par3 == 1 || par3 == 2) {
		// Non-Handler-25 direct hit path — FUN_4092D9 lines 209-227
		int16 srcId = b.readSint16LE(); // body[0] (offset +8): source enemy ID

		debug("Rebel2 Opcode3: par3=%d par4=%d srcId=%d isBitSet=%d",
			par3, par4, srcId, isBitSet(srcId));

		if (!isBitSet(srcId)) {
			_rebelHitCounter++;
			debug("Rebel2: Incremented hit counter -> %d", _rebelHitCounter);

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
					if (!_rebelInvulnerable) {
						_playerDamage += directHitDamage;
						if (_playerDamage > 255)
							_playerDamage = 255;
						debug("Rebel2: DIRECT HIT damage from enemy %d. par3=%d par4=%d damage=%d total=%d",
							srcId, par3, par4, directHitDamage, _playerDamage);
					}
					initDamageFlash();
				}
			}
		}
	} else if (par3 == 5) {
		// Non-Handler-25 probabilistic damage — FUN_4092D9 lines 228-239
		b.skip(2); // Skip body[0]
		int16 srcId = b.readSint16LE(); // body[1] (offset +10)

		debug("Rebel2 Opcode3: par3=5 srcId=%d isBitSet=%d", srcId, isBitSet(srcId));

		if (!isBitSet(srcId)) {
			LevelDifficultyParams params = getDifficultyParams();
			int probability = (params.shotAccuracy >= 0) ? params.shotAccuracy : 0;

			int roll = _vm->_rnd.getRandomNumber(99);
			debug("Rebel2 Opcode3: probability=%d roll=%d (need roll < prob)", probability, roll);

			if (roll < probability) {
				if (!_rebelInvulnerable) {
					int damageAmount = (params.shotDamage >= 0) ? params.shotDamage : 0;
					_playerDamage += damageAmount;
					if (_playerDamage > 255)
						_playerDamage = 255;
					debug("Rebel2: PROBABILISTIC damage from enemy %d. Damage=%d total=%d",
						srcId, damageAmount, _playerDamage);
				}
				if (_rebelHandler == 8) {
					triggerDamageEffect();
				} else {
					initDamageFlash();
				}
			}
		}
	} else {
		debug("Rebel2 Opcode3: UNHANDLED par3=%d par4=%d", par3, par4);
	}
}

void InsaneRebel2::iactRebel2Opcode6(byte *renderBitmap, Common::SeekableReadStream &b, int32 chunkSize, int16 par2, int16 par3, int16 par4) {
	// Opcode 6: Level setup / mode switch
	// Based on FUN_41CADB case 4 (switch on *local_14 - 2 == 4, meaning opcode 6)
	//
	// For Handler 8 (third-person on foot) - FUN_00401234 case 4:
	// - par3 sets ship level mode (DAT_0043e000)
	// - par4 == 1 triggers status bar display and state reset
	// - Updates ship position based on mouse input
	//
	// For Handler 0x26/0x19 (turret/FPS):
	// - Same par4 == 1 behavior
	// - Different view offset calculations

	debug("Rebel2 IACT Opcode 6: par2=%d par3=%d par4=%d", par2, par3, par4);

	// Update handler type if par2 is a known handler value (from FUN_4033CF case 6)
	if (par2 == 7 || par2 == 8 || par2 == 0x19 || par2 == 0x26) {
		// Reset Level 2 background flag when transitioning away from Handler 8
		if (_rebelHandler == 8 && par2 != 8) {
			_level2BackgroundLoaded = false;
		}
		_rebelHandler = par2;
		debug("Rebel2 Opcode 6: Setting handler=%d", par2);
	}

	// Handler 8 specific logic (third-person on foot) - FUN_00401234 case 4
	if (_rebelHandler == 8) {
		// Set ship level mode (DAT_0043e000 = par3)
		_shipLevelMode = par3;

		// If par4 == 1, enable status bar
		if (par4 == 1) {
			_rebelStatusBarSprite = 5;  // Status bar sprite for Handler 8
		}

		// Reset state when shipLevelMode != 0 && par4 == 1 (FUN_401234 lines 97-103)
		// Guard with _rebelOp6Initialized: runs once per wave video, not per frame.
		if (_shipLevelMode != 0 && par4 == 1 && !_rebelOp6Initialized) {
			clearBit(0);
			for (int i = 0; i < 512; i++) {
				_rebelLinks[i][0] = 0;
				_rebelLinks[i][1] = 0;
				_rebelLinks[i][2] = 0;
			}
			_rebelWaveState = _rebelPhaseState;
			_rebelOp6Initialized = true;
			debug("Rebel2 Opcode 6 (Handler 8): Wave init, wave=0x%x", _rebelWaveState);
		}

		// Skip position calculation for special modes 4 and 5
		if (_shipLevelMode != 4 && _shipLevelMode != 5) {
			// ===== Movement Range Transition (Covered vs Shooting) =====
			// Based on FUN_00401234 lines 85-120:
			// Mode 2 = "Covered" state - contract movement range to 41 (0x29)
			// Other modes = "Shooting" state - expand movement range to 127 (0x7f)
			// Transition happens gradually at ±10 per frame for smooth animation
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

			// Map mouse position (-127 to 127 range) to ship target
			// Mouse is 0-320, center is 160. Map to -127 to 127 range
			int16 mouseOffsetX = (int16)((_vm->_mouse.x - 160) * 127 / 160);
			int16 mouseOffsetY = (int16)((_vm->_mouse.y - 100) * 127 / 100);

			// Clamp X offset to movement range limit (covered/shooting state)
			// Based on FUN_00401234 lines 119-136
			if (mouseOffsetX > _movementRangeLimit)
				mouseOffsetX = _movementRangeLimit;
			if (mouseOffsetX < -_movementRangeLimit)
				mouseOffsetX = -_movementRangeLimit;
			// Y offset always uses full range (±127)
			if (mouseOffsetY > 127)
				mouseOffsetY = 127;
			if (mouseOffsetY < -127)
				mouseOffsetY = -127;

			// Calculate target positions using the original formula
			_shipTargetX = (int16)(((mouseOffsetX * 5 + 0x27b) * 0x40) / 0xfe);
			_shipTargetY = (int16)(-((mouseOffsetY * 5 + 0x27b) * 0x10) / 0xfe);

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

			// Calculate ship direction indices for sprite selection
			// Map mouse position to 5x7 direction grid (like Handler 7)
			int16 mouseX = _vm->_mouse.x;
			int16 mouseY = _vm->_mouse.y;

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

		// Update firing state from mouse button
		// Mode 4 (autopilot) disables shooting - FUN_00401CCF line 82-84
		if (_shipLevelMode == 4) {
			_shipFiring = false;
		} else {
			_shipFiring = (_vm->VAR(_vm->VAR_LEFTBTN_HOLD) != 0);
		}

		debug("Rebel2 Opcode 6 (Handler 8): mode=%d range=%d shipPos=(%d,%d) target=(%d,%d) firing=%d dir=(%d,%d,%d)",
			_shipLevelMode, _movementRangeLimit, _shipPosX, _shipPosY, _shipTargetX, _shipTargetY, _shipFiring,
			_shipDirectionH, _shipDirectionV, _shipDirectionIndex);

		// Handler 8 doesn't use the same view offset logic as other handlers
		// Skip the rest of the function for Handler 8
		return;
	}

	// Handler 7 specific logic (third-person ship) - FUN_0040d836 / FUN_0040c3cc
	// Used for Level 3 and similar space combat levels
	if (_rebelHandler == 7) {
		// Set control mode: DAT_004437c0 = param_5[3] = par4 in FUN_40C3CC case 4.
		// This determines collision mode and shooting capability:
		//   Mode 0: Obstacle avoidance — SECONDARY zones, corridor boundaries
		//   Mode 1: Tunnel flight — PRIMARY zones, per-edge push-back (hMargin=0x28)
		//   Mode 2: Combat mode — shooting ENABLED, SECONDARY zones
		//   Mode 3: Tunnel flight — PRIMARY zones, per-edge push-back (hMargin=0x0f)
		_flyControlMode = par4;
		debug("Rebel2 Opcode 6 (Handler 7): Control mode set to %d (shooting %s)",
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
			_rebelStatusBarSprite = 5;  // Status bar sprite
			debug("Rebel2 Opcode 6 (Handler 7): Status bar enabled (body flag=%d)", bodyStatusFlag);
		}

		// ============================================================
		// Ship position update — FUN_40C3CC case 4, lines 49-327
		// ============================================================
		// Velocity-based physics with momentum/inertia:
		//   Mouse offset from center → scaled input [-127,127]
		//   → velocity history averaging → physics delta (clamped ±12/frame)
		//   → position clamping → corridor collision → perspective offsets
		//
		// Level data table (DAT_0047e0e8 + level*0x242 + difficulty*0x22):
		//   offset 0: smoothing param (>>4 +1 = window size)
		//   offset 2: Y speed          offset 4: X speed (levelSpeed)
		//   offset 6: wind multiplier  offset 14: corridor damage
		// We don't have the actual level data, so we use calibrated defaults.

		// --- Step 1: Mouse input as offset from screen center ---
		// DAT_0047a7e0 = mouseX - 160, DAT_0047a7e2 = mouseY - 100
		// _vm->_mouse.x/y are in virtual screen coords (0-319, 0-199)
		// consistent with handler 8 which uses _vm->_mouse.x directly.
		int16 inputX = (int16)(_vm->_mouse.x - 160);  // DAT_0047a7e0
		int16 inputY = (int16)(_vm->_mouse.y - 100);  // DAT_0047a7e2

		// Clamp: mouse mode uses [-160, 160] for X, [-127, 127] for Y (lines 55-70)
		if (inputX > 160)
			inputX = 160;
		if (inputX < -160)
			inputX = -160;
		if (inputY > 127)
			inputY = 127;
		if (inputY < -127)
			inputY = -127;

		// --- Step 2: Scale to [-127, 127] (lines 82-84) ---
		// Mouse mode: local_c = (DAT_0047a7e0 * 0x7f) / 0xa0
		int16 local_c = (int16)((inputX * 127) / 160);
		int16 local_14 = inputY;  // Y already in [-127, 127]

		// --- Step 3: Velocity history + smoothed average (lines 141-157) ---
		for (int i = 24; i > 0; i--) {
			_velocityHistory[i] = _velocityHistory[i - 1];
		}
		_velocityHistory[0] = local_c;

		// Window size = (levelData[0] >> 4) + 1. Calibrated default: 5.
		const int smoothWindow = 5;
		int velSum = 0;
		for (int i = 0; i < smoothWindow; i++) {
			velSum += _velocityHistory[i];
		}
		_smoothedVelocity = (int16)(velSum / smoothWindow);  // DAT_0044370c

		// --- Step 4: Wind history (lines 158-173) ---
		// Wind multiplier comes from level data[6]. Without data, use 0 (no wind).
		const int16 windMult = 0;
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

		// --- Step 5: Position delta (lines 174-242) ---
		// levelSpeed (offset 4): calibrated so max velocity (127) → delta 12.
		//   8 = (speed * 127) >> 9 → speed ≈ 32
		// levelYSpeed (offset 2): calibrated so max input (127) → delta ~6.
		//   6 = (speed * 127) >> 10 → speed ≈ 48
		const int16 levelSpeed = 32;
		const int16 levelYSpeed = 48;
		int16 absSmoothVel = ABS(_smoothedVelocity);
		int16 positionDeltaX;

		if (_flyControlMode == 1) {
			// Mode 1: Full cross-axis coupling (lines 174-186)
			// Banking: vertical input deflects horizontal movement
			if (local_c < 1) {
				positionDeltaX = (int16)((levelSpeed * _smoothedVelocity - absSmoothVel * local_14 - windEffectX) >> 9);
			} else {
				positionDeltaX = (int16)((levelSpeed * _smoothedVelocity + absSmoothVel * local_14 - windEffectX) >> 9);
			}
		} else {
			// Mode 0/2/3: Reduced cross-axis coupling (lines 218-230)
			if (local_c < 1) {
				positionDeltaX = (int16)((levelSpeed * _smoothedVelocity - (absSmoothVel * local_14 >> 2) - windEffectX) >> 9);
			} else {
				positionDeltaX = (int16)((levelSpeed * _smoothedVelocity + (absSmoothVel * local_14 >> 2) - windEffectX) >> 9);
			}
		}

		// Clamp X delta to ±12 per frame (lines 187-192 / 231-236)
		if (positionDeltaX < -11)
			positionDeltaX = -12;
		if (positionDeltaX > 11)
			positionDeltaX = 12;

		// Apply X delta (line 193 / 237)
		_flyShipScreenX += positionDeltaX;

		// Y delta
		if (_flyControlMode == 1) {
			// Mode 1: clamped to ±12 with wind (lines 194-216)
			int yCalc = levelYSpeed * local_14 - (windEffectY >> 1);
			int yDelta = yCalc >> 10;
			if (yDelta < -12)
				yDelta = -12;
			if (yDelta > 12)
				yDelta = 12;
			_flyShipScreenY -= (int16)yDelta;
		} else {
			// Mode 0/2/3: unclamped (lines 238-241)
			_flyShipScreenY -= (int16)((levelYSpeed * local_14) >> 10);
		}

		// Store vertical input for direction sprite (line 243)
		_verticalInput = local_14;  // DAT_0044370e

		// Ship facing direction (line 244)
		_facingRight = (0xd4 < _smoothedVelocity + _flyShipScreenX);

		// --- Step 6: Position clamping (lines 245-256) ---
		if (_flyShipScreenX > 0x194)
			_flyShipScreenX = 0x194;  // 404
		if (_flyShipScreenY > 0xF0)
			_flyShipScreenY = 0xF0;    // 240
		if (_flyShipScreenX < 0x14)
			_flyShipScreenX = 0x14;    // 20
		if (_flyShipScreenY < 0x14)
			_flyShipScreenY = 0x14;    // 20

		// --- Step 7: Corridor collision — mode 0/2 only (lines 257-292) ---
		if (_flyControlMode == 0 || _flyControlMode == 2) {
			LevelDifficultyParams wallParams = getDifficultyParams();
			int corridorWallDmg = (wallParams.dodgeDamage >= 0) ? wallParams.dodgeDamage : 0;

			// Right boundary (lines 258-270)
			// Original: position is ALWAYS clamped; damage/bounce only when cooldown < 5
			if (_corridorRightX < _flyShipScreenX) {
				_flyShipScreenX = _corridorRightX;
				if (_hitCooldown < 5) {
					for (int i = 0; i < 25; i++)
						_velocityHistory[i] = -127;
					_hitCooldown = 10;
					_spaceShotDirection = 1;
					initDamageFlash();
					if (!_rebelInvulnerable) {
						_playerDamage += corridorWallDmg;
						if (_playerDamage > 255)
							_playerDamage = 255;
					}
					_rebelHitCounter++;
					playSfx(1, 127, 100);  // CRASH.SAD, right wall → pan right
				}
			}
			// Left boundary (lines 271-283)
			if (_flyShipScreenX < _corridorLeftX) {
				_flyShipScreenX = _corridorLeftX;
				if (_hitCooldown < 5) {
					for (int i = 0; i < 25; i++)
						_velocityHistory[i] = 127;
					_hitCooldown = 10;
					_spaceShotDirection = 0;
					initDamageFlash();
					if (!_rebelInvulnerable) {
						_playerDamage += corridorWallDmg;
						if (_playerDamage > 255)
							_playerDamage = 255;
					}
					_rebelHitCounter++;
					playSfx(1, 127, -100);  // CRASH.SAD, left wall → pan left
				}
			}
			// Y boundary clamping — no damage (lines 285-292)
			if (_corridorBottomY < _flyShipScreenY) {
				_flyShipScreenY = _corridorBottomY;
			}
			if (_flyShipScreenY < _corridorTopY) {
				_flyShipScreenY = _corridorTopY;
			}
		}

		// --- Step 8: Perspective offsets (lines 293-316) ---
		// f(x) = (focal * center * |offset|) / ((center - focal) * |offset| + focal * center)
		// Close view (DAT_0047a7fc < 1): focalX=0x34, focalY=0x2d
		// Far view (DAT_0047a7fc >= 1): focalX=0x2b, focalY=0x19
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

		// View shift = clamped smoothed velocity (FUN_0040d836 lines 68-74)
		_viewShift = _smoothedVelocity;
		if (_viewShift > 127)
			_viewShift = 127;
		if (_viewShift < -127)
			_viewShift = -127;

		// --- Step 9: Direction sprite (FUN_0040d836 lines 88-106) ---
		// 5x7 grid: vDir(0-4) * 7 + hDir(0-6) = sprite index (0-34)
		// vDir from vertical input: (0xa0 - verticalInput) >> 6
		int16 vDir = (int16)(((int)(0xa0 - _verticalInput) + ((0xa0 - _verticalInput) < 0 ? 63 : 0)) >> 6);
		if (vDir < 0)
			vDir = 0;
		if (vDir > 4)
			vDir = 4;

		// hDir from smoothed velocity: (0x95 - smoothedVelocity) / 0x2b
		int16 hDir = (int16)((0x95 - _smoothedVelocity) / 0x2b);
		if (hDir < 0)
			hDir = 0;
		if (hDir > 6)
			hDir = 6;

		// Hysteresis at center (lines 90-97, 98-105)
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

		_shipFiring = (_flyControlMode == 2) && (_vm->VAR(_vm->VAR_LEFTBTN_HOLD) != 0);

		debug("Rebel2 H7: pos=(%d,%d) vel=%d vIn=%d dx=%d dir=%d mode=%d",
			_flyShipScreenX, _flyShipScreenY, _smoothedVelocity,
			_verticalInput, positionDeltaX, _shipDirectionIndex, _flyControlMode);

		return;
	}

	// Handler 25 (0x19) specific logic (mixed mode - speeder bike)
	// Based on FUN_0041cadb case 4 (opcode 6) lines 113-229
	if (_rebelHandler == 25) {
		// Read the reset flag from IACT data at offset 8-9 (local_14[4] in decompiled code)
		// The stream position should be at offset 8 after par4 was read
		// From FUN_0041cadb line 114: if (local_14[4] == 1) { ... reset ... }
		int16 par5 = 0;
		if (b.pos() + 2 <= b.size()) {
			int64 savedPos = b.pos();
			par5 = b.readSint16LE();
			b.seek(savedPos);  // Don't consume the stream
		}

		// If par5 == 1, enable status bar and reset state (lines 114-121)
		// Note: This is local_14[4] in the decompiled code, NOT local_14[3] (par4)
		if (par5 == 1) {
			_rebelStatusBarSprite = 5;
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
				debug("Rebel2 Opcode 6 (Handler 25): Wave init, wave=0x%x autopilot=%d damageLevel=%d",
					_rebelWaveState, _rebelAutopilot, _rebelDamageLevel);
			}
		}

		// Set sprite mode (DAT_00457900 = local_14[3]) - controls which GRD sprite to render
		// From FUN_0041cadb line 122: DAT_00457900 = local_14[3];
		// In ScummVM's IACT parsing: local_14[3] = offset 6-7 = par4
		// Mode 1: Uncovered, shooting position - sprite on left
		// Mode 2: Covered, vertical shift
		// Mode 3: Transition between covered/uncovered - sprite position depends on direction
		// Mode 4: Alternative uncovered position - sprite on right
		_grdSpriteMode = par4;  // local_14[3] maps to par4 (offset 6-7)

		debug("Rebel2 Handler25 Opcode6: par2=%d par3=%d par4=%d(mode) par5=%d(reset) autopilot=%d damageLevel=%d controlMode=%d",
			par2, par3, par4, par5, _rebelAutopilot, _rebelDamageLevel, _rebelControlMode);

		// Autopilot logic (lines 123-146)
		// From original FUN_0041cadb - NO damageLevel check, toggle happens immediately
		// The damage level counter provides the smooth visual transition
		if (!_rebelInvulnerable) {
			if (_rebelAutopilot == 0) {
				// Uncovered: RIGHT button enters cover
				if ((_rebelControlMode & 2) != 0) {
					_rebelAutopilot = 1;
					debug("Rebel2 Handler25: Entering cover (right click), controlMode=%d", _rebelControlMode);
				}
			} else {
				// Covered: ANY button exits cover
				if (_rebelControlMode != 0) {
					_rebelAutopilot = 0;
					debug("Rebel2 Handler25: Exiting cover (button click), controlMode=%d", _rebelControlMode);
				}
			}
			// Clear control mode after processing (sticky flags consumed)
			_rebelControlMode = 0;
		} else {
			// Invulnerable mode: random autopilot changes
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

		// Update damage level counter (lines 147-154)
		// This provides the smooth transition animation between covered/uncovered states
		int prevDamageLevel = _rebelDamageLevel;
		if (_rebelAutopilot == 0) {
			// Uncovered: decrement damage level towards 0
			if (_rebelDamageLevel > 0) {
				_rebelDamageLevel--;
			}
		} else {
			// Covered: increment damage level towards 5
			if (_rebelDamageLevel < 5) {
				_rebelDamageLevel++;
			}
		}
		if (_rebelDamageLevel != prevDamageLevel) {
			debug("Rebel2 Handler25: damageLevel transition %d -> %d (autopilot=%d)",
				prevDamageLevel, _rebelDamageLevel, _rebelAutopilot);
		}

		// Flight direction logic for mode 3 (lines 155-177)
		if (_grdSpriteMode == 3) {
			if (_rebelDamageLevel == 5) {
				// At max damage, check for direction change input
				// For now, use mouse X position to determine direction
				int16 mouseX = _vm->_mouse.x;
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

		// Calculate sprite and view offset positions based on mode (lines 182-213)
		// DAT_0045790c = view offset X (for corridor overlay)
		// DAT_0045790e = view offset Y (for corridor overlay)
		// DAT_00457910 = sprite position X (relative to center)
		// DAT_00457912 = sprite position Y (relative to center)
		if (_grdSpriteMode == 1) {
			// Mode 1: Uncovered, shooting - sprite shifts left as damage increases
			_rebelViewMode1 = 0x0e;
			_rebelViewMode2 = 0;
			_rebelViewOffsetX = _rebelDamageLevel * -5 + -14;   // DAT_0045790c
			_rebelViewOffset2X = _rebelDamageLevel * -22;       // DAT_00457910
			_rebelViewOffsetY = 0;                              // DAT_0045790e
			_rebelViewOffset2Y = 0;                             // DAT_00457912
		} else if (_grdSpriteMode == 4) {
			// Mode 4: Alternative uncovered - sprite shifts right
			_rebelViewMode1 = 0x22;
			_rebelViewMode2 = 0;
			_rebelViewOffsetX = _rebelDamageLevel * 10 + -16;   // DAT_0045790c
			_rebelViewOffset2X = _rebelDamageLevel * 17 + -85;  // DAT_00457910 (0x11 = 17, -0x55 = -85)
			_rebelViewOffsetY = 0;
			_rebelViewOffset2Y = 0;
		} else if (_grdSpriteMode == 2) {
			// Mode 2: Covered - vertical shift
			_rebelViewMode1 = 0;
			_rebelViewMode2 = 0x0e;
			_rebelViewOffsetY = _rebelDamageLevel * -5 + -14;   // DAT_0045790e
			_rebelViewOffset2Y = (5 - _rebelDamageLevel) * 15 + -60;  // DAT_00457912 (0xf = 15, -0x3c = -60)
			_rebelViewOffsetX = 0;
			_rebelViewOffset2X = 0;
		} else if (_grdSpriteMode == 3) {
			// Mode 3: Transition - direction-dependent horizontal shift
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
			// Mode 0 or unknown: use Mode 1 defaults as fallback
			_rebelViewMode1 = 0x0e;
			_rebelViewMode2 = 0;
			_rebelViewOffsetX = _rebelDamageLevel * -5 + -14;
			_rebelViewOffset2X = _rebelDamageLevel * -22;
			_rebelViewOffsetY = 0;
			_rebelViewOffset2Y = 0;
			debug("Rebel2 Opcode 6 (Handler 25): Unknown mode %d, using Mode 1 fallback", _grdSpriteMode);
		}

		debug("Rebel2 Opcode 6 (Handler 25): mode=%d damage=%d dir=%d autopilot=%d viewOff=(%d,%d) spritePos=(%d,%d)",
			_grdSpriteMode, _rebelDamageLevel, _rebelFlightDir, _rebelAutopilot,
			_rebelViewOffsetX, _rebelViewOffsetY, _rebelViewOffset2X, _rebelViewOffset2Y);

		// Set FOBJ position offsets (FUN_00424510 in original, line 214)
		// All subsequent FOBJs in this frame will be shifted by these offsets
		if (_player) {
			_player->_fobjOffsetX = _rebelViewOffsetX;
			_player->_fobjOffsetY = _rebelViewOffsetY;
		}

		// Draw corridor overlay OPAQUELY (FUN_00428A10 in original, line 216)
		// This wipes previous frame content so codec 23 delta skip regions show clean corridor
		if (renderBitmap) {
			EmbeddedSanFrame &corridorOverlay = _rebelEmbeddedHud[4];
			if (corridorOverlay.valid && corridorOverlay.pixels) {
				int pitch = (_player && _player->_width > 0) ? _player->_width : 320;
				int bufHeight = (_player && _player->_height > 0) ? _player->_height : 200;

				int srcOffsetX = 0;
				int srcOffsetY = 0;
				int destX = _rebelViewOffsetX;
				int destY = _rebelViewOffsetY;
				int drawWidth = corridorOverlay.width;
				int drawHeight = corridorOverlay.height;

				if (destX < 0) { srcOffsetX = -destX; drawWidth -= srcOffsetX; destX = 0; }
				if (destY < 0) { srcOffsetY = -destY; drawHeight -= srcOffsetY; destY = 0; }
				if (destX + drawWidth > pitch)
					drawWidth = pitch - destX;
				if (destY + drawHeight > bufHeight)
					drawHeight = bufHeight - destY;
				if (drawWidth > corridorOverlay.width - srcOffsetX)
					drawWidth = corridorOverlay.width - srcOffsetX;
				if (drawHeight > corridorOverlay.height - srcOffsetY)
					drawHeight = corridorOverlay.height - srcOffsetY;

				if (drawWidth > 0 && drawHeight > 0) {
					for (int y = 0; y < drawHeight; y++) {
						memcpy(renderBitmap + (destY + y) * pitch + destX,
							   corridorOverlay.pixels + (srcOffsetY + y) * corridorOverlay.width + srcOffsetX,
							   drawWidth);
					}
				}
				debug("Rebel2 Opcode 6: Corridor overlay drawn at (%d,%d) size(%d,%d)",
					_rebelViewOffsetX, _rebelViewOffsetY, corridorOverlay.width, corridorOverlay.height);
			}
		}

		return;
	}

	// Step 1: If par4 == 1, initialize/reset state (lines 114-121)
	if (par4 == 1) {
		// Draw status bar sprite 5 (FUN_0040bb87 equivalent)
		_rebelStatusBarSprite = (_rebelLevelType == 5) ? 53 : 5;

		// Per-wave init: clear bits, links, reset wave state.
		// In the original game, FUN_00423880 runs ONCE at video-start callback
		// registration time, not per-frame. Guard with _rebelOp6Initialized so
		// this fires once per wave video (reset in procPreRendering at frame 0).
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
			debug("Rebel2 Opcode 6: Wave init - cleared bits/links, waveState=0x%x", _rebelWaveState);
		}
	}

	// Step 2: Set level type (DAT_00457900 = par3)
	_rebelLevelType = par3;

	// Step 3: Autopilot/control mode logic (lines 123-146)
	// This determines whether the ship flies on autopilot or manual control
	if (!_rebelInvulnerable) {
		// Normal mode: check control mode flags
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
		// Invulnerable mode: random autopilot changes
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

	// Step 4: Update damage level counter (lines 147-154)
	if (_rebelAutopilot == 0) {
		if (_rebelDamageLevel > 0) {
			_rebelDamageLevel--;
		}
	} else {
		if (_rebelDamageLevel < 5) {
			_rebelDamageLevel++;
		}
	}

	// Handle level type 3 special direction logic (lines 155-181)
	if (_rebelLevelType == 3) {
		if (_rebelDamageLevel == 5) {
			// Check for joystick/key input to change direction
			// Simplified: use mouse position
			if (_vm->_mouse.x > 75) {
				_rebelFlightDir = 1;
			}
			if (_vm->_mouse.x < -75) {
				_rebelFlightDir = 0;
			}
		}
	} else {
		_rebelFlightDir = 0;
	}

	// Step 5: Calculate view offsets based on level type (lines 182-213)
	switch (_rebelLevelType) {
	case 1:
		// Type 1: Vertical movement
		_rebelViewMode1 = 0x0e;
		_rebelViewMode2 = 0;
		_rebelViewOffsetX = _rebelDamageLevel * -5 - 0x0e;
		_rebelViewOffset2X = _rebelDamageLevel * -0x16;
		_rebelViewOffsetY = 0;
		_rebelViewOffset2Y = 0;
		break;

	case 4:
		// Type 4: Different vertical movement
		_rebelViewMode1 = 0x22;
		_rebelViewMode2 = 0;
		_rebelViewOffsetX = _rebelDamageLevel * 10 - 0x10;
		_rebelViewOffset2X = _rebelDamageLevel * 0x11 - 0x55;
		_rebelViewOffsetY = 0;
		_rebelViewOffset2Y = 0;
		break;

	case 2:
		// Type 2: Horizontal movement
		_rebelViewMode1 = 0;
		_rebelViewMode2 = 0x0e;
		_rebelViewOffsetY = _rebelDamageLevel * -5 - 0x0e;
		_rebelViewOffset2Y = (5 - _rebelDamageLevel) * 0x0f - 0x3c;
		_rebelViewOffsetX = 0;
		_rebelViewOffset2X = 0;
		break;

	case 3:
		// Type 3: Direction-based movement
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
		// Default: No special offsets
		_rebelViewMode1 = 0;
		_rebelViewMode2 = 0;
		_rebelViewOffsetX = 0;
		_rebelViewOffsetY = 0;
		_rebelViewOffset2X = 0;
		_rebelViewOffset2Y = 0;
		break;
	}

	debug("Rebel2 Opcode 6: levelType=%d autopilot=%d damageLevel=%d viewOffset=(%d,%d)",
		_rebelLevelType, _rebelAutopilot, _rebelDamageLevel, _rebelViewOffsetX, _rebelViewOffsetY);

	// Detect and load embedded ANIM (SAN) within the remaining IACT payload
	// Note: chunkSize is the remaining IACT payload size after par1-par4 header
	{
		int64 startPos = b.pos();
		// Use chunkSize (remaining IACT payload) rather than b.size() (entire FRME stream)
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
						// Limit to remaining IACT payload (chunkSize - offset into payload)
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
}

void InsaneRebel2::iactRebel2Opcode8(byte *renderBitmap, Common::SeekableReadStream &b, int32 chunkSize, int16 par2, int16 par3, int16 par4) {
	// Opcode 8: HUD/Ship resource loading
	// Dispatches to handler-specific loading functions based on current handler and parameters.
	//
	// Handler-specific routing (based on retail disassembly):
	//   Handler 7 (FUN_0040c3cc):  FLY NUT sprites via par4 (1, 2, 3, 11)
	//   Handler 8 (FUN_00401234):  POV NUT sprites via par3 (1, 3, 6, 7) or background via par4=5
	//   Handler 0x26 (FUN_00407fcb): Turret HUD NUT via par3 (1-4)
	//   Handler 0x19: Mixed turret mode, similar to 0x26
	//
	// Sound loading: par3 in range 21-47

	debug("Rebel2 IACT Opcode 8: handler=%d par2=%d par3=%d par4=%d (gameState=%d)",
		_rebelHandler, par2, par3, par4, _gameState);

	int64 startPos = b.pos();
	int64 remaining = (chunkSize > 0) ? chunkSize : (b.size() - startPos);

	// ===== Handler 7: FLY NUT Loading (Third-Person Ship) =====
	// FUN_0040c3cc case 6: par4 determines FLY sprite slot
	bool isHandler7FLY = (_rebelHandler == 7 && (par4 == 1 || par4 == 2 || par4 == 3 || par4 == 11));
	if (isHandler7FLY && remaining >= 14) {
		if (loadHandler7FlySprites(b, remaining, par4)) {
			b.seek(startPos);
			return;
		}
		b.seek(startPos);
	}

	// ===== Sound Loading (par3 21-47) =====
	if (par3 >= 21 && par3 <= 47) {
		debug("Rebel2 Opcode 8: Sound loading subcase %d (not implemented)", par3);
		// TODO: Implement sound loading via FUN_004118df equivalent
		return;
	}

	// ===== Scan for embedded ANIM data =====
	// Remaining handlers require finding ANIM tag in the stream
	debug("Rebel2 Opcode 8: Scanning for ANIM tag (startPos=%lld remaining=%lld)",
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
	debug("Rebel2 Opcode 8: Read %d bytes for ANIM scan", bytesRead);

	// Find ANIM tag
	int animOffset = -1;
	for (int i = 0; i + 8 <= bytesRead; ++i) {
		if (READ_BE_UINT32(scanBuf + i) == MKTAG('A','N','I','M')) {
			animOffset = i;
			debug("Rebel2 Opcode 8: Found ANIM at offset %d", i);
			break;
		}
	}

	if (animOffset < 0) {
		debug("Rebel2 Opcode 8: No ANIM tag found");
		free(scanBuf);
		b.seek(startPos);
		return;
	}

	// Extract ANIM data
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

	bool handled = false;

	// ===== Handler 0x26/0x19: Turret HUD Overlays =====
	// FUN_00407fcb case 8: par3 1-4 for HUD NUT loading
	if (!handled && (_rebelHandler == 0x26 || _rebelHandler == 0x19)) {
		if (par3 >= 1 && par3 <= 4) {
			handled = loadTurretHudOverlay(animData, animDataSize, par3);
		}
	}

	// ===== Handler 8: POV Ship Sprites or Background =====
	// FUN_00401234 case 6: par4 selects POV NUT type (1,3,6,7) or background (5)
	// NOTE: par3 is always 0 for Handler 8; par4 contains the actual sprite type
	if (!handled && _rebelHandler == 8) {
		// Check for background loading first (par4=5)
		if (par4 == 5) {
			handled = loadLevel2Background(animData, animDataSize, renderBitmap);
		}
		// Check for POV NUT sprites (par4=1,3,6,7)
		else if (par4 == 1 || par4 == 3 || par4 == 6 || par4 == 7) {
			handled = loadHandler8ShipSprites(animData, animDataSize, par4);
		}
	}

	// ===== Handler 25 (0x19): Level 2 GRD Ship Sprites and Background =====
	// FUN_0041cadb case 6 (opcode 8): Uses PAR4 for switch selection
	//   par4=1: GRD001 - Primary ship sprite -> DAT_00482240 / _grd001Sprite
	//   par4=2: GRD002 - Secondary ship sprite -> DAT_00482238 / _grd002Sprite
	//   par4=4: 350x230 corridor overlay -> DAT_00482268, draws immediately
	//   par4=5: 320x200 background -> DAT_0048226c
	//   par4=6: Overlay -> DAT_00482250, draws immediately
	//   par4=7: Overlay -> DAT_00482248, draws immediately
	if (!handled && _rebelHandler == 25) {
		if (par4 == 1 || par4 == 2) {
			// GRD ship sprites - load into NutRenderer for per-frame rendering
			handled = loadHandler25GrdSprites(animData, animDataSize, par4);
		} else if (par4 == 5) {
			// Background (320x200) - stored for per-frame restoration
			handled = loadLevel2Background(animData, animDataSize, renderBitmap);
		} else if (par4 == 4 || par4 == 6 || par4 == 7) {
			// Overlays - draw immediately to renderBitmap
			// These complete the visual scene along with the background
			debug("Rebel2 Opcode 8: Handler 25 overlay par4=%d - drawing to screen", par4);
			loadEmbeddedSan(par4, animData, animDataSize, renderBitmap);
			handled = true;
		}
	}

	// ===== Fallback: Embedded SAN HUD overlays =====
	// For other cases, load as embedded SAN frame to HUD overlay slots
	if (!handled) {
		// Skip high-res data (par3 == 2, 4)
		if (par3 == 2 || par3 == 4) {
			debug("Rebel2 Opcode 8: Skipping high-res HUD par3=%d", par3);
			handled = true;
		} else {
			// Determine userId: Handler 0x19 uses par3, others use par4
			// Heuristic: if par3 is valid GRD range (1-13) and par4 is invalid, prefer par3
			int userId;
			bool usePar3 = (_rebelHandler == 0x19);
			if (!usePar3 && par3 >= 1 && par3 <= 13 && (par4 <= 0 || par4 >= 1000)) {
				usePar3 = true;
			}
			userId = usePar3 ? par3 : par4;

			// Skip audio tracks (userId >= 1000)
			if (userId > 0 && userId < 1000) {
				debug("Rebel2 Opcode 8: Loading embedded SAN HUD userId=%d (handler=%d par3=%d par4=%d)",
					userId, _rebelHandler, par3, par4);
				loadEmbeddedSan(userId, animData, animDataSize, renderBitmap);
				handled = true;
			}
		}
	}

	if (!handled) {
		debug("Rebel2 Opcode 8: Unhandled case - handler=%d par3=%d par4=%d", _rebelHandler, par3, par4);
	}

	free(animData);
	free(scanBuf);
	b.seek(startPos);
}

// ======================= Opcode 8 Helper Functions =======================
// These helper functions are extracted from the original monolithic iactRebel2Opcode8
// to improve code readability and match the retail FUN_* function structure.

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

	debug("Rebel2 loadHandler7FlySprites: header bytes: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
		header[0], header[1], header[2], header[3], header[4],
		header[5], header[6], header[7], header[8], header[9]);

	// Size is at offset 14 from IACT start = bytes 6-9 of our header buffer
	uint32 nutSize = READ_LE_UINT32(header + 6);
	debug("Rebel2 loadHandler7FlySprites: par4=%d nutSize=%u remaining=%lld",
		par4, nutSize, (long long)remaining);

	if (nutSize == 0 || nutSize > (uint32)(remaining - 10)) {
		return false;
	}

	byte *nutData = (byte *)malloc(nutSize);
	if (!nutData) {
		return false;
	}

	int bytesRead = b.read(nutData, nutSize);
	debug("Rebel2 loadHandler7FlySprites: Read %d/%u bytes, first 16: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
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

	// Load as NUT
	NutRenderer *newNut = new NutRenderer(_vm, nutData, bytesRead);
	if (!newNut || newNut->getNumChars() <= 0) {
		debug("Rebel2 loadHandler7FlySprites: NUT load failed for par4=%d", par4);
		delete newNut;
		free(nutData);
		return false;
	}

	debug("Rebel2 loadHandler7FlySprites: Loaded FLY NUT par4=%d with %d sprites",
		par4, newNut->getNumChars());

	// Assign to appropriate slot based on par4 (matches FUN_0040c3cc case 6 switch)
	bool assigned = true;
	switch (par4) {
	case 1:  // FLY001 - Ship direction sprites (35 frames)
		delete _flyShipSprite;
		_flyShipSprite = newNut;
		debug("Rebel2: _flyShipSprite set with %d sprites", newNut->getNumChars());
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

bool InsaneRebel2::loadTurretHudOverlay(byte *animData, int32 size, int16 par3) {
	// Handler 0x26/0x19 turret HUD overlay loading - FUN_00407fcb case 8
	// Resolution-dependent loading:
	//   par3 == 1: Low-res primary HUD (DAT_0047fe78 / _hudOverlayNut)
	//   par3 == 2: High-res primary HUD (skip in 320x200 mode)
	//   par3 == 3: Low-res secondary HUD (DAT_0047fe80 / _hudOverlay2Nut)
	//   par3 == 4: High-res secondary HUD (skip in 320x200 mode)

	if (!animData || size <= 0) {
		return false;
	}

	// ScummVM runs at 320x200 (low-res), skip high-res data
	if (par3 == 2 || par3 == 4) {
		debug("Rebel2 loadTurretHudOverlay: Skipping high-res HUD par3=%d (running in low-res mode)", par3);
		return true;  // Successfully "handled" by skipping
	}

	if (par3 != 1 && par3 != 3) {
		return false;  // Not a turret HUD slot
	}

	NutRenderer *newNut = new NutRenderer(_vm, animData, size);
	if (!newNut || newNut->getNumChars() <= 0) {
		debug("Rebel2 loadTurretHudOverlay: NUT load failed for par3=%d", par3);
		delete newNut;
		return false;
	}

	debug("Rebel2 loadTurretHudOverlay: Loaded turret HUD NUT par3=%d with %d sprites",
		par3, newNut->getNumChars());

	if (par3 == 1) {
		// Low-res primary HUD overlay
		delete _hudOverlayNut;
		_hudOverlayNut = newNut;
	} else {  // par3 == 3
		// Low-res secondary HUD overlay
		delete _hudOverlay2Nut;
		_hudOverlay2Nut = newNut;
	}

	return true;
}

bool InsaneRebel2::loadHandler8ShipSprites(byte *animData, int32 size, int16 par4) {
	// Handler 8 ship POV NUT loading - FUN_00401234 case 6 (opcode 8)
	// par4 values (from IACT data offset +6, NOT par3 which is always 0):
	//   1: POV001 - Primary ship sprite (DAT_0047e010 / _shipSprite)
	//   3: POV004 - Secondary ship sprite (DAT_0047e028 / _shipSprite2)
	//   6: POV002 - Ship overlay 1 (DAT_0047e020 / _shipOverlay1)
	//   7: POV003 - Ship overlay 2 (DAT_0047e018 / _shipOverlay2)

	if (!animData || size <= 0) {
		return false;
	}

	// Only handle valid POV sprite slots
	if (par4 != 1 && par4 != 3 && par4 != 6 && par4 != 7) {
		return false;
	}

	NutRenderer *newNut = new NutRenderer(_vm, animData, size);
	if (!newNut || newNut->getNumChars() <= 0) {
		debug("Rebel2 loadHandler8ShipSprites: NUT load failed for par4=%d", par4);
		delete newNut;
		return false;
	}

	debug("Rebel2 loadHandler8ShipSprites: Loaded ship NUT par4=%d with %d sprites",
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
	case 6:  // POV002 - Ship overlay 1
		delete _shipOverlay1;
		_shipOverlay1 = newNut;
		break;
	case 7:  // POV003 - Ship overlay 2
		delete _shipOverlay2;
		_shipOverlay2 = newNut;
		break;
	default:
		delete newNut;
		return false;
	}

	return true;
}

bool InsaneRebel2::loadHandler25GrdSprites(byte *animData, int32 size, int16 par4) {
	// Handler 25 GRD ship NUT loading - FUN_0041cadb case 6 (opcode 8)
	// par4 values (from IACT data offset +6):
	//   1: GRD001 - Primary ship sprite (DAT_00482240 / _grd001Sprite)
	//   2: GRD002 - Secondary ship sprite (DAT_00482238 / _grd002Sprite)

	if (!animData || size <= 0) {
		return false;
	}

	// Only handle valid GRD sprite slots
	if (par4 != 1 && par4 != 2) {
		return false;
	}

	NutRenderer *newNut = new NutRenderer(_vm, animData, size);
	if (!newNut || newNut->getNumChars() <= 0) {
		debug("Rebel2 loadHandler25GrdSprites: NUT load failed for par4=%d", par4);
		delete newNut;
		return false;
	}

	debug("Rebel2 loadHandler25GrdSprites: Loaded GRD NUT par4=%d with %d sprites",
		par4, newNut->getNumChars());

	switch (par4) {
	case 1:  // GRD001 - Primary ship sprite
		delete _grd001Sprite;
		_grd001Sprite = newNut;
		debug("Rebel2: _grd001Sprite set with %d sprites", newNut->getNumChars());
		break;
	case 2:  // GRD002 - Secondary ship sprite
		delete _grd002Sprite;
		_grd002Sprite = newNut;
		debug("Rebel2: _grd002Sprite set with %d sprites", newNut->getNumChars());
		break;
	default:
		delete newNut;
		return false;
	}

	return true;
}

bool InsaneRebel2::loadLevel2Background(byte *animData, int32 size, byte *renderBitmap) {
	// Level 2 background loading from embedded ANIM - FUN_00401234 case 5
	// par4=5 contains the background image embedded as ANIM with FOBJ codec 3
	// Creates 320x200 buffer (DAT_0047e030 / _level2Background)

	if (!animData || size < 8) {
		return false;
	}

	debug("Rebel2 loadLevel2Background: Loading Level 2 background (animSize=%d)", size);

	// Allocate background buffer if needed (320x200 = 64000 bytes)
	if (_level2Background == nullptr) {
		_level2Background = (byte *)malloc(320 * 200);
		if (!_level2Background) {
			return false;
		}
		memset(_level2Background, 0, 320 * 200);
	}

	// Parse embedded ANIM to find FOBJ
	// Structure: ANIM tag at offset 0, AHDR, then FRME with FOBJ
	int animOffset = 0;
	if (READ_BE_UINT32(animData) == MKTAG('A','N','I','M')) {
		uint32 animSize = READ_BE_UINT32(animData + 4);
		debug("Rebel2 loadLevel2Background: Found ANIM tag, size=%u", animSize);

		// Skip ANIM header (8 bytes) + AHDR chunk
		if (size >= 16 && READ_BE_UINT32(animData + 8) == MKTAG('A','H','D','R')) {
			uint32 ahdrSize = READ_BE_UINT32(animData + 12);
			animOffset = 8 + 8 + ahdrSize;  // After ANIM tag + AHDR
			debug("Rebel2 loadLevel2Background: AHDR size=%u, FRME expected at offset %d", ahdrSize, animOffset);
		}
	}

	// Look for FRME containing FOBJ
	bool foundBackground = false;
	for (int scanPos = animOffset; scanPos + 16 < size && !foundBackground; scanPos++) {
		if (READ_BE_UINT32(animData + scanPos) == MKTAG('F','R','M','E')) {
			int frmeSize = READ_BE_UINT32(animData + scanPos + 4);
			debug("Rebel2 loadLevel2Background: Found FRME at %d, size=%d", scanPos, frmeSize);

			for (int fobjPos = scanPos + 8; fobjPos + 18 < scanPos + 8 + frmeSize && fobjPos + 18 < size; fobjPos++) {
				if (READ_BE_UINT32(animData + fobjPos) == MKTAG('F','O','B','J')) {
					byte *fobjData = animData + fobjPos + 8;

					// FOBJ header: codec(2), x(2), y(2), w(2), h(2)
					int16 codec = READ_LE_INT16(fobjData);
					int16 fobjX = READ_LE_INT16(fobjData + 2);
					int16 fobjY = READ_LE_INT16(fobjData + 4);
					int16 fobjW = READ_LE_INT16(fobjData + 6);
					int16 fobjH = READ_LE_INT16(fobjData + 8);

					debug("Rebel2 loadLevel2Background: Found FOBJ: codec=%d pos=(%d,%d) size=%dx%d",
						codec, fobjX, fobjY, fobjW, fobjH);

					// Decode codec 3 (RLE) into background buffer
					// Use smushDecodeRLEOpaque to write ALL colors including color 0 (black).
					// The standard smushDecodeRLE treats color 0 as transparent, which causes
					// the background to appear as a "sketch" with black pixels missing.
					if (codec == 3 && fobjW > 0 && fobjH > 0 && fobjW <= 320 && fobjH <= 200) {
						byte *rleData = fobjData + 14;  // Skip full 14-byte FOBJ header
						smushDecodeRLEOpaque(_level2Background, rleData, fobjX, fobjY, fobjW, fobjH, 320);

						debug("Rebel2 loadLevel2Background: Decoded Level 2 background (%dx%d at %d,%d)",
							fobjW, fobjH, fobjX, fobjY);
						_level2BackgroundLoaded = true;
						foundBackground = true;

						// Copy to render bitmap immediately if provided
						if (renderBitmap) {
							for (int by = 0; by < 200; by++) {
								memcpy(renderBitmap + by * 320, _level2Background + by * 320, 320);
							}
							debug("Rebel2 loadLevel2Background: Copied to renderBitmap");
						}
					}
					break;
				}
			}
			break;
		}
	}

	if (!foundBackground) {
		debug("Rebel2 loadLevel2Background: Failed to find/decode background FOBJ");
	}

	return foundBackground;
}

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
			debug("Rebel2 Opcode 9: TRES string index=%d -> \"%s\"", stringIndex, textStr);
		} else {
			debug("Rebel2 Opcode 9: TRES string index=%d not found", stringIndex);
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

		debug("Rebel2 Opcode 9: Inline text at (%d,%d) flags=0x%x -> \"%s\"", posX, posY, textFlags, textStr);
	}

	if (!textStr || textStr[0] == '\0') {
		debug("Rebel2 Opcode 9: Empty text string, skipping");
		return;
	}

	// Check difficulty gate (flag bit 3 = 0x08)
	// If set, only show text if difficulty check passes (we skip this check for simplicity)
	// In retail: FUN_00425d30(0) is called

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
		debug("Rebel2 Opcode 9: No message font loaded (_rebelMsgFont is null)");
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

	// Draw the text string (with converted character indices)
	if (textFlags & 0x04) {
		// Word-wrapped text
		_rebelMsgFont->drawStringWrap(convertedText, renderBitmap, clipRect, posX, posY, textColor, styleFlags);
	} else {
		// Single-line text
		_rebelMsgFont->drawString(convertedText, renderBitmap, clipRect, posX, posY, textColor, styleFlags);
	}

	debug("Rebel2 Opcode 9: Rendered subtitle at (%d,%d) flags=0x%x clip=(%d,%d,%d,%d)",
		posX, posY, textFlags, clipX, clipY, clipW, clipH);
}

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

	// If disabled, stop processing this object
	if (disabled) {
		// debug("Rebel2: Skipping Opcode 4 for disabled enemy ID=%d", enemyId);
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
	debug(5, "Rebel2 Opcode4: handler=%d enemyId=%d par2=%d par3=%d par4/type=%d pos=(%d,%d) size=(%d,%d)",
		_rebelHandler, enemyId, par2, par3, par4, x, y, w, h);

	bool found = false;
	Common::List<enemy>::iterator it;
	for (it = _enemies.begin(); it != _enemies.end(); ++it) {
		if (it->id == enemyId) {
			it->rect = Common::Rect(x, y, x + w, y + h);
			it->type = par4;  // Enemy type from IACT offset +6 (userId)
			// Only re-activate if not destroyed
			if (!it->destroyed) {
				it->active = true;
			}
			found = true;
			break;
		}
	}
	if (!found) {
		init_enemyStruct(enemyId, x, y, w, h, true, false, -1, par4);
	}
}

void InsaneRebel2::init_enemyStruct(int id, int32 x, int32 y, int32 w, int32 h, bool active, bool destroyed, int32 explosionFrame, int type) {
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
