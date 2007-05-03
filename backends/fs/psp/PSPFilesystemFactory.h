#ifndef PSPFILESYSTEMFACTORY_H_
#define PSPFILESYSTEMFACTORY_H_

#include "backends/fs/AbstractFilesystemFactory.h"

/**
 * Creates PSPFilesystemNode objects.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemFactory.
 */
class PSPFilesystemFactory : public AbstractFilesystemFactory {	
public:
	typedef Common::String String;
	
	/**
	 * Creates an instance of PSPFilesystemFactory using the Singleton pattern.
	 * 
	 * @return A unique instance of PSPFilesytemFactory.
	 */
	static PSPFilesystemFactory *instance();
	
	/**
	 * Destructor.
	 */
	virtual ~PSPFilesystemFactory() {};
		
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	PSPFilesystemFactory() {};
		
private:
	static PSPFilesystemFactory *_instance;
};

#endif /*PSPFILESYSTEMFACTORY_H_*/
