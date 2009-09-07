/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "common/config-manager.h"

#include "engines/stark/stark.h"
#include "engines/stark/adpcm.h"
#include "engines/stark/sound.h"

#include "sound/mixer.h"

namespace Stark {

StarkEngine::StarkEngine(OSystem *syst, const StarkGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, 127);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
}

StarkEngine::~StarkEngine() {
	_xArchive.close();
}

Common::Error StarkEngine::run() {
	// Load in the main archive
	
	if (getGameID() == GID_TLJ) {
		if (!_xArchive.open("45/00/00.xarc")) {
			printf("Could not open x.xarc!\n");
			return Common::kNoError;
		}
		/*XRCFile *xrc = new XRCFile(_graphArchive.getRawData(0));
		delete xrc;*/

		Audio::SoundHandle *s = new Audio::SoundHandle();
		Common::File _f;
		_f.open("45/xarc/00000003.iss");
		Common::SeekableReadStream *dat = _f.readStream(_f.size());

		ISS *sound = new ISS(dat);
		_mixer->playInputStream(Audio::Mixer::kPlainSoundType, s, sound->_stream);

		g_system->delayMillis(100000);
		
	} else {
		
	}

	return Common::kNoError;
}

} // end of namespace Stark
