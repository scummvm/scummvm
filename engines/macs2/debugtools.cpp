/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "macs2/debugtools.h"
#include "backends/imgui/imgui.h"
#include "common/debug.h"
#include "common/memstream.h"
#include "common/str-enc.h"
#include "common/system.h"
#include "common/ustr.h"
#include "macs2/detection.h"
#include "macs2/gameobjects.h"
#include "macs2/macs2.h"
#include "macs2/music.h"
#include "macs2/view1.h"

namespace Macs2 {

static bool _showScript = false;
static bool _showObjectScripts = false;
static bool _showVariables = false;
static bool _showCharacters = false;
static bool _showInventory = false;
static bool _showAnimations = false;
static bool _showSceneMaps = false;
static bool _showImageResources = false;
static bool _showDebugOutput = false;
static bool _showPathfindingOverlay = false;
static bool _showDebugOutputOverlay = false;
static bool _showPathOverlay = false;
static bool _showSound = false;
static bool _showAnimViewer = false;
static uint16 _animViewerObjIndex = 0;
static int _animViewerBlobIndex = 0;

static const struct {
	uint16 id;
	const char *name;
} kSpecialNames[] = {
	{0x01, "interactedUse"},
	{0x02, "interactedLook"},
	{0x03, "interactedTalk"},
	{0x04, "areaAtActor"},
	{0x0B, "isRepeatRun"},
	{0x0D, "dialogueResult"},
	{0x23, "pathWalkable"},
	{0x24, "actorX"},
	{0x25, "actorY"},
	{0x26, "isSceneInit"},
	{0x27, "areaRepeatRun"},
	{0x28, "invCheck"},
	{0x29, "animBlobRange"},
	{0x2A, "invCombine"},
	{0x2B, "invAction"},
	{0x2C, "interactedMap"},
	{0x2D, "curScene"},
	{0x2E, "const2"},
	{0x2F, "prevScene"},
	{0x30, "musicActive"},
	{0x31, "soundActive"},
};

static const char *getSpecialName(uint16 val) {
	for (const auto &s : kSpecialNames) {
		if (s.id == val)
			return s.name;
	}
	return nullptr;
}

static const char *getOpcodeName(uint8 opcode) {
	switch (opcode) {
	case 0x01:
		return "setVar";
	case 0x02:
		return "setVarOr";
	case 0x03:
		return "ifFalse";
	case 0x04:
		return "ifTrue";
	case 0x05:
		return "compare";
	case 0x06:
		return "ifInteraction";
	case 0x07:
		return "endIf";
	case 0x08:
		return "else";
	case 0x09:
		return "nop09";
	case 0x0A:
		return "printStringLeft";
	case 0x0B:
		return "moveObject";
	case 0x0C:
		return "changeScene";
	case 0x0D:
		return "showDialogue";
	case 0x0E:
		return "changeAnimation";
	case 0x0F:
		return "frameWait";
	case 0x10:
		return "walkToPosition";
	case 0x11:
		return "waitForWalk";
	case 0x12:
		return "setPathfinding";
	case 0x13:
		return "skipUntil14";
	case 0x14:
		return "skipWord";
	case 0x15:
		return "clearDialogueChoices";
	case 0x16:
		return "addDialogueChoice";
	case 0x17:
		return "showDialogueChoice";
	case 0x18:
		return "dismissPanel";
	case 0x19:
		return "walkToAndPickup";
	case 0x1A:
		return "setPickupFrames";
	case 0x1B:
		return "setupObject";
	case 0x1C:
		return "setSkippable";
	case 0x1D:
		return "clearSkippable";
	case 0x1E:
		return "playAnimation";
	case 0x1F:
		return "pathWalkable = testPathfinding";
	case 0x20:
		return "setYOffset";
	case 0x21:
		return "setMotion";
	case 0x22:
		return "setOrientation";
	case 0x23:
		return "moveToPosition";
	case 0x24:
		return "addValues";
	case 0x25:
		return "subValues";
	case 0x26:
		return "loadSpecialAnim";
	case 0x27:
		return "setDirection";
	case 0x28:
		return "stopAnimation";
	case 0x29:
		return "openInventory";
	case 0x2A:
		return "loadObjectAnim";
	case 0x2B:
		return "checkObjectData";
	case 0x2C:
		return "invCheck = checkInventory";
	case 0x2D:
		return "setSnapToTarget";
	case 0x2E:
		return "animRangeTest = testSceneAnimFrame";
	case 0x2F:
		return "animRangeTest = testObjectAnimFrame";
	case 0x30:
		return "printStringRight";
	case 0x31:
		return "setPaletteDarkness";
	case 0x32:
		return "setObjectShading";
	case 0x33:
		return "setObjectScaling";
	case 0x34:
		return "setHotspotOverride";
	case 0x35:
		return "setObjectBounds";
	case 0x36:
		return "dismissAllPanels";
	case 0x37:
		return "resetToSceneScript";
	case 0x38:
		return "loadOverlayFont";
	case 0x39:
		return "endOverlayText";
	case 0x3A:
		return "addOverlayTextEntry";
	case 0x3B:
		return "clearOverlayText";
	case 0x3C:
		return "fadeToBlack";
	case 0x3D:
		return "fadeFromBlack";
	case 0x3E:
		return "loadPcmSound";
	case 0x3F:
		return "freePcmSound";
	case 0x40:
		return "playPcmSound";
	case 0x41:
		return "waitForSound";
	case 0x42:
		return "stopPcmSound";
	case 0x43:
		return "loadMusicSlot";
	case 0x44:
		return "playMusicSlot";
	case 0x45:
		return "stopMusicSlot";
	case 0x46:
		return "freeMusicSlot";
	case 0x47:
		return "waitForMusic";
	case 0x48:
		return "getObjectX";
	case 0x49:
		return "getObjectY";
	case 0x4A:
		return "getObjectField8";
	case 0x4B:
		return "getObjectOrientation";
	case 0x4C:
		return "clearActorInventory";
	case 0x4D:
		return "setPathfindingRemap";
	case 0x4E:
		return "waitForAdlib";
	default:
		return "???";
	}
}

static const char *getCompareOpName(uint8 sub) {
	switch (sub) {
	case 0x01:
		return "==";
	case 0x02:
		return "!=";
	case 0x03:
		return "<";
	case 0x04:
		return ">";
	case 0x05:
		return "<=";
	case 0x06:
		return ">=";
	default:
		return "?";
	}
}

static Common::String decodeScriptValue(Common::MemoryReadStream *script) {
	if (script->pos() + 3 > script->size())
		return "?";
	uint8 type = script->readByte();
	uint16 val = script->readUint16LE();
	if (type == 0x00) {
		if (val > 0x400 && val <= 0x600)
			return Common::String::format("obj_0x%x", val - 0x400);
		return Common::String::format("%u", val);
	}
	if (type == 0xFF) {
		const char *name = getSpecialName(val);
		if (name)
			return name;
		if (val >= 0x0E && val <= 0x22)
			return Common::String::format("%u", val - 0x0D);
		return Common::String::format("special[0x%02x]", val);
	}
	return Common::String::format("var[%u]", val);
}

// String stream context for decodeParams - set before calling decompileScript
static Common::MemoryReadStream *_decompileStringStream = nullptr;

static Common::String decodeParams(Common::MemoryReadStream *script, uint8 opcode, int64 dataStart, uint8 length) {
	if (length == 0)
		return "";
	int64 savedPos = script->pos();
	script->seek(dataStart);
	Common::String result;
	switch (opcode) {
	case 0x01:
	case 0x02: {
		script->readByte();
		uint16 vi = script->readUint16LE();
		Common::String v = decodeScriptValue(script);
		result = Common::String::format(" var[%u] = %s", vi, v.c_str());
		break;
	}
	case 0x03:
	case 0x04: {
		Common::String v = decodeScriptValue(script);
		result = Common::String::format(" (%s)", v.c_str());
		break;
	}
	case 0x05: {
		uint8 c = script->readByte();
		Common::String a = decodeScriptValue(script), b = decodeScriptValue(script);
		result = Common::String::format(" (%s %s %s)", a.c_str(), getCompareOpName(c), b.c_str());
		break;
	}
	case 0x06: {
		uint8 s = script->readByte();
		Common::String i = decodeScriptValue(script), a = decodeScriptValue(script), b = decodeScriptValue(script);
		result = Common::String::format(" %s(%s, %s, %s)", s == 2 ? "NOT " : "", i.c_str(), a.c_str(), b.c_str());
		break;
	}
	case 0x0A:
	case 0x30: {
		if (length >= 10) {
			script->seek(dataStart + 6);
			uint16 o = script->readUint16LE(), n = script->readUint16LE();
			if (n > 0 && n < 50 && _decompileStringStream) {
				auto ss = g_engine->decodeStrings(_decompileStringStream, o, n);
				if (!ss.empty())
					result = Common::String::format(" \"%s\"", Common::convertFromU32String(Common::convertToU32String(ss[0].c_str(), Common::kDos850), Common::kUtf8).c_str());
			}
		}
		break;
	}
	case 0x0B: {
		Common::String o = decodeScriptValue(script), s = decodeScriptValue(script), x = decodeScriptValue(script), y = decodeScriptValue(script);
		result = Common::String::format(" obj=%s scene=%s pos=(%s,%s)", o.c_str(), s.c_str(), x.c_str(), y.c_str());
		break;
	}
	case 0x0C: {
		Common::String s = decodeScriptValue(script), m = decodeScriptValue(script), sp = decodeScriptValue(script);
		result = Common::String::format(" scene=%s mode=%s speed=%s", s.c_str(), m.c_str(), sp.c_str());
		break;
	}
	case 0x0D: {
		Common::String o = decodeScriptValue(script), x = decodeScriptValue(script), y = decodeScriptValue(script), sd = decodeScriptValue(script);
		if (script->pos() + 4 <= dataStart + length) {
			uint16 off = script->readUint16LE(), n = script->readUint16LE();
			if (n > 0 && n < 50 && _decompileStringStream) {
				auto ss = g_engine->decodeStrings(_decompileStringStream, off, n);
				if (!ss.empty()) {
					result = Common::String::format(" obj=%s \"%s\"", o.c_str(), Common::convertFromU32String(Common::convertToU32String(ss[0].c_str(), Common::kDos850), Common::kUtf8).c_str());
					break;
				}
			}
		}
		result = Common::String::format(" obj=%s pos=(%s,%s) side=%s", o.c_str(), x.c_str(), y.c_str(), sd.c_str());
		break;
	}
	case 0x0F:
	case 0x31:
	case 0x3C:
	case 0x3D: {
		Common::String v = decodeScriptValue(script);
		result = Common::String::format(" %s", v.c_str());
		break;
	}
	case 0x10: {
		Common::String o = decodeScriptValue(script), x = decodeScriptValue(script), y = decodeScriptValue(script);
		result = Common::String::format(" obj=%s pos=(%s,%s)", o.c_str(), x.c_str(), y.c_str());
		break;
	}
	case 0x11:
	case 0x47: {
		Common::String o = decodeScriptValue(script);
		result = Common::String::format(" obj=%s", o.c_str());
		break;
	}
	case 0x48:
	case 0x49:
	case 0x4A:
	case 0x4B: {
		// These opcodes read an object, then save to a variable (subOpcode + varIndex)
		Common::String o = decodeScriptValue(script);
		if (script->pos() + 3 <= dataStart + length) {
			script->readByte(); // subOpcode (always 0x01..0xFE)
			uint16 varIdx = script->readUint16LE();
			result = Common::String::format(" var[%u] = obj=%s", varIdx, o.c_str());
		} else {
			result = Common::String::format(" obj=%s", o.c_str());
		}
		break;
	}
	case 0x12: {
		Common::String a = decodeScriptValue(script), b = decodeScriptValue(script), c = decodeScriptValue(script);
		result = Common::String::format(" area=%s active=%s val=%s", a.c_str(), b.c_str(), c.c_str());
		break;
	}
	case 0x14: {
		uint16 w = script->readUint16LE();
		result = Common::String::format(" 0x%04x", w);
		break;
	}
	case 0x16: {
		Common::String idx = decodeScriptValue(script);
		if (script->pos() + 4 <= dataStart + length) {
			uint16 off = script->readUint16LE(), n = script->readUint16LE();
			if (n > 0 && n < 50 && _decompileStringStream) {
				auto ss = g_engine->decodeStrings(_decompileStringStream, off, n);
				if (!ss.empty()) {
					result = Common::String::format(" idx=%s \"%s\"", idx.c_str(), Common::convertFromU32String(Common::convertToU32String(ss[0].c_str(), Common::kDos850), Common::kUtf8).c_str());
					break;
				}
			}
		}
		result = Common::String::format(" idx=%s", idx.c_str());
		break;
	}
	case 0x17: {
		Common::String o = decodeScriptValue(script), x = decodeScriptValue(script), y = decodeScriptValue(script), sd = decodeScriptValue(script);
		result = Common::String::format(" obj=%s pos=(%s,%s) side=%s", o.c_str(), x.c_str(), y.c_str(), sd.c_str());
		break;
	}
	case 0x19: {
		Common::String a = decodeScriptValue(script), o = decodeScriptValue(script);
		result = Common::String::format(" actor=%s obj=%s", a.c_str(), o.c_str());
		break;
	}
	case 0x1A: {
		Common::String o = decodeScriptValue(script), a = decodeScriptValue(script), b = decodeScriptValue(script);
		result = Common::String::format(" obj=%s start=%s end=%s", o.c_str(), a.c_str(), b.c_str());
		break;
	}
	case 0x1B: {
		Common::String o = decodeScriptValue(script), s = decodeScriptValue(script), v = decodeScriptValue(script);
		result = Common::String::format(" obj=%s slot=%s speed=%s", o.c_str(), s.c_str(), v.c_str());
		break;
	}
	case 0x1E: {
		Common::String o = decodeScriptValue(script), s = decodeScriptValue(script), f = decodeScriptValue(script);
		result = Common::String::format(" obj=%s slot=%s frame=%s", o.c_str(), s.c_str(), f.c_str());
		break;
	}
	case 0x1F: {
		Common::String o = decodeScriptValue(script), x = decodeScriptValue(script), y = decodeScriptValue(script);
		result = Common::String::format(" obj=%s pos=(%s,%s)", o.c_str(), x.c_str(), y.c_str());
		break;
	}
	case 0x20: {
		Common::String o = decodeScriptValue(script), v = decodeScriptValue(script);
		result = Common::String::format(" obj=%s offset=%s", o.c_str(), v.c_str());
		break;
	}
	case 0x21: {
		Common::String o = decodeScriptValue(script), t = decodeScriptValue(script), d = decodeScriptValue(script), m = decodeScriptValue(script);
		result = Common::String::format(" obj=%s target=%s delta=%s dist=%s", o.c_str(), t.c_str(), d.c_str(), m.c_str());
		break;
	}
	case 0x22: {
		Common::String o = decodeScriptValue(script), a = decodeScriptValue(script);
		result = Common::String::format(" obj=%s anim=%s", o.c_str(), a.c_str());
		break;
	}
	case 0x23: {
		Common::String o = decodeScriptValue(script), x = decodeScriptValue(script), y = decodeScriptValue(script), v = decodeScriptValue(script);
		result = Common::String::format(" obj=%s pos=(%s,%s) voff=%s", o.c_str(), x.c_str(), y.c_str(), v.c_str());
		break;
	}
	case 0x24:
	case 0x25: {
		// First value is both operand and save target (original seeks back to re-read it)
		Common::String a = decodeScriptValue(script), b = decodeScriptValue(script);
		const char *op = (opcode == 0x24) ? "+" : "-";
		result = Common::String::format(" %s = %s %s %s", a.c_str(), a.c_str(), op, b.c_str());
		break;
	}
	case 0x26: {
		Common::String o = decodeScriptValue(script);
		Common::String df = decodeScriptValue(script);
		uint8 ai = script->readByte();
		result = Common::String::format(" obj=%s decode=%s anim=%u", o.c_str(), df.c_str(), ai);
		break;
	}
	case 0x27: {
		Common::String o = decodeScriptValue(script), v = decodeScriptValue(script);
		result = Common::String::format(" obj=%s maxFrame=%s", o.c_str(), v.c_str());
		break;
	}
	case 0x29: {
		Common::String o = decodeScriptValue(script);
		result = Common::String::format(" obj=%s", o.c_str());
		break;
	}
	case 0x2A: {
		Common::String o = decodeScriptValue(script), s = decodeScriptValue(script), d = decodeScriptValue(script);
		uint8 ai = script->readByte();
		result = Common::String::format(" obj=%s slot=%s decode=%s idx=%u", o.c_str(), s.c_str(), d.c_str(), ai);
		break;
	}
	case 0x2B: {
		Common::String o = decodeScriptValue(script);
		result = Common::String::format(" obj=%s", o.c_str());
		break;
	}
	case 0x2C:
	case 0x32:
	case 0x33:
	case 0x2D: {
		Common::String o = decodeScriptValue(script), v = decodeScriptValue(script);
		result = Common::String::format(" obj=%s val=%s", o.c_str(), v.c_str());
		break;
	}
	case 0x2E: {
		Common::String a = decodeScriptValue(script), lo = decodeScriptValue(script), hi = decodeScriptValue(script);
		result = Common::String::format(" anim=%s range=[%s,%s]", a.c_str(), lo.c_str(), hi.c_str());
		break;
	}
	case 0x2F: {
		Common::String o = decodeScriptValue(script), s = decodeScriptValue(script), lo = decodeScriptValue(script), hi = decodeScriptValue(script);
		result = Common::String::format(" obj=%s slot=%s range=[%s,%s]", o.c_str(), s.c_str(), lo.c_str(), hi.c_str());
		break;
	}
	case 0x34:
	case 0x4D: {
		Common::String a = decodeScriptValue(script), b = decodeScriptValue(script);
		result = Common::String::format(" %s -> %s", a.c_str(), b.c_str());
		break;
	}
	case 0x35: {
		Common::String o = decodeScriptValue(script), p = decodeScriptValue(script), a = decodeScriptValue(script), b = decodeScriptValue(script), c = decodeScriptValue(script);
		result = Common::String::format(" obj=%s parent=%s (%s,%s,%s)", o.c_str(), p.c_str(), a.c_str(), b.c_str(), c.c_str());
		break;
	}
	case 0x38:
	case 0x3E: {
		uint8 r = script->readByte();
		result = Common::String::format(" res=%u", r);
		break;
	}
	case 0x3A: {
		Common::String x = decodeScriptValue(script), y = decodeScriptValue(script), a = decodeScriptValue(script);
		uint16 so = script->readUint16LE(), t = script->readUint16LE();
		result = Common::String::format(" pos=(%s,%s) align=%s str=%u type=%u", x.c_str(), y.c_str(), a.c_str(), so, t);
		break;
	}
	case 0x43: {
		Common::String s = decodeScriptValue(script);
		uint8 r = script->readByte();
		result = Common::String::format(" slot=%s res=%u", s.c_str(), r);
		break;
	}
	case 0x44:
	case 0x45: {
		Common::String s = decodeScriptValue(script), a = decodeScriptValue(script), b = decodeScriptValue(script);
		result = Common::String::format(" slot=%s %s %s", s.c_str(), a.c_str(), b.c_str());
		break;
	}
	case 0x46: {
		Common::String s = decodeScriptValue(script);
		result = Common::String::format(" slot=%s", s.c_str());
		break;
	}
	case 0x07:
	case 0x08:
	case 0x0E:
	case 0x13:
	case 0x15:
	case 0x18:
	case 0x1C:
	case 0x1D:
	case 0x28:
	case 0x36:
	case 0x37:
	case 0x39:
	case 0x3B:
	case 0x3F:
	case 0x40:
	case 0x41:
	case 0x42:
	case 0x4C:
	case 0x4E:
		break;
	default:
		if (length > 0 && length <= 30) {
			result = " [";
			for (uint8 i = 0; i < length && script->pos() < script->size(); i++) {
				if (i)
					result += " ";
				result += Common::String::format("%02x", script->readByte());
			}
			result += "]";
		}
		break;
	}
	script->seek(savedPos);
	return result;
}

struct DecompiledLine {
	uint32 offset;
	int indent;
	Common::String text;
};

static Common::Array<DecompiledLine> decompileScript(Common::MemoryReadStream *script) {
	Common::Array<DecompiledLine> lines;
	if (!script)
		return lines;
	int64 savedPos = script->pos();
	script->seek(0);
	int indent = 0;

	while (script->pos() < script->size() - 1) {
		uint32 pos = (uint32)script->pos();
		uint8 opcode = script->readByte();
		if (opcode == 0)
			continue;
		uint8 length = script->readByte();
		int64 blockEnd = script->pos() + length;

		if (opcode == 0x07 || opcode == 0x08)
			indent = MAX(0, indent - 1);

		DecompiledLine line;
		line.offset = pos;
		line.indent = indent;
		Common::String params = decodeParams(script, opcode, script->pos(), length);
		line.text = Common::String::format("%s%s", getOpcodeName(opcode), params.c_str());
		lines.push_back(line);

		if (opcode >= 0x03 && opcode <= 0x06)
			indent++;
		if (opcode == 0x08)
			indent++;

		script->seek(blockEnd);
	}
	script->seek(savedPos);
	return lines;
}

static Common::Array<DecompiledLine> &cachedDecompile() {
	static Common::Array<DecompiledLine> *arr = nullptr;
	if (!arr)
		arr = new Common::Array<DecompiledLine>();
	return *arr;
}

static Common::Array<uint32> &collapsedBlocks() {
	static Common::Array<uint32> *arr = nullptr;
	if (!arr)
		arr = new Common::Array<uint32>();
	return *arr;
}

struct Breakpoint {
	uint32 offset;
	bool enabled;
};

static Common::Array<Breakpoint> &breakpoints() {
	static Common::Array<Breakpoint> *arr = nullptr;
	if (!arr)
		arr = new Common::Array<Breakpoint>();
	return *arr;
}

static int _cachedSceneIndex = -1;
bool _scriptDebugPaused = false;
bool _scriptDebugStepRequested = false;
static bool _scriptFollowPC = true;
static bool _showBreakpoints = false;

static int findBreakpoint(uint32 offset) {
	for (uint i = 0; i < breakpoints().size(); i++)
		if (breakpoints()[i].offset == offset)
			return (int)i;
	return -1;
}

static void toggleBreakpoint(uint32 offset) {
	int idx = findBreakpoint(offset);
	if (idx >= 0)
		breakpoints().remove_at(idx);
	else
		breakpoints().push_back({offset, true});
}

static bool hasEnabledBreakpoint(uint32 offset) {
	for (uint i = 0; i < breakpoints().size(); i++)
		if (breakpoints()[i].offset == offset && breakpoints()[i].enabled)
			return true;
	return false;
}

static bool isBlockCollapsed(uint32 offset) {
	for (uint i = 0; i < collapsedBlocks().size(); i++)
		if (collapsedBlocks()[i] == offset)
			return true;
	return false;
}

static void toggleBlockCollapse(uint32 offset) {
	for (uint i = 0; i < collapsedBlocks().size(); i++) {
		if (collapsedBlocks()[i] == offset) {
			collapsedBlocks().remove_at(i);
			return;
		}
	}
	collapsedBlocks().push_back(offset);
}

// Called from ScriptExecutor before executing each opcode
bool scriptDebuggerShouldPause() {
	if (_scriptDebugStepRequested) {
		_scriptDebugStepRequested = false;
		return false; // allow one step
	}
	if (_scriptDebugPaused)
		return true; // stay paused

	// Check breakpoints against current script position
	Script::ScriptExecutor *exec = g_engine->_scriptExecutor;
	uint32 pos = exec->getScriptPosition();
	if (hasEnabledBreakpoint(pos)) {
		_scriptDebugPaused = true;
		return true;
	}
	return false;
}

static void showScriptWindow() {
	if (!_showScript)
		return;
	ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Scene Script", &_showScript)) {
		Script::ScriptExecutor *exec = g_engine->_scriptExecutor;
		int currentScene = Scenes::instance()._currentSceneIndex;

		// --- Debugger toolbar ---
		if (_scriptDebugPaused) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
			if (ImGui::Button("Continue"))
				_scriptDebugPaused = false;
			ImGui::PopStyleColor();
			ImGui::SameLine();
			bool isWaiting = exec->isWaitingForCallback() && !exec->_debugPaused;
			if (isWaiting) {
				ImGui::BeginDisabled();
				ImGui::Button("Step (waiting)");
				ImGui::EndDisabled();
			} else {
				if (ImGui::Button("Step"))
					_scriptDebugStepRequested = true;
			}
		} else {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
			if (ImGui::Button("Pause"))
				_scriptDebugPaused = true;
			ImGui::PopStyleColor();
		}
		ImGui::SameLine();
		ImGui::Checkbox("Follow PC", &_scriptFollowPC);
		ImGui::SameLine();
		if (ImGui::Button("Copy")) {
			Common::String full;
			for (uint i = 0; i < cachedDecompile().size(); i++) {
				const DecompiledLine &l = cachedDecompile()[i];
				for (int j = 0; j < l.indent; j++)
					full += "  ";
				full += Common::String::format("%04x: %s\n", l.offset, l.text.c_str());
			}
			g_system->setTextInClipboard(Common::U32String(full));
		}
		ImGui::SameLine();
		ImGui::Text("| Scene: %d | Pos: %u/%u | %s | Obj: 0x%x",
					currentScene, exec->getDebugOpcodePosition(), exec->getScriptEndPosition(),
					exec->isExecuting() ? (_scriptDebugPaused ? "PAUSED" : (exec->isWaitingForCallback() ? "WAITING" : "RUNNING")) : "Idle",
					exec->getExecutingObjectId());
		ImGui::Separator();

