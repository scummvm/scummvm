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
#include "msvc.h"

#include <fstream>
#include <algorithm>
#include <cstring>


std::map<MSVC_Architecture, StringList> s_arch_disabled_features{
	// NASM not supported for Windows on AMD64 target
	{ MSVC_Architecture::ARCH_AMD64, { "nasm" } },

	// NASM not supported for WoA target
	// No OpenGL, OpenGL ES on Windows on ARM
	// https://github.com/microsoft/vcpkg/issues/11248 [fribidi] Fribidi doesn't cross-compile on x86-64 to target arm/arm64
	{ MSVC_Architecture::ARCH_ARM64, { "nasm", "opengl", "opengles", "fribidi" } },
};

namespace CreateProjectTool {

//////////////////////////////////////////////////////////////////////////
// MSVC Provider (Base class)
//////////////////////////////////////////////////////////////////////////
MSVCProvider::MSVCProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version, const MSVCVersion &msvc)
	: ProjectProvider(global_warnings, project_warnings, version), _msvcVersion(msvc) {

	_enableLanguageExtensions = tokenize(ENABLE_LANGUAGE_EXTENSIONS, ',');
	_disableEditAndContinue   = tokenize(DISABLE_EDIT_AND_CONTINUE, ',');
}

void MSVCProvider::createWorkspace(const BuildSetup &setup) {
	UUIDMap::const_iterator svmUUID = _uuidMap.find(setup.projectName);
	if (svmUUID == _uuidMap.end())
		error("No UUID for \"" + setup.projectName + "\" project created");

	const std::string svmProjectUUID = svmUUID->second;
	assert(!svmProjectUUID.empty());

	std::string solutionUUID = createUUID(setup.projectName + ".sln");

	std::ofstream solution((setup.outputDir + '/' + setup.projectName + ".sln").c_str());
	if (!solution)
		error("Could not open \"" + setup.outputDir + '/' + setup.projectName + ".sln\" for writing");

	solution << "Microsoft Visual Studio Solution File, Format Version " << _msvcVersion.solutionFormat << "\n";
	solution << "# Visual Studio " << _msvcVersion.solutionVersion << "\n";

	// Write main project
	if (!setup.devTools) {
		solution << "Project(\"{" << solutionUUID << "}\") = \"" << setup.projectName << "\", \"" << setup.projectName << getProjectExtension() << "\", \"{" << svmProjectUUID << "}\"\n";

		// Project dependencies are moved to vcxproj files in Visual Studio 2010
		if (_version < 10)
			writeReferences(setup, solution);

		solution << "EndProject\n";
	}

	// Note we assume that the UUID map only includes UUIDs for enabled engines!
	for (UUIDMap::const_iterator i = _uuidMap.begin(); i != _uuidMap.end(); ++i) {
		if (i->first == setup.projectName)
			continue;

		solution << "Project(\"{" << solutionUUID << "}\") = \"" << i->first << "\", \"" << i->first << getProjectExtension() << "\", \"{" << i->second << "}\"\n"
		         << "EndProject\n";
	}

	solution << "Global\n"
	            "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n"
	            "\t\tDebug|Win32 = Debug|Win32\n"
	            "\t\tAnalysis|Win32 = Analysis|Win32\n"
	            "\t\tLLVM|Win32 = LLVM|Win32\n"
	            "\t\tRelease|Win32 = Release|Win32\n"
	            "\t\tDebug|x64 = Debug|x64\n"
	            "\t\tAnalysis|x64 = Analysis|x64\n"
	            "\t\tLLVM|x64 = LLVM|x64\n"
	            "\t\tRelease|x64 = Release|x64\n"
	            "\t\tDebug|arm64 = Debug|arm64\n"
	            "\t\tAnalysis|arm64 = Analysis|arm64\n"
	            "\t\tLLVM|arm64 = LLVM|arm64\n"
	            "\t\tRelease|arm64 = Release|arm64\n"
	            "\tEndGlobalSection\n"
	            "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n";

	for (UUIDMap::const_iterator i = _uuidMap.begin(); i != _uuidMap.end(); ++i) {
		solution << "\t\t{" << i->second << "}.Debug|Win32.ActiveCfg = Debug|Win32\n"
		            "\t\t{" << i->second << "}.Debug|Win32.Build.0 = Debug|Win32\n"
		            "\t\t{" << i->second << "}.Analysis|Win32.ActiveCfg = Analysis|Win32\n"
		            "\t\t{" << i->second << "}.Analysis|Win32.Build.0 = Analysis|Win32\n"
		            "\t\t{" << i->second << "}.LLVM|Win32.ActiveCfg = LLVM|Win32\n"
		            "\t\t{" << i->second << "}.LLVM|Win32.Build.0 = LLVM|Win32\n"
		            "\t\t{" << i->second << "}.Release|Win32.ActiveCfg = Release|Win32\n"
		            "\t\t{" << i->second << "}.Release|Win32.Build.0 = Release|Win32\n"
		            "\t\t{" << i->second << "}.Debug|x64.ActiveCfg = Debug|x64\n"
		            "\t\t{" << i->second << "}.Debug|x64.Build.0 = Debug|x64\n"
		            "\t\t{" << i->second << "}.Analysis|x64.ActiveCfg = Analysis|x64\n"
		            "\t\t{" << i->second << "}.Analysis|x64.Build.0 = Analysis|x64\n"
		            "\t\t{" << i->second << "}.LLVM|x64.ActiveCfg = LLVM|x64\n"
		            "\t\t{" << i->second << "}.LLVM|x64.Build.0 = LLVM|x64\n"
		            "\t\t{" << i->second << "}.Release|x64.ActiveCfg = Release|x64\n"
		            "\t\t{" << i->second << "}.Release|x64.Build.0 = Release|x64\n"
		            "\t\t{" << i->second << "}.Debug|arm64.ActiveCfg = Debug|arm64\n"
		            "\t\t{" << i->second << "}.Debug|arm64.Build.0 = Debug|arm64\n"
		            "\t\t{" << i->second << "}.Analysis|arm64.ActiveCfg = Analysis|arm64\n"
		            "\t\t{" << i->second << "}.Analysis|arm64.Build.0 = Analysis|arm64\n"
		            "\t\t{" << i->second << "}.LLVM|arm64.ActiveCfg = LLVM|arm64\n"
		            "\t\t{" << i->second << "}.LLVM|arm64.Build.0 = LLVM|arm64\n"
		            "\t\t{" << i->second << "}.Release|arm64.ActiveCfg = Release|arm64\n"
		            "\t\t{" << i->second << "}.Release|arm64.Build.0 = Release|arm64\n";
	}

	solution << "\tEndGlobalSection\n"
	            "\tGlobalSection(SolutionProperties) = preSolution\n"
	            "\t\tHideSolutionNode = FALSE\n"
	            "\tEndGlobalSection\n"
	            "EndGlobal\n";
}

