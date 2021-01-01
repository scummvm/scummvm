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

#ifndef TOOLS_CREATE_PROJECT_CMAKE_H
#define TOOLS_CREATE_PROJECT_CMAKE_H

#include "create_project.h"

namespace CreateProjectTool {

/**
 * A ProjectProvider used to generate CMake project descriptions
 *
 * Generated CMake projects are minimal, and will only work with GCC.
 */
class CMakeProvider : public ProjectProvider {
public:
	CMakeProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version = 0);

protected:

	void createWorkspace(const BuildSetup &setup);

	void createOtherBuildFiles(const BuildSetup &) {}

	void addResourceFiles(const BuildSetup &setup, StringList &includeList, StringList &excludeList) {}

	void createProjectFile(const std::string &name, const std::string &uuid, const BuildSetup &setup, const std::string &moduleDir,
	                       const StringList &includeList, const StringList &excludeList);

	void writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int indentation,
	                            const std::string &objPrefix, const std::string &filePrefix);

	const char *getProjectExtension();

private:
	enum SDLVersion {
		kSDLVersionAny,
		kSDLVersion1,
		kSDLVersion2
	};

	/**
	 * CMake properties for a library required by a feature
	 */
	struct Library {
		const char *feature;
		const char *pkgConfig;
		SDLVersion sdlVersion;
		const char *module;
		const char *package;
		const char *includesVar;
		const char *librariesVar;
		const char *libraries;
	};

	const Library *getLibraryFromFeature(const char *feature, bool useSDL2) const;

	void writeWarnings(std::ofstream &output) const;
	void writeDefines(const BuildSetup &setup, std::ofstream &output) const;
	void writeEngines(const BuildSetup &setup, std::ofstream &workspace) const;
	void writeSubEngines(const BuildSetup &setup, std::ofstream &workspace) const;
	void writeEngineOptions(std::ofstream &workspace) const;
	void writeGeneratePluginsTable(std::ofstream &workspace) const;
	void writeEnginesLibrariesHandling(const BuildSetup &setup, std::ofstream &workspace) const;
	void writeEngineDefinitions(std::ofstream &workspace) const;
	void writeFeatureLibSearch(const BuildSetup &setup, std::ofstream &workspace, const char *feature) const;
	bool featureExcluded(const char *name) const;
	const EngineDesc &findEngineDesc(const std::string &name, const EngineDescList &engines) const;
};

} // End of CreateProjectTool namespace

#endif // TOOLS_CREATE_PROJECT_CMAKE_H