		Common::MemoryReadStream *script = Scenes::instance()._currentSceneScript;
		if (script) {
			if (_cachedSceneIndex != currentScene) {
				_decompileStringStream = Scenes::instance()._currentSceneStrings;
				cachedDecompile() = decompileScript(script);
				_decompileStringStream = nullptr;
				_cachedSceneIndex = currentScene;
				collapsedBlocks().clear();
			}
			uint32 currentPos = exec->getDebugOpcodePosition();
			if (ImGui::BeginChild("ScriptView", ImVec2(0, 0), ImGuiChildFlags_Borders)) {
				int skipUntilIndent = -1; // for collapsed block skipping
				for (uint i = 0; i < cachedDecompile().size(); i++) {
					const DecompiledLine &l = cachedDecompile()[i];

					// Skip lines inside collapsed blocks
					if (skipUntilIndent >= 0) {
						if (l.indent > skipUntilIndent)
							continue;
						skipUntilIndent = -1;
					}

					uint32 nextOff = (i + 1 < cachedDecompile().size()) ? cachedDecompile()[i + 1].offset : exec->getScriptEndPosition();
					bool isCurrent = (currentPos >= l.offset && currentPos < nextOff);

					// Check if this is a collapsible block header (if/else/compare/ifInteraction)
					bool isBlockHeader = (l.text.hasPrefix("ifTrue") || l.text.hasPrefix("ifFalse") ||
										  l.text.hasPrefix("compare") || l.text.hasPrefix("ifInteraction") ||
										  l.text.hasPrefix("else"));
					bool isCollapsed = isBlockCollapsed(l.offset);

					// Breakpoint red dot gutter
					int bpIdx = findBreakpoint(l.offset);
					bool hasBp = (bpIdx >= 0);
					ImVec2 cursorPos = ImGui::GetCursorScreenPos();
					float lineH = ImGui::GetTextLineHeight();
					ImVec2 dotCenter(cursorPos.x + 6, cursorPos.y + lineH * 0.5f);

					// Invisible button for click detection in gutter area
					Common::String btnId = Common::String::format("##bp%u", l.offset);
					ImGui::InvisibleButton(btnId.c_str(), ImVec2(14, lineH));
					if (ImGui::IsItemClicked())
						toggleBreakpoint(l.offset);
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip(hasBp ? "Remove breakpoint" : "Set breakpoint");

					// Draw the red dot (re-check after possible toggle)
					bpIdx = findBreakpoint(l.offset);
					if (bpIdx >= 0) {
						ImU32 col = breakpoints()[bpIdx].enabled ? IM_COL32(255, 0, 0, 255) : IM_COL32(128, 64, 64, 200);
						ImGui::GetWindowDrawList()->AddCircleFilled(dotCenter, 5.0f, col);
					}
					ImGui::SameLine();

					if (isCurrent)
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));

					Common::String ind;
					for (int j = 0; j < l.indent; j++)
						ind += "  ";

					if (isBlockHeader) {
						// Clickable collapse toggle
						const char *arrow = isCollapsed ? "[+]" : "[-]";
						Common::String label = Common::String::format("%04x: %s%s %s###line%u", l.offset, ind.c_str(), arrow, l.text.c_str(), l.offset);
						if (ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_None)) {
							toggleBlockCollapse(l.offset);
						}
						if (isCollapsed)
							skipUntilIndent = l.indent;
					} else {
						ImGui::Text("%04x: %s%s", l.offset, ind.c_str(), l.text.c_str());
					}

