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
#include "sci/engine/seg_manager.h"
#include "sci/engine/state.h"

namespace Sci {


/**
 * Verify the the given condition is true, output the message if condition is false, and exit.
 * @param cond	condition to be verified
 * @param msg	the message to be printed if condition fails
 */
#define VERIFY( cond, msg ) if (!(cond)) {\
		error("%s, line, %d, %s", __FILE__, __LINE__, msg); \
	}


#define DEFAULT_SCRIPTS 32
#define DEFAULT_OBJECTS 8	    // default # of objects per script
#define DEFAULT_OBJECTS_INCREMENT 4 // Number of additional objects to instantiate if we're running out of them

#undef DEBUG_segMan // Define to turn on debugging

SegManager::SegManager(ResourceManager *resMan) {
	_heap.push_back(0);

	Clones_seg_id = 0;
	Lists_seg_id = 0;
	Nodes_seg_id = 0;
	Hunks_seg_id = 0;

	_exportsAreWide = false;
	_resMan = resMan;

	int result = 0;

	result = createClassTable();

	if (result)
		error("SegManager: Failed to initialize class table");
}

// Destroy the object, free the memorys if allocated before
SegManager::~SegManager() {
	// Free memory
	for (uint i = 0; i < _heap.size(); i++) {
		if (_heap[i])
			deallocate(i, false);
	}
}

SegmentId SegManager::findFreeSegment() const {
	// FIXME: This is a very crude approach: We find a free segment id by scanning
	// from the start. This can be slow if the number of segments becomes large.
	// Optimizations are possible and easy, but I refrain from doing any until this
	// refactoring is complete.
	// One very simple yet probably effective approach would be to add
	// "int firstFreeSegment", which is updated when allocating/freeing segments.
	// There are some scenarios where it performs badly, but we should first check
	// whether those occur at all. If so, there are easy refinements that deal
	// with this. Finally, we could switch to a more elaborate scheme,
	// such as keeping track of all free segments. But I doubt this is necessary.
	uint seg = 1;
	while (seg < _heap.size() && _heap[seg]) {
		++seg;
	}
	assert(seg < 65536);
	return seg;
}

SegmentObj *SegManager::allocSegment(SegmentObj *mem, SegmentId *segid) {
	// Find a free segment
	*segid = findFreeSegment();

	if (!mem)
		error("SegManager: invalid mobj");

	// ... and put it into the (formerly) free segment.
	if (*segid >= (int)_heap.size()) {
		assert(*segid == (int)_heap.size());
		_heap.push_back(0);
	}
	_heap[*segid] = mem;

	return mem;
}

Script *SegManager::allocateScript(int script_nr, SegmentId *seg_id) {
	// Check if the script already has an allocated segment. If it
	// does, return that segment.
	*seg_id = _scriptSegMap.getVal(script_nr, 0);
	if (*seg_id > 0) {
		return (Script *)_heap[*seg_id];
	}

	// allocate the SegmentObj
	SegmentObj *mem = allocSegment(new Script(), seg_id);

	// Add the script to the "script id -> segment id" hashmap
	_scriptSegMap[script_nr] = *seg_id;

	return (Script *)mem;
}

void Script::setScriptSize(int script_nr, ResourceManager *resMan) {
	Resource *script = resMan->findResource(ResourceId(kResourceTypeScript, script_nr), 0);
	Resource *heap = resMan->findResource(ResourceId(kResourceTypeHeap, script_nr), 0);
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

	_scriptSize = script->size;
	_heapSize = 0; // Set later

	if (!script || (getSciVersion() >= SCI_VERSION_1_1 && !heap)) {
		error("SegManager::setScriptSize: failed to load %s", !script ? "script" : "heap");
	}
	if (oldScriptHeader) {
		_bufSize = script->size + READ_LE_UINT16(script->data) * 2;
		//locals_size = READ_LE_UINT16(script->data) * 2;
	} else if (getSciVersion() < SCI_VERSION_1_1) {
		_bufSize = script->size;
	} else {
		_bufSize = script->size + heap->size;
		_heapSize = heap->size;

		// Ensure that the start of the heap resource can be word-aligned.
		if (script->size & 2) {
			_bufSize++;
			_scriptSize++;
		}

		if (_bufSize > 65535) {
			error("Script and heap sizes combined exceed 64K."
			          "This means a fundamental design bug was made in SCI\n"
			          "regarding SCI1.1 games.\nPlease report this so it can be"
			          "fixed in the next major version");
			return;
		}
	}
}

