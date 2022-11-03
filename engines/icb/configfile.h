/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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
 */

#ifndef ICB_CONFIGFILE_H
#define ICB_CONFIGFILE_H

#include "common/str.h"
#include "common/hash-str.h"

namespace ICB {

class ConfigFile {
	Common::HashMap<Common::String, Common::HashMap<Common::String, Common::String> > _dataSet;

public:
	ConfigFile();
	void writeSetting(const Common::String &section, const Common::String &key, const Common::String &value) {}
	Common::String readSetting(const Common::String &section, const Common::String &key, const Common::String &defaultValue) const;
	int32 readIntSetting(const Common::String &section, const Common::String &key, int32 defaultValue) const;

	void readFile(const Common::String &filename);
};

} // End of namespace ICB

#endif
