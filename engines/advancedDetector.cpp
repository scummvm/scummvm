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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/debug.h"
#include "common/util.h"
#include "common/hash-str.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/md5.h"
#include "common/config-manager.h"
#include "common/textconsole.h"

#include "engines/advancedDetector.h"

/**
 * A list of pointers to ADGameDescription structs (or subclasses thereof).
 */
typedef Common::Array<const ADGameDescription*> ADGameDescList;


/**
 * Detect games in specified directory.
 * Parameters language and platform are used to pass on values
 * specified by the user. I.e. this is used to restrict search scope.
 *
 * @param fslist	FSList to scan or NULL for scanning all specified
 *  default directories.
 * @param params	a ADParams struct containing various parameters
 * @param language	restrict results to specified language only
 * @param platform	restrict results to specified platform only
 * @return	list of ADGameDescription (or subclass) pointers corresponding to matched games
 */
static ADGameDescList detectGame(const Common::FSList &fslist, const ADParams &params, Common::Language language, Common::Platform platform, const Common::String &extra);


/**
 * Returns list of targets supported by the engine.
 * Distinguishes engines with single ID
 */
static GameList gameIDList(const ADParams &params) {
	if (params.singleid != NULL) {
		GameList gl;

		const PlainGameDescriptor *g = params.list;
		while (g->gameid) {
			if (0 == scumm_stricmp(params.singleid, g->gameid)) {
				gl.push_back(GameDescriptor(g->gameid, g->description));

				return gl;
			}
			g++;
		}
		error("Engine %s doesn't have its singleid specified in ids list", params.singleid);
	}

	return GameList(params.list);
}

static void upgradeTargetIfNecessary(const ADParams &params) {
	if (params.obsoleteList == 0)
		return;

	Common::String gameid = ConfMan.get("gameid");

	for (const ADObsoleteGameID *o = params.obsoleteList; o->from; ++o) {
		if (gameid.equalsIgnoreCase(o->from)) {
			gameid = o->to;
			ConfMan.set("gameid", gameid);

			if (o->platform != Common::kPlatformUnknown)
				ConfMan.set("platform", Common::getPlatformCode(o->platform));

			warning("Target upgraded from %s to %s", o->from, o->to);

			// WORKAROUND: Fix for bug #1719463: "DETECTOR: Launching
			// undefined target adds launcher entry"
			if (ConfMan.hasKey("id_came_from_command_line")) {
				warning("Target came from command line. Skipping save");
			} else {
				ConfMan.flushToDisk();
			}
			break;
		}
	}
}

namespace AdvancedDetector {

GameDescriptor findGameID(
	const char *gameid,
	const PlainGameDescriptor *list,
	const ADObsoleteGameID *obsoleteList
	) {
	// First search the list of supported game IDs for a match.
	const PlainGameDescriptor *g = findPlainGameDescriptor(gameid, list);
	if (g)
		return GameDescriptor(*g);

	// If we didn't find the gameid in the main list, check if it
	// is an obsolete game id.
	if (obsoleteList != 0) {
		const ADObsoleteGameID *o = obsoleteList;
		while (o->from) {
			if (0 == scumm_stricmp(gameid, o->from)) {
				g = findPlainGameDescriptor(o->to, list);
				if (g && g->description)
					return GameDescriptor(gameid, "Obsolete game ID (" + Common::String(g->description) + ")");
				else
					return GameDescriptor(gameid, "Obsolete game ID");
			}
			o++;
		}
	}

	// No match found
	return GameDescriptor();
}

}	// End of namespace AdvancedDetector

static GameDescriptor toGameDescriptor(const ADGameDescription &g, const PlainGameDescriptor *sg) {
	const char *title = 0;
	const char *extra;

	if (g.flags & ADGF_USEEXTRAASTITLE) {
		title = g.extra;
		extra = "";
	} else {
		while (sg->gameid) {
			if (!scumm_stricmp(g.gameid, sg->gameid))
				title = sg->description;
			sg++;
		}

		extra = g.extra;
	}

	GameDescriptor gd(g.gameid, title, g.language, g.platform);
	gd.updateDesc(extra);
	return gd;
}

