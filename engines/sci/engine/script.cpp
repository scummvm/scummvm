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

#include "sci/console.h"
#include "sci/sci.h"
#include "sci/resource.h"
#include "sci/util.h"
#include "sci/engine/features.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/script.h"

#include "common/util.h"

namespace Sci {

const char *sciObjectTypeNames[] = {
	"terminator", "object", "code", "synonyms", "said", "strings", "class",
	"exports", "pointers", "preload text", "local vars"
};

Script::Script()
	: SegmentObj(SEG_TYPE_SCRIPT), _buf() {
	freeScript();
}

Script::~Script() {
	freeScript();
}

void Script::freeScript() {
	_nr = 0;

	_buf.clear();
	_script.clear();
	_heap.clear();
	_exports.clear();
	_numExports = 0;
	_synonyms.clear();
	_numSynonyms = 0;

	_localsOffset = 0;
	_localsSegment = 0;
	_localsBlock = NULL;
	_localsCount = 0;

	_lockers = 1;
	_markedAsDeleted = false;
	_objects.clear();

	_offsetLookupArray.clear();
	_offsetLookupObjectCount = 0;
	_offsetLookupStringCount = 0;
	_offsetLookupSaidCount = 0;
}

enum {
	kSci11NumExportsOffset = 6,
	kSci11ExportTableOffset = 8
};

void Script::load(int script_nr, ResourceManager *resMan, ScriptPatcher *scriptPatcher) {
	freeScript();

	Resource *script = resMan->findResource(ResourceId(kResourceTypeScript, script_nr), false);
	if (!script)
		error("Script %d not found", script_nr);

	_nr = script_nr;
	uint32 scriptSize = script->size();
	uint32 bufSize = scriptSize;

	if (getSciVersion() == SCI_VERSION_0_EARLY) {
		bufSize += script->getUint16LEAt(0) * 2;
	} else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1_LATE) {
		// In SCI1.1 - SCI2.1, the heap was in a separate space from the script. We append
		// it to the end of the script, and adjust addressing accordingly.
		// However, since we address the heap with a 16-bit pointer, the
		// combined size of the stack and the heap must be 64KB. So far this has
		// worked for SCI11, SCI2 and SCI21 games. SCI3 games use a different
		// script format, and theoretically they can exceed the 64KB boundary
		// using relocation.
		Resource *heap = resMan->findResource(ResourceId(kResourceTypeHeap, script_nr), false);
		bufSize += heap->size();

		// Ensure that the start of the heap resource can be word-aligned.
		if (script->size() & 2) {
			++bufSize;
			++scriptSize;
		}

		// As mentioned above, the script and the heap together should not exceed 64KB
		if (script->size() + heap->size() > 65535)
			error("Script and heap sizes combined exceed 64K. This means a fundamental "
					"design bug was made regarding SCI1.1 and newer games.\n"
					"Please report this error to the ScummVM team");
	} else if (getSciVersion() == SCI_VERSION_3) {
		// Check for scripts over 64KB. These won't work with the current 16-bit address
		// scheme. We need an overlaying mechanism, or a mechanism to split script parts
		// in different segments to handle these. For now, simply stop when such a script
		// is found.
		//
		// Known large SCI 3 scripts are:
		// Lighthouse: 9, 220, 270, 351, 360, 490, 760, 765, 800
		// LSL7: 240, 511, 550
		// Phantasmagoria 2: none (hooray!)
		// RAMA: 70
		//
		// TODO: Remove this once such a mechanism is in place
		if (script->size() > 65535)
			warning("TODO: SCI script %d is over 64KB - it's %u bytes long. This can't "
			      "be fully handled at the moment", script_nr, script->size());
	}

	uint extraLocalsWorkaround = 0;
	if (g_sci->getGameId() == GID_FANMADE && _nr == 1 && script->size() == 11140) {
		// WORKAROUND: Script 1 in Ocean Battle doesn't have enough locals to
		// fit the string showing how many shots are left (a nasty script bug,
		// corrupting heap memory). We add 10 more locals so that it has enough
		// space to use as the target for its kFormat operation. Fixes bug
		// #3059871.
		extraLocalsWorkaround = 10;
	}
	bufSize += extraLocalsWorkaround * 2;

	SciSpan<byte> outBuffer = _buf->allocate(bufSize, script->name() + " buffer");
	script->copyDataTo(outBuffer);
	// The word-aligned script size is used here because other parts of the code
	// currently rely on finding the start of the heap by reading the script
	// size
	_script = _buf->subspan(0, scriptSize, script->name());

