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

namespace CreateProjectTool {

#define DEBUG_XCODE_HASH 0

#ifdef ENABLE_IOS
#define IOS_TARGET 0
#define OSX_TARGET 1
#define SIM_TARGET 2
#else
#define OSX_TARGET 0
#endif

#define ADD_DEFINE(defines, name) \
	defines.push_back(name);

#define ADD_SETTING(config, key, value) \
	config.settings[key] = Setting(value, "", SettingsNoQuote);

#define ADD_SETTING_ORDER(config, key, value, order) \
	config.settings[key] = Setting(value, "", SettingsNoQuote, 0, order);

#define ADD_SETTING_ORDER_NOVALUE(config, key, comment, order) \
	config.settings[key] = Setting("", comment, SettingsNoValue, 0, order);

#define ADD_SETTING_QUOTE(config, key, value) \
	config.settings[key] = Setting(value);

#define ADD_SETTING_QUOTE_VAR(config, key, value) \
	config.settings[key] = Setting(value, "", SettingsQuoteVariable);

#define ADD_SETTING_LIST(config, key, values, flags, indent) \
	config.settings[key] = Setting(values, flags, indent);

#define REMOVE_SETTING(config, key) \
	config.settings.erase(key);

#define ADD_BUILD_FILE(id, name, fileRefId, comment) { \
	Object *buildFile = new Object(this, id, name, "PBXBuildFile", "PBXBuildFile", comment); \
	buildFile->addProperty("fileRef", fileRefId, name, SettingsNoValue); \
	_buildFile.add(buildFile); \
	_buildFile.flags = SettingsSingleItem; \
}

#define ADD_FILE_REFERENCE(id, name, properties) { \
	Object *fileRef = new Object(this, id, name, "PBXFileReference", "PBXFileReference", name); \
	if (!properties.fileEncoding.empty()) fileRef->addProperty("fileEncoding", properties.fileEncoding, "", SettingsNoValue); \
	if (!properties.lastKnownFileType.empty()) fileRef->addProperty("lastKnownFileType", properties.lastKnownFileType, "", SettingsNoValue|SettingsQuoteVariable); \
	if (!properties.fileName.empty()) fileRef->addProperty("name", properties.fileName, "", SettingsNoValue|SettingsQuoteVariable); \
	if (!properties.filePath.empty()) fileRef->addProperty("path", properties.filePath, "", SettingsNoValue|SettingsQuoteVariable); \
	if (!properties.sourceTree.empty()) fileRef->addProperty("sourceTree", properties.sourceTree, "", SettingsNoValue); \
	_fileReference.add(fileRef); \
	_fileReference.flags = SettingsSingleItem; \
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

XcodeProvider::Group::Group(XcodeProvider *objectParent, const std::string &groupName, const std::string &uniqueName, const std::string &path) : Object(objectParent, uniqueName, groupName, "PBXGroup", "", groupName) {
	addProperty("name", name, "", SettingsNoValue|SettingsQuoteVariable);
	addProperty("sourceTree", "<group>", "", SettingsNoValue|SettingsQuoteVariable);
	
	if (path != "") {
		addProperty("path", path, "", SettingsNoValue|SettingsQuoteVariable);
	}
	_childOrder = 0;
	_treeName = uniqueName;
}

void XcodeProvider::Group::ensureChildExists(const std::string &name) {
	std::map<std::string, Group*>::iterator it = _childGroups.find(name);
	if (it == _childGroups.end()) {
		Group *child = new Group(parent, name, this->_treeName + '/' + name, name);
		_childGroups[name] = child;
		addChildGroup(child);
		parent->_groups.add(child);
	}
}

void XcodeProvider::Group::addChildInternal(const std::string &id, const std::string &comment) {
	if (properties.find("children") == properties.end()) {
		Property children;
		children.hasOrder = true;
		children.flags = SettingsAsList;
		properties["children"] = children;
	}
	properties["children"].settings[id] = Setting("", comment + " in Sources", SettingsNoValue, 0, _childOrder++);
	if (_childOrder == 1) {
		// Force children to use () even when there is only 1 child.
		// Also this enforces the use of "," after the single item, instead of ; (see writeProperty)
		properties["children"].flags |= SettingsSingleItem;
	} else {
		properties["children"].flags ^= SettingsSingleItem;
	}

}

void XcodeProvider::Group::addChildGroup(const Group* group) {
	addChildInternal(parent->getHash(group->_treeName), group->_treeName);
}

void XcodeProvider::Group::addChildFile(const std::string &name) {
	std::string id = "FileReference_" + _treeName + "/" + name;
	addChildInternal(parent->getHash(id), name);
	FileProperty property = FileProperty(name, name, name, "\"<group>\"");

	parent->addFileReference(id, name, property);
	if (producesObjectFileOnOSX(name)) {
		parent->addBuildFile(_treeName + "/" + name, name, parent->getHash(id), name + " in Sources");
	}
}

void XcodeProvider::Group::addChildByHash(const std::string &hash, const std::string &name) {
	addChildInternal(hash, name);
}

XcodeProvider::Group *XcodeProvider::Group::getChildGroup(const std::string &name) {
	std::map<std::string, Group*>::iterator it = _childGroups.find(name);
	assert(it != _childGroups.end());
	return it->second;
}

XcodeProvider::Group *XcodeProvider::touchGroupsForPath(const std::string &path) {
	if (_rootSourceGroup == NULL) {
		assert (path == _projectRoot);
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
	if (!properties.fileEncoding.empty()) fileRef->addProperty("fileEncoding", properties.fileEncoding, "", SettingsNoValue);
	if (!properties.lastKnownFileType.empty()) fileRef->addProperty("lastKnownFileType", properties.lastKnownFileType, "", SettingsNoValue|SettingsQuoteVariable);
	if (!properties.fileName.empty()) fileRef->addProperty("name", properties.fileName, "", SettingsNoValue|SettingsQuoteVariable);
	if (!properties.filePath.empty()) fileRef->addProperty("path", properties.filePath, "", SettingsNoValue|SettingsQuoteVariable);
	if (!properties.sourceTree.empty()) fileRef->addProperty("sourceTree", properties.sourceTree, "", SettingsNoValue);
	_fileReference.add(fileRef);
	_fileReference.flags = SettingsSingleItem;
}

void XcodeProvider::addProductFileReference(const std::string &id, const std::string &name) {
	Object *fileRef = new Object(this, id, name, "PBXFileReference", "PBXFileReference", name);
	fileRef->addProperty("explicitFileType", "compiled.mach-o.executable", "", SettingsNoValue|SettingsQuoteVariable);
	fileRef->addProperty("includeInIndex", "0", "", SettingsNoValue);
	fileRef->addProperty("path", name, "", SettingsNoValue|SettingsQuoteVariable);
	fileRef->addProperty("sourceTree", "BUILT_PRODUCTS_DIR", "", SettingsNoValue);
	_fileReference.add(fileRef);
	_fileReference.flags = SettingsSingleItem;
}

void XcodeProvider::addBuildFile(const std::string &id, const std::string &name, const std::string &fileRefId, const std::string &comment) {

	Object *buildFile = new Object(this, id, name, "PBXBuildFile", "PBXBuildFile", comment);
	buildFile->addProperty("fileRef", fileRefId, name, SettingsNoValue);
	_buildFile.add(buildFile);
	_buildFile.flags = SettingsSingleItem;
}

XcodeProvider::XcodeProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version)
	: ProjectProvider(global_warnings, project_warnings, version) {
	_rootSourceGroup = NULL;
}

