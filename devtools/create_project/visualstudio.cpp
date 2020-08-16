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

#include "visualstudio.h"
#include "config.h"

#include <algorithm>
#include <fstream>

namespace CreateProjectTool {

//////////////////////////////////////////////////////////////////////////
// Visual Studio Provider (Visual Studio 2008)
//////////////////////////////////////////////////////////////////////////

VisualStudioProvider::VisualStudioProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version, const MSVCVersion &msvc)
    : MSVCProvider(global_warnings, project_warnings, version, msvc) {

	_archs.push_back(ARCH_X86);
	_archs.push_back(ARCH_AMD64);
}

const char *VisualStudioProvider::getProjectExtension() {
	return ".vcproj";
}

const char *VisualStudioProvider::getPropertiesExtension() {
	return ".vsprops";
}

void VisualStudioProvider::createProjectFile(const std::string &name, const std::string &uuid, const BuildSetup &setup, const std::string &moduleDir,
                                             const StringList &includeList, const StringList &excludeList) {
	const std::string projectFile = setup.outputDir + '/' + name + getProjectExtension();
	std::ofstream project(projectFile.c_str());
	if (!project || !project.is_open()) {
		error("Could not open \"" + projectFile + "\" for writing");
		return;
	}

	project << "<?xml version=\"1.0\" encoding=\"windows-1252\"?>\n"
	        << "<VisualStudioProject\n"
	        << "\tProjectType=\"Visual C++\"\n"
	        << "\tVersion=\"" << _version << ".00\"\n"
	        << "\tName=\"" << name << "\"\n"
	        << "\tProjectGUID=\"{" << uuid << "}\"\n"
	        << "\tRootNamespace=\"" << name << "\"\n"
	        << "\tKeyword=\"Win32Proj\"\n";

	project << "\tTargetFrameworkVersion=\"131072\"\n";

	project << "\t>\n"
	           "\t<Platforms>\n";
	for (std::list<MSVC_Architecture>::const_iterator arch = _archs.begin(); arch != _archs.end(); ++arch) {
		project << "\t\t<Platform Name=\"" << getMSVCConfigName(*arch) << "\" />\n";
	}
	project << "\t</Platforms>\n"
	        << "\t<Configurations>\n";

	// Check for project-specific warnings:
	std::map<std::string, std::list<std::string> >::iterator warningsIterator = _projectWarnings.find(name);

	if (setup.devTools || setup.tests || name == setup.projectName) {
		for (std::list<MSVC_Architecture>::const_iterator arch = _archs.begin(); arch != _archs.end(); ++arch) {
			outputConfiguration(project, setup, false, "Debug", *arch);
			outputConfiguration(project, setup, false, "Analysis", *arch);
			outputConfiguration(project, setup, false, "LLVM", *arch);
			outputConfiguration(project, setup, true, "Release", *arch);
		}

	} else {
		bool enableLanguageExtensions = find(_enableLanguageExtensions.begin(), _enableLanguageExtensions.end(), name) != _enableLanguageExtensions.end();
		bool disableEditAndContinue = find(_disableEditAndContinue.begin(), _disableEditAndContinue.end(), name) != _disableEditAndContinue.end();

		std::string warnings = "";
		if (warningsIterator != _projectWarnings.end())
			for (StringList::const_iterator i = warningsIterator->second.begin(); i != warningsIterator->second.end(); ++i)
				warnings += *i + ';';

		std::string toolConfig;
		toolConfig = (!warnings.empty() ? "DisableSpecificWarnings=\"" + warnings + "\"" : "");
		toolConfig += (disableEditAndContinue ? "DebugInformationFormat=\"3\" " : "");
		toolConfig += (enableLanguageExtensions ? "DisableLanguageExtensions=\"false\" " : "");

		for (std::list<MSVC_Architecture>::const_iterator arch = _archs.begin(); arch != _archs.end(); ++arch) {
			outputConfiguration(setup, project, toolConfig, "Debug", *arch);
			outputConfiguration(setup, project, toolConfig, "Analysis", *arch);
			outputConfiguration(setup, project, toolConfig, "LLVM", *arch);
			outputConfiguration(setup, project, toolConfig, "Release", *arch);
		}
	}

	project << "\t</Configurations>\n"
	        << "\t<Files>\n";

	std::string modulePath;
	if (!moduleDir.compare(0, setup.srcDir.size(), setup.srcDir)) {
		modulePath = moduleDir.substr(setup.srcDir.size());
		if (!modulePath.empty() && modulePath.at(0) == '/')
			modulePath.erase(0, 1);
	}

	if (!modulePath.empty())
		addFilesToProject(moduleDir, project, includeList, excludeList, setup.filePrefix + '/' + modulePath);
	else
		addFilesToProject(moduleDir, project, includeList, excludeList, setup.filePrefix);

	// Output auto-generated test runner
	if (setup.tests) {
		project << "\t\t<File RelativePath=\"test_runner.cpp\" />\n";
	}

	project << "\t</Files>\n"
	        << "</VisualStudioProject>\n";
}

