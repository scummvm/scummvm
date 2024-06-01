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

#include "graphics/opengl/shader.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingodec/context.h"
#include "director/lingo/lingodec/script.h"
#include "director/cast.h"
#include "director/castmember/bitmap.h"
#include "director/castmember/text.h"
#include "director/castmember/script.h"
#include "director/debugger.h"
#include "director/movie.h"
#include "director/picture.h"
#include "director/score.h"

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

static uint32 getLineFromPC() {
	const uint pc = g_lingo->_state->pc;
	if (_state->_functions._scripts.empty())
		return 0;
	const Common::Array<uint> &offsets = _state->_functions._scripts[_state->_functions._current].startOffsets;
	for (uint i = 0; i < offsets.size(); i++) {
		if (pc <= offsets[i])
			return i;
	}
	return 0;
}

static bool stepOverShouldPauseDebugger() {
	const uint32 line = getLineFromPC();

	// we stop when we are :
	// - in the same callstack level and the statement line is different
	// - OR we go up in the callstack
	if (((g_lingo->_state->callstack.size() == _state->_dbg._callstackSize) && (line != _state->_dbg._lastLinePC)) ||
		 (g_lingo->_state->callstack.size() < _state->_dbg._callstackSize)) {
		_state->_dbg._lastLinePC = line;
		return true;
	}

	return false;
}

static bool stepInShouldPauseDebugger() {
	const uint32 line = getLineFromPC();

	// we stop when:
	// - the statement line is different
	// - OR when the callstack level change
	if ((g_lingo->_state->callstack.size() != _state->_dbg._callstackSize) || (_state->_dbg._lastLinePC != line)) {
		_state->_dbg._lastLinePC = line;
		return true;
	}
	return false;
}

static bool stepOutShouldPause() {
	const uint32 line = getLineFromPC();

	// we stop when:
	// - the statement line is different
	// - OR we go up in the callstack
	if (g_lingo->_state->callstack.size() < _state->_dbg._callstackSize) {
		_state->_dbg._lastLinePC = line;
		return true;
	}

	return false;
}

static void dgbStop() {
	g_lingo->_exec._state = kPause;
	g_lingo->_exec._shouldPause = nullptr;
	_state->_dbg._isScriptDirty = true;
}

static void dbgStepOver() {
	g_lingo->_exec._state = kRunning;
	_state->_dbg._lastLinePC = getLineFromPC();
	_state->_dbg._callstackSize = g_lingo->_state->callstack.size();
	g_lingo->_exec._shouldPause = stepOverShouldPauseDebugger;
	_state->_dbg._isScriptDirty = true;
}

static void dbgStepInto() {
	g_lingo->_exec._state = kRunning;
	_state->_dbg._lastLinePC = getLineFromPC();
	_state->_dbg._callstackSize = g_lingo->_state->callstack.size();
	g_lingo->_exec._shouldPause = stepInShouldPauseDebugger;
	_state->_dbg._isScriptDirty = true;
}

static void dbgStepOut() {
	g_lingo->_exec._state = kRunning;
	_state->_dbg._lastLinePC = getLineFromPC();
	_state->_dbg._callstackSize = g_lingo->_state->callstack.size();
	g_lingo->_exec._shouldPause = stepOutShouldPause;
	_state->_dbg._isScriptDirty = true;
}

