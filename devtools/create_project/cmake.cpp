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
#include "cmake.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iterator>

namespace CreateProjectTool {

CMakeProvider::CMakeProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, StringList &global_errors, const int version)
	: ProjectProvider(global_warnings, project_warnings, global_errors, version) {
}

const CMakeProvider::Library *CMakeProvider::getLibraryFromFeature(const char *feature, bool useSDL2) const {
	static const Library s_libraries[] = {
		{ "sdl",        "sdl",               kSDLVersion1,   "FindSDL",      "SDL",      "SDL_INCLUDE_DIR",       "SDL_LIBRARY",         nullptr, nullptr },
		{ "sdl",        "sdl2",              kSDLVersion2,   nullptr,        "SDL2",     nullptr,                 "SDL2_LIBRARIES",      nullptr, nullptr },
		{ "freetype2",  "freetype2",         kSDLVersionAny, "FindFreetype", "Freetype", "FREETYPE_INCLUDE_DIRS", "FREETYPE_LIBRARIES",  nullptr, nullptr },
		{ "zlib",       "zlib",              kSDLVersionAny, "FindZLIB",     "ZLIB",     "ZLIB_INCLUDE_DIRS",     "ZLIB_LIBRARIES",      nullptr, nullptr },
		{ "png",        "libpng",            kSDLVersionAny, "FindPNG",      "PNG",      "PNG_INCLUDE_DIRS",      "PNG_LIBRARIES",       nullptr, nullptr },
		{ "jpeg",       "libjpeg",           kSDLVersionAny, "FindJPEG",     "JPEG",     "JPEG_INCLUDE_DIRS",     "JPEG_LIBRARIES",      nullptr, nullptr },
		{ "mpeg2",      "libmpeg2",          kSDLVersionAny, "FindMPEG2",    "MPEG2",    "MPEG2_INCLUDE_DIRS",    "MPEG2_mpeg2_LIBRARY", nullptr, nullptr },
		{ "opengl",     nullptr,             kSDLVersionAny, "FindOpenGL",   "OpenGL",   "OPENGL_INCLUDE_DIR",    "OPENGL_gl_LIBRARY",   nullptr, nullptr },
		{ "libcurl",    "libcurl",           kSDLVersionAny, "FindCURL",     "CURL",     "CURL_INCLUDE_DIRS",     "CURL_LIBRARIES",      nullptr, nullptr },
		{ "sdlnet",     nullptr,             kSDLVersion1,   "FindSDL_net",  "SDL_net",  "SDL_NET_INCLUDE_DIRS",  "SDL_NET_LIBRARIES",   nullptr, nullptr },
		LibraryProps("sdlnet", "SDL2_net", kSDLVersion2).Libraries("SDL2_net"),
		LibraryProps("flac", "flac").Libraries("FLAC"),
		LibraryProps("mad", "mad").Libraries("mad"),
		LibraryProps("mikmod", "mikmod").Libraries("mikmod"),
		LibraryProps("openmpt", "openmpt").Libraries("openmpt"),
		LibraryProps("ogg", "ogg").Libraries("ogg"),
		LibraryProps("vorbis", "vorbisfile vorbis").Libraries("vorbisfile vorbis"),
		LibraryProps("tremor", "vorbisidec").Libraries("vorbisidec"),
		LibraryProps("theoradec", "theoradec").Libraries("theoradec"),
		LibraryProps("vpx", "vpx").Libraries("vpx"),
		LibraryProps("fluidsynth", "fluidsynth").Libraries("fluidsynth"),
		LibraryProps("faad", "faad2").Libraries("faad"),
		LibraryProps("fribidi", "fribidi").Libraries("fribidi"),
		LibraryProps("discord", "discord").Libraries("discord-rpc"),
		LibraryProps("tts").WinLibraries("sapi ole32"),
		LibraryProps("enet").WinLibraries("winmm ws2_32"),
		LibraryProps("retrowave", "retrowave").Libraries("retrowave"),
		LibraryProps("a52", "a52").Libraries("a52"),
		LibraryProps("mpc", "mpcdec").Libraries("mpcdec")
	};

	for (unsigned int i = 0; i < sizeof(s_libraries) / sizeof(s_libraries[0]); i++) {
		bool matchingSDL = (s_libraries[i].sdlVersion == kSDLVersionAny)
		                   || (useSDL2 && s_libraries[i].sdlVersion == kSDLVersion2)
		                   || (!useSDL2 && s_libraries[i].sdlVersion == kSDLVersion1);
		if (std::strcmp(feature, s_libraries[i].feature) == 0 && matchingSDL) {
			return &s_libraries[i];
		}
	}

	return nullptr;
}

