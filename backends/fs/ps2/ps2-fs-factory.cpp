#include "backends/fs/ps2/ps2-fs-factory.h"
#include "backends/fs/ps2/ps2-fs.cpp"

DECLARE_SINGLETON(Ps2FilesystemFactory);

AbstractFilesystemNode *Ps2FilesystemFactory::makeRootFileNode() const {
	return new Ps2FilesystemNode();
}

AbstractFilesystemNode *Ps2FilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new Ps2FilesystemNode();
}

AbstractFilesystemNode *Ps2FilesystemFactory::makeFileNodePath(const String &path) const {
	return new Ps2FilesystemNode(path);
}