static void showControlPanel() {
	if (!_state->_w.controlPanel)
		return;

	ImVec2 vp(ImGui::GetMainViewport()->Size);
	ImGui::SetNextWindowPos(ImVec2(vp.x - 220.0f, 20.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(200, 103), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Control Panel", &_state->_w.controlPanel)) {
		Movie *movie = g_director->getCurrentMovie();
		Score *score = movie->getScore();
		ImDrawList *dl = ImGui::GetWindowDrawList();

		ImU32 color = ImGui::GetColorU32(ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
		ImU32 color_red = ImGui::GetColorU32(ImVec4(1.0f, 0.6f, 0.6f, 1.0f));
		ImU32 active_color = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 0.4f, 1.0f));
		ImU32 bgcolor = ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 1.0f, 1.0f));
		ImVec2 p = ImGui::GetCursorScreenPos();
		ImVec2 buttonSize(20, 14);
		float bgX1 = -4.0f, bgX2 = 21.0f;

		int frameNum = score->getCurrentFrameNum();

		if (_state->_prevFrame != -1 && _state->_prevFrame != frameNum) {
			score->_playState = kPlayPaused;
			_state->_prevFrame = -1;
		}

		{ // Rewind
			ImGui::InvisibleButton("Rewind", buttonSize);

			if (ImGui::IsItemClicked(0)) {
				score->_playState = kPlayStarted;
				score->setCurrentFrame(1);
			}

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->AddTriangleFilled(ImVec2(p.x, p.y + 8), ImVec2(p.x + 8, p.y), ImVec2(p.x + 8, p.y + 16), color);
			dl->AddTriangleFilled(ImVec2(p.x + 8, p.y + 8), ImVec2(p.x + 16, p.y), ImVec2(p.x + 16, p.y + 16), color);

			ImGui::SetItemTooltip("Rewind");
			ImGui::SameLine();
		}

		{ // Step Back
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Step Back", ImVec2(18, 16));

			if (ImGui::IsItemClicked(0)) {
				score->_playState = kPlayStarted;

				score->setCurrentFrame(frameNum - 1);
				_state->_prevFrame = frameNum;
			}

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->AddTriangleFilled(ImVec2(p.x, p.y + 8), ImVec2(p.x + 9, p.y), ImVec2(p.x + 9, p.y + 16), color);
			dl->AddRectFilled(ImVec2(p.x + 11, p.y), ImVec2(p.x + 17, p.y + 16), color);

			ImGui::SetItemTooltip("Step Back");
			ImGui::SameLine();
		}

		{ // Stop
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Stop", buttonSize);

			if (ImGui::IsItemClicked(0)) {
				score->_playState = kPlayPaused;
				g_lingo->_exec._state = kPause;
				g_lingo->_exec._shouldPause = nullptr;
				_state->_dbg._isScriptDirty = true;
			}

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			ImU32 stopColor = (score->_playState == kPlayPaused) ? active_color : color;
			dl->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 16, p.y + 16), stopColor);

			ImGui::SetItemTooltip("Stop");
			ImGui::SameLine();
		}

		{ // Step
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Step", buttonSize);

			if (ImGui::IsItemClicked(0)) {
				score->_playState = kPlayStarted;

				score->setCurrentFrame(frameNum + 1);
				_state->_prevFrame = frameNum;
			}

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 6, p.y + 16), color);
			dl->AddTriangleFilled(ImVec2(p.x + 8, p.y + 2), ImVec2(p.x + 8, p.y + 14), ImVec2(p.x + 16, p.y + 8), color);

			ImGui::SetItemTooltip("Step");
			ImGui::SameLine();
		}

		{ // Play
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Play", buttonSize);

			if (ImGui::IsItemClicked(0)) {
				score->_playState = kPlayStarted;
				g_lingo->_exec._state = kRunning;
				g_lingo->_exec._shouldPause = nullptr;
			}

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->AddTriangleFilled(ImVec2(p.x, p.y), ImVec2(p.x, p.y + 16), ImVec2(p.x + 14, p.y + 8), color);

			ImGui::SetItemTooltip("Play");
			ImGui::SameLine();
		}

		char buf[6];

		snprintf(buf, 6, "%d", score->getCurrentFrameNum());

		ImGui::SetNextItemWidth(35);
		ImGui::InputText("##frame", buf, 5, ImGuiInputTextFlags_CharsDecimal);
		ImGui::SetItemTooltip("Frame");

		{
			ImGui::Separator();
			ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.5f, 1.0f), movie->getArchive()->getPathName().toString().c_str());
			ImGui::SetItemTooltip(movie->getArchive()->getPathName().toString().c_str());
		}

		ImGui::Separator();
		ImGui::Separator();
		ImGui::Text("Lingo:");
		ImGui::SameLine();
		{ // Step over
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Step Over", buttonSize);

			if (ImGui::IsItemClicked(0)) {
				score->_playState = kPlayStarted;
				if (g_lingo->_exec._state == kRunning) {
					dgbStop();
				} else {
					dbgStepOver();
				}
			}

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->PathArcToFast(ImVec2(p.x + 9, p.y + 15), 10.0f, 7, 11);
			dl->PathStroke(color_red, 0, 2);
			dl->AddLine(ImVec2(p.x + 18, p.y + 5), ImVec2(p.x + 18, p.y + 10), color_red, 2);
			dl->AddLine(ImVec2(p.x + 14, p.y + 10), ImVec2(p.x + 18, p.y + 10), color_red, 2);
			dl->AddCircleFilled(ImVec2(p.x + 9, p.y + 15), 2.0f, color);

			ImGui::SetItemTooltip("Step Over");
			ImGui::SameLine();
		}

		{ // Step into
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Step Into", buttonSize);

			if (ImGui::IsItemClicked(0)) {
				score->_playState = kPlayStarted;
				if (g_lingo->_exec._state == kRunning) {
					dgbStop();
				} else {
					dbgStepInto();
				}
			}

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->AddLine(ImVec2(p.x + 8.5f, p.y + 1), ImVec2(p.x + 8.5f, p.y + 10), color_red, 2);
			dl->AddLine(ImVec2(p.x + 5.5f, p.y + 6), ImVec2(p.x + 8.5f, p.y + 9), color_red, 2);
			dl->AddLine(ImVec2(p.x + 12, p.y + 6), ImVec2(p.x + 8.5f, p.y + 9), color_red, 2);
			dl->AddCircleFilled(ImVec2(p.x + 9, p.y + 15), 2.0f, color);

			ImGui::SetItemTooltip("Step Into");
			ImGui::SameLine();
		}

		{ // Step out
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Step Out", buttonSize);

			if (ImGui::IsItemClicked(0)) {
				score->_playState = kPlayStarted;
				if (g_lingo->_exec._state == kRunning) {
					dgbStop();
				} else {
					dbgStepOut();
				}
			}

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->AddLine(ImVec2(p.x + 8.5f, p.y + 1), ImVec2(p.x + 8.5f, p.y + 10), color_red, 2);
			dl->AddLine(ImVec2(p.x + 5.5f, p.y + 5), ImVec2(p.x + 8.5f, p.y + 1), color_red, 2);
			dl->AddLine(ImVec2(p.x + 12, p.y + 5), ImVec2(p.x + 8.5f, p.y + 1), color_red, 2);
			dl->AddCircleFilled(ImVec2(p.x + 9, p.y + 15), 2.0f, color);

			ImGui::SetItemTooltip("Step Out");
		}
	}
	ImGui::End();
}

static void showCallStack() {
	if (!_state->_w.callStack)
		return;

	Director::Lingo *lingo = g_director->getLingo();
	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(120, 120), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("CallStack", &_state->_w.callStack)) {
		ImGui::Text("%s", lingo->formatCallStack(lingo->_state->pc).c_str());
	}
	ImGui::End();
}

