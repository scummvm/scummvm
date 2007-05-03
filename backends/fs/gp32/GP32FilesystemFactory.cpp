#include "backends/fs/gp32/GP32FilesystemFactory.h"
#include "backends/fs/gp32/gp32-fs.cpp"

GP32FilesystemFactory *GP32FilesystemFactory::_instance = 0;

GP32FilesystemFactory *GP32FilesystemFactory::instance(){
	if(_instance == 0){
		_instance = new GP32FilesystemFactory();
	}
	return _instance;
}

AbstractFilesystemNode *GP32FilesystemFactory::makeRootFileNode() const {
	return new GP32FilesystemNode();
}

AbstractFilesystemNode *GP32FilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new GP32FilesystemNode();
}

AbstractFilesystemNode *GP32FilesystemFactory::makeFileNodePath(const String &path) const {
	return new GP32FilesystemNode(path);
}
