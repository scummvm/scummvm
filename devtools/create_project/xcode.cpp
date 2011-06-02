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

#include "xcode.h"

#include <fstream>
#include <algorithm>

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#else
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#endif

namespace CreateProjectTool {

#define DEBUG_XCODE_HASH 0

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

#define ADD_BUILD_FILE(id, name, comment) { \
	Object *buildFile = new Object(this, id, name, "PBXBuildFile", "PBXBuildFile", comment); \
	buildFile->addProperty("fileRef", getHash(name), name, SettingsNoValue); \
	_buildFile.add(buildFile); \
	_buildFile.flags = SettingsSingleItem; \
}

#define ADD_FILE_REFERENCE(name, properties) { \
	Object *fileRef = new Object(this, name, name, "PBXFileReference", "PBXFileReference", name); \
	if (!properties.fileEncoding.empty()) fileRef->addProperty("fileEncoding", properties.fileEncoding, "", SettingsNoValue); \
	if (!properties.lastKnownFileType.empty()) fileRef->addProperty("lastKnownFileType", properties.lastKnownFileType, "", SettingsNoValue); \
	if (!properties.fileName.empty()) fileRef->addProperty("name", properties.fileName, "", SettingsNoValue); \
	if (!properties.filePath.empty()) fileRef->addProperty("path", properties.filePath, "", SettingsNoValue); \
	if (!properties.sourceTree.empty()) fileRef->addProperty("sourceTree", properties.sourceTree, "", SettingsNoValue); \
	_fileReference.add(fileRef); \
	_fileReference.flags = SettingsSingleItem; \
}

XCodeProvider::XCodeProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version)
	: ProjectProvider(global_warnings, project_warnings, version) {
}

void XCodeProvider::createWorkspace(const BuildSetup &setup) {
	// Create project folder
	std::string workspace = setup.outputDir + '/' + "scummvm.xcodeproj";

#if defined(_WIN32) || defined(WIN32)
	if (!CreateDirectory(workspace.c_str(), NULL))
		if (GetLastError() != ERROR_ALREADY_EXISTS)
			error("Could not create folder \"" + setup.outputDir + '/' + "scummvm.xcodeproj\"");
#else
	if (mkdir(workspace.c_str(), 0777) == -1) {
		if (errno == EEXIST) {
			// Try to open as a folder (might be a file / symbolic link)
			DIR *dirp = opendir(workspace.c_str());
			if (dirp == NULL) {
				error("Could not create folder \"" + setup.outputDir + '/' + "scummvm.xcodeproj\"");
			} else {
				// The folder exists, just close the stream and return
				closedir(dirp);
			}
		} else {
			error("Could not create folder \"" + setup.outputDir + '/' + "scummvm.xcodeproj\"");
		}
	}
#endif

	// Setup global objects
	setupDefines(setup);
	_targets.push_back("ScummVM-iPhone");
	_targets.push_back("ScummVM-OS X");
	_targets.push_back("ScummVM-Simulator");

	setupCopyFilesBuildPhase();
	setupFrameworksBuildPhase();
	setupNativeTarget();
	setupProject();
	setupResourcesBuildPhase();
	setupBuildConfiguration();
}

// We are done with constructing all the object graph and we got through every project, output the main project file
// (this is kind of a hack since other providers use separate project files)
void XCodeProvider::createOtherBuildFiles(const BuildSetup &setup) {
	// This needs to be done at the end when all build files have been accounted for
	setupSourcesBuildPhase();

	ouputMainProjectFile(setup);
}

