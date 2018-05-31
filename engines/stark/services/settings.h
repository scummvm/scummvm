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

#ifndef STARK_SERVICES_SETTINGS_H
#define STARK_SERVICES_SETTINGS_H

#include "common/config-manager.h"

#include "engines/stark/services/services.h"
#include "engines/advancedDetector.h"

namespace Audio {
class Mixer;
}

namespace Stark {

/**
 * Settings services.
 * 
 * Maintains the settings of the game.
 */
class Settings {
public:
	enum BoolSettingIndex {
		kHighModel,
		kSubtitle,
		kSpecialFX,
		kShadow,
		kHighFMV,
		kTimeSkip
	};

	enum IntSettingIndex {
		kVoice,
		kMusic,
		kSfx,
		kSaveLoadPage
	};

	static bool isDemo() {
		return StarkGameDescription->flags & ADGF_DEMO;
	}

	explicit Settings(Audio::Mixer *mixer);
	~Settings() {}

	/** Get the settings value */
	bool getBoolSetting(BoolSettingIndex index) { return ConfMan.getBool(_boolKey[index]); }
	int getIntSetting(IntSettingIndex index) { return ConfMan.getInt(_intKey[index]); }

	/** Flip the boolean settings */
	void flipSetting(BoolSettingIndex index) { 
		ConfMan.setBool(_boolKey[index], !getBoolSetting(index));
	}

	/** Set the integer settings */
	void setIntSetting(IntSettingIndex index, int value);

	/** Check whether low-resolution fmv is available */
	bool hasLowResFMV() { return _hasLowRes; }

private:
	Audio::Mixer *_mixer;
	bool _hasLowRes;

	const char *_boolKey[6];
	const char *_intKey[4];
};

} // End of namespace Stark

#endif // STARK_SERVICES_SETTINGS_H
