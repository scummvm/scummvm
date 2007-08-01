#include "backends/factories/psp/psp-fs-factory.h"
#include "backends/fs/psp/psp_fs.cpp"
#include "backends/file/base-file.h"

DECLARE_SINGLETON(PSPFilesystemFactory);

AbstractFilesystemNode *PSPFilesystemFactory::makeRootFileNode() const {
	return new PSPFilesystemNode();
}

AbstractFilesystemNode *PSPFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new PSPFilesystemNode();
}

AbstractFilesystemNode *PSPFilesystemFactory::makeFileNodePath(const String &path) const {
	return new PSPFilesystemNode(path, true);
}

BaseFile *PSPFilesystemFactory::makeBaseFile() const {
	return new BaseFile();
}
