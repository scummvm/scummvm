/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "common/stream.h"
#include "common/util.h"
#include "./context.h"
#include "./names.h"
#include "./resolver.h"
#include "./script.h"

namespace LingoDec {

struct ScriptContextMapEntry;

/* ScriptContext */

void ScriptContext::read(Common::SeekableReadStream &stream) {
	// Lingo scripts are always big endian regardless of file endianness
	unknown0 = stream.readSint32BE();
	unknown1 = stream.readSint32BE();
	entryCount = stream.readUint32BE();
	entryCount2 = stream.readUint32BE();
	entriesOffset = stream.readUint16BE();
	unknown2 = stream.readSint16BE();
	unknown3 = stream.readSint32BE();
	unknown4 = stream.readSint32BE();
	unknown5 = stream.readSint32BE();
	lnamSectionID = stream.readSint32BE();
	validCount = stream.readUint16BE();
	flags = stream.readUint16BE();
	freePointer = stream.readSint16BE();

	stream.seek(entriesOffset);
	sectionMap.resize(entryCount);
	for (auto &entry : sectionMap) {
		entry.read(stream);
	}

	lnam = resolver->getScriptNames(lnamSectionID);
	for (uint32 i = 1; i <= entryCount; i++) {
		auto section = sectionMap[i - 1];
		if (section.sectionID > -1) {
			Script *script = resolver->getScript(section.sectionID);
			script->setContext(this);
			scripts[i] = script;
		}
	}

	for (auto it = scripts.begin(); it != scripts.end(); ++it) {
		Script *script = it->second;
		if (script->isFactory()) {
			Script *parent = scripts[script->parentNumber + 1];
			parent->factories.push_back(script);
		}
	}
}

bool ScriptContext::validName(int id) const {
	return lnam->validName(id);
}

Common::String ScriptContext::getName(int id) const {
	return lnam->getName(id);
}

void ScriptContext::parseScripts() {
	for (auto it = scripts.begin(); it != scripts.end(); ++it) {
		it->second->parse();
	}
}

/* ScriptContextMapEntry */

void ScriptContextMapEntry::read(Common::SeekableReadStream &stream) {
	unknown0 = stream.readSint32BE();
	sectionID = stream.readSint32BE();
	unknown1 = stream.readUint16BE();
	unknown2 = stream.readUint16BE();
}

} // namespace LingoDec
