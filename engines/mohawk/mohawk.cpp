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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/events.h"
#include "common/keyboard.h"

#include "base/plugins.h"
#include "base/version.h"

#include "mohawk/mohawk.h"

namespace Mohawk {

MohawkEngine::MohawkEngine(OSystem *syst, const MohawkGameDescription *gamedesc) : Engine(syst), _gameDescription(gamedesc) {
	if (!_mixer->isReady())
		error ("Sound initialization failed");

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
}

MohawkEngine::~MohawkEngine() {
	delete _sound;
	delete _video;
	delete _pauseDialog;

	for (uint32 i = 0; i < _mhk.size(); i++)
		delete _mhk[i];
	_mhk.clear();
}

Common::Error MohawkEngine::run() {
	_sound = new Sound(this);
	_video = new VideoManager(this);
	_pauseDialog = new PauseDialog(this, "The game is paused. Press any key to continue.");

	return Common::kNoError;
}

void MohawkEngine::pauseEngineIntern(bool pause) {
	if (pause) {
		_video->pauseVideos();
		_sound->pauseSound();
		_sound->pauseSLST();
	} else {
		_video->resumeVideos();
		_sound->resumeSound();
		_sound->resumeSLST();
		_system->updateScreen();
	}
}

void MohawkEngine::pauseGame() {
	runDialog(*_pauseDialog);
}

Common::SeekableReadStream *MohawkEngine::getRawData(uint32 tag, uint16 id) {
	for (uint32 i = 0; i < _mhk.size(); i++)
		if (_mhk[i]->hasResource(tag, id))
			return _mhk[i]->getRawData(tag, id);

	error ("Could not find a \'%s\' resource with ID %04x", tag2str(tag), id);

	return 0;
}
	
bool MohawkEngine::hasResource(uint32 tag, uint16 id) {
	for (uint32 i = 0; i < _mhk.size(); i++)
		if (_mhk[i]->hasResource(tag, id))
			return true;
	
	return false;
}

} // End of namespace Mohawk
