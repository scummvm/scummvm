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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sci/sci.h"
#include "sci/resource.h"
#include "sci/util.h"
#include "sci/engine/features.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/script.h"

#include "common/util.h"

namespace Sci {

Script::Script() : SegmentObj(SEG_TYPE_SCRIPT) {
	_nr = 0;
	_buf = NULL;
	_bufSize = 0;
	_scriptSize = 0;
	_heapSize = 0;

	_synonyms = NULL;
	_heapStart = NULL;
	_exportTable = NULL;

	_localsOffset = 0;
	_localsSegment = 0;
	_localsBlock = NULL;
	_localsCount = 0;

	_markedAsDeleted = false;
}

Script::~Script() {
	freeScript();
}

void Script::freeScript() {
	free(_buf);
	_buf = NULL;
	_bufSize = 0;

	_objects.clear();
}

void Script::init(int script_nr, ResourceManager *resMan) {
	Resource *script = resMan->findResource(ResourceId(kResourceTypeScript, script_nr), 0);

	_localsOffset = 0;
	_localsBlock = NULL;
	_localsCount = 0;

	_markedAsDeleted = false;

	_nr = script_nr;
	_buf = 0;
	_heapStart = 0;

	_scriptSize = script->size;
	_bufSize = script->size;
	_heapSize = 0;

	_lockers = 1;

	if (getSciVersion() == SCI_VERSION_0_EARLY) {
		_bufSize += READ_LE_UINT16(script->data) * 2;
	} else if (getSciVersion() >= SCI_VERSION_1_1) {
		/**
		 * In SCI11, the heap was in a separate space from the script.
		 * We append it to the end of the script, and adjust addressing accordingly.
		 * However, since we address the heap with a 16-bit pointer, the combined
		 * size of the stack and the heap must be 64KB. So far this has worked
		 * for SCI11, SCI2 and SCI21 games. SCI3 games use a different script format,
		 * and theoretically they can exceed the 64KB boundary using relocation.
		 */
		Resource *heap = resMan->findResource(ResourceId(kResourceTypeHeap, script_nr), 0);
		_bufSize += heap->size;
		_heapSize = heap->size;

		// Ensure that the start of the heap resource can be word-aligned.
		if (script->size & 2) {
			_bufSize++;
			_scriptSize++;
		}

		// As mentioned above, the script and the heap together should not exceed 64KB
		if (script->size + heap->size > 65535)
			error("Script and heap sizes combined exceed 64K. This means a fundamental "
					"design bug was made regarding SCI1.1 and newer games.\nPlease "
					"report this error to the ScummVM team");
	}
}

void Script::load(ResourceManager *resMan) {
	Resource *script = resMan->findResource(ResourceId(kResourceTypeScript, _nr), 0);
	assert(script != 0);

	_buf = (byte *)malloc(_bufSize);
	assert(_buf);

	assert(_bufSize >= script->size);
	memcpy(_buf, script->data, script->size);

	if (getSciVersion() >= SCI_VERSION_1_1) {
		Resource *heap = resMan->findResource(ResourceId(kResourceTypeHeap, _nr), 0);
		assert(heap != 0);

		_heapStart = _buf + _scriptSize;

		assert(_bufSize - _scriptSize <= heap->size);
		memcpy(_heapStart, heap->data, heap->size);
	}

	_exportTable = 0;
	_numExports = 0;
	_synonyms = 0;
	_numSynonyms = 0;
	
	if (getSciVersion() >= SCI_VERSION_1_1) {
		if (READ_LE_UINT16(_buf + 1 + 5) > 0) {	// does the script have an export table?
			_exportTable = (const uint16 *)(_buf + 1 + 5 + 2);
			_numExports = READ_SCI11ENDIAN_UINT16(_exportTable - 1);
		}
		_localsOffset = _scriptSize + 4;
		_localsCount = READ_SCI11ENDIAN_UINT16(_buf + _localsOffset - 2);
	} else {
		_exportTable = (const uint16 *)findBlock(SCI_OBJ_EXPORTS);
		if (_exportTable) {
			_numExports = READ_SCI11ENDIAN_UINT16(_exportTable + 1);
			_exportTable += 3;	// skip header plus 2 bytes (_exportTable is a uint16 pointer)
		}
		_synonyms = findBlock(SCI_OBJ_SYNONYMS);
		if (_synonyms) {
			_numSynonyms = READ_SCI11ENDIAN_UINT16(_synonyms + 2) / 4;
			_synonyms += 4;	// skip header
		}
		const byte* localsBlock = findBlock(SCI_OBJ_LOCALVARS);
		if (localsBlock) {
			_localsOffset = localsBlock - _buf + 4;
			_localsCount = (READ_LE_UINT16(_buf + _localsOffset - 2) - 4) >> 1;	// half block size
		}
	}

	if (getSciVersion() > SCI_VERSION_0_EARLY) {
		// Does the script actually have locals? If not, set the locals offset to 0
		if (!_localsCount)
			_localsOffset = 0;

		if (_localsOffset + _localsCount * 2 + 1 >= (int)_bufSize) {
			error("Locals extend beyond end of script: offset %04x, count %d vs size %d", _localsOffset, _localsCount, _bufSize);
			_localsCount = (_bufSize - _localsOffset) >> 1;
		}
	} else {
		// Old script block. There won't be a localvar block in this case.
		// Instead, the script starts with a 16 bit int specifying the
		// number of locals we need; these are then allocated and zeroed.
		_localsCount = READ_LE_UINT16(_buf);
		_localsOffset = -_localsCount * 2; // Make sure it's invalid
	}
}

Object *Script::allocateObject(uint16 offset) {
	return &_objects[offset];
}

Object *Script::getObject(uint16 offset) {
	if (_objects.contains(offset))
		return &_objects[offset];
	else
		return 0;
}

const Object *Script::getObject(uint16 offset) const {
	if (_objects.contains(offset))
		return &_objects[offset];
	else
		return 0;
}

Object *Script::scriptObjInit(reg_t obj_pos, bool fullObjectInit) {
	Object *obj;

	if (getSciVersion() < SCI_VERSION_1_1 && fullObjectInit)
		obj_pos.offset += 8;	// magic offset (SCRIPT_OBJECT_MAGIC_OFFSET)

	VERIFY(obj_pos.offset < _bufSize, "Attempt to initialize object beyond end of script\n");

	obj = allocateObject(obj_pos.offset);

	VERIFY(obj_pos.offset + kOffsetFunctionArea < (int)_bufSize, "Function area pointer stored beyond end of script\n");

	obj->init(_buf, obj_pos, fullObjectInit);

	return obj;
}

void Script::scriptObjRemove(reg_t obj_pos) {
	if (getSciVersion() < SCI_VERSION_1_1)
		obj_pos.offset += 8;

	_objects.erase(obj_pos.toUint16());
}

// This helper function is used by Script::relocateLocal and Object::relocate
// Duplicate in segment.cpp and script.cpp
static bool relocateBlock(Common::Array<reg_t> &block, int block_location, SegmentId segment, int location, size_t scriptSize) {
	int rel = location - block_location;

	if (rel < 0)
		return false;

	uint idx = rel >> 1;

	if (idx >= block.size())
		return false;

	if (rel & 1) {
		error("Attempt to relocate odd variable #%d.5e (relative to %04x)\n", idx, block_location);
		return false;
	}
	block[idx].segment = segment; // Perform relocation
	if (getSciVersion() >= SCI_VERSION_1_1)
		block[idx].offset += scriptSize;

	return true;
}

bool Script::relocateLocal(SegmentId segment, int location) {
	if (_localsBlock)
		return relocateBlock(_localsBlock->_locals, _localsOffset, segment, location, _scriptSize);
	else
		return false;
}

void Script::relocate(reg_t block) {
	byte *heap = _buf;
	uint16 heapSize = (uint16)_bufSize;
	uint16 heapOffset = 0;

	if (getSciVersion() >= SCI_VERSION_1_1) {
		heap = _heapStart;
		heapSize = (uint16)_heapSize;
		heapOffset = _scriptSize;
	}

	VERIFY(block.offset < (uint16)heapSize && READ_SCI11ENDIAN_UINT16(heap + block.offset) * 2 + block.offset < (uint16)heapSize,
	       "Relocation block outside of script\n");

	int count = READ_SCI11ENDIAN_UINT16(heap + block.offset);
	int exportIndex = 0;
	int pos = 0;

	for (int i = 0; i < count; i++) {
		pos = READ_SCI11ENDIAN_UINT16(heap + block.offset + 2 + (exportIndex * 2)) + heapOffset;
		// This occurs in SCI01/SCI1 games where usually one export value
		// is zero. It seems that in this situation, we should skip the
		// export and move to the next one, though the total count of valid
		// exports remains the same
		if (!pos) {
			exportIndex++;
			pos = READ_SCI11ENDIAN_UINT16(heap + block.offset + 2 + (exportIndex * 2)) + heapOffset;
			if (!pos)
				error("Script::relocate(): Consecutive zero exports found");
		}

		// In SCI0-SCI1, script local variables, objects and code are relocated. We only relocate
		// locals and objects here, and ignore relocation of code blocks. In SCI1.1 and newer
		// versions, only locals and objects are relocated.
		if (!relocateLocal(block.segment, pos)) {
			// Not a local? It's probably an object or code block. If it's an object, relocate it.
			const ObjMap::iterator end = _objects.end();
			for (ObjMap::iterator it = _objects.begin(); it != end; ++it)
				if (it->_value.relocate(block.segment, pos, _scriptSize))
					break;
		}

		exportIndex++;
	}
}

void Script::incrementLockers() {
	_lockers++;
}

void Script::decrementLockers() {
	if (_lockers > 0)
		_lockers--;
}

int Script::getLockers() const {
	return _lockers;
}

void Script::setLockers(int lockers) {
	_lockers = lockers;
}

uint16 Script::validateExportFunc(int pubfunct) {
	bool exportsAreWide = (g_sci->_features->detectLofsType() == SCI_VERSION_1_MIDDLE);

	if (_numExports <= pubfunct) {
		error("validateExportFunc(): pubfunct is invalid");
		return 0;
	}

	if (exportsAreWide)
		pubfunct *= 2;
	uint16 offset = READ_SCI11ENDIAN_UINT16(_exportTable + pubfunct);
	VERIFY(offset < _bufSize, "invalid export function pointer");

	return offset;
}

byte *Script::findBlock(int type) {
	byte *buf = _buf;
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

	if (oldScriptHeader)
		buf += 2;

	do {
		int seekerType = READ_LE_UINT16(buf);

		if (seekerType == 0)
			break;
		if (seekerType == type)
			return buf;

		int seekerSize = READ_LE_UINT16(buf + 2);
		assert(seekerSize > 0);
		buf += seekerSize;
	} while (1);

	return NULL;
}

// memory operations

void Script::mcpyInOut(int dst, const void *src, size_t n) {
	if (_buf) {
		assert(dst + n <= _bufSize);
		memcpy(_buf + dst, src, n);
	}
}

bool Script::isValidOffset(uint16 offset) const {
	return offset < _bufSize;
}

SegmentRef Script::dereference(reg_t pointer) {
	if (pointer.offset > _bufSize) {
		error("Script::dereference(): Attempt to dereference invalid pointer %04x:%04x into script segment (script size=%d)",
				  PRINT_REG(pointer), (uint)_bufSize);
		return SegmentRef();
	}

	SegmentRef ret;
	ret.isRaw = true;
	ret.maxSize = _bufSize - pointer.offset;
	ret.raw = _buf + pointer.offset;
	return ret;
}

void SegManager::scriptInitialiseLocals(SegmentId segmentId) {
	Script *scr = getScript(segmentId);

	LocalVariables *locals = allocLocalsSegment(scr);
	if (locals) {
		if (getSciVersion() > SCI_VERSION_0_EARLY) {
			const byte *base = (const byte *)(scr->_buf + scr->getLocalsOffset());

			for (uint16 i = 0; i < scr->getLocalsCount(); i++)
				locals->_locals[i] = make_reg(0, READ_SCI11ENDIAN_UINT16(base + i * 2));
		} else {
			// In SCI0 early, locals are set at run time, thus zero them all here
			for (uint16 i = 0; i < scr->getLocalsCount(); i++)
				locals->_locals[i] = NULL_REG;
		}
	}
}

void SegManager::scriptInitialiseClasses(SegmentId seg) {
	Script *scr = getScript(seg);
	const byte *seeker = 0;
	uint16 mult = 0;
	
	if (getSciVersion() >= SCI_VERSION_1_1) {
		seeker = scr->_heapStart + 4 + READ_SCI11ENDIAN_UINT16(scr->_heapStart + 2) * 2;
		mult = 2;
	} else {
		seeker = scr->findBlock(SCI_OBJ_CLASS);
		mult = 1;
	}

	if (!seeker)
		return;

	while (true) {
		// In SCI0-SCI1, this is the segment type. In SCI11, it's a marker (0x1234)
		uint16 marker = READ_SCI11ENDIAN_UINT16(seeker);
		bool isClass;
		uint16 classpos = seeker - scr->_buf;
		int16 species;

		if (!marker)
			break;

		if (getSciVersion() >= SCI_VERSION_1_1) {
			isClass = (READ_SCI11ENDIAN_UINT16(seeker + 14) & kInfoFlagClass);	// -info- selector
			species = READ_SCI11ENDIAN_UINT16(seeker + 10);
		} else {
			isClass = (marker == SCI_OBJ_CLASS);
			species = READ_SCI11ENDIAN_UINT16(seeker + 12);
			classpos += 12;
		}

		if (isClass) {
			// WORKAROUND for an invalid species access in the demo of LSL2
			if (g_sci->getGameId() == GID_LSL2 && g_sci->isDemo() && species == (int)classTableSize())
				resizeClassTable(classTableSize() + 1);

			if (species < 0 || species >= (int)classTableSize())
				error("Invalid species %d(0x%x) not in interval [0,%d) while instantiating script %d\n",
						  species, species, classTableSize(), scr->_nr);

			setClassOffset(species, make_reg(seg, classpos));
		}

		seeker += READ_SCI11ENDIAN_UINT16(seeker + 2) * mult;
	}
}

void SegManager::scriptInitialiseObjectsSci0(SegmentId seg) {
	Script *scr = getScript(seg);
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);
	const byte *seeker = scr->_buf + (oldScriptHeader ? 2 : 0);

