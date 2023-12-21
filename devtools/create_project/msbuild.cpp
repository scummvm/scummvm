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

#include "msbuild.h"
#include "config.h"

#include <algorithm>
#include <fstream>

namespace CreateProjectTool {

//////////////////////////////////////////////////////////////////////////
// MSBuild Provider (Visual Studio 2010 and later)
//////////////////////////////////////////////////////////////////////////

MSBuildProvider::MSBuildProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, StringList &global_errors, const int version, const MSVCVersion &msvc)
	: MSVCProvider(global_warnings, project_warnings, global_errors, version, msvc) {

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

inline void outputConfiguration(std::ostream &project, const std::string &config, MSVC_Architecture arch) {
	project << "\t\t<ProjectConfiguration Include=\"" << config << "|" << getMSVCConfigName(arch) << "\">\n"
	        << "\t\t\t<Configuration>" << config << "</Configuration>\n"
	        << "\t\t\t<Platform>" << getMSVCConfigName(arch) << "</Platform>\n"
	        << "\t\t</ProjectConfiguration>\n";
}

inline void outputConfigurationType(const BuildSetup &setup, std::ostream &project, const std::string &name, const std::string &config, MSVC_Architecture arch, const MSVCVersion &msvc) {
	project << "\t<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='" << config << "|" << getMSVCConfigName(arch) << "'\" Label=\"Configuration\">\n";
	if (name == setup.projectName || setup.devTools || setup.tests) {
		project << "\t\t<ConfigurationType>Application</ConfigurationType>\n";
	} else {
		project << "\t\t<ConfigurationType>StaticLibrary</ConfigurationType>\n";
	}
	project << "\t\t<PlatformToolset>" << (config == "LLVM" ? msvc.toolsetLLVM : msvc.toolsetMSVC ) << "</PlatformToolset>\n";
	project << "\t\t<CharacterSet>" << (setup.useWindowsUnicode ? "Unicode" : "NotSet") << "</CharacterSet>\n";
	if (msvc.version >= 16 && config == "Analysis") {
		project << "\t\t<EnableASAN>true</EnableASAN>\n";
	}	
	project << "\t</PropertyGroup>\n";
}

inline void outputProperties(const BuildSetup &setup, std::ostream &project, const std::string &config, MSVC_Architecture arch) {
	project << "\t<ImportGroup Condition=\"'$(Configuration)|$(Platform)'=='" << config << "|" << getMSVCConfigName(arch) << "'\" Label=\"PropertySheets\">\n"
	        << "\t\t<Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />\n"
	        << "\t\t<Import Project=\"" << setup.projectDescription + '_' << config << getMSVCArchName(arch) << ".props" << "\" />\n"
	        << "\t</ImportGroup>\n";
}

} // End of anonymous namespace