static GLuint loadTextureFromSurface(Graphics::Surface *surface, const byte *palette, int palCount) {

	// Create a OpenGL texture identifier
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

	// Upload pixels into texture
	Graphics::Surface *s = surface->convertTo(Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0), palette, palCount);
	glPixelStorei(GL_UNPACK_ALIGNMENT, s->format.bytesPerPixel);

	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, s->w, s->h, 0, GL_RGB, GL_UNSIGNED_BYTE, s->getPixels()));
	s->free();
	delete s;
	return image_texture;
}

static const char *toString(ScriptType scriptType) {
	static const char *scriptTypes[] = {
		"Score",
		"Cast",
		"Movie",
		"Event",
		"Test",
		"???",
		"???",
		"Parent",
	};
	if (scriptType < 0 || scriptType > kMaxScriptType)
		return "???";
	return scriptTypes[(int)scriptType];
}

static const char *toIcon(CastType castType) {
	static const char *castTypes[] = {
		"",        // Empty
		"\uf79e",  // Bitmap		// backround_dot_large
		"\ue8da",  // FilmLoop		// theaters
		"\uf6f1",  // Text			// match_case
		"\ue40a",  // Palette		// palette
		"\uefa2",  // Picture		// imagesmode
		"\ue050",  // Sound			// volume_up
		"\uf4ab",  // Button		// slab_serif
		"\ue602",  // Shape			// shapes
		"\ue02c",  // Movie			// movie
		"\uf49a",  // DigitalVideo	// animated_images
		"\uf0c8",  // Script		// forms_apps_script
		"\uf4f1",  // RTE			// brand_family
		"?",       // ???
		"\uf50c"}; // Transition	// transition_fade
	if (castType < 0 || castType > kCastTransition)
		return "";
	return castTypes[(int)castType];
}