void XcodeProvider::createWorkspace(const BuildSetup &setup) {
	// Create project folder
	std::string workspace = setup.outputDir + '/' + PROJECT_NAME ".xcodeproj";
	createDirectory(workspace);
	_projectRoot = setup.srcDir;
	touchGroupsForPath(_projectRoot);
	
	// Setup global objects
	setupDefines(setup);
#ifdef ENABLE_IOS
	_targets.push_back(PROJECT_DESCRIPTION "-iPhone");
#endif
	_targets.push_back(PROJECT_DESCRIPTION "-OS X");
#ifdef ENABLE_IOS
	_targets.push_back(PROJECT_DESCRIPTION "-Simulator");
#endif
	setupCopyFilesBuildPhase();
	setupFrameworksBuildPhase();
	setupNativeTarget();
	setupProject();
	setupResourcesBuildPhase();
	setupBuildConfiguration();
}

// We are done with constructing all the object graph and we got through every project, output the main project file
// (this is kind of a hack since other providers use separate project files)
void XcodeProvider::createOtherBuildFiles(const BuildSetup &setup) {
	// This needs to be done at the end when all build files have been accounted for
	setupSourcesBuildPhase();

	ouputMainProjectFile(setup);
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
	if (modulePath.size())
		addFilesToProject(moduleDir, project, includeList, excludeList, setup.filePrefix + '/' + modulePath);
	else
		addFilesToProject(moduleDir, project, includeList, excludeList, setup.filePrefix);
}

//////////////////////////////////////////////////////////////////////////
// Main Project file
//////////////////////////////////////////////////////////////////////////
void XcodeProvider::ouputMainProjectFile(const BuildSetup &setup) {
	std::ofstream project((setup.outputDir + '/' + PROJECT_NAME ".xcodeproj" + '/' + "project.pbxproj").c_str());
	if (!project)
		error("Could not open \"" + setup.outputDir + '/' + PROJECT_NAME ".xcodeproj" + '/' + "project.pbxproj\" for writing");

	//////////////////////////////////////////////////////////////////////////
	// Header
	project << "// !$*UTF8*$!\n"
	           "{\n"
	           "\t" << writeSetting("archiveVersion", "1", "", SettingsNoQuote) << ";\n"
	           "\tclasses = {\n"
	           "\t};\n"
	           "\t" << writeSetting("objectVersion", "46", "", SettingsNoQuote) << ";\n"
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
	           "\t" << writeSetting("rootObject", getHash("PBXProject"), "Project object", SettingsNoQuote) << ";\n"
	           "}\n";

}

//////////////////////////////////////////////////////////////////////////
// Files
//////////////////////////////////////////////////////////////////////////
void XcodeProvider::writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int indentation,
                                           const StringList &duplicate, const std::string &objPrefix, const std::string &filePrefix) {

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
			writeFileListToProject(*node, projectFile, indentation + 1, duplicate, objPrefix + node->name + '_', filePrefix + node->name + '/');
	}
}

//////////////////////////////////////////////////////////////////////////
// Setup functions
//////////////////////////////////////////////////////////////////////////
void XcodeProvider::setupCopyFilesBuildPhase() {
	// Nothing to do here
}

#define DEF_SYSFRAMEWORK(framework) properties[framework".framework"] = FileProperty("wrapper.framework", framework".framework", "System/Library/Frameworks/" framework ".framework", "SDKROOT"); \
	ADD_SETTING_ORDER_NOVALUE(children, getHash(framework".framework"), framework".framework", fwOrder++);
	
#define DEF_LOCALLIB_STATIC(lib) properties[lib".a"] = FileProperty("archive.ar", lib".a", "/opt/local/lib/" lib ".a", "\"<group>\""); \
	ADD_SETTING_ORDER_NOVALUE(children, getHash(lib".a"), lib".a", fwOrder++);

/**
 * Sets up the frameworks build phase.
 *
 * (each native target has different build rules)
 */
