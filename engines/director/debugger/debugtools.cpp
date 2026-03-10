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
	if (id.castLib == SHARED_CAST_LIB)
		return getHandler(movie->getSharedCast(), id, handlerId);

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

	if (id.castLib == SHARED_CAST_LIB)
		cast = movie->getSharedCast();
	else
		cast = movie->getCasts()->getVal(id.castLib);

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

static ScriptContext *findHandlerContext(Cast *cast, const Common::String &handlerName) {
	if (!cast || !cast->_lingoArchive)
		return nullptr;

	for (int i = 0; i <= kMaxScriptType; i++) {
		if (cast->_lingoArchive->scriptContexts[i].empty())
			continue;
		for (auto &scriptContext : cast->_lingoArchive->scriptContexts[i]) {
			if (scriptContext._value && scriptContext._value->_functionHandlers.contains(handlerName))
				return scriptContext._value;
		}
	}
	return nullptr;
}

ScriptContext *resolveHandlerContext(int32 nameIndex, const CastMemberID &refId, const Common::String &handlerName) {
	Movie *movie = g_director->getCurrentMovie();
	if (!movie)
		return nullptr;

	Cast *cast = nullptr;
	if (refId.castLib == SHARED_CAST_LIB) {
		cast = movie->getSharedCast();
	} else {
		cast = movie->getCasts()->getVal(refId.castLib);
	}

	if (cast && cast->_lingoArchive && nameIndex >= 0 && (uint32)nameIndex < cast->_lingoArchive->names.size()) {
		if (cast->_lingoArchive->names[nameIndex] != handlerName) {
			ScriptContext *local = cast->_lingoArchive->findScriptContext(refId.member);
			if (local && local->_functionHandlers.contains(handlerName))
				return local;
		}
	}

	if (ScriptContext *ctx = findHandlerContext(cast, handlerName))
		return ctx;

	Cast *shared = movie->getSharedCast();
	if (shared && shared != cast) {
		if (ScriptContext *ctx = findHandlerContext(shared, handlerName))
			return ctx;
	}

	return nullptr;
}

ImGuiScript buildImGuiHandlerScript(ScriptContext *ctx, int castLibID, const Common::String &handlerName, const Common::String &moviePath) {
	ImGuiScript script;
	if (!ctx)
		return script;

	Movie *movie = g_director->getCurrentMovie();
	Cast *cast = nullptr;
	if (castLibID == SHARED_CAST_LIB) {
		cast = movie ? movie->getSharedCast() : nullptr;
	} else {
		cast = movie ? movie->getCasts()->getVal(castLibID) : nullptr;
	}

	int castId = ctx->_id;
	bool childScript = false;
	if (castId == -1) {
		childScript = true;
		if (cast) {
			castId = cast->getCastIdByScriptId(ctx->_parentNumber);
		}
	}

	CastMemberID memberID(castId, castLibID);
	script = toImGuiScript(ctx->_scriptType, memberID, handlerName);
	script.byteOffsets = ctx->_functionByteOffsets[script.handlerId];
	script.moviePath = moviePath;
	script.handlerName = formatHandlerName(ctx->_scriptId, castId, script.handlerId, ctx->_scriptType, childScript);
	return script;
}

