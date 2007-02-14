/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
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

#include "common/stdafx.h"

#include "base/plugins.h"

#include "common/util.h"
#include "common/hash-str.h"
#include "common/file.h"
#include "common/md5.h"
#include "common/advancedDetector.h"
#include "common/config-manager.h"

namespace Common {

namespace AdvancedDetector {

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
static ADGameDescList detectGame(const FSList *fslist, const Common::ADParams &params, Language language, Platform platform);


GameList gameIDList(const Common::ADParams &params) {
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

void upgradeTargetIfNecessary(const Common::ADParams &params) {
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
			ConfMan.flushToDisk();
			break;
		}
	}
}

GameDescriptor findGameID(
	const char *gameid,
	const Common::ADParams &params
	) {
	const PlainGameDescriptor *g = params.list;
	while (g->gameid) {
		if (0 == scumm_stricmp(gameid, g->gameid))
			return *g;
		g++;
	}

	GameDescriptor gs;

	if (params.obsoleteList != 0) {
		const Common::ADObsoleteGameID *o = params.obsoleteList;
		while (o->from) {
			if (0 == scumm_stricmp(gameid, o->from)) {
				gs["gameid"] = gameid;
				gs["description"] = "Obsolete game ID";
				return gs;
			}
			o++;
		}
	} else
		return GameDescriptor(g->gameid, g->description);
	return gs;
}

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

	if (desc->platform != kPlatformPC && desc->platform != kPlatformUnknown) {
		res = res + "-" + getPlatformAbbrev(desc->platform);
	}

	if (desc->language != EN_ANY && desc->language != UNK_LANG) {
		res = res + "-" + getLanguageCode(desc->language);
	}

	return res;
}

GameList detectAllGames(
	const FSList &fslist,
	const Common::ADParams &params
	) {
	ADGameDescList matches = detectGame(&fslist, params, Common::UNK_LANG, Common::kPlatformUnknown);

	GameList detectedGames;
	for (uint i = 0; i < matches.size(); i++) {
		GameDescriptor desc(toGameDescriptor(*matches[i], params.list));

		if (params.singleid != NULL) {
			desc["preferredtarget"] = desc["gameid"];
			desc["gameid"] = params.singleid;
		}

		if (params.flags & kADFlagAugmentPreferredTarget) {
			if (!desc.contains("preferredtarget"))
				desc["preferredtarget"] = desc["gameid"];

			desc["preferredtarget"] = generatePreferredTarget(desc["preferredtarget"], matches[i]);
		}

		detectedGames.push_back(desc);
	}

	return detectedGames;
}

const ADGameDescription *detectBestMatchingGame(
	const Common::ADParams &params
	) {
	const ADGameDescription *agdDesc = 0;
	Common::Language language = Common::UNK_LANG;
	Common::Platform platform = Common::kPlatformUnknown;

	if (ConfMan.hasKey("language"))
		language = Common::parseLanguage(ConfMan.get("language"));
	if (ConfMan.hasKey("platform"))
		platform = Common::parsePlatform(ConfMan.get("platform"));

	Common::String gameid = ConfMan.get("gameid");

	ADGameDescList matches = detectGame(0, params, language, platform);

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

	if (agdDesc != 0) {
		debug(2, "Running %s", toGameDescriptor(*agdDesc, params.list).description().c_str());
	}

	return agdDesc;
}

PluginError detectGameForEngineCreation(
	const Common::ADParams &params
	) {
	Common::String gameid = ConfMan.get("gameid");

	FSList fslist;
	FilesystemNode dir(ConfMan.get("path"));
	if (!dir.listDir(fslist, FilesystemNode::kListFilesOnly)) {
		return kInvalidPathError;
	}

	ADGameDescList matches = detectGame(&fslist, params, Common::UNK_LANG, Common::kPlatformUnknown);

	// We have single ID set, so we have a game if there are hits
	if (params.singleid != NULL && matches.size())
		return kNoError;

	for (uint i = 0; i < matches.size(); i++) {
		if (matches[i]->gameid == gameid) {
			return kNoError;
		}
	}

	return kNoGameDataFoundError;
}

