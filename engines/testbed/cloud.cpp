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

#include "common/config-manager.h"
#include "common/stream.h"
#include "common/util.h"
#include "testbed/fs.h"
#include "testbed/cloud.h"
#include "backends/cloud/cloudmanager.h"

namespace Testbed {

CloudTestSuite::CloudTestSuite() {
	// Cloud tests depend on CloudMan.
	// If there is no Storage connected to it, disable this test suite.
	if (CloudMan.getCurrentStorage() == nullptr) {
		logPrintf("WARNING! : No Storage connected to CloudMan found. Skipping Cloud tests\n");
		Testsuite::enable(false);
	}

	addTest("UserInfo", &CloudTests::testInfo, true);
	addTest("ListDirectory", &CloudTests::testDirectoryListing, true);
	addTest("CreateDirectory", &CloudTests::testDirectoryCreating, true);
	addTest("FileUpload", &CloudTests::testUploading, true);
	addTest("FileDownload", &CloudTests::testDownloading, true);
	addTest("FolderDownload", &CloudTests::testFolderDownloading, true);
	addTest("SyncSaves", &CloudTests::testSavesSync, true);
}

/*
void CloudTestSuite::enable(bool flag) {
	Testsuite::enable(ConfParams.isGameDataFound() ? flag : false);
}
*/

///// TESTS GO HERE /////

bool CloudTests::waitForCallback() {
	const int TIMEOUT = 30;

	Common::Point pt;
	pt.x = 10; pt.y = 10;
	Testsuite::writeOnScreen("Waiting for callback...", pt);

	int left = TIMEOUT;
	while (--left) {
		if (ConfParams.isCloudTestCallbackCalled()) return true;
		if (ConfParams.isCloudTestErrorCallbackCalled()) return true;
		g_system->delayMillis(1000);
	}
	return false;
}

bool CloudTests::waitForCallbackMore() {
	while (!waitForCallback()) {
		Common::String info = "It takes more time than expected. Do you want to skip the test or wait more?";
		if (Testsuite::handleInteractiveInput(info, "Wait", "Skip", kOptionRight)) {
			Testsuite::logPrintf("Info! Skipping test : info()\n");
			return false;
		}
	}
	return true;
}

const char *CloudTests::getRemoteTestPath() {
	if (CloudMan.getStorageIndex() == Cloud::kStorageDropboxId)
		return "/testbed";
	return "testbed";
}

void CloudTests::infoCallback(Cloud::Storage::StorageInfoResponse response) {
	ConfParams.setCloudTestCallbackCalled(true);
	Testsuite::logPrintf("Info! User's ID: %s\n", response.value.uid().c_str());
	Testsuite::logPrintf("Info! User's email: %s\n", response.value.email().c_str());
	Testsuite::logPrintf("Info! User's name: %s\n", response.value.name().c_str());
	Testsuite::logPrintf("Info! User's quota: %lu bytes used / %lu bytes available\n", response.value.used(), response.value.available());
}

void CloudTests::directoryListedCallback(Cloud::Storage::FileArrayResponse response) {
	ConfParams.setCloudTestCallbackCalled(true);
	if (response.value.size() == 0) {
		Testsuite::logPrintf("Warning! Directory is empty!\n");
		return;
	}

	Common::String directory, file;
	uint32 directories = 0, files = 0;
	for (uint32 i = 0; i < response.value.size(); ++i) {
		if (response.value[i].isDirectory()) {
			if (++directories == 1) directory = response.value[i].path();
		} else {
			if (++files == 1) file = response.value[i].name();
		}
	}

	if (directories == 0) {
		Testsuite::logPrintf("Info! %u files listed, first one is '%s'\n", files, file.c_str());
	} else if (files == 0) {
		Testsuite::logPrintf("Info! %u directories listed, first one is '%s'\n", directories, directory.c_str());
	} else {
		Testsuite::logPrintf("Info! %u directories and %u files listed\n", directories, files);
		Testsuite::logPrintf("Info! First directory is '%s' and first file is '%s'\n", directory.c_str(), file.c_str());
	}
}

void CloudTests::directoryCreatedCallback(Cloud::Storage::BoolResponse response) {
	ConfParams.setCloudTestCallbackCalled(true);
	if (response.value) {
		Testsuite::logPrintf("Info! Directory created!\n");
	} else {
		Testsuite::logPrintf("Info! Such directory already exists!\n");
	}
}

void CloudTests::fileUploadedCallback(Cloud::Storage::UploadResponse response) {
	ConfParams.setCloudTestCallbackCalled(true);
	Testsuite::logPrintf("Info! Uploaded file into '%s'\n", response.value.path().c_str());
	Testsuite::logPrintf("Info! It's id = '%s' and size = '%u'\n", response.value.id().c_str(), response.value.size());
}

void CloudTests::fileDownloadedCallback(Cloud::Storage::BoolResponse response) {
	ConfParams.setCloudTestCallbackCalled(true);
	if (response.value) {
		Testsuite::logPrintf("Info! File downloaded!\n");
	} else {
		Testsuite::logPrintf("Info! Failed to download the file!\n");
	}
}

void CloudTests::directoryDownloadedCallback(Cloud::Storage::FileArrayResponse response) {
	ConfParams.setCloudTestCallbackCalled(true);
	if (response.value.size() == 0) {
		Testsuite::logPrintf("Info! Directory is downloaded successfully!\n");
	} else {
		Testsuite::logPrintf("Warning! %u files were not downloaded during folder downloading!\n", response.value.size());
	}
}

void CloudTests::savesSyncedCallback(Cloud::Storage::BoolResponse response) {
	ConfParams.setCloudTestCallbackCalled(true);
	if (response.value) {
		Testsuite::logPrintf("Info! Saves are synced successfully!\n");
	} else {
		Testsuite::logPrintf("Warning! Saves were not synced!\n");
	}
}

void CloudTests::errorCallback(Networking::ErrorResponse response) {
	ConfParams.setCloudTestErrorCallbackCalled(true);
	Testsuite::logPrintf("Info! Error Callback was called\n");
	Testsuite::logPrintf("Info! code = %ld, message = %s\n", response.httpResponseCode, response.response.c_str());
}

/** This test calls Storage::info(). */

TestExitStatus CloudTests::testInfo() {
	ConfParams.setCloudTestCallbackCalled(false);
	ConfParams.setCloudTestErrorCallbackCalled(false);

	if (CloudMan.getCurrentStorage() == nullptr) {
		Testsuite::logPrintf("Couldn't find connected Storage\n");
		return kTestFailed;
	}

	Common::String info = Common::String::format(
		"Welcome to the Cloud test suite!\n"
		"We're going to use the %s cloud storage which is connected right now.\n\n"
		"Testing Cloud Storage API info() method.\n"
		"In this test we'll try to list user infomation.",
		CloudMan.getCurrentStorage()->name().c_str()
	);

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : info()\n");
		return kTestSkipped;
	}

