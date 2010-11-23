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
 * $URL$
 * $Id$
 *
 */

#include "msvc.h"

#include <fstream>

#include <algorithm>

namespace CreateProjectTool {

//////////////////////////////////////////////////////////////////////////
// MSVC Provider (Base class)
//////////////////////////////////////////////////////////////////////////
MSVCProvider::MSVCProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version)
	: ProjectProvider(global_warnings, project_warnings, version) {
}

void MSVCProvider::createWorkspace(const BuildSetup &setup) {
	UUIDMap::const_iterator svmUUID = _uuidMap.find("scummvm");
	if (svmUUID == _uuidMap.end())
		error("No UUID for \"scummvm\" project created");

	const std::string svmProjectUUID = svmUUID->second;
	assert(!svmProjectUUID.empty());

	std::string solutionUUID = createUUID();

	std::ofstream solution((setup.outputDir + '/' + "scummvm.sln").c_str());
	if (!solution)
		error("Could not open \"" + setup.outputDir + '/' + "scummvm.sln\" for writing");

	solution << "Microsoft Visual Studio Solution File, Format Version " << _version + 1 << ".00\n";
	solution << "# Visual Studio " << getVisualStudioVersion() << "\n";

	solution << "Project(\"{" << solutionUUID << "}\") = \"scummvm\", \"scummvm" << getProjectExtension() << "\", \"{" << svmProjectUUID << "}\"\n";

	// Project dependencies are moved to vcxproj files in Visual Studio 2010
	if (_version < 10)
		writeReferences(solution);

	solution << "EndProject\n";

	// Note we assume that the UUID map only includes UUIDs for enabled engines!
	for (UUIDMap::const_iterator i = _uuidMap.begin(); i != _uuidMap.end(); ++i) {
		if (i->first == "scummvm")
			continue;

		solution << "Project(\"{" << solutionUUID << "}\") = \"" << i->first << "\", \"" << i->first << getProjectExtension() << "\", \"{" << i->second << "}\"\n"
		         << "EndProject\n";
	}

	solution << "Global\n"
	            "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n"
	            "\t\tDebug|Win32 = Debug|Win32\n"
				"\t\tAnalysis|Win32 = Analysis|Win32\n"
	            "\t\tRelease|Win32 = Release|Win32\n"
	            "\t\tDebug|x64 = Debug|x64\n"
				"\t\tAnalysis|x64 = Analysis|x64\n"
	            "\t\tRelease|x64 = Release|x64\n"
	            "\tEndGlobalSection\n"
	            "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n";

	for (UUIDMap::const_iterator i = _uuidMap.begin(); i != _uuidMap.end(); ++i) {
		solution << "\t\t{" << i->second << "}.Debug|Win32.ActiveCfg = Debug|Win32\n"
		            "\t\t{" << i->second << "}.Debug|Win32.Build.0 = Debug|Win32\n"
					"\t\t{" << i->second << "}.Analysis|Win32.ActiveCfg = Analysis|Win32\n"
					"\t\t{" << i->second << "}.Analysis|Win32.Build.0 = Analysis|Win32\n"
		            "\t\t{" << i->second << "}.Release|Win32.ActiveCfg = Release|Win32\n"
		            "\t\t{" << i->second << "}.Release|Win32.Build.0 = Release|Win32\n"
		            "\t\t{" << i->second << "}.Debug|x64.ActiveCfg = Debug|x64\n"
		            "\t\t{" << i->second << "}.Debug|x64.Build.0 = Debug|x64\n"
					"\t\t{" << i->second << "}.Analysis|x64.ActiveCfg = Analysis|x64\n"
					"\t\t{" << i->second << "}.Analysis|x64.Build.0 = Analysis|x64\n"
		            "\t\t{" << i->second << "}.Release|x64.ActiveCfg = Release|x64\n"
		            "\t\t{" << i->second << "}.Release|x64.Build.0 = Release|x64\n";
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
	createBuildProp(setup, true, false, false);
	createBuildProp(setup, true, true, false);
	createBuildProp(setup, false, false, false);
	createBuildProp(setup, false, false, true);
	createBuildProp(setup, false, true, false);
	createBuildProp(setup, false, true, true);
}

void MSVCProvider::createGlobalProp(const BuildSetup &setup) {
	std::ofstream properties((setup.outputDir + '/' + "ScummVM_Global" + getPropertiesExtension()).c_str());
	if (!properties)
		error("Could not open \"" + setup.outputDir + '/' + "ScummVM_Global" + getPropertiesExtension() + "\" for writing");

	outputGlobalPropFile(properties, 32, setup.defines, convertPathToWin(setup.filePrefix));
	properties.close();

	properties.open((setup.outputDir + '/' + "ScummVM_Global64" + getPropertiesExtension()).c_str());
	if (!properties)
		error("Could not open \"" + setup.outputDir + '/' + "ScummVM_Global64" + getPropertiesExtension() + "\" for writing");

	// HACK: We must disable the "nasm" feature for x64. To achieve that we must duplicate the feature list and
	// recreate a define list.
	FeatureList x64Features = setup.features;
	setFeatureBuildState("nasm", x64Features, false);
	StringList x64Defines = getFeatureDefines(x64Features);
	StringList x64EngineDefines = getEngineDefines(setup.engines);
	x64Defines.splice(x64Defines.end(), x64EngineDefines);

	outputGlobalPropFile(properties, 64, x64Defines, convertPathToWin(setup.filePrefix));
}

std::string MSVCProvider::getPreBuildEvent() const {
	std::string cmdLine = "";

	cmdLine = "@echo off\n"
	          "echo Executing Pre-Build script...\n"
			  "echo.\n"
			  "@call &quot;$(SolutionDir)../../tools/create_project/scripts/prebuild.cmd&quot; &quot;$(SolutionDir)/../..&quot;\n"
	          "EXIT /B0";

	return cmdLine;
}

std::string MSVCProvider::getPostBuildEvent(bool isWin32) const {
	std::string cmdLine = "";

	cmdLine = "@echo off\n"
	          "echo Executing Post-Build script...\n"
	          "echo.\n"
	          "@call &quot;$(SolutionDir)../../tools/create_project/scripts/postbuild.cmd&quot; &quot;$(SolutionDir)/../..&quot; &quot;$(OutDir)&quot; ";

	cmdLine += (isWin32) ? "x86" : "x64";

	cmdLine += "\n"
	           "EXIT /B0";

	return cmdLine;
}

//////////////////////////////////////////////////////////////////////////
// Visual Studio Provider
//////////////////////////////////////////////////////////////////////////

VisualStudioProvider::VisualStudioProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version)
	: MSVCProvider(global_warnings, project_warnings, version) {
}

