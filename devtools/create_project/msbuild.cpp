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

#include "msbuild.h"
#include "config.h"

#include <algorithm>
#include <fstream>

namespace CreateProjectTool {

//////////////////////////////////////////////////////////////////////////
// MSBuild Provider (Visual Studio 2010 and later)
//////////////////////////////////////////////////////////////////////////

MSBuildProvider::MSBuildProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version, const MSVCVersion &msvc)
    : MSVCProvider(global_warnings, project_warnings, version, msvc) {

	_archs.push_back(ARCH_X86);
	_archs.push_back(ARCH_AMD64);
	_archs.push_back(ARCH_ARM64);
}

const char *MSBuildProvider::getProjectExtension() {
	return ".vcxproj";
}

const char *MSBuildProvider::getPropertiesExtension() {
	return ".props";
}

namespace {

inline void outputConfiguration(std::ostream &project, const std::string &config, const std::string &platform) {
	project << "\t\t<ProjectConfiguration Include=\"" << config << "|" << platform << "\">\n"
	        << "\t\t\t<Configuration>" << config << "</Configuration>\n"
	        << "\t\t\t<Platform>" << platform << "</Platform>\n"
	        << "\t\t</ProjectConfiguration>\n";
}

inline void outputConfigurationType(const BuildSetup &setup, std::ostream &project, const std::string &name, const std::string &config, const std::string &toolset) {
	project << "\t<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='" << config << "'\" Label=\"Configuration\">\n"
	        << "\t\t<ConfigurationType>" << ((name == setup.projectName || setup.devTools || setup.tests) ? "Application" : "StaticLibrary") << "</ConfigurationType>\n"
	        << "\t\t<PlatformToolset>" << toolset << "</PlatformToolset>\n"
	        << "\t</PropertyGroup>\n";
}

inline void outputProperties(std::ostream &project, const std::string &config, const std::string &properties) {
	project << "\t<ImportGroup Condition=\"'$(Configuration)|$(Platform)'=='" << config << "'\" Label=\"PropertySheets\">\n"
	        << "\t\t<Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />\n"
	        << "\t\t<Import Project=\"" << properties << "\" />\n"
	        << "\t</ImportGroup>\n";
}

} // End of anonymous namespace

