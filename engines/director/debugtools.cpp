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

#define IMGUI_DEFINE_MATH_OPERATORS

#include "backends/imgui/imgui.h"
#include "backends/imgui/imgui_fonts.h"
#include "common/config-manager.h"
#include "graphics/surface.h"
#include "graphics/opengl/shader.h"
#include "image/png.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingodec/ast.h"
#include "director/lingo/lingodec/codewritervisitor.h"
#include "director/lingo/lingodec/context.h"
#include "director/lingo/lingodec/handler.h"
#include "director/lingo/lingodec/names.h"
#include "director/lingo/lingodec/resolver.h"
#include "director/lingo/lingodec/script.h"
#include "director/cast.h"
#include "director/castmember/bitmap.h"
#include "director/castmember/castmember.h"
#include "director/castmember/text.h"
#include "director/castmember/script.h"
#include "director/channel.h"
#include "director/debugtools.h"
#include "director/frame.h"
#include "director/movie.h"
#include "director/picture.h"
#include "director/score.h"
#include "director/sprite.h"
#include "director/types.h"

namespace Director {

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
	Common::Array<ImGuiScriptCodeLine> lingoCode;
	Common::Array<ImGuiScriptCodeLine> byteCode;

	bool operator==(const ImGuiScript &c) const {
		return moviePath == c.moviePath && score == c.score && id == c.id && type == c.type && handlerId == c.handlerId;
	}
	bool operator!=(const ImGuiScript &c) const {
		return !(*this == c);
	}
} ImGuiScript;

class ImGuiNodeVisitor : public LingoDec::NodeVisitor {
public:
	explicit ImGuiNodeVisitor(ImGuiScript &script) : _script(script) {}

	virtual void visit(const LingoDec::BlockNode &node) override {
		_indent++;
		for (const auto &child : node.children) {
			child->accept(*this);
		}
		_indent--;
	}

	virtual void visit(const LingoDec::HandlerNode &node) override {
		byteCode(node);

		if (node.handler->isGenericEvent) {
			node.block->accept(*this);
			return;
		}

		LingoDec::Script *script = node.handler->script;
		bool isMethod = script->isFactory();
		{
			Common::String code;
			if (isMethod) {
				code += "method ";
			} else {
				code += "on ";
			}
			code += node.handler->name;

			if (node.handler->argumentNames.size() > 0) {
				code += " ";
				for (size_t i = 0; i < node.handler->argumentNames.size(); i++) {
					if (i > 0)
						code += ", ";
					code += node.handler->argumentNames[i];
				}
			}
			write(node._startOffset, code);
		}

		{
			Common::String code;
			if (isMethod && node.handler->script->propertyNames.size() > 0 && node.handler == &node.handler->script->handlers[0]) {
				code += "instance ";
				for (size_t i = 0; i < node.handler->script->propertyNames.size(); i++) {
					if (i > 0)
						code += ", ";
					code += node.handler->script->propertyNames[i];
				}
				write(node._startOffset, code);
			}
		}

		{
			if (node.handler->globalNames.size() > 0) {
				Common::String code;
				code += "global ";
				for (size_t i = 0; i < node.handler->globalNames.size(); i++) {
					if (i > 0)
						code += ", ";
					code += node.handler->globalNames[i];
				}
				write(node._startOffset, code);
			}
		}

		node.block->accept(*this);

		{
			Common::String code;
			if (!isMethod) {
				code += "end";
			}
			write(node.block->_endOffset, code);
		}
	}

	virtual void visit(const LingoDec::RepeatWhileStmtNode &node) override {
		LingoDec::CodeWriterVisitor code(_dot, false);
		code.write("repeat while ");
		node.condition->accept(code);
		write(node._startOffset, code._str);

		node.block->accept(*this);

		write(node._endOffset, "end repeat");
	}

	virtual void visit(const LingoDec::RepeatWithInStmtNode &node) override {
		LingoDec::CodeWriterVisitor code(_dot, false);
		code.write("repeat with ");
		code.write(node.varName);
		code.write(" in ");
		node.list->accept(code);
		write(node._startOffset, code._str);
		node.block->accept(*this);
		write(node._endOffset, "end repeat");
	}