					if (isCurrent) {
						ImGui::PopStyleColor();
						if (_scriptFollowPC && exec->isExecuting())
							ImGui::SetScrollHereY(0.5f);
					}
				}
			}
			ImGui::EndChild();
		}
	}
	ImGui::End();
}

static void showBreakpointsWindow() {
	if (!_showBreakpoints)
		return;
	ImGui::SetNextWindowSize(ImVec2(350, 250), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Breakpoints", &_showBreakpoints)) {
		if (ImGui::Button("Clear All"))
			breakpoints().clear();
		ImGui::SameLine();
		ImGui::Text("(%u breakpoints)", (uint)breakpoints().size());
		ImGui::Separator();
		for (int i = 0; i < (int)breakpoints().size(); i++) {
			ImGui::PushID(i);
			ImGui::Checkbox("##en", &breakpoints()[i].enabled);
			ImGui::SameLine();
			ImGui::Text("0x%04x", breakpoints()[i].offset);
			// Find opcode name for this offset
			for (uint j = 0; j < cachedDecompile().size(); j++) {
				if (cachedDecompile()[j].offset == breakpoints()[i].offset) {
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", cachedDecompile()[j].text.c_str());
					break;
				}
			}
			ImGui::SameLine(ImGui::GetWindowWidth() - 30);
			if (ImGui::SmallButton("X")) {
				ImGui::PopID();
				breakpoints().remove_at(i);
				i--;
				continue;
			}
			ImGui::PopID();
		}
	}
	ImGui::End();
}

static void showVariablesWindow() {
	if (!_showVariables)
		return;
	ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Script Variables", &_showVariables)) {
		Script::ScriptExecutor *exec = g_engine->_scriptExecutor;
		if (ImGui::CollapsingHeader("Internal State", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Mouse Mode: 0x%x", (int)exec->_cursorMode);
			ImGui::Text("Interacted Object: 0x%x", exec->_interactedObjectID);
			ImGui::Text("Interacted Other: 0x%x", exec->_interactedInventoryItemId);
			ImGui::Text("Script Skippable: %s", exec->_scriptSkippable ? "Y" : "N");
			ImGui::Text("Inventory Action/Combine: %s/%s", exec->_inventoryActionFlag ? "Y" : "N", exec->_inventoryCombineFlag ? "Y" : "N");
			ImGui::Text("Path Walkable Result: %s", exec->_pathWalkableResult ? "Y" : "N");
			ImGui::Text("Pickup In Progress: %s", exec->_pickupInProgress ? "Y" : "N");
			ImGui::Text("Repeat Run Flag (0x0B isRepeatRun): %s", exec->getRepeatRunFlag() ? "Y" : "N");
			ImGui::Text("Scene Init Run (0x26): %s", exec->getSpecialValue(0x26) ? "Y" : "N");
			ImGui::Text("Movement Repeat (0x27 areaRepeatRun): %s", exec->_isRepeatRun ? "Y" : "N");
			ImGui::Text("Walk Target Object: %u", exec->_walkTargetObjectIndex);
			if (exec->_walkTargetObjectIndex > 0) {
				View1 *wView = (View1 *)g_engine->findView("View1");
				Character *wChar = wView ? wView->getCharacterByIndex(exec->_walkTargetObjectIndex) : nullptr;
				if (wChar) {
					Common::Point wPos = wChar->getPosition();
					ImGui::Text("  Position: (%d, %d)", wPos.x, wPos.y);
					ImGui::Text("  Final Dest: (%d, %d)", wChar->_pathFinalDestination.x, wChar->_pathFinalDestination.y);
					ImGui::Text("  HasMotionVOffset: %s  TargetVOffset: %u  VOffsetScale: %u",
								wChar->hasPendingVerticalMotion() ? "Y" : "N",
								wChar->_motionTargetVerticalOffset,
								wChar->_gameObject->_verticalOffsetScale);
				}
			}
		}
		if (ImGui::CollapsingHeader("Runtime Specials (FF, read-only)", ImGuiTreeNodeFlags_DefaultOpen)) {
			for (const auto &s : kSpecialNames) {
				uint32 val = exec->getSpecialValue(s.id);
				ImGui::Text("%-15s (FF:%02x) = %u (0x%x)", s.name, s.id, val, val);
			}
		}
		if (ImGui::CollapsingHeader("Variables (non-zero)", ImGuiTreeNodeFlags_DefaultOpen)) {
			for (int i = 0; i < 0x800; i++) {
				uint32 val = exec->getVariableValue(i);
				if (val != 0)
					ImGui::Text("[%03x] = %u (0x%x)", i, val, val);
			}
		}
		if (ImGui::CollapsingHeader("Dialogue Choices", ImGuiTreeNodeFlags_DefaultOpen)) {
			View1 *view = (View1 *)g_engine->findView("View1");
			if (view && view->_isDialogueChoiceInputActive) {
				ImGui::Text("Showing: Y | Count: %u", view->_dialogueChoiceCount);
				ImGui::Text("BoxPos: (%d,%d)", view->_stringBoxPosition.x, view->_stringBoxPosition.y);
				Common::Point mousePos = g_system->getEventManager()->getMousePos();
				int lineHeight = g_engine->maxGlyphHeight + 2;
				int firstLineY = view->_stringBoxPosition.y + 9;
				int relY = mousePos.y - firstLineY;
				int hoveredChoice = -1;
				if (relY >= 0 && lineHeight > 0) {
					int hoveredLine = relY / lineHeight;
					int cumulativeLines = 0;
					for (uint i = 0; i < view->_dialogueChoiceLineCounts.size(); i++) {
						if (hoveredLine < cumulativeLines + (int)view->_dialogueChoiceLineCounts[i]) {
							hoveredChoice = i + 1;
							break;
						}
						cumulativeLines += view->_dialogueChoiceLineCounts[i];
					}
				}
				ImGui::Text("Mouse: (%d,%d) | Hovered: %d", mousePos.x, mousePos.y, hoveredChoice);
				ImGui::Text("LineHeight: %d | FirstLineY: %d", lineHeight, firstLineY);
				for (uint i = 0; i < view->_dialogueChoiceLineCounts.size(); i++) {
					uint16 scriptIdx = (i < g_engine->_scriptExecutor->_dialogueChoiceScriptIndices.size())
										   ? g_engine->_scriptExecutor->_dialogueChoiceScriptIndices[i]
										   : 0;
					ImGui::Text("  Choice %u: %u lines, scriptIndex=%u", i + 1, view->_dialogueChoiceLineCounts[i], scriptIdx);
				}
				ImGui::Separator();
				for (uint i = 0; i < g_engine->_scriptExecutor->_dialogueChoices.size(); i++) {
					ImGui::Text("  Choice %u:", i + 1);
					for (uint j = 0; j < g_engine->_scriptExecutor->_dialogueChoices[i].size(); j++) {
						ImGui::Text("    [%u] \"%s\"", j, g_engine->_scriptExecutor->_dialogueChoices[i][j].c_str());
					}
				}
			} else {
				ImGui::TextUnformatted("Showing: N");
			}
			ImGui::Text("chosenDialogueOption (FF:0D): %d", g_engine->_scriptExecutor->_chosenDialogueOption);
		}
	}
	ImGui::End();
}

