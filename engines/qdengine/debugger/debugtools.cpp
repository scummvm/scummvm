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

	qdAnimationFrame *frame = animation->get_frame(frameNum);

	int sx = 10, sy = 10;
	if (frame) {
		sx = frame->size_x();
		sy = frame->size_y();
	}
	Graphics::ManagedSurface surface(sx, sy, g_engine->_pixelformat);

	if (frame) {
		grDispatcher::instance()->surfaceOverride(&surface);

		frame->redraw(sx / 2, sy / 2, 0);

		grDispatcher::instance()->resetSurfaceOverride();
	}

	_state->_frames[key] = { (ImTextureID)(intptr_t)loadTextureFromSurface(surface.surfacePtr()), sx, sy };

	return _state->_frames[key];
}

void showImage(const ImGuiImage &image, const char *name, float thumbnailSize) {
	ImVec2 size = { (float)image.width * 2, (float)image.height * 2 };

	ImGui::BeginGroup();
	ImVec2 screenPos = ImGui::GetCursorScreenPos();
	ImGui::GetWindowDrawList()->AddRect(screenPos, screenPos + ImVec2(size.x, size.y), 0xFFFFFFFF);

	ImGui::Image(image.id, size);
	ImGui::EndGroup();
	//setToolTipImage(image, name);
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

		ImGuiTextFilter nameFilter;
		nameFilter.Draw();
		ImGui::Separator();

		// Iterate through the 3 resource pak files
		for (int i = 0; i < 3; i++) {
			Common::Archive *archive = qdFileManager::instance().get_package(i);
			Common::ArchiveMemberList members;

			if (archive)
				archive->listMembers(members);

			if (archive && ImGui::TreeNode(Common::String::format("Resource/resource%d.pak", i).c_str())) {

				for (auto &it : members) {
					const char *fileName = (char *)transCyrillic(it->getFileName());
					if (nameFilter.PassFilter(fileName) && ImGui::Selectable(fileName) && it->getFileName().hasSuffixIgnoreCase(".qda")) {
						_state->_qdaToDisplay = it->getPathInArchive();
						_state->_qdaToDisplayFrame = 0;
					}

				}

				ImGui::TreePop();
			}
		}

		ImGui::EndChild();

		ImGui::SameLine();

		{ // Right pane
			ImGui::BeginChild("ChildR", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_Border);

			ImGuiImage imgID;
			if (!_state->_qdaToDisplay.empty()) {
				imgID = getImageID(_state->_qdaToDisplay, _state->_qdaToDisplayFrame);

				ImGui::Text("Frame %s: %d  [%d x %d]", transCyrillic(_state->_qdaToDisplay.toString()), _state->_qdaToDisplayFrame,
						imgID.width, imgID.height);
			} else {
				ImGui::Text("Frame <none>");
			}

			ImGui::Separator();

			if (!_state->_qdaToDisplay.empty()) {
				showImage(imgID, (char *)transCyrillic(_state->_qdaToDisplay.toString()), 120.0f);
			} else {
				ImGui::InvisibleButton("##canvas", ImVec2(32.f, 32.f));
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
