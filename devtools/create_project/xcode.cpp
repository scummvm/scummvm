/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "xcode.h"

#include <fstream>
#include <algorithm>

#ifdef MACOSX
#include <sstream>
#include <iomanip>
#include <CommonCrypto/CommonCrypto.h>

// If we want to unset the sdk version in the executable to work around bug #11430
// (blury display on retina screens when building with SDK 10.14+).
// This workaround only works with Xcode 11+.
//#define MACOSX_NO_SDKVERSION
#endif

namespace CreateProjectTool {

#define LAST_XCODE_VERSION "0830"

#define DEBUG_XCODE_HASH 0

#define IOS_TARGET 0
#define OSX_TARGET 1

#define ADD_DEFINE(defines, name) \
	defines.push_back(name);

#define REMOVE_DEFINE(defines, name) \
	{ ValueList::iterator i = std::find(defines.begin(), defines.end(), name); if (i != defines.end()) defines.erase(i); }

#define CONTAINS_DEFINE(defines, name) \
	(std::find(defines.begin(), defines.end(), name) != defines.end())

#define ADD_SETTING(config, key, value) \
	config._settings[key] = Setting(value, "", kSettingsNoQuote);

#define ADD_SETTING_ORDER(config, key, value, order) \
	config._settings[key] = Setting(value, "", kSettingsNoQuote, 0, order);

#define ADD_SETTING_ORDER_NOVALUE(config, key, comment, order) \
	config._settings[key] = Setting("", comment, kSettingsNoValue, 0, order);

#define ADD_SETTING_QUOTE(config, key, value) \
	config._settings[key] = Setting(value);

#define ADD_SETTING_QUOTE_VAR(config, key, value) \
	config._settings[key] = Setting(value, "", kSettingsQuoteVariable);

#define ADD_SETTING_LIST(config, key, values, flags, indent) \
	config._settings[key] = Setting(values, flags, indent);

#define REMOVE_SETTING(config, key) \
	config._settings.erase(key);

#define ADD_BUILD_FILE(id, name, fileRefId, comment) { \
	Object *buildFile = new Object(this, id, name, "PBXBuildFile", "PBXBuildFile", comment); \
	buildFile->addProperty("fileRef", fileRefId, name, kSettingsNoValue); \
	_buildFile.add(buildFile); \
	_buildFile._flags = kSettingsSingleItem; \
}

#define ADD_FILE_REFERENCE(id, name, properties) { \
	Object *fileRef = new Object(this, id, name, "PBXFileReference", "PBXFileReference", name); \
	if (!properties._fileEncoding.empty()) fileRef->addProperty("fileEncoding", properties._fileEncoding, "", kSettingsNoValue); \
	if (!properties._lastKnownFileType.empty()) fileRef->addProperty("lastKnownFileType", properties._lastKnownFileType, "", kSettingsNoValue|kSettingsQuoteVariable); \
	if (!properties._fileName.empty()) fileRef->addProperty("name", properties._fileName, "", kSettingsNoValue|kSettingsQuoteVariable); \
	if (!properties._filePath.empty()) fileRef->addProperty("path", properties._filePath, "", kSettingsNoValue|kSettingsQuoteVariable); \
	if (!properties._sourceTree.empty()) fileRef->addProperty("sourceTree", properties._sourceTree, "", kSettingsNoValue); \
	_fileReference.add(fileRef); \
	_fileReference._flags = kSettingsSingleItem; \
}

bool producesObjectFileOnOSX(const std::string &fileName) {
	std::string n, ext;
	splitFilename(fileName, n, ext);

	// Note that the difference between this and the general producesObjectFile is that
	// this one adds Objective-C(++), and removes asm-support.
	if (ext == "cpp" || ext == "c" || ext == "m" || ext == "mm")
		return true;
	else
		return false;
}

bool targetIsIOS(const std::string &targetName) {
	return targetName.length() > 4 && targetName.substr(targetName.length() - 4) == "-iOS";
}

bool shouldSkipFileForTarget(const std::string &fileID, const std::string &targetName, const std::string &fileName) {
	// Rules:
	// - if the parent directory is "backends/platform/ios7", the file belongs to the iOS target.
	// - if the parent directory is "/sdl", the file belongs to the macOS target.
	// - if the file has a suffix, like "_osx", or "_ios", the file belongs to one of the target.
	// - if the file is an macOS icon file (icns), it belongs to the macOS target.
	std::string name, ext;
	splitFilename(fileName, name, ext);

	if (targetIsIOS(targetName)) {
		// iOS target: we skip all files with the "_osx" suffix
		if (name.length() > 4 && name.substr(name.length() - 4) == "_osx") {
			return true;
		}
		// We don't need SDL for the iOS target
		static const std::string sdl_directory = "/sdl/";
		static const std::string surfacesdl_directory = "/surfacesdl/";
		static const std::string doublebufferdl_directory = "/doublebuffersdl/";
		if (fileID.find(sdl_directory) != std::string::npos
		 || fileID.find(surfacesdl_directory) != std::string::npos
		 || fileID.find(doublebufferdl_directory) != std::string::npos) {
			return true;
		 }
		if (ext == "icns") {
			return true;
		}
	}
	else {
		// macOS target: we skip all files with the "_ios" suffix
		if (name.length() > 4 && name.substr(name.length() - 4) == "_ios") {
			return true;
		}
		// macOS target: we skip all files with the "ios7_" prefix
		if (name.length() > 5 && name.substr(0, 5) == "ios7_") {
			return true;
		}
		// parent directory
		const std::string directory = fileID.substr(0, fileID.length() - fileName.length());
		static const std::string iphone_directory = "backends/platform/ios7";
		if (directory.length() > iphone_directory.length() && directory.substr(directory.length() - iphone_directory.length()) == iphone_directory) {
			return true;
		}
	}
	return false;
}

XcodeProvider::Group::Group(XcodeProvider *objectParent, const std::string &groupName, const std::string &uniqueName, const std::string &path) : Object(objectParent, uniqueName, groupName, "PBXGroup", "", groupName) {
	bool path_is_absolute = (path.length() > 0 && path.at(0) == '/');
	addProperty("name", _name, "", kSettingsNoValue | kSettingsQuoteVariable);
	addProperty("sourceTree", path_is_absolute ? "<absolute>" : "<group>", "", kSettingsNoValue | kSettingsQuoteVariable);

	if (path != "") {
		addProperty("path", path, "", kSettingsNoValue | kSettingsQuoteVariable);
	}
	_childOrder = 0;
	_treeName = uniqueName;
}

void XcodeProvider::Group::ensureChildExists(const std::string &name) {
	std::map<std::string, Group*>::iterator it = _childGroups.find(name);
	if (it == _childGroups.end()) {
		Group *child = new Group(_parent, name, this->_treeName + '/' + name, name);
		_childGroups[name] = child;
		addChildGroup(child);
		_parent->_groups.add(child);
	}
}

void XcodeProvider::Group::addChildInternal(const std::string &id, const std::string &comment) {
	if (_properties.find("children") == _properties.end()) {
		Property children;
		children._hasOrder = true;
		children._flags = kSettingsAsList;
		_properties["children"] = children;
	}
	_properties["children"]._settings[id] = Setting("", comment + " in Sources", kSettingsNoValue, 0, _childOrder++);
	if (_childOrder == 1) {
		// Force children to use () even when there is only 1 child.
		// Also this enforces the use of "," after the single item, instead of ; (see writeProperty)
		_properties["children"]._flags |= kSettingsSingleItem;
	} else {
		_properties["children"]._flags ^= kSettingsSingleItem;
	}

}

void XcodeProvider::Group::addChildGroup(const Group *group) {
	addChildInternal(_parent->getHash(group->_treeName), group->_treeName);
}

void XcodeProvider::Group::addChildFile(const std::string &name) {
	std::string id = "FileReference_" + _treeName + "/" + name;
	addChildInternal(_parent->getHash(id), name);
	FileProperty property = FileProperty(name, name, name, "\"<group>\"");

	_parent->addFileReference(id, name, property);
	if (producesObjectFileOnOSX(name)) {
		_parent->addBuildFile(_treeName + "/" + name, name, _parent->getHash(id), name + " in Sources");
	}
}

void XcodeProvider::Group::addChildByHash(const std::string &hash, const std::string &name) {
	addChildInternal(hash, name);
}

XcodeProvider::Group *XcodeProvider::Group::getChildGroup(const std::string &name) {
	std::map<std::string, Group *>::iterator it = _childGroups.find(name);
	assert(it != _childGroups.end());
	return it->second;
}

XcodeProvider::Group *XcodeProvider::touchGroupsForPath(const std::string &path) {
	if (_rootSourceGroup == NULL) {
		assert(path == _projectRoot);
		_rootSourceGroup = new Group(this, "Sources", path, path);
		_groups.add(_rootSourceGroup);
		return _rootSourceGroup;
	} else {
		assert(path.find(_projectRoot) == 0);
		std::string subPath = path.substr(_projectRoot.size() + 1);
		Group *currentGroup = _rootSourceGroup;
		size_t firstPathComponent = subPath.find_first_of('/');
		// We assume here that all paths have trailing '/', otherwise this breaks.
		while (firstPathComponent != std::string::npos) {
			currentGroup->ensureChildExists(subPath.substr(0, firstPathComponent));
			currentGroup = currentGroup->getChildGroup(subPath.substr(0, firstPathComponent));
			subPath = subPath.substr(firstPathComponent + 1);
			firstPathComponent = subPath.find_first_of('/');
		}
		return currentGroup;
	}
}

