/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef LINGODEC_CONTEXT_H
#define LINGODEC_CONTEXT_H

#include "common/array.h"
#include "common/stablemap.h"

namespace Common {
class SeekableReadStream;
}

namespace LingoDec {

class ChunkResolver;
struct Script;
struct ScriptContextMapEntry;
struct ScriptNames;

/* ScriptContext */

struct ScriptContext {
	int32 unknown0;
	int32 unknown1;
	uint32 entryCount;
	uint32 entryCount2;
	uint16 entriesOffset;
	int16 unknown2;
	int32 unknown3;
	int32 unknown4;
	int32 unknown5;
	int32 lnamSectionID;
	uint16 validCount;
	uint16 flags;
	int16 freePointer;

	unsigned int version;
	ChunkResolver *resolver;
	ScriptNames *lnam;
	Common::Array<ScriptContextMapEntry> sectionMap;
	Common::StableMap<uint32, Script *> scripts;

	ScriptContext(unsigned int version_, ChunkResolver *resolver_) : version(version_),
																   resolver(resolver_),
																   lnam(nullptr) {}

	void read(Common::SeekableReadStream &stream);
	bool validName(int id) const;
	Common::String getName(int id) const;
	void parseScripts();
};

/* ScriptContextMapEntry */

struct ScriptContextMapEntry {
	int32 unknown0;
	int32 sectionID;
	uint16 unknown1;
	uint16 unknown2;

	void read(Common::SeekableReadStream &stream);
};

} // namespace LingoDec

#endif // LINGODEC_CONTEXT_H
