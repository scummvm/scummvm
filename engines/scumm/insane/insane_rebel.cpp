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



#include "engines/engine.h"
#include "common/system.h"
#include "common/memstream.h"

#include "scumm/actor.h"
#include "scumm/file.h"
#include "scumm/resource.h"
#include "scumm/scumm_v7.h"
#include "scumm/sound.h"

#include "scumm/imuse/imuse.h"
#include "scumm/imuse_digi/dimuse_engine.h"

#include "scumm/smush/smush_player.h"
#include "scumm/smush/smush_font.h"

#include "scumm/insane/insane_rebel.h"


namespace Scumm {

InsaneRebel2::InsaneRebel2(ScummEngine_v7 *scumm) {
	_vm = scumm;

	// Initialize parent class pointers to nullptr to avoid crash in ~Insane()
	// because Insane() default constructor leaves them uninitialized.
	_smush_roadrashRip = nullptr;
	_smush_roadrsh2Rip = nullptr;
	_smush_roadrsh3Rip = nullptr;
	_smush_goglpaltRip = nullptr;
	_smush_tovista1Flu = nullptr;
	_smush_tovista2Flu = nullptr;
	_smush_toranchFlu = nullptr;
	_smush_minedrivFlu = nullptr;
	_smush_minefiteFlu = nullptr;
	_smush_bencutNut = nullptr;
	_smush_bensgoggNut = nullptr;

	// Rebel Assault 2 specific initialization can go here
	_rebelHudPrimary = nullptr;
	_rebelHudSecondary = nullptr;
	_rebelHudCockpit = nullptr;
	_rebelHudExplosion = nullptr;
	_rebelHudDamage = nullptr;
	_rebelHudEffects = nullptr;
	_rebelHudHiRes = nullptr;

	// Rebel Assault 2: Load cockpit sprites NUT which contains crosshairs, explosions, status bar
	// CPITIMAG.NUT = low-res (320x200), CPITIMHI.NUT = high-res (640x480)
	// For now, use CPITIMAG since the game runs at 320x200
	_smush_iconsNut = new NutRenderer(_vm, "SYSTM/CPITIMAG.NUT");
	_smush_icons2Nut = nullptr;  // Not used for Rebel2
	_smush_cockpitNut = new NutRenderer(_vm, "SYSTM/DISPFONT.NUT");

	_rebelEnemies.clear();
	_rebelHandler = 8;  // Default to Handler 8 (ground vehicle) for Level 1
	_rebelLevelType = 0;  // Level type from Opcode 6 par3, determines HUD sprite variant

	_speed = 12;
	_insaneIsRunning = false;

	_numberArray = 0;
	_emulateInterrupt = 0;
	_flag1d = 0;
	_objArray1Idx = 0;
	_objArray1Idx2 = 0;
	_objArray2Idx = 0;
	_objArray2Idx2 = 0;
	_currSceneId = 1;
	_timer6Id = 0;
	_timerSpriteId = 0;
	_temp2SceneId = 0;
	_tempSceneId = 0;
	_currEnemy = -1;
	_currScenePropIdx = 0;
	_currScenePropSubIdx = 0;
	_currTrsMsg = 0;
	_sceneData2Loaded = 0;
	_sceneData1Loaded = 0;
	_keyboardDisable = 0;
	_needSceneSwitch = false;
	_idx2Exceeded = 0;
	_tiresRustle = false;
	_keybOldDx = 0;
	_keybOldDy = 0;
	_velocityX = 0;
	_velocityY = 0;
	_keybX = 0;
	_keybY = 0;
	_firstBattle = false;
	_battleScene = true;
	_kickBenProgress = false;
	_weaponBenJustSwitched = false;
	_kickEnemyProgress = false;
	_weaponEnemyJustSwitched = false;
	_beenCheated = 0;
	_posBrokenTruck = 0;
	_posBrokenCar = 0;
	_posFatherTorque = 0;
	_posCave = 0;
	_posVista = 0;
	_roadBranch = false;
	_roadStop = false;
	_carIsBroken = false;
	_benHasGoggles = false;
	_mineCaveIsNear = false;
	_objectDetected = false;
	_approachAnim = -1;
	_val54d = 0;
	_val57d = 0;
	_val115_ = false;
	_roadBumps = false;
	_val211d = 0;
	_val213d = 0;
	_metEnemiesListTail = 0;
	_smlayer_room = 0;
	_smlayer_room2 = 0;
	_isBenCut = 0;
	_continueFrame = 0;
	_continueFrame1 = 0;
	_counter1 = 0;
	_iactSceneId = 0;
	_iactSceneId2 = 0;

	int i, j;
	
	for (i = 0; i < 12; i++)
		_metEnemiesList[i] = 0;

	for (i = 0; i < 9; i++)
		for (j = 0; j < 9; j++)
			_enHdlVar[i][j] = 0;

	for (i = 0; i < 0x200; i++)
		_iactBits[i] = 0;

	for (i = 0; i < 512; i++) {
		_rebelLinks[i][0] = 0;
		_rebelLinks[i][1] = 0;
		_rebelLinks[i][2] = 0;
	}
}


InsaneRebel2::~InsaneRebel2() {
	delete _rebelHudPrimary;
	delete _rebelHudSecondary;
	delete _rebelHudCockpit;
	delete _rebelHudExplosion;
	delete _rebelHudDamage;
	delete _rebelHudEffects;
	delete _rebelHudHiRes;
}


int32 InsaneRebel2::processMouse() {
	int32 buttons = 0;

	// Get button state directly from event manager (SCUMM VARs aren't updated during SMUSH)
	static bool wasPressed = false;
	bool isPressed = (_vm->_system->getEventManager()->getButtonState() & 1) != 0;
	
	// Edge detection: only trigger on button press (not hold)
	if (isPressed && !wasPressed) {
		Common::Point mousePos(_vm->_mouse.x, _vm->_mouse.y);
		debug("Rebel2 Click: Mouse=(%d,%d) Enemies=%d", 
			mousePos.x, mousePos.y, _rebelEnemies.size());

		// Check for hit on any active enemy
		Common::List<RebelEnemy>::iterator it;
		for (it = _rebelEnemies.begin(); it != _rebelEnemies.end(); ++it) {
			debug("  Enemy ID=%d active=%d destroyed=%d rect=(%d,%d)-(%d,%d) contains=%d",
				it->id, it->active, it->destroyed,
				it->rect.left, it->rect.top, it->rect.right, it->rect.bottom,
				it->rect.contains(mousePos));
				
			if (it->active && it->rect.contains(mousePos)) {
				// Enemy hit!
				it->active = false;
				it->destroyed = true;  // Mark as destroyed so IACT won't re-activate
				it->explosionFrame = 0;  // Start explosion animation
				it->explosionComplete = false;  // Explosion not yet finished
				debug("Rebel2: HIT enemy ID=%d at (%d,%d) - Rect: (%d,%d)-(%d,%d)", 
					it->id, mousePos.x, mousePos.y,
					it->rect.left, it->rect.top, it->rect.right, it->rect.bottom);

				// Disable self
				setBit(it->id);

				// Handle dependencies
				int id = it->id;
				if (id >= 0 && id < 512) {
					// Slot 2: Enable (Explosion?)
					if (_rebelLinks[id][2] != 0) {
						clearBit(_rebelLinks[id][2]); 
						debug("Rebel2: Enabled dependency Slot 2 (ID=%d) for Parent %d", _rebelLinks[id][2], id);
					}
					// Slot 1: Enable (Explosion?)
					if (_rebelLinks[id][1] != 0) {
						clearBit(_rebelLinks[id][1]);
						debug("Rebel2: Enabled dependency Slot 1 (ID=%d) for Parent %d", _rebelLinks[id][1], id);
					}
					// Slot 0: Disable (Shots?)
					if (_rebelLinks[id][0] != 0) {
						setBit(_rebelLinks[id][0]);
						debug("Rebel2: Disabled dependency Slot 0 (ID=%d) for Parent %d", _rebelLinks[id][0], id);
					}
				}

				// Note: Background saving and masking is handled in procPostRendering
				// where we have access to the render bitmap
				// TODO: Play explosion sound
				// TODO: Update score
				// Only hit one enemy per click
				break;
			}
		}
	}
	wasPressed = isPressed;
	return buttons;
}

bool InsaneRebel2::shouldSkipFrameUpdate(int left, int top, int width, int height) {
	// Only check for Rebel2
	if (_vm->_game.id != GID_REBEL2) {
		return false;
	}
	
	Common::Rect updateRect(left, top, left + width, top + height);
	int updateArea = width * height;
	
	// Check if this update region significantly overlaps with any destroyed enemy
	Common::List<RebelEnemy>::iterator it;
	for (it = _rebelEnemies.begin(); it != _rebelEnemies.end(); ++it) {
		if (it->destroyed) {
			// Calculate the intersection of the update rect and enemy rect
			Common::Rect enemyRect = it->rect;
			
			if (updateRect.intersects(enemyRect)) {
				// Calculate the intersection area
				int intLeft = MAX(updateRect.left, enemyRect.left);
				int intTop = MAX(updateRect.top, enemyRect.top);
				int intRight = MIN(updateRect.right, enemyRect.right);
				int intBottom = MIN(updateRect.bottom, enemyRect.bottom);
				int intArea = (intRight - intLeft) * (intBottom - intTop);
				
				// Require at least 70% overlap to skip the update
				// This prevents unrelated frame updates from being incorrectly skipped
				if (intArea * 100 >= updateArea * 70) {
					debug("Rebel2: Skipping frame update (%d,%d %dx%d) - %d%% overlap with destroyed enemy ID=%d",
						left, top, width, height, (intArea * 100) / updateArea, it->id);
					return true;
				}
			}
		}
	}
	
	return false;
}

bool InsaneRebel2::isBitSet(int n) {
	assert (n < 0x200);

	return (_iactBits[n] != 0);
}

void InsaneRebel2::setBit(int n) {
	assert (n < 0x200);

	_iactBits[n] = 1;
}

void InsaneRebel2::clearBit(int n) {
	assert (n < 0x200);

	_iactBits[n] = 0;
}

void InsaneRebel2::procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					  int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
					  int16 par1, int16 par2, int16 par3, int16 par4) {
	if (_keyboardDisable)
		return;

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
		
		//debug("Rebel2 IACT Opcode 4: ID=%d X=%d Y=%d W=%d H=%d (par2=%d par3=%d par4=%d)", 
		//	enemyId, x, y, w, h, par2, par3, par4);

		// Update RebelEnemy list for hit detection
		bool found = false;
		Common::List<RebelEnemy>::iterator it;
		for (it = _rebelEnemies.begin(); it != _rebelEnemies.end(); ++it) {
			if (it->id == enemyId) {
				it->rect = Common::Rect(x, y, x + w, y + h);
				// Only re-activate if not destroyed
				if (!it->destroyed) {
					it->active = true;
				}
				found = true;
				break;
			}
		}
		if (!found) {
			RebelEnemy e;
			e.id = enemyId;
			e.rect = Common::Rect(x, y, x + w, y + h);
			e.active = true;
			e.destroyed = false;
			e.explosionFrame = -1;  // No explosion playing
			_rebelEnemies.push_back(e);
		}

		// Render Bounding Box for visual verification
		// Use player dimensions for clipping
		int bufWidth = (_player && _player->_width > 0) ? _player->_width : 320;
		int bufHeight = (_player && _player->_height > 0) ? _player->_height : 200;
		int pitch = bufWidth;
		
		// Draw the bounding box in color 255 (bright white) - only for non-destroyed enemies
		// Skip drawing if this enemy has been destroyed
		if (renderBitmap && w > 0 && h > 0 && !found) {
			// Only draw for newly created enemies (not destroyed ones)
			drawRect(renderBitmap, pitch, x, y, w, h, 255, bufWidth, bufHeight);
		} else if (found && !it->destroyed) {
			// Draw for existing non-destroyed enemies
			drawRect(renderBitmap, pitch, x, y, w, h, 255, bufWidth, bufHeight);
		}

		// Debug for potential HUD Setup (Opcode 4 with word@8 == 1)
		if (enemyId == 1) {
			debug("Rebel2 IACT Opcode 4: HUD Setup Candidate? (enemyId/val@8 == 1). par4=%d", par4);
		}

	} else if (par1 == 2) {
		// Opcode 2: Often used for bit setting
		// Disassembly shows this is handled but we don't have full context
		// debug("Rebel2 IACT Opcode 2: par2=%d par3=%d par4=%d", par2, par3, par4);
		
		// Handle dependency linking (par3 == 4)
		if (par3 == 4) {
			int16 childId = b.readSint16LE(); // Offset +8
			int16 parentId = b.readSint16LE(); // Offset +10
			
			if (parentId >= 0 && parentId < 512) {
				// Shift links
				_rebelLinks[parentId][2] = _rebelLinks[parentId][1];
				_rebelLinks[parentId][1] = _rebelLinks[parentId][0];
				_rebelLinks[parentId][0] = childId;
				
				// Apply initial state based on parent state
				// If parent is ALIVE (Bit Clear) -> Disable new Child (Set Bit)
				// If parent is DEAD (Bit Set) -> Enable new Child (Clear Bit)
				if (!isBitSet(parentId)) {
					setBit(childId);
					debug("Rebel2: Linked ID=%d to Parent=%d (Slot 0). Parent Alive -> Child Disabled.", childId, parentId);
				} else {
					clearBit(childId);
					debug("Rebel2: Linked ID=%d to Parent=%d (Slot 0). Parent Dead -> Child Enabled.", childId, parentId);
				}
			}
		} else {
			// Skip extra data if any? Opcode 2 might vary in size.
			// Based on disassembly, it accesses offset +8 and +10 for case 4.
			// Currently we don't know size for other cases, but hopefully they don't crash stream reading.
			// 'b' is SeekableReadStream, so we might need to skip if we don't read?
			// The caller `procIACT` passes `size` but that's for the WHOLE chunk.
			// We can't skip unknown sub-opcodes easily without size map.
			// Assuming only par3=4 has extra data for now.
		}
		
	} else if (par1 == 3) {
		// Opcode 3: Often used for clearing/resetting
		debug("Rebel2 IACT Opcode 3: par2=%d par3=%d par4=%d", par2, par3, par4);
		
	} else if (par1 == 5) {
		// Opcode 5: Special handling based on par2 value
		// Disassembly shows sub-opcodes 0xD (13) and 0xE (14)
		debug("Rebel2 IACT Opcode 5: par2=%d par3=%d par4=%d", par2, par3, par4);
		
	} else if (par1 == 6) {
		// Opcode 6: Scene trigger / mode switch
		// Disassembly shows it sets DAT_0047ee84 (handler type) to par2 for values 7, 8, 0x19, 0x26
		// This determines which rendering handler and crosshair sprite to use
		// par3 is stored as DAT_004436de (level type) which affects HUD sprite selection
		
		// Update handler type if par2 is a known handler value
		if (par2 == 7 || par2 == 8 || par2 == 0x19 || par2 == 0x26) {
			_rebelHandler = par2;
			_rebelLevelType = par3;  // Store level type (affects HUD sprite: 5 vs 53)
			debug("Rebel2 IACT Opcode 6: Setting handler=%d levelType=%d (par4=%d)", par2, par3, par4);
		}
		
		// Check for Status Bar Enable trigger (par4 == 1 means enable status bar drawing)
		// Based on FUN_407FCB line 79: if (param_5[4] == 1) { FUN_0040bb87(...) }
		// Note: Sprite 5 is a 136x13 status bar, NOT the full cockpit (which is baked into video)
		if (par4 == 1) {
			debug("Rebel2 IACT Opcode 6: Status Bar ENABLED - will draw sprite %d (136x13) from CPITIMAG.NUT", 
				(_rebelLevelType == 5) ? 53 : 5);
		}
		
		// Debug: Confirm handler type after update
		if (_rebelHandler == 0x26) {
			debug("Rebel2: Handler set to TURRET (0x26/38) - status bar sprite 5, crosshair sprites 48+");
		}

	} else if (par1 == 8) {
		// TODO
	} else if (par1 == 9) {
		// Opcode 9: Text/subtitle display
		debug("Rebel2 IACT Opcode 9: par2=%d par3=%d par4=%d (text)", par2, par3, par4);
		
	} else if (par1 == 0 || par1 == 1) {
		// Low Opcodes seen in logs
		debug("Rebel2 IACT: Low Opcode %d (par2=%d par3=%d par4=%d)", par1, par2, par3, par4);
	} else {
		debug("Rebel2 IACT: Unknown Opcode %d (par2=%d par3=%d par4=%d)", par1, par2, par3, par4);
	}
}