void CMakeProvider::createWorkspace(const BuildSetup &setup) {
	std::string filename = setup.outputDir + "/CMakeLists.txt";
	std::ofstream workspace(filename.c_str());
	if (!workspace || !workspace.is_open())
		error("Could not open \"" + filename + "\" for writing");

	workspace << "cmake_minimum_required(VERSION 3.13)\n";
	workspace << "project(" << setup.projectDescription << ")\n\n";

	workspace << R"(set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 11) # Globally enable C++11

find_package(PkgConfig QUIET)
include(CMakeParseArguments)

set(SCUMMVM_LIBS)

macro(find_feature)
	set(_OPTIONS_ARGS)
	set(_ONE_VALUE_ARGS name findpackage_name include_dirs_var libraries_var)
	set(_MULTI_VALUE_ARGS pkgconfig_name libraries win_libraries)
	cmake_parse_arguments(_feature "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN})

	if (_feature_pkgconfig_name AND PKG_CONFIG_FOUND)
		pkg_check_modules(${_feature_name} REQUIRED ${_feature_pkgconfig_name})
		include_directories(${${_feature_name}_INCLUDE_DIRS})
		list(APPEND SCUMMVM_LIBS ${${_feature_name}_LIBRARIES})
	endif()

	if (NOT ${_feature_name}_FOUND)
		if (_feature_findpackage_name)
			find_package(${_feature_findpackage_name} REQUIRED)
		endif()
		if (_feature_include_dirs_var)
			include_directories(${${_feature_include_dirs_var}} REQUIRED)
		endif()
		if (_feature_libraries_var)
			list(APPEND SCUMMVM_LIBS ${${_feature_libraries_var}})
		endif()
		if (_feature_libraries)
			list(APPEND SCUMMVM_LIBS ${_feature_libraries})
		endif()
		if (WIN32 AND _feature_win_libraries)
			list(APPEND SCUMMVM_LIBS ${_feature_win_libraries})
		endif()
	endif()
endmacro()

)";

	workspace << R"EOS(function(add_engine engine_name)
	string(TOUPPER ${engine_name} _engine_var)
	set(_enable_engine_var "ENABLE_${_engine_var}")
	if(NOT ${_enable_engine_var})
		return()
	endif()
	add_library(${engine_name} ${ARGN})

	# Generate definitions for the engine
	add_definitions(-D${_enable_engine_var})
	foreach(SUB_ENGINE IN LISTS SUB_ENGINES_${_engine_var})
		add_definitions(-DENABLE_${SUB_ENGINE})
	endforeach(SUB_ENGINE)
	file(APPEND "engines/plugins_table.h" "#if PLUGIN_ENABLED_STATIC(${_engine_var})\n")
	file(APPEND "engines/plugins_table.h" "LINK_PLUGIN(${_engine_var})\n")
	file(APPEND "engines/plugins_table.h" "#endif\n")

	# Enable C++11
	set_property(TARGET ${engine_name} PROPERTY CXX_STANDARD 11)
	set_property(TARGET ${engine_name} PROPERTY CXX_STANDARD_REQUIRED ON)

	# Link against the engine
	target_link_libraries()EOS" << setup.projectName << R"( ${engine_name})
endfunction()

)";

	workspace << "# Define the engines and subengines\n";
	writeEngines(setup, workspace);
	writeSubEngines(setup, workspace);
	workspace << "# Generate options for the engines\n";
	writeEngineOptions(workspace);

	std::string includeDirsList;
	for (const std::string &includeDir : setup.includeDirs)
		includeDirsList += includeDir + ' ';

	workspace << "include_directories(${" << setup.projectDescription << "_SOURCE_DIR}/" <<  setup.filePrefix << " ${" << setup.projectDescription << "_SOURCE_DIR}/" <<  setup.filePrefix << "/engines "
			  << includeDirsList << "$ENV{"<<LIBS_DEFINE<<"}/include .)\n\n";

	workspace << "# Libraries and features\n\n";
	writeFeatureLibSearch(setup, workspace, "sdl");

	workspace << R"(# Depending on how SDL2 was built, there can be either and imported target or flags variables
# Define the flags variables from the imported target if necessary
if (TARGET SDL2::SDL2)
	get_target_property(SDL2_INCLUDE_DIRS SDL2::SDL2 INTERFACE_INCLUDE_DIRECTORIES)
	get_target_property(SDL2_LIBRARIES SDL2::SDL2 LOCATION)
endif()
include_directories(${SDL2_INCLUDE_DIRS})

