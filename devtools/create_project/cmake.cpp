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
#include "cmake.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iterator>

namespace CreateProjectTool {

CMakeProvider::CMakeProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version)
	: ProjectProvider(global_warnings, project_warnings, version) {
}

const CMakeProvider::Library *CMakeProvider::getLibraryFromFeature(const char *feature, bool useSDL2) const {
	static const Library s_libraries[] = {
		{ "sdl",        "sdl",               kSDLVersion1,   "FindSDL",      "SDL",      "SDL_INCLUDE_DIR",       "SDL_LIBRARY",         0            },
		{ "sdl",        "sdl2",              kSDLVersion2,   0,              "SDL2",     0,                       "SDL2_LIBRARIES",      0            },
		{ "freetype",   "freetype2",         kSDLVersionAny, "FindFreetype", "Freetype", "FREETYPE_INCLUDE_DIRS", "FREETYPE_LIBRARIES",  0            },
		{ "libz",       "zlib",              kSDLVersionAny, "FindZLIB",     "ZLIB",     "ZLIB_INCLUDE_DIRS",     "ZLIB_LIBRARIES",      0            },
		{ "png",        "libpng",            kSDLVersionAny, "FindPNG",      "PNG",      "PNG_INCLUDE_DIRS",      "PNG_LIBRARIES",       0            },
		{ "jpeg",       "libjpeg",           kSDLVersionAny, "FindJPEG",     "JPEG",     "JPEG_INCLUDE_DIRS",     "JPEG_LIBRARIES",      0            },
		{ "mpeg2",      "libmpeg2",          kSDLVersionAny, "FindMPEG2",    "MPEG2",    "MPEG2_INCLUDE_DIRS",    "MPEG2_mpeg2_LIBRARY", 0            },
		{ "flac",       "flac",              kSDLVersionAny, 0,              0,          0,                       0,                     "FLAC"       },
		{ "mad",        "mad",               kSDLVersionAny, 0,              0,          0,                       0,                     "mad"        },
		{ "ogg",        "ogg",               kSDLVersionAny, 0,              0,          0,                       0,                     "ogg"        },
		{ "vorbis",     "vorbisfile vorbis", kSDLVersionAny, 0,              0,          0,                       0,                     "vorbisfile vorbis" },
		{ "tremor",     "vorbisidec",        kSDLVersionAny, 0,              0,          0,                       0,                     "vorbisidec" },
		{ "theora",     "theoradec",         kSDLVersionAny, 0,              0,          0,                       0,                     "theoradec"  },
		{ "fluidsynth", "fluidsynth",        kSDLVersionAny, 0,              0,          0,                       0,                     "fluidsynth" },
		{ "faad",       "faad2",             kSDLVersionAny, 0,              0,          0,                       0,                     "faad"       },
		{ "fribidi",    "fribidi",           kSDLVersionAny, 0,              0,          0,                       0,                     "fribidi"    },
		{ "discord",    "discord",           kSDLVersionAny, 0,              0,          0,                       0,                     "discord-rpc"},
		{ "opengl",     0,                   kSDLVersionAny, "FindOpenGL",   "OpenGL",   "OPENGL_INCLUDE_DIR",    "OPENGL_gl_LIBRARY",   0            },
		{ "glew",       "glew",              kSDLVersionAny, "FindGLEW",     "GLEW",     "GLEW_INCLUDE_DIR",      "GLEW_LIBRARIES",      0            },
		{ "libcurl",    "libcurl",           kSDLVersionAny, "FindCURL",     "CURL",     "CURL_INCLUDE_DIRS",     "CURL_LIBRARIES",      0            },
		{ "sdlnet",     0,                   kSDLVersion1,   "FindSDL_net",  "SDL_net",  "SDL_NET_INCLUDE_DIRS",  "SDL_NET_LIBRARIES",   0            },
		{ "sdlnet",     "SDL2_net",          kSDLVersion2,   0,              0,          0,                       0,                     "SDL2_net"   }
	};

	for (unsigned int i = 0; i < sizeof(s_libraries) / sizeof(s_libraries[0]); i++) {
		bool matchingSDL = (s_libraries[i].sdlVersion == kSDLVersionAny)
		                   || (useSDL2 && s_libraries[i].sdlVersion == kSDLVersion2)
		                   || (!useSDL2 && s_libraries[i].sdlVersion == kSDLVersion1);
		if (std::strcmp(feature, s_libraries[i].feature) == 0 && matchingSDL) {
			return &s_libraries[i];
		}
	}

	return 0;
}

