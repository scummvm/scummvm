/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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
#ifndef COMMON_ADVANCED_DETECTOR_H
#define COMMON_ADVANCED_DETECTOR_H

#include "common/fs.h"

namespace Common {

struct ADGameFileDescription {
	const char *fileName;
	uint16 fileType;
	const char *md5;
};

struct ADGameDescription {
	const char *name;
	const char *extra;
	int filesCount;
	const ADGameFileDescription *filesDescriptions;
	Language language;
	Platform platform;
};

typedef Array<int> ADList;
typedef Array<const ADGameDescription*> ADGameDescList;

class AdvancedDetector {

public:
	AdvancedDetector();
	~AdvancedDetector() {};


	void registerGameDescriptions(ADGameDescList gameDescriptions) {
		_gameDescriptions = gameDescriptions;
	}

	/**
	 * Specify number of bytes which are used to calculate MD5.
	 * Default value is 0 which means whole file.
	 */
	void setFileMD5Bytes(int bytes) { _fileMD5Bytes = bytes; }

	/**
	 * Detect games in specified directory.
	 * Parameters language and platform are used to pass on values
	 * specified by the user. I.e. this is used to restrict search scope.
	 *
	 * @param fslist	FSList to scan or NULL for scanning all specified
	 *  default directories.
	 * @param language	restrict results to specified language only
	 * @param platform	restrict results to specified platform only
	 * @return	list of indexes to GameDescriptions of matched games
	 */
	ADList detectGame(const FSList *fslist, Language language, Platform platform);

private:
	ADGameDescList _gameDescriptions;

	int _fileMD5Bytes;

	String getDescription(int num) const;
};

}	// End of namespace Common

#endif
