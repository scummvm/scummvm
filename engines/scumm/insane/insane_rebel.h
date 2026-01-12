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

#if !defined(SCUMM_INSANE_REBEL_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_INSANE_REBEL_H

#include "scumm/nut_renderer.h"

#include "scumm/smush/smush_player.h"

#include "scumm/insane/insane.h"

#include "common/list.h"
#include "common/rect.h"

namespace Scumm {

class InsaneRebel2 : public Insane {

public:
	InsaneRebel2(ScummEngine_v7 *scumm);
	~InsaneRebel2();

	NutRenderer *_smush_cockpitNut;
	NutRenderer *_smush_dispfontNut;  // DAT_00482200 - DISPFONT.NUT for status bar (difficulty, shields, lives, score)

	// Font used for opcode 9 text/subtitle rendering (DIHIFONT / TALKFONT)
	SmushFont *_rebelMsgFont;
	bool _introCursorPushed; // true when we've pushed an invisible cursor for intro
	

	int32 processMouse() override;
	bool isBitSet(int n) override;
	void setBit(int n) override;

	void iactRebel2Scene1(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				  int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
				  int16 par1, int16 par2, int16 par3, int16 par4);

	void procPostRendering(byte *renderBitmap, int32 codecparam, int32 setupsan12,
							   int32 setupsan13, int32 curFrame, int32 maxFrame) override;

	void procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					  int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
					  int16 par1, int16 par2, int16 par3, int16 par4) override;

	void drawLine(byte *dst, int pitch, int width, int height, int x0, int y0, int x1, int y1, byte color);
	// mask231: when true, color 231 is treated as transparent (legacy sprites). For laser beams set false.
	void drawTexturedLine(byte *dst, int pitch, int width, int height, int x0, int y0, int x1, int y1, NutRenderer *nut, int spriteIdx, int v, bool mask231 = true);

	void drawLaserBeam(byte *dst, int pitch, int width, int height, int x0, int y0, int x1, int y1, int progress, int maxProgress, int thickness, int param_9, NutRenderer *nut, int spriteIdx);
	void renderNutSprite(byte *dst, int pitch, int width, int height, int x, int y, NutRenderer *nut, int spriteIdx);

	struct enemy {
		int id;
		Common::Rect rect;
		bool active;
		bool destroyed;           // Set when enemy is shot - prevents re-activation
		int explosionFrame;       // Current explosion animation frame (0-32, -1 = done)
		bool explosionComplete;   // True when explosion animation has finished
		byte *savedBackground;    // Saved background pixels at moment of destruction
		int savedBgWidth;         // Width of saved background
		int savedBgHeight;        // Height of saved background
	};

	void init_enemyStruct(int id, int32 x, int32 y, int32 w, int32 h, bool active, bool destroyed, int32 explosionFrame);
	void enemyUpdate(byte *renderBitmap, Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4);

	Common::List<enemy> _enemies;
	
	// Current handler type for Rebel Assault 2 (determines crosshair sprite)
	// Handler 0: Background only
	// Handler 7: Space flight - uses crosshair sprite 0x2F (47)
	// Handler 8: Ground vehicle - uses crosshair sprite 0x2E (46)  
	// Handler 0x19: Mixed/turret view - uses crosshair sprite 0x2F (47)
	// Handler 0x26: Full turret - crosshair varies by level type
	int _rebelHandler;
	
	// Level type from IACT opcode 6 par3 (corresponds to DAT_004436de)
	// Determines crosshair variant for turret mode:
	// - levelType == 5: Use sprites 0x30+ (48+) for crosshair
	// - levelType != 5: Use sprites 0-3 for crosshair (with animation)
	int _rebelLevelType;

	// Status bar sprite index (5 or 53) triggered by Opcode 6 par4
	// 0 = disabled
	int _rebelStatusBarSprite;

	// Embedded SAN HUD overlays (extracted from IACT chunks)
	// These are decoded frame buffers from embedded ANIM data
	// Slots 1-4 correspond to userId in the IACT wrapper
	struct EmbeddedSanFrame {
		byte *pixels;      // Decoded frame pixels (8-bit indexed)
		int width;         // Frame width
		int height;        // Frame height
		int renderX;       // X position to render (0 = centered based on slot)
		int renderY;       // Y position to render
		bool valid;        // True if this slot has valid data
	};
	
	EmbeddedSanFrame _rebelEmbeddedHud[5];  // Index 0 unused, 1-4 for userId slots
	
	// Check if a partial frame update should be skipped (overlaps with destroyed enemy)
	bool shouldSkipFrameUpdate(int left, int top, int width, int height) override;
	
	// Load and decode an embedded SAN animation from IACT chunk data
	// userId: HUD slot (1-4), animData: raw ANIM data, size: data size, renderBitmap: current frame buffer
	void loadEmbeddedSan(int userId, byte *animData, int32 size, byte *renderBitmap) override;



	int16 _rebelLinks[512][3]; // Dependency links: Slot 0 (Disable on death), Slot 1/2 (Enable on death)
	void clearBit(int n);

	struct Explosion {
		int x, y;
		int width, height;
		int counter;     // Duration counter (starts at 10)
		int scale;       // Determines sprite set (small/med/large)
		bool active;
	};
	
	Explosion _explosions[5];
	void spawnExplosion(int x, int y, int objectHalfWidth);

	int16 _playerDamage;  // 0 to 255 (Accumulated damage)
	int16 _playerLives;
	int32 _playerScore;

	int _viewX;
	int _viewY;


	struct Shot {
		bool active;
		int counter;
		int x, y;       // Target position
	};
	Shot _shots[2];
	void spawnShot(int x, int y);

	/* Difficulty Level (0, 1, 2 = Easy, Med, Hard) */
	int _difficulty;
	void drawCornerBrackets(byte *dst, int pitch, int width, int height, int x, int y, int w, int h, byte color);

};

} // End of namespace Insane

#endif
