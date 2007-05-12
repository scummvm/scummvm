#ifndef PS2_FILESYSTEM_FACTORY_H
#define PS2_FILESYSTEM_FACTORY_H

#include "backends/fs/abstract-fs-factory.h"

/**
 * Creates PS2FilesystemNode objects.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemFactory.
 */
class Ps2FilesystemFactory : public AbstractFilesystemFactory {	
public:
	typedef Common::String String;
	
	/**
	 * Creates an instance of Ps2FilesystemFactory using the Singleton pattern.
	 * 
	 * @return A unique instance of Ps2FilesytemFactory.
	 */
	static Ps2FilesystemFactory *instance();
		
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	Ps2FilesystemFactory() {};
		
private:
	static Ps2FilesystemFactory *_instance;
};

#endif /*PS2_FILESYSTEM_FACTORY_H*/
