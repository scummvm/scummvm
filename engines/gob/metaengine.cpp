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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#include "engines/advancedDetector.h"

#include "common/config-manager.h"
#include "common/hashmap.h"

#include "common/translation.h"

#include "gui/chooser.h"
#include "gui/message.h"
#include "gui/unknown-game-dialog.h"

#include "gob/gameidtotype.h"
#include "gob/gob.h"

// For struct GOBGameDescription.
#include "gob/detection/detection.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_COPY_PROTECTION,
                {
                        _s("Enable copy protection"),
                        _s("Enable any copy protection that would otherwise be bypassed by default."),
                        "copy_protection",
                        false,
                        0,
                        0
                },
        },
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class GobMetaEngine : public AdvancedMetaEngine<Gob::GOBGameDescription> {
public:
	const char *getName() const override {
		return "gob";
	}

	bool hasFeature(MetaEngineFeature f) const override;

	Common::Error createInstance(OSystem *syst, Engine **engine, const Gob::GOBGameDescription *desc) const override;

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return optionsList;
	}
};

bool GobMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

bool Gob::GobEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher);
}

Common::Error GobMetaEngine::createInstance(OSystem *syst, Engine **engine, const Gob::GOBGameDescription *gd) const {

	Gob::GobEngine *gobEngine = new Gob::GobEngine(syst);
	*engine = gobEngine;
	gobEngine->initGame(gd);
	Common::ErrorCode errorCode = Common::kNoError;

	if (gobEngine->gameTypeHasAddOns())
		errorCode = gobEngine->updateAddOns(this, gd);

	return errorCode;
}


#if PLUGIN_ENABLED_DYNAMIC(GOB)
	REGISTER_PLUGIN_DYNAMIC(GOB, PLUGIN_TYPE_ENGINE, GobMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GOB, PLUGIN_TYPE_ENGINE, GobMetaEngine);
#endif