void MSBuildProvider::createProjectFile(const std::string &name, const std::string &uuid, const BuildSetup &setup, const std::string &moduleDir,
										const StringList &includeList, const StringList &excludeList, const std::string &pchIncludeRoot, const StringList &pchDirs, const StringList &pchExclude) {
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
		outputConfiguration(project, "Debug", *arch);
		outputConfiguration(project, "Analysis", *arch);
		outputConfiguration(project, "LLVM", *arch);
		outputConfiguration(project, "Release", *arch);
	}
	project << "\t</ItemGroup>\n";

	// Project name & Guid
	project << "\t<PropertyGroup Label=\"Globals\">\n"
	        << "\t\t<ProjectGuid>{" << uuid << "}</ProjectGuid>\n"
	        << "\t\t<RootNamespace>" << name << "</RootNamespace>\n"
	        << "\t\t<Keyword>Win32Proj</Keyword>\n"
	        << "\t\t<VCTargetsPath Condition=\"'$(VCTargetsPath" << _version << ")' != '' and '$(VSVersion)' == '' and $(VisualStudioVersion) == ''\">$(VCTargetsPath" << _version << ")</VCTargetsPath>\n"
	        << "\t</PropertyGroup>\n";

	// Shared configuration
	project << "\t<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\n";

	for (std::list<MSVC_Architecture>::const_iterator arch = _archs.begin(); arch != _archs.end(); ++arch) {
		outputConfigurationType(setup, project, name, "Release", *arch, _msvcVersion);
		outputConfigurationType(setup, project, name, "Analysis", *arch, _msvcVersion);
		outputConfigurationType(setup, project, name, "LLVM", *arch, _msvcVersion);
		outputConfigurationType(setup, project, name, "Debug", *arch, _msvcVersion);
	}

	project << "\t<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\n"
	        << "\t<ImportGroup Label=\"ExtensionSettings\">\n"
	        << "\t</ImportGroup>\n";

	for (std::list<MSVC_Architecture>::const_iterator arch = _archs.begin(); arch != _archs.end(); ++arch) {
		outputProperties(setup, project, "Release", *arch);
		outputProperties(setup, project, "Analysis", *arch);
		outputProperties(setup, project, "LLVM", *arch);
		outputProperties(setup, project, "Debug", *arch);
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
		addFilesToProject(moduleDir, project, includeList, excludeList, pchIncludeRoot, pchDirs, pchExclude, setup.filePrefix + '/' + modulePath);
	else
		addFilesToProject(moduleDir, project, includeList, excludeList, pchIncludeRoot, pchDirs, pchExclude, setup.filePrefix);

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
	if (!setup.devTools && !setup.tests && name != setup.projectName && !enableLanguageExtensions && !disableEditAndContinue && warningsIterator == _projectWarnings.end())
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
			        << "\t\t\t<Command>" << getPostBuildEvent(arch, setup, isRelease) << "</Command>\n"
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

void MSBuildProvider::outputGlobalPropFile(const BuildSetup &setup, std::ofstream &properties, MSVC_Architecture arch, const StringList &defines, const std::string &prefix) {

	std::string warnings;
	for (StringList::const_iterator i = _globalWarnings.begin(); i != _globalWarnings.end(); ++i)
		warnings += *i + ';';

	std::string warningsAsErrors;
	for (StringList::const_iterator i = _globalErrors.begin(); i != _globalErrors.end(); ++i)
		warningsAsErrors += "/we\"" + (*i) + "\" ";

	std::string definesList;
	for (StringList::const_iterator i = defines.begin(); i != defines.end(); ++i)
		definesList += *i + ';';

	// Add define to include revision header
	if (setup.runBuildEvents)
		definesList += REVISION_DEFINE ";";

	std::string includeDirsList;
	for (StringList::const_iterator i = setup.includeDirs.begin(); i != setup.includeDirs.end(); ++i)
		includeDirsList += convertPathToWin(*i) + ';';

	std::string libraryDirsList;
	for (StringList::const_iterator i = setup.libraryDirs.begin(); i != setup.libraryDirs.end(); ++i)
		libraryDirsList += convertPathToWin(*i) + ';';

	std::string includeSDL = (setup.useSDL2 ? "SDL2" : "SDL");
	properties << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
			   << "<Project DefaultTargets=\"Build\" ToolsVersion=\"" << _msvcVersion.project << "\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
			   << "\t<PropertyGroup>\n"
			   << "\t\t<_PropertySheetDisplayName>" << setup.projectDescription << "_Global</_PropertySheetDisplayName>\n";

	std::string libsPath;
	if (setup.libsDir.empty())
		libsPath = "$(" LIBS_DEFINE ")";
	else
		libsPath = convertPathToWin(setup.libsDir);

	if (!setup.useVcpkg) {
		properties << "\t\t<ExecutablePath>" << libsPath << "\\bin;" << libsPath << "\\bin\\" << getMSVCArchName(arch) << ";" << libsPath << "\\$(Configuration)\\bin;$(ExecutablePath)</ExecutablePath>\n"
				   << "\t\t<LibraryPath>" << libraryDirsList << libsPath << "\\lib\\" << getMSVCArchName(arch) << ";" << libsPath << "\\lib\\" << getMSVCArchName(arch) << "\\$(Configuration);" << libsPath << "\\lib;" << libsPath << "\\$(Configuration)\\lib;$(LibraryPath)</LibraryPath>\n"
				   << "\t\t<IncludePath>" << includeDirsList << libsPath << "\\include;" << libsPath << "\\include\\" << includeSDL << ";$(IncludePath)</IncludePath>\n";
	}
	properties << "\t\t<OutDir>$(Configuration)" << getMSVCArchName(arch) << "\\</OutDir>\n"
			   << "\t\t<IntDir>$(Configuration)" << getMSVCArchName(arch) << "\\$(ProjectName)\\</IntDir>\n"
			   << "\t</PropertyGroup>\n"
			   << "\t<ItemDefinitionGroup>\n"
			   << "\t\t<ClCompile>\n"
			   << "\t\t\t<DisableLanguageExtensions>true</DisableLanguageExtensions>\n"
			   << "\t\t\t<DisableSpecificWarnings>" << warnings << ";%(DisableSpecificWarnings)</DisableSpecificWarnings>\n"
			   << "\t\t\t<AdditionalIncludeDirectories>.;" << prefix << ";" << prefix << "\\engines;";
	if (setup.tests) {
		properties << prefix << "\\test\\cxxtest;";
	}
	// HACK to workaround SDL/SDL.h includes
	if (setup.useVcpkg) {
		properties << "$(_ZVcpkgCurrentInstalledDir)include\\" << includeSDL;
	}
	properties << "%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>\n"
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
			   << "\t\t\t<AdditionalOptions>/utf-8 " << (_msvcVersion.version >= 15 ? "/Zc:__cplusplus " : "") << warningsAsErrors << "%(AdditionalOptions)</AdditionalOptions>\n"
	           << "\t\t</ClCompile>\n"
	           << "\t\t<Link>\n"
	           << "\t\t\t<IgnoreSpecificDefaultLibraries>%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>\n";
	// console subsystem is required for text-console, tools, and tests
	if (!setup.useWindowsSubsystem || setup.featureEnabled("text-console") || setup.devTools || setup.tests) {
		properties << "\t\t\t<SubSystem>Console</SubSystem>\n";
	} else {
		properties << "\t\t\t<SubSystem>Windows</SubSystem>\n";
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
			   << "\t\t<LinkIncremental>" << ((isRelease || configuration == "Analysis") ? "false" : "true") << "</LinkIncremental>\n"
	           << "\t\t<GenerateManifest>false</GenerateManifest>\n";

	if (setup.useVcpkg) {
		properties << "\t\t<VcpkgTriplet>" << getMSVCArchName(arch) << "-windows</VcpkgTriplet>\n";
		properties << "\t\t<VcpkgConfiguration>" << (isRelease ? "Release" : "Debug") << "</VcpkgConfiguration>\n";
	}

	properties << "\t</PropertyGroup>\n"
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
		           << "\t\t\t<EnablePREfast>false</EnablePREfast>\n"
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
		// Since MSVC 2015 Edit and Continue is supported for x86 and x86-64, but not for ARM.
		if (configuration != "Analysis" && (arch == ARCH_X86 || (arch == ARCH_AMD64 && _version >= 14))) {
			properties << "\t\t\t<DebugInformationFormat>EditAndContinue</DebugInformationFormat>\n";
		} else {
			properties << "\t\t\t<DebugInformationFormat>ProgramDatabase</DebugInformationFormat>\n";
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

void MSBuildProvider::insertPathIntoDirectory(FileNode &dir, const std::string &path) {
	size_t separatorLoc = path.find('\\');
	if (separatorLoc != std::string::npos) {
		// Inside of a subdirectory

		std::string subdirName = path.substr(0, separatorLoc);

		FileNode::NodeList::iterator dirIt = dir.children.begin();
		FileNode::NodeList::iterator dirItEnd = dir.children.end();
		while (dirIt != dirItEnd) {
			if ((*dirIt)->name == subdirName)
				break;

			++dirIt;
		}

		FileNode *dirNode = nullptr;
		if (dirIt == dirItEnd) {
			dirNode = new FileNode(subdirName);
			dir.children.push_back(dirNode);
		} else {
			dirNode = *dirIt;
		}

		insertPathIntoDirectory(*dirNode, path.substr(separatorLoc + 1));
	} else {
		FileNode *fileNode = new FileNode(path);
		dir.children.push_back(fileNode);
	}
}

void MSBuildProvider::createFileNodesFromPCHList(FileNode &dir, const std::string &pathBase, const StringList &pchCompileFiles) {
	for (StringList::const_iterator it = pchCompileFiles.begin(), itEnd = pchCompileFiles.end(); it != itEnd; ++it) {
		const std::string &pchPath = *it;

		if (pchPath.size() > pathBase.size() && pchPath.substr(0, pathBase.size()) == pathBase) {
			std::string internalPath = pchPath.substr(pathBase.size());

			insertPathIntoDirectory(dir, internalPath);
		}
	}
}

void MSBuildProvider::writeFileListToProject(const FileNode &dir, std::ostream &projectFile, const int,
											 const std::string &objPrefix, const std::string &filePrefix,
											 const std::string &pchIncludeRoot, const StringList &pchDirs, const StringList &pchExclude) {
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

	StringList pchCompileFiles;

	// Output compile, include, other and resource files
	outputCompileFiles(projectFile, pchIncludeRoot, pchDirs, pchExclude, pchCompileFiles);
	outputFiles(projectFile, _includeFiles, "ClInclude");
	outputFiles(projectFile, _otherFiles, "None");
	outputFiles(projectFile, _resourceFiles, "ResourceCompile");

	if (pchCompileFiles.size() > 0) {
		// Generate filters and additional compile files for PCH files
		FileNode pchDir(dir.name);
		createFileNodesFromPCHList(pchDir, convertPathToWin(dir.name) + '\\', pchCompileFiles);

		StringList backupFilters = _filters;
		_filters.clear();

		_filters.push_back(""); // init filters
		computeFileList(pchDir, objPrefix, filePrefix);
		_filters.pop_back(); // remove last empty filter

		// Combine lists, removing duplicates
		for (StringList::const_iterator it = backupFilters.begin(), itEnd = backupFilters.end(); it != itEnd; ++it) {
			if (std::find(_filters.begin(), _filters.end(), *it) != _filters.end())
				_filters.push_back(*it);
		}
	}

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

void MSBuildProvider::outputCompileFiles(std::ostream &projectFile, const std::string &pchIncludeRoot, const StringList &pchDirs, const StringList &pchExclude, StringList &outPCHFiles) {
	const FileEntries &files = _compileFiles;

	const bool hasPCH = (pchDirs.size() > 0);

	std::string pchIncludeRootWin;
	StringList pchDirsWin;
	StringList pchExcludeWin;

	if (hasPCH) {
		pchIncludeRootWin = convertPathToWin(pchIncludeRoot);

		// Convert PCH paths to Win
		for (StringList::const_iterator entry = pchDirs.begin(), end = pchDirs.end(); entry != end; ++entry) {
			std::string convertedPath = convertPathToWin(*entry);
			if (convertedPath.size() < pchIncludeRootWin.size() || convertedPath.substr(0, pchIncludeRootWin.size()) != pchIncludeRootWin) {
				error("PCH path '" + convertedPath + "' wasn't located under PCH include root '" + pchIncludeRootWin + "'");
			}

			pchDirsWin.push_back(convertPathToWin(*entry));
		}
		for (StringList::const_iterator entry = pchExclude.begin(), end = pchExclude.end(); entry != end; ++entry) {
			const std::string path = *entry;

			if (path.size() >= 2 && path[path.size() - 1] == 'o' && path[path.size() - 2] == '.')
				pchExcludeWin.push_back(convertPathToWin(path.substr(0, path.size() - 2)));
		}
	}

	std::map<std::string, PCHInfo> pchMap;

	if (!files.empty()) {
		projectFile << "\t<ItemGroup>\n";
		for (FileEntries::const_iterator entry = files.begin(), end = files.end(); entry != end; ++entry) {
			std::string pchIncludePath, pchFilePath, pchFileName;

			bool fileHasPCH = false;
			if (hasPCH)
				fileHasPCH = calculatePchPaths(entry->path, pchIncludeRootWin, pchDirsWin, pchExcludeWin, '\\', pchIncludePath, pchFilePath, pchFileName);

			if (fileHasPCH) {
				std::string pchOutputFileName = "$(IntDir)dists\\msvc\\%(RelativeDir)" + pchFileName.substr(0, pchFileName.size() - 2) + ".pch";

				PCHInfo &pchInfo = pchMap[pchFilePath];
				pchInfo.file = pchIncludePath;
				pchInfo.outputFile = pchOutputFileName;

				projectFile << "\t\t<ClCompile Include=\"" << (*entry).path << "\">\n";
				projectFile << "\t\t\t<PrecompiledHeader>Use</PrecompiledHeader>\n";
				projectFile << "\t\t\t<PrecompiledHeaderFile>" << pchIncludePath << "</PrecompiledHeaderFile>\n";
				projectFile << "\t\t\t<PrecompiledHeaderOutputFile>" << pchOutputFileName << "</PrecompiledHeaderOutputFile>\n";
				projectFile << "\t\t</ClCompile>\n";
			} else {
				projectFile << "\t\t<ClCompile Include=\"" << (*entry).path << "\" />\n";
			}
		}

		// Flush PCH files
		for (std::map<std::string, PCHInfo>::const_iterator pchIt = pchMap.begin(), pchItEnd = pchMap.end(); pchIt != pchItEnd; ++pchIt) {
			const PCHInfo &pchInfo = pchIt->second;

			const std::string &filePath = pchIt->first;
			assert(filePath.size() >= 2 && filePath.substr(filePath.size() - 2) == ".h");

			std::string cppFilePath = filePath.substr(0, filePath.size() - 2) + ".cpp";

			std::string expectedContents = "/* This file is automatically generated by create_project */\n"
										   "/* DO NOT EDIT MANUALLY */\n"
										   "#include \"" + pchInfo.file + "\"\n";

			// Try to avoid touching the generated .cpp if it's identical to the expected output.
			// If we touch the file, then every file that includes PCH needs to be recompiled.
			std::ifstream pchInputFile(cppFilePath.c_str());
			bool needToEmit = true;
			if (pchInputFile && pchInputFile.is_open()) {
				std::string fileContents;
				for (;;) {
					char buffer[1024];
					size_t numRead = sizeof(buffer) - 1;
					pchInputFile.read(buffer, numRead);

					buffer[pchInputFile.gcount()] = '\0';

					fileContents += buffer;

					if (pchInputFile.eof() || pchInputFile.fail())
						break;

					if (fileContents.size() > expectedContents.size())
						break;
				}

				needToEmit = (fileContents != expectedContents);
				pchInputFile.close();
			}

			if (needToEmit) {
				std::ofstream pchOutputFile(cppFilePath.c_str());
				if (!pchOutputFile || !pchOutputFile.is_open()) {
					error("Could not open \"" + cppFilePath + "\" for writing");
					return;
				}

				pchOutputFile << expectedContents;
				pchOutputFile.close();
			}

			projectFile << "\t\t<ClCompile Include=\"" << cppFilePath << "\">\n";
			projectFile << "\t\t\t<PrecompiledHeader>Create</PrecompiledHeader>\n";
			projectFile << "\t\t\t<PrecompiledHeaderFile>" << pchInfo.file << "</PrecompiledHeaderFile>\n";
			projectFile << "\t\t\t<PrecompiledHeaderOutputFile>" << pchInfo.outputFile << "</PrecompiledHeaderOutputFile>\n";
			projectFile << "\t\t</ClCompile>\n";

			outPCHFiles.push_back(cppFilePath);
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