void XcodeProvider::setupFrameworksBuildPhase() {
	_frameworksBuildPhase.comment = "PBXFrameworksBuildPhase";

	// Just use a hardcoded id for the Frameworks-group
	Group *frameworksGroup = new Group(this, "Frameworks", "PBXGroup_CustomTemplate_Frameworks_", "");

	Property children;
	children.hasOrder = true;
	children.flags = SettingsAsList;

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
	DEF_SYSFRAMEWORK("CoreFoundation");
	DEF_SYSFRAMEWORK("CoreMIDI");
	DEF_SYSFRAMEWORK("Foundation");
	DEF_SYSFRAMEWORK("IOKit");
	DEF_SYSFRAMEWORK("OpenGLES");
	DEF_SYSFRAMEWORK("QuartzCore");
	DEF_SYSFRAMEWORK("QuickTime");
	DEF_SYSFRAMEWORK("UIKit");
	// Optionals:
	DEF_SYSFRAMEWORK("OpenGL");

	// Local libraries
	DEF_LOCALLIB_STATIC("libFLAC");
	DEF_LOCALLIB_STATIC("libmad");
	DEF_LOCALLIB_STATIC("libvorbisidec");
	DEF_LOCALLIB_STATIC("libfreetype");
//	DEF_LOCALLIB_STATIC("libmpeg2");

	frameworksGroup->properties["children"] = children;
	_groups.add(frameworksGroup);
	// Force this to be added as a sub-group in the root.
	_rootSourceGroup->addChildGroup(frameworksGroup);


	// Declare this here, as it's used across the three targets
	int order = 0;
#ifdef ENABLE_IOS
	//////////////////////////////////////////////////////////////////////////
	// iPhone
	Object *framework_iPhone = new Object(this, "PBXFrameworksBuildPhase_" + _targets[IOS_TARGET], "PBXFrameworksBuildPhase", "PBXFrameworksBuildPhase", "", "Frameworks");

	framework_iPhone->addProperty("buildActionMask", "2147483647", "", SettingsNoValue);
	framework_iPhone->addProperty("runOnlyForDeploymentPostprocessing", "0", "", SettingsNoValue);

	// List of frameworks
	Property iPhone_files;
	iPhone_files.hasOrder = true;
	iPhone_files.flags = SettingsAsList;

	ValueList frameworks_iPhone;
	frameworks_iPhone.push_back("CoreAudio.framework");
	frameworks_iPhone.push_back("CoreFoundation.framework");
	frameworks_iPhone.push_back("Foundation.framework");
	frameworks_iPhone.push_back("UIKit.framework");
	frameworks_iPhone.push_back("AudioToolbox.framework");
	frameworks_iPhone.push_back("QuartzCore.framework");
	frameworks_iPhone.push_back("libmad.a");
	//frameworks_iPhone.push_back("libmpeg2.a");
	frameworks_iPhone.push_back("libFLAC.a");
	frameworks_iPhone.push_back("libvorbisidec.a");
	frameworks_iPhone.push_back("OpenGLES.framework");

	for (ValueList::iterator framework = frameworks_iPhone.begin(); framework != frameworks_iPhone.end(); framework++) {
		std::string id = "Frameworks_" + *framework + "_iphone";
		std::string comment = *framework + " in Frameworks";

		ADD_SETTING_ORDER_NOVALUE(iPhone_files, getHash(id), comment, order++);
		ADD_BUILD_FILE(id, *framework, getHash(*framework), comment);
		ADD_FILE_REFERENCE(*framework, *framework, properties[*framework]);
	}

	framework_iPhone->properties["files"] = iPhone_files;

	_frameworksBuildPhase.add(framework_iPhone);
#endif
	//////////////////////////////////////////////////////////////////////////
	// ScummVM-OS X
	Object *framework_OSX = new Object(this, "PBXFrameworksBuildPhase_" + _targets[OSX_TARGET], "PBXFrameworksBuildPhase", "PBXFrameworksBuildPhase", "", "Frameworks");

	framework_OSX->addProperty("buildActionMask", "2147483647", "", SettingsNoValue);
	framework_OSX->addProperty("runOnlyForDeploymentPostprocessing", "0", "", SettingsNoValue);

	// List of frameworks
	Property osx_files;
	osx_files.hasOrder = true;
	osx_files.flags = SettingsAsList;

	ValueList frameworks_osx;
	frameworks_osx.push_back("CoreFoundation.framework");
	frameworks_osx.push_back("Foundation.framework");
	frameworks_osx.push_back("AudioToolbox.framework");
	frameworks_osx.push_back("QuickTime.framework");
	frameworks_osx.push_back("CoreMIDI.framework");
	frameworks_osx.push_back("CoreAudio.framework");
	frameworks_osx.push_back("QuartzCore.framework");
	frameworks_osx.push_back("Carbon.framework");
	frameworks_osx.push_back("ApplicationServices.framework");
	frameworks_osx.push_back("IOKit.framework");
	frameworks_osx.push_back("Cocoa.framework");
	frameworks_osx.push_back("AudioUnit.framework");
	// Optionals:
	frameworks_osx.push_back("OpenGL.framework");

	order = 0;
	for (ValueList::iterator framework = frameworks_osx.begin(); framework != frameworks_osx.end(); framework++) {
		std::string id = "Frameworks_" + *framework + "_osx";
		std::string comment = *framework + " in Frameworks";

		ADD_SETTING_ORDER_NOVALUE(osx_files, getHash(id), comment, order++);
		ADD_BUILD_FILE(id, *framework, getHash(*framework), comment);
		ADD_FILE_REFERENCE(*framework, *framework, properties[*framework]);
	}

	framework_OSX->properties["files"] = osx_files;

	_frameworksBuildPhase.add(framework_OSX);
#ifdef ENABLE_IOS
	//////////////////////////////////////////////////////////////////////////
	// Simulator
	Object *framework_simulator = new Object(this, "PBXFrameworksBuildPhase_" + _targets[SIM_TARGET], "PBXFrameworksBuildPhase", "PBXFrameworksBuildPhase", "", "Frameworks");

	framework_simulator->addProperty("buildActionMask", "2147483647", "", SettingsNoValue);
	framework_simulator->addProperty("runOnlyForDeploymentPostprocessing", "0", "", SettingsNoValue);

	// List of frameworks
	Property simulator_files;
	simulator_files.hasOrder = true;
	simulator_files.flags = SettingsAsList;

	ValueList frameworks_simulator;
	frameworks_simulator.push_back("CoreAudio.framework");
	frameworks_simulator.push_back("CoreFoundation.framework");
	frameworks_simulator.push_back("Foundation.framework");
	frameworks_simulator.push_back("UIKit.framework");
	frameworks_simulator.push_back("AudioToolbox.framework");
	frameworks_simulator.push_back("QuartzCore.framework");
	frameworks_simulator.push_back("OpenGLES.framework");

	order = 0;
	for (ValueList::iterator framework = frameworks_simulator.begin(); framework != frameworks_simulator.end(); framework++) {
		std::string id = "Frameworks_" + *framework + "_simulator";
		std::string comment = *framework + " in Frameworks";

		ADD_SETTING_ORDER_NOVALUE(simulator_files, getHash(id), comment, order++);
		ADD_BUILD_FILE(id, *framework, getHash(*framework), comment);
		ADD_FILE_REFERENCE(*framework, *framework, properties[*framework]);
	}

	framework_simulator->properties["files"] = simulator_files;

	_frameworksBuildPhase.add(framework_simulator);
#endif
}

void XcodeProvider::setupNativeTarget() {
	_nativeTarget.comment = "PBXNativeTarget";

	// Just use a hardcoded id for the Products-group
	Group *productsGroup = new Group(this, "Products", "PBXGroup_CustomTemplate_Products_" , "");
	// Output native target section
	for (unsigned int i = 0; i < _targets.size(); i++) {
#ifndef ENABLE_IOS
		if (i != OSX_TARGET) { // TODO: Fix iOS-targets, for now just disable them.
			continue;
		}
#endif
		Object *target = new Object(this, "PBXNativeTarget_" + _targets[i], "PBXNativeTarget", "PBXNativeTarget", "", _targets[i]);

		target->addProperty("buildConfigurationList", getHash("XCConfigurationList_" + _targets[i]), "Build configuration list for PBXNativeTarget \"" + _targets[i] + "\"", SettingsNoValue);

		Property buildPhases;
		buildPhases.hasOrder = true;
		buildPhases.flags = SettingsAsList;
		buildPhases.settings[getHash("PBXResourcesBuildPhase_" + _targets[i])] = Setting("", "Resources", SettingsNoValue, 0, 0);
		buildPhases.settings[getHash("PBXSourcesBuildPhase_" + _targets[i])] = Setting("", "Sources", SettingsNoValue, 0, 1);
		buildPhases.settings[getHash("PBXFrameworksBuildPhase_" + _targets[i])] = Setting("", "Frameworks", SettingsNoValue, 0, 2);
		target->properties["buildPhases"] = buildPhases;

		target->addProperty("buildRules", "", "", SettingsNoValue|SettingsAsList);

		target->addProperty("dependencies", "", "", SettingsNoValue|SettingsAsList);

		target->addProperty("name", _targets[i], "", SettingsNoValue|SettingsQuoteVariable);
		target->addProperty("productName", PROJECT_NAME, "", SettingsNoValue);
		addProductFileReference("PBXFileReference_" PROJECT_DESCRIPTION ".app_" + _targets[i], PROJECT_DESCRIPTION ".app");
		productsGroup->addChildByHash(getHash("PBXFileReference_" PROJECT_DESCRIPTION ".app_" + _targets[i]), PROJECT_DESCRIPTION ".app");
		target->addProperty("productReference", getHash("PBXFileReference_" PROJECT_DESCRIPTION ".app_" + _targets[i]), PROJECT_DESCRIPTION ".app", SettingsNoValue);
		target->addProperty("productType", "com.apple.product-type.application", "", SettingsNoValue|SettingsQuoteVariable);

		_nativeTarget.add(target);
	}
	_rootSourceGroup->addChildGroup(productsGroup);
	_groups.add(productsGroup);
}

