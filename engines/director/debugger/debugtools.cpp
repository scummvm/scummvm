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

#include "director/director.h"
#include "director/lingo/lingodec/context.h"
#include "director/lingo/lingodec/script.h"
#include "director/cast.h"
#include "director/castmember/bitmap.h"
#include "director/debugger.h"
#include "director/movie.h"
#include "director/picture.h"

#include "director/debugger/debugtools.h"
#include "director/debugger/dt-internal.h"

namespace Director {

namespace DT {

ImGuiState *_state = nullptr;

bool toggleButton(const char *label, bool *p_value, bool inverse) {
	int pop = 0;
	if (*p_value != inverse) {
		ImVec4 hovered = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_Button, hovered);
		pop = 1;
	}
	bool result = ImGui::Button(label);
	if (result) {
		*p_value = !*p_value;
	}
	ImGui::PopStyleColor(pop);
	return result;
}

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
		if ((p.second->castID & 0xFFFF) != id.member)
			continue;
		;
		for (const LingoDec::Handler &handler : p.second->handlers) {
			if (handler.name == handlerId) {
				return &handler;
			}
		}
	}
	return nullptr;
}

const LingoDec::Handler *getHandler(CastMemberID id, const Common::String &handlerId) {
	const Director::Movie *movie = g_director->getCurrentMovie();
	for (const auto it : *movie->getCasts()) {
		const Cast *cast = it._value;
		const LingoDec::Handler *handler = getHandler(cast, id, handlerId);
		if (handler)
			return handler;
	}
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
	if (castMember->_type != CastType::kCastBitmap)
		return {};

	BitmapCastMember *bmpMember = (BitmapCastMember *)castMember;
	Common::Rect bbox(bmpMember->getBbox());
	Graphics::Surface *bmp = bmpMember->getMatte(bbox);
	if (!bmp)
		return {};

	if (_state->_cast._textures.contains(bmp))
		return _state->_cast._textures[bmp];

	Picture *pic = bmpMember->_picture;
	if (!pic)
		return {};

	ImTextureID textureID = g_system->getImGuiTexture(pic->_surface, pic->_palette, pic->_paletteColors);
	_state->_cast._textures[bmp] = {textureID, pic->_surface.w, pic->_surface.h};
	return _state->_cast._textures[bmp];
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
	ImVec2 imgPos = pos + ImVec2(1 + (thumbnailSize - 2 - size.x) * 0.5f, 1 + (thumbnailSize - 2 - size.y) * 0.5f);
	ImGui::SetCursorPos(imgPos);
	ImGui::Image(image.id, size);
	ImGui::EndGroup();
	setToolTipImage(image, name);
}

void displayVariable(const Common::String &name, bool changed) {
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

	if (changed)
		var_color = ImGui::GetColorU32(_state->_colors._var_ref_changed);

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
	uint index = _state->_functions._scripts.size();
	if (index && _state->_functions._scripts[index - 1] == script) {
		_state->_functions._showScript = true;
		return;
	}
	_state->_functions._scripts.push_back(script);
	_state->_functions._current = index;
	_state->_functions._showScript = true;
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

static void showSettings() {
	if (!_state->_w.settings)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(480, 240), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Settings", &_state->_w.settings)) {
		ImGui::ColorEdit4("Breakpoint disabled", &_state->_colors._bp_color_disabled.x);
		ImGui::ColorEdit4("Breakpoint enabled", &_state->_colors._bp_color_enabled.x);
		ImGui::ColorEdit4("Breakpoint hover", &_state->_colors._bp_color_hover.x);

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

		ImGui::SeparatorText("Logger");
		ImGui::ColorEdit4("Error", &_state->_colors._logger_error.x);
		ImGui::ColorEdit4("Error Button", &_state->_colors._logger_error_b.x);
		ImGui::ColorEdit4("Warning", &_state->_colors._logger_warning.x);
		ImGui::ColorEdit4("Warning Button", &_state->_colors._logger_warning_b.x);
		ImGui::ColorEdit4("Info", &_state->_colors._logger_info.x);
		ImGui::ColorEdit4("Info Button", &_state->_colors._logger_info_b.x);
		ImGui::ColorEdit4("Debug", &_state->_colors._logger_debug.x);
		ImGui::ColorEdit4("Debug Button", &_state->_colors._logger_debug_b.x);
	}
	ImGui::End();
}

void onLog(LogMessageType::Type type, int level, uint32 debugChannels, const char *message) {
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
	ImGui::addTTFFontFromArchive("MaterialSymbolsSharp.ttf", 16.f, &icons_config, icons_ranges);

	_state = new ImGuiState();

	_state->_tinyFont = ImGui::addTTFFontFromArchive("FreeSans.ttf", 10.0f, nullptr, nullptr);

	_state->_archive.memEdit.ReadOnly = true;

	_state->_logger = new ImGuiLogger;

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
			ImGui::MenuItem("CallStack", NULL, &_state->_w.callStack);
			ImGui::MenuItem("Breakpoints", NULL, &_state->_w.bpList);
			ImGui::MenuItem("Vars", NULL, &_state->_w.vars);
			ImGui::MenuItem("Watched Vars", NULL, &_state->_w.watchedVars);
			ImGui::MenuItem("Logger", NULL, &_state->_w.logger);
			ImGui::MenuItem("Archive", NULL, &_state->_w.archive);

			ImGui::SeparatorText("Misc");
			if (ImGui::MenuItem("Save state")) {
			}
			if (ImGui::MenuItem("Load state")) {
			}
			ImGui::Separator();
			ImGui::MenuItem("Settings", NULL, &_state->_w.settings);

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	showScriptCasts();
	showScripts();

	showControlPanel();
	showVars();
	showCallStack();
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

} // namespace DT
} // namespace Director