	if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1_LATE) {
		Resource *heap = resMan->findResource(ResourceId(kResourceTypeHeap, _nr), false);
		assert(heap);

		SciSpan<byte> outHeap = outBuffer.subspan(scriptSize, heap->size(), heap->name(), 0);
		heap->copyDataTo(outHeap);
		_heap = outHeap;
	}

	// Check scripts (+ possibly SCI 1.1 heap) for matching signatures and patch those, if found
	scriptPatcher->processScript(_nr, outBuffer);

	if (getSciVersion() <= SCI_VERSION_1_LATE) {
		// Some buggy game scripts contain two export tables (e.g. script 912
		// in Camelot and script 306 in KQ4); in these scripts, the first table
		// is broken, so we ignore it and use the last one instead
		// Fixes bugs #3039785 and #3037595.
		SciSpan<const uint16> exportTable = findBlockSCI0(SCI_OBJ_EXPORTS, true).subspan<const uint16>(0);
		if (exportTable) {
			// The export table is after the block header (4 bytes / 2 uint16s)
			// and the number of exports (2 bytes / 1 uint16).
			// The exports span does not need to be explicitly sized since the
			// maximum size was already determined by findBlockSCI0
			_exports = exportTable.subspan(3);
			_numExports = exportTable.getUint16SEAt(2);
		}

		SciSpan<const byte> synonymTable = findBlockSCI0(SCI_OBJ_SYNONYMS);
		if (synonymTable) {
			// the synonyms table is after the block header (4 bytes),
			// and each synonym entry is 4 bytes
			_synonyms = synonymTable.subspan(4);
			_numSynonyms = _synonyms.size() / 4;
		}

		SciSpan<const byte> localsTable = findBlockSCI0(SCI_OBJ_LOCALVARS);
		if (localsTable) {
			// skip header (4 bytes)
			_localsOffset = localsTable - *_buf + 4;
			_localsCount = localsTable.size() / 2 - 2;
		}
	} else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1_LATE) {
		_numExports = _buf->getUint16SEAt(kSci11NumExportsOffset);
		if (_numExports) {
			_exports = _buf->subspan<const uint16>(kSci11ExportTableOffset, _numExports * sizeof(uint16));
		}

		_localsOffset = _script.size() + 4;
		_localsCount = _buf->getUint16SEAt(_localsOffset - 2);
	} else if (getSciVersion() == SCI_VERSION_3) {
		_localsCount = _buf->getUint16LEAt(12);
		_numExports = _buf->getUint16LEAt(20);
		if (_numExports) {
			_exports = _buf->subspan<const uint16>(22, _numExports * sizeof(uint16));
			// SCI3 local variables always start dword-aligned
			if (_numExports % 2)
				_localsOffset = 22 + _numExports * 2;
			else
				_localsOffset = 24 + _numExports * 2;
		}
	}

	// WORKAROUND: Increase locals, if needed (check above)
	_localsCount += extraLocalsWorkaround;

	if (getSciVersion() == SCI_VERSION_0_EARLY) {
		// SCI0 early
		// Old script block. There won't be a localvar block in this case.
		// Instead, the script starts with a 16 bit int specifying the
		// number of locals we need; these are then allocated and zeroed.
		_localsCount = _buf->getUint16LEAt(0);
		_localsOffset = -_localsCount * 2; // Make sure it's invalid
	} else {
		// SCI0 late and newer
		// Does the script actually have locals? If not, set the locals offset to 0
		if (!_localsCount)
			_localsOffset = 0;

		if (_localsOffset + _localsCount * 2 + 1 >= (int)_buf->size()) {
			error("Locals extend beyond end of script: offset %04x, count %d vs size %d", _localsOffset, _localsCount, (int)_buf->size());
		}
	}

	// find all strings of this script
	identifyOffsets();
}

