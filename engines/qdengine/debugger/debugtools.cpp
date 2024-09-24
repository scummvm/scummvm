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
#define IMGUI_DEFINE_MATH_OPERATORS

#include "backends/imgui/imgui.h"
#include "backends/imgui/imgui_fonts.h"

#include "common/archive.h"
#include "common/compression/unzip.h"
#include "common/debug.h"
#include "common/path.h"
#include "common/stack.h"
#include "common/system.h"

#include "graphics/opengl/shader.h"
#include "graphics/managed_surface.h"

#include "qdengine/debugger/dt-internal.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_animation.h"
#include "qdengine/qdcore/qd_animation_frame.h"
#include "qdengine/qdcore/qd_file_manager.h"
#include "qdengine/qdengine.h"
#include "qdengine/system/graphics/gr_dispatcher.h"

namespace QDEngine {

const int TILES_ID = -1337;

ImGuiState *_state = nullptr;

static GLuint loadTextureFromSurface(Graphics::Surface *surface) {
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
	Graphics::Surface *s = surface->convertTo(Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0));
	glPixelStorei(GL_UNPACK_ALIGNMENT, s->format.bytesPerPixel);

	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, s->w, s->h, 0, GL_RGB, GL_UNSIGNED_BYTE, s->getPixels()));
	s->free();
	delete s;
	return image_texture;
}

ImGuiImage getImageID(Common::Path filename, int frameNum) {
	Common::String key = Common::String::format("%s:%d", filename.toString().c_str(), frameNum);

	if (_state->_frames.contains(key))
		return _state->_frames[key];

	int sx = 10, sy = 10;
	Graphics::ManagedSurface *surface = nullptr;

	if (_state->_displayMode == kDisplayQDA) {
		// Load the animation
		qdAnimation *animation = new qdAnimation();
		animation->qda_load(filename);

		_state->_qdaToDisplayFrameCount = animation->num_frames();

		if (frameNum == TILES_ID) {
			if (animation->tileAnimation()) {
				surface = animation->tileAnimation()->dumpTiles(25);

				sx = surface->w;
				sy = surface->h;
			}
		} else if (frameNum < 0) { // Tiles
			if (animation->tileAnimation()) {
				surface = animation->tileAnimation()->dumpFrameTiles(-frameNum + 1, 0.91670f);

				sx = surface->w;
				sy = surface->h;
			}
		} else {
			if (animation->tileAnimation()) {
				Vect2i size = animation->tileAnimation()->frameSize();

				sx = size.x;
				sy = size.y;
			} else {
				qdAnimationFrame *frame = animation->get_frame(frameNum);

				if (frame) {
					sx = frame->size_x();
					sy = frame->size_y();
				}
			}

			surface = new Graphics::ManagedSurface(sx, sy, g_engine->_pixelformat);

			animation->set_cur_frame(frameNum);

			grDispatcher::instance()->surfaceOverride(surface);
			animation->redraw(sx / 2, sy / 2, 0, 0.91670f, 0);
			grDispatcher::instance()->resetSurfaceOverride();
		}
		delete animation;
	} else if (_state->_displayMode == kDisplayTGA) {
		qdSprite *sprite = new qdSprite();
		if (sprite->load(filename)) {
			sx = sprite->size_x();
			sy = sprite->size_y();

			surface = new Graphics::ManagedSurface(sx, sy, g_engine->_pixelformat);

			grDispatcher::instance()->surfaceOverride(surface);
			sprite->redraw(sx / 2, sy / 2, 0);
			grDispatcher::instance()->resetSurfaceOverride();
		} else {
			warning("Error loading TGA file '%s'", transCyrillic(filename.toString()));
		}
		delete sprite;
	}

	if (surface)
		_state->_frames[key] = { (ImTextureID)(intptr_t)loadTextureFromSurface(surface->surfacePtr()), sx, sy };

	delete surface;

	return _state->_frames[key];
}

void showImage(const ImGuiImage &image, const char *name, float scale) {
	ImVec2 size = { (float)image.width * scale, (float)image.height * scale };

	ImGui::BeginGroup();
	ImVec2 screenPos = ImGui::GetCursorScreenPos();
	ImGui::GetWindowDrawList()->AddRect(screenPos, screenPos + ImVec2(size.x, size.y), 0xFFFFFFFF);

	ImGui::Image(image.id, size);
	ImGui::EndGroup();
	//setToolTipImage(image, name);
}

FileTree::FileTree(Common::Path *p, Common::String n, bool node, int i) {
	id = i;

	if (!node) {
		name = (char *)transCyrillic(n);
	} else {
		path = *p;
		name = (char *)transCyrillic(p->baseName());
	}
}