const char *VisualStudioProvider::getProjectExtension() {
	return ".vcproj";
}

const char *VisualStudioProvider::getPropertiesExtension() {
	return ".vsprops";
}

int VisualStudioProvider::getVisualStudioVersion() {
	if (_version == 9)
		return 2008;

	if (_version == 8)
		return 2005;

	error("Unsupported version passed to createScummVMSolution");
}

#define OUTPUT_CONFIGURATION_SCUMMVM(config, platform, props) { \
	project << "\t\t<Configuration Name=\"" << config << "|" << platform << "\" ConfigurationType=\"1\" InheritedPropertySheets=\".\\ScummVM_" << config << props << ".vsprops\">\n" \
	           "\t\t\t<Tool\tName=\"VCCLCompilerTool\" DisableLanguageExtensions=\"false\" />\n" \
	           "\t\t\t<Tool\tName=\"VCLinkerTool\" OutputFile=\"$(OutDir)/scummvm.exe\"\n" \
	           "\t\t\t\tAdditionalDependencies=\"" << libraries << "\"\n" \
	           "\t\t\t/>\n" \
	           "\t\t</Configuration>\n"; \
}

#define OUTPUT_CONFIGURATION_SCUMMVM_DEBUG(config, platform, props, isWin32) { \
	project << "\t\t<Configuration Name=\"" << config << "|" << platform << "\" ConfigurationType=\"1\" InheritedPropertySheets=\".\\ScummVM_" << config << props << ".vsprops\">\n" \
	           "\t\t\t<Tool\tName=\"VCCLCompilerTool\" DisableLanguageExtensions=\"false\" />\n" \
	           "\t\t\t<Tool\tName=\"VCLinkerTool\" OutputFile=\"$(OutDir)/scummvm.exe\"\n" \
	           "\t\t\t\tAdditionalDependencies=\"" << libraries << "\"\n" \
	           "\t\t\t/>\n" \
	           "\t\t\t<Tool\tName=\"VCPreBuildEventTool\"\n" \
	           "\t\t\t\tCommandLine=\"" << getPreBuildEvent() << "\"\n" \
	           "\t\t\t/>\n" \
	           "\t\t\t<Tool\tName=\"VCPostBuildEventTool\"\n" \
	           "\t\t\t\tCommandLine=\"" << getPostBuildEvent(isWin32) << "\"\n" \
	           "\t\t\t/>\n" \
	           "\t\t</Configuration>\n"; \
}

#define OUTPUT_CONFIGURATION(config, platform, props) { \
	project << "\t\t<Configuration Name=\"" << config << "|" << platform << "\" ConfigurationType=\"4\" InheritedPropertySheets=\".\\ScummVM_" << config << props << ".vsprops\">\n" \
	           "\t\t\t<Tool Name=\"VCCLCompilerTool\" "<< toolConfig << "/>\n" \
	           "\t\t</Configuration>\n"; \
}

void VisualStudioProvider::createProjectFile(const std::string &name, const std::string &uuid, const BuildSetup &setup, const std::string &moduleDir,
                                             const StringList &includeList, const StringList &excludeList) {
	const std::string projectFile = setup.outputDir + '/' + name + getProjectExtension();
	std::ofstream project(projectFile.c_str());
	if (!project)
		error("Could not open \"" + projectFile + "\" for writing");

	project << "<?xml version=\"1.0\" encoding=\"windows-1252\"?>\n"
	           "<VisualStudioProject\n"
	           "\tProjectType=\"Visual C++\"\n"
	           "\tVersion=\"" << _version << ".00\"\n"
	           "\tName=\"" << name << "\"\n"
	           "\tProjectGUID=\"{" << uuid << "}\"\n"
	           "\tRootNamespace=\"" << name << "\"\n"
	           "\tKeyword=\"Win32Proj\"\n";

	if (_version >= 9)
		project << "\tTargetFrameworkVersion=\"131072\"\n";

	project << "\t>\n"
	           "\t<Platforms>\n"
	           "\t\t<Platform Name=\"Win32\" />\n"
	           "\t\t<Platform Name=\"x64\" />\n"
	           "\t</Platforms>\n"
	           "\t<Configurations>\n";

	// Check for project-specific warnings:
	std::map< std::string, std::list<std::string> >::iterator warningsIterator = _projectWarnings.find(name);

	if (name == "scummvm") {
		std::string libraries;

		for (StringList::const_iterator i = setup.libraries.begin(); i != setup.libraries.end(); ++i)
			libraries += ' ' + *i + ".lib";

		// Win32
		OUTPUT_CONFIGURATION_SCUMMVM_DEBUG("Debug", "Win32", "", true);
		OUTPUT_CONFIGURATION_SCUMMVM_DEBUG("Analysis", "Win32", "", true);
		OUTPUT_CONFIGURATION_SCUMMVM("Release", "Win32", "");

		// x64
		// For 'x64' we must disable NASM support. Usually we would need to disable the "nasm" feature for that and
		// re-create the library list, BUT since NASM doesn't link any additional libraries, we can just use the
		// libraries list created for IA-32. If that changes in the future, we need to adjust this part!
		OUTPUT_CONFIGURATION_SCUMMVM_DEBUG("Debug", "x64", "64", true);
		OUTPUT_CONFIGURATION_SCUMMVM_DEBUG("Analysis", "x64", "64", true);
		OUTPUT_CONFIGURATION_SCUMMVM("Release", "x64", "64");

	} else {
		std::string warnings = "";
		if (warningsIterator != _projectWarnings.end())
			for (StringList::const_iterator i = warningsIterator->second.begin(); i != warningsIterator->second.end(); ++i)
				warnings +=  *i + ';';

		std::string toolConfig;
		toolConfig  = (!warnings.empty() ? "DisableSpecificWarnings=\"" + warnings + "\"" : "");
		toolConfig += (name == "tinsel" ? "DebugInformationFormat=\"3\" " : "");
		toolConfig += (name == "sword25" ? "DisableLanguageExtensions=\"false\" " : "");

		// Win32
		OUTPUT_CONFIGURATION("Debug", "Win32", "");
		OUTPUT_CONFIGURATION("Analysis", "Win32", "");
		OUTPUT_CONFIGURATION("Release", "Win32", "");
		OUTPUT_CONFIGURATION("Debug", "x64", "64");
		OUTPUT_CONFIGURATION("Analysis", "x64", "64");
		OUTPUT_CONFIGURATION("Release", "x64", "64");
	}

	project << "\t</Configurations>\n"
	           "\t<Files>\n";

	std::string modulePath;
	if (!moduleDir.compare(0, setup.srcDir.size(), setup.srcDir)) {
		modulePath = moduleDir.substr(setup.srcDir.size());
		if (!modulePath.empty() && modulePath.at(0) == '/')
			modulePath.erase(0, 1);
	}

	if (modulePath.size())
		addFilesToProject(moduleDir, project, includeList, excludeList, setup.filePrefix + '/' + modulePath);
	else
		addFilesToProject(moduleDir, project, includeList, excludeList, setup.filePrefix);

	project << "\t</Files>\n"
	           "</VisualStudioProject>\n";
}

