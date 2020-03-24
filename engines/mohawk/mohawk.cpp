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

#include "common/scummsys.h"
#include "common/error.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "mohawk/mohawk.h"
#include "mohawk/cursors.h"
#include "mohawk/dialogs.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"

namespace Mohawk {

MohawkEngine::MohawkEngine(OSystem *syst, const MohawkGameDescription *gamedesc) : Engine(syst), _gameDescription(gamedesc) {
	// Setup mixer
	syncSoundSettings();

	_pauseDialog = nullptr;
	_cursor = nullptr;
}

MohawkEngine::~MohawkEngine() {
	delete _pauseDialog;
	delete _cursor;
	closeAllArchives();
}

Common::Error MohawkEngine::run() {
	_pauseDialog = new PauseDialog(this, _("The game is paused. Press any key to continue."));

	return Common::kNoError;
}

void MohawkEngine::pauseGame() {
	runDialog(*_pauseDialog);
}

Common::SeekableReadStream *MohawkEngine::getResource(uint32 tag, uint16 id) {
	for (uint32 i = 0; i < _mhk.size(); i++)
		if (_mhk[i]->hasResource(tag, id))
			return _mhk[i]->getResource(tag, id);

	error("Could not find a '%s' resource with ID %04x", tag2str(tag), id);
}

bool MohawkEngine::hasResource(uint32 tag, uint16 id) {
	for (uint32 i = 0; i < _mhk.size(); i++)
		if (_mhk[i]->hasResource(tag, id))
			return true;

	return false;
}

bool MohawkEngine::hasResource(uint32 tag, const Common::String &resName) {
	for (uint32 i = 0; i < _mhk.size(); i++)
		if (_mhk[i]->hasResource(tag, resName))
			return true;

	return false;
}

uint32 MohawkEngine::getResourceOffset(uint32 tag, uint16 id) {
	for (uint32 i = 0; i < _mhk.size(); i++)
		if (_mhk[i]->hasResource(tag, id))
			return _mhk[i]->getOffset(tag, id);

	error("Could not find a '%s' resource with ID %04x", tag2str(tag), id);
}

uint16 MohawkEngine::findResourceID(uint32 tag, const Common::String &resName) {
	for (uint32 i = 0; i < _mhk.size(); i++)
		if (_mhk[i]->hasResource(tag, resName))
			return _mhk[i]->findResourceID(tag, resName);

	error("Could not find a '%s' resource matching name '%s'", tag2str(tag), resName.c_str());
}

Common::String MohawkEngine::getResourceName(uint32 tag, uint16 id) {
	for (uint32 i = 0; i < _mhk.size(); i++)
		if (_mhk[i]->hasResource(tag, id)) {
			return _mhk[i]->getName(tag, id);
		}

	error("Could not find a \'%s\' resource with ID %04x", tag2str(tag), id);
}

void MohawkEngine::closeAllArchives() {
	for (uint32 i = 0; i < _mhk.size(); i++)
		delete _mhk[i];

	_mhk.clear();
}

} // End of namespace Mohawk
