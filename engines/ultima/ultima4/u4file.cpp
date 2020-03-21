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

#include "ultima/ultima4/u4file.h"
#include "ultima/ultima4/debug.h"
#include "common/debug.h"
#include "common/savefile.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima4 {

using Std::map;
using Common::String;
using Std::vector;

/**
 * A specialization of U4FILE that uses C stdio internally.
 */
class U4FILE_stdio : public U4FILE {
private:
	Common::SeekableReadStream *_in;
	Common::WriteStream *_out;
public:
    static U4FILE *openForReading(const Common::String &fname);
	static U4FILE *openForWriting(const Common::String &fname);

	U4FILE_stdio(Common::SeekableReadStream *rs) : _in(rs), _out(nullptr) {}
	U4FILE_stdio(Common::WriteStream *ws) : _in(nullptr), _out(ws) {}
	virtual ~U4FILE_stdio() { close(); }

    void close() override;
    int seek(long offset, int whence) override;
    long tell() override;
    size_t read(void *ptr, size_t size, size_t nmemb) override;
    int getc() override;
    int putc(int c) override;
    long length() override;
};

/**
 * A specialization of U4FILE that reads files out of zip archives
 * automatically.
 */
class U4FILE_zip : public U4FILE {
public:
    static U4FILE *open(const Common::String &fname, const U4ZipPackage *package);

    virtual void close();
    virtual int seek(long offset, int whence);
    virtual long tell();
    virtual size_t read(void *ptr, size_t size, size_t nmemb);
    virtual int getc();
    virtual int putc(int c);
    virtual long length();

private:
	Common::SeekableReadStream *_file;
};

extern bool verbose;

/**
 * Returns true if the upgrade is present.
 */
bool u4isUpgradeAvailable() {
    bool avail = false;
    U4FILE *pal;
    if ((pal = u4fopen("u4vga.pal")) != NULL) {
        avail = true;
        u4fclose(pal);
    }
    return avail;
}

/**
 * Returns true if the upgrade is not only present, but is installed
 * (switch.bat or setup.bat has been run)
 */
bool u4isUpgradeInstalled() {
    U4FILE *u4f = NULL;
    long int filelength;
    bool result = false;

    /* FIXME: Is there a better way to determine this? */
    u4f = u4fopen("ega.drv");
    if (u4f) {

        filelength = u4f->length();
        u4fclose(u4f);

        /* see if (ega.drv > 5k).  If so, the upgrade is installed */
        if (filelength > (5 * 1024))
            result = true;
    }

    if (verbose)
        debug("u4isUpgradeInstalled %d\n", (int) result);

    return result;
}

/**
 * Creates a new zip package.
 */
U4ZipPackage::U4ZipPackage(const Common::String &name, const Common::String &path, bool extension) {
    this->_name = name;
    this->_path = path;
    this->_extension = extension;
}

void U4ZipPackage::addTranslation(const Common::String &value, const Common::String &translation) {
    _translations[value] = translation;
}
    
const Common::String &U4ZipPackage::translate(const Common::String &name) const {
    Std::map<Common::String, Common::String>::const_iterator i = _translations.find(name);
    if (i != _translations.end())
        return i->_value;
    else
        return name;
}

U4ZipPackageMgr *U4ZipPackageMgr::_instance = NULL;

U4ZipPackageMgr *U4ZipPackageMgr::getInstance() {
    if (_instance == NULL) {
        _instance = new U4ZipPackageMgr();
    }
    return _instance;
}

void U4ZipPackageMgr::destroy() {
    if (_instance != NULL) {
        delete _instance;
        _instance = NULL;
    }
}
    
void U4ZipPackageMgr::add(U4ZipPackage *package) {
    _packages.push_back(package);
}