static void showAnimViewerWindow() {
	if (!_showAnimViewer)
		return;
	ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
	Common::String title = Common::String::format("Animation Viewer - obj 0x%x blob %d###AnimViewer", _animViewerObjIndex, _animViewerBlobIndex);
	if (ImGui::Begin(title.c_str(), &_showAnimViewer)) {
		GameObject *obj = nullptr;
		for (auto o : GameObjects::instance()._objects) {
			if (o && o->_index == _animViewerObjIndex) {
				obj = o;
				break;
			}
		}
		bool hasBlob = obj && ((_animViewerBlobIndex == -1 && !obj->_overloadAnimation.empty()) ||
							   (_animViewerBlobIndex >= 0 && _animViewerBlobIndex < (int)obj->_blobs.size() && !obj->_blobs[_animViewerBlobIndex].empty()));
		if (hasBlob) {
			// Blob selector
			const char *currentLabel = _animViewerBlobIndex == -1 ? "Overload" : Common::String::format("%d", _animViewerBlobIndex).c_str();
			if (ImGui::BeginCombo("Blob", currentLabel)) {
				for (int b = 0; b < (int)obj->_blobs.size(); b++) {
					if (obj->_blobs[b].empty())
						continue;
					bool selected = (b == _animViewerBlobIndex);
					if (ImGui::Selectable(Common::String::format("%d", b).c_str(), selected))
						_animViewerBlobIndex = b;
				}
				if (!obj->_overloadAnimation.empty()) {
					bool selected = (_animViewerBlobIndex == -1);
					if (ImGui::Selectable("Overload", selected))
						_animViewerBlobIndex = -1;
				}
				ImGui::EndCombo();
			}
			ImGui::Separator();

			Common::Array<uint8> &blob = (_animViewerBlobIndex == -1) ? obj->_overloadAnimation : obj->_blobs[_animViewerBlobIndex];
			AnimBlobView blobView(blob);
			if (blobView.isValid()) {
				uint16 frameCount = blobView.frameCount();
				ImGui::Text("Frames: %u  SeqLen: %u  Blob size: %u bytes", frameCount, blobView.sequenceLength(), (uint)blob.size());

				static Graphics::ManagedSurface animViewSurface;
				float availWidth = ImGui::GetContentRegionAvail().x;
				float xCursor = 0;

				for (uint16 f = 0; f < frameCount; f++) {
					AnimBlobView::FrameInfo fi;
					if (!blobView.getFrameInfo(f, fi))
						break;

					animViewSurface.create(fi.width, fi.height, Graphics::PixelFormat::createFormatCLUT8());
					animViewSurface.fillRect(Common::Rect(fi.width, fi.height), 0);
					for (uint16 row = 0; row < fi.height; row++) {
						for (uint16 col = 0; col < fi.width; col++) {
							byte pixel = fi.pixels[row * fi.width + col];
							if (pixel != 0)
								animViewSurface.setPixel(col, row, pixel);
						}
					}

					ImTextureID texId = (ImTextureID)(intptr_t)g_system->getImGuiTexture(*animViewSurface.surfacePtr(), g_engine->_pal, 256);
					if (texId) {
						float scale = MIN(128.0f / (float)fi.width, 128.0f / (float)fi.height);
						if (scale > 3.0f)
							scale = 3.0f;
						float imgW = fi.width * scale;
						float imgH = fi.height * scale;

						if (xCursor + imgW > availWidth && xCursor > 0) {
							xCursor = 0;
						} else if (xCursor > 0) {
							ImGui::SameLine();
						}

						ImGui::BeginGroup();
						ImGui::Image(texId, ImVec2(imgW, imgH));
						ImGui::Text("%u (%d,%d)", f, fi.offsetX, fi.offsetY);
						ImGui::EndGroup();

						xCursor += imgW + 8;
					}
				}
			}
		} else {
			ImGui::Text("Object 0x%x blob %d not available", _animViewerObjIndex, _animViewerBlobIndex);
		}
	}
	ImGui::End();
}

// Bounding box overlay for hovered scene object
static Common::Rect _hoveredObjectBounds;
static bool _hasHoveredObjectBounds = false;

static void drawHoveredObjectOverlay() {
	if (!_hasHoveredObjectBounds)
		return;
	ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	// The game is 320x200 but may use aspect ratio correction (4:3 = 320x240 effective)
	// ScummVM fits the game into the window preserving aspect ratio with letterboxing
	int gameW = g_system->getWidth();  // 320
	int gameH = g_system->getHeight(); // 200
	// Effective display aspect: if aspect ratio correction is on, it's 4:3
	float effectiveH = (float)gameH;
	if (g_system->getFeatureState(OSystem::kFeatureAspectRatioCorrection))
		effectiveH = (float)gameW * 3.0f / 4.0f; // 240 for 320-wide

	float scaleX = displaySize.x / (float)gameW;
	float scaleY = displaySize.y / effectiveH;
	float scale = MIN(scaleX, scaleY);
	float renderedW = (float)gameW * scale;
	float renderedH = effectiveH * scale;
	float offsetX = (displaySize.x - renderedW) * 0.5f;
	float offsetY = (displaySize.y - renderedH) * 0.5f;
	// Map game coords (0..319, 0..199) into the rendered area
	float pixScaleX = renderedW / (float)gameW;
	float pixScaleY = renderedH / (float)gameH;
	ImVec2 tl(offsetX + _hoveredObjectBounds.left * pixScaleX, offsetY + _hoveredObjectBounds.top * pixScaleY);
	ImVec2 br(offsetX + _hoveredObjectBounds.right * pixScaleX, offsetY + _hoveredObjectBounds.bottom * pixScaleY);
	ImGui::GetBackgroundDrawList()->AddRect(tl, br, IM_COL32(0, 255, 0, 255), 0.0f, 0, 2.0f);
}

