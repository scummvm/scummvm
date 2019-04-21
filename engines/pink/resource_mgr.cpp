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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/substream.h"

#include "pink/cel_decoder.h"
#include "pink/file.h"
#include "pink/pink.h"
#include "pink/resource_mgr.h"
#include "pink/sound.h"
#include "pink/objects/pages/game_page.h"

namespace Pink {

ResourceMgr::ResourceMgr()
		: _game(nullptr), _resDescTable(nullptr), _resCount(0) {}

ResourceMgr::~ResourceMgr() {
	clear();
}

void ResourceMgr::init(PinkEngine *game, Page *page) {
	OrbFile *orb = game->getOrb();
	_game = game;

	ObjectDescription *objDesc = orb->getObjDesc(page->getName().c_str());
	_resCount = objDesc->resourcesCount;
	orb->loadObject(page, objDesc);
	_resDescTable = orb->createResDescTable(objDesc);

	debugC(kPinkDebugLoadingResources, "%d Resource descriptions are loaded", _resCount);
}

void ResourceMgr::clear() {
	delete[] _resDescTable;
	_resDescTable = nullptr;
}

static int resDescComp(const void *a, const void *b) {
	return scumm_stricmp((const char *)a, (const char *)b);
}

Common::SafeSeekableSubReadStream *ResourceMgr::getResourceStream(const Common::String &name) {
	Common::SeekableReadStream *stream;

	ResourceDescription *desc = (ResourceDescription *)bsearch(name.c_str(), _resDescTable, _resCount, sizeof(ResourceDescription), resDescComp);

	if (desc->inBro)
		stream = _game->getBro();
	else
		stream = _game->getOrb();

	stream->seek(desc->offset);

	debugC(kPinkDebugLoadingResources, "Got stream of %s resource", name.c_str());
	return new Common::SafeSeekableSubReadStream(stream, desc->offset,
												 desc->offset + desc->size);
}

} // End of namespace Pink
