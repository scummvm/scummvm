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

#include "msvc.h"
#include "config.h"

#include <algorithm>
#include <cstring>
#include <fstream>

namespace CreateProjectTool {

//////////////////////////////////////////////////////////////////////////
// MSVC Provider (Base class)
//////////////////////////////////////////////////////////////////////////
MSVCProvider::MSVCProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, StringList &global_errors, const int version, const MSVCVersion &msvc)
	: ProjectProvider(global_warnings, project_warnings, global_errors), _version(version), _msvcVersion(msvc) {

	_enableLanguageExtensions = tokenize(ENABLE_LANGUAGE_EXTENSIONS, ',');
	_disableEditAndContinue = tokenize(DISABLE_EDIT_AND_CONTINUE, ',');

	// NASM not supported for Windows on AMD64 target
	StringList amd64_disabled_features;
	amd64_disabled_features.push_back("nasm");
	_arch_disabled_features[ARCH_AMD64] = amd64_disabled_features;
	// NASM not supported for WoA target
	StringList arm64_disabled_features;
	arm64_disabled_features.push_back("nasm");
	_arch_disabled_features[ARCH_ARM64] = arm64_disabled_features;
}

std::string MSVCProvider::getLibraryFromFeature(const char *feature, const BuildSetup &setup, bool isRelease) const {
	static const MSVCLibrary s_libraries[] = {
		// Libraries
		{       "sdl", "SDL.lib",                   "SDLd.lib",        kSDLVersion1,   "winmm.lib imm32.lib version.lib setupapi.lib"    },
		{       "sdl", "SDL2.lib",                  "SDL2d.lib",       kSDLVersion2,   "winmm.lib imm32.lib version.lib setupapi.lib"    },
		{       "sdl", "SDL3.lib",                  "SDL3d.lib",       kSDLVersion3,   "winmm.lib imm32.lib version.lib setupapi.lib"    },
		{      "zlib", "zlib.lib",                  "zlibd.lib",       kSDLVersionAny, nullptr                                           },
		{       "mad", "mad.lib",                   nullptr,           kSDLVersionAny, nullptr                                           },
		{   "fribidi", "fribidi.lib",               nullptr,           kSDLVersionAny, nullptr                                           },
		{       "ogg", "ogg.lib",                   nullptr,           kSDLVersionAny, nullptr                                           },
		{    "vorbis", "vorbis.lib vorbisfile.lib", nullptr,           kSDLVersionAny, nullptr                                           },
		{    "tremor", "vorbisidec.lib",            nullptr,           kSDLVersionAny, nullptr                                           },
		{      "flac", "FLAC.lib",                  nullptr,           kSDLVersionAny, nullptr                                           },
		{       "png", "libpng16.lib",              "libpng16d.lib",   kSDLVersionAny, nullptr                                           },
		{       "gif", "gif.lib",                   nullptr,           kSDLVersionAny, nullptr                                           },
		{      "faad", "faad.lib",                  nullptr,           kSDLVersionAny, nullptr                                           },
		{    "mikmod", "mikmod.lib",                nullptr,           kSDLVersionAny, nullptr                                           },
		{   "openmpt", "openmpt.lib",               nullptr,           kSDLVersionAny, nullptr                                           },
		{     "mpeg2", "mpeg2.lib",                 nullptr,           kSDLVersionAny, nullptr                                           },
		{ "theoradec", "theora.lib",                nullptr,           kSDLVersionAny, nullptr                                           },
		{       "vpx", "vpx.lib",                   nullptr,           kSDLVersionAny, nullptr                                           },
		{ "freetype2", "freetype.lib",              "freetyped.lib",   kSDLVersionAny, nullptr                                           },
		{      "jpeg", "jpeg.lib",                  nullptr,           kSDLVersionAny, nullptr                                           },
		{"fluidsynth", "fluidsynth.lib",            nullptr,           kSDLVersionAny, nullptr                                           },
		{ "fluidlite", "fluidlite.lib",             nullptr,           kSDLVersionAny, nullptr                                           },
		{   "libcurl", "libcurl.lib",               "libcurl-d.lib",   kSDLVersionAny, "ws2_32.lib wldap32.lib crypt32.lib normaliz.lib" },
		{    "sdlnet", "SDL_net.lib",               nullptr,           kSDLVersion1,   "iphlpapi.lib"                                    },
		{    "sdlnet", "SDL2_net.lib",              "SDL2_netd.lib",   kSDLVersion2,   "iphlpapi.lib"                                    },
		{    "sdlnet", "SDL3_net.lib",              "SDL3_netd.lib",   kSDLVersion3,   "iphlpapi.lib"                                    },
		{   "discord", "discord-rpc.lib",           nullptr,           kSDLVersionAny, nullptr                                           },
		{ "retrowave", "RetroWave.lib",             nullptr,           kSDLVersionAny, nullptr                                           },
		{       "a52", "a52.lib",                   nullptr,           kSDLVersionAny, nullptr                                           },
		{       "mpc", "libmpcdec.lib",             "libmpcdec_d.lib", kSDLVersionAny, nullptr                                           },
		// Feature flags with library dependencies
		{   "updates", "WinSparkle.lib",            nullptr,           kSDLVersionAny, nullptr                                           },
		{       "tts", nullptr,                     nullptr,           kSDLVersionAny, "sapi.lib"                                        },
		{    "opengl", nullptr,                     nullptr,           kSDLVersionAny, nullptr                                           },
		{      "enet", nullptr,                     nullptr,           kSDLVersionAny, "winmm.lib ws2_32.lib"                            }
	};

	const MSVCLibrary *library = nullptr;
	for (unsigned int i = 0; i < sizeof(s_libraries) / sizeof(s_libraries[0]); i++) {
		if (std::strcmp(feature, s_libraries[i].feature) == 0 &&
			((s_libraries[i].sdl == kSDLVersionAny) ||
			(s_libraries[i].sdl == setup.useSDL))) {
			library = &s_libraries[i];
			break;
		}
	}

	std::string libs;
	if (library) {
		// Dependencies come first
		if (library->depends) {
			libs += library->depends;
			libs += " ";
		}

		// Vcpkg already adds the libs
		if (!setup.useVcpkg) {
			const char *basename = library->release;
			// Debug name takes priority
			if (!isRelease && library->debug) {
				basename = library->debug;
			}
			if (basename) {
				libs += basename;
			}
		}
	}

	return libs;
}

