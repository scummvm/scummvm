#ifndef AMIGAOS_FILESYSTEM_FACTORY_H
#define AMIGAOS_FILESYSTEM_FACTORY_H

#include "backends/fs/abstract-fs-factory.h"

/**
 * Creates AmigaOSFilesystemNode objects.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemFactory.
 */
class AmigaOSFilesystemFactory : public AbstractFilesystemFactory {	
public:
	typedef Common::String String;
	
	/**
	 * Creates an instance of AmigaOSFilesystemFactory using the Singleton pattern.
	 * 
	 * @return A unique instance of AmigaOSFilesytemFactory.
	 */
	static AmigaOSFilesystemFactory *instance();
		
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	AmigaOSFilesystemFactory() {};
		
private:
	static AmigaOSFilesystemFactory *_instance;
};

#endif /*AMIGAOS_FILESYSTEM_FACTORY_H*/
