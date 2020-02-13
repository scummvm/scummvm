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

void Script::freeScript(const bool keepLocalsSegment) {
	_nr = 0;

	_buf.clear();
	_script.clear();
	_heap.clear();
	_exports.clear();
	_numExports = 0;
	_synonyms.clear();
	_numSynonyms = 0;

	_localsOffset = 0;
	if (!keepLocalsSegment) {
		_localsSegment = 0;
	}
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

void Script::load(int script_nr, ResourceManager *resMan, ScriptPatcher *scriptPatcher, bool applyScriptPatches) {
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
		// script format, and they can exceed the 64KB boundary using relocation.
		Resource *heap = resMan->findResource(ResourceId(kResourceTypeHeap, script_nr), false);
		bufSize += heap->size();

		// Ensure that the start of the heap resource can be word-aligned.
		if (script->size() & 2) {
			++bufSize;
			++scriptSize;
		}

		// As mentioned above, the script and the heap together should not exceed 64KB
		if (script->size() + heap->size() > 65535)
			error("Script and heap %d sizes combined exceed 64K. This means a fundamental "
					"design bug was made regarding SCI1.1 and newer games.\n"
					"Please report this error to the ScummVM team", script_nr);
	} else if (getSciVersion() == SCI_VERSION_3 && script->size() > 0x3FFFF) {
		error("Script %d size exceeds 256K (it is %u bytes).\n"
			  "Please report this error to the ScummVM team", script_nr, script->size());
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
	if (applyScriptPatches)
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

		_localsOffset = getHeapOffset() + 4;
		_localsCount = _buf->getUint16SEAt(_localsOffset - 2);
	} else if (getSciVersion() == SCI_VERSION_3) {
		_localsCount = _buf->getUint16LEAt(12);
		_numExports = _buf->getUint16LEAt(20);
		if (_numExports) {
			_exports = _buf->subspan<const uint16>(22, _numExports * sizeof(uint16));
		}

		// SCI3 local variables always start dword-aligned
		if (_numExports % 2)
			_localsOffset = 22 + _numExports * sizeof(uint16);
		else
			_localsOffset = 24 + _numExports * sizeof(uint16);
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
			error("Locals extend beyond end of script %d: offset %04x, count %u vs size %u", _nr, _localsOffset, _localsCount, _buf->size());
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

#ifdef ENABLE_SCI32
	} else if (getSciVersion() == SCI_VERSION_3) {
		// SCI3
		uint32 sci3StringOffset = 0;
		uint32 sci3RelocationOffset = 0;
		uint32 sci3BoundaryOffset = 0;

		if (_buf->size() < 22)
			error("Script::identifyOffsets(): script %d smaller than expected SCI3-header", _nr);

		_codeOffset = _buf->getUint32LEAt(0);
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
#endif
	}
}

#ifdef ENABLE_SCI32
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
#endif

Object *Script::getObject(uint32 offset) {
	if (_objects.contains(offset))
		return &_objects[offset];
	else
		return 0;
}

const Object *Script::getObject(uint32 offset) const {
	if (_objects.contains(offset))
		return &_objects[offset];
	else
		return 0;
}

Object *Script::scriptObjInit(reg_t obj_pos, bool fullObjectInit) {
	if (obj_pos.getOffset() >= _buf->size())
		error("Attempt to initialize object beyond end of script %d (%u >= %u)", _nr, obj_pos.getOffset(), _buf->size());

	// Get the object at the specified position and init it. This will
	// automatically "allocate" space for it in the _objects map if necessary.
	Object *obj = &_objects[obj_pos.getOffset()];
	obj->init(*this, obj_pos, fullObjectInit);

	return obj;
}

bool relocateBlock(Common::Array<reg_t> &block, int block_location, SegmentId segment, int location, uint32 heapOffset) {
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

	block[idx].setSegment(segment);
	block[idx].incOffset(heapOffset);
	return true;
}

#ifdef ENABLE_SCI32
int Script::relocateOffsetSci3(uint32 offset) const {
	int relocStart = _buf->getUint32LEAt(8);
	int relocCount = _buf->getUint16LEAt(18);
	SciSpan<const byte> seeker = _buf->subspan(relocStart);

	for (int i = 0; i < relocCount; ++i) {
		if (seeker.getUint32SEAt(0) == offset) {
			return _buf->getUint16SEAt(offset) + seeker.getUint32SEAt(4);
		}
		seeker += 10;
	}

	return -1;
}
#endif

