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

template<class Key, class Val>
static Common::StableMap<Key, Val> toMap(std::initializer_list<Common::Pair<Key, Val>> items) {
	Common::StableMap<Key, Val> stableMap;
	for(auto item : items) {
		stableMap.insert(Common::Pair<Key, Val>(item.first, item.second));
	}
	return stableMap;
}

/* StandardNames */

Common::StableMap<unsigned int, Common::String> StandardNames::opcodeNames = toMap<unsigned int, Common::String>({
	// single-byte
	{ kOpRet,				"ret" },
	{ kOpRetFactory,		"retfactory" },
	{ kOpPushZero,			"pushzero" },
	{ kOpMul,				"mul" },
	{ kOpAdd,				"add" },
	{ kOpSub,				"sub" },
	{ kOpDiv,				"div" },
	{ kOpMod,				"mod" },
	{ kOpInv,				"inv" },
	{ kOpJoinStr,			"joinstr" },
	{ kOpJoinPadStr,		"joinpadstr" },
	{ kOpLt,				"lt" },
	{ kOpLtEq,				"lteq" },
	{ kOpNtEq,				"nteq" },
	{ kOpEq,				"eq" },
	{ kOpGt,				"gt" },
	{ kOpGtEq,				"gteq" },
	{ kOpAnd,				"and" },
	{ kOpOr,				"or" },
	{ kOpNot,				"not" },
	{ kOpContainsStr,		"containsstr" },
	{ kOpContains0Str,		"contains0str" },
	{ kOpGetChunk,			"getchunk" },
	{ kOpHiliteChunk,		"hilitechunk" },
	{ kOpOntoSpr,			"ontospr" },
	{ kOpIntoSpr,			"intospr" },
	{ kOpGetField,			"getfield" },
	{ kOpStartTell,			"starttell" },
	{ kOpEndTell,			"endtell" },
	{ kOpPushList,			"pushlist" },
	{ kOpPushPropList,		"pushproplist" },
	{ kOpSwap,				"swap" },

	// multi-byte
	{ kOpPushInt8,			"pushint8" },
	{ kOpPushArgListNoRet,	"pusharglistnoret" },
	{ kOpPushArgList,		"pusharglist" },
	{ kOpPushCons,			"pushcons" },
	{ kOpPushSymb,			"pushsymb" },
	{ kOpPushVarRef,		"pushvarref" },
	{ kOpGetGlobal2,		"getglobal2" },
	{ kOpGetGlobal,			"getglobal" },
	{ kOpGetProp,			"getprop" },
	{ kOpGetParam,			"getparam" },
	{ kOpGetLocal,			"getlocal" },
	{ kOpSetGlobal2,		"setglobal2" },
	{ kOpSetGlobal,			"setglobal" },
	{ kOpSetProp,			"setprop" },
	{ kOpSetParam,			"setparam" },
	{ kOpSetLocal,			"setlocal" },
	{ kOpJmp,				"jmp" },
	{ kOpEndRepeat,			"endrepeat" },
	{ kOpJmpIfZ,			"jmpifz" },
	{ kOpLocalCall,			"localcall" },
	{ kOpExtCall,			"extcall" },
	{ kOpObjCallV4,			"objcallv4" },
	{ kOpPut,				"put" },
	{ kOpPutChunk,			"putchunk" },
	{ kOpDeleteChunk,		"deletechunk" },
	{ kOpGet,				"get" },
	{ kOpSet,				"set" },
	{ kOpGetMovieProp,		"getmovieprop" },
	{ kOpSetMovieProp,		"setmovieprop" },
	{ kOpGetObjProp,		"getobjprop" },
	{ kOpSetObjProp,		"setobjprop" },
	{ kOpTellCall,			"tellcall" },
	{ kOpPeek,				"peek" },
	{ kOpPop,				"pop" },
	{ kOpTheBuiltin,		"thebuiltin" },
	{ kOpObjCall,			"objcall" },
	{ kOpPushChunkVarRef,	"pushchunkvarref" },
	{ kOpPushInt16,			"pushint16" },
	{ kOpPushInt32,			"pushint32" },
	{ kOpGetChainedProp,	"getchainedprop" },
	{ kOpPushFloat32,		"pushfloat32" },
	{ kOpGetTopLevelProp,	"gettoplevelprop" },
	{ kOpNewObj,			"newobj" }
});

