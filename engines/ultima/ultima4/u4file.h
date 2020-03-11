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

    const Common::String &getFilename() const { return _name; }
    const Common::String &getInternalPath() const { return _path; }
    bool isExtension() const { return _extension; }
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
    const Std::vector<U4ZipPackage *> &getPackages() const { return packages; }

private:
    U4ZipPackageMgr();
    ~U4ZipPackageMgr();

    static U4ZipPackageMgr *instance;
    Std::vector<U4ZipPackage *> packages;
};

#ifdef putc
#undef putc
#endif
#ifdef getc
#undef getc
#endif

/**
 * An abstract interface for file access.
 */
class U4FILE {
public:
    virtual ~U4FILE() {}

    virtual void close() = 0;
    virtual int seek(long offset, int whence) = 0;
    virtual long tell() = 0;
    virtual size_t read(void *ptr, size_t size, size_t nmemb) = 0;
    virtual int getc() = 0;
    virtual int putc(int c) = 0;
    virtual long length() = 0;

    int getshort();
};

/** A replacement class to manage path searching. Very open-concept */
#define u4Path (*U4PATH::getInstance())
class U4PATH {
public:
	U4PATH() : defaultsHaveBeenInitd(false){}
    void initDefaultPaths();

    static U4PATH * instance;
    static U4PATH * getInstance();


	Common::List<Common::String> rootResourcePaths;

    Common::List<Common::String> u4ForDOSPaths;
    Common::List<Common::String> u4ZipPaths;
    Common::List<Common::String> musicPaths;
    Common::List<Common::String> soundPaths;
    Common::List<Common::String> configPaths;
    Common::List<Common::String> graphicsPaths;

private:
    bool defaultsHaveBeenInitd;

};

bool u4isUpgradeAvailable();
bool u4isUpgradeInstalled();
U4FILE *u4fopen(const Common::String &fname);
U4FILE *u4fopen_stdio(const Common::String &fname);
U4FILE *u4fopen_zip(const Common::String &fname, U4ZipPackage *package);
void u4fclose(U4FILE *f);
int u4fseek(U4FILE *f, long offset, int whence);
long u4ftell(U4FILE *f);
size_t u4fread(void *ptr, size_t size, size_t nmemb, U4FILE *f);
int u4fgetc(U4FILE *f);
int u4fgetshort(U4FILE *f);
int u4fputc(int c, U4FILE *f);
long u4flength(U4FILE *f);
Std::vector<Common::String> u4read_stringtable(U4FILE *f, long offset, int nstrings);

Common::String u4find_path(const Common::String &fname, Common::List<Common::String> specificSubPaths);
Common::String u4find_music(const Common::String &fname);
Common::String u4find_sound(const Common::String &fname);
Common::String u4find_conf(const Common::String &fname);
Common::String u4find_graphics(const Common::String &fname);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