static const char *toString(CastType castType) {
	static const char *castTypes[] = {
		"Empty",
		"Bitmap",
		"FilmLoop",
		"Text",
		"Palette",
		"Picture",
		"Sound",
		"Button",
		"Shape",
		"Movie",
		"DigitalVideo",
		"Script",
		"RTE",
		"???",
		"Transition"};
	if (castType < 0 || castType > kCastTransition)
		return "???";
	return castTypes[(int)castType];
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

	ImTextureID textureID = (ImTextureID)(intptr_t)loadTextureFromSurface(&pic->_surface, pic->_palette, pic->_paletteColors);
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

Common::String getDisplayName(CastMember *castMember) {
	const CastMemberInfo *castMemberInfo = castMember->getInfo();
	Common::String name(castMemberInfo ? castMemberInfo->name : "");
	if (!name.empty())
		return name;
	if (castMember->_type == kCastText) {
		TextCastMember *textCastMember = (TextCastMember *)castMember;
		return textCastMember->getText();
	}
	return Common::String::format("%u", castMember->getID());
}

static void showCast() {
	if (!_state->_w.cast)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(520, 240), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Cast", &_state->_w.cast)) {
		// display a toolbar with: grid/list/filters buttons + name filter
		toggleButton("\ue896", &_state->_cast._listView); // list
		ImGui::SetItemTooltip("List");
		ImGui::SameLine();
		toggleButton("\ue9b0", &_state->_cast._listView, true); // grid_view
		ImGui::SetItemTooltip("Grid");
		ImGui::SameLine();

		if (ImGui::Button("\uef4f")) { // filter_alt
			ImGui::OpenPopup("filters_popup");
		}
		ImGui::SameLine();

		if (ImGui::BeginPopup("filters_popup")) {
			ImGui::CheckboxFlags("All", &_state->_cast._typeFilter, 0x7FFF);
			ImGui::Separator();
			for (int i = 0; i <= 14; i++) {
				ImGui::PushID(i);
				Common::String option(Common::String::format("%s %s", toIcon((CastType)i), toString((CastType)i)));
				ImGui::CheckboxFlags(option.c_str(), &_state->_cast._typeFilter, 1 << i);
				ImGui::PopID();
			}
			ImGui::EndPopup();
		}
		_state->_cast._nameFilter.Draw();
		ImGui::Separator();

		// display a list or a grid
		const float sliderHeight = _state->_cast._listView ? 0.f : 38.f;
		const ImVec2 childsize = ImGui::GetContentRegionAvail();
		Movie *movie = g_director->getCurrentMovie();
		ImGui::BeginChild("##cast", ImVec2(childsize.x, childsize.y - sliderHeight));
		if (_state->_cast._listView) {
			if (ImGui::BeginTable("Resources", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
				ImGui::TableSetupColumn("Name", 0, 120.f);
				ImGui::TableSetupColumn("#", 0, 20.f);
				ImGui::TableSetupColumn("Script", 0, 80.f);
				ImGui::TableSetupColumn("Type", 0, 80.f);
				ImGui::TableSetupColumn("Preview", 0, 32.f);
				ImGui::TableHeadersRow();

				for (auto it : *movie->getCasts()) {
					Cast *cast = it._value;
					if (!cast->_loadedCast)
						continue;

					for (auto castMember : *cast->_loadedCast) {
						if (!castMember._value->isLoaded())
							continue;

						Common::String name(getDisplayName(castMember._value));
						if (!_state->_cast._nameFilter.PassFilter(name.c_str()))
							continue;
						if ((castMember._value->_type != kCastTypeAny) && !(_state->_cast._typeFilter & (1 << (int)castMember._value->_type)))
							continue;

						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("%s %s", toIcon(castMember._value->_type), name.c_str());

						ImGui::TableNextColumn();
						ImGui::Text("%d", castMember._key);

						ImGui::TableNextColumn();
						if (castMember._value->_type == CastType::kCastLingoScript) {
							ScriptCastMember *scriptMember = (ScriptCastMember *)castMember._value;
							ImGui::Text("%s", toString(scriptMember->_scriptType));
						}
						ImGui::TableNextColumn();
						ImGui::Text("%s", toString(castMember._value->_type));

						ImGui::TableNextColumn();
						ImGuiImage imgID = getImageID(castMember._value);
						if (imgID.id) {
							showImage(imgID, name.c_str(), 32.f);
						}
					}
				}

				ImGui::EndTable();
			}
		} else {
			const float thumbnailSize = (float)_state->_cast._thumbnailSize;
			const float contentWidth = ImGui::GetContentRegionAvail().x;
			int columns = contentWidth / (thumbnailSize + 8.f);
			columns = columns < 1 ? 1 : columns;
			if (ImGui::BeginTable("Cast", columns)) {
				for (auto it : *movie->getCasts()) {
					const Cast *cast = it._value;
					if (!cast->_loadedCast)
						continue;

					for (auto castMember : *cast->_loadedCast) {
						if (!castMember._value->isLoaded())
							continue;

						Common::String name(getDisplayName(castMember._value));
						if (!_state->_cast._nameFilter.PassFilter(name.c_str()))
							continue;
						if ((castMember._value->_type != kCastTypeAny) && !(_state->_cast._typeFilter & (1 << (int)castMember._value->_type)))
							continue;

						ImGui::TableNextColumn();

						ImGui::BeginGroup();
						const ImVec2 textSize = ImGui::CalcTextSize(name.c_str());
						float textWidth = textSize.x;
						float textHeight = textSize.y;
						if (textWidth > thumbnailSize) {
							textWidth = thumbnailSize;
							textHeight *= (textSize.x / textWidth);
						}

						ImGuiImage imgID = getImageID(castMember._value);
						if (imgID.id) {
							showImage(imgID, name.c_str(), thumbnailSize);
						} else {
							ImGui::PushID(castMember._key);
							ImGui::InvisibleButton("##canvas", ImVec2(thumbnailSize, thumbnailSize));
							ImGui::PopID();
							const ImVec2 p0 = ImGui::GetItemRectMin();
							const ImVec2 p1 = ImGui::GetItemRectMax();
							ImGui::PushClipRect(p0, p1, true);
							ImDrawList *draw_list = ImGui::GetWindowDrawList();
							draw_list->AddRect(p0, p1, IM_COL32_WHITE);
							const ImVec2 pos = p0 + ImVec2((thumbnailSize - textWidth) * 0.5f, (thumbnailSize - textHeight) * 0.5f);
							draw_list->AddText(nullptr, 0.f, pos, IM_COL32_WHITE, name.c_str(), 0, thumbnailSize);
							draw_list->AddText(nullptr, 0.f, p1 - ImVec2(16, 16), IM_COL32_WHITE, toIcon(castMember._value->_type));
							ImGui::PopClipRect();
						}
						ImGui::EndGroup();
					}
				}
				ImGui::EndTable();
			}
		}
		ImGui::EndChild();

		// in the footer display a slider for the grid view: thumbnail size
		if (!_state->_cast._listView) {
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::SliderInt("Thumbnail Size", &_state->_cast._thumbnailSize, 32, 256);
		}
	}
	ImGui::End();
}

static void displayVariable(const Common::String &name) {
	const ImU32 var_color = ImGui::GetColorU32(ImVec4(0.9f, 0.9f, 0.0f, 1.0f));

	const ImU32 disp_color_disabled = ImGui::GetColorU32(ImVec4(0.9f, 0.08f, 0.0f, 0.0f));
	const ImU32 disp_color_enabled = ImGui::GetColorU32(ImVec4(0.9f, 0.08f, 0.0f, 1.0f));
	const ImU32 disp_color_hover = ImGui::GetColorU32(ImVec4(0.42f, 0.17f, 0.13f, 1.0f));
	ImU32 color;

	color = disp_color_disabled;

	if (_state->_variables.contains(name))
		color = disp_color_enabled;

	ImDrawList *dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 eyeSize = ImGui::CalcTextSize("\ue8f4 ");	// visibility
	ImVec2 textSize = ImGui::CalcTextSize(name.c_str());

	ImGui::InvisibleButton("Line", ImVec2(textSize.x + eyeSize.x, textSize.y));
	if (ImGui::IsItemClicked(0)) {
		if (color == disp_color_enabled) {
			_state->_variables.erase(name);
			color = disp_color_disabled;
		} else {
			_state->_variables[name] = true;
			color = disp_color_enabled;
		}
	}

	if (color == disp_color_disabled && ImGui::IsItemHovered()) {
		color = disp_color_hover;
	}

	dl->AddText(pos, color, "\ue8f4 ");	// visibility
	dl->AddText(ImVec2(pos.x + eyeSize.x, pos.y), var_color, name.c_str());
}

static void showVars() {
	if (!_state->_w.vars)
		return;

	// take a snapshot of the variables every 500 ms
	if ((g_director->getTotalPlayTime() - _state->_vars._lastTimeRefreshed) > 500) {
		if (g_lingo->_state->localVars) {
			_state->_vars._locals = *g_lingo->_state->localVars;
		} else {
			_state->_vars._locals.clear();
		}

		_state->_vars._globals = g_lingo->_globalvars;
		_state->_vars._lastTimeRefreshed = g_director->getTotalPlayTime();
	}

	Director::Lingo *lingo = g_director->getLingo();
	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Vars", &_state->_w.vars)) {
		Common::Array<Common::String> keyBuffer;

		if (ImGui::CollapsingHeader("Global vars:", ImGuiTreeNodeFlags_DefaultOpen)) {
			for (auto &it : _state->_vars._globals) {
				keyBuffer.push_back(it._key);
			}
			Common::sort(keyBuffer.begin(), keyBuffer.end());
			for (auto &i : keyBuffer) {
				Datum &val = _state->_vars._globals.getVal(i);
				displayVariable(i);
				ImGui::SameLine();
				ImGui::Text(" - [%s] %s", val.type2str(), formatStringForDump(val.asString(true)).c_str());
			}
			keyBuffer.clear();
		}
		if (ImGui::CollapsingHeader("Local vars:", ImGuiTreeNodeFlags_None)) {
			if (!_state->_vars._locals.empty()) {
				for (auto &it : _state->_vars._locals) {
					keyBuffer.push_back(it._key);
				}
				Common::sort(keyBuffer.begin(), keyBuffer.end());
				for (auto &i : keyBuffer) {
					Datum &val = _state->_vars._locals.getVal(i);
					displayVariable(i);
					ImGui::SameLine();
					ImGui::Text(" - [%s] %s", val.type2str(), formatStringForDump(val.asString(true)).c_str());
				}
				keyBuffer.clear();
			} else {
				ImGui::Text("  (no local vars)");
			}
		}
		if (ImGui::CollapsingHeader("Instance/property vars:", ImGuiTreeNodeFlags_None)) {
			if (lingo->_state->me.type == OBJECT && lingo->_state->me.u.obj->getObjType() & (kFactoryObj | kScriptObj)) {
				ScriptContext *script = static_cast<ScriptContext *>(lingo->_state->me.u.obj);
				for (uint32 i = 1; i <= script->getPropCount(); i++) {
					keyBuffer.push_back(script->getPropAt(i));
				}
				Common::sort(keyBuffer.begin(), keyBuffer.end());
				for (auto &i : keyBuffer) {
					Datum val = script->getProp(i);
					displayVariable(i);
					ImGui::SameLine();
					ImGui::Text(" - [%s] %s", val.type2str(), formatStringForDump(val.asString(true)).c_str());
				}
				keyBuffer.clear();
			} else {
				ImGui::Text("  (no instance or property)");
			}
		}
	}
	ImGui::End();
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
		ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "%d", scriptId.member);

		ImGui::SetItemTooltip(scriptId.asString().c_str());

		if (ImGui::IsItemClicked(0))
			addScriptCastToDisplay(scriptId);
	} else {
		ImGui::Selectable("  ");
	}
}

