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


#include "sci/engine/state.h"
#include "sci/scicore/resource.h"

namespace Sci {

// Default kernel name table
#define SCI0_KNAMES_WELL_DEFINED 0x6e
#define SCI0_KNAMES_DEFAULT_ENTRIES_NR 0x72
#define SCI1_KNAMES_DEFAULT_ENTRIES_NR 0x7E

static const char *sci0_default_knames[SCI0_KNAMES_DEFAULT_ENTRIES_NR] = {
	/*0x00*/ "Load",
	/*0x01*/ "UnLoad",
	/*0x02*/ "ScriptID",
	/*0x03*/ "DisposeScript",
	/*0x04*/ "Clone",
	/*0x05*/ "DisposeClone",
	/*0x06*/ "IsObject",
	/*0x07*/ "RespondsTo",
	/*0x08*/ "DrawPic",
	/*0x09*/ "Show",
	/*0x0a*/ "PicNotValid",
	/*0x0b*/ "Animate",
	/*0x0c*/ "SetNowSeen",
	/*0x0d*/ "NumLoops",
	/*0x0e*/ "NumCels",
	/*0x0f*/ "CelWide",
	/*0x10*/ "CelHigh",
	/*0x11*/ "DrawCel",
	/*0x12*/ "AddToPic",
	/*0x13*/ "NewWindow",
	/*0x14*/ "GetPort",
	/*0x15*/ "SetPort",
	/*0x16*/ "DisposeWindow",
	/*0x17*/ "DrawControl",
	/*0x18*/ "HiliteControl",
	/*0x19*/ "EditControl",
	/*0x1a*/ "TextSize",
	/*0x1b*/ "Display",
	/*0x1c*/ "GetEvent",
	/*0x1d*/ "GlobalToLocal",
	/*0x1e*/ "LocalToGlobal",
	/*0x1f*/ "MapKeyToDir",
	/*0x20*/ "DrawMenuBar",
	/*0x21*/ "MenuSelect",
	/*0x22*/ "AddMenu",
	/*0x23*/ "DrawStatus",
	/*0x24*/ "Parse",
	/*0x25*/ "Said",
	/*0x26*/ "SetSynonyms",
	/*0x27*/ "HaveMouse",
	/*0x28*/ "SetCursor",
	/*0x29*/ "FOpen",
	/*0x2a*/ "FPuts",
	/*0x2b*/ "FGets",
	/*0x2c*/ "FClose",
	/*0x2d*/ "SaveGame",
	/*0x2e*/ "RestoreGame",
	/*0x2f*/ "RestartGame",
	/*0x30*/ "GameIsRestarting",
	/*0x31*/ "DoSound",
	/*0x32*/ "NewList",
	/*0x33*/ "DisposeList",
	/*0x34*/ "NewNode",
	/*0x35*/ "FirstNode",
	/*0x36*/ "LastNode",
	/*0x37*/ "EmptyList",
	/*0x38*/ "NextNode",
	/*0x39*/ "PrevNode",
	/*0x3a*/ "NodeValue",
	/*0x3b*/ "AddAfter",
	/*0x3c*/ "AddToFront",
	/*0x3d*/ "AddToEnd",
	/*0x3e*/ "FindKey",
	/*0x3f*/ "DeleteKey",
	/*0x40*/ "Random",
	/*0x41*/ "Abs",
	/*0x42*/ "Sqrt",
	/*0x43*/ "GetAngle",
	/*0x44*/ "GetDistance",
	/*0x45*/ "Wait",
	/*0x46*/ "GetTime",
	/*0x47*/ "StrEnd",
	/*0x48*/ "StrCat",
	/*0x49*/ "StrCmp",
	/*0x4a*/ "StrLen",
	/*0x4b*/ "StrCpy",
	/*0x4c*/ "Format",
	/*0x4d*/ "GetFarText",
	/*0x4e*/ "ReadNumber",
	/*0x4f*/ "BaseSetter",
	/*0x50*/ "DirLoop",
	/*0x51*/ "CanBeHere",
	/*0x52*/ "OnControl",
	/*0x53*/ "InitBresen",
	/*0x54*/ "DoBresen",
	/*0x55*/ "DoAvoider",
	/*0x56*/ "SetJump",
	/*0x57*/ "SetDebug",
	/*0x58*/ "InspectObj",
	/*0x59*/ "ShowSends",
	/*0x5a*/ "ShowObjs",
	/*0x5b*/ "ShowFree",
	/*0x5c*/ "MemoryInfo",
	/*0x5d*/ "StackUsage",
	/*0x5e*/ "Profiler",
	/*0x5f*/ "GetMenu",
	/*0x60*/ "SetMenu",
	/*0x61*/ "GetSaveFiles",
	/*0x62*/ "GetCWD",
	/*0x63*/ "CheckFreeSpace",
	/*0x64*/ "ValidPath",
	/*0x65*/ "CoordPri",
	/*0x66*/ "StrAt",
	/*0x67*/ "DeviceInfo",
	/*0x68*/ "GetSaveDir",
	/*0x69*/ "CheckSaveGame",
	/*0x6a*/ "ShakeScreen",
	/*0x6b*/ "FlushResources",
	/*0x6c*/ "SinMult",
	/*0x6d*/ "CosMult",
	/*0x6e*/ "SinDiv",
	/*0x6f*/ "CosDiv",
	/*0x70*/ "Graph",
	/*0x71*/ SCRIPT_UNKNOWN_FUNCTION_STRING
};

static const char *sci1_default_knames[SCI1_KNAMES_DEFAULT_ENTRIES_NR] = {
	/*0x00*/ "Load",
	/*0x01*/ "UnLoad",
	/*0x02*/ "ScriptID",
	/*0x03*/ "DisposeScript",
	/*0x04*/ "Clone",
	/*0x05*/ "DisposeClone",
	/*0x06*/ "IsObject",
	/*0x07*/ "RespondsTo",
	/*0x08*/ "DrawPic",
	/*0x09*/ "Show",
	/*0x0a*/ "PicNotValid",
	/*0x0b*/ "Animate",
	/*0x0c*/ "SetNowSeen",
	/*0x0d*/ "NumLoops",
	/*0x0e*/ "NumCels",
	/*0x0f*/ "CelWide",
	/*0x10*/ "CelHigh",
	/*0x11*/ "DrawCel",
	/*0x12*/ "AddToPic",
	/*0x13*/ "NewWindow",
	/*0x14*/ "GetPort",
	/*0x15*/ "SetPort",
	/*0x16*/ "DisposeWindow",
	/*0x17*/ "DrawControl",
	/*0x18*/ "HiliteControl",
	/*0x19*/ "EditControl",
	/*0x1a*/ "TextSize",
	/*0x1b*/ "Display",
	/*0x1c*/ "GetEvent",
	/*0x1d*/ "GlobalToLocal",
	/*0x1e*/ "LocalToGlobal",
	/*0x1f*/ "MapKeyToDir",
	/*0x20*/ "DrawMenuBar",
	/*0x21*/ "MenuSelect",
	/*0x22*/ "AddMenu",
	/*0x23*/ "DrawStatus",
	/*0x24*/ "Parse",
	/*0x25*/ "Said",
	/*0x26*/ "SetSynonyms",
	/*0x27*/ "HaveMouse",
	/*0x28*/ "SetCursor",
	/*0x29*/ "SaveGame",
	/*0x2a*/ "RestoreGame",
	/*0x2b*/ "RestartGame",
	/*0x2c*/ "GameIsRestarting",
	/*0x2d*/ "DoSound",
	/*0x2e*/ "NewList",
	/*0x2f*/ "DisposeList",
	/*0x30*/ "NewNode",
	/*0x31*/ "FirstNode",
	/*0x32*/ "LastNode",
	/*0x33*/ "EmptyList",
	/*0x34*/ "NextNode",
	/*0x35*/ "PrevNode",
	/*0x36*/ "NodeValue",
	/*0x37*/ "AddAfter",
	/*0x38*/ "AddToFront",
	/*0x39*/ "AddToEnd",
	/*0x3a*/ "FindKey",
	/*0x3b*/ "DeleteKey",
	/*0x3c*/ "Random",
	/*0x3d*/ "Abs",
	/*0x3e*/ "Sqrt",
	/*0x3f*/ "GetAngle",
	/*0x40*/ "GetDistance",
	/*0x41*/ "Wait",
	/*0x42*/ "GetTime",
	/*0x43*/ "StrEnd",
	/*0x44*/ "StrCat",
	/*0x45*/ "StrCmp",
	/*0x46*/ "StrLen",
	/*0x47*/ "StrCpy",
	/*0x48*/ "Format",
	/*0x49*/ "GetFarText",
	/*0x4a*/ "ReadNumber",
	/*0x4b*/ "BaseSetter",
	/*0x4c*/ "DirLoop",
	/*0x4d*/ "CanBeHere",
	/*0x4e*/ "OnControl",
	/*0x4f*/ "InitBresen",
	/*0x50*/ "DoBresen",
	/*0x51*/ "DoAvoider",
	/*0x52*/ "SetJump",
	/*0x53*/ "SetDebug",
	/*0x54*/ "InspectObj",
	/*0x55*/ "ShowSends",
	/*0x56*/ "ShowObjs",
	/*0x57*/ "ShowFree",
	/*0x58*/ "MemoryInfo",
	/*0x59*/ "StackUsage",
	/*0x5a*/ "Profiler",
	/*0x5b*/ "GetMenu",
	/*0x5c*/ "SetMenu",
	/*0x5d*/ "GetSaveFiles",
	/*0x5e*/ "GetCWD",
	/*0x5f*/ "CheckFreeSpace",
	/*0x60*/ "ValidPath",
	/*0x61*/ "CoordPri",
	/*0x62*/ "StrAt",
	/*0x63*/ "DeviceInfo",
	/*0x64*/ "GetSaveDir",
	/*0x65*/ "CheckSaveGame",
	/*0x66*/ "ShakeScreen",
	/*0x67*/ "FlushResources",
	/*0x68*/ "SinMult",
	/*0x69*/ "CosMult",
	/*0x6a*/ "SinDiv",
	/*0x6b*/ "CosDiv",
	/*0x6c*/ "Graph",
	/*0x6d*/ "Joystick",
	/*0x6e*/ "ShiftScreen",
	/*0x6f*/ "Palette",
	/*0x70*/ "MemorySegment",
	/*0x71*/ "MoveCursor",
	/*0x72*/ "Memory",
	/*0x73*/ "ListOps",
	/*0x74*/ "FileIO",
	/*0x75*/ "DoAudio",
	/*0x76*/ "DoSync",
	/*0x77*/ "AvoidPath",
	/*0x78*/ "Sort",
	/*0x79*/ "ATan",
	/*0x7a*/ "Lock",         
	/*0x7b*/ "StrSplit",       
	/*0x7c*/ "GetMessage",
	/*0x7d*/ "IsItSkip"
};

int *vocabulary_get_classes(ResourceManager *resmgr, int* count) {
	Resource* r;
	int *c;
	unsigned int i;

	if ((r = resmgr->findResource(kResourceTypeVocab, 996, 0)) == NULL)
		return 0;

	c = (int *)sci_malloc(sizeof(int) * r->size / 2);
	for (i = 2; i < r->size; i += 4) {
		c[i/4] = READ_LE_UINT16(r->data + i);
	}
	*count = r->size / 4;

	return c;
}

int vocabulary_get_class_count(ResourceManager *resmgr) {
	Resource* r;

	if ((r = resmgr->findResource(kResourceTypeVocab, 996, 0)) == 0)
		return 0;

	return r->size / 4;
}

bool vocabulary_get_snames(ResourceManager *resmgr, sci_version_t version, Common::StringList &selectorNames) {
	int count;

	Resource *r = resmgr->findResource(kResourceTypeVocab, 997, 0);

	if (!r) // No such resource?
		return false;

	count = READ_LE_UINT16(r->data) + 1; // Counter is slightly off

	for (int i = 0; i < count; i++) {
		int offset = READ_LE_UINT16(r->data + 2 + i * 2);
		int len = READ_LE_UINT16(r->data + offset);
		
		Common::String tmp((const char *)r->data + offset + 2, len);
		selectorNames.push_back(tmp);
		if ((version != 0) && (version < SCI_VERSION_FTU_NEW_SCRIPT_HEADER)) {
			// Early SCI versions used the LSB in the selector ID as a read/write
			// toggle. To compensate for that, we add every selector name twice.
			selectorNames.push_back(tmp);
		}
	}

	return true;
}

int vocabulary_lookup_sname(const Common::StringList &selectorNames, const char *sname) {
	for (uint pos = 0; pos < selectorNames.size(); ++pos) {
		if (selectorNames[pos] == sname)
			return pos;
	}

	return -1;
}

opcode* vocabulary_get_opcodes(ResourceManager *resmgr) {
	opcode* o;
	int count, i = 0;
	Resource* r = resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_OPCODES, 0);

