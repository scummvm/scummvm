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


} // namespace Parallaction
