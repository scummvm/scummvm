#include "common/config-manager.h"
#include "common/stream.h"
#include "common/savefile.h"
#include "common/util.h"

#include "testbed/fs.h"

namespace Testbed {
/**
 * This test does the following:
 * 1) acquires the game-data path
 * 2) In the game-root it navigates to "directory" and opens the file "file"
 *
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
	printf("LOG: Message Extracted from %s/%s : %s\n",directory.getName().c_str(), file, msg.c_str());


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
	int numFailed = 0;
	
	if (!gameRoot.isDirectory()) {
		printf("LOG:game Path should be a directory");
		return false;
	}
	
	const char *dirList[] = {"test1" ,"Test2", "TEST3" , "tEST4", "test5"};
	const char *file[] = {"file.txt", "File.txt", "FILE.txt", "fILe.txt", "file"};

	for (unsigned int i = 0; i < ARRAYSIZE(dirList); i++) {
		Common::String dirName = dirList[i];
		Common::String fileName = file[i];
		Common::FSNode directory = gameRoot.getChild(dirName); 

		if (!readDataFromFile(directory, fileName.c_str())) {
			printf("LOG : reading from %s/%s failed\n", dirName.c_str(), fileName.c_str());
			numFailed++;
		}
		
		dirName.toLowercase();
		fileName.toLowercase();
		directory = gameRoot.getChild(dirName); 
		
		if (!readDataFromFile(directory, fileName.c_str())) {
			printf("LOG : reading from %s/%s failed\n", dirName.c_str(), fileName.c_str());
			numFailed++;
		}
		
		dirName.toUppercase();
		fileName.toUppercase();
		directory = gameRoot.getChild(dirName); 
		
		if (!readDataFromFile(directory, fileName.c_str())) {
			printf("LOG : reading from %s/%s failed\n", dirName.c_str(), fileName.c_str());
			numFailed++;
		}
	}
	
	printf("LOG:failed %d out of 15\n", numFailed);
	return false;
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

bool FStests::testOpeningSaveFile() {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::OutSaveFile *saveFile = saveFileMan->openForSaving("saveFile.0");

	saveFile->writeString("State:FS tests");
	saveFile->flush();
	delete saveFile;

	return true;
}

FSTestSuite::FSTestSuite() {
	addTest("ReadingFile", &FStests::testReadFile);	
	addTest("WritingFile", &FStests::testWriteFile);	
	addTest("OpeningSaveFile", &FStests::testOpeningSaveFile);
}
const char *FSTestSuite::getName() const {
	return "File System";
}

} // End of namespace Testbed
