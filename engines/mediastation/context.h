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

#ifndef MEDIASTATION_CONTEXT_H
#define MEDIASTATION_CONTEXT_H

#include "common/str.h"
#include "common/path.h"
#include "common/hashmap.h"
#include "graphics/palette.h"

#include "mediastation/datafile.h"
#include "mediastation/actor.h"

namespace MediaStation {

enum StreamType {
	kDocumentDefStream = 0x01,
	kControlCommandsStream = 0x0D,
};

enum ContextSectionType {
	kEndOfContextData = 0x00,
	kContextCreateData = 0x0e,
	kContextDestroyData = 0x0f,
	kContextLoadCompleteSection = 0x10,
	kContextCreateActorData = 0x11,
	kContextDestroyActorData = 0x12,
	kContextActorLoadComplete = 0x13,
	kContextCreateVariableData = 0x14,
	kContextFunctionSection = 0x31,
	kContextNameData = 0xbb8
};

class ScreenActor;

class Context : public Datafile {
public:
	Context(const Common::Path &path);
	~Context();

	uint32 _unk1;
	uint32 _subfileCount;
	uint32 _fileSize;
	ScreenActor *_screenActor = nullptr;

	Actor *getActorById(uint actorId);
	Actor *getActorByChunkReference(uint chunkReference);
	ScriptValue *getVariable(uint variableId);

private:
	// This is not an internal file ID, but the number of the file
	// as it appears in the filename. For instance, the context in
	// "100.cxt" would have file number 100.
	uint _id = 0;
	Common::String _contextName;

	Common::HashMap<uint, Actor *> _actors;
	Common::HashMap<uint, Actor *> _actorsByChunkReference;
	Common::HashMap<uint, ScriptValue *> _variables;

	void readHeaderSections(Subfile &subfile, Chunk &chunk);

	void readControlCommands(Chunk &chunk);
	void readCommandFromStream(ContextSectionType sectionType, Chunk &chunk);
	void readCreateContextData(Chunk &chunk);
	void readDestroyContextData(Chunk &chunk);
	void readCreateActorData(Chunk &chunk);
	void readDestroyActorData(Chunk &chunk);
	void readActorLoadComplete(Chunk &chunk);
	void readCreateVariableData(Chunk &chunk);
	void readContextNameData(Chunk &chunk);

	void readActorFromLaterSubfile(Subfile &subfile);
};

} // End of namespace MediaStation

#endif
