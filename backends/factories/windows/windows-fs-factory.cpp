#include "backends/factories/windows/windows-fs-factory.h"
#include "backends/fs/windows/windows-fs.cpp"
#include "backends/file/base-file.h"

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

BaseFile *WindowsFilesystemFactory::makeBaseFile() const {
	return new BaseFile();
}
