#ifndef ABSTRACT_FILESYSTEM_FACTORY_H
#define ABSTRACT_FILESYSTEM_FACTORY_H

#include "common/str.h"

/**
 * Creates concrete FilesystemNode objects depending on the current architecture.
 */
class AbstractFilesystemFactory {
public:
	typedef Common::String String;

	/**
	 * Destructor.
	 */
	virtual ~AbstractFilesystemFactory() {};
	
	/**
	 * Returns a node representing the "current directory".
	 * If your system does not support this concept, you can either try to
	 * emulate it or simply return some "sensible" default directory node,
	 * e.g. the same value as getRoot() returns.
	 */
	virtual AbstractFilesystemNode *makeCurrentDirectoryFileNode() const = 0;
	
	/**
	 * Construct a node based on a path; the path is in the same format as it
	 * would be for calls to fopen().
	 *
	 * Furthermore getNodeForPath(oldNode.path()) should create a new node
	 * identical to oldNode. Hence, we can use the "path" value for persistent
	 * storage e.g. in the config file.
	 *
	 * @param path The path string to create a FilesystemNode for.
	 */
	virtual AbstractFilesystemNode *makeFileNodePath(const String &path) const = 0;
	
	/**
	 * Returns a special node representing the filesystem root.
	 * The starting point for any file system browsing.
	 * 
	 * On Unix, this will be simply the node for / (the root directory).
	 * On Windows, it will be a special node which "contains" all drives (C:, D:, E:).
	 */
	virtual AbstractFilesystemNode *makeRootFileNode() const = 0;
};

#endif /*ABSTRACT_FILESYSTEM_FACTORY_H*/
