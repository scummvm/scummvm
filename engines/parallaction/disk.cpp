/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#include "parallaction/defs.h"
#include "parallaction/graphics.h"
#include "parallaction/parallaction.h"
#include "parallaction/disk.h"

namespace Parallaction {

void openTalk(const char *name, Cnv *cnv) {

	char* ext = strstr(name, ".talk");
	if (ext != NULL) {
		// npc talk
		_vm->_graphics->loadCnv(name, cnv);

	} else {
		// character talk

		char v20[PATH_LEN];
		char *v24 = const_cast<char*>(name);
		if (!scumm_strnicmp(v24, "mini", 4)) {
			v24+=4;
		}

		if (_engineFlags & kEngineMiniDonna) {
			sprintf(v20, "%stta", v24);
		} else {
			sprintf(v20, "%stal", v24);
		}

		_vm->_graphics->loadExternalCnv(v20, cnv);

	}

}

void openLocation(const char *name, char* script) {

	char archivefile[PATH_LEN];

	if (_vm->_characterName[0] == 'm') {
		sprintf(archivefile, "%s%s", _vm->_characterName+4, _vm->_languageDir);
	} else {
		if (_vm->_characterName[0] == 'D') strcpy(archivefile, _vm->_languageDir);
		else {
			sprintf(archivefile, "%s%s", _vm->_characterName, _vm->_languageDir);
		}
	}
	strcat(archivefile, name);
	strcat(archivefile, ".loc");

	_vm->_archive.close();

	_vm->_languageDir[2] = '\0';
	_vm->_archive.open(_vm->_languageDir);
	_vm->_languageDir[2] = '/';

	if (!_vm->_archive.openArchivedFile(archivefile)) {
		sprintf(archivefile, "%s%s.loc", _vm->_languageDir, name);
		if (!_vm->_archive.openArchivedFile(archivefile))
			error("can't find location file '%s'", name);
	}

	uint32 count = _vm->_archive.size();
	_vm->_archive.read(script, count);
	_vm->_archive.closeArchivedFile();
	_vm->_archive.close();

}

void loadHead(const char* name, StaticCnv* cnv) {

	char path[PATH_LEN];

	if (!scumm_strnicmp(name, "mini", 4)) {
		name += 4;
	}

	snprintf(path, 8, "%shead", name);
	path[8] = '\0';

	_vm->_graphics->loadExternalStaticCnv(path, cnv);

}


void loadPointer(StaticCnv* cnv) {
	_vm->_graphics->loadExternalStaticCnv("pointer", cnv);
}

void loadFont(const char* name, Cnv* cnv) {
	char path[PATH_LEN];

	sprintf(path, "%scnv", name);
	_vm->_graphics->loadExternalCnv(path, cnv);
}

} // namespace Parallaction