int SegManager::deallocate(SegmentId seg, bool recursive) {
	SegmentObj *mobj;
	VERIFY(check(seg), "invalid seg id");

	mobj = _heap[seg];

	if (mobj->getType() == SEG_TYPE_SCRIPT) {
		Script *scr = (Script *)mobj;
		_scriptSegMap.erase(scr->_nr);
		if (recursive && scr->_localsSegment)
			deallocate(scr->_localsSegment, recursive);
	}

	delete mobj;
	_heap[seg] = NULL;

	return 1;
}

bool SegManager::isHeapObject(reg_t pos) {
	Object *obj = getObject(pos);
	if (obj == NULL)
		return false;
	if (obj->_flags & OBJECT_FLAG_FREED)
		return false;
	Script *scr = getScriptIfLoaded(pos.segment);
	return !(scr && scr->_markedAsDeleted);
}

void SegManager::deallocateScript(int script_nr) {
	SegmentId seg = getScriptSegment(script_nr);
	deallocate(seg, true);
}

Script *SegManager::getScript(const SegmentId seg) {
	if (seg < 1 || (uint)seg >= _heap.size()) {
		error("SegManager::getScript(): seg id %x out of bounds", seg);
	}
	if (!_heap[seg]) {
		error("SegManager::getScript(): seg id %x is not in memory", seg);
	}
	if (_heap[seg]->getType() != SEG_TYPE_SCRIPT) {
		error("SegManager::getScript(): seg id %x refers to type %d != SEG_TYPE_SCRIPT", seg, _heap[seg]->getType());
	}
	return (Script *)_heap[seg];
}

Script *SegManager::getScriptIfLoaded(const SegmentId seg) {
	if (seg < 1 || (uint)seg >= _heap.size() || !_heap[seg] || _heap[seg]->getType() != SEG_TYPE_SCRIPT)
		return 0;
	return (Script *)_heap[seg];
}

SegmentId SegManager::findSegmentByType(int type) {
	for (uint i = 0; i < _heap.size(); i++)
		if (_heap[i] && _heap[i]->getType() == type)
			return i;
	return 0;
}

SegmentObj *SegManager::getSegmentObj(SegmentId seg) {
	if (seg < 1 || (uint)seg >= _heap.size() || !_heap[seg])
		return 0;
	return _heap[seg];
}

SegmentType SegManager::getSegmentType(SegmentId seg) {
	if (seg < 1 || (uint)seg >= _heap.size() || !_heap[seg])
		return SEG_TYPE_INVALID;
	return _heap[seg]->getType();
}

Object *SegManager::getObject(reg_t pos) {
	SegmentObj *mobj = getSegmentObj(pos.segment);
	Object *obj = NULL;

	if (mobj != NULL) {
		if (mobj->getType() == SEG_TYPE_CLONES) {
			CloneTable *ct = (CloneTable *)mobj;
			if (ct->isValidEntry(pos.offset))
				obj = &(ct->_table[pos.offset]);
		} else if (mobj->getType() == SEG_TYPE_SCRIPT) {
			Script *scr = (Script *)mobj;
			if (pos.offset <= scr->_bufSize && pos.offset >= -SCRIPT_OBJECT_MAGIC_OFFSET
			        && RAW_IS_OBJECT(scr->_buf + pos.offset)) {
				obj = scr->getObject(pos.offset);
			}
		}
	}

	return obj;
}

const char *SegManager::getObjectName(reg_t pos) {
	Object *obj = getObject(pos);
	if (!obj)
		return "<no such object>";

	reg_t nameReg = obj->getNameSelector();
	if (nameReg.isNull())
		return "<no name>";

	const char *name = derefString(nameReg);
	if (!name)
		return "<invalid name>";

	return name;
}