// External helpers from smush_player.cpp but we are already in Scumm namespace
extern void smushDecodeRLE(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);
extern void smushDecodeUncompressed(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);

void InsaneRebel2::loadEmbeddedSan(int userId, byte *animData, int32 size, byte *renderBitmap) {
	// Validate userId (1-4 for HUD slots)
	if (userId < 1 || userId > 4 || !animData || size < 32) {
		debug("Rebel2: Invalid embedded SAN: userId=%d, size=%d", userId, size);
		return;
	}
	
	Common::MemoryReadStream stream(animData, size);
	
	// Read ANIM header
	uint32 animTag = stream.readUint32BE();
	if (animTag != MKTAG('A','N','I','M')) {
		debug("Rebel2: Embedded SAN missing ANIM tag, got 0x%08X", animTag);
		return;
	}
	uint32 animSize = stream.readUint32BE();
	debug("Rebel2: Parsing embedded ANIM: userId=%d, reported size=%u, actual=%d", userId, animSize, size - 8);
	
	// Iterate through chunks to find FRME -> FOBJ
	while (!stream.eos() && stream.pos() < size) {
		uint32 tag = stream.readUint32BE();
		uint32 chunkSize = stream.readUint32BE();
		int32 nextChunkPos = stream.pos() + chunkSize;

		if (tag == MKTAG('F','R','M','E')) {
			// Iterate sub-chunks in FRME
			while (stream.pos() < nextChunkPos && !stream.eos()) {
				uint32 subTag = stream.readUint32BE();
				uint32 subSize = stream.readUint32BE();
				int32 nextSubPos = stream.pos() + subSize;

				if (subTag == MKTAG('F','O','B','J')) {
					// Found FOBJ - Embedded HUD Frame
					// Read FOBJ header
					int codec = stream.readUint16LE();
					int left = stream.readUint16LE();
					int top = stream.readUint16LE();
					int width = stream.readUint16LE();
					int height = stream.readUint16LE();
					stream.readUint16LE();  // unknown
					stream.readUint16LE();  // unknown
					
					debug("Rebel2: Embedded HUD frame: userId=%d, %dx%d at (%d,%d), codec=%d", 
						userId, width, height, left, top, codec);
					
					// Allocate storage for the decoded frame
					EmbeddedSanFrame &frame = _rebelEmbeddedHud[userId];
					
					if (width > 0 && height > 0 && width <= 800 && height <= 480) {
						if (frame.width != width || frame.height != height || !frame.pixels) {
							frame.clear();
							frame.pixels = (byte *)calloc(width * height, 1);
							frame.width = width;
							frame.height = height;
						}
						
						// Update render position from FOBJ header
						frame.renderX = left;
						frame.renderY = top;
						
						// Read the raw FOBJ data
						int32 dataSize = subSize - 14;
						if (dataSize > 0) {
							byte *fobjData = (byte *)malloc(dataSize);
							stream.read(fobjData, dataSize);
							
							// Decode based on codec
							if (codec == 1 || codec == 3) {
								// RLE use existing decoder
								smushDecodeRLE(frame.pixels, fobjData, 0, 0, width, height, width);
								frame.valid = true;
								debug("Rebel2: Decoded embedded HUD (codec %d/RLE): %dx%d", codec, width, height);
							} else if (codec == 20) {
								// Uncompressed
								smushDecodeUncompressed(frame.pixels, fobjData, 0, 0, width, height, width);
								frame.valid = true;
								debug("Rebel2: Decoded embedded HUD (codec 20/raw): %dx%d", width, height);
							} else if (codec == 21 || codec == 44) {
								// Codec 21/44: Line update codec
								byte *srcPtr = fobjData;
								for (int row = 0; row < height && srcPtr < fobjData + dataSize; row++) {
									int lineDataSize = READ_LE_UINT16(srcPtr);
									srcPtr += 2;
									byte *lineEnd = srcPtr + lineDataSize;
									byte *lineDst = frame.pixels + row * width;
									int x = 0;
									while (srcPtr < lineEnd && x < width) {
										int skip = READ_LE_UINT16(srcPtr);
										srcPtr += 2;
										x += skip;
										if (srcPtr >= lineEnd) break;
										int count = READ_LE_UINT16(srcPtr) + 1;
										srcPtr += 2;
										while (count-- > 0 && x < width && srcPtr < lineEnd) {
											lineDst[x++] = *srcPtr++;
										}
									}
									srcPtr = lineEnd;
								}
								frame.valid = true;
								debug("Rebel2: Decoded embedded HUD (codec 21/line update): %dx%d", width, height);
							} else if (codec == 45) {
								// Codec 45: Block delta (simple copy for now)
								int copySize = MIN((int)dataSize, width * height);
								memcpy(frame.pixels, fobjData, copySize);
								frame.valid = true;
								debug("Rebel2: Decoded embedded HUD (codec 45/block): %dx%d", width, height);
							} else {
								debug("Rebel2: TODO: Decode codec %d for embedded HUD", codec);
								frame.valid = false;
							}
							
							// Draw immediately to renderBitmap if valid
							if (frame.valid && renderBitmap) {
								int pitch = (_player && _player->_width > 0) ? _player->_width : 320;
								int bufHeight = (_player && _player->_height > 0) ? _player->_height : 200;
								
								for (int y = 0; y < height && (frame.renderY + y) < bufHeight; y++) {
									for (int x = 0; x < width && (frame.renderX + x) < pitch; x++) {
										byte pixel = frame.pixels[y * width + x];
										if (pixel != 0) {  // 0 = transparent
											int destX = frame.renderX + x;
											int destY = frame.renderY + y;
											if (destX >= 0 && destY >= 0) {
												renderBitmap[destY * pitch + destX] = pixel;
											}
										}
									}
								}
								debug("Rebel2: Rendered embedded HUD %d at (%d,%d)", userId, frame.renderX, frame.renderY);
							}
							
							free(fobjData);
						}
					}
					
					// Done with FOBJ - assume only one relevant frame per embedded SAN
					stream.seek(nextChunkPos);
					goto end_parsing;
				} else {
					// Skip other sub-chunks (AHDR inside FRME?) or padding
					stream.seek(nextSubPos);
					if (subSize & 1) stream.skip(1);
				}
			}
		} else {
			// Skip non-FRME chunks (AHDR, etc at top level)
			stream.seek(nextChunkPos);
			if (chunkSize & 1) stream.skip(1);
		}
	}
	
	debug("Rebel2: No FOBJ found in embedded SAN userId=%d", userId);

end_parsing:;
}