/**
 * Generate a preferred target value as
 *   GAMEID-PLAFORM-LANG
 * or (if ADGF_DEMO has been set)
 *   GAMEID-demo-PLAFORM-LANG
 */
static Common::String generatePreferredTarget(const Common::String &id, const ADGameDescription *desc) {
	Common::String res(id);

	if (desc->flags & ADGF_DEMO) {
		res = res + "-demo";
	}

	if (desc->flags & ADGF_CD) {
		res = res + "-cd";
	}

	if (desc->platform != Common::kPlatformPC && desc->platform != Common::kPlatformUnknown) {
		res = res + "-" + getPlatformAbbrev(desc->platform);
	}

	if (desc->language != Common::EN_ANY && desc->language != Common::UNK_LANG && !(desc->flags & ADGF_DROPLANGUAGE)) {
		res = res + "-" + getLanguageCode(desc->language);
	}

	return res;
}

static void updateGameDescriptor(GameDescriptor &desc, const ADGameDescription *realDesc, const ADParams &params) {
	if (params.singleid != NULL) {
		desc["preferredtarget"] = desc["gameid"];
		desc["gameid"] = params.singleid;
	}

	if (!(params.flags & kADFlagDontAugmentPreferredTarget)) {
		if (!desc.contains("preferredtarget"))
			desc["preferredtarget"] = desc["gameid"];

		desc["preferredtarget"] = generatePreferredTarget(desc["preferredtarget"], realDesc);
	}

	if (params.flags & kADFlagUseExtraAsHint)
		desc["extra"] = realDesc->extra;

	desc.setGUIOptions(realDesc->guioptions | params.guioptions);
	desc.appendGUIOptions(getGameGUIOptionsDescriptionLanguage(realDesc->language));

	if (realDesc->flags & ADGF_ADDENGLISH)
		desc.appendGUIOptions(getGameGUIOptionsDescriptionLanguage(Common::EN_ANY));
}

bool cleanupPirated(ADGameDescList &matched) {
	// OKay, now let's sense presense of pirated games
	if (!matched.empty()) {
		for (uint j = 0; j < matched.size();) {
			if (matched[j]->flags & ADGF_PIRATED)
				matched.remove_at(j);
			else
				++j;
		}

		// We ruled out all variants and now have nothing
		if (matched.empty()) {

			warning("Illegitimate copy of the game detected. We give no support in such cases %d", matched.size());

			return true;
		}
	}

	return false;
}


GameList AdvancedMetaEngine::detectGames(const Common::FSList &fslist) const {
	ADGameDescList matches = detectGame(fslist, params, Common::UNK_LANG, Common::kPlatformUnknown, "");
	GameList detectedGames;

	if (cleanupPirated(matches))
		return detectedGames;

	if (matches.empty()) {
		// Use fallback detector if there were no matches by other means
		const ADGameDescription *fallbackDesc = fallbackDetect(fslist);
		if (fallbackDesc != 0) {
			GameDescriptor desc(toGameDescriptor(*fallbackDesc, params.list));
			updateGameDescriptor(desc, fallbackDesc, params);
			detectedGames.push_back(desc);
		}
	} else {
		// Otherwise use the found matches
		for (uint i = 0; i < matches.size(); i++) {
			GameDescriptor desc(toGameDescriptor(*matches[i], params.list));
			updateGameDescriptor(desc, matches[i], params);
			detectedGames.push_back(desc);
		}
	}

	return detectedGames;
}