// validate the seg
// return:
//	false - invalid seg
//	true  - valid seg
bool SegManager::check(SegmentId seg) {
	if (seg < 1 || (uint)seg >= _heap.size()) {
		return false;
	}
	if (!_heap[seg]) {
		warning("SegManager: seg %x is removed from memory, but not removed from hash_map", seg);
		return false;
	}
	return true;
}

void SegManager::setExportAreWide(bool flag) {
	_exportsAreWide = flag;
}

int Script::relocateBlock(Common::Array<reg_t> &block, int block_location, SegmentId segment, int location) {
	int rel = location - block_location;

	if (rel < 0)
		return 0;

	uint idx = rel >> 1;

	if (idx >= block.size())
		return 0;

	if (rel & 1) {
		warning("Attempt to relocate odd variable #%d.5e (relative to %04x)\n", idx, block_location);
		return 0;
	}
	block[idx].segment = segment; // Perform relocation
	if (getSciVersion() >= SCI_VERSION_1_1)
		block[idx].offset += _scriptSize;

	return 1;
}

int Script::relocateLocal(SegmentId segment, int location) {
	if (_localsBlock)
		return relocateBlock(_localsBlock->_locals, _localsOffset, segment, location);
	else
		return 0; // No hands, no cookies
}

int Script::relocateObject(Object &obj, SegmentId segment, int location) {
	return relocateBlock(obj._variables, obj._pos.offset, segment, location);
}

void Script::scriptAddCodeBlock(reg_t location) {
	CodeBlock cb;
	cb.pos = location;
	cb.size = READ_LE_UINT16(_buf + location.offset - 2);
	_codeBlocks.push_back(cb);
}

void Script::scriptRelocate(reg_t block) {
	VERIFY(block.offset < (uint16)_bufSize && READ_LE_UINT16(_buf + block.offset) * 2 + block.offset < (uint16)_bufSize,
	       "Relocation block outside of script\n");

	int count = READ_LE_UINT16(_buf + block.offset);

	for (int i = 0; i <= count; i++) {
		int pos = READ_LE_UINT16(_buf + block.offset + 2 + (i * 2));
		if (!pos)
			continue; // FIXME: A hack pending investigation

		if (!relocateLocal(block.segment, pos)) {
			bool done = false;
			uint k;

			ObjMap::iterator it;
			const ObjMap::iterator end = _objects.end();
			for (it = _objects.begin(); !done && it != end; ++it) {
				if (relocateObject(it->_value, block.segment, pos))
					done = true;
			}

			for (k = 0; !done && k < _codeBlocks.size(); k++) {
				if (pos >= _codeBlocks[k].pos.offset &&
				        pos < _codeBlocks[k].pos.offset + _codeBlocks[k].size)
					done = true;
			}

			if (!done) {
				printf("While processing relocation block %04x:%04x:\n", PRINT_REG(block));
				printf("Relocation failed for index %04x (%d/%d)\n", pos, i + 1, count);
				if (_localsBlock)
					printf("- locals: %d at %04x\n", _localsBlock->_locals.size(), _localsOffset);
				else
					printf("- No locals\n");
				for (it = _objects.begin(), k = 0; it != end; ++it, ++k)
					printf("- obj#%d at %04x w/ %d vars\n", k, it->_value._pos.offset, it->_value._variables.size());
				// SQ3 script 71 has broken relocation entries.
				printf("Trying to continue anyway...\n");
			}
		}
	}
}

