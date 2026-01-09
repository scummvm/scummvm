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

 	// Helper for debugging IACT boxes
	void drawRect(byte *dst, int pitch, int x, int y, int w, int h, byte color, int bufWidth, int bufHeight) {
		Common::Rect r(x, y, x + w, y + h);
		Common::Rect screen(0, 0, bufWidth, bufHeight);
		Common::Rect clipped = r;
		clipped.clip(screen);

		if (!clipped.isValidRect()) return;

		// Top
		if (r.top >= 0 && r.top < bufHeight) {
			int startX = MAX((int)r.left, 0);
			int endX = MIN((int)r.right, bufWidth);
			for (int k = startX; k < endX; k++) dst[r.top * pitch + k] = color;
		}
		// Bottom
		if (r.bottom > 0 && r.bottom <= bufHeight) {
			int startX = MAX((int)r.left, 0);
			int endX = MIN((int)r.right, bufWidth);
			for (int k = startX; k < endX; k++) dst[(r.bottom - 1) * pitch + k] = color;
		}
		// Left
		if (r.left >= 0 && r.left < bufWidth) {
			int startY = MAX((int)r.top, 0);
			int endY = MIN((int)r.bottom, bufHeight);
			for (int k = startY; k < endY; k++) dst[k * pitch + r.left] = color;
		}
		// Right
		if (r.right > 0 && r.right <= bufWidth) {
			int startY = MAX((int)r.top, 0);
			int endY = MIN((int)r.bottom, bufHeight);
			for (int k = startY; k < endY; k++) dst[k * pitch + (r.right - 1)] = color;
		}
	}

public:
	InsaneRebel2(ScummEngine_v7 *scumm);
	~InsaneRebel2();

	NutRenderer *_smush_cockpitNut;
	NutRenderer *_smush_dispfontNut;  // DAT_00482200 - DISPFONT.NUT for status bar (difficulty, shields, lives, score)
	
	// Rebel Assault 2: Dynamically loaded HUD overlays (from CHK scripts)
	// These correspond to the original game's global variables
	NutRenderer *_rebelHudPrimary;     // DAT_00482240 - Primary HUD overlay
	NutRenderer *_rebelHudSecondary;   // DAT_00482238 - Secondary HUD graphics
	NutRenderer *_rebelHudCockpit;     // DAT_00482268 - Ship cockpit frame overlay
	NutRenderer *_rebelHudExplosion;   // DAT_00482250 - Explosion overlay sprites
	NutRenderer *_rebelHudDamage;      // DAT_00482248 - Damage indicator sprites
	NutRenderer *_rebelHudEffects;     // DAT_00482258 - Additional effects
	NutRenderer *_rebelHudHiRes;       // DAT_00482260 - High-res HUD alternative

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

	struct RebelEnemy {
		int id;
		Common::Rect rect;
		bool active;
		bool destroyed;           // Set when enemy is shot - prevents re-activation
		int explosionFrame;       // Current explosion animation frame (0-32, -1 = done)
		bool explosionComplete;   // True when explosion animation has finished
		byte *savedBackground;    // Saved background pixels at moment of destruction
		int savedBgWidth;         // Width of saved background
		int savedBgHeight;        // Height of saved background
		
		RebelEnemy() : id(0), active(false), destroyed(false), explosionFrame(-1),
		               explosionComplete(false), savedBackground(nullptr), 
		               savedBgWidth(0), savedBgHeight(0) {}
		~RebelEnemy() { free(savedBackground); }
	};

	Common::List<RebelEnemy> _rebelEnemies;
	
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
		
		EmbeddedSanFrame() : pixels(nullptr), width(0), height(0), 
		                     renderX(0), renderY(0), valid(false) {}
		~EmbeddedSanFrame() { free(pixels); }
		void clear() { free(pixels); pixels = nullptr; width = height = 0; valid = false; }
	};
	
	EmbeddedSanFrame _rebelEmbeddedHud[5];  // Index 0 unused, 1-4 for userId slots
	
	// Check if a partial frame update should be skipped (overlaps with destroyed enemy)
	bool shouldSkipFrameUpdate(int left, int top, int width, int height) override;
	
	// Load and decode an embedded SAN animation from IACT chunk data
	// userId: HUD slot (1-4), animData: raw ANIM data, size: data size, renderBitmap: current frame buffer
	void loadEmbeddedSan(int userId, byte *animData, int32 size, byte *renderBitmap) override;

	int16 _rebelLinks[512][3]; // Dependency links: Slot 0 (Disable on death), Slot 1/2 (Enable on death)
	void clearBit(int n);

};

} // End of namespace Insane

#endif
