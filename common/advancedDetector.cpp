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

using namespace AdvancedDetector;

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
static ADGameDescList detectGame(const FSList *fslist, const Common::ADParams &params, Language language, Platform platform, const Common::String extra);


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

	const char *gameid = ConfMan.get("gameid").c_str();

	for (const Common::ADObsoleteGameID *o = params.obsoleteList; o->from; ++o) {
		if (!scumm_stricmp(gameid, o->from)) {
			gameid = o->to;
			ConfMan.set("gameid", o->to);

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
	ADGameDescList matches = detectGame(&fslist, params, Common::UNK_LANG, Common::kPlatformUnknown, "");
	GameList detectedGames;

	// Use fallback detector if there were no matches by other means
	if (matches.empty()) {
		const Common::ADGameDescription *fallbackDesc = fallbackDetect(&fslist);
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

PluginError AdvancedMetaEngine::createInstance(OSystem *syst, Engine **engine) const {
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

	ADGameDescList matches = detectGame(0, params, language, platform, extra);

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
		agdDesc = fallbackDetect(NULL);
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

typedef HashMap<String, bool> StringSet;
typedef HashMap<String, int32> IntMap;

static void reportUnknown(StringMap &filesMD5, IntMap &filesSize) {
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

static ADGameDescList detectGame(const FSList *fslist, const Common::ADParams &params, Language language, Platform platform, const Common::String extra) {
	StringSet filesList;

	StringMap filesMD5;
	IntMap filesSize;
	IntMap allFiles;

	File testFile;

	String tstr;

	uint i;
	char md5str[32+1];

	bool fileMissing;
	const ADGameFileDescription *fileDesc;
	const ADGameDescription *g;
	const byte *descPtr;

	debug(3, "Starting detection");

	// First we compose list of files which we need MD5s for
	for (descPtr = params.descs; ((const ADGameDescription *)descPtr)->gameid != 0; descPtr += params.descItemSize) {
		g = (const ADGameDescription *)descPtr;

		for (fileDesc = g->filesDescriptions; fileDesc->fileName; fileDesc++) {
			tstr = String(fileDesc->fileName);
			tstr.toLowercase();
			filesList[tstr] = true;
		}
	}

	// TODO/FIXME: Fingolfin says: It's not good that we have two different code paths here,
	// one using a FSList, one using File::open, as that will lead to discrepancies and subtle
	// problems caused by those.
	if (fslist != 0) {
		// Get the information of the existing files
		for (FSList::const_iterator file = fslist->begin(); file != fslist->end(); ++file) {
			if (file->isDirectory()) continue;
			tstr = file->getName();
			tstr.toLowercase();

			// Strip any trailing dot
			if (tstr.lastChar() == '.')
				tstr.deleteLastChar();

			allFiles[tstr] = true;

			debug(3, "+ %s", tstr.c_str());

			if (!filesList.contains(tstr)) continue;

			if (!md5_file_string(*file, md5str, params.md5Bytes))
				continue;
			filesMD5[tstr] = md5str;

			debug(3, "> %s: %s", tstr.c_str(), md5str);

			if (testFile.open(file->getPath())) {
				filesSize[tstr] = (int32)testFile.size();
				testFile.close();
			}
		}
	} else {
		// Get the information of the requested files
		for (StringSet::const_iterator file = filesList.begin(); file != filesList.end(); ++file) {
			tstr = file->_key;

			debug(3, "+ %s", tstr.c_str());
			if (!filesMD5.contains(tstr)) {
				if (testFile.open(tstr) || testFile.open(tstr + ".")) {
					filesSize[tstr] = (int32)testFile.size();
					testFile.close();

					if (md5_file_string(file->_key.c_str(), md5str, params.md5Bytes)) {
						filesMD5[tstr] = md5str;
						debug(3, "> %s: %s", tstr.c_str(), md5str);
					}
				}
			}
		}
	}

	ADGameDescList matched;
	int maxFilesMatched = 0;

	// MD5 based matching
	for (i = 0, descPtr = params.descs; ((const ADGameDescription *)descPtr)->gameid != 0; descPtr += params.descItemSize, ++i) {
		g = (const ADGameDescription *)descPtr;
		fileMissing = false;

		// Do not even bother to look at entries which do not have matching
		// language and platform (if specified).
		if ((language != UNK_LANG && g->language != UNK_LANG && g->language != language) ||
			(platform != kPlatformUnknown && g->platform != kPlatformUnknown && g->platform != platform)) {
			continue;
		}

		if ((params.flags & kADFlagUseExtraAsHint) && extra != "" && g->extra != extra)
			continue;

		// Try to match all files for this game
		for (fileDesc = g->filesDescriptions; fileDesc->fileName; fileDesc++) {
			tstr = fileDesc->fileName;
			tstr.toLowercase();

			if (!filesMD5.contains(tstr)) {
				fileMissing = true;
				break;
			}
			if (fileDesc->md5 != NULL) {
				if (fileDesc->md5 != filesMD5[tstr]) {
					debug(3, "MD5 Mismatch. Skipping (%s) (%s)", fileDesc->md5, filesMD5[tstr].c_str());
					fileMissing = true;
					break;
				}
			}

			if (fileDesc->fileSize != -1) {
				if (fileDesc->fileSize != filesSize[tstr]) {
					debug(3, "Size Mismatch. Skipping");
					fileMissing = true;
					break;
				}
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

	// We've found a match
	if (!matched.empty())
		return matched;

	if (!filesMD5.empty())
		reportUnknown(filesMD5, filesSize);

	// Filename based fallback
	if (params.fileBasedFallback != 0) {
		const ADFileBasedFallback *ptr = params.fileBasedFallback;
		const char* const* filenames = 0;

		// First we create list of files required for detection.
		// The filenames can be different than the MD5 based match ones.
		for (; ptr->desc; ptr++) {
			filenames = ptr->filenames;
			for (; *filenames; filenames++) {
				tstr = String(*filenames);
				tstr.toLowercase();

				if (!allFiles.contains(tstr)) {
					if (testFile.open(tstr) || testFile.open(tstr + ".")) {
						allFiles[tstr] = true;
						testFile.close();
					}
				}
			}
		}

		// Then we perform the actual filename matching. If there are
		// several matches, only the one with the maximum numbers of
		// files is considered.
		int maxNumMatchedFiles = 0;
		const ADGameDescription *matchedDesc = 0;

		ptr = params.fileBasedFallback;

		for (; ptr->desc; ptr++) {
			const ADGameDescription *agdesc = (const ADGameDescription *)ptr->desc;
			int numMatchedFiles = 0;
			fileMissing = false;

			filenames = ptr->filenames;
			for (; *filenames; filenames++) {
				if (fileMissing) {
					continue;
				}

				tstr = String(*filenames);
				tstr.toLowercase();

				debug(3, "++ %s", *filenames);
				if (!allFiles.contains(tstr)) {
					fileMissing = true;
					continue;
				}

				numMatchedFiles++;
			}

			if (!fileMissing)
				debug(4, "Matched: %s", agdesc->gameid);

			if (!fileMissing && numMatchedFiles > maxNumMatchedFiles) {
				matchedDesc = agdesc;
				maxNumMatchedFiles = numMatchedFiles;

				debug(4, "and overriden");
			}
		}

		if (matchedDesc) { // We got a match
			matched.push_back(matchedDesc);
			if (params.flags & kADFlagPrintWarningOnFileBasedFallback) {
				printf("Your game version has been detected using filename matching as a\n");
				printf("variant of %s.\n", matchedDesc->gameid);
				printf("If this is an original and unmodified version, please report any\n");
				printf("information previously printed by ScummVM to the team.\n");
			}
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
