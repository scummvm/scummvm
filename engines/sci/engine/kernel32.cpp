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

#ifdef ENABLE_SCI32

#include "sci/engine/kernel.h"
#include "sci/engine/segment.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/graphics/frameout.h"
#include "sci/graphics/screen.h"

#include "common/system.h"

namespace Sci {

// SCI2 Kernel Functions

reg_t kIsHiRes(EngineState *s, int argc, reg_t *argv) {
	// Returns 0 if the screen width or height is less than 640 or 400,
	// respectively.
	if (g_system->getWidth() < 640 || g_system->getHeight() < 400)
		return make_reg(0, 0);

	return make_reg(0, 1);
}

reg_t kArray(EngineState *s, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case 0: { // New
		reg_t arrayHandle;
		SciArray<reg_t> *array = s->_segMan->allocateArray(&arrayHandle);
		array->setType(argv[2].toUint16());
		array->setSize(argv[1].toUint16());
		return arrayHandle;
	}
	case 1: { // Size
		SciArray<reg_t> *array = s->_segMan->lookupArray(argv[1]);
		return make_reg(0, array->getSize());
	}
	case 2: { // At (return value at an index)
		SciArray<reg_t> *array = s->_segMan->lookupArray(argv[1]);
		return array->getValue(argv[2].toUint16());
	}
	case 3: { // Atput (put value at an index)
		SciArray<reg_t> *array = s->_segMan->lookupArray(argv[1]);

		uint32 index = argv[2].toUint16();
		uint32 count = argc - 3;

		if (index + count > 65535)
			break;

		if (array->getSize() < index + count)
			array->setSize(index + count);

		for (uint16 i = 0; i < count; i++)
			array->setValue(i + index, argv[i + 3]);

		return argv[1]; // We also have to return the handle
	}
	case 4: // Free
		// Freeing of arrays is handled by the garbage collector
		return s->r_acc;
	case 5: { // Fill
		SciArray<reg_t> *array = s->_segMan->lookupArray(argv[1]);
		uint16 index = argv[2].toUint16();

		// A count of -1 means fill the rest of the array
		uint16 count = argv[3].toSint16() == -1 ? array->getSize() - index : argv[3].toUint16();
		uint16 arraySize = array->getSize();

		if (arraySize < index + count)
			array->setSize(index + count);

		for (uint16 i = 0; i < count; i++)
			array->setValue(i + index, argv[4]);

		return argv[1];
	}
	case 6: { // Cpy
		if (s->_segMan->getSegmentObj(argv[1].segment)->getType() != SEG_TYPE_ARRAY ||
			s->_segMan->getSegmentObj(argv[3].segment)->getType() != SEG_TYPE_ARRAY) {
			// Happens in the RAMA demo
			warning("kArray(Cpy): Request to copy a segment which isn't an array, ignoring");
			return NULL_REG;
		}

		SciArray<reg_t> *array1 = s->_segMan->lookupArray(argv[1]);
		SciArray<reg_t> *array2 = s->_segMan->lookupArray(argv[3]);
		uint32 index1 = argv[2].toUint16();
		uint32 index2 = argv[4].toUint16();

		// The original engine ignores bad copies too
		if (index2 > array2->getSize())
			break;

		// A count of -1 means fill the rest of the array
		uint32 count = argv[5].toSint16() == -1 ? array2->getSize() - index2 : argv[5].toUint16();

		if (array1->getSize() < index1 + count)
			array1->setSize(index1 + count);

		for (uint16 i = 0; i < count; i++)
			array1->setValue(i + index1, array2->getValue(i + index2));

		return argv[1];
	}
	case 7: // Cmp
		// Not implemented in SSCI
		return s->r_acc;
	case 8: { // Dup
		SciArray<reg_t> *array = s->_segMan->lookupArray(argv[1]);
		reg_t arrayHandle;
		SciArray<reg_t> *dupArray = s->_segMan->allocateArray(&arrayHandle);

		dupArray->setType(array->getType());
		dupArray->setSize(array->getSize());

		for (uint32 i = 0; i < array->getSize(); i++)
			dupArray->setValue(i, array->getValue(i));

		return arrayHandle;
	}
	case 9: // Getdata
		if (!s->_segMan->isHeapObject(argv[1]))
			return argv[1];

		return readSelector(s->_segMan, argv[1], SELECTOR(data));
	default:
		error("Unknown kArray subop %d", argv[0].toUint16());
	}

