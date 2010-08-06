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
		// In SCI11, the heap was in a separate space from the script. We append
		// it to the end of the script, and adjust addressing accordingly.
		// However, since we address the heap with a 16-bit pointer, the
		// combined size of the stack and the heap must be 64KB. So far this has
		// worked for SCI11, SCI2 and SCI21 games. SCI3 games use a different
		// script format, and theoretically they can exceed the 64KB boundary
		// using relocation.
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
					"design bug was made regarding SCI1.1 and newer games.\n"
					"Please report this error to the ScummVM team");
	}
}

// signatures are built like this:
//  - first a counter of the bytes that follow
//  - then the actual bytes that need to get matched
//  - then another counter of bytes (0 for EOS)
//  - if not EOS, an adjust offset and the actual bytes
//  - rinse and repeat


// this here gets called on entry and when going out of game windows
//  uEvt::port will not get changed after kDisposeWindow but a bit later, so
//  we would get an invalid port handle to a kSetPort call. We just patch in
//  resetting of the port selector. We destroy the stop/fade code in there,
//  it seems it isn't used at all in the game.
const byte hoyle4SignaturePortFix[] = {
	28,
	0x39, 0x09,        // pushi 09
	0x89, 0x0b,        // lsg 0b
	0x39, 0x64,        // pushi 64
	0x38, 0xc8, 0x00,  // pushi 00c8
	0x38, 0x2c, 0x01,  // pushi 012c
	0x38, 0x90, 0x01,  // pushi 0190
	0x38, 0xf4, 0x01,  // pushi 01f4
	0x38, 0x58, 0x02,  // pushi 0258
	0x38, 0xbc, 0x02,  // pushi 02bc
	0x38, 0x20, 0x03,  // pushi 0320
	0x46,              // calle [xxxx] [xxxx] [xx]
	+5, 43,            // [skip 5 bytes]
	0x30, 0x27, 0x00,  // bnt 0027 -> end of routine
	0x87, 0x00,        // lap 00
	0x30, 0x19, 0x00,  // bnt 0019 -> fade out
	0x87, 0x01,        // lap 01
	0x30, 0x14, 0x00,  // bnt 0014 -> fade out
	0x38, 0xa7, 0x00,  // pushi 00a7
	0x76,              // push0
	0x80, 0x29, 0x01,  // lag 0129
	0x4a, 0x04,        // send 04 (song::stop)
	0x39, 0x27,        // pushi 27
	0x78,              // push1
	0x8f, 0x01,        // lsp 01
	0x51, 0x54,        // class 54
	0x4a, 0x06,        // send 06 (PlaySong::play)
	0x33, 0x09,        // jmp 09 -> end of routine
	0x38, 0xaa, 0x00,  // pushi 00aa
	0x76,              // push0
	0x80, 0x29, 0x01,  // lag 0129
	0x4a, 0x04,        // send 04
	0x48,              // ret
	0
};

#define PATCH_END             0xFFFF
#define PATCH_ADDTOOFFSET     0x8000
#define PATCH_GETORIGINALBYTE 0x4000

const uint16 hoyle4PatchPortFix[] = {
	PATCH_ADDTOOFFSET | +33,
	0x38, 0x31, 0x01,  // pushi 0131 (selector curEvent)
	0x76,              // push0
	0x80, 0x50, 0x00,  // lag 0050 (global var 80h, "User")
	0x4a, 0x04,        // send 04 (read User::curEvent)

	0x38, 0x93, 0x00,  // pushi 0093 (selector port)
	0x78,              // push1
	0x76,              // push0
	0x4a, 0x06,        // send 06 (write 0 to that object::port)
	0x48,              // ret
	PATCH_END
};

//    script, description,                                   magic DWORD,                adjust
const SciScriptSignature hoyle4Signatures[] = {
    {      0, "port fix when disposing windows",             CONSTANT_LE_32(0x00C83864),    -5, hoyle4SignaturePortFix,   hoyle4PatchPortFix },
    {      0, NULL,                                          0,                              0, NULL,                     NULL }
};


