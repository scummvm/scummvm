#ifndef SYMBIANFILESYSTEMFACTORY_H_
#define SYMBIANFILESYSTEMFACTORY_H_

#include "backends/fs/AbstractFilesystemFactory.h"

/**
 * Creates SymbianFilesystemNode objects.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemFactory.
 */
class SymbianFilesystemFactory : public AbstractFilesystemFactory {	
public:
	typedef Common::String String;
	
	/**
	 * Creates an instance of SymbianFilesystemFactory using the Singleton pattern.
	 * 
	 * @return A unique instance of SymbianFilesytemFactory.
	 */
	static SymbianFilesystemFactory *instance();
	
	/**
	 * Destructor.
	 */
	virtual ~SymbianFilesystemFactory() {};
		
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	SymbianFilesystemFactory() {};
		
private:
	static SymbianFilesystemFactory *_instance;
};

#endif /*SYMBIANFILESYSTEMFACTORY_H_*/
