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

#include "sci/engine/features.h"
#include "sci/engine/kernel.h"
#include "sci/engine/segment.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/graphics/frameout.h"
#include "sci/graphics/screen.h"

#include "common/system.h"

namespace Sci {

// NOTE: 0x72-0x79, 0x85-0x86, 0x88 are from the GK2 demo (which has debug support) and are
// just Dummy in other SCI2 games.
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
	/*0x6b*/ "CantBeHere",
	/*0x6c*/ "InitBresen",
	/*0x6d*/ "DoBresen",
	/*0x6e*/ "SetJump",
	/*0x6f*/ "AvoidPath",
	/*0x70*/ "InPolygon",
	/*0x71*/ "MergePoly",
	/*0x72*/ "SetDebug",
	/*0x73*/ "InspectObject",
	/*0x74*/ "MemoryInfo",
	/*0x75*/ "Profiler",
	/*0x76*/ "Record",
	/*0x77*/ "PlayBack",
	/*0x78*/ "MonoOut",
	/*0x79*/ "SetFatalStr",
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
	/*0x85*/ "IntegrityChecking",
	/*0x86*/ "CheckIntegrity",
	/*0x87*/ "ObjectIntersect",
	/*0x88*/ "MarkMemory",
	/*0x89*/ "TextWidth",
	/*0x8a*/ "PointSize",

	// GK2 Demo only kernel functions
	/*0x8b*/ "AddLine",
	/*0x8c*/ "DeleteLine",
	/*0x8d*/ "UpdateLine",
	/*0x8e*/ "AddPolygon",
	/*0x8f*/ "DeletePolygon",
	/*0x90*/ "UpdatePolygon",
	/*0x91*/ "Bitmap",
	/*0x92*/ "ScrollWindow",
	/*0x93*/ "SetFontRes",
	/*0x94*/ "MovePlaneItems",
	/*0x95*/ "PreloadResource",
	/*0x96*/ "Dummy",
	/*0x97*/ "ResourceTrack",
	/*0x98*/ "CheckCDisc",
	/*0x99*/ "GetSaveCDisc",
	/*0x9a*/ "TestPoly",
	/*0x9b*/ "WinHelp",
	/*0x9c*/ "LoadChunk",
	/*0x9d*/ "SetPalStyleRange",
	/*0x9e*/ "AddPicAt",
	/*0x9f*/ "MessageBox"
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
	/*0x60*/ "CantBeHere",
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
	/*0x7f*/ "WinHelp", // Windows only
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
	/*0x8b*/ "SetPalStyleRange",
	/*0x8c*/ "AddPicAt",
	/*0x8d*/ "Dummy",
	/*0x8e*/ "NewRoom",
	/*0x8f*/ "Dummy",
	/*0x90*/ "Priority",
	/*0x91*/ "MorphOn",
	/*0x92*/ "PlayVMD",
	/*0x93*/ "SetHotRectangles",
	/*0x94*/ "MulDiv",
	/*0x95*/ "GetSierraProfileInt", // Windows only
	/*0x96*/ "GetSierraProfileString", // Windows only
	/*0x97*/ "SetWindowsOption", // Windows only
	/*0x98*/ "GetWindowsOption", // Windows only
	/*0x99*/ "WinDLL", // Windows only

	// SCI3
	/*0x9a*/ "Dummy",
	/*0x9b*/ "Dummy",
	/*0x9c*/ "DeletePic"
};

enum {
	kKernelEntriesSci2 = 0x8b,
	kKernelEntriesGk2Demo = 0xa0,
	kKernelEntriesSci21 = 0x9a,
	kKernelEntriesSci3 = 0x9d
};

void Kernel::setKernelNamesSci2() {
	_kernelNames = Common::StringArray(sci2_default_knames, kKernelEntriesSci2);
}

void Kernel::setKernelNamesSci21(GameFeatures *features) {
	// Some SCI games use a modified SCI2 kernel table instead of the
	// SCI2.1/SCI3 kernel table. The GK2 demo does this as well as at
	// least one version of KQ7. We detect which version to use based on
	// where kDoSound is called from Sound::play().

	// This is interesting because they all have the same interpreter
	// version (2.100.002), yet they would not be compatible with other
	// games of the same interpreter.

	if (features->detectSci21KernelType() == SCI_VERSION_2) {
		_kernelNames = Common::StringArray(sci2_default_knames, kKernelEntriesGk2Demo);
		// OnMe is IsOnMe here, but they should be compatible
		_kernelNames[0x23] = "Robot"; // Graph in SCI2
		_kernelNames[0x2e] = "Priority"; // DisposeTextBitmap in SCI2
	} else {
		// TODO: Differentiate between SCI2.1/3
		_kernelNames = Common::StringArray(sci21_default_knames, kKernelEntriesSci3);
	}
}

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
			if ((uint32)s->_segMan->sysStrings->_strings[sysStringId]._maxSize < index1 + count) {
				free(s->_segMan->sysStrings->_strings[sysStringId]._value);
				s->_segMan->sysStrings->_strings[sysStringId]._maxSize = index1 + count;
				s->_segMan->sysStrings->_strings[sysStringId]._value = (char *)calloc(index1 + count, sizeof(char));
			}

			strncpy(s->_segMan->sysStrings->_strings[sysStringId]._value + index1, string2 + index2, count);
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
	reg_t planeObject = readSelector(s->_segMan, targetObject, SELECTOR(plane));
	if (!planeObject.isNull()) {
		uint16 itemX = readSelectorValue(s->_segMan, targetObject, SELECTOR(x));
		uint16 itemY = readSelectorValue(s->_segMan, targetObject, SELECTOR(y));
		uint16 planeResY = readSelectorValue(s->_segMan, planeObject, SELECTOR(resY));
		uint16 planeResX = readSelectorValue(s->_segMan, planeObject, SELECTOR(resX));
		uint16 planeTop = readSelectorValue(s->_segMan, planeObject, SELECTOR(top));
		uint16 planeLeft = readSelectorValue(s->_segMan, planeObject, SELECTOR(left));
		planeTop = (planeTop * g_sci->_gfxScreen->getHeight()) / planeResY;
		planeLeft = (planeLeft * g_sci->_gfxScreen->getWidth()) / planeResX;

		// Adjust the bounding rectangle of the object by the object's
		// actual X, Y coordinates
		itemY = ((itemY * g_sci->_gfxScreen->getHeight()) / planeResY);
		itemX = ((itemX * g_sci->_gfxScreen->getWidth()) / planeResX);
		itemY += planeTop;
		itemX += planeLeft;

		nsRect.translate(itemX, itemY);
	}

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
	case 0:
		if (argc != 4) {
			warning("kCreateTextBitmap(0): expected 4 arguments, got %i", argc);
			return NULL_REG;
		}
		reg_t object = argv[3];
		Common::String text = s->_segMan->getString(readSelector(s->_segMan, object, SELECTOR(text)));
		debug("kCreateTextBitmap: %s", text.c_str());
	}

	return NULL_REG;
}

} // End of namespace Sci

#endif	// ENABLE_SCI32
