#ifndef WINDOWSFILESYSTEMFACTORY_H_
#define WINDOWSFILESYSTEMFACTORY_H_

#include "backends/fs/AbstractFilesystemFactory.h"

/**
 * Creates WindowsFilesystemNode objects.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemFactory.
 */
class WindowsFilesystemFactory : public AbstractFilesystemFactory {	
public:
	typedef Common::String String;
	
	/**
	 * Creates an instance of WindowsFilesystemFactory using the Singleton pattern.
	 * 
	 * @return A unique instance of WindowsFilesytemFactory.
	 */
	static WindowsFilesystemFactory *instance();
	
	/**
	 * Destructor.
	 */
	virtual ~WindowsFilesystemFactory() {};
	
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	WindowsFilesystemFactory() {};
		
private:
	static WindowsFilesystemFactory *_instance;
};

#endif /*WINDOWSFILESYSTEMFACTORY_H_*/