	if (CloudMan.info(
			new Common::GlobalFunctionCallback<Cloud::Storage::StorageInfoResponse>(&infoCallback),
			new Common::GlobalFunctionCallback<Networking::ErrorResponse>(&errorCallback)
		) == nullptr) {
		Testsuite::logPrintf("Warning! No Request is returned!\n");
	}

	if (!waitForCallbackMore()) return kTestSkipped;
	Testsuite::clearScreen();

	if (ConfParams.isCloudTestErrorCallbackCalled()) {
		Testsuite::logPrintf("Error callback was called\n");
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("Info was displayed\n");
	return kTestPassed;
}

TestExitStatus CloudTests::testDirectoryListing() {
	ConfParams.setCloudTestCallbackCalled(false);
	ConfParams.setCloudTestErrorCallbackCalled(false);

	if (CloudMan.getCurrentStorage() == nullptr) {
		Testsuite::logPrintf("Couldn't find connected Storage\n");
		return kTestFailed;
	}

	Common::String info = "Testing Cloud Storage API listDirectory() method.\n"
		"In this test we'll try to list root directory.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : listDirectory()\n");
		return kTestSkipped;
	}

	if (CloudMan.listDirectory(
			"",
			new Common::GlobalFunctionCallback<Cloud::Storage::FileArrayResponse>(&directoryListedCallback),
			new Common::GlobalFunctionCallback<Networking::ErrorResponse>(&errorCallback)
		) == nullptr) {
		Testsuite::logPrintf("Warning! No Request is returned!\n");
	}

	if (!waitForCallbackMore()) return kTestSkipped;
	Testsuite::clearScreen();

	if (ConfParams.isCloudTestErrorCallbackCalled()) {
		Testsuite::logPrintf("Error callback was called\n");
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("Directory was listed\n");
	return kTestPassed;
}