// this is called on every death dialog. Problem is at least the german
//  version of lsl6 gets title text that is far too long for the
//  available temp space resulting in temp space corruption
//  This patch moves the title text around, so this overflow
//  doesn't happen anymore. We would otherwise get a crash
//  calling for invalid views (this happens of course also
//  in sierra sci)
const byte larry6SignatureDeathDialog[] = {
	7,
	0x3e, 0x33, 0x01,             // link 0133 (offset 0x20)
	0x35, 0xff,                   // ldi ff
	0xa3, 0x00,                   // sal 00
	+255, 0,
	+255, 0,
	+170, 12,                     // [skip 680 bytes]
	0x8f, 0x01,                   // lsp 01 (offset 0x2cf)
	0x7a,                         // push2
	0x5a, 0x04, 0x00, 0x0e, 0x01, // lea 0004 010e
	0x36,                         // push
	0x43, 0x7c, 0x0e,             // kMessage[7c] 0e
	+90, 10,                      // [skip 90 bytes]
	0x38, 0xd6, 0x00,             // pushi 00d6 (offset 0x335)
	0x78,                         // push1
	0x5a, 0x04, 0x00, 0x0e, 0x01, // lea 0004 010e
	0x36,                         // push
	+76, 11,                      // [skip 76 bytes]
	0x38, 0xcd, 0x00,             // pushi 00cd (offset 0x38b)
	0x39, 0x03,                   // pushi 03
	0x5a, 0x04, 0x00, 0x0e, 0x01, // lea 0004 010e
	0x36,
	0
};

const uint16 larry6PatchDeathDialog[] = {
	0x3e, 0x00, 0x02,             // link 0200
	PATCH_ADDTOOFFSET | +687,
	0x5a, 0x04, 0x00, 0x40, 0x01, // lea 0004 0140
	PATCH_ADDTOOFFSET | +98,
	0x5a, 0x04, 0x00, 0x40, 0x01, // lea 0004 0140
	PATCH_ADDTOOFFSET | +82,
	0x5a, 0x04, 0x00, 0x40, 0x01, // lea 0004 0140
	PATCH_END
};

//    script, description,                                   magic DWORD,                adjust
const SciScriptSignature larry6Signatures[] = {
    {     82, "death dialog memory corruption",              CONSTANT_LE_32(0x3501333e),     0, larry6SignatureDeathDialog, larry6PatchDeathDialog },
    {      0, NULL,                                          0,                              0, NULL,                       NULL }
};

// will actually patch previously found signature area
void Script::applyPatch(const uint16 *patch, byte *scriptData, const uint32 scriptSize, int32 signatureOffset) {
	int32 offset = signatureOffset;
	uint16 patchWord = *patch;

	while (patchWord != PATCH_END) {
		if (patchWord & PATCH_ADDTOOFFSET) {
			offset += patchWord & ~PATCH_ADDTOOFFSET;
		} else if (patchWord & PATCH_GETORIGINALBYTE) {
			// TODO: implement this
		} else {
			scriptData[offset] = patchWord & 0xFF;
			offset++;
		}
		patch++;
		patchWord = *patch;
	}	
}

// will return -1 if no match was found, otherwise an offset to the start of the signature match
int32 Script::findSignature(const SciScriptSignature *signature, const byte *scriptData, const uint32 scriptSize) {
	if (scriptSize < 4) // we need to find a DWORD, so less than 4 bytes is not okay
		return -1;

	const uint32 magicDWord = signature->magicDWord; // is platform-specific BE/LE form, so that the later match will work
	const uint32 searchLimit = scriptSize - 3;
	uint32 DWordOffset = 0;
	// first search for the magic DWORD
	while (DWordOffset < searchLimit) {
		if (magicDWord == *(const uint32 *)(scriptData + DWordOffset)) {
			// magic DWORD found, check if actual signature matches
			uint32 offset = DWordOffset + signature->magicOffset;
			uint32 byteOffset = offset;
			const byte *signatureData = signature->data;
			byte matchAdjust = 1;
			while (matchAdjust) {
				byte matchBytesCount = *signatureData++;
				if ((byteOffset + matchBytesCount) > scriptSize) // Out-Of-Bounds?
					break;
				if (memcmp(signatureData, &scriptData[byteOffset], matchBytesCount)) // Byte-Mismatch?
					break;
				// those bytes matched, adjust offsets accordingly
				signatureData += matchBytesCount;
				byteOffset += matchBytesCount;
				// get offset...
				matchAdjust = *signatureData++;
				byteOffset += matchAdjust;
			}
			if (!matchAdjust) // all matches worked?
				return offset;
		}
		DWordOffset++;
	}
	// nothing found
	return -1;
}