void CMakeProvider::createWorkspace(const BuildSetup &setup) {
	std::string filename = setup.outputDir + "/CMakeLists.txt";
	std::ofstream workspace(filename.c_str());
	if (!workspace || !workspace.is_open())
		error("Could not open \"" + filename + "\" for writing");

	workspace << "cmake_minimum_required(VERSION 3.2)\n";
	workspace << "project(" << setup.projectDescription << ")\n\n";

	workspace << "set(CMAKE_EXPORT_COMPILE_COMMANDS ON)\n";
	workspace << "find_package(PkgConfig QUIET)\n";
	workspace << "include(CMakeParseArguments)\n";
	workspace << "\n";
	workspace << "set(SCUMMVM_LIBS)\n";
	workspace << "\n";

	workspace << "macro(find_feature)\n";
	workspace << "\tset(_OPTIONS_ARGS)\n";
	workspace << "\tset(_ONE_VALUE_ARGS name findpackage_name include_dirs_var libraries_var)\n";
	workspace << "\tset(_MULTI_VALUE_ARGS pkgconfig_name libraries)\n";
	workspace << "\tcmake_parse_arguments(_feature \"${_OPTIONS_ARGS}\" \"${_ONE_VALUE_ARGS}\" \"${_MULTI_VALUE_ARGS}\" ${ARGN})\n";
	workspace << "\n";
	workspace << "\tif (_feature_pkgconfig_name AND PKG_CONFIG_FOUND)\n";
	workspace << "\t\tpkg_check_modules(${_feature_name} REQUIRED ${_feature_pkgconfig_name})\n";
	workspace << "\t\tinclude_directories(${${_feature_name}_INCLUDE_DIRS})\n";
	workspace << "\t\tlist(APPEND SCUMMVM_LIBS ${${_feature_name}_LIBRARIES})\n";
	workspace << "\tendif()\n\n";
	workspace << "\tif (NOT ${_feature_name}_FOUND)\n";
	workspace << "\t\tif (_feature_findpackage_name)\n";
	workspace << "\t\t\tfind_package(${_feature_findpackage_name} REQUIRED)\n";
	workspace << "\t\tendif()\n";
	workspace << "\t\tif (_feature_include_dirs_var)\n";
	workspace << "\t\t\tinclude_directories(${${_feature_include_dirs_var}} REQUIRED)\n";
	workspace << "\t\tendif()\n";
	workspace << "\t\tif (_feature_libraries_var)\n";
	workspace << "\t\t\tlist(APPEND SCUMMVM_LIBS ${${_feature_libraries_var}})\n";
	workspace << "\t\tendif()\n";
	workspace << "\t\tif (_feature_libraries)\n";
	workspace << "\t\t\tlist(APPEND SCUMMVM_LIBS ${_feature_libraries})\n";
	workspace << "\t\tendif()\n";
	workspace << "\tendif()\n";
	workspace << "endmacro()\n\n";

	workspace << "# Define the engines and subengines\n";
	writeEngines(setup, workspace);
	writeSubEngines(setup, workspace);
	workspace << "# Generate options for the engines\n";
	writeEngineOptions(workspace);

	workspace << "include_directories(${" << setup.projectDescription << "_SOURCE_DIR}/" <<  setup.filePrefix << " ${" << setup.projectDescription << "_SOURCE_DIR}/" <<  setup.filePrefix << "/engines "
			"$ENV{"<<LIBS_DEFINE<<"}/include .)\n\n";

	workspace << "# Libraries and features\n\n";
	writeFeatureLibSearch(setup, workspace, "sdl");

	workspace << "# Depending on how SDL2 was built, there can be either and imported target or flags variables\n";
	workspace << "# Define the flags variables from the imported target if necessary\n";
	workspace << "if (TARGET SDL2::SDL2)\n";
	workspace << "\tget_target_property(SDL2_INCLUDE_DIRS SDL2::SDL2 INTERFACE_INCLUDE_DIRECTORIES)\n";
	workspace << "\tget_target_property(SDL2_LIBRARIES SDL2::SDL2 LOCATION)\n";
	workspace << "endif()\n";
	workspace << "include_directories(${SDL2_INCLUDE_DIRS})\n\n";

	for (FeatureList::const_iterator i = setup.features.begin(), end = setup.features.end(); i != end; ++i) {
		if (!i->enable || featureExcluded(i->name)) continue;

		writeFeatureLibSearch(setup, workspace, i->name);
		workspace << "add_definitions(-D" << i->define << ")\n";
	}
	workspace << "\n";

	writeWarnings(workspace);
	writeDefines(setup, workspace);
	workspace << "# Generate definitions for the engines\n";
	writeEngineDefinitions(workspace);

	workspace << "# Generate \"engines/plugins_table.h\"\n";
	writeGeneratePluginsTable(workspace);
}

