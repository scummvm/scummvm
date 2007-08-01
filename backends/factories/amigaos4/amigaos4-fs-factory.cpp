#include "backends/factories/amigaos4/amigaos4-fs-factory.h"
#include "backends/fs/amigaos4/amigaos4-fs.cpp"
#include "backends/file/amigaos4/amigaos4-file.h"

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

BaseFile *AmigaOSFilesystemFactory::makeBaseFile() const {
	return new AmigaOSFile();
}
