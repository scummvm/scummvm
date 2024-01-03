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

#ifndef DGDS_SCRIPTS_H
#define DGDS_SCRIPTS_H

#include "common/rect.h"
#include "dgds/parser.h"

namespace Dgds {

class DgdsEngine;
class DgdsChunk;


class ScriptParserData : public ParserData {
public:
	ScriptParserData() : scr(nullptr) {}
	Common::String filename;
	Common::SeekableReadStream *scr;
	Common::HashMap<uint16, Common::String> _tags;
};

class TTMData : public ScriptParserData {
public:
};

struct TTMState {
	TTMState() : dataPtr(nullptr), scene(0), delay(0) {}
	const TTMData *dataPtr;
	uint16 scene;
	int delay;
};

class ADSData : public ScriptParserData {
public:
	ADSData() : count(0), scriptDatas(nullptr) {}
	uint16 count;
	Common::Array<Common::String> names;
	TTMData *scriptDatas;
};

struct ADSState {
	ADSState() : dataPtr(nullptr), scene(0), subIdx(0), subMax(0) {}
	const ADSData *dataPtr;
	uint16 scene;
	uint16 subIdx, subMax;

	Common::Array<TTMState> scriptStates;
};

class ADSInterpreter {
public:
	ADSInterpreter(DgdsEngine *vm);

	bool load(const Common::String &filename, ADSData *data);
	void unload(ADSData *data);

	void init(ADSState *scriptState, const ADSData *scriptData);
	bool run(ADSState *script);

protected:
	DgdsEngine *_vm;

	Common::String _filename;
	ADSData *_scriptData;
};

class TTMInterpreter {
public:
	TTMInterpreter(DgdsEngine *vm);

	bool load(const Common::String &filename, TTMData *data);
	void unload(TTMData *data);

	void init(TTMState *scriptState, const TTMData *scriptData);
	bool run(TTMState *script);

protected:
	DgdsEngine *_vm;

	//Common::String _filename;
	//TTMData *_scriptData;

	//Common::String _bmpNames[16];
};

} // End of namespace Dgds

#endif // DGDS_SCRIPTS_H