void XcodeProvider::setupProject() {
	_project.comment = "PBXProject";

	Object *project = new Object(this, "PBXProject", "PBXProject", "PBXProject", "", "Project object");

	project->addProperty("buildConfigurationList", getHash("XCConfigurationList_scummvm"), "Build configuration list for PBXProject \"" PROJECT_NAME "\"", SettingsNoValue);
	project->addProperty("compatibilityVersion", "Xcode 3.2", "", SettingsNoValue|SettingsQuoteVariable);
	project->addProperty("developmentRegion", "English", "", SettingsNoValue);
	project->addProperty("hasScannedForEncodings", "1", "", SettingsNoValue);

	// List of known regions
	Property regions;
	regions.flags = SettingsAsList;
	ADD_SETTING_ORDER_NOVALUE(regions, "English", "", 0);
	ADD_SETTING_ORDER_NOVALUE(regions, "Japanese", "", 1);
	ADD_SETTING_ORDER_NOVALUE(regions, "French", "", 2);
	ADD_SETTING_ORDER_NOVALUE(regions, "German", "", 3);
	project->properties["knownRegions"] = regions;

	project->addProperty("mainGroup", _rootSourceGroup->getHashRef(), "CustomTemplate", SettingsNoValue);
	project->addProperty("projectDirPath", _projectRoot, "", SettingsNoValue|SettingsQuoteVariable);
	project->addProperty("projectRoot", "", "", SettingsNoValue|SettingsQuoteVariable);

	// List of targets
	Property targets;
	targets.flags = SettingsAsList;
#ifdef ENABLE_IOS
	targets.settings[getHash("PBXNativeTarget_" + _targets[IOS_TARGET])] = Setting("", _targets[IOS_TARGET], SettingsNoValue, 0, 0);
#endif
	targets.settings[getHash("PBXNativeTarget_" + _targets[OSX_TARGET])] = Setting("", _targets[OSX_TARGET], SettingsNoValue, 0, 1);
#ifdef ENABLE_IOS
	targets.settings[getHash("PBXNativeTarget_" + _targets[SIM_TARGET])] = Setting("", _targets[SIM_TARGET], SettingsNoValue, 0, 2);
#endif
	project->properties["targets"] = targets;
#ifndef ENABLE_IOS
	// Force list even when there is only a single target
	project->properties["targets"].flags |= SettingsSingleItem;
#endif

	_project.add(project);
}

void XcodeProvider::setupResourcesBuildPhase() {
	_resourcesBuildPhase.comment = "PBXResourcesBuildPhase";

	// Setup resource file properties
	std::map<std::string, FileProperty> properties;
	properties["scummclassic.zip"] = FileProperty("archive.zip", "", "scummclassic.zip", "\"<group>\"");
	properties["scummmodern.zip"]  = FileProperty("archive.zip", "", "scummmodern.zip", "\"<group>\"");

	properties["kyra.dat"]         = FileProperty("file", "", "kyra.dat", "\"<group>\"");
	properties["lure.dat"]         = FileProperty("file", "", "lure.dat", "\"<group>\"");
	properties["queen.tbl"]        = FileProperty("file", "", "queen.tbl", "\"<group>\"");
	properties["sky.cpt"]          = FileProperty("file", "", "sky.cpt", "\"<group>\"");
	properties["drascula.dat"]     = FileProperty("file", "", "drascula.dat", "\"<group>\"");
	properties["hugo.dat"]         = FileProperty("file", "", "hugo.dat", "\"<group>\"");
	properties["teenagent.dat"]    = FileProperty("file", "", "teenagent.dat", "\"<group>\"");
	properties["toon.dat"]         = FileProperty("file", "", "toon.dat", "\"<group>\"");

	properties["Default.png"]      = FileProperty("image.png", "", "Default.png", "\"<group>\"");
	properties["icon.png"]         = FileProperty("image.png", "", "icon.png", "\"<group>\"");
	properties["icon-72.png"]      = FileProperty("image.png", "", "icon-72.png", "\"<group>\"");
	properties["icon4.png"]        = FileProperty("image.png", "", "icon4.png", "\"<group>\"");

	// Same as for containers: a rule for each native target
	for (unsigned int i = 0; i < _targets.size(); i++) {
		Object *resource = new Object(this, "PBXResourcesBuildPhase_" + _targets[i], "PBXResourcesBuildPhase", "PBXResourcesBuildPhase", "", "Resources");

		resource->addProperty("buildActionMask", "2147483647", "", SettingsNoValue);

		// Add default files
		Property files;
		files.hasOrder = true;
		files.flags = SettingsAsList;

		ValueList files_list;
		files_list.push_back("scummclassic.zip");
		files_list.push_back("scummmodern.zip");
		files_list.push_back("kyra.dat");
		files_list.push_back("lure.dat");
		files_list.push_back("queen.tbl");
		files_list.push_back("sky.cpt");
		files_list.push_back("Default.png");
		files_list.push_back("icon.png");
		files_list.push_back("icon-72.png");
		files_list.push_back("icon4.png");
		files_list.push_back("drascula.dat");
		files_list.push_back("hugo.dat");
		files_list.push_back("teenagent.dat");
		files_list.push_back("toon.dat");

		int order = 0;
		for (ValueList::iterator file = files_list.begin(); file != files_list.end(); file++) {
			std::string id = "PBXResources_" + *file;
			std::string comment = *file + " in Resources";

			ADD_SETTING_ORDER_NOVALUE(files, getHash(id), comment, order++);
			// TODO Fix crash when adding build file for data
			//ADD_BUILD_FILE(id, *file, comment);
			ADD_FILE_REFERENCE(*file, *file, properties[*file]);
		}

		// Add custom files depending on the target
		if (_targets[i] == PROJECT_DESCRIPTION "-OS X") {
			files.settings[getHash("PBXResources_" PROJECT_NAME ".icns")] = Setting("", PROJECT_NAME ".icns in Resources", SettingsNoValue, 0, 6);

			// Remove 2 iphone icon files
			files.settings.erase(getHash("PBXResources_Default.png"));
			files.settings.erase(getHash("PBXResources_icon.png"));
		}

		resource->properties["files"] = files;

		resource->addProperty("runOnlyForDeploymentPostprocessing", "0", "", SettingsNoValue);

		_resourcesBuildPhase.add(resource);
	}
}