	virtual void visit(const LingoDec::RepeatWithToStmtNode &node) override {
		LingoDec::CodeWriterVisitor code(_dot, false);
		code.write("repeat with ");
		code.write(node.varName);
		code.write(" = ");
		node.start->accept(code);
		if (node.up) {
			code.write(" to ");
		} else {
			code.write(" down to ");
		}
		node.end->accept(code);
		write(node._startOffset, code._str);
		node.block->accept(*this);
		write(node._endOffset, "end repeat");
	}

	virtual void visit(const LingoDec::IfStmtNode &node) override {
		{
			LingoDec::CodeWriterVisitor code(_dot, false);
			code.write("if ");
			node.condition->accept(code);
			code.write(" then");
			write(node._startOffset, code._str);
		}
		node.block1->accept(*this);
		if (node.hasElse) {
			write(node.block2->_startOffset, "else");
			node.block2->accept(*this);
		}
		write(node._endOffset, "end if");
	}

	virtual void visit(const LingoDec::TellStmtNode &node) override {
		LingoDec::CodeWriterVisitor code(_dot, false);
		code.write("tell ");
		node.window->accept(*this);
		write(node._startOffset, code._str);
		node.block->accept(*this);
		write(node._endOffset, "end tell");
	}

	virtual void defaultVisit(const LingoDec::Node &node) override {
		LingoDec::CodeWriterVisitor code(_dot, false);
		node.accept(code);
		write(node._startOffset, code._str);
	}

private:
	void byteCode(const LingoDec::HandlerNode &node) {
		LingoDec::Handler *handler = node.handler;
		bool isMethod = handler->script->isFactory();

		if (!handler->isGenericEvent) {
			Common::String code;
			if (isMethod) {
				code += "method ";
			} else {
				code += "on ";
			}
			code += handler->name;
			if (handler->argumentNames.size() > 0) {
				code += " ";
				for (size_t i = 0; i < handler->argumentNames.size(); i++) {
					if (i > 0)
						code += ", ";
					code += handler->argumentNames[i];
				}
			}
			writeByteCode(0, code);
		}
		for (uint i = 0; i < handler->bytecodeArray.size(); i++) {
			LingoDec::CodeWriterVisitor code(_dot, true);
			code.indent();
			auto &bytecode = handler->bytecodeArray[i];
			code.write(LingoDec::StandardNames::getOpcodeName(bytecode.opID));
			switch (bytecode.opcode) {
			case LingoDec::kOpJmp:
			case LingoDec::kOpJmpIfZ:
				code.write(" ");
				code.write(posToString(bytecode.pos + bytecode.obj));
				break;
			case LingoDec::kOpEndRepeat:
				code.write(" ");
				code.write(posToString(bytecode.pos - bytecode.obj));
				break;
			case LingoDec::kOpPushFloat32:
				code.write(" ");
				code.write(Common::String::format("%g", (*(const float *)(&bytecode.obj))));
				break;
			default:
				if (bytecode.opID > 0x40) {
					code.write(" ");
					code.write(Common::String::format("%d", bytecode.obj));
				}
				break;
			}
			if (bytecode.translation) {
				code.write(" ...");
				while (code.lineWidth() < 49) {
					code.write(".");
				}
				code.write(" ");
				if (bytecode.translation->isExpression) {
					code.write("<");
				}
				bytecode.translation->accept(code);
				if (bytecode.translation->isExpression) {
					code.write(">");
				}
			}
			writeByteCode(bytecode.pos, code._str);
		}
		if (!handler->isGenericEvent) {
			if (!isMethod) {
				writeByteCode(node._endOffset, "end");
			}
		}
	}

	void write(uint32 offset, const Common::String &code) {
		Common::String s;
		for (int i = 0; i < _indent; i++) {
			s += "  ";
		}
		_script.lingoCode.push_back({offset, s + code});
	}

	void writeByteCode(uint32 offset, const Common::String &code) {
		Common::String s;
		for (int i = 0; i < _indent; i++) {
			s += "  ";
		}
		_script.byteCode.push_back({offset, s + code});
	}

	Common::String posToString(int32 pos) {
		return Common::String::format("[%3d]", pos);
	}

private:
	ImGuiScript &_script;
	bool _dot = false;
	int _indent = 0;
};

