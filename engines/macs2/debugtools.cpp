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
#include "common/system.h"
#include "macs2/detection.h"
#include "macs2/gameobjects.h"
#include "macs2/macs2.h"
#include "macs2/script/scriptexecutor.h"
#include "macs2/view1.h"

namespace Macs2 {

static bool _showScript = false;
static bool _showVariables = false;
static bool _showCharacters = false;
static bool _showInventory = false;
static bool _showAnimations = false;
static bool _showSceneMaps = false;

static const char *getOpcodeName(uint8 opcode) {
	switch (opcode) {
	case 0x01:
		return "setVar";
	case 0x02:
		return "setVarOr";
	case 0x03:
		return "ifTrue";
	case 0x04:
		return "ifFalse";
	case 0x05:
		return "compare";
	case 0x06:
		return "ifInteraction";
	case 0x07:
		return "endIf";
	case 0x08:
		return "else";
	case 0x0A:
		return "printString";
	case 0x0B:
		return "moveObject";
	case 0x0C:
		return "changeScene";
	case 0x0D:
		return "showDialogue";
	case 0x0E:
		return "changeAnim";
	case 0x0F:
		return "frameWait";
	case 0x10:
		return "walkTo";
	case 0x11:
		return "waitForWalk";
	case 0x12:
		return "setPathOverride";
	case 0x13:
		return "loadAnim";
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
		return "setAnimSpeed";
	case 0x1C:
		return "setSkippable";
	case 0x1D:
		return "clearSkippable";
	case 0x1E:
		return "loadAnimBlob";
	case 0x1F:
		return "setPosition";
	case 0x20:
		return "setVerticalOffset";
	case 0x21:
		return "setMotion";
	case 0x22:
		return "setAnimIndex";
	case 0x23:
		return "moveToPosition";
	case 0x24:
		return "add";
	case 0x25:
		return "subtract";
	case 0x26:
		return "loadSpecialAnim";
	case 0x27:
		return "setMaxAnimFrame";
	case 0x28:
		return "nop28";
	case 0x29:
		return "loadSong";
	case 0x2A:
		return "loadAnimFromScene";
	case 0x2B:
		return "setShading";
	case 0x2C:
		return "setParent";
	case 0x2D:
		return "setScaling";
	case 0x2E:
		return "checkAnimRange";
	case 0x2F:
		return "checkBlobRange";
	case 0x30:
		return "nop30";
	case 0x31:
		return "setVolume";
	case 0x32:
		return "setClickable";
	case 0x33:
		return "setVisible";
	case 0x34:
		return "setHotspotRemap";
	case 0x35:
		return "setBoundsAttach";
	case 0x36:
		return "dismissAllPanels";
	case 0x37:
		return "resetScript";
	case 0x38:
		return "loadOverlayFont";
	case 0x39:
		return "endOverlayText";
	case 0x3A:
		return "addOverlayEntry";
	case 0x3B:
		return "clearOverlayEntries";
	case 0x3C:
		return "fadeToBlack";
	case 0x3D:
		return "fadeFromBlack";
	case 0x3E:
		return "loadSoundRes";
	case 0x3F:
		return "clearSound";
	case 0x40:
		return "playSound";
	case 0x41:
		return "waitForSound";
	case 0x42:
		return "stopSound";
	case 0x43:
		return "loadMusicSlot";
	case 0x44:
		return "playMusic";
	case 0x45:
		return "stopMusic";
	case 0x46:
		return "freeMusic";
	case 0x47:
		return "waitForMusic";
	case 0x48:
		return "getObjectX";
	case 0x49:
		return "getObjectY";
	case 0x4A:
		return "getObjectField";
	case 0x4B:
		return "getObjectOrient";
	case 0x4C:
		return "clearActorItems";
	case 0x4D:
		return "setAreaRemap";
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
	if (type == 0x00)
		return Common::String::format("%u", val);
	if (type == 0xFF) {
		switch (val) {
		case 0x01:
			return "interactedUse";
		case 0x02:
			return "interactedLook";
		case 0x03:
			return "interactedTalk";
		case 0x04:
			return "areaAtActor";
		case 0x0B:
			return "isRepeatRun";
		case 0x0D:
			return "dialogueResult";
		case 0x23:
			return "pathWalkable";
		case 0x24:
			return "actorX";
		case 0x25:
			return "actorY";
		case 0x26:
			return "isSceneInit";
		case 0x28:
			return "invCheck";
		case 0x2A:
			return "invCombine";
		case 0x2B:
			return "invAction";
		case 0x2D:
			return "curScene";
		case 0x2F:
			return "prevScene";
		default:
			return Common::String::format("special[0x%02x]", val);
		}
	}
	return Common::String::format("var[%u]", val);
}

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
	case 0x0A: {
		if (length >= 10) {
			script->seek(dataStart + 6);
			uint16 o = script->readUint16LE(), n = script->readUint16LE();
			if (n > 0 && n < 50 && Scenes::instance()._currentSceneStrings) {
				auto ss = g_engine->DecodeStrings(Scenes::instance()._currentSceneStrings, o, n);
				if (!ss.empty())
					result = Common::String::format(" \"%s\"", ss[0].c_str());
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
			if (n > 0 && n < 50 && Scenes::instance()._currentSceneStrings) {
				auto ss = g_engine->DecodeStrings(Scenes::instance()._currentSceneStrings, off, n);
				if (!ss.empty()) {
					result = Common::String::format(" obj=%s \"%s\"", o.c_str(), ss[0].c_str());
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
	case 0x48:
	case 0x49:
	case 0x4A:
	case 0x4B: {
		Common::String o = decodeScriptValue(script);
		result = Common::String::format(" obj=%s", o.c_str());
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
			if (n > 0 && n < 50 && Scenes::instance()._currentSceneStrings) {
				auto ss = g_engine->DecodeStrings(Scenes::instance()._currentSceneStrings, off, n);
				if (!ss.empty()) {
					result = Common::String::format(" idx=%s \"%s\"", idx.c_str(), ss[0].c_str());
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
		Common::String a = decodeScriptValue(script), b = decodeScriptValue(script);
		result = Common::String::format(" %s, %s", a.c_str(), b.c_str());
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
	case 0x30:
	case 0x36:
	case 0x37:
	case 0x39:
	case 0x3B:
	case 0x3F:
	case 0x40:
	case 0x41:
	case 0x42:
	case 0x47:
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

static Common::Array<DecompiledLine> _cachedDecompile;
static int _cachedSceneIndex = -1;

static void showScriptWindow() {
	if (!_showScript)
		return;
	ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Scene Script", &_showScript)) {
		Script::ScriptExecutor *exec = g_engine->_scriptExecutor;
		int currentScene = Scenes::instance()._currentSceneIndex;
		ImGui::Text("Scene: %d | Pos: %u/%u | %s | Obj: 0x%x",
					currentScene, exec->getScriptPosition(), exec->getScriptEndPosition(),
					exec->isExecuting() ? "RUNNING" : "Idle", exec->getExecutingObjectId());
		ImGui::Separator();

		Common::MemoryReadStream *script = Scenes::instance()._currentSceneScript;
		if (script) {
			if (_cachedSceneIndex != currentScene) {
				_cachedDecompile = decompileScript(script);
				_cachedSceneIndex = currentScene;
			}
			uint32 currentPos = exec->getScriptPosition();
			if (ImGui::BeginChild("ScriptView", ImVec2(0, 0), ImGuiChildFlags_Borders)) {
				for (uint i = 0; i < _cachedDecompile.size(); i++) {
					const DecompiledLine &l = _cachedDecompile[i];
					uint32 nextOff = (i + 1 < _cachedDecompile.size()) ? _cachedDecompile[i + 1].offset : exec->getScriptEndPosition();
					bool isCurrent = (currentPos >= l.offset && currentPos < nextOff);
					if (isCurrent)
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
					Common::String ind;
					for (int j = 0; j < l.indent; j++)
						ind += "  ";
					ImGui::Text("%04x: %s%s", l.offset, ind.c_str(), l.text.c_str());
					if (isCurrent) {
						ImGui::PopStyleColor();
						if (exec->isExecuting())
							ImGui::SetScrollHereY(0.5f);
					}
				}
			}
			ImGui::EndChild();
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
			ImGui::Text("Mouse Mode: 0x%x", (int)exec->_mouseMode);
			ImGui::Text("Interacted Object: 0x%x", exec->_interactedObjectID);
			ImGui::Text("Interacted Other: 0x%x", exec->_interactedOtherObjectID);
			ImGui::Text("Script Skippable: %s", exec->_scriptSkippable ? "Y" : "N");
			ImGui::Text("Inventory Action/Combine: %s/%s", exec->_inventoryActionFlag ? "Y" : "N", exec->_inventoryCombineFlag ? "Y" : "N");
		}
		if (ImGui::CollapsingHeader("Runtime Specials (FF, read-only)", ImGuiTreeNodeFlags_DefaultOpen)) {
			static const struct {
				uint16 id;
				const char *name;
			} specials[] = {
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
				{0x28, "invCheck"},
				{0x2A, "invCombine"},
				{0x2B, "invAction"},
				{0x2D, "curScene"},
				{0x2F, "prevScene"},
			};
			for (const auto &s : specials) {
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
	}
	ImGui::End();
}

static void showCharactersWindow() {
	if (!_showCharacters)
		return;
	ImGui::SetNextWindowSize(ImVec2(550, 450), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Characters", &_showCharacters)) {
		View1 *view = (View1 *)g_engine->findView("View1");
		if (view) {
			for (uint i = 0; i < view->_characters.size(); i++) {
				Character *c = view->_characters[i];
				if (!c || !c->_gameObject)
					continue;
				Common::Point pos = c->getPosition();
				Common::String header = Common::String::format("[%u] obj=0x%x (%d,%d)###char%u", i, c->_gameObject->_index, pos.x, pos.y, i);
				if (ImGui::CollapsingHeader(header.c_str())) {
					ImGui::Text("Orient: %u  Anim: %u  VOffset: %u",
								c->_gameObject->Orientation, c->_animationIndex, c->getVerticalOffset());
					ImGui::Text("Visible: %s  Clickable: %s  Frozen: %s",
								c->_gameObject->IsVisible ? "Y" : "N",
								c->_gameObject->IsClickable ? "Y" : "N",
								c->_gameObject->HasBoundsAttachment ? "Y" : "N");

					// Pathfinding state
					ImGui::Separator();
					ImGui::Text("Movement:");
					ImGui::Text("  Lerping: %s  DirSet: %s  FollowPath: %s",
								c->isLerping() ? "Y" : "N",
								c->isDirectionSet() ? "Y" : "N",
								c->_isFollowingPath ? "Y" : "N");
					if (c->isLerping()) {
						Common::Point end = c->getEndPosition();
						ImGui::Text("  Target: (%d,%d)  Final: (%d,%d)",
									end.x, end.y, c->_pathFinalDestination.x, c->_pathFinalDestination.y);
						ImGui::Text("  Bresenham: dX=%d dY=%d err=%d",
									c->getStepDeltaX(), c->getStepDeltaY(), c->getStepError());

						// Walk speed calculation
						int depth = 0;
						if (pos.y >= (int)g_engine->word51FD)
							depth = ((int)pos.y - (int)g_engine->word51FD) * (int)g_engine->word51FF / 100;
						uint16 animSpd = 2;
						uint8 orient = c->_gameObject->Orientation;
						if (orient >= 1 && orient <= 0x15 && (uint)(orient - 1) < c->_gameObject->BlobSpeeds.size()) {
							animSpd = c->_gameObject->BlobSpeeds[orient - 1];
							if (animSpd == 0)
								animSpd = 2;
						}
						int spd = (int)animSpd * ((int)g_engine->word5201 + depth) / 100;
						if (spd < 1)
							spd = 1;
						ImGui::Text("  Speed: %d (anim=%u depth=%d w5201=%u)",
									spd, animSpd, depth, g_engine->word5201);
					}
					if (c->_isFollowingPath && !c->_path.empty()) {
						ImGui::Text("  Path[%d/%u]: ", c->_currentPathIndex, (uint)c->_path.size());
						ImGui::SameLine();
						for (uint p = 0; p < c->_path.size(); p++) {
							if (p > 0)
								ImGui::SameLine(0, 2);
							if ((int)p == c->_currentPathIndex)
								ImGui::TextColored(ImVec4(0, 1, 0, 1), "%u", c->_path[p]);
							else
								ImGui::Text("%u", c->_path[p]);
						}
					}

					// Direction availability
					if (c->_gameObject->Blobs.size() >= 8) {
						ImGui::Text("  DirAvail: ");
						ImGui::SameLine();
						const char *dirNames[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
						for (int d = 0; d < 8; d++) {
							if (d > 0)
								ImGui::SameLine(0, 4);
							bool avail = d < (int)c->_gameObject->Blobs.size() && !c->_gameObject->Blobs[d].empty();
							if (avail)
								ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", dirNames[d]);
							else
								ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1), "%s", dirNames[d]);
						}
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
	ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Inventory", &_showInventory)) {
		View1 *view = (View1 *)g_engine->findView("View1");
		if (view) {
			ImGui::Text("Source: obj %u | Active: %s | Showing: %s",
						view->_inventorySource ? view->_inventorySource->_index : 0,
						view->_activeInventoryItem ? Common::String::format("0x%x", view->_activeInventoryItem->_index).c_str() : "None",
						view->_isShowingInventory ? "Y" : "N");
			ImGui::Separator();
			for (uint i = 0; i < view->_inventoryItems.size(); i++) {
				GameObject *obj = view->_inventoryItems[i];
				ImGui::Text("  [%u] obj 0x%x scene=%d", i, obj->_index, obj->SceneIndex);
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
		ImGui::Text("BG Anims: %u", g_engine->_numBackgroundAnimations);
		for (int i = 0; i < g_engine->_numBackgroundAnimations; i++) {
			BackgroundAnimation &a = g_engine->_backgroundAnimations[i];
			ImGui::Text("  [%d] pos=(%u,%u) frames=%u cur=%u", i, a.X, a.Y, a.numFrames, a.FrameIndex);
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
			if (ImGui::BeginTabItem("Pathfinding Overlay")) {
				selectedTab = 2;
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
					if (pt.Position.x >= 0 && pt.Position.x < 320 && pt.Position.y >= 0 && pt.Position.y < 200) {
						// Draw cross at node
						for (int d = -2; d <= 2; d++) {
							int px = pt.Position.x + d, py = pt.Position.y;
							if (px >= 0 && px < 320)
								overlayComposite.setPixel(px, py, 0xFF);
							px = pt.Position.x;
							py = pt.Position.y + d;
							if (py >= 0 && py < 200)
								overlayComposite.setPixel(px, py, 0xFF);
						}
						// Draw connections
						for (uint8 adj : pt.adjacentPoints) {
							if (adj == 0 || adj > 16)
								continue;
							PathfindingPoint &other = g_engine->pathfindingPoints[adj - 1];
							overlayComposite.drawLine(pt.Position.x, pt.Position.y, other.Position.x, other.Position.y, 0xFE);
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
					if (pos.x >= 1 && pos.x < 319 && pos.y >= 1 && pos.y < 199) {
						for (int d = -3; d <= 3; d++) {
							overlayComposite.setPixel(pos.x + d, pos.y, 0x04);
							overlayComposite.setPixel(pos.x, pos.y + d, 0x04);
						}
					}
				}
			}
			surface = &overlayComposite;
		}

		if (surface && surface->w > 0 && surface->h > 0) {
			ImTextureID texId = (ImTextureID)(intptr_t)g_system->getImGuiTexture(*surface->surfacePtr(), g_engine->_pal, 256);
			if (texId) {
				ImVec2 avail = ImGui::GetContentRegionAvail();
				float scale = MIN(avail.x / 320.0f, avail.y / 200.0f);
				ImGui::Image(texId, ImVec2(320.0f * scale, 200.0f * scale));

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

		// Scene data info
		ImGui::Separator();
		ImGui::Text("word51FD=%u  word51FF=%u  word5201=%u",
					g_engine->word51FD, g_engine->word51FF, g_engine->word5201);
		ImGui::Text("Pathfinding points: %u  Path nodes: %u",
					(uint)g_engine->pathfindingPoints.size(), (uint)g_engine->_path.size());

		// Node detail table
		if (ImGui::CollapsingHeader("Node Graph", ImGuiTreeNodeFlags_DefaultOpen)) {
			View1 *view = (View1 *)g_engine->findView("View1");
			Character *protagonist = view ? view->getCharacterByIndex(1) : nullptr;
			Common::Point charPos = protagonist ? protagonist->getPosition() : Common::Point(0, 0);

			for (int i = 0; i < (int)g_engine->pathfindingPoints.size(); i++) {
				const PathfindingPoint &pt = g_engine->pathfindingPoints[i];
				// Check reachability from character
				bool reachable = protagonist && protagonist->isPathWalkable(charPos, pt.Position);
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
				ImGui::TextColored(color, "%2d: (%3d,%3d)", i, pt.Position.x, pt.Position.y);
				ImGui::SameLine(150);
				// Adjacency
				Common::String adj;
				for (uint j = 0; j < pt.adjacentPoints.size(); j++) {
					if (j > 0)
						adj += ",";
					adj += Common::String::format("%d", pt.adjacentPoints[j] - 1);
				}
				ImGui::TextColored(color, "-> [%s]%s%s",
								   adj.empty() ? "isolated" : adj.c_str(),
								   reachable ? " (R)" : "",
								   inPath ? " *PATH*" : "");
			}
			if (protagonist && !protagonist->_path.empty()) {
				ImGui::Separator();
				ImGui::Text("Active path: ");
				ImGui::SameLine();
				for (uint p = 0; p < protagonist->_path.size(); p++) {
					if (p > 0) {
						ImGui::SameLine(0, 2);
						ImGui::Text("->");
						ImGui::SameLine(0, 2);
					}
					ImGui::Text("%u", protagonist->_path[p]);
					ImGui::SameLine(0, 0);
				}
				ImGui::NewLine();
				ImGui::Text("PathIdx: %d/%u  Following: %s  Dest: (%d,%d)",
							protagonist->_currentPathIndex, (uint)protagonist->_path.size(),
							protagonist->_isFollowingPath ? "Y" : "N",
							protagonist->_pathFinalDestination.x, protagonist->_pathFinalDestination.y);
			}
		}
	}
	ImGui::End();
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

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Debug")) {
			ImGui::MenuItem("Script", NULL, &_showScript);
			ImGui::MenuItem("Variables", NULL, &_showVariables);
			ImGui::MenuItem("Characters", NULL, &_showCharacters);
			ImGui::MenuItem("Inventory", NULL, &_showInventory);
			ImGui::MenuItem("Animations", NULL, &_showAnimations);
			ImGui::MenuItem("Scene Maps", NULL, &_showSceneMaps);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Speed")) {
			if (ImGui::MenuItem("Normal", NULL, g_engine->_gameSpeedMode == 0))
				g_engine->_gameSpeedMode = 0;
			if (ImGui::MenuItem("Fast", NULL, g_engine->_gameSpeedMode == 1))
				g_engine->_gameSpeedMode = 1;
			if (ImGui::MenuItem("Slow", NULL, g_engine->_gameSpeedMode == 2))
				g_engine->_gameSpeedMode = 2;
			ImGui::EndMenu();
		}
		ImGui::Text("| Scene: %d | Speed: %s", Scenes::instance()._currentSceneIndex,
					g_engine->_gameSpeedMode == 0 ? "Normal" : (g_engine->_gameSpeedMode == 1 ? "Fast" : "Slow"));
		ImGui::EndMainMenuBar();
	}

	showScriptWindow();
	showVariablesWindow();
	showCharactersWindow();
	showInventoryWindow();
	showAnimationsWindow();
	showSceneMapsWindow();
}

void onImGuiCleanup() {
}

} // namespace Macs2
