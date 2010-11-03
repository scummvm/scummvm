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

#ifndef ASYLUM_CONFIGURATIONMANAGER_H
#define ASYLUM_CONFIGURATIONMANAGER_H

#include "common/array.h"
#include "common/singleton.h"
#include "common/config-manager.h"
#include "video.h"

namespace Asylum {

enum VideoSubtitles;

/**
 * The ConfigurationManager is where the common configuration options used
 * by the engine are bound.
 *
 * These options can be set through the main menu
 */
class ConfigurationManager: public Common::Singleton<ConfigurationManager> {
public:

	// XXX Throwing some globals in here until
	// I know how to better handle them
	int  word_446EE4;

    VideoSubtitles showMovieSubtitles;
	Common::Array<int>  movieSubtitles;
	bool showEncounterSubtitles;
	Common::Array<int>  encounterSubtitles; // FIXME: take this out
    int  musicVolume;
	int  ambientVolume;
	int  sfxVolume;
	int  voiceVolume;
	int  movieVolume;
	bool musicStatus; // On or Off
	bool reverseStereo;
	int  gammaLevel;

	/**
	 * XXX The game's performance settings seem to only affect
	 * the sound quality.
	 *
	 * (performance > 0) ? 22050hz/16bit : 11025hz/8bit
	 */
	int performance;
	int versionKey;
	int quickSaveKey;
	int quickLoadKey;
	int switchToSaraKey;
	int switchToGrimwallKey;
	int switchToOlmecKey;

	/**
	 * Load configuration file
	 */
	void read();

	/**
	 * Save a value to the configuration file
	 */
	void write();

private:
	friend class Common::Singleton<SingletonBaseType>;

	ConfigurationManager();
	virtual ~ConfigurationManager();

}; // end of class ConfigurationManager

#define Config (::Asylum::ConfigurationManager::instance())

} // end of namespace Asylum

#endif
