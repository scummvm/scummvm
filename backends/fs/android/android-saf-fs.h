/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ANDROID_SAF_FILESYSTEM_H
#define ANDROID_SAF_FILESYSTEM_H

#include <jni.h>

#include "backends/fs/abstract-fs.h"

#include "backends/fs/android/android-fs.h"

/**
 * Implementation of the ScummVM file system API.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class AndroidSAFFilesystemNode final : public AbstractFSNode, public AndroidFSNode {
protected:
	// SAFFSTree
	static jmethodID _MID_getTreeId;
	static jmethodID _MID_pathToNode;
	static jmethodID _MID_getChildren;
	static jmethodID _MID_getChild;
	static jmethodID _MID_createDirectory;
	static jmethodID _MID_createFile;
	static jmethodID _MID_createReadStream;
	static jmethodID _MID_createWriteStream;
	static jmethodID _MID_removeNode;
	static jmethodID _MID_removeTree;

	static jfieldID _FID__treeName;
	static jfieldID _FID__root;

	// SAFFSNode
	static jfieldID _FID__parent;
	static jfieldID _FID__path;
	static jfieldID _FID__documentId;
	static jfieldID _FID__flags;

	static bool _JNIinit;

protected:
	static const int DIRECTORY = 1;
	static const int WRITABLE  = 2;
	static const int READABLE  = 4;

	jobject _safTree;
	// When null, node doesn't exist yet
	// In this case _path is the parent path, _newName the node name and _safParent the parent SAF object
	jobject _safNode;

	Common::String _path;
	int _flags;
	jobject _safParent;

	// Used when creating a new node
	// Also used for root node to store its pretty name
	Common::String _newName;

	/**
	 * Creates an AndroidSAFFilesystemNode given its tree and its node
	 *
	 * @param safTree SAF root in Java side
	 * @param safNode SAF node in Java side
	 */
	AndroidSAFFilesystemNode(jobject safTree, jobject safNode);

public:
	static const char SAF_MOUNT_POINT[];

	/**
	 * Init JNI parts related to SAF
	 * Called by AndroidFilesystemFactory::AndroidFilesystemFactory()
	 */
	static void initJNI();

	/**
	 * Creates an AndroidSAFFilesystemNode given its absolute path
	 *
	 * @param path Path of the node
	 */
	static AndroidSAFFilesystemNode *makeFromPath(const Common::String &path);

	/**
	 * Creates an AndroidSAFFilesystemNode given its tree object
	 * @param safTree SAF root in Java side. Must be a local reference and must not be used after this call.
	 *
	 */
	static AndroidSAFFilesystemNode *makeFromTree(jobject safTree);

	/**
	 * Copy constructor.
	 *
	 * @note Needed because we keep references
	 */
	AndroidSAFFilesystemNode(const AndroidSAFFilesystemNode &node);

	/**
	 * Destructor.
	 */
	~AndroidSAFFilesystemNode() override;

	bool exists() const override { return _safNode != nullptr; }
	Common::U32String getDisplayName() const override { return Common::U32String(getName()); }
	Common::String getName() const override;
	Common::String getPath() const override;
	bool isDirectory() const override { return _flags & DIRECTORY; }
	bool isReadable() const override { return _flags & READABLE; }
	bool isWritable() const override { return _flags & WRITABLE; }

	AbstractFSNode *getChild(const Common::String &n) const override;
	bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const override;
	AbstractFSNode *getParent() const override;

	Common::SeekableReadStream *createReadStream() override;
	Common::SeekableWriteStream *createWriteStream(bool atomic) override;
	bool createDirectory() override;

	bool remove() override;

	/**
	 * Removes the SAF tree.
	 * Only works on the root node
	 */
	void removeTree();
protected:
	/**
	 * Creates an non-existent AndroidSAFFilesystemNode given its tree, parent node and name
	 *
	 * @param safTree SAF root in Java side
	 * @param safParent SAF parent node in Java side
	 * @param path Parent path
	 * @param name Item name
	 */
	AndroidSAFFilesystemNode(jobject safTree, jobject safParent,
	                         const Common::String &path, const Common::String &name);

	void cacheData();
};

class AddSAFFakeNode final : public AbstractFSNode, public AndroidFSNode {
protected:
	AbstractFSNode *getChild(const Common::String &name) const override;
	AbstractFSNode *getParent() const override;

public:
	static const char SAF_ADD_FAKE_PATH[];

	AddSAFFakeNode(bool fromPath) : _proxied(nullptr), _fromPath(fromPath) { }
	~AddSAFFakeNode() override;

	bool exists() const override;

	bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const override;

	// I18N: This is displayed in the file browser to let the user choose a new folder for Android Storage Attached Framework
	Common::U32String getDisplayName() const override;
	Common::String getName() const override;
	Common::String getPath() const override;

	bool isDirectory() const override { return true; }
	bool isReadable() const override;
	bool isWritable() const override;

	Common::SeekableReadStream *createReadStream() override { return nullptr; }
	Common::SeekableWriteStream *createWriteStream(bool atomic) override { return nullptr; }

	bool createDirectory() override { return false; }
	bool remove() override { return false; }

private:
	void makeProxySAF() const;

	bool _fromPath;
	mutable AbstractFSNode *_proxied;
};
#endif
