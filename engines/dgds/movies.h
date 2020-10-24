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

#ifndef DGDS_MOVIES_H
#define DGDS_MOVIES_H

namespace Dgds {

class DgdsEngine;
class DgdsChunk;

struct TTMData {
	char filename[13];
	Common::SeekableReadStream *scr;
};

struct TTMState {
	const TTMData *dataPtr;
	uint16 scene;
	int delay;
};

struct ADSData {
	char filename[13];

	uint16 count;
	char **names;
	TTMData *scriptDatas;

	Common::SeekableReadStream *scr;
};

struct ADSState {
	const ADSData *dataPtr;
	uint16 scene;
	uint16 subIdx, subMax;

	TTMState *scriptStates;
};

class ADSInterpreter {
public:
	ADSInterpreter(DgdsEngine *vm);

	bool load(const char *filename, ADSData *data);
	void unload(ADSData *data);

	bool callback(DgdsChunk &chunk);

	void init(ADSState *scriptState, const ADSData *scriptData);
	bool run(ADSState *script);

protected:
	DgdsEngine *_vm;

	const char *_filename;
	ADSData *_scriptData;
};

class TTMInterpreter {
public:
	TTMInterpreter(DgdsEngine *vm);

	bool load(const char *filename, TTMData *data);
	void unload(TTMData *data);

	bool callback(DgdsChunk &chunk);

	void init(TTMState *scriptState, const TTMData *scriptData);
	bool run(TTMState *script);

protected:
	DgdsEngine *_vm;

	const char *_filename;
	TTMData *_scriptData;

	//Common::String _bmpNames[16];
};

} // End of namespace Dgds

#endif // DGDS_MOVIES_H
