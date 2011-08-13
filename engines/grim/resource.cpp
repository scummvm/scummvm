/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/grim/resource.h"
#include "engines/grim/colormap.h"
#include "engines/grim/costume.h"
#include "engines/grim/keyframe.h"
#include "engines/grim/material.h"
#include "engines/grim/grim.h"
#include "engines/grim/lipsync.h"
#include "engines/grim/savegame.h"
#include "engines/grim/lab.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/font.h"
#include "engines/grim/model.h"

namespace Grim {

ResourceLoader *g_resourceloader = NULL;

ResourceLoader::ResourceLoader() {
	int lab_counter = 0;
	_cacheDirty = false;
	_cacheMemorySize = 0;

	Lab *l;
	Common::ArchiveMemberList files;

	SearchMan.listMatchingMembers(files, "*.lab");
	SearchMan.listMatchingMembers(files, "*.m4b");

	if (files.empty())
		error("Cannot find game data - check configuration file");

	for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
		const Common::String filename = (*x)->getName();
		l = new Lab();

		if (l->open(filename)) {
			if (filename.equalsIgnoreCase("data005.lab"))
				_labs.push_front(l);
			else
				_labs.push_back(l);
			lab_counter++;
		} else {
			delete l;
		}
	}

	files.clear();

	if (g_grim->getGameFlags() & ADGF_DEMO) {
		SearchMan.listMatchingMembers(files, "*.mus");

		for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
			const Common::String filename = (*x)->getName();
			l = new Lab();

			if (l->open(filename)) {
				_labs.push_back(l);
				lab_counter++;
			} else {
				delete l;
			}
		}
	}
}

template<typename T>
void clearList(Common::List<T> &list) {
	while (!list.empty()) {
		T p = list.front();
		list.erase(list.begin());
		delete p;
	}
}

ResourceLoader::~ResourceLoader() {
	for (Common::Array<ResourceCache>::iterator i = _cache.begin(); i != _cache.end(); ++i) {
		ResourceCache &r = *i;
		delete[] r.fname;
		delete r.resPtr;
	}
	clearList(_labs);
	clearList(_models);
	clearList(_colormaps);
	clearList(_keyframeAnims);
	clearList(_fonts);
	clearList(_lipsyncs);
}

const Lab *ResourceLoader::getLab(const Common::String &filename) const {
	for (LabList::const_iterator i = _labs.begin(); i != _labs.end(); ++i)
		if ((*i)->getFileExists(filename))
			return *i;

	return NULL;
}

static int sortCallback(const void *entry1, const void *entry2) {
	return scumm_stricmp(((ResourceLoader::ResourceCache *)entry1)->fname, ((ResourceLoader::ResourceCache *)entry2)->fname);
}

Block *ResourceLoader::getFileFromCache(const Common::String &filename) {
	ResourceLoader::ResourceCache *entry = getEntryFromCache(filename);
	if (entry)
		return entry->resPtr;
	else
		return NULL;
}

ResourceLoader::ResourceCache *ResourceLoader::getEntryFromCache(const Common::String &filename) {
	if (_cache.empty())
		return NULL;

	if (_cacheDirty) {
		qsort(_cache.begin(), _cache.size(), sizeof(ResourceCache), sortCallback);
		_cacheDirty = false;
	}

	ResourceCache key;
	key.fname = const_cast<char *>(filename.c_str());

	return (ResourceLoader::ResourceCache *)bsearch(&key, _cache.begin(), _cache.size(), sizeof(ResourceCache), sortCallback);
}

bool ResourceLoader::getFileExists(const Common::String &filename) const {
	return getLab(filename) != NULL;
}

Block *ResourceLoader::getFileBlock(const Common::String &filename) const {
	const Lab *l = getLab(filename);
	if (!l)
		return NULL;
	else
		return l->getFileBlock(filename);
}

Block *ResourceLoader::getBlock(const char *filename) {
    Common::String fname = filename;
    fname.toLowercase();
    Block *b = getFileFromCache(fname);
    if (!b) {
        b = getFileBlock(fname);
		if (b) {
			putIntoCache(fname, b);
		}
    }

    return b;
}

LuaFile *ResourceLoader::openNewStreamLuaFile(const char *filename) const {
	const Lab *l = getLab(filename);

	if (!l)
		return NULL;
	else
		return l->openNewStreamLua(filename);
}