Common::Error AdvancedMetaEngine::createInstance(OSystem *syst, Engine **engine) const {
	assert(engine);
	upgradeTargetIfNecessary(params);

	const ADGameDescription *agdDesc = 0;
	Common::Language language = Common::UNK_LANG;
	Common::Platform platform = Common::kPlatformUnknown;
	Common::String extra;

	if (ConfMan.hasKey("language"))
		language = Common::parseLanguage(ConfMan.get("language"));
	if (ConfMan.hasKey("platform"))
		platform = Common::parsePlatform(ConfMan.get("platform"));
	if (params.flags & kADFlagUseExtraAsHint)
		if (ConfMan.hasKey("extra"))
			extra = ConfMan.get("extra");

	Common::String gameid = ConfMan.get("gameid");

	Common::String path;
	if (ConfMan.hasKey("path")) {
		path = ConfMan.get("path");
	} else {
		path = ".";

		// This situation may happen only when game was
		// launched from a command line with wrong target and
		// no path was provided.
		//
		// A dummy entry will get created and will keep game path
		// We mark this entry, so it will not be added to the
		// config file.
		//
		// Fixes bug #1544799
		ConfMan.setBool("autoadded", true);

		warning("No path was provided. Assuming the data files are in the current directory");
	}
	Common::FSNode dir(path);
	Common::FSList files;
	if (!dir.isDirectory() || !dir.getChildren(files, Common::FSNode::kListAll)) {
		warning("Game data path does not exist or is not a directory (%s)", path.c_str());
		return Common::kNoGameDataFoundError;
	}

	ADGameDescList matches = detectGame(files, params, language, platform, extra);

	if (cleanupPirated(matches))
		return Common::kNoGameDataFoundError;

	if (params.singleid == NULL) {
		for (uint i = 0; i < matches.size(); i++) {
			if (matches[i]->gameid == gameid) {
				agdDesc = matches[i];
				break;
			}
		}
	} else if (matches.size() > 0) {
		agdDesc = matches[0];
	}

	if (agdDesc == 0) {
		// Use fallback detector if there were no matches by other means
		agdDesc = fallbackDetect(files);
		if (agdDesc != 0) {
			// Seems we found a fallback match. But first perform a basic
			// sanity check: the gameid must match.
			if (params.singleid == NULL && agdDesc->gameid != gameid)
				agdDesc = 0;
		}
	}

	if (agdDesc == 0)
		return Common::kNoGameDataFoundError;

	// If the GUI options were updated, we catch this here and update them in the users config
	// file transparently.
	Common::String lang = getGameGUIOptionsDescriptionLanguage(agdDesc->language);
	if (agdDesc->flags & ADGF_ADDENGLISH)
		lang += " " + getGameGUIOptionsDescriptionLanguage(Common::EN_ANY);

	Common::updateGameGUIOptions(agdDesc->guioptions | params.guioptions, lang);


	debug(2, "Running %s", toGameDescriptor(*agdDesc, params.list).description().c_str());
	if (!createInstance(syst, engine, agdDesc))
		return Common::kNoGameDataFoundError;
	else
		return Common::kNoError;
}

struct SizeMD5 {
	int size;
	Common::String md5;
};

typedef Common::HashMap<Common::String, SizeMD5, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> SizeMD5Map;
typedef Common::HashMap<Common::String, Common::FSNode, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;

static void reportUnknown(const Common::FSNode &path, const SizeMD5Map &filesSizeMD5) {
	// TODO: This message should be cleaned up / made more specific.
	// For example, we should specify at least which engine triggered this.
	//
	// Might also be helpful to display the full path (for when this is used
	// from the mass detector).
	printf("The game in '%s' seems to be unknown.\n", path.getPath().c_str());
	printf("Please, report the following data to the Residual team along with name\n");
	printf("of the game you tried to add and its version/language/etc.:\n");

	for (SizeMD5Map::const_iterator file = filesSizeMD5.begin(); file != filesSizeMD5.end(); ++file)
		printf("  {\"%s\", 0, \"%s\", %d},\n", file->_key.c_str(), file->_value.md5.c_str(), file->_value.size);

	printf("\n");
}

static ADGameDescList detectGameFilebased(const FileMap &allFiles, const ADParams &params);

static void composeFileHashMap(const Common::FSList &fslist, FileMap &allFiles, int depth, const char * const *directoryGlobs) {
	if (depth <= 0)
		return;

	if (fslist.empty())
		return;

	// First we compose a hashmap of all files in fslist.
	// Includes nifty stuff like removing trailing dots and ignoring case.
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory()) {
			Common::FSList files;

			if (!directoryGlobs)
				continue;

			bool matched = false;
			for (const char * const *glob = directoryGlobs; *glob; glob++)
				if (file->getName().matchString(*glob, true)) {
					matched = true;
					break;
				}

			if (!matched)
				continue;

			if (!file->getChildren(files, Common::FSNode::kListAll))
				continue;

			composeFileHashMap(files, allFiles, depth - 1, directoryGlobs);
		}

		Common::String tstr = file->getName();

		// Strip any trailing dot
		if (tstr.lastChar() == '.')
			tstr.deleteLastChar();

		allFiles[tstr] = *file;	// Record the presence of this file
	}
}