# Explicitly support MacPorts (hopefully harmless on other platforms)
link_directories(/opt/local/lib)

)";

	for (const Feature &feature : setup.features) {
		if (!feature.enable || featureExcluded(feature.name)) continue;

		writeFeatureLibSearch(setup, workspace, feature.name);

		if (!feature.define || !feature.define[0]) continue;
		workspace << "add_definitions(-D" << feature.define << ")\n";
	}
	workspace << "\n";

	writeWarnings(workspace);
	writeDefines(setup, workspace);

	workspace << R"(# Generate "engines/plugins_table.h"
file(REMOVE "engines/plugins_table.h")
file(APPEND "engines/plugins_table.h" "/* This file is automatically generated by CMake */\n")

)";
}

void CMakeProvider::writeFeatureLibSearch(const BuildSetup &setup, std::ofstream &workspace, const char *feature) const {
	const Library *library = getLibraryFromFeature(feature, setup.useSDL2);
	if (library) {
		workspace << "find_feature(";
		workspace << "name " << library->feature;
		if (library->pkgConfig) {
			workspace << " pkgconfig_name ";
			workspace << library->pkgConfig;
		}
		if (library->package) {
			workspace << " findpackage_name ";
			workspace << library->package;
		}
		if (library->includesVar) {
			workspace << " include_dirs_var ";
			workspace << library->includesVar;
		}
		if (library->librariesVar) {
			workspace << " libraries_var ";
			workspace << library->librariesVar;
		}
		if (library->libraries) {
			workspace << " libraries ";
			workspace << library->libraries;
		}
		if (library->winLibraries) {
			workspace << " win_libraries ";
			workspace << library->winLibraries;
		}
		workspace << ")\n";
	}
}

void CMakeProvider::writeEngines(const BuildSetup &setup, std::ofstream &workspace) const {
	workspace << "set(ENGINES";
	for (const EngineDesc &engine : setup.engines) {
		// We ignore all sub engines here because they require special handling.
		if (!engine.enable || isSubEngine(engine.name, setup.engines)) {
			continue;
		}

		workspace << " " << toUpper(engine.name);
	}
	workspace << ")\n";
}

void CMakeProvider::writeSubEngines(const BuildSetup &setup, std::ofstream &workspace) const {
	for (const EngineDesc &engine : setup.engines) {
		// We ignore all sub engines here because they are handled in the inner loop
		if (!engine.enable || isSubEngine(engine.name, setup.engines) || engine.subEngines.empty()) {
			continue;
		}

		workspace << "set(SUB_ENGINES_" << toUpper(engine.name);
		for (const std::string &subEngineName : engine.subEngines) {
			const EngineDesc &subEngine = findEngineDesc(subEngineName, setup.engines);
			if (!subEngine.enable) continue;
			workspace << " " << toUpper(subEngineName);
		}
		workspace << ")\n";
	}

	workspace << "\n";
}

static std::string filePrefix(const BuildSetup &setup, const std::string &moduleDir) {
	std::string modulePath;
	if (!moduleDir.compare(0, setup.srcDir.size(), setup.srcDir)) {
		modulePath = moduleDir.substr(setup.srcDir.size());
		if (!modulePath.empty() && modulePath.at(0) == '/')
			modulePath.erase(0, 1);
	}
	return modulePath.empty() ? setup.filePrefix : setup.filePrefix + '/' + modulePath;
}

void CMakeProvider::createProjectFile(const std::string &name, const std::string &, const BuildSetup &setup, const std::string &moduleDir,
									  const StringList &includeList, const StringList &excludeList, const std::string &pchIncludeRoot, const StringList &pchDirs, const StringList &pchExclude) {

	const std::string projectFile = setup.outputDir + "/CMakeLists.txt";
	std::ofstream project(projectFile.c_str(), std::ofstream::out | std::ofstream::app);
	if (!project)
		error("Could not open \"" + projectFile + "\" for writing");

	if (name == setup.projectName) {
		project << "add_executable(" << name;
		// console subsystem is required for text-console, tools, and tests
		if (setup.useWindowsSubsystem && !setup.featureEnabled("text-console") && !setup.devTools && !setup.tests) {
			project << " WIN32";
		}
		project << "\n";
	} else if (name == setup.projectName + "-detection") {
		project << "list(APPEND SCUMMVM_LIBS " << name << ")\n";
		project << "add_library(" << name << "\n";
	} else {
		enginesStr << "add_engine(" << name << "\n";
		addFilesToProject(moduleDir, enginesStr, includeList, excludeList, pchIncludeRoot, pchDirs, pchExclude, filePrefix(setup, moduleDir));
		enginesStr << ")\n\n";
		return;
	}

	addFilesToProject(moduleDir, project, includeList, excludeList, pchIncludeRoot, pchDirs, pchExclude, filePrefix(setup, moduleDir));

	project << ")\n";
	project << "\n";

	if (name == setup.projectName) {
		project << "# Engines libraries handling\n";
		writeEnginesLibrariesHandling(setup, project);

		project << "# Libraries\n";
		const Library *sdlLibrary = getLibraryFromFeature("sdl", setup.useSDL2);
		std::string libraryDirsList;
		for (const std::string &libraryDir : setup.libraryDirs)
			libraryDirsList += libraryDir + ' ';
		project << "target_link_libraries(" << name << " " << libraryDirsList << "${" << sdlLibrary->librariesVar << "} ${SCUMMVM_LIBS})\n";

		project << "if (WIN32)\n";
		project << "\ttarget_sources(" << name << " PUBLIC " << setup.filePrefix << "/dists/" << name << ".rc)\n";
		project << "\ttarget_link_libraries(" << name << " winmm)\n";
		project << "endif()\n";
		project << "\n";

		project << "set_property(TARGET " << name << " PROPERTY CXX_STANDARD 11)\n";
		project << "set_property(TARGET " << name << " PROPERTY CXX_STANDARD_REQUIRED ON)\n";
	}
}

