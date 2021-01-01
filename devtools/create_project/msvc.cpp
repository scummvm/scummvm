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

#include "msvc.h"
#include "config.h"

#include <algorithm>
#include <cstring>
#include <fstream>

namespace CreateProjectTool {

//////////////////////////////////////////////////////////////////////////
// MSVC Provider (Base class)
//////////////////////////////////////////////////////////////////////////
MSVCProvider::MSVCProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version, const MSVCVersion &msvc)
    : ProjectProvider(global_warnings, project_warnings, version), _msvcVersion(msvc) {

	_enableLanguageExtensions = tokenize(ENABLE_LANGUAGE_EXTENSIONS, ',');
	_disableEditAndContinue = tokenize(DISABLE_EDIT_AND_CONTINUE, ',');

	// NASM not supported for Windows on AMD64 target
	StringList amd64_disabled_features;
	amd64_disabled_features.push_back("nasm");
	_arch_disabled_features[ARCH_AMD64] = amd64_disabled_features;
	// NASM not supported for WoA target
	// No OpenGL on Windows on ARM
	// https://github.com/microsoft/vcpkg/issues/11248 [fribidi] Fribidi doesn't cross-compile on x86-64 to target arm/arm64
	StringList arm64_disabled_features;
	arm64_disabled_features.push_back("nasm");
	arm64_disabled_features.push_back("opengl");
	arm64_disabled_features.push_back("fribidi");
	_arch_disabled_features[ARCH_ARM64] = arm64_disabled_features;
}

std::string MSVCProvider::getLibraryFromFeature(const char *feature, const BuildSetup &setup, bool isRelease) const {
	static const MSVCLibrary s_libraries[] = {
		// Libraries
		{       "sdl", "SDL.lib",                   "SDLd.lib",      "winmm.lib imm32.lib version.lib setupapi.lib",    0 },
		{      "sdl2", "SDL2.lib",                  "SDL2d.lib",     "winmm.lib imm32.lib version.lib setupapi.lib",    0 },
		{      "libz", "zlib.lib",                  "zlibd.lib",     0,                                                 0 },
		{       "mad", "mad.lib",                   0,               0,                                                 "libmad.lib" },
		{   "fribidi", "libfribidi.lib",            0,               0,                                                 0 },
		{       "ogg", "ogg.lib",                   0,               0,                                                 "libogg_static.lib" },
		{    "vorbis", "vorbis.lib vorbisfile.lib", 0,               0,                                                 "libvorbisfile_static.lib libvorbis_static.lib" },
		{      "flac", "FLAC.lib",                  0,               0,                                                 "libFLAC_static.lib win_utf8_io_static.lib" },
		{       "png", "libpng16.lib",              "libpng16d.lib", 0,                                                 0 },
		{      "faad", "faad.lib",                  0,               0,                                                 "libfaad.lib" },
		{     "mpeg2", "mpeg2.lib",                 0,               0,                                                 "libmpeg2.lib" },
		{    "theora", "theora.lib",                0,               0,                                                 "libtheora_static.lib" },
		{  "freetype", "freetype.lib",              "freetyped.lib", 0,                                                 0 },
		{      "jpeg", "jpeg.lib",                  "jpegd.lib",     0,                                                 "jpeg-static.lib" },
		{"fluidsynth", "fluidsynth.lib",            0,               0,                                                 "libfluidsynth.lib" },
		{   "libcurl", "libcurl.lib",               "libcurl-d.lib", "ws2_32.lib wldap32.lib crypt32.lib normaliz.lib", 0 },
		{    "sdlnet", "SDL_net.lib",               0,               "iphlpapi.lib",                                    0 },
		{   "sdl2net", "SDL2_net.lib",              0,               "iphlpapi.lib",                                    "SDL_net.lib" },
		{   "discord", "discord-rpc.lib",           0,               0,                                                 0 },
		{      "glew", "glew32.lib",                "glew32d.lib",   0,                                                 0 },
		// Feature flags with library dependencies
		{   "updates", "winsparkle.lib",            0,               0,                                                 0 },
		{       "tts", 0,                           0,               "sapi.lib",                                        0 },
		{    "opengl", 0,                           0,               "opengl32.lib",                                    0 }
	};

	// HACK for switching SDL_net to SDL2_net
	const char *sdl2net = "sdl2net";
	if (std::strcmp(feature, "sdlnet") == 0 && setup.useSDL2) {
		feature = sdl2net;
	}

	const MSVCLibrary *library = 0;
	for (unsigned int i = 0; i < sizeof(s_libraries) / sizeof(s_libraries[0]); i++) {
		if (std::strcmp(feature, s_libraries[i].feature) == 0) {
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

		const char *basename = library->release;
		if (setup.useCanonicalLibNames) {
			// Debug name takes priority
			if (!isRelease && library->debug) {
				basename = library->debug;
			}
		} else {
			// Legacy name ignores configuration
			if (library->legacy) {
				basename = library->legacy;
			}
		}
		if (basename) {
			libs += basename;
		}
	}
	
	return libs;
}

std::string MSVCProvider::outputLibraryDependencies(const BuildSetup &setup, bool isRelease) const {
	std::string libs;

	if (setup.useSDL2) {
		libs += getLibraryFromFeature("sdl2", setup, isRelease);
	} else {
		libs += getLibraryFromFeature("sdl", setup, isRelease);
	}
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
		         << "\t\tAnalysis|" << getMSVCConfigName(*arch) << " = Analysis|" << getMSVCConfigName(*arch) << "\n"
		         << "\t\tLLVM|" << getMSVCConfigName(*arch) << " = LLVM|" << getMSVCConfigName(*arch) << "\n"
		         << "\t\tRelease|" << getMSVCConfigName(*arch) << " = Release|" << getMSVCConfigName(*arch) << "\n";
	}

	solution << "\tEndGlobalSection\n"
	            "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n";

	for (UUIDMap::const_iterator i = _allProjUuidMap.begin(); i != _allProjUuidMap.end(); ++i) {
		for (std::list<MSVC_Architecture>::const_iterator arch = _archs.begin(); arch != _archs.end(); ++arch) {
			solution << "\t\t{" << i->second << "}.Debug|" << getMSVCConfigName(*arch) << ".ActiveCfg = Debug|" << getMSVCConfigName(*arch) << "\n"
			         << "\t\t{" << i->second << "}.Debug|" << getMSVCConfigName(*arch) << ".Build.0 = Debug|" << getMSVCConfigName(*arch) << "\n"
			         << "\t\t{" << i->second << "}.Analysis|" << getMSVCConfigName(*arch) << ".ActiveCfg = Analysis|" << getMSVCConfigName(*arch) << "\n"
			         << "\t\t{" << i->second << "}.Analysis|" << getMSVCConfigName(*arch) << ".Build.0 = Analysis|" << getMSVCConfigName(*arch) << "\n"
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

void MSVCProvider::createOtherBuildFiles(const BuildSetup &setup) {
	// Create the global property file
	createGlobalProp(setup);

	// Create the configuration property files (for Debug and Release with 32 and 64bits versions)
	// Note: we use the debug properties for the analysis configuration
	for (std::list<MSVC_Architecture>::const_iterator arch = _archs.begin(); arch != _archs.end(); ++arch) {
		createBuildProp(setup, true, *arch, "Release");
		createBuildProp(setup, false, *arch, "Debug");
		createBuildProp(setup, false, *arch, "Analysis");
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

		outputGlobalPropFile(archSetup, properties, *arch, archSetup.defines, convertPathToWin(archSetup.filePrefix), archSetup.runBuildEvents);
		properties.close();
	}
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

} // namespace CreateProjectTool