bool Script::relocateLocal(SegmentId segment, int location, uint32 offset) {
	if (_localsBlock)
		return relocateBlock(_localsBlock->_locals, _localsOffset, segment, location, offset);
	else
		return false;
}

uint32 Script::getRelocationOffset(const uint32 offset) const {
	if (getSciVersion() == SCI_VERSION_3) {
		SciSpan<const byte> relocStart = _buf->subspan(_buf->getUint32SEAt(8));
		const uint relocCount = _buf->getUint16SEAt(18);

		for (uint i = 0; i < relocCount; ++i) {
			if (offset == relocStart.getUint32SEAt(0)) {
				return relocStart.getUint32SEAt(4);
			}
			relocStart += 10;
		}
	} else {
		const SciSpan<const uint16> relocTable = getRelocationTableSci0Sci21();
		for (uint i = 0; i < relocTable.size(); ++i) {
			if (relocTable.getUint16SEAt(i) == offset) {
				return getHeapOffset();
			}
		}
	}

	return kNoRelocation;
}

const SciSpan<const uint16> Script::getRelocationTableSci0Sci21() const {
	SciSpan<const byte> relocationBlock;
	uint16 numEntries;
	uint16 dataOffset;

	if (getSciVersion() < SCI_VERSION_1_1) {
		relocationBlock = findBlockSCI0(SCI_OBJ_POINTERS);

		if (!relocationBlock) {
			return SciSpan<const uint16>();
		}

		if (relocationBlock != findBlockSCI0(SCI_OBJ_POINTERS, true)) {
			warning("Script %d has multiple relocation tables", _nr);
		}

		numEntries = relocationBlock.getUint16SEAt(4);

		if (!numEntries) {
			return SciSpan<const uint16>();
		}

		dataOffset = 6;

		// Starting somewhere around SQ1, and continuing through the rest of
		// SCI1, the relocation table in scripts started including an extra
		// null entry at the beginning of the table, without a corresponding
		// increase in the entry count. While this change is consistent in
		// most of the SCI1mid+ games (all scripts in LSL1, Jones CD,
		// EQ floppy, SQ1, LSL5, and Ms Astro Chicken have the null entry),
		// a few games include scripts without the null entry (Castle of Dr
		// Brain 947 & 997, PQ3 997, KQ5 CD 975 & 997). Since 0 is never a
		// valid relocation offset, we just skip it if we see it
		const uint16 firstEntry = relocationBlock.getUint16SEAt(6);
		if (firstEntry == 0) {
			dataOffset += 2;
		}
	} else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1_LATE) {
		relocationBlock = _heap.subspan(_heap.getUint16SEAt(0));

		if (!relocationBlock) {
			return SciSpan<const uint16>();
		}

		numEntries = relocationBlock.getUint16SEAt(0);

		if (!numEntries) {
			return SciSpan<const uint16>();
		}

		dataOffset = 2;
	} else {
		error("Invalid engine version called Script::getRelocationTableSci0Sci21 on script %d", _nr);
	}

	// This check should work correctly even with SCI1.1+ because the relocation
	// table is always at the very end of the heap in these games
	if (dataOffset + numEntries * sizeof(uint16) != relocationBlock.size()) {
		warning("Script %d unexpected relocation table size %u", _nr, relocationBlock.size());
	}

	return relocationBlock.subspan<const uint16>(dataOffset, numEntries * sizeof(uint16));
}

void Script::relocateSci0Sci21(const SegmentId segmentId) {
	const SciSpan<const uint16> relocEntries = getRelocationTableSci0Sci21();

	const uint32 heapOffset = getHeapOffset();

	for (uint i = 0; i < relocEntries.size(); ++i) {
		const uint pos = relocEntries.getUint16SEAt(i) + heapOffset;

		// In SCI0-SCI1, script local variables, objects and code are relocated.
		// We only relocate locals and objects here, and ignore relocation of
		// code blocks. In SCI1.1 and newer versions, only locals and objects
		// are relocated.
		if (!relocateLocal(segmentId, pos, getHeapOffset())) {
			// Not a local? It's probably an object or code block. If it's an
			// object, relocate it.
			const ObjMap::iterator end = _objects.end();
			for (ObjMap::iterator it = _objects.begin(); it != end; ++it)
				if (it->_value.relocateSci0Sci21(segmentId, pos, getHeapOffset()))
					break;
		}
	}
}

