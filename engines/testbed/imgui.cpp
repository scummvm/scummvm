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

#include "common/system.h"
#include "common/events.h"
#include "engines/engine.h"
#include "engines/testbed/imgui.h"

#include "backends/imgui/imgui.h"
#include "backends/imgui/imgui_fonts.h"

namespace Testbed {

static bool p_supported = false;
static bool p_open = false;

static void onImGuiInit() {
	p_supported = true;
}

void onImGuiRender() {
	if (p_open) {
		ImGui::GetIO().ConfigFlags &= ~(ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse);

		ImGui::ShowDemoWindow(&p_open);
	} else {
		ImGui::GetIO().ConfigFlags |= (ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse);
	}
}

static void onImGuiCleanup() {
}

TestExitStatus Imguitests::testImGui() {
	Testsuite::clearScreen();
	Common::String info = "ImGui test. You should expect an ImGui dialog to appear on screen.\"";
	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : testImGui\n");
		return kTestSkipped;
	}
	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Testing ImGui", pt);

	p_open = true;

	ImGuiCallbacks callbacks;
	callbacks.init = onImGuiInit;
	callbacks.render = onImGuiRender;
	callbacks.cleanup = onImGuiCleanup;
	g_system->setImGuiCallbacks(callbacks);

	// Update the screen once so that it has the chance to initialise.
	g_system->updateScreen();

	if (!p_supported) {
		Testsuite::logDetailedPrintf("ImGui failed\n");
		return kTestFailed;
	}

	Common::EventManager *eventMan = g_system->getEventManager();
	Common::Event event;
	while (p_open) {
		while (eventMan->pollEvent(event)) {
			// Quit if explicitly requested
			if (Engine::shouldQuit()) {
				return kTestPassed;
			}
		}

		g_system->updateScreen();
	}

	Common::String prompt = "Did you see an ImGui dialog?";
	if (!Testsuite::handleInteractiveInput(prompt, "Yes", "No", kOptionLeft)) {
		Testsuite::logDetailedPrintf("ImGui failed\n");
		return kTestFailed;
	}
	return kTestPassed;
}

ImGuiTestSuite::ImGuiTestSuite() {
	_isTsEnabled = true;
	addTest("testImGui", &Imguitests::testImGui, true);
}

} // End of namespace Testbed