void XcodeProvider::setupSourcesBuildPhase() {
	_sourcesBuildPhase.comment = "PBXSourcesBuildPhase";

	// Setup source file properties
	std::map<std::string, FileProperty> properties;

	// Same as for containers: a rule for each native target
	for (unsigned int i = 0; i < _targets.size(); i++) {
		Object *source = new Object(this, "PBXSourcesBuildPhase_" + _targets[i], "PBXSourcesBuildPhase", "PBXSourcesBuildPhase", "", "Sources");

		source->addProperty("buildActionMask", "2147483647", "", SettingsNoValue);

		Property files;
		files.hasOrder = true;
		files.flags = SettingsAsList;

		int order = 0;
		for (std::vector<Object*>::iterator file = _buildFile.objects.begin(); file !=_buildFile.objects.end(); ++file) {
			if (!producesObjectFileOnOSX((*file)->name)) {
				continue;
			}
			std::string comment = (*file)->name + " in Sources";
			ADD_SETTING_ORDER_NOVALUE(files, getHash((*file)->id), comment, order++);
		}

		source->properties["files"] = files;

		source->addProperty("runOnlyForDeploymentPostprocessing", "0", "", SettingsNoValue);

		_sourcesBuildPhase.add(source);
	}
}

// Setup all build configurations
void XcodeProvider::setupBuildConfiguration() {

	_buildConfiguration.comment = "XCBuildConfiguration";
	_buildConfiguration.flags = SettingsAsList;

	///****************************************
	// * iPhone
	// ****************************************/
#ifdef ENABLE_IOS
	// Debug
	Object *iPhone_Debug_Object = new Object(this, "XCBuildConfiguration_" PROJECT_DESCRIPTION "-iPhone_Debug", _targets[IOS_TARGET] /* ScummVM-iPhone */, "XCBuildConfiguration", "PBXNativeTarget", "Debug");
	Property iPhone_Debug;
	ADD_SETTING_QUOTE(iPhone_Debug, "ARCHS", "$(ARCHS_UNIVERSAL_IPHONE_OS)");
	ADD_SETTING_QUOTE(iPhone_Debug, "CODE_SIGN_IDENTITY", "iPhone Developer");
	ADD_SETTING_QUOTE_VAR(iPhone_Debug, "CODE_SIGN_IDENTITY[sdk=iphoneos*]", "iPhone Developer");
	ADD_SETTING(iPhone_Debug, "COMPRESS_PNG_FILES", "NO");
	ADD_SETTING(iPhone_Debug, "COPY_PHASE_STRIP", "NO");
	ADD_SETTING_QUOTE(iPhone_Debug, "DEBUG_INFORMATION_FORMAT", "dwarf-with-dsym");
	ValueList iPhone_FrameworkSearchPaths;
	iPhone_FrameworkSearchPaths.push_back("$(inherited)");
	iPhone_FrameworkSearchPaths.push_back("\"$(SDKROOT)$(SYSTEM_LIBRARY_DIR)/PrivateFrameworks\"");
	ADD_SETTING_LIST(iPhone_Debug, "FRAMEWORK_SEARCH_PATHS", iPhone_FrameworkSearchPaths, SettingsAsList, 5);
	ADD_SETTING(iPhone_Debug, "GCC_DYNAMIC_NO_PIC", "NO");
	ADD_SETTING(iPhone_Debug, "GCC_ENABLE_CPP_EXCEPTIONS", "NO");
	ADD_SETTING(iPhone_Debug, "GCC_ENABLE_FIX_AND_CONTINUE", "NO");
	ADD_SETTING(iPhone_Debug, "GCC_OPTIMIZATION_LEVEL", "0");
	ADD_SETTING(iPhone_Debug, "GCC_PRECOMPILE_PREFIX_HEADER", "NO");
	ADD_SETTING_QUOTE(iPhone_Debug, "GCC_PREFIX_HEADER", "");
	ADD_SETTING(iPhone_Debug, "GCC_THUMB_SUPPORT", "NO");
	ADD_SETTING(iPhone_Debug, "GCC_UNROLL_LOOPS", "YES");
	ValueList iPhone_HeaderSearchPaths;
	iPhone_HeaderSearchPaths.push_back("$(SRCROOT)/engines/");
	iPhone_HeaderSearchPaths.push_back("$(SRCROOT)");
	iPhone_HeaderSearchPaths.push_back("include/");
	ADD_SETTING_LIST(iPhone_Debug, "HEADER_SEARCH_PATHS", iPhone_HeaderSearchPaths, SettingsAsList|SettingsQuoteVariable, 5);
	ADD_SETTING(iPhone_Debug, "INFOPLIST_FILE", "Info.plist");
	ValueList iPhone_LibPaths;
	iPhone_LibPaths.push_back("$(inherited)");
	iPhone_LibPaths.push_back("\"$(SRCROOT)/lib\"");
	ADD_SETTING_LIST(iPhone_Debug, "LIBRARY_SEARCH_PATHS", iPhone_LibPaths, SettingsAsList, 5);
	ADD_SETTING(iPhone_Debug, "ONLY_ACTIVE_ARCH", "YES");
	ADD_SETTING(iPhone_Debug, "PREBINDING", "NO");
	ADD_SETTING(iPhone_Debug, "PRODUCT_NAME", PROJECT_DESCRIPTION);
	ADD_SETTING_QUOTE(iPhone_Debug, "PROVISIONING_PROFILE", "EF590570-5FAC-4346-9071-D609DE2B28D8");
	ADD_SETTING_QUOTE_VAR(iPhone_Debug, "PROVISIONING_PROFILE[sdk=iphoneos*]", "");
	ADD_SETTING(iPhone_Debug, "SDKROOT", "iphoneos4.0");
	ADD_SETTING_QUOTE(iPhone_Debug, "TARGETED_DEVICE_FAMILY", "1,2");

	iPhone_Debug_Object->addProperty("name", "Debug", "", SettingsNoValue);
	iPhone_Debug_Object->properties["buildSettings"] = iPhone_Debug;

	// Release
	Object *iPhone_Release_Object = new Object(this, "XCBuildConfiguration_" PROJECT_DESCRIPTION "-iPhone_Release", _targets[IOS_TARGET] /* ScummVM-iPhone */, "XCBuildConfiguration", "PBXNativeTarget", "Release");
	Property iPhone_Release(iPhone_Debug);
	ADD_SETTING(iPhone_Release, "GCC_OPTIMIZATION_LEVEL", "3");
	ADD_SETTING(iPhone_Release, "COPY_PHASE_STRIP", "YES");
	REMOVE_SETTING(iPhone_Release, "GCC_DYNAMIC_NO_PIC");
	ADD_SETTING(iPhone_Release, "WRAPPER_EXTENSION", "app");

	iPhone_Release_Object->addProperty("name", "Release", "", SettingsNoValue);
	iPhone_Release_Object->properties["buildSettings"] = iPhone_Release;

	_buildConfiguration.add(iPhone_Debug_Object);
	_buildConfiguration.add(iPhone_Release_Object);
#endif
	/****************************************
	 * scummvm
	 ****************************************/

	// Debug
	Object *scummvm_Debug_Object = new Object(this, "XCBuildConfiguration_" PROJECT_NAME "_Debug", PROJECT_NAME, "XCBuildConfiguration", "PBXProject", "Debug");
	Property scummvm_Debug;
	ADD_SETTING(scummvm_Debug, "ALWAYS_SEARCH_USER_PATHS", "NO");
	ADD_SETTING_QUOTE(scummvm_Debug, "USER_HEADER_SEARCH_PATHS", "$(SRCROOT) $(SRCROOT)/engines");
	ADD_SETTING_QUOTE(scummvm_Debug, "ARCHS", "$(ARCHS_STANDARD_32_BIT)");
	ADD_SETTING_QUOTE(scummvm_Debug, "CODE_SIGN_IDENTITY", "Don't Code Sign");
	ADD_SETTING_QUOTE_VAR(scummvm_Debug, "CODE_SIGN_IDENTITY[sdk=iphoneos*]", "Don't Code Sign");
	ADD_SETTING_QUOTE(scummvm_Debug, "FRAMEWORK_SEARCH_PATHS", "");
	ADD_SETTING(scummvm_Debug, "GCC_C_LANGUAGE_STANDARD", "c99");
	ADD_SETTING(scummvm_Debug, "GCC_ENABLE_CPP_EXCEPTIONS", "NO");
	ADD_SETTING(scummvm_Debug, "GCC_ENABLE_CPP_RTTI", "YES");
	ADD_SETTING(scummvm_Debug, "GCC_INPUT_FILETYPE", "automatic");
	ADD_SETTING(scummvm_Debug, "GCC_OPTIMIZATION_LEVEL", "0");
	ValueList scummvm_defines(_defines);
	ADD_DEFINE(scummvm_defines, "IPHONE");
	ADD_DEFINE(scummvm_defines, "XCODE");
	ADD_DEFINE(scummvm_defines, "IPHONE_OFFICIAL");
	ADD_SETTING_LIST(scummvm_Debug, "GCC_PREPROCESSOR_DEFINITIONS", scummvm_defines, SettingsNoQuote|SettingsAsList, 5);
	ADD_SETTING(scummvm_Debug, "GCC_THUMB_SUPPORT", "NO");
	ADD_SETTING(scummvm_Debug, "GCC_USE_GCC3_PFE_SUPPORT", "NO");
	ADD_SETTING(scummvm_Debug, "GCC_WARN_ABOUT_RETURN_TYPE", "YES");
	ADD_SETTING(scummvm_Debug, "GCC_WARN_UNUSED_VARIABLE", "YES");
	ValueList scummvm_HeaderPaths;
	scummvm_HeaderPaths.push_back("include/");
	scummvm_HeaderPaths.push_back("$(SRCROOT)/engines/");
	scummvm_HeaderPaths.push_back("$(SRCROOT)");
	ADD_SETTING_LIST(scummvm_Debug, "HEADER_SEARCH_PATHS", scummvm_HeaderPaths, SettingsQuoteVariable|SettingsAsList, 5);
	ADD_SETTING_QUOTE(scummvm_Debug, "LIBRARY_SEARCH_PATHS", "");
	ADD_SETTING(scummvm_Debug, "ONLY_ACTIVE_ARCH", "YES");
	ADD_SETTING_QUOTE(scummvm_Debug, "OTHER_CFLAGS", "");
	ADD_SETTING_QUOTE(scummvm_Debug, "OTHER_LDFLAGS", "-lz");
	ADD_SETTING(scummvm_Debug, "PREBINDING", "NO");
	ADD_SETTING(scummvm_Debug, "SDKROOT", "macosx");

	scummvm_Debug_Object->addProperty("name", "Debug", "", SettingsNoValue);
	scummvm_Debug_Object->properties["buildSettings"] = scummvm_Debug;

	// Release
	Object *scummvm_Release_Object = new Object(this, "XCBuildConfiguration_" PROJECT_NAME "_Release", PROJECT_NAME, "XCBuildConfiguration", "PBXProject", "Release");
	Property scummvm_Release(scummvm_Debug);
	REMOVE_SETTING(scummvm_Release, "GCC_C_LANGUAGE_STANDARD");       // Not sure why we remove that, or any of the other warnings
	REMOVE_SETTING(scummvm_Release, "GCC_WARN_ABOUT_RETURN_TYPE");
	REMOVE_SETTING(scummvm_Release, "GCC_WARN_UNUSED_VARIABLE");
	REMOVE_SETTING(scummvm_Release, "ONLY_ACTIVE_ARCH");

	scummvm_Release_Object->addProperty("name", "Release", "", SettingsNoValue);
	scummvm_Release_Object->properties["buildSettings"] = scummvm_Release;

	_buildConfiguration.add(scummvm_Debug_Object);
	_buildConfiguration.add(scummvm_Release_Object);

	/****************************************
	 * ScummVM-OS X
	 ****************************************/

	// Debug
	Object *scummvmOSX_Debug_Object = new Object(this, "XCBuildConfiguration_" PROJECT_DESCRIPTION "-OSX_Debug", _targets[OSX_TARGET] /* ScummVM-OS X */, "XCBuildConfiguration", "PBXNativeTarget", "Debug");
	Property scummvmOSX_Debug;
	ADD_SETTING_QUOTE(scummvmOSX_Debug, "ARCHS", "$(NATIVE_ARCH)");
	ADD_SETTING(scummvmOSX_Debug, "COMPRESS_PNG_FILES", "NO");
	ADD_SETTING(scummvmOSX_Debug, "COPY_PHASE_STRIP", "NO");
	ADD_SETTING_QUOTE(scummvmOSX_Debug, "DEBUG_INFORMATION_FORMAT", "dwarf-with-dsym");
	ADD_SETTING_QUOTE(scummvmOSX_Debug, "FRAMEWORK_SEARCH_PATHS", "");
	ADD_SETTING(scummvmOSX_Debug, "GCC_C_LANGUAGE_STANDARD", "c99");
	ADD_SETTING(scummvmOSX_Debug, "GCC_ENABLE_CPP_EXCEPTIONS", "NO");
	ADD_SETTING(scummvmOSX_Debug, "GCC_ENABLE_CPP_RTTI", "YES");
	ADD_SETTING(scummvmOSX_Debug, "GCC_DYNAMIC_NO_PIC", "NO");
	ADD_SETTING(scummvmOSX_Debug, "GCC_ENABLE_FIX_AND_CONTINUE", "NO");
	ADD_SETTING(scummvmOSX_Debug, "GCC_OPTIMIZATION_LEVEL", "0");
	ADD_SETTING(scummvmOSX_Debug, "GCC_PRECOMPILE_PREFIX_HEADER", "NO");
	ADD_SETTING_QUOTE(scummvmOSX_Debug, "GCC_PREFIX_HEADER", "");
	ValueList scummvmOSX_defines(_defines);
	ADD_DEFINE(scummvmOSX_defines, "SDL_BACKEND");
	ADD_DEFINE(scummvmOSX_defines, "MACOSX");
	ADD_SETTING_LIST(scummvmOSX_Debug, "GCC_PREPROCESSOR_DEFINITIONS", scummvmOSX_defines, SettingsNoQuote|SettingsAsList, 5);
	ADD_SETTING_QUOTE(scummvmOSX_Debug, "GCC_VERSION", "");
	ValueList scummvmOSX_HeaderPaths;
	scummvmOSX_HeaderPaths.push_back("/opt/local/include/SDL");
	scummvmOSX_HeaderPaths.push_back("/opt/local/include");
	scummvmOSX_HeaderPaths.push_back("/opt/local/include/freetype2");
	scummvmOSX_HeaderPaths.push_back("include/");
	scummvmOSX_HeaderPaths.push_back("$(SRCROOT)/engines/");
	scummvmOSX_HeaderPaths.push_back("$(SRCROOT)");
	ADD_SETTING_LIST(scummvmOSX_Debug, "HEADER_SEARCH_PATHS", scummvmOSX_HeaderPaths, SettingsQuoteVariable|SettingsAsList, 5);
	ADD_SETTING_QUOTE(scummvmOSX_Debug, "INFOPLIST_FILE", "$(SRCROOT)/dists/macosx/Info.plist");
	ValueList scummvmOSX_LibPaths;
	scummvmOSX_LibPaths.push_back("/sw/lib");
	scummvmOSX_LibPaths.push_back("/opt/local/lib");
	scummvmOSX_LibPaths.push_back("\"$(inherited)\"");
	scummvmOSX_LibPaths.push_back("\"\\\\\\\"$(SRCROOT)/lib\\\\\\\"\"");  // mmmh, all those slashes, it's almost Christmas \o/
	ADD_SETTING_LIST(scummvmOSX_Debug, "LIBRARY_SEARCH_PATHS", scummvmOSX_LibPaths, SettingsNoQuote|SettingsAsList, 5);
	ADD_SETTING_QUOTE(scummvmOSX_Debug, "OTHER_CFLAGS", "");
	ValueList scummvmOSX_LdFlags;
	scummvmOSX_LdFlags.push_back("-lSDLmain");
	scummvmOSX_LdFlags.push_back("-logg");
	scummvmOSX_LdFlags.push_back("-lpng");
	scummvmOSX_LdFlags.push_back("-ljpeg");
	scummvmOSX_LdFlags.push_back("-ltheora");
	scummvmOSX_LdFlags.push_back("-lfreetype");
	scummvmOSX_LdFlags.push_back("-lvorbisfile");
	scummvmOSX_LdFlags.push_back("-lvorbis");
	scummvmOSX_LdFlags.push_back("-lmad");
	scummvmOSX_LdFlags.push_back("-lFLAC");
	scummvmOSX_LdFlags.push_back("-lSDL");
	scummvmOSX_LdFlags.push_back("-lz");
	ADD_SETTING_LIST(scummvmOSX_Debug, "OTHER_LDFLAGS", scummvmOSX_LdFlags, SettingsAsList, 5);
	ADD_SETTING(scummvmOSX_Debug, "PREBINDING", "NO");
	ADD_SETTING(scummvmOSX_Debug, "PRODUCT_NAME", PROJECT_DESCRIPTION);

	scummvmOSX_Debug_Object->addProperty("name", "Debug", "", SettingsNoValue);
	scummvmOSX_Debug_Object->properties["buildSettings"] = scummvmOSX_Debug;

	// Release
	Object *scummvmOSX_Release_Object = new Object(this, "XCBuildConfiguration_" PROJECT_DESCRIPTION "-OSX_Release", _targets[OSX_TARGET] /* ScummVM-OS X */, "XCBuildConfiguration", "PBXNativeTarget", "Release");
	Property scummvmOSX_Release(scummvmOSX_Debug);
	ADD_SETTING(scummvmOSX_Release, "COPY_PHASE_STRIP", "YES");
	REMOVE_SETTING(scummvmOSX_Release, "GCC_DYNAMIC_NO_PIC");
	REMOVE_SETTING(scummvmOSX_Release, "GCC_OPTIMIZATION_LEVEL");
	ADD_SETTING(scummvmOSX_Release, "WRAPPER_EXTENSION", "app");

	scummvmOSX_Release_Object->addProperty("name", "Release", "", SettingsNoValue);
	scummvmOSX_Release_Object->properties["buildSettings"] = scummvmOSX_Release;

	_buildConfiguration.add(scummvmOSX_Debug_Object);
	_buildConfiguration.add(scummvmOSX_Release_Object);
#ifdef ENABLE_IOS
	/****************************************
	 * ScummVM-Simulator
	 ****************************************/

	// Debug
	Object *scummvmSimulator_Debug_Object = new Object(this, "XCBuildConfiguration_" PROJECT_DESCRIPTION "-Simulator_Debug", _targets[SIM_TARGET] /* ScummVM-Simulator */, "XCBuildConfiguration", "PBXNativeTarget", "Debug");
	Property scummvmSimulator_Debug(iPhone_Debug);
	ADD_SETTING_QUOTE(scummvmSimulator_Debug, "FRAMEWORK_SEARCH_PATHS", "$(inherited)");
	ADD_SETTING_LIST(scummvmSimulator_Debug, "GCC_PREPROCESSOR_DEFINITIONS", scummvm_defines, SettingsNoQuote|SettingsAsList, 5);
	ADD_SETTING(scummvmSimulator_Debug, "SDKROOT", "iphonesimulator3.2");
	ADD_SETTING_QUOTE(scummvmSimulator_Debug, "VALID_ARCHS", "i386 x86_64");
	REMOVE_SETTING(scummvmSimulator_Debug, "TARGETED_DEVICE_FAMILY");

	scummvmSimulator_Debug_Object->addProperty("name", "Debug", "", SettingsNoValue);
	scummvmSimulator_Debug_Object->properties["buildSettings"] = scummvmSimulator_Debug;

	// Release
	Object *scummvmSimulator_Release_Object = new Object(this, "XCBuildConfiguration_" PROJECT_DESCRIPTION "-Simulator_Release", _targets[SIM_TARGET] /* ScummVM-Simulator */, "XCBuildConfiguration", "PBXNativeTarget", "Release");
	Property scummvmSimulator_Release(scummvmSimulator_Debug);
	ADD_SETTING(scummvmSimulator_Release, "COPY_PHASE_STRIP", "YES");
	ADD_SETTING(scummvmSimulator_Release, "GCC_OPTIMIZATION_LEVEL", "3");
	REMOVE_SETTING(scummvmSimulator_Release, "GCC_DYNAMIC_NO_PIC");
	ADD_SETTING(scummvmSimulator_Release, "WRAPPER_EXTENSION", "app");

	scummvmSimulator_Release_Object->addProperty("name", "Release", "", SettingsNoValue);
	scummvmSimulator_Release_Object->properties["buildSettings"] = scummvmSimulator_Release;

	_buildConfiguration.add(scummvmSimulator_Debug_Object);
	_buildConfiguration.add(scummvmSimulator_Release_Object);

	//////////////////////////////////////////////////////////////////////////
	// Configuration List
	_configurationList.comment = "XCConfigurationList";
	_configurationList.flags = SettingsAsList;
#endif
	// Warning: This assumes we have all configurations with a Debug & Release pair
	for (std::vector<Object *>::iterator config = _buildConfiguration.objects.begin(); config != _buildConfiguration.objects.end(); config++) {

		Object *configList = new Object(this, "XCConfigurationList_" + (*config)->name, (*config)->name, "XCConfigurationList", "", "Build configuration list for " +  (*config)->refType + " \"" + (*config)->name + "\"");

		Property buildConfigs;
		buildConfigs.flags = SettingsAsList;

		buildConfigs.settings[getHash((*config)->id)] = Setting("", "Debug", SettingsNoValue, 0, 0);
		buildConfigs.settings[getHash((*(++config))->id)] = Setting("", "Release", SettingsNoValue, 0, 1);

		configList->properties["buildConfigurations"] = buildConfigs;

		configList->addProperty("defaultConfigurationIsVisible", "0", "", SettingsNoValue);
		configList->addProperty("defaultConfigurationName", "Release", "", SettingsNoValue);

		_configurationList.add(configList);
	}
}