static void renderCastScript(Symbol &sym) {
	if (sym.type != HANDLER)
		return;

	Director::Lingo *lingo = g_director->getLingo();
	Common::String handlerName;

	if (sym.ctx && sym.ctx->_id)
		handlerName = Common::String::format("%d:", sym.ctx->_id);

	handlerName += lingo->formatFunctionName(sym);

	ImGui::Text("%s", handlerName.c_str());

	ImDrawList *dl = ImGui::GetWindowDrawList();

	ImVec4 color;

	uint pc = 0;
	while (pc < sym.u.defn->size()) {
		ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 mid(pos.x + 7, pos.y + 7);
		Common::String bpName = Common::String::format("%s-%d", handlerName.c_str(), pc);

		color = _state->_colors._bp_color_disabled;

		Director::Breakpoint *bp = getBreakpoint(handlerName, sym.ctx->_id, pc);
		if (bp)
			color = _state->_colors._bp_color_enabled;

		ImGui::InvisibleButton("Line", ImVec2(16, ImGui::GetFontSize()));
		if (ImGui::IsItemClicked(0)) {
			if (bp) {
				g_lingo->delBreakpoint(bp->id);
				color = _state->_colors._bp_color_disabled;
			} else {
				Director::Breakpoint newBp;
				newBp.type = kBreakpointFunction;
				newBp.funcName = handlerName;
				newBp.funcOffset = pc;
				g_lingo->addBreakpoint(newBp);
				color = _state->_colors._bp_color_enabled;
			}
		}

		if (color == _state->_colors._bp_color_disabled && ImGui::IsItemHovered()) {
			color = _state->_colors._bp_color_hover;
		}

		dl->AddCircleFilled(mid, 4.0f, ImColor(color));
		dl->AddLine(ImVec2(pos.x + 16.0f, pos.y), ImVec2(pos.x + 16.0f, pos.y + 17), ImColor(_state->_colors._line_color));

		ImGui::SetItemTooltip("Click to add a breakpoint");

		ImGui::SameLine();
		ImGui::Text("[%5d] ", pc);
		ImGui::SameLine();
		ImGui::Text("%s", lingo->decodeInstruction(sym.u.defn, pc, &pc).c_str());
	}
}

static void renderScript(ImGuiScript &script, bool showByteCode) {
	if (script.oldAst) {
		renderOldScriptAST(script, showByteCode);
		_state->_dbg._isScriptDirty = false;
		return;
	}

	if (!script.root)
		return;

	renderScriptAST(script, showByteCode);
	_state->_dbg._isScriptDirty = false;
}

static bool showScriptCast(CastMemberID &id) {
	Common::String wName("Script ");
	wName += id.asString();

	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(240, 240), ImGuiCond_FirstUseEver);

	bool closed = true;

	if (ImGui::Begin(wName.c_str(), &closed)) {
		Cast *cast = g_director->getCurrentMovie()->getCasts()->getVal(id.castLib);
		ScriptContext *ctx = g_director->getCurrentMovie()->getScriptContext(kScoreScript, id);

		if (ctx) {
			for (auto &handler : ctx->_functionHandlers)
				renderCastScript(handler._value);
		} else if (cast->_lingoArchive->factoryContexts.contains(id.member)) {
			for (auto &it : *cast->_lingoArchive->factoryContexts.getVal(id.member)) {
				for (auto &handler : it._value->_functionHandlers)
					renderCastScript(handler._value);
			}
		} else {
			ImGui::Text("[Nothing]");
		}
	}
	ImGui::End();

	if (!closed)
		return false;

	return true;
}

