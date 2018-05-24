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

class Engine;

namespace Common {
class String;
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
		kSfx
	};

	explicit Settings(Engine *engine);
	~Settings() {}

	/** Save all the settings */
	void save() const;

	/** Get the settings value */
	bool getBoolSetting(BoolSettingIndex index) { return _boolSettings[index]; }
	int getIntSetting(IntSettingIndex index) { return _intSettings[index]; }

	/** Flip the boolean settings */
	void flipSetting(BoolSettingIndex index) { _boolSettings[index] = !_boolSettings[index]; }

	/** Set the integer settings */
	void setIntSetting(IntSettingIndex index, int value) { _intSettings[index] = value; }

	/** Check whether low-resolution fmv is available */
	bool hasLowResFMV() { return _hasLowRes; }

private:
	Engine *_engine;
	bool _boolSettings[8];
	int _intSettings[3];
	const Common::String &_domainName;
	bool _hasLowRes;

	void loadConf(const Common::String &key, bool &value, bool defaultValue) {
		value = ConfMan.hasKey(key, _domainName) ? ConfMan.getBool(key, _domainName) : defaultValue;
	}

	void loadConf(const Common::String &key, int &value, int defaultValue) {
		value = ConfMan.hasKey(key, _domainName) ? ConfMan.getInt(key, _domainName) : defaultValue;
	}

	bool getBoolDefault(const Common::String &key) const {
		return ConfMan.getBool(key, "residualvm");
	}

	bool getIntDefault(const Common::String &key) const {
		return ConfMan.getInt(key, "residualvm");
	}

	void saveConf(const Common::String &key, bool value) const {
		ConfMan.setBool(key, value, _domainName);
	}

	void saveConf(const Common::String &key, int value) const {
		ConfMan.setInt(key, value, _domainName);
	}
};

} // End of namespace Stark

#endif // STARK_SERVICES_SETTINGS_H
