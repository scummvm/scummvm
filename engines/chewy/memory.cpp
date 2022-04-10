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

#include "common/memstream.h"
#include "chewy/memory.h"
#include "chewy/globals.h"
#include "chewy/types.h"
#include "chewy/resource.h"

namespace Chewy {

TafInfo *Memory::taf_adr(const char *filename) {
	SpriteResource *res = new SpriteResource(filename);
	int32 imageCount = res->getChunkCount();
	uint32 size = res->getAllSize() + imageCount * 8 + sizeof(TafInfo);
	uint32 kgroesse = imageCount * sizeof(byte *);

	byte *tmp1 = (byte *)MALLOC(size + PALETTE_SIZE + kgroesse);
	TafInfo *tinfo = (TafInfo *)tmp1;
	tinfo->_image = (byte **)(tmp1 + sizeof(TafInfo));
	tinfo->_palette = tmp1 + size;
	tinfo->_count = imageCount;
	memcpy(tinfo->_palette, res->getSpritePalette(), PALETTE_SIZE);
	byte *imgPtr = tmp1 + sizeof(TafInfo) + kgroesse;

	for (int i = 0; i < imageCount; i++) {
		tinfo->_image[i] = imgPtr;
		imgPtr += res->getSpriteData(i, &tinfo->_image[i], false);
	}

	tinfo->_correction = (int16 *)(tmp1 + (size + 768l));
	memcpy(tinfo->_correction, res->getSpriteCorrectionsTable(), imageCount * 2 * sizeof(int16));

	delete res;

	return tinfo;
}

TafSeqInfo *Memory::taf_seq_adr(int16 image_start, int16 image_anz) {
	TafSeqInfo *ts_info = nullptr;
	SpriteResource *res = new SpriteResource(CH_SPZ_FILE);
	uint32 size = 0;

	for (int16 i = 0; i < image_anz; i++) {
		TAFChunk *sprite = res->getSprite(i + image_start);
		size += sprite->width * sprite->height;
		delete sprite;
	}

	size += image_anz * sizeof(byte *);
	size += image_anz * sizeof(char *);
	size += ((uint32)sizeof(TafSeqInfo));

	byte *tmp1 = (byte *)MALLOC(size + image_anz * sizeof(byte *));
	ts_info = (TafSeqInfo *)tmp1;
	ts_info->_count = image_anz;
	ts_info->_image = (byte **)(tmp1 + sizeof(TafSeqInfo));
	ts_info->_correction = (int16 *)(tmp1 + size);
	byte *sp_ptr = tmp1 + (((uint32)sizeof(TafSeqInfo)) + (image_anz * sizeof(char *)));

	for (int16 i = 0; i < image_anz; i++) {
		ts_info->_image[i] = sp_ptr;
		sp_ptr += res->getSpriteData(i + image_start, &ts_info->_image[i], false);
	}

	uint16 *correctionsTable = res->getSpriteCorrectionsTable() + image_start * 2;
	memcpy(ts_info->_correction, correctionsTable, image_anz * 2 * sizeof(int16));

	delete res;

	return ts_info;
}

} // namespace Chewy
