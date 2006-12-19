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

PluginError real_ADVANCED_DETECTOR_ENGINE_CREATE(
	DetectedGameList (*detectFunc)(const FSList &fslist),
	const Common::ADObsoleteGameID *obsoleteList
	) {
	const char *gameid = ConfMan.get("gameid").c_str();

	if (obsoleteList != 0) {
		for (const Common::ADObsoleteGameID *o = obsoleteList; o->from; ++o) {
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

	DetectedGameList detectedGames = detectFunc(fslist);

	for (uint i = 0; i < detectedGames.size(); i++) {
		if (detectedGames[i].gameid == gameid) {
			return kNoError;
		}
	}

	return kNoGameDataFoundError;
}

GameList real_ADVANCED_DETECTOR_GAMEID_LIST(const PlainGameDescriptor *list) {
	GameList games;
	const PlainGameDescriptor *g = list;
	while (g->gameid) {
		games.push_back(*g);
		g++;
	}
	return games;
}

GameDescriptor real_ADVANCED_DETECTOR_FIND_GAMEID(
	const char *gameid,
	const PlainGameDescriptor *list,
	const Common::ADObsoleteGameID *obsoleteList
	) {
	const PlainGameDescriptor *g = list;
	while (g->gameid) {
		if (0 == scumm_stricmp(gameid, g->gameid))
			return *g;
		g++;
	}

	GameDescriptor gs;
	if (obsoleteList != 0) {
		const Common::ADObsoleteGameID *o = obsoleteList;
		while (o->from) {
			if (0 == scumm_stricmp(gameid, o->from)) {
				gs.gameid = gameid;
				gs.description = "Obsolete game ID";
				return gs;
			}
			o++;
		}
	} else
		return *g;
	return gs;
}

DetectedGame toDetectedGame(const ADGameDescription &g, const PlainGameDescriptor *sg) {
	const char *title = 0;

	while (sg->gameid) {
		if (!scumm_stricmp(g.name, sg->gameid))
			title = sg->description;
		sg++;
	}

	DetectedGame dg(g.name, title, g.language, g.platform);
	dg.updateDesc(g.extra);
	return dg;
}

DetectedGameList real_ADVANCED_DETECTOR_DETECT_GAMES_FUNCTION(
	const FSList &fslist,
	const byte *descs,
	const int descItemSize,
	const int md5Bytes,
	const PlainGameDescriptor *list
	) {
	DetectedGameList detectedGames;
	Common::AdvancedDetector AdvDetector;
	Common::ADList matches;
	Common::ADGameDescList descList;
	const byte *descPtr;

	for (descPtr = descs; *descPtr != NULL; descPtr += descItemSize)
		descList.push_back((const ADGameDescription *)descPtr);

	AdvDetector.registerGameDescriptions(descList);
	AdvDetector.setFileMD5Bytes(md5Bytes);

	debug(3, "%s: cnt: %d", ((const ADGameDescription *)descs)->name,  descList.size());

	matches = AdvDetector.detectGame(&fslist, Common::UNK_LANG, Common::kPlatformUnknown);

	for (uint i = 0; i < matches.size(); i++)
		detectedGames.push_back(toDetectedGame(*(const ADGameDescription *)(descs + matches[i] * descItemSize), list));

	return detectedGames;
}

int real_ADVANCED_DETECTOR_DETECT_INIT_GAME(
	const byte *descs,
	const int descItemSize,
	const int md5Bytes,
	const PlainGameDescriptor *list
	) {
	int gameNumber = -1;

	DetectedGameList detectedGames;
	Common::AdvancedDetector AdvDetector;
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

	for (descPtr = descs; *descPtr != NULL; descPtr += descItemSize)
		descList.push_back((const ADGameDescription *)descPtr);

	AdvDetector.registerGameDescriptions(descList);
	AdvDetector.setFileMD5Bytes(md5Bytes);

	matches = AdvDetector.detectGame(0, language, platform);

	for (uint i = 0; i < matches.size(); i++) {
		if (toDetectedGame(*(const ADGameDescription *)(descs + matches[i] * descItemSize), list).gameid == gameid) {
			gameNumber = matches[i];
			break;
		}
	}

	if (gameNumber >= (int)descList.size() || gameNumber == -1) {
		error("TODO invalid gameNumber %d (max. expected value: %d)", gameNumber, descList.size());
	}

	debug(2, "Running %s", toDetectedGame(*(const ADGameDescription *)(descs + gameNumber * descItemSize), list).description.c_str());

	return gameNumber;
}


AdvancedDetector::AdvancedDetector() {
	_fileMD5Bytes = 0;
}

String AdvancedDetector::getDescription(int num) const {
	char tmp[256];
	const ADGameDescription *g = _gameDescriptions[num];

	snprintf(tmp, 256, "%s (%s %s/%s)", g->name, g->extra,
			 getPlatformDescription(g->platform), getLanguageDescription(g->language));

	return String(tmp);
}

ADList AdvancedDetector::detectGame(const FSList *fslist, Language language, Platform platform) {
	typedef HashMap<String, bool, CaseSensitiveString_Hash, CaseSensitiveString_EqualTo> StringSet;
	StringSet filesList;

	typedef StringMap StringMap;
	StringMap filesMD5;

	String tstr, tstr2;
	
	uint i;
	int j;
	char md5str[32+1];
	uint8 md5sum[16];
	int *matched;

	uint matchedCount = 0;
	bool fileMissing;
	const ADGameFileDescription *fileDesc;

	assert(_gameDescriptions.size());

	matched = new int[_gameDescriptions.size()];

	// First we compose list of files which we need MD5s for
	for (i = 0; i < _gameDescriptions.size(); i++) {
		for (j = 0; _gameDescriptions[i]->filesDescriptions[j].fileName; j++) {
			tstr = String(_gameDescriptions[i]->filesDescriptions[j].fileName);
			tstr.toLowercase();
			tstr2 = tstr + ".";
			filesList[tstr] = true;
			filesList[tstr2] = true;
		}
	}
	
	if (fslist != 0) {
		for (FSList::const_iterator file = fslist->begin(); file != fslist->end(); ++file) {
			if (file->isDirectory()) continue;
			tstr = file->name();
			tstr.toLowercase();
			tstr2 = tstr + ".";

			if (!filesList.contains(tstr) && !filesList.contains(tstr2)) continue;

			if (!md5_file(*file, md5sum, _fileMD5Bytes)) continue;
			for (j = 0; j < 16; j++) {
				sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
			}
			filesMD5[tstr] = String(md5str);
			filesMD5[tstr2] = String(md5str);
		}
	} else {
		File testFile;

		for (StringSet::const_iterator file = filesList.begin(); file != filesList.end(); ++file) {
			tstr = file->_key;
			tstr.toLowercase();

			debug(3, "+ %s", tstr.c_str());
			if (!filesMD5.contains(tstr)) {
				if (testFile.open(file->_key)) {
					testFile.close();

					if (md5_file(file->_key.c_str(), md5sum, _fileMD5Bytes)) {
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

	for (i = 0; i < _gameDescriptions.size(); i++) {
		fileMissing = false;

		// Try to open all files for this game
		for (j = 0; _gameDescriptions[i]->filesDescriptions[j].fileName; j++) {
			fileDesc = &_gameDescriptions[i]->filesDescriptions[j];
			tstr = fileDesc->fileName;
			tstr.toLowercase();
			tstr2 = tstr + ".";

			if (!filesMD5.contains(tstr) && !filesMD5.contains(tstr2)) {
				fileMissing = true;
				break;
			}
			if (strcmp(fileDesc->md5, filesMD5[tstr].c_str()) && strcmp(fileDesc->md5, filesMD5[tstr2].c_str())) {
				fileMissing = true;
				break;
			}
			debug(3, "Matched file: %s", tstr.c_str());
		}
		if (!fileMissing) {
			debug(2, "Found game: %s (%d)", getDescription(i).c_str(), i);
			matched[matchedCount++] = i;
		} else {
			debug(5, "Skipping game: %s (%d)", getDescription(i).c_str(), i);
		}
	}

	if (!filesMD5.empty() && (matchedCount == 0)) {
		printf("MD5s of your game version are unknown. Please, report following data to\n");
		printf("ScummVM team along with your game name and version:\n");

		for (StringMap::const_iterator file = filesMD5.begin(); file != filesMD5.end(); ++file)
			printf("%s: %s\n", file->_key.c_str(), file->_value.c_str());
	}

	// We have some resource sets which are superpositions of other
	// Now remove lesser set if bigger matches too

	if (matchedCount > 1) {
		// Search max number
		int maxcount = 0;
		for (i = 0; i < matchedCount; i++) {
			int fCount = 0;
			for (j = 0; _gameDescriptions[i]->filesDescriptions[j].fileName; j++)
				fCount++;
			maxcount = MAX(fCount, maxcount);
		}

		// Now purge targets with number of files lesser than max
		for (i = 0; i < matchedCount; i++) {
			if ((_gameDescriptions[matched[i]]->language != language && language != UNK_LANG) ||
				(_gameDescriptions[matched[i]]->platform != platform && platform != kPlatformUnknown)) {
				debug(2, "Purged %s", getDescription(matched[i]).c_str());
				matched[i] = -1;
				continue;
			}

			int fCount = 0;
			for (j = 0; _gameDescriptions[matched[i]]->filesDescriptions[j].fileName; j++)
				fCount++;

			if (fCount < maxcount) {
				debug(2, "Purged: %s", getDescription(matched[i]).c_str());
				matched[i] = -1;
			}
		}
	}


	ADList returnMatches;
	for (i = 0; i < matchedCount; i++)
		if (matched[i] != -1)
			returnMatches.push_back(matched[i]);

	delete[] matched;
	return returnMatches;
}

}	// End of namespace Common
