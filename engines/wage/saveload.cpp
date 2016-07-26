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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/file.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "gui/saveload.h"

#include "graphics/thumbnail.h"
#include "graphics/surface.h"

#include "wage/wage.h"
#include "wage/world.h"
#include "wage/entities.h"

#define SAVEGAME_CURRENT_VERSION 1

//
// Version 0 (ScummVM):  first ScummVM version
//

namespace Wage {

static const uint32 AGIflag = MKTAG('W', 'A', 'G', 'E');

int WageEngine::saveGame(const Common::String &fileName, const Common::String &descriptionString) {
	Common::OutSaveFile *out;
	int result = 0;

	debug(9, "WageEngine::saveGame(%s, %s)", fileName.c_str(), descriptionString.c_str());
	if (!(out = _saveFileMan->openForSaving(fileName))) {
		warning("Can't create file '%s', game not saved", fileName.c_str());
		return -1;
	} else {
		debug(9, "Successfully opened %s for writing", fileName.c_str());
	}

	out->writeUint32BE(AGIflag);

	// Write description of saved game, limited to WAGE_SAVEDGAME_DESCRIPTION_LEN characters + terminating NUL
	const int WAGE_SAVEDGAME_DESCRIPTION_LEN = 127;
	char description[WAGE_SAVEDGAME_DESCRIPTION_LEN + 1];

	memset(description, 0, sizeof(description));
	strncpy(description, descriptionString.c_str(), WAGE_SAVEDGAME_DESCRIPTION_LEN);
	assert(WAGE_SAVEDGAME_DESCRIPTION_LEN + 1 == 128); // safety
	out->write(description, 128);

	out->writeByte(SAVEGAME_CURRENT_VERSION);
	debug(9, "Writing save game version (%d)", SAVEGAME_CURRENT_VERSION);

	// Thumbnail
	Graphics::saveThumbnail(*out);

	// Counters
	out->writeSint16LE(_world->_scenes.size()); //numScenes
	out->writeSint16LE(_world->_chrs.size()); //numChars
	out->writeSint16LE(_world->_objs.size()); //numObjs

	// Hex Offsets
	out->writeSint32LE(0); //state.getChrsHexOffset()
	out->writeSint32LE(0); //state.getObjsHexOffset()

	// Unique 8-byte World Signature
	out->writeSint32LE(0); //8-byte ints? seriously?

	// More Counters
	out->writeSint32LE(_world->_player->_context._visits); //visitNum
	out->writeSint32LE(0); //state.getLoopNum()
	out->writeSint32LE(_world->_player->_context._kills); //killNum

	// Hex offset to player character
	out->writeSint32LE(0); //state.getPlayerHexOffset()

	// character in this scene?
	out->writeSint32LE(0); //state.getPresCharHexOffset()

	// Hex offset to current scene
	out->writeSint32LE(0); //state.getCurSceneHexOffset()

	// wearing a helmet?
	out->writeSint32LE(0); //hex offset for <_world->_player->_armor[Chr::ChrArmorType::HEAD_ARMOR]> //state.getHelmetIndex()

	// holding a shield?
	out->writeSint32LE(0); //hex offset for <_world->_player->_armor[Chr::ChrArmorType::SHIELD_ARMOR]> //state.getShieldIndex()

	// wearing chest armor?
	out->writeSint32LE(0); //hex offset for <_world->_player->_armor[Chr::ChrArmorType::BODY_ARMOR]> //state.getChestArmIndex()

	// wearing spiritual armor?
	out->writeSint32LE(0); //hex offset for <_world->_player->_armor[Chr::ChrArmorType::MAGIC_ARMOR]> //state.getSprtArmIndex()

	// TODO:
	out->writeSint16LE(0xffff);	// ???? - always FFFF
	out->writeSint16LE(0xffff);	// ???? - always FFFF
	out->writeSint16LE(0xffff);	// ???? - always FFFF
	out->writeSint16LE(0xffff);	// ???? - always FFFF

	// did a character just escape?
	out->writeSint32LE(0); //state.getRunCharHexOffset()

	// players experience points
	out->writeSint32LE(_world->_player->_context._experience);

	out->writeSint16LE(0); //state.getAim()
	out->writeSint16LE(0); //state.getOpponentAim()

	// TODO:
	out->writeSint16LE(0x0000);	// always 0
	out->writeSint16LE(0x0000);	// always 0
	out->writeSint16LE(0x0000);	// always 0

	// Base character stats
	out->writeByte(_world->_player->_physicalStrength); //state.getBasePhysStr()
	out->writeByte(_world->_player->_physicalHp); //state.getBasePhysHp()
	out->writeByte(_world->_player->_naturalArmor); //state.getBasePhysArm()
	out->writeByte(_world->_player->_physicalAccuracy); //state.getBasePhysAcc()
	out->writeByte(_world->_player->_spiritualStength); //state.getBaseSprtStr()
	out->writeByte(_world->_player->_spiritialHp); //state.getBaseSprtHp()
	out->writeByte(_world->_player->_resistanceToMagic); //state.getBaseSprtArm()
	out->writeByte(_world->_player->_spiritualAccuracy); //state.getBaseSprtAcc()
	out->writeByte(_world->_player->_runningSpeed); //state.getBaseRunSpeed()

	// TODO:
	out->writeByte(0x0A);		// ???? - always seems to be 0x0A

	/*
	// write user vars
	for (short var : state.getUserVars())
		out->->writeSint16LE(var);

	// write updated info for all scenes
	out.write(state.getSceneData());

	// write updated info for all characters
	out.write(state.getChrData());

	// write updated info for all objects
	out.write(state.getObjData());
	*/

	out->finalize();
	if (out->err()) {
		warning("Can't write file '%s'. (Disk full?)", fileName.c_str());
		result = -1;
	} else
		debug(9, "Saved game %s in file %s", descriptionString.c_str(), fileName.c_str());

	delete out;
	return result;
}

Common::String WageEngine::getSavegameFilename(int16 slotId) const {
	Common::String saveLoadSlot = _targetName;
	saveLoadSlot += Common::String::format(".%.3d", slotId);
	return saveLoadSlot;
}

Common::Error WageEngine::saveGameState(int slot, const Common::String &description) {
	Common::String saveLoadSlot = getSavegameFilename(slot);
	if (saveGame(saveLoadSlot, description) == 0)
		return Common::kNoError;
	else
		return Common::kUnknownError;
}

} // End of namespace Agi
