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

#if defined(USE_PRINTING)

#include "base/version.h"

#include "common/rect.h"
#include "common/str.h"
#include "common/system.h"
#include "common/util.h"
#include "common/file.h"

#include "engines/engine.h"

#include "graphics/pm5544.h"

#include "gui/gui-manager.h"

#include "backends/printing/printman.h"

#include "testbed/printing.h"

namespace Testbed {

PrintingTestSuite::PrintingTestSuite() {
	addTest("Print Test Page", &PrintingTests::printTestPage);
}

TestExitStatus PrintingTests::printTestPage() {
	if (!ConfParams.isSessionInteractive()) {
		return kTestSkipped;
	}
	if (!g_gui.theme()->supportsImages()) {
		Testsuite::logPrintf("No logo to load in this theme, skipping test : printTestPage\n");
		return kTestSkipped;
	}

	if (Testsuite::handleInteractiveInput("Print a test page?", "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : printTestPage\n");
		return kTestSkipped;
	}

	Common::PrintingManager *pm = g_system->getPrintingManager();
	if (!pm) {
		warning("Could not get PrintingManager!");
		return kTestFailed;
	}

	// Print ScummVM logo
	const Graphics::ManagedSurface *logo = g_gui.theme()->getImageSurface("logo.bmp");
	if (!logo) {
		warning("Failed to load the ScummVM logo.");
		return kTestFailed;
	}

	pm->printImage(*logo);

	// The test pattern is CLUT-8
	Graphics::ManagedSurface *testPattern = Graphics::renderPM5544(800, 800);
	pm->printImage(*testPattern);

	return kTestPassed;
}

} // End of namespace Testbed

#endif