void VisualStudioProvider::writeReferences(std::ofstream &output) {
	output << "\tProjectSection(ProjectDependencies) = postProject\n";

	for (UUIDMap::const_iterator i = _uuidMap.begin(); i != _uuidMap.end(); ++i) {
		if (i->first == "scummvm")
			continue;

		output << "\t\t{" << i->second << "} = {" << i->second << "}\n";
	}

	output << "\tEndProjectSection\n";
}

void VisualStudioProvider::outputGlobalPropFile(std::ofstream &properties, int bits, const StringList &defines, const std::string &prefix) {
	std::string warnings;
	for (StringList::const_iterator i = _globalWarnings.begin(); i != _globalWarnings.end(); ++i)
		warnings +=  *i + ';';

	std::string definesList;
	for (StringList::const_iterator i = defines.begin(); i != defines.end(); ++i) {
		if (i != defines.begin())
			definesList += ';';
		definesList += *i;
	}

	properties << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>\n"
	              "<VisualStudioPropertySheet\n"
	              "\tProjectType=\"Visual C++\"\n"
	              "\tVersion=\"8.00\"\n"
	              "\tName=\"ScummVM_Global\"\n"
	              "\tOutputDirectory=\"$(ConfigurationName)" << bits << "\"\n"
	              "\tIntermediateDirectory=\"$(ConfigurationName)" << bits << "/$(ProjectName)\"\n"
	              "\t>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCCLCompilerTool\"\n"
	              "\t\tDisableLanguageExtensions=\"true\"\n"
	              "\t\tDisableSpecificWarnings=\"" << warnings << "\"\n"
	              "\t\tAdditionalIncludeDirectories=\"" << prefix << ";" << prefix << "\\engines;$(SCUMMVM_LIBS)\\include\"\n"
	              "\t\tPreprocessorDefinitions=\"" << definesList << "\"\n"
	              "\t\tExceptionHandling=\"0\"\n"
	              "\t\tRuntimeTypeInfo=\"false\"\n"
	              "\t\tWarningLevel=\"4\"\n"
	              "\t\tWarnAsError=\"false\"\n"
	              "\t\tCompileAs=\"0\"\n"
	              "\t\t/>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCLibrarianTool\"\n"
	              "\t\tIgnoreDefaultLibraryNames=\"\"\n"
	              "\t/>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCLinkerTool\"\n"
	              "\t\tIgnoreDefaultLibraryNames=\"\"\n"
	              "\t\tSubSystem=\"1\"\n"
	              "\t\tEntryPointSymbol=\"WinMainCRTStartup\"\n"
	              "\t/>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCResourceCompilerTool\"\n"
	              "\t\tPreprocessorDefinitions=\"HAS_INCLUDE_SET\"\n"
	              "\t\tAdditionalIncludeDirectories=\"$(SCUMMVM_LIBS)\\lib\\" << (bits == 32 ? "x86" : "x64") << "\"\n"
	              "\t/>\n"
	              "</VisualStudioPropertySheet>\n";

	properties.flush();
}