static ADGameDescList detectGame(const Common::FSList &fslist, const ADParams &params, Common::Language language, Common::Platform platform, const Common::String &extra) {
	FileMap allFiles;
	SizeMD5Map filesSizeMD5;

	const ADGameFileDescription *fileDesc;
	const ADGameDescription *g;
	const byte *descPtr;

	if (fslist.empty())
		return ADGameDescList();
	Common::FSNode parent = fslist.begin()->getParent();
	debug(3, "Starting detection in dir '%s'", parent.getPath().c_str());

	// First we compose a hashmap of all files in fslist.
	// Includes nifty stuff like removing trailing dots and ignoring case.
	composeFileHashMap(fslist, allFiles, (params.depth == 0 ? 1 : params.depth), params.directoryGlobs);

	// Check which files are included in some ADGameDescription *and* present
	// in fslist. Compute MD5s and file sizes for these files.
	for (descPtr = params.descs; ((const ADGameDescription *)descPtr)->gameid != 0; descPtr += params.descItemSize) {
		g = (const ADGameDescription *)descPtr;

		for (fileDesc = g->filesDescriptions; fileDesc->fileName; fileDesc++) {
			Common::String fname = fileDesc->fileName;
			SizeMD5 tmp;

			if (filesSizeMD5.contains(fname))
				continue;

			// FIXME/TODO: We don't handle the case that a file is listed as a regular
			// file and as one with resource fork.

			if (g->flags & ADGF_MACRESFORK) {
				Common::MacResManager *macResMan = new Common::MacResManager();

				if (macResMan->open(parent, fname)) {
					tmp.md5 = macResMan->computeResForkMD5AsString(params.md5Bytes);
					tmp.size = macResMan->getResForkDataSize();
					debug(3, "> '%s': '%s'", fname.c_str(), tmp.md5.c_str());
					filesSizeMD5[fname] = tmp;
				}

				delete macResMan;
			} else {
				if (allFiles.contains(fname)) {
					debug(3, "+ %s", fname.c_str());

					Common::File testFile;

					if (testFile.open(allFiles[fname])) {
						tmp.size = (int32)testFile.size();
						tmp.md5 = Common::computeStreamMD5AsString(testFile, params.md5Bytes);
					} else {
						tmp.size = -1;
					}

					debug(3, "> '%s': '%s'", fname.c_str(), tmp.md5.c_str());
					filesSizeMD5[fname] = tmp;
				}
			}
		}
	}

	ADGameDescList matched;
	int maxFilesMatched = 0;
	bool gotAnyMatchesWithAllFiles = false;

	// MD5 based matching
	uint i;
	for (i = 0, descPtr = params.descs; ((const ADGameDescription *)descPtr)->gameid != 0; descPtr += params.descItemSize, ++i) {
		g = (const ADGameDescription *)descPtr;
		bool fileMissing = false;

		// Do not even bother to look at entries which do not have matching
		// language and platform (if specified).
		if ((language != Common::UNK_LANG && g->language != Common::UNK_LANG && g->language != language
			 && !(language == Common::EN_ANY && (g->flags & ADGF_ADDENGLISH))) ||
			(platform != Common::kPlatformUnknown && g->platform != Common::kPlatformUnknown && g->platform != platform)) {
			continue;
		}

		if ((params.flags & kADFlagUseExtraAsHint) && !extra.empty() && g->extra != extra)
			continue;

		bool allFilesPresent = true;
		int curFilesMatched = 0;

		// Try to match all files for this game
		for (fileDesc = g->filesDescriptions; fileDesc->fileName; fileDesc++) {
			Common::String tstr = fileDesc->fileName;

			if (!filesSizeMD5.contains(tstr)) {
				fileMissing = true;
				allFilesPresent = false;
				break;
			}

			if (fileDesc->md5 != NULL && fileDesc->md5 != filesSizeMD5[tstr].md5) {
				debug(3, "MD5 Mismatch. Skipping (%s) (%s)", fileDesc->md5, filesSizeMD5[tstr].md5.c_str());
				fileMissing = true;
				break;
			}

			if (fileDesc->fileSize != -1 && fileDesc->fileSize != filesSizeMD5[tstr].size) {
				debug(3, "Size Mismatch. Skipping");
				fileMissing = true;
				break;
			}

			debug(3, "Matched file: %s", tstr.c_str());
			curFilesMatched++;
		}

		// We found at least one entry with all required files present.
		// That means that we got new variant of the game.
		//
		// Without this check we would have erroneous checksum display
		// where only located files will be enlisted.
		//
		// Potentially this could rule out variants where some particular file
		// is really missing, but the developers should better know about such
		// cases.
		if (allFilesPresent)
			gotAnyMatchesWithAllFiles = true;

		if (!fileMissing) {
			debug(2, "Found game: %s (%s %s/%s) (%d)", g->gameid, g->extra,
			 getPlatformDescription(g->platform), getLanguageDescription(g->language), i);

			if (curFilesMatched > maxFilesMatched) {
				debug(2, " ... new best match, removing all previous candidates");
				maxFilesMatched = curFilesMatched;

				matched.clear();	// Remove any prior, lower ranked matches.
				matched.push_back(g);
			} else if (curFilesMatched == maxFilesMatched) {
				matched.push_back(g);
			} else {
				debug(2, " ... skipped");
			}

		} else {
			debug(5, "Skipping game: %s (%s %s/%s) (%d)", g->gameid, g->extra,
			 getPlatformDescription(g->platform), getLanguageDescription(g->language), i);
		}
	}

	// We didn't find a match
	if (matched.empty()) {
		if (!filesSizeMD5.empty() && gotAnyMatchesWithAllFiles) {
			reportUnknown(parent, filesSizeMD5);
		}

		// Filename based fallback
		if (params.fileBasedFallback != 0)
			matched = detectGameFilebased(allFiles, params);
	}

	return matched;
}

