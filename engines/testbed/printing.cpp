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

#include "common/events.h"
#include "common/keyboard.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/system.h"
#include "common/util.h"

#include "engines/engine.h"

#include "gui/gui-manager.h"

#include "backends/printing/printman.h"

#include "testbed/printing.h"
#include "testbed/graphics.h"

namespace Testbed {


PrintingTestSuite::PrintingTestSuite() {
	addTest("Abort Job", &PrintingTests::abortJob);
	addTest("Print Test Page", &PrintingTests::printTestPage);
	//addTest("Print the GPL", &PrintingTests::printGPL);
}

TestExitStatus PrintingTests::abortJob() {
	if (!ConfParams.isSessionInteractive()) {
		return kTestSkipped;
	}
	PrintingManager *pm = g_system->getPrintingManager();
	if (!pm)
		return kTestFailed;

	PrintJob *job = pm->createJob("ScummVM to be aborted");
	job->abortJob();
	delete job;

	return kTestPassed;
}


TestExitStatus PrintingTests::printTestPage() {
	if (!ConfParams.isSessionInteractive()) {
		return kTestSkipped;
	}
	if (!g_gui.theme()->supportsImages())
		return kTestSkipped;
	const Graphics::ManagedSurface *logo = g_gui.theme()->getImageSurface("logo.bmp");
	if (!logo)
		return kTestFailed;
	
	PrintingManager *pm = g_system->getPrintingManager();
	if (!pm)
		return kTestFailed;

	PrintJob *job = pm->createJob("ScummVM Test page");

	job->drawBitmap(*logo, 0, 0);

	//output build details
	gScummVMVersionDate;

	job->pageFinished();
	job->endDoc();
	delete job;
	
	return kTestPassed;
}

TestExitStatus PrintingTests::printGPL() {
	if (!ConfParams.isSessionInteractive()) {
		return kTestSkipped;
	}

	PrintingManager *pm = g_system->getPrintingManager();
	if (!pm)
		return kTestFailed;

	PrintJob *job = pm->createJob("The GPL");
	job->endDoc();
	delete job;

	return kTestPassed;
}

} // End of namespace Testbed
