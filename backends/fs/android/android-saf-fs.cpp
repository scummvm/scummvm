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

// Allow use of stuff in <time.h> and abort()
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_abort

// Disable printf override in common/forbidden.h to avoid
// clashes with log.h from the Android SDK.
// That header file uses
//   __attribute__ ((format(printf, 3, 4)))
// which gets messed up by our override mechanism; this could
// be avoided by either changing the Android SDK to use the equally
// legal and valid
//   __attribute__ ((format(__printf__, 3, 4)))
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the Android port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

// Allow calling of fdopen
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE

// Allow calling of close system call
#include <unistd.h>

#include "backends/platform/android/android.h"
#include "backends/platform/android/jni-android.h"

#include "backends/fs/android/android-fs-factory.h"
#include "backends/fs/android/android-saf-fs.h"

#include "backends/fs/posix/posix-iostream.h"

#include "common/debug.h"
#include "common/translation.h"
#include "common/util.h"

jmethodID AndroidSAFFilesystemNode::_MID_getTreeId = 0;
jmethodID AndroidSAFFilesystemNode::_MID_pathToNode = 0;
jmethodID AndroidSAFFilesystemNode::_MID_getChildren = 0;
jmethodID AndroidSAFFilesystemNode::_MID_getChild = 0;
jmethodID AndroidSAFFilesystemNode::_MID_createDirectory = 0;
jmethodID AndroidSAFFilesystemNode::_MID_createFile = 0;
jmethodID AndroidSAFFilesystemNode::_MID_createReadStream = 0;
jmethodID AndroidSAFFilesystemNode::_MID_createWriteStream = 0;
jmethodID AndroidSAFFilesystemNode::_MID_removeNode = 0;
jmethodID AndroidSAFFilesystemNode::_MID_removeTree = 0;

jfieldID AndroidSAFFilesystemNode::_FID__treeName = 0;
jfieldID AndroidSAFFilesystemNode::_FID__root = 0;

jfieldID AndroidSAFFilesystemNode::_FID__parent = 0;
jfieldID AndroidSAFFilesystemNode::_FID__path = 0;
jfieldID AndroidSAFFilesystemNode::_FID__documentId = 0;
jfieldID AndroidSAFFilesystemNode::_FID__flags = 0;

bool AndroidSAFFilesystemNode::_JNIinit = false;

const char AndroidSAFFilesystemNode::SAF_MOUNT_POINT[] = "/saf/";

