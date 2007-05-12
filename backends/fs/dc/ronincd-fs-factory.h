#ifndef RONINCD_FILESYSTEM_FACTORY_H
#define RONINCD_FILESYSTEM_FACTORY_H

#include "backends/fs/abstract-fs-factory.h"

/**
 * Creates RoninCDFilesystemNode objects.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemFactory.
 */
class RoninCDFilesystemFactory : public AbstractFilesystemFactory {	
public:
	typedef Common::String String;
	
	/**
	 * Creates an instance of RoninCDFilesystemFactory using the Singleton pattern.
	 * 
	 * @return A unique instance of RoninCDFilesytemFactory.
	 */
	static RoninCDFilesystemFactory *instance();
		
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	RoninCDFilesystemFactory() {};
		
private:
	static RoninCDFilesystemFactory *_instance;
};

#endif /*RONINCD_FILESYSTEM_FACTORY_H*/