std::string MSVCProvider::outputLibraryDependencies(const BuildSetup &setup, bool isRelease) const {
	std::string libs;

	// SDL is always enabled
	libs += getLibraryFromFeature("sdl", setup, isRelease);
	libs += " ";
	for (FeatureList::const_iterator i = setup.features.begin(); i != setup.features.end(); ++i) {
		if (i->enable) {
			std::string lib = getLibraryFromFeature(i->name, setup, isRelease);
			if (!lib.empty())
				libs += lib + " ";
		}
	}

	return libs;
}

void MSVCProvider::createWorkspace(const BuildSetup &setup) {
	if (setup.useSlnx)
		createWorkspaceXml(setup);
	else
		createWorkspaceClassic(setup);
}

void MSVCProvider::createWorkspaceClassic(const BuildSetup &setup) {
	UUIDMap::const_iterator svmUUID = _allProjUuidMap.find(setup.projectName);
	if (svmUUID == _allProjUuidMap.end())
		error("No UUID for \"" + setup.projectName + "\" project created");

	const std::string svmProjectUUID = svmUUID->second;
	assert(!svmProjectUUID.empty());

	std::string solutionUUID = createUUID(setup.projectName + ".sln");

	std::ofstream solution((setup.outputDir + '/' + setup.projectName + ".sln").c_str());
	if (!solution || !solution.is_open()) {
		error("Could not open \"" + setup.outputDir + '/' + setup.projectName + ".sln\" for writing");
		return;
	}

	solution << "Microsoft Visual Studio Solution File, Format Version " << _msvcVersion.solutionFormat << "\n";
	solution << "# Visual Studio " << _msvcVersion.solutionVersion << "\n";
	if (_version >= 12) {
		solution << "VisualStudioVersion = " << _msvcVersion.project << ".0.0\n";
		solution << "MinimumVisualStudioVersion = 10.0.40219.1\n";
	}

	// Write main project
	if (!setup.devTools) {
		solution << "Project(\"{" << solutionUUID << "}\") = \"" << setup.projectName << "\", \"" << setup.projectName << getProjectExtension() << "\", \"{" << svmProjectUUID << "}\"\n";

		// Project dependencies are moved to vcxproj files in Visual Studio 2010
		if (_version < 10)
			writeReferences(setup, solution);

		solution << "EndProject\n";
	}

	// Note we assume that the UUID map only includes UUIDs for enabled engines!
	for (UUIDMap::const_iterator i = _engineUuidMap.begin(); i != _engineUuidMap.end(); ++i) {
		solution << "Project(\"{" << solutionUUID << "}\") = \"" << i->first << "\", \"" << i->first << getProjectExtension() << "\", \"{" << i->second << "}\"\n"
		         << "EndProject\n";
	}

	solution << "Global\n"
	            "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n";

	for (std::list<MSVC_Architecture>::const_iterator arch = _archs.begin(); arch != _archs.end(); ++arch) {
		solution << "\t\tDebug|" << getMSVCConfigName(*arch) << " = Debug|" << getMSVCConfigName(*arch) << "\n"
		         << "\t\tASan|" << getMSVCConfigName(*arch) << " = ASan|" << getMSVCConfigName(*arch) << "\n"
		         << "\t\tLLVM|" << getMSVCConfigName(*arch) << " = LLVM|" << getMSVCConfigName(*arch) << "\n"
		         << "\t\tRelease|" << getMSVCConfigName(*arch) << " = Release|" << getMSVCConfigName(*arch) << "\n";
	}

	solution << "\tEndGlobalSection\n"
	            "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n";

	for (UUIDMap::const_iterator i = _allProjUuidMap.begin(); i != _allProjUuidMap.end(); ++i) {
		for (std::list<MSVC_Architecture>::const_iterator arch = _archs.begin(); arch != _archs.end(); ++arch) {
			solution << "\t\t{" << i->second << "}.Debug|" << getMSVCConfigName(*arch) << ".ActiveCfg = Debug|" << getMSVCConfigName(*arch) << "\n"
			         << "\t\t{" << i->second << "}.Debug|" << getMSVCConfigName(*arch) << ".Build.0 = Debug|" << getMSVCConfigName(*arch) << "\n"
			         << "\t\t{" << i->second << "}.ASan|" << getMSVCConfigName(*arch) << ".ActiveCfg = ASan|" << getMSVCConfigName(*arch) << "\n"
			         << "\t\t{" << i->second << "}.ASan|" << getMSVCConfigName(*arch) << ".Build.0 = ASan|" << getMSVCConfigName(*arch) << "\n"
			         << "\t\t{" << i->second << "}.LLVM|" << getMSVCConfigName(*arch) << ".ActiveCfg = LLVM|" << getMSVCConfigName(*arch) << "\n"
			         << "\t\t{" << i->second << "}.LLVM|" << getMSVCConfigName(*arch) << ".Build.0 = LLVM|" << getMSVCConfigName(*arch) << "\n"
			         << "\t\t{" << i->second << "}.Release|" << getMSVCConfigName(*arch) << ".ActiveCfg = Release|" << getMSVCConfigName(*arch) << "\n"
			         << "\t\t{" << i->second << "}.Release|" << getMSVCConfigName(*arch) << ".Build.0 = Release|" << getMSVCConfigName(*arch) << "\n";
		}
	}

	solution << "\tEndGlobalSection\n"
	         << "\tGlobalSection(SolutionProperties) = preSolution\n"
	         << "\t\tHideSolutionNode = FALSE\n"
	         << "\tEndGlobalSection\n"
	         << "EndGlobal\n";
}

