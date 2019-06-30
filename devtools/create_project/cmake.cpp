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
		{ "sdl",       kSDLVersion1,   "FindSDL",      "SDL",      "SDL_INCLUDE_DIR",       "SDL_LIBRARY",         0            },
		{ "sdl",       kSDLVersion2,   0,              "SDL2",     0,                       "SDL2_LIBRARIES",      0            },
		{ "freetype",  kSDLVersionAny, "FindFreetype", "Freetype", "FREETYPE_INCLUDE_DIRS", "FREETYPE_LIBRARIES",  0            },
		{ "libz",      kSDLVersionAny, "FindZLIB",     "ZLIB",     "ZLIB_INCLUDE_DIRS",     "ZLIB_LIBRARIES",      0            },
		{ "png",       kSDLVersionAny, "FindPNG",      "PNG",      "PNG_INCLUDE_DIRS",      "PNG_LIBRARIES",       0            },
		{ "jpeg",      kSDLVersionAny, "FindJPEG",     "JPEG",     "JPEG_INCLUDE_DIRS",     "JPEG_LIBRARIES",      0            },
		{ "mpeg2",     kSDLVersionAny, "FindMPEG2",    "MPEG2",    "MPEG2_INCLUDE_DIRS",    "MPEG2_mpeg2_LIBRARY", 0            },
		{ "flac",      kSDLVersionAny, 0,              0,          0,                       0,                     "FLAC"       },
		{ "mad",       kSDLVersionAny, 0,              0,          0,                       0,                     "mad"        },
		{ "ogg",       kSDLVersionAny, 0,              0,          0,                       0,                     "ogg"        },
		{ "vorbis",    kSDLVersionAny, 0,              0,          0,                       0,                     "vorbisfile vorbis" },
		{ "tremor",    kSDLVersionAny, 0,              0,          0,                       0,                     "vorbisidec" },
		{ "theora",    kSDLVersionAny, 0,              0,          0,                       0,                     "theoradec"  },
		{ "fluidsynth",kSDLVersionAny, 0,              0,          0,                       0,                     "fluidsynth" },
		{ "faad",      kSDLVersionAny, 0,              0,          0,                       0,                     "faad"       },
		{ "libcurl",   kSDLVersionAny, "FindCURL",     "CURL",     "CURL_INCLUDE_DIRS",     "CURL_LIBRARIES",      0            },
		{ "sdlnet",    kSDLVersion1,   "FindSDL_net",  "SDL_net",  "SDL_NET_INCLUDE_DIRS",  "SDL_NET_LIBRARIES",   0            },
		{ "sdlnet",    kSDLVersion2,   0,              0,          0,                       0,                     "SDL2_net"   }
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
	if (!workspace)
		error("Could not open \"" + filename + "\" for writing");

	workspace << "cmake_minimum_required(VERSION 3.2)\n"
			"project(" << setup.projectDescription << ")\n\n";

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
	workspace << "    get_target_property(SDL2_INCLUDE_DIRS SDL2::SDL2 INTERFACE_INCLUDE_DIRECTORIES)\n";
	workspace << "    get_target_property(SDL2_LIBRARIES SDL2::SDL2 LOCATION)\n";
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
		if (library->module) {
			workspace << "Include(" << library->module << ")\n";
		}
		if (library->package) {
			workspace << "Find_Package(" << library->package << " REQUIRED)\n";
		}
		if (library->includesVar) {
			workspace << "include_directories(${" << library->includesVar << "})\n";
		}
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

	if (name == setup.projectName) {
		project << "add_executable(" << name << "\n";
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

	if (modulePath.size())
		addFilesToProject(moduleDir, project, includeList, excludeList, setup.filePrefix + '/' + modulePath);
	else
		addFilesToProject(moduleDir, project, includeList, excludeList, setup.filePrefix);


	project << ")\n\n";
	if (name != setup.projectName) {
		project << "endif()\n";
	}

	if (name == setup.projectName) {
		project << "# Engines libraries handling\n";
		writeEnginesLibrariesHandling(setup, project);

		project << "# Libraries\n";
		const Library *sdlLibrary = getLibraryFromFeature("sdl", setup.useSDL2);
		project << "target_link_libraries(" << name << " ${" << sdlLibrary->librariesVar << "})\n";

		for (FeatureList::const_iterator i = setup.features.begin(), end = setup.features.end(); i != end; ++i) {
			if (!i->enable || featureExcluded(i->name)) continue;

			const Library *library = getLibraryFromFeature(i->name, setup.useSDL2);
			if (!library) continue;

			if (library->librariesVar) {
				project << "target_link_libraries(" << name << " ${" << library->librariesVar << "})\n";
			} else {
				project << "target_link_libraries(" << name << " " << library->libraries << ")\n";
			}
		}
		project << "if (WIN32)\n";
		project << "    target_sources(" << name << " PUBLIC " << setup.filePrefix << "/dists/" << name << ".rc)\n";
		project << "    target_link_libraries(" << name << " winmm)\n";
		project << "endif()\n";
		project << "\n";

		project << "set_property(TARGET " << name << " PROPERTY CXX_STANDARD 11)\n";
		project << "set_property(TARGET " << name << " PROPERTY CXX_STANDARD_REQUIRED ON)\n";
	}
}

