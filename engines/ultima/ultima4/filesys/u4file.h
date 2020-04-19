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

#ifndef ULTIMA4_FILE_H
#define ULTIMA4_FILE_H

#include "common/file.h"
#include "common/hash-str.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

/**
 * Represents zip files that game resources can be loaded from.
 */
class U4ZipPackage {
public:
	U4ZipPackage(const Common::String &name, const Common::String &path, bool extension);
	void addTranslation(const Common::String &value, const Common::String &translation);

	const Common::String &getFilename() const {
		return _name;
	}
	const Common::String &getInternalPath() const {
		return _path;
	}
	bool isExtension() const {
		return _extension;
	}
	const Common::String &translate(const Common::String &name) const;

private:
	Common::String _name;                /**< filename */
	Common::String _path;                /**< the path within the zipfile where resources are located */
	bool _extension;             /**< whether this zipfile is an extension with config information */
	Common::HashMap<Common::String, Common::String> _translations; /**< mapping from standard resource names to internal names */
};

/**
 * Keeps track of available zip packages.
 */
class U4ZipPackageMgr {
public:
	static U4ZipPackageMgr *getInstance();
	static void destroy();

	void add(U4ZipPackage *package);
	const Std::vector<U4ZipPackage *> &getPackages() const {
		return _packages;
	}

private:
	U4ZipPackageMgr();
	~U4ZipPackageMgr();

	static U4ZipPackageMgr *_instance;
	Std::vector<U4ZipPackage *> _packages;
};

#ifdef putc
#undef putc
#endif
#ifdef getc
#undef getc
#endif

/**
 * Returns true if the upgrade is present.
 */
extern bool u4isUpgradeAvailable();

/**
 * Returns true if the upgrade is not only present, but is installed
 * (switch.bat or setup.bat has been run)
 */
extern bool u4isUpgradeInstalled();

extern Common::File *u4fopen(const Common::String &fname);
extern Common::File *u4fopen_zip(const Common::String &fname, Common::Archive *archive);
extern void u4fclose(Common::File *f);
extern int u4fseek(Common::File *f, long offset, int whence);
extern long u4ftell(Common::File *f);
extern size_t u4fread(void *ptr, size_t size, size_t nmemb, Common::File *f);
extern int u4fgetc(Common::File *f);
extern int u4fgetshort(Common::File *f);
extern long u4flength(Common::File *f);
extern Std::vector<Common::String> u4read_stringtable(Common::File *f, long offset, int nstrings);

extern Common::String u4find_path(const Common::String &fname, Common::List<Common::String> specificSubPaths);
extern Common::String u4find_music(const Common::String &fname);
extern Common::String u4find_sound(const Common::String &fname);
extern Common::String u4find_conf(const Common::String &fname);
extern Common::String u4find_graphics(const Common::String &fname);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
