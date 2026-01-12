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

	// Rebel Assault 2: Load cockpit sprites NUT which contains crosshairs, explosions, status bar
	// CPITIMAG.NUT = low-res (320x200), CPITIMHI.NUT = high-res (640x480)
	// For now, use CPITIMAG since the game runs at 320x200
	_smush_iconsNut = new NutRenderer(_vm, "SYSTM/CPITIMAG.NUT");
	_smush_icons2Nut = nullptr;  // Not used for Rebel2
	_smush_cockpitNut = new NutRenderer(_vm, "SYSTM/DISPFONT.NUT");

	// Load DIHIFONT.NUT for in-video messages/subtitles (Opcode 9)
	_rebelMsgFont = new SmushFont(_vm, "SYSTM/DIHIFONT.NUT", true);

	_enemies.clear();
	_rebelHandler = 8;  // Default to Handler 8 (ground vehicle) for Level 1
	_rebelLevelType = 0;  // Level type from Opcode 6 par3, determines HUD sprite variant

	_playerDamage = 0;
	_playerLives = 3;
	_playerScore = 0;
	_viewX = 0;
	_viewY = 0;

	_difficulty = 1; // Default to Medium (1). TODO: Read from game config

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

	for (i = 0; i < 5; i++) {
		_explosions[i].active = false;
		_explosions[i].counter = 0;
	}

	for (i = 0; i < 2; i++) {
		_shots[i].active = false;
		_shots[i].counter = 0;
	}

	for (i = 0; i < 5; i++) {
		_rebelEmbeddedHud[i].pixels = nullptr;
		_rebelEmbeddedHud[i].width = 0;
		_rebelEmbeddedHud[i].height = 0;
		_rebelEmbeddedHud[i].renderX = 0;
		_rebelEmbeddedHud[i].renderY = 0;
		_rebelEmbeddedHud[i].valid = false;
	}
}


InsaneRebel2::~InsaneRebel2() {
	delete _rebelMsgFont;
}


