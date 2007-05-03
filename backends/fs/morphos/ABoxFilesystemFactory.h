#ifndef ABOXFILESYSTEMFACTORY_H_
#define ABOXFILESYSTEMFACTORY_H_

#include "backends/fs/AbstractFilesystemFactory.h"

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
	
	/**
	 * Destructor.
	 */
	virtual ~ABoxFilesystemFactory() {};
		
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	ABoxFilesystemFactory() {};
		
private:
	static ABoxFilesystemFactory *_instance;
};

#endif /*ABOXFILESYSTEMFACTORY_H_*/