namespace Gob {

GameType GobEngine::getGameType(const char *gameId) const {
	const GameIdToType *gameInfo = gameIdToType;

	while (gameInfo->gameId != nullptr) {
		if (!strcmp(gameId, gameInfo->gameId))
			return gameInfo->gameType;
		gameInfo++;
	}

	error("Unknown game ID: %s", gameId);
}

bool GobEngine::gameTypeHasAddOns() {
	return  getGameType() == kGameTypeAdibou1 ||
			getGameType() == kGameTypeAdibou2 ||
			getGameType() == kGameTypeAdi2 ||
			getGameType() == kGameTypeAdi4;
}


// Accelerator, to discard some directories we know have no chance to be add-ons
bool GobEngine::dirCanBeGameAddOn(Common::FSDirectory dir) const {
	if (getGameType() == kGameTypeAdibou2)
		return dir.hasFile("intro_ap.stk");

	return true;
}

// To display a warning if a directory likely to be an add-on does not match anything
bool GobEngine::dirMustBeGameAddOn(Common::FSDirectory dir) const {
	if (getGameType() == kGameTypeAdibou2)
		return dir.hasFile("intro_ap.stk");

	return false;
}

Common::ErrorCode GobEngine::updateAddOns(const GobMetaEngine *metaEngine, const GOBGameDescription *gd) const {
	const Plugin *detectionPlugin = EngineMan.findDetectionPlugin(metaEngine->getName());
	if (!detectionPlugin) {
		warning("Engine plugin for GOB not present. Add-ons detection is disabled");
		return Common::kNoError;
	}

	// Update silently the targets associated with the add-ons, unless some unsupported version is detected

	// List already registered add-ons for this game, and detect removed ones
	Common::ConfigManager::DomainMap::iterator iter = ConfMan.beginGameDomains();
	Common::HashMap<Common::Path, Common::String, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> existingAddOnsPaths;

	bool anyAddOnRemoved = false;
	for (; iter != ConfMan.endGameDomains(); ++iter) {
		Common::String name(iter->_key);
		Common::ConfigManager::Domain &dom = iter->_value;

		Common::String parent;
		if (dom.tryGetVal("parent", parent) && parent == ConfMan.getActiveDomainName()) {
			// Existing add-on, check if its path still exists
			Common::Path addOnPath(Common::Path::fromConfig(dom.getVal("path")));
			if (addOnPath.empty() || !Common::FSNode(addOnPath).isDirectory()) {
				// Path does not exist, remove the add-on
				debug("Removing entry of deleted add-on '%s' (former path: '%s')",
					  name.c_str(),
					  addOnPath.toString(Common::Path::kNativeSeparator).c_str());

				ConfMan.removeGameDomain(name);
				anyAddOnRemoved = true;
			} else {
				existingAddOnsPaths[addOnPath] = name;
			}
		}
	}

	if (anyAddOnRemoved)
		ConfMan.flushToDisk();

	// Look for newly added add-ons
	bool anyAddOnAdded = false;
	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	Common::FSList subdirNodes;
	gameDataDir.getChildren(subdirNodes, Common::FSNode::kListDirectoriesOnly);
	for (const Common::FSNode &subdirNode : subdirNodes) {
		Common::FSDirectory subdir(subdirNode);
		if (dirCanBeGameAddOn(subdir)) {
			Common::FSList files;
			if (!subdirNode.getChildren(files, Common::FSNode::kListAll))
				continue;

			ADCacheMan.clear();

			DetectedGames detectedGames = detectionPlugin->get<MetaEngineDetection>().detectGames(files);
			DetectedGames detectedAddOns;
			for (DetectedGame &game : detectedGames) {
				if (game.isAddOn) {
					detectedAddOns.push_back(game);
				}
			}

			int idx = 0;
			if (detectedAddOns.empty() && dirMustBeGameAddOn(subdir)) {
				Common::U32String msgFormat(_("The directory '%s' looks like an add-on for the game '%s', but ScummVM could not find any matching add-on in it."));
				Common::U32String msg = Common::U32String::format(msgFormat,
																  subdirNode.getPath().toString(Common::Path::kNativeSeparator).c_str(),
																  gd->desc.gameId);

				GUI::MessageDialog alert(msg);
				alert.runModal();
				continue;
			} else if (detectedAddOns.size() == 1) {
				// Exact match
				idx = 0;
			} else {
				// Display the candidates to the user and let her/him pick one
				Common::U32StringArray list;
				for (idx = 0; idx < (int)detectedAddOns.size(); idx++) {
					Common::U32String description = detectedAddOns[idx].description;

					if (detectedAddOns[idx].hasUnknownFiles) {
						description += Common::U32String(" - ");
						// Unknown game variant
						description += _("Unknown variant");
					}

					list.push_back(description);
				}

				Common::U32String msgFormat(_("Directory '%s' matches several add-ons, please pick one."));
				Common::U32String msg = Common::U32String::format(msgFormat,
																  subdirNode.getPath().toString(Common::Path::kNativeSeparator).c_str());

				GUI::ChooserDialog dialog(msg);
				dialog.setList(list);
				idx = dialog.runModal();
				if (idx < 0)
					return Common::kUserCanceled;
			}

			if (0 <= idx && idx < (int)detectedAddOns.size()) {
				DetectedGame &selectedAddOn = detectedAddOns[idx];
				selectedAddOn.path = subdirNode.getPath();
				selectedAddOn.shortPath = subdirNode.getDisplayName();

				if (selectedAddOn.hasUnknownFiles) {
					debug("Detected an unknown variant of add-on '%s' (path: '%s')",
						  selectedAddOn.gameId.c_str(),
						  subdirNode.getPath().toString(Common::Path::kNativeSeparator).c_str());
					GUI::UnknownGameDialog dialog(selectedAddOn);
					dialog.runModal();
					continue; // Do not create an entry for unknown variants
				}

				if (selectedAddOn.gameSupportLevel != kStableGame) {
					if (!warnUserAboutUnsupportedAddOn(selectedAddOn.description)) {
						return Common::kUserCanceled;
					}
				}

				Common::String addOnName;
				if (existingAddOnsPaths.tryGetVal(subdirNode.getPath(), addOnName)) {
					debug("Detected existing add-on '%s' (path: '%s')",
						  addOnName.c_str(),
						  subdirNode.getPath().toString(Common::Path::kNativeSeparator).c_str());
				} else {
					Common::String domain = EngineMan.createTargetForGame(selectedAddOn);
					debug("Detected new add-on '%s' (path: '%s')",
						  domain.c_str(),
						  subdirNode.getPath().toString(Common::Path::kNativeSeparator).c_str());
					ConfMan.set("parent", ConfMan.getActiveDomainName(), domain);
					anyAddOnAdded = true;
				}
			}
		}
	}

	if (anyAddOnAdded)
		ConfMan.flushToDisk();

	return Common::kNoError;
}

void GobEngine::initGame(const GOBGameDescription *gd) {
	if (gd->startTotBase == nullptr)
		_startTot = "intro.tot";
	else
		_startTot = gd->startTotBase;

	if (gd->startStkBase == nullptr)
		_startStk = "intro.stk";
	else
		_startStk = gd->startStkBase;

	_demoIndex = gd->demoIndex;

	_gameType = getGameType(gd->desc.gameId);
	_features = gd->features;
	_language = gd->desc.language;
	_platform = gd->desc.platform;
	_extra = gd->desc.extra;

	_enableAdibou2FreeBananasWorkaround = gd->desc.flags & GF_ENABLE_ADIBOU2_FREE_BANANAS_WORKAROUND;
	_enableAdibou2FlowersInfiniteLoopWorkaround = gd->desc.flags & GF_ENABLE_ADIBOU2_FLOWERS_INFINITE_LOOP_WORKAROUND;
}

} // End of namespace Gob

