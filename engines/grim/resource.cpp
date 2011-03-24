/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
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
#include "engines/grim/actor.h"


namespace Grim {

ResourceLoader *g_resourceloader = NULL;

ResourceLoader::ResourceLoader() {
	int lab_counter = 0;
	_cacheDirty = false;
	_cacheMemorySize = 0;

	Lab *l;
	Common::ArchiveMemberList files;

	SearchMan.listMatchingMembers(files, "*.lab");

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

	if (g_grim->getGameFlags() & GF_DEMO) {
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

ResourceLoader::~ResourceLoader() {
	for (LabList::const_iterator i = _labs.begin(); i != _labs.end(); ++i)
		delete *i;
}

const Lab *ResourceLoader::getLab(const char *filename) const {
	for (LabList::const_iterator i = _labs.begin(); i != _labs.end(); ++i)
		if ((*i)->fileExists(filename))
			return *i;

	return NULL;
}

static int sortCallback(const void *entry1, const void *entry2) {
	return strcasecmp(((ResourceLoader::ResourceCache *)entry1)->fname, ((ResourceLoader::ResourceCache *)entry2)->fname);
}

Block *ResourceLoader::getFileFromCache(const char *filename) {
	ResourceLoader::ResourceCache *entry = getEntryFromCache(filename);
	if (entry)
		return entry->resPtr;
	else
		return NULL;
}

ResourceLoader::ResourceCache *ResourceLoader::getEntryFromCache(const char *filename) {
	if (_cache.empty())
		return NULL;

	if (_cacheDirty) {
		qsort(_cache.begin(), _cache.size(), sizeof(ResourceCache), sortCallback);
		_cacheDirty = false;
	}

	ResourceCache key;
	key.fname = (char *)filename;

	return (ResourceLoader::ResourceCache *)bsearch(&key, _cache.begin(), _cache.size(), sizeof(ResourceCache), sortCallback);
}

bool ResourceLoader::fileExists(const char *filename) const {
	return getLab(filename) != NULL;
}

Block *ResourceLoader::getFileBlock(const char *filename) const {
	const Lab *l = getLab(filename);
	if (!l)
		return NULL;
	else
		return l->getFileBlock(filename);
}

Block *ResourceLoader::getBlock(const char *filename) {
    Common::String fname = filename;
    fname.toLowercase();
    Block *b = getFileFromCache(fname.c_str());
    if (!b) {
        b = getFileBlock(fname.c_str());
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

int ResourceLoader::fileLength(const char *filename) const {
	const Lab *l = getLab(filename);
	if (l)
		return l->fileLength(filename);
	else
		return 0;
}

void ResourceLoader::putIntoCache(Common::String fname, Block *res) {
	ResourceCache entry;
	entry.resPtr = res;
	entry.fname = new char[fname.size() + 1];
	strcpy(entry.fname, fname.c_str());
	_cacheMemorySize += res->len();
	_cache.push_back(entry);
	_cacheDirty = true;
}

Bitmap *ResourceLoader::loadBitmap(const char *filename) {
	Common::String fname = filename;
	fname.toLowercase();
	Block *b = getFileFromCache(fname.c_str());
	if (!b) {
		b = getFileBlock(fname.c_str());
		if (!b) {	// Grim sometimes asks for non-existant bitmaps (eg, ha_overhead)
			warning("Could not find bitmap %s", filename);
			return NULL;
		}
		putIntoCache(fname, b);
	}

	Bitmap *result = g_grim->registerBitmap(filename, b->data(), b->len());
	_bitmaps.push_back(result);

	return result;
}

CMap *ResourceLoader::loadColormap(const char *filename) {
	Common::String fname = filename;
	fname.toLowercase();
	Block *b = getFileFromCache(fname.c_str());
	if (!b) {
		b = getFileBlock(fname.c_str());
		if (!b) {
			error("Could not find colormap %s", filename);
        }
		putIntoCache(fname, b);
	}

	CMap *result = new CMap(filename, b->data(), b->len());
	_colormaps.push_back(result);

	return result;
}

Costume *ResourceLoader::loadCostume(const char *filename, Costume *prevCost) {
	Common::String fname = filename;
	fname.toLowercase();
	Block *b = getFileFromCache(fname.c_str());
	if (!b) {
		b = getFileBlock(fname.c_str());
		if (!b)
			error("Could not find costume \"%s\"", filename);
		putIntoCache(fname, b);
	}
	Costume *result = new Costume(filename, b->data(), b->len(), prevCost);
	_costumes.push_back(result);

	return result;
}

Font *ResourceLoader::loadFont(const char *filename) {
	Common::String fname = filename;
	fname.toLowercase();
	Block *b = getFileFromCache(fname.c_str());
	if (!b) {
		b = getFileBlock(fname.c_str());
		if (!b)
			error("Could not find font file %s", filename);
		putIntoCache(fname, b);
	}

	Font *result = new Font(filename, b->data(), b->len());
	_fonts.push_back(result);

	return result;
}

KeyframeAnim *ResourceLoader::loadKeyframe(const char *filename) {
	Common::String fname = filename;
	fname.toLowercase();
	Block *b = getFileFromCache(fname.c_str());
	if (!b) {
		b = getFileBlock(fname.c_str());
		if (!b)
			error("Could not find keyframe file %s", filename);
		putIntoCache(fname, b);
	}

	KeyframeAnim *result = new KeyframeAnim(filename, b->data(), b->len());
	_keyframeAnims.push_back(result);

	return result;
}

LipSync *ResourceLoader::loadLipSync(const char *filename) {
	Common::String fname = filename;
	fname.toLowercase();
	LipSync *result;
	Block *b = getFileFromCache(fname.c_str());
	if (!b) {
		b = getFileBlock(fname.c_str());
		if (!b)
			return NULL;
	}

	result = new LipSync(filename, b->data(), b->len());

	// Some lipsync files have no data
	if (result->isValid()) {
		putIntoCache(fname, b);
		_lipsyncs.push_back(result);
	} else {
		delete result;
		delete b;
		result = NULL;
	}

	return result;
}

Material *ResourceLoader::loadMaterial(const char *filename, CMap *c) {
	Common::String fname = Common::String(filename);
	fname.toLowercase();
	Block *b = getFileFromCache(fname.c_str());
	if (!b) {
		b = getFileBlock(fname.c_str());
		if (!b)
			error("Could not find material %s", filename);
		putIntoCache(fname, b);
	}

	Material *result = new Material(fname.c_str(), b->data(), b->len(), c);
	_materials.push_back(result);

	return result;
}

Model *ResourceLoader::loadModel(const char *filename, CMap *c) {
	Common::String fname = filename;
	fname.toLowercase();
	Block *b = getFileFromCache(fname.c_str());
	if (!b) {
		b = getFileBlock(fname.c_str());
		if (!b)
			error("Could not find model %s", filename);
		putIntoCache(fname, b);
	}

	Model *result = new Model(filename, b->data(), b->len(), c);
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
			_cacheMemorySize -= _cache[i].resPtr->len();
			delete _cache[i].resPtr;
			_cache.remove_at(i);
			_cacheDirty = true;
		}
	}
}

void ResourceLoader::uncacheMaterial(Material *mat) {
	_materials.remove(mat);
}

void ResourceLoader::uncacheBitmap(Bitmap *bitmap) {
	_bitmaps.remove(bitmap);
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

void ResourceLoader::uncacheCostume(Costume *c) {
	_costumes.remove(c);
}

void ResourceLoader::uncacheLipSync(LipSync *s) {
	_lipsyncs.remove(s);
}

MaterialPtr ResourceLoader::getMaterial(const char *fname, CMap *c) {
	for (Common::List<Material *>::const_iterator i = _materials.begin(); i != _materials.end(); ++i) {
		Material *m = *i;
		if (strcmp(fname, m->_fname.c_str()) == 0 && *m->_cmap == *c) {
			return m;
		}
	}

	return loadMaterial(fname, c);
}

BitmapPtr ResourceLoader::getBitmap(const char *fname) {
	for (Common::List<Bitmap *>::const_iterator i = _bitmaps.begin(); i != _bitmaps.end(); ++i) {
		Bitmap *b = *i;
		if (strcmp(fname, b->filename()) == 0) {
			return b;
		}
	}

	return loadBitmap(fname);
}

ModelPtr ResourceLoader::getModel(const char *fname, CMap *c) {
	for (Common::List<Model *>::const_iterator i = _models.begin(); i != _models.end(); ++i) {
		Model *m = *i;
		if (strcmp(fname, m->_fname.c_str()) == 0 && *m->_cmap == *c) {
			return m;
		}
	}

	return loadModel(fname, c);
}

CMapPtr ResourceLoader::getColormap(const char *fname) {
	for (Common::List<CMap *>::const_iterator i = _colormaps.begin(); i != _colormaps.end(); ++i) {
		CMap *c = *i;
		if (strcmp(fname, c->_fname.c_str()) == 0) {
			return c;
		}
	}

	return loadColormap(fname);
}

KeyframeAnimPtr ResourceLoader::getKeyframe(const char *fname) {
	for (Common::List<KeyframeAnim *>::const_iterator i = _keyframeAnims.begin(); i != _keyframeAnims.end(); ++i) {
		KeyframeAnim *k = *i;
		if (strcmp(fname, k->filename()) == 0) {
			return k;
		}
	}

	return loadKeyframe(fname);
}

FontPtr ResourceLoader::getFont(const char *fname) {
	for (Common::List<Font *>::const_iterator i = _fonts.begin(); i != _fonts.end(); ++i) {
		Font *f = *i;
		if (strcmp(fname, f->getFilename().c_str()) == 0) {
			return f;
		}
	}

	return loadFont(fname);
}

CostumePtr ResourceLoader::getCostume(const char *fname, Costume *prev) {
	for (Common::List<Costume *>::const_iterator i = _costumes.begin(); i != _costumes.end(); ++i) {
		Costume *c = *i;
		if (strcmp(fname, c->filename()) == 0) {
			return c;
		}
	}

	return loadCostume(fname, prev);
}

LipSyncPtr ResourceLoader::getLipSync(const char *fname) {
	for (Common::List<LipSync *>::const_iterator i = _lipsyncs.begin(); i != _lipsyncs.end(); ++i) {
		LipSync *l = *i;
		if (strcmp(fname, l->filename()) == 0) {
			return l;
		}
	}

	return loadLipSync(fname);
}

} // end of namespace Grim