void VisualStudioProvider::createBuildProp(const BuildSetup &setup, bool isRelease, bool isWin32, bool enableAnalysis) {
	const std::string outputType = (enableAnalysis ? "Analysis" : (isRelease ? "Release" : "Debug"));
	const std::string outputBitness = (isWin32 ? "32" : "64");

	std::ofstream properties((setup.outputDir + '/' + "ScummVM_" + outputType + (isWin32 ? "" : "64") + getPropertiesExtension()).c_str());
	if (!properties)
		error("Could not open \"" + setup.outputDir + '/' + "ScummVM_" + outputType + (isWin32 ? "" : "64") + getPropertiesExtension() + "\" for writing");

	properties << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>\n"
	              "<VisualStudioPropertySheet\n"
	              "\tProjectType=\"Visual C++\"\n"
	              "\tVersion=\"8.00\"\n"
	              "\tName=\"ScummVM_" << outputType << outputBitness << "\"\n"
	              "\tInheritedPropertySheets=\".\\ScummVM_Global" << (isWin32 ? "" : "64") << ".vsprops\"\n"
	              "\t>\n"
	              "\t<Tool\n"
	              "\t\tName=\"VCCLCompilerTool\"\n";

	if (isRelease) {
		properties << "\t\tEnableIntrinsicFunctions=\"true\"\n"
		              "\t\tWholeProgramOptimization=\"true\"\n"
		              "\t\tPreprocessorDefinitions=\"WIN32;RELEASE_BUILD\"\n"
		              "\t\tStringPooling=\"true\"\n"
		              "\t\tBufferSecurityCheck=\"false\"\n"
		              "\t\tDebugInformationFormat=\"0\"\n"
		              "\t\tAdditionalOption=\"" << (enableAnalysis ? "/analyze" : "") << "\"\n"
		              "\t/>\n"
		              "\t<Tool\n"
		              "\t\tName=\"VCLinkerTool\"\n"
		              "\t\tLinkIncremental=\"1\"\n"
		              "\t\tIgnoreDefaultLibraryNames=\"\"\n"
		              "\t\tSetChecksum=\"true\"\n";
	} else {
		properties << "\t\tOptimization=\"0\"\n"
		              "\t\tPreprocessorDefinitions=\"WIN32\"\n"
		              "\t\tMinimalRebuild=\"true\"\n"
		              "\t\tBasicRuntimeChecks=\"3\"\n"
		              "\t\tRuntimeLibrary=\"1\"\n"
		              "\t\tEnableFunctionLevelLinking=\"true\"\n"
		              "\t\tWarnAsError=\"false\"\n"
		              "\t\tDebugInformationFormat=\"" << (isWin32 ? "4" : "3") << "\"\n"	// For x64 format "4" (Edit and continue) is not supported, thus we default to "3"
		              "\t\tAdditionalOption=\"" << (enableAnalysis ? "/analyze" : "") << "\"\n"
		              "\t/>\n"
		              "\t<Tool\n"
		              "\t\tName=\"VCLinkerTool\"\n"
		              "\t\tLinkIncremental=\"2\"\n"
		              "\t\tGenerateDebugInformation=\"true\"\n"
		              "\t\tIgnoreDefaultLibraryNames=\"libcmt.lib\"\n";
	}

	properties << "\t/>\n"
	              "</VisualStudioPropertySheet>\n";

	properties.flush();
	properties.close();
}

void VisualStudioProvider::writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int indentation,
                                                  const StringList &duplicate, const std::string &objPrefix, const std::string &filePrefix) {
	const std::string indentString = getIndent(indentation + 2);

	if (indentation)
		projectFile << getIndent(indentation + 1) << "<Filter\tName=\"" << dir.name << "\">\n";

	for (FileNode::NodeList::const_iterator i = dir.children.begin(); i != dir.children.end(); ++i) {
		const FileNode *node = *i;

		if (!node->children.empty()) {
			writeFileListToProject(*node, projectFile, indentation + 1, duplicate, objPrefix + node->name + '_', filePrefix + node->name + '/');
		} else {
			if (producesObjectFile(node->name)) {
				std::string name, ext;
				splitFilename(node->name, name, ext);
				const bool isDuplicate = (std::find(duplicate.begin(), duplicate.end(), name + ".o") != duplicate.end());

				if (ext == "asm") {
					std::string objFileName = "$(IntDir)\\";
					if (isDuplicate)
						objFileName += objPrefix;
					objFileName += "$(InputName).obj";

					const std::string toolLine = indentString + "\t\t<Tool Name=\"VCCustomBuildTool\" CommandLine=\"nasm.exe -f win32 -g -o &quot;" + objFileName + "&quot; &quot;$(InputPath)&quot;&#x0D;&#x0A;\" Outputs=\"" + objFileName + "\" />\n";

					// NASM is not supported for x64, thus we do not need to add additional entries here :-).
					projectFile << indentString << "<File RelativePath=\"" << convertPathToWin(filePrefix + node->name) << "\">\n"
					            << indentString << "\t<FileConfiguration Name=\"Debug|Win32\">\n"
					            << toolLine
					            << indentString << "\t</FileConfiguration>\n"
								<< indentString << "\t<FileConfiguration Name=\"Analysis|Win32\">\n"
								<< toolLine
								<< indentString << "\t</FileConfiguration>\n"
					            << indentString << "\t<FileConfiguration Name=\"Release|Win32\">\n"
					            << toolLine
					            << indentString << "\t</FileConfiguration>\n"
					            << indentString << "</File>\n";
				} else {
					if (isDuplicate) {
						const std::string toolLine = indentString + "\t\t<Tool Name=\"VCCLCompilerTool\" ObjectFile=\"$(IntDir)\\" + objPrefix + "$(InputName).obj\" XMLDocumentationFileName=\"$(IntDir)\\" + objPrefix + "$(InputName).xdc\" />\n";

						projectFile << indentString << "<File RelativePath=\"" << convertPathToWin(filePrefix + node->name) << "\">\n"
						            << indentString << "\t<FileConfiguration Name=\"Debug|Win32\">\n"
						            << toolLine
						            << indentString << "\t</FileConfiguration>\n"
									<< indentString << "\t<FileConfiguration Name=\"Analysis|Win32\">\n"
									<< toolLine
									<< indentString << "\t</FileConfiguration>\n"
						            << indentString << "\t<FileConfiguration Name=\"Release|Win32\">\n"
						            << toolLine
						            << indentString << "\t</FileConfiguration>\n"
									<< indentString << "\t<FileConfiguration Name=\"Debug|x64\">\n"
									<< toolLine
									<< indentString << "\t</FileConfiguration>\n"
									<< indentString << "\t<FileConfiguration Name=\"Analysis|x64\">\n"
									<< toolLine
									<< indentString << "\t</FileConfiguration>\n"
						            << indentString << "\t<FileConfiguration Name=\"Release|x64\">\n"
						            << toolLine
						            << indentString << "\t</FileConfiguration>\n"
						            << indentString << "</File>\n";
					} else {
						projectFile << indentString << "<File RelativePath=\"" << convertPathToWin(filePrefix + node->name) << "\" />\n";
					}
				}
			} else {
				projectFile << indentString << "<File RelativePath=\"" << convertPathToWin(filePrefix + node->name) << "\" />\n";
			}
		}
	}

	if (indentation)
		projectFile << getIndent(indentation + 1) << "</Filter>\n";
}

//////////////////////////////////////////////////////////////////////////
// MSBuild Provider (Visual Studio 2010)
//////////////////////////////////////////////////////////////////////////

MSBuildProvider::MSBuildProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version)
	: MSVCProvider(global_warnings, project_warnings, version) {

}

