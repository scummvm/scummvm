#include "backends/factories/symbian/symbian-fs-factory.h"
#include "backends/fs/symbian/symbian-fs.cpp"
#include "backends/file/symbian/symbian-file.h"

DECLARE_SINGLETON(SymbianFilesystemFactory);

AbstractFilesystemNode *SymbianFilesystemFactory::makeRootFileNode() const {
	return new SymbianFilesystemNode(true);
}

AbstractFilesystemNode *SymbianFilesystemFactory::makeCurrentDirectoryFileNode() const {
	char path[MAXPATHLEN];
	getcwd(path, MAXPATHLEN);
	return new SymbianFilesystemNode(path);
}

AbstractFilesystemNode *SymbianFilesystemFactory::makeFileNodePath(const String &path) const {
	return new SymbianFilesystemNode(path);
}

BaseFile *SymbianFilesystemFactory::makeBaseFile() const {
	return new SymbianFile();
}
