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

#ifndef DIRECTOR_DEBUGER_DT_INTERNAL_H
#define DIRECTOR_DEBUGER_DT_INTERNAL_H

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "graphics/surface.h"

#include "backends/imgui/imgui.h"
#include "backends/imgui/imgui_fonts.h"
#include "backends/imgui/components/imgui_logger.h"

#include "director/debugger/imgui_memory_editor.h"

#include "director/types.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingodec/ast.h"
#include "director/lingo/lingodec/handler.h"

namespace Director {
namespace DT {

#define kMaxColumnsInTable 512

typedef struct ImGuiImage {
	ImTextureID id;
	int16 width;
	int16 height;
} ImGuiImage;

typedef struct ImGuiScriptCodeLine {
	uint32 pc;
	Common::String codeLine;
} ImGuiScriptCodeLine;

typedef struct ImGuiScript {
	bool score = false;
	CastMemberID id;
	ScriptType type;
	Common::String handlerId;
	Common::String handlerName;
	Common::String moviePath;
	Common::Array<uint32> byteOffsets;

	bool isMethod = false;
	bool isGenericEvent = false;
	Common::StringArray argumentNames;
	Common::StringArray propertyNames;
	Common::StringArray globalNames;
	Common::SharedPtr<LingoDec::HandlerNode> root;
	Common::Array<LingoDec::Bytecode> bytecodeArray;
	Common::Array<uint> startOffsets;
	Common::SharedPtr<Node> oldAst;

	bool operator==(const ImGuiScript &c) const {
		return moviePath == c.moviePath && score == c.score && id == c.id && handlerId == c.handlerId;
	}
	bool operator!=(const ImGuiScript &c) const {
		return !(*this == c);
	}
} ImGuiScript;

typedef struct ImGuiWindows {
	bool controlPanel = true;
	bool callStack = false;
	bool vars = false;
	bool channels = false;
	bool cast = false;
	bool funcList = false;
	bool score = false;
	bool bpList = false;
	bool settings = false;
	bool logger = false;
	bool archive = false;
	bool watchedVars = false;
} ImGuiWindows;

typedef struct ImGuiState {
	struct {
		Common::HashMap<Graphics::Surface *, ImGuiImage> _textures;
		bool _listView = true;
		int _thumbnailSize = 64;
		ImGuiTextFilter _nameFilter;
		int _typeFilter = 0x7FFF;
	} _cast;
	struct {
		Common::Array<ImGuiScript> _scripts;
		uint _current = 0;
		ImGuiTextFilter _nameFilter;
		bool _showByteCode = false;
		bool _showScript = false;
	} _functions;
	struct {
		uint _lastLinePC = 0;
		uint _callstackSize = 0;
		bool _isScriptDirty = false; // indicates whether or not we have to display the script corresponding to the current stackframe
	} _dbg;

	struct {
		ImVec4 _bp_color_disabled = ImVec4(0.9f, 0.08f, 0.0f, 0.0f);
		ImVec4 _bp_color_enabled = ImVec4(0.9f, 0.08f, 0.0f, 1.0f);
		ImVec4 _bp_color_hover = ImVec4(0.42f, 0.17f, 0.13f, 1.0f);

		ImVec4 _current_statement = ImColor(IM_COL32(0xFF, 0xFF, 0x00, 0xFF));
		ImVec4 _line_color = ImVec4(0.44f, 0.44f, 0.44f, 1.0f);
		ImVec4 _call_color = ImColor(IM_COL32(0xFF, 0xC5, 0x5C, 0xFF));
		ImVec4 _builtin_color = ImColor(IM_COL32(0x60, 0x7C, 0xFF, 0xFF));
		ImVec4 _var_color = ImColor(IM_COL32(0x4B, 0xCD, 0x5E, 0xFF));
		ImVec4 _literal_color = ImColor(IM_COL32(0xFF, 0x9F, 0xDA, 0x9E));
		ImVec4 _comment_color = ImColor(IM_COL32(0xFF, 0xA5, 0x9D, 0x95));
		ImVec4 _type_color = ImColor(IM_COL32(0x13, 0xC5, 0xF9, 0xFF));
		ImVec4 _keyword_color = ImColor(IM_COL32(0xC1, 0xC1, 0xC1, 0xFF));
		ImVec4 _the_color = ImColor(IM_COL32(0xFF, 0x49, 0xEF, 0xFF));

		ImVec4 _script_ref = ImColor(IM_COL32(0x7f, 0x7f, 0xff, 0xfff));
		ImVec4 _var_ref = ImColor(IM_COL32(0xe6, 0xe6, 0x00, 0xff));
		ImVec4 _var_ref_changed = ImColor(IM_COL32(0xFF, 0x00, 0x00, 0xFF));
	} _colors;

	struct {
		DatumHash _locals;
		DatumHash _globals;

		DatumHash _prevLocals;
		DatumHash _prevGlobals;

		uint32 _lastTimeRefreshed = 0;
	} _vars;

	ImGuiWindows _w;
	ImGuiWindows _savedW;
	bool _wasHidden = false;

	Common::List<CastMemberID> _scriptCasts;
	Common::HashMap<Common::String, bool, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _variables;
	int _prevFrame = -1;
	struct {
		int frame = -1;
		int channel = -1;
	} _selectedScoreCast;

	int _scoreMode = 0;
	int _scoreFrameOffset = 1;

	ImFont *_tinyFont = nullptr;

	struct {
		Common::Path path;
		uint32 resType = 0;
		uint32 resId = 0;

		byte *data = nullptr;
		uint32 dataSize = 0;

		MemoryEditor memEdit;
	} _archive;

	ImGuiEx::ImGuiLogger *_logger = nullptr;
} ImGuiState;

// debugtools.cpp
ImGuiScript toImGuiScript(ScriptType scriptType, CastMemberID id, const Common::String &handlerId);
void setScriptToDisplay(const ImGuiScript &script);
Director::Breakpoint *getBreakpoint(const Common::String &handlerName, uint16 scriptId, uint pc);
void displayScriptRef(CastMemberID &scriptId);
ImGuiImage getImageID(CastMember *castMember);
Common::String getDisplayName(CastMember *castMember);
void showImage(const ImGuiImage &image, const char *name, float thumbnailSize);
ImVec4 convertColor(uint32 color);
void displayVariable(const Common::String &name, bool changed);

void showCast();        // dt-cast.cpp
void showControlPanel(); // dt-controlpanel.cpp

// dt-lists.cpp
void showCallStack();
void showVars();
void showWatchedVars();
void showBreakpointList();
void showArchive();

// dt-score.cpp
void showScore();
void showChannels();

void renderOldScriptAST(ImGuiScript &script, bool showByteCode);    // dt-script-d2.cpp
void renderScriptAST(ImGuiScript &script, bool showByteCode);       // dt-script-d4.cpp

// dt-scripts.cpp
void showFuncList();
void showScriptCasts();
void showScripts();

extern ImGuiState *_state;

}

}

#endif
