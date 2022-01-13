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

	const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const override;
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

static const ExtraGuiOption comiObjectLabelsOption = {
	_s("Show Object Line"),
	_s("Show the names of objects at the bottom of the screen"),
	"object_labels",
	true,
	0,
	0
};

static const ExtraGuiOption mmnesObjectLabelsOption = {
	_s("Use NES Classic Palette"),
	_s("Use a more neutral color palette that closely emulates the NES Classic"),
	"mm_nes_classic_palette",
	false,
	0,
	0
};

static const ExtraGuiOption fmtownsTrimTo200 = {
	_s("Trim FM-TOWNS games to 200 pixels height"),
	_s("Cut the extra 40 pixels at the bottom of the screen, to make it standard 200 pixels height, allowing using 'aspect ratio correction'"),
	"trim_fmtowns_to_200_pixels",
	false,
	0,
	0
};

static const ExtraGuiOption macV3LowQualityMusic = {
	_s("Play simplified music"),
	_s("This music was presumably intended for low-end Macs, and uses only one channel."),
	"mac_v3_low_quality_music",
	false,
	0,
	0
};

static const ExtraGuiOption smoothScrolling = {
	_s("Enable smooth scrolling"),
	_s("(instead of the normal 8-pixels steps scrolling)"),
	"smooth_scroll",
	true,
	0,
	1
};

static const ExtraGuiOption semiSmoothScrolling = {
	_s("Allow semi-smooth scrolling"),
	_s("Allow scrolling to be less smooth during the fast camera movement in the intro."),
	"semi_smooth_scroll",
	false,
	1,
	0
};

static const ExtraGuiOption enableEnhancements {
	_s("Enable game-specific enhancements"),
	_s("Allow ScummVM to make small enhancements to the game, usually based on other versions of the same game."),
	"enable_enhancements",
	true,
	0,
	0
};

static const ExtraGuiOption audioOverride {
	_s("Load modded audio"),
	_s("Replace music, sound effects, and speech clips with modded audio files, if available."),
	"audio_override",
	true,
	0,
	0
};

const ExtraGuiOptions ScummMetaEngineDetection::getExtraGuiOptions(const Common::String &target) const {
	ExtraGuiOptions options;
	// Query the GUI options
	const Common::String guiOptionsString = ConfMan.get("guioptions", target);
	const Common::String gameid = ConfMan.get("gameid", target);
	const Common::String extra = ConfMan.get("extra", target);
	const Common::String guiOptions = parseGameGUIOptions(guiOptionsString);
	const Common::Platform platform = Common::parsePlatform(ConfMan.get("platform", target));

	if (target.empty() || guiOptions.contains(GUIO_ENHANCEMENTS)) {
		options.push_back(enableEnhancements);
	}
	if (target.empty() || guiOptions.contains(GUIO_AUDIO_OVERRIDE)) {
		options.push_back(audioOverride);
	}
	if (target.empty() || gameid == "comi") {
		options.push_back(comiObjectLabelsOption);
	}
	if (target.empty() || platform == Common::kPlatformNES) {
		options.push_back(mmnesObjectLabelsOption);
	}
	if (target.empty() || platform == Common::kPlatformFMTowns) {
		options.push_back(smoothScrolling);
		if (target.empty() || gameid == "loom")
			options.push_back(semiSmoothScrolling);
		if (guiOptions.contains(GUIO_TRIM_FMTOWNS_TO_200_PIXELS))
			options.push_back(fmtownsTrimTo200);
	}

	// The Steam Mac versions of Loom and Indy 3 are more akin to the VGA
	// DOS versions, and that's how ScummVM usually sees them. But that
	// rebranding does not happen until later.

	// The low quality music in Loom was probably intended for low-end
	// Macs. It plays only one channel, instead of three.

	if (target.empty() || (gameid == "loom" && platform == Common::kPlatformMacintosh && extra != "Steam")) {
		options.push_back(macV3LowQualityMusic);
	}

	return options;
}

REGISTER_PLUGIN_STATIC(SCUMM_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, ScummMetaEngineDetection);
