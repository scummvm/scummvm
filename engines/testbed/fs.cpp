#include "common/config-manager.h"
#include "common/stream.h"

#include "testbed/fs.h"

namespace Testbed {
/**
 * This test does the following:
 * 1) acquires the game-data path
 * 2) In the game-root it navigates to "directory" and opens the file "file"
 * The former two are directories while the latter is a text file used for game engine detection
 *
 * Both the directories contain the file testbed.conf each which has a message written in it.
 * The code accesses the appropriate file using the fileSystem API, creates a read stream of it and
 * compares the message contained in it, with what it expects.
 *
 */
bool FStests::readDataFromFile(Common::FSNode &directory, const char *file) {
	
	
	Common::FSDirectory nestedDir(directory);

	Common::SeekableReadStream *readStream = nestedDir.createReadStreamForMember(file);

	if (!readStream) {
		printf("LOG:Can't open game file for reading\n");
		return false;
	} 
	
	Common::String msg = readStream->readLine();
	delete readStream;
	printf("LOG: Message Extracted from %s : %s\n", file, msg.c_str());


	Common::String expectedMsg = "It works!";

	if (!msg.equals(expectedMsg)) {
		printf("LOG: Can't read Correct data from file\n");
		return false;
	}

	return true;
}


bool FStests::testReadFile() {
	const Common::String &path = ConfMan.get("path");
	Common::FSNode gameRoot(path);
	
	if (!gameRoot.isDirectory()) {
		printf("LOG:game Path should be a directory");
		return false;
	}
	
	Common::FSList dirList;
	gameRoot.getChildren(dirList);
	
	const char *file[] = {"file.txt", "File.txt", "FILE.txt", "fILe.txt", "file."};

	for (unsigned int i = 0; i < dirList.size(); i++) {
		Common::String fileName = file[i];
		if (!readDataFromFile(dirList[i], fileName.c_str())) {
			printf("LOG : reading from %s failed", fileName.c_str());
			return false;
		}
		
		fileName.toLowercase();
		
		if (!readDataFromFile(dirList[i], fileName.c_str())) {
			printf("LOG : reading from %s failed", fileName.c_str());
			return false;
		}
		
		fileName.toUppercase();
		
		if (!readDataFromFile(dirList[i], fileName.c_str())) {
			printf("LOG : reading from %s failed", fileName.c_str());
			return false;
		}
	}

	return true;
}

/**
 * This test creates a file testbed.out, writes a sample data and confirms if
 * it is same by reading the file again.
 */

bool FStests::testWriteFile() {
	const Common::String &path = ConfMan.get("path");
	Common::FSNode gameRoot(path);

	Common::FSNode fileToWrite = gameRoot.getChild("testbed.out");
	
	Common::WriteStream *ws = fileToWrite.createWriteStream();
	
	if (!ws) {
		printf("LOG: Can't open writable file in game data dir\n");
		return false;
	}
	
	ws->writeString("ScummVM Rocks!");
	ws->flush();
	delete ws;

	Common::SeekableReadStream *rs = fileToWrite.createReadStream();
	Common::String readFromFile = rs->readLine();
	delete rs;

	if (readFromFile.equals("ScummVM Rocks!")) {
		// All good
		printf("LOG: Data written and read correctly\n");
		return true;
	}

	
	return false;
}

/**
 * This test creates a savefile for the given testbed-state and could be reloaded using the saveFile API.
 * It is intended to test saving and loading from savefiles.
 */
/*
GFXtests::testSavingGame() {
	Common::SaveFileManager saveFileMan = g_system->getSavefileManager();
}*/

FSTestSuite::FSTestSuite() {
	addTest("openingFile", &FStests::testReadFile);	
	addTest("WritingFile", &FStests::testWriteFile);	
}
const char *FSTestSuite::getName() const {
	return "File System";
}

} // End of namespace Testbed
