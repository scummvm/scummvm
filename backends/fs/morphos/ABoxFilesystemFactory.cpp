#include "backends/fs/morphos/ABoxFilesystemFactory.h"
#include "backends/fs/morphos/abox-fs.cpp"

ABoxFilesystemFactory *ABoxFilesystemFactory::_instance = 0;

ABoxFilesystemFactory *ABoxFilesystemFactory::instance(){
	if(_instance == 0){
		_instance = new ABoxFilesystemFactory();
	}
	return _instance;
}

AbstractFilesystemNode *ABoxFilesystemFactory::makeRootFileNode() const {
	return new ABoxFilesystemNode();
}

AbstractFilesystemNode *ABoxFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new ABoxFilesystemNode();
}

AbstractFilesystemNode *ABoxFilesystemFactory::makeFileNodePath(const String &path) const {
	return new ABoxFilesystemNode(path);
}
