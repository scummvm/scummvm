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

#include "engines/advancedDetector.h"

namespace Stark {

Settings::Settings(Audio::Mixer *mixer, const ADGameDescription *gd) :
		_mixer(mixer),
		_isDemo(gd->flags & ADGF_DEMO),
		_language(gd->language) {
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
	_intKey[kSaveLoadPage] = "saveload_lastpage";

	// Register default settings
	ConfMan.registerDefault(_boolKey[kHighModel], true);
	ConfMan.registerDefault(_boolKey[kSubtitle], true);
	ConfMan.registerDefault(_boolKey[kSpecialFX], true);
	ConfMan.registerDefault(_boolKey[kShadow], true);
	ConfMan.registerDefault(_boolKey[kHighFMV], true);
	ConfMan.registerDefault(_boolKey[kTimeSkip], false);
	ConfMan.registerDefault(_intKey[kSaveLoadPage], 0);
	ConfMan.registerDefault("replacement_png_premultiply_alpha", false);
	ConfMan.registerDefault("ignore_font_settings", true);

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
			break;
		default:
			return;
	}

	_mixer->setVolumeForSoundType(type, value);
}

bool Settings::isAssetsModEnabled() const {
	return ConfMan.getBool("enable_assets_mod");
}

bool Settings::shouldPreMultiplyReplacementPNGs() const {
	return ConfMan.getBool("replacement_png_premultiply_alpha");
}

Gfx::Texture::SamplingFilter Settings::getImageSamplingFilter() const {
	return ConfMan.getBool("use_linear_filtering") ? Gfx::Texture::kLinear : Gfx::Texture::kNearest;
}

bool Settings::isFontAntialiasingEnabled() const {
	return ConfMan.getBool("enable_font_antialiasing");
}

Common::CodePage Settings::getTextCodePage() const {
	switch (_language) {
	case Common::PL_POL:
		return Common::kWindows1250;
	case Common::RU_RUS:
		return Common::kWindows1251;
	default:
		return Common::kWindows1252;
	}
}

bool Settings::shouldIgnoreFontSettings() const {
	return ConfMan.getBool("ignore_font_settings") && _language == Common::EN_ANY;
}

} // End of namespace Stark
