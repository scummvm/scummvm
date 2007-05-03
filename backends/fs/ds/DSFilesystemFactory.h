#ifndef DSFILESYSTEMFACTORY_H_
#define DSFILESYSTEMFACTORY_H_

#include "backends/fs/AbstractFilesystemFactory.h"

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
	
	/**
	 * Destructor.
	 */
	virtual ~DSFilesystemFactory() {};
		
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	DSFilesystemFactory() {};
		
private:
	static DSFilesystemFactory *_instance;
};

#endif /*DSFILESYSTEMFACTORY_H_*/
