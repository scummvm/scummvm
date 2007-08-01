#ifndef PS2_FILESYSTEM_FACTORY_H
#define PS2_FILESYSTEM_FACTORY_H

#include "common/singleton.h"
#include "backends/factories/abstract-fs-factory.h"

/**
 * Creates PS2FilesystemNode objects.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemFactory.
 */
class Ps2FilesystemFactory : public AbstractFilesystemFactory, public Common::Singleton<Ps2FilesystemFactory> {	
public:
	typedef Common::String String;
		
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	virtual BaseFile *makeBaseFile() const;
	
protected:
	Ps2FilesystemFactory() {};
		
private:
	friend class Common::Singleton<SingletonBaseType>;
};

#endif /*PS2_FILESYSTEM_FACTORY_H*/