void Script::identifyOffsets() {
	offsetLookupArrayEntry arrayEntry;
	SciSpan<const byte> scriptDataPtr;
	SciSpan<const byte> stringStartPtr;
	SciSpan<const byte> stringDataPtr;
	byte stringDataByte  = 0;
	uint16 typeObject_id = 0;
	uint16 typeString_id = 0;
	uint16 typeSaid_id   = 0;

	uint16 blockType = 0;
	uint16 blockSize = 0;

	_offsetLookupArray.clear();
	_offsetLookupObjectCount = 0;
	_offsetLookupStringCount = 0;
	_offsetLookupSaidCount = 0;
	_codeOffset = 0;

	if (getSciVersion() < SCI_VERSION_1_1) {
		// SCI0 + SCI1
		scriptDataPtr = *_buf;

		// Go through all blocks
		if (getSciVersion() == SCI_VERSION_0_EARLY) {
			if (scriptDataPtr.size() < 2)
				error("Script::identifyOffsets(): unexpected end of script %d", _nr);
			scriptDataPtr += 2;
		}

		for (;;) {
			if (scriptDataPtr.size() < 2)
				error("Script::identifyOffsets(): unexpected end of script %d", _nr);

			blockType = scriptDataPtr.getUint16LEAt(0);
			scriptDataPtr += 2;
			if (blockType == 0) // end of blocks detected
				break;

			if (scriptDataPtr.size() < 2)
				error("Script::identifyOffsets(): unexpected end of script %d", _nr);

			blockSize = scriptDataPtr.getUint16LEAt(0);
			if (blockSize < 4)
				error("Script::identifyOffsets(): invalid block size in script %d", _nr);
			blockSize     -= 4; // block size includes block-type UINT16 and block-size UINT16
			scriptDataPtr += 2;

			if (scriptDataPtr.size() < blockSize)
				error("Script::identifyOffsets(): invalid block size in script %d", _nr);

			switch (blockType) {
			case SCI_OBJ_OBJECT:
			case SCI_OBJ_CLASS:
				typeObject_id++;
				arrayEntry.type       = SCI_SCR_OFFSET_TYPE_OBJECT;
				arrayEntry.id         = typeObject_id;
				arrayEntry.offset     = scriptDataPtr - *_buf + 8; // Calculate offset inside script data (VM uses +8)
				arrayEntry.stringSize = 0;
				_offsetLookupArray.push_back(arrayEntry);
				_offsetLookupObjectCount++;
				break;

			case SCI_OBJ_STRINGS:
				// string block detected, we now grab all NUL terminated strings out of this block
				stringDataPtr = scriptDataPtr.subspan(0, blockSize);

				arrayEntry.type       = SCI_SCR_OFFSET_TYPE_STRING;

				for (;;) {
					if (stringDataPtr.size() < 1) // no more bytes left
						break;

					stringStartPtr = stringDataPtr;

					if (stringDataPtr.size() == 1) {
						// only 1 byte left and that byte is a [00], in that case we also exit
						stringDataByte = *stringStartPtr;
						if (stringDataByte == 0x00)
							break;
					}

					// now look for terminating [NUL]
					for (;;) {
						stringDataByte = *stringDataPtr;
						stringDataPtr++;
						if (!stringDataByte) // NUL found, exit this loop
							break;
						if (stringDataPtr.size() < 1) {
							// no more bytes left
							warning("Script::identifyOffsets(): string without terminating NUL in script %d", _nr);
							break;
						}
					}

					if (stringDataByte)
						break;

					typeString_id++;
					arrayEntry.id         = typeString_id;
					arrayEntry.offset     = stringStartPtr - *_buf; // Calculate offset inside script data
					arrayEntry.stringSize = stringDataPtr - stringStartPtr;
					_offsetLookupArray.push_back(arrayEntry);
					_offsetLookupStringCount++;
				}
				break;

			case SCI_OBJ_SAID:
				// said block detected, we now try to find every single said "string" inside this block
				// said strings are terminated with a 0xFF, the string itself may contain words (2 bytes), where
				//  the second byte of a word may also be a 0xFF.
				stringDataPtr  = scriptDataPtr.subspan(0, blockSize);

				arrayEntry.type       = SCI_SCR_OFFSET_TYPE_SAID;

				for (;;) {
					if (stringDataPtr.size() < 1) // no more bytes left
						break;

					stringStartPtr = stringDataPtr;
					if (stringDataPtr.size() == 1) {
						// only 1 byte left and that byte is a [00], in that case we also exit
						// happens in some scripts, for example Conquests of Camelot, script 997
						// may have been a bug in the compiler or just an intentional filler byte
						stringDataByte = *stringStartPtr;
						if (stringDataByte == 0x00)
							break;
					}

					// now look for terminating 0xFF
					for (;;) {
						stringDataByte = *stringDataPtr;
						stringDataPtr++;
						if (stringDataByte == 0xFF) // Terminator found, exit this loop
							break;
						if (stringDataPtr.size() < 1) // no more bytes left
							error("Script::identifyOffsets(): said-string without terminator in script %d", _nr);
						if (stringDataByte < 0xF0) {
							// Part of a word, skip second byte
							stringDataPtr++;
							if (stringDataPtr.size() < 1) // no more bytes left
								error("Script::identifyOffsets(): said-string without terminator in script %d", _nr);
						}
					}

					typeSaid_id++;
					arrayEntry.id         = typeSaid_id;
					arrayEntry.offset     = stringStartPtr - *_buf; // Calculate offset inside script data
					arrayEntry.stringSize = 0;
					_offsetLookupArray.push_back(arrayEntry);
					_offsetLookupSaidCount++;
				}
				break;

			default:
				break;
			}

			scriptDataPtr  += blockSize;
		}

	} else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1_LATE) {
		// Strings in SCI1.1 up to SCI2 come after the object instances
		scriptDataPtr = _heap;

		enum {
			kExportSize = sizeof(uint16),
			kPropertySize = sizeof(uint16),
			kNumMethodsSize = sizeof(uint16),
			kPropDictEntrySize = 2,
			kMethDictEntrySize = 4
		};

		SciSpan<const byte> hunkPtr = _buf->subspan(kSci11ExportTableOffset + _numExports * kExportSize);

		if (scriptDataPtr.size() < 4)
			error("Script::identifyOffsets(): unexpected end of script in script %d", _nr);

		uint16 endOfStringOffset = scriptDataPtr.getUint16SEAt(0);
		uint16 objectStartOffset = scriptDataPtr.getUint16SEAt(2) * 2 + 4;

		if (scriptDataPtr.size() < objectStartOffset)
			error("Script::identifyOffsets(): object start is beyond heap size in script %d", _nr);
		if (scriptDataPtr.size() < endOfStringOffset)
			error("Script::identifyOffsets(): end of string is beyond heap size in script %d", _nr);

		SciSpan<const byte> endOfStringPtr = scriptDataPtr.subspan(endOfStringOffset);

		scriptDataPtr  += objectStartOffset;

		// go through all objects
		for (;;) {
			if (scriptDataPtr.size() < 2)
				error("Script::identifyOffsets(): unexpected end of script %d", _nr);

			blockType = scriptDataPtr.getUint16SEAt(0);
			scriptDataPtr  += 2;
			if (blockType != SCRIPT_OBJECT_MAGIC_NUMBER)
				break;

			// Object found, add offset of object
			typeObject_id++;
			arrayEntry.type       = SCI_SCR_OFFSET_TYPE_OBJECT;
			arrayEntry.id         = typeObject_id;
			arrayEntry.offset     = scriptDataPtr - *_buf - 2; // the VM uses a pointer to the Magic-Number
			arrayEntry.stringSize = 0;
			_offsetLookupArray.push_back(arrayEntry);
			_offsetLookupObjectCount++;

			if (scriptDataPtr.size() < 2)
				error("Script::identifyOffsets(): unexpected end of script in script %d", _nr);

			const uint16 numProperties = scriptDataPtr.getUint16SEAt(0);
			blockSize = numProperties * kPropertySize;
			if (blockSize < 4)
				error("Script::identifyOffsets(): invalid block size in script %d", _nr);
			scriptDataPtr  += 2;

			const uint16 scriptNum = scriptDataPtr.getUint16SEAt(6);

			if (scriptNum != 0xFFFF) {
				hunkPtr += numProperties * kPropDictEntrySize;
			}

			const uint16 numMethods = hunkPtr.getUint16SEAt(0);
			hunkPtr += kNumMethodsSize + numMethods * kMethDictEntrySize;

			blockSize -= 4; // blocksize contains UINT16 type and UINT16 size
			if (scriptDataPtr.size() < blockSize)
				error("Script::identifyOffsets(): invalid block size in script %d", _nr);

			scriptDataPtr  += blockSize;
		}

		_codeOffset = hunkPtr - *_buf;

		// now scriptDataPtr points to right at the start of the strings
		if (scriptDataPtr > endOfStringPtr)
			error("Script::identifyOffsets(): string block / end-of-string block mismatch in script %d", _nr);

		stringDataPtr = scriptDataPtr.subspan(0, endOfStringPtr - scriptDataPtr);

		arrayEntry.type       = SCI_SCR_OFFSET_TYPE_STRING;
		for (;;) {
			if (stringDataPtr.size() < 1) // no more bytes left
				break;

			stringStartPtr = stringDataPtr;
			// now look for terminating [NUL]
			for (;;) {
				stringDataByte = *stringDataPtr;
				stringDataPtr++;
				if (!stringDataByte) // NUL found, exit this loop
					break;
				if (stringDataPtr.size() < 1) {
				    // no more bytes left
					warning("Script::identifyOffsets(): string without terminating NUL in script %d", _nr);
					break;
				}
			}

			if (stringDataByte)
				break;

			typeString_id++;
			arrayEntry.id         = typeString_id;
			arrayEntry.offset     = stringStartPtr - *_buf; // Calculate offset inside script data
			arrayEntry.stringSize = stringDataPtr - stringStartPtr;
			_offsetLookupArray.push_back(arrayEntry);
			_offsetLookupStringCount++;
		}

	} else if (getSciVersion() == SCI_VERSION_3) {
		// SCI3
		uint32 sci3StringOffset = 0;
		uint32 sci3RelocationOffset = 0;
		uint32 sci3BoundaryOffset = 0;

		if (_buf->size() < 22)
			error("Script::identifyOffsets(): script %d smaller than expected SCI3-header", _nr);

		sci3StringOffset = _buf->getUint32LEAt(4);
		sci3RelocationOffset = _buf->getUint32LEAt(8);

		if (sci3RelocationOffset > _buf->size())
			error("Script::identifyOffsets(): relocation offset is beyond end of script %d", _nr);

		// First we get all the objects
		scriptDataPtr = getSci3ObjectsPointer();
		for (;;) {
			if (scriptDataPtr.size() < 2)
				error("Script::identifyOffsets(): unexpected end of script %d", _nr);

			blockType = scriptDataPtr.getUint16SEAt(0);
			scriptDataPtr += 2;
			if (blockType != SCRIPT_OBJECT_MAGIC_NUMBER)
				break;

			// Object found, add offset of object
			typeObject_id++;
			arrayEntry.type       = SCI_SCR_OFFSET_TYPE_OBJECT;
			arrayEntry.id         = typeObject_id;
			arrayEntry.offset     = scriptDataPtr - *_buf - 2; // the VM uses a pointer to the Magic-Number
			arrayEntry.stringSize = 0;
			_offsetLookupArray.push_back(arrayEntry);
			_offsetLookupObjectCount++;

			if (scriptDataPtr.size() < 2)
				error("Script::identifyOffsets(): unexpected end of script in script %d", _nr);

			blockSize = scriptDataPtr.getUint16SEAt(0);
			if (blockSize < 4)
				error("Script::identifyOffsets(): invalid block size in script %d", _nr);
			scriptDataPtr  += 2;
			blockSize -= 4; // blocksize contains UINT16 type and UINT16 size
			if (scriptDataPtr.size() < blockSize)
				error("Script::identifyOffsets(): invalid block size in script %d", _nr);

			scriptDataPtr  += blockSize;
		}

		// And now we get all the strings
		if (sci3StringOffset > 0) {
			// string offset set, we expect strings
			if (sci3StringOffset > _buf->size())
				error("Script::identifyOffsets(): string offset is beyond end of script %d", _nr);

			if (sci3RelocationOffset < sci3StringOffset)
				error("Script::identifyOffsets(): string offset points beyond relocation offset in script %d", _nr);

			stringDataPtr  = _buf->subspan(sci3StringOffset, sci3RelocationOffset - sci3StringOffset);

			arrayEntry.type       = SCI_SCR_OFFSET_TYPE_STRING;

			for (;;) {
				if (stringDataPtr.size() < 1) // no more bytes left
					break;

				stringStartPtr = stringDataPtr;

				if (stringDataPtr.size() == 1) {
					// only 1 byte left and that byte is a [00], in that case we also exit
					stringDataByte = *stringStartPtr;
					if (stringDataByte == 0x00)
						break;
				}

				// now look for terminating [NUL]
				for (;;) {
					stringDataByte = *stringDataPtr;
					stringDataPtr++;
					if (!stringDataByte) // NUL found, exit this loop
						break;
					if (stringDataPtr.size() < 1) {
						// no more bytes left
						warning("Script::identifyOffsets(): string without terminating NUL in script %d", _nr);
						break;
					}
				}

				if (stringDataByte)
					break;

				typeString_id++;
				arrayEntry.id         = typeString_id;
				arrayEntry.offset     = stringStartPtr - *_buf; // Calculate offset inside script data
				arrayEntry.stringSize = stringDataPtr - stringStartPtr;
				_offsetLookupArray.push_back(arrayEntry);
				_offsetLookupStringCount++;

				// SCI3 seems to have aligned all string on DWORD boundaries
				sci3BoundaryOffset = stringDataPtr - *_buf; // Calculate current offset inside script data
				sci3BoundaryOffset = sci3BoundaryOffset & 3; // Check boundary offset
				if (sci3BoundaryOffset) {
					// lower 2 bits are set? Then we have to adjust the offset
					sci3BoundaryOffset = 4 - sci3BoundaryOffset;
					if (stringDataPtr.size() < sci3BoundaryOffset)
						error("Script::identifyOffsets(): SCI3 string boundary adjustment goes beyond end of string block in script %d", _nr);
					stringDataPtr += sci3BoundaryOffset;
				}
			}
		}
	}
}