static void showCharactersWindow() {
	_hasHoveredObjectBounds = false;
	if (!_showCharacters)
		return;
	ImGui::SetNextWindowSize(ImVec2(550, 550), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Scene Objects", &_showCharacters)) {
		View1 *view = (View1 *)g_engine->findView("View1");
		if (view) {
			// Active characters (objects with runtime animation data)
			if (ImGui::CollapsingHeader("Active Characters", ImGuiTreeNodeFlags_DefaultOpen)) {
				for (uint i = 0; i < view->_characters.size(); i++) {
					Character *c = view->_characters[i];
					if (!c || !c->_gameObject)
						continue;
					Common::Point pos = c->getPosition();
					Common::String header = Common::String::format("[%u] obj=0x%x (%d,%d)###char%u", i, c->_gameObject->_index, pos.x, pos.y, i);
					bool charOpen = ImGui::CollapsingHeader(header.c_str());
					if (ImGui::IsItemHovered()) {
						_hasHoveredObjectBounds = true;
						_hoveredObjectBounds = Common::Rect(c->_gameObject->_lastDrawX, c->_gameObject->_lastDrawY,
															c->_gameObject->_lastDrawX + c->_gameObject->_lastDrawWidth,
															c->_gameObject->_lastDrawY + c->_gameObject->_lastDrawHeight);
					}
					if (charOpen) {
						ImGui::PushID(i);

						// --- Editable Position ---
						int posXY[2] = {pos.x, pos.y};
						if (ImGui::InputInt2("Position", posXY)) {
							c->setPosition(Common::Point((int16)posXY[0], (int16)posXY[1]));
						}

						// --- Editable GameObject fields ---
						int orient = (int)c->_gameObject->_orientation;
						if (ImGui::InputInt("Orientation", &orient)) {
							c->_gameObject->_orientation = (uint8)CLIP(orient, 0, 255);
						}

						int animIdx = (int)c->_animationIndex;
						if (ImGui::InputInt("Animation Index", &animIdx)) {
							c->_animationIndex = (uint8)CLIP(animIdx, 0, 255);
						}

						ImGui::Text("Vertical Offset: %u", c->getVerticalOffset());

						bool hasShading = c->_gameObject->_hasShading;
						if (ImGui::Checkbox("HasShading", &hasShading))
							c->_gameObject->_hasShading = hasShading;
						ImGui::SameLine();
						bool hasScaling = c->_gameObject->_hasScaling;
						if (ImGui::Checkbox("HasScaling", &hasScaling))
							c->_gameObject->_hasScaling = hasScaling;
						ImGui::SameLine();
						bool hasBounds = c->_gameObject->_hasBoundsAttachment;
						if (ImGui::Checkbox("BoundsAttach", &hasBounds))
							c->_gameObject->_hasBoundsAttachment = hasBounds;

						// --- Motion fields ---
						if (ImGui::TreeNode("Motion")) {
							int motionTarget = (int)c->_motionTargetVerticalOffset;
							if (ImGui::InputInt("Target VOffset", &motionTarget))
								c->_motionTargetVerticalOffset = (uint16)CLIP(motionTarget, 0, 65535);
							int motionDelta = (int)c->_motionVerticalOffsetDelta;
							if (ImGui::InputInt("VOffset Delta", &motionDelta))
								c->_motionVerticalOffsetDelta = (uint16)CLIP(motionDelta, 0, 65535);
							int motionDist = (int)c->_motionDistanceUnits;
							if (ImGui::InputInt("Distance Units", &motionDist))
								c->_motionDistanceUnits = (uint16)CLIP(motionDist, 0, 65535);
							int motionProg = (int)c->_motionProgress;
							if (ImGui::InputInt("Progress", &motionProg))
								c->_motionProgress = (uint16)CLIP(motionProg, 0, 65535);
							ImGui::Text("Pending VOffset Motion: %s", c->hasPendingVerticalMotion() ? "Y" : "N");
							bool shouldMirror = c->_shouldMirrorCurrentAnimation;
							if (ImGui::Checkbox("Mirror Animation", &shouldMirror))
								c->_shouldMirrorCurrentAnimation = shouldMirror;
							ImGui::TreePop();
						}

						// --- Pickup fields ---
						if (ImGui::TreeNode("Pickup")) {
							int pickupFrame = (int)c->_pickupFrameCounter;
							if (ImGui::InputInt("Frame Counter", &pickupFrame))
								c->_pickupFrameCounter = (uint16)CLIP(pickupFrame, 0, 65535);
							bool pickupTransferred = c->_pickupItemTransferred;
							if (ImGui::Checkbox("Item Transferred", &pickupTransferred))
								c->_pickupItemTransferred = pickupTransferred;
							ImGui::Text("Previous Orientation: %u", c->_previousOrientation);
							ImGui::TreePop();
						}

						// --- Dirty rect ---
						if (ImGui::TreeNode("Dirty Rect")) {
							ImGui::Text("Dirty: (%d,%d)-(%d,%d)", c->_gameObject->_dirtyLeft, c->_gameObject->_dirtyTop,
										c->_gameObject->_dirtyRight, c->_gameObject->_dirtyBottom);
							ImGui::Text("LastDraw: (%d,%d) %ux%u", c->_gameObject->_lastDrawX, c->_gameObject->_lastDrawY,
										c->_gameObject->_lastDrawWidth, c->_gameObject->_lastDrawHeight);
							ImGui::TreePop();
						}

						// --- Movement / Pathfinding ---
						ImGui::Separator();
						ImGui::TextUnformatted("Movement:");

						bool dirSet = c->_stepDirectionSet;
						if (ImGui::Checkbox("DirSet", &dirSet))
							c->_stepDirectionSet = dirSet;
						ImGui::SameLine();

						// Direction availability
						if (c->_gameObject->_blobs.size() >= 8) {
							ImGui::TextUnformatted("DirAvail: ");
							ImGui::SameLine();
							const char *dirNames[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
							for (int d = 0; d < 8; d++) {
								if (d > 0)
									ImGui::SameLine(0, 4);
								bool avail = d < (int)c->_gameObject->_blobs.size() && !c->_gameObject->_blobs[d].empty();
								if (avail)
									ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", dirNames[d]);
								else
									ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1), "%s", dirNames[d]);
							}
						}

						// Animation frame preview - open in separate window
						if (ImGui::TreeNode("Animations")) {
							for (int blobIdx = 0; blobIdx < (int)c->_gameObject->_blobs.size(); blobIdx++) {
								Common::Array<uint8> &blob = c->_gameObject->_blobs[blobIdx];
								if (blob.empty())
									continue;
								AnimBlobView bv(blob);
								ImGui::PushID(blobIdx);
								Common::String btnLabel = Common::String::format("Blob %d (%u frames)", blobIdx, bv.frameCount());
								if (ImGui::Button(btnLabel.c_str())) {
									_animViewerObjIndex = c->_gameObject->_index;
									_animViewerBlobIndex = blobIdx;
									_showAnimViewer = true;
								}
								ImGui::PopID();
								if ((blobIdx + 1) % 5 != 0)
									ImGui::SameLine();
							}
							if (!c->_gameObject->_overloadAnimation.empty()) {
								ImGui::NewLine();
								AnimBlobView ov(c->_gameObject->_overloadAnimation);
								Common::String ovLabel = Common::String::format("Overload (%u frames, trigger orient=%u)",
																				ov.frameCount(), c->_gameObject->_overloadAnimTriggerDirection);
								if (ImGui::Button(ovLabel.c_str())) {
									_animViewerObjIndex = c->_gameObject->_index;
									_animViewerBlobIndex = -1; // sentinel for overload
									_showAnimViewer = true;
								}
							}
							ImGui::NewLine();
							ImGui::TreePop();
						}

						// Dump character state to JSON file
						if (ImGui::Button("Dump JSON")) {
							GameObject *go = c->_gameObject;
							Common::String filename = Common::String::format("obj_%u_dump.json", go->_index);
							Common::DumpFile df;
							if (df.open(Common::Path(filename))) {
								df.writeString("{\n");
								df.writeString(Common::String::format("  \"index\": %u,\n", go->_index));
								df.writeString(Common::String::format("  \"position\": [%d, %d],\n", go->_position.x, go->_position.y));
								df.writeString(Common::String::format("  \"sceneIndex\": %u,\n", go->_sceneIndex));
								df.writeString(Common::String::format("  \"orientation\": %u,\n", go->_orientation));
								df.writeString(Common::String::format("  \"verticalOffsetScale\": %u,\n", go->_verticalOffsetScale));
								df.writeString(Common::String::format("  \"hasScaling\": %s,\n", go->_hasScaling ? "true" : "false"));
								df.writeString(Common::String::format("  \"hasShading\": %s,\n", go->_hasShading ? "true" : "false"));
								df.writeString(Common::String::format("  \"hasBoundsAttachment\": %s,\n", go->_hasBoundsAttachment ? "true" : "false"));
								df.writeString(Common::String::format("  \"boundsAttachmentObjectID\": %u,\n", go->_boundsAttachmentObjectID));
								df.writeString(Common::String::format("  \"overloadAnimTriggerDirection\": %u,\n", go->_overloadAnimTriggerDirection));
								df.writeString(Common::String::format("  \"dataOffset\": %u,\n", go->_dataOffset));
								df.writeString("  \"blobs\": [\n");
								for (uint b = 0; b < go->_blobs.size(); b++) {
									df.writeString(Common::String::format("    {\"slot\": %u, \"size\": %u, \"sourceKey\": %u, \"walkSpeed\": %u, \"mirror\": %s}%s\n",
																		  b, (uint)go->_blobs[b].size(),
																		  b < go->_blobSourceKeys.size() ? go->_blobSourceKeys[b] : 0,
																		  b < go->_blobWalkSpeeds.size() ? go->_blobWalkSpeeds[b] : 0,
																		  (b < go->_blobMirrorFlags.size() && go->_blobMirrorFlags[b]) ? "true" : "false",
																		  b + 1 < go->_blobs.size() ? "," : ""));
								}
								df.writeString("  ],\n");
								df.writeString(Common::String::format("  \"character\": {\n"));
								df.writeString(Common::String::format("    \"positionX\": %d,\n", c->getPosition().x));
								df.writeString(Common::String::format("    \"positionY\": %d,\n", c->getPosition().y));
								df.writeString(Common::String::format("    \"shouldMirror\": %s,\n", c->_shouldMirrorCurrentAnimation ? "true" : "false"));
								df.writeString(Common::String::format("    \"verticalOffset\": %u\n", c->getVerticalOffset()));
								df.writeString("  }\n");
								df.writeString("}\n");
								df.close();
								warning("Dumped character state to %s", filename.c_str());
							}
						}

						ImGui::PopID();
					}
				}
			} // end Active Characters header

			// All other objects in this scene (not active characters)
			uint16 sceneIdx = (uint16)Scenes::instance()._currentSceneIndex;
			if (ImGui::CollapsingHeader("Other Scene Objects")) {
				for (auto obj : GameObjects::instance()._objects) {
					if (obj == nullptr || obj->_sceneIndex != sceneIdx)
						continue;
					// Skip objects already shown as active characters
					bool isActive = false;
					for (uint i = 0; i < view->_characters.size(); i++) {
						if (view->_characters[i] && view->_characters[i]->_gameObject == obj) {
							isActive = true;
							break;
						}
					}
					if (isActive)
						continue;
					Common::String hdr = Common::String::format("obj=0x%x (%d,%d) shade=%s scale=%s###scnobj%u",
																obj->_index, obj->_position.x, obj->_position.y,
																obj->_hasShading ? "Y" : "N", obj->_hasScaling ? "Y" : "N", obj->_index);
					bool objOpen = ImGui::CollapsingHeader(hdr.c_str());
					if (ImGui::IsItemHovered()) {
						_hasHoveredObjectBounds = true;
						// Scene objects don't have sprite bounds; show a small rect around position
						_hoveredObjectBounds = Common::Rect(obj->_position.x - 8, obj->_position.y - 16,
															obj->_position.x + 8, obj->_position.y + 2);
					}
					if (objOpen) {
						ImGui::Text("Orient: %u  VOffsetScale: %u", obj->_orientation, obj->_verticalOffsetScale);
						ImGui::Text("Blobs: %u  Script: %u bytes", (uint)obj->_blobs.size(), (uint)obj->_script.size());
					}
				}
			}
		}
	}
	ImGui::End();
}

static void showInventoryWindow() {
	if (!_showInventory)
		return;
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Inventory", &_showInventory)) {
		View1 *view = (View1 *)g_engine->findView("View1");
		if (view) {
			ImGui::Text("Source: obj %u | Active: %s | Showing: %s",
						view->_inventorySource ? view->_inventorySource->_index : 0,
						view->_activeInventoryItem ? Common::String::format("0x%x", view->_activeInventoryItem->_index).c_str() : "None",
						view->_uiPanelState == View1::kUiPanelInventory ? "Y" : "N");
			ImGui::Separator();

			if (ImGui::CollapsingHeader("Current Inventory", ImGuiTreeNodeFlags_DefaultOpen)) {
				for (uint i = 0; i < view->_inventoryItems.size(); i++) {
					GameObject *obj = view->_inventoryItems[i];
					const Common::String &name = (obj->_index < GameObjects::instance()._objectNames.size() && !GameObjects::instance()._objectNames[obj->_index].empty())
													 ? GameObjects::instance()._objectNames[obj->_index]
													 : "???";
					Common::String utf8Name = Common::U32String(name.c_str(), Common::kDos850).encode(Common::kUtf8);
					ImGui::PushID(obj->_index);
					if (ImGui::Button("Remove")) {
						obj->_sceneIndex = 0;
						view->setInventorySource(view->_inventorySource);
					}
					ImGui::SameLine();
					ImGui::Text("[%u] 0x%02x %s", i, obj->_index, utf8Name.c_str());
					ImGui::PopID();
				}
			}

			if (ImGui::CollapsingHeader("All Items")) {
				static char filterBuf[64] = "";
				ImGui::InputText("Filter", filterBuf, sizeof(filterBuf));
				const uint16 protagonistInvScene = view->_inventorySource ? view->_inventorySource->_index + 0x400 : 0x401;
				for (GameObject *obj : GameObjects::instance()._objects) {
					if (obj == nullptr || obj->_index <= 1)
						continue;
					if (obj->_blobs.size() <= 0x13 || obj->_blobs[0x13].empty())
						continue;
					const Common::String &name = (obj->_index < GameObjects::instance()._objectNames.size() && !GameObjects::instance()._objectNames[obj->_index].empty())
													 ? GameObjects::instance()._objectNames[obj->_index]
													 : "???";
					Common::String utf8Name = Common::U32String(name.c_str(), Common::kDos850).encode(Common::kUtf8);
					if (filterBuf[0] != '\0' && !utf8Name.contains(filterBuf))
						continue;
					bool inInventory = (obj->_sceneIndex == protagonistInvScene);
					ImGui::PushID(0x1000 + obj->_index);
					if (inInventory) {
						if (ImGui::Button("Remove")) {
							obj->_sceneIndex = 0;
							view->setInventorySource(view->_inventorySource);
						}
					} else {
						if (ImGui::Button("  Add ")) {
							obj->_sceneIndex = protagonistInvScene;
							view->setInventorySource(view->_inventorySource);
						}
					}
					ImGui::SameLine();
					ImGui::Text("0x%02x %s%s", obj->_index, utf8Name.c_str(), inInventory ? " [IN]" : "");
					ImGui::PopID();
				}
			}
		}
	}
	ImGui::End();
}

