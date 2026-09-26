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

#include "config.h"
#include "xcode.h"

#include <limits.h>
#include <stdlib.h>

#include <fstream>
#include <algorithm>

#ifdef MACOSX
#include <sstream>
#include <iomanip>
#include <CommonCrypto/CommonDigest.h>
#endif

namespace CreateProjectTool {

#define LAST_XCODE_VERSION "0830"

#define DEBUG_XCODE_HASH 0

#define IOS_TARGET PROJECT_DESCRIPTION "-iOS"
#define TVOS_TARGET PROJECT_DESCRIPTION "-tvOS"
#define OSX_TARGET PROJECT_DESCRIPTION "-macOS"

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
	config._settings[key] = Setting
("", comment, kSettingsNoValue, 0, order);

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

bool targetIsTVOS(const std::string &targetName) {
	return targetName.length() > 5 && targetName.substr(targetName.length() - 5) == "-tvOS";
}

bool shouldSkipFileForTarget(const std::string &fileID, const std::string &targetName, const std::string &fileName) {
	// Rules:
	// - if the parent directory is "backends/platform/ios7", the file belongs to the iOS target.
	// - if the parent directory is "/sdl", the file belongs to the macOS target.
	// - if the file has a suffix, like "_osx", or "_ios", the file belongs to one of the target.
	// - if the file is a macOS icon file (icns), it belongs to the macOS target.
	std::string name, ext;
	splitFilename(fileName, name, ext);

	if (targetIsIOS(targetName) || targetIsTVOS(targetName)) {
		if (targetIsIOS(targetName)) {
			// skip tvos dist files
			if (fileID.find("dists/tvos/") != std::string::npos)
				return true;
		} else {
			// skip ios dist files
			if (fileID.find("dists/ios7/") != std::string::npos)
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
		Group *child = new
 Group(_parent, name, this->_treeName + '/' + name, name);
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
	if (_rootSourceGroup == nullptr) {
		assert(path == _projectRoot);
		_rootSourceGroup = new Group(this, "Sources", path, path);
		_gr
oups.add(_rootSourceGroup);
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
	fileRef->addProperty("includeInIndex",
 "0", "", kSettingsNoValue);
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

XcodeProvider::XcodeProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, StringList &global_errors)
	: ProjectProvider(global_warnings, project_warnings, global_errors) {
	_rootSourceGroup = nullptr;
}

void XcodeProvider::addResourceFiles(const BuildSetup &setup, StringList &includeList, StringList &excludeList) {
	if (setup.appleEmbedded) {
		includeList.push_back(setup.srcDir + "/dists/ios7/Info.plist");
		includeList.push_back(setup.srcDir + "/dists/ios7/PrivacyInfo.xcprivacy");
		includeList.push_back(setup.srcDir + "/dists/tvos/Info.plist");
		includeList.push_back(setup.srcDir + "/dists/tvos/PrivacyInfo.xcprivacy");
	}

	ValueList &resources = getResourceFiles(setup);
	for (const auto &resource : resources) {
		includeList.push_back(setup.srcDir + "/" + resource);
	}

	StringList pchDirs, pchEx;

	StringList td;
	if (setup.appleEmbedded) {
		createModuleList(setup.srcDir + "/backends/platform/ios7", setup.defines, td, includeList, excludeList, pchDirs, pchEx);
	}
}

void XcodeProvider::createWorkspace(const BuildSetup &setup) {
	// Create project folder
	std::string workspace = setup.outputDir + '/' + PROJECT_NAME ".xcodeproj";
	createDirectory(workspace);
	_projectRoot = setup.srcDir;
	touchGroupsForPath(_projectRoot);

	// Setup global objects
	setupDefines(setup);
	if (setup.appleEm
bedded) {
		_targets.push_back(IOS_TARGET);
		_targets.push_back(TVOS_TARGET);
	} else {
		_targets.push_back(OSX_TARGET);
	}
	setupCopyFilesBuildPhase();
	setupFrameworksBuildPhase(setup);
	setupNativeTarget();
	setupProject();
	setupResourcesBuildPhase(setup);
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
									  const StringList &includeList, const StringList &excludeList, const std::string &pchIncludeRoot, const StringList &pchDirs, const StringList &pchExclude) {
	std::string modulePath;
	if (!moduleDir.compare(0, setup.srcDir.size(), setup.srcDir)) {
		modulePath = moduleDir.substr(setup.srcDir.size());
		if (!modulePath.empty() && modulePath.at(0) == '/')
			modulePath.erase(0, 1);
	}

	std::ofstream project;
	if (!modulePath.empty())
		addFilesToProject(moduleDir, project, includeList, excludeList, pchIncludeRoot, pchDirs, pchExclude, setup.filePrefix + '/' + modulePath);
	else
		addFilesToProject(moduleDir, project, includeList, excludeList, pchIncludeRoot, pchDirs, pchExclude, setup.filePrefix);
}

//////////////////////////////////////////////////////////////////////////
// Main Project file
//////////////////////////////////////////////////////////////////////////
void XcodeProvider::outputMainProjectFile(const BuildSetup &setup) {
	std::ofstream project((setup.outputDir + '/' + PROJECT_NAME ".xcodeproj" + '/' + "project.pbxproj").c_str(
));
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
void XcodeProvider::writeFileListToProject(const FileNode &dir, std::ostream &projectFile, const int indentation,
										   const std::string &objPrefix, const std::string &filePrefix,
										   const std::string &pchIncludeRoot, const StringList &pchDirs, const StringList &pchExclude) {

	// Ensure that top-level groups are generated for i.e. engines/
	Group *group = touchGroupsForPath(filePrefix);
	for (const auto *node : dir.children) {
		// Iff it is a file, then add (build) file references. Since we're us
ing Groups and not File References
		// for folders, we shouldn't add folders as file references, obviously.
		if (node->children.empty()) {
			group->addChildFile(node->name);
		}
		// Process child nodes
		if (!node->children.empty())
			writeFileListToProject(*node, projectFile, indentation + 1, objPrefix + node->name + '_', filePrefix + node->name + '/', pchIncludeRoot, pchDirs, pchExclude);
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

#define DEF_LOCALXCFRAMEWORK(xcframework,path) { properties[xcframework".xcframework"] = FileProperty("wrapper.xcframework", xcframework".xcframework", path + "/frameworks/" + xcframework ".xcframework", "\"<group>\""); \
	ADD_SETTING_ORDER_NOVALUE(children, getHash(xcframework".xcframework"), xcframework".xcframework", fwOrder++); }

/**
 * Sets up the frameworks build phase.
 *
 * (each native target has different build rules)
 */
void XcodeProvider::setupFrameworksBuildPhas
e(const BuildSetup &setup) {
	_frameworksBuildPhase._comment = "PBXFrameworksBuildPhase";

	// Just use a hardcoded id for the Frameworks-group
	Group *frameworksGroup = new Group(this, "Frameworks", "PBXGroup_CustomTemplate_Frameworks_", "");

	Property children;
	children._hasOrder = true;
	children._flags = kSettingsAsList;

	std::string projectOutputDirectory;
#ifdef POSIX
	char tmpbuf[PATH_MAX];
	char *rp = realpath(setup.outputDir.c_str(), tmpbuf);
	projectOutputDirectory = rp;
#endif

	// Setup framework file properties
	std::map<std::string, FileProperty> properties;
	int fwOrder = 0;
	// Frameworks
	DEF_SYSFRAMEWORK("ApplicationServices");
	DEF_SYSFRAMEWORK("AudioToolbox");
	DEF_SYSFRAMEWORK("AudioUnit");
	DEF_SYSFRAMEWORK("AVFAudio");
	DEF_SYSFRAMEWORK("Carbon");
	DEF_SYSFRAMEWORK("Cocoa");
	DEF_SYSFRAMEWORK("CoreAudio");
	DEF_SYSFRAMEWORK("CoreMIDI");
	DEF_SYSFRAMEWORK("CoreGraphics");
	DEF_SYSFRAMEWORK("CoreFoundation");
	DEF_SYSFRAMEWORK("Foundation");
	DEF_SYSFRAMEWORK("GameController");
	DEF_SYSFRAMEWORK("IOKit");
	DEF_SYSFRAMEWORK("OpenGL");
	DEF_SYSFRAMEWORK("OpenGLES");
	DEF_SYSFRAMEWORK("QuartzCore");
	DEF_SYSFRAMEWORK("UIKit");
	DEF_SYSFRAMEWORK("Security");
	DEF_SYSFRAMEWORK("SystemConfiguration");
	DEF_SYSTBD("libiconv");

	std::string libSDL = setup.getSDLName();

	// Local libraries
	if (CONTAINS_DEFINE(setup.defines, "USE_FAAD")) {
		DEF_LOCALLIB_STATIC("libfaad");
		DEF_LOCALXCFRAMEWORK("faad", projectOutputDirectory);
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_FLAC")) {
		DEF_LOCALLIB_STATIC("libFLAC");
		DEF_LOCALXCFRAMEWORK("FLAC", projectOutputDirectory);
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_FLUIDLITE")) {
		DEF_LOCALLIB_STATIC("libfluidlite");
	} else if (CONTAINS_DEFINE(setup.defines, "USE_FLUIDSYNTH")) {
		DEF_LOCALLIB_STATIC("libfluidsynth");
		DEF_LOCALLIB_STATIC("libffi");
		DEF_LOCALLIB_STATIC("libglib-2.0");
		DEF_SYSTBD("libffi");
		DEF_LOCALXCFRAMEWORK("fluidsynth", projectOutputDirectory);
		DEF_LOCALXCFRAMEWORK("ffi"
, projectOutputDirectory);
		DEF_LOCALXCFRAMEWORK("intl", projectOutputDirectory);
		DEF_LOCALXCFRAMEWORK("bz2", projectOutputDirectory);
		DEF_LOCALXCFRAMEWORK("glib-2.0", projectOutputDirectory);
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_FREETYPE2")) {
		DEF_LOCALLIB_STATIC("libfreetype");
		DEF_LOCALXCFRAMEWORK("freetype", projectOutputDirectory);
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_JPEG")) {
		DEF_LOCALLIB_STATIC("libjpeg");
		DEF_LOCALXCFRAMEWORK("jpeg", projectOutputDirectory);
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_LIBCURL")) {
		DEF_LOCALLIB_STATIC("libcurl");
		DEF_LOCALXCFRAMEWORK("curl", projectOutputDirectory);
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_MAD")) {
		DEF_LOCALLIB_STATIC("libmad");
		DEF_LOCALXCFRAMEWORK("mad", projectOutputDirectory);
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_MIKMOD")) {
		DEF_LOCALLIB_STATIC("libmikmod");
		DEF_LOCALXCFRAMEWORK("mikmod", projectOutputDirectory);
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_OPENMPT")) {
		DEF_LOCALLIB_STATIC("libopenmpt");
		DEF_LOCALXCFRAMEWORK("openmpt", projectOutputDirectory);
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_MPEG2")) {
		DEF_LOCALLIB_STATIC("libmpeg2");
		DEF_LOCALXCFRAMEWORK("mpeg2", projectOutputDirectory);
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_FRIBIDI")) {
		DEF_LOCALLIB_STATIC("libfribidi");
		DEF_LOCALXCFRAMEWORK("fribidi", projectOutputDirectory);
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_PNG")) {
		DEF_LOCALLIB_STATIC("libpng");
		DEF_LOCALXCFRAMEWORK("png", projectOutputDirectory);
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_GIF")) {
		DEF_LOCALLIB_STATIC("libgif");
		DEF_LOCALXCFRAMEWORK("gif", projectOutputDirectory);
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_OGG")) {
		DEF_LOCALLIB_STATIC("libogg");
		DEF_LOCALXCFRAMEWORK("ogg", projectOutputDirectory);
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_VORBIS")) {
		DEF_LOCALLIB_STATIC("libvorbis");
		DEF_LOCALLIB_STATIC("libvorbisfile");
		DEF_LOCALXCFRAMEWORK("vorbis", projectOutputDirectory
);
		DEF_LOCALXCFRAMEWORK("vorbisfile", projectOutputDirectory);
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_TREMOR")) {
		DEF_LOCALLIB_STATIC("libvorbisidec");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_THEORADEC")) {
		DEF_LOCALLIB_STATIC("libtheoradec");
		DEF_LOCALXCFRAMEWORK("theoradec", projectOutputDirectory);
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_RETROWAVE")) {
		DEF_LOCALLIB_STATIC("libretrowave");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_VPX")) {
		DEF_LOCALLIB_STATIC("libvpx");
		DEF_LOCALXCFRAMEWORK("vpx", projectOutputDirectory);
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_ZLIB")) {
		DEF_SYSTBD("libz");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_DISCORD")) {
		DEF_LOCALLIB_STATIC("libdiscord-rpc");
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_A52")) {
		DEF_LOCALLIB_STATIC("liba52");
		DEF_LOCALXCFRAMEWORK("a52", projectOutputDirectory);
	}
	if (CONTAINS_DEFINE(setup.defines, "USE_MPCDEC")) {
		DEF_LOCALLIB_STATIC("libmpcdec");
		DEF_LOCALXCFRAMEWORK("mpcdec", projectOutputDirectory);
	}

	if (setup.useSDL == kSDLVersion3) {
		if (!setup.appleEmbedded) DEF_LOCALLIB_STATIC("libSDL3");
		if (CONTAINS_DEFINE(setup.defines, "USE_SDL_NET")) {
			DEF_LOCALLIB_STATIC("libSDL3_net");
			DEF_LOCALXCFRAMEWORK("SDL3_net", projectOutputDirectory);
		}
	} else if (setup.useSDL == kSDLVersion2) {
		if (!setup.appleEmbedded) DEF_LOCALLIB_STATIC("libSDL2");
		if (CONTAINS_DEFINE(setup.defines, "USE_SDL_NET")) {
			DEF_LOCALLIB_STATIC("libSDL2_net");
			DEF_LOCALXCFRAMEWORK("SDL2_net", projectOutputDirectory);
		}
	} else if (setup.useSDL == kSDLVersion1) {
		if (!setup.appleEmbedded) DEF_LOCALLIB_STATIC("libSDL");
		if (CONTAINS_DEFINE(setup.defines, "USE_SDL_NET")) {
			DEF_LOCALLIB_STATIC("libSDL_net");
		}
	}

	frameworksGroup->_properties["children"] = children;
	_groups.add(frameworksGroup);
	// Force this to be added as a sub-group in the root.
	_rootSourceGroup->addChildGroup(frameworksGroup);


	///////////////////////////////////////////////////////
///////////////////
	// ScummVM-macOS
	if (!setup.appleEmbedded) {
		Object *framework_OSX = new Object(this, "PBXFrameworksBuildPhase_" OSX_TARGET, "PBXFrameworksBuildPhase", "PBXFrameworksBuildPhase", "", "Frameworks");

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

		if (CONTAINS_DEFINE(setup.defines, "USE_TTS") &&
			!CONTAINS_DEFINE(setup.defines, "USE_NS_SPEECH_SYNTHESIZER")) {
			frameworks_osx.push_back("AVFAudio.framework");
		}

		if (CONTAINS_DEFINE(setup.defines, "USE_FAAD")) {
			frameworks_osx.push_back(getLibString("faad", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_FLAC")) {
			frameworks_osx.push_back(getLibString("FLAC", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_FLUIDSYNTH") &&
			!CONTAINS_DEFINE(setup.defines, "USE_FLUIDLITE")) {
			frameworks_osx.push_back(getLibString("fluidsynth", setup.useXCFramework));
			frameworks_osx.push_back(getLibString("glib-2.0", setup.useXCFramework));
			frameworks_osx.push_back("libffi.tbd");
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_FREETYPE2")) {
			frameworks_osx.push_back(ge
tLibString("freetype", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_JPEG")) {
			frameworks_osx.push_back(getLibString("jpeg", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_LIBCURL")) {
			frameworks_osx.push_back(getLibString("curl", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_MAD")) {
			frameworks_osx.push_back(getLibString("mad", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_MIKMOD")) {
			frameworks_osx.push_back("libmikmod.a");
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_OPENMPT")) {
			frameworks_osx.push_back("libopenmpt.a");
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_MPEG2")) {
			frameworks_osx.push_back(getLibString("mpeg2", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_FRIBIDI")) {
			frameworks_osx.push_back(getLibString("fribidi", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_PNG")) {
			frameworks_osx.push_back(getLibString("png", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_GIF")) {
			frameworks_osx.push_back(getLibString("gif", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_OGG")) {
			frameworks_osx.push_back(getLibString("ogg", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_VORBIS")) {
			frameworks_osx.push_back(getLibString("vorbis", setup.useXCFramework));
			frameworks_osx.push_back(getLibString("vorbisfile", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_TREMOR")) {
			frameworks_osx.push_back(getLibString("vorbisidec", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_THEORADEC")) {
			frameworks_osx.push_back(getLibString("theoradec", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_RETROWAVE")) {
			frameworks_osx.push_back(getLibString("retrowave", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_VPX")) {
			frameworks_osx.push_back(ge
tLibString("vpx", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_ZLIB")) {
			frameworks_osx.push_back("libz.tbd");
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_DISCORD")) {
			frameworks_osx.push_back(getLibString("discord-rpc", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_A52")) {
			frameworks_osx.push_back(getLibString("a52", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_MPCDEC")) {
			frameworks_osx.push_back(getLibString("mpcdec", setup.useXCFramework));
		}

		frameworks_osx.push_back(getLibString(libSDL, setup.useXCFramework));
		if (CONTAINS_DEFINE(setup.defines, "USE_SDL_NET"))
			frameworks_osx.push_back(getLibString(libSDL + "_net", setup.useXCFramework));

		int order = 0;
		for (const auto &framework : frameworks_osx) {
			std::string id = "Frameworks_" + framework + "_osx";
			std::string comment = framework + " in Frameworks";

			ADD_SETTING_ORDER_NOVALUE(osx_files, getHash(id), comment, order++);
			ADD_BUILD_FILE(id, framework, getHash(framework), comment);
			ADD_FILE_REFERENCE(framework, framework, properties[framework]);
		}

		framework_OSX->_properties["files"] = osx_files;

		_frameworksBuildPhase.add(framework_OSX);
	}

	//////////////////////////////////////////////////////////////////////////
	// ScummVM-iOS
	if (setup.appleEmbedded) {
		Object *framework_iPhone = new Object(this, "PBXFrameworksBuildPhase_" IOS_TARGET, "PBXFrameworksBuildPhase", "PBXFrameworksBuildPhase", "", "Frameworks");

		framework_iPhone->addProperty("buildActionMask", "2147483647", "", kSettingsNoValue);
		framework_iPhone->addProperty("runOnlyForDeploymentPostprocessing", "0", "", kSettingsNoValue);

		// List of frameworks
		Property iOS_files;
		iOS_files._hasOrder = true;
		iOS_files._flags = kSettingsAsList;

		ValueList frameworks_iOS;
		frameworks_iOS.push_back("CoreAudio.framework");
		frameworks_iOS.push_back("CoreGraphics.framework");
		frameworks_iOS.push_back("CoreFoundation.framew
ork");
		frameworks_iOS.push_back("Foundation.framework");
		frameworks_iOS.push_back("GameController.framework");
		frameworks_iOS.push_back("UIKit.framework");
		frameworks_iOS.push_back("SystemConfiguration.framework");
		frameworks_iOS.push_back("AudioToolbox.framework");
		frameworks_iOS.push_back("QuartzCore.framework");
		frameworks_iOS.push_back("OpenGLES.framework");
		frameworks_iOS.push_back("CoreMIDI.framework");

		if (CONTAINS_DEFINE(setup.defines, "USE_TTS")) {
			frameworks_iOS.push_back("AVFAudio.framework");
		}

		if (CONTAINS_DEFINE(setup.defines, "USE_FAAD")) {
			frameworks_iOS.push_back(getLibString("faad", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_FLAC")) {
			frameworks_iOS.push_back(getLibString("FLAC", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_FREETYPE2")) {
			frameworks_iOS.push_back(getLibString("freetype", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_JPEG")) {
			frameworks_iOS.push_back(getLibString("jpeg", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_PNG")) {
			frameworks_iOS.push_back(getLibString("png", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_GIF")) {
			frameworks_iOS.push_back(getLibString("gif", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_OGG")) {
			frameworks_iOS.push_back(getLibString("ogg", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_VORBIS")) {
			frameworks_iOS.push_back(getLibString("vorbis", setup.useXCFramework));
			frameworks_iOS.push_back(getLibString("vorbisfile", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_TREMOR")) {
			frameworks_iOS.push_back(getLibString("vorbisidec", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_THEORADEC")) {
			frameworks_iOS.push_back(getLibString("theoradec", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_VPX")) {
			frameworks_iOS.
push_back(getLibString("vpx", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_MAD")) {
			frameworks_iOS.push_back(getLibString("mad", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_MIKMOD")) {
			frameworks_iOS.push_back(getLibString("mikmod", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_OPENMPT")) {
			frameworks_iOS.push_back(getLibString("openmpt", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_MPEG2")) {
			frameworks_iOS.push_back(getLibString("mpeg2", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defines, "USE_FRIBIDI")) {
			frameworks_iOS.push_back(getLibString("fribidi", setup.useXCFramework));
		}
		if (CONTAINS_DEFINE(setup.defin

... [Content truncated]