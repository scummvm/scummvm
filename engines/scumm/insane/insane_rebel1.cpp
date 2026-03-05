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
#include "common/events.h"
#include "common/endian.h"
#include "scumm/scumm_v7.h"
#include "scumm/scumm.h"
#include "scumm/smush/smush_player.h"
#include "scumm/insane/insane_rebel1.h"

namespace Scumm {

// Decode BOMP RLE (codec 21) sprite data into a flat pixel buffer.
// Same algorithm as NutRenderer::codec21 but without palette tracking.
static void decodeBomp(byte *dst, const byte *src, int width, int height, int pitch) {
	while (height--) {
		byte *dstNext = dst + pitch;
		const byte *srcNext = src + 2 + READ_LE_UINT16(src);
		src += 2;
		int len = width;
		byte *d = dst;
		do {
			int offs = READ_LE_UINT16(src); src += 2;
			d += offs;
			len -= offs;
			if (len <= 0)
				break;
			int w = READ_LE_UINT16(src) + 1; src += 2;
			len -= w;
			if (len < 0)
				w += len;
			memcpy(d, src, w);
			src += w;
			d += w;
		} while (len > 0);
		dst = dstNext;
		src = srcNext;
	}
}

InsaneRebel1::InsaneRebel1(ScummEngine_v7 *scumm) : Insane(), _vm(scumm) {
	_shipPosX = 192;
	_shipPosY = 160;
	_shipDirIndex = 0;
	_screenWidth = 384;
	_screenHeight = 242;

	// Null out Insane base class pointers that the default constructor doesn't initialize
	_smush_roadrashRip = nullptr;
	_smush_roadrsh2Rip = nullptr;
	_smush_roadrsh3Rip = nullptr;
	_smush_goglpaltRip = nullptr;
	_smush_tovista1Flu = nullptr;
	_smush_tovista2Flu = nullptr;
	_smush_toranchFlu = nullptr;
	_smush_minedrivFlu = nullptr;
	_smush_minefiteFlu = nullptr;
	_smush_bensgoggNut = nullptr;
	_smush_bencutNut = nullptr;
	_smush_iconsNut = nullptr;
	_smush_icons2Nut = nullptr;
}

InsaneRebel1::~InsaneRebel1() {
}

// Load an RA1 NUT sprite file (ANIM v1).
// RA1 NUTs can have odd-size FOBJ chunks padded to 2-byte alignment within
// FRME containers. This loader handles that padding properly, unlike the
// shared NutRenderer::loadFont which assumes even-size chunks.
bool InsaneRebel1::loadRA1Nut(const char *filename, RA1SpriteBank &bank) {
	ScummFile *file = _vm->instantiateScummFile();
	_vm->openFile(*file, filename);
	if (!file->isOpen()) {
		warning("InsaneRebel1::loadRA1Nut: can't open %s", filename);
		delete file;
		return false;
	}

	uint32 tag = file->readUint32BE();
	if (tag != MKTAG('A','N','I','M')) {
		warning("InsaneRebel1::loadRA1Nut: no ANIM tag in %s", filename);
		delete file;
		return false;
	}
	uint32 animSize = file->readUint32BE();
	byte *data = (byte *)malloc(animSize);
	file->read(data, animSize);
	file->close();
	delete file;

	// data[0..3] = AHDR tag, data[4..7] = AHDR size
	if (READ_BE_UINT32(data) != MKTAG('A','H','D','R')) {
		warning("InsaneRebel1::loadRA1Nut: no AHDR in %s", filename);
		free(data);
		return false;
	}

	bank.numSprites = READ_LE_UINT16(data + 10);
	bank.sprites = new RA1Sprite[bank.numSprites];

	// Pass 1: Walk chunks with alignment to compute total decoded size.
	uint32 offset = 0;
	uint32 decodedSize = 0;
	for (int i = 0; i < bank.numSprites; i++) {
		// Skip current chunk (AHDR or previous FRME)
		uint32 chunkSize = READ_BE_UINT32(data + offset + 4);
		offset += chunkSize + 8;
		if (chunkSize & 1) offset++;  // Word-align

		// Now at FRME; skip its header to reach FOBJ
		offset += 8;
		if (offset + 22 > animSize) break;

		uint16 w = READ_LE_UINT16(data + offset + 14);
		uint16 h = READ_LE_UINT16(data + offset + 16);
		decodedSize += w * h;
	}

	bank.decodedData = (byte *)calloc(decodedSize, 1);
	byte *decPtr = bank.decodedData;

	// Pass 2: Decode sprites.
	offset = 0;
	for (int i = 0; i < bank.numSprites; i++) {
		uint32 chunkSize = READ_BE_UINT32(data + offset + 4);
		offset += chunkSize + 8;
		if (chunkSize & 1) offset++;

		offset += 8;  // Skip FRME header → now at FOBJ
		if (offset + 22 > animSize) break;

		int codec = READ_LE_UINT16(data + offset + 8);
		bank.sprites[i].xoffs = READ_LE_INT16(data + offset + 10);
		bank.sprites[i].yoffs = READ_LE_INT16(data + offset + 12);
		bank.sprites[i].width = READ_LE_UINT16(data + offset + 14);
		bank.sprites[i].height = READ_LE_UINT16(data + offset + 16);
		bank.sprites[i].data = decPtr;

		int pixelCount = bank.sprites[i].width * bank.sprites[i].height;
		const byte *fobjData = data + offset + 22;

		if (codec == 21) {
			decodeBomp(decPtr, fobjData, bank.sprites[i].width,
					   bank.sprites[i].height, bank.sprites[i].width);
		} else {
			warning("InsaneRebel1::loadRA1Nut: unsupported codec %d in sprite %d", codec, i);
		}

		decPtr += pixelCount;
	}

	free(data);
	debug(1, "InsaneRebel1::loadRA1Nut('%s'): %d sprites, %d bytes decoded",
		  filename, bank.numSprites, decodedSize);
	return true;
}

void InsaneRebel1::loadLevelSprites(int level) {
	Common::String filename = Common::String::format("LVL%d/L%dBANK1.NUT", level, level);
	loadRA1Nut(filename.c_str(), _shipBank);
}

void InsaneRebel1::procPreRendering(byte *renderBitmap) {
}

void InsaneRebel1::procPostRendering(byte *renderBitmap, int32 codecparam, int32 setupsan12,
	int32 setupsan13, int32 curFrame, int32 maxFrame) {

	if (_shipBank.numSprites == 0 || !renderBitmap)
		return;

	int width = _player->_width;
	int height = _player->_height;
	if (width == 0) width = _screenWidth;
	if (height == 0) height = _screenHeight;
	int pitch = width;

	// Read mouse position and map to normalized range (-1.0 to 1.0)
	Common::Point mousePos = g_system->getEventManager()->getMousePos();
	float normX = CLIP((float)(mousePos.x - width / 2) / (float)(width / 2), -1.0f, 1.0f);
	float normY = CLIP((float)(mousePos.y - height / 2) / (float)(height / 2), -1.0f, 1.0f);

	// Smooth ship position toward mouse (max 8 pixels/frame horizontal, 6 vertical)
	int targetX = width / 2 + (int)(normX * width / 4);
	int targetY = height / 2 + (int)(normY * height / 4);
	_shipPosX += CLIP(targetX - _shipPosX, -8, 8);
	_shipPosY += CLIP(targetY - _shipPosY, -6, 6);

	// Map mouse to direction index (5 horizontal x 7 vertical = 35 core sprites).
	int hZone = CLIP((int)((normX + 1.0f) * 2.5f), 0, 4);
	int vZone = CLIP((int)((normY + 1.0f) * 3.5f), 0, 6);
	_shipDirIndex = CLIP(hZone * 7 + vZone, 0, _shipBank.numSprites - 1);

	renderShip(renderBitmap, pitch, width, height);
}

void InsaneRebel1::renderShip(byte *dst, int pitch, int width, int height) {
	if (_shipDirIndex < 0 || _shipDirIndex >= _shipBank.numSprites)
		return;

	const RA1Sprite &spr = _shipBank.sprites[_shipDirIndex];
	int drawX = _shipPosX - spr.width / 2;
	int drawY = _shipPosY - spr.height / 2;
	renderSprite(dst, pitch, width, height, drawX, drawY, spr);
}

void InsaneRebel1::renderSprite(byte *dst, int pitch, int width, int height,
								int x, int y, const RA1Sprite &spr) {
	if (!spr.data || spr.width <= 0 || spr.height <= 0)
		return;

	int drawX = x, drawY = y, drawW = spr.width, drawH = spr.height;
	int srcOffsetX = 0, srcOffsetY = 0;

	if (drawX < 0) { srcOffsetX = -drawX; drawW += drawX; drawX = 0; }
	if (drawY < 0) { srcOffsetY = -drawY; drawH += drawY; drawY = 0; }
	if (drawX + drawW > width) drawW = width - drawX;
	if (drawY + drawH > height) drawH = height - drawY;
	if (drawW <= 0 || drawH <= 0)
		return;

	for (int iy = 0; iy < drawH; iy++) {
		const byte *s = spr.data + (srcOffsetY + iy) * spr.width + srcOffsetX;
		byte *d = dst + (drawY + iy) * pitch + drawX;
		for (int ix = 0; ix < drawW; ix++) {
			byte px = s[ix];
			if (px != 0)
				d[ix] = px;
		}
	}
}

void InsaneRebel1::procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
	int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
	int16 par1, int16 par2, int16 par3, int16 par4) {
}

