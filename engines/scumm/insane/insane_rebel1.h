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
 * Proof-of-concept for Star Wars: Rebel Assault (RA1).
 * Loads level 1 and renders the player ship sprite.
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
	void renderShip(byte *dst, int pitch, int width, int height);
	void renderSprite(byte *dst, int pitch, int width, int height,
					  int x, int y, const RA1Sprite &sprite);

	ScummEngine_v7 *_vm;

	RA1SpriteBank _shipBank;

	int _shipPosX;
	int _shipPosY;
	int _shipDirIndex;

	int _screenWidth;
	int _screenHeight;
};

} // End of namespace Scumm

#endif
