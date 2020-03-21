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

#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/error.h"
#include "ultima/ultima4/event.h"
#include "ultima/ultima4/filesystem.h"
#include "ultima/ultima4/utils.h"

namespace Ultima {
namespace Ultima4 {

using namespace std;

/*
 * Initialize static members
 */ 
Settings *Settings::_instance = NULL;

bool SettingsData::operator==(const SettingsData &s) const {    
    int fieldsSize = (int *)&_end_of_bitwise_comparators - (int *)this;
    if (memcmp(this, &s, fieldsSize) != 0)
        return false;

    if (_filter != s._filter)
        return false;
    if (_gemLayout != s._gemLayout)
        return false;
    if (_lineOfSight != s._lineOfSight)
        return false;
    if (_videoType != s._videoType)
        return false;
    if (_battleDiff != s._battleDiff)
        return false;
    if (_logging != s._logging)
        return false;
    if (_game != s._game)
        return false;

    return true;
}

bool SettingsData::operator!=(const SettingsData &s) const {
    return !operator==(s);
}


/**
 * Default contructor.  Settings is a singleton so this is private.
 */
Settings::Settings() {
	init();

    _battleDiffs.push_back("Normal");
    _battleDiffs.push_back("Hard");
    _battleDiffs.push_back("Expert");
}


/**
 * Initialize the settings.
 */
void Settings::init() {
    read();
}


/**
 * Return the global instance of settings.
 */
Settings &Settings::getInstance() {
    if (_instance == NULL)
        _instance = new Settings();
    return *_instance;
}

void Settings::setData(const SettingsData &data) {
    // bitwise copy is safe
    *(SettingsData *)this = data;
}

/**
 * Read settings
 */
bool Settings::read() {
	Common::File settingsFile;

	/* default settings */
    _scale                 = DEFAULT_SCALE;
    _fullscreen            = DEFAULT_FULLSCREEN;
    _filter                = DEFAULT_FILTER;
    _videoType             = DEFAULT_VIDEO_TYPE;
    _gemLayout             = DEFAULT_GEM_LAYOUT;
    _lineOfSight           = DEFAULT_LINEOFSIGHT;
    _screenShakes          = DEFAULT_SCREEN_SHAKES;
    _gamma                 = DEFAULT_GAMMA;
    _musicVol              = DEFAULT_MUSIC_VOLUME;
    _soundVol              = DEFAULT_SOUND_VOLUME;
    _volumeFades           = DEFAULT_VOLUME_FADES;
    _shortcutCommands      = DEFAULT_SHORTCUT_COMMANDS;
    _keydelay              = DEFAULT_KEY_DELAY;
    _keyinterval           = DEFAULT_KEY_INTERVAL;
    _filterMoveMessages    = DEFAULT_FILTER_MOVE_MESSAGES;
    _battleSpeed           = DEFAULT_BATTLE_SPEED;
    _enhancements          = DEFAULT_ENHANCEMENTS;    
    _gameCyclesPerSecond   = DEFAULT_CYCLES_PER_SECOND;
    _screenAnimationFramesPerSecond = DEFAULT_ANIMATION_FRAMES_PER_SECOND;
    _debug                 = DEFAULT_DEBUG;
    _battleDiff            = DEFAULT_BATTLE_DIFFICULTY;
    _validateXml           = DEFAULT_VALIDATE_XML;
    _spellEffectSpeed      = DEFAULT_SPELL_EFFECT_SPEED;
    _campTime              = DEFAULT_CAMP_TIME;
    _innTime               = DEFAULT_INN_TIME;
    _shrineTime            = DEFAULT_SHRINE_TIME;
    _shakeInterval         = DEFAULT_SHAKE_INTERVAL;
    _titleSpeedRandom      = DEFAULT_TITLE_SPEED_RANDOM;
    _titleSpeedOther       = DEFAULT_TITLE_SPEED_OTHER;

    _pauseForEachMovement  = DEFAULT_PAUSE_FOR_EACH_MOVEMENT;
    _pauseForEachTurn	  = DEFAULT_PAUSE_FOR_EACH_TURN;

    /* all specific minor enhancements default to "on", any major enhancements default to "off" */
    _enhancementsOptions._activePlayer     = true;
    _enhancementsOptions._u5spellMixing    = true;
    _enhancementsOptions._u5shrines        = true;
    _enhancementsOptions._slimeDivides     = true;
    _enhancementsOptions._gazerSpawnsInsects = true;
    _enhancementsOptions._textColorization = false;
    _enhancementsOptions._c64chestTraps    = true;
    _enhancementsOptions._smartEnterKey    = true;
    _enhancementsOptions._peerShowsObjects = false;
    _enhancementsOptions._u5combat         = false;
    _enhancementsOptions._u4TileTransparencyHack = true;
    _enhancementsOptions._u4TileTransparencyHackPixelShadowOpacity = DEFAULT_SHADOW_PIXEL_OPACITY;
    _enhancementsOptions._u4TrileTransparencyHackShadowBreadth = DEFAULT_SHADOW_PIXEL_SIZE;

    _innAlwaysCombat = 0;
    _campingAlwaysCombat = 0;

    /* mouse defaults to on */
    _mouseOptions.enabled = 1;

    _logging = DEFAULT_LOGGING;
    _game = "Ultima IV";

#ifdef TODO
	if (!settingsFile.open(_filename))
		return false;

	while(fgets(buffer, sizeof(buffer), settingsFile) != NULL) {
        while (Common::isSpace(buffer[strlen(buffer) - 1]))
            buffer[strlen(buffer) - 1] = '\0';

        if (strstr(buffer, "scale=") == buffer)
            scale = (unsigned int) strtoul(buffer + strlen("scale="), NULL, 0);
        else if (strstr(buffer, "fullscreen=") == buffer)
            fullscreen = (int) strtoul(buffer + strlen("fullscreen="), NULL, 0);
        else if (strstr(buffer, "filter=") == buffer)
            filter = buffer + strlen("filter=");
        else if (strstr(buffer, "video=") == buffer)
            videoType = buffer + strlen("video=");
        else if (strstr(buffer, "gemLayout=") == buffer)
            gemLayout = buffer + strlen("gemLayout=");
        else if (strstr(buffer, "lineOfSight=") == buffer)
            lineOfSight = buffer + strlen("lineOfSight=");
        else if (strstr(buffer, "screenShakes=") == buffer)
            screenShakes = (int) strtoul(buffer + strlen("screenShakes="), NULL, 0);        
        else if (strstr(buffer, "gamma=") == buffer)
            gamma = (int) strtoul(buffer + strlen("gamma="), NULL, 0);        
        else if (strstr(buffer, "musicVol=") == buffer)
            musicVol = (int) strtoul(buffer + strlen("musicVol="), NULL, 0);
        else if (strstr(buffer, "soundVol=") == buffer)
            soundVol = (int) strtoul(buffer + strlen("soundVol="), NULL, 0);
        else if (strstr(buffer, "volumeFades=") == buffer)
            volumeFades = (int) strtoul(buffer + strlen("volumeFades="), NULL, 0);        
        else if (strstr(buffer, "shortcutCommands=") == buffer)
            shortcutCommands = (int) strtoul(buffer + strlen("shortcutCommands="), NULL, 0);
        else if (strstr(buffer, "keydelay=") == buffer)
            keydelay = (int) strtoul(buffer + strlen("keydelay="), NULL, 0);
        else if (strstr(buffer, "keyinterval=") == buffer)
            keyinterval = (int) strtoul(buffer + strlen("keyinterval="), NULL, 0);
        else if (strstr(buffer, "filterMoveMessages=") == buffer)
            filterMoveMessages = (int) strtoul(buffer + strlen("filterMoveMessages="), NULL, 0);
        else if (strstr(buffer, "battlespeed=") == buffer)
            battleSpeed = (int) strtoul(buffer + strlen("battlespeed="), NULL, 0);
        else if (strstr(buffer, "enhancements=") == buffer)
            enhancements = (int) strtoul(buffer + strlen("enhancements="), NULL, 0);        
        else if (strstr(buffer, "gameCyclesPerSecond=") == buffer)
            gameCyclesPerSecond = (int) strtoul(buffer + strlen("gameCyclesPerSecond="), NULL, 0);
        else if (strstr(buffer, "debug=") == buffer)
            debug = (int) strtoul(buffer + strlen("debug="), NULL, 0);
        else if (strstr(buffer, "battleDiff=") == buffer)
            battleDiff = buffer + strlen("battleDiff=");
        else if (strstr(buffer, "validateXml=") == buffer)
            validateXml = (int) strtoul(buffer + strlen("validateXml="), NULL, 0);
        else if (strstr(buffer, "spellEffectSpeed=") == buffer)
            spellEffectSpeed = (int) strtoul(buffer + strlen("spellEffectSpeed="), NULL, 0);
        else if (strstr(buffer, "campTime=") == buffer)
            campTime = (int) strtoul(buffer + strlen("campTime="), NULL, 0);
        else if (strstr(buffer, "innTime=") == buffer)
            innTime = (int) strtoul(buffer + strlen("innTime="), NULL, 0);
        else if (strstr(buffer, "shrineTime=") == buffer)
            shrineTime = (int) strtoul(buffer + strlen("shrineTime="), NULL, 0);
        else if (strstr(buffer, "shakeInterval=") == buffer)
            shakeInterval = (int) strtoul(buffer + strlen("shakeInterval="), NULL, 0);
        else if (strstr(buffer, "titleSpeedRandom=") == buffer)
            titleSpeedRandom = (int) strtoul(buffer + strlen("titleSpeedRandom="), NULL, 0);
        else if (strstr(buffer, "titleSpeedOther=") == buffer)
            titleSpeedOther = (int) strtoul(buffer + strlen("titleSpeedOther="), NULL, 0);
        
        /* minor enhancement options */
        else if (strstr(buffer, "activePlayer=") == buffer)
            enhancementsOptions.activePlayer = (int) strtoul(buffer + strlen("activePlayer="), NULL, 0);
        else if (strstr(buffer, "u5spellMixing=") == buffer)
            enhancementsOptions.u5spellMixing = (int) strtoul(buffer + strlen("u5spellMixing="), NULL, 0);
        else if (strstr(buffer, "u5shrines=") == buffer)
            enhancementsOptions.u5shrines = (int) strtoul(buffer + strlen("u5shrines="), NULL, 0);
        else if (strstr(buffer, "slimeDivides=") == buffer)
            enhancementsOptions.slimeDivides = (int) strtoul(buffer + strlen("slimeDivides="), NULL, 0);
        else if (strstr(buffer, "gazerSpawnsInsects=") == buffer)
            enhancementsOptions.gazerSpawnsInsects = (int) strtoul(buffer + strlen("gazerSpawnsInsects="), NULL, 0);
        else if (strstr(buffer, "textColorization=") == buffer)
            enhancementsOptions.textColorization = (int) strtoul(buffer + strlen("textColorization="), NULL, 0);
        else if (strstr(buffer, "c64chestTraps=") == buffer)
            enhancementsOptions.c64chestTraps = (int) strtoul(buffer + strlen("c64chestTraps="), NULL, 0);                
        else if (strstr(buffer, "smartEnterKey=") == buffer)
            enhancementsOptions.smartEnterKey = (int) strtoul(buffer + strlen("smartEnterKey="), NULL, 0);        
        
        /* major enhancement options */
        else if (strstr(buffer, "peerShowsObjects=") == buffer)
            enhancementsOptions.peerShowsObjects = (int) strtoul(buffer + strlen("peerShowsObjects="), NULL, 0);
        else if (strstr(buffer, "u5combat=") == buffer)
            enhancementsOptions.u5combat = (int) strtoul(buffer + strlen("u5combat="), NULL, 0);
        else if (strstr(buffer, "innAlwaysCombat=") == buffer)
            innAlwaysCombat = (int) strtoul(buffer + strlen("innAlwaysCombat="), NULL, 0);
        else if (strstr(buffer, "campingAlwaysCombat=") == buffer)
            campingAlwaysCombat = (int) strtoul(buffer + strlen("campingAlwaysCombat="), NULL, 0);    

        /* mouse options */
        else if (strstr(buffer, "mouseEnabled=") == buffer)
            mouseOptions.enabled = (int) strtoul(buffer + strlen("mouseEnabled="), NULL, 0);
        else if (strstr(buffer, "logging=") == buffer)
            logging = buffer + strlen("logging=");
        else if (strstr(buffer, "game=") == buffer)
            game = buffer + strlen("game=");

        /* graphics enhancements options */
        else if (strstr(buffer, "renderTileTransparency=") == buffer)
            enhancementsOptions.u4TileTransparencyHack = (int) strtoul(buffer + strlen("renderTileTransparency="), NULL, 0);
        else if (strstr(buffer, "transparentTilePixelShadowOpacity=") == buffer)
        	enhancementsOptions.u4TileTransparencyHackPixelShadowOpacity = (int) strtoul(buffer + strlen("transparentTilePixelShadowOpacity="), NULL, 0);
        else if (strstr(buffer, "transparentTileShadowSize=") == buffer)
        	enhancementsOptions.u4TrileTransparencyHackShadowBreadth = (int) strtoul(buffer + strlen("transparentTileShadowSize="), NULL, 0);



        /**
         * FIXME: this is just to avoid an error for those who have not written
         * a new xu4.cfg file since these items were removed.  Remove them after a reasonable
         * amount of time 
         *
         * remove:  attackspeed, minorEnhancements, majorEnhancements, vol
         */
        
        else if (strstr(buffer, "attackspeed=") == buffer);
        else if (strstr(buffer, "minorEnhancements=") == buffer)
            enhancements = (int)strtoul(buffer + strlen("minorEnhancements="), NULL, 0);
        else if (strstr(buffer, "majorEnhancements=") == buffer);
        else if (strstr(buffer, "vol=") == buffer)
            musicVol = soundVol = (int) strtoul(buffer + strlen("vol="), NULL, 0);        
        
        /***/

        else
            errorWarning("invalid line in settings file %s", buffer);
    }

    fclose(settingsFile);
#endif

    eventTimerGranularity = (1000 / _gameCyclesPerSecond);
    return true;
}

/**
 * Write the settings out into a human readable file.  This also
 * notifies observers that changes have been commited.
 */
bool Settings::write() {    
#ifdef TODO
	FILE *settingsFile;
        
    settingsFile = fopen(filename.c_str(), "wt");
    if (!settingsFile) {
        errorWarning("can't write settings file");
        return false;
    }    

    fprintf(settingsFile, 
            "scale=%d\n"
            "fullscreen=%d\n"
            "filter=%s\n"
            "video=%s\n"
            "gemLayout=%s\n"
            "lineOfSight=%s\n"
            "screenShakes=%d\n"
            "gamma=%d\n"
            "musicVol=%d\n"
            "soundVol=%d\n"
            "volumeFades=%d\n"
            "shortcutCommands=%d\n"
            "keydelay=%d\n"
            "keyinterval=%d\n"
            "filterMoveMessages=%d\n"
            "battlespeed=%d\n"
            "enhancements=%d\n"            
            "gameCyclesPerSecond=%d\n"
            "debug=%d\n"
            "battleDiff=%s\n"
            "validateXml=%d\n"
            "spellEffectSpeed=%d\n"
            "campTime=%d\n"
            "innTime=%d\n"
            "shrineTime=%d\n"
            "shakeInterval=%d\n"
            "titleSpeedRandom=%d\n"
            "titleSpeedOther=%d\n"
            "activePlayer=%d\n"
            "u5spellMixing=%d\n"
            "u5shrines=%d\n"
            "slimeDivides=%d\n"
            "gazerSpawnsInsects=%d\n"
            "textColorization=%d\n"
            "c64chestTraps=%d\n"            
            "smartEnterKey=%d\n"
            "peerShowsObjects=%d\n"
            "u5combat=%d\n"
            "innAlwaysCombat=%d\n"
            "campingAlwaysCombat=%d\n"
            "mouseEnabled=%d\n"
            "logging=%s\n"
            "game=%s\n"
            "renderTileTransparency=%d\n"
            "transparentTilePixelShadowOpacity=%d\n"
            "transparentTileShadowSize=%d\n",
            scale,
            fullscreen,
            filter.c_str(),
            videoType.c_str(),
            gemLayout.c_str(),
            lineOfSight.c_str(),
            screenShakes,
            gamma,
            musicVol,
            soundVol,
            volumeFades,
            shortcutCommands,
            keydelay,
            keyinterval,
            filterMoveMessages,
            battleSpeed,
            enhancements,            
            gameCyclesPerSecond,
            debug,
            battleDiff.c_str(),
            validateXml,
            spellEffectSpeed,
            campTime,
            innTime,
            shrineTime,
            shakeInterval,
            titleSpeedRandom,
            titleSpeedOther,
            enhancementsOptions.activePlayer,
            enhancementsOptions.u5spellMixing,
            enhancementsOptions.u5shrines,
            enhancementsOptions.slimeDivides,
            enhancementsOptions.gazerSpawnsInsects,
            enhancementsOptions.textColorization,
            enhancementsOptions.c64chestTraps,            
            enhancementsOptions.smartEnterKey,
            enhancementsOptions.peerShowsObjects,
            enhancementsOptions.u5combat,
            innAlwaysCombat,
            campingAlwaysCombat,
            mouseOptions.enabled,
            logging.c_str(),
            game.c_str(),
            enhancementsOptions.u4TileTransparencyHack,
            enhancementsOptions.u4TileTransparencyHackPixelShadowOpacity,
            enhancementsOptions.u4TrileTransparencyHackShadowBreadth);

    fclose(settingsFile);

    setChanged();
    notifyObservers(NULL);
#endif
    return true;
}

const Std::vector<Common::String> &Settings::getBattleDiffs() { 
    return _battleDiffs;
}

} // End of namespace Ultima4
} // End of namespace Ultima