void populateFileList() {
	Common::Array<Common::Path> files;

	// Iterate through the 3 resource pak files
	for (int i = 0; i < qdFileManager::instance().get_num_packages(); i++) {
		Common::Archive *archive = qdFileManager::instance().get_package(i);
		Common::ArchiveMemberList members;

		if (archive)
			archive->listMembers(members);

		for (auto &it : members)
			files.push_back(it->getPathInArchive());
	}

	Common::sort(files.begin(), files.end());

	// Now build a tree
	Common::Path curr;
	Common::Stack<FileTree *> treeStack;

	_state->_files.name = "Resource";
	treeStack.push(&_state->_files);

	int id = 0;
	for (int f = 0; f < files.size(); f++) {
		// Skip duplicates between the archives
		if (f && files[f] == files[f - 1])
			continue;

		Common::Path parent = files[f].getParent();

		if (parent != curr) {
			Common::StringArray curArr = curr.splitComponents();
			Common::StringArray newArr = parent.splitComponents();

			if (curArr.back().empty())
				curArr.pop_back();

			if (newArr.back().empty())
				newArr.pop_back();

			int pos = 0;
			while (pos < curArr.size() && pos < newArr.size() && curArr[pos] == newArr[pos])
				pos++;

			// if we need to close directories
			if (pos < curArr.size()) {
				for (int i = pos; i < curArr.size(); i++)
					(void)treeStack.pop();
			}

			for (; pos < newArr.size(); pos++) {
				if (id == 0 && newArr[pos] == "Resource") // Skip the root node
					continue;

				treeStack.top()->children.push_back(new FileTree(nullptr, newArr[pos], false, ++id));
				treeStack.push(treeStack.top()->children.back());
			}

			curr = parent;
		}

		treeStack.top()->children.push_back(new FileTree(&files[f], "", true, ++id));

		id++;
	}
}

