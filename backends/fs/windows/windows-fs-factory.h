#ifndef WINDOWS_FILESYSTEM_FACTORY_H
#define WINDOWS_FILESYSTEM_FACTORY_H

#include "backends/fs/abstract-fs-factory.h"

/**
 * Creates WindowsFilesystemNode objects.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemFactory.
 */
class WindowsFilesystemFactory : public AbstractFilesystemFactory {	
public:
	typedef Common::String String;
	
	/**
	 * Creates an instance of WindowsFilesystemFactory using the Singleton pattern.
	 * 
	 * @return A unique instance of WindowsFilesytemFactory.
	 */
	static WindowsFilesystemFactory *instance();
	
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	WindowsFilesystemFactory() {};
		
private:
	static WindowsFilesystemFactory *_instance;
};

#endif /*WINDOWS_FILESYSTEM_FACTORY_H*/
