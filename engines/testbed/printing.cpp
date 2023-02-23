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

#ifdef USE_PRINTING

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

	Common::Point pos;

	Common::Rect logoArea(pos.x, pos.y, pos.x + logo->w * 4, pos.y+logo->h * 4);
	job->drawBitmap(*logo, logoArea);
	pos += Common::Point(0, logoArea.height());

	job->drawText(gScummVMVersionDate, pos);
	pos += Common::Point(0, job->getTextBounds(gScummVMVersionDate).height());

	if (job->supportsColors()) {
		job->setTextColor(255, 0, 0);
		job->drawText("Red text", pos);
		pos += Common::Point(0, job->getTextBounds("Red text").height());

		job->setTextColor(0, 255, 0);
		job->drawText("Green text", pos);
		pos += Common::Point(0, job->getTextBounds("Green text").height());

		job->setTextColor(0, 0, 255);
		job->drawText("Blue text", pos);
		pos += Common::Point(0, job->getTextBounds("Blue text").height());

		job->setTextColor(0, 0, 0);
		job->drawText("Black text", pos);
		pos += Common::Point(0, job->getTextBounds("Black text").height());
	} else {
		job->drawText("Grayscale printing only, no text color test", pos);
		pos += Common::Point(0, job->getTextBounds("Grayscale printing only, no text color test").height());
	}

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

	pm->printPlainTextFile("The GPL", f);

	f.close();

	return kTestPassed;
}

} // End of namespace Testbed
#endif