void XcodeProvider::addFileReference(const std::string &id, const std::string &name, FileProperty properties) {
	Object *fileRef = new Object(this, id, name, "PBXFileReference", "PBXFileReference", name);
	if (!properties._fileEncoding.empty()) fileRef->addProperty("fileEncoding", properties._fileEncoding, "", kSettingsNoValue);
	if (!properties._lastKnownFileType.empty()) fileRef->addProperty("lastKnownFileType", properties._lastKnownFileType, "", kSettingsNoValue | kSettingsQuoteVariable);
	if (!properties._fileName.empty()) fileRef->addProperty("name", properties._fileName, "", kSettingsNoValue | kSettingsQuoteVariable);
	if (!properties._filePath.empty()) fileRef->addProperty("path", properties._filePath, "", kSettingsNoValue | kSettingsQuoteVariable);
	if (!properties._sourceTree.empty()) fileRef->addProperty("sourceTree", properties._sourceTree, "", kSettingsNoValue);
	_fileReference.add(fileRef);
	_fileReference._flags = kSettingsSingleItem;
}

void XcodeProvider::addProductFileReference(const std::string &id, const std::string &name) {
	Object *fileRef = new Object(this, id, name, "PBXFileReference", "PBXFileReference", name);
	fileRef->addProperty("explicitFileType", "wrapper.application", "", kSettingsNoValue | kSettingsQuoteVariable);
	fileRef->addProperty("includeInIndex", "0", "", kSettingsNoValue);
	fileRef->addProperty("path", name, "", kSettingsNoValue | kSettingsQuoteVariable);
	fileRef->addProperty("sourceTree", "BUILT_PRODUCTS_DIR", "", kSettingsNoValue);
	_fileReference.add(fileRef);
	_fileReference._flags = kSettingsSingleItem;
}

void XcodeProvider::addBuildFile(const std::string &id, const std::string &name, const std::string &fileRefId, const std::string &comment) {

	Object *buildFile = new Object(this, id, name, "PBXBuildFile", "PBXBuildFile", comment);
	buildFile->addProperty("fileRef", fileRefId, name, kSettingsNoValue);
	_buildFile.add(buildFile);
	_buildFile._flags = kSettingsSingleItem;
}

XcodeProvider::XcodeProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version)
	: ProjectProvider(global_warnings, project_warnings, version) {
	_rootSourceGroup = NULL;
}

void XcodeProvider::addResourceFiles(const BuildSetup &setup, StringList &includeList, StringList &excludeList) {
	includeList.push_back(setup.srcDir + "/dists/ios7/Info.plist");

	ValueList &resources = getResourceFiles();
	for (ValueList::iterator it = resources.begin(); it != resources.end(); ++it) {
		includeList.push_back(setup.srcDir + "/" + *it);
	}

	StringList td;
	createModuleList(setup.srcDir + "/backends/platform/ios7", setup.defines, td, includeList, excludeList);
}

void XcodeProvider::createWorkspace(const BuildSetup &setup) {
	// Create project folder
	std::string workspace = setup.outputDir + '/' + PROJECT_NAME ".xcodeproj";
	createDirectory(workspace);
	_projectRoot = setup.srcDir;
	touchGroupsForPath(_projectRoot);

	// Setup global objects
	setupDefines(setup);
	_targets.push_back(PROJECT_DESCRIPTION "-iOS");
	_targets.push_back(PROJECT_DESCRIPTION "-macOS");
	setupCopyFilesBuildPhase();
	setupFrameworksBuildPhase(setup);
	setupNativeTarget();
	setupProject();
	setupResourcesBuildPhase();
	setupBuildConfiguration(setup);
	setupImageAssetCatalog(setup);
}

// We are done with constructing all the object graph and we got through every project, output the main project file
// (this is kind of a hack since other providers use separate project files)
void XcodeProvider::createOtherBuildFiles(const BuildSetup &setup) {
	// This needs to be done at the end when all build files have been accounted for
	setupSourcesBuildPhase();

	outputMainProjectFile(setup);
}

// Store information about a project here, for use at the end
void XcodeProvider::createProjectFile(const std::string &, const std::string &, const BuildSetup &setup, const std::string &moduleDir,
                                      const StringList &includeList, const StringList &excludeList) {
	std::string modulePath;
	if (!moduleDir.compare(0, setup.srcDir.size(), setup.srcDir)) {
		modulePath = moduleDir.substr(setup.srcDir.size());
		if (!modulePath.empty() && modulePath.at(0) == '/')
			modulePath.erase(0, 1);
	}

	std::ofstream project;
	if (!modulePath.empty())
		addFilesToProject(moduleDir, project, includeList, excludeList, setup.filePrefix + '/' + modulePath);
	else
		addFilesToProject(moduleDir, project, includeList, excludeList, setup.filePrefix);
}

//////////////////////////////////////////////////////////////////////////
// Main Project file
//////////////////////////////////////////////////////////////////////////
void XcodeProvider::outputMainProjectFile(const BuildSetup &setup) {
	std::ofstream project((setup.outputDir + '/' + PROJECT_NAME ".xcodeproj" + '/' + "project.pbxproj").c_str());
	if (!project)
		error("Could not open \"" + setup.outputDir + '/' + PROJECT_NAME ".xcodeproj" + '/' + "project.pbxproj\" for writing");

	//////////////////////////////////////////////////////////////////////////
	// Header
	project << "// !$*UTF8*$!\n"
	           "{\n"
	           "\t" << writeSetting("archiveVersion", "1", "", kSettingsNoQuote) << ";\n"
	           "\tclasses = {\n"
	           "\t};\n"
	           "\t" << writeSetting("objectVersion", "46", "", kSettingsNoQuote) << ";\n"
	           "\tobjects = {\n";

	//////////////////////////////////////////////////////////////////////////
	// List of objects
	project << _buildFile.toString();
	project << _copyFilesBuildPhase.toString();
	project << _fileReference.toString();
	project << _frameworksBuildPhase.toString();
	project << _groups.toString();
	project << _nativeTarget.toString();
	project << _project.toString();
	project << _resourcesBuildPhase.toString();
	project << _sourcesBuildPhase.toString();
	project << _buildConfiguration.toString();
	project << _configurationList.toString();

	//////////////////////////////////////////////////////////////////////////
	// Footer
	project << "\t};\n"
	           "\t" << writeSetting("rootObject", getHash("PBXProject"), "Project object", kSettingsNoQuote) << ";\n"
	           "}\n";

}

//////////////////////////////////////////////////////////////////////////
// Files
//////////////////////////////////////////////////////////////////////////
void XcodeProvider::writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int indentation,
                                           const std::string &objPrefix, const std::string &filePrefix) {

	// Ensure that top-level groups are generated for i.e. engines/
	Group *group = touchGroupsForPath(filePrefix);
	for (FileNode::NodeList::const_iterator i = dir.children.begin(); i != dir.children.end(); ++i) {
		const FileNode *node = *i;

		// Iff it is a file, then add (build) file references. Since we're using Groups and not File References
		// for folders, we shouldn't add folders as file references, obviously.
		if (node->children.empty()) {
			group->addChildFile(node->name);
		}
		// Process child nodes
		if (!node->children.empty())
			writeFileListToProject(*node, projectFile, indentation + 1, objPrefix + node->name + '_', filePrefix + node->name + '/');
	}
}

//////////////////////////////////////////////////////////////////////////
// Setup functions
//////////////////////////////////////////////////////////////////////////
void XcodeProvider::setupCopyFilesBuildPhase() {
	// Nothing to do here
}

#define DEF_SYSFRAMEWORK(framework) { properties[framework".framework"] = FileProperty("wrapper.framework", framework".framework", "System/Library/Frameworks/" framework ".framework", "SDKROOT"); \
	ADD_SETTING_ORDER_NOVALUE(children, getHash(framework".framework"), framework".framework", fwOrder++); }

#define DEF_SYSTBD(lib) { properties[lib".tbd"] = FileProperty("sourcecode.text-based-dylib-definition", lib".tbd", "usr/lib/" lib ".tbd", "SDKROOT"); \
	ADD_SETTING_ORDER_NOVALUE(children, getHash(lib".tbd"), lib".tbd", fwOrder++); }

#define DEF_LOCALLIB_STATIC_PATH(path,lib,absolute) { properties[lib".a"] = FileProperty("archive.ar", lib ".a", path, (absolute ? "\"<absolute>\"" : "\"<group>\"")); \
	ADD_SETTING_ORDER_NOVALUE(children, getHash(lib".a"), lib".a", fwOrder++); }

