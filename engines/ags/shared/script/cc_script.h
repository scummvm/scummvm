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

#ifndef AGS_SHARED_SCRIPT_CC_SCRIPT_H
#define AGS_SHARED_SCRIPT_CC_SCRIPT_H

#include "ags/lib/std/memory.h"
#include "ags/shared/core/types.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Stream;
}
}
using namespace AGS; // FIXME later

struct ccScript {
public:
	char *globaldata;
	int32_t globaldatasize;
	int32_t *code;                // executable byte-code, 32-bit per op or arg
	int32_t codesize; // TODO: find out if we can make it size_t
	char *strings;
	int32_t stringssize;
	char *fixuptypes;             // global data/string area/ etc
	int32_t *fixups;              // code array index to fixup (in ints)
	int numfixups;
	int importsCapacity;
	char **imports;
	int numimports;
	int exportsCapacity;
	char **exports;   // names of exports
	int32_t *export_addr; // high byte is type; low 24-bits are offset
	int numexports;
	int instances;
	// 'sections' allow the interpreter to find out which bit
	// of the code came from header files, and which from the main file
	char **sectionNames;
	int32_t *sectionOffsets;
	int numSections;
	int capacitySections;

	static ccScript *CreateFromStream(Shared::Stream *in);

	ccScript();
	ccScript(const ccScript &src);
	virtual ~ccScript(); // there are few derived classes, so dtor should be virtual

	// write the script to disk (after compiling)
	void        Write(Shared::Stream *out);
	// read back a script written with Write
	bool        Read(Shared::Stream *in);
	const char *GetSectionName(int32_t offset);

protected:
	// free the memory occupied by the script - do NOT attempt to run the
	// script after calling this function
	void        Free();
};

typedef std::shared_ptr<ccScript> PScript;

} // namespace AGS3

#endif
