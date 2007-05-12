#ifndef DS_FILESYSTEM_FACTORY_H
#define DS_FILESYSTEM_FACTORY_H

#include "backends/fs/abstract-fs-factory.h"

/**
 * Creates DSFilesystemNode objects.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemFactory.
 */
class DSFilesystemFactory : public AbstractFilesystemFactory, public Common::Singleton<DSFilesystemFactory> {	
public:
	typedef Common::String String;
		
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	DSFilesystemFactory() {};
		
private:
	friend class Common::Singleton<SingletonBaseType>;
};

#endif /*DS_FILESYSTEM_FACTORY_H*/
