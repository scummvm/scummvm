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
 * $URL: https://www.switchlink.se/svn/picture/script.cpp $
 * $Id: script.cpp 4 2008-08-08 14:21:30Z johndoe $
 *
 */

#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "sound/mixer.h"

#include "picture/picture.h"
#include "picture/animation.h"
#include "picture/input.h"
#include "picture/palette.h"
#include "picture/resource.h"
#include "picture/script.h"
#include "picture/screen.h"
#include "picture/segmap.h"

namespace Picture {

// TODO: Saveload is not working yet

void PictureEngine::savegame(const char *filename) {

	Common::OutSaveFile *out;
	if (!(out = g_system->getSavefileManager()->openForSaving(filename))) {
		warning("Can't create file '%s', game not saved", filename);
	}

	// Save game variables
	for (uint variable = 0; variable < 22; variable++) {
	    int16 value = _script->getGameVar(variable);
	    out->writeUint16LE(value);
	}

	_palette->saveState(out);
	_script->saveState(out);
	_anim->saveState(out);

	// Save GUI
	{
	
	}

/*
case 0: return "mouseDisabled";
case 1: return "mouseY";
case 2: return "mouseX";
case 3: return "mouseButton";
case 4: return "verbLineY";
case 5: return "verbLineX";
case 6: return "verbLineWidth";
case 7: return "verbLineCount";
case 8: return "verbLineNum";
case 9: return "talkTextItemNum";
case 10: return "talkTextY";
case 11: return "talkTextX";
case 12: return "talkTextFontColor";
case 13: return "cameraY";
case 14: return "cameraX";
case 15: return "walkSpeedY";
case 16: return "walkSpeedX";
case 17: return "flag01";
case 18: return "sceneResIndex";
case 19: return "cameraTop";
case 20: return "sceneHeight";
case 21: return "sceneWidth";
*/

/*
PersistentGameVarRef <offset _sceneHeight, 2>
PersistentGameVarRef <offset _sceneWidth, 2>
PersistentGameVarRef <offset screenFlag01, 2>
PersistentGameVarRef <offset currentSequenceResIndex, 4>
PersistentGameVarRef <offset currentSequenceLoopCount, 4>
PersistentGameVarRef <offset sequenceVolume, 4>
PersistentGameVarRef <offset dword_99AC0, 4>
PersistentGameVarRef <offset verbLineNum, 22h>
*/

	delete out;

}

void PictureEngine::loadgame(const char *filename) {
}


} // End of namespace Picture
