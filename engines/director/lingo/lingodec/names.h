/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef LINGODEC_NAMES_H
#define LINGODEC_NAMES_H

#include "common/array.h"
#include "common/stablemap.h"

namespace Common {
class SeekableReadStream;
class String;
}

namespace LingoDec {

/* StandardNames */

struct StandardNames {
	static Common::StableMap<unsigned int, Common::String> opcodeNames;
	static Common::StableMap<unsigned int, Common::String> binaryOpNames;
	static Common::StableMap<unsigned int, Common::String> chunkTypeNames;
	static Common::StableMap<unsigned int, Common::String> putTypeNames;
	static Common::StableMap<unsigned int, Common::String> moviePropertyNames;
	static Common::StableMap<unsigned int, Common::String> whenEventNames;
	static Common::StableMap<unsigned int, Common::String> timeNames;
	static Common::StableMap<unsigned int, Common::String> menuPropertyNames;
	static Common::StableMap<unsigned int, Common::String> menuItemPropertyNames;
	static Common::StableMap<unsigned int, Common::String> soundPropertyNames;
	static Common::StableMap<unsigned int, Common::String> spritePropertyNames;
	static Common::StableMap<unsigned int, Common::String> animationPropertyNames;
	static Common::StableMap<unsigned int, Common::String> animation2PropertyNames;
	static Common::StableMap<unsigned int, Common::String> memberPropertyNames;

	static Common::String getOpcodeName(uint8_t id);
	static Common::String getName(const Common::StableMap<unsigned int, Common::String> &nameMap, unsigned int id);
};

/* ScriptNames */

struct ScriptNames {
	int32_t unknown0;
	int32_t unknown1;
	uint32_t len1;
	uint32_t len2;
	uint16_t namesOffset;
	uint16_t namesCount;
	Common::Array<Common::String> names;

	unsigned int version;

	ScriptNames(unsigned int version) : version(version) {}
	void read(Common::SeekableReadStream &stream);
	bool validName(int id) const;
	Common::String getName(int id) const;
};

} // namespace LingoDec

#endif // LINGODEC_NAMES_H