/**
 * Check for each ADFileBasedFallback record whether all files listed
 * in it are present. If multiple pass this test, we pick the one with
 * the maximal number of matching files. In case of a tie, the entry
 * coming first in the list is chosen.
 */
static ADGameDescList detectGameFilebased(const FileMap &allFiles, const ADParams &params) {
	const ADFileBasedFallback *ptr;
	const char* const* filenames;

	int maxNumMatchedFiles = 0;
	const ADGameDescription *matchedDesc = 0;

	for (ptr = params.fileBasedFallback; ptr->desc; ++ptr) {
		const ADGameDescription *agdesc = (const ADGameDescription *)ptr->desc;
		int numMatchedFiles = 0;
		bool fileMissing = false;

		for (filenames = ptr->filenames; *filenames; ++filenames) {
			debug(3, "++ %s", *filenames);
			if (!allFiles.contains(*filenames)) {
				fileMissing = true;
				break;
			}

			numMatchedFiles++;
		}

		if (!fileMissing) {
			debug(4, "Matched: %s", agdesc->gameid);

			if (numMatchedFiles > maxNumMatchedFiles) {
				matchedDesc = agdesc;
				maxNumMatchedFiles = numMatchedFiles;

				debug(4, "and overridden");
			}
		}
	}

	ADGameDescList matched;

	if (matchedDesc) { // We got a match
		matched.push_back(matchedDesc);
		if (params.flags & kADFlagPrintWarningOnFileBasedFallback) {
			printf("Your game version has been detected using filename matching as a\n");
			printf("variant of %s.\n", matchedDesc->gameid);
			printf("If this is an original and unmodified version, please report any\n");
			printf("information previously printed by Residual to the team.\n");
		}
	}

	return matched;
}

GameList AdvancedMetaEngine::getSupportedGames() const {
	return gameIDList(params);
}
GameDescriptor AdvancedMetaEngine::findGame(const char *gameid) const {
	return AdvancedDetector::findGameID(gameid, params.list, params.obsoleteList);
}