Common::StableMap<unsigned int, Common::String> StandardNames::binaryOpNames = toMap<unsigned int, Common::String>({
	{ kOpMul,			"*" },
	{ kOpAdd,			"+" },
	{ kOpSub,			"-" },
	{ kOpDiv,			"/" },
	{ kOpMod,			"mod" },
	{ kOpJoinStr,		"&" },
	{ kOpJoinPadStr,	"&&" },
	{ kOpLt,			"<" },
	{ kOpLtEq,			"<=" },
	{ kOpNtEq,			"<>" },
	{ kOpEq,			"=" },
	{ kOpGt,			">" },
	{ kOpGtEq,			">=" },
	{ kOpAnd,			"and" },
	{ kOpOr,			"or" },
	{ kOpContainsStr,	"contains" },
	{ kOpContains0Str,	"starts" }
});

Common::StableMap<unsigned int, Common::String> StandardNames::chunkTypeNames = toMap<unsigned int, Common::String>({
	{ kChunkChar, "char" },
	{ kChunkWord, "word" },
	{ kChunkItem, "item" },
	{ kChunkLine, "line" }
});

Common::StableMap<unsigned int, Common::String> StandardNames::putTypeNames = toMap<unsigned int, Common::String>({
	{ kPutInto,		"into" },
	{ kPutAfter,	"after" },
	{ kPutBefore,	"before" }
});

Common::StableMap<unsigned int, Common::String> StandardNames::moviePropertyNames = toMap<unsigned int, Common::String>({
	{ 0x00, "floatPrecision" },
	{ 0x01, "mouseDownScript" },
	{ 0x02, "mouseUpScript" },
	{ 0x03, "keyDownScript" },
	{ 0x04, "keyUpScript" },
	{ 0x05, "timeoutScript" },
	{ 0x06, "short time" },
	{ 0x07, "abbr time" },
	{ 0x08, "long time" },
	{ 0x09, "short date" },
	{ 0x0a, "abbr date" },
	{ 0x0b, "long date" }
});

Common::StableMap<unsigned int, Common::String> StandardNames::whenEventNames = toMap<unsigned int, Common::String>({
	{ 0x01, "mouseDown" },
	{ 0x02, "mouseUp" },
	{ 0x03, "keyDown" },
	{ 0x04, "keyUp" },
	{ 0x05, "timeOut" },
});

Common::StableMap<unsigned int, Common::String> StandardNames::menuPropertyNames = toMap<unsigned int, Common::String>({
	{ 0x01, "name" },
	{ 0x02, "number of menuItems" }
});

Common::StableMap<unsigned int, Common::String> StandardNames::menuItemPropertyNames = toMap<unsigned int, Common::String>({
	{ 0x01, "name" },
	{ 0x02, "checkMark" },
	{ 0x03, "enabled" },
	{ 0x04, "script" }
});

Common::StableMap<unsigned int, Common::String> StandardNames::soundPropertyNames = toMap<unsigned int, Common::String>({
	{ 0x01, "volume" }
});

Common::StableMap<unsigned int, Common::String> StandardNames::spritePropertyNames = toMap<unsigned int, Common::String>({
	{ 0x01, "type" },
	{ 0x02, "backColor" },
	{ 0x03, "bottom" },
	{ 0x04, "castNum" },
	{ 0x05, "constraint" },
	{ 0x06, "cursor" },
	{ 0x07, "foreColor" },
	{ 0x08, "height" },
	{ 0x09, "immediate" },
	{ 0x0a, "ink" },
	{ 0x0b, "left" },
	{ 0x0c, "lineSize" },
	{ 0x0d, "locH" },
	{ 0x0e, "locV" },
	{ 0x0f, "movieRate" },
	{ 0x10, "movieTime" },
	{ 0x11, "pattern" },
	{ 0x12, "puppet" },
	{ 0x13, "right" },
	{ 0x14, "startTime" },
	{ 0x15, "stopTime" },
	{ 0x16, "stretch" },
	{ 0x17, "top" },
	{ 0x18, "trails" },
	{ 0x19, "visible" },
	{ 0x1a, "volume" },
	{ 0x1b, "width" },
	{ 0x1c, "blend" },
	{ 0x1d, "scriptNum" },
	{ 0x1e, "moveableSprite" },
	{ 0x1f, "editableText" },
	{ 0x20, "scoreColor" },
	{ 0x21, "loc" },
	{ 0x22, "rect" },
	{ 0x23, "memberNum" },
	{ 0x24, "castLibNum" },
	{ 0x25, "member" },
	{ 0x26, "scriptInstanceList" },
	{ 0x27, "currentTime" },
	{ 0x28, "mostRecentCuePoint" },
	{ 0x29, "tweened" },
	{ 0x2a, "name" }
});