TestExitStatus CloudTests::testDirectoryCreating() {
	ConfParams.setCloudTestCallbackCalled(false);
	ConfParams.setCloudTestErrorCallbackCalled(false);

	if (CloudMan.getCurrentStorage() == nullptr) {
		Testsuite::logPrintf("Couldn't find connected Storage\n");
		return kTestFailed;
	}

	Common::String info = "Testing Cloud Storage API createDirectory() method.\n"
		"In this test we'll try to create a 'testbed' directory.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : createDirectory()\n");
		return kTestSkipped;
	}

	Common::String info2 = "We'd list the root directory, create the directory and the list it again.\n"
		"If all goes smoothly, you'd notice that there are more directories now.";
	Testsuite::displayMessage(info2);

	// list root directory
	if (CloudMan.listDirectory(
			"",
			new Common::GlobalFunctionCallback<Cloud::Storage::FileArrayResponse>(&directoryListedCallback),
			new Common::GlobalFunctionCallback<Networking::ErrorResponse>(&errorCallback)
		) == nullptr) {
		Testsuite::logPrintf("Warning! No Request is returned!\n");
	}

	if (!waitForCallbackMore()) return kTestSkipped;
	Testsuite::clearScreen();

	if (ConfParams.isCloudTestErrorCallbackCalled()) {
		Testsuite::logPrintf("Error callback was called\n");
		return kTestFailed;
	}

	ConfParams.setCloudTestCallbackCalled(false);

	// create 'testbed'
	if (CloudMan.getCurrentStorage()->createDirectory(
			getRemoteTestPath(),
			new Common::GlobalFunctionCallback<Cloud::Storage::BoolResponse>(&directoryCreatedCallback),
			new Common::GlobalFunctionCallback<Networking::ErrorResponse>(&errorCallback)
		) == nullptr) {
		Testsuite::logPrintf("Warning! No Request is returned!\n");
	}

	if (!waitForCallbackMore()) return kTestSkipped;
	Testsuite::clearScreen();

	if (ConfParams.isCloudTestErrorCallbackCalled()) {
		Testsuite::logPrintf("Error callback was called\n");
		return kTestFailed;
	}

	ConfParams.setCloudTestCallbackCalled(false);

	// list it again
	if (CloudMan.listDirectory(
			"",
			new Common::GlobalFunctionCallback<Cloud::Storage::FileArrayResponse>(&directoryListedCallback),
			new Common::GlobalFunctionCallback<Networking::ErrorResponse>(&errorCallback)
		) == nullptr) {
		Testsuite::logPrintf("Warning! No Request is returned!\n");
	}

	if (!waitForCallbackMore()) return kTestSkipped;
	Testsuite::clearScreen();

	if (ConfParams.isCloudTestErrorCallbackCalled()) {
		Testsuite::logPrintf("Error callback was called\n");
		return kTestFailed;
	}

	if (Testsuite::handleInteractiveInput("Was the CloudMan able to create a 'testbed' directory?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! Directory was not created!\n");
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("Directory was created\n");
	return kTestPassed;
}

