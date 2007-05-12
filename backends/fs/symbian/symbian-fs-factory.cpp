#include "backends/fs/symbian/symbian-fs-factory.h"
#include "backends/fs/symbian/symbian-fs.cpp"

SymbianFilesystemFactory *SymbianFilesystemFactory::_instance = 0;

SymbianFilesystemFactory *SymbianFilesystemFactory::instance(){
	if(_instance == 0){
		_instance = new SymbianFilesystemFactory();
	}
	return _instance;
}

AbstractFilesystemNode *SymbianFilesystemFactory::makeRootFileNode() const {
	return new SymbianFilesystemNode(true);
}

AbstractFilesystemNode *SymbianFilesystemFactory::makeCurrentDirectoryFileNode() const {
	char path[MAXPATHLEN];
	getcwd(path, MAXPATHLEN);
	return new SymbianFilesystemNode(path);
}

AbstractFilesystemNode *SymbianFilesystemFactory::makeFileNodePath(const String &path) const {
	return new SymbianFilesystemNode(path);
}
