#include "backends/fs/posix/posix-fs-factory.h"
#include "backends/fs/posix/posix-fs.cpp"

POSIXFilesystemFactory *POSIXFilesystemFactory::_instance = 0;

POSIXFilesystemFactory *POSIXFilesystemFactory::instance(){
	if(_instance == 0){
		_instance = new POSIXFilesystemFactory();
	}
	return _instance;
}

AbstractFilesystemNode *POSIXFilesystemFactory::makeRootFileNode() const {
	return new POSIXFilesystemNode();
}

AbstractFilesystemNode *POSIXFilesystemFactory::makeCurrentDirectoryFileNode() const {
	char buf[MAXPATHLEN];
	getcwd(buf, MAXPATHLEN);
	return new POSIXFilesystemNode(buf, true);
}

AbstractFilesystemNode *POSIXFilesystemFactory::makeFileNodePath(const String &path) const {
	return new POSIXFilesystemNode(path, true);
}
