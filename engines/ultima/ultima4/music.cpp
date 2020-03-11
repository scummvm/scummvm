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

#include "ultima/ultima4/music.h"
#include "ultima/ultima4/sound.h"
#include "ultima/ultima4/config.h"
#include "ultima/ultima4/context.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/error.h"
#include "ultima/ultima4/event.h"
#include "ultima/ultima4/location.h"
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/u4file.h"

namespace Ultima {
namespace Ultima4 {

using Common::String;
using Std::vector;

/*
 * Static variables
 */
Music *Music::instance = NULL;
bool Music::fading = false;
bool Music::on = false;
bool Music::functional = true;

/*
 * Constructors/Destructors
 */

/**
 * Initiliaze the music
 */
Music::Music() : introMid(TOWNS), current(NONE), playing(NULL), logger(new Debug("debug/music.txt", "Music")) {
    filenames.reserve(MAX);
    filenames.push_back("");    // filename for MUSIC_NONE;

    TRACE(*logger, "Initializing music");

    /*
     * load music track filenames from xml config file
     */
    const Config *config = Config::getInstance();

    TRACE_LOCAL(*logger, "Loading music tracks");

    vector<ConfigElement> musicConfs = config->getElement("music").getChildren();
    Std::vector<ConfigElement>::const_iterator i = musicConfs.begin();
    Std::vector<ConfigElement>::const_iterator theEnd = musicConfs.end();
    for (; i != theEnd; ++i) {
        if (i->getName() != "track")
            continue;

        filenames.push_back(i->getString("file"));
        TRACE_LOCAL(*logger, Common::String("\tTrack file: ") + filenames.back());
    }

	create_sys(); // Call the Sound System specific creation file.

	// Set up the volume.
    on = settings.musicVol;
    setMusicVolume(settings.musicVol);
    setSoundVolume(settings.soundVol);
    TRACE(*logger, Common::String("Music initialized: volume is ") + (on ? "on" : "off"));
}

/**
 * Stop playing the music and cleanup
 */
Music::~Music() {
    TRACE(*logger, "Uninitializing music");
    eventHandler->getTimer()->remove(&Music::callback);
	destroy_sys(); // Call the Sound System specific destruction file.

    TRACE(*logger, "Music uninitialized");
    delete logger;
}


bool Music::load(Type music) {
    ASSERT(music < MAX, "Attempted to load an invalid piece of music in Music::load()");

    /* music already loaded */
    if (music == current) {
        /* tell calling function it didn't load correctly (because it's already playing) */
        if (isPlaying())
            return false;
        /* it loaded correctly */
        else 
            return true;
    }

    Common::String pathname(u4find_music(filenames[music]));
    if (!pathname.empty()) {
		bool status = load_sys(pathname);
		if (status)
			current = music;
		return status;
    }
    return false;
}

/**
 * Ensures that the music is playing if it is supposed to be, or off
 * if it is supposed to be turned off.
 */
void Music::callback(void *data) {    
    eventHandler->getTimer()->remove(&Music::callback);

    if (musicMgr->on && !isPlaying())
        musicMgr->play();
    else if (!musicMgr->on && isPlaying())
        musicMgr->stop();
}
    
/**
 * Main music loop
 */
void Music::play() {
    playMid(c->location->map->music);
}

/**
 * Cycle through the introduction music
 */
void Music::introSwitch(int n) {
    if (n > NONE && n < MAX) {
        introMid = static_cast<Type>(n);
        intro();
    }
}

/**
 * Toggle the music on/off (usually by pressing 'v')
 */
bool Music::toggle() {
    eventHandler->getTimer()->remove(&Music::callback);

    on = !on;
    if (!on)
        fadeOut(1000);
    else
        fadeIn(1000, true);

    eventHandler->getTimer()->add(&Music::callback, settings.gameCyclesPerSecond);
    return on;    
}

/**
 * Fade out the music
 */
void Music::fadeOut(int msecs) {
	// fade the music out even if 'on' is false
	if (!functional)
		return;

	if (isPlaying()) {
		if (!settings.volumeFades)
			stop();
		else {
			fadeOut_sys(msecs);
		}
	}
}

/**
 * Fade in the music
 */
void Music::fadeIn(int msecs, bool loadFromMap) {
	if (!functional || !on)
		return;

	if (!isPlaying()) {
		/* make sure we've got something loaded to play */
		if (loadFromMap || !playing)
			load(c->location->map->music);

		if (!settings.volumeFades)
			play();
		else {
			fadeIn_sys(msecs, loadFromMap);
		}
	}
}

int Music::increaseMusicVolume() {
    if (++settings.musicVol > MAX_VOLUME)
        settings.musicVol = MAX_VOLUME;
    else
        setMusicVolume(settings.musicVol);
    return (settings.musicVol * 100 / MAX_VOLUME);  // percentage
}

int Music::decreaseMusicVolume() {
    if (--settings.musicVol < 0)
        settings.musicVol = 0;
    else
        setMusicVolume(settings.musicVol);
    return (settings.musicVol * 100 / MAX_VOLUME);  // percentage
}



int Music::increaseSoundVolume() {
    if (++settings.soundVol > MAX_VOLUME)
        settings.soundVol = MAX_VOLUME;
    else
        setSoundVolume(settings.soundVol);
    return (settings.soundVol * 100 / MAX_VOLUME);  // percentage
}

int Music::decreaseSoundVolume() {
    if (--settings.soundVol < 0)
        settings.soundVol = 0;
    else
        setSoundVolume(settings.soundVol);
    return (settings.soundVol * 100 / MAX_VOLUME);  // percentage
}

} // End of namespace Ultima4
} // End of namespace Ultima