void VisualStudioProvider::outputConfiguration(std::ostream &project, const BuildSetup &setup, bool isRelease, const std::string &config, const MSVC_Architecture arch) {
	std::string libraries = outputLibraryDependencies(setup, isRelease);

	project << "\t\t<Configuration Name=\"" << config << "|" << getMSVCConfigName(arch) << "\" ConfigurationType=\"1\" InheritedPropertySheets=\".\\" << setup.projectDescription << "_" << config << getMSVCArchName(arch) << ".vsprops\">\n"
	        << "\t\t\t<Tool\tName=\"VCCLCompilerTool\" DisableLanguageExtensions=\"false\" DebugInformationFormat=\"3\" />\n"
	        << "\t\t\t<Tool\tName=\"VCLinkerTool\" OutputFile=\"$(OutDir)/" << setup.projectName << ".exe\"\n"
	        << "\t\t\t\tAdditionalDependencies=\"" << libraries << "\"\n"
	        << "\t\t\t/>\n";
	outputBuildEvents(project, setup, arch);
	project << "\t\t</Configuration>\n";
}

void VisualStudioProvider::outputConfiguration(const BuildSetup &setup, std::ostream &project, const std::string &toolConfig, const std::string &config, const MSVC_Architecture arch) {
	project << "\t\t<Configuration Name=\"" << config << "|" << getMSVCConfigName(arch) << "\" ConfigurationType=\"4\" InheritedPropertySheets=\".\\" << setup.projectDescription << "_" << config << getMSVCArchName(arch) << ".vsprops\">\n"
	        << "\t\t\t<Tool Name=\"VCCLCompilerTool\" " << toolConfig << "/>\n"
	        << "\t\t</Configuration>\n";
}

void VisualStudioProvider::outputBuildEvents(std::ostream &project, const BuildSetup &setup, const MSVC_Architecture arch) {
	if (!setup.devTools && !setup.tests && setup.runBuildEvents) {
		project << "\t\t\t<Tool\tName=\"VCPreBuildEventTool\"\n"
		        << "\t\t\t\tCommandLine=\"" << getPreBuildEvent() << "\"\n"
		        << "\t\t\t/>\n"
		        << "\t\t\t<Tool\tName=\"VCPostBuildEventTool\"\n"
		        << "\t\t\t\tCommandLine=\"" << getPostBuildEvent(arch, setup) << "\"\n"
		        << "\t\t\t/>\n";
	}

	// Generate runner file before build for tests
	if (setup.tests) {
		project << "\t\t\t<Tool\tName=\"VCPreBuildEventTool\"\n"
		        << "\t\t\t\tCommandLine=\"" << getTestPreBuildEvent(setup) << "\"\n"
		        << "\t\t\t/>\n";

		project << "\t\t\t<Tool\tName=\"VCPostBuildEventTool\"\n"
		        << "\t\t\t\tCommandLine=\"$(TargetPath)\" IgnoreExitCode=\"true\"\n"
		        << "\t\t\t/>\n";
	}
}

void VisualStudioProvider::writeReferences(const BuildSetup &setup, std::ofstream &output) {
	output << "\tProjectSection(ProjectDependencies) = postProject\n";

	for (UUIDMap::const_iterator i = _engineUuidMap.begin(); i != _engineUuidMap.end(); ++i) {
		output << "\t\t{" << i->second << "} = {" << i->second << "}\n";
	}

	output << "\tEndProjectSection\n";
}