#define DEF_LOCALLIB_STATIC(lib) DEF_LOCALLIB_STATIC_PATH(lib ".a", lib, false)


/**
 * Sets up the frameworks build phase.
 *
 * (each native target has different build rules)
 */
void XcodeProvider::setupFrameworksBuildPhase(const BuildSetup &setup) {
	_frameworksBuildPhase._comment = "PBXFrameworksBuildPhase";

	// Just use a hardcoded id for the Frameworks-group
	Group *frameworksGroup = new Group(this, "Frameworks", "PBXGroup_CustomTemplate_Frameworks_", "");

	Property children;
	children._hasOrder = true;
	children._flags = kSettingsAsList;

	// Setup framework file properties
	std::map<std::string, FileProperty> properties;
	int fwOrder = 0;
	// Frameworks
	DEF_SYSFRAMEWORK("ApplicationServices");
	DEF_SYSFRAMEWORK("AudioToolbox");
	DEF_SYSFRAMEWORK("AudioUnit");
	DEF_SYSFRAMEWORK("Carbon");
	DEF_SYSFRAMEWORK("Cocoa");
	DEF_SYSFRAMEWORK("CoreAudio");
	DEF_SYSFRAMEWORK("CoreMIDI");
	DEF_SYSFRAMEWORK("CoreGraphics");
	DEF_SYSFRAMEWORK("CoreFoundation");
	DEF_SYSFRAMEWORK("Foundation");
	DEF_SYSFRAMEWORK("IOKit");
	DEF_SYSFRAMEWORK("OpenGL");
	DEF_SYSFRAMEWORK("OpenGLES");
	DEF_SYSFRAMEWORK("QuartzCore");
	DEF_SYSFRAMEWORK("UIKit");
	DEF_SYSFRAMEWORK("Security");
	DEF_SYSFRAMEWORK("SystemConfiguration");
	DEF_SYSTBD("libiconv");

	// Local libraries
	if (CONTAINS_DEFINE(setup.defines, "USE_FAAD")) {
		DEF_LOCALLIB_STATIC("libfaad");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_FLAC")) {
		DEF_LOCALLIB_STATIC("libFLAC");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_FLUIDSYNTH")) {
		DEF_LOCALLIB_STATIC("libfluidsynth");
		DEF_LOCALLIB_STATIC("libffi");
		DEF_LOCALLIB_STATIC("libglib-2.0");
		DEF_SYSTBD("libffi");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_FREETYPE2")) {
		DEF_LOCALLIB_STATIC("libfreetype");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_JPEG")) {
		DEF_LOCALLIB_STATIC("libjpeg");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_LIBCURL")) {
		DEF_LOCALLIB_STATIC("libcurl");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_MAD")) {
		DEF_LOCALLIB_STATIC("libmad");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_MPEG2")) {
		DEF_LOCALLIB_STATIC("libmpeg2");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_FRIBIDI")) {
		DEF_LOCALLIB_STATIC("libfribidi");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_PNG")) {
		DEF_LOCALLIB_STATIC("libpng");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_OGG")) {
		DEF_LOCALLIB_STATIC("libogg");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_VORBIS")) {
		DEF_LOCALLIB_STATIC("libvorbis");
		DEF_LOCALLIB_STATIC("libvorbisfile");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_TREMOR")) {
		DEF_LOCALLIB_STATIC("libvorbisidec");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_THEORADEC")) {
		DEF_LOCALLIB_STATIC("libtheoradec");
	if (CONTAINS_DEFINE(setup.defines, "USE_GLEW")) {
		DEF_LOCALLIB_STATIC("libGLEW");
	}
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_ZLIB")) {
		DEF_SYSTBD("libz");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_DISCORD")) {
		DEF_LOCALLIB_STATIC("libdiscord-rpc");
	}

	if (setup.useSDL2) {
		DEF_LOCALLIB_STATIC("libSDL2main");
		DEF_LOCALLIB_STATIC("libSDL2");
		if (CONTAINS_DEFINE(setup.defines, "USE_SDL_NET")) {
			DEF_LOCALLIB_STATIC("libSDL2_net");
		}
	} else {
		DEF_LOCALLIB_STATIC("libSDLmain");
		DEF_LOCALLIB_STATIC("libSDL");
		if (CONTAINS_DEFINE(setup.defines, "USE_SDL_NET")) {
			DEF_LOCALLIB_STATIC("libSDL_net");
		}
	}

	frameworksGroup->_properties["children"] = children;
	_groups.add(frameworksGroup);
	// Force this to be added as a sub-group in the root.
	_rootSourceGroup->addChildGroup(frameworksGroup);


	// Declare this here, as it's used across all the targets
	int order = 0;

	//////////////////////////////////////////////////////////////////////////
	// ScummVM-iOS
	Object *framework_iPhone = new Object(this, "PBXFrameworksBuildPhase_" + _targets[IOS_TARGET], "PBXFrameworksBuildPhase", "PBXFrameworksBuildPhase", "", "Frameworks");

	framework_iPhone->addProperty("buildActionMask", "2147483647", "", kSettingsNoValue);
	framework_iPhone->addProperty("runOnlyForDeploymentPostprocessing", "0", "", kSettingsNoValue);

	// List of frameworks
	Property iOS_files;
	iOS_files._hasOrder = true;
	iOS_files._flags = kSettingsAsList;

	ValueList frameworks_iOS;
	frameworks_iOS.push_back("CoreAudio.framework");
	frameworks_iOS.push_back("CoreGraphics.framework");
	frameworks_iOS.push_back("CoreFoundation.framework");
	frameworks_iOS.push_back("Foundation.framework");
	frameworks_iOS.push_back("UIKit.framework");
	frameworks_iOS.push_back("SystemConfiguration.framework");
	frameworks_iOS.push_back("AudioToolbox.framework");
	frameworks_iOS.push_back("QuartzCore.framework");
	frameworks_iOS.push_back("OpenGLES.framework");

	if (CONTAINS_DEFINE(setup.defines, "USE_FAAD")) {
		frameworks_iOS.push_back("libfaad.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_FLAC")) {
		frameworks_iOS.push_back("libFLAC.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_FREETYPE2")) {
		frameworks_iOS.push_back("libfreetype.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_JPEG")) {
		frameworks_iOS.push_back("libjpeg.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_PNG")) {
		frameworks_iOS.push_back("libpng.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_OGG")) {
		frameworks_iOS.push_back("libogg.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_VORBIS")) {
		frameworks_iOS.push_back("libvorbis.a");
		frameworks_iOS.push_back("libvorbisfile.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_TREMOR")) {
		frameworks_iOS.push_back("libvorbisidec.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_THEORADEC")) {
		frameworks_iOS.push_back("libtheoradec.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_MAD")) {
		frameworks_iOS.push_back("libmad.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_MPEG2")) {
		frameworks_iOS.push_back("libmpeg2.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_FRIBIDI")) {
		frameworks_iOS.push_back("libfribidi.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_FLUIDSYNTH")) {
		frameworks_iOS.push_back("libfluidsynth.a");
		frameworks_iOS.push_back("libglib-2.0.a");
		frameworks_iOS.push_back("libffi.a");
		frameworks_iOS.push_back("CoreMIDI.framework");
		frameworks_iOS.push_back("libiconv.tbd");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_ZLIB")) {
		frameworks_iOS.push_back("libz.tbd");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_LIBCURL")) {
		frameworks_iOS.push_back("libcurl.a");
		frameworks_iOS.push_back("Security.framework");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_SDL_NET")) {
		if (setup.useSDL2)
			frameworks_iOS.push_back("libSDL2_net.a");
		else
			frameworks_iOS.push_back("libSDL_net.a");
	}

	for (ValueList::iterator framework = frameworks_iOS.begin(); framework != frameworks_iOS.end(); framework++) {
		std::string id = "Frameworks_" + *framework + "_iphone";
		std::string comment = *framework + " in Frameworks";

		ADD_SETTING_ORDER_NOVALUE(iOS_files, getHash(id), comment, order++);
		ADD_BUILD_FILE(id, *framework, getHash(*framework), comment);
		ADD_FILE_REFERENCE(*framework, *framework, properties[*framework]);
	}

	framework_iPhone->_properties["files"] = iOS_files;

	_frameworksBuildPhase.add(framework_iPhone);

	//////////////////////////////////////////////////////////////////////////
	// ScummVM-macOS
	Object *framework_OSX = new Object(this, "PBXFrameworksBuildPhase_" + _targets[OSX_TARGET], "PBXFrameworksBuildPhase", "PBXFrameworksBuildPhase", "", "Frameworks");

	framework_OSX->addProperty("buildActionMask", "2147483647", "", kSettingsNoValue);
	framework_OSX->addProperty("runOnlyForDeploymentPostprocessing", "0", "", kSettingsNoValue);

	// List of frameworks
	Property osx_files;
	osx_files._hasOrder = true;
	osx_files._flags = kSettingsAsList;

	ValueList frameworks_osx;
	frameworks_osx.push_back("CoreFoundation.framework");
	frameworks_osx.push_back("Foundation.framework");
	frameworks_osx.push_back("AudioToolbox.framework");
	frameworks_osx.push_back("CoreMIDI.framework");
	frameworks_osx.push_back("CoreAudio.framework");
	frameworks_osx.push_back("QuartzCore.framework");
	frameworks_osx.push_back("Carbon.framework");
	frameworks_osx.push_back("ApplicationServices.framework");
	frameworks_osx.push_back("IOKit.framework");
	frameworks_osx.push_back("Cocoa.framework");
	frameworks_osx.push_back("OpenGL.framework");
	frameworks_osx.push_back("AudioUnit.framework");

	if (CONTAINS_DEFINE(setup.defines, "USE_FAAD")) {
		frameworks_osx.push_back("libfaad.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_FLAC")) {
		frameworks_osx.push_back("libFLAC.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_FLUIDSYNTH")) {
		frameworks_osx.push_back("libfluidsynth.a");
		frameworks_osx.push_back("libglib-2.0.a");
		frameworks_osx.push_back("libffi.tbd");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_FREETYPE2")) {
		frameworks_osx.push_back("libfreetype.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_JPEG")) {
		frameworks_osx.push_back("libjpeg.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_LIBCURL")) {
		frameworks_osx.push_back("libcurl.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_MAD")) {
		frameworks_osx.push_back("libmad.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_MPEG2")) {
		frameworks_osx.push_back("libmpeg2.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_FRIBIDI")) {
		frameworks_osx.push_back("libfribidi.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_PNG")) {
		frameworks_osx.push_back("libpng.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_OGG")) {
		frameworks_osx.push_back("libogg.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_VORBIS")) {
		frameworks_osx.push_back("libvorbis.a");
		frameworks_osx.push_back("libvorbisfile.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_TREMOR")) {
		frameworks_osx.push_back("libvorbisidec.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_THEORADEC")) {
		frameworks_osx.push_back("libtheoradec.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_GLEW")) {
		frameworks_osx.push_back("libGLEW.a");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_ZLIB")) {
		frameworks_osx.push_back("libz.tbd");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_DISCORD")) {
		frameworks_osx.push_back("libdiscord-rpc.a");
	}

	if (setup.useSDL2) {
		frameworks_osx.push_back("libSDL2main.a");
		frameworks_osx.push_back("libSDL2.a");
		if (CONTAINS_DEFINE(setup.defines, "USE_SDL_NET"))
			frameworks_osx.push_back("libSDL2_net.a");
	} else {
		frameworks_osx.push_back("libSDLmain.a");
		frameworks_osx.push_back("libSDL.a");
		if (CONTAINS_DEFINE(setup.defines, "USE_SDL_NET"))
			frameworks_osx.push_back("libSDL_net.a");
	}

	order = 0;
	for (ValueList::iterator framework = frameworks_osx.begin(); framework != frameworks_osx.end(); framework++) {
		std::string id = "Frameworks_" + *framework + "_osx";
		std::string comment = *framework + " in Frameworks";

		ADD_SETTING_ORDER_NOVALUE(osx_files, getHash(id), comment, order++);
		ADD_BUILD_FILE(id, *framework, getHash(*framework), comment);
		ADD_FILE_REFERENCE(*framework, *framework, properties[*framework]);
	}

	framework_OSX->_properties["files"] = osx_files;

	_frameworksBuildPhase.add(framework_OSX);
}

