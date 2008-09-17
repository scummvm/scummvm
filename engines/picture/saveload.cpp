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
#include "picture/palette.h"
#include "picture/resource.h"
#include "picture/script.h"
#include "picture/screen.h"
#include "picture/segmap.h"

namespace Picture {

/* TODO:
	- Saveload is working so far but only one slot is supported until the game menu is implemented
	- Save with F6; Load with F9
	- Saving during an animation (AnimationPlayer) is not working correctly yet
	- Maybe switch to SCUMM/Tinsel serialization approach?
*/

#define SAVEGAME_VERSION 0 // 0 is dev version until in official SVN

void PictureEngine::savegame(const char *filename) {

	Common::OutSaveFile *out;
	if (!(out = g_system->getSavefileManager()->openForSaving(filename))) {
		warning("Can't create file '%s', game not saved", filename);
		return;
	}

	out->writeUint32LE(SAVEGAME_VERSION);

	out->writeUint16LE(_cameraX);
	out->writeUint16LE(_cameraY);
	out->writeUint16LE(_cameraHeight);

	out->writeUint16LE(_guiHeight);

	out->writeUint16LE(_sceneWidth);
	out->writeUint16LE(_sceneHeight);
	out->writeUint32LE(_sceneResIndex);

	out->writeUint16LE(_walkSpeedX);
	out->writeUint16LE(_walkSpeedY);

	out->writeUint32LE(_counter01);
	out->writeUint32LE(_counter02);
	out->writeByte(_movieSceneFlag ? 1 : 0);
	out->writeByte(_flag01);

	out->writeUint16LE(_mouseX);
	out->writeUint16LE(_mouseY);
	out->writeUint16LE(_mouseDisabled);

	_palette->saveState(out);
	_script->saveState(out);
	_anim->saveState(out);
	_screen->saveState(out);

	delete out;

}

void PictureEngine::loadgame(const char *filename) {

	Common::InSaveFile *in;
	if (!(in = g_system->getSavefileManager()->openForLoading(filename))) {
		warning("Can't open file '%s', game not loaded", filename);
		return;
	}
	
	uint32 version = in->readUint32LE();
	if (version != SAVEGAME_VERSION) {
		warning("Savegame '%s' too old, game not loaded (got v%d, need v%d)", filename, version, SAVEGAME_VERSION);
		return;
	}
	
	_cameraX = in->readUint16LE();
	_cameraY = in->readUint16LE();
	_cameraHeight = in->readUint16LE();

	_guiHeight = in->readUint16LE();

	_sceneWidth = in->readUint16LE();
	_sceneHeight = in->readUint16LE();
	_sceneResIndex = in->readUint32LE();

	_walkSpeedX = in->readUint16LE();
	_walkSpeedY = in->readUint16LE();

	_counter01 = in->readUint32LE();
	_counter02 = in->readUint32LE();
	_movieSceneFlag = in->readByte() != 0;
	_flag01 = in->readByte();

	_mouseX = in->readUint16LE();
	_mouseY = in->readUint16LE();
	_mouseDisabled = in->readUint16LE();
	
	_system->warpMouse(_mouseX, _mouseY);
 	_system->showMouse(_mouseDisabled == 0);

	_palette->loadState(in);
	_script->loadState(in);
	_anim->loadState(in);
	_screen->loadState(in);

	delete in;

	loadScene(_sceneResIndex);
	_screen->clearSprites();

	_newCameraX = _cameraX;
	_newCameraY = _cameraY;

}

} // End of namespace Picture