/**
 * Display all open scripts
 */
static void displayScriptCasts() {
	if (_state->_scriptCasts.empty())
		return;

	for (Common::List<CastMemberID>::iterator scr = _state->_scriptCasts.begin(); scr != _state->_scriptCasts.end();) {
		if (!showScriptCast(*scr))
			scr = _state->_scriptCasts.erase(scr);
		else
			scr++;
	}
}

static void updateCurrentScript() {
	if ((g_lingo->_exec._state != kPause) || !_state->_dbg._isScriptDirty)
		return;

	Common::Array<CFrame *> &callstack = g_lingo->_state->callstack;
	if (callstack.empty())
		return;

	// show current script of the current stack frame
	CFrame *head = callstack[callstack.size() - 1];
	Director::Movie *movie = g_director->getCurrentMovie();
	ScriptContext *scriptContext = head->sp.ctx;
	int castLibID = movie->getCast()->_castLibID;
	ImGuiScript script = toImGuiScript(scriptContext->_scriptType, CastMemberID(head->sp.ctx->_id, castLibID), *head->sp.name);
	script.byteOffsets = scriptContext->_functionByteOffsets[script.handlerId];
	script.moviePath = movie->getArchive()->getPathName().toString();
	script.handlerName = head->sp.ctx->_id ? Common::String::format("%d:%s", head->sp.ctx->_id, script.handlerId.c_str()) : script.handlerId;
	setScriptToDisplay(script);
}

static void displayScripts() {
	updateCurrentScript();

	if (!_state->_functions._showScript)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(240, 240), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Script", &_state->_functions._showScript)) {
		ImGui::BeginDisabled(_state->_functions._scripts.empty() || _state->_functions._current == 0);
		if (ImGui::Button("\ue5c4")) { // Backward	// arrow_back
			_state->_functions._current--;
		}
		ImGui::EndDisabled();
		ImGui::SetItemTooltip("Backward");
		ImGui::SameLine();

		ImGui::BeginDisabled(_state->_functions._current >= _state->_functions._scripts.size() - 1);
		if (ImGui::Button("\ue5c8")) { // Forward	// arrow_forward
			_state->_functions._current++;
		}
		ImGui::EndDisabled();
		ImGui::SetItemTooltip("Forward");
		ImGui::SameLine();

		const char *currentScript = nullptr;
		if (_state->_functions._current < _state->_functions._scripts.size()) {
			currentScript = _state->_functions._scripts[_state->_functions._current].handlerName.c_str();
		}

		if (ImGui::BeginCombo("##handlers", currentScript)) {
			for (uint i = 0; i < _state->_functions._scripts.size(); i++) {
				auto &script = _state->_functions._scripts[i];
				bool selected = i == _state->_functions._current;
				if (ImGui::Selectable(script.handlerName.c_str(), &selected)) {
					_state->_functions._current = i;
				}
			}
			ImGui::EndCombo();
		}

		if (!_state->_functions._scripts[_state->_functions._current].oldAst) {
			ImGui::SameLine(0, 20);
			toggleButton("\uf569", &_state->_functions._showByteCode, true); // Lingo		// package_2
			ImGui::SetItemTooltip("Lingo");
			ImGui::SameLine();

			toggleButton("\uf500", &_state->_functions._showByteCode); // Bytecode	// stacks
			ImGui::SetItemTooltip("Bytecode");
		}

		ImGui::Separator();
		const ImVec2 childsize = ImGui::GetContentRegionAvail();
		ImGui::BeginChild("##script", childsize);
		ImGuiScript &script = _state->_functions._scripts[_state->_functions._current];
		renderScript(script, _state->_functions._showByteCode);
		ImGui::EndChild();
	}
	ImGui::End();
}

static Common::String getHandlerName(Symbol &sym) {
	Common::String handlerName;

	if (sym.ctx && sym.ctx->_id)
		handlerName = Common::String::format("%d:", sym.ctx->_id);
	handlerName += g_lingo->formatFunctionName(sym);

	return handlerName;
}

