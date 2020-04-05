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

/*
 * Initialize static members
 */
Settings *Settings::_instance = NULL;

bool SettingsEnhancementOptions::operator==(const SettingsEnhancementOptions &s) const {
	return _activePlayer == s._activePlayer
		&& _u5spellMixing == s._u5spellMixing
		&& _u5shrines == s._u5shrines
		&& _u5combat == s._u5combat
		&& _slimeDivides == s._slimeDivides
		&& _gazerSpawnsInsects == s._gazerSpawnsInsects
		&& _textColorization == s._textColorization
		&& _c64chestTraps == s._c64chestTraps
		&& _smartEnterKey == s._smartEnterKey
		&& _peerShowsObjects == s._peerShowsObjects
		&& _u4TileTransparencyHack == s._u4TileTransparencyHack
		&& _u4TileTransparencyHackPixelShadowOpacity == s._u4TileTransparencyHackPixelShadowOpacity
		&& _u4TrileTransparencyHackShadowBreadth == s._u4TrileTransparencyHackShadowBreadth;
}

/*-------------------------------------------------------------------*/

bool SettingsData::operator==(const SettingsData &s) const {
	return _battleSpeed == s._battleSpeed
		&& _campingAlwaysCombat == s._campingAlwaysCombat
		&& _campTime == s._campTime
		&& _debug == s._debug
		&& _enhancements == s._enhancements
		&& _enhancementsOptions == s._enhancementsOptions
		&& _filterMoveMessages == s._filterMoveMessages
		&& _gameCyclesPerSecond == s._gameCyclesPerSecond
		&& _screenAnimationFramesPerSecond == s._screenAnimationFramesPerSecond
		&& _innAlwaysCombat == s._innAlwaysCombat
		&& _innTime == s._innTime
		&& _keydelay == s._keydelay
		&& _keyinterval == s._keyinterval
		&& _mouseOptions == s._mouseOptions
		&& _screenShakes == s._screenShakes
		&& _gamma == s._gamma
		&& _shakeInterval == s._shakeInterval
		&& _shortcutCommands == s._shortcutCommands
		&& _shrineTime == s._shrineTime
		&& _spellEffectSpeed == s._spellEffectSpeed
		&& _validateXml == s._validateXml
		&& _volumeFades == s._volumeFades
		&& _titleSpeedRandom == s._titleSpeedRandom
		&& _titleSpeedOther == s._titleSpeedOther;
}

bool SettingsData::operator!=(const SettingsData &s) const {
	return !operator==(s);
}

/*-------------------------------------------------------------------*/

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
	// default settings
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
	_pauseForEachTurn      = DEFAULT_PAUSE_FOR_EACH_TURN;

	// all specific minor enhancements default to "on", any major enhancements default to "off"
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

	// mouse defaults to on
	_mouseOptions._enabled = 1;

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
		_mouseOptions._enabled = ConfMan.getBool("mouseEnabled");
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


	_eventTimerGranularity = (1000 / _gameCyclesPerSecond);
	return true;
}

bool Settings::write() {
	ConfMan.set("gemLayout", _gemLayout);
	ConfMan.set("lineOfSight", _lineOfSight);
	ConfMan.setBool("screenShakes", _screenShakes);
	ConfMan.setInt("gamma", _gamma);

	ConfMan.setBool("volumeFades", _volumeFades);
	ConfMan.setBool("shortcutCommands", _shortcutCommands);
	ConfMan.setInt("keydelay", _keydelay);
	ConfMan.setBool("filterMoveMessages", _filterMoveMessages);
	ConfMan.setInt("battlespeed", _battleSpeed);
	ConfMan.setBool("enhancements", _enhancements);
	ConfMan.setInt("gameCyclesPerSecond", _gameCyclesPerSecond);
	ConfMan.set("battleDiff", _battleDiff);
	ConfMan.setBool("validateXml", _validateXml);

	ConfMan.setInt("spellEffectSpeed", _spellEffectSpeed);
	ConfMan.setInt("campTime", _campTime);
	ConfMan.setInt("innTime", _innTime);
	ConfMan.setInt("shrineTime", _shrineTime);
	ConfMan.setInt("shakeInterval", _shakeInterval);
	ConfMan.setInt("titleSpeedRandom", _titleSpeedRandom);
	ConfMan.setInt("titleSpeedOther", _titleSpeedOther);

	ConfMan.setBool("activePlayer", _enhancementsOptions._activePlayer);
	ConfMan.setBool("u5spellMixing", _enhancementsOptions._u5spellMixing);
	ConfMan.setBool("u5shrines", _enhancementsOptions._u5shrines);
	ConfMan.setBool("slimeDivides", _enhancementsOptions._slimeDivides);
	ConfMan.setBool("gazerSpawnsInsects", _enhancementsOptions._gazerSpawnsInsects);
	ConfMan.setBool("textColorization", _enhancementsOptions._textColorization);
	ConfMan.setBool("c64chestTraps", _enhancementsOptions._c64chestTraps);
	ConfMan.setBool("smartEnterKey", _enhancementsOptions._smartEnterKey);

	ConfMan.setBool("peerShowsObjects", _enhancementsOptions._peerShowsObjects);
	ConfMan.setBool("u5combat", _enhancementsOptions._u5combat);
	ConfMan.setBool("innAlwaysCombat", _innAlwaysCombat);
	ConfMan.setBool("campingAlwaysCombat", _campingAlwaysCombat);

	ConfMan.setBool("mouseEnabled", _mouseOptions._enabled);
	ConfMan.set("logging", _logging);

	ConfMan.setBool("renderTileTransparency",
		_enhancementsOptions._u4TileTransparencyHack);
	ConfMan.setInt("transparentTilePixelShadowOpacity",
		_enhancementsOptions._u4TileTransparencyHackPixelShadowOpacity);
	ConfMan.setInt("transparentTileShadowSize",
		_enhancementsOptions._u4TrileTransparencyHackShadowBreadth);

	ConfMan.flushToDisk();

	setChanged();
	notifyObservers(nullptr);

	return true;
}

const Std::vector<Common::String> &Settings::getBattleDiffs() {
	return _battleDiffs;
}

} // End of namespace Ultima4
} // End of namespace Ultima