	// if the resource couldn't be loaded, leave
	if (r == NULL) {
		fprintf(stderr, "unable to load vocab.%03d\n", VOCAB_RESOURCE_OPCODES);
		return NULL;
	}

	count = READ_LE_UINT16(r->data);

	o = (opcode*)sci_malloc(sizeof(opcode) * 256);
	for (i = 0; i < count; i++) {
		int offset = READ_LE_UINT16(r->data + 2 + i * 2);
		int len = READ_LE_UINT16(r->data + offset) - 2;
		o[i].type = READ_LE_UINT16(r->data + offset + 2);
		o[i].number = i;
		o[i].name = (char *)sci_malloc(len + 1);
		memcpy(o[i].name, r->data + offset + 4, len);
		o[i].name[len] = '\0';
#ifdef VOCABULARY_DEBUG
		printf("Opcode %02X: %s, %d\n", i, o[i].name, o[i].type);
#endif
	}
	for (i = count; i < 256; i++) {
		o[i].type = 0;
		o[i].number = i;
		o[i].name = (char *)sci_malloc(strlen("undefined") + 1);
		strcpy(o[i].name, "undefined");
	}
	return o;
}

void vocabulary_free_opcodes(opcode *opcodes) {
	int i;
	if (!opcodes)
		return;

	for (i = 0; i < 256; i++) {
		if (opcodes[i].name)
			free(opcodes[i].name);
	}
	free(opcodes);
}

