#ifndef GP32_FILESYSTEM_FACTORY_H
#define GP32_FILESYSTEM_FACTORY_H

#include "common/singleton.h"
#include "backends/fs/abstract-fs-factory.h"

/**
 * Creates GP32FilesystemNode objects.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemFactory.
 */
class GP32FilesystemFactory : public AbstractFilesystemFactory, public Common::Singleton<GP32FilesystemFactory> {	
public:
	typedef Common::String String;
		
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	GP32FilesystemFactory() {};
		
private:
	friend class Common::Singleton<SingletonBaseType>;
};

#endif /*GP32_FILESYSTEM_FACTORY_H*/
