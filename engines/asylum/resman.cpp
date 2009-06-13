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

#include "common/stream.h"
#include "sound/audiostream.h"
#include "sound/wave.h"

#include "asylum/resman.h"

namespace Asylum {

ResourceManager::ResourceManager(AsylumEngine *vm): _vm(vm) {
	_video = new Video(_vm->_mixer);
}

ResourceManager::~ResourceManager() {
	delete _video;
}

bool ResourceManager::loadVideo(uint8 fileNum) {
	return _video->playVideo(fileNum);
}

bool ResourceManager::loadCursor(uint8 fileNum, uint32 offet, uint32 index) {
	GraphicResource *cur = getGraphic(fileNum, offet, index);
	_vm->_system->setMouseCursor(cur->data, cur->width, cur->height, 1, 1, 0);
	_vm->_system->showMouse(true);

	// TODO proper error check
	return true;
}

GraphicResource* ResourceManager::getGraphic(uint8 fileNum, uint32 offset, uint32 index) {
	// FIXME: This doesn't make sense semantically. On one hand, we got a bundle, a file
	// containing multiple files. Then, on the other hand, we got a file inside that bundle,
	// which is a bundle again! The file in the bundle should be a resource, not another bundle, 
	// i.e. getEntry() is wrong here
	Bundle *bun = getBundle(fileNum);
	Bundle *ent = bun->getEntry(offset);
	GraphicBundle *gra;
	GraphicResource *res;

	if(!ent->initialized){
		gra = new GraphicBundle(fileNum, ent->offset, ent->size);
		bun->setEntry(offset, gra);
	}else{
		gra = (GraphicBundle*)bun->getEntry(offset);
	}

	res = gra->getEntry(index);

	// Load the graphic data if it's not already loaded
	/*
	if (!res->data) {
		// FIXME: this is currently leaking, and it's wrong, as we can't find individual frames in graphic resources
		res->data = (byte *)malloc(res->size - 16);
		memcpy(res->data, gra->getData() + gra->getContentOffset() + 16, res->size - 16);
	}
	*/

	return res;
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
