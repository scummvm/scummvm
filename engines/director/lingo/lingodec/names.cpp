/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "common/str.h"
#include "common/stream.h"
#include "common/util.h"
#include "./enums.h"
#include "./names.h"

namespace LingoDec {

namespace StandardNames {
/* StandardNames */

const char *const opcodeNamesS[] = {
	// single-byte
	"unk00",
	"ret",			// kOpRet			0x01
	"retfactory",	// kOpRetFactory	0x02
	"pushzero",		// kOpPushZero		0x03
	"mul",			// kOpMul			0x04
	"add",			// kOpAdd			0x05
	"sub",			// kOpSub			0x06
	"div",			// kOpDiv			0x07
	"mod",			// kOpMod			0x08
	"inv",			// kOpInv			0x09
	"joinstr",		// kOpJoinStr		0x0a
	"joinpadstr",	// kOpJoinPadStr	0x0b
	"lt",			// kOpLt			0x0c
	"lteq",			// kOpLtEq			0x0d
	"nteq",			// kOpNtEq			0x0e
	"eq",			// kOpEq			0x0f
	"gt",			// kOpGt			0x10
	"gteq",			// kOpGtEq			0x11
	"and",			// kOpAnd			0x12
	"or",			// kOpOr			0x13
	"not",			// kOpNot			0x14
	"containsstr",	// kOpContainsStr	0x15
	"contains0str",	// kOpContains0Str	0x16
	"getchunk",		// kOpGetChunk		0x17
	"hilitechunk",	// kOpHiliteChunk	0x18
	"ontospr",		// kOpOntoSpr		0x19
	"intospr",		// kOpIntoSpr		0x1a
	"getfield",		// kOpGetField		0x1b
	"starttell",	// kOpStartTell		0x1c
	"endtell",		// kOpEndTell		0x1d
	"pushlist",		// kOpPushList		0x1e
	"pushproplist",	// kOpPushPropList	0x1f
	"unk20",
	"swap",			// kOpSwap			0x21
};

const char *const opcodeNamesM[] = {
	// multi-byte
	"unk40",
	"pushint8",			// kOpPushInt8			0x41
	"pusharglistnoret",	// kOpPushArgListNoRet	0x42
	"pusharglist",		// kOpPushArgList		0x43
	"pushcons",			// kOpPushCons			0x44
	"pushsymb",			// kOpPushSymb			0x45
	"pushvarref",		// kOpPushVarRef		0x46
	"unk47",
	"getglobal2",		// kOpGetGlobal2		0x48
	"getglobal",		// kOpGetGlobal			0x49
	"getprop",			// kOpGetProp			0x4a
	"getparam",			// kOpGetParam			0x4b
	"getlocal",			// kOpGetLocal			0x4c
	"unk4d",
	"setglobal2",		// kOpSetGlobal2		0x4e
	"setglobal",		// kOpSetGlobal			0x4f
	"setprop",			// kOpSetProp			0x50
	"setparam",			// kOpSetParam			0x51
	"setlocal",			// kOpSetLocal			0x52
	"jmp",				// kOpJmp				0x53
	"endrepeat",		// kOpEndRepeat			0x54
	"jmpifz",			// kOpJmpIfZ			0x55
	"localcall",		// kOpLocalCall			0x56
	"extcall",			// kOpExtCall			0x57
	"objcallv4",		// kOpObjCallV4			0x58
	"put",				// kOpPut				0x59
	"putchunk",			// kOpPutChunk			0x5a
	"deletechunk",		// kOpDeleteChunk		0x5b
	"get",				// kOpGet				0x5c
	"set",				// kOpSet				0x5d
	"unk5e",
	"getmovieprop",		// kOpGetMovieProp		0x5f
	"setmovieprop",		// kOpSetMovieProp		0x60
	"getobjprop",		// kOpGetObjProp		0x61
	"setobjprop",		// kOpSetObjProp		0x62
	"tellcall",			// kOpTellCall			0x63
	"peek",				// kOpPeek				0x64
	"pop",				// kOpPop				0x65
	"thebuiltin",		// kOpTheBuiltin		0x66
	"objcall",			// kOpObjCall			0x67
	"unk68",
	"unk69",
	"unk6a",
	"unk6b",
	"unk6c",
	"pushchunkvarref",	// kOpPushChunkVarRef	0x6d
	"pushint16",		// kOpPushInt16			0x6e
	"pushint32",		// kOpPushInt32			0x6f
	"getchainedprop",	// kOpGetChainedProp	0x70
	"pushfloat32",		// kOpPushFloat32		0x71
	"gettoplevelprop",	// kOpGetTopLevelProp	0x72
	"newobj",			// kOpNewObj			0x73
};

const char *const binaryOpNames[] = {
	"unk00",
	"unk01",
	"unk02",
	"unk03",
	"*",		// kOpMul			0x04
	"+",		// kOpAdd			0x05
	"-",		// kOpSub			0x06
	"/",		// kOpDiv			0x07
	"mod",		// kOpMod			0x08
	"unk09",
	"&",		// kOpJoinStr		0x0a
	"&&",		// kOpJoinPadStr	0x0b
	"<",		// kOpLt			0x0c
	"<=",		// kOpLtEq			0x0d
	"<>",		// kOpNtEq			0x0e
	"=",		// kOpEq			0x0f
	">",		// kOpGt			0x10
	">=",		// kOpGtEq			0x11
	"and",		// kOpAnd			0x12
	"or",		// kOpOr			0x13
	"unk14",
	"contains",	// kOpContainsStr	0x15
	"starts",	// kOpContains0Str	0x16
};

const char *const chunkTypeNames[] = {
	"unk00",
	"char", // kChunkChar   0x01
	"word", // kChunkWord   0x02
	"item", // kChunkItem   0x03
	"line", // kChunkLine   0x04
};

const char *const putTypeNames[] = {
	"unk00",
	"into",		// kPutInto		0x01
	"after",	// kPutAfter	0x02
	"before",	// kPutBefore	0x03
};

const char *const moviePropertyNames[] = {
	"floatPrecision",	// 0x00
	"mouseDownScript",	// 0x01
	"mouseUpScript",	// 0x02
	"keyDownScript",	// 0x03
	"keyUpScript",		// 0x04
	"timeoutScript",	// 0x05
	"short time",		// 0x06
	"abbr time",		// 0x07
	"long time",		// 0x08
	"short date",		// 0x09
	"abbr date",		// 0x0a
	"long date",		// 0x0b
};

const char *const whenEventNames[] = {
	"unk00",
	"mouseDown",// 0x01
	"mouseUp",	// 0x02
	"keyDown",	// 0x03
	"keyUp",	// 0x04
	"timeOut",	// 0x05
};

const char *const menuPropertyNames[] = {
	"unk00",
	"name",					// 0x01
	"number of menuItems",	// 0x02
};

const char *const menuItemPropertyNames[] = {
	"unk00",
	"name",		// 0x01
	"checkMark",// 0x02
	"enabled",	// 0x03
	"script",	// 0x04
};

const char *const soundPropertyNames[] = {
	"unk00",
	"volume", // 0x01
};

const char *const spritePropertyNames[] = {
	"unk00",
	"type",					// 0x01
	"backColor",			// 0x02
	"bottom",				// 0x03
	"castNum",				// 0x04
	"constraint",			// 0x05
	"cursor",				// 0x06
	"foreColor",			// 0x07
	"height",				// 0x08
	"immediate",			// 0x09
	"ink",					// 0x0a
	"left",					// 0x0b
	"lineSize",				// 0x0c
	"locH",					// 0x0d
	"locV",					// 0x0e
	"movieRate",			// 0x0f
	"movieTime",			// 0x10
	"pattern",				// 0x11
	"puppet",				// 0x12
	"right",				// 0x13
	"startTime",			// 0x14
	"stopTime",				// 0x15
	"stretch",				// 0x16
	"top",					// 0x17
	"trails",				// 0x18
	"visible",				// 0x19
	"volume",				// 0x1a
	"width",				// 0x1b
	"blend",				// 0x1c
	"scriptNum",			// 0x1d
	"moveableSprite",		// 0x1e
	"editableText",			// 0x1f
	"scoreColor",			// 0x20
	"loc",					// 0x21
	"rect",					// 0x22
	"memberNum",			// 0x23
	"castLibNum",			// 0x24
	"member",				// 0x25
	"scriptInstanceList",	// 0x26
	"currentTime",			// 0x27
	"mostRecentCuePoint",	// 0x28
	"tweened",				// 0x29
	"name",					// 0x2a
};

const char *const animationPropertyNames[] = {
	"unk00",
	"beepOn", 					// 0x01
	"buttonStyle", 				// 0x02
	"centerStage", 				// 0x03
	"checkBoxAccess", 			// 0x04
	"checkboxType", 			// 0x05
	"colorDepth", 				// 0x06
	"colorQD", 					// 0x07
	"exitLock", 				// 0x08
	"fixStageSize", 			// 0x09
	"fullColorPermit", 			// 0x0a
	"imageDirect", 				// 0x0b
	"doubleClick", 				// 0x0c
	"key", 						// 0x0d
	"lastClick", 				// 0x0e
	"lastEvent", 				// 0x0f
	"keyCode", 					// 0x10
	"lastKey", 					// 0x11
	"lastRoll", 				// 0x12
	"timeoutLapsed",	 		// 0x13
	"multiSound", 				// 0x14
	"pauseState", 				// 0x15
	"quickTimePresent", 		// 0x16
	"selEnd", 					// 0x17
	"selStart", 				// 0x18
	"soundEnabled", 			// 0x19
	"soundLevel", 				// 0x1a
	"stageColor", 				// 0x1b
	// 0x1c indicates dontPassEvent was called.
	// It doesn't seem to have a Lingo-accessible name.
	"unk1c",
	"switchColorDepth",			// 0x1d
	"timeoutKeyDown",			// 0x1e
	"timeoutLength",			// 0x1f
	"timeoutMouse",				// 0x20
	"timeoutPlay",				// 0x21
	"timer",					// 0x22
	"preLoadRAM",				// 0x23
	"videoForWindowsPresent",	// 0x24
	"netPresent",				// 0x25
	"safePlayer",				// 0x26
	"soundKeepDevice",			// 0x27
	"soundMixMedia",			// 0x28
};

const char *const animation2PropertyNames[] = {
	"unk00",
	"perFrameHook", 		// 0x01
	"number of castMembers",// 0x02
	"number of menus", 		// 0x03
	"number of castLibs", 	// 0x04
	"number of xtras", 		// 0x05
};

const char *const memberPropertyNames[] = {
	"unk00",
	"name",			// 0x01
	"text",			// 0x02
	"textStyle",	// 0x03
	"textFont",		// 0x04
	"textHeight",	// 0x05
	"textAlign",	// 0x06
	"textSize",		// 0x07
	"picture",		// 0x08
	"hilite",		// 0x09
	"number",		// 0x0a
	"size",			// 0x0b
	"loop",			// 0x0c
	"duration",		// 0x0d
	"controller",	// 0x0e
	"directToStage",// 0x0f
	"sound",		// 0x10
	"foreColor",	// 0x11
	"backColor",	// 0x12
	"type",			// 0x13
};

Common::String getOpcodeName(byte id) {
	if (id < 0x22)
		return opcodeNamesS[id];

	if (id < 0x40)
		return Common::String::format("unk%02x" , id);

	id = id % 0x40;

	if (id < 0x34)
		return opcodeNamesM[id];

	return Common::String::format("unk%02x" , id);
}

} // namespace StandardNames

/* ScriptNames */

void ScriptNames::read(Common::SeekableReadStream &stream) {
	// Lingo scripts are always big endian regardless of file endianness
	unknown0 = stream.readSint32BE();
	unknown1 = stream.readSint32BE();
	len1 = stream.readUint32BE();
	len2 = stream.readUint32BE();
	namesOffset = stream.readUint16BE();
	namesCount = stream.readUint16BE();

	stream.seek(namesOffset);
	names.resize(namesCount);
	for (auto &name : names) {
		name = stream.readPascalString();
	}
}

bool ScriptNames::validName(int id) const {
	return -1 < id && (unsigned)id < names.size();
}

Common::String ScriptNames::getName(int id) const {
	if (validName(id))
		return names[id];
	return Common::String::format("UNKNOWN_NAME_%d", id);
}

}
