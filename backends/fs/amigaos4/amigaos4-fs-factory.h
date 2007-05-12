#ifndef AMIGAOS_FILESYSTEM_FACTORY_H
#define AMIGAOS_FILESYSTEM_FACTORY_H

#include "backends/fs/abstract-fs-factory.h"

/**
 * Creates AmigaOSFilesystemNode objects.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemFactory.
 */
class AmigaOSFilesystemFactory : public AbstractFilesystemFactory, public Common::Singleton<AmigaOSFilesystemFactory> {	
public:
	typedef Common::String String;
		
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	AmigaOSFilesystemFactory() {};
		
private:
	friend class Common::Singleton<SingletonBaseType>;
};

#endif /*AMIGAOS_FILESYSTEM_FACTORY_H*/