void XcodeProvider::setupNativeTarget() {
	_nativeTarget._comment = "PBXNativeTarget";

	// Just use a hardcoded id for the Products-group
	Group *productsGroup = new Group(this, "Products", "PBXGroup_CustomTemplate_Products_" , "");
	// Output native target section
	for (unsigned int i = 0; i < _targets.size(); i++) {
		Object *target = new Object(this, "PBXNativeTarget_" + _targets[i], "PBXNativeTarget", "PBXNativeTarget", "", _targets[i]);

		target->addProperty("buildConfigurationList", getHash("XCConfigurationList_" + _targets[i]), "Build configuration list for PBXNativeTarget \"" + _targets[i] + "\"", kSettingsNoValue);

		Property buildPhases;
		buildPhases._hasOrder = true;
		buildPhases._flags = kSettingsAsList;
		buildPhases._settings[getHash("PBXResourcesBuildPhase_" + _targets[i])] = Setting("", "Resources", kSettingsNoValue, 0, 0);
		buildPhases._settings[getHash("PBXSourcesBuildPhase_" + _targets[i])] = Setting("", "Sources", kSettingsNoValue, 0, 1);
		buildPhases._settings[getHash("PBXFrameworksBuildPhase_" + _targets[i])] = Setting("", "Frameworks", kSettingsNoValue, 0, 2);
		target->_properties["buildPhases"] = buildPhases;

		target->addProperty("buildRules", "", "", kSettingsNoValue | kSettingsAsList);

		target->addProperty("dependencies", "", "", kSettingsNoValue | kSettingsAsList);

		target->addProperty("name", _targets[i], "", kSettingsNoValue | kSettingsQuoteVariable);
		target->addProperty("productName", PROJECT_NAME, "", kSettingsNoValue);
		addProductFileReference("PBXFileReference_" PROJECT_DESCRIPTION ".app_" + _targets[i], PROJECT_DESCRIPTION ".app");
		productsGroup->addChildByHash(getHash("PBXFileReference_" PROJECT_DESCRIPTION ".app_" + _targets[i]), PROJECT_DESCRIPTION ".app");
		target->addProperty("productReference", getHash("PBXFileReference_" PROJECT_DESCRIPTION ".app_" + _targets[i]), PROJECT_DESCRIPTION ".app", kSettingsNoValue);
		target->addProperty("productType", "com.apple.product-type.application", "", kSettingsNoValue | kSettingsQuoteVariable);

		_nativeTarget.add(target);
	}
	_rootSourceGroup->addChildGroup(productsGroup);
	_groups.add(productsGroup);
}

void XcodeProvider::setupProject() {
	_project._comment = "PBXProject";

	Object *project = new Object(this, "PBXProject", "PBXProject", "PBXProject", "", "Project object");

	project->addProperty("buildConfigurationList", getHash("XCConfigurationList_" PROJECT_NAME), "Build configuration list for PBXProject \"" PROJECT_NAME "\"", kSettingsNoValue);
	project->addProperty("compatibilityVersion", "Xcode 3.2", "", kSettingsNoValue | kSettingsQuoteVariable);
	project->addProperty("developmentRegion", "English", "", kSettingsNoValue);
	project->addProperty("hasScannedForEncodings", "1", "", kSettingsNoValue);
	project->addProperty("attributes", "{ LastUpgradeCheck = " LAST_XCODE_VERSION "; }", "", kSettingsNoQuote | kSettingsNoValue);

	// List of known regions
	Property regions;
	regions._flags = kSettingsAsList;
	ADD_SETTING_ORDER_NOVALUE(regions, "English", "", 0);
	ADD_SETTING_ORDER_NOVALUE(regions, "Japanese", "", 1);
	ADD_SETTING_ORDER_NOVALUE(regions, "French", "", 2);
	ADD_SETTING_ORDER_NOVALUE(regions, "German", "", 3);
	project->_properties["knownRegions"] = regions;

	project->addProperty("mainGroup", _rootSourceGroup->getHashRef(), "CustomTemplate", kSettingsNoValue);
	project->addProperty("productRefGroup", getHash("PBXGroup_CustomTemplate_Products_"), "" , kSettingsNoValue);
	project->addProperty("projectDirPath", _projectRoot, "", kSettingsNoValue | kSettingsQuoteVariable);
	project->addProperty("projectRoot", "", "", kSettingsNoValue | kSettingsQuoteVariable);

	// List of targets
	Property targets;
	targets._flags = kSettingsAsList;
	targets._settings[getHash("PBXNativeTarget_" + _targets[IOS_TARGET])] = Setting("", _targets[IOS_TARGET], kSettingsNoValue, 0, 0);
	targets._settings[getHash("PBXNativeTarget_" + _targets[OSX_TARGET])] = Setting("", _targets[OSX_TARGET], kSettingsNoValue, 0, 1);
	project->_properties["targets"] = targets;

	// Force list even when there is only a single target
	project->_properties["targets"]._flags |= kSettingsSingleItem;

	_project.add(project);
}

