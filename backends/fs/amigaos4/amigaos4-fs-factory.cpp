#include "backends/fs/amigaos4/amigaos4-fs-factory.h"
#include "backends/fs/amigaos4/amigaos4-fs.cpp"

AmigaOSFilesystemFactory *AmigaOSFilesystemFactory::_instance = 0;

AmigaOSFilesystemFactory *AmigaOSFilesystemFactory::instance(){
	if(_instance == 0){
		_instance = new AmigaOSFilesystemFactory();
	}
	return _instance;
}

AbstractFilesystemNode *AmigaOSFilesystemFactory::makeRootFileNode() const {
	return new AmigaOSFilesystemNode();
}

AbstractFilesystemNode *AmigaOSFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new AmigaOSFilesystemNode();
}

AbstractFilesystemNode *AmigaOSFilesystemFactory::makeFileNodePath(const String &path) const {
	return new AmigaOSFilesystemNode(path);
}