// Alternative kernel func names retriever. Required for KQ1/SCI (at least).
static void _vocabulary_get_knames0alt(const Resource *r, Common::StringList &names) {
	uint idx = 0;

	while (idx < r->size) {
		Common::String tmp((const char *)r->data + idx);
		names.push_back(tmp);
		idx += tmp.size() + 1;
	}

	// The mystery kernel function- one in each SCI0 package
	names.push_back(SCRIPT_UNKNOWN_FUNCTION_STRING);
}

static void vocabulary_get_knames0(ResourceManager *resmgr, Common::StringList &names) {
	int count, i, index = 2, empty_to_add = 1;
	Resource *r = resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_KNAMES, 0);

	if (!r) { // No kernel name table found? Fall back to default table
		names.resize(SCI0_KNAMES_DEFAULT_ENTRIES_NR);
		for (i = 0; i < SCI0_KNAMES_DEFAULT_ENTRIES_NR; i++)
			names[i] = sci0_default_knames[i];
		return;
	}

	count = READ_LE_UINT16(r->data);

	if (count > 1023) {
		_vocabulary_get_knames0alt(r, names);
		return;
	}

	if (count < SCI0_KNAMES_WELL_DEFINED) {
		empty_to_add = SCI0_KNAMES_WELL_DEFINED - count;
		sciprintf("Less than %d kernel functions; adding %d\n", SCI0_KNAMES_WELL_DEFINED, empty_to_add);
	}

	names.resize(count + 1 + empty_to_add);

	for (i = 0; i < count; i++) {
		int offset = READ_LE_UINT16(r->data + index);
		int len = READ_LE_UINT16(r->data + offset);
		//fprintf(stderr,"Getting name %d of %d...\n", i, count);
		index += 2;
		names[i] = Common::String((const char *)r->data + offset + 2, len);
	}

	for (i = 0; i < empty_to_add; i++) {
		names[count + i] = SCRIPT_UNKNOWN_FUNCTION_STRING;
	}
}