void MSBuildProvider::createProjectFile(const std::string &name, const std::string &uuid, const BuildSetup &setup, const std::string &moduleDir,
                                        const StringList &includeList, const StringList &excludeList) {
	const std::string projectFile = setup.outputDir + '/' + name + getProjectExtension();
	std::ofstream project(projectFile.c_str());
	if (!project || !project.is_open()) {
		error("Could not open \"" + projectFile + "\" for writing");
		return;
	}

	project << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
	        << "<Project DefaultTargets=\"Build\" ToolsVersion=\"" << _msvcVersion.project << "\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
	        << "\t<ItemGroup Label=\"ProjectConfigurations\">\n";

	for (std::list<MSVC_Architecture>::const_iterator arch = _archs.begin(); arch != _archs.end(); ++arch) {
		outputConfiguration(project, "Debug", getMSVCConfigName(*arch));
		outputConfiguration(project, "Analysis", getMSVCConfigName(*arch));
		outputConfiguration(project, "LLVM", getMSVCConfigName(*arch));
		outputConfiguration(project, "Release", getMSVCConfigName(*arch));
	}
	project << "\t</ItemGroup>\n";

	// Project name & Guid
	project << "\t<PropertyGroup Label=\"Globals\">\n"
	        << "\t\t<ProjectGuid>{" << uuid << "}</ProjectGuid>\n"
	        << "\t\t<RootNamespace>" << name << "</RootNamespace>\n"
	        << "\t\t<Keyword>Win32Proj</Keyword>\n"
	        << "\t\t<VCTargetsPath Condition=\"'$(VCTargetsPath" << _version << ")' != '' and '$(VSVersion)' == '' and $(VisualStudioVersion) == ''\">$(VCTargetsPath" << _version << ")</VCTargetsPath>\n";

	for (std::list<MSVC_Architecture>::const_iterator arch = _archs.begin(); arch != _archs.end(); ++arch) {
		project << "\t\t<VcpkgTriplet Condition=\"'$(Platform)' == '" << getMSVCConfigName(*arch) << "'\">" << getMSVCArchName(*arch) << "-windows</VcpkgTriplet>\n";
	}

	project << "\t</PropertyGroup>\n";

	// Shared configuration
	project << "\t<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\n";

	for (std::list<MSVC_Architecture>::const_iterator arch = _archs.begin(); arch != _archs.end(); ++arch) {
		outputConfigurationType(setup, project, name, "Release|" + getMSVCConfigName(*arch), _msvcVersion.toolsetMSVC);
		outputConfigurationType(setup, project, name, "Analysis|" + getMSVCConfigName(*arch), _msvcVersion.toolsetMSVC);
		outputConfigurationType(setup, project, name, "LLVM|" + getMSVCConfigName(*arch), _msvcVersion.toolsetLLVM);
		outputConfigurationType(setup, project, name, "Debug|" + getMSVCConfigName(*arch), _msvcVersion.toolsetMSVC);
	}

	project << "\t<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\n"
	        << "\t<ImportGroup Label=\"ExtensionSettings\">\n"
	        << "\t</ImportGroup>\n";

	for (std::list<MSVC_Architecture>::const_iterator arch = _archs.begin(); arch != _archs.end(); ++arch) {
		outputProperties(project, "Release|" + getMSVCConfigName(*arch), setup.projectDescription + "_Release" + getMSVCArchName(*arch) + ".props");
		outputProperties(project, "Analysis|" + getMSVCConfigName(*arch), setup.projectDescription + "_Analysis" + getMSVCArchName(*arch) + ".props");
		outputProperties(project, "LLVM|" + getMSVCConfigName(*arch), setup.projectDescription + "_LLVM" + getMSVCArchName(*arch) + ".props");
		outputProperties(project, "Debug|" + getMSVCConfigName(*arch), setup.projectDescription + "_Debug" + getMSVCArchName(*arch) + ".props");
	}

	project << "\t<PropertyGroup Label=\"UserMacros\" />\n";

	// Project-specific settings (analysis uses debug properties)
	for (std::list<MSVC_Architecture>::const_iterator arch = _archs.begin(); arch != _archs.end(); ++arch) {
		BuildSetup archsetup = setup;
		std::map<MSVC_Architecture, StringList>::const_iterator disabled_features_it = _arch_disabled_features.find(*arch);
		if (disabled_features_it != _arch_disabled_features.end()) {
			for (StringList::const_iterator j = disabled_features_it->second.begin(); j != disabled_features_it->second.end(); ++j) {
				archsetup = removeFeatureFromSetup(archsetup, *j);
			}
		}
		outputProjectSettings(project, name, archsetup, false, *arch, "Debug");
		outputProjectSettings(project, name, archsetup, false, *arch, "Analysis");
		outputProjectSettings(project, name, archsetup, false, *arch, "LLVM");
		outputProjectSettings(project, name, archsetup, true, *arch, "Release");
	}

	// Files
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

	// Output references for the main project
	if (name == setup.projectName)
		writeReferences(setup, project);

	// Output auto-generated test runner
	if (setup.tests) {
		project << "\t<ItemGroup>\n";
		project << "\t\t<ClCompile Include=\"test_runner.cpp\" />\n";
		project << "\t</ItemGroup>\n";
	}

	// Visual Studio 2015 and up automatically import natvis files that are part of the project
	if (name == PROJECT_NAME && _version >= 14) {
		project << "\t<ItemGroup>\n";
		project << "\t\t<None Include=\"" << setup.srcDir << "/devtools/create_project/scripts/scummvm.natvis\" />\n";
		project << "\t</ItemGroup>\n";
	}

	project << "\t<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\n"
	           "\t<ImportGroup Label=\"ExtensionTargets\">\n"
	           "\t</ImportGroup>\n";

	if (setup.tests) {
		// We override the normal target to ignore the exit code (this allows us to have a clean output and not message about the command exit code)
		project << "\t\t<Target Name=\"PostBuildEvent\">\n"
		        << "\t\t\t<Message Text=\"Description: Run tests\" />\n"
		        << "\t\t\t<Exec Command=\"$(TargetPath)\"  IgnoreExitCode=\"true\" />\n"
		        << "\t\t</Target>\n";
	}

	project << "</Project>\n";

	// Output filter file if necessary
	createFiltersFile(setup, name);
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
	if (!filters || !filters.is_open()) {
		error("Could not open \"" + filtersFile + "\" for writing");
		return;
	}

	filters << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
	        << "<Project ToolsVersion=\"" << _msvcVersion.project << "\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n";

	// Output the list of filters
	filters << "\t<ItemGroup>\n";
	for (std::list<std::string>::iterator filter = _filters.begin(); filter != _filters.end(); ++filter) {
		filters << "\t\t<Filter Include=\"" << *filter << "\">\n"
		        << "\t\t\t<UniqueIdentifier>" << createUUID() << "</UniqueIdentifier>\n"
		        << "\t\t</Filter>\n";
	}
	filters << "\t</ItemGroup>\n";

	// Output files
	outputFilter(filters, _compileFiles, "ClCompile");
	outputFilter(filters, _includeFiles, "ClInclude");
	outputFilter(filters, _otherFiles, "None");
	outputFilter(filters, _resourceFiles, "ResourceCompile");
	outputFilter(filters, _asmFiles, "CustomBuild");

	filters << "</Project>";
}

