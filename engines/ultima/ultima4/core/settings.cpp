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
#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/core/utils.h"
#include "common/file.h"

namespace Ultima {
namespace Ultima4 {

/*
 * Initialize static members
 */
Settings *Settings::_instance = nullptr;

bool SettingsEnhancementOptions::operator==(const SettingsEnhancementOptions &s) const {
	return _activePlayer == s._activePlayer
		&& _u5SpellMixing == s._u5SpellMixing
		&& _u5Shrines == s._u5Shrines
		&& _u5Combat == s._u5Combat
		&& _slimeDivides == s._slimeDivides
		&& _gazerSpawnsInsects == s._gazerSpawnsInsects
		&& _textColorization == s._textColorization
		&& _c64ChestTraps == s._c64ChestTraps
		&& _smartEnterKey == s._smartEnterKey
		&& _peerShowsObjects == s._peerShowsObjects
		&& _u4TileTransparencyHack == s._u4TileTransparencyHack
		&& _u4TileTransparencyHackPixelShadowOpacity == s._u4TileTransparencyHackPixelShadowOpacity
		&& _u4TrileTransparencyHackShadowBreadth == s._u4TrileTransparencyHackShadowBreadth;
}

/*-------------------------------------------------------------------*/

bool SettingsData::operator==(const SettingsData &s) const {
	return _videoType == s._videoType
		&& _battleSpeed == s._battleSpeed
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
	// Synchronize settings
	Shared::ConfSerializer s(false);
	synchronize(s);

	// Set up various other constants that aren't configurable
	_game = "Ultima IV";
	_debug = gDebugLevel > 0;

	_innAlwaysCombat = 0;
	_campingAlwaysCombat = 0;
	_screenAnimationFramesPerSecond = DEFAULT_ANIMATION_FRAMES_PER_SECOND;

	bool isEnhanced = _videoType != "EGA";
	_scale = isEnhanced ? 2 : 1;
	_filter = isEnhanced ? "Scale2x" : "point";
  
	_battleDiffs.push_back("Normal");
	_battleDiffs.push_back("Hard");
	_battleDiffs.push_back("Expert");

	_eventTimerGranularity = (1000 / _gameCyclesPerSecond);
}

Settings &Settings::getInstance() {
	if (_instance == nullptr)
		_instance = new Settings();
	return *_instance;
}

void Settings::setData(const SettingsData &data) {
	// bitwise copy is safe
	*(SettingsData *)this = data;

	bool isEnhanced = _videoType != "EGA";
	_scale = isEnhanced ? 2 : 1;
	_filter = isEnhanced ? "Scale2x" : "point";
}

bool Settings::write() {
	Shared::ConfSerializer s(true);
	synchronize(s);

	setChanged();
	notifyObservers(nullptr);

	return true;
}

void Settings::synchronize(Shared::ConfSerializer &s) {
	// General settings
	bool isEnhanced = g_ultima->isEnhanced();
	s.syncAsString("video", _videoType, isEnhanced ? "new" : "EGA");
	s.syncAsString("gemLayout", _gemLayout, DEFAULT_GEM_LAYOUT);
	s.syncAsString("lineOfSight", _lineOfSight, DEFAULT_LINEOFSIGHT);
	s.syncAsBool("screenShakes", _screenShakes, DEFAULT_SCREEN_SHAKES);
	s.syncAsInt("gamma", _gamma, DEFAULT_GAMMA);
	s.syncAsBool("volumeFades", _volumeFades, DEFAULT_VOLUME_FADES);
	s.syncAsBool("shortcutCommands", _shortcutCommands, DEFAULT_SHORTCUT_COMMANDS);
	s.syncAsBool("filterMoveMessages", _filterMoveMessages, DEFAULT_FILTER_MOVE_MESSAGES);
	s.syncAsInt("battlespeed", _battleSpeed, DEFAULT_BATTLE_SPEED);
	s.syncAsBool("enhancements", _enhancements, DEFAULT_ENHANCEMENTS);
	s.syncAsInt("gameCyclesPerSecond", _gameCyclesPerSecond, DEFAULT_CYCLES_PER_SECOND);
	s.syncAsString("battleDiff", _battleDiff, DEFAULT_BATTLE_DIFFICULTY);
	s.syncAsBool("validateXml", _validateXml, DEFAULT_VALIDATE_XML);
	s.syncAsInt("spellEffectSpeed", _spellEffectSpeed, DEFAULT_SPELL_EFFECT_SPEED);
	s.syncAsInt("campTime", _campTime, DEFAULT_CAMP_TIME);
	s.syncAsInt("innTime", _innTime, DEFAULT_INN_TIME);
	s.syncAsInt("shrineTime", _shrineTime, DEFAULT_SHRINE_TIME);
	s.syncAsInt("shakeInterval", _shakeInterval, DEFAULT_SHAKE_INTERVAL);
	s.syncAsInt("titleSpeedRandom", _titleSpeedRandom, DEFAULT_TITLE_SPEED_RANDOM);
	s.syncAsInt("titleSpeedOther", _titleSpeedOther, DEFAULT_TITLE_SPEED_OTHER);
	s.syncAsBool("innAlwaysCombat", _innAlwaysCombat, false);
	s.syncAsBool("campingAlwaysCombat", _campingAlwaysCombat, false);
	s.syncAsBool("u5spellMixing", _enhancementsOptions._u5SpellMixing, isEnhanced);

	// all specific minor enhancements default to "on", any major enhancements default to "off"
	// minor enhancement options
	s.syncAsBool("activePlayer", _enhancementsOptions._activePlayer, true);
	s.syncAsBool("u5shrines", _enhancementsOptions._u5Shrines, true);
	s.syncAsBool("slimeDivides", _enhancementsOptions._slimeDivides, true);
	s.syncAsBool("gazerSpawnsInsects", _enhancementsOptions._gazerSpawnsInsects, true);
	s.syncAsBool("textColorization", _enhancementsOptions._textColorization, false);
	s.syncAsBool("c64chestTraps", _enhancementsOptions._c64ChestTraps, true);
	s.syncAsBool("smartEnterKey", _enhancementsOptions._smartEnterKey, true);

	// major enhancement options
	s.syncAsBool("peerShowsObjects", _enhancementsOptions._peerShowsObjects, false);
	s.syncAsBool("u5combat", _enhancementsOptions._u5Combat, false);

	// graphics enhancements options
	s.syncAsBool("renderTileTransparency", _enhancementsOptions._u4TileTransparencyHack, true);
	s.syncAsInt("transparentTilePixelShadowOpacity", _enhancementsOptions._u4TileTransparencyHackPixelShadowOpacity, DEFAULT_SHADOW_PIXEL_OPACITY);
	s.syncAsInt("transparentTileShadowSize", _enhancementsOptions._u4TrileTransparencyHackShadowBreadth, DEFAULT_SHADOW_PIXEL_SIZE);

	// mouse options
	s.syncAsBool("mouseEnabled", _mouseOptions._enabled, true);
	s.syncAsString("logging", _logging, DEFAULT_LOGGING);
}

const Std::vector<Common::String> &Settings::getBattleDiffs() {
	return _battleDiffs;
}

} // End of namespace Ultima4
} // End of namespace Ultima
