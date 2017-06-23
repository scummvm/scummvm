/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "testbed/webserver.h"
#include "backends/networking/sdl_net/localwebserver.h"
#include "common/config-manager.h"

namespace Testbed {

WebserverTestSuite::WebserverTestSuite() {
	addTest("ResolveIP", &WebserverTests::testIP, true);
	addTest("IndexPage", &WebserverTests::testIndexPage, true);
	addTest("FilesPage", &WebserverTests::testFilesPageInvalidParameterValue, true);
	addTest("CreateDirectory", &WebserverTests::testFilesPageCreateDirectory, true);
	addTest("UploadFile", &WebserverTests::testFilesPageUploadFile, true);
	addTest("UploadDirectory", &WebserverTests::testFilesPageUploadDirectory, true);
	addTest("DownloadFile", &WebserverTests::testFilesPageDownloadFile, true);
}

///// TESTS GO HERE /////

/** This test calls Storage::info(). */

bool WebserverTests::startServer() {
	Common::Point pt;
	pt.x = 10; pt.y = 10;
	Testsuite::writeOnScreen("Starting webserver...", pt);
	LocalServer.start();
	g_system->delayMillis(500);
	Testsuite::clearScreen();
	return LocalServer.isRunning();
}

TestExitStatus WebserverTests::testIP() {
	if (!startServer()) {
		Testsuite::logPrintf("Error! Can't start local webserver!\n");
		return kTestFailed;
	}

	Common::String info = "Welcome to the Webserver test suite!\n"
		"You would be visiting different server's pages and saying whether they work like they should.\n\n"
		"Testing Webserver's IP resolving.\n"
		"In this test we'll try to resolve server's IP.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : IP resolving\n");
		return kTestSkipped;
	}

	if (Testsuite::handleInteractiveInput(
			Common::String::format("Is this your machine's IP?\n%s", LocalServer.getAddress().c_str()),
			"Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! IP was not resolved!\n");
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("IP was resolved\n");
	return kTestPassed;
}

TestExitStatus WebserverTests::testIndexPage() {
	if (!startServer()) {
		Testsuite::logPrintf("Error! Can't start local webserver!\n");
		return kTestFailed;
	}

	Common::String info = "Testing Webserver's index page.\n"
		"In this test we'll try to open server's index page.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : index page\n");
		return kTestSkipped;
	}

	g_system->openUrl(LocalServer.getAddress());
	if (Testsuite::handleInteractiveInput(
		Common::String::format("The %s page opens well?", LocalServer.getAddress().c_str()),
		"Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! Couldn't open server's index page!\n");
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("Server's index page is OK\n");
	return kTestPassed;
}

TestExitStatus WebserverTests::testFilesPageInvalidParameterValue() {
	if (!startServer()) {
		Testsuite::logPrintf("Error! Can't start local webserver!\n");
		return kTestFailed;
	}

	Common::String info = "Testing Webserver's files page.\n"
		"In this test we'll try to pass invalid parameter to files page.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : files page (invalid parameter)\n");
		return kTestSkipped;
	}

	g_system->openUrl(LocalServer.getAddress()+"files?path=error");
	if (Testsuite::handleInteractiveInput(
		Common::String::format("The %sfiles?path=error page displays error message?", LocalServer.getAddress().c_str()),
		"Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! No error message on invalid parameter in '/files'!\n");
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("Server's files page detects invalid paramters fine\n");
	return kTestPassed;
}

TestExitStatus WebserverTests::testFilesPageCreateDirectory() {
	if (!startServer()) {
		Testsuite::logPrintf("Error! Can't start local webserver!\n");
		return kTestFailed;
	}

	Common::String info = "Testing Webserver's files page Create Directory feature.\n"
		"In this test you'll try to create directory.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : files page create directory\n");
		return kTestSkipped;
	}

	g_system->openUrl(LocalServer.getAddress() + "files?path=/root/");
	if (Testsuite::handleInteractiveInput(
		Common::String::format("You could go to %sfiles page, navigate to some directory with write access and create a directory there?", LocalServer.getAddress().c_str()),
		"Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! Create Directory is not working!\n");
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("Create Directory is OK\n");
	return kTestPassed;
}

TestExitStatus WebserverTests::testFilesPageUploadFile() {
	if (!startServer()) {
		Testsuite::logPrintf("Error! Can't start local webserver!\n");
		return kTestFailed;
	}

	Common::String info = "Testing Webserver's files page Upload Files feature.\n"
		"In this test you'll try to upload a file.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : files page file upload\n");
		return kTestSkipped;
	}

	g_system->openUrl(LocalServer.getAddress() + "files?path=/root/");
	if (Testsuite::handleInteractiveInput(
		Common::String::format("You're able to upload a file in some directory with write access through %sfiles page?", LocalServer.getAddress().c_str()),
		"Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! Upload Files is not working!\n");
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("Upload Files is OK\n");
	return kTestPassed;
}

TestExitStatus WebserverTests::testFilesPageUploadDirectory() {
	if (!startServer()) {
		Testsuite::logPrintf("Error! Can't start local webserver!\n");
		return kTestFailed;
	}

	Common::String info = "Testing Webserver's files page Upload Directory feature.\n"
		"In this test you'll try to upload a directory (works in Chrome only).";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : files page directory upload\n");
		return kTestSkipped;
	}

	g_system->openUrl(LocalServer.getAddress() + "files?path=/root/");
	if (Testsuite::handleInteractiveInput(
		Common::String::format("You're able to upload a directory into some directory with write access through %sfiles page using Chrome?", LocalServer.getAddress().c_str()),
		"Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! Upload Directory is not working!\n");
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("Upload Directory is OK\n");
	return kTestPassed;
}

TestExitStatus WebserverTests::testFilesPageDownloadFile() {
	if (!startServer()) {
		Testsuite::logPrintf("Error! Can't start local webserver!\n");
		return kTestFailed;
	}

	Common::String info = "Testing Webserver's files downloading feature.\n"
		"In this test you'll try to download a file.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : files page download\n");
		return kTestSkipped;
	}

	g_system->openUrl(LocalServer.getAddress() + "files?path=/root/");
	if (Testsuite::handleInteractiveInput(
		Common::String::format("You're able to download a file through %sfiles page?", LocalServer.getAddress().c_str()),
		"Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! Files downloading is not working!\n");
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("Files downloading is OK\n");
	return kTestPassed;
}

} // End of namespace Testbed
