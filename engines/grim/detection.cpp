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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/advancedDetector.h"
#include "grim/debug.h"
#include "grim/detection.h"
#include "grim/detection_tables.h"

static const DebugChannelDef debugFlagList[] = {
	{Grim::Debug::Info, "info", ""},
	{Grim::Debug::Warning, "warning", ""},
	{Grim::Debug::Error, "error", ""},
	{Grim::Debug::Engine, "engine", ""},
	{Grim::Debug::Lua, "lua", ""},
	{Grim::Debug::Bitmaps, "bitmaps", ""},
	{Grim::Debug::Models, "models", ""},
	{Grim::Debug::Actors, "actors", ""},
	{Grim::Debug::Costumes, "costumes", ""},
	{Grim::Debug::Chores, "chores", ""},
	{Grim::Debug::Fonts, "fonts", ""},
	{Grim::Debug::Keyframes, "keyframes", ""},
	{Grim::Debug::Movie, "movie", ""},
	{Grim::Debug::Sound, "sound", ""},
	{Grim::Debug::Scripts, "scripts", ""},
	{Grim::Debug::Sets, "sets", ""},
	{Grim::Debug::TextObjects, "textobjects", ""},
	{Grim::Debug::Patchr, "patchr", ""},
	{Grim::Debug::Lipsync, "lipsync", ""},
	{Grim::Debug::Sprites, "sprites", ""},
	DEBUG_CHANNEL_END
};

namespace Grim {

static const PlainGameDescriptor grimGames[] = {
	{"grim", "Grim Fandango"},
	{"monkey4", "Escape From Monkey Island"},
	{nullptr, nullptr}
};

class GrimMetaEngineDetection : public AdvancedMetaEngineDetection<Grim::GrimGameDescription> {
public:
	GrimMetaEngineDetection() : AdvancedMetaEngineDetection(Grim::gameDescriptions, grimGames) {
		_guiOptions = GUIO_NOMIDI;
		_flags |= kADFlagCanTranscodeTraditionalChineseToSimplified;
	}

	PlainGameDescriptor findGame(const char *gameid) const override {
		return Engines::findGameID(gameid, _gameIds, obsoleteGameIDsTable);
	}

	Common::Error identifyGame(DetectedGame &game, const void **descriptor) override {
		Engines::upgradeTargetIfNecessary(obsoleteGameIDsTable);
		return AdvancedMetaEngineDetection::identifyGame(game, descriptor);
	}

	const char *getEngineName() const override {
		return "Grim";
	}

	const char *getName() const override {
		return "grim";
	}

	const char *getOriginalCopyright() const override {
		return "LucasArts GrimE Games (C) LucasArts";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	DetectedGame toDetectedGame(const ADDetectedGame &adGame, ADDetectedGameExtraInfo *extraInfo) const override {
		DetectedGame game = AdvancedMetaEngineDetection::toDetectedGame(adGame, extraInfo);
		GrimGameType gameID = reinterpret_cast<const GrimGameDescription *>(adGame.desc)->gameType;

		if (gameID == GType_MONKEY4 && adGame.desc->language == Common::Language::ZH_TWN) {
			game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(Common::ZH_TWN));
			game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(Common::ZH_CHN));
		}

		return game;
	}
};

} // End of namespace Grim


REGISTER_PLUGIN_STATIC(GRIM_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Grim::GrimMetaEngineDetection);
