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

void InsaneRebel2::procPreRendering(byte *renderBitmap) {
	Insane::procPreRendering(renderBitmap);

	updateGameplayAimFromGamepad();

	// Seamless continuation segments keep opcode 6 state across video loops.
	if (_player && _player->_frame == 0) {
		const bool shieldContinuation = _rebelShieldGateActive && (_player->_curVideoFlags & 0x40) != 0;
		if (!shieldContinuation)
			_rebelOp6Initialized = false;
	}

	// Level 2 sprites rely on a restored 320-wide background before FOBJ decoding.
	if (_rebelHandler == 8 && _level2BackgroundLoaded && _level2Background && renderBitmap) {
		int bufferPitch = (_player && _player->_width > 0) ? _player->_width : 320;
		if (bufferPitch == 320) {
			for (int y = 0; y < 200; y++) {
				memcpy(renderBitmap + y * 320, _level2Background + y * 320, 320);
			}
		}
	}

	// Chapter previews are selected by scrolling the O_LEVEL.SAN FOBJ strip.
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

void InsaneRebel2::procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					  int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
					  int16 par1, int16 par2, int16 par3, int16 par4) {
	debugC(DEBUG_INSANE, "IACT: opcode=%d par2=%d par3=%d par4=%d gameState=%d sceneId=%d",
		par1, par2, par3, par4, _gameState, _currSceneId);

	if (_keyboardDisable)
		return;

	if (_gameState == kStateMainMenu && par1 == 8 && par4 >= 1000) {
		debugC(DEBUG_INSANE, "IACT: Menu mode - processing embedded ANIM (userId=%d)", par4);

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

void InsaneRebel2::iactRebel2Scene1(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				  int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
				  int16 par1, int16 par2, int16 par3, int16 par4) {
	if (par1 == 4) {
		enemyUpdate(renderBitmap, b, par2, par3, par4);
	} else if (par1 == 2) {
		iactRebel2Opcode2(b, par2, par3, par4);
	} else if (par1 == 3) {
		iactRebel2Opcode3(b, par2, par3, par4);
	}
	else if (par1 == 5) {
		debugC(DEBUG_INSANE, "IACT Opcode 5: par2=%d par3=%d par4=%d", par2, par3, par4);

		if (par2 == 0x0D || par2 == 0x0E) {
			registerCollisionZone(b, par2, par4);
		}

	} else if (par1 == 7) {
		int16 body0 = 0, body1 = 0;
		if (b.size() - b.pos() >= 4) {
			body0 = b.readSint16LE();
			body1 = b.readSint16LE();
		}

		switch (par4) {
		case 0:
			_windParamX = body0;
			_windParamY = body1;
			debugC(DEBUG_INSANE, "Opcode 7 par4=0: wind=(%d,%d)", body0, body1);
			break;
		case 1:
			_corridorLeftX = body0;
			_corridorTopY = body1;
			if (_flyControlMode == 2) {
				_corridorLeftX += 15;
			} else if (_flyControlMode == 0) {
				_corridorLeftX += 20;
			}
			debugC(DEBUG_INSANE, "Opcode 7 par4=1: corridor left=%d top=%d (adjusted left=%d)",
				body0, body1, _corridorLeftX);
			break;
		case 2:
			_corridorRightX = body0;
			_corridorBottomY = body1;
			if (_flyControlMode == 2) {
				_corridorRightX -= 15;
			} else if (_flyControlMode == 0) {
				_corridorRightX -= 20;
			}
			debugC(DEBUG_INSANE, "Opcode 7 par4=2: corridor right=%d bottom=%d (adjusted right=%d)",
				body0, body1, _corridorRightX);
			break;
		case 5:
			_flyOverlayRepeatCount = body0;
			debugC(DEBUG_INSANE, "Opcode 7 par4=5: flyOverlayRepeat=%d", _flyOverlayRepeatCount);
			break;
		default:
			debugC(DEBUG_INSANE, "Opcode 7 par4=%d: body=(%d,%d) — unknown sub-opcode", par4, body0, body1);
			break;
		}

	} else if (par1 == 6) {
		iactRebel2Opcode6(renderBitmap, b, size, par2, par3, par4);
	} else if (par1 == 8) {
		iactRebel2Opcode8(renderBitmap, b, size, par2, par3, par4);
	} else if (par1 == 9) {
		iactRebel2Opcode9(renderBitmap, b, par2, par3, par4);
	} else if (par1 == 0 || par1 == 1) {
		debugC(DEBUG_INSANE, "IACT: Low Opcode %d (par2=%d par3=%d par4=%d)", par1, par2, par3, par4);
	} else {
		debugC(DEBUG_INSANE, "IACT: Unknown Opcode %d (par2=%d par3=%d par4=%d)", par1, par2, par3, par4);
	}
}

void InsaneRebel2::iactRebel2Opcode2(Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4) {
	if (par3 == 4) {
		int16 childId = b.readSint16LE();
		int16 parentId = b.readSint16LE();

		if (parentId >= 1 && parentId < 512 && childId >= 1 && childId < 512) {
			_rebelLinks[parentId][2] = _rebelLinks[parentId][1];
			_rebelLinks[parentId][1] = _rebelLinks[parentId][0];
			_rebelLinks[parentId][0] = childId;

			// Linked children are hidden while the parent is alive.
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
	} else if (par3 == 1) {
		int16 value = par4;
		int16 targetId = b.readSint16LE();

		if (targetId < 1 || targetId >= 0x200)
			return;

		if ((_rebelHandler == 8 || _rebelHandler == 25) && value != 0) {
			if (value == 100) {
				clearBit(targetId);
				debugC(DEBUG_INSANE, "Opcode2 (H%d): Force ENABLE target=%d (par4=100)", _rebelHandler, targetId);
			} else {
				int bitMask = 1 << (value & 0x1f);
				if ((_rebelWaveState & bitMask) != 0) {
					setBit(targetId);
					debugC(DEBUG_INSANE, "Opcode2 (H%d): Disable target=%d (type %d killed, wave=0x%x)", _rebelHandler, targetId, value, _rebelWaveState);
				}
			}
			return;
		}

		if (value > 1 && value < 10) {
			if (_vm->_rnd.getRandomNumber(value) == 0) {
				setBit(targetId);
				debugC(DEBUG_INSANE, "IACT Opcode2: Random DISABLE target=%d (value=%d)", targetId, value);
			}
		} else if (value > 10 && value < 20) {
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
		} else if (value > 99 && value < 110) {
			int idx = value - 100;
			if (!isBitSet(targetId)) {
				_rebelValueCounters[idx]++;
				debugC(DEBUG_INSANE, "IACT Opcode2: Increment counter[%d] -> %d (target=%d)", idx, _rebelValueCounters[idx], targetId);
			}
			_rebelLastCounter = _rebelValueCounters[idx];
			if (_rebelShieldGateActive) {
				_rebelGaugeArmed = true;
				_rebelLastArmedSlot = idx;
			}
		} else if (value > 0x3ff) {
			for (int slot = 1; slot <= 9; ++slot) {
				if ((value & (1 << (slot - 1))) != 0) {
					if (!isBitSet(targetId)) {
						_rebelValueCounters[slot]++;
						debugC(DEBUG_INSANE, "IACT Opcode2: Increment counter[%d] (mask 0x%x) -> %d (target=%d)", slot, value, _rebelValueCounters[slot], targetId);
					}
					_rebelLastCounter = _rebelValueCounters[slot];
					if (_rebelShieldGateActive) {
						_rebelGaugeArmed = true;
						_rebelLastArmedSlot = slot;
					}
				}
			}
		}

		debugC(DEBUG_INSANE, "IACT Opcode2: Unhandled par3=%d par4=%d", par3, par4);
	}
}

void InsaneRebel2::iactRebel2Opcode3(Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4) {
	if (_rebelHandler == 25) {
		int16 srcIdBody0 = b.readSint16LE();
		int16 srcIdBody1 = b.readSint16LE();

		if (par3 == 5) {
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
			_rebelHitCounter++;
			debugC(DEBUG_INSANE, "H25 hit counter++ -> %d (par3=1 par4=%d, no damage)",
				_rebelHitCounter, par4);
		} else {
			debugC(DEBUG_INSANE, "Opcode3: H25 par3=%d par4=%d (no action)", par3, par4);
		}

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
					_rebelDeathCause = 1;
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
		b.skip(2);
		int16 srcId = b.readSint16LE();

		debugC(DEBUG_INSANE, "Opcode3: par3=5 srcId=%d isBitSet=%d", srcId, isBitSet(srcId));

		if (!isBitSet(srcId)) {
			LevelDifficultyParams params = getDifficultyParams();
			int probability = (params.shotAccuracy >= 0) ? params.shotAccuracy : 0;

			int roll = _vm->_rnd.getRandomNumber(99);
			debugC(DEBUG_INSANE, "Opcode3: probability=%d roll=%d (need roll < prob)", probability, roll);

			if (roll < probability) {
				_rebelDeathCause = 0;
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

void InsaneRebel2::updateOpcode6Handler(int16 par2) {
	if (par2 == 7 || par2 == 8 || par2 == 0x19 || par2 == 0x26) {
		if (_rebelHandler == 8 && par2 != 8) {
			_level2BackgroundLoaded = false;
		}
		_rebelHandler = par2;
		debugC(DEBUG_INSANE, "Opcode 6: Setting handler=%d", par2);
	}
}

void InsaneRebel2::handleOpcode6Handler8(Common::SeekableReadStream &b, int16 par4) {
	_shipLevelMode = par4;

	int16 bodyStatusFlag = 0;
	if (b.pos() + 2 <= b.size()) {
		int64 savedPos = b.pos();
		bodyStatusFlag = b.readSint16LE();
		b.seek(savedPos);
	}

	if (bodyStatusFlag == 1) {
		_rebelStatusBarSprite = 5;
		if (_smush_iconsNut && _smush_iconsNut->getNumChars() > 5) {
			initLaserTexture(_smush_iconsNut, 5);
		}
	}

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

	if (_shipLevelMode != 4 && _shipLevelMode != 5) {
		// Cover and shooting states ease toward different movement ranges.
		if (_shipLevelMode == 2) {
			if (_movementRangeLimit > 41) {
				_movementRangeLimit -= 10;
			}
			if (_movementRangeLimit < 41) {
				_movementRangeLimit = 41;
			}
		} else {
			if (_movementRangeLimit < 127) {
				_movementRangeLimit += 10;
			}
			if (_movementRangeLimit > 127) {
				_movementRangeLimit = 127;
			}
		}

		Common::Point aimPos = getGameplayAimPoint();
		int16 mouseOffsetX = (int16)((aimPos.x - 160) * 127 / 160);
		int16 mouseOffsetY = (int16)((aimPos.y - 100) * 127 / 100);

		if (mouseOffsetX > _movementRangeLimit)
			mouseOffsetX = _movementRangeLimit;
		if (mouseOffsetX < -_movementRangeLimit)
			mouseOffsetX = -_movementRangeLimit;
		if (mouseOffsetY > 127)
			mouseOffsetY = 127;
		if (mouseOffsetY < -127)
			mouseOffsetY = -127;

		_shipTargetX = (int16)(((mouseOffsetX * 5 + 0x27b) * 0x40) / 0xfe);
		_shipTargetY = (int16)(((mouseOffsetY * 5 + 0x27b) * 0x10) / 0xfe);

		const int16 maxStep = 50;
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

		// Shift subsequent FOBJ weapon sprites after moving the handler-8 camera.
		if (_player) {
			_player->_fobjOffsetX = -_shipPosX;
			_player->_fobjOffsetY = -_shipPosY;
		}

		int16 mouseX = aimPos.x;
		int16 mouseY = aimPos.y;

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

void InsaneRebel2::handleOpcode6Handler7(Common::SeekableReadStream &b, int16 par4) {
	//   Mode 0: Obstacle avoidance - SECONDARY zones, corridor boundaries
	//   Mode 1: Tunnel flight - PRIMARY zones, per-edge push-back (hMargin=0x28)
	//   Mode 2: Combat mode - shooting ENABLED, SECONDARY zones
	//   Mode 3: Tunnel flight - PRIMARY zones, per-edge push-back (hMargin=0x0f)
	_flyControlMode = par4;
	debugC(DEBUG_INSANE, "Opcode 6 (Handler 7): Control mode set to %d (shooting %s)",
		par4, (par4 == 2) ? "ENABLED" : "DISABLED");

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

	const int flightParamIndex = CLIP(_rebelLevelType - 1, 0, 16);
	const LevelDifficultyParams &flightParams =
		kDifficultyTable[CLIP(_difficulty, 0, 5)][flightParamIndex];

	const Common::Point aimPos = getGameplayAimPoint();
	const int16 mouseX = aimPos.x;
	const int16 mouseY = aimPos.y;
	int16 inputX = (int16)(mouseX - 160);
	int16 inputY = (int16)(mouseY - 100);

	if (inputX > 160)
		inputX = 160;
	if (inputX < -160)
		inputX = -160;
	if (inputY > 127)
		inputY = 127;
	if (inputY < -127)
		inputY = -127;

	int16 scaledInputX = (int16)((inputX * 127) / 160);
	int16 scaledInputY = _optControlsFlipped ? (int16)-inputY : inputY;

	const bool useDirectGamepadFlight = _gamepadAimActive && _selectedLevel == 10;

	// Level 10 is the only Handler 7 stage that needs direct full-range
	// gamepad axes; other Handler 7 stages use bounded target steering to avoid
	// harsh perspective shifts from a held stick.
	if (!useDirectGamepadFlight) {
		scaledInputX = (int16)((scaledInputX * kRA2Handler7DirectInputNumerator) /
			kRA2Handler7DirectInputDenominator);
		scaledInputY = (int16)((scaledInputY * kRA2Handler7DirectInputNumerator) /
			kRA2Handler7DirectInputDenominator);
	}
	const bool useTargetSteering = !useDirectGamepadFlight;
	int16 mouseFlightTargetX = _flyShipScreenX;
	if (useTargetSteering) {
		mouseFlightTargetX = (int16)(0xd4 + (scaledInputX * kRA2Handler7MouseTargetRangeX) / 127);
		mouseFlightTargetX = CLIP<int16>(mouseFlightTargetX, 0x14, 0x194);
	}

	for (int i = 24; i > 0; i--) {
		_velocityHistory[i] = _velocityHistory[i - 1];
	}
	_velocityHistory[0] = scaledInputX;

	const int smoothWindow = 5;
	int velSum = 0;
	for (int i = 0; i < smoothWindow; i++) {
		velSum += _velocityHistory[i];
	}
	_smoothedVelocity = (int16)(velSum / smoothWindow);

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

	const int16 levelSpeed = flightParams.slideRate;
	const int16 levelYSpeed = flightParams.liftRate;
	int16 absSmoothVel = ABS(_smoothedVelocity);
	int16 positionDeltaX;

	if (_flyControlMode == 1) {
		// Banking: vertical input deflects horizontal movement.
		if (scaledInputX < 1) {
			positionDeltaX = (int16)((levelSpeed * _smoothedVelocity - absSmoothVel * scaledInputY - windEffectX) >> 9);
		} else {
			positionDeltaX = (int16)((levelSpeed * _smoothedVelocity + absSmoothVel * scaledInputY - windEffectX) >> 9);
		}
	} else {
		if (scaledInputX < 1) {
			positionDeltaX = (int16)((levelSpeed * _smoothedVelocity - (absSmoothVel * scaledInputY >> 2) - windEffectX) >> 9);
		} else {
			positionDeltaX = (int16)((levelSpeed * _smoothedVelocity + (absSmoothVel * scaledInputY >> 2) - windEffectX) >> 9);
		}
	}

	if (positionDeltaX < -11)
		positionDeltaX = -12;
	if (positionDeltaX > 11)
		positionDeltaX = 12;

	if (useTargetSteering) {
		int targetDeltaX = mouseFlightTargetX - _flyShipScreenX;
		const int targetSteeringDivisor = 4;
		positionDeltaX = (int16)CLIP<int>(targetDeltaX / targetSteeringDivisor, -12, 12);
		if (positionDeltaX == 0 && targetDeltaX != 0)
			positionDeltaX = (targetDeltaX < 0) ? -1 : 1;
	}

	_flyShipScreenX += positionDeltaX;

	if (_flyControlMode == 1) {
		int yCalc = levelYSpeed * scaledInputY - (windEffectY >> 1);
		int yDelta = yCalc >> 10;
		if (yDelta < -12)
			yDelta = -12;
		if (yDelta > 12)
			yDelta = 12;
		_flyShipScreenY -= (int16)yDelta;
	} else {
		_flyShipScreenY -= (int16)((levelYSpeed * scaledInputY) >> 10);
	}

	_verticalInput = scaledInputY;

	_facingRight = (0xd4 < _smoothedVelocity + _flyShipScreenX);

	if (_flyShipScreenX > 0x194)
		_flyShipScreenX = 0x194;  // 404
	if (_flyShipScreenY > 0xF0)
		_flyShipScreenY = 0xF0;    // 240
	if (_flyShipScreenX < 0x14)
		_flyShipScreenX = 0x14;    // 20
	if (_flyShipScreenY < 0x14)
		_flyShipScreenY = 0x14;    // 20

	if (_flyControlMode == 0 || _flyControlMode == 2) {
		LevelDifficultyParams wallParams = getDifficultyParams();
		int corridorWallDmg = (wallParams.dodgeDamage >= 0) ? wallParams.dodgeDamage : 0;

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
				playSfx(1, 127, 100);
			}
		}

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
				playSfx(1, 127, -100);
			}
		}

		if (_corridorBottomY < _flyShipScreenY) {
			_flyShipScreenY = _corridorBottomY;
		}
		if (_flyShipScreenY < _corridorTopY) {
			_flyShipScreenY = _corridorTopY;
		}
	}

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

	_viewShift = _smoothedVelocity;
	if (_viewShift > 127)
		_viewShift = 127;
	if (_viewShift < -127)
		_viewShift = -127;

	int16 vDir = (int16)(((int)(0xa0 - _verticalInput) + ((0xa0 - _verticalInput) < 0 ? 63 : 0)) >> 6);
	if (vDir < 0)
		vDir = 0;
	if (vDir > 4)
		vDir = 4;

	int16 hDir = (int16)((0x95 - _smoothedVelocity) / 0x2b);
	if (hDir < 0)
		hDir = 0;
	if (hDir > 6)
		hDir = 6;

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

void InsaneRebel2::handleOpcode6Handler25(byte *renderBitmap, Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4) {
	int16 par5 = 0;
	if (b.pos() + 2 <= b.size()) {
		int64 savedPos = b.pos();
		par5 = b.readSint16LE();
		b.seek(savedPos);  // Don't consume the stream
	}

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

	_grdSpriteMode = par4;

	debugC(DEBUG_INSANE, "Handler25 Opcode6: par2=%d par3=%d par4=%d(mode) par5=%d(reset) autopilot=%d damageLevel=%d controlMode=%d",
		par2, par3, par4, par5, _rebelAutopilot, _rebelDamageLevel, _rebelControlMode);

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

		_rebelControlMode = 0;
	} else {
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

	int prevDamageLevel = _rebelDamageLevel;
	if (_rebelAutopilot == 0) {
		if (_rebelDamageLevel > 0) {
			_rebelDamageLevel--;
		}
	} else {
		if (_rebelDamageLevel < 5) {
			_rebelDamageLevel++;
		}
	}
	if (_rebelDamageLevel != prevDamageLevel) {
		debugC(DEBUG_INSANE, "Handler25: damageLevel transition %d -> %d (autopilot=%d)",
			prevDamageLevel, _rebelDamageLevel, _rebelAutopilot);
	}

	if (_grdSpriteMode == 3) {
		if (_rebelDamageLevel == 5) {
			int16 mouseX = getGameplayAimPoint().x;
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

	if (_grdSpriteMode == 1) {
		_rebelViewMode1 = 0x0e;
		_rebelViewMode2 = 0;
		_rebelViewOffsetX = _rebelDamageLevel * -5 + -14;
		_rebelViewOffset2X = _rebelDamageLevel * -22;
		_rebelViewOffsetY = 0;
		_rebelViewOffset2Y = 0;
	} else if (_grdSpriteMode == 4) {
		_rebelViewMode1 = 0x22;
		_rebelViewMode2 = 0;
		_rebelViewOffsetX = _rebelDamageLevel * 10 + -16;
		_rebelViewOffset2X = _rebelDamageLevel * 17 + -85;
		_rebelViewOffsetY = 0;
		_rebelViewOffset2Y = 0;
	} else if (_grdSpriteMode == 2) {
		_rebelViewMode1 = 0;
		_rebelViewMode2 = 0x0e;
		_rebelViewOffsetY = _rebelDamageLevel * -5 + -14;
		_rebelViewOffset2Y = (5 - _rebelDamageLevel) * 15 + -60;
		_rebelViewOffsetX = 0;
		_rebelViewOffset2X = 0;
	} else if (_grdSpriteMode == 3) {
		_rebelViewMode1 = 0x0f;
		_rebelViewMode2 = 0;
		int16 dirMultX = (_rebelFlightDir == 0) ? 3 : -3;
		int16 dirMultX2 = (_rebelFlightDir == 0) ? 20 : -20;
		_rebelViewOffsetX = dirMultX * (5 - _rebelDamageLevel) + -15;
		_rebelViewOffset2X = dirMultX2 * (5 - _rebelDamageLevel);
		_rebelViewOffsetY = 0;
		_rebelViewOffset2Y = 0;
	} else {
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

	if (_player) {
		_player->_fobjOffsetX = _rebelViewOffsetX;
		_player->_fobjOffsetY = _rebelViewOffsetY;
	}

	drawHandler25CorridorOverlay(renderBitmap);
}

void InsaneRebel2::handleOpcode6Turret(Common::SeekableReadStream &b, int16 par4) {
	_rebelLevelType = par4;

	int16 par5 = 0;
	if (b.pos() + 2 <= b.size()) {
		int64 savedPos = b.pos();
		par5 = b.readSint16LE();
		b.seek(savedPos);
	}

	if (par5 == 1) {
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

void InsaneRebel2::updateOpcode6GenericFlightState() {
	if (!_rebelAutoPlay) {
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

	if (_rebelAutopilot == 0) {
		if (_rebelDamageLevel > 0) {
			_rebelDamageLevel--;
		}
	} else {
		if (_rebelDamageLevel < 5) {
			_rebelDamageLevel++;
		}
	}

	if (_rebelLevelType == 3) {
		if (_rebelDamageLevel == 5) {
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

	switch (_rebelLevelType) {
	case 1:
		_rebelViewMode1 = 0x0e;
		_rebelViewMode2 = 0;
		_rebelViewOffsetX = _rebelDamageLevel * -5 - 0x0e;
		_rebelViewOffset2X = _rebelDamageLevel * -0x16;
		_rebelViewOffsetY = 0;
		_rebelViewOffset2Y = 0;
		break;

	case 4:
		_rebelViewMode1 = 0x22;
		_rebelViewMode2 = 0;
		_rebelViewOffsetX = _rebelDamageLevel * 10 - 0x10;
		_rebelViewOffset2X = _rebelDamageLevel * 0x11 - 0x55;
		_rebelViewOffsetY = 0;
		_rebelViewOffset2Y = 0;
		break;

	case 2:
		_rebelViewMode1 = 0;
		_rebelViewMode2 = 0x0e;
		_rebelViewOffsetY = _rebelDamageLevel * -5 - 0x0e;
		_rebelViewOffset2Y = (5 - _rebelDamageLevel) * 0x0f - 0x3c;
		_rebelViewOffsetX = 0;
		_rebelViewOffset2X = 0;
		break;

	case 3:
		_rebelViewMode1 = 0x0f;
		_rebelViewMode2 = 0;
		{
			int dirFactor = (_rebelFlightDir == 0) ? 3 : -3;
			int dirFactor2 = (_rebelFlightDir == 0) ? 0x14 : -0x14;
			_rebelViewOffsetX = dirFactor * (5 - _rebelDamageLevel) - 0x0f;
			_rebelViewOffset2X = dirFactor2 * (5 - _rebelDamageLevel);
		}
		_rebelViewOffsetY = 0;
		_rebelViewOffset2Y = 0;
		break;

	default:
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

void InsaneRebel2::scanOpcode6EmbeddedAnim(byte *renderBitmap, Common::SeekableReadStream &b, int32 chunkSize, int16 par4) {
	int64 startPos = b.pos();

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

void InsaneRebel2::iactRebel2Opcode6(byte *renderBitmap, Common::SeekableReadStream &b, int32 chunkSize, int16 par2, int16 par3, int16 par4) {
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

bool InsaneRebel2::loadOpcode8Handler7FlySprites(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4) {
	const bool fly004ForResolution = (!isHiRes() && par4 == 10) || (isHiRes() && par4 == 11);
	if (_rebelHandler == 7 && (par4 == 10 || par4 == 11) && !fly004ForResolution) {
		b.seek(startPos);
		return true;
	}

	bool isHandler7FLY = (_rebelHandler == 7 && (par4 == 1 || par4 == 2 || par4 == 3 || fly004ForResolution));
	if (isHandler7FLY && remaining >= 14) {
		if (loadHandler7FlySprites(b, remaining, par4)) {
			b.seek(startPos);
			return true;
		}
		b.seek(startPos);
	}

	return false;
}

bool InsaneRebel2::loadOpcode8Handler7ShotTable(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4) {
	if (_rebelHandler != 7 || (par4 != 12 && par4 != 13))
		return false;

	if (loadHandler7ShotTable(b, startPos, remaining, par4)) {
		b.seek(startPos);
		return true;
	}

	b.seek(startPos);
	return false;
}

bool InsaneRebel2::loadOpcode8EdgeTable(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4) {
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

bool InsaneRebel2::loadOpcode8AuxSfx(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4) {
	//   0x1f-0x25 (31-37) -> aux buffer 1, 0x28 (40) -> aux buffer 3,
	//   0x29-0x2f (41-47) -> aux buffer 2
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
		b.seek(startPos + 6);
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

bool InsaneRebel2::loadOpcode8ShotOriginTable(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4) {
	if (_rebelHandler == 25 && par4 == 8) {
		if (loadHandler25ShotOriginTable(b, startPos, remaining)) {
			b.seek(startPos);
			return true;
		}
	}

	return false;
}

void InsaneRebel2::loadOpcode8EmbeddedAnim(byte *renderBitmap, Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par3, int16 par4) {
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

bool InsaneRebel2::handleOpcode8EmbeddedAnim(byte *renderBitmap, byte *animData, int32 animDataSize, int16 par3, int16 par4) {
	bool handled = false;

	if (!handled && _rebelHandler == 0x26) {
		int hudSelector = (par4 >= 1 && par4 <= 4) ? par4 : par3;

		if (hudSelector >= 1 && hudSelector <= 4) {
			handled = loadTurretHudOverlay(animData, animDataSize, hudSelector);
		}
	}

	// NOTE: par3 is always 0 for Handler 8; par4 contains the actual sprite type.
	if (!handled && _rebelHandler == 8) {
		if (par4 == 5) {
			handled = loadLevel2Background(animData, animDataSize, renderBitmap);
		} else if (par4 == 1 || par4 == 3 || par4 == 6 || par4 == 7) {
			handled = loadHandler8ShipSprites(animData, animDataSize, par4);
		}
	}

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

	if (!handled) {
		const bool highRes = isHiRes();
		const bool highResHud = (par3 == 2 || par3 == 4);
		const bool lowResHud = (par3 == 1 || par3 == 3);

		if ((!highRes && highResHud) || (highRes && lowResHud)) {
			debugC(DEBUG_INSANE, "Opcode 8: Skipping %s HUD par3=%d while running in %s mode",
				highResHud ? "high-res" : "low-res", par3, highRes ? "high-res" : "low-res");
			handled = true;
		} else {
			// Heuristic: if par3 is valid GRD range (1-13) and par4 is invalid, prefer par3.
			bool usePar3 = (_rebelHandler == 0x19);
			if (!usePar3 && par3 >= 1 && par3 <= 13 && (par4 <= 0 || par4 >= 1000)) {
				usePar3 = true;
			}
			int userId = usePar3 ? par3 : par4;

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

bool InsaneRebel2::loadHandler25ShotOriginTable(Common::SeekableReadStream &b, int64 startPos, int64 remaining) {
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
				value = 32768;
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

bool InsaneRebel2::loadHandler7ShotTable(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4) {
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

bool InsaneRebel2::loadHandler7FlySprites(Common::SeekableReadStream &b, int64 remaining, int16 par4) {
	if (remaining < 14) {
		return false;
	}

	byte header[10];
	if (b.read(header, 10) != 10) {
		return false;
	}

	debugC(DEBUG_INSANE, "loadHandler7FlySprites: header bytes: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
		header[0], header[1], header[2], header[3], header[4],
		header[5], header[6], header[7], header[8], header[9]);

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

	if (bytesRead >= 8) {
		uint32 animTag = READ_BE_UINT32(nutData);
		if (animTag != MKTAG('A','N','I','M')) {
			warning("Rebel2 loadHandler7FlySprites: No ANIM tag! Data may be corrupted");
			free(nutData);
			return false;
		}
	}

	NutRenderer *newNut = makeRebel2SpriteFromData(_vm, nutData, bytesRead);
	if (!newNut || newNut->getNumChars() <= 0) {
		debugC(DEBUG_INSANE, "loadHandler7FlySprites: NUT load failed for par4=%d", par4);
		delete newNut;
		free(nutData);
		return false;
	}

	debugC(DEBUG_INSANE, "loadHandler7FlySprites: Loaded FLY NUT par4=%d with %d sprites",
		par4, newNut->getNumChars());

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
	case 10:
	case 11:
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

bool InsaneRebel2::loadTurretHudOverlay(byte *animData, int32 size, int16 selector) {
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

bool InsaneRebel2::loadHandler8ShipSprites(byte *animData, int32 size, int16 par4) {

	if (!animData || size <= 0) {
		return false;
	}

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

bool InsaneRebel2::loadHandler25GrdSprites(byte *animData, int32 size, int16 par4) {

	if (!animData || size <= 0) {
		return false;
	}

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

bool InsaneRebel2::loadLevel2Background(byte *animData, int32 size, byte *renderBitmap) {
	if (!animData || size < 8) {
		return false;
	}

	debugC(DEBUG_INSANE, "loadLevel2Background: Loading Level 2 background (animSize=%d)", size);

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

			int codec = stream.readUint16LE();
			int fobjX = stream.readSint16LE();
			int fobjY = stream.readSint16LE();
			int fobjW = stream.readSint16LE();
			int fobjH = stream.readSint16LE();
			stream.readUint16LE();  // unknown
			stream.readUint16LE();  // unknown

			debugC(DEBUG_INSANE, "loadLevel2Background: Found FOBJ: codec=%d pos=(%d,%d) size=%dx%d",
				codec, fobjX, fobjY, fobjW, fobjH);

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

void InsaneRebel2::iactRebel2Opcode9(byte *renderBitmap, Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4) {
	int64 startPos = b.pos();

	uint32 tag = b.readUint32BE();

	const char *textStr = nullptr;
	char textBuffer[512];
	int16 posX = 160;
	int16 posY = 150;
	int16 textFlags = 1;
	int16 clipX = 16, clipY = 16, clipW = 288, clipH = 168;

	if (tag == MKTAG('T','R','E','S')) {
		int32 stringIndex = b.readSint32LE();

		if (_player && _player->getString(stringIndex)) {
			textStr = _player->getString(stringIndex);
			debugC(DEBUG_INSANE, "Opcode 9: TRES string index=%d -> \"%s\"", stringIndex, textStr);
		} else {
			debugC(DEBUG_INSANE, "Opcode 9: TRES string index=%d not found", stringIndex);
			return;
		}

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
		b.seek(startPos);

		posX = b.readSint16LE();
		posY = b.readSint16LE();
		textFlags = b.readSint16LE();
		clipX = b.readSint16LE();
		clipY = b.readSint16LE();
		clipW = b.readSint16LE();
		clipH = b.readSint16LE();
		b.skip(2);

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

	int width = (_player && _player->_width > 0) ? _player->_width : 320;
	int height = (_player && _player->_height > 0) ? _player->_height : 200;

	if (posX < 16)
		posX = 16;
	if (posX > 304)
		posX = 304;
	if (posY < 16)
		posY = 16;
	if (posY > 196)
		posY = 196;

	if (!_rebelMsgFont) {
		debugC(DEBUG_INSANE, "Opcode 9: No message font loaded (_rebelMsgFont is null)");
		return;
	}

	if (!(textFlags & 0x04)) {
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

	TextStyleFlags styleFlags = kStyleAlignLeft;
	if (textFlags & 0x01) {
		styleFlags = kStyleAlignCenter;
	} else if (textFlags & 0x02) {
		styleFlags = kStyleAlignRight;
	}
	if (textFlags & 0x04) {
		styleFlags = (TextStyleFlags)(styleFlags | kStyleWordWrap);
	}

	int16 textColor = 255;

	// Convert inline text to font indices while preserving format escapes.
	char convertedText[512];
	int srcLen = strlen(textStr);
	int dstIdx = 0;
	int numChars = _rebelMsgFont->getNumChars();

	for (int i = 0; i < srcLen && dstIdx < (int)sizeof(convertedText) - 1; i++) {
		byte ch = (byte)textStr[i];

		if (ch == '^' && i + 1 < srcLen) {
			byte next = (byte)textStr[i + 1];
			if (next == 'f' && i + 3 < srcLen) {
				// Reserve room for the escape bytes plus the terminating NUL.
				if (dstIdx + 4 > (int)sizeof(convertedText) - 1)
					break;
				convertedText[dstIdx++] = textStr[i++];
				convertedText[dstIdx++] = textStr[i++];
				convertedText[dstIdx++] = textStr[i++];
				convertedText[dstIdx++] = textStr[i];
				continue;
			} else if (next == 'c' && i + 4 < srcLen) {
				if (dstIdx + 5 > (int)sizeof(convertedText) - 1)
					break;
				convertedText[dstIdx++] = textStr[i++];
				convertedText[dstIdx++] = textStr[i++];
				convertedText[dstIdx++] = textStr[i++];
				convertedText[dstIdx++] = textStr[i++];
				convertedText[dstIdx++] = textStr[i];
				continue;
			} else if (next == 'l') {
				if (dstIdx + 2 > (int)sizeof(convertedText) - 1)
					break;
				convertedText[dstIdx++] = textStr[i++];
				convertedText[dstIdx++] = textStr[i];
				continue;
			} else if (next == '^') {
				i++;
				ch = '^';
			}
		}

		if (ch >= 'a' && ch <= 'z') {
			ch = ch - 'a' + 'A';
		}

		if (ch >= 32 && ch < (byte)(32 + numChars)) {
			convertedText[dstIdx++] = ch - 32;
		} else if (ch == '\n' || ch == '\r') {
			convertedText[dstIdx++] = ch;
		} else {
			convertedText[dstIdx++] = 0;
		}
	}
	convertedText[dstIdx] = '\0';

	if (ConfMan.getBool("subtitles")) {
		Rebel2FontSet fontSet;
		fontSet.numFonts = 1;
		fontSet.fonts[0] = _rebelMsgFont;

		if (textFlags & 0x04) {
			drawRebel2StringWrap(fontSet, convertedText, dstIdx, renderBitmap, clipRect, posX, posY, width, textColor, styleFlags);
		} else {
			drawRebel2String(fontSet, convertedText, dstIdx, renderBitmap, clipRect, posX, posY, width, textColor, styleFlags);
		}
	}

	debugC(DEBUG_INSANE, "Opcode 9: Rendered subtitle at (%d,%d) flags=0x%x clip=(%d,%d,%d,%d)",
		posX, posY, textFlags, clipX, clipY, clipW, clipH);
}

void InsaneRebel2::enemyUpdate(byte *renderBitmap, Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4) {
	int16 enemyId = b.readSint16LE();
	int16 x = b.readSint16LE();

	bool disabled = isBitSet(enemyId);

	int16 y = b.readSint16LE();
	int16 w = b.readSint16LE();
	int16 h = b.readSint16LE();

	// Turret surface targets mirror their gauge group state: hidden while the group
	// counter sits at zero, shown while it is nonzero, with a blink-out (alternating
	// odd/even ticks) during the 6 frames after the group is depleted.
	if (par3 == 2 && _rebelHandler == 0x26) {
		if (par4 >= 100 && par4 < 110) {
			const int surfaceIdx = par4 - 100;
			if (_rebelValueCounters[surfaceIdx] == 0 && (_rebelGaugeBlink[surfaceIdx] & 1) == 0)
				setBit(enemyId);
			else
				clearBit(enemyId);
			if (_rebelGaugeBlink[surfaceIdx] != 0)
				_rebelGaugeBlink[surfaceIdx]--;
		}
		return;
	}

	if (disabled) {
		return;
	}

	debugC(DEBUG_INSANE, "Opcode4: handler=%d enemyId=%d par2=%d par3=%d par4/type=%d pos=(%d,%d) size=(%d,%d)",
		_rebelHandler, enemyId, par2, par3, par4, x, y, w, h);

	bool found = false;
	Common::List<enemy>::iterator it;
	for (it = _enemies.begin(); it != _enemies.end(); ++it) {
		if (it->id == enemyId) {
			it->rect = Common::Rect(x, y, x + w, y + h);
			it->type = par4;
			// IACT bit state is authoritative; clear stale destruction state here.
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

void InsaneRebel2::initEnemyStruct(int id, int32 x, int32 y, int32 w, int32 h, bool active, bool destroyed, int32 explosionFrame, int type) {
	enemy e;
	e.id = id;
	e.type = type;
	e.rect = Common::Rect(x, y, x + w, y + h);
	e.active = active;
	e.destroyed = destroyed;
	e.explosionFrame = explosionFrame;
	e.explosionComplete = false;
	e.savedBackground = nullptr;
	e.savedBgWidth = 0;
	e.savedBgHeight = 0;
	_enemies.push_back(e);
}

} // End of namespace Scumm
