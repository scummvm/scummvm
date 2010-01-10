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

#include "wage/wage.h"
#include "wage/macresman.h"
#include "wage/entities.h"
#include "wage/script.h"
#include "wage/world.h"

#include "common/stream.h"

namespace Wage {

World::World() {
	_storageScene._name = STORAGESCENE;
	_orderedScenes.push_back(&_storageScene);
	_scenes[STORAGESCENE] = &_storageScene;
}

bool World::loadWorld(MacResManager *resMan) {
	int resSize;
	MacResIDArray resArray;
	byte *res;
	MacResIDArray::const_iterator iter;

	if ((resArray = resMan->getResIDArray("GCOD")).size() == 0)
		return false;

	// Load global script
	res = resMan->getResource("GCOD", resArray[0], &resSize);
	_globalScript = new Script(res);

	// Load main configuration
	if ((resArray = resMan->getResIDArray("VERS")).size() == 0)
		return false;

	if (resArray.size() > 1)
		warning("Too many VERS resources");

	res = resMan->getResource("VERS", resArray[0], &resSize);

	Common::MemoryReadStream readS(res, resSize);
	readS.skip(10);
	byte b = readS.readByte();
	_weaponMenuDisabled = (b != 0);
	if (b != 0 && b != 1)
		error("Unexpected value for weapons menu");

	readS.skip(3);
	_aboutMessage = readPascalString(readS);

	if (!stricmp(resMan->getFileName().c_str(), "Scepters"))
		readS.skip(1); // ????

	_soundLibrary1 = readPascalString(readS);
	_soundLibrary2 = readPascalString(readS);

	free(res);

	// Load scenes
	resArray = resMan->getResIDArray("ASCN");
	for (iter = resArray.begin(); iter != resArray.end(); ++iter) {
		res = resMan->getResource("ASCN", *iter, &resSize);
		Scene *scene = new Scene(resMan->getResName("ASCN", *iter), res);

		res = resMan->getResource("ACOD", *iter, &resSize);
		if (res != NULL)
			scene->_script = new Script(res);

		res = resMan->getResource("ATXT", *iter, &resSize);
		if (res != NULL) {
			Common::MemoryReadStream readT(res, resSize);
			scene->_textBounds = readRect(readT);
			scene->_fontType = readT.readUint16LE();
			scene->_fontSize = readT.readUint16LE();
			
			for (int i = 12; i < resSize; i++)
				if (res[i] == 0x0d)
					res[i] = '\n';
			String text(&((char*)res)[12], resSize - 12);
			scene->_text = text;

			free(res);
		}
		addScene(scene);
	}
	
	// Load Objects
	resArray = resMan->getResIDArray("AOBJ");
	for (iter = resArray.begin(); iter != resArray.end(); ++iter) {
		res = resMan->getResource("AOBJ", *iter, &resSize);
		addObj(new Obj(resMan->getResName("AOBJ", *iter), res));
	}

	// Load Characters
	resArray = resMan->getResIDArray("ACHR");
	for (iter = resArray.begin(); iter != resArray.end(); ++iter) {
		res = resMan->getResource("ACHR", *iter, &resSize);
		Chr *chr = new Chr(resMan->getResName("ACHR", *iter), res);

		addChr(chr);
		// TODO: What if there's more than one player character?
		if (chr->_playerCharacter)
			_player = chr;
	}

	// Load Sounds
	resArray = resMan->getResIDArray("ASND");
	for (iter = resArray.begin(); iter != resArray.end(); ++iter) {
		res = resMan->getResource("ASND", *iter, &resSize);
		Sound *sound = new Sound(res);
		sound->_name = resMan->getResName("ASND", *iter);
		addSound(sound);
	}
	
	if (_soundLibrary1.size() > 0) {
		loadExternalSounds(_soundLibrary1);
	}
	if (_soundLibrary2.size() > 0) {
		loadExternalSounds(_soundLibrary2);
	}

	// Load Patterns
	res = resMan->getResource("PAT#", 900, &resSize);
	if (res != NULL) {
		Common::MemoryReadStream readP(res, resSize);
		int count = readP.readUint16LE();
		for (int i = 0; i < count; i++) {
			byte *pattern = (byte *)malloc(8);
			for (int j = 0; j < 8; j++) {
				pattern[j] = readP.readByte();
				_patterns.push_back(pattern);
			}
		}
		
		free(res);
	}

	return true;
}

void World::loadExternalSounds(String fname) {
}

} // End of namespace Wage
