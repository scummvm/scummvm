#include "backends/fs/palmos/palmos-fs-factory.h"
#include "backends/fs/palmos/palmos-fs.cpp"

AbstractFilesystemNode *PalmOSFilesystemFactory::makeRootFileNode() const {
	return new PalmOSFilesystemNode();
}

AbstractFilesystemNode *PalmOSFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new PalmOSFilesystemNode();
}

AbstractFilesystemNode *PalmOSFilesystemFactory::makeFileNodePath(const String &path) const {
	return new PalmOSFilesystemNode(path);
}