	do {
		uint16 objType = READ_SCI11ENDIAN_UINT16(seeker);
		if (!objType)
			break;

		switch (objType) {
		case SCI_OBJ_OBJECT:
		case SCI_OBJ_CLASS:
			{
				reg_t addr = make_reg(seg, seeker - scr->_buf + 4);
				Object *obj = scr->scriptObjInit(addr);
				obj->initSpecies(this, addr);

				if (!obj->initBaseObject(this, addr)) {
					// Script 202 of KQ5 French has an invalid object. This is non-fatal.
					warning("Failed to locate base object for object at %04X:%04X; skipping", PRINT_REG(addr));
					scr->scriptObjRemove(addr);
				}
			}
			break;

		default:
			break;
		}

		seeker += READ_SCI11ENDIAN_UINT16(seeker + 2);
	} while ((uint32)(seeker - scr->_buf) < scr->getScriptSize() - 2);
}

void SegManager::scriptInitialiseObjectsSci11(SegmentId seg) {
	Script *scr = getScript(seg);
	const byte *seeker = scr->_heapStart + 4 + READ_SCI11ENDIAN_UINT16(scr->_heapStart + 2) * 2;

	while (READ_SCI11ENDIAN_UINT16(seeker) == SCRIPT_OBJECT_MAGIC_NUMBER) {
		reg_t reg = make_reg(seg, seeker - scr->_buf);
		Object *obj = scr->scriptObjInit(reg);

		// Copy base from species class, as we need its selector IDs
		obj->setSuperClassSelector(
			getClassAddress(obj->getSuperClassSelector().offset, SCRIPT_GET_LOCK, NULL_REG));

		// If object is instance, get -propDict- from class and set it for this object
		//  This is needed for ::isMemberOf() to work.
		// Example testcase - room 381 of sq4cd - if isMemberOf() doesn't work, talk-clicks on the robot will act like
		//                     clicking on ego
		if (!obj->isClass()) {
			reg_t classObject = obj->getSuperClassSelector();
			Object *classObj = getObject(classObject);
			obj->setPropDictSelector(classObj->getPropDictSelector());
		}

		// Set the -classScript- selector to the script number.
		// FIXME: As this selector is filled in at run-time, it is likely
		// that it is supposed to hold a pointer. The Obj::isKindOf method
		// uses this selector together with -propDict- to compare classes.
		// For the purpose of Obj::isKindOf, using the script number appears
		// to be sufficient.
		obj->setClassScriptSelector(make_reg(0, scr->_nr));

		seeker += READ_SCI11ENDIAN_UINT16(seeker + 2) * 2;
	}
}

