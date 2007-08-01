#include "backends/factories/palmos/palmos-fs-factory.h"
#include "backends/fs/palmos/palmos-fs.cpp"
#include "backends/file/base-file.h"

DECLARE_SINGLETON(PalmOSFilesystemFactory);

AbstractFilesystemNode *PalmOSFilesystemFactory::makeRootFileNode() const {
	return new PalmOSFilesystemNode();
}

AbstractFilesystemNode *PalmOSFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new PalmOSFilesystemNode();
}

AbstractFilesystemNode *PalmOSFilesystemFactory::makeFileNodePath(const String &path) const {
	return new PalmOSFilesystemNode(path);
}

BaseFile *PalmOSFilesystemFactory::makeBaseFile() const {
	return new BaseFile();
}