void CMakeProvider::writeFeatureLibSearch(const BuildSetup &setup, std::ofstream &workspace, const char *feature) const {
	const Library *library = getLibraryFromFeature(feature, setup.useSDL2);
	if (library) {
		workspace << "find_feature(";
		workspace << "name " << library->feature;
		workspace << " pkgconfig_name ";
		if (library->pkgConfig) {
			workspace << library->pkgConfig;
		} else {
			workspace << "IGNORE";
		}
		workspace << " findpackage_name ";
		if (library->package) {
			workspace << library->package;
		} else {
			workspace << "IGNORE";
		}
		workspace << " include_dirs_var ";
		if (library->includesVar) {
			workspace << library->includesVar;
		} else {
			workspace << "IGNORE";
		}
		workspace << " libraries_var ";
		if (library->librariesVar) {
			workspace << library->librariesVar;
		} else {
			workspace << "IGNORE";
		}
		workspace << " libraries ";
		if (library->libraries) {
			workspace << library->libraries;
		} else {
			workspace << "IGNORE";
		}
		workspace << ")\n";
	}
}

void CMakeProvider::writeEngines(const BuildSetup &setup, std::ofstream &workspace) const {
	workspace << "set(ENGINES";
	for (EngineDescList::const_iterator i = setup.engines.begin(), end = setup.engines.end(); i != end; ++i) {
		// We ignore all sub engines here because they require special handling.
		if (!i->enable || isSubEngine(i->name, setup.engines)) {
			continue;
		}

		std::string engineName;
		std::transform(i->name.begin(), i->name.end(), std::back_inserter(engineName), toupper);

		workspace << " " << engineName;
	}
	workspace << ")\n";
}

void CMakeProvider::writeSubEngines(const BuildSetup &setup, std::ofstream &workspace) const {
	for (EngineDescList::const_iterator i = setup.engines.begin(), end = setup.engines.end(); i != end; ++i) {
		// We ignore all sub engines here because they are handled in the inner loop
		if (!i->enable || isSubEngine(i->name, setup.engines) || i->subEngines.empty()) {
			continue;
		}

		std::string engineName;
		std::transform(i->name.begin(), i->name.end(), std::back_inserter(engineName), toupper);

		workspace << "set(SUB_ENGINES_" << engineName;
		for (StringList::const_iterator j = i->subEngines.begin(), subEnd = i->subEngines.end(); j != subEnd; ++j) {
			const EngineDesc &subEngine = findEngineDesc(*j, setup.engines);
			if (!subEngine.enable) continue;

			std::string subEngineName;
			std::transform(j->begin(), j->end(), std::back_inserter(subEngineName), toupper);

			workspace << " " << subEngineName;
		}
		workspace << ")\n";
	}

	workspace << "\n";
}

