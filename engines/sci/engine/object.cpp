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


#include "sci/engine/kernel.h"
#include "sci/engine/object.h"
#include "sci/engine/seg_manager.h"

namespace Sci {

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
	if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1)
		block[idx].offset += scriptSize;

	return true;
}

void Object::init(byte *buf, reg_t obj_pos, bool initVariables) {
	byte *data = buf + obj_pos.offset;
	_baseObj = data;
	_pos = obj_pos;

	if (getSciVersion() <= SCI_VERSION_1_LATE) {
		_variables.resize(READ_LE_UINT16(data + kOffsetSelectorCounter));
		_baseVars = (uint16 *)(_baseObj + _variables.size() * 2);
		_methodCount = READ_LE_UINT16(data + READ_LE_UINT16(data + kOffsetFunctionArea) - 2);
		_baseMethod = Common::Array<uint16>((const uint16 *)(data + READ_LE_UINT16(data + kOffsetFunctionArea)),
						    _methodCount*2+2);
	} else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1) {
		_variables.resize(READ_SCI11ENDIAN_UINT16(data + 2));
		_baseVars = (uint16 *)(buf + READ_SCI11ENDIAN_UINT16(data + 4));
		_methodCount = READ_SCI11ENDIAN_UINT16(buf + READ_SCI11ENDIAN_UINT16(data + 6));
		_baseMethod = Common::Array<uint16>((const uint16 *) (buf + READ_SCI11ENDIAN_UINT16(data + 6)),
						    _methodCount*2+3);
	} else if (getSciVersion() == SCI_VERSION_3) {
		initSelectorsSci3(buf);
	}

	if (initVariables) {
		if (getSciVersion() <= SCI_VERSION_2_1) {
			for (uint i = 0; i < _variables.size(); i++)
				_variables[i] = make_reg(0, READ_SCI11ENDIAN_UINT16(data + (i * 2)));
		} else {
			_infoSelectorSci3 = make_reg(0, READ_SCI11ENDIAN_UINT16(_baseObj + 10));
		}
	}
}

const Object *Object::getClass(SegManager *segMan) const {
	return isClass() ? this : segMan->getObject(getSuperClassSelector());
}

int Object::locateVarSelector(SegManager *segMan, Selector slc) const {
	const byte *buf = 0;
	uint varnum = 0;

	if (getSciVersion() <= SCI_VERSION_2_1) {
		const Object *obj = getClass(segMan);
		varnum = getSciVersion() <= SCI_VERSION_1_LATE ? getVarCount() : obj->getVariable(1).toUint16();
		buf = (const byte *)obj->_baseVars;
	} else if (getSciVersion() == SCI_VERSION_3) {
		varnum = _variables.size();
		buf = (const byte *)_baseVars;
	}

	for (uint i = 0; i < varnum; i++)
		if (READ_SCI11ENDIAN_UINT16(buf + (i << 1)) == slc) // Found it?
			return i; // report success

	return -1; // Failed
}

bool Object::relocateSci0Sci21(SegmentId segment, int location, size_t scriptSize) {
	return relocateBlock(_variables, getPos().offset, segment, location, scriptSize);
}

bool Object::relocateSci3(SegmentId segment, int location, int offset, size_t scriptSize) {
	assert(_propertyOffsetsSci3);

	for (uint i = 0; i < _variables.size(); ++i) {
		if (location == _propertyOffsetsSci3[i]) {
			_variables[i].segment = segment;
			_variables[i].offset += offset;
			return true;
		}
	}

	return false;
}

int Object::propertyOffsetToId(SegManager *segMan, int propertyOffset) const {
	int selectors = getVarCount();

	if (propertyOffset < 0 || (propertyOffset >> 1) >= selectors) {
		error("Applied propertyOffsetToId to invalid property offset %x (property #%d not in [0..%d])",
		          propertyOffset, propertyOffset >> 1, selectors - 1);
		return -1;
	}

	if (getSciVersion() < SCI_VERSION_1_1) {
		const byte *selectoroffset = ((const byte *)(_baseObj)) + kOffsetSelectorSegment + selectors * 2;
		return READ_SCI11ENDIAN_UINT16(selectoroffset + propertyOffset);
	} else {
		const Object *obj = this;
		if (!isClass())
			obj = segMan->getObject(getSuperClassSelector());

		return READ_SCI11ENDIAN_UINT16((const byte *)obj->_baseVars + propertyOffset);
	}
}

