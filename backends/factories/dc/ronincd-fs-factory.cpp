#include "backends/factories/dc/ronincd-fs-factory.h"
#include "backends/fs/dc/dc-fs.cpp"
#include "backends/file/base-file.h"

DECLARE_SINGLETON(RoninCDFilesystemFactory);

AbstractFilesystemNode *RoninCDFilesystemFactory::makeRootFileNode() const {
	return new RoninCDFilesystemNode();
}

AbstractFilesystemNode *RoninCDFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new RoninCDFilesystemNode();
}

AbstractFilesystemNode *RoninCDFilesystemFactory::makeFileNodePath(const String &path) const {
	return new RoninCDFilesystemNode(path, true);
}

BaseFile *RoninCDFilesystemFactory::makeBaseFile() const {
	return new BaseFile();
}
