#include "backends/fs/windows/windows-fs-factory.h"
#include "backends/fs/windows/windows-fs.cpp"

WindowsFilesystemFactory *WindowsFilesystemFactory::_instance = 0;

WindowsFilesystemFactory *WindowsFilesystemFactory::instance(){
	if(_instance == 0){
		_instance = new WindowsFilesystemFactory();
	}
	return _instance;
}

AbstractFilesystemNode *WindowsFilesystemFactory::makeRootFileNode() const {
	return new WindowsFilesystemNode();
}

AbstractFilesystemNode *WindowsFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new WindowsFilesystemNode(NULL, true);
}

AbstractFilesystemNode *WindowsFilesystemFactory::makeFileNodePath(const String &path) const {
	return new WindowsFilesystemNode(path, false);
}
