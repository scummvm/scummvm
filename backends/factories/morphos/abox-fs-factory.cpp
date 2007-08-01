#include "backends/factories/morphos/abox-fs-factory.h"
#include "backends/fs/morphos/abox-fs.cpp"
#include "backends/file/base-file.h"

DECLARE_SINGLETON(ABoxFilesystemFactory);

AbstractFilesystemNode *ABoxFilesystemFactory::makeRootFileNode() const {
	return new ABoxFilesystemNode();
}

AbstractFilesystemNode *ABoxFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new ABoxFilesystemNode();
}

AbstractFilesystemNode *ABoxFilesystemFactory::makeFileNodePath(const String &path) const {
	return new ABoxFilesystemNode(path);
}

BaseFile *ABoxFilesystemFactory::makeBaseFile() const {
	return new BaseFile();
}
