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

#include "common/archive.h"
#include "common/compression/unzip.h"
#include "common/debug.h"
#include "common/path.h"
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

	// Load the animation
	qdAnimation *animation = new qdAnimation();
	animation->qda_load(filename);

	_state->_qdaToDisplayFrameCount = animation->num_frames();

	int sx = 10, sy = 10;
	Graphics::ManagedSurface *surface = nullptr;

	if (frameNum == TILES_ID) {
		if (animation->tileAnimation()) {
			surface = animation->tileAnimation()->dumpTiles(25);

			sx = surface->w;
			sy = surface->h;
		}
	} else if (frameNum < 0) { // Tiles
		if (animation->tileAnimation()) {
			surface = animation->tileAnimation()->dumpFrameTiles(-frameNum + 1, 0.91670);

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
		animation->redraw(sx / 2, sy / 2, 0, 0.91670, 0);
		grDispatcher::instance()->resetSurfaceOverride();
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

void populateFileList() {
	// Iterate through the 3 resource pak files
	for (int i = 0; i < qdFileManager::instance().get_num_packages(); i++) {
		Common::Archive *archive = qdFileManager::instance().get_package(i);
		Common::ArchiveMemberList members;

		if (archive)
			archive->listMembers(members);

		for (auto &it : members)
			_state->_files.push_back(it->getPathInArchive());
	}

	Common::sort(_state->_files.begin(), _state->_files.end());
}

void showArchives() {
	if (!_state->_showArchives)
		return;

	// Calculate the viewport size
	ImVec2 viewportSize = ImGui::GetMainViewport()->Size;

	// Calculate the window size
	ImVec2 windowSize = ImVec2(
		viewportSize.x * 0.7f,
		viewportSize.y * 0.7f
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
		ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.3f, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None);

		ImGui::Button("\uef4f"); // Filter	// filter_alt
		ImGui::SameLine();

		_state->_nameFilter.Draw();
		ImGui::Separator();

		if (_state->_files.empty())
			populateFileList();

		for (auto &it : _state->_files) {
			const char *fileName = (char *)transCyrillic(it.baseName());
			if (_state->_nameFilter.PassFilter(fileName)) {
				if (ImGui::Selectable(fileName, _state->_qdaToDisplay == it)) {
					if (it.baseName().hasSuffixIgnoreCase(".qda")) {
						_state->_qdaToDisplay = it;
						_state->_qdaToDisplayFrame = 0;
						_state->_qdaIsPlaying = false;
					}
				}
			}
		}

		ImGui::EndChild();

		ImGui::SameLine();

		{ // Right pane
			ImGui::BeginChild("ChildR", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_Border);

			int totalFrames = _state->_qdaToDisplayFrameCount;


			ImGuiImage imgID;
			if (!_state->_qdaToDisplay.empty()) {
				imgID = getImageID(_state->_qdaToDisplay, _state->_qdaToDisplayFrame);

				ImGui::Text("Frame %s: %d  [%d x %d]", transCyrillic(_state->_qdaToDisplay.toString()), _state->_qdaToDisplayFrame,
						imgID.width, imgID.height);
			} else {
				ImGui::Text("Frame <none>");
			}

			ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if (ImGui::BeginTabBar("FrameTabBar", tab_bar_flags)) {

				if (ImGui::BeginTabItem("Animation")) {

					ImGui::Button("\ue020"); // Fast Rewind    // fast_rewind

					ImGui::SameLine();
					if (ImGui::Button("\ue045")) { // Skip Previous    // skip_previous
						_state->_qdaToDisplayFrame = _state->_qdaToDisplayFrame + totalFrames - 1;
						_state->_qdaToDisplayFrame %= totalFrames;
					}
					ImGui::SameLine();
					if (ImGui::Button("\ue037")) // Play    // play_arrow
						_state->_qdaIsPlaying = !_state->_qdaIsPlaying;

					ImGui::SameLine();
					if (ImGui::Button("\ue044")) { // Skip Next    // skip_next
						_state->_qdaToDisplayFrame += 1;
						_state->_qdaToDisplayFrame %= totalFrames;
					}
					ImGui::SameLine();
					ImGui::Button("\ue01f"); // Fast Forward    // fast_forward
					ImGui::SameLine();

					// Frame Count
					char buf[6];
					snprintf(buf, 6, "%d", _state->_qdaToDisplayFrame);

					ImGui::SetNextItemWidth(35);
					ImGui::InputText("##frame", buf, 5, ImGuiInputTextFlags_CharsDecimal);
					ImGui::SetItemTooltip("Frame");

					ImGui::Separator();

					if (!_state->_qdaToDisplay.empty()) {
						showImage(imgID, (char *)transCyrillic(_state->_qdaToDisplay.toString()), 2.0);
					} else {
						ImGui::InvisibleButton("##canvas", ImVec2(32.f, 32.f));
					}

					ImGui::SameLine();

					imgID = getImageID(_state->_qdaToDisplay, -_state->_qdaToDisplayFrame - 1);

					showImage(imgID, (char *)transCyrillic(_state->_qdaToDisplay.toString()), 2.0);

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Tiles")) {
					if (!_state->_qdaToDisplay.empty()) {
						imgID = getImageID(_state->_qdaToDisplay, TILES_ID);

						showImage(imgID, (char *)transCyrillic(_state->_qdaToDisplay.toString()), 1.0);
					} else {
						ImGui::InvisibleButton("##canvas", ImVec2(32.f, 32.f));
					}

					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
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

	static const ImWchar icons_ranges[] = {0xE000, 0xF8FF, 0};
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
