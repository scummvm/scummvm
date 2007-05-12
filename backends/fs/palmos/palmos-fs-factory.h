#ifndef PALMOS_FILESYSTEM_FACTORY_H
#define PALMOS_FILESYSTEM_FACTORY_H

#include "backends/fs/abstract-fs-factory.h"

/**
 * Creates PalmOSFilesystemNode objects.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemFactory.
 */
class PalmOSFilesystemFactory : public AbstractFilesystemFactory {	
public:
	typedef Common::String String;
	
	/**
	 * Creates an instance of PalmOSFilesystemFactory using the Singleton pattern.
	 * 
	 * @return A unique instance of PalmOSFilesytemFactory.
	 */
	static PalmOSFilesystemFactory *instance();
		
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	PalmOSFilesystemFactory() {};
		
private:
	static PalmOSFilesystemFactory *_instance;
};

#endif /*PALMOS_FILESYSTEM_FACTORY_H*/
