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

#include "twp/twpimgui.h"
#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#ifdef USE_IMGUI
#include "graphics/imgui/backends/imgui_impl_sdl2_scummvm.h"
#include "graphics/imgui/backends/imgui_impl_opengl3_scummvm.h"
#include "backends/graphics3d/openglsdl/openglsdl-graphics3d.h"
// here I undefined these symbols because of the <X11/Xlib.h> defining them
// and messing with all classes or structure using the same names
#undef Bool
#undef CursorShape
#undef Expose
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#undef FontChange
#undef None
#undef Status
#undef Unsorted

#include "twp/debugtools.h"
#endif

namespace Twp {

#ifdef USE_IMGUI
SDL_Window *g_window = nullptr;
#endif

void DearImGui::init() {
#ifdef USE_IMGUI
	// Setup Dear ImGui
	OpenGLSdlGraphics3dManager *manager = dynamic_cast<OpenGLSdlGraphics3dManager *>(g_system->getPaletteManager());
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	g_window = manager->getWindow()->getSDLWindow();
	SDL_GLContext glContext = SDL_GL_GetCurrentContext();
	ImGui_ImplSDL2_InitForOpenGL(g_window, glContext);
	ImGui_ImplOpenGL3_Init("#version 110");
	ImGui::StyleColorsDark();

	ImGuiIO &io = ImGui::GetIO();
	Common::Path initPath(ConfMan.getPath("savepath"));
	initPath = initPath.appendComponent("twp_imgui.ini");
	io.IniFilename = initPath.toString().c_str();
#endif
}

void DearImGui::cleanup() {
#ifdef USE_IMGUI
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
#endif
}

bool DearImGui::processEvent(const Common::Event *event) {
#ifdef USE_IMGUI
	ImGui_ImplSDL2_ProcessEvent(event);
	ImGuiIO &io = ImGui::GetIO();
	if (io.WantTextInput || io.WantCaptureMouse)
		return true;
#endif
	return false;
}

void DearImGui::render() {
#ifdef USE_IMGUI
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(g_window);
	ImGui::NewFrame();
	onRender();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
}

void TwpImGui::onRender() {
#ifdef USE_IMGUI
	onImGuiRender();
#endif
}

} // namespace Twp