SciSpan<const byte> Script::getSci3ObjectsPointer() {
	SciSpan<const byte> ptr;

	// SCI3 local variables always start dword-aligned
	if (_numExports % 2)
		ptr = _buf->subspan(22 + _numExports * sizeof(uint16));
	else
		ptr = _buf->subspan(24 + _numExports * sizeof(uint16));

	// SCI3 object structures always start dword-aligned
	if (_localsCount % 2)
		ptr += 2 + _localsCount * sizeof(uint16);
	else
		ptr += _localsCount * sizeof(uint16);

	return ptr;
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
		obj_pos.incOffset(8);	// magic offset (SCRIPT_OBJECT_MAGIC_OFFSET)

	if (obj_pos.getOffset() >= _buf->size())
		error("Attempt to initialize object beyond end of script");

	// Get the object at the specified position and init it. This will
	// automatically "allocate" space for it in the _objects map if necessary.
	Object *obj = &_objects[obj_pos.getOffset()];
	obj->init(*_buf, obj_pos, fullObjectInit);

	return obj;
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
	block[idx].setSegment(segment); // Perform relocation
	if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1_LATE)
		block[idx].incOffset(scriptSize);

	return true;
}

int Script::relocateOffsetSci3(uint32 offset) const {
	int relocStart = _buf->getUint32LEAt(8);
	int relocCount = _buf->getUint16LEAt(18);
	SciSpan <const byte> seeker = _buf->subspan(relocStart);

	for (int i = 0; i < relocCount; ++i) {
		if (seeker.getUint32SEAt(0) == offset) {
			return _buf->getUint16SEAt(offset) + seeker.getUint32SEAt(4);
		}
		seeker += 10;
	}

	return -1;
}

