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

#include "mediastation/mediastation.h"
#include "mediastation/datum.h"
#include "mediastation/contextparameters.h"
#include "mediastation/mediascript/variable.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

ContextParameters::ContextParameters(Chunk &chunk) : _contextName(nullptr) {
	_fileNumber = Datum(chunk, kDatumTypeUint16_1).u.i;
	uint sectionType = static_cast<ContextParametersSectionType>(Datum(chunk, kDatumTypeUint16_1).u.i);
	while (sectionType != kContextParametersEmptySection) {
		debugC(5, kDebugLoading, "ContextParameters::ContextParameters: sectionType = 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
		switch (sectionType) {
		case kContextParametersName: {
			uint repeatedFileNumber = Datum(chunk, kDatumTypeUint16_1).u.i;
			if (repeatedFileNumber != _fileNumber) {
				warning("ContextParameters::ContextParameters(): Repeated file number didn't match: %d != %d", repeatedFileNumber, _fileNumber);
			}
			_contextName = Datum(chunk, kDatumTypeString).u.string;
			// TODO: This is likely just an end flag.
			uint endingFlag = Datum(chunk, kDatumTypeUint16_1).u.i;
			if (endingFlag != 0) {
				warning("ContextParameters::ContextParameters(): Got non-zero ending flag 0x%x", endingFlag);
			}
			break;
		}

		case kContextParametersFileNumber: {
			error("ContextParameters::ContextParameters(): Section type FILE_NUMBER not implemented yet");
			break;
		}

		case kContextParametersVariable: {
			uint repeatedFileNumber = Datum(chunk, kDatumTypeUint16_1).u.i;
			if (repeatedFileNumber != _fileNumber) {
				warning("ContextParameters::ContextParameters(): Repeated file number didn't match: %d != %d", repeatedFileNumber, _fileNumber);
			}
			// The trouble here is converting the variable to an operand.
			// They are two totally separate types!
			Variable *variable = new Variable(chunk);
			Operand operand;
			if (g_engine->_variables.contains(variable->_id)) {
				error("ContextParameters::ContextParameters(): Variable with ID 0x%x already exists", variable->_id);
			} else {
				g_engine->_variables.setVal(variable->_id, variable);
				debugC(5, kDebugScript, "ContextParameters::ContextParameters(): Created global variable %d", variable->_id);
			}
			break;
		}

		case kContextParametersBytecode: {
			Function *function = new Function(chunk);
			_functions.setVal(function->_id, function);
			break;
		}

		default: {
			error("ContextParameters::ContextParameters(): Unknown section type 0x%x", static_cast<uint>(sectionType));
		}
		}
		sectionType = Datum(chunk, kDatumTypeUint16_1).u.i;
	}
}

ContextParameters::~ContextParameters() {
	delete _contextName;
	_contextName = nullptr;

	for (auto it = _functions.begin(); it != _functions.end(); ++it) {
		delete it->_value;
	}
	_functions.clear();
}

} // End of namespace MediaStation