void MSVCProvider::createOtherBuildFiles(const BuildSetup &setup) {
	// Create the global property file
	createGlobalProp(setup);

	// Create the configuration property files (for Debug and Release with 32 and 64bits versions)
	// Note: we use the debug properties for the analysis configuration
	createBuildProp(setup, true, MSVC_Architecture::ARCH_AMD64, "Release");
	createBuildProp(setup, true, MSVC_Architecture::ARCH_X86, "Release");
	createBuildProp(setup, true, MSVC_Architecture::ARCH_ARM64, "Release");
	createBuildProp(setup, false, MSVC_Architecture::ARCH_AMD64, "Debug");
	createBuildProp(setup, false, MSVC_Architecture::ARCH_X86, "Debug");
	createBuildProp(setup, false, MSVC_Architecture::ARCH_ARM64, "Debug");
	createBuildProp(setup, false, MSVC_Architecture::ARCH_AMD64, "Analysis");
	createBuildProp(setup, false, MSVC_Architecture::ARCH_X86, "Analysis");
	createBuildProp(setup, false, MSVC_Architecture::ARCH_ARM64, "Analysis");
	createBuildProp(setup, false, MSVC_Architecture::ARCH_AMD64, "LLVM");
	createBuildProp(setup, false, MSVC_Architecture::ARCH_X86, "LLVM");
	createBuildProp(setup, false, MSVC_Architecture::ARCH_ARM64, "LLVM");
}

void MSVCProvider::addResourceFiles(const BuildSetup &setup, StringList &includeList, StringList &excludeList) {
	includeList.push_back(setup.srcDir + "/icons/" + setup.projectName + ".ico");
	includeList.push_back(setup.srcDir + "/dists/" + setup.projectName + ".rc");
}

