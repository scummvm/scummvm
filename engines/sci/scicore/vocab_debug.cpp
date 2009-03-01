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

const char *sci0_default_knames[SCI0_KNAMES_DEFAULT_ENTRIES_NR] = {
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

const char *sci1_default_knames[SCI1_KNAMES_DEFAULT_ENTRIES_NR] = {
	"Load",
	"UnLoad",
	"ScriptID",
	"DisposeScript",
	"Clone",
	"DisposeClone",
	"IsObject",
	"RespondsTo",
	"DrawPic",
	"Show",
	"PicNotValid",
	"Animate",
	"SetNowSeen",
	"NumLoops",
	"NumCels",
	"CelWide",
	"CelHigh",
	"DrawCel",
	"AddToPic",
	"NewWindow",
	"GetPort",
	"SetPort",
	"DisposeWindow",
	"DrawControl",
	"HiliteControl",
	"EditControl",
	"TextSize",
	"Display",
	"GetEvent",
	"GlobalToLocal",
	"LocalToGlobal",
	"MapKeyToDir",
	"DrawMenuBar",
	"MenuSelect",
	"AddMenu",
	"DrawStatus",
	"Parse",
	"Said",
	"SetSynonyms",
	"HaveMouse",
	"SetCursor",
	"SaveGame",
	"RestoreGame",
	"RestartGame",
	"GameIsRestarting",
	"DoSound",
	"NewList",
	"DisposeList",
	"NewNode",
	"FirstNode",
	"LastNode",
	"EmptyList",
	"NextNode",
	"PrevNode",
	"NodeValue",
	"AddAfter",
	"AddToFront",
	"AddToEnd",
	"FindKey",
	"DeleteKey",
	"Random",
	"Abs",
	"Sqrt",
	"GetAngle",
	"GetDistance",
	"Wait",
	"GetTime",
	"StrEnd",
	"StrCat",
	"StrCmp",
	"StrLen",
	"StrCpy",
	"Format",
	"GetFarText",
	"ReadNumber",
	"BaseSetter",
	"DirLoop",
	"CanBeHere",
	"OnControl",
	"InitBresen",
	"DoBresen",
	"DoAvoider",
	"SetJump",
	"SetDebug",
	"InspectObj",
	"ShowSends",
	"ShowObjs",
	"ShowFree",
	"MemoryInfo",
	"StackUsage",
	"Profiler",
	"GetMenu",
	"SetMenu",
	"GetSaveFiles",
	"GetCWD",
	"CheckFreeSpace",
	"ValidPath",
	"CoordPri",
	"StrAt",
	"DeviceInfo",
	"GetSaveDir",
	"CheckSaveGame",
	"ShakeScreen",
	"FlushResources",
	"SinMult",
	"CosMult",
	"SinDiv",
	"CosDiv",
	"Graph",
	"Joystick",
	"ShiftScreen",
	"Palette",
	"MemorySegment",
	"Intersections",
	"Memory",
	"ListOps",
	"FileIO",
	"DoAudio",
	"DoSync",
	"AvoidPath",
	"Sort",
	"ATan",
	"Lock",         
	"StrSplit",       
	"GetMessage",
	SCRIPT_UNKNOWN_FUNCTION_STRING
};

int getInt(unsigned char* d) {
	return d[0] | (d[1] << 8);
}

int *vocabulary_get_classes(ResourceManager *resmgr, int* count) {
	Resource* r;
	int *c;
	unsigned int i;

	if ((r = resmgr->findResource(kResourceTypeVocab, 996, 0)) == NULL)
		return 0;

	c = (int *)sci_malloc(sizeof(int) * r->size / 2);
	for (i = 2; i < r->size; i += 4) {
		c[i/4] = getInt(r->data + i);
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

	count = getInt(r->data) + 1; // Counter is slightly off

	for (int i = 0; i < count; i++) {
		int offset = getInt(r->data + 2 + i * 2);
		int len = getInt(r->data + offset);
		
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

	count = getInt(r->data);

	o = (opcode*)sci_malloc(sizeof(opcode) * 256);
	for (i = 0; i < count; i++) {
		int offset = getInt(r->data + 2 + i * 2);
		int len = getInt(r->data + offset) - 2;
		o[i].type = getInt(r->data + offset + 2);
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
static char **_vocabulary_get_knames0alt(int *names, Resource *r) {
	unsigned int mallocsize = 32;
	char **retval = (char **)sci_malloc(sizeof(char *) * mallocsize);
	unsigned int i = 0, index = 0;

	while (index < r->size) {
		int slen = strlen((char *) r->data + index) + 1;

		retval[i] = (char *)sci_malloc(slen);
		memcpy(retval[i++], r->data + index, slen);
		// Wouldn't normally read this, but the cleanup code wants to free() this

		index += slen;

		if (i == mallocsize)
			retval = (char **)sci_realloc(retval, sizeof(char *) * (mallocsize <<= 1));
	}

	*names = i + 1;
	retval = (char **)sci_realloc(retval, sizeof(char *) * (i + 2));
	retval[i] = (char *)sci_malloc(strlen(SCRIPT_UNKNOWN_FUNCTION_STRING) + 1);
	strcpy(retval[i], SCRIPT_UNKNOWN_FUNCTION_STRING);
	// The mystery kernel function- one in each SCI0 package

	retval[i + 1] = NULL; // Required for cleanup

	return retval;
}

static char **vocabulary_get_knames0(ResourceManager *resmgr, int* names) {
	char** t;
	int count, i, index = 2, empty_to_add = 1;
	Resource *r = resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_KNAMES, 0);

	if (!r) { // No kernel name table found? Fall back to default table
		t = (char **)sci_malloc((SCI0_KNAMES_DEFAULT_ENTRIES_NR + 1) * sizeof(char*));
		*names = SCI0_KNAMES_DEFAULT_ENTRIES_NR - 1; // index of last element

		for (i = 0; i < SCI0_KNAMES_DEFAULT_ENTRIES_NR; i++)
			t[i] = sci_strdup(sci0_default_knames[i]);

		t[SCI0_KNAMES_DEFAULT_ENTRIES_NR] = NULL; // Terminate list

		return t;
	}

	count = getInt(r->data);

	if (count > 1023)
		return _vocabulary_get_knames0alt(names, r);

	if (count < SCI0_KNAMES_WELL_DEFINED) {
		empty_to_add = SCI0_KNAMES_WELL_DEFINED - count;
		sciprintf("Less than %d kernel functions; adding %d\n", SCI0_KNAMES_WELL_DEFINED, empty_to_add);
	}

	t = (char **)sci_malloc(sizeof(char*) * (count + 1 + empty_to_add));
	for (i = 0; i < count; i++) {
		int offset = getInt(r->data + index);
		int len = getInt(r->data + offset);
		//fprintf(stderr,"Getting name %d of %d...\n", i, count);
		index += 2;
		t[i] = (char *)sci_malloc(len + 1);
		memcpy(t[i], r->data + offset + 2, len);
		t[i][len] = '\0';
	}

	for (i = 0; i < empty_to_add; i++) {
		t[count + i] = (char *)sci_malloc(strlen(SCRIPT_UNKNOWN_FUNCTION_STRING) + 1);
		strcpy(t[count + i], SCRIPT_UNKNOWN_FUNCTION_STRING);
	}

	t[count+empty_to_add] = 0;
	*names = count + empty_to_add;

	return t;
}

//NOTE: Untested
static char **vocabulary_get_knames1(ResourceManager *resmgr, int *count) {
	char **t = NULL;
	unsigned int size = 64, used = 0, pos = 0;
	Resource *r = resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_KNAMES, 0);
	if(r == NULL) {// failed to open vocab.999 (happens with SCI1 demos)
		t = (char **)sci_malloc((SCI1_KNAMES_DEFAULT_ENTRIES_NR + 1) * sizeof(char*));
		*count = SCI1_KNAMES_DEFAULT_ENTRIES_NR - 1; // index of last element

		for (int i = 0; i < SCI1_KNAMES_DEFAULT_ENTRIES_NR; i++)
			t[i] = sci_strdup(sci1_default_knames[i]);

		t[SCI1_KNAMES_DEFAULT_ENTRIES_NR] = NULL; // Terminate list

		return t;
	}
	while (pos < r->size) {
		int len;
		if ((used == size - 1) || (!t)) {
			size *= 2;
			t = (char **)sci_realloc(t, size * sizeof(char*));
		}
		len = strlen((char *)r->data + pos);
		t[used] = (char *)sci_malloc(len + 1);
		strcpy(t[used], (char *)r->data + pos);
		used++;
		pos += len + 1;
	}
	*count = used;
	t = (char **)sci_realloc(t, (used + 1) * sizeof(char*));
	t[used] = NULL;

	return t;
}
//
static char **vocabulary_get_knames11(ResourceManager *resmgr, int *count) {
/*
 999.voc format for SCI1.1 games:
	[b] # of kernel functions
	[w] unknown
	[offset to function name info]
		...
    {[w name-len][function name]}
		...
*/
	char **t = NULL;
	//unsigned int size = 64, pos = 3;
	int len;
	Resource *r = resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_KNAMES, 0);
	if(r == NULL) // failed to open vocab.999 (happens with SCI1 demos)
		return 0; // FIXME: should return a default table for this engine 
	byte nCnt = *r->data, i;
	t = (char **)sci_malloc(nCnt * sizeof(char*) + 1);
	
	for (i = 0; i < nCnt; i++) {
		int off = READ_LE_UINT16(r->data + 2 * i + 2);
		len = READ_LE_UINT16(r->data + off);
		t[i] = (char *)sci_malloc(len + 1);
		memcpy(t[i], (char *)r->data + off + 2, len);
		t[i][len] = 0;
	}
	*count = nCnt;
	t[nCnt] = NULL;

	return t;
}

char **vocabulary_get_knames(ResourceManager *resmgr, int *count) {
	switch (resmgr->_sciVersion) {
	case SCI_VERSION_0:
	case SCI_VERSION_01:
	case SCI_VERSION_01_VGA:
	case SCI_VERSION_01_VGA_ODD:
		return vocabulary_get_knames0(resmgr, count);
	case SCI_VERSION_1_EARLY:
	case SCI_VERSION_1_LATE:
		return vocabulary_get_knames1(resmgr, count);
	case SCI_VERSION_1_1:
	case SCI_VERSION_32:
		return vocabulary_get_knames11(resmgr, count);
	default:
		return 0;
	}
}

void vocabulary_free_knames(char **names) {
	int i = 0;

	while (names[i]) {
		free(names[i]);
		i++;
	}

	free(names);
}

} // End of namespace Sci