void MSBuildProvider::outputFilter(std::ostream &filters, const FileEntries &files, const std::string &action) {
	if (!files.empty()) {
		filters << "\t<ItemGroup>\n";
		for (FileEntries::const_iterator entry = files.begin(), end = files.end(); entry != end; ++entry) {
			if ((*entry).filter != "") {
				filters << "\t\t<" << action << " Include=\"" << (*entry).path << "\">\n"
				        << "\t\t\t<Filter>" << (*entry).filter << "</Filter>\n"
				        << "\t\t</" << action << ">\n";
			} else {
				filters << "\t\t<" << action << " Include=\"" << (*entry).path << "\" />\n";
			}
		}
		filters << "\t</ItemGroup>\n";
	}
}

void MSBuildProvider::writeReferences(const BuildSetup &setup, std::ofstream &output) {
	output << "\t<ItemGroup>\n";

	for (UUIDMap::const_iterator i = _engineUuidMap.begin(); i != _engineUuidMap.end(); ++i) {
		output << "\t<ProjectReference Include=\"" << i->first << ".vcxproj\">\n"
		       << "\t\t<Project>{" << i->second << "}</Project>\n"
		       << "\t</ProjectReference>\n";
	}

	output << "\t</ItemGroup>\n";
}

void MSBuildProvider::outputProjectSettings(std::ofstream &project, const std::string &name, const BuildSetup &setup, bool isRelease, MSVC_Architecture arch, const std::string &configuration) {
	// Check for project-specific warnings:
	std::map<std::string, StringList>::iterator warningsIterator = _projectWarnings.find(name);
	bool enableLanguageExtensions = find(_enableLanguageExtensions.begin(), _enableLanguageExtensions.end(), name) != _enableLanguageExtensions.end();
	bool disableEditAndContinue = find(_disableEditAndContinue.begin(), _disableEditAndContinue.end(), name) != _disableEditAndContinue.end();

	// Nothing to add here, move along!
	if ((!setup.devTools || !setup.tests) && name != setup.projectName && !enableLanguageExtensions && !disableEditAndContinue && warningsIterator == _projectWarnings.end())
		return;

	std::string warnings = "";
	if (warningsIterator != _projectWarnings.end())
		for (StringList::const_iterator i = warningsIterator->second.begin(); i != warningsIterator->second.end(); ++i)
			warnings += *i + ';';

	project << "\t<ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='" << configuration << "|" << getMSVCConfigName(arch) << "'\">\n"
	        << "\t\t<ClCompile>\n";

	// Language Extensions
	if (setup.devTools || setup.tests || name == setup.projectName || enableLanguageExtensions) {
		project << "\t\t\t<DisableLanguageExtensions>false</DisableLanguageExtensions>\n";
		project << "\t\t\t<ConformanceMode>false</ConformanceMode>\n"; // Required for Windows SDK 8.1
	}

	// Edit and Continue
	if ((name == setup.projectName || disableEditAndContinue) && !isRelease)
		project << "\t\t\t<DebugInformationFormat>ProgramDatabase</DebugInformationFormat>\n";

	// Warnings
	if (warningsIterator != _projectWarnings.end())
		project << "\t\t\t<DisableSpecificWarnings>" << warnings << ";%(DisableSpecificWarnings)</DisableSpecificWarnings>\n";

	project << "\t\t</ClCompile>\n";

	// Link configuration for main project
	if (name == setup.projectName || setup.devTools || setup.tests) {
		std::string libraries = outputLibraryDependencies(setup, isRelease);

		// MSBuild uses ; for separators instead of spaces
		for (std::string::iterator i = libraries.begin(); i != libraries.end(); ++i) {
			if (*i == ' ') {
				*i = ';';
			}
		}

		project << "\t\t<Link>\n"
		        << "\t\t\t<OutputFile>$(OutDir)" << ((setup.devTools || setup.tests) ? name : setup.projectName) << ".exe</OutputFile>\n"
		        << "\t\t\t<AdditionalDependencies>" << libraries << "%(AdditionalDependencies)</AdditionalDependencies>\n"
		        << "\t\t</Link>\n";

		if (!setup.devTools && !setup.tests && setup.runBuildEvents) {
			project << "\t\t<PreBuildEvent>\n"
			        << "\t\t\t<Message>Generate revision</Message>\n"
			        << "\t\t\t<Command>" << getPreBuildEvent() << "</Command>\n"
			        << "\t\t</PreBuildEvent>\n";

			// Copy data files to the build folder
			project << "\t\t<PostBuildEvent>\n"
			        << "\t\t\t<Message>Copy data files to the build folder</Message>\n"
			        << "\t\t\t<Command>" << getPostBuildEvent(arch, setup) << "</Command>\n"
			        << "\t\t</PostBuildEvent>\n";
		} else if (setup.tests) {
			project << "\t\t<PreBuildEvent>\n"
			        << "\t\t\t<Message>Generate runner.cpp</Message>\n"
			        << "\t\t\t<Command>" << getTestPreBuildEvent(setup) << "</Command>\n"
			        << "\t\t</PreBuildEvent>\n";
		}
	}

	project << "\t</ItemDefinitionGroup>\n";
}

