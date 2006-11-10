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

#include "common/util.h"
#include "common/hash-str.h"
#include "common/file.h"
#include "common/md5.h"
#include "common/advancedDetector.h"

namespace Common {

bool ADTrue() {
	return true;
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
	int filesCount;

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
		for (j = 0; j < _gameDescriptions[i]->filesCount; j++) {
			tstr = String(_gameDescriptions[i]->filesDescriptions[j].fileName);
			tstr.toLowercase();
			tstr2 = tstr + ".";
			filesList[tstr] = true;
			filesList[tstr2] = true;
		}
	}
	
	if (fslist != NULL) {
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
		filesCount = _gameDescriptions[i]->filesCount;		
		fileMissing = false;

		// Try to open all files for this game
		for (j = 0; j < filesCount; j++) {
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
			debug(2, "Found game: %s", getDescription(i).c_str());
			matched[matchedCount++] = i;
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
			maxcount = MAX(_gameDescriptions[matched[i]]->filesCount, maxcount);
		}

		// Now purge targets with number of files lesser than max
		for (i = 0; i < matchedCount; i++) {
			if ((_gameDescriptions[matched[i]]->language != language && language != UNK_LANG) ||
				(_gameDescriptions[matched[i]]->platform != platform && platform != kPlatformUnknown)) {
				debug(2, "Purged %s", getDescription(matched[i]).c_str());
				matched[i] = -1;
				continue;
			}

			if (_gameDescriptions[matched[i]]->filesCount < maxcount) {
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