	return NULL_REG;
}

reg_t kText(EngineState *s, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case 0:
		return kTextSize(s, argc - 1, argv + 1);
	default:
		// TODO: Other subops here too, perhaps kTextColors and kTextFonts
		warning("kText(%d)", argv[0].toUint16());
		break;
	}

	return s->r_acc;
}

reg_t kString(EngineState *s, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case 0: { // New
		reg_t stringHandle;
		SciString *string = s->_segMan->allocateString(&stringHandle);
		string->setSize(argv[1].toUint16());

		// Make sure the first character is a null character
		if (string->getSize() > 0)
			string->setValue(0, 0);

		return stringHandle;
		}
	case 1: // Size
		return make_reg(0, s->_segMan->getString(argv[1]).size());
	case 2: { // At (return value at an index)
		if (argv[1].segment == s->_segMan->getStringSegmentId())
			return make_reg(0, s->_segMan->lookupString(argv[1])->getRawData()[argv[2].toUint16()]);

		return make_reg(0, s->_segMan->getString(argv[1])[argv[2].toUint16()]);
	}
	case 3: { // Atput (put value at an index)
		SciString *string = s->_segMan->lookupString(argv[1]);

		uint32 index = argv[2].toUint16();
		uint32 count = argc - 3;

		if (index + count > 65535)
			break;

		if (string->getSize() < index + count)
			string->setSize(index + count);

		for (uint16 i = 0; i < count; i++)
			string->setValue(i + index, argv[i + 3].toUint16());

		return argv[1]; // We also have to return the handle
	}
	case 4: // Free
		// Freeing of strings is handled by the garbage collector
		return s->r_acc;
	case 5: { // Fill
		SciString *string = s->_segMan->lookupString(argv[1]);
		uint16 index = argv[2].toUint16();

		// A count of -1 means fill the rest of the array
		uint16 count = argv[3].toSint16() == -1 ? string->getSize() - index : argv[3].toUint16();
		uint16 stringSize = string->getSize();

		if (stringSize < index + count)
			string->setSize(index + count);

		for (uint16 i = 0; i < count; i++)
			string->setValue(i + index, argv[4].toUint16());

		return argv[1];
	}
	case 6: { // Cpy
		const char *string2 = 0;
		uint32 string2Size = 0;

		if (argv[3].segment == s->_segMan->getStringSegmentId()) {
			SciString *string = s->_segMan->lookupString(argv[3]);
			string2 = string->getRawData();
			string2Size = string->getSize();
		} else {
			Common::String string = s->_segMan->getString(argv[3]);
			string2 = string.c_str();
			string2Size = string.size() + 1;
		}

		uint32 index1 = argv[2].toUint16();
		uint32 index2 = argv[4].toUint16();

		// The original engine ignores bad copies too
		if (index2 > string2Size)
			break;

		// A count of -1 means fill the rest of the array
		uint32 count = argv[5].toSint16() == -1 ? string2Size - index2 + 1 : argv[5].toUint16();
	
		// We have a special case here for argv[1] being a system string
		if (argv[1].segment == s->_segMan->getSysStringsSegment()) {
			// Resize if necessary
			const uint16 sysStringId = argv[1].toUint16();
			SystemString *sysString = s->_segMan->getSystemString(sysStringId);
			assert(sysString);
			if ((uint32)sysString->_maxSize < index1 + count) {
				free(sysString->_value);
				sysString->_maxSize = index1 + count;
				sysString->_value = (char *)calloc(index1 + count, sizeof(char));
			}

			strncpy(sysString->_value + index1, string2 + index2, count);
		} else {
			SciString *string1 = s->_segMan->lookupString(argv[1]);

			if (string1->getSize() < index1 + count)
				string1->setSize(index1 + count);

			// Note: We're accessing from c_str() here because the
			// string's size ignores the trailing 0 and therefore
			// triggers an assert when doing string2[i + index2].
			for (uint16 i = 0; i < count; i++)
				string1->setValue(i + index1, string2[i + index2]);
		}
		
	} return argv[1];
	case 7: { // Cmp
		Common::String string1 = argv[1].isNull() ? "" : s->_segMan->getString(argv[1]);
		Common::String string2 = argv[2].isNull() ? "" : s->_segMan->getString(argv[2]);

		if (argc == 4) // Strncmp
			return make_reg(0, strncmp(string1.c_str(), string2.c_str(), argv[3].toUint16()));
		else           // Strcmp
			return make_reg(0, strcmp(string1.c_str(), string2.c_str()));
	}
	case 8: { // Dup
		const char *rawString = 0;
		uint32 size = 0;

		if (argv[1].segment == s->_segMan->getStringSegmentId()) {
			SciString *string = s->_segMan->lookupString(argv[1]);
			rawString = string->getRawData();
			size = string->getSize();
		} else {
			Common::String string = s->_segMan->getString(argv[1]);
			rawString = string.c_str();
			size = string.size() + 1;
		}

		reg_t stringHandle;
		SciString *dupString = s->_segMan->allocateString(&stringHandle);
		dupString->setSize(size);

		for (uint32 i = 0; i < size; i++)
			dupString->setValue(i, rawString[i]);

		return stringHandle;
	}
	case 9: // Getdata
		if (!s->_segMan->isHeapObject(argv[1]))
			return argv[1];

		return readSelector(s->_segMan, argv[1], SELECTOR(data));
	case 10: // Stringlen
		return make_reg(0, s->_segMan->strlen(argv[1]));
	case 11: { // Printf
		reg_t stringHandle;
		s->_segMan->allocateString(&stringHandle);

		reg_t *adjustedArgs = new reg_t[argc];
		adjustedArgs[0] = stringHandle;
		memcpy(&adjustedArgs[1], argv + 1, (argc - 1) * sizeof(reg_t));

		kFormat(s, argc, adjustedArgs);
		delete[] adjustedArgs;
		return stringHandle;
		}
	case 12: // Printf Buf
		return kFormat(s, argc - 1, argv + 1);
	case 13: { // atoi
		Common::String string = s->_segMan->getString(argv[1]);
		return make_reg(0, (uint16)atoi(string.c_str()));
	}
	default:
		error("Unknown kString subop %d", argv[0].toUint16());
	}

	return NULL_REG;
}

