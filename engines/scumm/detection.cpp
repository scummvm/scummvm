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

#include "base/plugins.h"

#include "engines/metaengine.h"

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/list.h"
#include "common/md5.h"
#include "common/system.h"
#include "common/translation.h"

#include "audio/mididrv.h"

#include "scumm/detection.h"
#include "scumm/detection_tables.h"
#include "scumm/file.h"
#include "scumm/file_nes.h"
#include "scumm/scumm.h"

#pragma mark -
#pragma mark --- Detection code ---
#pragma mark -


// Various methods to help in core detection.
#include "scumm/detection_internal.h"


#pragma mark -
#pragma mark --- Plugin code ---
#pragma mark -

static const DebugChannelDef debugFlagList[] = {
		{Scumm::DEBUG_SCRIPTS, "SCRIPTS", "Track script execution"},
		{Scumm::DEBUG_OPCODES, "OPCODES", "Track opcode execution"},
		{Scumm::DEBUG_IMUSE, "IMUSE", "Track iMUSE events"},
		{Scumm::DEBUG_RESOURCE, "RESOURCE", "Track resource loading/management"},
		{Scumm::DEBUG_VARS, "VARS", "Track variable changes",},
		{Scumm::DEBUG_ACTORS, "ACTORS", "Actor-related debug"},
		{Scumm::DEBUG_SOUND, "SOUND", "Sound related debug"},
		{Scumm::DEBUG_INSANE, "INSANE", "Track INSANE"},
		{Scumm::DEBUG_SMUSH, "SMUSH", "Track SMUSH"},
		{Scumm::DEBUG_MOONBASE_AI, "MOONBASEAI", "Track Moonbase AI"},
		{Scumm::DEBUG_NETWORK, "NETWORK", "Track Networking"},
		DEBUG_CHANNEL_END
};

using namespace Scumm;