bool Script::relocateLocal(SegmentId segment, int location) {
	if (_localsBlock)
		return relocateBlock(_localsBlock->_locals, _localsOffset, segment, location, _script.size());
	else
		return false;
}

void Script::relocateSci0Sci21(reg_t block) {
	SciSpan<const byte> heap = *_buf;
	uint16 heapOffset = 0;

	if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1_LATE) {
		heap = _heap;
		heapOffset = _script.size();
	}

	if (block.getOffset() >= (uint16)heap.size() ||
		heap.getUint16SEAt(block.getOffset()) * 2 + block.getOffset() >= (uint16)heap.size())
	    error("Relocation block outside of script");

	int count = heap.getUint16SEAt(block.getOffset());
	int exportIndex = 0;
	int pos = 0;

	for (int i = 0; i < count; i++) {
		pos = heap.getUint16SEAt(block.getOffset() + 2 + (exportIndex * 2)) + heapOffset;
		// This occurs in SCI01/SCI1 games where usually one export value is
		// zero. It seems that in this situation, we should skip the export and
		// move to the next one, though the total count of valid exports remains
		// the same
		if (!pos) {
			exportIndex++;
			pos = heap.getUint16SEAt(block.getOffset() + 2 + (exportIndex * 2)) + heapOffset;
			if (!pos)
				error("Script::relocate(): Consecutive zero exports found");
		}

		// In SCI0-SCI1, script local variables, objects and code are relocated.
		// We only relocate locals and objects here, and ignore relocation of
		// code blocks. In SCI1.1 and newer versions, only locals and objects
		// are relocated.
		if (!relocateLocal(block.getSegment(), pos)) {
			// Not a local? It's probably an object or code block. If it's an
			// object, relocate it.
			const ObjMap::iterator end = _objects.end();
			for (ObjMap::iterator it = _objects.begin(); it != end; ++it)
				if (it->_value.relocateSci0Sci21(block.getSegment(), pos, _script.size()))
					break;
		}

		exportIndex++;
	}
}

