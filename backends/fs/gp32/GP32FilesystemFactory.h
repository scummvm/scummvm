#ifndef GP32FILESYSTEMFACTORY_H_
#define GP32FILESYSTEMFACTORY_H_

#include "backends/fs/AbstractFilesystemFactory.h"

/**
 * Creates GP32FilesystemNode objects.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemFactory.
 */
class GP32FilesystemFactory : public AbstractFilesystemFactory {	
public:
	typedef Common::String String;
	
	/**
	 * Creates an instance of GP32FilesystemFactory using the Singleton pattern.
	 * 
	 * @return A unique instance of GP32FilesytemFactory.
	 */
	static GP32FilesystemFactory *instance();
	
	/**
	 * Destructor.
	 */
	virtual ~GP32FilesystemFactory() {};
		
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	GP32FilesystemFactory() {};
		
private:
	static GP32FilesystemFactory *_instance;
};

#endif /*GP32FILESYSTEMFACTORY_H_*/
