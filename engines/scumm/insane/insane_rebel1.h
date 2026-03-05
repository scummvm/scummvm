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
#include "scumm/insane/insane.h"

namespace Scumm {

class ScummEngine_v7;
class SmushPlayer;

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

	RA1SpriteBank() : numSprites(0), sprites(nullptr), decodedData(nullptr) {}
	~RA1SpriteBank() { delete[] sprites; free(decodedData); }
};

/**
 * Star Wars: Rebel Assault (RA1) game logic.
 * Adapts RA2 Handler 7 (ship flight) physics for RA1's 384x242 resolution.
 */
class InsaneRebel1 : public Insane {
public:
	InsaneRebel1(ScummEngine_v7 *scumm);
	~InsaneRebel1() override;

	void procPreRendering(byte *renderBitmap) override;
	void procPostRendering(byte *renderBitmap, int32 codecparam, int32 setupsan12,
		int32 setupsan13, int32 curFrame, int32 maxFrame) override;
	void procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
		int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
		int16 par1, int16 par2, int16 par3, int16 par4) override;
	void procSKIP(int32 subSize, Common::SeekableReadStream &b) override;

	void handleGameChunk(int32 subSize, Common::SeekableReadStream &b);

	void playLevel(int level);

private:
	bool loadRA1Nut(const char *filename, RA1SpriteBank &bank);
	void loadLevelSprites(int level);
	void updateShipPhysics();
	void renderShip(byte *dst, int pitch, int width, int height);
	void renderHUD(byte *dst, int pitch, int width, int height);
	void renderSprite(byte *dst, int pitch, int width, int height,
					  int x, int y, const RA1Sprite &sprite);

	// Audio
	void initAudio(int sampleRate);
	void terminateAudio();
	void queueAudioData(int trackIdx, uint8 *data, int32 size, int volume, int pan);
public:
	void processAudioFrame(int16 feedSize);
private:

	ScummEngine_v7 *_vm;

	RA1SpriteBank _shipBank;
	RA1SpriteBank _displayBank;   // SYS/DISPLAY.NUT — bottom status bar

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
	int16 _windHistoryX[15];         // Wind X history buffer
	int16 _windHistoryY[15];         // Wind Y history buffer
	int16 _windParamX;               // Wind X (from GAME opcode 0x07 sub-opcode 0)
	int16 _windParamY;               // Wind Y (from GAME opcode 0x07 sub-opcode 0)

	// Perspective view offsets
	int16 _perspectiveX;
	int16 _perspectiveY;
	int16 _viewShift;                // Clamped smoothed velocity for view transform

	// Control mode (from GAME opcode 0x5E)
	int16 _flyControlMode;

	// Hit cooldown timer
	int16 _hitCooldown;

	// HUD state
	int16 _playerDamage;    // 0-255, higher = more damage
	int _score;
	int _pilots;            // Lives remaining

	// Audio state (same structure as RA2)
	static const int kMaxAudioTracks = 4;
	Audio::QueuingAudioStream *_audioStreams[kMaxAudioTracks];
	Audio::SoundHandle _audioHandles[kMaxAudioTracks];
	bool _audioTrackActive[kMaxAudioTracks];
	int _audioSampleRate;
};

} // End of namespace Scumm

#endif
