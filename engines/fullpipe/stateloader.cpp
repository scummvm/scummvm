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
 */

#include "fullpipe/fullpipe.h"

#include "common/file.h"
#include "common/list.h"

#include "fullpipe/utils.h"
#include "fullpipe/objects.h"

namespace Fullpipe {

bool FullpipeEngine::loadGam(const char *fname) {
	g_gameLoader = new CGameLoader();

	if (g_gameLoader->loadFile(fname)) {
		// TODO
	} else
		return false;

	return true;
}

bool CGameLoader::loadFile(const char *fname) {
	CFile file;

	if (!file.open(fname))
		return false;

	_gameName = file.readPascalString();

	_gameProject = new GameProject(file);

	return true;
}

CGameLoader::~CGameLoader() {
	free(_gameName);
}

GameProject::GameProject(CFile &file) {
	_field_4 = 0;
	_headerFilename = 0;
	_field_10 = 12;

	// FIXME
	int _gameProjectVersion = file.readUint32LE();
	int _gameProjectValue = file.readUint16LE();
	int _scrollSpeed = file.readUint32LE();

	_headerFilename = file.readPascalString();

	_sceneTagList = new SceneTagList(file);

	debug(0, "_gameProjectVersion = %d", _gameProjectVersion);
	debug(0, "_gameProjectValue = %d", _gameProjectValue);
	debug(0, "_scrollSpeed = %d", _scrollSpeed);
	debug(0, "_headerFilename = %s", _headerFilename);

	if (_gameProjectVersion >= 3)
		_field_4 = file.readUint32LE();

	if (_gameProjectVersion >= 5) {
		file.readUint32LE();
		file.readUint32LE();
	}
}

GameProject::~GameProject() {
	free(_headerFilename);
}

SceneTagList::SceneTagList(CFile &file) {
	int numEntries = file.readUint16LE();

	debug(0, "numEntries: %d", numEntries);

	for (int i = 0; i < numEntries; i++) {
		SceneTag *t = new SceneTag(file);
		list.push_back(*t);
	}
}

SceneTag::SceneTag(CFile &file) {
	_field_4 = 0;
	_scene = 0;

	_sceneId = file.readUint16LE();

	_tag = file.readPascalString();

	debug(0, "sceneId: %d  tag: %s", _sceneId, _tag);
}

SceneTag::~SceneTag() {
	free(_tag);
}

} // End of namespace Fullpipe
