#include "backends/fs/dc/ronincd-fs-factory.h"
#include "backends/fs/dc/dc-fs.cpp"

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