reg_t kSave(EngineState *s, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case 2: // GetSaveDir
		// Yay! Reusing the old kernel function!
		return kGetSaveDir(s, argc - 1, argv + 1);
	case 8:
		// TODO
		// This function has to return something other than 0 to proceed
		return s->r_acc;
	default:
		warning("Unknown/unhandled kSave subop %d", argv[0].toUint16());
	}

	return NULL_REG;
}

reg_t kAddScreenItem(EngineState *s, int argc, reg_t *argv) {
	reg_t viewObj = argv[0];

	g_sci->_gfxFrameout->kernelAddScreenItem(viewObj);
	return NULL_REG;
}

reg_t kUpdateScreenItem(EngineState *s, int argc, reg_t *argv) {
	//reg_t viewObj = argv[0];

	//warning("kUpdateScreenItem, object %04x:%04x, view %d, loop %d, cel %d, pri %d", PRINT_REG(viewObj), viewId, loopNo, celNo, priority);
	return NULL_REG;
}

reg_t kDeleteScreenItem(EngineState *s, int argc, reg_t *argv) {
	reg_t viewObj = argv[0];

	g_sci->_gfxFrameout->kernelDeleteScreenItem(viewObj);

	/*
	reg_t viewObj = argv[0];
	uint16 viewId = readSelectorValue(s->_segMan, viewObj, SELECTOR(view));
	int16 loopNo = readSelectorValue(s->_segMan, viewObj, SELECTOR(loop));
	int16 celNo = readSelectorValue(s->_segMan, viewObj, SELECTOR(cel));
	//int16 leftPos = 0;
	//int16 topPos = 0;
	int16 priority = readSelectorValue(s->_segMan, viewObj, SELECTOR(priority));
	//int16 control = 0;
	*/

	// TODO

	//warning("kDeleteScreenItem, view %d, loop %d, cel %d, pri %d", viewId, loopNo, celNo, priority);
	return NULL_REG;
}