void Script::matchSignatureAndPatch(uint16 scriptNr, byte *scriptData, const uint32 scriptSize) {
	const SciScriptSignature *signatureTable = NULL;
	if (g_sci->getGameId() == GID_HOYLE4)
		signatureTable = hoyle4Signatures;
	if (g_sci->getGameId() == GID_LSL6)
		signatureTable = larry6Signatures;

	if (signatureTable) {
		while (signatureTable->data) {
			if (scriptNr == signatureTable->scriptNr) {
				int32 foundOffset = findSignature(signatureTable, scriptData, scriptSize);
				if (foundOffset != -1) {
					// found, so apply the patch
					warning("matched %s on script %d offset %d", signatureTable->description, scriptNr, foundOffset);
					applyPatch(signatureTable->patch, scriptData, scriptSize, foundOffset);
				}
			}
			signatureTable++;
		}
	}
}

void Script::load(ResourceManager *resMan) {
	Resource *script = resMan->findResource(ResourceId(kResourceTypeScript, _nr), 0);
	assert(script != 0);

	_buf = (byte *)malloc(_bufSize);
	assert(_buf);

	assert(_bufSize >= script->size);
	memcpy(_buf, script->data, script->size);

	// Check scripts for matching signatures and patch those, if found
	matchSignatureAndPatch(_nr, _buf, script->size);

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
	if (getSciVersion() < SCI_VERSION_1_1 && fullObjectInit)
		obj_pos.offset += 8;	// magic offset (SCRIPT_OBJECT_MAGIC_OFFSET)

	VERIFY(obj_pos.offset < _bufSize, "Attempt to initialize object beyond end of script\n");

	VERIFY(obj_pos.offset + kOffsetFunctionArea < (int)_bufSize, "Function area pointer stored beyond end of script\n");

	// Get the object at the specified position and init it. This will
	// automatically "allocate" space for it in the _objects map if necessary.
	Object *obj = &_objects[obj_pos.offset];
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
	const byte *heap = _buf;
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
		// This occurs in SCI01/SCI1 games where usually one export value is
		// zero. It seems that in this situation, we should skip the export and
		// move to the next one, though the total count of valid exports remains
		// the same
		if (!pos) {
			exportIndex++;
			pos = READ_SCI11ENDIAN_UINT16(heap + block.offset + 2 + (exportIndex * 2)) + heapOffset;
			if (!pos)
				error("Script::relocate(): Consecutive zero exports found");
		}

		// In SCI0-SCI1, script local variables, objects and code are relocated.
		// We only relocate locals and objects here, and ignore relocation of
		// code blocks. In SCI1.1 and newer versions, only locals and objects
		// are relocated.
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

	if (offset == 0) {
		// Check if the game has a second export table (e.g. script 912 in Camelot)
		// Fixes bug #3039785
		const uint16 *secondExportTable = (const uint16 *)findBlock(SCI_OBJ_EXPORTS, 0);

		if (secondExportTable) {
			secondExportTable += 3;	// skip header plus 2 bytes (secondExportTable is a uint16 pointer)
			offset = READ_SCI11ENDIAN_UINT16(secondExportTable + pubfunct);
			VERIFY(offset < _bufSize, "invalid export function pointer");
		}
	}

	return offset;
}