// Store information about a project here, for use at the end
void XCodeProvider::createProjectFile(const std::string &, const std::string &, const BuildSetup &setup, const std::string &moduleDir,
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
void XCodeProvider::ouputMainProjectFile(const BuildSetup &setup) {
	std::ofstream project((setup.outputDir + '/' + "scummvm.xcodeproj" + '/' + "project.pbxproj").c_str());
	if (!project)
		error("Could not open \"" + setup.outputDir + '/' + "scummvm.xcodeproj" + '/' + "project.pbxproj\" for writing");

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
void XCodeProvider::writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int indentation,
                                           const StringList &duplicate, const std::string &objPrefix, const std::string &filePrefix) {

	// Init root group
	_groups.comment = "PBXGroup";
	Object *group = new Object(this, "PBXGroup", "PBXGroup", "PBXGroup", "", "");

	//Property children;
	//children.flags = SettingsAsList;
	//group->properties["children"] = children;
	group->addProperty("children", "", "", SettingsNoValue|SettingsAsList);

	group->addProperty("sourceTree", "<group>", "", SettingsNoValue|SettingsQuoteVariable);

	_groups.add(group);

	// TODO Add files
}

//////////////////////////////////////////////////////////////////////////
// Setup functions
//////////////////////////////////////////////////////////////////////////
void XCodeProvider::setupCopyFilesBuildPhase() {
	// Nothing to do here
}

/**
 * Sets up the frameworks build phase.
 *
 * (each native target has different build rules)
 */
void XCodeProvider::setupFrameworksBuildPhase() {
	_frameworksBuildPhase.comment = "PBXFrameworksBuildPhase";

	// Setup framework file properties
	std::map<std::string, FileProperty> properties;

	// Frameworks
	properties["ApplicationServices.framework"] = FileProperty("wrapper.framework", "ApplicationServices.framework", "System/Library/Frameworks/ApplicationServices.framework", "SDKROOT");
	properties["AudioToolbox.framework"]        = FileProperty("wrapper.framework", "AudioToolbox.framework", "System/Library/Frameworks/AudioToolbox.framework", "SDKROOT");
	properties["AudioUnit.framework"]           = FileProperty("wrapper.framework", "AudioUnit.framework", "System/Library/Frameworks/AudioUnit.framework", "SDKROOT");
	properties["Carbon.framework"]              = FileProperty("wrapper.framework", "Carbon.framework", "System/Library/Frameworks/Carbon.framework", "SDKROOT");
	properties["Cocoa.framework"]               = FileProperty("wrapper.framework", "Cocoa.framework", "System/Library/Frameworks/Cocoa.framework", "SDKROOT");
	properties["CoreAudio.framework"]           = FileProperty("wrapper.framework", "CoreAudio.framework", "System/Library/Frameworks/CoreAudio.framework", "SDKROOT");
	properties["CoreFoundation.framework"]      = FileProperty("wrapper.framework", "CoreFoundation.framework", "System/Library/Frameworks/CoreFoundation.framework", "SDKROOT");
	properties["CoreMIDI.framework"]            = FileProperty("wrapper.framework", "CoreMIDI.framework", "System/Library/Frameworks/CoreMIDI.framework", "SDKROOT");
	properties["Foundation.framework"]          = FileProperty("wrapper.framework", "Foundation.framework", "System/Library/Frameworks/Foundation.framework", "SDKROOT");
	properties["IOKit.framework"]               = FileProperty("wrapper.framework", "IOKit.framework", "System/Library/Frameworks/IOKit.framework", "SDKROOT");
	properties["OpenGLES.framework"]            = FileProperty("wrapper.framework", "OpenGLES.framework", "System/Library/Frameworks/OpenGLES.framework", "SDKROOT");
	properties["QuartzCore.framework"]          = FileProperty("wrapper.framework", "QuartzCore.framework", "System/Library/Frameworks/QuartzCore.framework", "SDKROOT");
	properties["QuickTime.framework"]           = FileProperty("wrapper.framework", "QuickTime.framework", "System/Library/Frameworks/QuickTime.framework", "SDKROOT");
	properties["UIKit.framework"]               = FileProperty("wrapper.framework", "UIKit.framework", "System/Library/Frameworks/UIKit.framework", "SDKROOT");

	// Local libraries
	properties["libFLAC.a"]                     = FileProperty("archive.ar", "libFLAC.a", "lib/libFLAC.a", "\"<group>\"");
	properties["libmad.a"]                      = FileProperty("archive.ar", "libmad.a", "lib/libmad.a", "\"<group>\"");
	//properties["libmpeg2.a"]                    = FileProperty("archive.ar", "libmpeg2.a", "lib/libmpeg2.a", "\"<group>\"");
	properties["libvorbisidec.a"]               = FileProperty("archive.ar", "libvorbisidec.a", "lib/libvorbisidec.a", "\"<group>\"");

	//////////////////////////////////////////////////////////////////////////
	// iPhone
	Object *framework_iPhone = new Object(this, "PBXFrameworksBuildPhase_" + _targets[0], "PBXFrameworksBuildPhase", "PBXFrameworksBuildPhase", "", "Frameworks");

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

	int order = 0;
	for (ValueList::iterator framework = frameworks_iPhone.begin(); framework != frameworks_iPhone.end(); framework++) {
		std::string id = "Frameworks_" + *framework + "_iphone";
		std::string comment = *framework + " in Frameworks";

		ADD_SETTING_ORDER_NOVALUE(iPhone_files, getHash(id), comment, order++);
		ADD_BUILD_FILE(id, *framework, comment);
		ADD_FILE_REFERENCE(*framework, properties[*framework]);
	}

	framework_iPhone->properties["files"] = iPhone_files;

	_frameworksBuildPhase.add(framework_iPhone);

	//////////////////////////////////////////////////////////////////////////
	// ScummVM-OS X
	Object *framework_OSX = new Object(this, "PBXFrameworksBuildPhase_" + _targets[1], "PBXFrameworksBuildPhase", "PBXFrameworksBuildPhase", "", "Frameworks");

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

	order = 0;
	for (ValueList::iterator framework = frameworks_osx.begin(); framework != frameworks_osx.end(); framework++) {
		std::string id = "Frameworks_" + *framework + "_osx";
		std::string comment = *framework + " in Frameworks";

		ADD_SETTING_ORDER_NOVALUE(osx_files, getHash(id), comment, order++);
		ADD_BUILD_FILE(id, *framework, comment);
		ADD_FILE_REFERENCE(*framework, properties[*framework]);
	}

	framework_OSX->properties["files"] = osx_files;

	_frameworksBuildPhase.add(framework_OSX);

	//////////////////////////////////////////////////////////////////////////
	// Simulator
	Object *framework_simulator = new Object(this, "PBXFrameworksBuildPhase_" + _targets[2], "PBXFrameworksBuildPhase", "PBXFrameworksBuildPhase", "", "Frameworks");

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
		ADD_BUILD_FILE(id, *framework, comment);
		ADD_FILE_REFERENCE(*framework, properties[*framework]);
	}

	framework_simulator->properties["files"] = simulator_files;

	_frameworksBuildPhase.add(framework_simulator);
}

