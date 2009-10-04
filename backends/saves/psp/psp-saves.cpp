/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifdef __PSP__

#include "backends/saves/psp/psp-saves.h"

#include "common/config-manager.h"
#include "common/savefile.h"

#include <pspkernel.h>

#define PSP_DEFAULT_SAVE_PATH "ms0:/residual_savegames"


PSPSaveFileManager::PSPSaveFileManager() {
	// Register default savepath
	ConfMan.registerDefault("savepath", PSP_DEFAULT_SAVE_PATH);
}
/*
PSPSaveFileManager::PSPSaveFileManager(const Common::String &defaultSavepath)
	: DefaultSaveFileManager(defaultSavepath) {
}
*/

void PSPSaveFileManager::checkPath(const Common::FSNode &dir) {
	const char *savePath = dir.getPath().c_str();
	clearError();

	//check if the save directory exists
	SceUID fd = sceIoDopen(savePath);
	if (fd < 0) {
		//No? then let's create it.
		sceIoMkdir(savePath, 0777);
	} else {
		//it exists, so close it again.
		sceIoDclose(fd);
	}
}
#endif