XcodeProvider::ValueList& XcodeProvider::getResourceFiles() const {
	static ValueList files;
	if (files.empty()) {
		files.push_back("gui/themes/scummclassic.zip");
		files.push_back("gui/themes/scummmodern.zip");
		files.push_back("gui/themes/scummremastered.zip");
		files.push_back("gui/themes/residualvm.zip");
		files.push_back("gui/themes/translations.dat");
		files.push_back("dists/engine-data/access.dat");
		files.push_back("dists/engine-data/cryo.dat");
		files.push_back("dists/engine-data/cryomni3d.dat");
		files.push_back("dists/engine-data/drascula.dat");
		files.push_back("dists/engine-data/encoding.dat");
		files.push_back("dists/engine-data/fonts.dat");
		files.push_back("dists/engine-data/hadesch_translations.dat");
		files.push_back("dists/engine-data/hugo.dat");
		files.push_back("dists/engine-data/kyra.dat");
		files.push_back("dists/engine-data/lure.dat");
		files.push_back("dists/engine-data/macgui.dat");
		files.push_back("dists/engine-data/myst3.dat");
		files.push_back("dists/engine-data/monkey4-patch.m4b");
		files.push_back("dists/engine-data/grim-patch.lab");
		files.push_back("dists/engine-data/macventure.dat");
		files.push_back("dists/engine-data/mort.dat");
		files.push_back("dists/engine-data/neverhood.dat");
		files.push_back("dists/engine-data/queen.tbl");
		files.push_back("dists/engine-data/sky.cpt");
		files.push_back("dists/engine-data/supernova.dat");
		files.push_back("dists/engine-data/teenagent.dat");
		files.push_back("dists/engine-data/titanic.dat");
		files.push_back("dists/engine-data/tony.dat");
		files.push_back("dists/engine-data/toon.dat");
		files.push_back("dists/engine-data/ultima.dat");
		files.push_back("dists/engine-data/wintermute.zip");
		files.push_back("dists/engine-data/xeen.ccs");
		files.push_back("dists/ios7/LaunchScreen_ios.storyboard");
		files.push_back("dists/pred.dic");
		files.push_back("dists/networking/wwwroot.zip");
		files.push_back("icons/scummvm.icns");
		files.push_back("AUTHORS");
		files.push_back("COPYING");
		files.push_back("COPYING.LGPL");
		files.push_back("COPYING.BSD");
		files.push_back("COPYING.FREEFONT");
		files.push_back("COPYING.OFL");
		files.push_back("NEWS");
		files.push_back("README");
	}
	return files;
}

void XcodeProvider::setupResourcesBuildPhase() {
	_resourcesBuildPhase._comment = "PBXResourcesBuildPhase";

	ValueList &files_list = getResourceFiles();

	// Same as for containers: a rule for each native target
	for (unsigned int i = 0; i < _targets.size(); i++) {
		Object *resource = new Object(this, "PBXResourcesBuildPhase_" + _targets[i], "PBXResourcesBuildPhase", "PBXResourcesBuildPhase", "", "Resources");

		resource->addProperty("buildActionMask", "2147483647", "", kSettingsNoValue);

		// Add default files
		Property files;
		files._hasOrder = true;
		files._flags = kSettingsAsList;

		int order = 0;
		for (ValueList::iterator file = files_list.begin(); file != files_list.end(); file++) {
			if (shouldSkipFileForTarget(*file, _targets[i], *file)) {
				continue;
			}
			std::string resourceAbsolutePath = _projectRoot + "/" + *file;
			std::string file_id = "FileReference_" + resourceAbsolutePath;
			std::string base = basename(*file);
			std::string comment = base + " in Resources";
			addBuildFile(resourceAbsolutePath, base, getHash(file_id), comment);
			ADD_SETTING_ORDER_NOVALUE(files, getHash(resourceAbsolutePath), comment, order++);
		}

		resource->_properties["files"] = files;

		resource->addProperty("runOnlyForDeploymentPostprocessing", "0", "", kSettingsNoValue);

		_resourcesBuildPhase.add(resource);
	}
}

void XcodeProvider::setupSourcesBuildPhase() {
	_sourcesBuildPhase._comment = "PBXSourcesBuildPhase";

	// Same as for containers: a rule for each native target
	for (unsigned int i = 0; i < _targets.size(); i++) {
		const std::string &targetName = _targets[i];
		Object *source = new Object(this, "PBXSourcesBuildPhase_" + _targets[i], "PBXSourcesBuildPhase", "PBXSourcesBuildPhase", "", "Sources");

		source->addProperty("buildActionMask", "2147483647", "", kSettingsNoValue);

		Property files;
		files._hasOrder = true;
		files._flags = kSettingsAsList;

		int order = 0;
		for (std::vector<Object *>::iterator file = _buildFile._objects.begin(); file != _buildFile._objects.end(); ++file) {
			const std::string &fileName = (*file)->_name;
			if (shouldSkipFileForTarget((*file)->_id, targetName, fileName)) {
				continue;
			}
			if (!producesObjectFileOnOSX(fileName)) {
				continue;
			}
			std::string comment = fileName + " in Sources";
			ADD_SETTING_ORDER_NOVALUE(files, getHash((*file)->_id), comment, order++);
		}

		setupAdditionalSources(targetName, files, order);

		source->_properties["files"] = files;

		source->addProperty("runOnlyForDeploymentPostprocessing", "0", "", kSettingsNoValue);

		_sourcesBuildPhase.add(source);
	}
}