void CMakeProvider::createProjectFile(const std::string &name, const std::string &, const BuildSetup &setup, const std::string &moduleDir,
                                           const StringList &includeList, const StringList &excludeList) {

	const std::string projectFile = setup.outputDir + "/CMakeLists.txt";
	std::ofstream project(projectFile.c_str(), std::ofstream::out | std::ofstream::app);
	if (!project)
		error("Could not open \"" + projectFile + "\" for writing");

	bool addEnableCheck = true;
	if (name == setup.projectName) {
		project << "add_executable(" << name << "\n";
		addEnableCheck = false;
	} else if (name == setup.projectName + "-detection") {
		project << "list(APPEND SCUMMVM_LIBS " << name << ")\n";
		project << "add_library(" << name << "\n";
		addEnableCheck = false;
	} else {
		std::string engineName;
		std::transform(name.begin(), name.end(), std::back_inserter(engineName), toupper);

		project << "if (ENABLE_" << engineName << ")\n";
		project << "add_library(" << name << "\n";
	}

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

	project << ")\n";
	if (addEnableCheck) {
		project << "endif()\n";
	}
	project << "\n";

	if (name == setup.projectName) {
		project << "# Engines libraries handling\n";
		writeEnginesLibrariesHandling(setup, project);

		project << "# Libraries\n";
		const Library *sdlLibrary = getLibraryFromFeature("sdl", setup.useSDL2);
		project << "target_link_libraries(" << name << " ${" << sdlLibrary->librariesVar << "} ${SCUMMVM_LIBS})\n";

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
	output << "set(CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS}";
	for (StringList::const_iterator i = _globalWarnings.begin(); i != _globalWarnings.end(); ++i) {
		output << " " << *i;
	}
	output << "\")\n";
}

void CMakeProvider::writeDefines(const BuildSetup &setup, std::ofstream &output) const {
	output << "if (WIN32)\n";
	output << "\tadd_definitions(-DWIN32)\n";
	output << "else()\n";
	output << "\tadd_definitions(-DPOSIX)\n";
	output << "endif()\n";

	output << "add_definitions(-DSDL_BACKEND)\n";
	if (setup.useSDL2) {
		output << "add_definitions(-DUSE_SDL2)\n";
	}

	if (setup.useStaticDetection) {
		output << "add_definitions(-DDETECTION_STATIC)\n";
	}
}

void CMakeProvider::writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int indentation,
                                                const std::string &objPrefix, const std::string &filePrefix) {

	std::string lastName;
	for (FileNode::NodeList::const_iterator i = dir.children.begin(); i != dir.children.end(); ++i) {
		const FileNode *node = *i;

		if (!node->children.empty()) {
			writeFileListToProject(*node, projectFile, indentation + 1, objPrefix + node->name + '_', filePrefix + node->name + '/');
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

void CMakeProvider::writeGeneratePluginsTable(std::ofstream &workspace) const {
	workspace << "file(REMOVE \"engines/plugins_table.h\")\n";
	workspace << "file(APPEND \"engines/plugins_table.h\" \"/* This file is automatically generated by CMake */\\n\")\n";
	workspace << "foreach(ENGINE IN LISTS ENGINES)\n";
	workspace << "\tif (ENABLE_${ENGINE})\n";
	workspace << "\t\tfile(APPEND \"engines/plugins_table.h\" \"#if PLUGIN_ENABLED_STATIC(${ENGINE})\\n\")\n";
	workspace << "\t\tfile(APPEND \"engines/plugins_table.h\" \"LINK_PLUGIN(${ENGINE})\\n\")\n";
	workspace << "\t\tfile(APPEND \"engines/plugins_table.h\" \"#endif\\n\")\n";
	workspace << "\tendif()\n";
	workspace << "endforeach()\n\n";
}

void CMakeProvider::writeEnginesLibrariesHandling(const BuildSetup &setup, std::ofstream &workspace) const {
	workspace << "foreach(ENGINE IN LISTS ENGINES)\n";
	workspace << "\tif (ENABLE_${ENGINE})\n";
	workspace << "\t\tstring(TOLOWER ${ENGINE} ENGINE_LIB)\n\n";
	workspace << "\t\t# Enable C++11\n";
	workspace << "\t\tset_property(TARGET ${ENGINE_LIB} PROPERTY CXX_STANDARD 11)\n";
	workspace << "\t\tset_property(TARGET ${ENGINE_LIB} PROPERTY CXX_STANDARD_REQUIRED ON)\n\n";
	workspace << "\t\t# Link against the engine\n";
	workspace << "\t\ttarget_link_libraries("<< setup.projectName <<" ${ENGINE_LIB})\n";
	workspace << "\tendif()\n";
	workspace << "endforeach()\n\n";
}

void CMakeProvider::writeEngineDefinitions(std::ofstream &workspace) const {
	workspace << "foreach(ENGINE IN LISTS ENGINES)\n";
	workspace << "\tif (ENABLE_${ENGINE})\n";
	workspace << "\t\tadd_definitions(-DENABLE_${ENGINE})\n";
	workspace << "\t\tforeach(SUB_ENGINE IN LISTS SUB_ENGINES_${ENGINE})\n";
	workspace << "\t\t\tadd_definitions(-DENABLE_${SUB_ENGINE})\n";
	workspace << "\t\tendforeach(SUB_ENGINE)\n";
	workspace << "\tendif()\n";
	workspace << "endforeach()\n\n";
}

bool CMakeProvider::featureExcluded(const char *name) const {
	return std::strcmp(name, "nasm") == 0 ||
			std::strcmp(name, "updates") == 0 ; // NASM is not supported for now
}

const EngineDesc &CMakeProvider::findEngineDesc(const std::string &name, const EngineDescList &engines) const {
	for (EngineDescList::const_iterator i = engines.begin(), end = engines.end(); i != end; ++i) {
		if (i->name == name) {
			return *i;
		}

	}

	error("Unable to find requested engine");
}

} // End of CreateProjectTool namespace
