#ifndef RONINCDFILESYSTEMFACTORY_H_
#define RONINCDFILESYSTEMFACTORY_H_

#include "backends/fs/AbstractFilesystemFactory.h"

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
	
	/**
	 * Destructor.
	 */
	virtual ~RoninCDFilesystemFactory() {};
		
	virtual AbstractFilesystemNode *makeRootFileNode() const;
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const;
	
protected:
	RoninCDFilesystemFactory() {};
		
private:
	static RoninCDFilesystemFactory *_instance;
};

#endif /*RONINCDFILESYSTEMFACTORY_H_*/
