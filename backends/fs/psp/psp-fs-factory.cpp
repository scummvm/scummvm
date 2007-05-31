#include "backends/fs/psp/psp-fs-factory.h"
#include "backends/fs/psp/psp_fs.cpp"

AbstractFilesystemNode *PSPFilesystemFactory::makeRootFileNode() const {
	return new PSPFilesystemNode();
}

AbstractFilesystemNode *PSPFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new PSPFilesystemNode();
}

AbstractFilesystemNode *PSPFilesystemFactory::makeFileNodePath(const String &path) const {
	return new PSPFilesystemNode(path, true);
}
