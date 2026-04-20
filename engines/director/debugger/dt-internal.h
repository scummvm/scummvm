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
#include "backends/imgui/components/imgui_memory_editor.h"

#include "director/types.h"
#include "director/window.h"
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
	uint pc = 0;

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
	bool vars = false;
	bool channels = false;
	bool cast = false;
	bool castDetails = false;
	bool funcList = false;
	bool score = false;
	bool bpList = false;
	bool settings = false;
	bool logger = false;
	bool archive = false;
	bool watchedVars = false;
	bool executionContext = false;
	bool search = false;
} ImGuiWindows;


enum SearchMode {
	kSearchAll = 0,
	kSearchHandlerNames,
	kSearchScriptBody,
};

typedef struct ScriptData {
	Common::Array<ImGuiScript> _scripts;
	uint _current = 0;
	bool _showByteCode = false;
	bool _showScript = false;
} ScriptData;

typedef struct WindowFlag {
	const char *name;
	bool *flag;
} WindowFlag;

enum ThemeID {
	kThemeDark = 0,
	kThemeLight,
	kThemeCount
};

struct DebuggerTheme {
	ImU32 tableLightColor;
	ImU32 tableDarkColor;
	ImU32 borderColor;
	ImU32 sidebarTextColor;
	ImU32 gridTextColor;
	ImU32 playhead_color;
	ImU32 current_statement_bg;
	ImU32 channel_toggle;
	ImU32 channel_hide_bg;
	ImU32 channelSelectedCol;
	ImU32 channelHoveredCol;
	ImU32 contColors[6];

	// Breakpoints
	ImVec4 bp_color_disabled;
	ImVec4 bp_color_enabled;
	ImVec4 bp_color_hover;

	// Syntax Highlighting
	ImVec4 current_statement;
	ImVec4 line_color;
	ImVec4 call_color;
	ImVec4 builtin_color;
	ImVec4 var_color;
	ImVec4 literal_color;
	ImVec4 comment_color;
	ImVec4 type_color;
	ImVec4 keyword_color;
	ImVec4 the_color;

	// Variable / Script References
	ImVec4 script_ref;
	ImVec4 var_ref;
	ImVec4 var_ref_changed;
	ImVec4 var_ref_out_of_scope;

	// Control Panel
	ImVec4 cp_color;
	ImVec4 cp_color_red;
	ImVec4 cp_active_color;
	ImVec4 cp_bgcolor;
	ImVec4 cp_playing_color;
	ImVec4 cp_path_color;

	// Logger
	ImVec4 logger_error_b;
	ImVec4 logger_warning_b;
	ImVec4 logger_info_b;
	ImVec4 logger_debug_b;
	ImVec4 logger_error;
	ImVec4 logger_warning;
	ImVec4 logger_info;
	ImVec4 logger_debug;
};

typedef struct ImGuiState {

	struct WatchLogEntry {
		Common::String varName;
		Common::String value;
		Common::String scriptRef;
	};

	struct ScoreConfig {
		float _sidebarWidth = 60.0f;
		float _cellWidth = 14.0f;
		float _cellHeight = 22.0f;
		int _visibleFrames = 60;
		int _visibleChannels = 20;
		float _tableWidth = _cellWidth * _visibleFrames;
		float _tableHeight = _cellHeight * _visibleChannels;
		float _rulerHeight = 30.0f;
		float _rulerWidth = _cellWidth * _visibleFrames;
		float _sidebar1Height = _cellHeight * 6;
		float _labelBarHeight = _cellHeight;
		float _cellHeightExtended = 5 * _cellHeight;
	} _scoreCfg;

	struct ScoreState {
		int xSliderValue = 1; // first visible frame, 1 indexed
		int channelScrollOffset = 1; // first visible channel, 1 indexed
	} _scoreState;

	struct {
		Common::HashMap<CastMember *, ImGuiImage> _textures;
		bool _listView = true;
		int _thumbnailSize = 64;
		ImGuiTextFilter _nameFilter;
		int _typeFilter = 0x7FFF;
	} _cast;

	struct {
		ImGuiTextFilter _nameFilter;
		bool _showScriptContexts = true;
		Common::HashMap<Window *, ScriptData> _windowScriptData;
	} _functions;
	struct {
		CastMember *_castMember;
		Common::HashMap<CastMember *, int> _filmLoopCurrentFrame;
	} _castDetails;

	struct {
		bool _isScriptDirty = false; // indicates whether or not we have to display the script corresponding to the current stackframe
		bool _goToDefinition = false;
		bool _scrollToPC = false;
		uint _lastLinePC = 0;
		uint _callstackSize = 0;
		Common::String _highlightQuery; // lowercased, empty disables highlight
		bool _suppressHighlight = false; // used to disable highlighting in Execution Context
	} _dbg;

	struct {
		char input[256] = {};
		bool dirty = false;
		int mode = kSearchAll;
		Common::Array<ImGuiScript> results;
	} _search;

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
	Common::HashMap<int, ImGuiScript> _openHandlers;
	bool _showCompleteScript = true;

	Common::HashMap<Common::String, bool, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _variables;
	int _prevFrame = -1;
	struct {
		int frame = -1;
		int channel = -1;
		bool isMainChannel = false;
	} _selectedScoreCast;

	struct {
		int frame = -1;
		int channel = -1;
	} _hoveredScoreCast;

	Common::Array<Common::Array<Common::Pair<uint, uint>>> _continuationData;
	Common::String _loadedContinuationData;

	Common::Array<WatchLogEntry> _watchLog;

	Common::String _scoreWindow;
	Common::String _channelsWindow;
	Common::String _castWindow;
	int _scoreMode = 0;
	int _scoreFrameOffset = 1;
	int _scorePageSlider = 0;
	int _selectedChannel = -1;
	bool _scrollToChannel = false;

	ImFont *_tinyFont = nullptr;

	int _activeThemeID = kThemeLight;
	const DebuggerTheme *theme = nullptr;

	struct {
		Common::Path path;
		uint32 resType = 0;
		uint32 resId = 0;

		byte *data = nullptr;
		uint32 dataSize = 0;

		MemoryEditor memEdit;
	} _archive;

	ImGuiEx::ImGuiLogger *_logger = nullptr;
	bool _ignoreMouse = false;
	bool _enableMultiViewport = true;

	Window *_windowToRedraw = nullptr;
} ImGuiState;

