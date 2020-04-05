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

#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/core/error.h"
#include "ultima/ultima4/events/event.h"
#include "ultima/ultima4/filesys/filesystem.h"
#include "ultima/ultima4/core/utils.h"
#include "common/config-manager.h"
#include "common/file.h"

namespace Ultima {
namespace Ultima4 {

using namespace std;

/*
 * Initialize static members
 */
Settings *Settings::_instance = NULL;

bool SettingsData::operator==(const SettingsData &s) const {
	// TODO: Refactor this to a clean comparison of individual settings
	int fieldsSize = (const int *)&_end_of_bitwise_comparators - (const int *)this;
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


Settings::Settings() {
	init();

	_battleDiffs.push_back("Normal");
	_battleDiffs.push_back("Hard");
	_battleDiffs.push_back("Expert");
}

void Settings::init() {
	read();
}

Settings &Settings::getInstance() {
	if (_instance == NULL)
		_instance = new Settings();
	return *_instance;
}

void Settings::setData(const SettingsData &data) {
	// bitwise copy is safe
	*(SettingsData *)this = data;
}

bool Settings::read() {
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
	_debug                 = gDebugLevel > 0;
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
	_pauseForEachTurn     = DEFAULT_PAUSE_FOR_EACH_TURN;

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

	if (ConfMan.hasKey("gemLayout"))
		_gemLayout = ConfMan.get("gemLayout");
	if (ConfMan.hasKey("lineOfSight"))
		_lineOfSight = ConfMan.get("lineOfSight");
	if (ConfMan.hasKey("screenShakes"))
		_screenShakes = ConfMan.getBool("screenShakes");
	if (ConfMan.hasKey("gamma"))
		_gamma = ConfMan.getInt("gamma");

	if (ConfMan.hasKey("volumeFades"))
		_volumeFades = ConfMan.getBool("volumeFades");
	if (ConfMan.hasKey("shortcutCommands"))
		_shortcutCommands = ConfMan.getBool("shortcutCommands");
	if (ConfMan.hasKey("keydelay"))
		_keydelay = ConfMan.getInt("keydelay");
	if (ConfMan.hasKey("filterMoveMessages"))
		_filterMoveMessages = ConfMan.getBool("filterMoveMessages");
	if (ConfMan.hasKey("battlespeed"))
		_battleSpeed = ConfMan.getInt("battlespeed");
	if (ConfMan.hasKey("enhancements"))
		_enhancements = ConfMan.getBool("enhancements");
	if (ConfMan.hasKey("gameCyclesPerSecond"))
		_gameCyclesPerSecond = ConfMan.getInt("gameCyclesPerSecond");
	if (ConfMan.hasKey("battleDiff"))
		_battleDiff = ConfMan.get("battleDiff");
	if (ConfMan.hasKey("validateXml"))
		_validateXml = ConfMan.getBool("validateXml");

	if (ConfMan.hasKey("spellEffectSpeed"))
		_spellEffectSpeed = ConfMan.getInt("spellEffectSpeed");
	if (ConfMan.hasKey("campTime"))
		_campTime = ConfMan.getInt("campTime");
	if (ConfMan.hasKey("innTime"))
		_innTime = ConfMan.getInt("innTime");
	if (ConfMan.hasKey("shrineTime"))
		_shrineTime = ConfMan.getInt("shrineTime");
	if (ConfMan.hasKey("shakeInterval"))
		_shakeInterval = ConfMan.getInt("shakeInterval");
	if (ConfMan.hasKey("titleSpeedRandom"))
		_titleSpeedRandom = ConfMan.getInt("titleSpeedRandom");
	if (ConfMan.hasKey("titleSpeedOther"))
		_titleSpeedOther = ConfMan.getInt("titleSpeedOther");

	// minor enhancement options
	if (ConfMan.hasKey("activePlayer"))
		_enhancementsOptions._activePlayer = ConfMan.getBool("activePlayer");
	if (ConfMan.hasKey("u5spellMixing"))
		_enhancementsOptions._u5spellMixing = ConfMan.getBool("u5spellMixing");
	if (ConfMan.hasKey("u5shrines"))
		_enhancementsOptions._u5shrines = ConfMan.getBool("u5shrines");
	if (ConfMan.hasKey("slimeDivides"))
		_enhancementsOptions._slimeDivides = ConfMan.getBool("slimeDivides");
	if (ConfMan.hasKey("gazerSpawnsInsects"))
		_enhancementsOptions._gazerSpawnsInsects = ConfMan.getBool("gazerSpawnsInsects");
	if (ConfMan.hasKey("textColorization"))
		_enhancementsOptions._textColorization = ConfMan.getBool("textColorization");
	if (ConfMan.hasKey("c64chestTraps"))
		_enhancementsOptions._c64chestTraps = ConfMan.getBool("c64chestTraps");
	if (ConfMan.hasKey("smartEnterKey"))
		_enhancementsOptions._smartEnterKey = ConfMan.getBool("smartEnterKey");

	// major enhancement options
	if (ConfMan.hasKey("peerShowsObjects"))
		_enhancementsOptions._peerShowsObjects = ConfMan.getBool("peerShowsObjects");
	if (ConfMan.hasKey("u5combat"))
		_enhancementsOptions._u5combat = ConfMan.getBool("u5combat");
	if (ConfMan.hasKey("innAlwaysCombat"))
		_innAlwaysCombat = ConfMan.getBool("innAlwaysCombat");
	if (ConfMan.hasKey("campingAlwaysCombat"))
		_campingAlwaysCombat = ConfMan.getBool("campingAlwaysCombat");

	// mouse options
	if (ConfMan.hasKey("mouseEnabled"))
		_mouseOptions.enabled = ConfMan.getBool("mouseEnabled");
	if (ConfMan.hasKey("logging"))
		_logging = ConfMan.get("logging");

	// graphics enhancements options
	if (ConfMan.hasKey("renderTileTransparency"))
		_enhancementsOptions._u4TileTransparencyHack = ConfMan.getBool("");
	if (ConfMan.hasKey("transparentTilePixelShadowOpacity"))
		_enhancementsOptions._u4TileTransparencyHackPixelShadowOpacity =
			ConfMan.getInt("transparentTilePixelShadowOpacity");
	if (ConfMan.hasKey("transparentTileShadowSize"))
		_enhancementsOptions._u4TrileTransparencyHackShadowBreadth =
		ConfMan.getInt("transparentTileShadowSize");


	eventTimerGranularity = (1000 / _gameCyclesPerSecond);
	return true;
}

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