void MSBuildProvider::outputGlobalPropFile(const BuildSetup &setup, std::ofstream &properties, MSVC_Architecture arch, const StringList &defines, const std::string &prefix, bool runBuildEvents) {

	std::string warnings;
	for (StringList::const_iterator i = _globalWarnings.begin(); i != _globalWarnings.end(); ++i)
		warnings += *i + ';';

	std::string definesList;
	for (StringList::const_iterator i = defines.begin(); i != defines.end(); ++i)
		definesList += *i + ';';

	// Add define to include revision header
	if (runBuildEvents)
		definesList += REVISION_DEFINE ";";

	properties << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
	           << "<Project DefaultTargets=\"Build\" ToolsVersion=\"" << _msvcVersion.project << "\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
	           << "\t<PropertyGroup>\n"
	           << "\t\t<_PropertySheetDisplayName>" << setup.projectDescription << "_Global</_PropertySheetDisplayName>\n"
	           << "\t\t<ExecutablePath>$(" << LIBS_DEFINE << ")\\bin;$(" << LIBS_DEFINE << ")\\bin\\" << getMSVCArchName(arch) << ";$(" << LIBS_DEFINE << ")\\$(Configuration)\\bin;$(ExecutablePath)</ExecutablePath>\n"
	           << "\t\t<LibraryPath>$(" << LIBS_DEFINE << ")\\lib\\" << getMSVCArchName(arch) << ";$(" << LIBS_DEFINE << ")\\lib\\" << getMSVCArchName(arch) << "\\$(Configuration);$(" << LIBS_DEFINE << ")\\lib;$(" << LIBS_DEFINE << ")\\$(Configuration)\\lib;$(LibraryPath)</LibraryPath>\n"
	           << "\t\t<IncludePath>$(" << LIBS_DEFINE << ")\\include;$(" << LIBS_DEFINE << ")\\include\\" << (setup.useSDL2 ? "SDL2" : "SDL") << ";$(IncludePath)</IncludePath>\n"
	           << "\t\t<OutDir>$(Configuration)" << getMSVCArchName(arch) << "\\</OutDir>\n"
	           << "\t\t<IntDir>$(Configuration)" << getMSVCArchName(arch) << "\\$(ProjectName)\\</IntDir>\n"
	           << "\t</PropertyGroup>\n"
	           << "\t<ItemDefinitionGroup>\n"
	           << "\t\t<ClCompile>\n"
	           << "\t\t\t<DisableLanguageExtensions>true</DisableLanguageExtensions>\n"
	           << "\t\t\t<DisableSpecificWarnings>" << warnings << ";%(DisableSpecificWarnings)</DisableSpecificWarnings>\n"
	           << "\t\t\t<AdditionalIncludeDirectories>.;" << prefix << ";" << prefix << "\\engines;" << (setup.tests ? prefix + "\\test\\cxxtest;" : "") << "%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>\n"
	           << "\t\t\t<PreprocessorDefinitions>" << definesList << "%(PreprocessorDefinitions)</PreprocessorDefinitions>\n"
	           << "\t\t\t<ExceptionHandling>" << ((setup.devTools || setup.tests) ? "Sync" : "") << "</ExceptionHandling>\n";

#if NEEDS_RTTI
	properties << "\t\t\t<RuntimeTypeInfo>true</RuntimeTypeInfo>\n";
#else
	properties << "\t\t\t<RuntimeTypeInfo>false</RuntimeTypeInfo>\n";
#endif

	properties << "\t\t\t<WarningLevel>Level4</WarningLevel>\n"
	           << "\t\t\t<TreatWarningAsError>false</TreatWarningAsError>\n"
	           << "\t\t\t<CompileAs>Default</CompileAs>\n"
	           << "\t\t\t<MultiProcessorCompilation>true</MultiProcessorCompilation>\n"
	           << "\t\t\t<ConformanceMode>true</ConformanceMode>\n"
	           << "\t\t\t<ObjectFileName>$(IntDir)dists\\msvc\\%(RelativeDir)</ObjectFileName>\n"
	           << "\t\t\t<AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>\n"
	           << "\t\t</ClCompile>\n"
	           << "\t\t<Link>\n"
	           << "\t\t\t<IgnoreSpecificDefaultLibraries>%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>\n";
	if (!setup.featureEnabled("text-console")) {
		properties << "\t\t\t<SubSystem>Windows</SubSystem>\n";
	} else {
		properties << "\t\t\t<SubSystem>Console</SubSystem>\n";
	}

	if (!setup.devTools && !setup.tests)
		properties << "\t\t\t<EntryPointSymbol>WinMainCRTStartup</EntryPointSymbol>\n";

	properties << "\t\t</Link>\n"
	           << "\t\t<ResourceCompile>\n"
	           << "\t\t\t<AdditionalIncludeDirectories>.;" << prefix << ";%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>\n"
	           << "\t\t\t<PreprocessorDefinitions>" << definesList << "%(PreprocessorDefinitions)</PreprocessorDefinitions>\n"
	           << "\t\t</ResourceCompile>\n"
	           << "\t</ItemDefinitionGroup>\n"
	           << "</Project>\n";

	properties.flush();
}