void Script::relocateSci3(reg_t block) {
	SciSpan<const byte> relocStart = _buf->subspan(_buf->getUint32SEAt(8));
	//int count = _bufSize - READ_SCI11ENDIAN_UINT32(_buf + 8);

	ObjMap::iterator it;
	for (it = _objects.begin(); it != _objects.end(); ++it) {
		SciSpan<const byte> seeker = relocStart;
		while (seeker.size()) {
			// TODO: Find out what UINT16 at (seeker + 8) means
			it->_value.relocateSci3(block.getSegment(),
						seeker.getUint32SEAt(0),
						seeker.getUint32SEAt(4),
						_script.size());
			seeker += 10;
		}
	}
}

void Script::incrementLockers() {
	assert(!_markedAsDeleted);
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
	assert(lockers == 0 || !_markedAsDeleted);
	_lockers = lockers;
}

uint32 Script::validateExportFunc(int pubfunct, bool relocSci3) {
	bool exportsAreWide = (g_sci->_features->detectLofsType() == SCI_VERSION_1_MIDDLE);

	if (_numExports <= (uint)pubfunct) {
		error("validateExportFunc(): pubfunct is invalid");
		return 0;
	}

	if (exportsAreWide)
		pubfunct *= 2;

	uint32 offset;

	if (getSciVersion() != SCI_VERSION_3) {
		offset = _exports.getUint16SEAt(pubfunct);
	} else {
		if (!relocSci3)
			offset = _exports.getUint16SEAt(pubfunct) + getCodeBlockOffsetSci3();
		else
			offset = relocateOffsetSci3(pubfunct * 2 + 22);
	}

	// TODO: Check if this should be done for SCI1.1 games as well
	if (getSciVersion() >= SCI_VERSION_2 && offset == 0) {
		offset = _codeOffset;
	}

	if (offset >= _buf->size())
		error("Invalid export function pointer");

	return offset;
}

SciSpan<const byte> Script::findBlockSCI0(ScriptObjectTypes type, bool findLastBlock) {
	SciSpan<const byte> foundBlock;

	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

	SciSpan<const byte> buf = *_buf;
	if (oldScriptHeader)
		buf += 2;

	for (;;) {
		const int blockType = buf.getUint16LEAt(0);

		if (blockType == 0)
			break;

		// the size in the block header includes the size of the header itself
		const int blockSize = buf.getUint16LEAt(2);
		assert(blockSize > 0);

		if (blockType == type) {
			foundBlock = buf.subspan(0, blockSize, Common::String::format("%s, %s block", _buf->name().c_str(), sciObjectTypeNames[type]));;

			if (!findLastBlock) {
				break;
			}
		}

		buf += blockSize;
	}

	return foundBlock;
}

// memory operations

bool Script::isValidOffset(uint16 offset) const {
	return offset < _buf->size();
}

SegmentRef Script::dereference(reg_t pointer) {
	if (pointer.getOffset() > _buf->size()) {
		error("Script::dereference(): Attempt to dereference invalid pointer %04x:%04x into script segment (script size=%u)",
				  PRINT_REG(pointer), _buf->size());
		return SegmentRef();
	}

	SegmentRef ret;
	ret.isRaw = true;
	ret.maxSize = _buf->size() - pointer.getOffset();
	ret.raw = const_cast<byte *>(_buf->getUnsafeDataAt(pointer.getOffset(), ret.maxSize));
	return ret;
}

