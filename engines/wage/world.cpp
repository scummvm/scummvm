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
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * $URL$
 * $Id$
 *
 */

#include "wage/wage.h"
#include "wage/entities.h"
#include "wage/script.h"
#include "wage/world.h"

#include "common/file.h"
#include "common/macresman.h"
#include "common/memstream.h"

namespace Wage {

World::World() {
	_storageScene._name = STORAGESCENE;
	_orderedScenes.push_back(&_storageScene);
	_scenes[STORAGESCENE] = &_storageScene;
}

bool World::loadWorld(Common::MacResManager *resMan) {
	Common::MacResIDArray resArray;
	Common::SeekableReadStream *res;
	Common::MacResIDArray::const_iterator iter;

	if ((resArray = resMan->getResIDArray(MKTAG('G','C','O','D'))).size() == 0)
		return false;

	// Load global script
	res = resMan->getResource(MKTAG('G','C','O','D'), resArray[0]);
	_globalScript = new Script(res, res->size());

	// Load main configuration
	if ((resArray = resMan->getResIDArray(MKTAG('V','E','R','S'))).size() == 0)
		return false;

	if (resArray.size() > 1)
		warning("Too many VERS resources");

	res = resMan->getResource(MKTAG('V','E','R','S'), resArray[0]);

	Common::MemoryReadStream readS(res, res->size());
	readS.skip(10);
	byte b = readS.readByte();
	_weaponMenuDisabled = (b != 0);
	if (b != 0 && b != 1)
		error("Unexpected value for weapons menu");

	readS.skip(3);
	_aboutMessage = readPascalString(readS);

	if (!scumm_stricmp(resMan->getFileName().c_str(), "Scepters"))
		readS.skip(1); // ????

	_soundLibrary1 = readPascalString(readS);
	_soundLibrary2 = readPascalString(readS);

	free(res);

	// Load scenes
	resArray = resMan->getResIDArray(MKTAG('A','S','C','N'));
	for (iter = resArray.begin(); iter != resArray.end(); ++iter) {
		res = resMan->getResource(MKTAG('A','S','C','N'), *iter);
		Scene *scene = new Scene(resMan->getResName(MKTAG('A','S','C','N'), *iter), res, res->size());

		res = resMan->getResource(MKTAG('A','C','O','D'), *iter);
		if (res != NULL)
			scene->_script = new Script(res, res->size());

		res = resMan->getResource(MKTAG('A','T','X','T'), *iter);
		if (res != NULL) {
			Common::MemoryReadStream readT(res, res->size());
			scene->_textBounds = readRect(readT);
			scene->_fontType = readT.readUint16BE();
			scene->_fontSize = readT.readUint16BE();
			
			for (int i = 12; i < res->size(); i++)
				if (res[i] == 0x0d)
					res[i] = '\n';
			String text(&((char*)res)[12], res->size() - 12);
			scene->_text = text;

			delete res;
		}
		addScene(scene);
	}
	
	// Load Objects
	resArray = resMan->getResIDArray(MKTAG('A','O','B','J'));
	for (iter = resArray.begin(); iter != resArray.end(); ++iter) {
		res = resMan->getResource(MKTAG('A','O','B','J'), *iter);
		addObj(new Obj(resMan->getResName(MKTAG('A','O','B','J'), *iter), res, res->size()));
	}

	// Load Characters
	resArray = resMan->getResIDArray(MKTAG('A','C','H','R'));
	for (iter = resArray.begin(); iter != resArray.end(); ++iter) {
		res = resMan->getResource(MKTAG('A','C','H','R'), *iter);
		Chr *chr = new Chr(resMan->getResName(MKTAG('A','C','H','R'), *iter), res, res->size());

		addChr(chr);
		// TODO: What if there's more than one player character?
		if (chr->_playerCharacter)
			_player = chr;
	}

	// Load Sounds
	resArray = resMan->getResIDArray(MKTAG('A','S','N','D'));
	for (iter = resArray.begin(); iter != resArray.end(); ++iter) {
		res = resMan->getResource(MKTAG('A','S','N','D'), *iter);
		addSound(new Sound(resMan->getResName(MKTAG('A','S','N','D')), *iter), res, res->size()));
	}
	
	if (_soundLibrary1.size() > 0) {
		loadExternalSounds(_soundLibrary1);
	}
	if (_soundLibrary2.size() > 0) {
		loadExternalSounds(_soundLibrary2);
	}

	// Load Patterns
	res = resMan->getResource(MKTAG('P','A','T','#'), 900);
	if (res != NULL) {
		Common::MemoryReadStream readP(res, res->size());
		int count = readP.readUint16BE();
		for (int i = 0; i < count; i++) {
			byte *pattern = (byte *)malloc(8);
			for (int j = 0; j < 8; j++) {
				pattern[j] = readP.readByte();
				_patterns.push_back(pattern);
			}
		}
		
		delete res;
	}

	return true;
}

void World::loadExternalSounds(String fname) {
	Common::File in;

	in.open(fname);
	if (!in.isOpen()) {
		warning("Cannot load sound file <%s>", fname.c_str());
		return;
	}
	in.close();

	Common::MacResManager *resMan;
	resMan = new Common::MacResManager(fname);

	Common::MacResIDArray resArray;
	Common::SeekableReadStream *res;
	Common::MacResIDArray::const_iterator iter;

	resArray = resMan->getResIDArray(MKTAG('A','S','N','D'));
	for (iter = resArray.begin(); iter != resArray.end(); ++iter) {
		res = resMan->getResource(MKTAG('A','S','N','D'), *iter);
		addSound(new Sound(resMan->getResName(MKTAG('A','S','N','D'), *iter), res, res->size()));
	}
}

} // End of namespace Wage