static void vocabulary_get_knames1(ResourceManager *resmgr, Common::StringList &names) {
	// vocab.999/999.voc is notoriously unreliable in SCI1 games, and should not be used
	// We hardcode the default SCI1 kernel names here (i.e. the ones inside the "special"
	// 999.voc file from FreeSCI). All SCI1 games seem to be working with this change, but
	// if any SCI1 game has different kernel vocabulary names, it might not work. It seems
	// that all SCI1 games use the same kernel vocabulary names though, so this seems to be
	// a safe change. If there's any SCI1 game with different kernel vocabulary names, we can
	// add special flags to it to our detector

	names.resize(SCI1_KNAMES_DEFAULT_ENTRIES_NR);
	for (int i = 0; i < SCI1_KNAMES_DEFAULT_ENTRIES_NR; i++)
		names[i] = sci1_default_knames[i];
}

//
static void vocabulary_get_knames11(ResourceManager *resmgr, Common::StringList &names) {
/*
 999.voc format for SCI1.1 games:
	[b] # of kernel functions
	[w] unknown
	[offset to function name info]
		...
    {[w name-len][function name]}
		...
*/
	//unsigned int size = 64, pos = 3;
	int len;
	Resource *r = resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_KNAMES, 0);
	if(r == NULL) // failed to open vocab.999 (happens with SCI1 demos)
		return; // FIXME: should return a default table for this engine 
	const byte nCnt = *r->data;
	
	names.resize(nCnt);
	for (int i = 0; i < nCnt; i++) {
		int off = READ_LE_UINT16(r->data + 2 * i + 2);
		len = READ_LE_UINT16(r->data + off);
		names[i] = Common::String((char *)r->data + off + 2, len);
	}
}

void vocabulary_get_knames(ResourceManager *resmgr, Common::StringList &names) {
	names.clear();

	switch (resmgr->_sciVersion) {
	case SCI_VERSION_0:
	case SCI_VERSION_01:
		vocabulary_get_knames0(resmgr, names);
		break;
	case SCI_VERSION_01_VGA:
	case SCI_VERSION_01_VGA_ODD:
		// HACK: KQ5 needs the SCI1 default vocabulary names to work correctly.
		// Having more vocabulary names (like in SCI1) doesn't seem to have any
		// ill effects, other than resulting in unmapped functions towards the
		// end, which are never used by the game interpteter anyway
		// return vocabulary_get_knames0(resmgr, count);
	case SCI_VERSION_1_EARLY:
	case SCI_VERSION_1_LATE:
		vocabulary_get_knames1(resmgr, names);
		break;
	case SCI_VERSION_1_1:
		vocabulary_get_knames11(resmgr, names);
		break;
#ifdef ENABLE_SCI32
	case SCI_VERSION_32:
		vocabulary_get_knames11(resmgr, names);
#endif
		break;
	default:
		break;
	}
}

} // End of namespace Sci