LocalVariables *Script::allocLocalsSegment(SegManager *segMan) {
	if (!getLocalsCount()) { // No locals
		return NULL;
	} else {
		LocalVariables *locals;

		if (_localsSegment) {
			locals = (LocalVariables *)segMan->getSegment(_localsSegment, SEG_TYPE_LOCALS);
			if (!locals || locals->getType() != SEG_TYPE_LOCALS || locals->script_id != getScriptNumber())
				error("Invalid script locals segment while allocating locals");
		} else
			locals = (LocalVariables *)segMan->allocSegment(new LocalVariables(), &_localsSegment);

		_localsBlock = locals;
		locals->script_id = getScriptNumber();
		locals->_locals.resize(getLocalsCount());

		return locals;
	}
}

void Script::initializeLocals(SegManager *segMan) {
	LocalVariables *locals = allocLocalsSegment(segMan);
	if (locals) {
		if (getSciVersion() > SCI_VERSION_0_EARLY) {
			const SciSpan<const byte> base = _buf->subspan(getLocalsOffset());

			for (uint16 i = 0; i < getLocalsCount(); i++)
				locals->_locals[i] = make_reg(0, base.getUint16SEAt(i * 2));
		} else {
			// In SCI0 early, locals are set at run time, thus zero them all here
			for (uint16 i = 0; i < getLocalsCount(); i++)
				locals->_locals[i] = NULL_REG;
		}
	}
}

void Script::syncLocalsBlock(SegManager *segMan) {
	_localsBlock = (_localsSegment == 0) ? NULL : (LocalVariables *)(segMan->getSegment(_localsSegment, SEG_TYPE_LOCALS));
}

void Script::initializeClasses(SegManager *segMan) {
	SciSpan<const byte> seeker;
	uint16 mult = 0;

	if (getSciVersion() <= SCI_VERSION_1_LATE) {
		seeker = _script;
		mult = 1;

		// SCI0 early has an extra two bytes of header
		if (getSciVersion() == SCI_VERSION_0_EARLY) {
			seeker += 2;
		}
	} else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1_LATE) {
		seeker = _heap.subspan(4 + _heap.getUint16SEAt(2) * 2);
		mult = 2;
	} else if (getSciVersion() == SCI_VERSION_3) {
		seeker = getSci3ObjectsPointer();
		mult = 1;
	}

	if (!seeker)
		return;

	uint16 marker;
	bool isClass = false;
	uint32 classpos;
	int16 species = 0;

	for (;;) {
		// In SCI0-SCI1, this is the segment type. In SCI11, it's a marker (0x1234)
		marker = seeker.getUint16SEAt(0);
		classpos = seeker - *_buf;

		if (getSciVersion() <= SCI_VERSION_1_LATE && !marker)
			break;

		if (getSciVersion() >= SCI_VERSION_1_1 && marker != SCRIPT_OBJECT_MAGIC_NUMBER)
			break;

		if (getSciVersion() <= SCI_VERSION_1_LATE) {
			isClass = (marker == SCI_OBJ_CLASS);
			if (isClass)
				species = seeker.getUint16SEAt(12);
			classpos += 12;
		} else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1_LATE) {
			isClass = (seeker.getUint16SEAt(14) & kInfoFlagClass);	// -info- selector
			species = seeker.getUint16SEAt(10);
		} else if (getSciVersion() == SCI_VERSION_3) {
			isClass = (seeker.getUint16SEAt(10) & kInfoFlagClass);
			species = seeker.getUint16SEAt(4);
		}

		if (isClass) {
			// WORKAROUNDs for off-by-one script errors
			if (species == (int)segMan->classTableSize()) {
				if (g_sci->getGameId() == GID_LSL2 && g_sci->isDemo())
					segMan->resizeClassTable(species + 1);
				else if (g_sci->getGameId() == GID_LSL3 && !g_sci->isDemo() && _nr == 500)
					segMan->resizeClassTable(species + 1);
				else if (g_sci->getGameId() == GID_SQ3 && !g_sci->isDemo() && _nr == 93)
					segMan->resizeClassTable(species + 1);
				else if (g_sci->getGameId() == GID_SQ3 && !g_sci->isDemo() && _nr == 99)
					segMan->resizeClassTable(species + 1);
			}

			if (species < 0 || species >= (int)segMan->classTableSize())
				error("Invalid species %d(0x%x) unknown max %d(0x%x) while instantiating script %d\n",
						  species, species, segMan->classTableSize(), segMan->classTableSize(), _nr);

			SegmentId segmentId = segMan->getScriptSegment(_nr);
			segMan->setClassOffset(species, make_reg(segmentId, classpos));
		}

		seeker += seeker.getUint16SEAt(2) * mult;
	}
}

