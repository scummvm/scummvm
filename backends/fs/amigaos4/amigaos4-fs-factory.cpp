#include "backends/fs/amigaos4/amigaos4-fs-factory.h"
#include "backends/fs/amigaos4/amigaos4-fs.cpp"

DECLARE_SINGLETON(AmigaOSFilesystemFactory);

AbstractFilesystemNode *AmigaOSFilesystemFactory::makeRootFileNode() const {
	return new AmigaOSFilesystemNode();
}

AbstractFilesystemNode *AmigaOSFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new AmigaOSFilesystemNode();
}

AbstractFilesystemNode *AmigaOSFilesystemFactory::makeFileNodePath(const String &path) const {
	return new AmigaOSFilesystemNode(path);
}