TestExitStatus CloudTests::testUploading() {
	ConfParams.setCloudTestCallbackCalled(false);
	ConfParams.setCloudTestErrorCallbackCalled(false);

	if (CloudMan.getCurrentStorage() == nullptr) {
		Testsuite::logPrintf("Couldn't find connected Storage\n");
		return kTestFailed;
	}

	Common::String info = "Testing Cloud Storage API upload() method.\n"
		"In this test we'll try to upload a 'test1/file.txt' file.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : upload()\n");
		return kTestSkipped;
	}

	if (!ConfParams.isGameDataFound()) {
		Testsuite::logPrintf("Info! Couldn't find the game data, so skipping test : upload()\n");
		return kTestSkipped;
	}

	const Common::String &path = ConfMan.get("path");
	Common::FSDirectory gameRoot(path);
	Common::FSDirectory *directory = gameRoot.getSubDirectory("test1");
	Common::FSNode node = directory->getFSNode().getChild("file.txt");
	delete directory;

	if (CloudMan.getCurrentStorage()->uploadStreamSupported()) {
		if (CloudMan.getCurrentStorage()->upload(
				Common::String(getRemoteTestPath()) + "/testfile.txt",
				node.createReadStream(),
				new Common::GlobalFunctionCallback<Cloud::Storage::UploadResponse>(&fileUploadedCallback),
				new Common::GlobalFunctionCallback<Networking::ErrorResponse>(&errorCallback)
			) == nullptr) {
			Testsuite::logPrintf("Warning! No Request is returned!\n");
		}
	} else {
		Common::String filepath = node.getPath();
		if (CloudMan.getCurrentStorage()->upload(
				Common::String(getRemoteTestPath()) + "/testfile.txt",
				filepath.c_str(),
				new Common::GlobalFunctionCallback<Cloud::Storage::UploadResponse>(&fileUploadedCallback),
				new Common::GlobalFunctionCallback<Networking::ErrorResponse>(&errorCallback)
			) == nullptr) {
			Testsuite::logPrintf("Warning! No Request is returned!\n");
		}
	}

	if (!waitForCallbackMore()) return kTestSkipped;
	Testsuite::clearScreen();

	if (ConfParams.isCloudTestErrorCallbackCalled()) {
		Testsuite::logPrintf("Error callback was called\n");
		return kTestFailed;
	}

	Common::String info2 = "upload() is finished. Do you want to list '/testbed' directory?";

	if (!Testsuite::handleInteractiveInput(info2, "Yes", "No", kOptionRight)) {
		ConfParams.setCloudTestCallbackCalled(false);

		if (CloudMan.listDirectory(
				getRemoteTestPath(),
				new Common::GlobalFunctionCallback<Cloud::Storage::FileArrayResponse>(&directoryListedCallback),
				new Common::GlobalFunctionCallback<Networking::ErrorResponse>(&errorCallback)
			) == nullptr) {
			Testsuite::logPrintf("Warning! No Request is returned!\n");
		}

		if (!waitForCallbackMore()) return kTestSkipped;
		Testsuite::clearScreen();

		if (ConfParams.isCloudTestErrorCallbackCalled()) {
			Testsuite::logPrintf("Error callback was called\n");
			return kTestFailed;
		}
	}

	if (Testsuite::handleInteractiveInput("Was the CloudMan able to upload into 'testbed/testfile.txt' file?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! File was not uploaded!\n");
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("File was uploaded\n");
	return kTestPassed;
}

TestExitStatus CloudTests::testDownloading() {
	ConfParams.setCloudTestCallbackCalled(false);
	ConfParams.setCloudTestErrorCallbackCalled(false);

	if (CloudMan.getCurrentStorage() == nullptr) {
		Testsuite::logPrintf("Couldn't find connected Storage\n");
		return kTestFailed;
	}

	Common::String info = "Testing Cloud Storage API download() method.\n"
		"In this test we'll try to download that 'testbed/testfile.txt' file.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : download()\n");
		return kTestSkipped;
	}

	const Common::String &path = ConfMan.get("path");
	Common::FSDirectory gameRoot(path);
	Common::FSNode node = gameRoot.getFSNode().getChild("downloaded_file.txt");
	Common::String filepath = node.getPath();
	if (CloudMan.getCurrentStorage()->download(
			Common::String(getRemoteTestPath()) + "/testfile.txt",
			filepath.c_str(),
			new Common::GlobalFunctionCallback<Cloud::Storage::BoolResponse>(&fileDownloadedCallback),
			new Common::GlobalFunctionCallback<Networking::ErrorResponse>(&errorCallback)
		) == nullptr) {
		Testsuite::logPrintf("Warning! No Request is returned!\n");
	}

	if (!waitForCallbackMore()) return kTestSkipped;
	Testsuite::clearScreen();

	if (ConfParams.isCloudTestErrorCallbackCalled()) {
		Testsuite::logPrintf("Error callback was called\n");
		return kTestFailed;
	}

	if (Testsuite::handleInteractiveInput("Was the CloudMan able to download into 'testbed/downloaded_file.txt' file?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! File was not downloaded!\n");
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("File was downloaded\n");
	return kTestPassed;
}

