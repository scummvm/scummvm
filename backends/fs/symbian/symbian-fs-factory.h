#ifndef SYMBIAN_FILESYSTEM_FACTORY_H
#define SYMBIAN_FILESYSTEM_FACTORY_H

#include "common/singleton.h"
#include "backends/fs/abstract-fs-factory.h"

/**
 * Creates SymbianFilesystemNode objects.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemFactory.
 */
class SymbianFilesystemFactory : public AbstractFilesystemFactory, public Common::Singleton<SymbianFilesystemFactory> {	
public:
	typedef Common::String String;
		
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	SymbianFilesystemFactory() {};
		
private:
	friend class Common::Singleton<SingletonBaseType>;
};

#endif /*SYMBIAN_FILESYSTEM_FACTORY_H*/
