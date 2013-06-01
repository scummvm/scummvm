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
	Common::File file;

	if (!file.open(fname))
		return false;

	char *tmp;
	int len = file.readByte();
	tmp = (char *)calloc(len + 1, 1);
	file.read(tmp, len);

	_gameName = tmp;

	_gameProject = new GameProject(file);

	return true;
}

CGameLoader::~CGameLoader() {
	free(_gameName);
}

GameProject::GameProject(Common::File &file) {
	_field_4 = 0;
	_headerFilename = 0;
	_field_10 = 12;

	// FIXME
	int _gameProjectVersion = file.readUint32LE();
	int _gameProjectValue = file.readUint16LE();
	int _scrollSpeed = file.readUint32LE();

	char *tmp;
	int len = file.readByte();
	tmp = (char *)calloc(len + 1, 1);
	file.read(tmp, len);
	
	_headerFilename = tmp;

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

SceneTagList::SceneTagList(Common::File &file) {
}

} // End of namespace Fullpipe