const char *MSBuildProvider::getProjectExtension() {
	return ".vcxproj";
}

const char *MSBuildProvider::getPropertiesExtension() {
	return ".props";
}

int MSBuildProvider::getVisualStudioVersion() {
	return 2010;
}

#define OUTPUT_CONFIGURATION_MSBUILD(config, platform) \
	(project << "\t\t<ProjectConfiguration Include=\"" << config << "|" << platform << "\">\n" \
	           "\t\t\t<Configuration>" << config << "</Configuration>\n" \
	           "\t\t\t<Platform>" << platform << "</Platform>\n" \
	           "\t\t</ProjectConfiguration>\n")

#define OUTPUT_CONFIGURATION_TYPE_MSBUILD(config) \
	(project << "\t<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='" << config << "'\" Label=\"Configuration\">\n" \
	           "\t\t<ConfigurationType>" << (name == "scummvm" ? "Application" : "StaticLibrary") << "</ConfigurationType>\n" \
	           "\t</PropertyGroup>\n")

#define OUTPUT_PROPERTIES_MSBUILD(config, properties) \
	(project << "\t<ImportGroup Condition=\"'$(Configuration)|$(Platform)'=='" << config << "'\" Label=\"PropertySheets\">\n" \
	           "\t\t<Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />\n" \
	           "\t\t<Import Project=\"" << properties << "\" />\n" \
	           "\t</ImportGroup>\n")

void MSBuildProvider::createProjectFile(const std::string &name, const std::string &uuid, const BuildSetup &setup, const std::string &moduleDir,
                                        const StringList &includeList, const StringList &excludeList) {
	const std::string projectFile = setup.outputDir + '/' + name + getProjectExtension();
	std::ofstream project(projectFile.c_str());
	if (!project)
		error("Could not open \"" + projectFile + "\" for writing");

	project << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
	           "<Project DefaultTargets=\"Build\" ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
	           "\t<ItemGroup Label=\"ProjectConfigurations\">\n";

	OUTPUT_CONFIGURATION_MSBUILD("Debug", "Win32");
	OUTPUT_CONFIGURATION_MSBUILD("Debug", "x64");
	OUTPUT_CONFIGURATION_MSBUILD("Analysis", "Win32");
	OUTPUT_CONFIGURATION_MSBUILD("Analysis", "x64");
	OUTPUT_CONFIGURATION_MSBUILD("Release", "Win32");
	OUTPUT_CONFIGURATION_MSBUILD("Release", "x64");

	project << "\t</ItemGroup>\n";

	// Project name & Guid
	project << "\t<PropertyGroup Label=\"Globals\">\n"
	           "\t\t<ProjectGuid>{" << uuid << "}</ProjectGuid>\n"
	           "\t\t<RootNamespace>" << name << "</RootNamespace>\n"
	           "\t\t<Keyword>Win32Proj</Keyword>\n"
	           "\t</PropertyGroup>\n";

	// Shared configuration
	project << "\t<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\n";

	OUTPUT_CONFIGURATION_TYPE_MSBUILD("Release|Win32");
	OUTPUT_CONFIGURATION_TYPE_MSBUILD("Analysis|Win32");
	OUTPUT_CONFIGURATION_TYPE_MSBUILD("Debug|Win32");
	OUTPUT_CONFIGURATION_TYPE_MSBUILD("Release|x64");
	OUTPUT_CONFIGURATION_TYPE_MSBUILD("Analysis|x64");
	OUTPUT_CONFIGURATION_TYPE_MSBUILD("Debug|x64");

	project << "\t<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\n"
	           "\t<ImportGroup Label=\"ExtensionSettings\">\n"
	           "\t</ImportGroup>\n";

	OUTPUT_PROPERTIES_MSBUILD("Release|Win32",  "ScummVM_Release.props");
	OUTPUT_PROPERTIES_MSBUILD("Analysis|Win32", "ScummVM_Analysis.props");
	OUTPUT_PROPERTIES_MSBUILD("Debug|Win32",    "ScummVM_Debug.props");
	OUTPUT_PROPERTIES_MSBUILD("Release|x64",    "ScummVM_Release64.props");
	OUTPUT_PROPERTIES_MSBUILD("Analysis|x64",   "ScummVM_Analysis64.props");
	OUTPUT_PROPERTIES_MSBUILD("Debug|x64",      "ScummVM_Debug64.props");

	project << "\t<PropertyGroup Label=\"UserMacros\" />\n";

	// Project-specific settings (analysis uses debug properties)
	outputProjectSettings(project, name, setup, false, true, false);
	outputProjectSettings(project, name, setup, false, true, true);
	outputProjectSettings(project, name, setup, true, true, false);
	outputProjectSettings(project, name, setup, false, false, false);
	outputProjectSettings(project, name, setup, false, false, true);
	outputProjectSettings(project, name, setup, true, false, false);

	// Files
	std::string modulePath;
	if (!moduleDir.compare(0, setup.srcDir.size(), setup.srcDir)) {
		modulePath = moduleDir.substr(setup.srcDir.size());
		if (!modulePath.empty() && modulePath.at(0) == '/')
			modulePath.erase(0, 1);
	}

	if (modulePath.size())
		addFilesToProject(moduleDir, project, includeList, excludeList, setup.filePrefix + '/' + modulePath);
	else
		addFilesToProject(moduleDir, project, includeList, excludeList, setup.filePrefix);

	// Output references for scummvm project
	if (name == "scummvm")
		writeReferences(project);

	project << "\t<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\n"
	           "\t<ImportGroup Label=\"ExtensionTargets\">\n"
	           "\t</ImportGroup>\n"
	           "</Project>\n";

	// Output filter file if necessary
	createFiltersFile(setup, name);
}

#define OUTPUT_FILTER_MSBUILD(files, action) \
	if (!files.empty()) { \
		filters << "\t<ItemGroup>\n"; \
		for (std::list<FileEntry>::const_iterator entry = files.begin(); entry != files.end(); ++entry) { \
			if ((*entry).filter != "") { \
				filters << "\t\t<" action " Include=\"" << (*entry).path << "\">\n" \
				           "\t\t\t<Filter>" << (*entry).filter << "</Filter>\n" \
				           "\t\t</" action ">\n"; \
			} else { \
				filters << "\t\t<" action " Include=\"" << (*entry).path << "\" />\n"; \
			} \
		} \
		filters << "\t</ItemGroup>\n"; \
	}

