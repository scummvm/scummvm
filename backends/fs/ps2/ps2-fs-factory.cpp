#include "backends/fs/ps2/ps2-fs-factory.h"
#include "backends/fs/ps2/ps2-fs.cpp"

Ps2FilesystemFactory *Ps2FilesystemFactory::_instance = 0;

Ps2FilesystemFactory *Ps2FilesystemFactory::instance(){
	if(_instance == 0){
		_instance = new Ps2FilesystemFactory();
	}
	return _instance;
}

AbstractFilesystemNode *Ps2FilesystemFactory::makeRootFileNode() const {
	return new Ps2FilesystemNode();
}

AbstractFilesystemNode *Ps2FilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new Ps2FilesystemNode();
}

AbstractFilesystemNode *Ps2FilesystemFactory::makeFileNodePath(const String &path) const {
	return new Ps2FilesystemNode(path);
}