void Script::initializeObjectsSci0(SegManager *segMan, SegmentId segmentId) {
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

	// We need to make two passes, as the objects in the script might be in the
	// wrong order (e.g. in the demo of Iceman) - refer to bug #3034713
	for (int pass = 1; pass <= 2; pass++) {
		SciSpan<const byte> seeker = _buf->subspan(oldScriptHeader ? 2 : 0);

		do {
			uint16 objType = seeker.getUint16SEAt(0);
			if (!objType)
				break;

			switch (objType) {
			case SCI_OBJ_OBJECT:
			case SCI_OBJ_CLASS:
				{
					reg_t addr = make_reg(segmentId, seeker - *_buf + 4);
					Object *obj = scriptObjInit(addr);
					obj->initSpecies(segMan, addr);

					if (pass == 2) {
						if (!obj->initBaseObject(segMan, addr)) {
							if ((_nr == 202 || _nr == 764) && g_sci->getGameId() == GID_KQ5) {
								// WORKAROUND: Script 202 of KQ5 French and German
								// (perhaps Spanish too?) has an invalid object.
								// This is non-fatal. Refer to bugs #3035396 and
								// #3150767.
								// Same happens with script 764, it seems to
								// contain junk towards its end.
								_objects.erase(addr.toUint16() - SCRIPT_OBJECT_MAGIC_OFFSET);
							} else {
								error("Failed to locate base object for object at %04X:%04X", PRINT_REG(addr));
							}
						}
					}
				}
				break;

			default:
				break;
			}

			seeker += seeker.getUint16SEAt(2);
		} while ((uint32)(seeker - *_buf) < getScriptSize() - 2);
	}

	const SciSpan<const byte> relocationBlock = findBlockSCI0(SCI_OBJ_POINTERS);
	if (relocationBlock)
		relocateSci0Sci21(make_reg(segmentId, relocationBlock - *_buf + 4));
}

void Script::initializeObjectsSci11(SegManager *segMan, SegmentId segmentId) {
	SciSpan<const byte> seeker = _heap.subspan(4 + _heap.getUint16SEAt(2) * 2);

	while (seeker.getUint16SEAt(0) == SCRIPT_OBJECT_MAGIC_NUMBER) {
		reg_t reg = make_reg(segmentId, seeker - *_buf);
		Object *obj = scriptObjInit(reg);

		// Copy base from species class, as we need its selector IDs
		obj->setSuperClassSelector(
			segMan->getClassAddress(obj->getSuperClassSelector().getOffset(), SCRIPT_GET_LOCK, 0));

		// -propDict- is used by Obj::isMemberOf to determine if an object
		// is an instance of a class. For classes, we therefore relocate
		// -propDict- to the script's segment. For instances, we copy
		// -propDict- from its class.
		// Example test case - room 381 of sq4cd - if isMemberOf() doesn't work,
		// talk-clicks on the robot will act like clicking on ego.
		if (obj->isClass()) {
			reg_t propDict = obj->getPropDictSelector();
			propDict.setSegment(segmentId);
			obj->setPropDictSelector(propDict);
		} else {
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

		seeker += seeker.getUint16SEAt(2) * 2;
	}

	relocateSci0Sci21(make_reg(segmentId, _heap.getUint16SEAt(0)));
}

void Script::initializeObjectsSci3(SegManager *segMan, SegmentId segmentId) {
	SciSpan<const byte> seeker = getSci3ObjectsPointer();

	while (seeker.getUint16SEAt(0) == SCRIPT_OBJECT_MAGIC_NUMBER) {
		// We call setSegment and setOffset directly here, instead of using
		// make_reg, as in large scripts, seeker - _buf can be larger than
		// a 16-bit integer
		reg_t reg;
		reg.setSegment(segmentId);
		reg.setOffset(seeker - *_buf);

		Object *obj = scriptObjInit(reg);
		obj->setSuperClassSelector(segMan->getClassAddress(obj->getSuperClassSelector().getOffset(), SCRIPT_GET_LOCK, 0));
		seeker += seeker.getUint16SEAt(2);
	}

	relocateSci3(make_reg(segmentId, 0));
}

void Script::initializeObjects(SegManager *segMan, SegmentId segmentId) {
	if (getSciVersion() <= SCI_VERSION_1_LATE)
		initializeObjectsSci0(segMan, segmentId);
	else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1_LATE)
		initializeObjectsSci11(segMan, segmentId);
	else if (getSciVersion() == SCI_VERSION_3)
		initializeObjectsSci3(segMan, segmentId);
}

reg_t Script::findCanonicAddress(SegManager *segMan, reg_t addr) const {
	addr.setOffset(0);
	return addr;
}

void Script::freeAtAddress(SegManager *segMan, reg_t addr) {
	/*
		debugC(kDebugLevelGC, "[GC] Freeing script %04x:%04x", PRINT_REG(addr));
		if (_localsSegment)
			debugC(kDebugLevelGC, "[GC] Freeing locals %04x:0000", _localsSegment);
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
	if (addr.getOffset() <= _buf->size() && addr.getOffset() >= (uint)-SCRIPT_OBJECT_MAGIC_OFFSET && offsetIsObject(addr.getOffset())) {
		const Object *obj = getObject(addr.getOffset());
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

bool Script::offsetIsObject(uint16 offset) const {
	return _buf->getUint16SEAt(offset + SCRIPT_OBJECT_MAGIC_OFFSET) == SCRIPT_OBJECT_MAGIC_NUMBER;
}

} // End of namespace Sci