static ADGameDescList detectGame(const FSList *fslist, const Common::ADParams &params, Language language, Platform platform) {
	typedef HashMap<String, bool, CaseSensitiveString_Hash, CaseSensitiveString_EqualTo> StringSet;
	StringSet filesList;

	typedef StringMap StringMap;
	typedef HashMap<String, int32, Common::CaseSensitiveString_Hash, Common::CaseSensitiveString_EqualTo> IntMap;
	StringMap filesMD5;
	IntMap filesSize;
	IntMap allFiles;

	String tstr, tstr2;
	
	uint i;
	int j;
	char md5str[32+1];
	uint8 md5sum[16];

	bool fileMissing;
	const ADGameFileDescription *fileDesc;

	Common::ADGameDescList gameDescriptions;

	debug(3, "Starting detection");

	for (const byte *descPtr = params.descs; ((const ADGameDescription *)descPtr)->gameid != 0; descPtr += params.descItemSize)
		gameDescriptions.push_back((const ADGameDescription *)descPtr);

	assert(gameDescriptions.size());

	debug(4, "List of descriptions: %d", gameDescriptions.size());

	// First we compose list of files which we need MD5s for
	for (i = 0; i < gameDescriptions.size(); i++) {
		for (j = 0; gameDescriptions[i]->filesDescriptions[j].fileName; j++) {
			tstr = String(gameDescriptions[i]->filesDescriptions[j].fileName);
			tstr.toLowercase();
			tstr2 = tstr + ".";
			filesList[tstr] = true;
			filesList[tstr2] = true;
		}
	}
	
	if (fslist != 0) {
		for (FSList::const_iterator file = fslist->begin(); file != fslist->end(); ++file) {
			Common::File f;

			if (file->isDirectory()) continue;
			tstr = file->name();
			tstr.toLowercase();
			tstr2 = tstr + ".";

			allFiles[tstr] = allFiles[tstr2] = 1;

			debug(3, "+ %s", tstr.c_str());

			if (!filesList.contains(tstr) && !filesList.contains(tstr2)) continue;

			if (!md5_file(*file, md5sum, params.md5Bytes)) continue;
			for (j = 0; j < 16; j++) {
				sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
			}
			filesMD5[tstr] = String(md5str);
			filesMD5[tstr2] = String(md5str);

			debug(3, "> %s: %s", tstr.c_str(), md5str);

			if (f.open(file->path())) {
				filesSize[tstr] = filesSize[tstr2] = (int32)f.size();
				f.close();
			}
		}
	} else {
		File testFile;

		for (StringSet::const_iterator file = filesList.begin(); file != filesList.end(); ++file) {
			tstr = file->_key;
			tstr.toLowercase();

			debug(3, "+ %s", tstr.c_str());
			if (!filesMD5.contains(tstr)) {
				if (testFile.open(file->_key)) {
					filesSize[tstr] = filesSize[tstr2] = (int32)testFile.size();
					testFile.close();

					if (md5_file(file->_key.c_str(), md5sum, params.md5Bytes)) {
						for (j = 0; j < 16; j++) {
							sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
						}
						filesMD5[tstr] = String(md5str);
						debug(3, "> %s: %s", tstr.c_str(), md5str);
					}
				}
			}
		}
	}

	ADGameDescList matched;
	int maxFilesMatched = 0;
	const ADGameDescription *g;

	for (i = 0; i < gameDescriptions.size(); i++) {
		g = gameDescriptions[i];
		fileMissing = false;

		// Do not even bother to look at entries which do not have matching
		// language and platform (if specified).
		if ((language != UNK_LANG && g->language != language) ||
			(platform != kPlatformUnknown && g->platform != platform)) {
			continue;
		}

		if (g->filesDescriptions[0].fileName == 0) {
			debug(5, "Skipping dummy entry: %s", g->gameid);
			continue;
		}

		// Try to open all files for this game
		for (j = 0; g->filesDescriptions[j].fileName; j++) {
			fileDesc = &g->filesDescriptions[j];
			tstr = fileDesc->fileName;
			tstr.toLowercase();
			tstr2 = tstr + ".";

			if (!filesMD5.contains(tstr) && !filesMD5.contains(tstr2)) {
				fileMissing = true;
				break;
			}
			if (fileDesc->md5 != NULL) {
				if (strcmp(fileDesc->md5, filesMD5[tstr].c_str()) && strcmp(fileDesc->md5, filesMD5[tstr2].c_str())) {
					debug(3, "MD5 Mismatch. Skipping (%s) (%s)", fileDesc->md5, filesMD5[tstr].c_str());
					fileMissing = true;
					break;
				}
			}

			if (fileDesc->fileSize != -1) {
				if (fileDesc->fileSize != filesSize[tstr] && fileDesc->fileSize != filesSize[tstr2]) {
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
			for (j = 0; g->filesDescriptions[j].fileName; j++)
				curFilesMatched++;
			
			if (curFilesMatched > maxFilesMatched) {
				debug(2, " ... new best match, removing all previous candidates");
				maxFilesMatched = curFilesMatched;
				matched.clear();
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

	if (!filesMD5.empty()) {
		// TODO: This message should be cleaned up / made more specific.
		// For example, we should specify at least which engine triggered this.
		printf("MD5s of your game version are unknown. Please, report following data to\n");
		printf("ScummVM team along with your game name and version:\n");

		for (StringMap::const_iterator file = filesMD5.begin(); file != filesMD5.end(); ++file)
			printf("%s: \"%s\", %d\n", file->_key.c_str(), file->_value.c_str(), filesSize[file->_key]);
	}

	if (params.fileBasedFallback != 0) {
		// The format of fileBasedFallback is like this:
		// It points to an array of strings (char pointers), separated into
		// multiple "rows".
		// First comes a gameid, then follows a list of filenames that have
		// to be present in order to generate a match; the row is terminated
		// by a zero byte.
		// The whole list is terminated by another zero byte (i.e. a zero gameid).
		const char **ptr = params.fileBasedFallback;

		// First we create list of files required for detection.
		if (allFiles.empty()) {
			File testFile;

			for (; *ptr; ptr++) {
				// skip the gameid
				ptr++;

				for (; *ptr; ptr++) {
					tstr = String(*ptr);
					tstr.toLowercase();

					if (!allFiles.contains(tstr)) {
						if (testFile.open(tstr)) {
							tstr2 = tstr + ".";
							allFiles[tstr] = allFiles[tstr2] = 1;
							testFile.close();
						}
					}
				}
			}
		}

		int maxNumMatchedFiles = 0;
		const char *matchedGameid = 0;

		ptr = params.fileBasedFallback;

		for (; *ptr; ptr++) {
			const char *entryGameid = *ptr++;
			fileMissing = false;
			int numMatchedFiles = 0;

			for (; *ptr; ptr++) {
				if (fileMissing) {
					continue;
				}

				tstr = String(*ptr);

				tstr.toLowercase();
				tstr2 = tstr + ".";

				debug(3, "++ %s", *ptr);
				if (!allFiles.contains(tstr) && !allFiles.contains(tstr2)) {
					fileMissing = true;
					continue;
				}

				numMatchedFiles++;
			}

			if (!fileMissing)
				debug(4, "Matched: %s", entryGameid);

			if (!fileMissing && numMatchedFiles > maxNumMatchedFiles) {
				matchedGameid = entryGameid;
				maxNumMatchedFiles = numMatchedFiles;

				debug(4, "and overriden");
			}
		}

		if (matchedGameid) { // We got a match
			for (i = 0; i < gameDescriptions.size(); i++) {
				g = gameDescriptions[i];
				if (g->filesDescriptions[0].fileName == 0) {
					if (!scumm_stricmp(g->gameid, matchedGameid)) {
						// FIXME: This warning, if ever seen by somebody, is
						// extremly cryptic!
						warning("But it looks like unknown variant of %s", matchedGameid);

						matched.push_back(g);
					}
				}
			}
		}
	}

	// If we still haven't got a match, try to use the fallback callback :-)
	if (matched.empty() && params.fallbackDetectFunc != 0) {
		matched = (*params.fallbackDetectFunc)(fslist);
	}

	return matched;
}

}	// End of namespace AdvancedDetector

}	// End of namespace Common
