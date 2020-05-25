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

#ifndef SCI_ENGINE_VM_HOOKS_H
#define SCI_ENGINE_VM_HOOKS_H

#include "sci/engine/vm.h"

namespace Sci {


#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))
#define HOOKARRAY(x) x, ARRAYSIZE(x)

/** _hooksMap keys are built from script number and PC's offset */
struct HookHashKey {
	int scriptNumber;
	uint32 offset;

	uint64 hash();

	bool operator==(const HookHashKey &other) const {
		return scriptNumber == other.scriptNumber && offset == other.offset;
	}


};

/** _hooksMap value entry */
struct HookEntry {
	/** These are used to make sure that the PC is indeed the requested place */
	const char *objName;
	Common::String selector;
	int exportId;
	const char *opcodeName;

	/** If all the previous match, patch */
	const byte *patch;
	uint patchSize;
};

/** Used for allGamesHooks - from it we build the specific _hooksMap */
struct GeneralHookEntry {
	SciGameId gameId;
	Common::Language language;			// language to be patched. UNK_LANG means to patch all languages
	HookHashKey key;
	HookEntry entry;
};

/** Hash key equality function */
struct HookHash : public Common::UnaryFunction<HookHashKey, uint64> {
	uint64 operator()(HookHashKey val) const { return val.hash(); }
};

/** VM Hook mechanism */
class VmHooks {
public:
	VmHooks();

	/** Called just before executing opcode, to check if there is a requried hook */
	void vm_hook_before_exec(Sci::EngineState *s);

	byte *data();

	bool isActive(Sci::EngineState *s);

	void advance(int offset);

private:
	/** Hash map of all game's hooks */
	Common::HashMap<HookHashKey, HookEntry, HookHash> _hooksMap;

	Common::Array<byte> _hookScriptData;

	/** Used to avoid double patching in row, and to support `call`ing */
	reg_t _lastPc;

	/** Raised after patch has ended, to avoid confusion with situation of returning from a `call` to the patch, and continue execution of original code */
	bool _just_finished;

	/** Location inside patch */
	int _location;
};


} // End of namespace Sci

#endif // SCI_ENGINE_VM_HOOKS_H
