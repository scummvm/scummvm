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

#if !defined(SCUMM_INSANE_REBEL1_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_INSANE_REBEL1_H

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/events.h"
#include "scumm/insane/insane.h"

namespace Scumm {

class ScummEngine_v7;
class SmushPlayer;
class SmushFont;

// Simple sprite bank for RA1 NUT files (ANIM v1 with odd-alignment padding).
// Separate from NutRenderer to avoid modifying shared NUT parsing code.
struct RA1Sprite {
	int16 xoffs;
	int16 yoffs;
	uint16 width;
	uint16 height;
	byte *data;  // Decoded pixel data (width * height bytes, 0 = transparent)
};

struct RA1SpriteBank {
	int numSprites;
	RA1Sprite *sprites;
	byte *decodedData;  // Single allocation for all decoded pixels
	uint32 decodedSize;

	RA1SpriteBank() : numSprites(0), sprites(nullptr), decodedData(nullptr), decodedSize(0) {}
	~RA1SpriteBank() { delete[] sprites; free(decodedData); }
};

/**
 * Star Wars: Rebel Assault (RA1) game logic.
 * Adapts RA2 Handler 7 (ship flight) physics for RA1's 384x242 resolution.
 */
class InsaneRebel1 : public Insane, public Common::EventObserver {
public:
	InsaneRebel1(ScummEngine_v7 *scumm);
	~InsaneRebel1() override;

	bool notifyEvent(const Common::Event &event) override;

	void procPreRendering(byte *renderBitmap) override;
	void procPostRendering(byte *renderBitmap, int32 codecparam, int32 setupsan12,
		int32 setupsan13, int32 curFrame, int32 maxFrame) override;
	void procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
		int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
		int16 par1, int16 par2, int16 par3, int16 par4) override;
	void procSKIP(int32 subSize, Common::SeekableReadStream &b) override;

	void handleGameChunk(int32 subSize, Common::SeekableReadStream &b);

	// Game flow (matching original at 0x15597)
	void runGame();

private:
	// Intro sequence: O1LOGO → O1OPEN (0x155ef-0x158f8)
	void playIntroSequence();
	void clearVideoBuffer();

	// Main menu loop on O1OPTION.ANM background (0x15968)
	// Returns: 1=Start New Game, 2=Game Options, 3=Enter Passcode, 4=Continue Demo, 5=Exit
	int runMainMenu();

	// Level 1 flow (0x16100): hangar → CU1 → gameplay → CU2 → turret → end
	// Returns true if level completed, false if player quit
	bool runLevel1();

	// Play a passive cinematic (no game callback, skippable)
	void playCinematic(const char *filename);

	// Play interactive gameplay video (with ship physics + HUD)
	void playInteractiveVideo(const char *filename);
	bool loadRA1Nut(const char *filename, RA1SpriteBank &bank);
	void loadLevelSprites(int level);
	void updateShipPhysics();
	void renderShip(byte *dst, int pitch, int width, int height);
	void renderHUD(byte *dst, int pitch, int width, int height);
	void renderMainMenuOverlay(byte *dst, int pitch, int width, int height);
	void renderSprite(byte *dst, int pitch, int width, int height,
					  int x, int y, const RA1Sprite &sprite);

	// Audio
	void initAudio(int sampleRate);
	void terminateAudio();
	void queueAudioData(int trackIdx, uint8 *data, int32 size, int volume, int pan);
public:
	void drawFontBankString(byte *dst, int pitch, int width, int height, int x, int y, const char *text);
	int getFontBankStringWidth(const char *text);
	void processAudioFrame(int16 feedSize);
private:

	ScummEngine_v7 *_vm;

	RA1SpriteBank _shipBank;
	RA1SpriteBank _displayBank;   // SYS/DISPLAY.NUT — bottom status bar
	RA1SpriteBank _hudFontBank;   // RA1 HUD text glyphs (TECHFONT/TALKFONT via RA1 loader)
	SmushFont *_menuFont;         // Use engine text renderer for correct TALKFONT character mapping

	// RA1 screen dimensions (384x242)
	int _screenWidth;
	int _screenHeight;

	// Ship game-coordinate position (adapted from RA2's [20,404]x[20,240])
	// RA1 coordinate space: [18,366]x[18,224], center=(192,121)
	int16 _shipPosX;
	int16 _shipPosY;

	// Direction sprite index (5x7 grid = 35 sprites, vDir*7 + hDir)
	int16 _shipDirIndex;

	// Corridor boundaries (set by GAME opcode 0x07)
	int16 _corridorLeftX;
	int16 _corridorTopY;
	int16 _corridorRightX;
	int16 _corridorBottomY;

	// Physics state (velocity-based movement from RA2 Handler 7)
	int16 _smoothedVelocity;         // Averaged horizontal velocity
	int16 _verticalInput;            // Stored vertical input component
	int16 _velocityHistory[25];      // Horizontal velocity ring buffer

	// Per-frame drift bias from GAME 0x07 field3 (multiplied by tuning "drift" param)
	// Original pipeline: xDelta added to 32-bit accumulator, position = accum >> 8
	// field4 is unused in the original assembly
	int16 _driftParam;
	int32 _driftAccum;           // 32-bit drift accumulator (position = accum >> 8)

	// Perspective view offsets
	int16 _perspectiveX;
	int16 _perspectiveY;
	int16 _viewShift;                // Clamped smoothed velocity for view transform

	// Control mode (from GAME opcode 0x5E)
	int16 _flyControlMode;

	// Damage system (from Ghidra decompilation of FUN_1DEB5)
	int16 _health;               // 0x7560: current health (init=98, negative=dead, max=98)
	int16 _lives;                // 0x7562: remaining extra lives
	int _score;                  // 0x7564: current score
	byte _damageFlags;           // 0x74D4: per-frame collision bitmask (cleared each frame)
	uint16 _gameLatch5D;         // 0x75D2: GAME 0x5D latch (scene/obstacle/event trigger)
	uint16 _gameLatch5F;         // 0x75D4: GAME 0x5F latch (probabilistic hit trigger)
	int16 _damageCooldown;       // 0x74D8: invulnerability timer (10 frames after hit)
	int16 _deathTimer;           // 0x756A: death animation countdown (30 on death)
	int16 _screenFlash;          // 0x7736: screen flash timer on hit
	uint32 _frameCounter;        // 0x7740: global frame counter

	static const int16 kMaxHealth = 98;
	static const int16 kDeathTimerInit = 30;
	static const int16 kDamageCooldownInit = 10;

	// Tuning damage values (TODO: load from data section per difficulty/level)
	static const int16 kLightDamage = 5;   // "wham" — wall/zone collision
	static const int16 kHeavyDamage = 15;  // "shot" — projectile hit

	// Audio state (same structure as RA2)
	static const int kMaxAudioTracks = 4;
	Audio::QueuingAudioStream *_audioStreams[kMaxAudioTracks];
	Audio::SoundHandle _audioHandles[kMaxAudioTracks];
	bool _audioTrackActive[kMaxAudioTracks];
	int _audioSampleRate;

	// True only while an interactive gameplay SMUSH is running.
	bool _interactiveVideoActive;

	// Main menu state (for O1OPTION interactive overlay)
	bool _menuActive;
	bool _menuConfirmed;
	int _menuSelection; // 0..4 maps to return values 1..5
	int _menuFrameCounter;
};

} // End of namespace Scumm

#endif