//////////////////////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////////////////////

// Setup global defines
void XcodeProvider::setupDefines(const BuildSetup &setup) {

	for (StringList::const_iterator i = setup.defines.begin(); i != setup.defines.end(); ++i) {
		if (*i == "HAVE_NASM")	// Not supported on Mac (TODO: change how it's handled in main class or add it only in MSVC/CodeBlocks providers?)
			continue;

		ADD_DEFINE(_defines, *i);
	}
	// Add special defines for Mac support
	ADD_DEFINE(_defines, "CONFIG_H");
	ADD_DEFINE(_defines, "SCUMM_NEED_ALIGNMENT");
	ADD_DEFINE(_defines, "SCUMM_LITTLE_ENDIAN");
	ADD_DEFINE(_defines, "UNIX");
	ADD_DEFINE(_defines, "SCUMMVM");
}

//////////////////////////////////////////////////////////////////////////
// Object hash
//////////////////////////////////////////////////////////////////////////

// TODO use md5 to compute a file hash (and fall back to standard key generation if not passed a file)
std::string XcodeProvider::getHash(std::string key) {

#if DEBUG_XCODE_HASH
	return key;
#else
	// Check to see if the key is already in the dictionary
	std::map<std::string, std::string>::iterator hashIterator = _hashDictionnary.find(key);
	if (hashIterator != _hashDictionnary.end())
		return hashIterator->second;

	// Generate a new key from the file hash and insert it into the dictionary
	std::string hash = newHash();
	_hashDictionnary[key] = hash;

	return hash;
#endif
}