void CMakeProvider::writeWarnings(std::ofstream &output) const {
	output << "SET (CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS}";
	for (StringList::const_iterator i = _globalWarnings.begin(); i != _globalWarnings.end(); ++i) {
		output << " " << *i;
	}
	output << "\")\n";
}

void CMakeProvider::writeDefines(const BuildSetup &setup, std::ofstream &output) const {
	output << "if (WIN32)\n";
	output << "    add_definitions(-DWIN32)\n";
	output << "else()\n";
	output << "    add_definitions(-DPOSIX)\n";
	output << "endif()\n";

	output << "add_definitions(-DSDL_BACKEND)\n\n";
}

void CMakeProvider::writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int indentation,
                                                const StringList &duplicate, const std::string &objPrefix, const std::string &filePrefix) {

	for (FileNode::NodeList::const_iterator i = dir.children.begin(); i != dir.children.end(); ++i) {
		const FileNode *node = *i;

		if (!node->children.empty()) {
			writeFileListToProject(*node, projectFile, indentation + 1, duplicate, objPrefix + node->name + '_', filePrefix + node->name + '/');
		} else {
			std::string name, ext;
			splitFilename(node->name, name, ext);
			projectFile << "\t" << filePrefix + node->name << "\n";
		}
	}
}

const char *CMakeProvider::getProjectExtension() {
	return ".txt";
}

void CMakeProvider::writeEngineOptions(std::ofstream &workspace) const {
	workspace << "foreach(ENGINE IN LISTS ENGINES)\n";
	workspace << "    OPTION(ENABLE_${ENGINE} \"Enable ${ENGINE}\" ON)\n";
	workspace << "endforeach(ENGINE)\n\n";
}

void CMakeProvider::writeGeneratePluginsTable(std::ofstream &workspace) const {
	workspace << "file(REMOVE \"engines/plugins_table.h\")\n";
	workspace << "file(APPEND \"engines/plugins_table.h\" \"/* This file is automatically generated by CMake */\\n\")\n";
	workspace << "foreach(ENGINE IN LISTS ENGINES)\n";
	workspace << "    if (ENABLE_${ENGINE})\n";
	workspace << "        file(APPEND \"engines/plugins_table.h\" \"#if PLUGIN_ENABLED_STATIC(${ENGINE})\\n\")\n";
	workspace << "        file(APPEND \"engines/plugins_table.h\" \"LINK_PLUGIN(${ENGINE})\\n\")\n";
	workspace << "        file(APPEND \"engines/plugins_table.h\" \"#endif\\n\")\n";
	workspace << "    endif()\n";
	workspace << "endforeach()\n\n";
}

void CMakeProvider::writeEnginesLibrariesHandling(const BuildSetup &setup, std::ofstream &workspace) const {
	workspace << "foreach(ENGINE IN LISTS ENGINES)\n";
	workspace << "    if (ENABLE_${ENGINE})\n";
	workspace << "        string(TOLOWER ${ENGINE} ENGINE_LIB)\n\n";
	workspace << "        # Enable C++11\n";
	workspace << "        set_property(TARGET ${ENGINE_LIB} PROPERTY CXX_STANDARD 11)\n";
	workspace << "        set_property(TARGET ${ENGINE_LIB} PROPERTY CXX_STANDARD_REQUIRED ON)\n\n";
	workspace << "        # Link against the engine\n";
	workspace << "        target_link_libraries("<< setup.projectName <<" ${ENGINE_LIB})\n";
	workspace << "    endif()\n";
	workspace << "endforeach()\n\n";
}

void CMakeProvider::writeEngineDefinitions(std::ofstream &workspace) const {
	workspace << "foreach(ENGINE IN LISTS ENGINES)\n";
	workspace << "    if (ENABLE_${ENGINE})\n";
	workspace << "        add_definitions(-DENABLE_${ENGINE})\n";
	workspace << "        foreach(SUB_ENGINE IN LISTS SUB_ENGINES_${ENGINE})\n";
	workspace << "            add_definitions(-DENABLE_${SUB_ENGINE})\n";
	workspace << "        endforeach(SUB_ENGINE)\n";
	workspace << "    endif()\n";
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
