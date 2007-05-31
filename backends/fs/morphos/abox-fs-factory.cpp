#include "backends/fs/morphos/abox-fs-factory.h"
#include "backends/fs/morphos/abox-fs.cpp"

AbstractFilesystemNode *ABoxFilesystemFactory::makeRootFileNode() const {
	return new ABoxFilesystemNode();
}

AbstractFilesystemNode *ABoxFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new ABoxFilesystemNode();
}

AbstractFilesystemNode *ABoxFilesystemFactory::makeFileNodePath(const String &path) const {
	return new ABoxFilesystemNode(path);
}
