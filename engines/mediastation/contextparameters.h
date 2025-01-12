/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#ifndef MEDIASTATION_CONTEXTPARAMETERS_H
#define MEDIASTATION_CONTEXTPARAMETERS_H

#include "common/str.h"
#include "common/hashmap.h"

#include "mediastation/chunk.h"
#include "mediastation/mediascript/variable.h"
#include "mediastation/mediascript/function.h"

namespace MediaStation {

enum ContextParametersSectionType {
	kContextParametersEmptySection = 0x0000,
	kContextParametersVariable = 0x0014,
	kContextParametersName = 0x0bb9,
	kContextParametersFileNumber = 0x0011,
	kContextParametersBytecode = 0x0017
};

class ContextParameters {
public:
	ContextParameters(Chunk &chunk);
	~ContextParameters();

	// This is not an internal file ID, but the number of the file
	// as it appears in the filename. For instance, the context in
	// "100.cxt" would have file number 100.
	uint _fileNumber;
	Common::String *_contextName;
	Common::HashMap<uint32, Function *> _functions;
};

} // End of namespace MediaStation

#endif