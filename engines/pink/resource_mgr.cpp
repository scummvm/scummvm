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
	_resDescTable = orb->getResDescTable(objDesc);
}

void ResourceMgr::clear() {
	delete[] _resDescTable;
	_resDescTable = nullptr;
}

CelDecoder *ResourceMgr::loadCEL(Common::String &name) {
	CelDecoder *decoder = new CelDecoder();
	decoder->loadStream(getResourceStream(name));
	return decoder;
}

Sound *ResourceMgr::loadSound(Common::String &name) {
	return new Sound(_game->_mixer, getResourceStream(name));
}

Common::String ResourceMgr::loadText(Common::String &name) {
	Common::SeekableReadStream *stream = getResourceStream(name);
	char *txt = new char[stream->size()];
	Common::String str(txt, stream->size());
	delete txt;
	return str;
}

static int resDescComp(const void *a, const void *b) {
	return scumm_stricmp((char *) a, (char *) b);
}

PinkEngine *ResourceMgr::getGame() const {
	return _game;
}

Common::SafeSeekableSubReadStream *ResourceMgr::getResourceStream(Common::String &name) {
	Common::SeekableReadStream *stream;

	ResourceDescription *desc = (ResourceDescription*) bsearch(name.c_str(), _resDescTable, _resCount, sizeof(ResourceDescription), resDescComp);

	if (desc->inBro)
		stream = _game->getBro();
	else
		stream = _game->getOrb();

	stream->seek(desc->offset);

	return new Common::SafeSeekableSubReadStream(stream, desc->offset,
												 desc->offset + desc->size);
}

} // End of namespace Pink