int script_instantiate(ResourceManager *resMan, SegManager *segMan, int scriptNum) {
	SegmentId segmentId = segMan->getScriptSegment(scriptNum);
	Script *scr = segMan->getScriptIfLoaded(segmentId);
	if (scr) {
		if (!scr->isMarkedAsDeleted()) {
			scr->incrementLockers();
			return segmentId;
		} else {
			scr->freeScript();
		}
	} else {
		scr = segMan->allocateScript(scriptNum, &segmentId);
	}

	scr->init(scriptNum, resMan);
	scr->load(resMan);
	segMan->scriptInitialiseLocals(segmentId);
	segMan->scriptInitialiseClasses(segmentId);

	if (getSciVersion() >= SCI_VERSION_1_1) {
		segMan->scriptInitialiseObjectsSci11(segmentId);
		scr->relocate(make_reg(segmentId, READ_SCI11ENDIAN_UINT16(scr->_heapStart)));
	} else {
		segMan->scriptInitialiseObjectsSci0(segmentId);
		byte *relocationBlock = scr->findBlock(SCI_OBJ_POINTERS);
		if (relocationBlock)
			scr->relocate(make_reg(segmentId, relocationBlock - scr->_buf + 4));
	}

	return segmentId;
}

void script_uninstantiate_sci0(SegManager *segMan, int script_nr, SegmentId seg) {
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);
	reg_t reg = make_reg(seg, oldScriptHeader ? 2 : 0);
	int objType, objLength = 0;
	Script *scr = segMan->getScript(seg);

	// Make a pass over the object in order uninstantiate all superclasses

	do {
		reg.offset += objLength; // Step over the last checked object

		objType = READ_SCI11ENDIAN_UINT16(scr->_buf + reg.offset);
		if (!objType)
			break;
		objLength = READ_SCI11ENDIAN_UINT16(scr->_buf + reg.offset + 2);

		reg.offset += 4; // Step over header

		if ((objType == SCI_OBJ_OBJECT) || (objType == SCI_OBJ_CLASS)) { // object or class?
			reg.offset += 8;	// magic offset (SCRIPT_OBJECT_MAGIC_OFFSET)
			int16 superclass = READ_SCI11ENDIAN_UINT16(scr->_buf + reg.offset + 2);

			if (superclass >= 0) {
				int superclass_script = segMan->getClass(superclass).script;

				if (superclass_script == script_nr) {
					if (scr->getLockers())
						scr->decrementLockers();  // Decrease lockers if this is us ourselves
				} else
					script_uninstantiate(segMan, superclass_script);
				// Recurse to assure that the superclass lockers number gets decreased
			}

			reg.offset += SCRIPT_OBJECT_MAGIC_OFFSET;
		} // if object or class

		reg.offset -= 4; // Step back on header

	} while (objType != 0);
}

void script_uninstantiate(SegManager *segMan, int script_nr) {
	SegmentId segment = segMan->getScriptSegment(script_nr);
	Script *scr = segMan->getScriptIfLoaded(segment);

	if (!scr) {   // Is it already loaded?
		//warning("unloading script 0x%x requested although not loaded", script_nr);
		// This is perfectly valid SCI behaviour
		return;
	}

	scr->decrementLockers();   // One less locker

	if (scr->getLockers() > 0)
		return;

	// Free all classtable references to this script
	for (uint i = 0; i < segMan->classTableSize(); i++)
		if (segMan->getClass(i).reg.segment == segment)
			segMan->setClassOffset(i, NULL_REG);

	if (getSciVersion() < SCI_VERSION_1_1)
		script_uninstantiate_sci0(segMan, script_nr, segment);
	// FIXME: Add proper script uninstantiation for SCI 1.1

	if (!scr->getLockers()) {
		// The actual script deletion seems to be done by SCI scripts themselves
		scr->markDeleted();
		debugC(kDebugLevelScripts, "Unloaded script 0x%x.", script_nr);
	}
}


} // End of namespace Sci