void Script::heapRelocate(reg_t block) {
	VERIFY(block.offset < (uint16)_heapSize && READ_LE_UINT16(_heapStart + block.offset) * 2 + block.offset < (uint16)_bufSize,
	       "Relocation block outside of script\n");

	if (_relocated)
		return;
	_relocated = true;
	int count = READ_LE_UINT16(_heapStart + block.offset);

	for (int i = 0; i < count; i++) {
		int pos = READ_LE_UINT16(_heapStart + block.offset + 2 + (i * 2)) + _scriptSize;

		if (!relocateLocal(block.segment, pos)) {
			bool done = false;
			uint k;

			ObjMap::iterator it;
			const ObjMap::iterator end = _objects.end();
			for (it = _objects.begin(); !done && it != end; ++it) {
				if (relocateObject(it->_value, block.segment, pos))
					done = true;
			}

			if (!done) {
				printf("While processing relocation block %04x:%04x:\n", PRINT_REG(block));
				printf("Relocation failed for index %04x (%d/%d)\n", pos, i + 1, count);
				if (_localsBlock)
					printf("- locals: %d at %04x\n", _localsBlock->_locals.size(), _localsOffset);
				else
					printf("- No locals\n");
				for (it = _objects.begin(), k = 0; it != end; ++it, ++k)
					printf("- obj#%d at %04x w/ %d vars\n", k, it->_value._pos.offset, it->_value._variables.size());
				error("Breakpoint in %s, line %d", __FILE__, __LINE__);
			}
		}
	}
}

// return the seg if script_id is valid and in the map, else 0
SegmentId SegManager::getScriptSegment(int script_id) const {
	return _scriptSegMap.getVal(script_id, 0);
}

SegmentId SegManager::getScriptSegment(int script_nr, ScriptLoadType load) {
	SegmentId segment;

	if ((load & SCRIPT_GET_LOAD) == SCRIPT_GET_LOAD)
		script_instantiate(_resMan, this, script_nr);

	segment = getScriptSegment(script_nr);

	if (segment > 0) {
		if ((load & SCRIPT_GET_LOCK) == SCRIPT_GET_LOCK)
			getScript(segment)->incrementLockers();
	}
	return segment;
}

reg_t SegManager::getClassAddress(int classnr, ScriptLoadType lock, reg_t caller) {
	if (classnr == 0xffff)
		return NULL_REG;

	if (classnr < 0 || (int)_classtable.size() <= classnr || _classtable[classnr].script < 0) {
		error("[VM] Attempt to dereference class %x, which doesn't exist (max %x)", classnr, _classtable.size());
		return NULL_REG;
	} else {
		Class *the_class = &_classtable[classnr];
		if (!the_class->reg.segment) {
			getScriptSegment(the_class->script, lock);

			if (!the_class->reg.segment) {
				error("[VM] Trying to instantiate class %x by instantiating script 0x%x (%03d) failed;"
				          " Entering debugger.", classnr, the_class->script, the_class->script);
				return NULL_REG;
			}
		} else
			if (caller.segment != the_class->reg.segment)
				getScript(the_class->reg.segment)->incrementLockers();

		return the_class->reg;
	}
}

Object *Script::scriptObjInit0(reg_t obj_pos) {
	Object *obj;

	obj_pos.offset -= SCRIPT_OBJECT_MAGIC_OFFSET;
	VERIFY(obj_pos.offset < _bufSize, "Attempt to initialize object beyond end of script\n");

	obj = allocateObject(obj_pos.offset);

	VERIFY(obj_pos.offset + SCRIPT_FUNCTAREAPTR_OFFSET < (int)_bufSize, "Function area pointer stored beyond end of script\n");

	{
		byte *data = (byte *)(_buf + obj_pos.offset);
		uint16 *funct_area = (uint16 *)(data + READ_LE_UINT16(data + SCRIPT_FUNCTAREAPTR_OFFSET));
		int variables_nr;
		int functions_nr;
		int is_class;

		obj->_flags = 0;
		obj->_pos = obj_pos;

		VERIFY((byte *)funct_area < _buf + _bufSize, "Function area pointer references beyond end of script");

		variables_nr = READ_LE_UINT16(data + SCRIPT_SELECTORCTR_OFFSET);
		functions_nr = READ_LE_UINT16(funct_area - 1);
		is_class = READ_LE_UINT16(data + SCRIPT_INFO_OFFSET) & SCRIPT_INFO_CLASS;

		obj->base_method = funct_area;
		obj->base_vars = NULL;

		VERIFY((byte *)funct_area + functions_nr * 2
		       // add again for classes, since those also store selectors
		       + (is_class ? functions_nr * 2 : 0) < _buf + _bufSize, "Function area extends beyond end of script");

		obj->_variables.resize(variables_nr);

		obj->methods_nr = functions_nr;
		obj->base = _buf;
		obj->base_obj = data;

		for (int i = 0; i < variables_nr; i++)
			obj->_variables[i] = make_reg(0, READ_LE_UINT16(data + (i * 2)));
	}

	return obj;
}

