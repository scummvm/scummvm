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

namespace StandardNames {
extern const char *const opcodeNamesS[];
extern const char *const opcodeNamesM[];
extern const char *const binaryOpNames[];
extern const char *const chunkTypeNames[];
extern const char *const putTypeNames[];

extern const char *const moviePropertyNames[];
extern const char *const whenEventNames[];
extern const char *const menuPropertyNames[];
extern const char *const menuItemPropertyNames[];
extern const char *const soundPropertyNames[];
extern const char *const spritePropertyNames[];
extern const char *const animationPropertyNames[];
extern const char *const animation2PropertyNames[];
extern const char *const memberPropertyNames[];

Common::String getOpcodeName(byte id);
}

/* ScriptNames */

struct ScriptNames {
	int32 unknown0;
	int32 unknown1;
	uint32 len1;
	uint32 len2;
	uint16 namesOffset;
	uint16 namesCount;
	Common::Array<Common::String> names;

	unsigned int version;

	ScriptNames(unsigned int version_) : version(version_) {}
	void read(Common::SeekableReadStream &stream);
	bool validName(int id) const;
	Common::String getName(int id) const;
};

} // namespace LingoDec

#endif // LINGODEC_NAMES_H