Common::File *ResourceLoader::openNewStreamFile(const char *filename) const {
	const Lab *l = getLab(filename);

	if (!l)
		return NULL;
	else
		return l->openNewStreamFile(filename);
}

Common::SeekableReadStream *ResourceLoader::openNewSubStreamFile(const char *filename) const {
	const Lab *l = getLab(filename);

	if (!l)
		return NULL;
	else
		return l->openNewSubStreamFile(filename);
}

int ResourceLoader::getFileLength(const char *filename) const {
	const Lab *l = getLab(filename);
	if (l)
		return l->getFileLength(filename);
	else
		return 0;
}

void ResourceLoader::putIntoCache(const Common::String &fname, Block *res) {
	ResourceCache entry;
	entry.resPtr = res;
	entry.fname = new char[fname.size() + 1];
	strcpy(entry.fname, fname.c_str());
	_cacheMemorySize += res->getLen();
	_cache.push_back(entry);
	_cacheDirty = true;
}

Bitmap *ResourceLoader::loadBitmap(const Common::String &filename) {
	Common::String fname = filename;
	fname.toLowercase();
	Block *b = getFileFromCache(fname);
	if (!b) {
		b = getFileBlock(fname);
		if (!b) {	// Grim sometimes asks for non-existant bitmaps (eg, ha_overhead)
			warning("Could not find bitmap %s", filename.c_str());
			return NULL;
		}
		putIntoCache(fname, b);
	}

	Bitmap *result = new Bitmap(filename, b->getData(), b->getLen());
	if (result)
		g_grim->registerBitmap(result);

	return result;
}

CMap *ResourceLoader::loadColormap(const Common::String &filename) {
	Block *b = getFileFromCache(filename);
	if (!b) {
		b = getFileBlock(filename);
		if (!b) {
			error("Could not find colormap %s", filename.c_str());
        }
		putIntoCache(filename, b);
	}

	CMap *result = new CMap(filename, b->getData(), b->getLen());
	_colormaps.push_back(result);

	return result;
}

static Common::String fixFilename(const Common::String filename, bool append = true) {
	Common::String fname(filename);
	if (g_grim->getGameType() == GType_MONKEY4) {
		int len = fname.size();
		for (int i = 0; i < len; i++) {
			if (fname[i] == '\\') {
				fname.setChar('/', i);
			}
		}
		// Append b to end of filename for EMI
		if (append)
			fname += "b";
	}
	return fname;
}

Costume *ResourceLoader::loadCostume(const Common::String &filename, Costume *prevCost) {
	Common::String fname = fixFilename(filename);
	fname.toLowercase();
	Block *b = getFileFromCache(fname);
	if (!b) {
		b = getFileBlock(fname);
		if (!b)
			error("Could not find costume \"%s\"", filename.c_str());
		putIntoCache(fname, b);
	}
	Costume *result = new Costume(filename, b->getData(), b->getLen(), prevCost);

	return result;
}

Font *ResourceLoader::loadFont(const Common::String &filename) {
	Block *b = getFileFromCache(filename);
	if (!b) {
		b = getFileBlock(filename);
		if (!b)
			error("Could not find font file %s", filename.c_str());
		putIntoCache(filename, b);
	}

	Font *result = new Font(filename, b->getData(), b->getLen());

	return result;
}

KeyframeAnim *ResourceLoader::loadKeyframe(const Common::String &filename) {
	Block *b = getFileFromCache(filename);
	if (!b) {
		b = getFileBlock(filename);
		if (!b)
			error("Could not find keyframe file %s", filename.c_str());
		putIntoCache(filename, b);
	}

	KeyframeAnim *result = new KeyframeAnim(filename, b->getData(), b->getLen());
	_keyframeAnims.push_back(result);

	return result;
}

LipSync *ResourceLoader::loadLipSync(const Common::String &filename) {
	LipSync *result;
	Block *b = getFileFromCache(filename);
	bool cached = true;
	if (!b) {
		b = getFileBlock(filename);
		if (!b)
			return NULL;
		cached = false;
	}

	result = new LipSync(filename, b->getData(), b->getLen());

	// Some lipsync files have no data
	if (result->isValid()) {
		if (!cached)
			putIntoCache(filename, b);
		_lipsyncs.push_back(result);
	} else {
		delete result;
		delete b;
		result = NULL;
	}

	return result;
}