Object *Script::scriptObjInit11(reg_t obj_pos) {
	Object *obj;

	VERIFY(obj_pos.offset < _bufSize, "Attempt to initialize object beyond end of script\n");

	obj = allocateObject(obj_pos.offset);

	VERIFY(obj_pos.offset + SCRIPT_FUNCTAREAPTR_OFFSET < (int)_bufSize, "Function area pointer stored beyond end of script\n");

	{
		byte *data = (byte *)(_buf + obj_pos.offset);
		uint16 *funct_area = (uint16 *)(_buf + READ_LE_UINT16(data + 6));
		uint16 *prop_area = (uint16 *)(_buf + READ_LE_UINT16(data + 4));
		int variables_nr;
		int functions_nr;
		int is_class;

		obj->_flags = 0;
		obj->_pos = obj_pos;

		VERIFY((byte *)funct_area < _buf + _bufSize, "Function area pointer references beyond end of script");

		variables_nr = READ_LE_UINT16(data + 2);
		functions_nr = READ_LE_UINT16(funct_area);
		is_class = READ_LE_UINT16(data + 14) & SCRIPT_INFO_CLASS;

		obj->base_method = funct_area;
		obj->base_vars = prop_area;

		VERIFY(((byte *)funct_area + functions_nr) < _buf + _bufSize, "Function area extends beyond end of script");

		obj->variable_names_nr = variables_nr;
		obj->_variables.resize(variables_nr);

		obj->methods_nr = functions_nr;
		obj->base = _buf;
		obj->base_obj = data;

		for (int i = 0; i < variables_nr; i++)
			obj->_variables[i] = make_reg(0, READ_LE_UINT16(data + (i * 2)));
	}

	return obj;
}

Object *Script::scriptObjInit(reg_t obj_pos) {
	if (getSciVersion() < SCI_VERSION_1_1)
		return scriptObjInit0(obj_pos);
	else
		return scriptObjInit11(obj_pos);
}

LocalVariables *SegManager::allocLocalsSegment(Script *scr, int count) {
	if (!count) { // No locals
		scr->_localsSegment = 0;
		scr->_localsBlock = NULL;
		return NULL;
	} else {
		LocalVariables *locals;

		if (scr->_localsSegment) {
			locals = (LocalVariables *)_heap[scr->_localsSegment];
			VERIFY(locals != NULL, "Re-used locals segment was NULL'd out");
			VERIFY(locals->getType() == SEG_TYPE_LOCALS, "Re-used locals segment did not consist of local variables");
			VERIFY(locals->script_id == scr->_nr, "Re-used locals segment belonged to other script");
		} else
			locals = (LocalVariables *)allocSegment(new LocalVariables(), &scr->_localsSegment);

		scr->_localsBlock = locals;
		locals->script_id = scr->_nr;
		locals->_locals.resize(count);

		return locals;
	}
}

void SegManager::scriptInitialiseLocalsZero(SegmentId seg, int count) {
	Script *scr = getScript(seg);

	scr->_localsOffset = -count * 2; // Make sure it's invalid

	allocLocalsSegment(scr, count);
}

void SegManager::scriptInitialiseLocals(reg_t location) {
	Script *scr = getScript(location.segment);
	unsigned int count;

	VERIFY(location.offset + 1 < (uint16)scr->_bufSize, "Locals beyond end of script\n");

	if (getSciVersion() >= SCI_VERSION_1_1)
		count = READ_LE_UINT16(scr->_buf + location.offset - 2);
	else
		count = (READ_LE_UINT16(scr->_buf + location.offset - 2) - 4) >> 1;
	// half block size

	scr->_localsOffset = location.offset;

	if (!(location.offset + count * 2 + 1 < scr->_bufSize)) {
		warning("Locals extend beyond end of script: offset %04x, count %x vs size %x", location.offset, count, (uint)scr->_bufSize);
		count = (scr->_bufSize - location.offset) >> 1;
	}

	LocalVariables *locals = allocLocalsSegment(scr, count);
	if (locals) {
		uint i;
		byte *base = (byte *)(scr->_buf + location.offset);

		for (i = 0; i < count; i++)
			locals->_locals[i].offset = READ_LE_UINT16(base + i * 2);
	}
}