void MSVCProvider::createWorkspaceXml(const BuildSetup &setup) {
	const auto svmUUID = _allProjUuidMap.find(setup.projectName);
	if (svmUUID == _allProjUuidMap.end())
		error("No UUID for \"" + setup.projectName + "\" project created");

	const std::string &svmProjectUUID = svmUUID->second;
	assert(!svmProjectUUID.empty());
	
	std::ofstream solution((setup.outputDir + '/' + setup.projectName + ".slnx").c_str());
	if (!solution || !solution.is_open()) {
		error("Could not open \"" + setup.outputDir + '/' + setup.projectName + ".slnx\" for writing");
		return;
	}

	solution << "<Solution>\n";

	solution << "\t<Configurations>\n";

	solution << "\t\t<BuildType Name=\"ASan\" />\n";
	solution << "\t\t<BuildType Name=\"Debug\" />\n";
	solution << "\t\t<BuildType Name=\"LLVM\" />\n";
	solution << "\t\t<BuildType Name=\"Release\" />\n";
	
	for (const auto &arch : _archs) {
		solution << "\t\t<Platform Name=\"" << getMSVCConfigName(arch) << "\" />\n"; 
	}
	solution << "\t</Configurations>\n";

	// Write main project
	if (!setup.devTools) {
		solution << "\t<Project Path=\"" << setup.projectName << getProjectExtension()
			<< "\" Id=\"" << svmProjectUUID << "\" "
			<< " DefaultStartup=\"true\""  /* DefaultStartup has no effect in VS2022, needs VS2026+ */
			<< " />\n";
	}

	for (const auto &engineUuid : _engineUuidMap) {
		solution << "\t<Project Path=\"" << engineUuid.first << getProjectExtension()
			<< "\" Id=\"" << engineUuid.second
			<< "\" />\n";
	}
	
	solution << "</Solution>\n";
}

