#include "backends/factories/ds/ds-fs-factory.h"
#include "backends/fs/ds/ds-fs.cpp"
#include "backends/file/ds/ds-file.h"
#include "dsmain.h" //for the isGBAMPAvailable() function

DECLARE_SINGLETON(DSFilesystemFactory);

AbstractFilesystemNode *DSFilesystemFactory::makeRootFileNode() const {
	if (DS::isGBAMPAvailable()) {
		return new DS::GBAMPFileSystemNode();
	} else {
		return new DS::DSFileSystemNode();
	}
}

AbstractFilesystemNode *DSFilesystemFactory::makeCurrentDirectoryFileNode() const {
	if (DS::isGBAMPAvailable()) {
		return new DS::GBAMPFileSystemNode();
	} else {
		return new DS::DSFileSystemNode();
	}
}

AbstractFilesystemNode *DSFilesystemFactory::makeFileNodePath(const String &path) const {
	if (DS::isGBAMPAvailable()) {
		return new DS::GBAMPFileSystemNode(path);
	} else {
		return new DS::DSFileSystemNode(path);
	}
}

BaseFile *DSFilesystemFactory::makeBaseFile() const {
	return new DSFile();
}
