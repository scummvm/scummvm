#ifndef DS_FILESYSTEM_FACTORY_H
#define DS_FILESYSTEM_FACTORY_H

#include "backends/fs/abstract-fs-factory.h"

/**
 * Creates DSFilesystemNode objects.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemFactory.
 */
class DSFilesystemFactory : public AbstractFilesystemFactory {	
public:
	typedef Common::String String;
	
	/**
	 * Creates an instance of DSFilesystemFactory using the Singleton pattern.
	 * 
	 * @return A unique instance of DSFilesytemFactory.
	 */
	static DSFilesystemFactory *instance();
		
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	DSFilesystemFactory() {};
		
private:
	static DSFilesystemFactory *_instance;
};

#endif /*DS_FILESYSTEM_FACTORY_H*/
