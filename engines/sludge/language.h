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
#ifndef SLUDGE_LANGUAGE_H
#define SLUDGE_LANGUAGE_H

#include "common/file.h"

#include "sludge/allfiles.h"

namespace Sludge {

class LanguageManager {
public:
	LanguageManager();
	~LanguageManager();

	void init();
	void kill();

	void createTable(Common::File *table);
	void setLanguageID(uint id);
	void saveLanguageSetting(Common::WriteStream *writeStream);
	void loadLanguageSetting(Common::SeekableReadStream *readStream);

private:
	uint _languageID; // id of selected language
	int _languageIdx; // index of selected language in table
	uint _numLanguages; // number of existing languages in game
	uint *_languageTable; // indexes of existing languages
	Common::String *_languageNames; // language names

	int getLanguageForFileB();
	void setLanguageIndex(int idx);
};

} // End of namespace Sludge

#endif
