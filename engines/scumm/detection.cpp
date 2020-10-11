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

#pragma mark -
#pragma mark --- Detection code ---
#pragma mark -


// Various methods to help in core detection.
#include "scumm/detection_internal.h"


#pragma mark -
#pragma mark --- Plugin code ---
#pragma mark -


using namespace Scumm;

class ScummMetaEngineDetection : public MetaEngineDetection {
public:
	const char *getEngineId() const override {
		return "scumm";
	}

	const char *getName() const override;
	const char *getOriginalCopyright() const override;

	PlainGameList getSupportedGames() const override;
	PlainGameDescriptor findGame(const char *gameid) const override;
	DetectedGames detectGames(const Common::FSList &fslist) const override;

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

DetectedGames ScummMetaEngineDetection::detectGames(const Common::FSList &fslist) const {
	DetectedGames detectedGames;
	Common::List<DetectorResult> results;
	::detectGames(fslist, results, 0);

	for (Common::List<DetectorResult>::iterator
	          x = results.begin(); x != results.end(); ++x) {
		const PlainGameDescriptor *g = findPlainGameDescriptor(x->game.gameid, gameDescriptions);
		assert(g);

		DetectedGame game = DetectedGame(getEngineId(), x->game.gameid, g->description, x->language, x->game.platform, x->extra);

		// Compute and set the preferred target name for this game.
		// Based on generateComplexID() in advancedDetector.cpp.
		game.preferredTarget = generatePreferredTarget(*x);

		game.setGUIOptions(x->game.guioptions + MidiDriver::musicType2GUIO(x->game.midi));
		game.appendGUIOptions(getGameGUIOptionsDescriptionLanguage(x->language));

		detectedGames.push_back(game);
	}

	return detectedGames;
}

const char *ScummMetaEngineDetection::getName() const {
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
	true
};

static const ExtraGuiOption mmnesObjectLabelsOption = {
	_s("Use NES Classic Palette"),
	_s("Use a more neutral color palette that closely emulates the NES Classic"),
	"mm_nes_classic_palette",
	false
};

const ExtraGuiOptions ScummMetaEngineDetection::getExtraGuiOptions(const Common::String &target) const {
	ExtraGuiOptions options;
	if (target.empty() || ConfMan.get("gameid", target) == "comi") {
		options.push_back(comiObjectLabelsOption);
	}
	if (target.empty() || Common::parsePlatform(ConfMan.get("platform", target)) == Common::kPlatformNES) {
		options.push_back(mmnesObjectLabelsOption);
	}
	return options;
}

REGISTER_PLUGIN_STATIC(SCUMM_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, ScummMetaEngineDetection);
