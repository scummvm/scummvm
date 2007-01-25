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
 * @return	list of indexes to GameDescriptions of matched games
 */
static ADList detectGame(ADGameDescList gameDescriptions, const FSList *fslist, const Common::ADParams &params, Language language, Platform platform);


PluginError ADVANCED_DETECTOR_ENGINE_CREATE(
	GameList (*detectFunc)(const FSList &fslist),
	const Common::ADParams &params
	) {
	const char *gameid = ConfMan.get("gameid").c_str();

	if (params.obsoleteList != 0) {
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

	FSList fslist;
	FilesystemNode dir(ConfMan.get("path"));
	if (!dir.listDir(fslist, FilesystemNode::kListFilesOnly)) {
		return kInvalidPathError;
	}

	GameList detectedGames = detectFunc(fslist);

	for (uint i = 0; i < detectedGames.size(); i++) {
		if (detectedGames[i].gameid() == gameid) {
			return kNoError;
		}
	}

	return kNoGameDataFoundError;
}

GameDescriptor ADVANCED_DETECTOR_FIND_GAMEID(
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

GameList ADVANCED_DETECTOR_DETECT_GAMES_FUNCTION(
	const FSList &fslist,
	const Common::ADParams &params
	) {
	GameList detectedGames;
	Common::ADList matches;
	Common::ADGameDescList descList;
	const byte *descPtr;

	for (descPtr = params.descs; ((const ADGameDescription *)descPtr)->gameid != 0; descPtr += params.descItemSize)
		descList.push_back((const ADGameDescription *)descPtr);

	debug(3, "%s: cnt: %d", ((const ADGameDescription *)params.descs)->gameid,  descList.size());

	matches = detectGame(descList, &fslist, params, Common::UNK_LANG, Common::kPlatformUnknown);

	for (uint i = 0; i < matches.size(); i++)
		detectedGames.push_back(toGameDescriptor(*(const ADGameDescription *)(params.descs + matches[i] * params.descItemSize), params.list));

	return detectedGames;
}

int ADVANCED_DETECTOR_DETECT_INIT_GAME(
	const Common::ADParams &params
	) {
	int gameNumber = -1;

	GameList detectedGames;
	Common::ADList matches;
	Common::ADGameDescList descList;
	const byte *descPtr;

	Common::Language language = Common::UNK_LANG;
	Common::Platform platform = Common::kPlatformUnknown;

	if (ConfMan.hasKey("language"))
		language = Common::parseLanguage(ConfMan.get("language"));
	if (ConfMan.hasKey("platform"))
		platform = Common::parsePlatform(ConfMan.get("platform"));

	Common::String gameid = ConfMan.get("gameid");

	for (descPtr = params.descs; ((const ADGameDescription *)descPtr)->gameid != 0; descPtr += params.descItemSize)
		descList.push_back((const ADGameDescription *)descPtr);

	matches = detectGame(descList, 0, params, language, platform);

	for (uint i = 0; i < matches.size(); i++) {
		if (((const ADGameDescription *)(params.descs + matches[i] * params.descItemSize))->gameid == gameid) {
			gameNumber = matches[i];
			break;
		}
	}

	if (gameNumber >= (int)descList.size() || gameNumber == -1) {
		error("TODO invalid gameNumber %d (max. expected value: %d)", gameNumber, descList.size());
	}

	debug(2, "Running %s", toGameDescriptor(*(const ADGameDescription *)(params.descs + gameNumber * params.descItemSize), params.list).description().c_str());

	return gameNumber;
}


static String getDescription(const ADGameDescription *g) {
	char tmp[256];

	snprintf(tmp, 256, "%s (%s %s/%s)", g->gameid, g->extra,
			 getPlatformDescription(g->platform), getLanguageDescription(g->language));

	return String(tmp);
}

static ADList detectGame(ADGameDescList gameDescriptions, const FSList *fslist, const Common::ADParams &params, Language language, Platform platform) {
	typedef HashMap<String, bool, CaseSensitiveString_Hash, CaseSensitiveString_EqualTo> StringSet;
	StringSet filesList;

	typedef StringMap StringMap;
	typedef HashMap<String, int32, Common::CaseSensitiveString_Hash, Common::CaseSensitiveString_EqualTo> IntMap;
	StringMap filesMD5;
	IntMap filesSize;

	String tstr, tstr2;
	
	uint i;
	int j;
	char md5str[32+1];
	uint8 md5sum[16];

	bool fileMissing;
	const ADGameFileDescription *fileDesc;

	assert(gameDescriptions.size());

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

			if (!filesList.contains(tstr) && !filesList.contains(tstr2)) continue;

			if (!md5_file(*file, md5sum, params.md5Bytes)) continue;
			for (j = 0; j < 16; j++) {
				sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
			}
			filesMD5[tstr] = String(md5str);
			filesMD5[tstr2] = String(md5str);

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

	ADList matched;
	int maxFilesMatched = 0;

	for (i = 0; i < gameDescriptions.size(); i++) {
		const ADGameDescription *g = gameDescriptions[i];
		fileMissing = false;

		// Do not even bother to look at entries which do not have matching
		// language and platform (if specified).
		if ((language != UNK_LANG && g->language != language) ||
			(platform != kPlatformUnknown && g->platform != platform)) {
			continue;
		}
		
		// Try to open all files for this game
		for (j = 0; g->filesDescriptions[j].fileName; j++) {
			fileDesc = &g->filesDescriptions[j];
			tstr = fileDesc->fileName;
			tstr.toLowercase();
			tstr2 = tstr + ".";

			if (fileDesc->md5 != NULL) {
				if (!filesMD5.contains(tstr) && !filesMD5.contains(tstr2)) {
					fileMissing = true;
					break;
				}
				if (strcmp(fileDesc->md5, filesMD5[tstr].c_str()) && strcmp(fileDesc->md5, filesMD5[tstr2].c_str())) {
					fileMissing = true;
					break;
				}
			}
			if (fileDesc->fileSize != -1) {
				if (!filesMD5.contains(tstr) && !filesMD5.contains(tstr2)) {
					fileMissing = true;
					break;
				}
				if (fileDesc->fileSize != filesSize[tstr] && fileDesc->fileSize != filesSize[tstr2]) {
					fileMissing = true;
					break;
				}
			}
			debug(3, "Matched file: %s", tstr.c_str());
		}
		if (!fileMissing) {
			debug(2, "Found game: %s (%d)", getDescription(g).c_str(), i);

			// Count the number of matching files. Then, only keep those
			// entries which match a maximal amount of files.
			int curFilesMatched = 0;
			for (j = 0; g->filesDescriptions[j].fileName; j++)
				curFilesMatched++;
			
			if (curFilesMatched > maxFilesMatched) {
				debug(2, " ... new best match, removing all previous candidates");
				maxFilesMatched = curFilesMatched;
				matched.clear();
				matched.push_back(i);
			} else if (curFilesMatched == maxFilesMatched) {
				matched.push_back(i);
			} else {
				debug(2, " ... skipped");
			}

		} else {
			debug(5, "Skipping game: %s (%d)", getDescription(g).c_str(), i);
		}
	}

	if (!filesMD5.empty() && matched.empty()) {
		printf("MD5s of your game version are unknown. Please, report following data to\n");
		printf("ScummVM team along with your game name and version:\n");

		for (StringMap::const_iterator file = filesMD5.begin(); file != filesMD5.end(); ++file)
			printf("%s: %s\n", file->_key.c_str(), file->_value.c_str());
	}

	return matched;
}

}	// End of namespace Common