reg_t kAddPlane(EngineState *s, int argc, reg_t *argv) {
	reg_t planeObj = argv[0];

	g_sci->_gfxFrameout->kernelAddPlane(planeObj);
	warning("kAddPlane object %04x:%04x", PRINT_REG(planeObj));
	return NULL_REG;
}

reg_t kDeletePlane(EngineState *s, int argc, reg_t *argv) {
	reg_t planeObj = argv[0];

	g_sci->_gfxFrameout->kernelDeletePlane(planeObj);
	warning("kDeletePlane object %04x:%04x", PRINT_REG(planeObj));
	return NULL_REG;
}

reg_t kUpdatePlane(EngineState *s, int argc, reg_t *argv) {
	reg_t planeObj = argv[0];

	g_sci->_gfxFrameout->kernelUpdatePlane(planeObj);
	return s->r_acc;
}

reg_t kRepaintPlane(EngineState *s, int argc, reg_t *argv) {
	reg_t picObj = argv[0];

	// TODO

	warning("kRepaintPlane object %04x:%04x", PRINT_REG(picObj));
	return NULL_REG;
}

reg_t kGetHighPlanePri(EngineState *s, int argc, reg_t *argv) {
	warning("kGetHighPlanePri: %d", g_sci->_gfxFrameout->kernelGetHighPlanePri());
	return make_reg(0, g_sci->_gfxFrameout->kernelGetHighPlanePri());
}

reg_t kFrameOut(EngineState *s, int argc, reg_t *argv) {
	// This kernel call likely seems to be doing the screen updates,
	// as its called right after a view is updated

	// TODO
	g_sci->_gfxFrameout->kernelFrameout();

	return NULL_REG;
}

reg_t kOnMe(EngineState *s, int argc, reg_t *argv) {
	// Tests if the cursor is on the passed object

	uint16 x = argv[0].toUint16();
	uint16 y = argv[1].toUint16();
	reg_t targetObject = argv[2];
	// TODO: argv[3] - it's usually 0
	Common::Rect nsRect;

	// Get the bounding rectangle of the object
	nsRect.left = readSelectorValue(s->_segMan, targetObject, SELECTOR(nsLeft));
	nsRect.top = readSelectorValue(s->_segMan, targetObject, SELECTOR(nsTop));
	nsRect.right = readSelectorValue(s->_segMan, targetObject, SELECTOR(nsRight));
	nsRect.bottom = readSelectorValue(s->_segMan, targetObject, SELECTOR(nsBottom));
	uint16 itemX = readSelectorValue(s->_segMan, targetObject, SELECTOR(x));
	uint16 itemY = readSelectorValue(s->_segMan, targetObject, SELECTOR(y));

	// If top and left are negative, we need to adjust coordinates by
	// the item's x and y (e.g. happens in GK1, day 1, with detective
	// Mosely's hotspot in his office)

	if (nsRect.left < 0)
		nsRect.translate(itemX, 0);
	
	if (nsRect.top < 0)
		nsRect.translate(0, itemY);

	// HACK: nsLeft and nsTop can be invalid, so try and fix them here
	// using x and y (e.g. with the inventory screen in GK1)
	if (nsRect.left == itemY && nsRect.top == itemX) {
		// Swap the values, as they're inversed (eh???)
		nsRect.left = itemX;
		nsRect.top = itemY;
	}

	/*
	warning("kOnMe: (%d, %d) on object %04x:%04x (%s), rect (%d, %d, %d, %d), parameter %d", 
		argv[0].toUint16(), argv[1].toUint16(), PRINT_REG(argv[2]), s->_segMan->getObjectName(argv[2]), 
		nsRect.left, nsRect.top, nsRect.right, nsRect.bottom,
		argv[3].toUint16());
	*/

	return make_reg(0, nsRect.contains(x, y));
}

