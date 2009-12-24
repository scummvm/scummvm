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

namespace Sci {

static const char *sci2_default_knames[] = {
	/*0x00*/ "Load",
	/*0x01*/ "UnLoad",
	/*0x02*/ "ScriptID",
	/*0x03*/ "DisposeScript",
	/*0x04*/ "Lock",
	/*0x05*/ "ResCheck",
	/*0x06*/ "Purge",
	/*0x07*/ "Clone",
	/*0x08*/ "DisposeClone",
	/*0x09*/ "RespondsTo",
	/*0x0a*/ "SetNowSeen",
	/*0x0b*/ "NumLoops",
	/*0x0c*/ "NumCels",
	/*0x0d*/ "CelWide",
	/*0x0e*/ "CelHigh",
	/*0x0f*/ "GetHighPlanePri",
	/*0x10*/ "GetHighItemPri",
	/*0x11*/ "ShakeScreen",
	/*0x12*/ "OnMe",
	/*0x13*/ "ShowMovie",
	/*0x14*/ "SetVideoMode",
	/*0x15*/ "AddScreenItem",
	/*0x16*/ "DeleteScreenItem",
	/*0x17*/ "UpdateScreenItem",
	/*0x18*/ "FrameOut",
	/*0x19*/ "AddPlane",
	/*0x1a*/ "DeletePlane",
	/*0x1b*/ "UpdatePlane",
	/*0x1c*/ "RepaintPlane",
	/*0x1d*/ "SetShowStyle",
	/*0x1e*/ "ShowStylePercent",
	/*0x1f*/ "SetScroll",
	/*0x20*/ "AddMagnify",
	/*0x21*/ "DeleteMagnify",
	/*0x22*/ "IsHiRes",
	/*0x23*/ "Graph",
	/*0x24*/ "InvertRect",
	/*0x25*/ "TextSize",
	/*0x26*/ "Message",
	/*0x27*/ "TextColors",
	/*0x28*/ "TextFonts",
	/*0x29*/ "Dummy",
	/*0x2a*/ "SetQuitStr",		 
	/*0x2b*/ "EditText",
	/*0x2c*/ "InputText",
	/*0x2d*/ "CreateTextBitmap",
	/*0x2e*/ "DisposeTextBitmap",
	/*0x2f*/ "GetEvent",
	/*0x30*/ "GlobalToLocal",
	/*0x31*/ "LocalToGlobal",
	/*0x32*/ "MapKeyToDir",
	/*0x33*/ "HaveMouse",
	/*0x34*/ "SetCursor",
	/*0x35*/ "VibrateMouse",
	/*0x36*/ "SaveGame",
	/*0x37*/ "RestoreGame",
	/*0x38*/ "RestartGame",
	/*0x39*/ "GameIsRestarting",
	/*0x3a*/ "MakeSaveCatName",
	/*0x3b*/ "MakeSaveFileName",
	/*0x3c*/ "GetSaveFiles",
	/*0x3d*/ "GetSaveDir",
	/*0x3e*/ "CheckSaveGame",
	/*0x3f*/ "CheckFreeSpace",
	/*0x40*/ "DoSound",
	/*0x41*/ "DoAudio",
	/*0x42*/ "DoSync",
	/*0x43*/ "NewList",
	/*0x44*/ "DisposeList",
	/*0x45*/ "NewNode",
	/*0x46*/ "FirstNode",
	/*0x47*/ "LastNode",
	/*0x48*/ "EmptyList",
	/*0x49*/ "NextNode",
	/*0x4a*/ "PrevNode",
	/*0x4b*/ "NodeValue",
	/*0x4c*/ "AddAfter",
	/*0x4d*/ "AddToFront",
	/*0x4e*/ "AddToEnd",
	/*0x4f*/ "Dummy",
	/*0x50*/ "Dummy",
	/*0x51*/ "FindKey",
	/*0x52*/ "Dummy",
	/*0x53*/ "Dummy",
	/*0x54*/ "Dummy",
	/*0x55*/ "DeleteKey",
	/*0x56*/ "Dummy",
	/*0x57*/ "Dummy",
	/*0x58*/ "ListAt",
	/*0x59*/ "ListIndexOf",
	/*0x5a*/ "ListEachElementDo",
	/*0x5b*/ "ListFirstTrue",
	/*0x5c*/ "ListAllTrue",
	/*0x5d*/ "Random",
	/*0x5e*/ "Abs",
	/*0x5f*/ "Sqrt",
	/*0x60*/ "GetAngle",
	/*0x61*/ "GetDistance",
	/*0x62*/ "ATan",
	/*0x63*/ "SinMult",
	/*0x64*/ "CosMult",
	/*0x65*/ "SinDiv",
	/*0x66*/ "CosDiv",
	/*0x67*/ "GetTime",
	/*0x68*/ "Platform",
	/*0x69*/ "BaseSetter",
	/*0x6a*/ "DirLoop",
	/*0x6b*/ "CanBeHere",
	/*0x6c*/ "InitBresen",
	/*0x6d*/ "DoBresen",
	/*0x6e*/ "SetJump",
	/*0x6f*/ "AvoidPath",
	/*0x70*/ "InPolygon",
	/*0x71*/ "MergePoly",
	/*0x72*/ "Dummy",
	/*0x73*/ "Dummy",
	/*0x74*/ "Dummy",
	/*0x75*/ "Dummy",
	/*0x76*/ "Dummy",
	/*0x77*/ "Dummy",
	/*0x78*/ "Dummy",
	/*0x79*/ "Dummy",
	/*0x7a*/ "GetCWD",
	/*0x7b*/ "ValidPath",
	/*0x7c*/ "FileIO",
	/*0x7d*/ "Dummy",
	/*0x7e*/ "DeviceInfo",
	/*0x7f*/ "Palette",
	/*0x80*/ "PalVary",
	/*0x81*/ "PalCycle",
	/*0x82*/ "Array",
	/*0x83*/ "String",
	/*0x84*/ "RemapColors",
	/*0x85*/ "Dummy",
	/*0x86*/ "Dummy",
	/*0x87*/ "ObjectIntersect",
	/*0x88*/ "Dummy",
	/*0x89*/ "TextWidth",
	/*0x8a*/ "PointSize"
};

static const char *sci21_default_knames[] = {
	/*0x00*/ "Load",
	/*0x01*/ "UnLoad",
	/*0x02*/ "ScriptID",
	/*0x03*/ "DisposeScript",
	/*0x04*/ "Lock",
	/*0x05*/ "ResCheck",
	/*0x06*/ "Purge",
	/*0x07*/ "SetLanguage",
	/*0x08*/ "Dummy",
	/*0x09*/ "Dummy",
	/*0x0a*/ "Clone",
	/*0x0b*/ "DisposeClone",
	/*0x0c*/ "RespondsTo",
	/*0x0d*/ "FindSelector",
	/*0x0e*/ "FindClass",
	/*0x0f*/ "Dummy",
	/*0x10*/ "Dummy",
	/*0x11*/ "Dummy",
	/*0x12*/ "Dummy",
	/*0x13*/ "Dummy",
	/*0x14*/ "SetNowSeen",
	/*0x15*/ "NumLoops",
	/*0x16*/ "NumCels",
	/*0x17*/ "IsOnMe",
	/*0x18*/ "AddMagnify",
	/*0x19*/ "DeleteMagnify",
	/*0x1a*/ "CelRect",
	/*0x1b*/ "BaseLineSpan",
	/*0x1c*/ "CelWide",
	/*0x1d*/ "CelHigh",
	/*0x1e*/ "AddScreenItem",
	/*0x1f*/ "DeleteScreenItem",
	/*0x20*/ "UpdateScreenItem",
	/*0x21*/ "FrameOut",
	/*0x22*/ "CelInfo",
	/*0x23*/ "Bitmap",
	/*0x24*/ "CelLink",
	/*0x25*/ "Dummy",
	/*0x26*/ "Dummy",
	/*0x27*/ "Dummy",
	/*0x28*/ "AddPlane",
	/*0x29*/ "DeletePlane",
	/*0x2a*/ "UpdatePlane",
	/*0x2b*/ "RepaintPlane",
	/*0x2c*/ "GetHighPlanePri",
	/*0x2d*/ "GetHighItemPri",
	/*0x2e*/ "SetShowStyle",
	/*0x2f*/ "ShowStylePercent",
	/*0x30*/ "SetScroll",
	/*0x31*/ "MovePlaneItems",
	/*0x32*/ "ShakeScreen",
	/*0x33*/ "Dummy",
	/*0x34*/ "Dummy",
	/*0x35*/ "Dummy",
	/*0x36*/ "Dummy",
	/*0x37*/ "IsHiRes",
	/*0x38*/ "SetVideoMode",
	/*0x39*/ "ShowMovie",
	/*0x3a*/ "Robot",
	/*0x3b*/ "CreateTextBitmap",
	/*0x3c*/ "Random",
	/*0x3d*/ "Abs",
	/*0x3e*/ "Sqrt",
	/*0x3f*/ "GetAngle",
	/*0x40*/ "GetDistance",
	/*0x41*/ "ATan",
	/*0x42*/ "SinMult",
	/*0x43*/ "CosMult",
	/*0x44*/ "SinDiv",
	/*0x45*/ "CosDiv",
	/*0x46*/ "Text",
	/*0x47*/ "Dummy",
	/*0x48*/ "Message",
	/*0x49*/ "Font",
	/*0x4a*/ "EditText",
	/*0x4b*/ "InputText",
	/*0x4c*/ "ScrollWindow",
	/*0x4d*/ "Dummy",
	/*0x4e*/ "Dummy",
	/*0x4f*/ "Dummy",
	/*0x50*/ "GetEvent",
	/*0x51*/ "GlobalToLocal",
	/*0x52*/ "LocalToGlobal",
	/*0x53*/ "MapKeyToDir",
	/*0x54*/ "HaveMouse",
	/*0x55*/ "SetCursor",
	/*0x56*/ "VibrateMouse", // NOTE: Not in SCI3, instead replaced by Dummy.
	/*0x57*/ "Dummy",
	/*0x58*/ "Dummy",
	/*0x59*/ "Dummy",
	/*0x5a*/ "List",
	/*0x5b*/ "Array",
	/*0x5c*/ "String",
	/*0x5d*/ "FileIO",
	/*0x5e*/ "BaseSetter",
	/*0x5f*/ "DirLoop",
	/*0x60*/ "CanBeHere",
	/*0x61*/ "InitBresen",
	/*0x62*/ "DoBresen",
	/*0x63*/ "SetJump",
	/*0x64*/ "AvoidPath",
	/*0x65*/ "InPolygon",
	/*0x66*/ "MergePoly",
	/*0x67*/ "ObjectIntersect",
	/*0x68*/ "Dummy",
	/*0x69*/ "MemoryInfo",
	/*0x6a*/ "DeviceInfo",
	/*0x6b*/ "Palette",
	/*0x6c*/ "PalVary",
	/*0x6d*/ "PalCycle",
	/*0x6e*/ "RemapColors",
	/*0x6f*/ "AddLine",
	/*0x70*/ "DeleteLine",
	/*0x71*/ "UpdateLine",
	/*0x72*/ "AddPolygon",
	/*0x73*/ "DeletePolygon",
	/*0x74*/ "UpdatePolygon",
	/*0x75*/ "DoSound",
	/*0x76*/ "DoAudio",
	/*0x77*/ "DoSync",
	/*0x78*/ "Save",
	/*0x79*/ "GetTime",
	/*0x7a*/ "Platform",
	/*0x7b*/ "CD",
	/*0x7c*/ "SetQuitStr",
	/*0x7d*/ "GetConfig",
	/*0x7e*/ "Table",
	/*0x7f*/ "Dummy",
	/*0x80*/ "Dummy",
	/*0x81*/ "Dummy",
	/*0x82*/ "Dummy",
	/*0x83*/ "Dummy",
	/*0x84*/ "Dummy",
	/*0x85*/ "Dummy",
	/*0x86*/ "Dummy",
	/*0x87*/ "Dummy",
	/*0x88*/ "Dummy",
	/*0x89*/ "Dummy",
	/*0x8a*/ "LoadChunk",
	/*0x8b*/ "SetPalStyleRange"
	/*0x8c*/ "AddPicAt",
	/*0x8d*/ "Dummy",
	/*0x8e*/ "NewRoom",
	/*0x8f*/ "Dummy",
	/*0x90*/ "Priority",
	/*0x91*/ "MorphOn",
	/*0x92*/ "PlayVMD",
	/*0x93*/ "SetHotRectangles",
	/*0x94*/ "MulDiv",
	/*0x95*/ "Dummy",
	/*0x96*/ "Dummy",
	/*0x97*/ "Dummy",
	/*0x98*/ "Dummy",
	/*0x99*/ "Dummy",

	// SCI3
	/*0x9a*/ "Dummy",
	/*0x9b*/ "Dummy",
	/*0x9c*/ "DeletePic"
};

void Kernel::setKernelNamesSci2() {
	_kernelNames = Common::StringList(sci2_default_knames, ARRAYSIZE(sci2_default_knames));
}

void Kernel::setKernelNamesSci21() {
	_kernelNames = Common::StringList(sci21_default_knames, ARRAYSIZE(sci21_default_knames));
}

// SCI2 Kernel Functions

reg_t kIsHiRes(EngineState *s, int argc, reg_t *argv) {
	// Returns 0 if the screen width or height is less than 640 or 400, respectively.
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
		s->_segMan->freeArray(argv[1]);
		return s->r_acc;
	case 5: { // Fill
		SciArray<reg_t> *array = s->_segMan->lookupArray(argv[1]);
		uint16 index = argv[2].toUint16();

		// A count of -1 means fill the rest of the array
		uint16 count = argv[3].toSint16() == -1 ? array->getSize() - index : argv[3].toUint16();

		if (array->getSize() < index + count)
			array->setSize(index + count);

		for (uint16 i = 0; i < count; i++)
			array->setValue(i + index, argv[4]);

		return argv[1];
		}
	case 6: { // Cpy
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
	
		return GET_SEL32(s->_segMan, argv[1], data);
	default:
		error("Unknown kArray subop %d", argv[0].toUint16());
	}
	
	return NULL_REG;
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
	case 2:  // At (return value at an index)
		return make_reg(0, s->_segMan->getString(argv[1])[argv[2].toUint16()]);
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
		s->_segMan->freeString(argv[1]);
		return s->r_acc;
	case 5: { // Fill
		SciString *string = s->_segMan->lookupString(argv[1]);
		uint16 index = argv[2].toUint16();

		// A count of -1 means fill the rest of the array
		uint16 count = argv[3].toSint16() == -1 ? string->getSize() - index : argv[3].toUint16();

		if (string->getSize() < index + count)
			string->setSize(index + count);

		for (uint16 i = 0; i < count; i++)
			string->setValue(i + index, argv[4].toUint16());

		return argv[1];
		}
	case 6: { // Cpy
		SciString *string1 = s->_segMan->lookupString(argv[1]);
		Common::String string2 = s->_segMan->getString(argv[3]);
		uint32 index1 = argv[2].toUint16();
		uint32 index2 = argv[4].toUint16();

		// The original engine ignores bad copies too
		if (index2 > string2.size())
			break;

		// A count of -1 means fill the rest of the array
		uint32 count = argv[5].toSint16() == -1 ? string2.size() - index2 + 1 : argv[5].toUint16();

		if (string1->getSize() < index1 + count)
			string1->setSize(index1 + count);

		// Note: We're accessing from c_str() here because the string's size ignores
		// the trailing 0 and therefore triggers an assert when doing string2[i + index2].
		for (uint16 i = 0; i < count; i++)
			string1->setValue(i + index1, string2.c_str()[i + index2]);

		return argv[1];
		}
	case 7: { // Cmp
		Common::String string1 = argv[1].isNull() ? "" : s->_segMan->getString(argv[1]);
		Common::String string2 = argv[2].isNull() ? "" : s->_segMan->getString(argv[2]);
			
		if (argc == 4) // Strncmp
			return make_reg(0, strncmp(string1.c_str(), string2.c_str(), argv[3].toUint16()));
		else           // Strcmp
			return make_reg(0, strcmp(string1.c_str(), string2.c_str()));
		}
	case 8: { // Dup
		Common::String string = s->_segMan->getString(argv[1]);
		reg_t stringHandle;
		SciString *dupString = s->_segMan->allocateString(&stringHandle);
		dupString->setSize(string.size() + 1);
		
		for (uint32 i = 0; i < string.size(); i++)
			dupString->setValue(i, string.c_str()[i]);
			
		dupString->setValue(dupString->getSize() - 1, 0);

		return stringHandle;
		}
	case 9: // Getdata
		if (!s->_segMan->isHeapObject(argv[1]))
			return argv[1];
	
		return GET_SEL32(s->_segMan, argv[1], data);
	case 10: // Stringlen
		return make_reg(0, s->_segMan->strlen(argv[1]));
	case 11: // Printf
		// TODO: Return a new formatting string
		warning("kString(Printf)");
		break;
	case 12: // Printf Buf
		// TODO: Store a formatted string in a specified string
		warning("kString(PrintfBuf)");
		break;
	case 13: // atoi
		// TODO: String to integer
		warning("kString(atoi)");
		break;
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
	default:
		warning("Unknown/unhandled kSave subop %d", argv[0].toUint16());
	}

	return NULL_REG;
}

} // End of namespace Sci

#endif	// ENABLE_SCI32