void MSBuildProvider::createFiltersFile(const BuildSetup &setup, const std::string &name) {
	// No filters => no need to create a filter file
	if (_filters.empty())
		return;

	// Sort all list alphabetically
	_filters.sort();
	_compileFiles.sort();
	_includeFiles.sort();
	_otherFiles.sort();
	_resourceFiles.sort();
	_asmFiles.sort();

	const std::string filtersFile = setup.outputDir + '/' + name + getProjectExtension() + ".filters";
	std::ofstream filters(filtersFile.c_str());
	if (!filters)
		error("Could not open \"" + filtersFile + "\" for writing");

	filters << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
	           "<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n";

	// Output the list of filters
	filters << "\t<ItemGroup>\n";
	for (std::list<std::string>::iterator filter = _filters.begin(); filter != _filters.end(); ++filter) {
		filters << "\t\t<Filter Include=\"" << *filter << "\">\n"
		           "\t\t\t<UniqueIdentifier>" << createUUID() << "</UniqueIdentifier>\n"
		           "\t\t</Filter>\n";
	}
	filters << "\t</ItemGroup>\n";

	// Output files
	OUTPUT_FILTER_MSBUILD(_compileFiles, "ClCompile")
	OUTPUT_FILTER_MSBUILD(_includeFiles, "ClInclude")
	OUTPUT_FILTER_MSBUILD(_otherFiles, "None")
	OUTPUT_FILTER_MSBUILD(_resourceFiles, "ResourceCompile")
	OUTPUT_FILTER_MSBUILD(_asmFiles, "CustomBuild")

	filters << "</Project>";
}

void MSBuildProvider::writeReferences(std::ofstream &output) {
	output << "\t<ItemGroup>\n";

	for (UUIDMap::const_iterator i = _uuidMap.begin(); i != _uuidMap.end(); ++i) {
		if (i->first == "scummvm")
			continue;

		output << "\t<ProjectReference Include=\"" << i->first << ".vcxproj\">\n"
		          "\t\t<Project>{" << i->second << "}</Project>\n"
		          "\t</ProjectReference>\n";
	}

	output << "\t</ItemGroup>\n";
}

void MSBuildProvider::outputProjectSettings(std::ofstream &project, const std::string &name, const BuildSetup &setup, bool isRelease, bool isWin32, bool enableAnalysis) {
	const std::string configuration = (enableAnalysis ? "Analysis" : (isRelease ? "Release" : "Debug"));

	// Check for project-specific warnings:
	std::map<std::string, StringList>::iterator warningsIterator = _projectWarnings.find(name);

	// Nothing to add here, move along!
	if (name != "scummvm" && name != "sword25" && name != "tinsel" && warningsIterator == _projectWarnings.end())
		return;

	std::string warnings = "";
	if (warningsIterator != _projectWarnings.end())
		for (StringList::const_iterator i = warningsIterator->second.begin(); i != warningsIterator->second.end(); ++i)
			warnings +=  *i + ';';

	project << "\t<ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='" << configuration << "|" << (isWin32 ? "Win32" : "x64") << "'\">\n"
	           "\t\t<ClCompile>\n";

	// Compile configuration
	if (name == "scummvm" || name == "sword25") {
		project << "\t\t\t<DisableLanguageExtensions>false</DisableLanguageExtensions>\n";
	} else {
		if (name == "tinsel" && !isRelease)
			project << "\t\t\t<DebugInformationFormat>ProgramDatabase</DebugInformationFormat>\n";

		if (warningsIterator != _projectWarnings.end())
			project << "\t\t\t<DisableSpecificWarnings>" << warnings << ";%(DisableSpecificWarnings)</DisableSpecificWarnings>\n";
	}

	project << "\t\t</ClCompile>\n";

	// Link configuration for scummvm project
	if (name == "scummvm") {
		std::string libraries;

		for (StringList::const_iterator i = setup.libraries.begin(); i != setup.libraries.end(); ++i)
			libraries += *i + ".lib;";

		project << "\t\t<Link>\n"
		           "\t\t\t<OutputFile>$(OutDir)scummvm.exe</OutputFile>\n"
		           "\t\t\t<AdditionalDependencies>" << libraries << "%(AdditionalDependencies)</AdditionalDependencies>\n"
		           "\t\t</Link>\n";

		// Only generate revision number in debug builds
		if (!isRelease) {
			project << "\t\t<PreBuildEvent>\n"
			           "\t\t\t<Message>Generate internal_version.h</Message>\n"
			           "\t\t\t<Command>" << getPreBuildEvent() << "</Command>\n"
			           "\t\t</PreBuildEvent>\n";
		}

		// Copy data files to the build folder
		project << "\t\t<PostBuildEvent>\n"
		           "\t\t\t<Message>Copy data files to the build folder</Message>\n"
	               "\t\t\t<Command>" << getPostBuildEvent(isWin32) << "</Command>\n"
		           "\t\t</PostBuildEvent>\n";
	}

	project << "\t</ItemDefinitionGroup>\n";
}