// Setup all build configurations
void XcodeProvider::setupBuildConfiguration(const BuildSetup &setup) {

	_buildConfiguration._comment = "XCBuildConfiguration";
	_buildConfiguration._flags = kSettingsAsList;

	std::string projectOutputDirectory;
#ifdef POSIX
	char *rp = realpath(setup.outputDir.c_str(), NULL);
	projectOutputDirectory = rp;
	free(rp);
#endif

	/****************************************
	 * ScummVM - Project Level
	 ****************************************/

	// Debug
	Object *scummvm_Debug_Object = new Object(this, "XCBuildConfiguration_" PROJECT_NAME "_Debug", PROJECT_NAME, "XCBuildConfiguration", "PBXProject", "Debug");
	Property scummvm_Debug;
	ADD_SETTING(scummvm_Debug, "ALWAYS_SEARCH_USER_PATHS", "NO");
	ADD_SETTING_QUOTE(scummvm_Debug, "USER_HEADER_SEARCH_PATHS", "$(SRCROOT) $(SRCROOT)/engines");
	ADD_SETTING(scummvm_Debug, "CLANG_ANALYZER_LOCALIZABILITY_NONLOCALIZED", "YES");
	if (CONTAINS_DEFINE(setup.defines, "USE_CXX11")) {
		ADD_SETTING(scummvm_Debug, "CLANG_CXX_LANGUAGE_STANDARD", "\"c++0x\"");
	}
	ADD_SETTING(scummvm_Debug, "CLANG_WARN_BOOL_CONVERSION", "YES");
	ADD_SETTING(scummvm_Debug, "CLANG_WARN_CONSTANT_CONVERSION", "YES");
	ADD_SETTING(scummvm_Debug, "CLANG_WARN_EMPTY_BODY", "YES");
	ADD_SETTING(scummvm_Debug, "CLANG_WARN_ENUM_CONVERSION", "YES");
	ADD_SETTING(scummvm_Debug, "CLANG_WARN_INFINITE_RECURSION", "YES");
	ADD_SETTING(scummvm_Debug, "CLANG_WARN_INT_CONVERSION", "YES");
	ADD_SETTING(scummvm_Debug, "CLANG_WARN_SUSPICIOUS_MOVE", "YES");
	ADD_SETTING(scummvm_Debug, "CLANG_WARN_UNREACHABLE_CODE", "YES");
	ADD_SETTING(scummvm_Debug, "CLANG_WARN__DUPLICATE_METHOD_MATCH", "YES");
	ADD_SETTING_QUOTE(scummvm_Debug, "CODE_SIGN_IDENTITY", "");
	ADD_SETTING_QUOTE_VAR(scummvm_Debug, "CODE_SIGN_IDENTITY[sdk=iphoneos*]", "");
	ADD_SETTING(scummvm_Debug, "ENABLE_STRICT_OBJC_MSGSEND", "YES");
	ADD_SETTING_QUOTE(scummvm_Debug, "FRAMEWORK_SEARCH_PATHS", "");
	ADD_SETTING(scummvm_Debug, "GCC_C_LANGUAGE_STANDARD", "c99");
	ADD_SETTING(scummvm_Debug, "GCC_ENABLE_CPP_EXCEPTIONS", "NO");
	ADD_SETTING(scummvm_Debug, "GCC_ENABLE_CPP_RTTI", "YES");
	ADD_SETTING(scummvm_Debug, "GCC_INPUT_FILETYPE", "automatic");
	ADD_SETTING(scummvm_Debug, "GCC_NO_COMMON_BLOCKS", "YES");
	ADD_SETTING(scummvm_Debug, "GCC_OPTIMIZATION_LEVEL", "0");
	ADD_SETTING(scummvm_Debug, "GCC_WARN_SIGN_COMPARE", "YES");
	ADD_SETTING(scummvm_Debug, "GCC_WARN_UNDECLARED_SELECTOR", "YES");
	ADD_SETTING(scummvm_Debug, "GCC_WARN_UNINITIALIZED_AUTOS", "YES");
	ADD_SETTING(scummvm_Debug, "GCC_WARN_UNUSED_FUNCTION", "YES");
	ValueList scummvm_WarningCFlags;
	scummvm_WarningCFlags.push_back("-Wno-multichar");
	scummvm_WarningCFlags.push_back("-Wno-undefined-var-template");
	scummvm_WarningCFlags.push_back("-Wc++11-extensions");
	ADD_SETTING_LIST(scummvm_Debug, "WARNING_CFLAGS", scummvm_WarningCFlags, kSettingsQuoteVariable | kSettingsAsList, 5);
	ValueList scummvm_defines(_defines);
	REMOVE_DEFINE(scummvm_defines, "MACOSX");
	REMOVE_DEFINE(scummvm_defines, "IPHONE");
	REMOVE_DEFINE(scummvm_defines, "IPHONE_IOS7");
	REMOVE_DEFINE(scummvm_defines, "IPHONE_SANDBOXED");
	REMOVE_DEFINE(scummvm_defines, "SDL_BACKEND");
	ADD_SETTING_LIST(scummvm_Debug, "GCC_PREPROCESSOR_DEFINITIONS", scummvm_defines, kSettingsNoQuote | kSettingsAsList, 5);
	ADD_SETTING(scummvm_Debug, "GCC_WARN_ABOUT_RETURN_TYPE", "YES");
	ADD_SETTING(scummvm_Debug, "GCC_WARN_UNUSED_VARIABLE", "YES");
	ValueList scummvm_HeaderPaths;
	scummvm_HeaderPaths.push_back("include/");
	scummvm_HeaderPaths.push_back("$(SRCROOT)/engines/");
	scummvm_HeaderPaths.push_back("$(SRCROOT)");
	ADD_SETTING_LIST(scummvm_Debug, "HEADER_SEARCH_PATHS", scummvm_HeaderPaths, kSettingsQuoteVariable | kSettingsAsList, 5);
	ADD_SETTING_QUOTE(scummvm_Debug, "LIBRARY_SEARCH_PATHS", "");
	ADD_SETTING(scummvm_Debug, "ONLY_ACTIVE_ARCH", "YES");
	ADD_SETTING_QUOTE(scummvm_Debug, "OTHER_CFLAGS", "");
	ADD_SETTING_QUOTE(scummvm_Debug, "OTHER_LDFLAGS", "");
	ADD_SETTING(scummvm_Debug, "ENABLE_TESTABILITY", "YES");

	scummvm_Debug_Object->addProperty("name", "Debug", "", kSettingsNoValue);
	scummvm_Debug_Object->_properties["buildSettings"] = scummvm_Debug;

	// Release
	Object *scummvm_Release_Object = new Object(this, "XCBuildConfiguration_" PROJECT_NAME "_Release", PROJECT_NAME, "XCBuildConfiguration", "PBXProject", "Release");
	Property scummvm_Release(scummvm_Debug);
	REMOVE_SETTING(scummvm_Release, "GCC_C_LANGUAGE_STANDARD");       // Not sure why we remove that, or any of the other warnings
	REMOVE_SETTING(scummvm_Release, "GCC_WARN_ABOUT_RETURN_TYPE");
	REMOVE_SETTING(scummvm_Release, "GCC_WARN_UNUSED_VARIABLE");
	REMOVE_SETTING(scummvm_Release, "ONLY_ACTIVE_ARCH");
	REMOVE_SETTING(scummvm_Release, "ENABLE_TESTABILITY");
	REMOVE_SETTING(scummvm_Release, "GCC_PREPROCESSOR_DEFINITIONS");
	ValueList scummvm_Release_defines(scummvm_defines);
	ADD_DEFINE(scummvm_Release_defines, "RELEASE_BUILD");
	ADD_SETTING_LIST(scummvm_Release, "GCC_PREPROCESSOR_DEFINITIONS", scummvm_Release_defines, kSettingsNoQuote | kSettingsAsList, 5);

	scummvm_Release_Object->addProperty("name", "Release", "", kSettingsNoValue);
	scummvm_Release_Object->_properties["buildSettings"] = scummvm_Release;

	_buildConfiguration.add(scummvm_Debug_Object);
	_buildConfiguration.add(scummvm_Release_Object);

	///****************************************
	// * ScummVM - iOS Target
	// ****************************************/

	// Debug
	Object *iPhone_Debug_Object = new Object(this, "XCBuildConfiguration_" PROJECT_DESCRIPTION "-iPhone_Debug", _targets[IOS_TARGET] /* ScummVM-iPhone */, "XCBuildConfiguration", "PBXNativeTarget", "Debug");
	Property iPhone_Debug;
	ADD_SETTING_QUOTE(iPhone_Debug, "CODE_SIGN_IDENTITY", "iPhone Developer");
	ADD_SETTING_QUOTE_VAR(iPhone_Debug, "CODE_SIGN_IDENTITY[sdk=iphoneos*]", "iPhone Developer");
	ADD_SETTING(iPhone_Debug, "COPY_PHASE_STRIP", "NO");
	ADD_SETTING_QUOTE(iPhone_Debug, "DEBUG_INFORMATION_FORMAT", "dwarf");
	ValueList iPhone_FrameworkSearchPaths;
	iPhone_FrameworkSearchPaths.push_back("$(inherited)");
	iPhone_FrameworkSearchPaths.push_back("\"$(SDKROOT)$(SYSTEM_LIBRARY_DIR)/PrivateFrameworks\"");
	ADD_SETTING_LIST(iPhone_Debug, "FRAMEWORK_SEARCH_PATHS", iPhone_FrameworkSearchPaths, kSettingsAsList, 5);
	ADD_SETTING(iPhone_Debug, "GCC_DYNAMIC_NO_PIC", "NO");
	ADD_SETTING(iPhone_Debug, "GCC_ENABLE_CPP_EXCEPTIONS", "NO");
	ADD_SETTING(iPhone_Debug, "GCC_OPTIMIZATION_LEVEL", "0");
	ADD_SETTING(iPhone_Debug, "GCC_PRECOMPILE_PREFIX_HEADER", "NO");
	ADD_SETTING(iPhone_Debug, "GCC_WARN_64_TO_32_BIT_CONVERSION", "NO");
	ADD_SETTING_QUOTE(iPhone_Debug, "GCC_PREFIX_HEADER", "");
	ADD_SETTING(iPhone_Debug, "GCC_UNROLL_LOOPS", "YES");
	ValueList iPhone_HeaderSearchPaths;
	iPhone_HeaderSearchPaths.push_back("$(SRCROOT)/engines/");
	iPhone_HeaderSearchPaths.push_back("$(SRCROOT)");
	iPhone_HeaderSearchPaths.push_back("\"" + projectOutputDirectory + "\"");
	iPhone_HeaderSearchPaths.push_back("\"" + projectOutputDirectory + "/include\"");
	if (CONTAINS_DEFINE(setup.defines, "USE_SDL_NET")) {
		if (setup.useSDL2)
			iPhone_HeaderSearchPaths.push_back("\"" + projectOutputDirectory + "/include/SDL2\"");
		else
			iPhone_HeaderSearchPaths.push_back("\"" + projectOutputDirectory + "include/SDL\"");
	}
	ADD_SETTING_LIST(iPhone_Debug, "HEADER_SEARCH_PATHS", iPhone_HeaderSearchPaths, kSettingsAsList | kSettingsQuoteVariable, 5);
	ADD_SETTING_QUOTE(iPhone_Debug, "INFOPLIST_FILE", "$(SRCROOT)/dists/ios7/Info.plist");
	ValueList iPhone_LibPaths;
	iPhone_LibPaths.push_back("$(inherited)");
	iPhone_LibPaths.push_back("\"" + projectOutputDirectory + "/lib\"");
	ADD_SETTING_LIST(iPhone_Debug, "LIBRARY_SEARCH_PATHS", iPhone_LibPaths, kSettingsAsList, 5);
	ADD_SETTING(iPhone_Debug, "ONLY_ACTIVE_ARCH", "YES");
	ADD_SETTING(iPhone_Debug, "PRODUCT_NAME", PROJECT_NAME);
	ADD_SETTING(iPhone_Debug, "PRODUCT_BUNDLE_IDENTIFIER", "\"org.scummvm.${PRODUCT_NAME}\"");
	ADD_SETTING(iPhone_Debug, "IPHONEOS_DEPLOYMENT_TARGET", "8.0");
	ADD_SETTING_QUOTE_VAR(iPhone_Debug, "PROVISIONING_PROFILE[sdk=iphoneos*]", "");
	ADD_SETTING(iPhone_Debug, "SDKROOT", "iphoneos");
	ADD_SETTING_QUOTE(iPhone_Debug, "TARGETED_DEVICE_FAMILY", "1,2");
	ValueList scummvmIOS_defines;
	ADD_DEFINE(scummvmIOS_defines, "\"$(inherited)\"");
	ADD_DEFINE(scummvmIOS_defines, "IPHONE");
	ADD_DEFINE(scummvmIOS_defines, "IPHONE_IOS7");
	ADD_DEFINE(scummvmIOS_defines, "IPHONE_SANDBOXED");
	if (CONTAINS_DEFINE(setup.defines, "USE_SDL_NET"))
		ADD_DEFINE(scummvmIOS_defines, "WITHOUT_SDL");
	ADD_SETTING_LIST(iPhone_Debug, "GCC_PREPROCESSOR_DEFINITIONS", scummvmIOS_defines, kSettingsNoQuote | kSettingsAsList, 5);
	ADD_SETTING(iPhone_Debug, "ASSETCATALOG_COMPILER_APPICON_NAME", "AppIcon");
	ADD_SETTING(iPhone_Debug, "ASSETCATALOG_COMPILER_LAUNCHIMAGE_NAME", "LaunchImage");

	iPhone_Debug_Object->addProperty("name", "Debug", "", kSettingsNoValue);
	iPhone_Debug_Object->_properties["buildSettings"] = iPhone_Debug;

	// Release
	Object *iPhone_Release_Object = new Object(this, "XCBuildConfiguration_" PROJECT_DESCRIPTION "-iPhone_Release", _targets[IOS_TARGET] /* ScummVM-iPhone */, "XCBuildConfiguration", "PBXNativeTarget", "Release");
	Property iPhone_Release(iPhone_Debug);
	ADD_SETTING(iPhone_Release, "GCC_OPTIMIZATION_LEVEL", "3");
	ADD_SETTING(iPhone_Release, "COPY_PHASE_STRIP", "YES");
	REMOVE_SETTING(iPhone_Release, "GCC_DYNAMIC_NO_PIC");
	ADD_SETTING(iPhone_Release, "WRAPPER_EXTENSION", "app");
	REMOVE_SETTING(iPhone_Release, "DEBUG_INFORMATION_FORMAT");
	ADD_SETTING_QUOTE(iPhone_Release, "DEBUG_INFORMATION_FORMAT", "dwarf-with-dsym");

	iPhone_Release_Object->addProperty("name", "Release", "", kSettingsNoValue);
	iPhone_Release_Object->_properties["buildSettings"] = iPhone_Release;

	_buildConfiguration.add(iPhone_Debug_Object);
	_buildConfiguration.add(iPhone_Release_Object);

	/****************************************
	 * ScummVM - macOS Target
	 ****************************************/

	// Debug
	Object *scummvmOSX_Debug_Object = new Object(this, "XCBuildConfiguration_" PROJECT_DESCRIPTION "-OSX_Debug", _targets[OSX_TARGET] /* ScummVM-macOS */, "XCBuildConfiguration", "PBXNativeTarget", "Debug");
	Property scummvmOSX_Debug;
	ADD_SETTING(scummvmOSX_Debug, "COMBINE_HIDPI_IMAGES", "YES");
	ADD_SETTING(scummvmOSX_Debug, "SDKROOT", "macosx");
	ADD_SETTING(scummvmOSX_Debug, "COPY_PHASE_STRIP", "NO");
	ADD_SETTING_QUOTE(scummvmOSX_Debug, "DEBUG_INFORMATION_FORMAT", "dwarf");
	ADD_SETTING_QUOTE(scummvmOSX_Debug, "FRAMEWORK_SEARCH_PATHS", "");
	if (CONTAINS_DEFINE(setup.defines, "USE_CXX11")) {
		ADD_SETTING(scummvmOSX_Debug, "CLANG_CXX_LANGUAGE_STANDARD", "\"c++0x\"");
	}
	ADD_SETTING(scummvmOSX_Debug, "GCC_C_LANGUAGE_STANDARD", "c99");
	ADD_SETTING(scummvmOSX_Debug, "GCC_ENABLE_CPP_EXCEPTIONS", "NO");
	ADD_SETTING(scummvmOSX_Debug, "GCC_ENABLE_CPP_RTTI", "YES");
	ADD_SETTING(scummvmOSX_Debug, "GCC_DYNAMIC_NO_PIC", "NO");
	ADD_SETTING(scummvmOSX_Debug, "GCC_OPTIMIZATION_LEVEL", "0");
	ADD_SETTING(scummvmOSX_Debug, "GCC_PRECOMPILE_PREFIX_HEADER", "NO");
	ADD_SETTING_QUOTE(scummvmOSX_Debug, "GCC_PREFIX_HEADER", "");
	ValueList scummvmOSX_defines;
	ADD_DEFINE(scummvmOSX_defines, "\"$(inherited)\"");
	ADD_DEFINE(scummvmOSX_defines, "SDL_BACKEND");
	ADD_DEFINE(scummvmOSX_defines, "MACOSX");
	ADD_SETTING_LIST(scummvmOSX_Debug, "GCC_PREPROCESSOR_DEFINITIONS", scummvmOSX_defines, kSettingsNoQuote | kSettingsAsList, 5);
	ADD_SETTING_QUOTE(scummvmOSX_Debug, "GCC_VERSION", "");
	ValueList scummvmOSX_HeaderPaths;
	if (setup.useSDL2) {
		scummvmOSX_HeaderPaths.push_back("/usr/local/include/SDL2");
		scummvmOSX_HeaderPaths.push_back("/opt/local/include/SDL2");
	} else {
		scummvmOSX_HeaderPaths.push_back("/usr/local/include/SDL");
		scummvmOSX_HeaderPaths.push_back("/opt/local/include/SDL");
	}
	scummvmOSX_HeaderPaths.push_back("/usr/local/include");
	scummvmOSX_HeaderPaths.push_back("/opt/local/include");
	scummvmOSX_HeaderPaths.push_back("/usr/local/include/freetype2");
	scummvmOSX_HeaderPaths.push_back("/opt/local/include/freetype2");
	scummvmOSX_HeaderPaths.push_back("include/");
	scummvmOSX_HeaderPaths.push_back("$(SRCROOT)/engines/");
	scummvmOSX_HeaderPaths.push_back("$(SRCROOT)");
	ADD_SETTING_LIST(scummvmOSX_Debug, "HEADER_SEARCH_PATHS", scummvmOSX_HeaderPaths, kSettingsQuoteVariable | kSettingsAsList, 5);
	ADD_SETTING_QUOTE(scummvmOSX_Debug, "INFOPLIST_FILE", "$(SRCROOT)/dists/macosx/Info.plist");
	ValueList scummvmOSX_LibPaths;
	scummvmOSX_LibPaths.push_back("/usr/local/lib");
	scummvmOSX_LibPaths.push_back("/opt/local/lib");
	scummvmOSX_LibPaths.push_back("\"$(inherited)\"");
	scummvmOSX_LibPaths.push_back("\"\\\"$(SRCROOT)/lib\\\"\"");
	ADD_SETTING_LIST(scummvmOSX_Debug, "LIBRARY_SEARCH_PATHS", scummvmOSX_LibPaths, kSettingsNoQuote | kSettingsAsList, 5);
	ADD_SETTING_QUOTE(scummvmOSX_Debug, "OTHER_CFLAGS", "");
	ADD_SETTING(scummvmOSX_Debug, "PRODUCT_NAME", PROJECT_NAME);
	ValueList scummvmOSX_LinkerFlags;
#ifdef MACOSX_NO_SDKVERSION
	scummvmOSX_LinkerFlags.push_back("-Xlinker");
	scummvmOSX_LinkerFlags.push_back("-platform_version");
	scummvmOSX_LinkerFlags.push_back("-Xlinker");
	scummvmOSX_LinkerFlags.push_back("macos");
	scummvmOSX_LinkerFlags.push_back("-Xlinker");
	// Since the option can only be used with Xcode 11, assume the min version targetted is 10.14
	scummvmOSX_LinkerFlags.push_back("10.14");
	scummvmOSX_LinkerFlags.push_back("-Xlinker");
	scummvmOSX_LinkerFlags.push_back("0.0.0");
	ADD_SETTING_LIST(scummvmOSX_Debug, "OTHER_LDFLAGS", scummvmOSX_LinkerFlags, kSettingsAsList, 5);
#endif

	scummvmOSX_Debug_Object->addProperty("name", "Debug", "", kSettingsNoValue);
	scummvmOSX_Debug_Object->_properties["buildSettings"] = scummvmOSX_Debug;

	// Release
	Object *scummvmOSX_Release_Object = new Object(this, "XCBuildConfiguration_" PROJECT_DESCRIPTION "-OSX_Release", _targets[OSX_TARGET] /* ScummVM-macOS */, "XCBuildConfiguration", "PBXNativeTarget", "Release");
	Property scummvmOSX_Release(scummvmOSX_Debug);
	ADD_SETTING(scummvmOSX_Release, "COPY_PHASE_STRIP", "YES");
	REMOVE_SETTING(scummvmOSX_Release, "GCC_DYNAMIC_NO_PIC");
	REMOVE_SETTING(scummvmOSX_Release, "GCC_OPTIMIZATION_LEVEL");
	ADD_SETTING(scummvmOSX_Release, "GCC_OPTIMIZATION_LEVEL", "3");
	ADD_SETTING(scummvmOSX_Release, "WRAPPER_EXTENSION", "app");
	REMOVE_SETTING(scummvmOSX_Release, "DEBUG_INFORMATION_FORMAT");
	ADD_SETTING_QUOTE(scummvmOSX_Release, "DEBUG_INFORMATION_FORMAT", "dwarf-with-dsym");
#ifdef MACOSX_NO_SDKVERSION
	ADD_SETTING_LIST(scummvmOSX_Release, "OTHER_LDFLAGS", scummvmOSX_LinkerFlags, kSettingsAsList, 5);
#endif

	scummvmOSX_Release_Object->addProperty("name", "Release", "", kSettingsNoValue);
	scummvmOSX_Release_Object->_properties["buildSettings"] = scummvmOSX_Release;

	_buildConfiguration.add(scummvmOSX_Debug_Object);
	_buildConfiguration.add(scummvmOSX_Release_Object);

	// Warning: This assumes we have all configurations with a Debug & Release pair
	for (std::vector<Object *>::iterator config = _buildConfiguration._objects.begin(); config != _buildConfiguration._objects.end(); config++) {

		Object *configList = new Object(this, "XCConfigurationList_" + (*config)->_name, (*config)->_name, "XCConfigurationList", "", "Build configuration list for " + (*config)->_refType + " \"" + (*config)->_name + "\"");

		Property buildConfigs;
		buildConfigs._flags = kSettingsAsList;

		buildConfigs._settings[getHash((*config)->_id)] = Setting("", "Debug", kSettingsNoValue, 0, 0);
		buildConfigs._settings[getHash((*(++config))->_id)] = Setting("", "Release", kSettingsNoValue, 0, 1);

		configList->_properties["buildConfigurations"] = buildConfigs;

		configList->addProperty("defaultConfigurationIsVisible", "0", "", kSettingsNoValue);
		configList->addProperty("defaultConfigurationName", "Release", "", kSettingsNoValue);

		_configurationList.add(configList);
	}
}