void MSBuildProvider::createBuildProp(const BuildSetup &setup, bool isRelease, MSVC_Architecture arch, const std::string &configuration) {
	std::ofstream properties((setup.outputDir + '/' + setup.projectDescription + "_" + configuration + getMSVCArchName(arch) + getPropertiesExtension()).c_str());
	if (!properties || !properties.is_open()) {
		error("Could not open \"" + setup.outputDir + '/' + setup.projectDescription + "_" + configuration + getMSVCArchName(arch) + getPropertiesExtension() + "\" for writing");
		return;
	}

	properties << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
	           << "<Project DefaultTargets=\"Build\" ToolsVersion=\"" << _msvcVersion.project << "\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
	           << "\t<ImportGroup Label=\"PropertySheets\">\n"
	           << "\t\t<Import Project=\"" << setup.projectDescription << "_Global" << getMSVCArchName(arch) << ".props\" />\n"
	           << "\t</ImportGroup>\n"
	           << "\t<PropertyGroup>\n"
	           << "\t\t<_PropertySheetDisplayName>" << setup.projectDescription << "_" << configuration << getMSVCArchName(arch) << "</_PropertySheetDisplayName>\n"
	           << "\t\t<LinkIncremental>" << (isRelease ? "false" : "true") << "</LinkIncremental>\n"
	           << "\t\t<GenerateManifest>false</GenerateManifest>\n"
	           << "\t</PropertyGroup>\n"
	           << "\t<ItemDefinitionGroup>\n"
	           << "\t\t<ClCompile>\n";

	if (isRelease) {
		properties << "\t\t\t<IntrinsicFunctions>true</IntrinsicFunctions>\n"
		           << "\t\t\t<WholeProgramOptimization>true</WholeProgramOptimization>\n"
		           << "\t\t\t<PreprocessorDefinitions>WIN32;RELEASE_BUILD;%(PreprocessorDefinitions)</PreprocessorDefinitions>\n"
		           << "\t\t\t<StringPooling>true</StringPooling>\n"
		           << "\t\t\t<BufferSecurityCheck>false</BufferSecurityCheck>\n"
		           << "\t\t\t<DebugInformationFormat></DebugInformationFormat>\n"
		           << "\t\t\t<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>\n"
		           << "\t\t\t<EnablePREfast>" << (configuration == "Analysis" ? "true" : "false") << "</EnablePREfast>\n"
		           << "\t\t</ClCompile>\n"
		           << "\t\t<Lib>\n"
		           << "\t\t\t<LinkTimeCodeGeneration>true</LinkTimeCodeGeneration>\n"
		           << "\t\t</Lib>\n"
		           << "\t\t<Link>\n"
		           << "\t\t\t<LinkTimeCodeGeneration>UseLinkTimeCodeGeneration</LinkTimeCodeGeneration>\n"
		           << "\t\t\t<IgnoreSpecificDefaultLibraries>%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>\n"
		           << "\t\t\t<SetChecksum>true</SetChecksum>\n";
	} else {
		properties << "\t\t\t<Optimization>Disabled</Optimization>\n"
		           << "\t\t\t<PreprocessorDefinitions>WIN32;" << (configuration == "LLVM" ? "_CRT_SECURE_NO_WARNINGS;" : "") << "%(PreprocessorDefinitions)</PreprocessorDefinitions>\n"
		           << "\t\t\t<BasicRuntimeChecks>EnableFastChecks</BasicRuntimeChecks>\n"
		           << "\t\t\t<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>\n"
		           << "\t\t\t<FunctionLevelLinking>true</FunctionLevelLinking>\n"
		           << "\t\t\t<TreatWarningAsError>false</TreatWarningAsError>\n";
		if (_version >= 14) {
			// Since MSVC 2015 Edit and Continue is supported for x86 and x86-64, but not for ARM.
			properties << "\t\t\t<DebugInformationFormat>" << (arch != ARCH_ARM64 ? "EditAndContinue" : "ProgramDatabase") << "</DebugInformationFormat>\n";
		} else {
			// Older MSVC versions did not support Edit and Continue for x64, thus we do not use it.
			properties << "\t\t\t<DebugInformationFormat>" << (arch == ARCH_X86 ? "EditAndContinue" : "ProgramDatabase") << "</DebugInformationFormat>\n";
		}
		properties << "\t\t\t<EnablePREfast>" << (configuration == "Analysis" ? "true" : "false") << "</EnablePREfast>\n";

		if (configuration == "LLVM") {
			properties << "\t\t\t<AdditionalOptions>-Wno-microsoft -Wno-long-long -Wno-multichar -Wno-unknown-pragmas -Wno-reorder -Wpointer-arith -Wcast-qual -Wshadow -Wnon-virtual-dtor -Wwrite-strings -Wno-conversion -Wno-shorten-64-to-32 -Wno-sign-compare -Wno-four-char-constants -Wno-nested-anon-types -Qunused-arguments %(AdditionalOptions)</AdditionalOptions>\n";
		}

		properties << "\t\t</ClCompile>\n"
		           << "\t\t<Link>\n"
		           << "\t\t\t<GenerateDebugInformation>true</GenerateDebugInformation>\n"
		           << "\t\t\t<ImageHasSafeExceptionHandlers>false</ImageHasSafeExceptionHandlers>\n";
	}

	properties << "\t\t</Link>\n"
	           << "\t</ItemDefinitionGroup>\n"
	           << "</Project>\n";

	properties.flush();
	properties.close();
}

