#ifndef ABOX_FILESYSTEM_FACTORY_H
#define ABOX_FILESYSTEM_FACTORY_H

#include "backends/fs/abstract-fs-factory.h"

/**
 * Creates ABoxFilesystemNode objects.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemFactory.
 */
class ABoxFilesystemFactory : public AbstractFilesystemFactory {	
public:
	typedef Common::String String;
	
	/**
	 * Creates an instance of ABoxFilesystemFactory using the Singleton pattern.
	 * 
	 * @return A unique instance of ABoxFilesytemFactory.
	 */
	static ABoxFilesystemFactory *instance();
		
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	ABoxFilesystemFactory() {};
		
private:
	static ABoxFilesystemFactory *_instance;
};

#endif /*ABOX_FILESYSTEM_FACTORY_H*/