typedef struct ImGuiState {
	struct {
		Common::HashMap<Graphics::Surface *, ImGuiImage> _textures;
		bool _listView = true;
		int _thumbnailSize = 64;
		ImGuiTextFilter _nameFilter;
		int _typeFilter = 0x7FFF;
	} _cast;
	struct {
		ImGuiScript _script;
		ImGuiTextFilter _nameFilter;
		bool _showScript = false;
		bool _showByteCode = false;
	} _functions;
	bool _showControlPanel = true;
	bool _showCallStack = false;
	bool _showVars = false;
	bool _showChannels = false;
	bool _showCast = false;
	bool _showFuncList = false;
	bool _showScore = false;
	Common::List<CastMemberID> _scriptCasts;
	Common::HashMap<Common::String, bool, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _breakpoints;
	Common::HashMap<Common::String, bool, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _variables;
	int _prevFrame = -1;

	struct {
		int frame = -1;
		int channel = -1;
	} _selectedScoreCast;
} ImGuiState;

ImGuiState *_state = nullptr;

static void showControlPanel() {
	if (!_state->_showControlPanel)
		return;

	ImVec2 vp(ImGui::GetMainViewport()->Size);
	ImGui::SetNextWindowPos(ImVec2(vp.x - 220.0f, 20.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(200, 80), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Control Panel", &_state->_showControlPanel)) {
		Score *score = g_director->getCurrentMovie()->getScore();
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

			if (ImGui::IsItemClicked(0))
				score->_playState = kPlayPaused;

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

			if (ImGui::IsItemClicked(0))
				score->_playState = kPlayStarted;

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->AddTriangleFilled(ImVec2(p.x, p.y), ImVec2(p.x, p.y + 16), ImVec2(p.x + 14, p.y + 8), color);

			ImGui::SetItemTooltip("Play");
			ImGui::SameLine();
		}

		char buf[6];

		snprintf(buf, 6, "%d", score->getCurrentFrameNum());

		ImGui::SetNextItemWidth(30);
		ImGui::InputText("##frame", buf, 5, ImGuiInputTextFlags_CharsDecimal);
		ImGui::SetItemTooltip("Frame");

		ImGui::Separator();
		ImGui::Separator();
		ImGui::Text("Lingo:");
		ImGui::SameLine();
		{ // Step over
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Step Over", buttonSize);

			if (ImGui::IsItemClicked(0))
				score->_playState = kPlayStarted;

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

			if (ImGui::IsItemClicked(0))
				score->_playState = kPlayStarted;

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->AddLine(ImVec2(p.x + 8.5f, p.y + 1), ImVec2(p.x + 8.5f, p.y + 10), color_red, 2);
			dl->AddLine(ImVec2(p.x + 5.5f, p.y + 6), ImVec2(p.x + 8.5f, p.y + 9), color_red, 2);
			dl->AddLine(ImVec2(p.x + 12, p.y + 6),   ImVec2(p.x + 8.5f, p.y + 9), color_red, 2);
			dl->AddCircleFilled(ImVec2(p.x + 9, p.y + 15), 2.0f, color);

			ImGui::SetItemTooltip("Step Into");
			ImGui::SameLine();
		}

		{ // Step out
			p = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("Step Out", buttonSize);

			if (ImGui::IsItemClicked(0))
				score->_playState = kPlayStarted;

			if (ImGui::IsItemHovered())
				dl->AddRectFilled(ImVec2(p.x + bgX1, p.y + bgX1), ImVec2(p.x + bgX2, p.y + bgX2), bgcolor, 3.0f, ImDrawFlags_RoundCornersAll);

			dl->AddLine(ImVec2(p.x + 8.5f, p.y + 1), ImVec2(p.x + 8.5f, p.y + 10), color_red, 2);
			dl->AddLine(ImVec2(p.x + 5.5f, p.y + 5), ImVec2(p.x + 8.5f, p.y + 1), color_red, 2);
			dl->AddLine(ImVec2(p.x + 12, p.y + 5),   ImVec2(p.x + 8.5f, p.y + 1), color_red, 2);
			dl->AddCircleFilled(ImVec2(p.x + 9, p.y + 15), 2.0f, color);

			ImGui::SetItemTooltip("Step Out");
			ImGui::SameLine();
		}
	}
	ImGui::End();
}