void VisualStudioProvider::outputGlobalPropFile(const BuildSetup &setup, std::ofstream &properties, MSVC_Architecture arch, const StringList &defines, const std::string &prefix, bool runBuildEvents) {
	std::string warnings;
	for (StringList::const_iterator i = _globalWarnings.begin(); i != _globalWarnings.end(); ++i)
		warnings += *i + ';';

	std::string definesList;
	for (StringList::const_iterator i = defines.begin(); i != defines.end(); ++i) {
		if (i != defines.begin())
			definesList += ';';
		definesList += *i;
	}

	// Add define to include revision header
	if (runBuildEvents)
		definesList += REVISION_DEFINE ";";

	properties << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>\n"
	           << "<VisualStudioPropertySheet\n"
	           << "\tProjectType=\"Visual C++\"\n"
	           << "\tVersion=\"8.00\"\n"
	           << "\tName=\"" << setup.projectDescription << "_Global\"\n"
	           << "\tOutputDirectory=\"$(ConfigurationName)" << getMSVCArchName(arch) << "\"\n"
	           << "\tIntermediateDirectory=\"$(ConfigurationName)" << getMSVCArchName(arch) << "/$(ProjectName)\"\n"
	           << "\t>\n"
	           << "\t<Tool\n"
	           << "\t\tName=\"VCCLCompilerTool\"\n"
	           << "\t\tDisableLanguageExtensions=\"" << (setup.devTools ? "false" : "true") << "\"\n"
	           << "\t\tDisableSpecificWarnings=\"" << warnings << "\"\n"
	           << "\t\tAdditionalIncludeDirectories=\".\\;" << prefix << ";" << prefix << "\\engines;$(" << LIBS_DEFINE << ")\\include;$(" << LIBS_DEFINE << ")\\include\\SDL;" << (setup.tests ? prefix + "\\test\\cxxtest;" : "") << "\"\n"
	           << "\t\tPreprocessorDefinitions=\"" << definesList << "\"\n"
	           << "\t\tExceptionHandling=\"" << ((setup.devTools || setup.tests || _version == 14) ? "1" : "0") << "\"\n";

#if NEEDS_RTTI
	properties << "\t\tRuntimeTypeInfo=\"true\"\n";
#else
	properties << "\t\tRuntimeTypeInfo=\"false\"\n";
#endif

	properties << "\t\tWarningLevel=\"4\"\n"
	           << "\t\tWarnAsError=\"false\"\n"
	           << "\t\tCompileAs=\"0\"\n"
	           << "\t\tObjectFile=\"$(IntDir)dists\\msvc\\%(RelativeDir)\"\n"
	           << "\t\t/>\n"
	           << "\t<Tool\n"
	           << "\t\tName=\"VCLibrarianTool\"\n"
	           << "\t\tIgnoreDefaultLibraryNames=\"\"\n"
	           << "\t/>\n"
	           << "\t<Tool\n"
	           << "\t\tName=\"VCLinkerTool\"\n"
	           << "\t\tIgnoreDefaultLibraryNames=\"\"\n";
	if (setup.featureEnabled("text-console")) {
		properties << "\t\tSubSystem=\"1\"\n";
	} else {
		properties << "\t\tSubSystem=\"2\"\n";
	}

	if (!setup.devTools && !setup.tests)
		properties << "\t\tEntryPointSymbol=\"WinMainCRTStartup\"\n";

	properties << "\t\tAdditionalLibraryDirectories=\"$(" << LIBS_DEFINE << ")\\lib\\" << getMSVCArchName(arch) << "\"\n"
	           << "\t/>\n"
	           << "\t<Tool\n"
	           << "\t\tName=\"VCResourceCompilerTool\"\n"
	           << "\t\tAdditionalIncludeDirectories=\".\\;" << prefix << "\"\n"
	           << "\t\tPreprocessorDefinitions=\"" << definesList << "\"\n"
	           << "\t/>\n"
	           << "</VisualStudioPropertySheet>\n";

	properties.flush();
}

void VisualStudioProvider::createBuildProp(const BuildSetup &setup, bool isRelease, MSVC_Architecture arch, const std::string &configuration) {

	std::ofstream properties((setup.outputDir + '/' + setup.projectDescription + "_" + configuration + getMSVCArchName(arch) + getPropertiesExtension()).c_str());
	if (!properties || !properties.is_open()) {
		error("Could not open \"" + setup.outputDir + '/' + setup.projectDescription + "_" + configuration + getMSVCArchName(arch) + getPropertiesExtension() + "\" for writing");
		return;
	}

	properties << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>\n"
	           << "<VisualStudioPropertySheet\n"
	           << "\tProjectType=\"Visual C++\"\n"
	           << "\tVersion=\"8.00\"\n"
	           << "\tName=\"" << setup.projectDescription << "_" << configuration << getMSVCArchName(arch) << "\"\n"
	           << "\tInheritedPropertySheets=\".\\" << setup.projectDescription << "_Global" << getMSVCArchName(arch) << ".vsprops\"\n"
	           << "\t>\n"
	           << "\t<Tool\n"
	           << "\t\tName=\"VCCLCompilerTool\"\n";

	if (isRelease) {
		properties << "\t\tEnableIntrinsicFunctions=\"true\"\n"
		           << "\t\tWholeProgramOptimization=\"true\"\n"
		           << "\t\tPreprocessorDefinitions=\"WIN32;RELEASE_BUILD\"\n"
		           << "\t\tStringPooling=\"true\"\n"
		           << "\t\tBufferSecurityCheck=\"false\"\n"
		           << "\t\tDebugInformationFormat=\"0\"\n"
		           << "\t\tRuntimeLibrary=\"0\"\n"
		           << "\t\tAdditionalOption=\"" << (configuration == "Analysis" ? "/analyze" : "") << "\"\n"
		           << "\t/>\n"
		           << "\t<Tool\n"
		           << "\t\tName=\"VCLinkerTool\"\n"
		           << "\t\tLinkIncremental=\"1\"\n"
		           << "\t\tGenerateManifest=\"false\"\n"
		           << "\t\tIgnoreDefaultLibraryNames=\"\"\n"
		           << "\t\tSetChecksum=\"true\"\n";
	} else {
		properties << "\t\tOptimization=\"0\"\n"
		           << "\t\tPreprocessorDefinitions=\"WIN32\"\n"
		           << "\t\tMinimalRebuild=\"true\"\n"
		           << "\t\tBasicRuntimeChecks=\"3\"\n"
		           << "\t\tRuntimeLibrary=\"1\"\n"
		           << "\t\tEnableFunctionLevelLinking=\"true\"\n"
		           << "\t\tWarnAsError=\"false\"\n"
		           << "\t\tDebugInformationFormat=\"" << (arch == ARCH_X86 ? "3" : "4") << "\"\n" // For x64 format "4" (Edit and continue) is not supported, thus we default to "3"
		           << "\t\tAdditionalOption=\"" << (configuration == "Analysis" ? "/analyze" : "") << "\"\n"
		           << "\t/>\n"
		           << "\t<Tool\n"
		           << "\t\tName=\"VCLinkerTool\"\n"
		           << "\t\tLinkIncremental=\"2\"\n"
		           << "\t\tGenerateManifest=\"false\"\n"
		           << "\t\tGenerateDebugInformation=\"true\"\n"
		           << "\t\tIgnoreDefaultLibraryNames=\"libcmt.lib\"\n";
	}

	properties << "\t/>\n"
	           << "</VisualStudioPropertySheet>\n";

	properties.flush();
	properties.close();
}

