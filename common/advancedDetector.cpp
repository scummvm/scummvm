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

#include "base/plugins.h"

#include "common/util.h"
#include "common/hash-str.h"
#include "common/file.h"
#include "common/md5.h"
#include "common/advancedDetector.h"
#include "common/config-manager.h"

namespace Common {

/**
 * A list of pointers to ADGameDescription structs (or subclasses thereof).
 */
typedef Array<const ADGameDescription*> ADGameDescList;


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
static ADGameDescList detectGame(const FSList &fslist, const Common::ADParams &params, Language language, Platform platform, const Common::String extra);


/**
 * Returns list of targets supported by the engine.
 * Distinguishes engines with single ID
 */
static GameList gameIDList(const Common::ADParams &params) {
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

static void upgradeTargetIfNecessary(const Common::ADParams &params) {
	if (params.obsoleteList == 0)
		return;

	String gameid = ConfMan.get("gameid");

	for (const Common::ADObsoleteGameID *o = params.obsoleteList; o->from; ++o) {
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
	const Common::ADObsoleteGameID *obsoleteList
	) {
	// First search the list of supported game IDs for a match.
	const PlainGameDescriptor *g = findPlainGameDescriptor(gameid, list);
	if (g)
		return GameDescriptor(*g);

	// If we didn't find the gameid in the main list, check if it
	// is an obsolete game id.
	if (obsoleteList != 0) {
		const Common::ADObsoleteGameID *o = obsoleteList;
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

	while (sg->gameid) {
		if (!scumm_stricmp(g.gameid, sg->gameid))
			title = sg->description;
		sg++;
	}

	GameDescriptor gd(g.gameid, title, g.language, g.platform);
	gd.updateDesc(g.extra);
	return gd;
}

/**
 * Generate a preferred target value as
 *   GAMEID-PLAFORM-LANG
 * or (if ADGF_DEMO has been set)
 *   GAMEID-demo-PLAFORM-LANG
 */
static String generatePreferredTarget(const String &id, const ADGameDescription *desc) {
	String res(id);

	if (desc->flags & ADGF_DEMO) {
		res = res + "-demo";
	}

	if (desc->flags & ADGF_CD) {
		res = res + "-cd";
	}

	if (desc->platform != kPlatformPC && desc->platform != kPlatformUnknown) {
		res = res + "-" + getPlatformAbbrev(desc->platform);
	}

	if (desc->language != EN_ANY && desc->language != UNK_LANG && !(desc->flags & ADGF_DROPLANGUAGE)) {
		res = res + "-" + getLanguageCode(desc->language);
	}

	return res;
}

static void updateGameDescriptor(GameDescriptor &desc, const ADGameDescription *realDesc, const Common::ADParams &params) {
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
}

GameList AdvancedMetaEngine::detectGames(const FSList &fslist) const {
	ADGameDescList matches = detectGame(fslist, params, Common::UNK_LANG, Common::kPlatformUnknown, "");
	GameList detectedGames;

	// Use fallback detector if there were no matches by other means
	if (matches.empty()) {
		const Common::ADGameDescription *fallbackDesc = fallbackDetect(fslist);
		if (fallbackDesc != 0) {
			GameDescriptor desc(toGameDescriptor(*fallbackDesc, params.list));
			updateGameDescriptor(desc, fallbackDesc, params);
			detectedGames.push_back(desc);
		}
	} else for (uint i = 0; i < matches.size(); i++) { // Otherwise use the found matches
		GameDescriptor desc(toGameDescriptor(*matches[i], params.list));
		updateGameDescriptor(desc, matches[i], params);
		detectedGames.push_back(desc);
	}

	return detectedGames;
}

Common::Error AdvancedMetaEngine::createInstance(OSystem *syst, Engine **engine) const {
	assert(engine);
	upgradeTargetIfNecessary(params);

	const ADGameDescription *agdDesc = 0;
	Common::Language language = Common::UNK_LANG;
	Common::Platform platform = Common::kPlatformUnknown;
	Common::String extra("");

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
		warning("No path was provided. Assuming the data files are in the current directory");
	}
	FSNode dir(path);
	FSList files;
	if (!dir.isDirectory() || !dir.getChildren(files, FSNode::kListAll)) {
		warning("Game data path does not exist or is not a directory (%s)", path.c_str());
		return kNoGameDataFoundError;
	}

	ADGameDescList matches = detectGame(files, params, language, platform, extra);

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

	if (agdDesc == 0) {
		return kNoGameDataFoundError;
	}

	debug(2, "Running %s", toGameDescriptor(*agdDesc, params.list).description().c_str());
	if (!createInstance(syst, engine, agdDesc)) {
		return kNoGameDataFoundError;
	}
	return kNoError;
}

typedef HashMap<String, bool, IgnoreCase_Hash, IgnoreCase_EqualTo> StringSet;
typedef HashMap<String, int32, IgnoreCase_Hash, IgnoreCase_EqualTo> IntMap;
typedef HashMap<String, FSNode, IgnoreCase_Hash, IgnoreCase_EqualTo> FileMap;

static void reportUnknown(const StringMap &filesMD5, const IntMap &filesSize) {
	// TODO: This message should be cleaned up / made more specific.
	// For example, we should specify at least which engine triggered this.
	//
	// Might also be helpful to display the full path (for when this is used
	// from the mass detector).
	printf("Your game version appears to be unknown. Please, report the following\n");
	printf("data to the ScummVM team along with name of the game you tried to add\n");
	printf("and its version/language/etc.:\n");

	for (StringMap::const_iterator file = filesMD5.begin(); file != filesMD5.end(); ++file)
		printf("  \"%s\", \"%s\", %d\n", file->_key.c_str(), file->_value.c_str(), filesSize[file->_key]);

	printf("\n");
}

static ADGameDescList detectGameFilebased(const FileMap &allFiles, const Common::ADParams &params);

static ADGameDescList detectGame(const FSList &fslist, const Common::ADParams &params, Language language, Platform platform, const Common::String extra) {
	FileMap allFiles;

	StringSet detectFiles;
	StringMap filesMD5;
	IntMap filesSize;

	const ADGameFileDescription *fileDesc;
	const ADGameDescription *g;
	const byte *descPtr;

	debug(3, "Starting detection");

	// First we compose an efficient to query set of all files in  fslist.
	// Includes nifty stuff like removing trailing dots and ignoring case.
	for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		String tstr = file->getName();

		// Strip any trailing dot
		if (tstr.lastChar() == '.')
			tstr.deleteLastChar();

		allFiles[tstr] = *file;	// Record the presence of this file
	}

	// Compute the set of files for which we need MD5s for. I.e. files which are
	// included in some ADGameDescription *and* present in fslist.
	for (descPtr = params.descs; ((const ADGameDescription *)descPtr)->gameid != 0; descPtr += params.descItemSize) {
		g = (const ADGameDescription *)descPtr;

		for (fileDesc = g->filesDescriptions; fileDesc->fileName; fileDesc++) {
			String tstr = fileDesc->fileName;
			if (allFiles.contains(tstr))
				detectFiles[tstr] = true;
		}
	}

	// Get the information for all detection files, if they exist
	for (StringSet::const_iterator file = detectFiles.begin(); file != detectFiles.end(); ++file) {
		String fname = file->_key;

		debug(3, "+ %s", fname.c_str());

		char md5str[32+1];
		if (!md5_file_string(allFiles[fname], md5str, params.md5Bytes))
			continue;
		filesMD5[fname] = md5str;

		debug(3, "> %s: %s", fname.c_str(), md5str);

		File testFile;
		if (testFile.open(allFiles[fname])) {
			filesSize[fname] = (int32)testFile.size();
			testFile.close();
		}
	}


	ADGameDescList matched;
	int maxFilesMatched = 0;

	// MD5 based matching
	uint i;
	for (i = 0, descPtr = params.descs; ((const ADGameDescription *)descPtr)->gameid != 0; descPtr += params.descItemSize, ++i) {
		g = (const ADGameDescription *)descPtr;
		bool fileMissing = false;

		// Do not even bother to look at entries which do not have matching
		// language and platform (if specified).
		if ((language != UNK_LANG && g->language != UNK_LANG && g->language != language) ||
			(platform != kPlatformUnknown && g->platform != kPlatformUnknown && g->platform != platform)) {
			continue;
		}

		if ((params.flags & kADFlagUseExtraAsHint) && !extra.empty() && g->extra != extra)
			continue;

		// Try to match all files for this game
		for (fileDesc = g->filesDescriptions; fileDesc->fileName; fileDesc++) {
			String tstr = fileDesc->fileName;

			if (!filesMD5.contains(tstr)) {
				fileMissing = true;
				break;
			}

			if (fileDesc->md5 != NULL && fileDesc->md5 != filesMD5[tstr]) {
				debug(3, "MD5 Mismatch. Skipping (%s) (%s)", fileDesc->md5, filesMD5[tstr].c_str());
				fileMissing = true;
				break;
			}

			if (fileDesc->fileSize != -1 && fileDesc->fileSize != filesSize[tstr]) {
				debug(3, "Size Mismatch. Skipping");
				fileMissing = true;
				break;
			}

			debug(3, "Matched file: %s", tstr.c_str());
		}
		if (!fileMissing) {
			debug(2, "Found game: %s (%s %s/%s) (%d)", g->gameid, g->extra,
			 getPlatformDescription(g->platform), getLanguageDescription(g->language), i);

			// Count the number of matching files. Then, only keep those
			// entries which match a maximal amount of files.
			int curFilesMatched = 0;
			for (fileDesc = g->filesDescriptions; fileDesc->fileName; fileDesc++)
				curFilesMatched++;

			if (curFilesMatched > maxFilesMatched) {
				debug(2, " ... new best match, removing all previous candidates");
				maxFilesMatched = curFilesMatched;

				for (uint j = 0; j < matched.size();) {
					if (matched[j]->flags & ADGF_KEEPMATCH)
						 ++j;
					else
						matched.remove_at(j);
				}
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
		if (!filesMD5.empty())
			reportUnknown(filesMD5, filesSize);
	
		// Filename based fallback
		if (params.fileBasedFallback != 0)
			matched = detectGameFilebased(allFiles, params);
	}

	return matched;
}

/**
 * Check for each ADFileBasedFallback record whether all files listed
 * in it  are present. If multiple pass this test, we pick the one with
 * the maximal number of matching files. In case of a tie, the entry
 * coming first in the list is chosen.
 */
static ADGameDescList detectGameFilebased(const FileMap &allFiles, const Common::ADParams &params) {
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
	
				debug(4, "and overriden");
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
			printf("information previously printed by ScummVM to the team.\n");
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

}	// End of namespace Common