void InsaneRebel1::procSKIP(int32 subSize, Common::SeekableReadStream &b) {
}

void InsaneRebel1::handleGameChunk(int32 subSize, Common::SeekableReadStream &b) {
	if (subSize < 8)
		return;

	uint32 opcode = b.readUint32BE();
	uint32 param1 = b.readUint32BE();

	switch (opcode) {
	case 0x5E:
		debug(5, "InsaneRebel1: GAME 0x5E (mode) param=%d", param1);
		break;
	case 0x5D:
		debug(5, "InsaneRebel1: GAME 0x5D (link) param=%d", param1);
		break;
	case 0x5F:
		debug(5, "InsaneRebel1: GAME 0x5F (event) param=%d", param1);
		break;
	case 0x07: case 0x08: case 0x09: case 0x0A: case 0x0B:
	case 0x19: case 0x1A:
	case 0x0D: case 0x0E:
		if (subSize >= 20) {
			b.readUint32BE(); b.readUint32BE(); b.readUint32BE();
		}
		break;
	default:
		debug(7, "InsaneRebel1: GAME unknown 0x%02x size=%d", opcode, subSize);
		break;
	}
}

void InsaneRebel1::playLevel(int level) {
	loadLevelSprites(level);

	Common::String filename = Common::String::format("LVL%d/L%dPLAY1L.ANM", level, level);
	debug(1, "InsaneRebel1::playLevel(%d): playing %s", level, filename.c_str());

	SmushPlayer *splayer = _vm->_splayer;
	_player = splayer;
	splayer->play(filename.c_str(), 12);
}

} // End of namespace Scumm