void MSVCProvider::createOtherBuildFiles(const BuildSetup &setup) {
	// Create the global property file
	createGlobalProp(setup);

	// Create the configuration property files (for Debug and Release with 32 and 64bits versions)
	// Note: we use the debug properties for the asan configuration
	for (std::list<MSVC_Architecture>::const_iterator arch = _archs.begin(); arch != _archs.end(); ++arch) {
		createBuildProp(setup, true, *arch, "Release");
		createBuildProp(setup, false, *arch, "Debug");
		createBuildProp(setup, false, *arch, "ASan");
		createBuildProp(setup, false, *arch, "LLVM");
	}
}

void MSVCProvider::addResourceFiles(const BuildSetup &setup, StringList &includeList, StringList &excludeList) {
	includeList.push_back(setup.srcDir + "/icons/" + setup.projectName + ".ico");
	includeList.push_back(setup.srcDir + "/dists/" + setup.projectName + ".rc");
}

void MSVCProvider::createGlobalProp(const BuildSetup &setup) {
	for (std::list<MSVC_Architecture>::const_iterator arch = _archs.begin(); arch != _archs.end(); ++arch) {
		std::ofstream properties((setup.outputDir + '/' + setup.projectDescription + "_Global" + getMSVCArchName(*arch) + getPropertiesExtension()).c_str());
		if (!properties)
			error("Could not open \"" + setup.outputDir + '/' + setup.projectDescription + "_Global" + getMSVCArchName(*arch) + getPropertiesExtension() + "\" for writing");

		BuildSetup archSetup = setup;
		std::map<MSVC_Architecture, StringList>::const_iterator arch_disabled_features_it = _arch_disabled_features.find(*arch);
		if (arch_disabled_features_it != _arch_disabled_features.end()) {
			for (StringList::const_iterator feature = arch_disabled_features_it->second.begin(); feature != arch_disabled_features_it->second.end(); ++feature) {
				archSetup = removeFeatureFromSetup(archSetup, *feature);
			}
		}

		outputGlobalPropFile(archSetup, properties, *arch, archSetup.defines, convertPathToWin(archSetup.filePrefix));
		properties.close();
	}
}

std::string MSVCProvider::getPreBuildEvent(const BuildSetup &setup) const {
	std::string cmdLine = "";

	cmdLine = "@echo off\n"
	          "echo Executing Pre-Build script...\n"
	          "echo.\n"
	          "@call &quot;$(SolutionDir)" + setup.filePrefix + "/devtools/create_project/scripts/prebuild.cmd&quot; &quot;$(SolutionDir)/" + setup.filePrefix + "&quot; &quot;$(SolutionDir)&quot;\n"
	          "EXIT /B0";

	return cmdLine;
}

std::string MSVCProvider::getTestPreBuildEvent(const BuildSetup &setup) const {
	// Build list of folders containing tests
	std::string target = "";

	for (StringList::const_iterator it = setup.testDirs.begin(); it != setup.testDirs.end(); ++it)
		target += " $(SolutionDir)" + *it + "*.h";

	std::string cmdLine = "";
	cmdLine = "if not exist \"$(SolutionDir)test\\runner\" mkdir \"$(SolutionDir)test\\runner\"\n"
	          "python3 &quot;$(SolutionDir)" + setup.filePrefix + "/test/cxxtest/cxxtestgen.py&quot; --runner=ParenPrinter --no-std --no-eh -o &quot;$(SolutionDir)test/runner/test_runner.cpp&quot;" + target;
	return cmdLine;
}

std::string MSVCProvider::getPostBuildEvent(MSVC_Architecture arch, const BuildSetup &setup, bool isRelease) const {
	std::string cmdLine = "";

	cmdLine = "@echo off\n"
	          "echo Executing Post-Build script...\n"
	          "echo.\n"
	          "@call &quot;$(SolutionDir)" + setup.filePrefix + "/devtools/create_project/scripts/postbuild.cmd&quot; &quot;$(SolutionDir)" + setup.filePrefix + "&quot; &quot;$(OutDir)&quot; ";

	cmdLine += setup.getSDLName();

	if (setup.useVcpkg) {
		cmdLine += " &quot;$(_ZVcpkgCurrentInstalledDir)";
		if (!isRelease) {
			cmdLine += "debug/";
		}
		cmdLine += "bin/&quot; ";
	} else {
		std::string libsPath;
		if (setup.libsDir.empty())
			libsPath = "%" LIBS_DEFINE "%";
		else
			libsPath = convertPathToWin(setup.libsDir);

		cmdLine += " &quot;";
		cmdLine += libsPath;
		cmdLine += "/lib/";
		cmdLine += getMSVCArchName(arch);
		cmdLine += "/$(Configuration)&quot; ";
	}

	// Specify if installer needs to be built or not
	cmdLine += (setup.createInstaller ? "1" : "0");

	cmdLine += "\n"
	           "EXIT /B0";

	return cmdLine;
}

} // namespace CreateProjectTool