void Object::initSpecies(SegManager *segMan, reg_t addr) {
	uint16 speciesOffset = getSpeciesSelector().offset;

	if (speciesOffset == 0xffff)		// -1
		setSpeciesSelector(NULL_REG);	// no species
	else
		setSpeciesSelector(segMan->getClassAddress(speciesOffset, SCRIPT_GET_LOCK, addr));
}

void Object::initSuperClass(SegManager *segMan, reg_t addr) {
	uint16 superClassOffset = getSuperClassSelector().offset;

	if (superClassOffset == 0xffff)			// -1
		setSuperClassSelector(NULL_REG);	// no superclass
	else
		setSuperClassSelector(segMan->getClassAddress(superClassOffset, SCRIPT_GET_LOCK, addr));
}

bool Object::initBaseObject(SegManager *segMan, reg_t addr, bool doInitSuperClass) {
	const Object *baseObj = segMan->getObject(getSpeciesSelector());

	if (baseObj) {
		_variables.resize(baseObj->getVarCount());
		// Copy base from species class, as we need its selector IDs
		_baseObj = baseObj->_baseObj;
		if (doInitSuperClass)
			initSuperClass(segMan, addr);
		return true;
	}

	return false;
}

const int EXTRA_GROUPS = 3;

void Object::initSelectorsSci3(const byte *buf) {
	const byte *groupInfo = _baseObj + 16;
	const byte *selectorBase = groupInfo + EXTRA_GROUPS * 32 * 2;
	int groups = g_sci->getKernel()->getSelectorNamesSize()/32;
	int methods, properties;

	if (g_sci->getKernel()->getSelectorNamesSize() % 32)
		++groups;

	methods = properties = 0;

	// Selectors are divided into groups of 32, of which the first
	// two selectors are always reserved (because their storage
	// space is used by the typeMask).
	// We don't know beforehand how many methods and properties
	// there are, so we count them first. 
	for (int groupNr = 0; groupNr < groups; ++groupNr) {
		byte groupLocation = groupInfo[groupNr];
		const byte *seeker = selectorBase + groupLocation * 32 * 2;

		if (groupLocation != 0)	{
			// This object actually has selectors belonging to this group 
			int typeMask = READ_SCI11ENDIAN_UINT32(seeker);

			for (int bit = 2; bit < 32; ++bit) {
				int value = READ_SCI11ENDIAN_UINT16(seeker + bit * 2);
				if (typeMask & (1 << bit)) { // Property
					++properties;
				} else if (value != 0xffff) { // Method
					++methods;
				} else {
					// Undefined selector
				}
				       
			}
		}
	}

	_variables.resize(properties);
	uint16 *propertyIds = (uint16 *) malloc(sizeof(uint16) * properties);
//	uint16 *methodOffsets = (uint16 *) malloc(sizeof(uint16) * 2 * methods);
	uint16 *propertyOffsets = (uint16 *) malloc(sizeof(uint16) * properties);
	int propertyCounter = 0;
	int methodCounter = 0;

	// Go through the whole thing again to get the property values
	// and method pointers
	for (int groupNr = 0; groupNr < groups; ++groupNr) {
		byte groupLocation = groupInfo[groupNr];
		const byte *seeker = selectorBase + groupLocation * 32 * 2;

		if (groupLocation != 0)	{
			// This object actually has selectors belonging to this group 
			int typeMask = READ_SCI11ENDIAN_UINT32(seeker);
			int groupBaseId = groupNr * 32;

			for (int bit = 2; bit < 32; ++bit) {
				int value = READ_SCI11ENDIAN_UINT16(seeker + bit * 2);
				if (typeMask & (1 << bit)) { // Property
					propertyIds[propertyCounter] = groupBaseId + bit;
					_variables[propertyCounter] = make_reg(0, value);
					propertyOffsets[propertyCounter] = (seeker + bit * 2) - buf;
					++propertyCounter;
				} else if (value != 0xffff) { // Method
					_baseMethod.push_back(groupBaseId + bit);
					_baseMethod.push_back(value + READ_SCI11ENDIAN_UINT32(buf));
//					methodOffsets[methodCounter] = (seeker + bit * 2) - buf;
					++methodCounter;
				} else /* Undefined selector */ {};
				       
			}
		}
	}

	_speciesSelectorSci3 = make_reg(0, READ_SCI11ENDIAN_UINT16(_baseObj + 4));
	_superClassPosSci3 = make_reg(0, READ_SCI11ENDIAN_UINT16(_baseObj + 8));

	_baseVars = propertyIds;
	_methodCount = methods;
	_propertyOffsetsSci3 = propertyOffsets;
	//_methodOffsetsSci3 = methodOffsets;
}

} // End of namespace Sci
