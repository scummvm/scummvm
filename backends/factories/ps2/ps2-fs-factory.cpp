#include "backends/factories/ps2/ps2-fs-factory.h"
#include "backends/fs/ps2/ps2-fs.cpp"
#include "backends/file/ps2/ps2-file.h"

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

BaseFile *Ps2FilesystemFactory::makeBaseFile() const {
	return new Ps2File();
}