static void showFuncList() {
	if (!_state->_w.funcList)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(480, 240), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Functions", &_state->_w.funcList)) {
		_state->_functions._nameFilter.Draw();
		ImGui::Separator();
		const ImVec2 childSize = ImGui::GetContentRegionAvail();
		ImGui::BeginChild("##functions", ImVec2(childSize.x, childSize.y));

		if (ImGui::BeginTable("Functions", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
			ImGui::TableSetupColumn("Function", 0, 240.f);
			ImGui::TableSetupColumn("Movie", 0, 60.f);
			ImGui::TableSetupColumn("Cast", 0, 60.f);
			ImGui::TableSetupColumn("Type", 0, 80.f);
			ImGui::TableHeadersRow();

			Movie *movie = g_director->getCurrentMovie();
			for (auto &cast : *movie->getCasts()) {
				for (int i = 0; i <= kMaxScriptType; i++) {
					if (cast._value->_lingoArchive->scriptContexts[i].empty())
						continue;
					Common::String scriptType(scriptType2str((ScriptType)i));
					for (auto &scriptContext : cast._value->_lingoArchive->scriptContexts[i]) {
						Common::String name = Common::String::format("%d", scriptContext._key);
						CastMemberInfo *cmi = cast._value->getCastMemberInfo(scriptContext._key);
						if (cmi && !cmi->name.empty()) {
							name += Common::String::format(" \"%s\"", cmi->name.c_str());
						}
						for (auto &functionHandler : scriptContext._value->_functionHandlers) {
							Common::String function = Common::String::format("%s: %s", name.c_str(), g_lingo->formatFunctionName(functionHandler._value).c_str());
							if (!_state->_functions._nameFilter.PassFilter(function.c_str()))
								continue;

							ImGui::TableNextRow();
							ImGui::TableNextColumn();
							if (ImGui::Selectable(function.c_str())) {
								CastMemberID memberID(scriptContext._key, cast._key);
								ImGuiScript script = toImGuiScript(scriptContext._value->_scriptType, memberID, functionHandler._key);
								script.byteOffsets = scriptContext._value->_functionByteOffsets[script.handlerId];
								script.moviePath = movie->getArchive()->getPathName().toString();
								script.handlerName = getHandlerName(functionHandler._value);
								setScriptToDisplay(script);
							}
							ImGui::TableNextColumn();
							ImGui::Text("%s", movie->getArchive()->getPathName().toString().c_str());
							ImGui::TableNextColumn();
							ImGui::Text("%d", cast._key);
							ImGui::TableNextColumn();
							ImGui::Text("%s", scriptType.c_str());
						}
					}
				}
			}

			Cast *sharedCast = movie->getSharedCast();
			if (sharedCast && sharedCast->_lingoArchive) {
				for (int i = 0; i <= kMaxScriptType; i++) {
					if (sharedCast->_lingoArchive->scriptContexts[i].empty())
						continue;
					Common::String scriptType(scriptType2str((ScriptType)i));
					for (auto &scriptContext : sharedCast->_lingoArchive->scriptContexts[i]) {
						Common::String name = Common::String::format("%d", scriptContext._key);
						CastMemberInfo *cmi = sharedCast->getCastMemberInfo(scriptContext._key);
						if (cmi && !cmi->name.empty()) {
							name += Common::String::format(" \"%s\"", cmi->name.c_str());
						}
						for (auto &functionHandler : scriptContext._value->_functionHandlers) {
							Common::String function = Common::String::format("%s: %s", name.c_str(), g_lingo->formatFunctionName(functionHandler._value).c_str());
							if (!_state->_functions._nameFilter.PassFilter(function.c_str()))
								continue;

							ImGui::TableNextRow();
							ImGui::TableNextColumn();
							if (ImGui::Selectable(function.c_str())) {
								CastMemberID memberID(scriptContext._key, SHARED_CAST_LIB);
								ImGuiScript script = toImGuiScript(scriptContext._value->_scriptType, memberID, functionHandler._key);
								script.byteOffsets = scriptContext._value->_functionByteOffsets[script.handlerId];
								script.moviePath = movie->getArchive()->getPathName().toString();
								script.handlerName = getHandlerName(functionHandler._value);
								setScriptToDisplay(script);
							}
							ImGui::TableNextColumn();
							ImGui::Text("%s", movie->getArchive()->getPathName().toString().c_str());
							ImGui::TableNextColumn();
							ImGui::Text("SHARED");
							ImGui::TableNextColumn();
							ImGui::Text("%s", scriptType.c_str());
						}
					}
				}
			}
			ImGui::EndTable();
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

// Make the UI compact because there are so many fields
static void PushStyleCompact() {
	ImGuiStyle &style = ImGui::GetStyle();
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, (float)(int)(style.FramePadding.y * 0.60f)));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, (float)(int)(style.ItemSpacing.y * 0.60f)));
}

static void PopStyleCompact() {
	ImGui::PopStyleVar(2);
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
		ImGui::Separator();
		ImGui::ColorEdit4("Line", &_state->_colors._line_color.x);
		ImGui::ColorEdit4("Call", &_state->_colors._call_color.x);
		ImGui::ColorEdit4("Builtin", &_state->_colors._builtin_color.x);
		ImGui::ColorEdit4("Variable", &_state->_colors._var_color.x);
		ImGui::ColorEdit4("Literal", &_state->_colors._literal_color.x);
		ImGui::ColorEdit4("Comment", &_state->_colors._comment_color.x);
		ImGui::ColorEdit4("Type", &_state->_colors._type_color.x);
		ImGui::ColorEdit4("Keyword", &_state->_colors._keyword_color.x);
		ImGui::ColorEdit4("The entity", &_state->_colors._the_color.x);
	}
	ImGui::End();
}