byte *Script::findBlock(int type, int skipBlockIndex) {
	byte *buf = _buf;
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);
	int blockIndex = 0;

	if (oldScriptHeader)
		buf += 2;

	do {
		int seekerType = READ_LE_UINT16(buf);

		if (seekerType == 0)
			break;
		if (seekerType == type && blockIndex != skipBlockIndex)
			return buf;

		int seekerSize = READ_LE_UINT16(buf + 2);
		assert(seekerSize > 0);
		buf += seekerSize;
		blockIndex++;
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

void Script::initialiseLocals(SegManager *segMan) {
	LocalVariables *locals = segMan->allocLocalsSegment(this);
	if (locals) {
		if (getSciVersion() > SCI_VERSION_0_EARLY) {
			const byte *base = (const byte *)(_buf + getLocalsOffset());

			for (uint16 i = 0; i < getLocalsCount(); i++)
				locals->_locals[i] = make_reg(0, READ_SCI11ENDIAN_UINT16(base + i * 2));
		} else {
			// In SCI0 early, locals are set at run time, thus zero them all here
			for (uint16 i = 0; i < getLocalsCount(); i++)
				locals->_locals[i] = NULL_REG;
		}
	}
}

void Script::initialiseClasses(SegManager *segMan) {
	const byte *seeker = 0;
	uint16 mult = 0;
	
	if (getSciVersion() >= SCI_VERSION_1_1) {
		seeker = _heapStart + 4 + READ_SCI11ENDIAN_UINT16(_heapStart + 2) * 2;
		mult = 2;
	} else {
		seeker = findBlock(SCI_OBJ_CLASS);
		mult = 1;
	}

	if (!seeker)
		return;

	uint16 marker;
	bool isClass;
	uint16 classpos;
	int16 species = 0;

	while (true) {
		// In SCI0-SCI1, this is the segment type. In SCI11, it's a marker (0x1234)
		marker = READ_SCI11ENDIAN_UINT16(seeker);
		classpos = seeker - _buf;

		if (!marker)
			break;

		if (getSciVersion() >= SCI_VERSION_1_1) {
			isClass = (READ_SCI11ENDIAN_UINT16(seeker + 14) & kInfoFlagClass);	// -info- selector
			species = READ_SCI11ENDIAN_UINT16(seeker + 10);
		} else {
			isClass = (marker == SCI_OBJ_CLASS);
			if (isClass)
				species = READ_SCI11ENDIAN_UINT16(seeker + 12);
			classpos += 12;
		}

		if (isClass) {
			// WORKAROUNDs for off-by-one script errors
			if (g_sci->getGameId() == GID_LSL2 && g_sci->isDemo() && species == (int)segMan->classTableSize())
				segMan->resizeClassTable(segMan->classTableSize() + 1);
			if (g_sci->getGameId() == GID_LSL3 && !g_sci->isDemo() && _nr == 500 && species == (int)segMan->classTableSize())
				segMan->resizeClassTable(segMan->classTableSize() + 1);
			if (g_sci->getGameId() == GID_SQ3 && !g_sci->isDemo() && _nr == 93 && species == (int)segMan->classTableSize())
				segMan->resizeClassTable(segMan->classTableSize() + 1);
			if (g_sci->getGameId() == GID_SQ3 && !g_sci->isDemo() && _nr == 99 && species == (int)segMan->classTableSize())
				segMan->resizeClassTable(segMan->classTableSize() + 1);

			if (species < 0 || species >= (int)segMan->classTableSize())
				error("Invalid species %d(0x%x) unknown max %d(0x%x) while instantiating script %d\n",
						  species, species, segMan->classTableSize(), segMan->classTableSize(), _nr);

			SegmentId segmentId = segMan->getScriptSegment(_nr);
			segMan->setClassOffset(species, make_reg(segmentId, classpos));
		}

		seeker += READ_SCI11ENDIAN_UINT16(seeker + 2) * mult;
	}
}

void Script::initialiseObjectsSci0(SegManager *segMan, SegmentId segmentId) {
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);
	const byte *seeker = _buf + (oldScriptHeader ? 2 : 0);

	do {
		uint16 objType = READ_SCI11ENDIAN_UINT16(seeker);
		if (!objType)
			break;

		switch (objType) {
		case SCI_OBJ_OBJECT:
		case SCI_OBJ_CLASS:
			{
				reg_t addr = make_reg(segmentId, seeker - _buf + 4);
				Object *obj = scriptObjInit(addr);
				obj->initSpecies(segMan, addr);

				if (!obj->initBaseObject(segMan, addr)) {
					if (_nr == 202 && g_sci->getGameId() == GID_KQ5) {
						// WORKAROUND: Script 202 of KQ5 French and German 
						// (perhaps Spanish too?) has an invalid object.
						// This is non-fatal. Refer to bug #3035396.
					} else {
						error("Failed to locate base object for object at %04X:%04X; skipping", PRINT_REG(addr));
					}
					scriptObjRemove(addr);
				}
			}
			break;

		default:
			break;
		}

		seeker += READ_SCI11ENDIAN_UINT16(seeker + 2);
	} while ((uint32)(seeker - _buf) < getScriptSize() - 2);

	byte *relocationBlock = findBlock(SCI_OBJ_POINTERS);
	if (relocationBlock)
		relocate(make_reg(segmentId, relocationBlock - getBuf() + 4));
}

