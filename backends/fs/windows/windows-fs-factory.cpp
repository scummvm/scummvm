#include "backends/fs/windows/windows-fs-factory.h"
#include "backends/fs/windows/windows-fs.cpp"

DECLARE_SINGLETON(WindowsFilesystemFactory);

AbstractFilesystemNode *WindowsFilesystemFactory::makeRootFileNode() const {
	return new WindowsFilesystemNode();
}

AbstractFilesystemNode *WindowsFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new WindowsFilesystemNode("", true);
}

AbstractFilesystemNode *WindowsFilesystemFactory::makeFileNodePath(const String &path) const {
	return new WindowsFilesystemNode(path, false);
}