static void showAnimationsWindow() {
	if (!_showAnimations)
		return;
	ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Animations", &_showAnimations)) {
		ImGui::Text("BG Anims: %u", (uint)g_engine->_backgroundAnimations.size());
		for (int i = 0; i < (int)g_engine->_backgroundAnimations.size(); i++) {
			BackgroundAnimation &a = g_engine->_backgroundAnimations[i];
			ImGui::Text("  [%d] pos=(%u,%u) frames=%u cur=%u", i, a._x, a._y, (uint)a._frames.size(), a._frameIndex);
		}
	}
	ImGui::End();
}

static void showSceneMapsWindow() {
	if (!_showSceneMaps)
		return;
	ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Scene Maps", &_showSceneMaps)) {
		static int selectedTab = 0;
		if (ImGui::BeginTabBar("MapTabs")) {
			if (ImGui::BeginTabItem("Pathfinding Map")) {
				selectedTab = 0;
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Depth Map")) {
				selectedTab = 1;
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Shadow Map")) {
				selectedTab = 6;
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Pathfinding Overlay")) {
				selectedTab = 2;
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Hotspot Map")) {
				selectedTab = 3;
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Hotspot Overlay")) {
				selectedTab = 5;
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Object Map")) {
				selectedTab = 4;
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		Graphics::ManagedSurface *surface = nullptr;
		static Graphics::ManagedSurface overlayComposite;

		if (selectedTab == 0) {
			surface = &g_engine->_pathfindingMap;
		} else if (selectedTab == 1) {
			surface = &g_engine->_depthMap;
		} else if (selectedTab == 2) {
			// Composite: pathfinding map + character path overlay + pathfinding points
			overlayComposite.copyFrom(g_engine->_pathfindingMap);
			View1 *view = (View1 *)g_engine->findView("View1");
			if (view) {
				// Draw pathfinding point nodes and connections
				for (int i = 0; i < 16; i++) {
					PathfindingPoint &pt = g_engine->pathfindingPoints[i];
					if (pt._position.x >= 0 && pt._position.x < 320 && pt._position.y >= 0 && pt._position.y < 200) {
						// Draw cross at node
						for (int d = -2; d <= 2; d++) {
							int px = pt._position.x + d, py = pt._position.y;
							if (px >= 0 && px < 320)
								overlayComposite.setPixel(px, py, 0xFF);
							px = pt._position.x;
							py = pt._position.y + d;
							if (py >= 0 && py < 200)
								overlayComposite.setPixel(px, py, 0xFF);
						}
						// Draw connections
						for (uint8 adj : pt._adjacentPoints) {
							if (adj == 0 || adj > 16)
								continue;
							PathfindingPoint &other = g_engine->pathfindingPoints[adj - 1];
							overlayComposite.drawLine(pt._position.x, pt._position.y, other._position.x, other._position.y, 0xFE);
						}
					}
				}
				// Draw current path
				if (g_engine->_path.size() >= 2) {
					for (uint i = 0; i < g_engine->_path.size() - 1; i++) {
						overlayComposite.drawLine(g_engine->_path[i].x, g_engine->_path[i].y,
												  g_engine->_path[i + 1].x, g_engine->_path[i + 1].y, 0x0F);
					}
				}
				// Draw character positions
				for (uint i = 0; i < view->_characters.size(); i++) {
					Character *c = view->_characters[i];
					if (!c || !c->_gameObject)
						continue;
					Common::Point pos = c->getPosition();
					if (pos.x >= 3 && pos.x < 317 && pos.y >= 3 && pos.y < 197) {
						for (int d = -3; d <= 3; d++) {
							overlayComposite.setPixel(pos.x + d, pos.y, 0x04);
							overlayComposite.setPixel(pos.x, pos.y + d, 0x04);
						}
					}
				}
			}
			surface = &overlayComposite;
		} else if (selectedTab == 3) {
			surface = &g_engine->_hotspotMap;
		} else if (selectedTab == 6) {
			surface = &g_engine->_shadowMap;
		} else if (selectedTab == 4) {
			// Object Map: use background image
			surface = &g_engine->_sceneBackground;
		} else if (selectedTab == 5) {
			// Hotspot Overlay: hotspot map with character positions and override info
			static Graphics::ManagedSurface hotspotOverlay;
			hotspotOverlay.copyFrom(g_engine->_hotspotMap);
			View1 *view = (View1 *)g_engine->findView("View1");
			if (view) {
				for (uint i = 0; i < view->_characters.size(); i++) {
					Character *c = view->_characters[i];
					if (!c || !c->_gameObject)
						continue;
					Common::Point pos = c->getPosition();
					if (pos.x >= 3 && pos.x < 317 && pos.y >= 3 && pos.y < 197) {
						for (int d = -3; d <= 3; d++) {
							hotspotOverlay.setPixel(pos.x + d, pos.y, 0x04);
							hotspotOverlay.setPixel(pos.x, pos.y + d, 0x04);
						}
					}
				}
			}
			surface = &hotspotOverlay;
		}

		if (surface && surface->w > 0 && surface->h > 0) {
			ImTextureID texId = (ImTextureID)(intptr_t)g_system->getImGuiTexture(*surface->surfacePtr(), g_engine->_pal, 256);
			if (texId) {
				ImVec2 avail = ImGui::GetContentRegionAvail();
				float scale = MIN(avail.x / 320.0f, avail.y / 200.0f);
				ImGui::Image(texId, ImVec2(320.0f * scale, 200.0f * scale));

				// Draw node IDs on the pathfinding overlay
				if (selectedTab == 2) {
					ImDrawList *dl = ImGui::GetWindowDrawList();
					ImVec2 imgOrigin = ImGui::GetItemRectMin();
					for (int i = 0; i < 16; i++) {
						PathfindingPoint &pt = g_engine->pathfindingPoints[i];
						if (pt._position.x >= 0 && pt._position.x < 320 && pt._position.y >= 0 && pt._position.y < 200) {
							char buf[4];
							snprintf(buf, sizeof(buf), "%d", i);
							ImVec2 pos(imgOrigin.x + pt._position.x * scale + 4, imgOrigin.y + pt._position.y * scale - 4);
							dl->AddText(pos, IM_COL32(255, 255, 0, 255), buf);
						}
					}
				}

				// Draw object positions and clickable IDs on Object Map
				if (selectedTab == 4) {
					ImDrawList *dl = ImGui::GetWindowDrawList();
					ImVec2 imgOrigin = ImGui::GetItemRectMin();
					static int _selectedObjectIdx = -1;
					uint16 sceneIdx = (uint16)Scenes::instance()._currentSceneIndex;

					// Draw all scene objects from GameObjects
					int objCount = 0;
					for (auto obj : GameObjects::instance()._objects) {
						if (obj == nullptr || obj->_sceneIndex != sceneIdx)
							continue;
						Common::Point pos = obj->_position;
						float sx = pos.x * scale;
						float sy = pos.y * scale;
						ImVec2 center(imgOrigin.x + sx, imgOrigin.y + sy);
						uint32 col;
						if (obj->_hasScaling)
							col = IM_COL32(0, 255, 0, 255);
						else if (obj->_hasShading)
							col = IM_COL32(255, 255, 0, 255);
						else
							col = IM_COL32(200, 200, 200, 200);
						dl->AddCircleFilled(center, MAX(4.0f * scale, 3.0f), col);
						char buf[8];
						snprintf(buf, sizeof(buf), "%x", obj->_index);
						ImVec2 textPos(center.x + 5, center.y - 6);
						dl->AddText(textPos, IM_COL32(255, 255, 255, 255), buf);
						objCount++;
					}
					// Handle clicks on objects
					if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
						ImVec2 mousePos2 = ImGui::GetMousePos();
						int clickIdx = 0;
						for (auto obj : GameObjects::instance()._objects) {
							if (obj == nullptr || obj->_sceneIndex != sceneIdx) {
								clickIdx++;
								continue;
							}
							float sx = imgOrigin.x + obj->_position.x * scale;
							float sy = imgOrigin.y + obj->_position.y * scale;
							if (fabs(mousePos2.x - sx) < 8 && fabs(mousePos2.y - sy) < 8) {
								_selectedObjectIdx = clickIdx;
								break;
							}
							clickIdx++;
						}
					}
					// Show selected object details
					if (_selectedObjectIdx >= 0) {
						int idx = 0;
						for (auto obj : GameObjects::instance()._objects) {
							if (obj == nullptr) {
								idx++;
								continue;
							}
							if (idx == _selectedObjectIdx) {
								ImGui::Separator();
								ImGui::Text("Object 0x%x  Scene:%u  Pos:(%d,%d)  Orient:%u",
											obj->_index, obj->_sceneIndex, obj->_position.x, obj->_position.y, obj->_orientation);
								ImGui::Text("HasShading:%s  HasScaling:%s",
											obj->_hasShading ? "Y" : "N", obj->_hasScaling ? "Y" : "N");
								ImGui::Text("Blobs: %u  Script: %u bytes",
											(uint)obj->_blobs.size(), (uint)obj->_script.size());
								break;
							}
							idx++;
						}
					}
					ImGui::Text("Objects in scene: %d", objCount);
				}

				// Show value at mouse hover
				ImVec2 imgPos = ImGui::GetItemRectMin();
				ImVec2 mousePos = ImGui::GetMousePos();
				int mx = (int)((mousePos.x - imgPos.x) / scale);
				int my = (int)((mousePos.y - imgPos.y) / scale);
				if (mx >= 0 && mx < 320 && my >= 0 && my < 200 && ImGui::IsItemHovered()) {
					uint8 val = surface->getPixel(mx, my);
					if (val >= 0xC8 && val <= 0xEF) {
						uint16 overrideResult;
						bool overrideActive = g_engine->getPathfindingOverride(val, overrideResult);
						if (overrideActive)
							ImGui::SetTooltip("(%d, %d) = %u (0x%02X) [override zone → %u = %s]",
											  mx, my, val, val, overrideResult,
											  overrideResult < 0xC8 ? "WALKABLE" : "non-walkable");
						else
							ImGui::SetTooltip("(%d, %d) = %u (0x%02X) [override zone, DISABLED → non-walkable]",
											  mx, my, val, val);
					} else if (val >= 0xF0) {
						ImGui::SetTooltip("(%d, %d) = %u (0x%02X) [non-walkable]", mx, my, val, val);
					} else {
						ImGui::SetTooltip("(%d, %d) = %u (0x%02X) [walkable, depth/area=%u]", mx, my, val, val, val);
					}
				}
			}
		}

		// Hotspot override table for Hotspot Overlay tab
		if (selectedTab == 5) {
			ImGui::Separator();
			ImGui::TextUnformatted("Hotspot Overrides:");
			for (uint i = 0; i < g_engine->_hotspotOverrides.size(); i++) {
				if (g_engine->_hotspotOverrides[i] != 0xFFFF) {
					ImGui::SameLine();
					ImGui::Text("[%u]→0x%x", i, g_engine->_hotspotOverrides[i]);
				}
			}
		}

		// Scene data info
		ImGui::Separator();
		ImGui::Text("_walkDepthThresholdY=%u  _walkDepthScaleFactor=%u  _walkBaseSpeedPct=%u",
					g_engine->_walkDepthThresholdY, g_engine->_walkDepthScaleFactor, g_engine->_walkBaseSpeedPct);
		ImGui::Text("Pathfinding points: %u  Path nodes: %u",
					(uint)g_engine->pathfindingPoints.size(), (uint)g_engine->_path.size());

		// Node detail table
		if (ImGui::CollapsingHeader("Node Graph", ImGuiTreeNodeFlags_DefaultOpen)) {
			View1 *view = (View1 *)g_engine->findView("View1");
			Character *protagonist = view ? view->getCharacterByIndex(Scenes::instance()._currentActorIndex) : nullptr;
			Common::Point charPos = protagonist ? protagonist->getPosition() : Common::Point(0, 0);

			for (int i = 0; i < (int)g_engine->pathfindingPoints.size(); i++) {
				const PathfindingPoint &pt = g_engine->pathfindingPoints[i];
				// Check reachability from character
				bool reachable = protagonist && g_engine->isPathWalkable(charPos.y, charPos.x, pt._position.y, pt._position.x);
				// Check if node is in current path
				bool inPath = false;
				if (protagonist) {
					for (uint p = 0; p < protagonist->_path.size(); p++) {
						if (protagonist->_path[p] == (uint16)i) {
							inPath = true;
							break;
						}
					}
				}

				ImVec4 color = inPath ? ImVec4(0, 1, 0, 1) : reachable ? ImVec4(0.5f, 1, 0.5f, 1)
																	   : ImVec4(0.7f, 0.7f, 0.7f, 1);
				ImGui::TextColored(color, "%2d: (%3d,%3d)", i, pt._position.x, pt._position.y);
				ImGui::SameLine(150);
				// Adjacency
				Common::String adj;
				for (uint j = 0; j < pt._adjacentPoints.size(); j++) {
					if (j > 0)
						adj += ",";
					adj += Common::String::format("%d", pt._adjacentPoints[j] - 1);
				}
				ImGui::TextColored(color, "-> [%s]%s%s",
								   adj.empty() ? "isolated" : adj.c_str(),
								   reachable ? " (R)" : "",
								   inPath ? " *PATH*" : "");
			}
			if (protagonist && !protagonist->_path.empty()) {
				ImGui::Separator();
				ImGui::TextUnformatted("Active path: ");
				ImGui::SameLine();
				for (uint p = 0; p < protagonist->_path.size(); p++) {
					if (p > 0) {
						ImGui::SameLine(0, 2);
						ImGui::TextUnformatted("->");
						ImGui::SameLine(0, 2);
					}
					ImGui::Text("%u", protagonist->_path[p]);
					ImGui::SameLine(0, 0);
				}
				ImGui::NewLine();
				ImGui::Text("PathIdx: %d/%u Dest: (%d,%d)",
							protagonist->_currentPathIndex, (uint)protagonist->_path.size(),
							protagonist->_pathFinalDestination.x, protagonist->_pathFinalDestination.y);
			}
		}
	}
	ImGui::End();
}

