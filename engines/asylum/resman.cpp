/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "asylum/resman.h"

namespace Asylum {

GraphicBundle* ResourceManager::getGraphic(uint8 fileNum, uint32 offset) {
	Bundle *bun = getBundle(fileNum);
	Bundle *ent = bun->getEntry(offset);

	if(!ent->initialized){
		GraphicBundle *gra = new GraphicBundle(fileNum, ent->offset, ent->size);
		bun->setEntry(offset, gra);
	}

	return (GraphicBundle*)bun->getEntry(offset);
}

void ResourceManager::getPalette(uint8 fileNum, uint32 offset, byte *palette) {
	// Sub-optimal, but a bit cleaner (till we get john_doe's code in)
	Common::File palFile;
	char filename[256];
	sprintf(filename, "res.%03d", fileNum);
	palFile.open(filename);

	// Read entries
	/*uint32 entryCount =*/ palFile.readUint32LE();
	palFile.skip(4 * offset);
	uint32 offs = palFile.readUint32LE();
	palFile.seek(offs, SEEK_SET);
	palFile.skip(32);	// TODO: what are these?
	palFile.read(palette, 256 * 3);
	palFile.close();
}

Bundle* ResourceManager::getBundle(uint8 fileNum) {
	// first check if the bundle exists in the cache
	Bundle* bun = NULL;

	for (uint32 i = 0; i < _bundleCache.size(); i++) {
		if (_bundleCache[i].id == fileNum ){
			*bun = _bundleCache[i];
		}
	}

	if(!bun) {
		bun = new Bundle(fileNum);
	}

	return bun;
}

} // end of namespace Asylum