void AndroidSAFFilesystemNode::initJNI() {
	if (_JNIinit) {
		return;
	}

	JNIEnv *env = JNI::getEnv();

#define FIND_METHOD(prefix, name, signature) do {                           \
    _MID_ ## prefix ## name = env->GetMethodID(cls, #name, signature);      \
        if (_MID_ ## prefix ## name == 0)                                   \
            error("Can't find method ID " #name);                           \
    } while (0)
#define FIND_FIELD(prefix, name, signature) do {                            \
    _FID_ ## prefix ## name = env->GetFieldID(cls, #name, signature);       \
        if (_FID_ ## prefix ## name == 0)                                   \
            error("Can't find field ID " #name);                            \
    } while (0)
#define SAFFSNodeSig "Lorg/scummvm/scummvm/SAFFSTree$SAFFSNode;"

	jclass cls = env->FindClass("org/scummvm/scummvm/SAFFSTree");

	FIND_METHOD(, getTreeId, "()Ljava/lang/String;");
	FIND_METHOD(, pathToNode, "(Ljava/lang/String;)" SAFFSNodeSig);
	FIND_METHOD(, getChildren, "(" SAFFSNodeSig ")[" SAFFSNodeSig);
	FIND_METHOD(, getChild, "(" SAFFSNodeSig "Ljava/lang/String;)" SAFFSNodeSig);
	FIND_METHOD(, createDirectory, "(" SAFFSNodeSig "Ljava/lang/String;)" SAFFSNodeSig);
	FIND_METHOD(, createFile, "(" SAFFSNodeSig "Ljava/lang/String;)" SAFFSNodeSig);
	FIND_METHOD(, createReadStream, "(" SAFFSNodeSig ")I");
	FIND_METHOD(, createWriteStream, "(" SAFFSNodeSig ")I");
	FIND_METHOD(, removeNode, "(" SAFFSNodeSig ")Z");
	FIND_METHOD(, removeTree, "()V");

	FIND_FIELD(, _treeName, "Ljava/lang/String;");
	FIND_FIELD(, _root, SAFFSNodeSig);

	cls = env->FindClass("org/scummvm/scummvm/SAFFSTree$SAFFSNode");

	FIND_FIELD(, _parent, SAFFSNodeSig);
	FIND_FIELD(, _path, "Ljava/lang/String;");
	FIND_FIELD(, _documentId, "Ljava/lang/String;");
	FIND_FIELD(, _flags, "I");

#undef SAFFSNodeSig
#undef FIND_FIELD
#undef FIND_METHOD

	_JNIinit = true;
}

AndroidSAFFilesystemNode *AndroidSAFFilesystemNode::makeFromPath(const Common::String &path) {
	if (!path.hasPrefix(SAF_MOUNT_POINT)) {
		// Not a SAF mount point
		return nullptr;
	}

	// Path is in the form /saf/<treeid>/<path>
	size_t pos = path.findFirstOf('/', sizeof(SAF_MOUNT_POINT) - 1);
	Common::String treeId;
	Common::String realPath;
	if (pos == Common::String::npos) {
		treeId = path.substr(sizeof(SAF_MOUNT_POINT) - 1);
	} else {
		treeId = path.substr(sizeof(SAF_MOUNT_POINT) - 1, pos - sizeof(SAF_MOUNT_POINT) + 1);
		realPath = path.substr(pos);
	}

	jobject safTree = JNI::findSAFTree(treeId);
	if (!safTree) {
		LOGW("AndroidSAFFilesystemNode::makeFromPath: tree id %s not found", treeId.c_str());
		return nullptr;
	}

	JNIEnv *env = JNI::getEnv();

	jstring pathObj = env->NewStringUTF(realPath.c_str());

	jobject node = env->CallObjectMethod(safTree, _MID_pathToNode, pathObj);

	env->DeleteLocalRef(pathObj);

	if (env->ExceptionCheck()) {
		LOGE("SAFFSTree::pathToNode failed");

		env->ExceptionDescribe();
		env->ExceptionClear();

		env->DeleteLocalRef(safTree);
		return nullptr;
	}

	if (node) {
		AndroidSAFFilesystemNode *ret = new AndroidSAFFilesystemNode(safTree, node);

		env->DeleteLocalRef(node);
		env->DeleteLocalRef(safTree);

		return ret;
	}

	// Node doesn't exist: we will try to make a node from the parent and
	// if it works we will create a non-existent node

	pos = realPath.findLastOf('/');
	if (pos == Common::String::npos || pos == 0) {
		// No / in path or at root, no parent and we have a tree: it's all good
		if (pos == 0) {
			realPath = realPath.substr(1);
		}
		AndroidSAFFilesystemNode *parent = makeFromTree(safTree);
		AndroidSAFFilesystemNode *ret = static_cast<AndroidSAFFilesystemNode *>(parent->getChild(realPath));
		delete parent;

		// safTree has already been released by makeFromTree
		return ret;
	}

	Common::String baseName(realPath.substr(pos + 1));
	realPath.erase(pos);

	pathObj = env->NewStringUTF(realPath.c_str());

	node = env->CallObjectMethod(safTree, _MID_pathToNode, pathObj);

	env->DeleteLocalRef(pathObj);

	if (env->ExceptionCheck()) {
		LOGE("SAFFSTree::pathToNode failed");

		env->ExceptionDescribe();
		env->ExceptionClear();

		env->DeleteLocalRef(safTree);
		return nullptr;
	}

	if (node) {
		AndroidSAFFilesystemNode *parent = new AndroidSAFFilesystemNode(safTree, node);
		env->DeleteLocalRef(node);
		env->DeleteLocalRef(safTree);

		AndroidSAFFilesystemNode *ret = static_cast<AndroidSAFFilesystemNode *>(parent->getChild(baseName));
		delete parent;

		return ret;
	}

	env->DeleteLocalRef(safTree);
	return nullptr;
}

AndroidSAFFilesystemNode *AndroidSAFFilesystemNode::makeFromTree(jobject safTree) {
	assert(safTree);

	JNIEnv *env = JNI::getEnv();

	jobject node = env->GetObjectField(safTree, _FID__root);
	if (!node) {
		env->DeleteLocalRef(safTree);
		return nullptr;
	}

	AndroidSAFFilesystemNode *ret = new AndroidSAFFilesystemNode(safTree, node);

	env->DeleteLocalRef(node);
	env->DeleteLocalRef(safTree);

	return ret;
}

AndroidSAFFilesystemNode::AndroidSAFFilesystemNode(jobject safTree, jobject safNode) :
	_flags(0), _safParent(nullptr) {

	JNIEnv *env = JNI::getEnv();

	_safTree = env->NewGlobalRef(safTree);
	assert(_safTree);
	_safNode = env->NewGlobalRef(safNode);
	assert(_safNode);

	cacheData();
}

AndroidSAFFilesystemNode::AndroidSAFFilesystemNode(jobject safTree, jobject safParent,
        const Common::String &path, const Common::String &name) :
	_safNode(nullptr), _flags(0), _safParent(nullptr) {

	JNIEnv *env = JNI::getEnv();

	_safTree = env->NewGlobalRef(safTree);
	assert(_safTree);
	_safParent = env->NewGlobalRef(safParent);
	assert(_safParent);

	// In this case _path is the parent
	_path = path;
	_newName = name;
}

// We need the custom copy constructor because of the reference
AndroidSAFFilesystemNode::AndroidSAFFilesystemNode(const AndroidSAFFilesystemNode &node)
	: AbstractFSNode(), _safNode(nullptr), _safParent(nullptr) {

	JNIEnv *env = JNI::getEnv();

	_safTree = env->NewGlobalRef(node._safTree);
	assert(_safTree);

	if (node._safNode) {
		_safNode = env->NewGlobalRef(node._safNode);
		assert(_safNode);
	}

	if (node._safParent) {
		_safParent = env->NewGlobalRef(node._safParent);
		assert(_safParent);
	}

	_path = node._path;
	_flags = node._flags;
	_newName = node._newName;
}

AndroidSAFFilesystemNode::~AndroidSAFFilesystemNode() {
	JNIEnv *env = JNI::getEnv();

	env->DeleteGlobalRef(_safTree);
	env->DeleteGlobalRef(_safNode);
	env->DeleteGlobalRef(_safParent);
}

Common::String AndroidSAFFilesystemNode::getName() const {
	if (!_safNode || !_safParent) {
		// _newName is for non-existent paths or root node pretty name
		return _newName;
	}

	return lastPathComponent(_path, '/');
}

Common::String AndroidSAFFilesystemNode::getPath() const {
	assert(_safTree != nullptr);

	if (_safNode != nullptr) {
		return _path;
	}

	// When no node, it means _path is the parent node
	return _path + "/" + _newName;
}

AbstractFSNode *AndroidSAFFilesystemNode::getChild(const Common::String &n) const {
	assert(_safTree != nullptr);
	assert(_safNode != nullptr);

	// Make sure the string contains no slashes
	assert(!n.contains('/'));

	JNIEnv *env = JNI::getEnv();

	jstring name = env->NewStringUTF(n.c_str());

	jobject child = env->CallObjectMethod(_safTree, _MID_getChild, _safNode, name);

	env->DeleteLocalRef(name);

	if (env->ExceptionCheck()) {
		LOGE("SAFFSTree::getChild failed");

		env->ExceptionDescribe();
		env->ExceptionClear();

		return nullptr;
	}

	if (child) {
		AndroidSAFFilesystemNode *ret = new AndroidSAFFilesystemNode(_safTree, child);
		env->DeleteLocalRef(child);
		return ret;
	}

	return new AndroidSAFFilesystemNode(_safTree, _safNode, _path, n);
}

bool AndroidSAFFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode,
        bool hidden) const {
	assert(_flags & DIRECTORY);

	assert(_safTree != nullptr);
	if (!_safNode) {
		return false;
	}

	JNIEnv *env = JNI::getEnv();

	jobjectArray array =
	    (jobjectArray)env->CallObjectMethod(_safTree, _MID_getChildren, _safNode);

	if (env->ExceptionCheck()) {
		LOGE("SAFFSTree::getChildren failed");

		env->ExceptionDescribe();
		env->ExceptionClear();

		return false;
	}

	if (!array) {
		// Fetching children failed: a log error has already been produced in Java code
		return false;
	}

	myList.clear();

	jsize size = env->GetArrayLength(array);
	myList.reserve(size);

	for (jsize i = 0; i < size; ++i) {
		jobject node = env->GetObjectArrayElement(array, i);

		myList.push_back(new AndroidSAFFilesystemNode(_safTree, node));

		env->DeleteLocalRef(node);
	}
	env->DeleteLocalRef(array);

	return true;
}

AbstractFSNode *AndroidSAFFilesystemNode::getParent() const {
	assert(_safTree != nullptr);
	// No need to check for _safNode: if node doesn't exist yet parent is its parent

	if (_safParent) {
		return new AndroidSAFFilesystemNode(_safTree, _safParent);
	}

	return AndroidFilesystemFactory::instance().makeRootFileNode();
}

Common::SeekableReadStream *AndroidSAFFilesystemNode::createReadStream() {
	assert(_safTree != nullptr);

	if (!_safNode) {
		return nullptr;
	}

	JNIEnv *env = JNI::getEnv();

	jint fd = env->CallIntMethod(_safTree, _MID_createReadStream, _safNode);

	if (env->ExceptionCheck()) {
		LOGE("SAFFSTree::createReadStream failed");

		env->ExceptionDescribe();
		env->ExceptionClear();

		return nullptr;
	}

	if (fd == -1) {
		return nullptr;
	}

	FILE *f = fdopen(fd, "r");
	if (!f) {
		close(fd);
		return nullptr;
	}

	return new PosixIoStream(f);
}

Common::SeekableWriteStream *AndroidSAFFilesystemNode::createWriteStream(bool atomic) {
	assert(_safTree != nullptr);

	JNIEnv *env = JNI::getEnv();

	if (!_safNode) {
		assert(_safParent);
		jstring name = env->NewStringUTF(_newName.c_str());

		// TODO: Add atomic support if possible
		jobject child = env->CallObjectMethod(_safTree, _MID_createFile, _safParent, name);

		env->DeleteLocalRef(name);

		if (env->ExceptionCheck()) {
			LOGE("SAFFSTree::createFile failed");

			env->ExceptionDescribe();
			env->ExceptionClear();

			return nullptr;
		}

		if (!child) {
			return nullptr;
		}

		_safNode = env->NewGlobalRef(child);
		assert(_safNode);

		env->DeleteLocalRef(child);

		cacheData();
	}

	jint fd = env->CallIntMethod(_safTree, _MID_createWriteStream, _safNode);
	if (env->ExceptionCheck()) {
		LOGE("SAFFSTree::createWriteStream failed");

		env->ExceptionDescribe();
		env->ExceptionClear();

		return nullptr;
	}

	if (fd == -1) {
		return nullptr;
	}

	FILE *f = fdopen(fd, "w");
	if (!f) {
		close(fd);
		return nullptr;
	}

	return new PosixIoStream(f);
}

bool AndroidSAFFilesystemNode::createDirectory() {
	assert(_safTree != nullptr);

	if (_safNode) {
		return _flags & DIRECTORY;
	}

	assert(_safParent);

	JNIEnv *env = JNI::getEnv();

	jstring name = env->NewStringUTF(_newName.c_str());

	jobject child = env->CallObjectMethod(_safTree, _MID_createDirectory, _safParent, name);

	env->DeleteLocalRef(name);

	if (env->ExceptionCheck()) {
		LOGE("SAFFSTree::createDirectory failed");

		env->ExceptionDescribe();
		env->ExceptionClear();

		return false;
	}

	if (!child) {
		return false;
	}

	_safNode = env->NewGlobalRef(child);
	assert(_safNode);

	env->DeleteLocalRef(child);

	cacheData();

	return true;
}

bool AndroidSAFFilesystemNode::remove() {
	assert(_safTree != nullptr);

	if (!_safNode) {
		return false;
	}

	if (!_safParent) {
		// It's the root of the tree: we can't delete it
		return false;
	}

	if (isDirectory()) {
		// Don't delete folders (yet?)
		return false;
	}

	JNIEnv *env = JNI::getEnv();

	bool result = env->CallBooleanMethod(_safTree, _MID_removeNode, _safNode);

	if (env->ExceptionCheck()) {
		LOGE("SAFFSTree::removeNode failed");

		env->ExceptionDescribe();
		env->ExceptionClear();

		return false;
	}

	if (!result) {
		return false;
	}

	env->DeleteGlobalRef(_safNode);
	_safNode = nullptr;

	// Create the parent node to fetch informations needed to make us a non-existent node
	AndroidSAFFilesystemNode *parent = new AndroidSAFFilesystemNode(_safTree, _safParent);

	size_t pos = _path.findLastOf('/');
	if (pos == Common::String::npos) {
		_newName = _path;
	} else {
		_newName = _path.substr(pos + 1);
	}
	_path = parent->_path;

	delete parent;

	return true;
}

void AndroidSAFFilesystemNode::removeTree() {
	assert(_safParent == nullptr);

	JNIEnv *env = JNI::getEnv();

	env->CallVoidMethod(_safTree, _MID_removeTree);

	if (env->ExceptionCheck()) {
		LOGE("SAFFSTree::removeTree failed");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}
}

void AndroidSAFFilesystemNode::cacheData() {
	JNIEnv *env = JNI::getEnv();

	_flags = env->GetIntField(_safNode, _FID__flags);

	jobject safParent = env->GetObjectField(_safNode, _FID__parent);
	if (safParent) {
		if (_safParent) {
			env->DeleteGlobalRef(_safParent);
		}
		_safParent = env->NewGlobalRef(safParent);
		assert(_safParent);
		env->DeleteLocalRef(safParent);
	}

	if (_safParent == nullptr) {
		jstring nameObj = (jstring)env->GetObjectField(_safTree, _FID__treeName);
		const char *nameP = env->GetStringUTFChars(nameObj, 0);
		if (nameP != 0) {
			_newName = Common::String(nameP);
			env->ReleaseStringUTFChars(nameObj, nameP);
		}
		env->DeleteLocalRef(nameObj);
	}

	Common::String workingPath;

	jstring pathObj = (jstring)env->GetObjectField(_safNode, _FID__path);
	const char *path = env->GetStringUTFChars(pathObj, 0);
	if (path == nullptr) {
		env->DeleteLocalRef(pathObj);
		error("SAFFSNode::_path is null");
		return;
	}
	workingPath = Common::String(path);
	env->ReleaseStringUTFChars(pathObj, path);
	env->DeleteLocalRef(pathObj);

	jstring idObj = (jstring)env->CallObjectMethod(_safTree, _MID_getTreeId);
	if (env->ExceptionCheck()) {
		env->ExceptionDescribe();
		env->ExceptionClear();

		env->ReleaseStringUTFChars(pathObj, path);
		env->DeleteLocalRef(pathObj);
		error("SAFFSTree::getTreeId failed");
		return;
	}

	if (!idObj) {
		error("SAFFSTree::getTreeId returned null");
		return;
	}

	const char *id = env->GetStringUTFChars(idObj, 0);
	if (id == nullptr) {
		error("Failed to get string from SAFFSTree::getTreeId");
		env->DeleteLocalRef(idObj);
		return;
	}

	_path = Common::String(SAF_MOUNT_POINT);
	_path += id;
	_path += workingPath;
	env->ReleaseStringUTFChars(idObj, id);
	env->DeleteLocalRef(idObj);
}

const char AddSAFFakeNode::SAF_ADD_FAKE_PATH[] = "/saf";

AddSAFFakeNode::~AddSAFFakeNode() {
	delete _proxied;
}

Common::U32String AddSAFFakeNode::getDisplayName() const {
	// I18N: This is displayed in the file browser to let the user choose a new folder for Android Storage Attached Framework
	return Common::U32String::format("\x01<%s>", _("Add a new folder").c_str());
}

Common::String AddSAFFakeNode::getName() const {
	return Common::String::format("\x01<%s>", _("Add a new folder").encode().c_str());
}

AbstractFSNode *AddSAFFakeNode::getChild(const Common::String &name) const {
	if (_fromPath) {
		// When starting from /saf try to get the tree node
		return AndroidSAFFilesystemNode::makeFromPath(Common::String(AndroidSAFFilesystemNode::SAF_MOUNT_POINT) + name);
	}
	// We can't call getChild as it's protected
	return nullptr;
}

AbstractFSNode *AddSAFFakeNode::getParent() const {
	// We are always just below the root and getParent is protected
	return AndroidFilesystemFactory::instance().makeRootFileNode();
}

bool AddSAFFakeNode::exists() const {
	if (_fromPath) {
		// /saf always exists when created as a path
		return true;
	}

	if (!_proxied) {
		makeProxySAF();
	}

	if (!_proxied) {
		return false;
	}

	return _proxied->exists();
}

bool AddSAFFakeNode::getChildren(AbstractFSList &list, ListMode mode, bool hidden) const {
	if (_fromPath) {
		// When built from path, /saf lists all SAF node but never proposes to add one
		if (mode == Common::FSNode::kListFilesOnly) {
			// All directories
			return true;
		}
		AndroidFilesystemFactory::instance().getSAFTrees(list, false);
		return true;
	}

	if (!_proxied) {
		makeProxySAF();
	}

	if (!_proxied) {
		return false;
	}

	return _proxied->getChildren(list, mode, hidden);
}

Common::String AddSAFFakeNode::getPath() const {
	if (_fromPath) {
		return SAF_ADD_FAKE_PATH;
	}

	if (!_proxied) {
		makeProxySAF();
	}

	if (!_proxied) {
		return "";
	}

	return _proxied->getPath();
}

bool AddSAFFakeNode::isReadable() const {
	if (_fromPath) {
		return true;
	}

	if (!_proxied) {
		makeProxySAF();
	}

	if (!_proxied) {
		return false;
	}

	return _proxied->isReadable();
}

bool AddSAFFakeNode::isWritable() const {
	if (_fromPath) {
		return false;
	}

	if (!_proxied) {
		makeProxySAF();
	}

	if (!_proxied) {
		return false;
	}

	return _proxied->isWritable();
}

void AddSAFFakeNode::makeProxySAF() const {
	assert(!_fromPath);

	if (_proxied) {
		return;
	}

	// I18N: This may be displayed in the Android UI used to add a Storage Attach Framework authorization
	jobject saftree = JNI::getNewSAFTree(true, true, "", _("Choose a new folder"));
	if (!saftree) {
		return;
	}

	_proxied = AndroidSAFFilesystemNode::makeFromTree(saftree);
}
