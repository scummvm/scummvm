#include "backends/fs/dc/ronincd-fs-factory.h"
#include "backends/fs/dc/dc-fs.cpp"

RoninCDFilesystemFactory *RoninCDFilesystemFactory::_instance = 0;

RoninCDFilesystemFactory *RoninCDFilesystemFactory::instance(){
	if(_instance == 0){
		_instance = new RoninCDFilesystemFactory();
	}
	return _instance;
}

AbstractFilesystemNode *RoninCDFilesystemFactory::makeRootFileNode() const {
	return new RoninCDFilesystemNode();
}

AbstractFilesystemNode *RoninCDFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new RoninCDFilesystemNode();
}

AbstractFilesystemNode *RoninCDFilesystemFactory::makeFileNodePath(const String &path) const {
	return new RoninCDFilesystemNode(path, true);
}