void Script::initialiseObjectsSci11(SegManager *segMan, SegmentId segmentId) {
	const byte *seeker = _heapStart + 4 + READ_SCI11ENDIAN_UINT16(_heapStart + 2) * 2;

	while (READ_SCI11ENDIAN_UINT16(seeker) == SCRIPT_OBJECT_MAGIC_NUMBER) {
		reg_t reg = make_reg(segmentId, seeker - _buf);
		Object *obj = scriptObjInit(reg);

		// Copy base from species class, as we need its selector IDs
		obj->setSuperClassSelector(
			segMan->getClassAddress(obj->getSuperClassSelector().offset, SCRIPT_GET_LOCK, NULL_REG));

		// If object is instance, get -propDict- from class and set it for this
		// object. This is needed for ::isMemberOf() to work.
		// Example testcase - room 381 of sq4cd - if isMemberOf() doesn't work,
		// talk-clicks on the robot will act like clicking on ego
		if (!obj->isClass()) {
			reg_t classObject = obj->getSuperClassSelector();
			const Object *classObj = segMan->getObject(classObject);
			obj->setPropDictSelector(classObj->getPropDictSelector());
		}

		// Set the -classScript- selector to the script number.
		// FIXME: As this selector is filled in at run-time, it is likely
		// that it is supposed to hold a pointer. The Obj::isKindOf method
		// uses this selector together with -propDict- to compare classes.
		// For the purpose of Obj::isKindOf, using the script number appears
		// to be sufficient.
		obj->setClassScriptSelector(make_reg(0, _nr));

		seeker += READ_SCI11ENDIAN_UINT16(seeker + 2) * 2;
	}

	relocate(make_reg(segmentId, READ_SCI11ENDIAN_UINT16(_heapStart)));
}

reg_t Script::findCanonicAddress(SegManager *segMan, reg_t addr) const {
	addr.offset = 0;
	return addr;
}

void Script::freeAtAddress(SegManager *segMan, reg_t addr) {
	/*
		debugC(2, kDebugLevelGC, "[GC] Freeing script %04x:%04x", PRINT_REG(addr));
		if (_localsSegment)
			debugC(2, kDebugLevelGC, "[GC] Freeing locals %04x:0000", _localsSegment);
	*/

	if (_markedAsDeleted)
		segMan->deallocateScript(_nr);
}

Common::Array<reg_t> Script::listAllDeallocatable(SegmentId segId) const {
	const reg_t r = make_reg(segId, 0);
	return Common::Array<reg_t>(&r, 1);
}

Common::Array<reg_t> Script::listAllOutgoingReferences(reg_t addr) const {
	Common::Array<reg_t> tmp;
	if (addr.offset <= _bufSize && addr.offset >= -SCRIPT_OBJECT_MAGIC_OFFSET && RAW_IS_OBJECT(_buf + addr.offset)) {
		const Object *obj = getObject(addr.offset);
		if (obj) {
			// Note all local variables, if we have a local variable environment
			if (_localsSegment)
				tmp.push_back(make_reg(_localsSegment, 0));

			for (uint i = 0; i < obj->getVarCount(); i++)
				tmp.push_back(obj->getVariable(i));
		} else {
			error("Request for outgoing script-object reference at %04x:%04x failed", PRINT_REG(addr));
		}
	} else {
		/*		warning("Unexpected request for outgoing script-object references at %04x:%04x", PRINT_REG(addr));*/
		/* Happens e.g. when we're looking into strings */
	}
	return tmp;
}

Common::Array<reg_t> Script::listObjectReferences() const {
	Common::Array<reg_t> tmp;

	// Locals, if present
	if (_localsSegment)
		tmp.push_back(make_reg(_localsSegment, 0));

	// All objects (may be classes, may be indirectly reachable)
	ObjMap::iterator it;
	const ObjMap::iterator end = _objects.end();
	for (it = _objects.begin(); it != end; ++it) {
		tmp.push_back(it->_value.getPos());
	}

	return tmp;
}

} // End of namespace Sci
