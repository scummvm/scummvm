#ifndef PS2FILESYSTEMFACTORY_H_
#define PS2FILESYSTEMFACTORY_H_

#include "backends/fs/AbstractFilesystemFactory.h"

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
	
	/**
	 * Destructor.
	 */
	virtual ~Ps2FilesystemFactory() {};
		
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	Ps2FilesystemFactory() {};
		
private:
	static Ps2FilesystemFactory *_instance;
};

#endif /*PS2FILESYSTEMFACTORY_H_*/
