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

#include "xcode.h"

#include <fstream>
#include <algorithm>

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#else
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#endif

namespace CreateProjectTool {

XCodeProvider::XCodeProvider(StringList &global_warnings, std::map<std::string, StringList> &project_warnings, const int version)
	: ProjectProvider(global_warnings, project_warnings, version) {
}

void XCodeProvider::createWorkspace(const BuildSetup &setup) {
	// TODO
}

void XCodeProvider::createOtherBuildFiles(const BuildSetup &setup) {
	// TODO
}

void XCodeProvider::createProjectFile(const std::string &, const std::string &, const BuildSetup &setup, const std::string &moduleDir,
                                      const StringList &includeList, const StringList &excludeList) {
	// TODO
}

void XCodeProvider::writeFileListToProject(const FileNode &dir, std::ofstream &projectFile, const int indentation,
                                           const StringList &duplicate, const std::string &objPrefix, const std::string &filePrefix) {
	// TODO
}

} // End of CreateProjectTool namespace