#ifdef ENABLE_SCI32
void Script::relocateSci3(const SegmentId segmentId) {
	SciSpan<const byte> relocEntry = _buf->subspan(_buf->getUint32SEAt(8));
	const uint relocCount = _buf->getUint16SEAt(18);

	for (uint i = 0; i < relocCount; ++i) {
		const uint location = relocEntry.getUint32SEAt(0);
		const uint offset = relocEntry.getUint32SEAt(4);

		if (!relocateLocal(segmentId, location, offset)) {
			const ObjMap::iterator end = _objects.end();
			for (ObjMap::iterator it = _objects.begin(); it != end; ++it) {
				if (it->_value.relocateSci3(segmentId, location, offset, _script.size())) {
					break;
				}
			}
		}

		relocEntry += 10;
	}
}
#endif

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
		error("Script %d validateExportFunc(): pubfunct %d is invalid", _nr, pubfunct);
	}

	if (exportsAreWide)
		pubfunct *= 2;

	int offset;

#ifdef ENABLE_SCI32
	if (getSciVersion() == SCI_VERSION_3) {
		if (!relocSci3) {
			offset = _exports.getUint16SEAt(pubfunct) + getCodeBlockOffset();
		} else {
			offset = relocateOffsetSci3(pubfunct * sizeof(uint16) + /* header size */ 22);
			// Some offsets below 0xFFFF are left as-is in the export table,
			// e.g. Lighthouse script 64990
			if (offset == -1) {
				offset = _exports.getUint16SEAt(pubfunct) + getCodeBlockOffset();
			}
		}
	} else
#endif
		offset = _exports.getUint16SEAt(pubfunct);

	// TODO: Check if this should be done for SCI1.1 games as well
	if (getSciVersion() >= SCI_VERSION_2 && offset == 0) {
		offset = getCodeBlockOffset();
	}

	if (offset == -1 || offset >= (int)_buf->size())
		error("Invalid export %d function pointer (%d) in script %d", pubfunct, offset, _nr);

	return offset;
}

SciSpan<const byte> Script::findBlockSCI0(ScriptObjectTypes type, bool findLastBlock) const {
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
			foundBlock = buf.subspan(0, blockSize, Common::String::format("%s, %s block", _buf->name().c_str(), sciObjectTypeNames[type]));

			if (!findLastBlock) {
				break;
			}
		}

		buf += blockSize;
	}

	return foundBlock;
}

// memory operations

bool Script::isValidOffset(uint32 offset) const {
	return offset < _buf->size();
}

SegmentRef Script::dereference(reg_t pointer) {
	if (pointer.getOffset() > _buf->size()) {
		error("Script::dereference(): Attempt to dereference invalid pointer %04x:%04x into script %d segment (script size=%u)",
				  PRINT_REG(pointer), _nr, _buf->size());
		return SegmentRef();
	}

	SegmentRef ret;
	ret.isRaw = true;
	ret.maxSize = _buf->size() - pointer.getOffset();
	ret.raw = _buf->getUnsafeDataAt(pointer.getOffset(), ret.maxSize);
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
				error("Invalid script %d locals segment while allocating locals", _nr);
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
				locals->_locals[i] = make_reg(0, base.getUint16SEAt(i * sizeof(uint16)));
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
#ifdef ENABLE_SCI32
	} else if (getSciVersion() == SCI_VERSION_3) {
		seeker = getSci3ObjectsPointer();
		mult = 1;
#endif
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
				error("Invalid species %d(0x%x) unknown max %d(0x%x) while instantiating script %d",
						  species, species, segMan->classTableSize(), segMan->classTableSize(), _nr);

			segMan->setClassOffset(species, make_reg32(segMan->getScriptSegment(_nr), classpos));
		}

		seeker += seeker.getUint16SEAt(2) * mult;
	}
}