U4ZipPackageMgr::U4ZipPackageMgr() {
#ifdef TODO
	unzFile f;
	
    Common::String upg_pathname(u4find_path("u4upgrad.zip", u4Path.u4ZipPaths));
    if (!upg_pathname.empty()) {
        /* upgrade zip is present */
        U4ZipPackage *upgrade = new U4ZipPackage(upg_pathname, "", false);
        upgrade->addTranslation("compassn.ega", "compassn.old");
        upgrade->addTranslation("courage.ega", "courage.old");
        upgrade->addTranslation("cove.tlk", "cove.old");
        upgrade->addTranslation("ega.drv", "ega.old"); // not actually used
        upgrade->addTranslation("honesty.ega", "honesty.old");
        upgrade->addTranslation("honor.ega", "honor.old");
        upgrade->addTranslation("humility.ega", "humility.old");
        upgrade->addTranslation("key7.ega", "key7.old");
        upgrade->addTranslation("lcb.tlk", "lcb.old");
        upgrade->addTranslation("love.ega", "love.old");
        upgrade->addTranslation("love.ega", "love.old");
        upgrade->addTranslation("minoc.tlk", "minoc.old");
        upgrade->addTranslation("rune_0.ega", "rune_0.old");
        upgrade->addTranslation("rune_1.ega", "rune_1.old");
        upgrade->addTranslation("rune_2.ega", "rune_2.old");
        upgrade->addTranslation("rune_3.ega", "rune_3.old");
        upgrade->addTranslation("rune_4.ega", "rune_4.old");
        upgrade->addTranslation("rune_5.ega", "rune_5.old");
        upgrade->addTranslation("sacrific.ega", "sacrific.old");
        upgrade->addTranslation("skara.tlk", "skara.old");
        upgrade->addTranslation("spirit.ega", "spirit.old");
        upgrade->addTranslation("start.ega", "start.old");
        upgrade->addTranslation("stoncrcl.ega", "stoncrcl.old");
        upgrade->addTranslation("truth.ega", "truth.old");
        upgrade->addTranslation("ultima.com", "ultima.old"); // not actually used
        upgrade->addTranslation("valor.ega", "valor.old");
        upgrade->addTranslation("yew.tlk", "yew.old");
        add(upgrade);
    }
	
	// Check for the default zip packages
	int flag = 0;
	Common::String pathname;

	do {
		//Check for the upgraded package once. unlikely it'll be renamed.
		pathname = u4find_path("ultima4-1.01.zip", u4Path.u4ZipPaths);
		if (!pathname.empty()) {
		    flag = 1;
			break;
		}

		// We check for all manner of generic packages, though.
		pathname = u4find_path("ultima4.zip", u4Path.u4ZipPaths);
		if (!pathname.empty()) {
		    flag = 1;
			break;
		}

	    pathname = u4find_path("Ultima4.zip", u4Path.u4ZipPaths);
		if (!pathname.empty()) {
		    flag = 1;
			break;
		}

	    pathname = u4find_path("ULTIMA4.zip", u4Path.u4ZipPaths);
		if (!pathname.empty()) {
		    flag = 1;
			break;
		}

	    pathname = u4find_path("u4.zip", u4Path.u4ZipPaths);
		if (!pathname.empty()) {
		    flag = 1;
			break;
		}
		
	    pathname = u4find_path("U4.zip", u4Path.u4ZipPaths);
		if (!pathname.empty()) {
		    flag = 1;
			break;
		}

		//search for the ultimaforever.com zip and variations
		pathname = u4find_path("UltimaIV.zip", u4Path.u4ZipPaths);
		if (!pathname.empty()) {
		    flag = 1;
			break;
		}
		
		pathname = u4find_path("Ultimaiv.zip", u4Path.u4ZipPaths);
		if (!pathname.empty()) {
		    flag = 1;
			break;
		}
		
		pathname = u4find_path("ULTIMAIV.zip", u4Path.u4ZipPaths);
		if (!pathname.empty()) {
		    flag = 1;
			break;
		}
		
		pathname = u4find_path("ultimaIV.zip", u4Path.u4ZipPaths);
		if (!pathname.empty()) {
		    flag = 1;
			break;
		}
		
		pathname = u4find_path("ultimaiv.zip", u4Path.u4ZipPaths);
		if (!pathname.empty()) {
		    flag = 1;
			break;
		}
		
		// If it's not found by this point, give up.
		break;
		
	} while (flag == 0);

	if (flag) {
		f = unzOpen(pathname.c_str());
		if (!f)
			return;
	
		//Now we detect the folder structure inside the zipfile.
		if (unzLocateFile(f, "charset.ega", 2) == UNZ_OK) {
			add(new U4ZipPackage(pathname, "", false));
			
    	} else if (unzLocateFile(f, "ultima4/charset.ega", 2) == UNZ_OK) {
			add(new U4ZipPackage(pathname, "ultima4/", false));

		} else if (unzLocateFile(f, "Ultima4/charset.ega", 2) == UNZ_OK) {
			add(new U4ZipPackage(pathname, "Ultima4/", false));

		} else if (unzLocateFile(f, "ULTIMA4/charset.ega", 2) == UNZ_OK) {
			add(new U4ZipPackage(pathname, "ULTIMA4/", false));

		} else if (unzLocateFile(f, "u4/charset.ega", 2) == UNZ_OK) {
			add(new U4ZipPackage(pathname, "u4/", false));

		} else if (unzLocateFile(f, "U4/charset.ega", 2) == UNZ_OK) {
			add(new U4ZipPackage(pathname, "U4/", false));

		}

		unzClose(f);

	}
#endif	
    /* scan for extensions */
}