static void displayQDA() {
	int totalFrames = _state->_qdaToDisplayFrameCount;

	ImGuiImage imgID;
	if (!_state->_fileToDisplay.empty()) {
		imgID = getImageID(_state->_fileToDisplay, _state->_qdaToDisplayFrame);

		ImGui::Text("Frame %s: %d of %d  [%d x %d]", transCyrillic(_state->_fileToDisplay.toString()), _state->_qdaToDisplayFrame + 1,
				totalFrames, imgID.width, imgID.height);
	} else {
		ImGui::Text("Frame <none>");
	}

	ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
	if (ImGui::BeginTabBar("FrameTabBar", tab_bar_flags)) {

		if (ImGui::BeginTabItem("Animation")) {

			if (ImGui::Button(ICON_MS_FAST_REWIND)) {
				_state->_qdaToDisplayFrame = 0;
				_state->_qdaIsPlaying = false;
			}

			ImGui::SameLine();
			if (ImGui::Button(ICON_MS_SKIP_PREVIOUS)) {
				_state->_qdaToDisplayFrame = _state->_qdaToDisplayFrame + totalFrames - 1;
				_state->_qdaToDisplayFrame %= totalFrames;
			}
			ImGui::SameLine();
			if (ImGui::Button(ICON_MS_PLAY_ARROW))
				_state->_qdaIsPlaying = !_state->_qdaIsPlaying;

			ImGui::SameLine();
			if (ImGui::Button(ICON_MS_SKIP_NEXT)) {
				_state->_qdaToDisplayFrame += 1;
				_state->_qdaToDisplayFrame %= totalFrames;
			}

			ImGui::SameLine();
			if (ImGui::Button(ICON_MS_FAST_FORWARD)) {
				_state->_qdaToDisplayFrame = totalFrames - 1;
				_state->_qdaIsPlaying = false;
			}

			ImGui::SameLine();

			// Frame Count
			char buf[6];
			snprintf(buf, 6, "%d", _state->_qdaToDisplayFrame);

			ImGui::SetNextItemWidth(35);
			ImGui::InputText("##frame", buf, 5, ImGuiInputTextFlags_CharsDecimal);
			ImGui::SetItemTooltip("Frame");

			ImGui::Separator();

			if (!_state->_fileToDisplay.empty()) {
				showImage(imgID, (char *)transCyrillic(_state->_fileToDisplay.toString()), 1.0);
			} else {
				ImGui::InvisibleButton("##canvas", ImVec2(32.f, 32.f));
			}

			ImGui::SameLine();

			imgID = getImageID(_state->_fileToDisplay, -_state->_qdaToDisplayFrame - 1);

			showImage(imgID, (char *)transCyrillic(_state->_fileToDisplay.toString()), 1.0);

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Tiles")) {
			if (!_state->_fileToDisplay.empty()) {
				imgID = getImageID(_state->_fileToDisplay, TILES_ID);

				showImage(imgID, (char *)transCyrillic(_state->_fileToDisplay.toString()), 1.0);
			} else {
				ImGui::InvisibleButton("##canvas", ImVec2(32.f, 32.f));
			}

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

static void displayTGA() {
	ImGuiImage imgID;

	imgID = getImageID(_state->_fileToDisplay, 0);

	ImGui::Text("TGA %s: [%d x %d]", transCyrillic(_state->_fileToDisplay.toString()), imgID.width, imgID.height);

	ImGui::Separator();

	showImage(imgID, (char *)transCyrillic(_state->_fileToDisplay.toString()), 1.0);
}

void displayTree(FileTree *tree) {
	if (tree->children.empty()) { // It is a file
		if (ImGui::Selectable(tree->name.c_str(), _state->_fileToDisplay == tree->path)) {
			_state->_fileToDisplay = tree->path;

			if (tree->name.hasSuffixIgnoreCase(".qda")) {
				_state->_qdaToDisplayFrame = 0;
				_state->_qdaIsPlaying = false;

				_state->_displayMode = kDisplayQDA;
			} else if (tree->name.hasSuffixIgnoreCase(".tga")) {
				_state->_displayMode = kDisplayTGA;
			} else {
				_state->_displayMode = -1;
			}
		}

		return;
	}

	if (ImGui::TreeNode((void*)(intptr_t)(tree->id), tree->name.c_str())) {
		for (auto &it : tree->children)
			displayTree(it);

		ImGui::TreePop();
	}
}

void showArchives() {
	if (!_state->_showArchives)
		return;

	// Calculate the viewport size
	ImVec2 viewportSize = ImGui::GetMainViewport()->Size;

	// Calculate the window size
	ImVec2 windowSize = ImVec2(
		viewportSize.x * 0.9f,
		viewportSize.y * 0.9f
	);

	// Calculate the centered position
	ImVec2 centeredPosition = ImVec2(
		(viewportSize.x - windowSize.x) * 0.5f,
		(viewportSize.y - windowSize.y) * 0.5f
	);

	// Set the next window position and size
	ImGui::SetNextWindowPos(centeredPosition, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Archives", &_state->_showArchives)) {
		ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.4f, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None);

		ImGui::Button(ICON_MS_FILTER_ALT);
		ImGui::SameLine();

		_state->_nameFilter.Draw();
		ImGui::Separator();

		if (_state->_files.children.empty())
			populateFileList();

		displayTree(&_state->_files);

		ImGui::EndChild();

		ImGui::SameLine();

		{ // Right pane
			ImGui::BeginChild("ChildR", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_Border);

			if (_state->_displayMode == kDisplayQDA) {
				displayQDA();
			} else if (_state->_displayMode == kDisplayTGA) {
				displayTGA();
			}

			ImGui::EndChild();
		}

	}
	ImGui::End();
}

void onImGuiInit() {
	ImGuiIO &io = ImGui::GetIO();
	io.Fonts->AddFontDefault();

	static const ImWchar cyrillic_ranges[] = {
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0400, 0x04FF, // Cyrillic
		0
	};

	io.FontDefault = ImGui::addTTFFontFromArchive("FreeSans.ttf", 16.0f, nullptr, cyrillic_ranges);;

	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = false;
	icons_config.OversampleH = 3;
	icons_config.OversampleV = 3;
	icons_config.GlyphOffset = {0, 4};

	static const ImWchar icons_ranges[] = {ICON_MIN_MS, ICON_MAX_MS, 0};
	ImGui::addTTFFontFromArchive("MaterialSymbolsSharp.ttf", 16.f, &icons_config, icons_ranges);

	_state = new ImGuiState();
}

void onImGuiRender() {
	if (!debugChannelSet(-1, kDebugImGui)) {
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse;
		return;
	}

	if (!_state)
		return;

	if (_state->_qdaIsPlaying && g_system->getMillis() > _state->_qdaNextFrameTimestamp) {
		_state->_qdaToDisplayFrame++;
		_state->_qdaToDisplayFrame %= _state->_qdaToDisplayFrameCount;

		_state->_qdaNextFrameTimestamp = g_system->getMillis() + 50; // 20 fps
	}

	ImGui::GetIO().ConfigFlags &= ~(ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse);

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("View")) {
			ImGui::SeparatorText("Windows");

			ImGui::MenuItem("Archives", NULL, &_state->_showArchives);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	showArchives();
}

void onImGuiCleanup() {
	delete _state;
	_state = nullptr;
}

} // namespace QDEngine
