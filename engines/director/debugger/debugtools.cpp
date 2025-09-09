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

#include "backends/imgui/IconsMaterialSymbols.h"

#include "graphics/macgui/mactext.h"

#include "image/png.h"

#include "director/director.h"
#include "director/lingo/lingodec/context.h"
#include "director/lingo/lingodec/script.h"
#include "director/cast.h"
#include "director/castmember/bitmap.h"
#include "director/castmember/shape.h"
#include "director/castmember/text.h"
#include "director/debugger.h"
#include "director/movie.h"
#include "director/window.h"
#include "director/score.h"
#include "director/channel.h"
#include "director/picture.h"

#include "director/debugger/debugtools.h"
#include "director/debugger/dt-internal.h"

namespace Director {

namespace DT {

ImGuiState *_state = nullptr;

const LingoDec::Handler *getHandler(const Cast *cast, CastMemberID id, const Common::String &handlerId) {
	if (!cast)
		return nullptr;
	const ScriptContext *ctx = cast->_lingoArchive->findScriptContext(id.member);
	if (!ctx || !ctx->_functionHandlers.contains(handlerId))
		return nullptr;
	// for the moment it's happening with Director version < 4
	if (!cast->_lingodec)
		return nullptr;
	for (auto p : cast->_lingodec->scripts) {
		if (cast->getCastIdByScriptId(p.first) != id.member)
			continue;

		for (const LingoDec::Handler &handler : p.second->handlers) {
			if (handler.name == handlerId) {
				return &handler;
			}
		}
		for (const LingoDec::Script *factoryScript : p.second->factories) {
			for (const LingoDec::Handler &handler : factoryScript->handlers) {
				if (handler.name == handlerId) {
					return &handler;
				}
			}
		}
	}
	return nullptr;
}

const LingoDec::Handler *getHandler(CastMemberID id, const Common::String &handlerId) {
	const Director::Movie *movie = g_director->getCurrentMovie();
	const Cast *cast = movie->getCasts()->getVal(id.castLib);

	const LingoDec::Handler *handler = getHandler(cast, id, handlerId);
	if (handler)
		return handler;

	return getHandler(movie->getSharedCast(), id, handlerId);
}

ImGuiScript toImGuiScript(ScriptType scriptType, CastMemberID id, const Common::String &handlerId) {
	ImGuiScript result;
	result.id = id;
	result.handlerId = handlerId;
	result.type = scriptType;

	const LingoDec::Handler *handler = getHandler(id, handlerId);
	if (!handler) {
		const ScriptContext *ctx;
		if (id.castLib == SHARED_CAST_LIB) {
			ctx = g_director->getCurrentMovie()->getSharedCast()->_lingoArchive->getScriptContext(scriptType, id.member);
		} else {
			ctx = g_director->getCurrentMovie()->getScriptContext(scriptType, id);
		}
		if (!ctx) return result;
		result.oldAst = ctx->_assemblyAST;
		return result;
	}

	result.bytecodeArray = handler->bytecodeArray;
	result.root = handler->ast.root;
	result.isGenericEvent = handler->isGenericEvent;
	result.argumentNames = handler->argumentNames;
	result.propertyNames = handler->script->propertyNames;
	result.globalNames = handler->globalNames;

	LingoDec::Script *script = handler->script;
	if (!script)
		return result;

	result.isMethod = script->isFactory();
	return result;
}

ScriptContext *getScriptContext(CastMemberID id) {
	const Director::Movie *movie = g_director->getCurrentMovie();;
	const Cast *cast = movie->getCasts()->getVal(id.castLib);

	if (!cast) {
		return nullptr;
	}

	ScriptContext *ctx = cast->_lingoArchive->findScriptContext(id.member);
	return ctx;
}

ScriptContext *getScriptContext(uint32 nameIndex, CastMemberID id, Common::String handlerName) {
	Movie *movie = g_director->getCurrentMovie();
	Cast *cast = movie->getCasts()->getVal(id.castLib);

	// If the name at nameIndex is not the same as handler name, means its a local script (in the same Lscr resource)
	if (cast && cast->_lingoArchive->names[nameIndex] != handlerName) {
		return cast->_lingoArchive->findScriptContext(id.member);
	}

	for (auto it : cast->_lingoArchive->scriptContexts[kMovieScript]) {
		if (it._value->_functionHandlers.contains(handlerName)) {
			return it._value;
		}
	}

	return nullptr;
}

Director::Breakpoint *getBreakpoint(const Common::String &handlerName, uint16 scriptId, uint pc) {
	auto &bps = g_lingo->getBreakpoints();
	for (uint i = 0; i < bps.size(); i++) {
		if (bps[i].type == kBreakpointFunction && bps[i].scriptId == scriptId && bps[i].funcName == handlerName && bps[i].funcOffset == pc) {
			return &bps[i];
		}
	}
	return nullptr;
}

ImGuiImage getImageID(CastMember *castMember) {
	if (castMember->_type != CastType::kCastBitmap) {
		return {};
	}

	BitmapCastMember *bmpMember = (BitmapCastMember *)castMember;

	if (_state->_cast._textures.contains(bmpMember)) {
		return _state->_cast._textures[bmpMember];
	}

	bmpMember->load();
	Picture *pic = bmpMember->_picture;
	if (!pic)
		return {};

	ImTextureID textureID = (ImTextureID)(intptr_t)g_system->getImGuiTexture(pic->_surface, pic->_palette, pic->_paletteColors);
	_state->_cast._textures[bmpMember] = {textureID, pic->_surface.w, pic->_surface.h};
	return _state->_cast._textures[bmpMember];
}

static void setToolTipImage(const ImGuiImage &image, const char *name) {
	if (ImGui::IsItemHovered() && ImGui::BeginTooltip()) {
		ImGui::Text("%s", name);
		ImGui::Image(image.id, ImVec2(image.width, image.height), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
		ImGui::EndTooltip();
	}
}

void showImage(const ImGuiImage &image, const char *name, float thumbnailSize) {
	ImVec2 size;
	if (image.width > image.height) {
		size = {thumbnailSize - 2, (thumbnailSize - 2) * image.height / image.width};
	} else {
		size = {(thumbnailSize - 2) * image.width / image.height, thumbnailSize - 2};
	}
	ImGui::BeginGroup();
	ImVec2 screenPos = ImGui::GetCursorScreenPos();
	ImGui::GetWindowDrawList()->AddRect(screenPos, screenPos + ImVec2(thumbnailSize, thumbnailSize), 0xFFFFFFFF);
	ImVec2 pos = ImGui::GetCursorPos();

	// Reserve the space of area thumbnailSize * thumbnailSize to make sure the column stretches properly
	ImGui::Dummy(ImVec2(thumbnailSize, thumbnailSize));
	ImVec2 imgPos = pos + ImVec2(1 + (thumbnailSize - 2 - size.x) * 0.5f, 1 + (thumbnailSize - 2 - size.y) * 0.5f);
	ImGui::SetCursorPos(imgPos);
	ImGui::Image(image.id, size);
	ImGui::EndGroup();
	setToolTipImage(image, name);
}

ImGuiImage getShapeID(CastMember *castMember) {
	if (castMember->_type != CastType::kCastShape) {
		return {};
	}

	if (_state->_cast._textures.contains(castMember)) {
		return _state->_cast._textures[castMember];
	}

	ShapeCastMember *shapeMember = (ShapeCastMember *)castMember;

	// Make a temporary Sprite
	Sprite *sprite = new Sprite();
	sprite->_movie = g_director->getCurrentMovie();
	sprite->setCast(CastMemberID(castMember->getID(), castMember->getCast()->_castLibID));
	sprite->_ink = shapeMember->_ink;
	sprite->_backColor = shapeMember->getBackColor();
	sprite->_foreColor = shapeMember->getForeColor();
	sprite->_pattern = shapeMember->_pattern;
	sprite->_thickness = shapeMember->_lineThickness;

	// Make a temporary channel to blit the shape from
	Channel *channel = new Channel(nullptr, sprite);

	Common::Rect bbox(castMember->getBbox());

	// Manually set the bbox of the channel to the bbox of cast member
	// Even though the BBox of the channel and cast member are the same, the channel's bbox is offset to a non zero origin
	// Depending upon the position of the sprite in the window
	channel->setBbox(bbox.left, bbox.top, bbox.right, bbox.bottom);
	Graphics::ManagedSurface *managedSurface = new Graphics::ManagedSurface(bbox.width(), bbox.height(), g_director->_pixelformat);

	Window::inkBlitFrom(channel, bbox, managedSurface);
	Graphics::Surface surface = managedSurface->rawSurface();

	if (debugChannelSet(8, kDebugImages)) {
		Common::String prepend = "shape";
		Common::String filename = Common::String::format("./dumps/%s-%s-%d.png", g_director->getCurrentMovie()->getMacName().c_str(), encodePathForDump(prepend).c_str(), castMember->getID());
		Common::DumpFile bitmapFile;

		bitmapFile.open(Common::Path(filename), true);
		Image::writePNG(bitmapFile, surface, g_director->getPalette());

		bitmapFile.close();
	}

	ImTextureID textureID = (ImTextureID)(intptr_t)g_system->getImGuiTexture(surface, g_director->getPalette(), g_director->getPaletteColorCount());

	delete managedSurface;
	delete sprite;
	delete channel;

	int16 width = surface.w, height = surface.h;

	_state->_cast._textures[castMember] = {textureID, width, height};
	return _state->_cast._textures[castMember];
}

ImGuiImage getTextID(CastMember *castMember) {
	if (castMember->_type != CastType::kCastText && castMember->_type != CastType::kCastButton && castMember->_type != CastType::kCastRichText) {
		return {};
	}

	if (_state->_cast._textures.contains(castMember)) {
		return _state->_cast._textures[castMember];
	}

	Common::Rect bbox(castMember->getBbox());

	// Make a temporary Sprite
	Sprite *sprite = new Sprite();
	sprite->_spriteType = kTextSprite;
	sprite->_movie = g_director->getCurrentMovie();
	sprite->setCast(CastMemberID(castMember->getID(), castMember->getCast()->_castLibID));
	sprite->_backColor = castMember->getBackColor();
	sprite->_foreColor = castMember->getForeColor();
	sprite->_editable = false;

	// Make a temporary channel to blit the shape from
	Channel *channel = new Channel(nullptr, sprite);

	Graphics::MacWidget *widget = castMember->createWidget(bbox, channel, kTextSprite);
	Graphics::Surface surface;
	surface.copyFrom(*widget->getSurface());

	if (debugChannelSet(8, kDebugImages)) {
		Common::String prepend = "text";
		Common::String filename = Common::String::format("./dumps/%s-%s-%d.png", g_director->getCurrentMovie()->getMacName().c_str(), encodePathForDump(prepend).c_str(), castMember->getID());
		Common::DumpFile bitmapFile;

		bitmapFile.open(Common::Path(filename), true);
		Image::writePNG(bitmapFile, surface, g_director->getPalette());

		bitmapFile.close();
	}

	ImTextureID textureID = (ImTextureID)(intptr_t)g_system->getImGuiTexture(surface, g_director->getPalette(), g_director->getPaletteColorCount());

	int16 width = surface.w, height = surface.h;
	surface.free();
	delete widget;
	delete sprite;
	delete channel;

	_state->_cast._textures[castMember] = {textureID, width, height};
	return _state->_cast._textures[castMember];
}

void displayVariable(const Common::String &name, bool changed, bool outOfScope) {
	ImU32 var_color = ImGui::GetColorU32(_state->_colors._var_ref);
	ImU32 color;

	color = ImGui::GetColorU32(_state->_colors._bp_color_disabled);

	if (_state->_variables.contains(name))
		color = ImGui::GetColorU32(_state->_colors._bp_color_enabled);

	ImDrawList *dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 eyeSize = ImGui::CalcTextSize(ICON_MS_VISIBILITY " ");
	ImVec2 textSize = ImGui::CalcTextSize(name.c_str());

	ImGui::InvisibleButton("Line", ImVec2(textSize.x + eyeSize.x, textSize.y));
	if (ImGui::IsItemClicked(0)) {
		if (color == ImGui::GetColorU32(_state->_colors._bp_color_enabled)) {
			_state->_variables.erase(name);
			color = ImGui::GetColorU32(_state->_colors._bp_color_disabled);
		} else {
			_state->_variables[name] = true;
			color = ImGui::GetColorU32(_state->_colors._bp_color_enabled);
		}
	}

	if (changed) {
		var_color = ImGui::GetColorU32(_state->_colors._var_ref_changed);
	} else if (outOfScope) {
		var_color = ImGui::GetColorU32(_state->_colors._var_ref_out_of_scope);
	}

	if (color == ImGui::GetColorU32(_state->_colors._bp_color_disabled) && ImGui::IsItemHovered()) {
		color = ImGui::GetColorU32(_state->_colors._bp_color_hover);
	}

	dl->AddText(pos, color, ICON_MS_VISIBILITY " ");
	dl->AddText(ImVec2(pos.x + eyeSize.x, pos.y), var_color, name.c_str());
}

ImVec4 convertColor(uint32 color) {
	if (g_director->_colorDepth <= 8) {
		float r = g_director->getPalette()[color * 3 + 0] * 1.0 / 255.0;
		float g = g_director->getPalette()[color * 3 + 1] * 1.0 / 255.0;
		float b = g_director->getPalette()[color * 3 + 2] * 1.0 / 255.0;

		return ImVec4(r, g, b, 1.0);
	}

	return ImGui::ColorConvertU32ToFloat4(color);
}

static void addScriptCastToDisplay(CastMemberID &id) {
	_state->_scriptCasts.remove(id);
	_state->_scriptCasts.push_back(id);
}

void setScriptToDisplay(const ImGuiScript &script) {
	ScriptData *scriptData = &_state->_functions._windowScriptData.getOrCreateVal(g_director->getCurrentWindow());
	uint index = scriptData->_scripts.size();
	if (index && scriptData->_scripts[index - 1] == script) {
		scriptData->_showScript = true;
		return;
	}
	scriptData->_scripts.push_back(script);
	scriptData->_current = index;
	scriptData->_showScript = true;
	_state->_dbg._scrollToPC = true;
}

void displayScriptRef(CastMemberID &scriptId) {
	if (scriptId.member) {
		ImGui::TextColored(_state->_colors._script_ref, "%d", scriptId.member);

		ImGui::SetItemTooltip(scriptId.asString().c_str());

		if (ImGui::IsItemClicked(0))
			addScriptCastToDisplay(scriptId);
	} else {
		ImGui::Selectable("  ");
	}
}

ImColor brightenColor(const ImColor& color, float factor) {
	ImVec4 col = color.Value;
	col.x = CLIP<float>(col.x * factor, 0.0f, 1.0f);
	col.y = CLIP<float>(col.y * factor, 0.0f, 1.0f);
	col.z = CLIP<float>(col.z * factor, 0.0f, 1.0f);
	return ImColor(col);
}

Window *windowListCombo(Common::String *target) {
	const Common::Array<Window *> *windowList = g_director->getWindowList();
	const Common::String selWin = *target;
	Window *res = nullptr;

	Common::String stage = g_director->getStage()->getCurrentMovie()->getMacName();

	// Check if the relevant window is gone
	bool found = false;
	for (auto window : (*windowList)) {
		if (window->getCurrentMovie()->getMacName() == selWin) {
			// Found the selected window
			found = true;
			res = window;
			break;
		}
	}

	// Our default is Stage
	if (selWin.empty() || windowList->empty() || !found) {
		*target = stage;
		res = g_director->getStage();
	}

	ImGui::Text("Window:");
	ImGui::SameLine();

	if (ImGui::BeginCombo("##window", selWin.c_str())) {
		bool selected = (*target == stage);
		if (ImGui::Selectable(stage.c_str(), selected))
			*target = stage;

		if (selected) {
			ImGui::SetItemDefaultFocus();
			res = g_director->getStage();
		}

		for (auto window : (*windowList)) {
			Common::String winName = window->getCurrentMovie()->getMacName();
			selected = (*target == winName);
			if (ImGui::Selectable(winName.c_str(), selected)) {
				*target = winName;
				res = window;
			}

			if (selected) {
				ImGui::SetItemDefaultFocus();
				res = window;
			}

		}
		ImGui::EndCombo();
	}

	return res;
}

static void showSettings() {
	if (!_state->_w.settings)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(480, 240), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Settings", &_state->_w.settings)) {
		ImGui::ColorEdit4("Breakpoint disabled", &_state->_colors._bp_color_disabled.x);
		ImGui::ColorEdit4("Breakpoint enabled", &_state->_colors._bp_color_enabled.x);
		ImGui::ColorEdit4("Breakpoint hover", &_state->_colors._bp_color_hover.x);

		ImGui::ColorEdit4("Channel toggle", &_state->_colors._channel_toggle.x);

		ImGui::SeparatorText("Lingo syntax");
		ImGui::ColorEdit4("Line", &_state->_colors._line_color.x);
		ImGui::ColorEdit4("Call", &_state->_colors._call_color.x);
		ImGui::ColorEdit4("Builtin", &_state->_colors._builtin_color.x);
		ImGui::ColorEdit4("Variable", &_state->_colors._var_color.x);
		ImGui::ColorEdit4("Literal", &_state->_colors._literal_color.x);
		ImGui::ColorEdit4("Comment", &_state->_colors._comment_color.x);
		ImGui::ColorEdit4("Type", &_state->_colors._type_color.x);
		ImGui::ColorEdit4("Keyword", &_state->_colors._keyword_color.x);
		ImGui::ColorEdit4("The entity", &_state->_colors._the_color.x);

		ImGui::SeparatorText("References");
		ImGui::ColorEdit4("Script", &_state->_colors._script_ref.x);
		ImGui::ColorEdit4("Variable", &_state->_colors._var_ref.x);
		ImGui::ColorEdit4("Variable changed", &_state->_colors._var_ref_changed.x);

		_state->_logger->drawColorOptions();
	}
	ImGui::End();
}

void onLog(LogMessageType::Type type, int level, uint32 debugChannel, const char *message) {
	switch (type) {
	case LogMessageType::kError:
		_state->_logger->addLog("[error]%s", message);
		break;
	case LogMessageType::kWarning:
		_state->_logger->addLog("[warn]%s", message);
		break;
	case LogMessageType::kInfo:
		_state->_logger->addLog("%s", message);
		break;
	case LogMessageType::kDebug:
		_state->_logger->addLog("[debug]%s", message);
		break;
	}
}

void onImGuiInit() {
	ImGuiIO &io = ImGui::GetIO();
	io.Fonts->AddFontDefault();

	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = false;
	icons_config.OversampleH = 3;
	icons_config.OversampleV = 3;
	icons_config.GlyphOffset = {0, 4};

	static const ImWchar icons_ranges[] = {ICON_MIN_MS, ICON_MAX_MS, 0};
	io.FontDefault = ImGui::addTTFFontFromArchive("MaterialSymbolsSharp.ttf", 16.f, &icons_config, icons_ranges);

	_state = new ImGuiState();

	_state->_tinyFont = ImGui::addTTFFontFromArchive("LiberationSans-Regular.ttf", 10.0f, nullptr, nullptr);

	_state->_archive.memEdit.ReadOnly = true;

	_state->_logger = new ImGuiEx::ImGuiLogger;

	Common::setLogWatcher(onLog);
}

void onImGuiRender() {
	if (!debugChannelSet(-1, kDebugImGui)) {
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse;
		return;
	}

	if (!_state)
		return;

	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags &= ~(ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse);

	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("View")) {
			ImGui::SeparatorText("Windows");

			if (ImGui::MenuItem("View All")) {
				if (_state->_wasHidden)
					_state->_w = _state->_savedW;

				_state->_wasHidden = false;
			}

			if (ImGui::MenuItem("Hide All")) {
				if (!_state->_wasHidden) {
					_state->_savedW = _state->_w;

					memset((void *)&_state->_w, 0, sizeof(_state->_w));
				}

				_state->_wasHidden = true;
			}

			ImGui::MenuItem("Control Panel", NULL, &_state->_w.controlPanel);
			ImGui::MenuItem("Score", NULL, &_state->_w.score);
			ImGui::MenuItem("Functions", NULL, &_state->_w.funcList);
			ImGui::MenuItem("Cast", NULL, &_state->_w.cast);
			ImGui::MenuItem("Channels", NULL, &_state->_w.channels);
			ImGui::MenuItem("Breakpoints", NULL, &_state->_w.bpList);
			ImGui::MenuItem("Vars", NULL, &_state->_w.vars);
			ImGui::MenuItem("Watched Vars", NULL, &_state->_w.watchedVars);
			ImGui::MenuItem("Logger", NULL, &_state->_w.logger);
			ImGui::MenuItem("Archive", NULL, &_state->_w.archive);
			ImGui::MenuItem("Execution Context", NULL, &_state->_w.executionContext);

			ImGui::SeparatorText("Misc");
			if (ImGui::MenuItem("Save state")) {
				saveCurrentState();
			}
			if (ImGui::MenuItem("Load state")) {
				loadSavedState();
			}
			ImGui::Separator();
			ImGui::MenuItem("Settings", NULL, &_state->_w.settings);

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	showScriptCasts();
	showExecutionContext();
	showHandlers();

	showControlPanel();
	showVars();
	showChannels();
	showCast();
	showFuncList();
	showScore();
	showBreakpointList();
	showSettings();
	showArchive();
	showWatchedVars();
	_state->_logger->draw("Logger", &_state->_w.logger);
}

void onImGuiCleanup() {
	Common::setLogWatcher(nullptr);
	if (_state) {
		free(_state->_archive.data);

		delete _state->_logger;
	}

	delete _state;
	_state = nullptr;
}

int getSelectedChannel(){
	return _state ? _state->_selectedChannel : -1;
}

Common::String formatHandlerName(int scriptId, int castId, Common::String handlerName, ScriptType scriptType, bool childScript) {
	Common::String formatted = Common::String();
	// Naming convention: <script id> (<cast id/cast id of parent script>): name of handler: script type
	if (childScript) {
		formatted = Common::String::format("%d (p<%d>):%s :%s", scriptId, castId, handlerName.size() ? handlerName.c_str() : "<unnamed>", scriptType2str(scriptType));
	} else {
		formatted = Common::String::format("%d (%d) :%s :%s", scriptId, castId, handlerName.size() ? handlerName.c_str() : "<unnamed>", scriptType2str(scriptType));
	}
	return formatted;
}

} // namespace DT
} // namespace Director