U4ZipPackageMgr::~U4ZipPackageMgr() {
    for (Std::vector<U4ZipPackage *>::iterator i = _packages.begin(); i != _packages.end(); i++)
        delete *i;
}

int U4FILE::getshort() {
    int byteLow = getc();
    return byteLow | (getc() << 8);
}

U4FILE *U4FILE_stdio::openForReading(const Common::String &fname) {
    Common::File *f = new Common::File();

	if (!f->open(fname)) {
		delete f;
		return nullptr;
	}

    return new U4FILE_stdio(f);
}

U4FILE *U4FILE_stdio::openForWriting(const Common::String &fname) {
	Common::OutSaveFile *saveFile = g_system->getSavefileManager()->openForSaving(fname);

	if (!saveFile)
		return nullptr;

	return new U4FILE_stdio(saveFile);
}

void U4FILE_stdio::close() {
    delete _in;
	delete _out;
	_in = nullptr;
	_out = nullptr;
}

int U4FILE_stdio::seek(long offset, int whence) {
	assert(_in);
	return _in->seek(offset, whence);
}

long U4FILE_stdio::tell() {
	return _in->pos();
}

size_t U4FILE_stdio::read(void *ptr, size_t size, size_t nmemb) {
	uint bytesRead = _in->read(ptr, size * nmemb);
	return bytesRead / size;
}

int U4FILE_stdio::getc() {
	return _in->eos() ? EOF : _in->readByte();
}

int U4FILE_stdio::putc(int c) {
	_out->writeByte(c);
	return 1;
}

long U4FILE_stdio::length() {
	return _in->size();
}

/*------------------------------------------------------------------------*/

/**
 * Opens a file from within a zip archive.
 */
U4FILE *U4FILE_zip::open(const Common::String &fname, const U4ZipPackage *package) {
#ifdef TODO
	U4FILE_zip *u4f;
    unzFile f;

    f = unzOpen(package->getFilename().c_str());
    if (!f)
        return NULL;

    Common::String pathname = package->getInternalPath() + package->translate(fname);

    if (unzLocateFile(f, pathname.c_str(), 2) == UNZ_END_OF_LIST_OF_FILE) {
        unzClose(f);
        return NULL;
    }
    unzOpenCurrentFile(f);

    u4f = new U4FILE_zip;
    u4f->zfile = f;

    return u4f;
#else
	return nullptr;
#endif
}

void U4FILE_zip::close() {
#ifdef TODO
    unzClose(zfile);
#endif
}