reg_t kIsOnMe(EngineState *s, int argc, reg_t *argv) {
	// Tests if the cursor is on the passed object, after adjusting the
	// coordinates of the object according to the object's plane

	uint16 x = argv[0].toUint16();
	uint16 y = argv[1].toUint16();
	reg_t targetObject = argv[2];
	// TODO: argv[3] - it's usually 0
	Common::Rect nsRect;

	// Get the bounding rectangle of the object
	nsRect.left = readSelectorValue(s->_segMan, targetObject, SELECTOR(nsLeft));
	nsRect.top = readSelectorValue(s->_segMan, targetObject, SELECTOR(nsTop));
	nsRect.right = readSelectorValue(s->_segMan, targetObject, SELECTOR(nsRight));
	nsRect.bottom = readSelectorValue(s->_segMan, targetObject, SELECTOR(nsBottom));

	// Get the object's plane
#if 0
	reg_t planeObject = readSelector(s->_segMan, targetObject, SELECTOR(plane));
	if (!planeObject.isNull()) {
		//uint16 itemX = readSelectorValue(s->_segMan, targetObject, SELECTOR(x));
		//uint16 itemY = readSelectorValue(s->_segMan, targetObject, SELECTOR(y));
		uint16 planeResY = readSelectorValue(s->_segMan, planeObject, SELECTOR(resY));
		uint16 planeResX = readSelectorValue(s->_segMan, planeObject, SELECTOR(resX));
		uint16 planeTop = readSelectorValue(s->_segMan, planeObject, SELECTOR(top));
		uint16 planeLeft = readSelectorValue(s->_segMan, planeObject, SELECTOR(left));
		planeTop = (planeTop * g_sci->_gfxScreen->getHeight()) / planeResY;
		planeLeft = (planeLeft * g_sci->_gfxScreen->getWidth()) / planeResX;

		// Adjust the bounding rectangle of the object by the object's
		// actual X, Y coordinates
		nsRect.top = ((nsRect.top * g_sci->_gfxScreen->getHeight()) / planeResY);
		nsRect.left = ((nsRect.left * g_sci->_gfxScreen->getWidth()) / planeResX);
		nsRect.bottom = ((nsRect.bottom * g_sci->_gfxScreen->getHeight()) / planeResY);
		nsRect.right = ((nsRect.right * g_sci->_gfxScreen->getWidth()) / planeResX);

		nsRect.translate(planeLeft, planeTop);
	}
#endif
	//warning("kIsOnMe: (%d, %d) on object %04x:%04x, parameter %d", argv[0].toUint16(), argv[1].toUint16(), PRINT_REG(argv[2]), argv[3].toUint16());

	return make_reg(0, nsRect.contains(x, y));
}

reg_t kInPolygon(EngineState *s, int argc, reg_t *argv) {
	// kAvoidPath already implements this
	return kAvoidPath(s, argc, argv);
}

reg_t kCreateTextBitmap(EngineState *s, int argc, reg_t *argv) {
	// TODO: argument 0 is usually 0, and arguments 1 and 2 are usually 1
	switch (argv[0].toUint16()) {
	case 0: {
		if (argc != 4) {
			warning("kCreateTextBitmap(0): expected 4 arguments, got %i", argc);
			return NULL_REG;
		}
		reg_t object = argv[3];
		Common::String text = s->_segMan->getString(readSelector(s->_segMan, object, SELECTOR(text)));
		debug("kCreateTextBitmap: %s", text.c_str());
	}
	default:
		warning("CreateTextBitmap(%d)", argv[0].toUint16());
	}

	return NULL_REG;
}

reg_t kCD(EngineState *s, int argc, reg_t *argv) {
	// TODO: Stub
	switch (argv[0].toUint16()) {
	case 0:
		// Return whether the contents of disc argv[1] is available.
		return TRUE_REG;
	default:
		warning("CD(%d)", argv[0].toUint16());
	}

	return NULL_REG;
}

} // End of namespace Sci

#endif	// ENABLE_SCI32
