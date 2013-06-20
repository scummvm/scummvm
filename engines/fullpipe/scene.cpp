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

#include "fullpipe/objects.h"

namespace Fullpipe {

void FullpipeEngine::accessScene(int sceneId) {
}

bool SceneTagList::load(MfcArchive &file) {
	int numEntries = file.readUint16LE();

	for (int i = 0; i < numEntries; i++) {
		SceneTag *t = new SceneTag();
		t->load(file);
		push_back(*t);
	}

	return true;
}

SceneTag::SceneTag() {
	_field_4 = 0;
	_scene = 0;
}

bool SceneTag::load(MfcArchive &file) {
	_field_4 = 0;
	_scene = 0;

	_sceneId = file.readUint16LE();

	_tag = file.readPascalString();

	debug(6, "sceneId: %d  tag: %s", _sceneId, _tag);

	return true;
}

SceneTag::~SceneTag() {
	free(_tag);
}

} // End of namespace Fullpipe