int U4FILE_zip::seek(long offset, int whence) {
#ifdef TODO
	char *buf;
    long pos;

    ASSERT(whence != SEEK_END, "seeking with whence == SEEK_END not allowed with zipfiles");
    pos = unztell(zfile);
    if (whence == SEEK_CUR)
        offset = pos + offset;
    if (offset == pos)
        return 0;
    if (offset < pos) {
        unzCloseCurrentFile(zfile);
        unzOpenCurrentFile(zfile);
        pos = 0;
    }
    ASSERT(offset - pos > 0, "error in U4FILE_zip::seek");
    buf = new char[offset - pos];
    unzReadCurrentFile(zfile, buf, offset - pos);
    delete [] buf;
    return 0;
#else
	return -1;
#endif
}

long U4FILE_zip::tell() {
#ifdef TODO
    return unztell(zfile);
#else
	return 0;
#endif
}

size_t U4FILE_zip::read(void *ptr, size_t size, size_t nmemb) {
#ifdef TODO
    size_t retval = unzReadCurrentFile(zfile, ptr, size * nmemb);
    if (retval > 0)
        retval = retval / size;

    return retval;
#else
	return 0;
#endif
}

int U4FILE_zip::getc() {
#ifdef TODO
    int retval;
    unsigned char c;

    if (unzReadCurrentFile(zfile, &c, 1) > 0)
        retval = c;
    else
        retval = EOF;

    return retval;
#else
	return 0;
#endif
}

int U4FILE_zip::putc(int c) {
    ASSERT(0, "zipfiles must be read-only!");
    return c;
}

long U4FILE_zip::length() {
#ifdef TODO
    unz_file_info fileinfo;

    unzGetCurrentFileInfo(zfile, &fileinfo,
                          NULL, 0,
                          NULL, 0,
                          NULL, 0);
    return fileinfo.uncompressed_size;
#else
	return 0;
#endif
}

/**
 * Open a data file from the Ultima 4 for DOS installation
 */
U4FILE *u4fopen(const Common::String &fname) {
    U4FILE *u4f = NULL;

    if (verbose)
        debug("looking for %s\n", fname.c_str());
#ifdef TODO
    /**
     * search for file within zipfiles (ultima4.zip, u4upgrad.zip, etc.)
     */
    const vector<U4ZipPackage *> &packages = U4ZipPackageMgr::getInstance()->getPackages(); 
    for (Std::vector<U4ZipPackage *>::const_reverse_iterator j = packages.rbegin();
			j != packages.rend(); ++j) {
        u4f = U4FILE_zip::open(fname, *j);
        if (u4f)
            return u4f; /* file was found, return it! */
    }

    /*
     * file not in a zipfile; check if it has been unzipped
     */
#endif

	if (!fname.empty()) {
        u4f = U4FILE_stdio::openForReading(fname);
        if (verbose && u4f != NULL)
            debug("%s successfully opened\n", fname.c_str());
    }

    return u4f;
}

/**
 * Opens a file with the standard C stdio facilities and wrap it in a
 * U4FILE.
 */
U4FILE *u4fopen_stdio(const Common::String &fname) {
    return U4FILE_stdio::openForReading(fname);
}

/**
 * Opens a file from a zipfile and wraps it in a U4FILE.
 */
U4FILE *u4fopen_zip(const Common::String &fname, U4ZipPackage *package) {
    return U4FILE_zip::open(fname, package);
}

/**
 * Closes a data file from the Ultima 4 for DOS installation.
 */
void u4fclose(U4FILE *f) {
    f->close();
    delete f;
}

int u4fseek(U4FILE *f, long offset, int whence) {
    return f->seek(offset, whence);
}

long u4ftell(U4FILE *f) {
    return f->tell();
}

size_t u4fread(void *ptr, size_t size, size_t nmemb, U4FILE *f) {
    return f->read(ptr, size, nmemb);
}

int u4fgetc(U4FILE *f) {
    return f->getc();
}