void MSVCProvider::createGlobalProp(const BuildSetup &setup) {
	std::ofstream properties((setup.outputDir + '/' + setup.projectDescription + "_Global" + getMSVCArchName(MSVC_Architecture::ARCH_X86) + getPropertiesExtension()).c_str());
	if (!properties)
		error("Could not open \"" + setup.outputDir + '/' + setup.projectDescription + "_Global" + getMSVCArchName(MSVC_Architecture::ARCH_X86) + getPropertiesExtension() + "\" for writing");

	outputGlobalPropFile(setup, properties, MSVC_Architecture::ARCH_X86, setup.defines, convertPathToWin(setup.filePrefix), setup.runBuildEvents);
	properties.close();

	properties.open((setup.outputDir + '/' + setup.projectDescription + "_Global" + getMSVCArchName(MSVC_Architecture::ARCH_AMD64) + getPropertiesExtension()).c_str());
	if (!properties)
		error("Could not open \"" + setup.outputDir + '/' + setup.projectDescription + "_Global" + getMSVCArchName(MSVC_Architecture::ARCH_AMD64) + getPropertiesExtension() + "\" for writing");

	BuildSetup amd64setup = setup;
	auto amd64_disabled_features_it = s_arch_disabled_features.find(MSVC_Architecture::ARCH_AMD64);
	if (amd64_disabled_features_it != s_arch_disabled_features.end()) {
		for (auto feature : amd64_disabled_features_it->second) {
			amd64setup = removeFeatureFromSetup(amd64setup, feature);
		}
	}

	outputGlobalPropFile(amd64setup, properties, MSVC_Architecture::ARCH_AMD64, amd64setup.defines, convertPathToWin(amd64setup.filePrefix), amd64setup.runBuildEvents);
	properties.close();

	properties.open((setup.outputDir + '/' + setup.projectDescription + "_Global" + getMSVCArchName(MSVC_Architecture::ARCH_ARM64) + getPropertiesExtension()).c_str());
	if (!properties)
		error("Could not open \"" + setup.outputDir + '/' + setup.projectDescription + "_Global" + getMSVCArchName(MSVC_Architecture::ARCH_ARM64) + getPropertiesExtension() + "\" for writing");

	BuildSetup arm64setup = setup;
	auto arm64_disabled_features_it = s_arch_disabled_features.find(MSVC_Architecture::ARCH_ARM64);
	if (arm64_disabled_features_it != s_arch_disabled_features.end()) {
		for (auto feature : arm64_disabled_features_it->second) {
			arm64setup = removeFeatureFromSetup(arm64setup, feature);
		}
	}
	outputGlobalPropFile(arm64setup, properties, MSVC_Architecture::ARCH_ARM64, arm64setup.defines, convertPathToWin(setup.filePrefix), setup.runBuildEvents);
	properties.close();
}

std::string MSVCProvider::getPreBuildEvent() const {
	std::string cmdLine = "";

	cmdLine = "@echo off\n"
	          "echo Executing Pre-Build script...\n"
	          "echo.\n"
	          "@call &quot;$(SolutionDir)../../devtools/create_project/scripts/prebuild.cmd&quot; &quot;$(SolutionDir)/../..&quot; &quot;$(SolutionDir)&quot;\n"
	          "EXIT /B0";

	return cmdLine;
}

std::string MSVCProvider::getTestPreBuildEvent(const BuildSetup &setup) const {
	// Build list of folders containing tests
	std::string target = "";

	for (StringList::const_iterator it = setup.testDirs.begin(); it != setup.testDirs.end(); ++it)
		target += " $(SolutionDir)" + *it + "*.h";

	return "&quot;$(SolutionDir)../../test/cxxtest/cxxtestgen.py&quot; --runner=ParenPrinter --no-std --no-eh -o &quot;$(SolutionDir)test_runner.cpp&quot;" + target;
}

std::string MSVCProvider::getPostBuildEvent(MSVC_Architecture arch, const BuildSetup &setup) const {
	std::string cmdLine = "";

	cmdLine = "@echo off\n"
	          "echo Executing Post-Build script...\n"
	          "echo.\n"
	          "@call &quot;$(SolutionDir)../../devtools/create_project/scripts/postbuild.cmd&quot; &quot;$(SolutionDir)/../..&quot; &quot;$(OutDir)&quot; ";

	cmdLine += (setup.useSDL2) ? "SDL2" : "SDL";

	cmdLine += " &quot;%" LIBS_DEFINE "%/lib/";
	cmdLine += getMSVCArchName(arch);
	cmdLine += "/$(Configuration)&quot; ";

	// Specify if installer needs to be built or not
	cmdLine += (setup.createInstaller ? "1" : "0");

	cmdLine += "\n"
	           "EXIT /B0";

	return cmdLine;
}

} // End of CreateProjectTool namespace