TestExitStatus CloudTests::testFolderDownloading() {
	ConfParams.setCloudTestCallbackCalled(false);
	ConfParams.setCloudTestErrorCallbackCalled(false);

	if (CloudMan.getCurrentStorage() == nullptr) {
		Testsuite::logPrintf("Couldn't find connected Storage\n");
		return kTestFailed;
	}

	Common::String info = "Testing Cloud Storage API downloadFolder() method.\n"
		"In this test we'll try to download remote 'testbed/' directory.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : downloadFolder()\n");
		return kTestSkipped;
	}

	const Common::String &path = ConfMan.get("path");
	Common::FSDirectory gameRoot(path);
	Common::FSNode node = gameRoot.getFSNode().getChild("downloaded_directory");
	Common::String filepath = node.getPath();
	if (CloudMan.downloadFolder(
			getRemoteTestPath(),
			filepath.c_str(),
			new Common::GlobalFunctionCallback<Cloud::Storage::FileArrayResponse>(&directoryDownloadedCallback),
			new Common::GlobalFunctionCallback<Networking::ErrorResponse>(&errorCallback)
		) == nullptr) {
		Testsuite::logPrintf("Warning! No Request is returned!\n");
	}

	if (!waitForCallbackMore()) return kTestSkipped;
	Testsuite::clearScreen();

	if (ConfParams.isCloudTestErrorCallbackCalled()) {
		Testsuite::logPrintf("Error callback was called\n");
		return kTestFailed;
	}

	if (Testsuite::handleInteractiveInput("Was the CloudMan able to download into 'testbed/downloaded_directory'?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! Directory was not downloaded!\n");
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("Directory was downloaded\n");
	return kTestPassed;
}

TestExitStatus CloudTests::testSavesSync() {
	ConfParams.setCloudTestCallbackCalled(false);
	ConfParams.setCloudTestErrorCallbackCalled(false);

	if (CloudMan.getCurrentStorage() == nullptr) {
		Testsuite::logPrintf("Couldn't find connected Storage\n");
		return kTestFailed;
	}

	Common::String info = "Testing Cloud Storage API syncSaves() method.\n"
		"In this test we'll try to sync your saves.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : syncSaves()\n");
		return kTestSkipped;
	}

	const Common::String &path = ConfMan.get("path");
	Common::FSDirectory gameRoot(path);
	Common::FSNode node = gameRoot.getFSNode().getChild("downloaded_directory");
	Common::String filepath = node.getPath();
	if (CloudMan.syncSaves(
			new Common::GlobalFunctionCallback<Cloud::Storage::BoolResponse>(&savesSyncedCallback),
			new Common::GlobalFunctionCallback<Networking::ErrorResponse>(&errorCallback)
		) == nullptr) {
		Testsuite::logPrintf("Warning! No Request is returned!\n");
	}

	if (!waitForCallbackMore()) return kTestSkipped;
	Testsuite::clearScreen();

	if (ConfParams.isCloudTestErrorCallbackCalled()) {
		Testsuite::logPrintf("Error callback was called\n");
		return kTestFailed;
	}

	if (Testsuite::handleInteractiveInput("Was the CloudMan able to sync saves?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! Saves were not synced!\n");
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("Saves were synced successfully\n");
	return kTestPassed;
}

} // End of namespace Testbed