bool isSeparator (char s) { return (s == '-'); }

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

	if (findLen == 0 )
		return input;

	for (;(pos = input.find(find, pos)) != std::string::npos;) {
		input.replace(pos, findLen, replaceStr);
		pos += replaceLen;
	}

	return input;
}

std::string XcodeProvider::writeProperty(const std::string &variable, Property &prop, int flags) const {
	std::string output;

	output += (flags & SettingsSingleItem ? "" : "\t\t\t") + variable + " = ";

	if (prop.settings.size() > 1 || (prop.flags & SettingsSingleItem))
		output += (prop.flags & SettingsAsList) ? "(\n" : "{\n";

	OrderedSettingList settings = prop.getOrderedSettingList();
	for (OrderedSettingList::const_iterator setting = settings.begin(); setting != settings.end(); ++setting) {
		if (settings.size() > 1 || (prop.flags & SettingsSingleItem))
			output += (flags & SettingsSingleItem ? " " : "\t\t\t\t");

		output += writeSetting((*setting).first, (*setting).second);

		// The combination of SettingsAsList, and SettingsSingleItem should use "," and not ";" (i.e children
		// in PBXGroup, so we special case that case here.
		if ((prop.flags & SettingsAsList) && (prop.settings.size() > 1 || (prop.flags & SettingsSingleItem))) {
			output += (prop.settings.size() > 0) ? ",\n" : "\n";
		} else {
			output += ";";
			output += (flags & SettingsSingleItem ? " " : "\n");
		}
	}

	if (prop.settings.size() > 1 || (prop.flags & SettingsSingleItem))
		output += (prop.flags & SettingsAsList) ? "\t\t\t);\n" : "\t\t\t};\n";

	return output;
}

