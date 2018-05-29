/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/services/settings.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/archiveloader.h"

#include "common/config-manager.h"
#include "common/debug.h"

#include "audio/mixer.h"

namespace Stark {

Settings::Settings(Audio::Mixer *mixer) :
		_mixer(mixer) {
	// Initialize keys
	_boolKey[kHighModel] = "enable_high_resolution_models";
	_boolKey[kSubtitle] = "subtitles";
	_boolKey[kSpecialFX] = "enable_special_effects";
	_boolKey[kShadow] = "enable_shadows";
	_boolKey[kHighFMV] = "play_high_resolution_videos";
	_boolKey[kTimeSkip] = "enable_time_skip";
	_intKey[kVoice] = "speech_volume";
	_intKey[kMusic] = "music_volume";
	_intKey[kSfx] = "sfx_volume";

	// Register default settings
	ConfMan.registerDefault(_boolKey[kHighModel], true);
	ConfMan.registerDefault(_boolKey[kSubtitle], true);
	ConfMan.registerDefault(_boolKey[kSpecialFX], true);
	ConfMan.registerDefault(_boolKey[kShadow], true);
	ConfMan.registerDefault(_boolKey[kHighFMV], true);
	ConfMan.registerDefault(_boolKey[kTimeSkip], false);

	// Use the FunCom logo video to check low-resolution fmv
	Common::SeekableReadStream *lowResFMV = StarkArchiveLoader->getExternalFile("1402_lo_res.bbb", "Global/");
	_hasLowRes = lowResFMV;
	delete lowResFMV;
}

void Settings::setIntSetting(IntSettingIndex index, int value) {
	ConfMan.setInt(_intKey[index], value);

	Audio::Mixer::SoundType type;
	switch (index) {
		case kVoice:
			type = Audio::Mixer::kSpeechSoundType;
			break;
		case kMusic:
			type = Audio::Mixer::kMusicSoundType;
			break;
		case kSfx:
			type = Audio::Mixer::kSFXSoundType;
	}

	_mixer->setVolumeForSoundType(type, value);
}

} // End of namespace Stark