void Script::initializeObjectsSci0(SegManager *segMan, SegmentId segmentId, bool applyScriptPatches) {
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
					reg_t addr = make_reg(segmentId, seeker - *_buf + 4 - SCRIPT_OBJECT_MAGIC_OFFSET);
					Object *obj;
					if (pass == 1) {
						obj = scriptObjInit(addr);
						obj->initSpecies(segMan, addr, applyScriptPatches);
					} else {
						obj = getObject(addr.getOffset());
						if (!obj->initBaseObject(segMan, addr, true, applyScriptPatches)) {
							if ((_nr == 202 || _nr == 764) && g_sci->getGameId() == GID_KQ5) {
								// WORKAROUND: Script 202 of KQ5 French and German
								// (perhaps Spanish too?) has an invalid object.
								// This is non-fatal. Refer to bugs #3035396 and
								// #3150767.
								// Same happens with script 764, it seems to
								// contain junk towards its end.
								_objects.erase(addr.toUint16() - SCRIPT_OBJECT_MAGIC_OFFSET);
							} else {
								error("Failed to locate base object for object at %04x:%04x in script %d", PRINT_REG(addr), _nr);
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

	relocateSci0Sci21(segmentId);
}

void Script::initializeObjectsSci11(SegManager *segMan, SegmentId segmentId, bool applyScriptPatches) {
	SciSpan<const byte> seeker = _heap.subspan(4 + _heap.getUint16SEAt(2) * 2);
	Common::Array<reg_t> mismatchedVarCountObjects;

	while (seeker.getUint16SEAt(0) == SCRIPT_OBJECT_MAGIC_NUMBER) {
		reg_t reg = make_reg(segmentId, seeker - *_buf);
		Object *obj = scriptObjInit(reg);

		// Copy base from species class, as we need its selector IDs
		obj->setSuperClassSelector(
			segMan->getClassAddress(obj->getSuperClassSelector().getOffset(), SCRIPT_GET_LOCK, 0, applyScriptPatches));

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

			// At least some versions of Island of Dr Brain have a bMessager
			// instance in script 0 with a var count greater than that of its
			// class; warn about this here to see if it shows up in any other
			// games
			if (obj->getVarCount() != classObj->getVarCount()) {
				// Warnings have to be deferred until after relocation for the
				// object name to be resolvable
				mismatchedVarCountObjects.push_back(reg);
			}
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

	relocateSci0Sci21(segmentId);

	for (uint i = 0; i < mismatchedVarCountObjects.size(); ++i) {
		const reg_t pos = mismatchedVarCountObjects[i];
		const Object *obj = segMan->getObject(pos);
		reg_t classObject = obj->getSuperClassSelector();
		const Object *classObj = segMan->getObject(classObject);

		warning("Object %04x:%04x (%s) from %s declares %d variables, "
				"but its class declares %d variables",
				PRINT_REG(pos),
				segMan->getObjectName(pos),
				_buf->name().c_str(),
				obj->getVarCount(),
				classObj->getVarCount());
	}
}

#ifdef ENABLE_SCI32
void Script::initializeObjectsSci3(SegManager *segMan, SegmentId segmentId, bool applyScriptPatches) {
	SciSpan<const byte> seeker = getSci3ObjectsPointer();

	while (seeker.getUint16SEAt(0) == SCRIPT_OBJECT_MAGIC_NUMBER) {
		Object *obj = scriptObjInit(make_reg32(segmentId, seeker - *_buf));
		obj->setSuperClassSelector(segMan->getClassAddress(obj->getSuperClassSelector().getOffset(), SCRIPT_GET_LOCK, 0, applyScriptPatches));
		seeker += seeker.getUint16SEAt(2);
	}

	relocateSci3(segmentId);
}
#endif

void Script::initializeObjects(SegManager *segMan, SegmentId segmentId, bool applyScriptPatches) {
	if (getSciVersion() <= SCI_VERSION_1_LATE)
		initializeObjectsSci0(segMan, segmentId, applyScriptPatches);
	else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1_LATE)
		initializeObjectsSci11(segMan, segmentId, applyScriptPatches);
#ifdef ENABLE_SCI32
	else if (getSciVersion() == SCI_VERSION_3)
		initializeObjectsSci3(segMan, segmentId, applyScriptPatches);
#endif
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
			error("Request for outgoing script-object reference at %04x:%04x failed in script %d", PRINT_REG(addr), _nr);
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

bool Script::offsetIsObject(uint32 offset) const {
	return _buf->getUint16SEAt(offset + SCRIPT_OBJECT_MAGIC_OFFSET) == SCRIPT_OBJECT_MAGIC_NUMBER;
}

} // End of namespace Sci
