#include "backends/fs/gp32/gp32-fs-factory.h"
#include "backends/fs/gp32/gp32-fs.cpp"

DECLARE_SINGLETON(GP32FilesystemFactory);

AbstractFilesystemNode *GP32FilesystemFactory::makeRootFileNode() const {
	return new GP32FilesystemNode();
}

AbstractFilesystemNode *GP32FilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new GP32FilesystemNode();
}

AbstractFilesystemNode *GP32FilesystemFactory::makeFileNodePath(const String &path) const {
	return new GP32FilesystemNode(path);
}