void MSBuildProvider::outputGlobalPropFile(std::ofstream &properties, int bits, const StringList &defines, const std::string &prefix) {

	std::string warnings;
	for (StringList::const_iterator i = _globalWarnings.begin(); i != _globalWarnings.end(); ++i)
		warnings +=  *i + ';';

	std::string definesList;
	for (StringList::const_iterator i = defines.begin(); i != defines.end(); ++i)
		definesList += *i + ';';

	properties << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
	              "<Project DefaultTargets=\"Build\" ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
	              "\t<PropertyGroup>\n"
	              "\t\t<_ProjectFileVersion>10.0.30319.1</_ProjectFileVersion>\n"
	              "\t\t<_PropertySheetDisplayName>ScummVM_Global</_PropertySheetDisplayName>\n"
	              "\t\t<ExecutablePath>$(SCUMMVM_LIBS)\\bin;$(ExecutablePath)</ExecutablePath>\n"
	              "\t\t<LibraryPath>$(SCUMMVM_LIBS)\\lib\\" << (bits == 32 ? "x86" : "x64") << ";$(LibraryPath)</LibraryPath>\n"
	              "\t\t<IncludePath>$(SCUMMVM_LIBS)\\include;$(IncludePath)</IncludePath>\n"
	              "\t\t<OutDir>$(Configuration)" << bits << "\\</OutDir>\n"
	              "\t\t<IntDir>$(Configuration)" << bits << "/$(ProjectName)\\</IntDir>\n"
	              "\t</PropertyGroup>\n"
	              "\t<ItemDefinitionGroup>\n"
	              "\t\t<ClCompile>\n"
	              "\t\t\t<DisableLanguageExtensions>true</DisableLanguageExtensions>\n"
	              "\t\t\t<DisableSpecificWarnings>" << warnings << ";%(DisableSpecificWarnings)</DisableSpecificWarnings>\n"
	              "\t\t\t<AdditionalIncludeDirectories>$(SCUMMVM_LIBS)\\include;" << prefix << ";" << prefix << "\\engines;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>\n"
	              "\t\t\t<PreprocessorDefinitions>" << definesList << "%(PreprocessorDefinitions)</PreprocessorDefinitions>\n"
	              "\t\t\t<ExceptionHandling></ExceptionHandling>\n"
	              "\t\t\t<RuntimeTypeInfo>false</RuntimeTypeInfo>\n"
	              "\t\t\t<WarningLevel>Level4</WarningLevel>\n"
	              "\t\t\t<TreatWarningAsError>false</TreatWarningAsError>\n"
	              "\t\t\t<CompileAs>Default</CompileAs>\n"
	              "\t\t</ClCompile>\n"
	              "\t\t<Link>\n"
	              "\t\t\t<IgnoreSpecificDefaultLibraries>%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>\n"
	              "\t\t\t<SubSystem>Console</SubSystem>\n"
	              "\t\t\t<EntryPointSymbol>WinMainCRTStartup</EntryPointSymbol>\n"
	              "\t\t</Link>\n"
	              "\t\t<ResourceCompile>\n"
	              "\t\t\t<PreprocessorDefinitions>HAS_INCLUDE_SET;%(PreprocessorDefinitions)</PreprocessorDefinitions>\n"
	              "\t\t\t<AdditionalIncludeDirectories>" << prefix << ";%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>\n"
	              "\t\t</ResourceCompile>\n"
	              "\t</ItemDefinitionGroup>\n"
	              "</Project>\n";

	properties.flush();
}

void MSBuildProvider::createBuildProp(const BuildSetup &setup, bool isRelease, bool isWin32, bool enableAnalysis) {
	const std::string outputType = (enableAnalysis ? "Analysis" : (isRelease ? "Release" : "Debug"));
	const std::string outputBitness = (isWin32 ? "32" : "64");

	std::ofstream properties((setup.outputDir + '/' + "ScummVM_" + outputType + (isWin32 ? "" : "64") + getPropertiesExtension()).c_str());
	if (!properties)
		error("Could not open \"" + setup.outputDir + '/' + "ScummVM_" + outputType + (isWin32 ? "" : "64") + getPropertiesExtension() + "\" for writing");

	properties << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
	              "<Project DefaultTargets=\"Build\" ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
	              "\t<ImportGroup Label=\"PropertySheets\">\n"
	              "\t\t<Import Project=\"ScummVM_Global" << (isWin32 ? "" : "64") << ".props\" />\n"
	              "\t</ImportGroup>\n"
	              "\t<PropertyGroup>\n"
	              "\t\t<_ProjectFileVersion>10.0.30319.1</_ProjectFileVersion>\n"
	              "\t\t<_PropertySheetDisplayName>ScummVM_" << outputType << outputBitness << "</_PropertySheetDisplayName>\n"
	              "\t\t<LinkIncremental>" << (isRelease ? "false" : "true") << "</LinkIncremental>\n"
	              "\t</PropertyGroup>\n"
	              "\t<ItemDefinitionGroup>\n"
	              "\t\t<ClCompile>\n";

	if (isRelease) {
		properties << "\t\t\t<IntrinsicFunctions>true</IntrinsicFunctions>\n"
		              "\t\t\t<WholeProgramOptimization>true</WholeProgramOptimization>\n"
		              "\t\t\t<PreprocessorDefinitions>WIN32;RELEASE_BUILD;%(PreprocessorDefinitions)</PreprocessorDefinitions>\n"
		              "\t\t\t<StringPooling>true</StringPooling>\n"
		              "\t\t\t<BufferSecurityCheck>false</BufferSecurityCheck>\n"
		              "\t\t\t<DebugInformationFormat></DebugInformationFormat>\n"
		              "\t\t\t<EnablePREfast>" << (enableAnalysis ? "true" : "false") << "</EnablePREfast>\n"
		              "\t\t</ClCompile>\n"
		              "\t\t<Link>\n"
		              "\t\t\t<IgnoreSpecificDefaultLibraries>%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>\n"
		              "\t\t\t<SetChecksum>true</SetChecksum>\n";
	} else {
		properties << "\t\t\t<Optimization>Disabled</Optimization>\n"
		              "\t\t\t<PreprocessorDefinitions>WIN32;%(PreprocessorDefinitions)</PreprocessorDefinitions>\n"
		              "\t\t\t<MinimalRebuild>true</MinimalRebuild>\n"
		              "\t\t\t<BasicRuntimeChecks>EnableFastChecks</BasicRuntimeChecks>\n"
		              "\t\t\t<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>\n"
		              "\t\t\t<FunctionLevelLinking>true</FunctionLevelLinking>\n"
		              "\t\t\t<TreatWarningAsError>false</TreatWarningAsError>\n"
		              "\t\t\t<DebugInformationFormat>" << (isWin32 ? "EditAndContinue" : "ProgramDatabase") << "</DebugInformationFormat>\n" // For x64 format Edit and continue is not supported, thus we default to Program Database
		              "\t\t\t<EnablePREfast>" << (enableAnalysis ? "true" : "false") << "</EnablePREfast>\n"
		              "\t\t</ClCompile>\n"
		              "\t\t<Link>\n"
		              "\t\t\t<GenerateDebugInformation>true</GenerateDebugInformation>\n"
		              "\t\t\t<IgnoreSpecificDefaultLibraries>libcmt.lib;%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>\n";
	}

	properties << "\t\t</Link>\n"
	              "\t</ItemDefinitionGroup>\n"
	              "</Project>\n";

	properties.flush();
	properties.close();
}