void VisualStudioProvider::writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int indentation,
                                                  const std::string &objPrefix, const std::string &filePrefix) {
	const std::string indentString = getIndent(indentation + 2);

	if (indentation)
		projectFile << getIndent(indentation + 1) << "<Filter\tName=\"" << dir.name << "\">\n";

	for (FileNode::NodeList::const_iterator i = dir.children.begin(); i != dir.children.end(); ++i) {
		const FileNode *node = *i;

		if (!node->children.empty()) {
			writeFileListToProject(*node, projectFile, indentation + 1, objPrefix + node->name + '_', filePrefix + node->name + '/');
		} else {
			std::string filePath = convertPathToWin(filePrefix + node->name);
			if (producesObjectFile(node->name)) {
				std::string name, ext;
				splitFilename(node->name, name, ext);

				if (ext == "asm") {
					std::string objFileName = "$(IntDir)\\";
					objFileName += objPrefix;
					objFileName += "$(InputName).obj";

					const std::string toolLine = indentString + "\t\t<Tool Name=\"VCCustomBuildTool\" CommandLine=\"nasm.exe -f win32 -g -o &quot;" + objFileName + "&quot; &quot;$(InputPath)&quot;&#x0D;&#x0A;\" Outputs=\"" + objFileName + "\" />\n";

					// NASM is not supported for x64, thus we do not need to add additional entries here :-).
					writeFileToProject(projectFile, filePath, ARCH_X86, indentString, toolLine);
				} else {
					projectFile << indentString << "<File RelativePath=\"" << filePath << "\" />\n";
				}
			} else {
				projectFile << indentString << "<File RelativePath=\"" << filePath << "\" />\n";
			}
		}
	}

	if (indentation)
		projectFile << getIndent(indentation + 1) << "</Filter>\n";
}

void VisualStudioProvider::writeFileToProject(std::ofstream &projectFile, const std::string &filePath, MSVC_Architecture arch,
                                              const std::string &indentString, const std::string &toolLine) {
	projectFile << indentString << "<File RelativePath=\"" << filePath << "\">\n"
	            << indentString << "\t<FileConfiguration Name=\"Debug|" << getMSVCConfigName(arch) << "\">\n"
	            << toolLine
	            << indentString << "\t</FileConfiguration>\n"
	            << indentString << "\t<FileConfiguration Name=\"Analysis|" << getMSVCConfigName(arch) << "\">\n"
	            << toolLine
	            << indentString << "\t</FileConfiguration>\n"
	            << indentString << "\t<FileConfiguration Name=\"LLVM|" << getMSVCConfigName(arch) << "\">\n"
	            << toolLine
	            << indentString << "\t</FileConfiguration>\n"
	            << indentString << "\t<FileConfiguration Name=\"Release|" << getMSVCConfigName(arch) << "\">\n"
	            << toolLine
	            << indentString << "\t</FileConfiguration>\n"
	            << indentString << "</File>\n";
}

} // namespace CreateProjectTool