void SegManager::scriptRelocateExportsSci11(SegmentId seg) {
	Script *scr = getScript(seg);
	for (int i = 0; i < scr->_numExports; i++) {
		/* We are forced to use an ugly heuristic here to distinguish function
		   exports from object/class exports. The former kind points into the
		   script resource, the latter into the heap resource.  */
		uint16 location = READ_LE_UINT16((byte *)(scr->_exportTable + i));
		if ((location < scr->_heapSize - 1) && (READ_LE_UINT16(scr->_heapStart + location) == SCRIPT_OBJECT_MAGIC_NUMBER)) {
			WRITE_LE_UINT16((byte *)(scr->_exportTable + i), location + scr->_heapStart - scr->_buf);
		} else {
			// Otherwise it's probably a function export,
			// and we don't need to do anything.
		}
	}
}

void SegManager::scriptInitialiseObjectsSci11(SegmentId seg) {
	Script *scr = getScript(seg);
	byte *seeker = scr->_heapStart + 4 + READ_LE_UINT16(scr->_heapStart + 2) * 2;

	while (READ_LE_UINT16(seeker) == SCRIPT_OBJECT_MAGIC_NUMBER) {
		if (READ_LE_UINT16(seeker + 14) & SCRIPT_INFO_CLASS) {
			int classpos = seeker - scr->_buf;
			int species = READ_LE_UINT16(seeker + 10);

			if (species < 0 || species >= (int)_classtable.size()) {
				error("Invalid species %d(0x%x) not in interval [0,%d) while instantiating script %d",
				          species, species, _classtable.size(), scr->_nr);
				return;
			}

			_classtable[species].reg.segment = seg;
			_classtable[species].reg.offset = classpos;
		}
		seeker += READ_LE_UINT16(seeker + 2) * 2;
	}

	seeker = scr->_heapStart + 4 + READ_LE_UINT16(scr->_heapStart + 2) * 2;
	while (READ_LE_UINT16(seeker) == SCRIPT_OBJECT_MAGIC_NUMBER) {
		reg_t reg;
		Object *obj;

		reg.segment = seg;
		reg.offset = seeker - scr->_buf;
		obj = scr->scriptObjInit(reg);

#if 0
		if (obj->_variables[5].offset != 0xffff) {
			obj->_variables[5] = INST_LOOKUP_CLASS(obj->_variables[5].offset);
			base_obj = s->segMan->getObject(obj->_variables[5]);
			obj->variable_names_nr = base_obj->variables_nr;
			obj->base_obj = base_obj->base_obj;
		}
#endif

		// Copy base from species class, as we need its selector IDs
		obj->setSuperClassSelector(
			getClassAddress(obj->getSuperClassSelector().offset, SCRIPT_GET_LOCK, NULL_REG));

		// Set the -classScript- selector to the script number.
		// FIXME: As this selector is filled in at run-time, it is likely
		// that it is supposed to hold a pointer. The Obj::isKindOf method
		// uses this selector together with -propDict- to compare classes.
		// For the purpose of Obj::isKindOf, using the script number appears
		// to be sufficient.
		obj->setClassScriptSelector(make_reg(0, scr->_nr));

		seeker += READ_LE_UINT16(seeker + 2) * 2;
	}
}

/*
static char *SegManager::dynprintf(char *msg, ...) {
	va_list argp;
	char *buf = (char *)malloc(strlen(msg) + 100);

	va_start(argp, msg);
	vsprintf(buf, msg, argp);
	va_end(argp);

	return buf;
}
*/