void XCodeProvider::setupNativeTarget() {
	_nativeTarget.comment = "PBXNativeTarget";

	// Output native target section
	for (unsigned int i = 0; i < _targets.size(); i++) {
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
		target->addProperty("productName", "scummvm", "", SettingsNoValue);
		target->addProperty("productReference", getHash("PBXFileReference_ScummVM.app_" + _targets[i]), "ScummVM.app", SettingsNoValue);
		target->addProperty("productType", "com.apple.product-type.application", "", SettingsNoValue|SettingsQuoteVariable);

		_nativeTarget.add(target);
	}
}

void XCodeProvider::setupProject() {
	_project.comment = "PBXProject";

	Object *project = new Object(this, "PBXProject", "PBXProject", "PBXProject", "", "Project object");

	project->addProperty("buildConfigurationList", getHash("XCConfigurationList_scummvm"), "Build configuration list for PBXProject \"scummvm\"", SettingsNoValue);
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

	project->addProperty("mainGroup", getHash("PBXGroup_CustomTemplate"), "CustomTemplate", SettingsNoValue);
	project->addProperty("projectDirPath", "", "", SettingsNoValue|SettingsQuoteVariable);
	project->addProperty("projectRoot", "", "", SettingsNoValue|SettingsQuoteVariable);

	// List of targets
	Property targets;
	targets.flags = SettingsAsList;
	targets.settings[getHash("PBXNativeTarget_" + _targets[0])] = Setting("", _targets[0], SettingsNoValue, 0, 0);
	targets.settings[getHash("PBXNativeTarget_" + _targets[1])] = Setting("", _targets[1], SettingsNoValue, 0, 1);
	targets.settings[getHash("PBXNativeTarget_" + _targets[2])] = Setting("", _targets[2], SettingsNoValue, 0, 2);
	project->properties["targets"] = targets;

	_project.add(project);
}

