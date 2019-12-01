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

#ifndef ULTIMA8_FILESYS_LISTFILES_H
#define ULTIMA8_FILESYS_LISTFILES_H

#include "ultima8/std/string.h"
#include "ultima8/filesys/file_system.h"

namespace Ultima8 {

int FileSystem::ListFiles(const std::string mask, FileList &files) {
	// TODO: Figure out if this is for standard filesystem or saves
#ifdef TODO
	glob_t globres;
	std::string name(mask);

	// get the 'root' (@u8 or whatever) stub
	const std::string rootpath(name.substr(0, name.find('/')));

	// munge the path to a 'real' one.
	if (!rewrite_virtual_path(name)) {
		perr << "Warning: FileSystem sandbox violation when accessing:"
		     << std::endl << "\t" << mask << std::endl;
		return -1;
	}

#if 0
	pout << "Root: " << rootpath << std::endl;
	pout << name << "\t" << name.size() << std::endl;
	pout << mask << '\t' << mask.size() << std::endl;
#endif

	// calculate the difference in length between the real path, the
	// original path, and the root @whatever
	uint32 newplen = name.size() - mask.size() + rootpath.size();

	int err = glob(name.c_str(), GLOB_NOSORT, 0, &globres);

	switch (err) {
	case 0:  //OK
		for (unsigned int i = 0; i < globres.gl_pathc; i++) {
			std::string newfname(globres.gl_pathv[i]);
#if 0
			pout << newfname << std::endl;
#endif
			newfname = rootpath + newfname.substr(newplen);
			// If the OS uses anything other then / as a path seperator,
			// they probably need to swap it back here...
#if 0
			pout << newfname << std::endl;
#endif
			files.push_back(newfname);
		}
		globfree(&globres);
		return 0;
	case 3:  //no matches
		return 0;
	default: //error
		perr << "Glob error " << err << std::endl;
		return err;
	}
#else
	error("TODO: ListFiles");
#endif
}

} // End of namespace Ultima8

#endif