static void showCallStack() {
	if (!_state->_showCallStack)
		return;

	Director::Lingo *lingo = g_director->getLingo();
	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(120, 120), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("CallStack", &_state->_showCallStack)) {
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
		"",       // Empty
		"\ue075", // Bitmap
		"\ue062", // FilmLoop
		"\ue0e1", // Text
		"\ue055", // Palette
		"\ue075", // Picture
		"\ue0f1", // Sound
		"\ue0cb", // Button
		"\ue058", // Shape
		"\ue0ee", // Movie
		"\ue062", // DigitalVideo
		"\ue0bc", // Script
		"",       // RTE
		"",       // ???
		""};      // Transition"
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

static ImGuiImage getImageID(CastMember *castMember) {
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

static void showImage(const ImGuiImage &image, const char *name, float thumbnailSize) {
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

static Common::String getDisplayName(CastMember *castMember) {
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
	if (!_state->_showCast)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(520, 240), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Cast", &_state->_showCast)) {
		// display a toolbar with: grid/list/filters buttons + name filter
		if (ImGui::Button("\ue07e")) {
			_state->_cast._listView = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("\ue06e")) {
			_state->_cast._listView = false;
		}
		ImGui::SameLine();

		if (ImGui::Button("\ue063")) {
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

static void displayVariable(Common::String &name) {
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
	ImVec2 eyeSize = ImGui::CalcTextSize("\ue05b ");
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

	dl->AddText(pos, color, "\ue05b ");
	dl->AddText(ImVec2(pos.x + eyeSize.x, pos.y), var_color, name.c_str());
}

static void showVars() {
	if (!_state->_showVars)
		return;

	Director::Lingo *lingo = g_director->getLingo();
	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Vars", &_state->_showVars)) {
		Common::Array<Common::String> keyBuffer;
		const ImVec4 head_color = ImVec4(0.9f, 0.08f, 0.0f, 1.0f);


		ImGui::TextColored(head_color, "Local vars:");
		if (lingo->_state->localVars) {
			for (auto &it : *lingo->_state->localVars) {
				keyBuffer.push_back(it._key);
			}
			Common::sort(keyBuffer.begin(), keyBuffer.end());
			for (auto &i : keyBuffer) {
				Datum &val = lingo->_state->localVars->getVal(i);
				displayVariable(i);
				ImGui::SameLine();
				ImGui::Text(" - [%s] %s", val.type2str(), formatStringForDump(val.asString(true)).c_str());
			}
			keyBuffer.clear();
		} else {
			ImGui::Text("  (no local vars)");
		}

		if (lingo->_state->me.type == OBJECT && lingo->_state->me.u.obj->getObjType() & (kFactoryObj | kScriptObj)) {
			ScriptContext *script = static_cast<ScriptContext *>(lingo->_state->me.u.obj);
			ImGui::TextColored(head_color, "Instance/property vars:");
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
		}

		ImGui::TextColored(head_color, "Global vars:");
		for (auto &it : lingo->_globalvars) {
			keyBuffer.push_back(it._key);
		}
		Common::sort(keyBuffer.begin(), keyBuffer.end());
		for (auto &i : keyBuffer) {
			Datum &val = lingo->_globalvars.getVal(i);
			displayVariable(i);
			ImGui::SameLine();
			ImGui::Text(" - [%s] %s", val.type2str(), formatStringForDump(val.asString(true)).c_str());
		}
		keyBuffer.clear();
	}
	ImGui::End();
}

static ImVec4 convertColor(uint32 color) {
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

static void setScriptToDisplay(const ImGuiScript &script) {
	_state->_functions._script = script;
	_state->_functions._showScript = true;
}

static void showChannels() {
	if (!_state->_showChannels)
		return;

	ImVec2 pos(40, 40);
	ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);

	ImVec2 windowSize = ImGui::GetMainViewport()->Size - pos - pos;
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Channels", &_state->_showChannels)) {
		Score *score = g_director->getCurrentMovie()->getScore();
		Frame &frame = *score->_currentFrame;

		CastMemberID defaultPalette = g_director->getCurrentMovie()->getCast()->_defaultPalette;
		ImGui::Text("TMPO:   tempo: %d, skipFrameFlag: %d, blend: %d, currentFPS: %d",
			frame._mainChannels.tempo, frame._mainChannels.skipFrameFlag, frame._mainChannels.blend, score->_currentFrameRate);
		if (!frame._mainChannels.palette.paletteId.isNull()) {
			ImGui::Text("PAL:    paletteId: %s, firstColor: %d, lastColor: %d, flags: %d, cycleCount: %d, speed: %d, frameCount: %d, fade: %d, delay: %d, style: %d, currentId: %s, defaultId: %s",
				frame._mainChannels.palette.paletteId.asString().c_str(), frame._mainChannels.palette.firstColor, frame._mainChannels.palette.lastColor, frame._mainChannels.palette.flags,
				frame._mainChannels.palette.cycleCount, frame._mainChannels.palette.speed, frame._mainChannels.palette.frameCount,
				frame._mainChannels.palette.fade, frame._mainChannels.palette.delay, frame._mainChannels.palette.style, g_director->_lastPalette.asString().c_str(), defaultPalette.asString().c_str());
		} else {
			ImGui::Text("PAL:    paletteId: 000, currentId: %s, defaultId: %s\n", g_director->_lastPalette.asString().c_str(), defaultPalette.asString().c_str());
		}
		ImGui::Text("TRAN:   transType: %d, transDuration: %d, transChunkSize: %d",
			frame._mainChannels.transType, frame._mainChannels.transDuration, frame._mainChannels.transChunkSize);
		ImGui::Text("SND: 1  sound1: %d, soundType1: %d", frame._mainChannels.sound1.member, frame._mainChannels.soundType1);
		ImGui::Text("SND: 2  sound2: %d, soundType2: %d", frame._mainChannels.sound2.member, frame._mainChannels.soundType2);
		ImGui::Text("LSCR:   actionId: %d", frame._mainChannels.actionId.member);

		if (ImGui::BeginTable("Channels", 21, ImGuiTableFlags_Borders)) {
			ImGuiTableFlags flags = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_AngledHeader;
			ImGui::TableSetupColumn("CH", flags);
			ImGui::TableSetupColumn("castId", flags);
			ImGui::TableSetupColumn("vis", flags);
			ImGui::TableSetupColumn("inkData", flags);
			ImGui::TableSetupColumn("ink", flags);
			ImGui::TableSetupColumn("trails", flags);
			ImGui::TableSetupColumn("stretch", flags);
			ImGui::TableSetupColumn("line", flags);
			ImGui::TableSetupColumn("dims", flags);
			ImGui::TableSetupColumn("type", flags);
			ImGui::TableSetupColumn("fg", flags);
			ImGui::TableSetupColumn("bg", flags);
			ImGui::TableSetupColumn("script", flags);
			ImGui::TableSetupColumn("colorcode", flags);
			ImGui::TableSetupColumn("blendAmount", flags);
			ImGui::TableSetupColumn("unk3", flags);
			ImGui::TableSetupColumn("constraint", flags);
			ImGui::TableSetupColumn("puppet", flags);
			ImGui::TableSetupColumn("moveable", flags);
			ImGui::TableSetupColumn("movieRate", flags);
			ImGui::TableSetupColumn("movieTime", flags);

			ImGui::TableAngledHeadersRow();
			for (int i = 0; i < frame._numChannels; i++) {
				Channel &channel = *score->_channels[i + 1];
				Sprite &sprite = *channel._sprite;

				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				ImGui::Text("%-3d", i + 1);
				ImGui::TableNextColumn();

				if (sprite._castId.member) {
					Common::Point position = channel.getPosition();
					ImGui::Text("%s", sprite._castId.asString().c_str());
					ImGui::TableNextColumn();
					ImGui::Checkbox("", &channel._visible);
					ImGui::TableNextColumn();
					ImGui::Text("0x%02x", sprite._inkData);
					ImGui::TableNextColumn();
					ImGui::Text("%d", sprite._ink);
					ImGui::TableNextColumn();
					ImGui::Checkbox("", &sprite._trails);
					ImGui::TableNextColumn();
					ImGui::Checkbox("", &sprite._stretch);
					ImGui::TableNextColumn();
					ImGui::Text("%d", sprite._thickness);
					ImGui::TableNextColumn();
					ImGui::Text("%dx%d@%d,%d", channel.getWidth(), channel.getHeight(), position.x, position.y);
					ImGui::TableNextColumn();
					ImGui::Text("%d (%s)", sprite._spriteType, spriteType2str(sprite._spriteType));
					ImGui::TableNextColumn();
					ImGui::Text("%3d", sprite._foreColor); ImGui::SameLine();
					ImGui::ColorButton("foreColor", convertColor(sprite._foreColor));
					ImGui::TableNextColumn();
					ImGui::Text("%3d", sprite._backColor); ImGui::SameLine();
					ImGui::ColorButton("backColor", convertColor(sprite._backColor));
					ImGui::TableNextColumn();
					if (sprite._scriptId.member) {
						ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "%s", sprite._scriptId.asString().c_str());

						if (ImGui::IsItemClicked(0))
							addScriptCastToDisplay(sprite._scriptId);
					}
					ImGui::TableNextColumn();
					ImGui::Text("0x%x", sprite._colorcode);
					ImGui::TableNextColumn();
					ImGui::Text("0x%x", sprite._blendAmount);
					ImGui::TableNextColumn();
					ImGui::Text("0x%x", sprite._unk3);
					ImGui::TableNextColumn();
					ImGui::Text("%d", channel._constraint);
					ImGui::TableNextColumn();
					ImGui::Checkbox("", &sprite._puppet);
					ImGui::TableNextColumn();
					ImGui::Checkbox("", &sprite._moveable);
					ImGui::TableNextColumn();
					if (channel._movieRate)
						ImGui::Text("%f", channel._movieRate);
					else
						ImGui::Text("0");
					ImGui::TableNextColumn();
					if (channel._movieRate)
						ImGui::Text("%d (%f)", channel._movieTime, (float)(channel._movieTime/60.0f));
					else
						ImGui::Text("0");
				} else {
					ImGui::Text("000");
				}
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();
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

	const ImU32 bp_color_disabled = ImGui::GetColorU32(ImVec4(0.9f, 0.08f, 0.0f, 0.0f));
	const ImU32 bp_color_enabled = ImGui::GetColorU32(ImVec4(0.9f, 0.08f, 0.0f, 1.0f));
	const ImU32 bp_color_hover = ImGui::GetColorU32(ImVec4(0.42f, 0.17f, 0.13f, 1.0f));
	const ImU32 line_color = ImGui::GetColorU32(ImVec4(0.44f, 0.44f, 0.44f, 1.0f));
	ImU32 color;

	uint pc = 0;
	while (pc < sym.u.defn->size()) {
		ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 mid(pos.x + 7, pos.y + 7);
		Common::String bpName = Common::String::format("%s-%d", handlerName.c_str(), pc);

		color = bp_color_disabled;

		if (_state->_breakpoints.contains(bpName))
			color = bp_color_enabled;

		ImGui::InvisibleButton("Line", ImVec2(16, ImGui::GetFontSize()));
		if (ImGui::IsItemClicked(0)) {
			if (color == bp_color_enabled) {
				_state->_breakpoints.erase(bpName);
				color = bp_color_disabled;
			} else {
				_state->_breakpoints[bpName] = true;
				color = bp_color_enabled;
			}
		}

		if (color == bp_color_disabled && ImGui::IsItemHovered()) {
			color = bp_color_hover;
		}

		dl->AddCircleFilled(mid, 4.0f, color);
		dl->AddLine(ImVec2(pos.x + 16.0f, pos.y), ImVec2(pos.x + 16.0f, pos.y + 17), line_color);

		ImGui::SetItemTooltip("Click to add a breakpoint");

		ImGui::SameLine();
		ImGui::Text("[%5d] ", pc);
		ImGui::SameLine();
		ImGui::Text("%s", lingo->decodeInstruction(sym.u.defn, pc, &pc).c_str());
	}
}

static void renderScript(ImGuiScript &script, bool showByteCode) {
	ImDrawList *dl = ImGui::GetWindowDrawList();

	const ImU32 bp_color_disabled = ImGui::GetColorU32(ImVec4(0.9f, 0.08f, 0.0f, 0.0f));
	const ImU32 bp_color_enabled = ImGui::GetColorU32(ImVec4(0.9f, 0.08f, 0.0f, 1.0f));
	const ImU32 bp_color_hover = ImGui::GetColorU32(ImVec4(0.42f, 0.17f, 0.13f, 1.0f));
	const ImU32 line_color = ImGui::GetColorU32(ImVec4(0.44f, 0.44f, 0.44f, 1.0f));
	ImU32 color;

	const Common::Array<ImGuiScriptCodeLine> &code = showByteCode ? script.byteCode : script.lingoCode;
	for (const auto& line : code) {
		ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 mid(pos.x + 7, pos.y + 7);
		Common::String bpName = Common::String::format("%s-%d", script.handlerId.c_str(), line.pc);

		color = bp_color_disabled;

		if (_state->_breakpoints.contains(bpName))
			color = bp_color_enabled;

		ImGui::InvisibleButton("Line", ImVec2(16, ImGui::GetFontSize()));
		if (ImGui::IsItemClicked(0)) {
			if (color == bp_color_enabled) {
				_state->_breakpoints.erase(bpName);
				color = bp_color_disabled;
			} else {
				_state->_breakpoints[bpName] = true;
				color = bp_color_enabled;
			}
		}

		if (color == bp_color_disabled && ImGui::IsItemHovered()) {
			color = bp_color_hover;
		}

		dl->AddCircleFilled(mid, 4.0f, color);
		dl->AddLine(ImVec2(pos.x + 16.0f, pos.y), ImVec2(pos.x + 16.0f, pos.y + 17), line_color);

		ImGui::SetItemTooltip("Click to add a breakpoint");

		ImGui::SameLine();
		ImGui::Text("[%5d] %s", line.pc, line.codeLine.c_str());
	}
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

static void displayScripts() {
	if (!_state->_functions._showScript)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(240, 240), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Script", &_state->_functions._showScript)) {
		if (ImGui::Button("\ue025")) { // Lingo
			_state->_functions._showByteCode = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("\ue079")) { // Bytecode
			_state->_functions._showByteCode = true;
		}
		ImGui::Separator();

		ImGui::Text("%s", _state->_functions._script.handlerName.c_str());
		renderScript(_state->_functions._script, _state->_functions._showByteCode);
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
	if (!_state->_showFuncList)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(480, 240), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Functions", &_state->_showFuncList)) {
		Lingo *lingo = g_director->getLingo();
		Movie *movie = g_director->getCurrentMovie();
		ScriptContext *csc = lingo->_state->context;

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

			if (csc) {
				Common::String scriptType(scriptType2str(csc->_scriptType));
				for (auto &functionHandler : csc->_functionHandlers) {
					Common::String function(g_lingo->formatFunctionName(functionHandler._value));
					if (!_state->_functions._nameFilter.PassFilter(function.c_str()))
						continue;

					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					if (ImGui::Selectable(function.c_str())) {
						ImGuiScript script;
						script.moviePath = movie->getArchive()->getPathName().toString();
						script.score = true;
						script.type = csc->_scriptType;
						script.handlerId = functionHandler._key;
						script.handlerName = getHandlerName(functionHandler._value);
						uint32 scriptId = movie->getCast()->getCastMemberInfo(csc->_id)->scriptId;
						const LingoDec::Script *s = movie->getCast()->_lingodec->scripts[scriptId];
						ImGuiNodeVisitor visitor(script);
						for (auto &h : s->handlers) {
							if (h.name != functionHandler._key)
								continue;
							h.ast.root->accept(visitor);
						}
						setScriptToDisplay(script);
					}
					ImGui::TableNextColumn();
					ImGui::Text("%s", movie->getArchive()->getPathName().toString().c_str());
					ImGui::TableNextColumn();
					ImGui::Text("-");
					ImGui::TableNextColumn();
					ImGui::Text("%s", scriptType.c_str());
				}
			}

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
								ImGuiScript script;
								script.moviePath = movie->getArchive()->getPathName().toString();
								script.id = memberID;
								script.type = (ScriptType)i;
								script.handlerId = functionHandler._key;
								script.handlerName = getHandlerName(functionHandler._value);
								uint32 scriptId = movie->getCastMemberInfo(memberID)->scriptId;
								const LingoDec::Script *s = cast._value->_lingodec->scripts[scriptId];
								ImGuiNodeVisitor visitor(script);
								for (auto &h : s->handlers) {
									if (h.name != functionHandler._key)
										continue;
									h.ast.root->accept(visitor);
								}
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
								ImGuiScript script;
								script.moviePath = movie->getArchive()->getPathName().toString();
								script.id = memberID;
								script.type = (ScriptType)i;
								script.handlerId = functionHandler._key;
								script.handlerName = getHandlerName(functionHandler._value);
								uint32 scriptId = sharedCast->getCastMemberInfo(scriptContext._key)->scriptId;
								const LingoDec::Script *s = sharedCast->_lingodec->scripts[scriptId];
								ImGuiNodeVisitor visitor(script);
								for (auto &h : s->handlers) {
									if (h.name != functionHandler._key)
										continue;
									h.ast.root->accept(visitor);
								}
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

static void showScore() {
	if (!_state->_showScore)
		return;

	ImVec2 pos(40, 40);
	ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);

	ImVec2 windowSize = ImGui::GetMainViewport()->Size - pos - pos;
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Score", &_state->_showScore)) {
		Score *score = g_director->getCurrentMovie()->getScore();
		uint numFrames = score->_scoreCache.size();
		Cast *cast = g_director->getCurrentMovie()->getCast();

		if (!numFrames) {
			ImGui::Text("No frames");
			ImGui::End();

			return;
		}

		{
			Sprite *sprite = nullptr;

			if (_state->_selectedScoreCast.frame != -1)
				sprite = score->_scoreCache[_state->_selectedScoreCast.frame]->_sprites[_state->_selectedScoreCast.channel];

			if (sprite) {
				CastMember *castMember = cast->getCastMember(sprite->_castId.member, true);

				ImGuiImage imgID = getImageID(castMember);
				if (imgID.id) {
					Common::String name(getDisplayName(castMember));
					showImage(imgID, name.c_str(), 32.f);
				} else {
					ImGui::InvisibleButton("##canvas", ImVec2(32.f, 32.f));
				}
			}
		}

		uint numChannels = score->_scoreCache[0]->_sprites.size();
		uint tableColumns = MAX(numFrames, 25U); // Set minimal table width to 25

		uint currentFrameNum = score->getCurrentFrameNum();
		ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.7f, 0.7f, 0.0f, 0.65f));

		if (ImGui::BeginTable("Score", tableColumns + 1,
					ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedSame |
					ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg)) {
			ImGuiTableFlags flags = ImGuiTableColumnFlags_WidthFixed;

			ImGui::TableSetupColumn("##", flags);
			for (uint i = 0; i < tableColumns; i++)
				ImGui::TableSetupColumn(Common::String::format("%-2d", i + 1).c_str(), flags);

			ImGui::TableHeadersRow();
			for (int ch = 0; ch < (int)numChannels - 1; ch++) {
				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				ImGui::Text("%-3d", ch + 1);

				for (uint f = 0; f < numFrames; f++) {
					Sprite &sprite = *score->_scoreCache[f]->_sprites[ch + 1];

					if (f == currentFrameNum)
						ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);

					ImGui::TableNextColumn();

					if (sprite._castId.member) {
						ImGui::Text("%d", sprite._castId.member);

						if (ImGui::IsItemClicked(0)) {
							_state->_selectedScoreCast.frame = f;
							_state->_selectedScoreCast.channel = ch + 1;
						}
					}
				}
			}
			ImGui::EndTable();
		}
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
	icons_config.GlyphOffset = {0, 3};

	static const ImWchar icons_ranges[] = {0xE000, 0xF8FF, 0};
	ImGui::addTTFFontFromArchive("OpenFontIcons.ttf", 13.f, &icons_config, icons_ranges);

	_state = new ImGuiState();
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

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("View")) {
			ImGui::MenuItem("Control Panel", NULL, &_state->_showControlPanel);
			ImGui::MenuItem("CallStack", NULL, &_state->_showCallStack);
			ImGui::MenuItem("Vars", NULL, &_state->_showVars);
			ImGui::MenuItem("Channels", NULL, &_state->_showChannels);
			ImGui::MenuItem("Cast", NULL, &_state->_showCast);
			ImGui::MenuItem("Functions", NULL, &_state->_showFuncList);
			ImGui::MenuItem("Score", NULL, &_state->_showScore);
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
}

void onImGuiCleanup() {
	delete _state;
	_state = nullptr;
}
} // namespace Director