void InsaneRebel2::procPostRendering(byte *renderBitmap, int32 codecparam, int32 setupsan12,
							   int32 setupsan13, int32 curFrame, int32 maxFrame) {

	processMouse();

	// Determine correct pitch for the video buffer (usually 320 for Rebel2)
	int width = _player->_width;
	int height = _player->_height;
	if (width == 0) width = _vm->_screenWidth;
	if (height == 0) height = _vm->_screenHeight;
	int pitch = width;

	// --- HUD Drawing Order (from FUN_004089ab assembly analysis) ---
	// Based on FUN_004089ab:
	// 1. Line 156: FUN_004288c0 fills status bar background at Y=0xb4 (180)
	// 2. Lines 171-226: Draw turret overlays, targeting reticle, crosshair
	// 3. Line 243: FUN_0041c012 draws status bar sprites LAST (on top)
	//
	// In FUN_0041c012:
	// - Sprites are drawn to buffer DAT_00482204 at position (0,0)
	// - Buffer is composited at Y=0xb4 (180) via FUN_0042f780
	// - DISPFONT.NUT (DAT_00482200) sprites 1-7 contain the status bar elements
	//
	// For ScummVM, we draw directly to screen at Y=180
	
	// Use video content coordinates, NOT buffer coordinates
	const int videoWidth = 320;    // Native video width
	const int videoHeight = 200;   // Native video height  
	const int statusBarY = 180;    // 0xb4 - status bar starts at Y=180 in video coords
	
	// ============================================================
	// STEP 0: Fill status bar background (FUN_004288c0 equivalent)
	// ============================================================
	// Clear the status bar area at Y=180-199 with background color
	// Original assembly: FUN_004288c0(local_8, 0, 0, 0xb4, 0x140, 0x14, 4)
	// This fills width=320, height=20 starting at Y=180 with color index 4
	const byte statusBarBgColor = 4;
	for (int y = statusBarY; y < videoHeight && y < height; y++) {
		for (int x = 0; x < videoWidth && x < pitch; x++) {
			renderBitmap[y * pitch + x] = statusBarBgColor;
		}
	}
	
	// ============================================================
	// STEP 1: Draw embedded SAN cockpit overlay FIRST (from IACT chunks)
	// ============================================================
	// The cockpit overlay forms the decorative frame at the bottom
	// userId 1: Left piece at X=0
	// userId 2: Right piece at X=slot1Width
	// These are drawn BEFORE the status bar so status bar appears on top
	
	int slot1Width = 0;
	if (_rebelEmbeddedHud[1].valid && _rebelEmbeddedHud[1].width > 0) {
		slot1Width = _rebelEmbeddedHud[1].width;
	}
	
	for (int hudSlot = 1; hudSlot <= 2; hudSlot++) {
		EmbeddedSanFrame &frame = _rebelEmbeddedHud[hudSlot];
		if (frame.valid && frame.pixels && frame.width > 0 && frame.height > 0) {
			int destX, destY;
			
			// Position the two HUD pieces horizontally adjacent
			if (hudSlot == 1) {
				destX = 0;
			} else {
				destX = slot1Width;
			}
			
			// Position at bottom of 320x200 video content
			// The cockpit overlay sits at Y = 200 - frameHeight
			destY = statusBarY - frame.height;
			if (destY < 0) destY = 0;
			
			// Draw frame with transparency (pixel 0 = transparent)
			for (int y = 0; y < frame.height && (destY + y) < height; y++) {
				for (int x = 0; x < frame.width && (destX + x) < pitch; x++) {
					byte pixel = frame.pixels[y * frame.width + x];
					if (pixel != 0) {  // Skip transparent pixels
						int fx = destX + x;
						int fy = destY + y;
						if (fx >= 0 && fy >= 0) {
							renderBitmap[fy * pitch + fx] = pixel;
						}
					}
				}
			}
			
			static bool debugEmbeddedOnce[5] = {false};
			if (!debugEmbeddedOnce[hudSlot]) {
				debug("Rebel2: Drawing embedded HUD slot %d (%dx%d) at (%d,%d)", 
					hudSlot, frame.width, frame.height, destX, destY);
				debugEmbeddedOnce[hudSlot] = true;
			}
		}
	}
	
	// ============================================================
	// STEP 2: Draw DISPFONT.NUT status bar sprites (FUN_0041c012 equivalent)
	// ============================================================
	// DISPFONT.NUT contains status bar elements - drawn ON TOP of cockpit overlay
	// From assembly FUN_0041c012:
	//   - Sprite 1: Status bar background frame (full width, drawn at 0,0)
	//   - Sprites 2-5: Difficulty stars (1-4 stars, drawn at 0,0)
	//   - Sprite 6: Shield bar fill (drawn with clip rect X=0x3f, Y=9, W=64, H=6)
	//   - Sprite 7: Shield alert (flashing red when shields critical)
	//
	// All sprites are drawn to buffer at (0,0), buffer composited at Y=180
	// For ScummVM, we draw directly at Y=statusBarY
	if (_smush_cockpitNut) {
		// Debug: Log DISPFONT.NUT sprite info once
		static bool loggedDispfont = false;
		if (!loggedDispfont) {
			int numSprites = _smush_cockpitNut->getNumChars();
			debug("Rebel2: DISPFONT.NUT has %d sprites, statusBarY=%d:", numSprites, statusBarY);
			for (int i = 0; i < numSprites && i < 10; i++) {
				int sw = _smush_cockpitNut->getCharWidth(i);
				int sh = _smush_cockpitNut->getCharHeight(i);
				debug("  Sprite %d: %dx%d", i, sw, sh);
			}
			loggedDispfont = true;
		}
		
		// Draw status bar background frame (sprite 1) at (0, statusBarY)
		// This sprite is the full-width status bar background
		if (_smush_cockpitNut->getNumChars() > 1) {
			smlayer_drawSomething(renderBitmap, pitch, 0, statusBarY, 0, _smush_cockpitNut, 1, 0, 0);
		}
		
		// Draw difficulty indicator (sprites 2-5 based on difficulty level 0-3)
		// Sprite index = difficulty + 2; capped at 4 max difficulty (sprite 5)
		// Assembly draws at (0,0) in buffer - same position as sprite 1
		int difficulty = 0;  // TODO: Read from game state (DAT_0047a7fa)
		if (difficulty > 3) difficulty = 3;
		int difficultySprite = difficulty + 2;  // sprites 2, 3, 4, or 5
		if (_smush_cockpitNut->getNumChars() > difficultySprite) {
			smlayer_drawSomething(renderBitmap, pitch, 0, statusBarY, 0, _smush_cockpitNut, difficultySprite, 0, 0);
		}
		
		// Draw shield bar (sprite 6) 
		// Assembly uses clip rect: X=0x3f(63), Y=0x9(9), W=0x40(64), H=0x6(6)
		// The width is scaled based on shield value (param_1 >> 2)
		// For now, draw at position (0, statusBarY) - sprite has internal positioning
		if (_smush_cockpitNut->getNumChars() > 6) {
			smlayer_drawSomething(renderBitmap, pitch, 0, statusBarY, 0, _smush_cockpitNut, 6, 0, 0);
		}
		
		// Draw shield alert overlay (sprite 7) when shields critical (> 0xAA = 170)
		// Only draws when frame counter bit 3 is clear (every 8 frames)
		// For now, skip - TODO: implement shield critical flashing
		
		// Draw lives indicator - assembly shows at X=0xa8 (168), Y=7
		// Uses sprite 1 again with different clip rect
		// TODO: Implement lives rendering
		
		// Draw score - uses FUN_00434cb0 (text rendering) at X=0x101(257)
		// TODO: Implement score rendering
	} else {
		static bool warnedNullOnce = false;
		if (!warnedNullOnce) {
			debug("Rebel2: WARNING - _smush_cockpitNut (DISPFONT.NUT) is null!");
			warnedNullOnce = true;
		}
	}

	// Debug: Draw bounding boxes for enemies
	// width/height/pitch already calculated above

	// Debug: Verify buffer format and drawing capability
	static uint32 lastDebugTime = 0;
	if ((_vm->_system->getMillis() - lastDebugTime) > 2000) {
		lastDebugTime = _vm->_system->getMillis();
		debug("Rebel2 Debug: Buffer %dx%d, Pitch %d, BPP %d, Enemies: %d", width, height, pitch, _vm->_virtscr[kMainVirtScreen].format.bytesPerPixel, _rebelEnemies.size());
	}

	Common::List<RebelEnemy>::iterator it;
	for (it = _rebelEnemies.begin(); it != _rebelEnemies.end(); ++it) {
		Common::Rect r = it->rect;
		
		// Clip the rect to screen bounds for safety
		Common::Rect clipped = r;
		clipped.clip(Common::Rect(0, 0, width, height));

		if (!clipped.isValidRect()) continue;
		
		if (it->destroyed) {
			// Handle destroyed enemies - draw explosion animation
			// The enemy frame updates are now skipped in decodeFrameObject via shouldSkipFrameUpdate
			
			// Draw explosion animation - only while animation is in progress
			// CPITIMAG.NUT indices 9-41 contain explosion frames, but we only use 8 for a quick effect
			const int EXPLOSION_FRAMES = 8;
			if (it->explosionFrame >= 0 && it->explosionFrame < EXPLOSION_FRAMES) {
				if (_smush_iconsNut) {
					int explosionSpriteIndex = 9 + it->explosionFrame;
					if (_smush_iconsNut->getNumChars() > explosionSpriteIndex) {
						int ew = _smush_iconsNut->getCharWidth(explosionSpriteIndex);
						int eh = _smush_iconsNut->getCharHeight(explosionSpriteIndex);
						// Center explosion on the enemy bounding box
						int cx = (r.left + r.right) / 2 - ew / 2;
						int cy = (r.top + r.bottom) / 2 - eh / 2;
						smlayer_drawSomething(renderBitmap, pitch, cx, cy, 0, _smush_iconsNut, explosionSpriteIndex, 0, 0);
					}
				}
				
				// Advance explosion frame
				it->explosionFrame++;
				
				// Check if explosion is now complete
				if (it->explosionFrame >= EXPLOSION_FRAMES) {
					it->explosionFrame = -1;  // Mark as done - prevents any further animation
					it->explosionComplete = true;
					debug("Rebel2: Explosion complete for enemy ID=%d", it->id);
				}
			}
			// After explosion is complete (explosionFrame == -1), do nothing
			// After explosion is complete (explosionFrame == -1), do nothing
		} else if (it->active && !isBitSet(it->id)) { // Only draw if active AND not disabled by IACT
			// Draw bounding box outline for active enemies (debug visualization)
			// Draw Top
			if (r.top >= 0 && r.top < height) {
				for (int x = clipped.left; x < clipped.right; x++) {
					renderBitmap[r.top * pitch + x] = 255;
				}
			}
			// Draw Bottom
			if (r.bottom > 0 && r.bottom <= height) {
				for (int x = clipped.left; x < clipped.right; x++) {
					renderBitmap[(r.bottom - 1) * pitch + x] = 255;
				}
			}
			// Draw Left
			if (r.left >= 0 && r.left < width) {
				for (int y = clipped.top; y < clipped.bottom; y++) {
					renderBitmap[y * pitch + r.left] = 255;
				}
			}
			// Draw Right
			if (r.right > 0 && r.right <= width) {
				for (int y = clipped.top; y < clipped.bottom; y++) {
					renderBitmap[y * pitch + (r.right - 1)] = 255;
				}
			}
		}
	}

	// Draw Crosshair/Reticle cursor
	// Sprite indices based on handler type (from original game disassembly):
	// - Handler 8 (ground vehicle): Index 0x2E (46)
	// - Handler 7 (space flight): Index 0x2F (47)
	// - Handler 0x19 (mixed/turret view): Index 0x2F (47)
	// - Handler 0x26 (full turret): Index 0x30+ (48+) with animation
	if (_smush_iconsNut) {
		//CursorMan.showMouse(false);
		int reticleIndex;
		switch (_rebelHandler) {
		case 7:   // Space flight
		case 0x19: // Mixed/turret view
			reticleIndex = 47;  // 0x2F
			break;
		case 0x26: // Full turret (with animation - simplified for now)
			reticleIndex = 48;  // 0x30
			break;
		case 8:   // Ground vehicle (Level 1) - FALLTHROUGH
		default:
			reticleIndex = 46;  // 0x2E
			break;
		}
		if (_smush_iconsNut->getNumChars() > reticleIndex) {
			int cw = _smush_iconsNut->getCharWidth(reticleIndex);
			int ch = _smush_iconsNut->getCharHeight(reticleIndex);
			
			static bool debugCrosshairOnce = false;
			if (!debugCrosshairOnce) {
				debug("Rebel2: Drawing crosshair sprite %d at (%d,%d) size %dx%d", 
					reticleIndex, _vm->_mouse.x - cw / 2, _vm->_mouse.y - ch / 2, cw, ch);
				debugCrosshairOnce = true;
			}
			
			// Center the crosshair on mouse position
			smlayer_drawSomething(renderBitmap, pitch, _vm->_mouse.x - cw / 2, _vm->_mouse.y - ch / 2, 0, _smush_iconsNut, reticleIndex, 0, 0);
		}
	}
}

}