void maybeHighlightLastItem(const Common::String &text) {
	if (!_state)
		return;
	const Common::String &q = _state->_dbg._highlightQuery;
	if (q.empty() || text.empty() || _state->_dbg._suppressHighlight)
		return;

	Common::String lower = text;
	lower.toLowercase();
	if (!lower.contains(q.c_str()))
		return;

	ImDrawList *dl = ImGui::GetWindowDrawList();
	const ImVec2 min = ImGui::GetItemRectMin();
	const ImVec2 max = ImGui::GetItemRectMax();
	if (max.x <= min.x || max.y <= min.y)
		return;

	ImU32 col = IM_COL32(255, 230, 0, 90);
	dl->AddRectFilled(min, max, col, 2.0f);
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

void showImageWrappedBorder(const ImGuiImage &image, const char *name, float imageSize) {
	ImVec2 size;
	if (image.width > image.height) {
		size = {imageSize, imageSize * image.height / image.width};
	} else {
		size = {imageSize * image.width / image.height, imageSize};
	}
	ImGui::BeginGroup();
	ImVec2 screenPos = ImGui::GetCursorScreenPos();
	ImGui::GetWindowDrawList()->AddRect(screenPos, screenPos + size, 0xFFFFFFFF);
	ImVec2 pos = ImGui::GetCursorPos();

	// Reserve the space of area size to make sure the column stretches properly
	ImGui::Dummy(size);
	ImGui::SetCursorPos(ImVec2(pos.x + 2, pos.y + 2));	// 1 pixel border + 1 pixel padding
	ImGui::Image(image.id, ImVec2(size.x - 4, size.y - 4)); // 2 pixels on each side for border and padding
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
	ImU32 var_color = ImGui::GetColorU32(_state->theme->var_ref);
	ImU32 color;

	color = ImGui::GetColorU32(_state->theme->bp_color_disabled);

	if (_state->_variables.contains(name))
		color = ImGui::GetColorU32(_state->theme->bp_color_enabled);

	ImDrawList *dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 eyeSize = ImGui::CalcTextSize(ICON_MS_VISIBILITY " ");
	ImVec2 textSize = ImGui::CalcTextSize(name.c_str());

	ImGui::InvisibleButton("Line", ImVec2(textSize.x + eyeSize.x, textSize.y));
	if (ImGui::IsItemClicked(0)) {
		if (color == ImGui::GetColorU32(_state->theme->bp_color_enabled)) {
			_state->_variables.erase(name);
			color = ImGui::GetColorU32(_state->theme->bp_color_disabled);
		} else {
			_state->_variables[name] = true;
			color = ImGui::GetColorU32(_state->theme->bp_color_enabled);
		}
	}

	if (changed) {
		var_color = ImGui::GetColorU32(_state->theme->var_ref_changed);
	} else if (outOfScope) {
		var_color = ImGui::GetColorU32(_state->theme->var_ref_out_of_scope);
	}

	if (color == ImGui::GetColorU32(_state->theme->bp_color_disabled) && ImGui::IsItemHovered()) {
		color = ImGui::GetColorU32(_state->theme->bp_color_hover);
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

void addToOpenHandlers(ImGuiScript handler) {
	_state->_openHandlers.erase(handler.id.member);
	_state->_openHandlers[handler.id.member] = handler;
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
		ImGui::TextColored(_state->theme->script_ref, "%d", scriptId.member);

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

static const DebuggerTheme themes[kThemeCount] = {
	// [kThemeDark]
	{
		IM_COL32(0x33, 0x33, 0x33, 0xFF), // tableLightColor
		IM_COL32(0x26, 0x26, 0x26, 0xFF), // tableDarkColor
		IM_COL32(0x66, 0x66, 0x66, 0x64), // borderColor
		IM_COL32(0xC8, 0xC8, 0xC8, 0xFF), // sidebarTextColor
		IM_COL32(0x1E, 0x1E, 0x1E, 0xFF), // gridTextColor
		IM_COL32(0xC8, 0x32, 0x00, 0xFF), // playhead_color
		IM_COL32(0xFF, 0xFF, 0x00, 0x20), // current_statement_bg
		IM_COL32(0x30, 0x30, 0xFF, 0xFF), // channel_toggle
		IM_COL32(0xB4, 0x32, 0x32, 0xC8), // channel_hide_bg
		IM_COL32(0x94, 0x00, 0xD3, 0xFF), // channelSelectedCol
		IM_COL32(0xFF, 0xFF, 0x00, 0x3C), // channelHoveredCol
		{								 // contColors
			IM_COL32(0xCE, 0xCE, 0xFF, 0x80),
			IM_COL32(0xFF, 0xFF, 0xCE, 0x80),
			IM_COL32(0xCE, 0xFF, 0xCE, 0x80),
			IM_COL32(0xCE, 0xFF, 0xFF, 0x80),
			IM_COL32(0xFF, 0xCE, 0xFF, 0x80),
			IM_COL32(0xFF, 0xCE, 0x9C, 0x80)
		},

		ImVec4(0.9f, 0.08f, 0.0f, 0.0f),   // bp_color_disabled
		ImVec4(0.9f, 0.08f, 0.0f, 1.0f),   // bp_color_enabled
		ImVec4(0.42f, 0.17f, 0.13f, 1.0f), // bp_color_hover

		ImVec4(1.00f, 1.00f, 0.00f, 1.0f),  // current_statement
		ImVec4(0.18f, 0.18f, 0.18f, 1.0f),  // line_color
		ImVec4(1.00f, 0.77f, 0.36f, 1.0f),  // call_color
		ImVec4(0.38f, 0.49f, 1.00f, 1.0f),  // builtin_color
		ImVec4(0.29f, 0.80f, 0.37f, 1.0f),  // var_color
		ImVec4(1.00f, 0.62f, 0.85f, 0.62f), // literal_color
		ImVec4(1.00f, 0.65f, 0.62f, 0.58f), // comment_color
		ImVec4(0.72f, 0.72f, 0.72f, 0.75f), // type_color
		ImVec4(0.76f, 0.76f, 0.76f, 1.0f),  // keyword_color
		ImVec4(1.00f, 0.29f, 0.94f, 1.0f),  // the_color

		ImVec4(0.50f, 0.50f, 1.00f, 1.0f), // script_ref
		ImVec4(0.90f, 0.90f, 0.00f, 1.0f), // var_ref
		ImVec4(1.00f, 0.00f, 0.00f, 1.0f), // var_ref_changed
		ImVec4(1.00f, 0.00f, 1.00f, 1.0f), // var_ref_out_of_scope

		ImVec4(0.8f, 0.8f, 0.8f, 1.0f), // cp_color
		ImVec4(1.0f, 0.6f, 0.6f, 1.0f), // cp_color_red
		ImVec4(1.0f, 1.0f, 0.4f, 1.0f), // cp_active_color
		ImVec4(0.2f, 0.2f, 1.0f, 1.0f), // cp_bgcolor
		ImVec4(0.3f, 0.3f, 1.0f, 1.0f), // cp_playing_color
		ImVec4(0.9f, 0.8f, 0.5f, 1.0f), // cp_path_color

		ImVec4(1.0f, 0.0f, 0.0f, 1.0f), // logger_error_b
		ImVec4(1.0f, 1.0f, 0.0f, 1.0f), // logger_warning_b
		ImVec4(10.f, 1.0f, 1.0f, 1.0f), // logger_info_b
		ImVec4(0.8f, 0.8f, 0.8f, 1.0f), // logger_debug_b
		ImVec4(1.0f, 0.4f, 0.4f, 1.0f), // logger_error
		ImVec4(1.0f, 1.0f, 0.4f, 1.0f), // logger_warning
		ImVec4(1.0f, 0.8f, 0.6f, 1.0f), // logger_info
		ImVec4(0.8f, 0.8f, 0.8f, 1.0f)  // logger_debug
	},
	// [kThemeLight]
	{
		IM_COL32(0xF0, 0xF0, 0xF0, 0xFF), // tableLightColor
		IM_COL32(0xD2, 0xD2, 0xD2, 0xFF), // tableDarkColor
		IM_COL32(0x96, 0x96, 0x96, 0x96), // borderColor
		IM_COL32(0x1E, 0x1E, 0x1E, 0xFF), // sidebarTextColor
		IM_COL32(0x1E, 0x1E, 0x1E, 0xFF), // gridTextColor
		IM_COL32(0xC8, 0x32, 0x00, 0xFF), // playhead_color
		IM_COL32(0xFF, 0xFF, 0x00, 0x60), // current_statement_bg
		IM_COL32(0x00, 0x00, 0xB0, 0xFF), // channel_toggle
		IM_COL32(0xDC, 0x3C, 0x3C, 0xC8), // channel_hide_bg
		IM_COL32(0x94, 0x00, 0xD3, 0xFF), // channelSelectedCol
		IM_COL32(0xD0, 0x90, 0x00, 0x50), // channelHoveredCol
		{								 // contColors
			IM_COL32(0xCE, 0xCE, 0xFF, 0x80),
			IM_COL32(0xFF, 0xFF, 0xCE, 0x80),
			IM_COL32(0xCE, 0xFF, 0xCE, 0x80),
			IM_COL32(0xCE, 0xFF, 0xFF, 0x80),
			IM_COL32(0xFF, 0xCE, 0xFF, 0x80),
			IM_COL32(0xFF, 0xCE, 0x9C, 0x80)
		},

		ImVec4(0.9f, 0.08f, 0.0f, 0.0f), // bp_color_disabled
		ImVec4(0.9f, 0.08f, 0.0f, 1.0f), // bp_color_enabled
		ImVec4(0.6f, 0.05f, 0.0f, 1.0f), // bp_color_hover

		ImVec4(0.82f, 0.44f, 0.00f, 1.0f), // current_statement
		ImVec4(0.18f, 0.18f, 0.18f, 1.0f), // line_color
		ImVec4(0.00f, 0.00f, 0.00f, 1.0f), // call_color
		ImVec4(0.00f, 0.53f, 0.00f, 1.0f), // builtin_color
		ImVec4(0.00f, 0.00f, 0.00f, 1.0f), // var_color
		ImVec4(0.33f, 0.33f, 0.33f, 1.0f), // literal_color
		ImVec4(0.87f, 0.00f, 0.00f, 1.0f), // comment_color
		ImVec4(0.33f, 0.33f, 0.33f, 1.0f), // type_color
		ImVec4(0.00f, 0.00f, 0.93f, 1.0f), // keyword_color
		ImVec4(0.00f, 0.00f, 0.93f, 1.0f), // the_color

		ImVec4(0.00f, 0.20f, 0.80f, 1.0f), // script_ref
		ImVec4(0.72f, 0.53f, 0.00f, 1.0f), // var_ref
		ImVec4(0.80f, 0.00f, 0.00f, 1.0f), // var_ref_changed
		ImVec4(0.60f, 0.00f, 0.60f, 1.0f), // var_ref_out_of_scope

		ImVec4(0.1f, 0.1f, 0.1f, 1.0f),	// cp_color
		ImVec4(0.8f, 0.0f, 0.0f, 1.0f),	// cp_color_red
		ImVec4(0.8f, 0.4f, 0.0f, 1.0f),	// cp_active_color
		ImVec4(0.7f, 0.8f, 1.0f, 0.5f),	// cp_bgcolor
		ImVec4(0.0f, 0.0f, 0.8f, 1.0f),	// cp_playing_color
		ImVec4(0.55f, 0.42f, 0.05f, 1.0f), // cp_path_color

		ImVec4(0.8f, 0.0f, 0.0f, 1.0f),	// logger_error_b
		ImVec4(0.8f, 0.4f, 0.0f, 1.0f),	// logger_warning_b
		ImVec4(0.1f, 0.1f, 0.1f, 1.0f),	// logger_info_b
		ImVec4(0.4f, 0.4f, 0.4f, 1.0f),	// logger_debug_b
		ImVec4(0.7f, 0.0f, 0.0f, 1.0f),	// logger_error
		ImVec4(0.7f, 0.4f, 0.0f, 1.0f),	// logger_warning
		ImVec4(0.15f, 0.15f, 0.15f, 1.0f), // logger_info
		ImVec4(0.4f, 0.4f, 0.4f, 1.0f)	 // logger_debug
	}
};

static const char *themeNames[kThemeCount] = {
	"Dark",
	"Light"
};

void setTheme(int themeIndex) {
	if (!_state || themeIndex < 0 || themeIndex >= kThemeCount)
		return;

	_state->_activeThemeID = themeIndex;
	_state->theme = &themes[themeIndex];

	if (themeIndex == kThemeLight) {
		ImGui::StyleColorsLight();
	} else {
		ImGui::StyleColorsDark();
	}

	if (_state->_logger) {
		auto &lc = _state->_logger->_colors;
		lc._logger_error_b = _state->theme->logger_error_b;
		lc._logger_warning_b = _state->theme->logger_warning_b;
		lc._logger_info_b = _state->theme->logger_info_b;
		lc._logger_debug_b = _state->theme->logger_debug_b;
		lc._logger_error = _state->theme->logger_error;
		lc._logger_warning = _state->theme->logger_warning;
		lc._logger_info = _state->theme->logger_info;
		lc._logger_debug = _state->theme->logger_debug;
	}
}

static void showSettings() {
	if (!_state->_w.settings)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(480, 240), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Settings", &_state->_w.settings)) {
		ImGui::SeparatorText("Global Theme");
		if (ImGui::Combo("Theme", &_state->_activeThemeID, themeNames, kThemeCount)) {
			setTheme(_state->_activeThemeID);
		}

		_state->_logger->drawColorOptions();

		ImGui::SeparatorText("Debugger Behavior");
		ImGui::Checkbox("Ignore Mouse Events", &_state->_ignoreMouse);
		ImGui::SetItemTooltip("Block mouse events from reaching Director.\nHold SHIFT to temporarily allow them.\nPress Ctrl+F1 to toggle this setting.");

		ImGuiIO &io = ImGui::GetIO();
		if (ImGui::Checkbox("Enable Multi-Viewport", &_state->_enableMultiViewport)) {
			if (_state->_enableMultiViewport) {
				io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
			} else {
				io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
			}
		}
		ImGui::SetItemTooltip("When disabled, all debugger windows are forced to stay inside the main ScummVM window.");
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

	setTheme(_state->_activeThemeID);

	Common::setLogWatcher(onLog);
}

void onImGuiRender() {
	if (!debugChannelSet(-1, kDebugImGui)) {
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse;
		return;
	}

	if (!_state)
		return;

	if (_state->_windowToRedraw) {
		Graphics::ManagedSurface *surface = _state->_windowToRedraw->getSurface();
		if (surface) {
			Common::Rect fullScreen(0, 0, surface->w, surface->h);

			_state->_windowToRedraw->addDirtyRect(fullScreen);
			_state->_windowToRedraw->setDirty(true);
		}

		_state->_windowToRedraw = nullptr;
	}

	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags &= ~(ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse);

	if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_F1)) {
		_state->_ignoreMouse = !_state->_ignoreMouse;

		Common::String msg = Common::String::format("Debug Mouse Ignore: %s", _state->_ignoreMouse ? "ON" : "OFF");
		g_system->displayMessageOnOSD(Common::U32String(msg));
	}

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
			ImGui::MenuItem("Search", NULL, &_state->_w.search);
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
	showCastDetails();
	showFuncList();
	showScore();
	showSearchBar();
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

void setSelectedChannel(int channel) {
	if (_state) {
		_state->_selectedChannel = channel;

		if (channel > 0) {
			_state->_scrollToChannel = true; 
			_state->_w.channels = true;
		}
	}
}

bool isMouseInputIgnored() {
	if (!_state || !_state->_ignoreMouse)
		return false;

	// Holding Shift temporarily allows mouse events to pass to the engine
	ImGuiIO &io = ImGui::GetIO();
	if (io.KeyShift)
		return false;

	return true;
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
