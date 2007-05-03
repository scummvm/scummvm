#include "backends/fs/psp/PSPFilesystemFactory.h"
#include "backends/fs/psp/psp_fs.cpp"

PSPFilesystemFactory *PSPFilesystemFactory::_instance = 0;

PSPFilesystemFactory *PSPFilesystemFactory::instance(){
	if(_instance == 0){
		_instance = new PSPFilesystemFactory();
	}
	return _instance;
}

AbstractFilesystemNode *PSPFilesystemFactory::makeRootFileNode() const {
	return new PSPFilesystemNode();
}

AbstractFilesystemNode *PSPFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new PSPFilesystemNode();
}

AbstractFilesystemNode *PSPFilesystemFactory::makeFileNodePath(const String &path) const {
	return new PSPFilesystemNode(path, true);
}