static void showBreakpointList() {
	if (!_state->_w.bpList)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(480, 240), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Breakpoints", &_state->_w.bpList)) {
		auto &bps = g_lingo->getBreakpoints();
		if (ImGui::BeginTable("BreakpointsTable", 5, ImGuiTableFlags_SizingFixedFit)) {
			for (uint i = 0; i < 5; i++)
				ImGui::TableSetupColumn(NULL, i == 2 ? ImGuiTableColumnFlags_WidthStretch : ImGuiTableColumnFlags_NoHeaderWidth);

			for (uint i = 0; i < bps.size(); i++) {
				if (bps[i].type != kBreakpointFunction)
					continue;

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImDrawList *dl = ImGui::GetWindowDrawList();
				ImVec2 pos = ImGui::GetCursorScreenPos();
				const ImVec2 mid(pos.x + 7, pos.y + 7);

				ImVec4 color = bps[i].enabled ? _state->_colors._bp_color_enabled : _state->_colors._bp_color_disabled;
				ImGui::InvisibleButton("Line", ImVec2(16, ImGui::GetFontSize()));
				if (ImGui::IsItemClicked(0)) {
					if (bps[i].enabled) {
						bps[i].enabled = false;
						color = _state->_colors._bp_color_disabled;
					} else {
						bps[i].enabled = true;
						color = _state->_colors._bp_color_enabled;
					}
				}

				if (!bps[i].enabled && ImGui::IsItemHovered()) {
					color = _state->_colors._bp_color_hover;
				}

				if (bps[i].enabled)
					dl->AddCircleFilled(mid, 4.0f, ImColor(color));
				else
					dl->AddCircle(mid, 4.0f, ImColor(_state->_colors._line_color));

				// enabled column
				ImGui::TableNextColumn();
				PushStyleCompact();
				ImGui::PushID(i);
				ImGui::Checkbox("", &bps[i].enabled);
				PopStyleCompact();

				// description
				ImGui::TableNextColumn();
				Common::String desc;
				if (bps[i].scriptId)
					desc = Common::String::format("%d: %s", bps[i].scriptId, bps[i].funcName.c_str());
				else
					desc = bps[i].funcName;
				ImGui::Text("%s", desc.c_str());

				// remove bp
				ImGui::TableNextColumn();
				pos = ImGui::GetCursorScreenPos();
				const bool del = ImGui::InvisibleButton("DelBp", ImVec2(16, ImGui::GetFontSize()));
				const bool hovered = ImGui::IsItemHovered();
				const float fontSize = ImGui::GetFontSize();
				const float cross_extent = ImGui::GetFontSize() * 0.5f * 0.7071f - 1.0f;
				const ImU32 cross_col = ImGui::GetColorU32(ImGuiCol_Text);
				const ImVec2 center = pos + ImVec2(0.5f + fontSize * 0.5f, 1.0f + fontSize * 0.5f);
				if (hovered)
					dl->AddCircleFilled(center, MAX(2.0f, fontSize * 0.5f + 1.0f), ImGui::GetColorU32(ImGuiCol_ButtonActive));
				dl->AddLine(center + ImVec2(+cross_extent, +cross_extent), center + ImVec2(-cross_extent, -cross_extent), cross_col, 1.0f);
				dl->AddLine(center + ImVec2(+cross_extent, -cross_extent), center + ImVec2(-cross_extent, +cross_extent), cross_col, 1.0f);

				// offset
				ImGui::TableNextColumn();
				ImGui::Text("%d", bps[i].funcOffset);
				ImGui::PopID();

				if (del) {
					g_lingo->delBreakpoint(bps[i].id);
					break;
				}
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();
}

static void showArchive() {
	if (!_state->_w.archive)
		return;

	ImVec2 pos(40, 40);
	ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);

	ImVec2 windowSize = ImGui::GetMainViewport()->Size * 0.8;
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Archive", &_state->_w.archive)) {
		{ // Left pane
			ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.3f, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None);

			for (auto &it : g_director->_allSeenResFiles) {
				Archive *archive = it._value;

				if (ImGui::TreeNode(archive->getPathName().toString().c_str())) {
					Common::Array<uint32> typeList = archive->getResourceTypeList();
					Common::sort(typeList.begin(), typeList.end());

					for (auto tag : typeList) {
						if (ImGui::TreeNode((void*)(intptr_t)tag, "%s", tag2str(tag))) {
							Common::Array<uint16> resList = archive->getResourceIDList(tag);
							Common::sort(resList.begin(), resList.end());

							for (auto id : resList) {
								if (ImGui::Selectable(Common::String::format("%d", id).c_str())) {
									_state->_archive.path = it._key;
									_state->_archive.resType = tag;
									_state->_archive.resId = id;

									free(_state->_archive.data);

									Common::SeekableReadStreamEndian *res = archive->getResource(tag, id);
									_state->_archive.data = (byte *)malloc(res->size());
									res->read(_state->_archive.data, res->size());
									_state->_archive.dataSize = res->size();

									delete res;
								}
							}

							ImGui::TreePop();
						}
					}

					ImGui::TreePop();
				}
			}

			ImGui::EndChild();
		}

		ImGui::SameLine();

		{ // Right pane
			ImGui::BeginChild("ChildR", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_Border);

			ImGui::Text("Resource %s %d (%d bytes)", tag2str(_state->_archive.resType), _state->_archive.resId, _state->_archive.dataSize);

			ImGui::Separator();

			if (!_state->_archive.path.empty())
				_state->_archive.memEdit.DrawContents(_state->_archive.data, _state->_archive.dataSize);

			ImGui::EndChild();
		}

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

static void showWatchedVars() {
	if (!_state->_w.watchedVars)
		return;

	if ((g_director->getTotalPlayTime() - _state->_vars._lastTimeRefreshed) > 500) {
		if (g_lingo->_state->localVars) {
			_state->_vars._locals = *g_lingo->_state->localVars;
		} else {
			_state->_vars._locals.clear();
		}

		_state->_vars._globals = g_lingo->_globalvars;
		_state->_vars._lastTimeRefreshed = g_director->getTotalPlayTime();
	}

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Watched Vars", &_state->_w.watchedVars)) {
		for (auto &v : _state->_variables) {
			Datum name(v._key);
			name.type = VARREF;
			Datum val = g_lingo->varFetch(name, true);

			displayVariable(v._key);
			ImGui::SameLine();
			ImGui::Text(" - [%s] %s", val.type2str(), formatStringForDump(val.asString(true)).c_str());
		}

		if (_state->_variables.empty())
			ImGui::Text("(no watched variables)");
	}
	ImGui::End();
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

	static const ImWchar icons_ranges[] = {0xE000, 0xF8FF, 0};
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

	displayScriptCasts();
	displayScripts();

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
		delete _state->_tinyFont;
		free(_state->_archive.data);

		delete _state->_logger;
	}

	delete _state;
	_state = nullptr;
}

} // namespace DT
} // namespace Director