DataStack *SegManager::allocateStack(int size, SegmentId *segid) {
	SegmentObj *mobj = allocSegment(new DataStack(), segid);
	DataStack *retval = (DataStack *)mobj;

	retval->_entries = (reg_t *)calloc(size, sizeof(reg_t));
	retval->_capacity = size;

	return retval;
}

SystemStrings *SegManager::allocateSysStrings(SegmentId *segid) {
	return (SystemStrings *)allocSegment(new SystemStrings(), segid);
}

SegmentId SegManager::allocateStringFrags() {
	SegmentId segid;

	allocSegment(new StringFrag(), &segid);

	return segid;
}

uint16 SegManager::validateExportFunc(int pubfunct, SegmentId seg) {
	Script *scr = getScript(seg);
	if (scr->_numExports <= pubfunct) {
		warning("validateExportFunc(): pubfunct is invalid");
		return 0;
	}

	if (_exportsAreWide)
		pubfunct *= 2;
	uint16 offset = READ_LE_UINT16((byte *)(scr->_exportTable + pubfunct));
	VERIFY(offset < scr->_bufSize, "invalid export function pointer");

	return offset;
}

void SegManager::freeHunkEntry(reg_t addr) {
	HunkTable *ht = (HunkTable *)GET_SEGMENT(*this, addr.segment, SEG_TYPE_HUNK);

	if (!ht) {
		warning("Attempt to free Hunk from address %04x:%04x: Invalid segment type", PRINT_REG(addr));
		return;
	}

	ht->freeEntry(addr.offset);
}

Hunk *SegManager::allocateHunkEntry(const char *hunk_type, int size, reg_t *reg) {
	Hunk *h = alloc_Hunk(reg);

	if (!h)
		return NULL;

	h->mem = malloc(size);
	h->size = size;
	h->type = hunk_type;

	return h;
}

Clone *SegManager::allocateClone(reg_t *addr) {
	CloneTable *table;
	int offset;

	if (!Clones_seg_id) {
		table = (CloneTable *)allocSegment(new CloneTable(), &(Clones_seg_id));
	} else
		table = (CloneTable *)_heap[Clones_seg_id];

	offset = table->allocEntry();

	*addr = make_reg(Clones_seg_id, offset);
	return &(table->_table[offset]);
}

void SegManager::reconstructClones() {
	for (uint i = 0; i < _heap.size(); i++) {
		if (_heap[i]) {
			SegmentObj *mobj = _heap[i];
			if (mobj->getType() == SEG_TYPE_CLONES) {
				CloneTable *ct = (CloneTable *)mobj;

				for (uint j = 0; j < ct->_table.size(); j++) {
					Object *base_obj;

					// Check if the clone entry is used
					uint entryNum = (uint)ct->first_free;
					bool isUsed = true;
					while (entryNum != ((uint) CloneTable::HEAPENTRY_INVALID)) {
						if (entryNum == j) {
							isUsed = false;
							break;
						}
						entryNum = ct->_table[entryNum].next_free;
					}

					if (!isUsed)
						continue;

					CloneTable::Entry &seeker = ct->_table[j];
					base_obj = getObject(seeker.getSpeciesSelector());
					if (!base_obj) {
						warning("Clone entry without a base class: %d", j);
						seeker.base = NULL;
						seeker.base_obj = NULL;
						seeker.base_vars = NULL;
						seeker.base_method = NULL;
					} else {
						seeker.base = base_obj->base;
						seeker.base_obj = base_obj->base_obj;
						seeker.base_vars = base_obj->base_vars;
						seeker.base_method = base_obj->base_method;
					}
				}	// end for
			}	// end if
		}	// end if
	}	// end for
}

List *SegManager::allocateList(reg_t *addr) {
	ListTable *table;
	int offset;

	if (!Lists_seg_id)
		allocSegment(new ListTable(), &(Lists_seg_id));
	table = (ListTable *)_heap[Lists_seg_id];

	offset = table->allocEntry();

	*addr = make_reg(Lists_seg_id, offset);
	return &(table->_table[offset]);
}