void XCodeProvider::setupResourcesBuildPhase() {
	// TODO
}

void XCodeProvider::setupSourcesBuildPhase() {
	// TODO
}

// Setup all build configurations
void XCodeProvider::setupBuildConfiguration() {

	_buildConfiguration.comment = "XCBuildConfiguration";
	_buildConfiguration.flags = SettingsAsList;

	///****************************************
	// * iPhone
	// ****************************************/

	//// Debug
	Object *iPhone_Debug_Object = new Object(this, "XCBuildConfiguration_ScummVM-iPhone_Debug", _targets[0] /* ScummVM-iPhone */, "XCBuildConfiguration", "PBXNativeTarget", "Debug");
	Property iPhone_Debug;
	iPhone_Debug.flags = SettingsSingleItem;
	// TODO Add settings

	iPhone_Debug_Object->addProperty("name", "Debug", "", SettingsNoValue);
	iPhone_Debug_Object->properties["buildSettings"] = iPhone_Debug;

	//// Release
	Object *iPhone_Release_Object = new Object(this, "XCBuildConfiguration_ScummVM-iPhone_Release", _targets[0] /* ScummVM-iPhone */, "XCBuildConfiguration", "PBXNativeTarget", "Release");
	Property iPhone_Release(iPhone_Debug);
	// TODO Add settings

	iPhone_Release_Object->addProperty("name", "Release", "", SettingsNoValue);
	iPhone_Release_Object->properties["buildSettings"] = iPhone_Release;

	_buildConfiguration.add(iPhone_Debug_Object);
	_buildConfiguration.add(iPhone_Release_Object);

	/****************************************
	 * scummvm
	 ****************************************/

	// Debug
	Object *scummvm_Debug_Object = new Object(this, "XCBuildConfiguration_scummvm_Debug", "scummvm", "XCBuildConfiguration", "PBXProject", "Debug");
	Property scummvm_Debug;
	scummvm_Debug.flags = SettingsSingleItem;
	// TODO Add settings

	scummvm_Debug_Object->addProperty("name", "Debug", "", SettingsNoValue);
	scummvm_Debug_Object->properties["buildSettings"] = scummvm_Debug;

	// Release
	Object *scummvm_Release_Object = new Object(this, "XCBuildConfiguration_scummvm_Release", "scummvm", "XCBuildConfiguration", "PBXProject", "Release");
	Property scummvm_Release(scummvm_Debug);
	// TODO Add settings

	scummvm_Release_Object->addProperty("name", "Release", "", SettingsNoValue);
	scummvm_Release_Object->properties["buildSettings"] = scummvm_Release;

	_buildConfiguration.add(scummvm_Debug_Object);
	_buildConfiguration.add(scummvm_Release_Object);

	/****************************************
	 * ScummVM-OS X
	 ****************************************/

	// Debug
	Object *scummvmOSX_Debug_Object = new Object(this, "XCBuildConfiguration_ScummVM-OSX_Debug", _targets[1] /* ScummVM-OS X */, "XCBuildConfiguration", "PBXNativeTarget", "Debug");
	Property scummvmOSX_Debug;
	scummvmOSX_Debug.flags = SettingsSingleItem;
	// TODO Add settings

	scummvmOSX_Debug_Object->addProperty("name", "Debug", "", SettingsNoValue);
	scummvmOSX_Debug_Object->properties["buildSettings"] = scummvmOSX_Debug;

	// Release
	Object *scummvmOSX_Release_Object = new Object(this, "XCBuildConfiguration_ScummVMOSX_Release", _targets[1] /* ScummVM-OS X */, "XCBuildConfiguration", "PBXNativeTarget", "Release");
	Property scummvmOSX_Release(scummvmOSX_Debug);
	// TODO Add settings

	scummvmOSX_Release_Object->addProperty("name", "Release", "", SettingsNoValue);
	scummvmOSX_Release_Object->properties["buildSettings"] = scummvmOSX_Release;

	_buildConfiguration.add(scummvmOSX_Debug_Object);
	_buildConfiguration.add(scummvmOSX_Release_Object);

	/****************************************
	 * ScummVM-Simulator
	 ****************************************/

	// Debug
	Object *scummvmSimulator_Debug_Object = new Object(this, "XCBuildConfiguration_ScummVM-Simulator_Debug", _targets[2] /* ScummVM-Simulator */, "XCBuildConfiguration", "PBXNativeTarget", "Debug");
	Property scummvmSimulator_Debug(iPhone_Debug);
	// TODO Add settings

	scummvmSimulator_Debug_Object->addProperty("name", "Debug", "", SettingsNoValue);
	scummvmSimulator_Debug_Object->properties["buildSettings"] = scummvmSimulator_Debug;

	// Release
	Object *scummvmSimulator_Release_Object = new Object(this, "XCBuildConfiguration_ScummVM-Simulator_Release", _targets[2] /* ScummVM-Simulator */, "XCBuildConfiguration", "PBXNativeTarget", "Release");
	Property scummvmSimulator_Release(scummvmSimulator_Debug);
	/// TODO Add settings

	scummvmSimulator_Release_Object->addProperty("name", "Release", "", SettingsNoValue);
	scummvmSimulator_Release_Object->properties["buildSettings"] = scummvmSimulator_Release;

	_buildConfiguration.add(scummvmSimulator_Debug_Object);
	_buildConfiguration.add(scummvmSimulator_Release_Object);

	////////////////////////////////////////////////////////////////////////////
	//// Configuration List
	_configurationList.comment = "XCConfigurationList";
	_configurationList.flags = SettingsAsList;

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
void XCodeProvider::setupDefines(const BuildSetup &setup) {

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
	ADD_DEFINE(_defines, "USE_TREMOR");
}

//////////////////////////////////////////////////////////////////////////
// Object hash
//////////////////////////////////////////////////////////////////////////

// TODO use md5 to compute a file hash (and fall back to standard key generation if not passed a file)
std::string XCodeProvider::getHash(std::string key) {

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

std::string XCodeProvider::newHash() const {
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

std::string XCodeProvider::writeProperty(const std::string &variable, Property &prop, int flags) const {
	std::string output;

	output += (flags & SettingsSingleItem ? "" : "\t\t\t") + variable + " = ";

	if (prop.settings.size() > 1 || (prop.flags & SettingsSingleItem))
		output += (prop.flags & SettingsAsList) ? "(\n" : "{\n";

	OrderedSettingList settings = prop.getOrderedSettingList();
	for (OrderedSettingList::const_iterator setting = settings.begin(); setting != settings.end(); ++setting) {
		if (settings.size() > 1 || (prop.flags & SettingsSingleItem))
			output += (flags & SettingsSingleItem ? " " : "\t\t\t\t");

		output += writeSetting((*setting).first, (*setting).second);

		if ((prop.flags & SettingsAsList) && prop.settings.size() > 1) {
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

std::string XCodeProvider::writeSetting(const std::string &variable, std::string value, std::string comment, int flags, int indent) const {
	return writeSetting(variable, Setting(value, comment, flags, indent));
}
// Heavily modified (not in a good way) function, imported from QMake XCode project generator (licensed under the QT license)
std::string XCodeProvider::writeSetting(const std::string &variable, const Setting &setting) const {
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
			if(!value.empty()) {
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