static void showImageResourcesWindow() {
	if (!_showImageResources)
		return;
	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Image Resources", &_showImageResources)) {
		ImGui::Text("Count: %u", (uint)g_engine->_imageResources.size());
		ImGui::Separator();
		static Graphics::ManagedSurface imgSurface;
		if (!g_engine->_imageResources.empty()) {
			// Layout all image resources into a 320-wide surface
			uint16 x = 0, y = 0, maxH = 0, totalH = 0;
			for (const AnimFrame &f : g_engine->_imageResources) {
				if (x + f._width > 320) {
					y += maxH;
					x = 0;
					maxH = 0;
				}
				maxH = MAX(maxH, f._height);
				x += f._width;
			}
			totalH = y + maxH;
			if (totalH == 0)
				totalH = 1;

			if (imgSurface.w != 320 || imgSurface.h != (int)totalH)
				imgSurface.create(320, totalH, Graphics::PixelFormat::createFormatCLUT8());
			imgSurface.fillRect(Common::Rect(320, totalH), 0);

			x = 0;
			y = 0;
			maxH = 0;
			for (const AnimFrame &f : g_engine->_imageResources) {
				if (x + f._width > 320) {
					y += maxH;
					x = 0;
					maxH = 0;
				}
				// Blit raw pixel data
				for (uint16 row = 0; row < f._height && (y + row) < totalH; row++) {
					for (uint16 col = 0; col < f._width; col++) {
						byte pixel = f._data[row * f._width + col];
						if (pixel != 0)
							imgSurface.setPixel(x + col, y + row, pixel);
					}
				}
				maxH = MAX(maxH, f._height);
				x += f._width;
			}

			ImTextureID texId = (ImTextureID)(intptr_t)g_system->getImGuiTexture(*imgSurface.surfacePtr(), g_engine->_pal, 256);
			if (texId) {
				ImVec2 avail = ImGui::GetContentRegionAvail();
				float scale = MIN(avail.x / 320.0f, avail.y / (float)totalH);
				if (scale < 1.0f)
					scale = 1.0f;
				ImGui::Image(texId, ImVec2(320.0f * scale, (float)totalH * scale));
			}
		}
	}
	ImGui::End();
}

static void showDebugOutputWindow() {
	if (!_showDebugOutput)
		return;
	ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Debug Output", &_showDebugOutput)) {
		Common::Point mousePos = g_system->getEventManager()->getMousePos();
		ImGui::Text("Mouse: %u %u", mousePos.x, mousePos.y);
		View1 *view = (View1 *)g_engine->findView("View1");
		if (view) {
			ImGui::Text("Hover Area: 0x%04x  Hotspot: 0x%04x", view->_hoverAreaId, view->_hoverHotspotId);
			ImGui::Text("Scaling: characterY=%u factor=%u", view->_scalingValues.characterY, view->_scalingValues.scalingFactor);
		}
		ImGui::Separator();
		for (const Common::String &line : g_engine->_debugOutput) {
			ImGui::TextUnformatted(line.c_str());
		}
	}
	ImGui::End();
}

static bool _showTextLog = false;
static void showTextLogWindow() {
	if (!_showTextLog)
		return;
	ImGui::SetNextWindowSize(ImVec2(500, 200), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Text Log", &_showTextLog)) {
		if (ImGui::BeginChild("TextLogScroll", ImVec2(0, 0), ImGuiChildFlags_Borders)) {
			for (const Common::String &line : g_engine->_textLog)
				ImGui::TextUnformatted(line.c_str());
		}
		ImGui::EndChild();
		g_engine->_textLog.clear();
	}
	ImGui::End();
}