bool hasEnding(std::string const &fullString, std::string const &ending) {
	if (fullString.length() > ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	} else {
		return false;
	}
}

void MSBuildProvider::outputNasmCommand(std::ostream &projectFile, const std::string &config, const std::string &prefix) {
	projectFile << "\t\t\t<Command Condition=\"'$(Configuration)|$(Platform)'=='" << config << "|Win32'\">nasm.exe -f win32 -g -o \"$(IntDir)" << prefix << "%(Filename).obj\" \"%(FullPath)\"</Command>\n"
	            << "\t\t\t<Outputs Condition=\"'$(Configuration)|$(Platform)'=='" << config << "|Win32'\">$(IntDir)" << prefix << "%(Filename).obj;%(Outputs)</Outputs>\n";
	if (_version >= 15) {
		projectFile << "\t\t\t<OutputItemType Condition=\"'$(Configuration)|$(Platform)'=='" << config << "|Win32'\">Object</OutputItemType>\n"
		            << "\t\t\t<BuildInParallel Condition=\"'$(Configuration)|$(Platform)'=='" << config << "|Win32'\">true</BuildInParallel>\n";
	}
}

void MSBuildProvider::writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int,
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
	computeFileList(dir, objPrefix, filePrefix);
	_filters.pop_back(); // remove last empty filter

	// Output compile, include, other and resource files
	outputFiles(projectFile, _compileFiles, "ClCompile");
	outputFiles(projectFile, _includeFiles, "ClInclude");
	outputFiles(projectFile, _otherFiles, "None");
	outputFiles(projectFile, _resourceFiles, "ResourceCompile");

	// Output asm files
	if (!_asmFiles.empty()) {
		projectFile << "\t<ItemGroup>\n";
		for (std::list<FileEntry>::const_iterator entry = _asmFiles.begin(); entry != _asmFiles.end(); ++entry) {

			projectFile << "\t\t<CustomBuild Include=\"" << (*entry).path << "\">\n"
			            << "\t\t\t<FileType>Document</FileType>\n";

			outputNasmCommand(projectFile, "Debug", (*entry).prefix);
			outputNasmCommand(projectFile, "Analysis", (*entry).prefix);
			outputNasmCommand(projectFile, "Release", (*entry).prefix);
			outputNasmCommand(projectFile, "LLVM", (*entry).prefix);

			projectFile << "\t\t</CustomBuild>\n";
		}
		projectFile << "\t</ItemGroup>\n";
	}
}

void MSBuildProvider::outputFiles(std::ostream &projectFile, const FileEntries &files, const std::string &action) {
	if (!files.empty()) {
		projectFile << "\t<ItemGroup>\n";
		for (FileEntries::const_iterator entry = files.begin(), end = files.end(); entry != end; ++entry) {
			projectFile << "\t\t<" << action << " Include=\"" << (*entry).path << "\" />\n";
		}
		projectFile << "\t</ItemGroup>\n";
	}
}

void MSBuildProvider::computeFileList(const FileNode &dir, const std::string &objPrefix, const std::string &filePrefix) {
	for (FileNode::NodeList::const_iterator i = dir.children.begin(); i != dir.children.end(); ++i) {
		const FileNode *node = *i;

		if (!node->children.empty()) {
			// Update filter
			std::string _currentFilter = _filters.back();
			_filters.back().append((_filters.back() == "" ? "" : "\\") + node->name);

			computeFileList(*node, objPrefix + node->name + '_', filePrefix + node->name + '/');

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

} // namespace CreateProjectTool