#define OUTPUT_NASM_COMMAND_MSBUILD(config) \
	projectFile << "\t\t\t<Command Condition=\"'$(Configuration)|$(Platform)'=='" << config << "|Win32'\">nasm.exe -f win32 -g -o \"$(IntDir)" << (isDuplicate ? (*entry).prefix : "") << "%(Filename).obj\" \"%(FullPath)\"</Command>\n" \
	               "\t\t\t<Outputs Condition=\"'$(Configuration)|$(Platform)'=='" << config << "|Win32'\">$(IntDir)" << (isDuplicate ? (*entry).prefix : "") << "%(Filename).obj;%(Outputs)</Outputs>\n";

#define OUPUT_FILES_MSBUILD(files, action) \
	if (!files.empty()) { \
		projectFile << "\t<ItemGroup>\n"; \
		for (std::list<FileEntry>::const_iterator entry = files.begin(); entry != files.end(); ++entry) { \
			projectFile << "\t\t<" action " Include=\"" << (*entry).path << "\" />\n"; \
		} \
		projectFile << "\t</ItemGroup>\n"; \
	}

bool hasEnding(std::string const &fullString, std::string const &ending) {
	if (fullString.length() > ending.length()) {
		return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
	} else {
		return false;
	}
}


void MSBuildProvider::writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int, const StringList &duplicate,
                                             const std::string &objPrefix, const std::string &filePrefix) {
	// Reset lists
	_filters.clear();
	_compileFiles.clear();
	_includeFiles.clear();
	_otherFiles.clear();
	_resourceFiles.clear();
	_asmFiles.clear();

	// Compute the list of files
	_filters.push_back(""); // init filters
	computeFileList(dir, duplicate, objPrefix, filePrefix);
	_filters.pop_back();    // remove last empty filter

	// Output compile files
	if (!_compileFiles.empty()) {
		projectFile << "\t<ItemGroup>\n";
		for (std::list<FileEntry>::const_iterator entry = _compileFiles.begin(); entry != _compileFiles.end(); ++entry) {
			const bool isDuplicate = (std::find(duplicate.begin(), duplicate.end(), (*entry).name + ".o") != duplicate.end());

			// Deal with duplicated file names
			if (isDuplicate) {
				projectFile << "\t\t<ClCompile Include=\"" << (*entry).path << "\">\n"
				               "\t\t\t<ObjectFileName>$(IntDir)" << (*entry).prefix << "%(Filename).obj</ObjectFileName>\n";

				if (hasEnding((*entry).path, "base\\version.cpp"))
					projectFile <<  "\t\t\t<PreprocessorDefinitions Condition=\"'$(Configuration)'=='Debug'\">SCUMMVM_SVN_REVISION#&quot; $(SCUMMVM_REVISION_STRING)&quot;;%(PreprocessorDefinitions)</PreprocessorDefinitions>\n";

				projectFile << "\t\t</ClCompile>\n";
			} else {
				projectFile << "\t\t<ClCompile Include=\"" << (*entry).path << "\" />\n";
			}
		}
		projectFile << "\t</ItemGroup>\n";
	}

	// Output include, other and resource files
	OUPUT_FILES_MSBUILD(_includeFiles, "ClInclude")
	OUPUT_FILES_MSBUILD(_otherFiles, "None")
	OUPUT_FILES_MSBUILD(_resourceFiles, "ResourceCompile")

	// Output asm files
	if (!_asmFiles.empty()) {
		projectFile << "\t<ItemGroup>\n";
		for (std::list<FileEntry>::const_iterator entry = _asmFiles.begin(); entry != _asmFiles.end(); ++entry) {

			const bool isDuplicate = (std::find(duplicate.begin(), duplicate.end(), (*entry).name + ".o") != duplicate.end());

			projectFile << "\t\t<CustomBuild Include=\"" << (*entry).path << "\">\n"
			               "\t\t\t<FileType>Document</FileType>\n";

			OUTPUT_NASM_COMMAND_MSBUILD("Debug")
			OUTPUT_NASM_COMMAND_MSBUILD("Analysis")
			OUTPUT_NASM_COMMAND_MSBUILD("Release")

			projectFile << "\t\t</CustomBuild>\n";
		}
		projectFile << "\t</ItemGroup>\n";
	}
}

void MSBuildProvider::computeFileList(const FileNode &dir, const StringList &duplicate, const std::string &objPrefix, const std::string &filePrefix) {
	for (FileNode::NodeList::const_iterator i = dir.children.begin(); i != dir.children.end(); ++i) {
		const FileNode *node = *i;

		if (!node->children.empty()) {
			// Update filter
			std::string _currentFilter = _filters.back();
			_filters.back().append((_filters.back() == "" ? "" : "\\") + node->name);

			computeFileList(*node, duplicate, objPrefix + node->name + '_', filePrefix + node->name + '/');

			// Reset filter
			_filters.push_back(_currentFilter);
		} else {
			// Filter files by extension
			std::string name, ext;
			splitFilename(node->name, name, ext);

			FileEntry entry;
			entry.name = name;
			entry.path = convertPathToWin(filePrefix + node->name);
			entry.filter = _filters.back();
			entry.prefix = objPrefix;

			if (ext == "cpp" || ext == "c")
				_compileFiles.push_back(entry);
			else if (ext == "h")
				_includeFiles.push_back(entry);
			else if (ext == "rc")
				_resourceFiles.push_back(entry);
			else if (ext == "asm")
				_asmFiles.push_back(entry);
			else
				_otherFiles.push_back(entry);
		}
	}
}

} // End of CreateProjectTool namespace