Material *ResourceLoader::loadMaterial(const Common::String &filename, CMap *c) {
	Common::String fname = fixFilename(filename, false);
	fname.toLowercase();
	Block *b = getFileFromCache(fname);
	if (!b) {
		b = getFileBlock(fname);
		if (!b)
			error("Could not find material %s", filename.c_str());
		putIntoCache(filename, b);
	}

	Material *result = new Material(fname, b->getData(), b->getLen(), c);

	return result;
}

Model *ResourceLoader::loadModel(const Common::String &filename, CMap *c, Model *parent) {
	Common::String fname = fixFilename(filename);
	Block *b = getFileFromCache(fname);
	if (!b) {
		b = getFileBlock(fname);
		if (!b)
			error("Could not find model %s", filename.c_str());
		putIntoCache(fname, b);
	}

	Model *result = new Model(filename, b->getData(), b->getLen(), c, parent);
	_models.push_back(result);

	return result;
}

void ResourceLoader::uncache(const char *filename) {
	Common::String fname = filename;
	fname.toLowercase();

	if (_cacheDirty) {
		qsort(_cache.begin(), _cache.size(), sizeof(ResourceCache), sortCallback);
		_cacheDirty = false;
	}

	for (unsigned int i = 0; i < _cache.size(); i++) {
		if (fname.compareTo(_cache[i].fname) == 0) {
			delete[] _cache[i].fname;
			_cacheMemorySize -= _cache[i].resPtr->getLen();
			delete _cache[i].resPtr;
			_cache.remove_at(i);
			_cacheDirty = true;
		}
	}
}

void ResourceLoader::uncacheModel(Model *m) {
	_models.remove(m);
}

void ResourceLoader::uncacheColormap(CMap *c) {
	_colormaps.remove(c);
}

void ResourceLoader::uncacheKeyframe(KeyframeAnim *k) {
	_keyframeAnims.remove(k);
}

void ResourceLoader::uncacheFont(Font *f) {
	_fonts.remove(f);
}

void ResourceLoader::uncacheLipSync(LipSync *s) {
	_lipsyncs.remove(s);
}

ModelPtr ResourceLoader::getModel(const Common::String &fname, CMap *c) {
	Common::String filename = fname;
	filename.toLowercase();
	for (Common::List<Model *>::const_iterator i = _models.begin(); i != _models.end(); ++i) {
		Model *m = *i;
		if (filename == m->_fname && *m->_cmap == *c) {
			return m;
		}
	}

	return loadModel(fname, c);
}

CMapPtr ResourceLoader::getColormap(const Common::String &fname) {
	Common::String filename = fname;
	filename.toLowercase();
	for (Common::List<CMap *>::const_iterator i = _colormaps.begin(); i != _colormaps.end(); ++i) {
		CMap *c = *i;
		if (filename.equals(c->_fname)) {
			return c;
		}
	}

	return loadColormap(fname);
}

KeyframeAnimPtr ResourceLoader::getKeyframe(const Common::String &fname) {
	Common::String filename = fname;
	filename.toLowercase();
	for (Common::List<KeyframeAnim *>::const_iterator i = _keyframeAnims.begin(); i != _keyframeAnims.end(); ++i) {
		KeyframeAnim *k = *i;
		if (filename == k->getFilename()) {
			return k;
		}
	}

	return loadKeyframe(fname);
}

FontPtr ResourceLoader::getFont(const Common::String &fname) {
	Common::String filename = fname;
	filename.toLowercase();
	for (Common::List<Font *>::const_iterator i = _fonts.begin(); i != _fonts.end(); ++i) {
		Font *f = *i;
		if (filename == f->getFilename()) {
			return f;
		}
	}

	Font *f = loadFont(fname);
	_fonts.push_back(f);

	return f;
}

LipSyncPtr ResourceLoader::getLipSync(const Common::String &fname) {
	Common::String filename = fname;
	filename.toLowercase();
	for (Common::List<LipSync *>::const_iterator i = _lipsyncs.begin(); i != _lipsyncs.end(); ++i) {
		LipSync *l = *i;
		if (filename == l->getFilename()) {
			return l;
		}
	}

	return loadLipSync(fname);
}

} // end of namespace Grim