void XcodeProvider::setupImageAssetCatalog(const BuildSetup &setup) {
	const std::string filename = "Images.xcassets";
	const std::string absoluteCatalogPath = _projectRoot + "/dists/ios7/" + filename;
	const std::string id = "FileReference_" + absoluteCatalogPath;
	Group *group = touchGroupsForPath(absoluteCatalogPath);
	group->addChildFile(filename);
	addBuildFile(absoluteCatalogPath, filename, getHash(id), "Image Asset Catalog");
}

void XcodeProvider::setupAdditionalSources(std::string targetName, Property &files, int &order) {
	if (targetIsIOS(targetName)) {
		const std::string absoluteCatalogPath = _projectRoot + "/dists/ios7/Images.xcassets";
		ADD_SETTING_ORDER_NOVALUE(files, getHash(absoluteCatalogPath), "Image Asset Catalog", order++);
	}
}

//////////////////////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////////////////////

// Setup global defines
void XcodeProvider::setupDefines(const BuildSetup &setup) {

	for (StringList::const_iterator i = setup.defines.begin(); i != setup.defines.end(); ++i) {
		if (*i == "USE_NASM")  // Not supported on Mac
			continue;

		ADD_DEFINE(_defines, *i);
	}
	// Add special defines for Mac support
	REMOVE_DEFINE(_defines, "MACOSX");
	REMOVE_DEFINE(_defines, "IPHONE");
	REMOVE_DEFINE(_defines, "IPHONE_IOS7");
	REMOVE_DEFINE(_defines, "IPHONE_SANDBOXED");
	REMOVE_DEFINE(_defines, "SDL_BACKEND");
	if (!CONTAINS_DEFINE(_defines, "USE_TEXT_CONSOLE_FOR_DEBUGGER")) {
		ADD_DEFINE(_defines, "USE_TEXT_CONSOLE_FOR_DEBUGGER");
	}
	ADD_DEFINE(_defines, "CONFIG_H");
	ADD_DEFINE(_defines, "UNIX");
	ADD_DEFINE(_defines, "SCUMMVM");
}

