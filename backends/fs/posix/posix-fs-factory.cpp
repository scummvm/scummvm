#include "backends/fs/posix/posix-fs-factory.h"
#include "backends/fs/posix/posix-fs.cpp"

AbstractFilesystemNode *POSIXFilesystemFactory::makeRootFileNode() const {
	return new POSIXFilesystemNode();
}

AbstractFilesystemNode *POSIXFilesystemFactory::makeCurrentDirectoryFileNode() const {
	char buf[MAXPATHLEN];
	getcwd(buf, MAXPATHLEN);
	return new POSIXFilesystemNode(buf, true);
}

AbstractFilesystemNode *POSIXFilesystemFactory::makeFileNodePath(const String &path) const {
	return new POSIXFilesystemNode(path, true);
}