void CMakeProvider::writeWarnings(std::ofstream &output) const {
	output << "if (MSVC)\n";
	// TODO: Support MSVC warnings
	output << "else()\n";
	output << "\tset(CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS}";
	for (const std::string &warning : _globalWarnings) {
		output << ' ' << warning;
	}
	output << "\")\n";
	output << "\tif(CMAKE_CXX_COMPILER_ID STREQUAL \"GNU\" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 12.0)\n";
	output << "\t\tset(CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} -Wno-address-of-packed-member\")\n";
	output << "\tendif()\n";
	output << "endif()\n";
}

void CMakeProvider::writeDefines(const BuildSetup &setup, std::ofstream &output) const {
	output << "if (WIN32)\n";
	output << "\tadd_definitions(-DWIN32)\n";
	output << "else()\n";
	output << "\tadd_definitions(-DPOSIX)\n";
	output << "\t# Hope for the best regarding fseeko and 64-bit off_t\n";
	output << "\tadd_definitions(-D_FILE_OFFSET_BITS=64)\n";
	output << "\tadd_definitions(-DHAS_FSEEKO_OFFT_64)\n";
	output << "endif()\n";

	output << "add_definitions(-DSDL_BACKEND)\n";
	if (setup.useSDL2) {
		output << "add_definitions(-DUSE_SDL2)\n";
	}

	if (getFeatureBuildState("opengl", setup.features)) {
		output << "add_definitions(-DUSE_GLAD)\n";
	}

	if (setup.useStaticDetection) {
		output << "add_definitions(-DDETECTION_STATIC)\n";
	}
}

void CMakeProvider::writeFileListToProject(const FileNode &dir, std::ostream &projectFile, const int indentation,
										   const std::string &objPrefix, const std::string &filePrefix,
										   const std::string &pchIncludeRoot, const StringList &pchDirs, const StringList &pchExclude) {

	std::string lastName;
	for (const FileNode *node : dir.children) {
		if (!node->children.empty()) {
			writeFileListToProject(*node, projectFile, indentation + 1, objPrefix + node->name + '_', filePrefix + node->name + '/', pchIncludeRoot, pchDirs, pchExclude);
		} else {
			std::string name, ext;
			splitFilename(node->name, name, ext);
			if (name != lastName) {
				if (!lastName.empty()) {
					projectFile << "\n";
				}
				projectFile << "\t";
			} else {
				projectFile << " ";
			}
			projectFile << filePrefix + node->name;
			lastName = name;
		}
	}
	projectFile << "\n";
}

const char *CMakeProvider::getProjectExtension() {
	return ".txt";
}

void CMakeProvider::writeEngineOptions(std::ofstream &workspace) const {
	workspace << "foreach(ENGINE IN LISTS ENGINES)\n";
	workspace << "\toption(ENABLE_${ENGINE} \"Enable ${ENGINE}\" ON)\n";
	workspace << "endforeach(ENGINE)\n\n";
}

void CMakeProvider::writeEnginesLibrariesHandling(const BuildSetup &setup, std::ofstream &workspace) const {
	workspace << enginesStr.str();

}

bool CMakeProvider::featureExcluded(const char *name) const {
	return std::strcmp(name, "nasm") == 0 ||
			std::strcmp(name, "updates") == 0 ; // NASM is not supported for now
}

const EngineDesc &CMakeProvider::findEngineDesc(const std::string &name, const EngineDescList &engines) const {
	for (const EngineDesc &engine : engines) {
		if (engine.name == name)
			return engine;
	}

	error("Unable to find requested engine");
}

} // End of CreateProjectTool namespace