static void showObjectScriptsWindow() {
	if (!_showObjectScripts)
		return;
	ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Object Scripts", &_showObjectScripts)) {
		uint16 sceneIdx = (uint16)Scenes::instance()._currentSceneIndex;
		Script::ScriptExecutor *exec = g_engine->_scriptExecutor;

		static int selectedObj = -1;
		static Common::Array<DecompiledLine> objDecompile;
		static uint16 lastSelectedObjIndex = 0xFFFF;

		if (ImGui::BeginChild("ObjList", ImVec2(200, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX)) {
			// Scene objects
			if (ImGui::TreeNodeEx("Scene Objects", ImGuiTreeNodeFlags_DefaultOpen)) {
				int idx = 0;
				for (auto obj : GameObjects::instance()._objects) {
					if (obj == nullptr || obj->_script.empty() || obj->_sceneIndex != sceneIdx) {
						idx++;
						continue;
					}
					bool isExecuting = exec->isExecuting() && exec->getExecutingObjectId() == obj->_index;
					Common::String label = Common::String::format("%s0x%x (%u bytes)###obj%d",
																  isExecuting ? "> " : "", obj->_index, (uint)obj->_script.size(), idx);
					if (isExecuting)
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
					if (ImGui::Selectable(label.c_str(), selectedObj == idx))
						selectedObj = idx;
					if (isExecuting)
						ImGui::PopStyleColor();
					idx++;
				}
				ImGui::TreePop();
			}
			// Inventory/global objects (scene == 0)
			if (ImGui::TreeNodeEx("Inventory/Global", ImGuiTreeNodeFlags_None)) {
				int idx = 0;
				for (auto obj : GameObjects::instance()._objects) {
					if (obj == nullptr || obj->_script.empty() || obj->_sceneIndex != 0) {
						idx++;
						continue;
					}
					bool isExecuting = exec->isExecuting() && exec->getExecutingObjectId() == obj->_index;
					Common::String label = Common::String::format("%s0x%x (%u bytes)###obj%d",
																  isExecuting ? "> " : "", obj->_index, (uint)obj->_script.size(), idx);
					if (isExecuting)
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
					if (ImGui::Selectable(label.c_str(), selectedObj == idx))
						selectedObj = idx;
					if (isExecuting)
						ImGui::PopStyleColor();
					idx++;
				}
				ImGui::TreePop();
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();

		if (ImGui::BeginChild("ObjScript", ImVec2(0, 0), ImGuiChildFlags_Borders)) {
			GameObject *selObj = nullptr;
			if (selectedObj >= 0 && selectedObj < (int)GameObjects::instance()._objects.size())
				selObj = GameObjects::instance()._objects[selectedObj];

			if (selObj && !selObj->_script.empty()) {
				if (selObj->_index != lastSelectedObjIndex) {
					Common::MemoryReadStream *s = selObj->getScriptStream();
					_decompileStringStream = GameObjects::readGameObjectStrings(selObj->_index, g_engine->_fileStream);
					objDecompile = decompileScript(s);
					delete _decompileStringStream;
					_decompileStringStream = nullptr;
					delete s;
					lastSelectedObjIndex = selObj->_index;
				}

				if (ImGui::Button("Copy")) {
					Common::String full;
					for (uint i = 0; i < objDecompile.size(); i++) {
						const DecompiledLine &l = objDecompile[i];
						for (int j = 0; j < l.indent; j++)
							full += "  ";
						full += Common::String::format("%04x: %s\n", l.offset, l.text.c_str());
					}
					g_system->setTextInClipboard(Common::U32String(full));
				}
				ImGui::SameLine();
				ImGui::Text("Object 0x%x%s | %u bytes | %u opcodes",
							selObj->_index,
							selObj->_sceneIndex == 0 ? " [inventory]" : "",
							(uint)selObj->_script.size(), (uint)objDecompile.size());
				ImGui::Separator();

				uint32 currentPos = 0xFFFFFFFF;
				if (exec->isExecuting() && exec->getExecutingObjectId() == selObj->_index)
					currentPos = exec->getDebugOpcodePosition();

				for (uint i = 0; i < objDecompile.size(); i++) {
					const DecompiledLine &l = objDecompile[i];
					uint32 nextOff = (i + 1 < objDecompile.size()) ? objDecompile[i + 1].offset : (uint32)selObj->_script.size();
					bool isCurrent = (currentPos >= l.offset && currentPos < nextOff);

					if (isCurrent)
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));

					Common::String ind;
					for (int j = 0; j < l.indent; j++)
						ind += "  ";
					ImGui::Text("%04x: %s%s", l.offset, ind.c_str(), l.text.c_str());

					if (isCurrent) {
						ImGui::PopStyleColor();
						ImGui::SetScrollHereY(0.5f);
					}
				}
			} else {
				ImGui::TextUnformatted("Select an object with a script from the list.");
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

static void showSoundWindow();
static void showDebugToolbarWindow();

bool shouldDrawPathfindingOverlay() {
	return _showPathfindingOverlay;
}

bool shouldDrawDebugOutputOverlay() {
	return _showDebugOutputOverlay;
}

bool shouldDrawPathOverlay() {
	return _showPathOverlay;
}

void onImGuiInit() {
	ImGui::GetIO().Fonts->AddFontDefault();
}

void onImGuiRender() {
	if (!debugChannelSet(-1, kDebugImGui)) {
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse;
		return;
	}
	ImGui::GetIO().ConfigFlags &= ~(ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse);

	showDebugToolbarWindow();
	showScriptWindow();
	showObjectScriptsWindow();
	showBreakpointsWindow();
	showVariablesWindow();
	showCharactersWindow();
	drawHoveredObjectOverlay();
	showAnimViewerWindow();
	showInventoryWindow();
	showAnimationsWindow();
	showSceneMapsWindow();
	showImageResourcesWindow();
	showDebugOutputWindow();
	showTextLogWindow();
	showSoundWindow();
}

static void showDebugToolbarWindow() {
	// Movable toolbar window instead of BeginMainMenuBar(), which reserves the full
	// viewport width and intercepts clicks along the top edge of the game view.
	ImGui::SetNextWindowPos(ImVec2(8.0f, 8.0f), ImGuiCond_FirstUseEver);
	const ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;
	if (!ImGui::Begin("MACS2 Debug", nullptr, flags)) {
		ImGui::End();
		return;
	}

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Debug")) {
			ImGui::MenuItem("Script", NULL, &_showScript);
			ImGui::MenuItem("Object Scripts", NULL, &_showObjectScripts);
			ImGui::MenuItem("Variables", NULL, &_showVariables);
			ImGui::MenuItem("Breakpoints", NULL, &_showBreakpoints);
			ImGui::MenuItem("Scene Objects", NULL, &_showCharacters);
			ImGui::MenuItem("Inventory", NULL, &_showInventory);
			ImGui::MenuItem("Animations", NULL, &_showAnimations);
			ImGui::MenuItem("Scene Maps", NULL, &_showSceneMaps);
			ImGui::MenuItem("Image Resources", NULL, &_showImageResources);
			ImGui::MenuItem("Debug Output", NULL, &_showDebugOutput);
			ImGui::MenuItem("Pathfinding Overlay", NULL, &_showPathfindingOverlay);
			ImGui::MenuItem("Debug Output Overlay", NULL, &_showDebugOutputOverlay);
			ImGui::MenuItem("Path Overlay", NULL, &_showPathOverlay);
			ImGui::MenuItem("Sound/Music", NULL, &_showSound);
			ImGui::MenuItem("Text Log", NULL, &_showTextLog);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Log Channels")) {
			static const struct {
				int flag;
				const char *name;
			} channels[] = {
				{kDebugGraphics, "Graphics"},
				{kDebugPath, "Path"},
				{kDebugScan, "Scan"},
				{kDebugFilePath, "FilePath"},
				{kDebugInput, "Input"},
				{kDebugImGui, "ImGui"},
				{kDebugScript, "Script"},
			};
			for (const auto &ch : channels) {
				bool enabled = debugChannelSet(-1, ch.flag);
				if (ImGui::MenuItem(ch.name, NULL, enabled)) {
					if (enabled) {
						DebugMan.disableDebugChannel(ch.flag);
					} else {
						DebugMan.enableDebugChannel(ch.flag);
					}
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Speed")) {
			if (ImGui::MenuItem("Normal", NULL, g_engine->_gameSpeedMode == 0)) {
				g_engine->_gameSpeedMode = 0;
			}
			if (ImGui::MenuItem("Fast", NULL, g_engine->_gameSpeedMode == 1)) {
				g_engine->_gameSpeedMode = 1;
			}
			if (ImGui::MenuItem("Slow", NULL, g_engine->_gameSpeedMode == 2)) {
				g_engine->_gameSpeedMode = 2;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Tools")) {
			View1 *toolsView = (View1 *)g_engine->findView("View1");
			if (toolsView) {
				ImGui::MenuItem("Auto-click (simulate repeated clicks)", NULL, &toolsView->_autoclickActive);
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Quick Start (Scene 6)")) {
				g_engine->changeScene(0x6);
			}
			ImGui::Separator();
			static int sceneInput = 1;
			ImGui::SetNextItemWidth(60);
			ImGui::InputInt("##scn", &sceneInput, 1, 10);
			ImGui::SameLine();
			if (ImGui::MenuItem("Change Scene")) {
				if (sceneInput > 0 && sceneInput <= 512) {
					g_engine->changeScene((uint32)sceneInput);
				}
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Run Script Executor")) {
				g_engine->runScriptExecutor(true);
			}
			static int dosSaveSlot = 0;
			ImGui::SetNextItemWidth(60);
			ImGui::InputInt("##dosslot", &dosSaveSlot, 1, 1);
			if (dosSaveSlot < 0) {
				dosSaveSlot = 0;
			}
			if (dosSaveSlot > 9) {
				dosSaveSlot = 9;
			}
			ImGui::SameLine();
			if (ImGui::MenuItem("Save to original DOS slot (SAVEGAME.N)")) {
				Common::Error err = g_engine->saveOriginalGameState(dosSaveSlot);
				if (err.getCode() != Common::kNoError) {
					warning("Failed to write original DOS save SAVEGAME.%d", dosSaveSlot);
				} else {
					debug("Wrote original DOS save SAVEGAME.%d", dosSaveSlot);
				}
			}
			static int dosLoadSlot = 0;
			ImGui::SetNextItemWidth(60);
			ImGui::InputInt("##dosloadslot", &dosLoadSlot, 1, 1);
			if (dosLoadSlot < 0) {
				dosLoadSlot = 0;
			}
			if (dosLoadSlot > 9) {
				dosLoadSlot = 9;
			}
			ImGui::SameLine();
			if (ImGui::MenuItem("Load original DOS slot (SAVEGAME.N)")) {
				Common::Error err = g_engine->loadGameState(100 + dosLoadSlot);
				if (err.getCode() != Common::kNoError) {
					warning("Failed to load original DOS save SAVEGAME.%d", dosLoadSlot);
				} else {
					debug("Loaded original DOS save SAVEGAME.%d", dosLoadSlot);
				}
			}
			if (ImGui::MenuItem("Reset Background + Fade")) {
				View1 *view = (View1 *)g_engine->findView("View1");
				if (view) {
					view->_backgroundSurface.copyFrom(g_engine->_sceneBackground);
					view->startFading();
					view->redraw();
				}
			}
			if (ImGui::MenuItem("Transfer Held Item (Drop)")) {
				View1 *view = (View1 *)g_engine->findView("View1");
				if (view && view->_uiPanelState == View1::kUiPanelInventory && view->_activeInventoryItem != nullptr) {
					if (view->isInventorySourceProtagonist()) {
						const uint16 currentScene = Scenes::instance()._currentSceneIndex;
						for (GameObject *obj : GameObjects::instance()._objects) {
							if (obj == nullptr) {
								continue;
							}
							if ((int16)obj->_sceneIndex < 0 || obj->_sceneIndex != currentScene) {
								continue;
							}
							if (0x13 >= obj->_blobs.size() || obj->_blobs[0x13].empty()) {
								continue;
							}
							view->transferInventoryItem(view->_activeInventoryItem, obj);
							view->_activeInventoryItem = nullptr;
							view->setInventorySource(view->_inventorySource);
							break;
						}
					} else {
						view->transferInventoryItem(view->_activeInventoryItem, GameObjects::instance().getProtagonistObject());
						view->_activeInventoryItem = nullptr;
						view->setInventorySource(view->_inventorySource);
					}
				}
			}
			ImGui::EndMenu();
		}
		ImGui::Separator();
		ImGui::Text("Scene: %d | Speed: %s", Scenes::instance()._currentSceneIndex,
					g_engine->_gameSpeedMode == 0 ? "Normal" : (g_engine->_gameSpeedMode == 1 ? "Fast" : "Slow"));
		ImGui::EndMenuBar();
	}

	ImGui::End();
}

static void showSoundWindow() {
	if (!_showSound)
		return;
	if (!ImGui::Begin("Sound/Music", &_showSound)) {
		ImGui::End();
		return;
	}

	Music *adlib = g_engine->getAdlib();
	const Music::DebugState &ds = adlib->_debug;

	ImGui::Text("Master Volume: %u/63", ds.masterVolume);
	ImGui::Text("OPL Channels: %u  Status: 0x%02X  NextEvent: %u", ds.numOplChannels, ds.statusFlags, ds.nextEventTimer);
	ImGui::Separator();

	// Channel selector
	static int selectedVoice = 0;
	if (ImGui::BeginCombo("Voice", Common::String::format("Voice %d", selectedVoice).c_str())) {
		for (int i = 0; i < 9; i++) {
			bool selected = (selectedVoice == i);
			const char *label = ds.voices[i].active
									? Common::String::format("Voice %d [CH%d N%02X]", i, ds.voices[i].channel, ds.voices[i].note).c_str()
									: Common::String::format("Voice %d (idle)", i).c_str();
			if (ImGui::Selectable(label, selected))
				selectedVoice = i;
			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	// Voice detail
	const auto &v = ds.voices[selectedVoice];
	ImGui::Text("Note: 0x%02X  MIDI Ch: %u  Vol: %u  Active: %s",
				v.note, v.channel, v.volume, v.active ? "YES" : "no");

	// Ring buffer waveform for selected voice
	ImGui::TextUnformatted("Activity:");
	int ringPos = ds.ringPos;
	float plotData[Music::kDebugRingSize];
	for (int i = 0; i < Music::kDebugRingSize; i++) {
		plotData[i] = ds.regHistory[selectedVoice][(ringPos + i) % Music::kDebugRingSize];
	}
	ImGui::PlotLines("##wave", plotData, Music::kDebugRingSize, 0, nullptr, 0.0f, 1.0f, ImVec2(0, 60));

	// All voices overview
	ImGui::Separator();
	ImGui::TextUnformatted("All Voices:");
	for (int i = 0; i < 9; i++) {
		float voiceData[Music::kDebugRingSize];
		for (int j = 0; j < Music::kDebugRingSize; j++) {
			voiceData[j] = ds.regHistory[i][(ringPos + j) % Music::kDebugRingSize];
		}
		ImGui::PlotLines(Common::String::format("V%d", i).c_str(), voiceData, Music::kDebugRingSize, 0, nullptr, 0.0f, 1.0f, ImVec2(0, 20));
	}

	ImGui::End();
}

void onImGuiCleanup() {
}

} // namespace Macs2
