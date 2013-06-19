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
 */

#ifndef FULLPIPE_UTILS_H
#define FULLPIPE_UTILS_H

#include "common/hash-str.h"
#include "common/array.h"
#include "common/file.h"

namespace Fullpipe {

class CObject;

typedef Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> ClassMap;

class MfcArchive : public Common::File {
	ClassMap _classMap;
	Common::Array<CObject *> _objectMap;
	Common::Array<int> _objectIdMap;

	int _lastIndex;
	int _level;

 public:
	MfcArchive();

	char *readPascalString(bool twoByte = false);
	int readCount();
	double readDouble();
	CObject *parseClass(bool *isCopyReturned);
	CObject *readClass();

	void incLevel() { _level++; }
	void decLevel() { _level--; }
	int getLevel() { return _level; }
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_UTILS_H */