Common::StableMap<unsigned int, Common::String> StandardNames::animationPropertyNames = toMap<unsigned int, Common::String>({
	{ 0x01, "beepOn" },
	{ 0x02, "buttonStyle" },
	{ 0x03, "centerStage" },
	{ 0x04, "checkBoxAccess" },
	{ 0x05, "checkboxType" },
	{ 0x06, "colorDepth" },
	{ 0x07, "colorQD" },
	{ 0x08, "exitLock" },
	{ 0x09, "fixStageSize" },
	{ 0x0a, "fullColorPermit" },
	{ 0x0b, "imageDirect" },
	{ 0x0c, "doubleClick" },
	{ 0x0d, "key" },
	{ 0x0e, "lastClick" },
	{ 0x0f, "lastEvent" },
	{ 0x10, "keyCode" },
	{ 0x11, "lastKey" },
	{ 0x12, "lastRoll"},
	{ 0x13, "timeoutLapsed" },
	{ 0x14, "multiSound" },
	{ 0x15, "pauseState" },
	{ 0x16, "quickTimePresent" },
	{ 0x17, "selEnd" },
	{ 0x18, "selStart" },
	{ 0x19, "soundEnabled" },
	{ 0x1a, "soundLevel" },
	{ 0x1b, "stageColor" },
	// 0x1c indicates dontPassEvent was called.
	// It doesn't seem to have a Lingo-accessible name.
	{ 0x1d, "switchColorDepth" },
	{ 0x1e, "timeoutKeyDown" },
	{ 0x1f, "timeoutLength" },
	{ 0x20, "timeoutMouse" },
	{ 0x21, "timeoutPlay" },
	{ 0x22, "timer" },
	{ 0x23, "preLoadRAM" },
	{ 0x24, "videoForWindowsPresent" },
	{ 0x25, "netPresent" },
	{ 0x26, "safePlayer" },
	{ 0x27, "soundKeepDevice" },
	{ 0x28, "soundMixMedia" }
});

Common::StableMap<unsigned int, Common::String> StandardNames::animation2PropertyNames = toMap<unsigned int, Common::String>({
	{ 0x01, "perFrameHook" },
	{ 0x02, "number of castMembers" },
	{ 0x03, "number of menus" },
	{ 0x04, "number of castLibs" },
	{ 0x05, "number of xtras" }
});

Common::StableMap<unsigned int, Common::String> StandardNames::memberPropertyNames = toMap<unsigned int, Common::String>({
	{ 0x01, "name" },
	{ 0x02, "text" },
	{ 0x03, "textStyle" },
	{ 0x04, "textFont" },
	{ 0x05, "textHeight" },
	{ 0x06, "textAlign" },
	{ 0x07, "textSize" },
	{ 0x08, "picture" },
	{ 0x09, "hilite" },
	{ 0x0a, "number" },
	{ 0x0b, "size" },
	{ 0x0c, "loop" },
	{ 0x0d, "duration" },
	{ 0x0e, "controller" },
	{ 0x0f, "directToStage" },
	{ 0x10, "sound" },
	{ 0x11, "foreColor" },
	{ 0x12, "backColor" },
	{ 0x13, "type" }
});

Common::String StandardNames::getOpcodeName(uint8_t id) {
	if (id >= 0x40)
		id = 0x40 + id % 0x40;
	auto it = opcodeNames.find(id);
	if (it == opcodeNames.end()){
		return Common::String::format("unk%02X" , id);
	}
	return it->second;
}

Common::String StandardNames::getName(const Common::StableMap<unsigned int, Common::String> &nameMap, unsigned int id) {
	auto it = nameMap.find(id);
	if (it == nameMap.end())
		return "ERROR";
	return it->second;
}

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