Node *SegManager::allocateNode(reg_t *addr) {
	NodeTable *table;
	int offset;

	if (!Nodes_seg_id)
		allocSegment(new NodeTable(), &(Nodes_seg_id));
	table = (NodeTable *)_heap[Nodes_seg_id];

	offset = table->allocEntry();

	*addr = make_reg(Nodes_seg_id, offset);
	return &(table->_table[offset]);
}

Hunk *SegManager::alloc_Hunk(reg_t *addr) {
	HunkTable *table;
	int offset;

	if (!Hunks_seg_id)
		allocSegment(new HunkTable(), &(Hunks_seg_id));
	table = (HunkTable *)_heap[Hunks_seg_id];

	offset = table->allocEntry();

	*addr = make_reg(Hunks_seg_id, offset);
	return &(table->_table[offset]);
}

SegmentRef SegManager::dereference(reg_t pointer) {
	SegmentRef ret;

	if (!pointer.segment || (pointer.segment >= _heap.size()) || !_heap[pointer.segment]) {
		// This occurs in KQ5CD when interacting with certain objects
		warning("Attempt to dereference invalid pointer %04x:%04x", PRINT_REG(pointer));
		return ret; /* Invalid */
	}

	SegmentObj *mobj = _heap[pointer.segment];
	return mobj->dereference(pointer);
}

static void *_kernel_dereference_pointer(SegManager *segMan, reg_t pointer, int entries) {
	SegmentRef ret = segMan->dereference(pointer);

	if (!ret.raw)
		return NULL;

	if (entries > ret.maxSize) {
		warning("Trying to dereference pointer %04x:%04x beyond end of segment", PRINT_REG(pointer));
		return NULL;
	}
	return ret.raw;

}

byte *SegManager::derefBulkPtr(reg_t pointer, int entries) {
	return (byte *)_kernel_dereference_pointer(this, pointer, entries);
}

reg_t *SegManager::derefRegPtr(reg_t pointer, int entries) {
	const int align = sizeof(reg_t);
	if (pointer.offset & (align - 1)) {
		warning("Unaligned pointer read: %04x:%04x expected with %d alignment", PRINT_REG(pointer), align);
		return NULL;
	}

	return (reg_t *)_kernel_dereference_pointer(this, pointer, entries);
}

char *SegManager::derefString(reg_t pointer, int entries) {
	return (char *)_kernel_dereference_pointer(this, pointer, entries);
}


byte *SegManager::allocDynmem(int size, const char *descr, reg_t *addr) {
	SegmentId seg;
	SegmentObj *mobj = allocSegment(new DynMem(), &seg);
	*addr = make_reg(seg, 0);

	DynMem &d = *(DynMem *)mobj;

	d._size = size;

	if (size == 0)
		d._buf = NULL;
	else
		d._buf = (byte *)malloc(size);

	d._description = descr;

	return (byte *)(d._buf);
}

const char *SegManager::getDescription(reg_t addr) {
	if (addr.segment < 1 || addr.segment >= _heap.size())
		return "";

	SegmentObj *mobj = _heap[addr.segment];

	switch (mobj->getType()) {
	case SEG_TYPE_DYNMEM:
		return (*(DynMem *)mobj)._description.c_str();
	default:
		return "";
	}
}

int SegManager::freeDynmem(reg_t addr) {
	if (addr.segment < 1 || addr.segment >= _heap.size() || !_heap[addr.segment] || _heap[addr.segment]->getType() != SEG_TYPE_DYNMEM)
		return 1; // error

	deallocate(addr.segment, true);

	return 0; // OK
}

int SegManager::createClassTable() {
	Resource *vocab996 = _resMan->findResource(ResourceId(kResourceTypeVocab, 996), 1);

	if (!vocab996)
		error("SegManager: failed to open vocab 996");

	int totalClasses = vocab996->size >> 2;
	_classtable.resize(totalClasses);

	for (uint16 classNr = 0; classNr < totalClasses; classNr++) {
		uint16 scriptNr = READ_LE_UINT16(vocab996->data + classNr * 4 + 2);

		_classtable[classNr].reg = NULL_REG;
		_classtable[classNr].script = scriptNr;
	}

	_resMan->unlockResource(vocab996);

	return 0;
}

} // End of namespace Sci