class ScummMetaEngineDetection : public MetaEngineDetection {
public:
	const char *getName() const override {
		return "scumm";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	const char *getEngineName() const override;
	const char *getOriginalCopyright() const override;

	PlainGameList getSupportedGames() const override;
	PlainGameDescriptor findGame(const char *gameid) const override;
	DetectedGames detectGames(const Common::FSList &fslist, uint32 /*skipADFlags*/, bool /*skipIncomplete*/) override;

	uint getMD5Bytes() const override {
		 return 1024 * 1024;
	}
	int getGameVariantCount() const override {
		int entries = 0;
		for (const GameSettings *g = gameVariantsTable; g->gameid; ++g)
			++entries;
		return entries;
	}

	Common::String parseAndCustomizeGuiOptions(const Common::String &optionsString, const Common::String &domain) const override;
};

PlainGameList ScummMetaEngineDetection::getSupportedGames() const {
	return PlainGameList(gameDescriptions);
}

PlainGameDescriptor ScummMetaEngineDetection::findGame(const char *gameid) const {
	return Engines::findGameID(gameid, gameDescriptions, obsoleteGameIDsTable);
}

static Common::String generatePreferredTarget(const DetectorResult &x) {
	Common::String res(x.game.gameid);

	if (x.game.preferredTag) {
		res = res + "-" + x.game.preferredTag;
	}

	if (x.game.features & GF_DEMO) {
		res = res + "-demo";
	}

	// Append the platform, if a non-standard one has been specified.
	if (x.game.platform != Common::kPlatformDOS && x.game.platform != Common::kPlatformUnknown) {
		// HACK: For CoMI, it's pointless to encode the fact that it's for Windows.
		if (x.game.id != GID_CMI)
			res = res + "-" + Common::getPlatformAbbrev(x.game.platform);
	}

	// Append the language, if a non-standard one has been specified.
	if (x.language != Common::EN_ANY && x.language != Common::UNK_LANG) {
		res = res + "-" + Common::getLanguageCode(x.language);
	}

	return res;
}

DetectedGames ScummMetaEngineDetection::detectGames(const Common::FSList &fslist, uint32 /*skipADFlags*/, bool /*skipIncomplete*/) {
	DetectedGames detectedGames;
	Common::List<DetectorResult> results;
	::detectGames(fslist, results, nullptr);

	for (Common::List<DetectorResult>::iterator
	          x = results.begin(); x != results.end(); ++x) {
		const PlainGameDescriptor *g = findPlainGameDescriptor(x->game.gameid, gameDescriptions);
		assert(g);

		DetectedGame game = DetectedGame(getName(), x->game.gameid, g->description, x->language, x->game.platform, x->extra);

		// Compute and set the preferred target name for this game.
		// Based on generateComplexID() in advancedDetector.cpp.
		game.preferredTarget = generatePreferredTarget(*x);

		game.setGUIOptions(x->game.guioptions + MidiDriver::musicType2GUIO(x->game.midi));
		game.appendGUIOptions(getGameGUIOptionsDescriptionLanguage(x->language));

		detectedGames.push_back(game);
	}

	return detectedGames;
}

const char *ScummMetaEngineDetection::getEngineName() const {
	return "SCUMM ["

#if defined(ENABLE_SCUMM_7_8) && defined(ENABLE_HE)
		"all games"
#else

		"v0-v6 games"

#if defined(ENABLE_SCUMM_7_8)
		", v7 & v8 games"
#endif
#if defined(ENABLE_HE)
		", HE71+ games"
#endif

#endif
		"]";
}

const char *ScummMetaEngineDetection::getOriginalCopyright() const {
	return "LucasArts SCUMM Games (C) LucasArts\n"
	       "Humongous SCUMM Games (C) Humongous";
}

Common::String ScummMetaEngineDetection::parseAndCustomizeGuiOptions(const Common::String &optionsString, const Common::String &domain) const {
	Common::String result = MetaEngineDetection::parseAndCustomizeGuiOptions(optionsString, domain);
	const char *defaultRenderOption = nullptr;

	const Common::Platform platform = Common::parsePlatform(ConfMan.get("platform", domain));
	const Common::String extra = ConfMan.get("extra", domain);

	// Add default rendermode option for target. We don't put the default mode into the
	// detection tables, due to the amount of targets we have. It it more convenient to
	// add the option here.
	switch (platform) {
	case Common::kPlatformAmiga:
		defaultRenderOption = GUIO_RENDERAMIGA;
		break;
	case Common::kPlatformApple2GS:
		defaultRenderOption = GUIO_RENDERAPPLE2GS;
		break;
	case Common::kPlatformMacintosh:
		defaultRenderOption = GUIO_RENDERMACINTOSH;
		break;
	case Common::kPlatformFMTowns:
		defaultRenderOption = GUIO_RENDERFMTOWNS;
		break;
	case Common::kPlatformAtariST:
		defaultRenderOption = GUIO_RENDERATARIST;
		break;
	case Common::kPlatformDOS:
		defaultRenderOption = (extra.equalsIgnoreCase("EGA") || extra.equalsIgnoreCase("V1") || extra.equalsIgnoreCase("V2")) ? GUIO_RENDEREGA : GUIO_RENDERVGA;
		break;
	case Common::kPlatformUnknown:
		// For targets that don't specify the platform (often happens with SCUMM6+ games) we stick with default VGA.
		defaultRenderOption = GUIO_RENDERVGA;
		break;
	default:
		// Leave this as nullptr for platforms that don't have a specific render option (SegaCD, NES, ...).
		// These targets will then have the full set of render mode options in the launcher options dialog.
		break;
	}

	// If the render option is already part of the string (specified in the
	// detection tables) we don't add it again.
	if (defaultRenderOption != nullptr && !result.contains(defaultRenderOption))
		result += defaultRenderOption;

	return result;
}

REGISTER_PLUGIN_STATIC(SCUMM_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, ScummMetaEngineDetection);