std::string XcodeProvider::writeSetting(const std::string &variable, std::string value, std::string comment, int flags, int indent) const {
	return writeSetting(variable, Setting(value, comment, flags, indent));
}

// Heavily modified (not in a good way) function, imported from the QMake
// XCode project generator pbuilder_pbx.cpp, writeSettings() (under LGPL 2.1)
std::string XcodeProvider::writeSetting(const std::string &variable, const Setting &setting) const {
	std::string output;
	const std::string quote = (setting.flags & SettingsNoQuote) ? "" : "\"";
	const std::string escape_quote = quote.empty() ? "" : "\\" + quote;
	std::string newline = "\n";

	// Get indent level
	for (int i = 0; i < setting.indent; ++i)
		newline += "\t";

	// Setup variable
	std::string var = (setting.flags & SettingsQuoteVariable) ? "\"" + variable + "\"" : variable;

	// Output a list
	if (setting.flags & SettingsAsList) {

		output += var + ((setting.flags & SettingsNoValue) ? "(" : " = (") + newline;

		for (unsigned int i = 0, count = 0; i < setting.entries.size(); ++i) {

			std::string value = setting.entries.at(i).value;
			if (!value.empty()) {
				if (count++ > 0)
					output += "," + newline;

				output += quote + replace(value, quote, escape_quote) + quote;

				std::string comment = setting.entries.at(i).comment;
				if (!comment.empty())
					output += " /* " + comment + " */";
			}

		}
		// Add closing ")" on new line
		newline.resize(newline.size() - 1);
		output += (setting.flags & SettingsNoValue) ? "\t\t\t)" : "," + newline + ")";
	} else {
		output += var;

		output += (setting.flags & SettingsNoValue) ? "" : " = " + quote;

		for(unsigned int i = 0; i < setting.entries.size(); ++i) {
			std::string value = setting.entries.at(i).value;
			if(i)
				output += " ";
			output += value;

			std::string comment = setting.entries.at(i).comment;
			if (!comment.empty())
				output += " /* " + comment + " */";
		}

		output += (setting.flags & SettingsNoValue) ? "" : quote;
	}
	return output;
}

} // End of CreateProjectTool namespace
