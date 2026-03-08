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

// LVL1 stage-2 0x5D damage/event codes observed in L1PLAY2.ANM.
// Original DOS loop uses table/mask-driven latch routing before FUN_1E6A7;
// in ScummVM's direct GAME dispatch path, map this known range explicitly.
static inline bool isL1Stage2DamageLatch(uint16 code) {
	return code >= 6 && code <= 18;
}

// FUN_223FE (0x223FE) coordinate transform used by FUN_1C54D/FUN_1C6E9.
// The original applies camera X offset directly and a Y term derived from
// DAT_41A2 (+ curve-table contribution). In current RA1 integration we keep
// the camera-offset part, which fixes left/right corridor asymmetry.
static void transformPoint223FE(int16 &x, int16 &y, int16 cameraX, int16 cameraY) {
	x = (int16)(x - cameraX);
	y = (int16)(y - cameraY);
}

// updateShipPhysics — FUN_1DEB5 (0x1DEB5). Accumulator-based position system.
// Roll accumulator (_74CA) driven by input, position accumulators (_74C2/_74C6)
// driven by roll + drift + cross-coupling. Ship position = base + accum >> 8.
void InsaneRebel1::updateShipPhysics() {
	_frameCounter++;

	// Decrement cooldown
	if (_damageCooldown > 0)
		_damageCooldown--;

	// --- Step 1: Mouse input as offset from screen center ---
	// Original: _DAT_756C (horizontal), _DAT_756E (vertical) in 320x200 space.
	int16 mouseX = (int16)_vm->_mouse.x;
	int16 mouseY = (int16)_vm->_mouse.y;
	if (_player && _player->_width > 0 && _player->_height > 0 &&
		(mouseX >= 320 || mouseY >= 200)) {
		mouseX = (int16)((mouseX * 320) / _player->_width);
		mouseY = (int16)((mouseY * 200) / _player->_height);
	}
	int16 inputX = (int16)(mouseX - 160);
	int16 inputY = (int16)(mouseY - 100);
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

	// Health regeneration: +1 every 32 frames (from original asm)
	if ((_frameCounter & 0x1F) == 0) {
		if (_health >= 0 && _health < kMaxHealth)
			_health++;
		if (_health >= 0)
			_score += 1;
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
	const byte modeFlags = 0;

	// RA1 latches consumed by handler family in FUN_1B297.
	if (_gameLatch5D == 0xFFFF || (_currentLevel == 0 && _flyControlMode == 2 &&
		isL1Stage2DamageLatch(_gameLatch5D)))
		_damageFlags |= 0x40;
	if (_gameLatch5F != 0 && _vm->_rnd.getRandomNumber((uint16)(_gameLatch5F - 1)) == 0)
		_damageFlags |= 0x80;

	if (counter == 0) {
		_posAccumX = 0;
		_posAccumY = 0;
		_rollAccum = 0;
		_liftSmooth = 0;
		_shipPosX = kRA1CenterX;
		_shipPosY = kRA1CenterY;
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
		int16 mouseX = (int16)_vm->_mouse.x;
		int16 mouseY = (int16)_vm->_mouse.y;
		if (_player && _player->_width > 0 && _player->_height > 0 &&
			(mouseX >= 320 || mouseY >= 200)) {
			mouseX = (int16)((mouseX * 320) / _player->_width);
			mouseY = (int16)((mouseY * 200) / _player->_height);
		}

		// FUN_1F3F8/FUN_231BE preprocesses mouse deltas before FUN_1E6A7 consumes
		// DAT_756C/DAT_756E. Keep X in 320-space then scale to ±127 (RA2 parity),
		// Y clamped to ±127.
		int16 rawInputX = CLIP<int16>((int16)(mouseX - 160), -160, 160);
		int16 inputX = (int16)((rawInputX * 127) / 160);
		int16 inputY = CLIP<int16>((int16)(mouseY - 100), -127, 127);

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

	// FUN_1D79C tail sets pointer center from offsets:
	//   _74BE = _74B6 + _74BA
	//   _74C0 = (_74B8 + _74BC - 0x23) - (_74BC >> 3)
	_shipPosX = (int16)(kRA1CenterX + offsetX);
	_shipPosY = (int16)((kRA1CenterY + offsetY - 0x23) - (offsetY >> 3));

	_perspectiveX = CLIP<int16>((int16)(offsetX + 0x20), 0, 0x40);
	_perspectiveY = CLIP<int16>((int16)(offsetY + 0x17), 0, 0x2E);

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

	// Regeneration via FUN_1BB0E call in this path.
	if ((_frameCounter & 0x1F) == 0) {
		if (_health >= 0 && _health < kMaxHealth)
			_health++;
		if (_health >= 0)
			_score += 1;
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
	if (_gameLatch5D == 0xFFFF)
		_damageFlags |= 0x40;
	if (_gameLatch5F != 0 && _vm->_rnd.getRandomNumber((uint16)(_gameLatch5F - 1)) == 0)
		_damageFlags |= 0x80;

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
	int16 mouseX = (int16)_vm->_mouse.x;
	int16 mouseY = (int16)_vm->_mouse.y;
	if (_player && _player->_width > 0 && _player->_height > 0 &&
		(mouseX >= 320 || mouseY >= 200)) {
		mouseX = (int16)((mouseX * 320) / _player->_width);
		mouseY = (int16)((mouseY * 200) / _player->_height);
	}
	int16 inputX = (int16)(mouseX - kRA1CenterX);
	// Assembly uses an inverted-Y convention in the averaging path.
	// In ScummVM screen coords (Y grows downward), convert here so moving
	// mouse up moves the pointer up on screen.
	int16 inputY = (int16)(kRA1CenterY - mouseY);
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

	_frameCounter++;

	debug(7, "RA1 asteroid: pos=(%d,%d) avg=(%d,%d) view=(%d,%d) health=%d flash=%d",
		_shipPosX, _shipPosY, _avgInputX, _avgInputY,
		_perspectiveX, _perspectiveY, _health, _screenFlash);
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

		// Shooting/targeting reset
		_playerFired = false;
		_fireCooldown = 0;
		memset(_shotSlots, 0, sizeof(_shotSlots));
		_shotAlternator = 0;
		_targetProximity = 0;
		_prevTargetProx = 0;
		_targetCount = 0;
		_prevTargetCount = 0;
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
		debug(5, "RA1 GAME 0x5E: reset state field1=%d mode=%d", (int32)param1, (int)_flyControlMode);
		break;

	case 0x5D:
		_gameLatch5D = (uint16)param1;
		debug(5, "RA1 GAME 0x5D (link/event latch) param=%u", _gameLatch5D);
		break;

	case 0x5F:
		_gameLatch5F = (uint16)param1;
		debug(5, "RA1 GAME 0x5F (random-hit latch) param=%u", _gameLatch5F);
		break;

	case 0x07:
		_activeGameOpcode = 0x07;
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
			transformPoint223FE(centerX, centerY, _perspectiveX, _perspectiveY);

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
			transformPoint223FE(centerX, centerY, _perspectiveX, _perspectiveY);

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
		// Asteroid/surface per-frame handler (FUN_1CDA7).
		// field1 = frame counter, field2 = max frames
		_gameCounter = param1;
		if (subSize >= 20) {
			uint32 maxFrames = b.readUint32BE(); // field2 (max frames)
			b.readUint32BE(); // field3
			b.readUint32BE(); // field4

			// RA1 scripts drive progression with GAME counters. In LVL2, finish the
			// interactive SMUSH once the script counter reaches the terminal frame.
			// This avoids getting stuck if container/frame parsing continues past the
			// intended gameplay endpoint.
			if (_interactiveVideoActive && _currentLevel == 1 && maxFrames > 0 &&
				_gameCounter >= (int32)maxFrames - 1) {
				_vm->_smushVideoShouldFinish = true;
				debug(1, "RA1 L2: finishing interactive video at GAME 0x0B counter=%d/%u", _gameCounter, maxFrames);
			}
		}
		debug(7, "RA1 GAME 0x0B: counter=%d", _gameCounter);
		break;

	case 0x5A:
		// Target detection — FUN_1C0EF (0x1C0EF). AABB from video stream.
		// Params: targetIdx, left, top, width, height
		if (subSize >= 24) {
			int16 targetIdx = (int16)param1;
			int16 left = (int16)b.readUint32BE();
			int16 top = (int16)b.readUint32BE();
			int16 w = (int16)b.readUint32BE();
			int16 h = (int16)b.readUint32BE();
			int16 right = left + w;
			int16 bottom = top + h;
			checkTargetHit(targetIdx, left, top, right, bottom);
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
	if (!_playerFired)
		return;

	// Find first available slot (timer < 1 or > 5), matching FUN_1CCA0.
	int slot = -1;
	for (int i = 0; i < kMaxShotSlots; i++) {
		if (_shotSlots[i].timer <= 0 || _shotSlots[i].timer > 5) {
			slot = i;
			break;
		}
	}
	if (slot < 0) {
		_playerFired = false;
		return;
	}

	// Record shot at current cursor position.
	const bool turretMode = (_activeGameOpcode == 0x08 || _activeGameOpcode == 0x0A);
	const int16 shipCenterX = turretMode ? (int16)(kRA1CenterX + (_perspectiveX - 0x20)) : _shipPosX;
	const int16 shipCenterY = turretMode ? (int16)(kRA1CenterY + (_perspectiveY - 0x17)) : _shipPosY;

	_shotSlots[slot].timer = 5;
	_shotSlots[slot].posX = _shipPosX;
	_shotSlots[slot].posY = _shipPosY;
	_shotSlots[slot].centerX = shipCenterX;
	_shotSlots[slot].centerY = shipCenterY;
	_shotSlots[slot].variant = _shotAlternator;
	_shotAlternator = 1 - _shotAlternator;

	_playerFired = false;

	debug(5, "RA1 shot: slot=%d pos=(%d,%d)", slot, _shotSlots[slot].posX, _shotSlots[slot].posY);
}

// checkTargetHit — FUN_1C0EF (0x1C0EF). AABB target detection with snap tolerance.
// Called from GAME 0x5A handler. Checks cursor proximity and shot hits.
void InsaneRebel1::checkTargetHit(int16 targetIdx, int16 left, int16 top, int16 right, int16 bottom) {
	int16 snap = _tuning.snap;
	int16 curX = _shipPosX;
	int16 curY = _shipPosY;

	_targetCount++;

	// Check proximity: cursor within target + snap + 5 margin
	if (curX > left - snap - 5 && curX < right + snap + 5 &&
		curY > top - snap - 5 && curY < bottom + snap + 5) {
		if (_targetProximity == 0)
			_targetProximity = 1;  // Near

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

						// Snap cursor to target center (original: _DAT_74BE/74C0 = target center)
						if (snap > 0) {
							_shipPosX = (left + right) / 2;
							_shipPosY = (top + bottom) / 2;
						}

						debug(5, "RA1 HIT: target=%d score=%d kills=%d", targetIdx, _score, _killCount);
						return;
					}
				}
			}
		}
	}
}

} // End of namespace Scumm
