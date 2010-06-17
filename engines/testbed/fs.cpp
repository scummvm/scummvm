#include "common/config-manager.h"
#include "common/stream.h"

#include "testbed/fs.h"

namespace Testbed {
/**
 * This test does the following:
 * 1) acquires the game-data path
 * 2) In the game-data dir, there are three files: testbed, TeStBeD and TESTBED
 * The former two are directories while the latter is a text file used for game engine detection
 *
 * Both the directories contain the file testbed.conf each which has a message written in it.
 * The code accesses the appropriate file using the fileSystem API, creates a read stream of it and
 * compares the message contained in it, with what it expects.
 *
 */
bool FStests::testOpenFile() {
	const Common::String &path = ConfMan.get("path");
	Common::FSNode gameRoot(path);
	
	if (!gameRoot.isDirectory()) {
		printf("LOG:game Path should be a directory");
		return false;
	}
	
	Common::FSNode subDir = gameRoot.getChild("TeStBeD");

	if (!subDir.exists()) {
		printf("LOG:Unable to recognize TeStBeD Inside the game Dir");
		return false;
	}
	
	Common::FSDirectory testBedDir(subDir);

	Common::SeekableReadStream *readStream = testBedDir.createReadStreamForMember("testbed.conf");

	if (!readStream) {
		printf("LOG:Can't open game file for reading\n");
		return false;
	} 
	
	Common::String msg = readStream->readLine();
	printf("LOG: Message Extracted: %s\n", msg.c_str());

	Common::String expectedMsg = "It works!";

	if (!msg.equals(expectedMsg)) {
		printf("LOG: Can't read Correct data from file\n");
		return false;
	}

	return true;
}


FSTestSuite::FSTestSuite() {
	addTest("openingFile", &FStests::testOpenFile);	
}
const char *FSTestSuite::getName() const {
	return "File System";
}

} // End of namespace Testbed
