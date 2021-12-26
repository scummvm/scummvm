/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_SYSTEM_CONFIG_H
#define ASYLUM_SYSTEM_CONFIG_H

#include "common/array.h"
#include "common/singleton.h"
#include "common/config-manager.h"

namespace Asylum {

/**
 * The ConfigurationManager is where the common configuration options used
 * by the engine are bound.
 *
 * These options can be set through the main menu
 */
class ConfigurationManager: public Common::Singleton<ConfigurationManager> {
public:

	//////////////////////////////////////////////////////////////////////////
	// Normal configuration
	//////////////////////////////////////////////////////////////////////////

	// Default options
	int32 musicVolume;
	int32 sfxVolume;
	int32 voiceVolume;
	bool showMovieSubtitles;

	// Engine options
	bool showEncounterSubtitles;
	int  gammaLevel;
	int32  ambientVolume;
	int32  movieVolume;
	bool musicStatus; // On or Off
	bool reverseStereo;
	int  performance;    ///< performance only affects sound: perf > 0 ? 22050hz/16bit : 11025hz/8bit
	char keyShowVersion;
	char keyQuickSave;
	char keyQuickLoad;
	char keySwitchToSara;
	char keySwitchToGrimwall;
	char keySwitchToOlmec;

	//////////////////////////////////////////////////////////////////////////
	// Misc configuration options (not shown in GUI)
	//////////////////////////////////////////////////////////////////////////

	// This will play the scene title loading up progress before the scene is entered.
	// This is just a convenience, as there's no need for the type of pre-loading that
	// was performed in the original
	bool showSceneLoading;

	// This option will prevent the intro movies from being played whenever the engine is started
	bool showIntro;

	/**
	 * Load configuration file
	 */
	void read();

	/**
	 * Save a value to the configuration file
	 */
	void write();

	/**
	 * Query if 'key' is assigned.
	 *
	 * @param key The key.
	 *
	 * @return true if key assigned, false if not.
	 */
	bool isKeyAssigned(char key) const;

private:
	friend class Common::Singleton<SingletonBaseType>;

	ConfigurationManager();
	virtual ~ConfigurationManager();

};

#define Config (::Asylum::ConfigurationManager::instance())

} // end of namespace Asylum

#endif // ASYLUM_SYSTEM_CONFIG_H
