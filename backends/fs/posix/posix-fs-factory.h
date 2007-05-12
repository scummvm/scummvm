#ifndef POSIX_FILESYSTEM_FACTORY_H
#define POSIX_FILESYSTEM_FACTORY_H

#include "backends/fs/abstract-fs-factory.h"

/**
 * Creates POSIXFilesystemNode objects.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemFactory.
 */
class POSIXFilesystemFactory : public AbstractFilesystemFactory {	
public:
	typedef Common::String String;
	
	/**
	 * Creates an instance of POSIXFilesystemFactory using the Singleton pattern.
	 * 
	 * @return A unique instance of POSIXFilesytemFactory.
	 */
	static POSIXFilesystemFactory *instance();
		
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	POSIXFilesystemFactory() {};
		
private:
	static POSIXFilesystemFactory *_instance;
};

#endif /*POSIX_FILESYSTEM_FACTORY_H*/
