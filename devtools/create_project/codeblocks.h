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

#ifndef TOOLS_CREATE_PROJECT_CODEBLOCKS_H
#define TOOLS_CREATE_PROJECT_CODEBLOCKS_H

#include "create_project.h"

namespace CreateProjectTool {

class CodeBlocksProvider final : public ProjectProvider {
public:
	CodeBlocksProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, StringList &global_errors, const int version = 0);

protected:

	void createWorkspace(const BuildSetup &setup) final;

	void createOtherBuildFiles(const BuildSetup &) final {}

	void addResourceFiles(const BuildSetup &setup, StringList &includeList, StringList &excludeList) final;

	void createProjectFile(const std::string &name, const std::string &uuid, const BuildSetup &setup, const std::string &moduleDir,
						   const StringList &includeList, const StringList &excludeList, const std::string &pchIncludeRoot, const StringList &pchDirs, const StringList &pchExclude) final;

	void writeFileListToProject(const FileNode &dir, std::ostream &projectFile, const int indentation,
								const std::string &objPrefix, const std::string &filePrefix,
								const std::string &pchIncludeRoot, const StringList &pchDirs, const StringList &pchExclude) final;

	void writeReferences(const BuildSetup &setup, std::ofstream &output) final;

	const char *getProjectExtension() final;

private:
	void writeWarnings(const std::string &name, std::ofstream &output) const;
	void writeDefines(const StringList &defines, std::ofstream &output) const;
};

} // End of CreateProjectTool namespace

#endif // TOOLS_CREATE_PROJECT_CODEBLOCKS_H