//////////////////////////////////////////////////////////////////////////
// Object hash
//////////////////////////////////////////////////////////////////////////

std::string XcodeProvider::getHash(std::string key) {

#if DEBUG_XCODE_HASH
	return key;
#else
	// Check to see if the key is already in the dictionary
	std::map<std::string, std::string>::iterator hashIterator = _hashDictionnary.find(key);
	if (hashIterator != _hashDictionnary.end())
		return hashIterator->second;

	// Generate a new key from the file hash and insert it into the dictionary
#ifdef MACOSX
	std::string hash = md5(key);
#else
	std::string hash = newHash();
#endif

	_hashDictionnary[key] = hash;

	return hash;
#endif
}

bool isSeparator(char s) { return (s == '-'); }

#ifdef MACOSX
std::string XcodeProvider::md5(std::string key) {
	unsigned char md[CC_MD5_DIGEST_LENGTH];
	CC_MD5(key.c_str(), (CC_LONG) key.length(), md);
	std::stringstream stream;
	stream << std::hex << std::setfill('0') << std::setw(2);
	for (int i=0; i<CC_MD5_DIGEST_LENGTH; i++) {
		stream << (unsigned int) md[i];
	}
	return stream.str();
}
#endif

std::string XcodeProvider::newHash() const {
	std::string hash = createUUID();

	// Remove { and - from UUID and resize to 96-bits uppercase hex string
	hash.erase(remove_if(hash.begin(), hash.end(), isSeparator), hash.end());

	hash.resize(24);
	std::transform(hash.begin(), hash.end(), hash.begin(), toupper);

	return hash;
}

//////////////////////////////////////////////////////////////////////////
// Output
//////////////////////////////////////////////////////////////////////////

std::string replace(std::string input, const std::string find, std::string replaceStr) {
	std::string::size_type pos = 0;
	std::string::size_type findLen = find.length();
	std::string::size_type replaceLen = replaceStr.length();

	if (findLen == 0)
		return input;

	for (; (pos = input.find(find, pos)) != std::string::npos;) {
		input.replace(pos, findLen, replaceStr);
		pos += replaceLen;
	}

	return input;
}

std::string XcodeProvider::writeProperty(const std::string &variable, Property &prop, int flags) const {
	std::string output;

	output += (flags & kSettingsSingleItem ? "" : "\t\t\t") + variable + " = ";

	if (prop._settings.size() > 1 || (prop._flags & kSettingsSingleItem))
		output += (prop._flags & kSettingsAsList) ? "(\n" : "{\n";

	OrderedSettingList settings = prop.getOrderedSettingList();
	for (OrderedSettingList::const_iterator setting = settings.begin(); setting != settings.end(); ++setting) {
		if (settings.size() > 1 || (prop._flags & kSettingsSingleItem))
			output += (flags & kSettingsSingleItem ? " " : "\t\t\t\t");

		output += writeSetting(setting->first, setting->second);

		// The combination of kSettingsAsList, and kSettingsSingleItem should use "," and not ";" (i.e children
		// in PBXGroup, so we special case that case here.
		if ((prop._flags & kSettingsAsList) && (prop._settings.size() > 1 || (prop._flags & kSettingsSingleItem))) {
			output += (prop._settings.size() > 0) ? ",\n" : "\n";
		} else {
			output += ";";
			output += (flags & kSettingsSingleItem ? " " : "\n");
		}
	}

	if (prop._settings.size() > 1 || (prop._flags & kSettingsSingleItem))
		output += (prop._flags & kSettingsAsList) ? "\t\t\t);\n" : "\t\t\t};\n";

	return output;
}

std::string XcodeProvider::writeSetting(const std::string &variable, std::string value, std::string comment, int flags, int indent) const {
	return writeSetting(variable, Setting(value, comment, flags, indent));
}

// Heavily modified (not in a good way) function, imported from the QMake
// XCode project generator pbuilder_pbx.cpp, writeSettings() (under LGPL 2.1)
std::string XcodeProvider::writeSetting(const std::string &variable, const Setting &setting) const {
	std::string output;
	const std::string quote = (setting._flags & kSettingsNoQuote) ? "" : "\"";
	const std::string escape_quote = quote.empty() ? "" : "\\" + quote;
	std::string newline = "\n";

	// Get indent level
	for (int i = 0; i < setting._indent; ++i)
		newline += "\t";

	// Setup variable
	std::string var = (setting._flags & kSettingsQuoteVariable) ? "\"" + variable + "\"" : variable;

	// Output a list
	if (setting._flags & kSettingsAsList) {
		output += var + ((setting._flags & kSettingsNoValue) ? "(" : " = (") + newline;

		for (unsigned int i = 0, count = 0; i < setting._entries.size(); ++i) {

			std::string value = setting._entries.at(i)._value;
			if (!value.empty()) {
				if (count++ > 0)
					output += "," + newline;

				output += quote + replace(value, quote, escape_quote) + quote;

				std::string comment = setting._entries.at(i)._comment;
				if (!comment.empty())
					output += " /* " + comment + " */";
			}

		}
		// Add closing ")" on new line
		newline.resize(newline.size() - 1);
		output += (setting._flags & kSettingsNoValue) ? "\t\t\t)" : "," + newline + ")";
	} else {
		output += var;

		output += (setting._flags & kSettingsNoValue) ? "" : " = " + quote;

		for (unsigned int i = 0; i < setting._entries.size(); ++i) {
			std::string value = setting._entries.at(i)._value;
			if (i)
				output += " ";
			output += value;

			std::string comment = setting._entries.at(i)._comment;
			if (!comment.empty())
				output += " /* " + comment + " */";
		}

		output += (setting._flags & kSettingsNoValue) ? "" : quote;
	}
	return output;
}

} // End of CreateProjectTool namespace