int32 InsaneRebel2::processMouse() {
	int32 buttons = 0;

	// Get button state directly from event manager (SCUMM VARs aren't updated during SMUSH)
	bool wasPressed = false;
	bool isPressed = (_vm->_system->getEventManager()->getButtonState() & 1) != 0;
	
	// Edge detection: only trigger on button press (not hold)
	if (isPressed && !wasPressed) {
		Common::Point mousePos(_vm->_mouse.x, _vm->_mouse.y);
		debug("Rebel2 Click: Mouse=(%d,%d) Enemies=%d", 
			mousePos.x, mousePos.y, _enemies.size());

		// Spawn visual shot immediately
		spawnShot(mousePos.x, mousePos.y);

		// Calculate world position for hit testing
		Common::Point worldMousePos(mousePos.x + _viewX, mousePos.y + _viewY);

		// Check for hit on any active enemy
		Common::List<enemy>::iterator it;
		for (it = _enemies.begin(); it != _enemies.end(); ++it) {
			debug("  Enemy ID=%d active=%d destroyed=%d rect=(%d,%d)-(%d,%d) contains=%d",
				it->id, it->active, it->destroyed,
				it->rect.left, it->rect.top, it->rect.right, it->rect.bottom,
				it->rect.contains(worldMousePos));
				
			if (it->active && it->rect.contains(worldMousePos)) {
				// Enemy hit!
				it->active = false;
				it->destroyed = true;  // Mark as destroyed so IACT won't re-activate
				debug("Rebel2: HIT enemy ID=%d at (%d,%d) - Rect: (%d,%d)-(%d,%d)", 
					it->id, mousePos.x, mousePos.y,
					it->rect.left, it->rect.top, it->rect.right, it->rect.bottom);

				// Spawn explosion using native system
				// Use width / 2 as the scale parameter
				spawnExplosion((it->rect.left + it->rect.right) / 2, 
							   (it->rect.top + it->rect.bottom) / 2, 
							   it->rect.width() / 2);

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
	Common::List<enemy>::iterator it;
	for (it = _enemies.begin(); it != _enemies.end(); ++it) {
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
		enemyUpdate(renderBitmap, b, par2, par3, par4);
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
		// debug("Rebel2 IACT Opcode 3: par2=%d par3=%d par4=%d", par2, par3, par4);
		
		// Handle damage dealing (Subcode 5 in FUN_401234)
		// Opcode 3 is complicated:
		// Based on FUN_401234, case 1 (Opcode 3), checks local_c (offset +4 / par2?)
		// In iactRebel2Scene1, par1=Opcode.
		// If par1 == 1 (Opcode 1 / FUN_401234 case 1):
		//   It checks local_c which is from offset +4 (par3).
		//   If par3 == 5 -> Damage Logic.
		// 
		// Actually, FUN_401234 switches on `*local_14 - 2`.
		// If `*local_14` (Opcode) is 3, switch value is 1.
		// So Opcode 3 -> Case 1.
		
		if (par1 == 3) {
			// Inside Case 1: local_c = local_14[2] which is offset +4 (par3)
			if (par3 == 5) {
				// Damage Logic
				// bVar1 = FUN_423970(local_14[5]); // Check if source enemy (offset +10 / par6?) is active
				// Read extra params from stream
				// Note: `procIACT` already read parems into par1..par4.
				// But Opcode 3 logic in FUN_401234 uses offset +10 (par6) which isn't passed to `procIACT` signature fully?
				// Wait, `procIACT` reads 4 shorts.
				// par1 (+0), par2 (+2), par3 (+4), par4 (+6).
				// We need +8 and +10.
				
				b.skip(2); // Offset +8 (par5 used in assembly but not here yet)
				int16 par6 = b.readSint16LE(); // Offset +10 (Enemy ID?)
				
				// Check if enemy is disabled (processed in FUN_423970)
				bool enemyDisabled = isBitSet(par6);
				
				if (!enemyDisabled) {
					// Probability check: Random(100) < Limit
					// The limit seems to come from a table or fixed value?
					// In FUN_401234: `sVar2 = *(short *)(&DAT_0047e0fc + ...)`
					// For now, let's just use a fixed probability or assume successful hit
					// if (rnd < prob) ...
					
					// Increment damage
					// In assembly: DAT_0047a7ec += damage
					// Damage amount also comes from table?
					int damageAmount = 5; // Placeholder
					
					// Only apply damage occasionally to simulate probability
					if ((_vm->_system->getMillis() % 100) < 20) { // 20% chance per frame (approx)
						_playerDamage += damageAmount;
						if (_playerDamage > 255) _playerDamage = 255;
						
						debug("Rebel2: Player HIT by Enemy %d. Damage=%d", par6, _playerDamage);
						
						// TODO: Flash screen red / shake
					}
				}
			}
		}
		
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
	bool found = false;
	Common::List<enemy>::iterator it;
	for (it = _enemies.begin(); it != _enemies.end(); ++it) {
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
		init_enemyStruct(enemyId, x, y, w, h, true, false, -1);
	}
}

void InsaneRebel2::init_enemyStruct(int id, int32 x, int32 y, int32 w, int32 h, bool active, bool destroyed, int32 explosionFrame) {
	enemy e;
	e.id = id;
	e.rect = Common::Rect(x, y, x + w, y + h);
	e.active = active;
	e.destroyed = destroyed;
	e.explosionFrame = explosionFrame;
	e.savedBackground = nullptr;
	e.savedBgWidth = 0;
	e.savedBgHeight = 0;
	_enemies.push_back(e);
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

void InsaneRebel2::spawnExplosion(int x, int y, int objectHalfWidth) {
	// Find first free slot (FUN_40A2E0 logic)
	for (int i = 0; i < 5; i++) {
		if (!_explosions[i].active || _explosions[i].counter <= 0) {
			_explosions[i].active = true;
			_explosions[i].counter = 10;
			_explosions[i].x = x;
			_explosions[i].y = y;
			_explosions[i].scale = objectHalfWidth;
			// TODO: Play sound via FUN_0041189e equivalent
			break;
		}
	}
}

void InsaneRebel2::spawnShot(int x, int y) {
	// Find free shot slot (2 slots total)
	for (int i = 0; i < 2; i++) {
		if (!_shots[i].active) {
			_shots[i].active = true;
			_shots[i].counter = 4; // Lasts 4 frames
			_shots[i].x = x + _viewX;
			_shots[i].y = y + _viewY;
			// TODO: Play laser sound
			break;
		}
	}
}

void InsaneRebel2::drawTexturedLine(byte *dst, int pitch, int width, int height, int x0, int y0, int x1, int y1, NutRenderer *nut, int spriteIdx, int v, bool mask231) {
	if (!nut || spriteIdx >= nut->getNumChars()) return;

	const byte *srcData = nut->getCharData(spriteIdx);
	int texW = nut->getCharWidth(spriteIdx);
	int texH = nut->getCharHeight(spriteIdx);
	
	if (!srcData || texW <= 0 || texH <= 0) return;
	if (v < 0) v = 0;
	if (v >= texH) v = texH - 1;

	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2;
	
	// Total length approximation for UV mapping
	int totalDist = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
	if (totalDist == 0) totalDist = 1;
	
	int currentDist = 0;

	for (;;) {
		if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height) {
			// Map currentDist/totalDist to 0..texW (Run along texture width)
			int u = (currentDist * texW) / totalDist;
			if (u >= texW) u = texW - 1;
			
			byte color = srcData[v * texW + u];
			
			// Check for transparency (0 and optionally 231)
			if (color != 0 && (!mask231 || color != 231)) { 
				dst[y0 * pitch + x0] = color;
			}
		}
		
		if (x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; }
		if (e2 <= dx) { err += dx; y0 += sy; }
		
		currentDist++;
	}
}

// Helper: draw a textured segment between two points using the game's original routine (FUN_00429360 port)
void drawTexturedSegment(byte *dst, int pitch, int width, int height, int param_3, int param_4, int param_5, int param_6, int param_7, const byte *param_8) {
	// Ported from FUN_00429360 (decompiled). Only 0 in texture is transparent.
	int sVar4 = 0;                // left
	int sVar1 = 0;                // top
	int sVar7 = width - 1;        // right
	int sVar10 = height - 1;      // bottom

	int px0 = param_3;
	int py0 = param_4;
	int px1 = param_5;
	int py1 = param_6;

	// Clip against screen bounds (translation of original clipping logic)
	if (px0 == px1) {
		if (px0 < sVar4 || px0 > sVar7) return;
	} else {
		if (px0 < sVar4) {
			if (px1 < sVar4) return;
			py0 = py1 + ((py0 - py1) * (sVar4 - px1)) / (px0 - px1);
			px0 = sVar4;
		} else if (px0 > sVar7) {
			if (px1 > sVar7) return;
			py0 = py1 + ((py0 - py1) * (sVar7 - px1)) / (px0 - px1);
			px0 = sVar7;
		}
		if (px1 < sVar4) {
			py1 = py0 + ((py1 - py0) * (sVar4 - px0)) / (px1 - px0);
			px1 = sVar4;
		} else if (px1 > sVar7) {
			py1 = py0 + ((py1 - py0) * (sVar7 - px0)) / (px1 - px0);
			px1 = sVar7;
		}
	}

	if (py0 == py1) {
		if (py0 < sVar1 || py0 > sVar10) return;
	} else {
		if (py0 < sVar1) {
			if (py1 < sVar1) return;
			px0 = px1 + ((px0 - px1) * (sVar1 - py1)) / (py0 - py1);
			py0 = sVar1;
		} else if (py0 > sVar10) {
			if (py1 > sVar10) return;
			px0 = px1 + ((px0 - px1) * (sVar10 - py1)) / (py0 - py1);
			py0 = sVar10;
		}
		if (py1 < sVar1) {
			px1 = px0 + ((px1 - px0) * (sVar1 - py0)) / (py1 - py0);
			py1 = sVar1;
		} else if (py1 > sVar10) {
			px1 = px0 + ((px1 - px0) * (sVar10 - py0)) / (py1 - py0);
			py1 = sVar10;
		}
	}

	int dx = px1 - px0;
	int dy = py1 - py0;
	int absdx = dx < 0 ? -dx : dx;
	int absdy = dy < 0 ? -dy : dy;

	// pointer into destination and texture
	byte *baseDst = dst;
	const byte *texPtr = param_8;

	if (absdx == 0) {
		if (absdy == 0) {
			if (*texPtr != 0) baseDst[py0 * pitch + px0] = *texPtr;
			return;
		}
		// vertical-ish
		int step = absdy + 1;
		int curY = py0;
		int signY = dy > 0 ? 1 : -1;
		int iVar9 = step; // adv counter
		for (int i = 0; i < step; i++) {
			if (*texPtr != 0) baseDst[curY * pitch + px0] = *texPtr;
			curY += signY;
			iVar9 -= param_7;
			while (iVar9 < 0) { texPtr++; iVar9 += step; }
		}
		return;
	}

	if (absdy == 0) {
		// horizontal-ish
		int step = absdx + 1;
		int curX = px0;
		int signX = dx > 0 ? 1 : -1;
		int iVar11 = step;
		for (int i = 0; i < step; i++) {
			if (*texPtr != 0) baseDst[py0 * pitch + curX] = *texPtr;
			curX += signX;
			iVar11 -= param_7;
			while (iVar11 < 0) { texPtr++; iVar11 += step; }
		}
		return;
	}

	// general case
	int steps = (absdx > absdy) ? absdx + 1 : absdy + 1;
	int x = px0, y = py0;
	int sx = dx > 0 ? 1 : -1;
	int sy = dy > 0 ? 1 : -1;
	int err = absdx - absdy;
	int iVar12 = steps;

	for (int i = 0; i < steps; i++) {
		if (x >= 0 && x < width && y >= 0 && y < height) {
			if (*texPtr != 0) baseDst[y * pitch + x] = *texPtr;
		}
		int e2 = 2 * err;
		if (e2 > -absdy) { err -= absdy; x += sx; }
		if (e2 < absdx) { err += absdx; y += sy; }
		iVar12 -= param_7;
		if (iVar12 < 0) { texPtr++; iVar12 += steps; }
	}
}


void InsaneRebel2::drawLaserBeam(byte *dst, int pitch, int width, int height, int x0, int y0, int x1, int y1, int progress, int maxProgress, int thickness, int param_9, NutRenderer *nut, int spriteIdx) {
	if (!nut || spriteIdx >= nut->getNumChars()) return;

	// Follow original FUN_0040BBF6 math precisely
	int texW = nut->getCharWidth(spriteIdx);
	int texH = nut->getCharHeight(spriteIdx);
	int param_11 = (_rebelLevelType <= 1) ? 12 : 25;

	if (maxProgress == 0) maxProgress = 1;
	int sVar7 = (param_11 * progress * 16) / maxProgress;

	int dx = x1 - x0;
	int dy = y1 - y0;
	int sVar6 = ((dx) * (param_11 + 1)) / param_11;
	int sVar1 = ((dy) * (param_11 + 1)) / param_11;

	int sVar4 = (sVar6 + x0) - (sVar6 * 16) / (sVar7 + 16);
	int sVar5 = (sVar1 + y0) - (sVar1 * 16) / (sVar7 + 16);
	int sVar6_end = (sVar6 + x0) - (sVar6 * 16) / (param_9 + sVar7 + 16);
	int sVar7_end = (sVar1 + y0) - (sVar1 * 16) / (param_9 + sVar7 + 16);

	const byte *srcBase = nut->getCharData(spriteIdx);
	if (!srcBase || texW <= 0 || texH <= 0) return;

	int iVar2 = abs(sVar5 - sVar7_end);
	int iVar3 = abs(sVar4 - sVar6_end);

	if (iVar2 < iVar3) {
		// Column major case (wide)
		iVar2 = abs(sVar4 - sVar6_end);
		long long temp = (long long)iVar2 * (long long)texH * (long long)thickness;
		// sVar1calc = (temp >> 3) / texW + 2
		int sVar1calc = (int)((temp >> 3) / texW) + 2;
		int local_24 = -sVar1calc;
		int sVar8 = sVar1calc >> 1;
		const byte *local_28 = srcBase;
		for (int local_2c = 0; local_2c < sVar1calc; local_2c++) {
			drawTexturedSegment(dst, pitch, width, height, sVar4, (sVar5 - sVar8) + local_2c,
						 sVar6_end, (sVar7_end - sVar8) + local_2c, texW, local_28);
			for (local_24 = texH + local_24; local_24 > 0; local_24 -= sVar1calc) {
				local_28 += texW;
			}
		}
	} else {
		// Row major case (tall)
		iVar2 = abs(sVar5 - sVar7_end);
		int local_30 = (int)(((long long)iVar2 * (long long)texH) / texW) + 2;
		if (texH < local_30) local_30 = texH;
		int local_24 = -local_30;
		const byte *local_28 = srcBase;
		int sVar1_half = local_30 >> 1;
		for (int local_2c = 0; local_2c < local_30; local_2c++) {
			drawTexturedSegment(dst, pitch, width, height, (sVar4 - sVar1_half) + local_2c, sVar5,
						 (sVar6_end - sVar1_half) + local_2c, sVar7_end, texW, local_28);
			for (local_24 = texH + local_24; local_24 > 0; local_24 -= local_30) {
				local_28 += texW;
			}
		}
	}
}
void InsaneRebel2::drawLine(byte *dst, int pitch, int width, int height, int x0, int y0, int x1, int y1, byte color) {
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2;

	for (;;) {
		if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height) {
			dst[y0 * pitch + x0] = color;
		}
		if (x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; }
		if (e2 <= dx) { err += dx; y0 += sy; }
	}
}

void InsaneRebel2::drawCornerBrackets(byte *dst, int pitch, int width, int height, int x, int y, int w, int h, byte color) {
	// Draw L-shaped brackets at corners of the rect (x,y,w,h)
	// Bracket size: approx 8 pixels
	int armLen = 2;
	if (armLen > w / 2) armLen = w / 2;
	if (armLen > h / 2) armLen = h / 2;

	int x2 = x + w - 1;
	int y2 = y + h - 1;

	// Top-Left Corner
	drawLine(dst, pitch, width, height, x, y, x + armLen, y, color);
	drawLine(dst, pitch, width, height, x, y, x, y + armLen, color);

	// Top-Right Corner
	drawLine(dst, pitch, width, height, x2 - armLen, y, x2, y, color);
	drawLine(dst, pitch, width, height, x2, y, x2, y + armLen, color);

	// Bottom-Left Corner
	drawLine(dst, pitch, width, height, x, y2, x + armLen, y2, color);
	drawLine(dst, pitch, width, height, x, y2 - armLen, x, y2, color);

	// Bottom-Right Corner
	drawLine(dst, pitch, width, height, x2 - armLen, y2, x2, y2, color);
	drawLine(dst, pitch, width, height, x2, y2 - armLen, x2, y2, color);
}

void InsaneRebel2::renderNutSprite(byte *dst, int pitch, int width, int height, int x, int y, NutRenderer *nut, int spriteIdx) {
	if (!nut || spriteIdx < 0 || spriteIdx >= nut->getNumChars()) return;

	int w = nut->getCharWidth(spriteIdx);
	int h = nut->getCharHeight(spriteIdx);
	const byte *src = nut->getCharData(spriteIdx);

	// Clipping
	int drawX = x;
	int drawY = y;
	int drawW = w;
	int drawH = h;
	int srcOffsetX = 0;
	int srcOffsetY = 0;

	if (drawX < 0) {
		srcOffsetX = -drawX;
		drawW += drawX;
		drawX = 0;
	}
	if (drawY < 0) {
		srcOffsetY = -drawY;
		drawH += drawY;
		drawY = 0;
	}

	if (drawX + drawW > width) {
		drawW = width - drawX;
	}
	if (drawY + drawH > height) {
		drawH = height - drawY;
	}

	if (drawW <= 0 || drawH <= 0) return;

	// Draw loop
	for (int iy = 0; iy < drawH; iy++) {
		const byte *s = src + (srcOffsetY + iy) * w + srcOffsetX;
		byte *d = dst + (drawY + iy) * pitch + drawX;
		for (int ix = 0; ix < drawW; ix++) {
			byte px = s[ix];
			if (px != 231 && px != 0) { // Check both 0 and 231 (0xE7) for transparency
				d[ix] = px;
			}
		}
	}
}

void InsaneRebel2::procPostRendering(byte *renderBitmap, int32 codecparam, int32 setupsan12,
							   int32 setupsan13, int32 curFrame, int32 maxFrame) {

	// Determine correct pitch for the video buffer (usually 320 for Rebel2)
	int width = _player->_width;
	int height = _player->_height;
	if (width == 0) width = _vm->_screenWidth;
	if (height == 0) height = _vm->_screenHeight;
	int pitch = width;

	// Calculate View/Scroll Offsets
	// Rebel Assault 2 uses a buffer larger (424x260) than screen (320x200)
	// Map mouse X (0-320) to Scroll X (0-104)
	// Map mouse Y (0-200) to Scroll Y (0-60)
	int maxScrollX = width - _vm->_screenWidth;
	int maxScrollY = height - _vm->_screenHeight;
	
	if (maxScrollX < 0) maxScrollX = 0;
	if (maxScrollY < 0) maxScrollY = 0;
	
	// Simple linear mapping: Center of screen corresponds to center of buffer
	_viewX = (_vm->_mouse.x * maxScrollX) / _vm->_screenWidth;
	_viewY = (_vm->_mouse.y * maxScrollY) / _vm->_screenHeight;
	
	_player->setScrollOffset(_viewX, _viewY);

	processMouse();

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
	// We draw directly to screen at Y=180
	
	// Use video content coordinates, NOT buffer coordinates
	const int videoWidth = 320;    // Native video width
	const int videoHeight = 200;   // Native video height  
	const int statusBarY = 180;    // 0xb4 - status bar starts at Y=180 in video coords

	// Hide HUD/status bar during intro videos (marked by SmushPlayer video flag 0x20)
	bool introPlaying = ((_player->_curVideoFlags & 0x20) != 0);
	if (!introPlaying) {
	// ============================================================
	// STEP 0: Fill status bar background (FUN_004288c0 equivalent)
	// ============================================================
	// Clear the status bar area at Y=180-199 with background color
	// Original assembly: FUN_004288c0(local_8, 0, 0, 0xb4, 0x140, 0x14, 4)
	// This fills width=320, height=20 starting at Y=180 with color index 4
	const byte statusBarBgColor = 4;

	for (int y = statusBarY; y < videoHeight; y++) {
		int destY = y + _viewY;
		if (destY >= height) continue;
		for (int x = 0; x < videoWidth; x++) {
			int destX = x + _viewX;
			if (destX >= pitch) continue;
			renderBitmap[destY * pitch + destX] = statusBarBgColor;
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

			// Apply View Offset for static screen elements
			destX += _viewX;
			destY += _viewY;
			
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
	if (_smush_cockpitNut) {
		// Draw status bar background frame (sprite 1) at (0, statusBarY)
		// This sprite is the full-width status bar background
		if (_smush_cockpitNut->getNumChars() > 1) {
			renderNutSprite(renderBitmap, pitch, width, height, _viewX, statusBarY + _viewY, _smush_cockpitNut, 1);
		}
		
		// Draw difficulty indicator (sprites 2-5 based on difficulty level 0-3)
		// Sprite index = difficulty + 2; capped at 4 max difficulty (sprite 5)
		// Assembly draws at (0,0) in buffer - same position as sprite 1
		int difficulty = 0;  // TODO: Read from game state (DAT_0047a7fa)
		if (difficulty > 3) difficulty = 3;
		int difficultySprite = difficulty + 2;  // sprites 2, 3, 4, or 5
		if (_smush_cockpitNut->getNumChars() > difficultySprite) {
			renderNutSprite(renderBitmap, pitch, width, height, _viewX, statusBarY + _viewY, _smush_cockpitNut, difficultySprite);
		}
		
		// Draw shield bar (sprite 6) 
		// Assembly uses clip rect: X=0x3f(63), Y=0x9(9), W=0x40(64), H=0x6(6)
		// The width is scaled based on shield value (param_1 >> 2)
		// For now, draw at position (0, statusBarY) - sprite has internal positioning
		if (_smush_cockpitNut->getNumChars() > 6) {
			// Calculate width based on damage. 
			// Assuming max damage 255 = empty bar.
			// Bar max width is 64 pixels.
			// Damage 0 = Width 64. Damage 255 = Width 0.
			// Width = 64 - (Damage / 4)
			int barWidth = 64 - (_playerDamage / 4);
			if (barWidth < 0) barWidth = 0;
			
			// We need to draw a partial sprite or use a clip rect.
			// smlayer_drawSomething supports scaling/clip?
			// The current implementation of smlayer_drawSomething just draws the whole sprite.
			// We can pass a "frame" or clip rect if we modify the function or use a lower level draw.
			// For now, let's just draw the full sprite if damage < 255, to verify it appears.
			// Ideally we should implement clipping.
			
			// NOTE: smlayer_drawSomething calls `_smush_cockpitNut->draw(...)`
			// We can't easily clip without modifying NutRenderer or using a custom draw loop.
			// Let's implement a custom draw loop for the shield bar here since it's simple.
			
			// Sprite 6 data
			const byte *src = _smush_cockpitNut->getCharData(6);
			int sw = _smush_cockpitNut->getCharWidth(6);
			int sh = _smush_cockpitNut->getCharHeight(6);
			int sx = 63; // Hardcoded X offset from assembly
			int sy = 9;  // Hardcoded Y offset (relative to status bar top)
			
			// Draw clipped width
			if (src && sw > 0 && sh > 0) {
				int drawWidth = MIN(barWidth, sw);
				for (int y = 0; y < sh; y++) {
					for (int x = 0; x < drawWidth; x++) {
						// Render to (0 + sx + x + viewX, statusBarY + sy + y + viewY)
						int destX = sx + x + _viewX;
						int destY = statusBarY + sy + y + _viewY;
						if (destX < pitch && destY < height) {
							byte pixel = src[y * sw + x];
							if (pixel != 0) {
								renderBitmap[destY * pitch + destX] = pixel;
							}
						}
					}
				}
			}
		}
		
		// Draw shield alert overlay (sprite 7) when shields critical (> 0xAA = 170)
		// Only draws when frame counter bit 3 is clear (every 8 frames)
		// For now, skip - TODO: implement shield critical flashing
		
		// Draw lives indicator - assembly shows at X=0xa8 (168), Y=7
		// Uses sprite 1 again with different clip rect
		// TODO: Implement lives rendering
		
		// Draw score - uses FUN_00434cb0 (text rendering) at X=0x101(257)
		// TODO: Implement score rendering
	} 

	Common::List<enemy>::iterator it;
	for (it = _enemies.begin(); it != _enemies.end(); ++it) {
		Common::Rect r = it->rect;
		
		// Clip the rect to screen bounds for safety
		Common::Rect clipped = r;
		clipped.clip(Common::Rect(0, 0, width, height));

		if (!clipped.isValidRect()) continue;
		
		if (it->destroyed) {
			// Handle destroyed enemies - draw explosion animation
			// The enemy frame updates are now skipped in decodeFrameObject via shouldSkipFrameUpdate
			// Note: Explosion rendering is now handled by the separate 5-slot system
		} else if (it->active && !isBitSet(it->id)) { // Only draw if active AND not disabled by IACT
			// Draw Green Indicators (Corner Brackets) for Easy (0) and Medium (1) difficulty
			// Hard (2) mode does not show indicators.
			if (_difficulty < 2) {
				const byte color = 5; // Green color index for brackets
				// Clip the rect to screen bounds for drawing logic is handled inside drawLine if implemented safely,
				// but drawCornerBrackets relies on drawLine which iterates.
				// We pass full screen width/height to safe-guard.
				drawCornerBrackets(renderBitmap, pitch, width, height, r.left, r.top, r.width(), r.height(), color);
			}
		}
	}

	// Draw 5-slot Explosion System
	if (_smush_iconsNut) {
		for (int i = 0; i < 5; i++) {
			if (_explosions[i].active) {
				if (_explosions[i].counter <= 0) {
					_explosions[i].active = false;
					continue;
				}

				// Determine base sprite index based on scale (FUN_409FBC logic)
				int baseIndex;
				if (_explosions[i].scale < 11) {
					baseIndex = 9;  // Small/Medium transition
				} else if (_explosions[i].scale < 21) {
					baseIndex = 19; // Medium/Large transition
				} else {
					baseIndex = 29; // Large/XL transition
				}
				
				// Formula: Base + (12 - Counter)
				// Counter goes 10 -> 1.
				// Frame goes Base+2 -> Base+11.
				int spriteIndex = baseIndex + (12 - _explosions[i].counter);
				
				if (_smush_iconsNut->getNumChars() > spriteIndex) {
					int ew = _smush_iconsNut->getCharWidth(spriteIndex);
					int eh = _smush_iconsNut->getCharHeight(spriteIndex);
					int cx = _explosions[i].x - ew / 2;
					int cy = _explosions[i].y - eh / 2;
					
					// Draw explosion
					renderNutSprite(renderBitmap, pitch, width, height, cx, cy, _smush_iconsNut, spriteIndex);
				}

				_explosions[i].counter--;
			}
		}
	}

	// Draw Laser Shots
	// Gun Positions (Approximate for Turret Mode / Default):
	// Left: (10, 190), Right: (310, 190) - Adjusted for low-res
	const int GUN_LEFT_X = 10;
	const int GUN_LEFT_Y = 190;
	const int GUN_RIGHT_X = 310;
	const int GUN_RIGHT_Y = 190;

	for (int i = 0; i < 2; i++) {
		if (_shots[i].active) {
			if (_shots[i].counter <= 0) {
				_shots[i].active = false;
				continue;
			}
			
			// Use Sprite 0 from CPITIMAG.NUT as the laser texture (15x15 projectile)
			// Confirmed by info.md: indexes 0-4 are laser/projectile
			if (_smush_iconsNut && _smush_iconsNut->getNumChars() > 0) {
				// Calculate progress
				int maxProgress = 4; // Max duration from table (supposedly)
				int progress = maxProgress - _shots[i].counter;

				// Draw Beams depending on Level Type
				// Scene 1 (LevelType 1) has 3 beams: Right, Middle, Left
				if (_rebelLevelType <= 1) { // Default or Type 1
					// Right Beam: Origin(310, 170), Thickness 8, LengthFac 12
					drawLaserBeam(renderBitmap, pitch, width, height, 
						310 + _viewX, 170 + _viewY, 
						_shots[i].x, _shots[i].y, 
						progress, maxProgress, 8, 12, _smush_iconsNut, 0);

					// Middle Beam: Origin(160, 380), Thickness 5, LengthFac 8
					// Note: 380 is virtual origin below screen
					drawLaserBeam(renderBitmap, pitch, width, height, 
						160 + _viewX, 380 + _viewY, 
						_shots[i].x, _shots[i].y, 
						progress, maxProgress, 5, 8, _smush_iconsNut, 0);

					// Left Beam: Origin(10, 170), Thickness 8, LengthFac 12
					drawLaserBeam(renderBitmap, pitch, width, height, 
						10 + _viewX, 170 + _viewY, 
						_shots[i].x, _shots[i].y, 
						progress, maxProgress, 8, 12, _smush_iconsNut, 0);
						
				} else {
					// Fallback for other levels (2 beams)
					drawLaserBeam(renderBitmap, pitch, width, height, 
						GUN_LEFT_X + _viewX, GUN_LEFT_Y + _viewY, 
						_shots[i].x, _shots[i].y, 
						progress, maxProgress, 8, 12, _smush_iconsNut, 0);

					drawLaserBeam(renderBitmap, pitch, width, height, 
						GUN_RIGHT_X + _viewX, GUN_RIGHT_Y + _viewY, 
						_shots[i].x, _shots[i].y, 
						progress, maxProgress, 8, 12, _smush_iconsNut, 0);
				}

				// Draw Projectile Impact
				// Using Sprite 0 (small flash) or similar at impact point
				renderNutSprite(renderBitmap, pitch, width, height, _shots[i].x - 7, _shots[i].y - 7, _smush_iconsNut, 0);
			}
			
			_shots[i].counter--;
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
			// Center the crosshair on mouse position (in world coordinates)
			renderNutSprite(renderBitmap, pitch, width, height, _vm->_mouse.x - cw / 2 + _viewX, _vm->_mouse.y - ch / 2 + _viewY, _smush_iconsNut, reticleIndex);
		}
	}
}

}

}
