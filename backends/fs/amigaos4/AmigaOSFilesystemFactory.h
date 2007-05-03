#ifndef AMIGAOSFILESYSTEMFACTORY_H_
#define AMIGAOSFILESYSTEMFACTORY_H_

#include "backends/fs/AbstractFilesystemFactory.h"

/**
 * Creates AmigaOSFilesystemNode objects.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemFactory.
 */
class AmigaOSFilesystemFactory : public AbstractFilesystemFactory {	
public:
	typedef Common::String String;
	
	/**
	 * Creates an instance of AmigaOSFilesystemFactory using the Singleton pattern.
	 * 
	 * @return A unique instance of AmigaOSFilesytemFactory.
	 */
	static AmigaOSFilesystemFactory *instance();
	
	/**
	 * Destructor.
	 */
	virtual ~AmigaOSFilesystemFactory() {};
		
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	AmigaOSFilesystemFactory() {};
		
private:
	static AmigaOSFilesystemFactory *_instance;
};

#endif /*AMIGAOSFILESYSTEMFACTORY_H_*/