// debugtools.cpp
ImGuiScript toImGuiScript(ScriptType scriptType, CastMemberID id, const Common::String &handlerId);
ScriptContext *getScriptContext(CastMemberID id);
ScriptContext *getScriptContext(uint32 nameIndex, CastMemberID castId, Common::String handler);
ScriptContext *resolveHandlerContext(int32 nameIndex, const CastMemberID &refId, const Common::String &handlerName);
ImGuiScript buildImGuiHandlerScript(ScriptContext *ctx, int castLibID, const Common::String &handlerName, const Common::String &moviePath);
void maybeHighlightLastItem(const Common::String &text);
void addToOpenHandlers(ImGuiScript handler);
void setScriptToDisplay(const ImGuiScript &script);
Director::Breakpoint *getBreakpoint(const Common::String &handlerName, uint16 scriptId, uint pc);
void displayScriptRef(CastMemberID &scriptId);
ImGuiImage getImageID(CastMember *castMember);
ImGuiImage getShapeID(CastMember *castMember);
ImGuiImage getTextID(CastMember *castMember);
Common::String getDisplayName(CastMember *castMember);
void showImage(const ImGuiImage &image, const char *name, float thumbnailSize);
void showImageWrappedBorder(const ImGuiImage &image, const char *name, float size);
ImVec4 convertColor(uint32 color);
void displayVariable(const Common::String &name, bool changed, bool outOfScope = false);
ImColor brightenColor(const ImColor &color, float factor);
Window *windowListCombo(Common::String *target);
Common::String formatHandlerName(int scriptId, int castId, Common::String handlerName, ScriptType scriptType, bool childScript);
void setTheme(int themeIndex);

// helper to draw thin rectangles for table grid
inline void addThinRect(ImDrawList *dl, ImVec2 min, ImVec2 max, ImU32 col, float thickness = 0.1f) {
	dl->AddLine(ImVec2(min.x, min.y), ImVec2(max.x, min.y), col, thickness); // top
	dl->AddLine(ImVec2(max.x, min.y), ImVec2(max.x, max.y), col, thickness); // right
	dl->AddLine(ImVec2(max.x, max.y), ImVec2(min.x, max.y), col, thickness); // bottom
	dl->AddLine(ImVec2(min.x, max.y), ImVec2(min.x, min.y), col, thickness); // left
}

void showCast();		// dt-cast.cpp
void showCastDetails();	// dt-castdetails.cpp
void showControlPanel();// dt-controlpanel.cpp

// dt-lists.cpp
void showVars();
void showWatchedVars();
void showBreakpointList();
void showArchive();

// dt-score.cpp
void showScore();
void showChannels();

void renderOldScriptAST(ImGuiScript &script, bool showByteCode, bool scrollTo);	// dt-script-d2.cpp
void renderScriptAST(ImGuiScript &script, bool showByteCode, bool scrollTo);	   // dt-script-d4.cpp

// dt-scripts.cpp
void showFuncList();
void showScriptCasts();
void showExecutionContext();
void showHandlers();

// dt-save-state.cpp
void saveCurrentState();
void loadSavedState();
Common::Array<WindowFlag> getWindowFlags();

// dt-search.cpp
void showSearchBar();

extern ImGuiState *_state;

} // End of namespace DT

} // End of namespace Director

#endif
