#include "backends/fs/palmos/palmos-fs-factory.h"
#include "backends/fs/palmos/palmos-fs.cpp"

PalmOSFilesystemFactory *PalmOSFilesystemFactory::_instance = 0;

PalmOSFilesystemFactory *PalmOSFilesystemFactory::instance(){
	if(_instance == 0){
		_instance = new PalmOSFilesystemFactory();
	}
	return _instance;
}

AbstractFilesystemNode *PalmOSFilesystemFactory::makeRootFileNode() const {
	return new PalmOSFilesystemNode();
}

AbstractFilesystemNode *PalmOSFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new PalmOSFilesystemNode();
}

AbstractFilesystemNode *PalmOSFilesystemFactory::makeFileNodePath(const String &path) const {
	return new PalmOSFilesystemNode(path);
}
