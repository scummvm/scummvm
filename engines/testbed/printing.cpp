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
#include "base/version.h"

#include "common/rect.h"
#include "common/str.h"
#include "common/system.h"
#include "common/util.h"
#include "common/file.h"

#include "engines/engine.h"

#include "gui/gui-manager.h"

#include "backends/printing/printman.h"

#include "testbed/printing.h"

namespace Testbed {


PrintingTestSuite::PrintingTestSuite() {
	addTest("Abort Job", &PrintingTests::abortJob);
	addTest("Print Test Page", &PrintingTests::printTestPage);
	addTest("Print the GPL", &PrintingTests::printGPL);
}

TestExitStatus PrintingTests::abortJob() {
	if (!ConfParams.isSessionInteractive()) {
		return kTestSkipped;
	}
	PrintingManager *pm = g_system->getPrintingManager();
	if (!pm) {
		warning("No PrintingManager!");
		return kTestFailed;
	}

	PrintJob *job = pm->createJob("ScummVM to be aborted");
	if (!job) {
		warning("Creating PrintJob failed!");
		return kTestFailed;
	}
	job->abortJob();
	delete job;

	return kTestPassed;
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

	const Graphics::ManagedSurface *logo = g_gui.theme()->getImageSurface("logo.bmp");
	if (!logo) {
		warning("Failed to load the scummvm logo.");
		return kTestFailed;
	}
	
	PrintingManager *pm = g_system->getPrintingManager();
	if (!pm) {
		warning("No PrintingManager!");
		return kTestFailed;
	}

	PrintJob *job = pm->createJob("ScummVM Test page");
	if (!job) {
		warning("Creating PrintJob failed!");
		return kTestFailed;
	}

	job->drawBitmap(*logo, 0, 0);

	job->drawText(gScummVMVersionDate, 0, logo->h + 10);

	job->pageFinished();
	job->endDoc();
	delete job;
	
	return kTestPassed;
}

TestExitStatus PrintingTests::printGPL() {
	if (!ConfParams.isSessionInteractive()) {
		return kTestSkipped;
	}

	Common::File f;
	if (!f.open("COPYING")) {
		warning("Failed to load COPYING");
		return kTestFailed;
	}

	if (Testsuite::handleInteractiveInput("Print the gpl to test long jobs?", "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : printGPL\n");
		f.close();
		return kTestSkipped;
	}

	PrintingManager *pm = g_system->getPrintingManager();
	if (!pm) {
		warning("No PrintingManager!");
		return kTestFailed;
	}

	PrintJob *job = pm->createJob("The GPL");
	if (!job) {
		warning("Creating PrintJob failed!");
		return kTestFailed;
	}

	while (!f.eos()) {
		Common::String line = f.readLine();

		//TODO: actually output the line
	}
	f.close();

	job->endDoc();
	delete job;

	return kTestPassed;
}

} // End of namespace Testbed