int u4fgetshort(U4FILE *f) {
    return f->getshort();
}

int u4fputc(int c, U4FILE *f) {
    return f->putc(c);
}

/**
 * Returns the length in bytes of a file.
 */
long u4flength(U4FILE *f) {
    return f->length();
}

/**
 * Read a series of zero terminated strings from a file.  The strings
 * are read from the given offset, or the current file position if
 * offset is -1.
 */
vector<Common::String> u4read_stringtable(U4FILE *f, long offset, int nstrings) {
    Common::String buffer;
    int i;
    vector<Common::String> strs;

    ASSERT(offset < u4flength(f), "offset begins beyond end of file");

    if (offset != -1)
        f->seek(offset, SEEK_SET);
    for (i = 0; i < nstrings; i++) {
        char c;
        buffer.clear();

        while ((c = f->getc()) != '\0')
            buffer += c;
        
        strs.push_back(buffer);
    }

    return strs;
}

Common::String u4find_path(const Common::String &fname, Common::List<Common::String> specificSubPaths) {
#ifdef TODO
	FILE *f = NULL;

    // Try absolute first
    char path[2048]; // Sometimes paths get big.

    f = fopen(fname.c_str(), "rb");
    if (f)
        strcpy(path, fname.c_str());

    // Try 'file://' protocol if specified
    if (f == NULL) {
        const Common::String file_url_prefix("file://");

        if (fname.compare(0, file_url_prefix.length(), file_url_prefix) == 0) {
            strcpy(path, fname.substr(file_url_prefix.length()).c_str());
            if (verbose) {
                debug("trying to open %s\n", path);
            }
            f = fopen(path, "rb");
        }
    }

    // Try paths
    if (f == NULL) {
        for (Common::List<Common::String>::iterator rootItr = u4Path.rootResourcePaths.begin();
                rootItr!=u4Path.rootResourcePaths.end() && !f;
                ++rootItr) {
            for (Common::List<Common::String>::iterator subItr = specificSubPaths.begin();
                    subItr!=specificSubPaths.end() && !f;
                    ++subItr) {

                snprintf(path, sizeof(path), "%s/%s/%s", rootItr->c_str(), subItr->c_str(), fname.c_str());

                if (verbose) {
                    debug("trying to open %s\n", path);
                }
                if ((f = fopen(path, "rb")) != NULL)
                    break;
            }
        }
    }
#if defined(IOS)
    if (f == NULL) {
        Common::String base = fname;
        Common::String ext = "";
        Common::String dir = "";
        // This is VERY dependant on the current layout of the XML files. It will fail in a general case.
        size_t seppos = fname.rfind('/');
        if (seppos != Common::String::npos)
            dir = fname.substr(0, seppos);        
        size_t pos = fname.rfind('.');
        if (pos != Common::String::npos) {
            if (seppos != Common::String::npos)
                base = fname.substr(seppos + 1, pos - seppos - 1);
            else
                base = fname.substr(0, pos);
            ext = fname.substr(pos + 1);
        }
        
        Common::String pathFile = U4IOS::getFileLocation(dir, base, ext);
        strncpy(path, pathFile.c_str(), 2048);
        if (verbose)
            debug("trying to open %s\n", path);
        
        f = fopen(path, "rb");
    }    
#endif

    if (verbose) {
        if (f != NULL)
            debug("%s successfully found\n", path);
        else 
            debug("%s not found\n", fname.c_str());
    }

    if (f) {
        delete f;
        return path;
    } else
        return "";
#else
	warning("findpath");
	return fname;
#endif
}

Common::String u4find_music(const Common::String &fname) {
	return "data/music/" + fname;
}

Common::String u4find_sound(const Common::String &fname) {
	return "data/sound/" + fname;
}

Common::String u4find_conf(const Common::String &fname) {
	return "data/conf/" + fname;
}

Common::String u4find_graphics(const Common::String &fname) {
	return "data/graphics/" + fname;
}

} // End of namespace Ultima4
} // End of namespace Ultima
