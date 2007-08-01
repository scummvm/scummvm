#ifndef WINDOWS_FILESYSTEM_FACTORY_H
#define WINDOWS_FILESYSTEM_FACTORY_H

#include "common/singleton.h"
#include "backends/factories/abstract-fs-factory.h"

/**
 * Creates WindowsFilesystemNode objects.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemFactory.
 */
class WindowsFilesystemFactory : public AbstractFilesystemFactory, public Common::Singleton<WindowsFilesystemFactory> {	
public:
	typedef Common::String String;
	
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	virtual BaseFile *makeBaseFile() const;
	
protected:
	WindowsFilesystemFactory() {};
		
private:
	friend class Common::Singleton<SingletonBaseType>;
};

#endif /*WINDOWS_FILESYSTEM_FACTORY_H*/
